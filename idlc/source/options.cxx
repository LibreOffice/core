/*************************************************************************
 *
 *  $RCSfile: options.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-19 09:31:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <stdio.h>

#ifndef _IDLC_OPTIONS_HXX_
#include <idlc/options.hxx>
#endif

using namespace rtl;

Options::Options()
{
}

Options::~Options()
{

}

sal_Bool Options::initOptions(int ac, char* av[], sal_Bool bCmdFile)
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
                case 'I':
                {
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            OString tmp("'-I', please check");
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

                    OString inc(s);
                    if ( inc.indexOf(';') > 0 )
                    {
                        OString tmp(s);
                        sal_Int32 count = tmp.getTokenCount(';');
                        inc = OString();
                        for (sal_Int32 i=0; i < count; i++)
                            inc = inc + " -I" + tmp.getToken(i, ';');
                    } else
                        inc = OString("-I") + s;

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
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            OString tmp("'-D', please check");
                            if (i <= ac - 1)
                            {
                                tmp += " your input '" + OString(av[i+1]) + "'";
                            }

                            throw IllegalArgument(tmp);
                        }
                    } else
                    {
                        s = av[i];
                    }

                    if (m_options.count("-D") > 0)
                    {
                        OString tmp(m_options["-D"]);
                        tmp = tmp + " " + s;
                        m_options["-D"] = tmp;
                    } else
                    {
                        m_options["-D"] = OString(s);
                    }
                    break;
                case 'C':
                    if (av[i][2] != '\0')
                    {
                        throw IllegalArgument(OString(av[i]) + ", please check your input");
                    }
                    if (m_options.count("-C") == 0)
                    {
                        m_options["-C"] = OString(s);
                    }
                    break;
                case 'h':
                case '?':
                    if (av[i][2] != '\0')
                    {
                        throw IllegalArgument(OString(av[i]) + ", please check your input");
                    } else
                    {
                        fprintf(stdout, "%s", prepareHelp().getStr());
                        exit(0);
                    }
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
                OString name(av[i]);
                name = name.toLowerCase();
                if ( name.lastIndexOf(".idl") != (name.getLength() - 4) )
                {
                    throw IllegalArgument("'" + OString(av[i]) +
                        "' is not a valid input file, only '*.idl' files will be accepted");
                }
                m_inputFiles.push_back(av[i]);
            }
        }
    }

    return ret;
}

OString Options::prepareHelp()
{
    OString help("\nusing: ");
    help += m_program + " [-options] file_1 ... file_n | @<filename>\n";
    help += "    file_n      = file_n specifies one or more idl files.\n";
    help += "                  Only files with the extension '.idl' are valid.\n";
    help += "    @<filename> = filename specifies the name of a command file.\n";
    help += "  Options:\n";
    help += "    -O<path>   = path describes the output directory.\n";
    help += "                 The generated output is a registry file with\n";
    help += "                 the same name as the idl input file.\n";
    help += "    -I<path>   = path specifies a directory where are include\n";
    help += "                 files will be searched by the preprocessor.\n";
    help += "                 Multible directories could be combined with ';'.\n";
    help += "    -D<name>   = name defines a macro for the preprocessor.\n";
    help += "    -C         = generate complete type information, including\n";
    help += "                 additional service information and documentation.\n";
    help += "    -h|-?      = print this help message and exit.\n";
    help += prepareVersion();

    return help;
}

OString Options::prepareVersion()
{
    OString version("\nSun Microsystems (R) ");
    version += m_program + " Version 1.0\n\n";
    return version;
}

const OString& Options::getProgramName() const
{
    return m_program;
}

sal_uInt16 Options::getNumberOfOptions() const
{
    return m_options.size();
}

sal_Bool Options::isValid(const OString& option)
{
    return (m_options.count(option) > 0);
}

const OString Options::getOption(const OString& option)
    throw( IllegalArgument )
{
    const OString ret;

    if (m_options.count(option) > 0)
    {
        return m_options[option];
    } else
    {
        throw IllegalArgument("Option is not valid or currently not set.");
    }

    return ret;
}

const OptionMap& Options::getOptions()
{
    return m_options;
}

sal_uInt16 Options::getNumberOfInputFiles() const
{
    return m_inputFiles.size();
}

const OString Options::getInputFile(sal_uInt16 index)
    throw( IllegalArgument )
{
    const OString ret;

    if (index < m_inputFiles.size())
    {
        return m_inputFiles[index];
    } else
    {
        throw IllegalArgument("index is out of bound.");
    }

    return ret;
}

const StringVector& Options::getInputFiles()
{
    return m_inputFiles;
}

