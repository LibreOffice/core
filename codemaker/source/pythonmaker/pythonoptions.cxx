/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pythonoptions.hxx"

#include <iostream>
#include <vector>
#include <cstdio>
#include <cstring>

#ifdef SAL_UNX
#define LOCAL_SEPARATOR '/'
#else
#define LOCAL_SEPARATOR '\\'
#endif

PythonOptions::PythonOptions()
    : Options()
{
    m_program = "pythonmaker"_ostr;
}

bool PythonOptions::initOptions(int ac, char* av[], bool bCmdFile)
{
    if (!bCmdFile)
    {
        OString name(av[0]);
        sal_Int32 index = name.lastIndexOf(LOCAL_SEPARATOR);
        m_program = name.copy(index > 0 ? index + 1 : 0);

        if (ac < 2)
        {
            std::cerr << prepareHelp();
            return false;
        }
    }

    // Start parsing from index 1 if it's the main call, or 0 if it's a command file call
    for (int i = (bCmdFile ? 0 : 1); i < ac; ++i)
    {
        OString argument(av[i]);

        if (argument.startsWith("-"))
        {
            if (argument == "-O"_ostr)
            {
                if (i + 1 < ac && av[i + 1][0] != '-')
                {
                    m_options["-O"_ostr] = OString(av[++i]);
                }
                else
                {
                    throw IllegalArgument("'-O' option requires a path."_ostr);
                }
            }
            else if (argument == "-h"_ostr || argument == "--help"_ostr)
            {
                std::cout << prepareHelp();
                return true;
            }
            else if (argument == "-nD"_ostr)
            {
                m_options["-nD"_ostr] = OString();
            }
            else if (argument == "-T"_ostr)
            {
                if (i + 1 < ac && av[i + 1][0] != '-')
                {
                    const char* types = av[++i];
                    if (m_options.count("-T"_ostr) > 0)
                    {
                        m_options["-T"_ostr] += ";"_ostr + types;
                    }
                    else
                    {
                        m_options["-T"_ostr] = OString(types);
                    }
                }
                else
                {
                    throw IllegalArgument("'-T' option requires a type list."_ostr);
                }
            }
            else if (argument == "-G"_ostr)
            {
                m_options["-G"_ostr] = OString();
            }
            else if (argument == "-Gc"_ostr)
            {
                m_options["-Gc"_ostr] = OString();
            }
            else if (argument == "-X"_ostr)
            {
                if (i + 1 < ac && av[i + 1][0] != '-')
                {
                    m_extra_input_files.emplace_back(av[++i]);
                }
                else
                {
                    throw IllegalArgument("'-X' option requires a file path."_ostr);
                }
            }
            else if (argument == "-v"_ostr || argument == "--verbose"_ostr)
            {
                m_options["--verbose"_ostr] = "true"_ostr;
            }
            else if (argument == "--version")
            {
                std::cout << prepareVersion() << "\n";
                return true;
            }
            else
            {
                throw IllegalArgument("Unknown option: "_ostr + argument);
            }
        }
        else if (argument.startsWith("@"))
        {
            // Command file processing
            FILE* cmdFile = fopen(argument.copy(1).getStr(), "r");
            if (cmdFile == nullptr)
            {
                std::cerr << "Cannot open command file: " << argument.copy(1) << '\n';
                std::cerr << prepareHelp();
                return false;
            }

            std::vector<char*> rargv;
            char buffer[1024]; // Increased buffer size for safety
            while (fscanf(cmdFile, "%1023s", buffer) != EOF)
            {
                rargv.push_back(strdup(buffer));
            }
            fclose(cmdFile);

            if (!initOptions(rargv.size(), rargv.data(), true))
            {
                // Free memory and propagate failure
                for (char* arg : rargv)
                {
                    free(arg);
                }
                return false;
            }

            // Free memory
            for (char* arg : rargv)
            {
                free(arg);
            }
        }
        else
        {
            // It's an input file
            m_inputFiles.emplace_back(argument);
        }
    }

    if (!bCmdFile) // Perform final checks only on the top-level call
    {
        if (m_inputFiles.empty())
        {
            throw IllegalArgument("At least one .rdb input file must be specified."_ostr);
        }
        if (!isValid("-O"_ostr))
        {
            throw IllegalArgument("Output directory '-O <path>' must be specified."_ostr);
        }
    }

    return true;
}

OString PythonOptions::prepareHelp()
{
    OString programName = m_program;
    if (programName.isEmpty())
    {
        programName = "pythonmaker"_ostr;
    }

    OString help
        = prepareVersion() + "\n" + programName
          + " - UNO Type Library to Python Stub Generator\n\n"
            "About:\n"
            "  This tool generates Python stub files (.pyi) from UNO type libraries (.rdb).\n"
            "  These stubs enable static type checking and provide rich\n"
            "  auto-completion for the LibreOffice API in modern code editors.\n\n"
            "Usage:\n"
            "  "
          + programName
          + " -O <out_dir> [options] <input.rdb>...\n\n"

            "Required Arguments:\n"
            "  -O, --output-dir <path>\n"
            "      The root directory for the generated Python stub package. The necessary\n"
            "      module directories (e.g., 'com/sun/star/') will be created inside.\n\n"

            "Filtering and Dependency Options:\n"
            "  -T, --types <type_list>\n"
            "      A semicolon-separated list of types to generate. Wildcards are supported.\n"
            "      If omitted, all types from the primary input RDB files are generated.\n"
            "  -X, --extra-types <file.rdb>\n"
            "      Load an extra RDB for dependency resolution (e.g., for base classes)\n"
            "      without generating stubs for its types. Can be specified multiple times.\n"
            "  -nD\n"
            "      No dependent types. Only generates stubs for the types explicitly matched\n"
            "      by the -T filter.\n\n"

            "Generation Control:\n"
            "  -G              Generate a file only if it does not already exist.\n"
            "  -Gc             Generate a file only if its content would change.\n\n"

            "Other Options:\n"
            "  -v, --verbose   Enable verbose logging of processed types and created files.\n"
            "  -h, --help      Display this help message and exit.\n"
            "  --version       Display version information and exit.\n"
            "  @<cmdfile>      Read arguments from a file (one argument per line).\n\n"

            "Examples:\n\n"
            "  1. Generate all stubs from 'acme_api.rdb' into a 'stubs' directory:\n"
            "     "
          + programName
          + " -O ./stubs ./acme_api.rdb\n\n"
            "  2. Generate stubs for only the 'org.company.widgets' module:\n"
            "     "
          + programName
          + " -O ./stubs -T \"org.company.widgets.*\" ./acme_api.rdb\n\n"
            "  3. Generate a specific interface, using 'core_types.rdb' for dependencies\n"
            "     (like com.sun.star.uno.XInterface), without generating 'core_types.rdb' itself:\n"
            "     "
          + programName
          + " -O ./stubs -T org.company.widgets.XButton -X ./core_types.rdb ./acme_api.rdb\n"
          + "\n"_ostr;

    return help;
}

OString PythonOptions::prepareVersion() const { return m_program + " Version 1.0 \n"_ostr; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
