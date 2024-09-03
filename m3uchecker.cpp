#ifdef _WIN32
// make MS happy
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <codecvt>
#include <windows.h>
#include <io.h>
#define access _access
#define waccess _waccess
#define F_OK 0
#else
#include <locale>
#include <unistd.h>
#define waccess(pathname, mode) [](const wchar_t* filename, int accessMode) -> int { \
    char mb_filename[256]; \
    wcstombs(mb_filename, filename, sizeof(mb_filename)); \
    return access(mb_filename, accessMode); \
}(pathname, mode)
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

std::string DirNameOf(const std::string& fname) {
    size_t pos = fname.find_last_of("\\/");
    return ((std::string::npos == pos) ? "" : fname.substr(0, pos)) +
#ifdef _WIN32
        "\\"
#else
        "/"
#endif
        ;
}

int SetWorkingDirectory(const char* path) {
    if (path == NULL) {
        return -1; // Invalid path
    }

#ifdef _WIN32
    // Windows specific code
#ifdef SetCurrentDirectory
#undef SetCurrentDirectory
#define SetCurrentDirectory SetCurrentDirectoryA
#endif // SetCurrentDirectory
    if (SetCurrentDirectory(path) == 0) {
        return -1; // Error setting the directory
    }
    return 0;
#else
    // Unix-like system specific code
    if (chdir(path) != 0) {
        return -1; // Error setting the directory
    }
    return 0;
#endif
}

bool file_exists(const char* filename) {
    return access(filename, F_OK) == 0;
}

int wfile_exists(const wchar_t* filename) {
    return waccess(filename, F_OK) == 0;
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "m3u checker\nBy nullptr-0 since 2023." << std::endl;
    std::string inputPath;
    if (argc == 1)
    {
        std::cout << "input path to m3u file:" << std::endl;
        std::getline(std::cin, inputPath);
    }
    else if (argc == 2)
    {
        inputPath = argv[1];
    }
    else if (argc > 2)
    {
        std::cerr << "m3u checker: too many parameters" << std::endl;
        return 2;
    }
    inputPath.erase(std::remove(inputPath.begin(), inputPath.end(), '\"'), inputPath.end());
    std::fstream inputFile(inputPath, std::ios::in);
    if (!inputFile.is_open())
    {
        std::cerr << "m3u checker: unable to open " << inputPath << std::endl;
        return 3;
    }
    if (SetWorkingDirectory(DirNameOf(inputPath).c_str()))
    {
        std::cerr << "m3u checker: unable to set working directory to " << DirNameOf(inputPath) << std::endl;
        return 1;
    }
    size_t lineNo = 0;
    size_t numFound = 0;
    size_t numNotFound = 0;
    std::string curLine;
    while (std::getline(inputFile, curLine))
    {
        ++lineNo;
        if (curLine != "" && curLine[0] != '#')
        {
#ifdef _WIN32
            std::replace(curLine.begin(), curLine.end(), '/', '\\');
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::wstring curLineW = converter.from_bytes(curLine);
            if (!wfile_exists(curLineW.c_str()))
#else
            std::replace(curLine.begin(), curLine.end(), '\\', '/');
            if (!file_exists(curLine.c_str()))
#endif
            {
                std::cerr << "Line " << lineNo << " Not Found: " << curLine << std::endl;
                ++numNotFound;
                continue;
            }
            ++numFound;
        }
    }
    std::cout << "m3u checker: " << numFound << " media found, " << numNotFound << " media not found." << std::endl;
    return 0;
}
