/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_registry.hxx"

#include <stdio.h>
#include <string.h>

#include <set>
#include <vector>
#include "registry/registry.hxx"
#include "registry/reader.hxx"
#include "registry/version.h"
#include <rtl/ustring.hxx>
#include <rtl/alloc.h>
#include <osl/process.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.hxx>

#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

using namespace ::rtl;
using namespace ::osl;

OUString shortName(const OUString& fullName)
{
    return fullName.copy(fullName.lastIndexOf('/') + 1);
}

sal_Bool isFileUrl(const OString& fileName)
{
    if (fileName.indexOf("file://") == 0 )
        return sal_True;
    return sal_False;
}

OUString convertToFileUrl(const OString& fileName)
{
    if ( isFileUrl(fileName) )
    {
        return OStringToOUString(fileName, osl_getThreadTextEncoding());
    }

    OUString uUrlFileName;
    OUString uFileName(fileName.getStr(), fileName.getLength(), osl_getThreadTextEncoding());
    if ( fileName.indexOf('.') == 0 || fileName.indexOf(SEPARATOR) < 0 )
    {
        OUString uWorkingDir;
        if (osl_getProcessWorkingDir(&uWorkingDir.pData) != osl_Process_E_None)
        {
            OSL_ASSERT(false);
        }
        if (FileBase::getAbsoluteFileURL(uWorkingDir, uFileName, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    } else
    {
        if (FileBase::getFileURLFromSystemPath(uFileName, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    }

    return uUrlFileName;
}

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

typedef ::std::set< OUString > StringSet;

class Options
{
public:
    Options()
        : m_bFullCheck(sal_False)
        , m_bForceOutput(sal_False)
        , m_bUnoTypeCheck(sal_False)
        , m_checkUnpublished(false)
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
    sal_Bool existsExcludeKeys()
        { return !m_excludeKeys.empty(); };
    StringSet& getExcludeKeys()
        { return m_excludeKeys; }
    sal_Bool matchedWithExcludeKey( const OUString& keyName);
    sal_Bool fullCheck()
        { return m_bFullCheck; }
    sal_Bool forceOutput()
        { return m_bForceOutput; }
    sal_Bool unoTypeCheck()
        { return m_bUnoTypeCheck; }
    bool checkUnpublished() const { return m_checkUnpublished; }
protected:
    OString     m_program;
    OString     m_regName1;
    OString     m_regName2;
    OString     m_startKey;
    StringSet   m_excludeKeys;
    sal_Bool    m_bFullCheck;
    sal_Bool    m_bForceOutput;
    sal_Bool    m_bUnoTypeCheck;
    bool m_checkUnpublished;
};

sal_Bool Options::initOptions(int ac, char* av[], sal_Bool bCmdFile)
{
    sal_Bool bRet = sal_True;
    sal_uInt16  i=0;

    if (!bCmdFile)
    {
        bCmdFile = sal_True;

        OString name(av[0]);
        sal_Int32 index = name.lastIndexOf(SEPARATOR);
        m_program = name.copy((index > 0 ? index+1 : 0));

        if (ac < 2)
        {
            fprintf(stdout, "%s", prepareHelp().getStr());
            bRet = sal_False;
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
                case 'r':
                case 'R':
                {
                    sal_Bool bFirst = sal_True;
                    if (av[i][2] == '2')
                    {
                        bFirst = sal_False;
                    } else if (av[i][2] != '1')
                    {
                        fprintf(stdout, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                    }
                    if (av[i][3] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            fprintf(stdout, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
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
                            fprintf(stdout, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
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
                            fprintf(stdout, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                            bRet = sal_False;
                            break;
                        }
                    } else
                    {
                        s = av[i] + 2;
                    }
                    m_excludeKeys.insert(S2U(s));
                    break;
                case 'c':
                case 'C':
                    if (av[i][2] != '\0')
                    {
                        fprintf(stdout, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                    }
                    m_bFullCheck = sal_True;
                    break;
                case 'f':
                case 'F':
                    if (av[i][2] != '\0')
                    {
                        fprintf(stdout, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                        bRet = sal_False;
                    }
                    m_bForceOutput = sal_True;
                    break;
                case 't':
                case 'T':
                    if (av[i][2] != '\0')
                    {
                        fprintf(stdout, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                    }
                    m_bUnoTypeCheck = sal_True;
                    break;
                case 'u':
                case 'U':
                    if (av[i][2] != '\0')
                    {
                        fprintf(stdout, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                    }
                    m_checkUnpublished = true;
                    break;
                case 'h':
                case '?':
                    if (av[i][2] != '\0')
                    {
                        fprintf(stdout, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                        bRet = sal_False;
                    } else
                    {
                        fprintf(stdout, "%s", prepareHelp().getStr());
                        exit(0);
                    }
                    break;
                default:
                    fprintf(stdout, "%s: unknown option '%s'\n", m_program.getStr(), av[i]);
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
                    fprintf(stdout, "%s", prepareHelp().getStr());
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

                    for (long j=0; j < rargc; j++)
                    {
                        free(rargv[j]);
                    }
                }
            } else
            {
                fprintf(stdout, "%s: unknown option '%s'\n", m_program.getStr(), av[i]);
                bRet = sal_False;
            }
        }
    }

    if ( bRet )
    {
        if ( m_regName1.getLength() == 0 )
        {
              fprintf(stdout, "%s: missing option '-r1'\n", m_program.getStr());
            bRet = sal_False;
        }
        if ( m_regName2.getLength() == 0 )
        {
              fprintf(stdout, "%s: missing option '-r2'\n", m_program.getStr());
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
    help += "     |S<name>   is specified the comparison starts with the root key.\n";
    help += "    -x<name>  = name specifies the name of a key which won't be compared. All\n";
    help += "     |X<name>   subkeys won't be compared also. This option can be used more than once.\n";
    help += "    -f|F      = force the detailed output of any diffenrences. Default\n";
    help += "                is that only the number of differences is returned.\n";
    help += "    -c|C      = make a complete check, that means any differences will be\n";
    help += "                detected. Default is only a compatibility check that means\n";
    help += "                only UNO typelibrary entries will be checked.\n";
    help += "    -t|T      = make an UNO type compatiblity check. This means that registry 2\n";
    help += "                will be checked against registry 1. If a interface in r2 contains\n";
    help += "                more methods or the methods are in a different order as in r1, r2 is\n";
    help += "                incompatible to r1. But if a service in r2 supports more properties as\n";
    help += "                in r1 and the new properties are 'optonal' it is compatible.\n";
    help += "    -u|U      = additionally check types that are unpublished in registry 1.\n";
    help += "    -h|-?     = print this help message and exit.\n\n";
    help += prepareVersion();

    return help;
}

OString Options::prepareVersion()
{
    OString version(m_program);
    version += " Version 1.0\n\n";
    return version;
}

sal_Bool Options::matchedWithExcludeKey( const OUString& keyName)
{
    if ( m_excludeKeys.empty() )
        return sal_False;

    StringSet::const_iterator iter = m_excludeKeys.begin();
    StringSet::const_iterator end = m_excludeKeys.end();

    while ( iter != end )
    {
        if ( keyName.indexOf(*iter) == 0)
            return sal_True;

        ++iter;
    }

    return sal_False;
}

static Options options;

static char const * getTypeClass(RTTypeClass typeClass)
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
        default:
            return "INVALID";
    }
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

static char const * getConstValueType(RTConstValue& constValue)
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
        default:
            return "NONE";
    }
}
static void printConstValue(RTConstValue& constValue)
{
    switch (constValue.m_type)
    {
        case RT_TYPE_NONE:
            fprintf(stdout, "none");
            break;
        case RT_TYPE_BOOL:
            fprintf(stdout, "%s", constValue.m_value.aBool ? "TRUE" : "FALSE");
            break;
        case RT_TYPE_BYTE:
            fprintf(stdout, "%d", constValue.m_value.aByte);
            break;
        case RT_TYPE_INT16:
            fprintf(stdout, "%d", constValue.m_value.aShort);
            break;
        case RT_TYPE_UINT16:
            fprintf(stdout, "%d", constValue.m_value.aUShort);
            break;
        case RT_TYPE_INT32:
            fprintf(
                stdout, "%ld",
                sal::static_int_cast< long >(constValue.m_value.aLong));
            break;
        case RT_TYPE_UINT32:
            fprintf(
                stdout, "%lu",
                sal::static_int_cast< unsigned long >(
                    constValue.m_value.aULong));
            break;
//      case RT_TYPE_INT64:
//          fprintf(stdout, "%d", constValue.m_value.aHyper);
//      case RT_TYPE_UINT64:
//          fprintf(stdout, "%d", constValue.m_value.aUHyper);
        case RT_TYPE_FLOAT:
            fprintf(stdout, "%f", constValue.m_value.aFloat);
            break;
        case RT_TYPE_DOUBLE:
            fprintf(stdout, "%f", constValue.m_value.aDouble);
            break;
        case RT_TYPE_STRING:
            fprintf(
                stdout, "%s",
                (rtl::OUStringToOString(
                    constValue.m_value.aString, RTL_TEXTENCODING_UTF8).
                 getStr()));
            break;
        default:
            break;
    }
}

static sal_uInt32 checkConstValue(const OUString& keyName,
                                  RTTypeClass typeClass,
                                  sal_Bool& bDump,
                                  RTConstValue& constValue1,
                                  RTConstValue& constValue2,
                                  sal_uInt16 index1)
{
    switch (constValue1.m_type)
    {
        case RT_TYPE_INVALID:
            break;
        case RT_TYPE_BOOL:
            if (constValue1.m_value.aBool != constValue2.m_value.aBool)
            {
                if ( options.forceOutput() && !options.unoTypeCheck() )
                {
                    if ( bDump )
                    {
                        fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stdout, "  Field %d: Value1 = %s  !=  Value2 = %s\n", index1,
                            constValue1.m_value.aBool ? "TRUE" : "FALSE",
                            constValue2.m_value.aBool ? "TRUE" : "FALSE");
                }
                return 1;
            }
            break;
        case RT_TYPE_BYTE:
            if (constValue1.m_value.aByte != constValue2.m_value.aByte)
            {
                if ( options.forceOutput() && !options.unoTypeCheck() )
                {
                    if ( bDump )
                    {
                        fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stdout, "  Field %d: Value1 = %d  !=  Value2 = %d\n", index1,
                            constValue1.m_value.aByte, constValue2.m_value.aByte);
                }
                return 1;
            }
            break;
        case RT_TYPE_INT16:
            if (constValue1.m_value.aShort != constValue2.m_value.aShort)
            {
                if ( options.forceOutput() && !options.unoTypeCheck() )
                {
                    if ( bDump )
                    {
                        fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stdout, "  Field %d: Value1 = %d  !=  Value2 = %d\n", index1,
                            constValue1.m_value.aShort, constValue2.m_value.aShort);
                }
                return 1;
            }
            break;
        case RT_TYPE_UINT16:
            if (constValue1.m_value.aUShort != constValue2.m_value.aUShort)
            {
                if ( options.forceOutput() && !options.unoTypeCheck() )
                {
                    if ( bDump )
                    {
                        fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stdout, "  Field %d: Value1 = %d  !=  Value2 = %d\n", index1,
                            constValue1.m_value.aUShort, constValue2.m_value.aUShort);
                }
                return 1;
            }
            break;
        case RT_TYPE_INT32:
            if (constValue1.m_value.aLong != constValue2.m_value.aLong)
            {
                if ( options.forceOutput() && !options.unoTypeCheck() )
                {
                    if ( bDump )
                    {
                        fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stdout, "  Field %d: Value1 = %ld  !=  Value2 = %ld\n", index1,
                            sal::static_int_cast< long >(constValue1.m_value.aLong),
                            sal::static_int_cast< long >(constValue2.m_value.aLong));
                }
                return 1;
            }
            break;
        case RT_TYPE_UINT32:
            if (constValue1.m_value.aULong != constValue2.m_value.aULong)
            {
                if ( options.forceOutput() && !options.unoTypeCheck() )
                {
                    if ( bDump )
                    {
                        fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stdout, "  Field %d: Value1 = %lu  !=  Value2 = %lu\n", index1,
                            sal::static_int_cast< unsigned long >(constValue1.m_value.aULong),
                            sal::static_int_cast< unsigned long >(constValue2.m_value.aULong));
                }
                return 1;
            }
            break;
        case RT_TYPE_INT64:
            if (constValue1.m_value.aHyper != constValue2.m_value.aHyper)
            {
                if ( options.forceOutput() && !options.unoTypeCheck() )
                {
                    if ( bDump )
                    {
                        fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(
                        stdout, "  Field %d: Value1 = %s  !=  Value2 = %s\n",
                        index1,
                        rtl::OUStringToOString(
                            rtl::OUString::valueOf(constValue1.m_value.aHyper),
                            RTL_TEXTENCODING_ASCII_US).getStr(),
                        rtl::OUStringToOString(
                            rtl::OUString::valueOf(constValue2.m_value.aHyper),
                            RTL_TEXTENCODING_ASCII_US).getStr());
                }
                return 1;
            }
            break;
        case RT_TYPE_UINT64:
            if (constValue1.m_value.aUHyper != constValue2.m_value.aUHyper)
            {
                if ( options.forceOutput() && !options.unoTypeCheck() )
                {
                    if ( bDump )
                    {
                        fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(
                        stdout, "  Field %d: Value1 = %s  !=  Value2 = %s\n",
                        index1,
                        rtl::OUStringToOString(
                            rtl::OUString::valueOf(
                                static_cast< sal_Int64 >(
                                    constValue1.m_value.aUHyper)),
                            RTL_TEXTENCODING_ASCII_US).getStr(),
                        rtl::OUStringToOString(
                            rtl::OUString::valueOf(
                                static_cast< sal_Int64 >(
                                    constValue2.m_value.aUHyper)),
                            RTL_TEXTENCODING_ASCII_US).getStr());
                        // printing the unsigned values as signed should be
                        // acceptable...
                }
                return 1;
            }
            break;
        case RT_TYPE_FLOAT:
            if (constValue1.m_value.aFloat != constValue2.m_value.aFloat)
            {
                if ( options.forceOutput() && !options.unoTypeCheck() )
                {
                    if ( bDump )
                    {
                        fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stdout, "  Field %d: Value1 = %f  !=  Value2 = %f\n", index1,
                            constValue1.m_value.aFloat, constValue2.m_value.aFloat);
                }
                return 1;
            }
            break;
        case RT_TYPE_DOUBLE:
            if (constValue1.m_value.aDouble != constValue2.m_value.aDouble)
            {
                if ( options.forceOutput() && !options.unoTypeCheck() )
                {
                    if ( bDump )
                    {
                        fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stdout, "  Field %d: Value1 = %f  !=  Value2 = %f\n", index1,
                            constValue1.m_value.aDouble, constValue2.m_value.aDouble);
                }
                return 1;
            }
            break;
        default:
            OSL_ASSERT(false);
            break;
    }
    return 0;
}

static sal_uInt32 checkField(const OUString& keyName,
                             RTTypeClass typeClass,
                             sal_Bool& bDump,
                             typereg::Reader& reader1,
                             typereg::Reader& reader2,
                             sal_uInt16 index1,
                             sal_uInt16 index2)
{
    sal_uInt32 nError = 0;
    if ( reader1.getFieldName(index1) !=
         reader2.getFieldName(index2) )
    {
        if ( options.forceOutput() && !options.unoTypeCheck() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  Field %d: Name1 = %s  !=  Name2 = %s\n", index1,
                    U2S(reader1.getFieldName(index1)), U2S(reader2.getFieldName(index2)));
        }
        nError++;
    }
    if ( reader1.getFieldTypeName(index1) !=
         reader2.getFieldTypeName(index2) )
    {
        if ( options.forceOutput() && !options.unoTypeCheck() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  Field %d: Type1 = %s  !=  Type2 = %s\n", index1,
                    U2S(reader1.getFieldTypeName(index1)), U2S(reader2.getFieldTypeName(index2)));
        }
        nError++;
    } else
    {
        RTConstValue constValue1 = reader1.getFieldValue(index1);
        RTConstValue constValue2 = reader2.getFieldValue(index2);
        if ( constValue1.m_type != constValue2.m_type )
        {
            if ( options.forceOutput() && !options.unoTypeCheck() )
            {
                if ( bDump )
                {
                    fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stdout, "  Field %d: Access1 = %s  !=  Access2 = %s\n", index1,
                        getConstValueType(constValue1), getConstValueType(constValue2));
                fprintf(stdout, "  Field %d: Value1 = ", index1);
                printConstValue(constValue1);
                fprintf(stdout, "  !=  Value2 = ");
                printConstValue(constValue1);
                fprintf(stdout, "\n;");
            }
            nError++;
        } else
        {
            nError += checkConstValue(keyName, typeClass, bDump, constValue1, constValue2, index1);
        }
    }

    if ( reader1.getFieldFlags(index1) != reader2.getFieldFlags(index2) )
    {
        if ( options.forceOutput() && !options.unoTypeCheck() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  Field %d: FieldAccess1 = %s  !=  FieldAccess2 = %s\n", index1,
                    getFieldAccess(reader1.getFieldFlags(index1)).getStr(),
                    getFieldAccess(reader1.getFieldFlags(index2)).getStr());
        }
        nError++;
    }

    if ( options.fullCheck() &&
         (reader1.getFieldDocumentation(index1) != reader2.getFieldDocumentation(index2)) )
    {
        if ( options.forceOutput() && !options.unoTypeCheck() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  Field %d: Doku1 = %s\n             Doku2 = %s\n", index1,
                    U2S(reader1.getFieldDocumentation(index1)), U2S(reader2.getFieldDocumentation(index2)));
        }
        nError++;
    }
    return nError;
}

static char const * getMethodMode(RTMethodMode methodMode)
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
        default:
            return "INVALID";
    }
}

static char const * getParamMode(RTParamMode paramMode)
{
    switch ( paramMode )
    {
        case RT_PARAM_IN:
            return "IN";
        case RT_PARAM_OUT:
            return "OUT";
        case RT_PARAM_INOUT:
            return "INOUT";
        default:
            return "INVALID";
    }
}

static sal_uInt32 checkMethod(const OUString& keyName,
                              RTTypeClass typeClass,
                              sal_Bool& bDump,
                              typereg::Reader& reader1,
                              typereg::Reader& reader2,
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
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  Method1 %d: Name1 = %s  !=  Name2 = %s\n", index,
                    U2S(reader1.getMethodName(index)),
                    U2S(reader2.getMethodName(index)));
        }
        nError++;
    }

    if ( reader1.getMethodReturnTypeName(index) !=
         reader2.getMethodReturnTypeName(index) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  Method1 %d: ReturnType1 = %s  !=  ReturnType2 = %s\n", index,
                    U2S(reader1.getMethodReturnTypeName(index)),
                    U2S(reader2.getMethodReturnTypeName(index)));
        }
        nError++;
    }

    sal_uInt16 nParams1 = (sal_uInt16)reader1.getMethodParameterCount(index);
    sal_uInt16 nParams2 = (sal_uInt16)reader2.getMethodParameterCount(index);
    if ( nParams1 != nParams2 )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  Method %d : nParameters1 = %d  !=  nParameters2 = %d\n", index, nParams1, nParams2);
        }
        nError++;
    }
    sal_uInt16 i=0;
    for (i=0; i < nParams1 && i < nParams2; i++)
    {
        if ( reader1.getMethodParameterTypeName(index, i) != reader2.getMethodParameterTypeName(index, i) )
        {
            if ( options.forceOutput() )
            {
                if ( bDump )
                {
                    fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stdout, "  Method %d, Parameter %d: Type1 = %s  !=  Type2 = %s\n", index, i,
                        U2S(reader1.getMethodParameterTypeName(index, i)),
                        U2S(reader2.getMethodParameterTypeName(index, i)));
            }
            nError++;
        }
        if ( options.fullCheck() &&
             (reader1.getMethodParameterName(index, i) != reader2.getMethodParameterName(index, i)) )
        {
            if ( options.forceOutput() )
            {
                if ( bDump )
                {
                    fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stdout, "  Method %d, Parameter %d: Name1 = %s  !=  Name2 = %s\n", index, i,
                        U2S(reader1.getMethodParameterName(index, i)),
                        U2S(reader2.getMethodParameterName(index, i)));
            }
            nError++;
        }
        if ( reader1.getMethodParameterFlags(index, i) != reader2.getMethodParameterFlags(index, i) )
        {
            if ( options.forceOutput() )
            {
                if ( bDump )
                {
                    fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stdout, "  Method %d, Parameter %d: Mode1 = %s  !=  Mode2 = %s\n", index, i,
                        getParamMode(reader1.getMethodParameterFlags(index, i)),
                        getParamMode(reader2.getMethodParameterFlags(index, i)));
            }
            nError++;
        }
    }
    if ( i < nParams1 && options.forceOutput() )
    {
        if ( bDump )
        {
            fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
            bDump = sal_False;
        }
        fprintf(stdout, "  Registry1: Method %d contains %d more parameters\n", index, nParams1 - i);
    }
    if ( i < nParams2 && options.forceOutput() )
    {
        if ( bDump )
        {
            fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
            bDump = sal_False;
        }
        fprintf(stdout, "  Registry2: Method %d contains %d more parameters\n", index, nParams2 - i);
    }

    sal_uInt16 nExcep1 = (sal_uInt16)reader1.getMethodExceptionCount(index);
    sal_uInt16 nExcep2 = (sal_uInt16)reader2.getMethodExceptionCount(index);
    if ( nExcep1 != nExcep2 )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  nExceptions1 = %d  !=  nExceptions2 = %d\n", nExcep1, nExcep2);
        }
        nError++;
    }
    for (i=0; i < nExcep1 && i < nExcep2; i++)
    {
        if ( reader1.getMethodExceptionTypeName(index, i) != reader2.getMethodExceptionTypeName(index, i) )
        {
            if ( options.forceOutput() )
            {
                if ( bDump )
                {
                    fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stdout, "  Method %d, Exception %d: Name1 = %s  !=  Name2 = %s\n", index, i,
                        U2S(reader1.getMethodExceptionTypeName(index, i)),
                        U2S(reader2.getMethodExceptionTypeName(index, i)));
            }
            nError++;
        }
    }
    if ( i < nExcep1 && options.forceOutput() )
    {
        if ( bDump )
        {
            fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
            bDump = sal_False;
        }
        fprintf(stdout, "  Registry1: Method %d contains %d more exceptions\n", index, nExcep1 - i);
    }
    if ( i < nExcep2 && options.forceOutput() )
    {
        if ( bDump )
        {
            fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
            bDump = sal_False;
        }
        fprintf(stdout, "  Registry2: Method %d contains %d more exceptions\n", index, nExcep2 - i);
    }

    if ( reader1.getMethodFlags(index) != reader2.getMethodFlags(index) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  Method %d: Mode1 = %s  !=  Mode2 = %s\n", index,
                    getMethodMode(reader1.getMethodFlags(index)),
                    getMethodMode(reader2.getMethodFlags(index)));
        }
        nError++;
    }

    if ( options.fullCheck() &&
         (reader1.getMethodDocumentation(index) != reader2.getMethodDocumentation(index)) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  Method %d: Doku1 = %s\n              Doku2 = %s\n", index,
                    U2S(reader1.getMethodDocumentation(index)),
                    U2S(reader2.getMethodDocumentation(index)));
        }
        nError++;
    }
    return nError;
}

