/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: options.cxx,v $
 * $Revision: 1.17 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_idlc.hxx"

#include <stdio.h>
#include /*MSVC trouble: <cstring>*/ <string.h>
#include <idlc/options.hxx>

using namespace rtl;

Options::Options(): m_stdin(false)
{
}

Options::~Options()
{
}

sal_Bool Options::initOptions(int ac, char* av[], sal_Bool bCmdFile)
    throw( IllegalArgument )
{
    sal_Bool    ret = sal_True;
    sal_uInt16  j=0;

    if (!bCmdFile)
    {
        bCmdFile = sal_True;

        m_program = av[0];

        if (ac < 2)
        {
            fprintf(stderr, "%s", prepareHelp().getStr());
            ret = sal_False;
        }

        j = 1;
    } else
    {
        j = 0;
    }

    char    *s=NULL;
    for (; j < ac; j++)
    {
        if (av[j][0] == '-')
        {
            switch (av[j][1])
            {
            case 'O':
                if (av[j][2] == '\0')
                {
                    if (j < ac - 1 && av[j+1][0] != '-')
                    {
                        j++;
                        s = av[j];
                    } else
                    {
                        OString tmp("'-O', please check");
                        if (j <= ac - 1)
                        {
                            tmp += " your input '" + OString(av[j+1]) + "'";
                        }

                        throw IllegalArgument(tmp);
                    }
                } else
                {
                    s = av[j] + 2;
                }

                m_options["-O"] = OString(s);
                break;
            case 'I':
            {
                if (av[j][2] == '\0')
                {
                    if (j < ac - 1 && av[j+1][0] != '-')
                    {
                        j++;
                        s = av[j];
                    } else
                    {
                        OString tmp("'-I', please check");
                        if (j <= ac - 1)
                        {
                            tmp += " your input '" + OString(av[j+1]) + "'";
                        }

                        throw IllegalArgument(tmp);
                    }
                } else
                {
                    s = av[j] + 2;
                }

                OString inc(s);
                if ( inc.indexOf(';') > 0 )
                {
                    OString tmp(s);
                    sal_Int32 nIndex = 0;
                    inc = OString();
                    do inc = inc + " -I\"" + tmp.getToken( 0, ';', nIndex ) +"\""; while( nIndex != -1 );
                } else
                    inc = OString("-I\"") + s + "\"";

                if (m_options.count("-I") > 0)
                {
                    OString tmp(m_options["-I"]);
                    tmp = tmp + " " + inc;
                    m_options["-I"] = tmp;
                } else
                {
                    m_options["-I"] = inc;
                }
            }
            break;
            case 'D':
                if (av[j][2] == '\0')
                {
                    if (j < ac - 1 && av[j+1][0] != '-')
                    {
                        j++;
                        s = av[j];
                    } else
                    {
                        OString tmp("'-D', please check");
                        if (j <= ac - 1)
                        {
                            tmp += " your input '" + OString(av[j+1]) + "'";
                        }

                        throw IllegalArgument(tmp);
                    }
                } else
                {
                    s = av[j];
                }

                if (m_options.count("-D") > 0)
                {
                    OString tmp(m_options["-D"]);
                    tmp = tmp + " " + s;
                    m_options["-D"] = tmp;
                } else
                    m_options["-D"] = OString(s);
                break;
            case 'C':
                if (av[j][2] != '\0')
                {
                    throw IllegalArgument(OString(av[j]) + ", please check your input");
                }
                if (m_options.count("-C") == 0)
                    m_options["-C"] = OString(av[j]);
                break;
            case 'c':
                if (av[j][2] == 'i' && av[j][3] == 'd' && av[j][4] == '\0')
                {
                    if (m_options.count("-cid") == 0)
                        m_options["-cid"] = OString(av[j]);
                } else
                    throw IllegalArgument(OString(av[j]) + ", please check your input");
                break;
            case 'w':
                if (av[j][2] == 'e' && av[j][3] == '\0') {
                    if (m_options.count("-we") == 0)
                        m_options["-we"] = OString(av[j]);
                } else {
                    if (av[j][2] == '\0') {
                        if (m_options.count("-w") == 0)
                            m_options["-w"] = OString(av[j]);
                    } else
                        throw IllegalArgument(OString(av[j]) + ", please check your input");
                }
                break;
            case 'h':
            case '?':
                if (av[j][2] != '\0')
                {
                    throw IllegalArgument(OString(av[j]) + ", please check your input");
                } else
                {
                    fprintf(stdout, "%s", prepareHelp().getStr());
                    exit(0);
                }
            case 's':
                if (/*MSVC trouble: std::*/strcmp(&av[j][2], "tdin") == 0)
                {
                    m_stdin = true;
                    break;
                }
                // fall through
            default:
                throw IllegalArgument("the option is unknown" + OString(av[j]));
            }
        } else
        {
            if (av[j][0] == '@')
            {
                FILE* cmdFile = fopen(av[j]+1, "r");
                  if( cmdFile == NULL )
                  {
                    fprintf(stderr, "%s", prepareHelp().getStr());
                    ret = sal_False;
                } else
                {
                    int rargc=0;
                    char* rargv[512];
                    char  buffer[512]="";

                    int i=0;
                    int found = 0;
                    char c;
                    while ( fscanf(cmdFile, "%c", &c) != EOF )
                    {
                        if (c=='\"') {
                            if (found) {
                                found=0;
                            } else {
                                found=1;
                                continue;
                            }
                        } else {
                            if (c!=13 && c!=10) {
                                if (found || c!=' ') {
                                    buffer[i++]=c;
                                    continue;
                                }
                            }
                            if (i==0)
                                continue;
                        }
                        buffer[i]='\0';
                        found=0;
                        i=0;
                        rargv[rargc]= strdup(buffer);
                        rargc++;
                        buffer[0]='\0';
                    }
                    if (buffer[0] != '\0') {
                        buffer[i]='\0';
                        rargv[rargc]= strdup(buffer);
                        rargc++;
                    }
                    fclose(cmdFile);

                    ret = initOptions(rargc, rargv, bCmdFile);

                    long ii = 0;
                    for (ii=0; ii < rargc; ii++)
                    {
                        free(rargv[ii]);
                    }
                }
            } else
            {
                OString name(av[j]);
                name = name.toAsciiLowerCase();
                if ( name.lastIndexOf(".idl") != (name.getLength() - 4) )
                {
                    throw IllegalArgument("'" + OString(av[j]) +
                        "' is not a valid input file, only '*.idl' files will be accepted");
                }
                m_inputFiles.push_back(av[j]);
            }
        }
    }

    return ret;
}

