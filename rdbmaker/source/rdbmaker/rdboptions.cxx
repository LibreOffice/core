/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rdboptions.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:45:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include    <stdio.h>
#include    <string.h>

#include    "rdboptions.hxx"

using namespace rtl;

sal_Bool RdbOptions::initOptions(int ac, char* av[], sal_Bool bCmdFile)
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
    for (; i < ac; i++)
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
                case 'X':
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

                    m_options["-X"] = OString(s);
                    break;
                case 'R':
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            OString tmp("'-R', please check");
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

                    m_options["-R"] = OString(s);
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
                case 'b':
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            OString tmp("'-b', please check");
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

                    m_options["-b"] = OString(s);
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
                case 'F':
                    if (av[i][2] == 'T')
                    {
                        if (av[i][3] == '\0')
                        {
                            if (i < ac - 1 && av[i+1][0] != '-')
                            {
                                i++;
                                s = av[i];
                            } else
                            {
                                OString tmp("'-FT', please check");
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

                        if (m_options.count("-FT") > 0)
                        {
                            OString tmp(m_options["-FT"]);
                            tmp = tmp + ";" + s;
                            m_options["-FT"] = tmp;
                        } else
                        {
                            m_options["-FT"] = OString(s);
                        }
                    } else
                    {
                        if (av[i][2] == '\0')
                        {
                            if (i < ac - 1 && av[i+1][0] != '-')
                            {
                                i++;
                                s = av[i];
                            } else
                            {
                                OString tmp("'-F', please check");
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

                        m_options["-F"] = OString(s);
                    }
                    break;
                case 'L':
                    if (av[i][2] != '\0')
                    {
                        OString tmp("'-L', please check");
                        if (i <= ac - 1)
                        {
                            tmp += " your input '" + OString(av[i+1]) + "'";
                        }

                        throw IllegalArgument(tmp);
                    }

                    m_options["-L"] = OString();
                    m_generateTypeList = sal_True;
                    break;
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
                m_inputFiles.push_back(av[i]);
            }
        }
    }

    return ret;
}

OString RdbOptions::prepareHelp()
{
    OString help("\nusing: ");
    help += m_program + " [-options] (-R<regname> | file_1 [... file_n])\n";
    help += "The rdbmaker supports 2 modes:\n";
    help += " 1. using the internal UNO type description manager -> use -R<regname>\n"
            "    where regname specifies the type library used by the UNO type description manager\n"
            "    after UNO is bootstrapped. This option disables the use of any other type libraries.\n"
            "    The tpye library must be a valid product type library which means that all types are\n"
            "    stored under the global base node UCR (Uno Core Reflection data).\n";
    help += " 2. using one or more type library files -> use file_1 ... file_n\n"
            "    file_1 .. file_n specifies one or more valid type library files which are used to\n"
            "    find the needed type information. The used type libraries have to support the same base\n"
            "    node (-B option).\n";
    help += "Options:\n";
    help += "    -O<filename> = filename specifies the name of the generated registry\n";
    help += "                   or text file.\n";
    help += "    -L           = specifies that only a text file is generated with the\n";
    help += "                   names of the specified types and their dependencies.\n";
    help += "                   Default is that a registry file will be created\n";
//  help += "    -X<xmlfile>  = xmlfile specifies the name of an xml description where\n";
//  help += "                   all types are specified which will be generated.\n";
    help += "    -T<name>     = name specifies a type or a list of types. The output for\n";
    help += "      [t1;...]     this type is generated.\n";
    help += "                   Example: 'com.sun.star.uno.XInterface' is a valid type.\n";
    help += "    -FT<name>    = name specifies a type or a list of types. For this types\n";
    help += "      [t1;...]     nothing will be generated.\n";
    help += "     |F<file>    = file specifies an text file. For the specified types in\n" ;
    help += "                   this file nothing will be generated.\n";
    help += "    -B<name>     = name specifies the base node. All types are searched under\n";
    help += "                   this node. Default is the root '/' of the registry files.\n";
    help += "                   This option takes effect using run mode 2 only.\n";
    help += "    -b<name>     = name specifies the base node of the output registry. All\n";
    help += "                   types will be generated under this node. Default is the\n";
    help += "                   root '/' of the registry file.\n";
    help += prepareVersion();

    return help;
}

OString RdbOptions::prepareVersion()
{
    OString version("\nSun Microsystems (R) ");
    version += m_program + " Version 2.0\n\n";

    return version;
}