static char const * getReferenceType(RTReferenceType refType)
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
        default:
            return "RT_REF_INVALID";
    }
}

static sal_uInt32 checkReference(const OUString& keyName,
                                 RTTypeClass typeClass,
                                 sal_Bool& bDump,
                                 typereg::Reader& reader1,
                                    typereg::Reader& reader2,
                                    sal_uInt16 index1,
                                 sal_uInt16 index2)
{
    sal_uInt32 nError = 0;
    if ( reader1.getReferenceTypeName(index1) !=
         reader2.getReferenceTypeName(index2) )
    {
        if ( options.forceOutput() && !options.unoTypeCheck() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  Reference %d: Name1 = %s  !=  Name2 = %s\n", index1,
                    U2S(reader1.getReferenceTypeName(index1)),
                    U2S(reader2.getReferenceTypeName(index2)));
        }
        nError++;
    }
    if ( reader1.getReferenceTypeName(index1) !=
         reader2.getReferenceTypeName(index2) )
    {
        if ( options.forceOutput() && !options.unoTypeCheck() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  Reference %d: Type1 = %s  !=  Type2 = %s\n", index1,
                    getReferenceType(reader1.getReferenceSort(index1)),
                    getReferenceType(reader2.getReferenceSort(index2)));
        }
        nError++;
    }
    if ( options.fullCheck() &&
         (reader1.getReferenceDocumentation(index1) != reader2.getReferenceDocumentation(index2)) )
    {
        if ( options.forceOutput() && !options.unoTypeCheck() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  Reference %d: Doku1 = %s\n                 Doku2 = %s\n", index1,
                    U2S(reader1.getReferenceDocumentation(index1)),
                    U2S(reader2.getReferenceDocumentation(index2)));
        }
        nError++;
    }
    if ( reader1.getReferenceFlags(index1) !=
         reader2.getReferenceFlags(index2) )
    {
        if ( options.forceOutput() && !options.unoTypeCheck() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "  Reference %d: Access1 = %s  !=  Access2 = %s\n", index1,
                    getFieldAccess(reader1.getReferenceFlags(index1)).getStr(),
                    getFieldAccess(reader1.getReferenceFlags(index2)).getStr());
        }
        nError++;
    }
    return nError;
}

