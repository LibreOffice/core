/*************************************************************************
 *
 *  $RCSfile: rdboptions.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:08 $
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
#include    <stdio.h>

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

OString RdbOptions::prepareHelp()
{
    OString help("\nusing: ");
    help += m_program + " [-options] [file_1 ... file_n]\nOptions:\n";
    help += "    [-R<regname>]= registry name specifies the registry used for bootstrapping\n"
            "                   uno. If no registry is specified, the program use the\n"
            "                   applicat.rdb finding near the executable.\n";
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


