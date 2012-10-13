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

#include    <stdio.h>
#include <string.h>
#include "javaoptions.hxx"
#include "osl/process.h"
#include "osl/thread.h"

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringToOString;

#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

sal_Bool JavaOptions::initOptions(int ac, char* av[], sal_Bool bCmdFile)
    throw( IllegalArgument )
{
    sal_Bool    ret = sal_True;
    sal_uInt16  i=0;

    if (!bCmdFile)
    {
        bCmdFile = sal_True;

        OString name(av[0]);
        sal_Int32 index = name.lastIndexOf(SEPARATOR);
        m_program = name.copy((index > 0 ? index+1 : 0));

        if (ac < 2)
        {
            fprintf(stderr, "%s", prepareHelp().getStr());
            ret = sal_False;
        }

        i = 1;
    } else
    {
        i = 0;
    }

    char    *s=NULL;
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
                case 'B':
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            OString tmp("'-B', please check");
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

                    m_options["-B"] = OString(s);
                    break;
                case 'n':
                    if (av[i][2] != 'D' || av[i][3] != '\0')
                    {
                        OString tmp("'-nD', please check");
                            tmp += " your input '" + OString(av[i]) + "'";
                        throw IllegalArgument(tmp);
                    }

                    m_options["-nD"] = OString("");
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

                        m_options["-Gc"] = OString("");
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

                    m_options["-G"] = OString("");
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

                    m_extra_input_files.push_back( s );
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
                  if( cmdFile == NULL )
                  {
                    fprintf(stderr, "%s", prepareHelp().getStr());
                    ret = sal_False;
                } else
                {
                    int rargc=0;
                    char* rargv[512];
                    char  buffer[512];

                    while ( fscanf(cmdFile, "%s", buffer) != EOF )
                    {
                        rargv[rargc]= strdup(buffer);
                        rargc++;
                    }
                    fclose(cmdFile);

                    ret = initOptions(rargc, rargv, bCmdFile);

                    for (long j=0; j < rargc; j++)
                    {
                        free(rargv[j]);
                    }
                }
            } else
            {
                if (bCmdFile)
                {
                    m_inputFiles.push_back(av[i]);
                } else
                {
                    OUString system_filepath;
                    if (osl_getCommandArg( i-1, &system_filepath.pData )
                        != osl_Process_E_None)
                    {
                        OSL_ASSERT(false);
                    }
                    m_inputFiles.push_back(OUStringToOString(system_filepath, osl_getThreadTextEncoding()));
                }
            }
        }
    }

    return ret;
}

OString JavaOptions::prepareHelp()
{
    OString help("\nusing: ");
    help += m_program + " [-options] file_1 ... file_n -Xfile_n+1 -Xfile_n+2\nOptions:\n";
    help += "    -O<path>   = path describes the root directory for the generated output.\n";
    help += "                 The output directory tree is generated under this directory.\n";
    help += "    -T<name>   = name specifies a type or a list of types. The output for this\n";
    help += "      [t1;...]   type and all dependent types are generated. If no '-T' option is \n";
    help += "                 specified, then output for all types is generated.\n";
    help += "                 Example: 'com.sun.star.uno.XInterface' is a valid type.\n";
    help += "    -B<name>   = name specifies the base node. All types are searched under this\n";
    help += "                 node. Default is the root '/' of the registry files.\n";
    help += "    -nD        = no dependent types are generated.\n";
    help += "    -G         = generate only target files which does not exists.\n";
    help += "    -Gc        = generate only target files which content will be changed.\n";
    help += "    -X<file>   = extra types which will not be taken into account for generation.\n\n";
    help += prepareVersion();

    return help;
}

OString JavaOptions::prepareVersion()
{
    OString version(m_program);
    version += " Version 2.0\n\n";
    return version;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