static sal_uInt32 checkFieldsWithoutOrder(const OUString& keyName,
                                          RTTypeClass typeClass,
                                          sal_Bool& bDump,
                                          typereg::Reader& reader1,
                                          typereg::Reader& reader2)
{
    sal_uInt32 nError = 0;

    sal_uInt16 nFields1 = (sal_uInt16)reader1.getFieldCount();
    sal_uInt16 nFields2 = (sal_uInt16)reader2.getFieldCount();
    sal_uInt16 i=0, j=0;

    if ( nFields1 > nFields2 )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
           fprintf(stdout, "  %s1 contains %d more properties as %s2\n",
                   getTypeClass(typeClass), nFields1-nFields2, getTypeClass(typeClass));
        }
    }

    sal_Bool bFound = sal_False;
    ::std::set< sal_uInt16 > moreProps;

    for (i=0; i < nFields1; i++)
    {
        for (j=0; j < nFields2; j++)
        {
            if (!checkField(keyName, typeClass, bDump, reader1, reader2, i, j))
            {
                bFound =  sal_True;
                moreProps.insert(j);
                break;
            }
        }
        if (!bFound)
        {
            if (options.forceOutput())
            {
                if ( bDump )
                {
                    fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stdout, "  incompatible change: Field %d ('%s') of r1 is not longer a property of this %s in r2\n",
                        i, U2S(shortName(reader1.getFieldName(i))), getTypeClass(typeClass));
            }
            nError++;
        } else
        {
            bFound = sal_False;
        }
    }

    if ( typeClass == RT_TYPE_SERVICE && !moreProps.empty() )
    {
        for (j=0; j < nFields2; j++)
        {
            if ( moreProps.find(j) == moreProps.end() )
            {
                if ( (reader2.getFieldFlags(j) & RT_ACCESS_OPTIONAL) != RT_ACCESS_OPTIONAL )
                {
                    if ( options.forceOutput() )
                    {
                        if ( bDump )
                        {
                            fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                            bDump = sal_False;
                        }
                        fprintf(stdout, "  incompatible change: Field %d ('%s') of r2 is a new property compared to this %s in r1 and is not 'optional'\n",
                                j, U2S(shortName(reader2.getFieldName(j))), getTypeClass(typeClass));
                    }
                    nError++;
                }
            }
        }
    }

    return nError;
}

