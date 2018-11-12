/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <stdio.h>
#include <string.h>
#include "javaoptions.hxx"
#include <osl/process.h>
#include <osl/thread.h>


#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

bool JavaOptions::initOptions(int ac, char* av[], bool bCmdFile)
{
    bool    ret = true;
    int i=0;

    if (!bCmdFile)
    {
        bCmdFile = true;

        OString name(av[0]);
        sal_Int32 index = name.lastIndexOf(SEPARATOR);
        m_program = name.copy(index > 0 ? index+1 : 0);

        if (ac < 2)
        {
            fprintf(stderr, "%s", prepareHelp().getStr());
            ret = false;
        }

        i = 1;
    }

    char    *s=nullptr;
    for( ; i < ac; i++)
    {
        if (av[i][0] == '-')
        {
            switch (av[i][1])
            {
                case 'O':
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            OString tmp("'-O', please check");
                            if (i <= ac - 1)
                            {
                                tmp += " your input '" + OString(av[i+1]) + "'";
                            }

                            throw IllegalArgument(tmp);
                        }
                    } else
                    {
                        s = av[i] + 2;
                    }

                    m_options["-O"] = OString(s);
                    break;
                case 'n':
                    if (av[i][2] != 'D' || av[i][3] != '\0')
                    {
                        OString tmp("'-nD', please check your input '" + OString(av[i]) + "'");
                        throw IllegalArgument(tmp);
                    }

                    m_options["-nD"] = OString();
                    break;
                case 'T':
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            OString tmp("'-T', please check");
                            if (i <= ac - 1)
                            {
                                tmp += " your input '" + OString(av[i+1]) + "'";
                            }

                            throw IllegalArgument(tmp);
                        }
                    } else
                    {
                        s = av[i] + 2;
                    }

                    if (m_options.count("-T") > 0)
                    {
                        OString tmp(m_options["-T"]);
                        tmp = tmp + ";" + s;
                        m_options["-T"] = tmp;
                    } else
                    {
                        m_options["-T"] = OString(s);
                    }
                    break;
                case 'G':
                    if (av[i][2] == 'c')
                    {
                        if (av[i][3] != '\0')
                        {
                            OString tmp("'-Gc', please check");
                            if (i <= ac - 1)
                            {
                                tmp += " your input '" + OString(av[i]) + "'";
                            }

                            throw IllegalArgument(tmp);
                        }

                        m_options["-Gc"] = OString();
                        break;
                    } else if (av[i][2] != '\0')
                    {
                        OString tmp("'-G', please check");
                        if (i <= ac - 1)
                        {
                            tmp += " your input '" + OString(av[i]) + "'";
                        }

                        throw IllegalArgument(tmp);
                    }

                    m_options["-G"] = OString();
                    break;
                case 'X': // support for eXtra type rdbs
                {
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            OString tmp("'-X', please check");
                            if (i <= ac - 1)
                            {
                                tmp += " your input '" + OString(av[i+1]) + "'";
                            }

                            throw IllegalArgument(tmp);
                        }
                    } else
                    {
                        s = av[i] + 2;
                    }

                    m_extra_input_files.emplace_back(s );
                    break;
                }

                default:
                    throw IllegalArgument("the option is unknown" + OString(av[i]));
            }
        } else
        {
            if (av[i][0] == '@')
            {
                FILE* cmdFile = fopen(av[i]+1, "r");
                if( cmdFile == nullptr )
                {
                    fprintf(stderr, "%s", prepareHelp().getStr());
                    ret = false;
                } else
                {
                    int rargc=0;
                    char* rargv[512];
                    char  buffer[512];

                    while (fscanf(cmdFile, "%511s", buffer) != EOF && rargc < 512)
                    {
                        rargv[rargc]= strdup(buffer);
                        rargc++;
                    }
                    fclose(cmdFile);

                    ret = initOptions(rargc, rargv, bCmdFile);

                    for (int j=0; j < rargc; j++)
                    {
                        free(rargv[j]);
                    }
                }
            } else
            {
                m_inputFiles.emplace_back(av[i]);
            }
        }
    }

    return ret;
}

OString JavaOptions::prepareHelp()
{
    OString help("\nusing: ");
    help += m_program + " [-options] file_1 ... file_n -Xfile_n+1 -Xfile_n+2\nOptions:\n"
          "    -O<path>   = path describes the root directory for the generated output.\n"
          "                 The output directory tree is generated under this directory.\n"
          "    -T<name>   = name specifies a type or a list of types. The output for this\n"
          "      [t1;...]   type and all dependent types are generated. If no '-T' option is\n"
          "                 specified, then output for all types is generated.\n"
          "                 Example: 'com.sun.star.uno.XInterface' is a valid type.\n"
          "    -nD        = no dependent types are generated.\n"
          "    -G         = generate only target files which does not exists.\n"
          "    -Gc        = generate only target files which content will be changed.\n"
          "    -X<file>   = extra types which will not be taken into account for generation.\n\n";
    help += prepareVersion();

    return help;
}

OString JavaOptions::prepareVersion() const
{
    OString version(m_program);
    version += " Version 2.0\n\n";
    return version;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
