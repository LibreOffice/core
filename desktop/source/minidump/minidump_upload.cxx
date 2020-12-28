/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <desktop/minidump.hxx>

#include <iostream>
#include <string>

#ifdef _WIN32
#include <memory>
#include <windows.h>

int wmain(int argc, wchar_t** argv)
#else
int main(int argc, char** argv)
#endif
{
    if (argc < 2)
    {
        std::cerr << "minidump_upload path_to_ini_file" << std::endl;
        return EXIT_FAILURE;
    }

#ifdef _WIN32
    const int nBytes = WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, nullptr, 0, nullptr, nullptr);
    auto buf = std::make_unique<char[]>(nBytes);
    if (WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, buf.get(), nBytes, nullptr, nullptr) == 0)
        return EXIT_FAILURE;
    std::string iniPath(buf.get());
#else
    std::string iniPath(argv[1]);
#endif
    std::string response;
    if (!crashreport::readConfig(iniPath, &response))
        return EXIT_FAILURE;

    std::cout << "Response: " << response << std::endl;
    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