static sal_uInt32 checkBlob(const OUString& keyName, typereg::Reader& reader1, sal_uInt32 size1,
                            typereg::Reader& reader2, sal_uInt32 size2)
{
    sal_uInt32 nError = 0;
    sal_Bool bDump = sal_True;

    if ( options.fullCheck() && (size1 != size2) )
    {
        if ( options.forceOutput() )
        {
            fprintf(
                stdout, "    Size1 = %lu    Size2 = %lu\n",
                sal::static_int_cast< unsigned long >(size1),
                sal::static_int_cast< unsigned long >(size2));
        }
    }
    if (reader1.isPublished()) {
        if (!reader2.isPublished()) {
            if (options.forceOutput()) {
                if (bDump) {
                    fprintf(stdout, "?: %s\n", U2S(keyName));
                    bDump = false;
                }
                fprintf(stdout, "    published in 1 but unpublished in 2\n");
            }
            ++nError;
        }
    } else if (!options.checkUnpublished()) {
        return nError;
    }
    if ( reader1.getTypeClass() != reader2.getTypeClass() )
    {
        if ( options.forceOutput() )
        {
            if (bDump) {
                fprintf(stdout, "?: %s\n", U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "    TypeClass1 = %s  !=  TypeClass2 = %s\n",
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
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "    TypeName1 = %s  !=  TypeName2 = %s\n",
                    U2S(reader1.getTypeName()), U2S(reader2.getTypeName()));
        }
        nError++;
    }
    if ( (typeClass == RT_TYPE_INTERFACE ||
          typeClass == RT_TYPE_STRUCT ||
          typeClass == RT_TYPE_EXCEPTION) )
    {
        if (reader1.getSuperTypeCount() != reader2.getSuperTypeCount()) {
            if (bDump) {
                fprintf(
                    stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = false;
            }
            fprintf(
                stdout, "    SuperTypeCount1 = %d  !=  SuperTypeCount2 = %d\n",
                static_cast< int >(reader1.getSuperTypeCount()),
                static_cast< int >(reader2.getSuperTypeCount()));
            ++nError;
        } else {
            for (sal_Int16 i = 0; i < reader1.getSuperTypeCount(); ++i) {
                if (reader1.getSuperTypeName(i) != reader2.getSuperTypeName(i))
                {
                    if ( options.forceOutput() )
                    {
                        if ( bDump )
                        {
                            fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                            bDump = sal_False;
                        }
                        fprintf(stdout, "    SuperTypeName1 = %s  !=  SuperTypeName2 = %s\n",
                                U2S(reader1.getSuperTypeName(i)), U2S(reader2.getSuperTypeName(i)));
                    }
                    nError++;
                }
            }
        }
    }
    sal_uInt16 nFields1 = (sal_uInt16)reader1.getFieldCount();
    sal_uInt16 nFields2 = (sal_uInt16)reader2.getFieldCount();
    sal_Bool bCheckNormal = sal_True;

    if ( (typeClass == RT_TYPE_SERVICE ||
          typeClass == RT_TYPE_MODULE ||
          typeClass == RT_TYPE_CONSTANTS) && options.unoTypeCheck() )
    {
        bCheckNormal = sal_False;
    }

    if ( bCheckNormal )
    {
        if ( nFields1 != nFields2 )
        {
            if ( options.forceOutput() )
            {
                if ( bDump )
                {
                    fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stdout, "    nFields1 = %d  !=  nFields2 = %d\n", nFields1, nFields2);
            }
            nError++;
        }
        sal_uInt16 i;
        for (i=0; i < nFields1 && i < nFields2; i++)
        {
            nError += checkField(keyName, typeClass, bDump, reader1, reader2, i, i);
        }
        if ( i < nFields1 && options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "    Registry1 contains %d more fields\n", nFields1 - i);
        }
        if ( i < nFields2 && options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "    Registry2 contains %d more fields\n", nFields2 - i);
        }
    } else
    {
        nError += checkFieldsWithoutOrder(keyName, typeClass, bDump, reader1, reader2);
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
                    fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                    bDump = sal_False;
                }
                fprintf(stdout, "    nMethods1 = %d  !=  nMethods2 = %d\n", nMethods1, nMethods2);
            }
            nError++;
        }
        sal_uInt16 i;
        for (i=0; i < nMethods1 && i < nMethods2; i++)
        {
            nError += checkMethod(keyName, typeClass, bDump, reader1, reader2, i);
        }
        if ( i < nMethods1 && options.forceOutput() )
        {
            fprintf(stdout, "    Registry1 contains %d more methods\n", nMethods1 - i);
        }
        if ( i < nMethods2 && options.forceOutput() )
        {
            fprintf(stdout, "    Registry2 contains %d more methods\n", nMethods2 - i);
        }
    }
    if ( typeClass == RT_TYPE_SERVICE )
    {
        sal_uInt16 nReference1 = (sal_uInt16)reader1.getReferenceCount();
        sal_uInt16 nReference2 = (sal_uInt16)reader2.getReferenceCount();

        if ( !bCheckNormal )
        {
            sal_uInt16 i=0, j=0;

            if ( nReference1 > nReference2 )
            {
                if ( options.forceOutput() )
                {
                    if ( bDump )
                    {
                        fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stdout, "    service1 contains %d more references as service2\n",
                            nReference1-nReference2);
                }
            }

            sal_Bool bFound = sal_False;
            ::std::set< sal_uInt16 > moreReferences;

            for (i=0; i < nReference1; i++)
            {
                for (j=0; j < nReference2; j++)
                {
                    if (!checkReference(keyName, typeClass, bDump, reader1, reader2, i, j))
                    {
                        bFound =  sal_True;
                        moreReferences.insert(j);
                        break;
                    }
                }
                if (!bFound)
                {
                    if (options.forceOutput())
                    {
                        if ( bDump )
                        {
                            fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                            bDump = sal_False;
                        }
                        fprintf(stdout, "  incompatible change: Reference %d ('%s') in 'r1' is not longer a reference of this service in 'r2'\n",
                                i, U2S(shortName(reader1.getReferenceTypeName(i))));
                    }
                    nError++;
                } else
                {
                    bFound = sal_False;
                }
            }

            if ( !moreReferences.empty() )
            {
                for (j=0; j < nReference2; j++)
                {
                    if ( moreReferences.find(j) == moreReferences.end() )
                    {
                        if ( (reader2.getReferenceFlags(j) & RT_ACCESS_OPTIONAL) != RT_ACCESS_OPTIONAL )
                        {
                            if ( options.forceOutput() )
                            {
                                if ( bDump )
                                {
                                    fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                                    bDump = sal_False;
                                }
                                fprintf(stdout, "  incompatible change: Reference %d ('%s') of r2 is a new reference compared to this service in r1 and is not 'optional'\n",
                                        j, U2S(shortName(reader2.getReferenceTypeName(j))));
                            }
                            nError++;
                        }
                    }
                }
            }
        } else
        {
            if ( nReference1 != nReference2 )
            {
                if ( options.forceOutput() )
                {
                    if ( bDump )
                    {
                        fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                        bDump = sal_False;
                    }
                    fprintf(stdout, "    nReferences1 = %d  !=  nReferences2 = %d\n", nReference1, nReference2);
                }
                nError++;
            }
            sal_uInt16 i;
            for (i=0; i < nReference1 && i < nReference2; i++)
            {
                nError += checkReference(keyName, typeClass, bDump, reader1, reader2, i, i);
            }
            if ( i < nReference1 && options.forceOutput() )
            {
                fprintf(stdout, "    Registry1 contains %d more references\n", nReference1 - i);
            }
            if ( i < nReference2 && options.forceOutput() )
            {
                fprintf(stdout, "    Registry2 contains %d more references\n", nReference2 - i);
            }
        }
    }

    if ( options.fullCheck() && (reader1.getDocumentation() != reader2.getDocumentation()) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "    Doku1 = %s\n    Doku2 = %s\n",
                    U2S(reader1.getDocumentation()), U2S(reader2.getDocumentation()));
        }
        nError++;
    }
