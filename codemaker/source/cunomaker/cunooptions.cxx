/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_codemaker.hxx"
#include    <stdio.h>

#include    "cunooptions.hxx"

using namespace rtl;

sal_Bool CunoOptions::initOptions(int ac, char* av[], sal_Bool bCmdFile)
    throw( IllegalArgument )
{
    sal_Bool    ret = sal_True;
    sal_uInt16  i=0;

    if (!bCmdFile)
    {
        bCmdFile = sal_True;

        m_program = av[0];

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
    for (i; i < ac; i++)
    {
        if (av[i][0] == '-')
        {
            switch (av[i][1])
            {
                case 'O':
                    if (av[i][2] == 'C')
                    {
                        if (av[i][3] == '\0')
                        {
                            if (i < ac - 1 && av[i+1][0] != '-')
                            {
                                i++;
                                s = av[i];
                            } else
                            {
                                OString tmp("'-OC', please check");
                                if (i <= ac - 1)
                                {
                                    tmp += " your input '" + OString(av[i+1]) + "'";
                                }

                                throw IllegalArgument(tmp);
                            }
                        } else
                        {
                            s = av[i] + 3;
                        }

                        m_options["-OC"] = OString(s);
                        break;
                    } else
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
                case 'U':
                    if (av[i][2] != '\0')
                    {
                        OString tmp("'-U', please check");
                        if (i <= ac - 1)
                        {
                            tmp += " your input '" + OString(av[i]) + "'";
                        }

                        throw IllegalArgument(tmp);
                    }

                    m_options["-U"] = OString("");
                    break;
/*
                case 'L':
                    if (av[i][2] != '\0')
                    {
                        OString tmp("'-L', please check");
                        if (i <= ac - 1)
                        {
                            tmp += " your input '" + OString(av[i]) + "'";
                        }

                        throw IllegalArgument(tmp);
                    }

                    if (isValid("-C") || isValid("-CS"))
                    {
                        OString tmp("'-L' could not be combined with '-C' or '-CS' option");
                        throw IllegalArgument(tmp);
                    }
                    m_options["-L"] = OString("");
                    break;
*/
                case 'C':
                    if (av[i][2] != '\0')
                    {
                        OString tmp("'-C', please check");
                        if (i <= ac - 1)
                        {
                            tmp += " your input '" + OString(av[i]) + "'";
                        }

                        throw IllegalArgument(tmp);
                    }

                    if (isValid("-L") || isValid("-CS"))
                    {
                        OString tmp("'-C' could not be combined with '-L' or '-CS' option");
                        throw IllegalArgument(tmp);
                    }
                    m_options["-C"] = OString("");
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
                    } else
                    if (av[i][2] != '\0')
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
                default:
                    throw IllegalArgument("the option is unknown" + OString(av[i]));
                    break;
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

                    for (long i=0; i < rargc; i++)
                    {
                        free(rargv[i]);
                    }
                }
            } else
            {
                m_inputFiles.push_back(av[i]);
            }
        }
    }

    return ret;
}

OString CunoOptions::prepareHelp()
{
    OString help("\nusing: ");
    help += m_program + " [-options] file_1 ... file_n\nOptions:\n";
    help += "    -O<path>   = path describes the root directory for the generated output.\n";
    help += "                 The output directory tree is generated under this directory.\n";
    help += "    -T<name>   = name specifies a type or a list of types. The output for this\n";
    help += "      [t1;...]   type is generated. If no '-T' option is specified,\n";
    help += "                 then output for all types is generated.\n";
    help += "                 Example: 'com.sun.star.uno.XInterface' is a valid type.\n";
    help += "    -B<name>   = name specifies the base node. All types are searched under this\n";
    help += "                 node. Default is the root '/' of the registry files.\n";
    help += "    -U         = activate the generating of a getCppuType_<name> function.\n";
//  help += "    -L         = getCppuType function with a known leak.\n";
    help += "    -C         = getCppuType_<name> function keeps comprehensive type information.\n";
    help += "    -G         = generate only target files which does not exists.\n";
    help += "    -Gc        = generate only target files which content will be changed.\n";
    help += prepareVersion();

    return help;
}

OString CunoOptions::prepareVersion()
{
    OString version(m_program);
    version += " Version 1.0\n\n";
    return version;
}


