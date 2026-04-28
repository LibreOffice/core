// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <filesystem>
#include <ranges>
#include <set>
#include <string>

#include <Poco/URI.h>

#include <COKit/COKit.hxx>

#if defined _WIN32
#include <Windows.h>
#include <common/Util.hpp>
#endif

static std::set<std::string> parseCommaSeparatedList(const std::string& list)
{
    std::set<std::string> result;
    for (auto part : list | std::views::split(','))
        result.emplace(part.begin(), part.end());
    return result;
}

static void convert(kit::Office* office, const std::string& input, std::set<std::string> outputFormats)
{
#ifdef _WIN32
    const std::string stem = Util::wide_string_to_string(std::filesystem::path(input).stem().native());
#else
    const std::string stem = std::filesystem::path(input).stem().native();
#endif

    try
    {
        kit::Document* doc = office->documentLoad(Poco::URI(Poco::Path(input)).toString().c_str(), NULL /* options */);
        if (!doc)
        {
            std::cerr << "Error: could not load document: " << office->getError() << "\n";
            return;
        }

        for (const auto& format : outputFormats)
        {
            try
            {
                if (!doc->saveAs((stem + "." + format).c_str(), format.c_str(), NULL /* options */))
                {
                    std::cerr << "Error: could not export document as " << format
                              << ": " << office->getError() << "\n";
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error: COKit exception while exporting " << input
                          << " as " << format << ": " << e.what() << "\n";
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: COKit exception while loading " << input
                  << ": " << e.what() << "\n";
    }
}

// On Windows, we deploy the .exe file with the magic that makes the program use CP_UTF8 as its
// codepage, so command-line arguments come in as UTF-8, which nicely matches what happens on Linux
// and macOS.

int main(int argc, char** argv)
{
#if defined _WIN32

#elif defined __APPLE__

#else

#endif

    if (argc < 2)
    {
        std::cout << "Usage: coconvert format1,... input ...\n";
        return 1;
    }

    kit::Office* office = NULL;
    try
    {
        std::string kit_path;

#if defined _WIN32
        wchar_t fileName[1000];
        GetModuleFileNameW(NULL, fileName, sizeof(fileName) / sizeof(fileName[0]));

        // CoConvert.exe is put in the "program" folder so kit_path can be that folder.

        kit_path = Util::wide_string_to_string(std::wstring(fileName));
        kit_path.resize(kit_path.find_last_of('\\') + 1);

#elif defined __APPLE__

#else

#endif

        office = kit::kit_cpp_init(kit_path.c_str());
        if (!office)
        {
            std::cerr << "Error: could not initialize COKit\n";
            return 1;
        }

        std::set<std::string> outputFormats = parseCommaSeparatedList(argv[1]);

        for (int i = 2; i < argc; i++)
            convert(office, argv[i], outputFormats);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: COKit exception: " << e.what() << "\n";
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