/*
    if ( nError &&
         (!keyName.compareTo(OUString::createFromAscii("/UCR/drafts"), 11) ||
          !keyName.compareTo(OUString::createFromAscii("/drafts"), 7)) )
    {
        if ( options.forceOutput() )
        {
            if ( bDump )
            {
                fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
                bDump = sal_False;
            }
            fprintf(stdout, "    Note: \"drafts\" type changed incompatible, no effect to the final API\n");
        }
        return 0;
    }
*/
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
        default:
            break;
        }

        if ( bEqual)
        {
            RegValue value1 = rtl_allocateMemory(size1);
            RegValue value2 = rtl_allocateMemory(size2);

            key1.getValue(tmpName, value1);
            key2.getValue(tmpName, value2);

            bEqual = (rtl_compareMemory(value1, value2, size1) == 0 );

            if ( !bEqual && valueType1 == RG_VALUETYPE_BINARY && valueType2 == RG_VALUETYPE_BINARY )
            {
                typereg::Reader reader1(
                    value1, size1, false, TYPEREG_VERSION_1);
                typereg::Reader reader2(
                    value2, size2, false, TYPEREG_VERSION_1);

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
                    fprintf(stdout, "Difference: key values of key \"%s\" are different\n",
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
            fprintf(stdout, "    Registry 1: key has no value\n");
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
                fprintf(stdout, "    Registry 1: Value: Type = RG_VALUETYPE_LONG\n");
                fprintf(
                    stdout, "                       Size = %lu\n",
                    sal::static_int_cast< unsigned long >(size1));
                fprintf(stdout, "                       Data = %p\n", value1);
                break;
            case RG_VALUETYPE_STRING:
                fprintf(stdout, "    Registry 1: Value: Type = RG_VALUETYPE_STRING\n");
                fprintf(
                    stdout, "                       Size = %lu\n",
                    sal::static_int_cast< unsigned long >(size1));
                fprintf(stdout, "                       Data = \"%s\"\n", (sal_Char*)value1);
                break;
            case RG_VALUETYPE_UNICODE:
                {
                OUString uStrValue((sal_Unicode*)value1);
                fprintf(stdout, "    Registry 1: Value: Type = RG_VALUETYPE_UNICODE\n");
                fprintf(
                    stdout, "                       Size = %lu\n",
                    sal::static_int_cast< unsigned long >(size1));
                fprintf(stdout, "                       Data = \"%s\"\n", U2S(uStrValue));
                }
                break;
            default:
                OSL_ASSERT(false);
                break;
            }

            rtl_freeMemory(value1);
        }
            break;
        case RG_VALUETYPE_BINARY:
            fprintf(stdout, "    Registry 1: Value: Type = RG_VALUETYPE_BINARY\n");
            break;
        case RG_VALUETYPE_LONGLIST:
            {
            RegistryValueList<sal_Int32> valueList;
            key1.getLongListValue(tmpName, valueList);
            fprintf(stdout, "    Registry 1: Value: Type = RG_VALUETYPE_LONGLIST\n");
            fprintf(
                stdout, "                       Size = %lu\n",
                sal::static_int_cast< unsigned long >(size1));
            sal_uInt32 length = valueList.getLength();
            for (sal_uInt32 i=0; i<length; i++)
            {
                fprintf(
                    stdout, "                       Data[%lu] = %ld\n",
                    sal::static_int_cast< unsigned long >(i),
                    sal::static_int_cast< long >(valueList.getElement(i)));
            }
            }
            break;
        case RG_VALUETYPE_STRINGLIST:
            {
            RegistryValueList<sal_Char*> valueList;
            key1.getStringListValue(tmpName, valueList);
            fprintf(stdout, "    Registry 1: Value: Type = RG_VALUETYPE_STRINGLIST\n");
            fprintf(
                stdout, "                       Size = %lu\n",
                sal::static_int_cast< unsigned long >(size1));
            sal_uInt32 length = valueList.getLength();
            for (sal_uInt32 i=0; i<length; i++)
            {
                fprintf(
                    stdout, "                       Data[%lu] = \"%s\"\n",
                    sal::static_int_cast< unsigned long >(i),
                    valueList.getElement(i));
            }
            }
            break;
        case RG_VALUETYPE_UNICODELIST:
            {
            RegistryValueList<sal_Unicode*> valueList;
            key1.getUnicodeListValue(tmpName, valueList);
            fprintf(stdout, "    Registry 1: Value: Type = RG_VALUETYPE_UNICODELIST\n");
            fprintf(
                stdout, "                       Size = %lu\n",
                sal::static_int_cast< unsigned long >(size1));
            sal_uInt32 length = valueList.getLength();
            OUString uStrValue;
            for (sal_uInt32 i=0; i<length; i++)
            {
                uStrValue = OUString(valueList.getElement(i));
                fprintf(
                    stdout, "                       Data[%lu] = \"%s\"\n",
                    sal::static_int_cast< unsigned long >(i), U2S(uStrValue));
            }
            }
            break;
        }

        switch (valueType2)
        {
        case RG_VALUETYPE_NOT_DEFINED:
            fprintf(stdout, "    Registry 2: key has no value\n");
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
                fprintf(stdout, "    Registry 2: Value: Type = RG_VALUETYPE_LONG\n");
                fprintf(
                    stdout, "                       Size = %lu\n",
                    sal::static_int_cast< unsigned long >(size2));
                fprintf(stdout, "                       Data = %p\n", value2);
                break;
            case RG_VALUETYPE_STRING:
                fprintf(stdout, "    Registry 2: Value: Type = RG_VALUETYPE_STRING\n");
                fprintf(
                    stdout, "                       Size = %lu\n",
                    sal::static_int_cast< unsigned long >(size2));
                fprintf(stdout, "                       Data = \"%s\"\n", (sal_Char*)value2);
                break;
            case RG_VALUETYPE_UNICODE:
                {
                OUString uStrValue((sal_Unicode*)value2);
                fprintf(stdout, "    Registry 2: Value: Type = RG_VALUETYPE_UNICODE\n");
                fprintf(
                    stdout, "                       Size = %lu\n",
                    sal::static_int_cast< unsigned long >(size2));
                fprintf(stdout, "                       Data = \"%s\"\n", U2S(uStrValue));
                }
                break;
            default:
                OSL_ASSERT(false);
                break;
            }

            rtl_freeMemory(value2);
        }
            break;
        case RG_VALUETYPE_BINARY:
            fprintf(stdout, "    Registry 2: Value: Type = RG_VALUETYPE_BINARY\n");
            break;
        case RG_VALUETYPE_LONGLIST:
            {
            RegistryValueList<sal_Int32> valueList;
            key2.getLongListValue(tmpName, valueList);
            fprintf(stdout, "    Registry 2: Value: Type = RG_VALUETYPE_LONGLIST\n");
            fprintf(
                stdout, "                       Size = %lu\n",
                sal::static_int_cast< unsigned long >(size2));
            sal_uInt32 length = valueList.getLength();
            for (sal_uInt32 i=0; i<length; i++)
            {
                fprintf(
                    stdout, "                       Data[%lu] = %ld\n",
                    sal::static_int_cast< unsigned long >(i),
                    sal::static_int_cast< long >(valueList.getElement(i)));
            }
            }
            break;
        case RG_VALUETYPE_STRINGLIST:
            {
            RegistryValueList<sal_Char*> valueList;
            key2.getStringListValue(tmpName, valueList);
            fprintf(stdout, "    Registry 2: Value: Type = RG_VALUETYPE_STRINGLIST\n");
            fprintf(
                stdout, "                       Size = %lu\n",
                sal::static_int_cast< unsigned long >(size2));
            sal_uInt32 length = valueList.getLength();
            for (sal_uInt32 i=0; i<length; i++)
            {
                fprintf(
                    stdout, "                       Data[%lu] = \"%s\"\n",
                    sal::static_int_cast< unsigned long >(i),
                    valueList.getElement(i));
            }
            }
            break;
        case RG_VALUETYPE_UNICODELIST:
            {
            RegistryValueList<sal_Unicode*> valueList;
            key2.getUnicodeListValue(tmpName, valueList);
            fprintf(stdout, "    Registry 2: Value: Type = RG_VALUETYPE_UNICODELIST\n");
            fprintf(
                stdout, "                       Size = %lu\n",
                sal::static_int_cast< unsigned long >(size2));
            sal_uInt32 length = valueList.getLength();
            OUString uStrValue;
            for (sal_uInt32 i=0; i<length; i++)
            {
                uStrValue = OUString(valueList.getElement(i));
                fprintf(
                    stdout, "                       Data[%lu] = \"%s\"\n",
                    sal::static_int_cast< unsigned long >(i), U2S(uStrValue));
            }
            }
            break;
        }
    }
    return nError;
}

