/*************************************************************************
 *
 *  $RCSfile: regcompare.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2001-02-15 16:01:07 $
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

#include <set>

#ifndef _REGISTRY_REGISTRY_HXX_
#include "registry/registry.hxx"
#endif
#ifndef _REGISTRY_REFLREAD_HXX_
#include "registry/reflread.hxx"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

using namespace ::rtl;

#define U2S( s ) \
    OUStringToOString(s, RTL_TEXTENCODING_UTF8).getStr()
#define S2U( s ) \
    OStringToOUString(s, RTL_TEXTENCODING_UTF8)

struct LessString
{
    sal_Bool operator()(const OUString& str1, const OUString& str2) const
    {
        return (str1 < str2);
    }
};

typedef ::std::set< OUString, LessString > StringSet;

class Options
{
public:
    Options()
        : m_bFullCheck(sal_False)
        , m_bForceOutput(sal_False)
        {}
    ~Options()
        {}

    sal_Bool initOptions(int ac, char* av[], sal_Bool bCmdFile=sal_False);

    OString prepareHelp();
    OString prepareVersion();

    const OString& getProgramName()
        { return m_program; }
    const OString& getRegName1()
        { return m_regName1; }
    const OString& getRegName2()
        { return m_regName2; }
    sal_Bool isStartKeyValid()
        { return (m_startKey.getLength() > 0); };
    const OString& getStartKey()
        { return m_startKey; }
    sal_Bool isExcludeKeyValid()
        { return (m_excludeKey.getLength() > 0); };
    const OString& getExcludeKey()
        { return m_excludeKey; }
    sal_Bool fullCheck()
        { return m_bFullCheck; }
    sal_Bool forceOutput()
        { return m_bForceOutput; }
protected:
    OString     m_program;
    OString     m_regName1;
    OString     m_regName2;
    OString     m_startKey;
    OString     m_excludeKey;
    sal_Bool    m_bFullCheck;
    sal_Bool    m_bForceOutput;
};

sal_Bool Options::initOptions(int ac, char* av[], sal_Bool bCmdFile)
{
    sal_Bool bRet = sal_True;
    sal_uInt16  i=0;

    if (!bCmdFile)
    {
        bCmdFile = sal_True;

        m_program = av[0];

        if (ac < 2)
        {
            fprintf(stderr, "%s", prepareHelp().getStr());
            bRet = sal_False;
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
                case 'r':
                case 'R':
                {
                    sal_Bool bFirst = sal_True;
                    if (av[i][2] == '2')
                    {
                        bFirst = sal_False;
                    } else if (av[i][2] != '1')
                    {
                        fprintf(stderr, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                    }
                    if (av[i][3] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            fprintf(stderr, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                            bRet = sal_False;
                            break;
                        }
                    } else
                    {
                        s = av[i] + 2;
                    }

                    if ( bFirst )
                    {
                        m_regName1 = OString(s);
                    } else
                    {
                        m_regName2 = OString(s);
                    }
                }
                    break;
                case 's':
                case 'S':
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            fprintf(stderr, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                            bRet = sal_False;
                            break;
                        }
                    } else
                    {
                        s = av[i] + 2;
                    }
                    m_startKey = OString(s);
                    break;
                case 'x':
                case 'X':
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            fprintf(stderr, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                            bRet = sal_False;
                            break;
                        }
                    } else
                    {
                        s = av[i] + 2;
                    }
                    m_excludeKey = OString(s);
                    break;
                case 'c':
                case 'C':
                    if (av[i][2] != '\0')
                    {
                        fprintf(stderr, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                    }
                    m_bFullCheck = sal_True;
                    break;
                case 'f':
                case 'F':
                    if (av[i][2] != '\0')
                    {
                        fprintf(stderr, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                        bRet = sal_False;
                    }
                    m_bForceOutput = sal_True;
                    break;
                case 'h':
                case '?':
                    if (av[i][2] != '\0')
                    {
                        fprintf(stderr, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                        bRet = sal_False;
                    } else
                    {
                        fprintf(stdout, "%s", prepareHelp().getStr());
                        exit(0);
                    }
                    break;
                default:
                    fprintf(stderr, "%s: unknown option '%s'\n", m_program.getStr(), av[i]);
                    bRet = sal_False;
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
                    bRet = sal_False;
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

                    bRet = initOptions(rargc, rargv, bCmdFile);

                    for (long i=0; i < rargc; i++)
                    {
                        free(rargv[i]);
                    }
                }
            } else
            {
                fprintf(stderr, "%s: unknown option '%s'\n", m_program.getStr(), av[i]);
                bRet = sal_False;
            }
        }
    }

    if ( bRet )
    {
        if ( m_regName1.getLength() == 0 )
        {
              fprintf(stderr, "%s: missing option '-r1'\n", m_program.getStr());
            bRet = sal_False;
        }
        if ( m_regName2.getLength() == 0 )
        {
              fprintf(stderr, "%s: missing option '-r2'\n", m_program.getStr());
            bRet = sal_False;
        }
    }

    return bRet;
}

OString Options::prepareHelp()
{
    OString help("\nusing: ");
    help += m_program + " -r1<filename> -r2<filename> [-options] | @<filename>\n";
    help += "    -r1<filename>  = filename specifies the name of the first registry.\n";
    help += "    -r2<filename>  = filename specifies the name of the second registry.\n";
    help += "    @<filename>    = filename specifies a command file.\n";
    help += "Options:\n";
    help += "    -s<name>  = name specifies the name of a start key. If no start key\n";
    help += "                is specified the comparison starts with the root key.n";
    help += "    -x<name>  = name specifies the name of a key which won't be compared.\n";
    help += "    -f        = force the detailed output of any diffenrences. Default\n";
    help += "                is that only the the number of differences is returned.\n";
    help += "    -c        = make a complete check, that means any differences will be\n";
    help += "                detected. Default is only a compatibility check that means\n";
    help += "                only UNO typelibrary entries will be checked.\n";
    help += "    -h|-?     = print this help message and exit.\n";
    help += prepareVersion();

    return help;
}

OString Options::prepareVersion()
{
    OString version("\nSun Microsystems (R) ");
    version += m_program + " Version 1.0\n\n";
    return version;
}

static Options options;

static const RegistryTypeReaderLoader & getRegistryTypeReaderLoader()
{
    static RegistryTypeReaderLoader aLoader;
    return aLoader;
}

static sal_Char* getTypeClass(RTTypeClass typeClass)
{
    switch (typeClass)
    {
        case RT_TYPE_INTERFACE:
            return "INTERFACE";
        case RT_TYPE_MODULE:
            return "MODULE";
        case RT_TYPE_STRUCT:
            return "STRUCT";
        case RT_TYPE_ENUM:
            return "ENUM";
        case RT_TYPE_EXCEPTION:
            return "EXCEPTION";
        case RT_TYPE_TYPEDEF:
            return "TYPEDEF";
        case RT_TYPE_SERVICE:
            return "SERVICE";
        case RT_TYPE_OBJECT:
            return "OBJECT";
        case RT_TYPE_CONSTANTS:
            return "CONSTANTS";
    }
    return "INVALID";
}

static OString getFieldAccess(RTFieldAccess fieldAccess)
{
    OString ret;
    if ( (fieldAccess & RT_ACCESS_INVALID) == RT_ACCESS_INVALID )
    {
        ret += OString("INVALID");
    }
    if ( (fieldAccess & RT_ACCESS_READONLY) == RT_ACCESS_READONLY )
    {
        ret += OString(ret.getLength() > 0 ? ",READONLY" : "READONLY");
    }
    if ( (fieldAccess & RT_ACCESS_OPTIONAL) == RT_ACCESS_OPTIONAL )
    {
        ret += OString(ret.getLength() > 0 ? ",OPTIONAL" : "OPTIONAL");
    }
    if ( (fieldAccess & RT_ACCESS_MAYBEVOID) == RT_ACCESS_MAYBEVOID )
    {
        ret += OString(ret.getLength() > 0 ? ",MAYBEVOID" : "MAYBEVOID");
    }
    if ( (fieldAccess & RT_ACCESS_BOUND) == RT_ACCESS_BOUND )
    {
        ret += OString(ret.getLength() > 0 ? ",BOUND" : "BOUND");
    }
    if ( (fieldAccess & RT_ACCESS_CONSTRAINED) == RT_ACCESS_CONSTRAINED )
    {
        ret += OString(ret.getLength() > 0 ? ",CONSTRAINED" : "CONSTRAINED");
    }
    if ( (fieldAccess & RT_ACCESS_TRANSIENT) == RT_ACCESS_TRANSIENT )
    {
        ret += OString(ret.getLength() > 0 ? ",TRANSIENT" : "TRANSIENT");
    }
    if ( (fieldAccess & RT_ACCESS_MAYBEAMBIGUOUS) == RT_ACCESS_MAYBEAMBIGUOUS )
    {
        ret += OString(ret.getLength() > 0 ? ",MAYBEAMBIGUOUS" : "MAYBEAMBIGUOUS");
    }
    if ( (fieldAccess & RT_ACCESS_MAYBEDEFAULT) == RT_ACCESS_MAYBEDEFAULT )
    {
        ret += OString(ret.getLength() > 0 ? ",MAYBEDEFAULT" : "MAYBEDEFAULT");
    }
    if ( (fieldAccess & RT_ACCESS_REMOVEABLE) == RT_ACCESS_REMOVEABLE )
    {
        ret += OString(ret.getLength() > 0 ? ",REMOVEABLE" : "REMOVEABLE");
    }
    if ( (fieldAccess & RT_ACCESS_ATTRIBUTE) == RT_ACCESS_ATTRIBUTE )
    {
        ret += OString(ret.getLength() > 0 ? ",ATTRIBUTE" : "ATTRIBUTE");
    }
    if ( (fieldAccess & RT_ACCESS_PROPERTY) == RT_ACCESS_PROPERTY )
    {
        ret += OString(ret.getLength() > 0 ? ",PROPERTY" : "PROPERTY");
    }
    if ( (fieldAccess & RT_ACCESS_CONST) == RT_ACCESS_CONST )
    {
        ret += OString(ret.getLength() > 0 ? ",CONST" : "CONST");
    }
    if ( (fieldAccess & RT_ACCESS_READWRITE) == RT_ACCESS_READWRITE )
    {
        ret += OString(ret.getLength() > 0 ? ",READWRITE" : "READWRITE");
    }
    return ret;
}

static sal_Char* getConstValueType(RTConstValue& constValue)
{
    switch (constValue.m_type)
    {
        case RT_TYPE_BOOL:
            return "sal_Bool";
        case RT_TYPE_BYTE:
            return "sal_uInt8";
        case RT_TYPE_INT16:
            return "sal_Int16";
        case RT_TYPE_UINT16:
            return "sal_uInt16";
        case RT_TYPE_INT32:
            return "sal_Int32";
        case RT_TYPE_UINT32:
            return "sal_uInt32";
//      case RT_TYPE_INT64:
//          return "sal_Int64";
//      case RT_TYPE_UINT64:
//          return "sal_uInt64";
        case RT_TYPE_FLOAT:
            return "float";
        case RT_TYPE_DOUBLE:
            return "double";
        case RT_TYPE_STRING:
            return "sal_Unicode*";
    }
    return "NONE";
}
static void printConstValue(RTConstValue& constValue)
{
    switch (constValue.m_type)
    {
        case RT_TYPE_NONE:
            fprintf(stderr, "none");
        case RT_TYPE_BOOL:
            fprintf(stderr, "%s", constValue.m_value.aBool ? "TRUE" : "FALSE");
        case RT_TYPE_BYTE:
            fprintf(stderr, "%d", constValue.m_value.aByte);
        case RT_TYPE_INT16:
            fprintf(stderr, "%d", constValue.m_value.aShort);
        case RT_TYPE_UINT16:
            fprintf(stderr, "%d", constValue.m_value.aUShort);
        case RT_TYPE_INT32:
            fprintf(stderr, "%d", constValue.m_value.aLong);
        case RT_TYPE_UINT32:
            fprintf(stderr, "%d", constValue.m_value.aULong);
//      case RT_TYPE_INT64:
//          fprintf(stderr, "%d", constValue.m_value.aHyper);
//      case RT_TYPE_UINT64:
//          fprintf(stderr, "%d", constValue.m_value.aUHyper);
        case RT_TYPE_FLOAT:
            fprintf(stderr, "%f", constValue.m_value.aFloat);
        case RT_TYPE_DOUBLE:
            fprintf(stderr, "%f", constValue.m_value.aDouble);
        case RT_TYPE_STRING:
            fprintf(stderr, "%s", constValue.m_value.aString);
    }
}

static sal_uInt32 checkConstValue(const OUString& keyName,
                                  RTTypeClass typeClass,
                                  sal_Bool& bDump,
                                  RTConstValue& constValue1,
                                  RTConstValue& constValue2,
                                  sal_uInt16 index)
{
    switch (constValue1.m_type)
    {
        case RT_TYPE_BOOL:
            if (constValue1.m_value.aBool != constValue2.m_value.aBool)
            {
                if ( options.forceOutput() )
                {
                    if ( bDump )
                    {
                        fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stderr, "    Field %d: Value1 = %s  !=  Value2 = %s\n", index,
                            constValue1.m_value.aBool ? "TRUE" : "FALSE",
                            constValue2.m_value.aBool ? "TRUE" : "FALSE");
                }
                return 1;
            }
            break;
        case RT_TYPE_BYTE:
            if (constValue1.m_value.aByte != constValue2.m_value.aByte)
            {
                if ( options.forceOutput() )
                {
                    if ( bDump )
                    {
                        fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stderr, "    Field %d: Value1 = %d  !=  Value2 = %d\n", index,
                            constValue1.m_value.aByte, constValue2.m_value.aByte);
                }
                return 1;
            }
            break;
        case RT_TYPE_INT16:
            if (constValue1.m_value.aShort != constValue2.m_value.aShort)
            {
                if ( options.forceOutput() )
                {
                    if ( bDump )
                    {
                        fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stderr, "    Field %d: Value1 = %d  !=  Value2 = %d\n", index,
                            constValue1.m_value.aShort, constValue2.m_value.aShort);
                }
                return 1;
            }
            break;
        case RT_TYPE_UINT16:
            if (constValue1.m_value.aUShort != constValue2.m_value.aUShort)
            {
                if ( options.forceOutput() )
                {
                    if ( bDump )
                    {
                        fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stderr, "    Field %d: Value1 = %d  !=  Value2 = %d\n", index,
                            constValue1.m_value.aUShort, constValue2.m_value.aUShort);
                }
                return 1;
            }
            break;
        case RT_TYPE_INT32:
            if (constValue1.m_value.aLong != constValue2.m_value.aLong)
            {
                if ( options.forceOutput() )
                {
                    if ( bDump )
                    {
                        fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stderr, "    Field %d: Value1 = %d  !=  Value2 = %d\n", index,
                            constValue1.m_value.aLong, constValue2.m_value.aLong);
                }
                return 1;
            }
            break;
        case RT_TYPE_UINT32:
            if (constValue1.m_value.aULong != constValue2.m_value.aULong)
            {
                if ( options.forceOutput() )
                {
                    if ( bDump )
                    {
                        fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stderr, "    Field %d: Value1 = %d  !=  Value2 = %d\n", index,
                            constValue1.m_value.aULong, constValue2.m_value.aULong);
                }
                return 1;
            }
            break;
//      case RT_TYPE_INT64:
//          if (constValue1.m_value.aHyper != constValue2.m_value.aHyper)
//          {
//              if ( options.forceOutput() )
//              {
//                  if ( bDump )
//                  {
//                      fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
//                      bDump = sal_False;
//                  }
//                  fprintf(stderr, "    Field %d: Value1 = %d  !=  Value2 = %d\n", index,
//                          constValue1.m_value.aHyper, constValue2.m_value.aHyper);
//              }
//              return 1;
//          }
//          break;
//      case RT_TYPE_UINT64:
//          if (constValue1.m_value.aUHyper != constValue2.m_value.aUHyper)
//          {
//              if ( options.forceOutput() )
//              {
//                  if ( bDump )
//                  {
//                      fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
//                      bDump = sal_False;
//                  }
//                  fprintf(stderr, "    Field %d: Value1 = %d  !=  Value2 = %d\n", index,
//                          constValue1.m_value.aUHyper, constValue2.m_value.aUHyper);
//              }
//              return 1;
//          }
//          break;
        case RT_TYPE_FLOAT:
            if (constValue1.m_value.aFloat != constValue2.m_value.aFloat)
            {
                if ( options.forceOutput() )
                {
                    if ( bDump )
                    {
                        fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stderr, "    Field %d: Value1 = %d  !=  Value2 = %d\n", index,
                            constValue1.m_value.aFloat, constValue2.m_value.aFloat);
                }
                return 1;
            }
            break;
        case RT_TYPE_DOUBLE:
            if (constValue1.m_value.aDouble != constValue2.m_value.aDouble)
            {
                if ( options.forceOutput() )
                {
                    if ( bDump )
                    {
                        fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stderr, "    Field %d: Value1 = %d  !=  Value2 = %d\n", index,
                            constValue1.m_value.aDouble, constValue2.m_value.aDouble);
                }
                return 1;
            }
            break;
        case RT_TYPE_STRING:
            if (rtl_ustr_compare(constValue1.m_value.aString, constValue2.m_value.aString) != 0)
            {
                if ( options.forceOutput() )
                {
                    if ( bDump )
                    {
                        fprintf(stderr, "%s\n", U2S(keyName));
                        fprintf(stderr, "    TypeClass = %s\n", getTypeClass(typeClass));
                        bDump = sal_False;
                    }
                    fprintf(stderr, "    Field %d: Value1 = %d  !=  Value2 = %d\n", index,
                            constValue1.m_value.aString, constValue2.m_value.aString);
                }
                return 1;
            }
    }
    return 0;
}

static sal_uInt32 checkField(const OUString& keyName,
                             RTTypeClass typeClass,
                             sal_Bool& bDump,
                             RegistryTypeReader& reader1,
                             RegistryTypeReader& reader2,
                             sal_uInt16 index)
{
    sal_uInt32 nError = 0;
    if ( reader1.getFieldName(index) !=
         reader2.getFieldName(index) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Field %d: Name1 = %s  !=  Name2 = %s\n", index,
                    U2S(reader1.getFieldName(index)), U2S(reader2.getFieldName(index)));
        }
        nError++;
    }
    if ( reader1.getFieldType(index) !=
         reader2.getFieldType(index) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Field %d: Type1 = %s  !=  Type2 = %s\n", index,
                    U2S(reader1.getFieldType(index)), U2S(reader2.getFieldType(index)));
        }
        nError++;
    } else
    {
        RTConstValue constValue1 = reader1.getFieldConstValue(index);
        RTConstValue constValue2 = reader2.getFieldConstValue(index);
        if ( constValue1.m_type != constValue2.m_type )
        {
            if ( options.forceOutput() )
            {
                if ( bDump )
                {
                    fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stderr, "    Field %d: Access1 = %s  !=  Access2 = %s\n", index,
                        getConstValueType(constValue1), getConstValueType(constValue2));
                fprintf(stderr, "    Field %d: Value1 = ", index);
                printConstValue(constValue1);
                fprintf(stderr, "  !=  Value2 = ");
                printConstValue(constValue1);
                fprintf(stderr, "\n;");
            }
            nError++;
        } else
        {
            nError += checkConstValue(keyName, typeClass, bDump, constValue1, constValue2, index);
        }
    }

    if ( reader1.getFieldAccess(index) != reader2.getFieldAccess(index) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Field %d: FieldAccess1 = %s  !=  FieldAccess2 = %s\n", index,
                    getFieldAccess(reader1.getFieldAccess(index)).getStr(),
                    getFieldAccess(reader1.getFieldAccess(index)).getStr());
        }
        nError++;
    }

    if ( options.fullCheck() &&
         (reader1.getFieldDoku(index) != reader2.getFieldDoku(index)) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Field %d: Doku1 = %s  !=  Doku2 = %s\n", index,
                    U2S(reader1.getFieldDoku(index)), U2S(reader2.getFieldDoku(index)));
        }
        nError++;
    }
    if ( options.fullCheck() &&
         (reader1.getFieldFileName(index) != reader2.getFieldFileName(index)) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Field %d: FileName1 = %s  !=  FileName2 = %s\n", index,
                    U2S(reader1.getFieldFileName(index)), U2S(reader2.getFieldFileName(index)));
        }
        nError++;
    }
    return nError;
}

static sal_Char* getMethodMode(RTMethodMode methodMode)
{
    switch ( methodMode )
    {
        case RT_MODE_ONEWAY:
            return "ONEWAY";
        case RT_MODE_ONEWAY_CONST:
            return "ONEWAY,CONST";
        case RT_MODE_TWOWAY:
            return "NONE";
        case RT_MODE_TWOWAY_CONST:
            return "CONST";
    }
    return "INVALID";
}

static sal_Char* getParamMode(RTParamMode paramMode)
{
    switch ( paramMode )
    {
        case RT_PARAM_IN:
            return "IN";
        case RT_PARAM_OUT:
            return "OUT";
        case RT_PARAM_INOUT:
            return "INOUT";
    }
    return "INVALID";
}

static sal_uInt32 checkMethod(const OUString& keyName,
                              RTTypeClass typeClass,
                              sal_Bool& bDump,
                              RegistryTypeReader& reader1,
                              RegistryTypeReader& reader2,
                              sal_uInt16 index)
{
    sal_uInt32 nError = 0;
    if ( reader1.getMethodName(index) !=
         reader2.getMethodName(index) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Method1 %d: Name1 = %s  !=  Name2 = %s\n", index,
                    U2S(reader1.getMethodName(index)),
                    U2S(reader2.getMethodName(index)));
        }
        nError++;
    }

    if ( reader1.getMethodReturnType(index) !=
         reader2.getMethodReturnType(index) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Method1 %d: ReturnType1 = %s  !=  ReturnType2 = %s\n", index,
                    U2S(reader1.getMethodReturnType(index)),
                    U2S(reader2.getMethodReturnType(index)));
        }
        nError++;
    }

    sal_uInt16 nParams1 = (sal_uInt16)reader1.getMethodParamCount(index);
    sal_uInt16 nParams2 = (sal_uInt16)reader2.getMethodParamCount(index);
    if ( nParams1 != nParams2 )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Method %d : nParameters1 = %d  !=  nParameters2 = %d\n", index, nParams1, nParams2);
        }
        nError++;
    }
    sal_uInt16 i=0;
    for (i=0; i < nParams1 && i < nParams2; i++)
    {
        if ( reader1.getMethodParamType(index, i) != reader2.getMethodParamType(index, i) )
        {
            if ( options.forceOutput() )
            {
                if ( bDump )
                {
                    fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stderr, "    Method %d, Parameter %d: Type1 = %s  !=  Type2 = %s\n", index, i,
                        U2S(reader1.getMethodParamType(index, i)),
                        U2S(reader2.getMethodParamType(index, i)));
            }
            nError++;
        }
        if ( options.fullCheck() &&
             (reader1.getMethodParamName(index, i) != reader2.getMethodParamName(index, i)) )
        {
            if ( options.forceOutput() )
            {
                if ( bDump )
                {
                    fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stderr, "    Method %d, Parameter %d: Name1 = %s  !=  Name2 = %s\n", index, i,
                        U2S(reader1.getMethodParamName(index, i)),
                        U2S(reader2.getMethodParamName(index, i)));
            }
            nError++;
        }
        if ( reader1.getMethodParamMode(index, i) != reader2.getMethodParamMode(index, i) )
        {
            if ( options.forceOutput() )
            {
                if ( bDump )
                {
                    fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stderr, "    Method %d, Parameter %d: Mode1 = %s  !=  Mode2 = %s\n", index, i,
                        getParamMode(reader1.getMethodParamMode(index, i)),
                        getParamMode(reader2.getMethodParamMode(index, i)));
            }
            nError++;
        }
    }
    if ( i < nParams1 && options.forceOutput() )
    {
        if ( bDump )
        {
            fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
            bDump = sal_False;
        }
        fprintf(stderr, "    Registry1: Method %d contains %d more parameters\n", nParams1 - i);
    }
    if ( i < nParams2 && options.forceOutput() )
    {
        if ( bDump )
        {
            fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
            bDump = sal_False;
        }
        fprintf(stderr, "    Registry2: Method %d contains %d more parameters\n", nParams2 - i);
    }

    sal_uInt16 nExcep1 = (sal_uInt16)reader1.getMethodExcCount(index);
    sal_uInt16 nExcep2 = (sal_uInt16)reader2.getMethodExcCount(index);
    if ( nExcep1 != nExcep2 )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    nExceptions1 = %d  !=  nExceptions2 = %d\n", nExcep1, nExcep2);
        }
        nError++;
    }
    for (i=0; i < nExcep1 && i < nExcep2; i++)
    {
        if ( reader1.getMethodExcType(index, i) != reader2.getMethodExcType(index, i) )
        {
            if ( options.forceOutput() )
            {
                if ( bDump )
                {
                    fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stderr, "    Method %d, Exception %d: Name1 = %s  !=  Name2 = %s\n", index, i,
                        U2S(reader1.getMethodExcType(index, i)),
                        U2S(reader2.getMethodExcType(index, i)));
            }
            nError++;
        }
    }
    if ( i < nExcep1 && options.forceOutput() )
    {
        if ( bDump )
        {
            fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
            bDump = sal_False;
        }
        fprintf(stderr, "    Registry1: Method %d contains %d more exceptions\n", nExcep1 - i);
    }
    if ( i < nExcep2 && options.forceOutput() )
    {
        if ( bDump )
        {
            fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
            bDump = sal_False;
        }
        fprintf(stderr, "    Registry2: Method %d contains %d more exceptions\n", nExcep2 - i);
    }

    if ( reader1.getMethodMode(index) != reader2.getMethodMode(index) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Method %d: Mode1 = %s  !=  Mode2 = %s\n", index,
                    getMethodMode(reader1.getMethodMode(index)),
                    getMethodMode(reader2.getMethodMode(index)));
        }
        nError++;
    }

    if ( options.fullCheck() &&
         (reader1.getMethodDoku(index) != reader2.getMethodDoku(index)) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Method %d: Doku1 = %s  !=  Doku2 = %s\n", index,
                    U2S(reader1.getMethodDoku(index)),
                    U2S(reader2.getMethodDoku(index)));
        }
        nError++;
    }
    return nError;
}

static sal_Char* getReferenceType(RTReferenceType refType)
{
    switch (refType)
    {
        case RT_REF_SUPPORTS:
            return "RT_REF_SUPPORTS";
        case RT_REF_OBSERVES:
            return "RT_REF_OBSERVES";
        case RT_REF_EXPORTS:
            return "RT_REF_EXPORTS";
        case RT_REF_NEEDS:
            return "RT_REF_NEEDS";
    }
    return "RT_REF_INVALID";
}

static sal_uInt32 checkReference(const OUString& keyName,
                                 RTTypeClass typeClass,
                                 sal_Bool& bDump,
                                 RegistryTypeReader& reader1,
                                    RegistryTypeReader& reader2,
                                    sal_uInt16 index)
{
    sal_uInt32 nError = 0;
    if ( reader1.getReferenceName(index) !=
         reader2.getReferenceName(index) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Reference %d: Name1 = %s  !=  Name2 = %s\n", index,
                    U2S(reader1.getReferenceName(index)),
                    U2S(reader2.getReferenceName(index)));
        }
        nError++;
    }
    if ( reader1.getReferenceType(index) !=
         reader2.getReferenceType(index) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Reference %d: Type1 = %s  !=  Type2 = %s\n", index,
                    getReferenceType(reader1.getReferenceType(index)),
                    getReferenceType(reader2.getReferenceType(index)));
        }
        nError++;
    }
    if ( options.fullCheck() &&
         (reader1.getReferenceDoku(index) != reader2.getReferenceDoku(index)) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Reference %d: Doku1 = %s  !=  Doku2 = %s\n", index,
                    U2S(reader1.getReferenceDoku(index)),
                    U2S(reader2.getReferenceDoku(index)));
        }
        nError++;
    }
    if ( reader1.getReferenceAccess(index) !=
         reader2.getReferenceAccess(index) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Reference %d: Access1 = %s  !=  Access2 = %s\n", index,
                    getFieldAccess(reader1.getReferenceAccess(index)).getStr(),
                    getFieldAccess(reader1.getReferenceAccess(index)).getStr());
        }
        nError++;
    }
    return nError;
}

static sal_uInt32 checkBlob(const OUString& keyName, RegistryTypeReader& reader1, sal_uInt32 size1,
                            RegistryTypeReader& reader2, sal_uInt32 size2)
{
    sal_uInt32 nError = 0;
    sal_Bool bDump = sal_True;

    if ( options.fullCheck() && (size1 != size2) )
    {
        if ( options.forceOutput() )
        {
            fprintf(stderr, "    Size1 = %d    Size2 = %d\n", size1, size2);
        }
    }
    if ( reader1.getTypeClass() != reader2.getTypeClass() )
    {
        if ( options.forceOutput() )
        {
            fprintf(stderr, "?: %s\n", U2S(keyName));
            bDump = sal_False;
            fprintf(stderr, "    TypeClass1 = %s  !=  TypeClass2 = %s\n",
                    getTypeClass(reader1.getTypeClass()),
                    getTypeClass(reader2.getTypeClass()));
        }
        return ++nError;
    }

    RTTypeClass typeClass = reader1.getTypeClass();

    if ( reader1.getTypeName() != reader2.getTypeName() )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    TypeName1 = %s  !=  TypeName2 = %s\n",
                    U2S(reader1.getTypeName()), U2S(reader2.getTypeName()));
        }
        nError++;
    }
    if ( (typeClass == RT_TYPE_INTERFACE ||
          typeClass == RT_TYPE_STRUCT ||
          typeClass == RT_TYPE_EXCEPTION) &&
         reader1.getSuperTypeName() != reader2.getSuperTypeName() )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    SuperTypeName1 = %s  !=  SuperTypeName2 = %s\n",
                    U2S(reader1.getSuperTypeName()), U2S(reader2.getSuperTypeName()));
        }
        nError++;
    }
    if ( typeClass == RT_TYPE_INTERFACE )
    {
        RTUik uik1, uik2;
        reader1.getUik(uik1);
        reader2.getUik(uik2);
        if ( uik1.m_Data1 != uik2.m_Data1 ||
             uik1.m_Data2 != uik2.m_Data2 ||
             uik1.m_Data3 != uik2.m_Data3 ||
             uik1.m_Data4 != uik2.m_Data4 ||
             uik1.m_Data5 != uik2.m_Data5 )
        {
            if ( options.forceOutput() )
            {
                if ( bDump )
                {
                    fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stderr, "    uik1.m_data1 = %X    uik2.m_data1 = %X\n", uik1.m_Data1, uik2.m_Data1);
                fprintf(stderr, "    uik1.m_data2 = %X    uik2.m_data2 = %X\n", uik1.m_Data2, uik2.m_Data2);
                fprintf(stderr, "    uik1.m_data3 = %X    uik2.m_data3 = %X\n", uik1.m_Data3, uik2.m_Data3);
                fprintf(stderr, "    uik1.m_data4 = %X    uik2.m_data4 = %X\n", uik1.m_Data4, uik2.m_Data4);
                fprintf(stderr, "    uik1.m_data5 = %X    uik2.m_data5 = %X\n", uik1.m_Data5, uik2.m_Data5);
            }
            nError++;
        }
    }
    sal_uInt16 nFields1 = (sal_uInt16)reader1.getFieldCount();
    sal_uInt16 nFields2 = (sal_uInt16)reader2.getFieldCount();
    if ( nFields1 != nFields2 )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    nFields1 = %d  !=  nFields2 = %d\n", nFields1, nFields2);
        }
        nError++;
    }
    sal_uInt16 i=0;
    for (i=0; i < nFields1 && i < nFields2; i++)
    {
        nError += checkField(keyName, typeClass, bDump, reader1, reader2, i);
    }
    if ( i < nFields1 && options.forceOutput() )
    {
        if ( bDump )
        {
            fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
            bDump = sal_False;
        }
        fprintf(stderr, "    Registry1 contains %d more fields\n", nFields1 - i);
    }
    if ( i < nFields2 && options.forceOutput() )
    {
        if ( bDump )
        {
            fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
            bDump = sal_False;
        }
        fprintf(stderr, "    Registry2 contains %d more fields\n", nFields2 - i);
    }
    if ( typeClass == RT_TYPE_INTERFACE )
    {
        sal_uInt16 nMethods1 = (sal_uInt16)reader1.getMethodCount();
        sal_uInt16 nMethods2 = (sal_uInt16)reader2.getMethodCount();
        if ( nMethods1 != nMethods2 )
        {
            if ( options.forceOutput() )
            {
                if ( bDump )
                {
                    fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stderr, "    nMethods1 = %d  !=  nMethods2 = %d\n", nMethods1, nMethods2);
            }
            nError++;
        }
        for (i=0; i < nMethods1 && i < nMethods2; i++)
        {
            nError += checkMethod(keyName, typeClass, bDump, reader1, reader2, i);
        }
        if ( i < nMethods1 && options.forceOutput() )
        {
            fprintf(stderr, "    Registry1 contains %d more methods\n", nMethods1 - i);
        }
        if ( i < nMethods2 && options.forceOutput() )
        {
            fprintf(stderr, "    Registry2 contains %d more methods\n", nMethods2 - i);
        }
    }
    if ( typeClass == RT_TYPE_SERVICE )
    {
        sal_uInt16 nReference1 = (sal_uInt16)reader1.getReferenceCount();
        sal_uInt16 nReference2 = (sal_uInt16)reader2.getReferenceCount();
        if ( nReference1 != nReference2 )
        {
            if ( options.forceOutput() )
            {
                if ( bDump )
                {
                    fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stderr, "    nReferences1 = %d  !=  nReferences2 = %d\n", nReference1, nReference2);
            }
            nError++;
        }
        for (i=0; i < nReference1 && i < nReference2; i++)
        {
            nError += checkReference(keyName, typeClass, bDump, reader1, reader2, i);
        }
        if ( i < nReference1 && options.forceOutput() )
        {
            fprintf(stderr, "    Registry1 contains %d more references\n", nReference1 - i);
        }
        if ( i < nReference2 && options.forceOutput() )
        {
            fprintf(stderr, "    Registry2 contains %d more references\n", nReference2 - i);
        }
    }

    if ( options.fullCheck() && (reader1.getDoku() != reader2.getDoku()) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stderr, "    Doku1 = %s  !=  Doku2 = %s\n",
                    U2S(reader1.getDoku()), U2S(reader2.getDoku()));
        }
        nError++;
    }
    if ( options.fullCheck() && (reader1.getFileName() != reader2.getFileName()) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stderr, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
            }
            fprintf(stderr, "    FileName1 = %s  !=  FileName2 = %s\n",
                    U2S(reader1.getFileName()), U2S(reader2.getFileName()));
        }
        nError++;
    }
    return nError;
}

static sal_uInt32 checkValueDifference(RegistryKey& key1, RegValueType valueType1, sal_uInt32 size1,
                                RegistryKey& key2, RegValueType valueType2, sal_uInt32 size2)
{
    OUString tmpName;
    sal_uInt32 nError = 0;

    if ( valueType1 == valueType2 )
    {
        sal_Bool bEqual = sal_True;
        if ( valueType1 == RG_VALUETYPE_LONGLIST ||
              valueType1 == RG_VALUETYPE_STRINGLIST ||
              valueType1 == RG_VALUETYPE_UNICODELIST )
        {
            switch (valueType1)
            {
            case RG_VALUETYPE_LONGLIST:
                {
                RegistryValueList<sal_Int32> valueList1;
                RegistryValueList<sal_Int32> valueList2;
                key1.getLongListValue(tmpName, valueList1);
                key2.getLongListValue(tmpName, valueList2);
                sal_uInt32 length1 = valueList1.getLength();
                sal_uInt32 length2 = valueList1.getLength();
                if ( length1 != length2 )
                {
                    bEqual = sal_False;
                    break;
                }
                for (sal_uInt32 i=0; i<length1; i++)
                {
                    if ( valueList1.getElement(i) != valueList2.getElement(i) )
                    {
                        bEqual = sal_False;
                        break;
                    }
                }
                }
                break;
            case RG_VALUETYPE_STRINGLIST:
                {
                RegistryValueList<sal_Char*> valueList1;
                RegistryValueList<sal_Char*> valueList2;
                key1.getStringListValue(tmpName, valueList1);
                key2.getStringListValue(tmpName, valueList2);
                sal_uInt32 length1 = valueList1.getLength();
                sal_uInt32 length2 = valueList1.getLength();
                if ( length1 != length2 )
                {
                    bEqual = sal_False;
                    break;
                }
                for (sal_uInt32 i=0; i<length1; i++)
                {
                    if ( strcmp(valueList1.getElement(i), valueList2.getElement(i)) != 0 )
                    {
                        bEqual = sal_False;
                        break;
                    }
                }
                }
                break;
            case RG_VALUETYPE_UNICODELIST:
                {
                RegistryValueList<sal_Unicode*> valueList1;
                RegistryValueList<sal_Unicode*> valueList2;
                key1.getUnicodeListValue(tmpName, valueList1);
                key2.getUnicodeListValue(tmpName, valueList2);
                sal_uInt32 length1 = valueList1.getLength();
                sal_uInt32 length2 = valueList1.getLength();
                if ( length1 != length2 )
                {
                    bEqual = sal_False;
                    break;
                }
                for (sal_uInt32 i=0; i<length1; i++)
                {
                    if ( rtl_ustr_compare(valueList1.getElement(i), valueList2.getElement(i)) != 0 )
                    {
                        bEqual = sal_False;
                        break;
                    }
                }
                }
                break;
            }
        }

        if ( bEqual)
        {
            RegValue value1 = rtl_allocateMemory(size1);
            RegValue value2 = rtl_allocateMemory(size2);

            key1.getValue(tmpName, value1);
            key2.getValue(tmpName, value2);

            bEqual = (rtl_compareMemory(value1, value2, size1) == 0 );

            if ( !bEqual && valueType1 == RG_VALUETYPE_BINARY && valueType1 == RG_VALUETYPE_BINARY )
            {
                RegistryTypeReader reader1(getRegistryTypeReaderLoader(),
                                           (sal_uInt8*)value1, size1, sal_False);
                RegistryTypeReader reader2(getRegistryTypeReaderLoader(),
                                           (sal_uInt8*)value2, size2, sal_False);

                if ( reader1.isValid() && reader2.isValid() )
                {
                    return checkBlob(key1.getName(), reader1, size1, reader2, size2);
                }
            }

            rtl_freeMemory(value1);
            rtl_freeMemory(value2);

            if ( bEqual )
            {
                return 0;
            } else
            {
                if ( options.forceOutput() )
                {
                    fprintf(stderr, "Difference: key values of key \"%s\" are different\n",
                            U2S(key1.getName()));
                }
                nError++;
            }
        }
    }

    if ( options.forceOutput() )
    {
        switch (valueType1)
        {
        case RG_VALUETYPE_NOT_DEFINED:
            fprintf(stderr, "    Registry 1: key has no value\n");
            break;
        case RG_VALUETYPE_LONG:
        case RG_VALUETYPE_STRING:
        case RG_VALUETYPE_UNICODE:
        {
            RegValue value1 = rtl_allocateMemory(size1);
            key1.getValue(tmpName, value1);

            switch (valueType1)
            {
            case RG_VALUETYPE_LONG:
                fprintf(stderr, "    Registry 1: Value: Type = RG_VALUETYPE_LONG\n");
                fprintf(stderr, "                       Size = %d\n", size1);
                fprintf(stderr, "                       Data = %d\n", (sal_Int32)value1);
                break;
            case RG_VALUETYPE_STRING:
                fprintf(stderr, "    Registry 1: Value: Type = RG_VALUETYPE_STRING\n");
                fprintf(stderr, "                       Size = %d\n", size1);
                fprintf(stderr, "                       Data = \"%s\"\n", (sal_Char*)value1);
                break;
            case RG_VALUETYPE_UNICODE:
                {
                OUString uStrValue((sal_Unicode*)value1);
                fprintf(stderr, "    Registry 1: Value: Type = RG_VALUETYPE_UNICODE\n");
                fprintf(stderr, "                       Size = %d\n", size1);
                fprintf(stderr, "                       Data = \"%s\"\n", U2S(uStrValue));
                }
                break;
            }

            rtl_freeMemory(value1);
        }
            break;
        case RG_VALUETYPE_BINARY:
            fprintf(stderr, "    Registry 1: Value: Type = RG_VALUETYPE_BINARY\n");
            break;
        case RG_VALUETYPE_LONGLIST:
            {
            RegistryValueList<sal_Int32> valueList;
            key1.getLongListValue(tmpName, valueList);
            fprintf(stderr, "    Registry 1: Value: Type = RG_VALUETYPE_LONGLIST\n");
            fprintf(stderr, "                       Size = %d\n", size1);
            sal_uInt32 length = valueList.getLength();
            for (sal_uInt32 i=0; i<length; i++)
            {
                fprintf(stderr, "                       Data[%d] = %d\n", i, valueList.getElement(i));
            }
            }
            break;
        case RG_VALUETYPE_STRINGLIST:
            {
            RegistryValueList<sal_Char*> valueList;
            key1.getStringListValue(tmpName, valueList);
            fprintf(stderr, "    Registry 1: Value: Type = RG_VALUETYPE_STRINGLIST\n");
            fprintf(stderr, "                       Size = %d\n", size1);
            sal_uInt32 length = valueList.getLength();
            for (sal_uInt32 i=0; i<length; i++)
            {
                fprintf(stderr, "                       Data[%d] = \"%s\"\n", i, valueList.getElement(i));
            }
            }
            break;
        case RG_VALUETYPE_UNICODELIST:
            {
            RegistryValueList<sal_Unicode*> valueList;
            key1.getUnicodeListValue(tmpName, valueList);
            fprintf(stderr, "    Registry 1: Value: Type = RG_VALUETYPE_UNICODELIST\n");
            fprintf(stderr, "                       Size = %d\n", size1);
            sal_uInt32 length = valueList.getLength();
            OUString uStrValue;
            for (sal_uInt32 i=0; i<length; i++)
            {
                uStrValue = OUString(valueList.getElement(i));
                fprintf(stderr, "                       Data[%d] = \"%s\"\n", i, U2S(uStrValue));
            }
            }
            break;
        }

        switch (valueType2)
        {
        case RG_VALUETYPE_NOT_DEFINED:
            fprintf(stderr, "    Registry 2: key has no value\n");
            break;
        case RG_VALUETYPE_LONG:
        case RG_VALUETYPE_STRING:
        case RG_VALUETYPE_UNICODE:
        {
            RegValue value2 = rtl_allocateMemory(size2);
            key2.getValue(tmpName, value2);

            switch (valueType2)
            {
            case RG_VALUETYPE_LONG:
                fprintf(stderr, "    Registry 2: Value: Type = RG_VALUETYPE_LONG\n");
                fprintf(stderr, "                       Size = %d\n", size2);
                fprintf(stderr, "                       Data = %d\n", (sal_Int32)value2);
                break;
            case RG_VALUETYPE_STRING:
                fprintf(stderr, "    Registry 2: Value: Type = RG_VALUETYPE_STRING\n");
                fprintf(stderr, "                       Size = %d\n", size2);
                fprintf(stderr, "                       Data = \"%s\"\n", (sal_Char*)value2);
                break;
            case RG_VALUETYPE_UNICODE:
                {
                OUString uStrValue((sal_Unicode*)value2);
                fprintf(stderr, "    Registry 2: Value: Type = RG_VALUETYPE_UNICODE\n");
                fprintf(stderr, "                       Size = %d\n", size2);
                fprintf(stderr, "                       Data = \"%s\"\n", U2S(uStrValue));
                }
                break;
            }

            rtl_freeMemory(value2);
        }
            break;
        case RG_VALUETYPE_BINARY:
            fprintf(stderr, "    Registry 2: Value: Type = RG_VALUETYPE_BINARY\n");
            break;
        case RG_VALUETYPE_LONGLIST:
            {
            RegistryValueList<sal_Int32> valueList;
            key2.getLongListValue(tmpName, valueList);
            fprintf(stderr, "    Registry 2: Value: Type = RG_VALUETYPE_LONGLIST\n");
            fprintf(stderr, "                       Size = %d\n", size2);
            sal_uInt32 length = valueList.getLength();
            for (sal_uInt32 i=0; i<length; i++)
            {
                fprintf(stderr, "                       Data[%d] = %d\n", i, valueList.getElement(i));
            }
            }
            break;
        case RG_VALUETYPE_STRINGLIST:
            {
            RegistryValueList<sal_Char*> valueList;
            key2.getStringListValue(tmpName, valueList);
            fprintf(stderr, "    Registry 2: Value: Type = RG_VALUETYPE_STRINGLIST\n");
            fprintf(stderr, "                       Size = %d\n", size2);
            sal_uInt32 length = valueList.getLength();
            for (sal_uInt32 i=0; i<length; i++)
            {
                fprintf(stderr, "                       Data[%d] = \"%s\"\n", i, valueList.getElement(i));
            }
            }
            break;
        case RG_VALUETYPE_UNICODELIST:
            {
            RegistryValueList<sal_Unicode*> valueList;
            key2.getUnicodeListValue(tmpName, valueList);
            fprintf(stderr, "    Registry 2: Value: Type = RG_VALUETYPE_UNICODELIST\n");
            fprintf(stderr, "                       Size = %d\n", size2);
            sal_uInt32 length = valueList.getLength();
            OUString uStrValue;
            for (sal_uInt32 i=0; i<length; i++)
            {
                uStrValue = OUString(valueList.getElement(i));
                fprintf(stderr, "                       Data[%d] = \"%s\"\n", i, U2S(uStrValue));
            }
            }
            break;
        }
    }
    return nError;
}

static sal_uInt32 checkDifferences(StringSet& keys, RegistryKeyNames& subKeyNames1,
                      RegistryKeyNames& subKeyNames2)
{
    sal_uInt32 nError = 0;
    sal_uInt32 length1 = subKeyNames1.getLength();
    sal_uInt32 length2 = subKeyNames2.getLength();
    sal_uInt32 i,j;
    sal_Bool bFound = sal_False;

    for (i=0; i<length1; i++)
    {
        for (j=0; j<length2; j++)
        {
            if ( subKeyNames1.getElement(i) == subKeyNames2.getElement(j) )
            {
                bFound = sal_True;
                keys.insert(subKeyNames1.getElement(i));
                break;
            }
        }
        if ( !bFound && options.fullCheck())
        {
            if ( options.forceOutput() )
            {
                fprintf(stderr, "EXISTENCE: key \"%s\" exists only in registry \"%s\"\n",
                        U2S(subKeyNames1.getElement(i)), options.getRegName1().getStr());
            }
            nError++;
        } else
            bFound = sal_False;
    }

    for (i=0; i<length2; i++)
    {
        for (j=0; j<length1; j++)
        {
            if ( subKeyNames2.getElement(i) == subKeyNames1.getElement(j) )
            {
                bFound = sal_True;
                keys.insert(subKeyNames2.getElement(i));
                break;
            }
        }
        if ( !bFound && options.fullCheck() )
        {
            if ( options.forceOutput() )
            {
                fprintf(stderr, "EXISTENCE: key \"%s\" exists only in registry \"%s\"\n",
                        U2S(subKeyNames2.getElement(i)), options.getRegName1().getStr());
            }
            nError++;
        } else
            bFound = sal_False;
    }
    return nError;
}

static sal_uInt32 compareKeys(RegistryKey& key1, RegistryKey& key2)
{
    sal_uInt32 nError = 0;

    RegValueType valueType1 = RG_VALUETYPE_NOT_DEFINED;
    RegValueType valueType2 = RG_VALUETYPE_NOT_DEFINED;
    sal_uInt32 size1 = 0;
    sal_uInt32 size2 = 0;
    OUString tmpName;
    RegError e1 = key1.getValueInfo(tmpName, &valueType1, &size1);
    RegError e2 = key2.getValueInfo(tmpName, &valueType2, &size2);

    if ( e1 == e2 && e1 != REG_VALUE_NOT_EXISTS && e1 != REG_INVALID_VALUE )
    {
        nError += checkValueDifference(key1, valueType1, size1, key2, valueType2, size2);
    } else
    {
        if ( e1 != REG_INVALID_VALUE || e2 != REG_INVALID_VALUE )
        {
            if ( options.forceOutput() )
            {
                fprintf(stderr, "VALUES: key values of key \"%s\" are different\n", U2S(key1.getName()));
            }
            nError++;
        }
    }

    RegistryKeyNames subKeyNames1;
    RegistryKeyNames subKeyNames2;

    key1.getKeyNames(tmpName, subKeyNames1);
    key2.getKeyNames(tmpName, subKeyNames2);

    StringSet keys;
    nError += checkDifferences(keys, subKeyNames1, subKeyNames2);

    StringSet::iterator iter = keys.begin();
    StringSet::iterator end = keys.end();

    RegistryKey subKey1, subKey2;
    OUString keyName;
    while ( iter !=  end )
    {
        keyName = OUString(*iter);
        if ( options.isExcludeKeyValid() &&
             (keyName == S2U(options.getExcludeKey())) )
        {
            iter++;
            continue;
        }

        keyName = keyName.getToken(keyName.getTokenCount('/')-1, '/');
        if ( key1.openKey(keyName, subKey1) )
        {
            if ( options.forceOutput() )
            {
                fprintf(stderr, "ERROR: could not open key \"%s\" in registry \"%s\"\n",
                        U2S(*iter), options.getRegName1().getStr());
            }
            nError++;
        }
        if ( key2.openKey(keyName, subKey2) )
        {
            if ( options.forceOutput() )
            {
                fprintf(stderr, "ERROR: could not open key \"%s\" in registry \"%s\"\n",
                        U2S(*iter), options.getRegName2().getStr());
            }
            nError++;
        }
        if ( subKey1.isValid() && subKey2.isValid() )
        {
            nError += compareKeys(subKey1, subKey2);
        }
        subKey1.closeKey();
        subKey2.closeKey();
        iter++;
    }

    return nError;
}

#if (defined UNX) || (defined OS2)
void main( int argc, char * argv[] )
#else

void _cdecl main( int argc, char * argv[] )
#endif
{
    if ( !options.initOptions(argc, argv) )
    {
        exit(1);
    }

    OUString regName1( S2U(options.getRegName1()) );
    OUString regName2( S2U(options.getRegName2()) );

    RegistryLoader regLoader;
    Registry reg1(regLoader);
    Registry reg2(regLoader);

    if ( reg1.open(regName1, REG_READONLY) )
    {
        fprintf(stderr, "%s: open registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getRegName1().getStr());
        exit(2);
    }
    if ( reg2.open(regName2, REG_READONLY) )
    {
        fprintf(stderr, "%s: open registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getRegName2().getStr());
        exit(3);
    }

    RegistryKey key1, key2;
    if ( reg1.openRootKey(key1) )
    {
        fprintf(stderr, "%s: open root key of registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getRegName1().getStr());
        exit(4);
    }
    if ( reg2.openRootKey(key2) )
    {
        fprintf(stderr, "%s: open root key of registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getRegName2().getStr());
        exit(5);
    }
    if ( options.isStartKeyValid() )
    {
        if ( options.isExcludeKeyValid() &&
             (options.getStartKey() == options.getExcludeKey()) )
        {
            fprintf(stderr, "%s: start key is equal to exclude key\n",
                    options.getProgramName().getStr());
            exit(6);
        }
        RegistryKey sk1, sk2;
        if ( key1.openKey(S2U(options.getStartKey()), sk1) )
        {
            fprintf(stderr, "%s: open start key of registry \"%s\" failed\n",
                    options.getProgramName().getStr(), options.getRegName1().getStr());
            exit(7);
        }
        if ( key2.openKey(S2U(options.getStartKey()), sk2) )
        {
            fprintf(stderr, "%s: open start key of registry \"%s\" failed\n",
                    options.getProgramName().getStr(), options.getRegName2().getStr());
            exit(8);
        }

        key1 = sk1;
        key2 = sk2;
    }

    sal_uInt32 nError = 0;
    if ( nError = compareKeys(key1, key2) )
    {
        fprintf(stderr, "%s: registries contain %d differences!\n",
                options.getProgramName().getStr(), nError);
    } else
    {
        fprintf(stderr, "%s: registries are equal!\n",
                options.getProgramName().getStr());
    }

    key1.closeKey();
    key2.closeKey();
    if ( reg1.close() )
    {
        fprintf(stderr, "%s: closing registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getRegName1().getStr());
        exit(9);
    }
    if ( reg2.close() )
    {
        fprintf(stderr, "%s: closing registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getRegName2().getStr());
        exit(10);
    }

    if ( nError > 0 )
        exit(11);
    else
        exit(0);
}