OString Options::prepareHelp()
{
    OString help("\nusing: ");
    help += m_program
        + " [-options] <file_1> ... <file_n> | @<filename> | -stdin\n";
    help += "    <file_n>    = file_n specifies one or more idl files.\n";
    help += "                  Only files with the extension '.idl' are valid.\n";
    help += "    @<filename> = filename specifies the name of a command file.\n";
    help += "    -stdin      = read idl file from standard input.\n";
    help += "  Options:\n";
    help += "    -O<path>    = path specifies the output directory.\n";
    help += "                  The generated output is a registry file with\n";
    help += "                  the same name as the idl input file (or 'stdin'\n";
    help += "                  for -stdin).\n";
    help += "    -I<path>    = path specifies a directory where include\n";
    help += "                  files will be searched by the preprocessor.\n";
    help += "                  Multiple directories can be combined with ';'.\n";
    help += "    -D<name>    = name defines a macro for the preprocessor.\n";
    help += "    -C          = generate complete type information, including\n";
    help += "                  documentation.\n";
    help += "    -cid        = check if identifiers fulfill the UNO naming\n";
    help += "                  requirements.\n";
    help += "    -w          = display warning messages.\n";
    help += "    -we         = treat warnings as errors.\n";
    help += "    -h|-?       = print this help message and exit.\n";
    help += prepareVersion();

    return help;
}

OString Options::prepareVersion()
{
    OString version("\nSun Microsystems (R) ");
    version += m_program + " Version 1.1\n\n";
    return version;
}

const OString& Options::getProgramName() const
{
    return m_program;
}

sal_uInt16 Options::getNumberOfOptions() const
{
    return (sal_uInt16)(m_options.size());
}

sal_Bool Options::isValid(const OString& option)
{
    return (m_options.count(option) > 0);
}

const OString Options::getOption(const OString& option)
    throw( IllegalArgument )
{
    if (m_options.count(option) > 0)
    {
        return m_options[option];
    } else
    {
        throw IllegalArgument("Option is not valid or currently not set.");
    }
}

const OptionMap& Options::getOptions()
{
    return m_options;
}