static bool hasPublishedChildren(RegistryKey & key) {
    RegistryKeyNames subKeyNames;
    key.getKeyNames(rtl::OUString(), subKeyNames);
    for (sal_uInt32 i = 0; i < subKeyNames.getLength(); ++i) {
        rtl::OUString keyName(subKeyNames.getElement(i));
        if (!options.matchedWithExcludeKey(keyName)) {
            keyName = keyName.copy(keyName.lastIndexOf('/') + 1);
            RegistryKey subKey;
            if (!key.openKey(keyName, subKey)) {
                if (options.forceOutput()) {
                    fprintf(
                        stdout,
                        ("WARNING: could not open key \"%s\" in registry"
                         " \"%s\"\n"),
                        U2S(subKeyNames.getElement(i)),
                        options.getRegName1().getStr());
                }
            }
            if (subKey.isValid()) {
                RegValueType type;
                sal_uInt32 size;
                if (subKey.getValueInfo(rtl::OUString(), &type, &size)
                    != REG_NO_ERROR)
                {
                    if (options.forceOutput()) {
                        fprintf(
                            stdout,
                            ("WARNING: could not read key \"%s\" in registry"
                             " \"%s\"\n"),
                            U2S(subKeyNames.getElement(i)),
                            options.getRegName1().getStr());
                    }
                } else if (type == RG_VALUETYPE_BINARY) {
                    char * value = new char[size];
                    bool published = false;
                    if (subKey.getValue(rtl::OUString(), value) != REG_NO_ERROR)
                    {
                        if (options.forceOutput()) {
                            fprintf(
                                stdout,
                                ("WARNING: could not read key \"%s\" in"
                                 " registry \"%s\"\n"),
                                U2S(subKeyNames.getElement(i)),
                                options.getRegName1().getStr());
                        }
                    } else {
                        published = typereg::Reader(
                            value, size, false, TYPEREG_VERSION_1).
                            isPublished();
                    }
                    delete[] value;
                    if (published) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

static sal_uInt32 checkDifferences(
    RegistryKey& key, StringSet& keys, RegistryKeyNames& subKeyNames1,
    RegistryKeyNames& subKeyNames2)
{
    sal_uInt32 nError = 0;
    sal_uInt32 length1 = subKeyNames1.getLength();
    sal_uInt32 length2 = subKeyNames2.getLength();
    sal_uInt32 i,j;

    for (i=0; i<length1; i++)
    {
        sal_Bool bFound = sal_False;
        for (j=0; j<length2; j++)
        {
            if ( subKeyNames1.getElement(i) == subKeyNames2.getElement(j) )
            {
                bFound = sal_True;
                keys.insert(subKeyNames1.getElement(i));
                break;
            }
        }
        if ( !bFound )
        {
            if ( options.fullCheck() )
            {
                if ( options.forceOutput() )
                {
                    fprintf(stdout, "EXISTENCE: key \"%s\" exists only in registry \"%s\"\n",
                            U2S(subKeyNames1.getElement(i)), options.getRegName1().getStr());
                }
                nError++;
            }
            else
            {
                rtl::OUString keyName(subKeyNames1.getElement(i));
                if (!options.matchedWithExcludeKey(keyName)) {
                    keyName = keyName.copy(keyName.lastIndexOf('/') + 1);
                    RegistryKey subKey;
                    if (key.openKey(keyName, subKey)) {
                        if (options.forceOutput()) {
                            fprintf(
                                stdout,
                                ("ERROR: could not open key \"%s\" in registry"
                                 " \"%s\"\n"),
                                U2S(subKeyNames1.getElement(i)),
                                options.getRegName1().getStr());
                        }
                        ++nError;
                    }
                    if (subKey.isValid()) {
                        RegValueType type;
                        sal_uInt32 size;
                        if (subKey.getValueInfo(rtl::OUString(), &type, &size)
                            != REG_NO_ERROR)
                        {
                            if (options.forceOutput()) {
                                fprintf(
                                    stdout,
                                    ("ERROR: could not read key \"%s\" in"
                                     " registry \"%s\"\n"),
                                    U2S(subKeyNames1.getElement(i)),
                                    options.getRegName1().getStr());
                            }
                            ++nError;
                        } else if (type == RG_VALUETYPE_BINARY) {
                            char * value = new char[size];
                            if (subKey.getValue(rtl::OUString(), value)
                                != REG_NO_ERROR)
                            {
                                if (options.forceOutput()) {
                                    fprintf(
                                        stdout,
                                        ("ERROR: could not read key \"%s\" in"
                                         " registry \"%s\"\n"),
                                        U2S(subKeyNames1.getElement(i)),
                                        options.getRegName1().getStr());
                                }
                                ++nError;
                            } else {
                                typereg::Reader reader(
                                    value, size, false, TYPEREG_VERSION_1);
                                if (reader.getTypeClass() == RT_TYPE_MODULE) {
                                    if (options.checkUnpublished()
                                        || hasPublishedChildren(subKey))
                                    {
                                        if (options.forceOutput()) {
                                            fprintf(
                                                stdout,
                                                ("EXISTENCE: module \"%s\""
                                                 " %sexists only in registry"
                                                 " 1\n"),
                                                U2S(subKeyNames1.getElement(
                                                        i)),
                                                (options.checkUnpublished()
                                                 ? ""
                                                 : "with published children "));
                                        }
                                        ++nError;
                                    }
                                } else if (options.checkUnpublished()
                                           || reader.isPublished())
                                {
                                    if (options.forceOutput()) {
                                        fprintf(
                                            stdout,
                                            ("EXISTENCE: %spublished key \"%s\""
                                             " exists only in registry 1\n"),
                                            reader.isPublished() ? "" : "un",
                                            U2S(subKeyNames1.getElement(i)));
                                    }
                                    ++nError;
                                }
                            }
                            delete[] value;
                        }
                    }
                }
            }
        }
    }

    for (i=0; i<length2; i++)
    {
        sal_Bool bFound = sal_False;
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
                fprintf(stdout, "EXISTENCE: key \"%s\" exists only in registry \"%s\"\n",
                        U2S(subKeyNames2.getElement(i)), options.getRegName2().getStr());
            }
            nError++;
        }
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
                fprintf(stdout, "VALUES: key values of key \"%s\" are different\n", U2S(key1.getName()));
            }
            nError++;
        }
    }

    RegistryKeyNames subKeyNames1;
    RegistryKeyNames subKeyNames2;

    key1.getKeyNames(tmpName, subKeyNames1);
    key2.getKeyNames(tmpName, subKeyNames2);

    StringSet keys;
    nError += checkDifferences(key1, keys, subKeyNames1, subKeyNames2);

    StringSet::iterator iter = keys.begin();
    StringSet::iterator end = keys.end();

    RegistryKey subKey1, subKey2;
    OUString keyName;
    while ( iter !=  end )
    {
        keyName = OUString(*iter);
        if ( options.matchedWithExcludeKey(keyName) )
        {
            ++iter;
            continue;
        }

        sal_Int32 nPos = keyName.lastIndexOf( '/' );
        keyName = keyName.copy( nPos != -1 ? nPos+1 : 0 );
        if ( key1.openKey(keyName, subKey1) )
        {
            if ( options.forceOutput() )
            {
                fprintf(stdout, "ERROR: could not open key \"%s\" in registry \"%s\"\n",
                        U2S(*iter), options.getRegName1().getStr());
            }
            nError++;
        }
        if ( key2.openKey(keyName, subKey2) )
        {
            if ( options.forceOutput() )
            {
                fprintf(stdout, "ERROR: could not open key \"%s\" in registry \"%s\"\n",
                        U2S(*iter), options.getRegName2().getStr());
            }
            nError++;
        }
        if ( subKey1.isValid() && subKey2.isValid() )
        {
            nError += compareKeys(subKey1, subKey2);
        }
        subKey1.releaseKey();
        subKey2.releaseKey();
        ++iter;
    }

    return nError;
}

#if (defined UNX) || (defined OS2) || defined __MINGW32__
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    if ( !options.initOptions(argc, argv) )
    {
        exit(1);
    }

    OUString regName1( convertToFileUrl(options.getRegName1()) );
    OUString regName2( convertToFileUrl(options.getRegName2()) );

    Registry reg1;
    Registry reg2;

    if ( reg1.open(regName1, REG_READONLY) )
    {
        fprintf(stdout, "%s: open registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getRegName1().getStr());
        exit(2);
    }
    if ( reg2.open(regName2, REG_READONLY) )
    {
        fprintf(stdout, "%s: open registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getRegName2().getStr());
        exit(3);
    }

    RegistryKey key1, key2;
    if ( reg1.openRootKey(key1) )
    {
        fprintf(stdout, "%s: open root key of registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getRegName1().getStr());
        exit(4);
    }
    if ( reg2.openRootKey(key2) )
    {
        fprintf(stdout, "%s: open root key of registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getRegName2().getStr());
        exit(5);
    }
    if ( options.isStartKeyValid() )
    {
        if ( options.matchedWithExcludeKey( S2U(options.getStartKey()) ) )
        {
            fprintf(stdout, "%s: start key is equal to one of the exclude keys\n",
                    options.getProgramName().getStr());
            exit(6);
        }
        RegistryKey sk1, sk2;
        if ( key1.openKey(S2U(options.getStartKey()), sk1) )
        {
            fprintf(stdout, "%s: open start key of registry \"%s\" failed\n",
                    options.getProgramName().getStr(), options.getRegName1().getStr());
            exit(7);
        }
        if ( key2.openKey(S2U(options.getStartKey()), sk2) )
        {
            fprintf(stdout, "%s: open start key of registry \"%s\" failed\n",
                    options.getProgramName().getStr(), options.getRegName2().getStr());
            exit(8);
        }

        key1 = sk1;
        key2 = sk2;
    }

    sal_uInt32 nError = compareKeys(key1, key2);
    if ( nError )
    {
        if ( options.unoTypeCheck() )
        {
            fprintf(stdout, "%s: registries are incompatible: %lu differences!\n",
                    options.getProgramName().getStr(),
                    sal::static_int_cast< unsigned long >(nError));
        } else
        {
            fprintf(stdout, "%s: registries contain %lu differences!\n",
                    options.getProgramName().getStr(),
                    sal::static_int_cast< unsigned long >(nError));
        }
    } else
    {
        if ( options.unoTypeCheck() )
        {
            fprintf(stdout, "%s: registries are compatible!\n",
                    options.getProgramName().getStr());
        } else
        {
            fprintf(stdout, "%s: registries are equal!\n",
                    options.getProgramName().getStr());
        }
    }

    key1.releaseKey();
    key2.releaseKey();
    if ( reg1.close() )
    {
        fprintf(stdout, "%s: closing registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getRegName1().getStr());
        exit(9);
    }
    if ( reg2.close() )
    {
        fprintf(stdout, "%s: closing registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getRegName2().getStr());
        exit(10);
    }

    return nError > 0 ? 11 : 0;
}


