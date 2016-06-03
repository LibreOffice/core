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

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "minidump_upload path_to_ini_file" << std::endl;
        return EXIT_FAILURE;
    }

    std::string iniPath(argv[1]);
    if (!readConfig(iniPath))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
