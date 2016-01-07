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


#include "registry/registry.hxx"
#include "registry/reader.hxx"
#include "registry/version.h"
#include "fileurl.hxx"
#include "options.hxx"

#include <rtl/ustring.hxx>
#include <osl/diagnose.h>

#include <stdio.h>
#include <string.h>

#include <set>
#include <vector>
#include <string>

using namespace registry::tools;

typedef std::set< OUString > StringSet;

class Options_Impl : public Options
{
public:
    explicit Options_Impl(char const * program)
        : Options(program),
          m_bFullCheck(false),
          m_bForceOutput(false),
          m_bUnoTypeCheck(false),
          m_checkUnpublished(false)
        {}

    std::string const & getRegName1() const { return m_regName1; }
    std::string const & getRegName2() const { return m_regName2; }

    bool isStartKeyValid() const { return !m_startKey.isEmpty(); }
    OUString const & getStartKey() const { return m_startKey; }
    bool matchedWithExcludeKey( const OUString& keyName) const;

    bool fullCheck() const { return m_bFullCheck; }
    bool forceOutput() const { return m_bForceOutput; }
    bool unoTypeCheck() const { return m_bUnoTypeCheck; }
    bool checkUnpublished() const { return m_checkUnpublished; }

protected:
    bool setRegName_Impl(char c, std::string const & param);

    virtual void printUsage_Impl() const override;
    virtual bool initOptions_Impl (std::vector< std::string > & rArgs) override;

    std::string m_regName1;
    std::string m_regName2;
    OUString    m_startKey;
    StringSet   m_excludeKeys;
    bool m_bFullCheck;
    bool m_bForceOutput;
    bool m_bUnoTypeCheck;
    bool m_checkUnpublished;
};

#define U2S( s ) OUStringToOString(s, RTL_TEXTENCODING_UTF8).getStr()

inline OUString makeOUString (std::string const & s)
{
    return OUString(s.c_str(), s.size(), RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

inline OUString shortName(OUString const & fullName)
{
    return fullName.copy(fullName.lastIndexOf('/') + 1);
}

bool Options_Impl::setRegName_Impl(char c, std::string const & param)
{
    bool one = (c == '1'), two = (c == '2');
    if (one)
        m_regName1 = param;
    if (two)
        m_regName2 = param;
    return (one || two);
}

//virtual
void Options_Impl::printUsage_Impl() const
{
    std::string const & rProgName = getProgramName();
    fprintf(stderr,
            "Usage: %s -r1<filename> -r2<filename> [-options] | @<filename>\n", rProgName.c_str()
            );
    fprintf(stderr,
            "    -r1<filename>  = filename specifies the name of the first registry.\n"
            "    -r2<filename>  = filename specifies the name of the second registry.\n"
            "    @<filename>    = filename specifies a command file.\n"
            "Options:\n"
            "    -s<name>  = name specifies the name of a start key. If no start key\n"
            "     |S<name>   is specified the comparison starts with the root key.\n"
            "    -x<name>  = name specifies the name of a key which won't be compared. All\n"
            "     |X<name>   subkeys won't be compared also. This option can be used more than once.\n"
            "    -f|F      = force the detailed output of any differences. Default\n"
            "                is that only the number of differences is returned.\n"
            "    -c|C      = make a complete check, that means any differences will be\n"
            "                detected. Default is only a compatibility check that means\n"
            "                only UNO typelibrary entries will be checked.\n"
            "    -t|T      = make an UNO type compatibility check. This means that registry 2\n"
            "                will be checked against registry 1. If a interface in r2 contains\n"
            "                more methods or the methods are in a different order as in r1, r2 is\n"
            "                incompatible to r1. But if a service in r2 supports more properties as\n"
            "                in r1 and the new properties are 'optional' it is compatible.\n"
            "    -u|U      = additionally check types that are unpublished in registry 1.\n"
            "    -h|-?     = print this help message and exit.\n"
            );
    fprintf(stderr,
            "\n%s Version 1.0\n\n", rProgName.c_str()
            );
}

// virtual
bool Options_Impl::initOptions_Impl (std::vector< std::string > & rArgs)
{
    std::vector< std::string >::const_iterator first = rArgs.begin(), last = rArgs.end();
    for (; first != last; ++first)
    {
        if ((*first)[0] != '-')
        {
            return badOption("invalid", (*first).c_str());
        }
        switch ((*first)[1])
        {
        case 'r':
        case 'R':
            {
                if (!((++first != last) && ((*first)[0] != '-')))
                {
                    return badOption("invalid", (*first).c_str());
                }

                std::string option(*first), param;
                if (option.size() == 1)
                {
                    // "-r<n><space><param>"
                    if (!((++first != last) && ((*first)[0] != '-')))
                    {
                        return badOption("invalid", (*first).c_str());
                    }
                    param = (*first);
                }
                else
                {
                    // "-r<n><param>"
                    param = std::string(&(option[1]), option.size() - 1);
                }
                if (!setRegName_Impl(option[0], param))
                {
                    return badOption("invalid", option.c_str());
                }
                break;
            }
        case 's':
        case 'S':
            {
                if (!((++first != last) && ((*first)[0] != '-')))
                {
                    return badOption("invalid", (*first).c_str());
                }
                m_startKey = makeOUString(*first);
                break;
            }
        case 'x':
        case 'X':
            {
                if (!((++first != last) && ((*first)[0] != '-')))
                {
                    return badOption("invalid", (*first).c_str());
                }
                m_excludeKeys.insert(makeOUString(*first));
                break;
            }
        case 'f':
        case 'F':
            {
                if ((*first).size() > 2)
                {
                    return badOption("invalid", (*first).c_str());
                }
                m_bForceOutput = true;
                break;
            }
        case 'c':
        case 'C':
            {
                if ((*first).size() > 2)
                {
                    return badOption("invalid", (*first).c_str());
                }
                m_bFullCheck = true;
                break;
            }
        case 't':
        case 'T':
            {
                if ((*first).size() > 2)
                {
                    return badOption("invalid", (*first).c_str());
                }
                m_bUnoTypeCheck = true;
                break;
            }
        case 'u':
        case 'U':
            {
                if ((*first).size() > 2)
                {
                    return badOption("invalid", (*first).c_str());
                }
                m_checkUnpublished = true;
                break;
            }
        case 'h':
        case '?':
            {
                if ((*first).size() > 2)
                {
                    return badOption("invalid", (*first).c_str());
                }
                return printUsage();
            }
        default:
            {
                return badOption("unknown", (*first).c_str());
            }
        }
    }

    if ( m_regName1.empty() )
    {
        return badOption("missing", "-r1");
    }
    if ( m_regName2.empty() )
    {
        return badOption("missing", "-r2");
    }
    return true;
}

bool Options_Impl::matchedWithExcludeKey( const OUString& keyName) const
{
    if (!m_excludeKeys.empty())
    {
        StringSet::const_iterator first = m_excludeKeys.begin(), last = m_excludeKeys.end();
        for (; first != last; ++first)
        {
            if (keyName.startsWith(*first))
                return true;
        }
    }
    return false;
}

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
    if ( (fieldAccess & RTFieldAccess::INVALID) == RTFieldAccess::INVALID )
    {
        ret += OString("INVALID");
    }
    if ( (fieldAccess & RTFieldAccess::READONLY) == RTFieldAccess::READONLY )
    {
        ret += OString(ret.isEmpty() ? "READONLY" : ",READONLY");
    }
    if ( (fieldAccess & RTFieldAccess::OPTIONAL) == RTFieldAccess::OPTIONAL )
    {
        ret += OString(ret.isEmpty() ? "OPTIONAL" : ",OPTIONAL");
    }
    if ( (fieldAccess & RTFieldAccess::MAYBEVOID) == RTFieldAccess::MAYBEVOID )
    {
        ret += OString(ret.isEmpty() ? "MAYBEVOID" : ",MAYBEVOID");
    }
    if ( (fieldAccess & RTFieldAccess::BOUND) == RTFieldAccess::BOUND )
    {
        ret += OString(ret.isEmpty() ? "BOUND" : ",BOUND");
    }
    if ( (fieldAccess & RTFieldAccess::CONSTRAINED) == RTFieldAccess::CONSTRAINED )
    {
        ret += OString(ret.isEmpty() ? "CONSTRAINED" : ",CONSTRAINED");
    }
    if ( (fieldAccess & RTFieldAccess::TRANSIENT) == RTFieldAccess::TRANSIENT )
    {
        ret += OString(ret.isEmpty() ? "TRANSIENT" : ",TRANSIENT");
    }
    if ( (fieldAccess & RTFieldAccess::MAYBEAMBIGUOUS) == RTFieldAccess::MAYBEAMBIGUOUS )
    {
        ret += OString(ret.isEmpty() ? "MAYBEAMBIGUOUS" : ",MAYBEAMBIGUOUS");
    }
    if ( (fieldAccess & RTFieldAccess::MAYBEDEFAULT) == RTFieldAccess::MAYBEDEFAULT )
    {
        ret += OString(ret.isEmpty() ? "MAYBEDEFAULT" : ",MAYBEDEFAULT");
    }
    if ( (fieldAccess & RTFieldAccess::REMOVABLE) == RTFieldAccess::REMOVABLE )
    {
        ret += OString(ret.isEmpty() ? "REMOVABLE" : ",REMOVABLE");
    }
    if ( (fieldAccess & RTFieldAccess::ATTRIBUTE) == RTFieldAccess::ATTRIBUTE )
    {
        ret += OString(ret.isEmpty() ? "ATTRIBUTE" : ",ATTRIBUTE");
    }
    if ( (fieldAccess & RTFieldAccess::PROPERTY) == RTFieldAccess::PROPERTY )
    {
        ret += OString(ret.isEmpty() ? "PROPERTY" : ",PROPERTY");
    }
    if ( (fieldAccess & RTFieldAccess::CONST) == RTFieldAccess::CONST )
    {
        ret += OString(ret.isEmpty() ? "CONST" : ",CONST");
    }
    if ( (fieldAccess & RTFieldAccess::READWRITE) == RTFieldAccess::READWRITE )
    {
        ret += OString(ret.isEmpty() ? "READWRITE" : ",READWRITE");
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
                (OUStringToOString(
                    constValue.m_value.aString, RTL_TEXTENCODING_UTF8).
                 getStr()));
            break;
        default:
            break;
    }
}

static void dumpTypeClass(bool & rbDump, RTTypeClass typeClass, OUString const & keyName)
{
    if (rbDump)
        fprintf(stdout, "%s: %s\n", getTypeClass(typeClass), U2S(keyName));
    rbDump = false;
}

static sal_uInt32 checkConstValue(Options_Impl const & options,
                                  const OUString& keyName,
                                  RTTypeClass typeClass,
                                  bool & bDump,
                                  RTConstValue& constValue1,
                                  RTConstValue& constValue2,
                                  sal_uInt16 index1)
{
    switch (constValue1.m_type)
    {
        case RT_TYPE_NONE:
            break;
        case RT_TYPE_BOOL:
            if (constValue1.m_value.aBool != constValue2.m_value.aBool)
            {
                if ( options.forceOutput() && !options.unoTypeCheck() )
                {
                    dumpTypeClass(bDump, typeClass, keyName);
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
                    dumpTypeClass(bDump, typeClass, keyName);
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
                    dumpTypeClass(bDump, typeClass, keyName);
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
                    dumpTypeClass(bDump, typeClass, keyName);
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
                    dumpTypeClass(bDump, typeClass, keyName);
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
                    dumpTypeClass(bDump, typeClass, keyName);
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
                    dumpTypeClass(bDump, typeClass, keyName);
                    fprintf(
                        stdout, "  Field %d: Value1 = %s  !=  Value2 = %s\n",
                        index1,
                        OUStringToOString(
                            OUString::number(constValue1.m_value.aHyper),
                            RTL_TEXTENCODING_ASCII_US).getStr(),
                        OUStringToOString(
                            OUString::number(constValue2.m_value.aHyper),
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
                    dumpTypeClass(bDump, typeClass, keyName);
                    fprintf(
                        stdout, "  Field %d: Value1 = %s  !=  Value2 = %s\n",
                        index1,
                        OUStringToOString(
                            OUString::number(
                                constValue1.m_value.aUHyper),
                            RTL_TEXTENCODING_ASCII_US).getStr(),
                        OUStringToOString(
                            OUString::number(
                                constValue2.m_value.aUHyper),
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
                    dumpTypeClass(bDump, typeClass, keyName);
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
                    dumpTypeClass(bDump, typeClass, keyName);
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

enum verbosity_t {SILENT, REPORT};
static sal_uInt32 checkField(Options_Impl const & options,
                             const OUString& keyName,
                             RTTypeClass typeClass,
                             bool & bDump,
                             typereg::Reader& reader1,
                             typereg::Reader& reader2,
                             sal_uInt16 index1,
                             sal_uInt16 index2,
                             verbosity_t const eVerbosity)
{
    sal_uInt32 nError = 0;
    if ( reader1.getFieldName(index1) != reader2.getFieldName(index2) )
    {
        if (options.forceOutput() && (REPORT == eVerbosity))
        {
            dumpTypeClass (bDump, typeClass, keyName);
            fprintf(stdout, "  Field %d: Name1 = %s  !=  Name2 = %s\n", index1,
                    U2S(reader1.getFieldName(index1)), U2S(reader2.getFieldName(index2)));
        }
        nError++;
    }
    if ( reader1.getFieldTypeName(index1) != reader2.getFieldTypeName(index2) )
    {
        if (options.forceOutput() && (REPORT == eVerbosity))
        {
            dumpTypeClass (bDump, typeClass, keyName);
            fprintf(stdout, "  Field %d: Type1 = %s  !=  Type2 = %s\n", index1,
                    U2S(reader1.getFieldTypeName(index1)), U2S(reader2.getFieldTypeName(index2)));
        }
        nError++;
    }
    else
    {
        RTConstValue constValue1 = reader1.getFieldValue(index1);
        RTConstValue constValue2 = reader2.getFieldValue(index2);
        if ( constValue1.m_type != constValue2.m_type )
        {
            if (options.forceOutput() && (REPORT == eVerbosity))
            {
                dumpTypeClass (bDump, typeClass, keyName);
                fprintf(stdout, "  Field %d: Access1 = %s  !=  Access2 = %s\n", index1,
                        getConstValueType(constValue1), getConstValueType(constValue2));
                fprintf(stdout, "  Field %d: Value1 = ", index1);
                printConstValue(constValue1);
                fprintf(stdout, "  !=  Value2 = ");
                printConstValue(constValue1);
                fprintf(stdout, "\n;");
            }
            nError++;
        }
        else
        {
            nError += checkConstValue(options, keyName, typeClass, bDump, constValue1, constValue2, index1);
        }
    }

    if ( reader1.getFieldFlags(index1) != reader2.getFieldFlags(index2) )
    {
        if (options.forceOutput() && (REPORT == eVerbosity))
        {
            dumpTypeClass (bDump, typeClass, keyName);
            fprintf(stdout, "  Field %d: FieldAccess1 = %s  !=  FieldAccess2 = %s\n", index1,
                    getFieldAccess(reader1.getFieldFlags(index1)).getStr(),
                    getFieldAccess(reader1.getFieldFlags(index2)).getStr());
        }
        nError++;
    }

    if ( options.fullCheck() && (reader1.getFieldDocumentation(index1) != reader2.getFieldDocumentation(index2)) )
    {
        if (options.forceOutput() && (REPORT == eVerbosity))
        {
            dumpTypeClass (bDump, typeClass, keyName);
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
        case RTMethodMode::ONEWAY:
            return "ONEWAY";
        case RTMethodMode::ONEWAY_CONST:
            return "ONEWAY,CONST";
        case RTMethodMode::TWOWAY:
            return "NONE";
        case RTMethodMode::TWOWAY_CONST:
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

static sal_uInt32 checkMethod(Options_Impl const & options,
                              const OUString& keyName,
                              RTTypeClass typeClass,
                              bool & bDump,
                              typereg::Reader& reader1,
                              typereg::Reader& reader2,
                              sal_uInt16 index)
{
    sal_uInt32 nError = 0;
    if ( reader1.getMethodName(index) != reader2.getMethodName(index) )
    {
        if ( options.forceOutput() )
        {
            dumpTypeClass (bDump, typeClass, keyName);
            fprintf(stdout, "  Method1 %d: Name1 = %s  !=  Name2 = %s\n", index,
                    U2S(reader1.getMethodName(index)),
                    U2S(reader2.getMethodName(index)));
        }
        nError++;
    }

    if ( reader1.getMethodReturnTypeName(index) != reader2.getMethodReturnTypeName(index) )
    {
        if ( options.forceOutput() )
        {
            dumpTypeClass (bDump, typeClass, keyName);
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
            dumpTypeClass (bDump, typeClass, keyName);
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
                dumpTypeClass (bDump, typeClass, keyName);
                fprintf(stdout, "  Method %d, Parameter %d: Type1 = %s  !=  Type2 = %s\n", index, i,
                        U2S(reader1.getMethodParameterTypeName(index, i)),
                        U2S(reader2.getMethodParameterTypeName(index, i)));
            }
            nError++;
        }
        if ( options.fullCheck() && (reader1.getMethodParameterName(index, i) != reader2.getMethodParameterName(index, i)) )
        {
            if ( options.forceOutput() )
            {
                dumpTypeClass (bDump, typeClass, keyName);
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
                dumpTypeClass (bDump, typeClass, keyName);
                fprintf(stdout, "  Method %d, Parameter %d: Mode1 = %s  !=  Mode2 = %s\n", index, i,
                        getParamMode(reader1.getMethodParameterFlags(index, i)),
                        getParamMode(reader2.getMethodParameterFlags(index, i)));
            }
            nError++;
        }
    }
    if ( i < nParams1 && options.forceOutput() )
    {
        dumpTypeClass (bDump, typeClass, keyName);
        fprintf(stdout, "  Registry1: Method %d contains %d more parameters\n", index, nParams1 - i);
    }
    if ( i < nParams2 && options.forceOutput() )
    {
        dumpTypeClass (bDump, typeClass, keyName);
        fprintf(stdout, "  Registry2: Method %d contains %d more parameters\n", index, nParams2 - i);
    }

    sal_uInt16 nExcep1 = (sal_uInt16)reader1.getMethodExceptionCount(index);
    sal_uInt16 nExcep2 = (sal_uInt16)reader2.getMethodExceptionCount(index);
    if ( nExcep1 != nExcep2 )
    {
        if ( options.forceOutput() )
        {
            dumpTypeClass (bDump, typeClass, keyName);
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
                dumpTypeClass (bDump, typeClass, keyName);
                fprintf(stdout, "  Method %d, Exception %d: Name1 = %s  !=  Name2 = %s\n", index, i,
                        U2S(reader1.getMethodExceptionTypeName(index, i)),
                        U2S(reader2.getMethodExceptionTypeName(index, i)));
            }
            nError++;
        }
    }
    if ( i < nExcep1 && options.forceOutput() )
    {
        dumpTypeClass (bDump, typeClass, keyName);
        fprintf(stdout, "  Registry1: Method %d contains %d more exceptions\n", index, nExcep1 - i);
    }
    if ( i < nExcep2 && options.forceOutput() )
    {
        dumpTypeClass (bDump, typeClass, keyName);
        fprintf(stdout, "  Registry2: Method %d contains %d more exceptions\n", index, nExcep2 - i);
    }

    if ( reader1.getMethodFlags(index) != reader2.getMethodFlags(index) )
    {
        if ( options.forceOutput() )
        {
            dumpTypeClass (bDump, typeClass, keyName);
            fprintf(stdout, "  Method %d: Mode1 = %s  !=  Mode2 = %s\n", index,
                    getMethodMode(reader1.getMethodFlags(index)),
                    getMethodMode(reader2.getMethodFlags(index)));
        }
        nError++;
    }

    if ( options.fullCheck() && (reader1.getMethodDocumentation(index) != reader2.getMethodDocumentation(index)) )
    {
        if ( options.forceOutput() )
        {
            dumpTypeClass (bDump, typeClass, keyName);
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
        case RTReferenceType::SUPPORTS:
            return "RTReferenceType::SUPPORTS";
        case RTReferenceType::OBSERVES:
            return "RTReferenceType::OBSERVES";
        case RTReferenceType::EXPORTS:
            return "RTReferenceType::EXPORTS";
        case RTReferenceType::NEEDS:
            return "RTReferenceType::NEEDS";
        default:
            return "RTReferenceType::INVALID";
    }
}

static sal_uInt32 checkReference(Options_Impl const & options,
                                 const OUString& keyName,
                                 RTTypeClass typeClass,
                                 bool & bDump,
                                 typereg::Reader& reader1,
                                    typereg::Reader& reader2,
                                    sal_uInt16 index1,
                                 sal_uInt16 index2)
{
    sal_uInt32 nError = 0;
    if ( reader1.getReferenceTypeName(index1) != reader2.getReferenceTypeName(index2) )
    {
        if ( options.forceOutput() && !options.unoTypeCheck() )
        {
            dumpTypeClass (bDump, typeClass, keyName);
            fprintf(stdout, "  Reference %d: Name1 = %s  !=  Name2 = %s\n", index1,
                    U2S(reader1.getReferenceTypeName(index1)),
                    U2S(reader2.getReferenceTypeName(index2)));
        }
        nError++;
    }
    if ( reader1.getReferenceTypeName(index1) != reader2.getReferenceTypeName(index2) )
    {
        if ( options.forceOutput() && !options.unoTypeCheck() )
        {
            dumpTypeClass (bDump, typeClass, keyName);
            fprintf(stdout, "  Reference %d: Type1 = %s  !=  Type2 = %s\n", index1,
                    getReferenceType(reader1.getReferenceSort(index1)),
                    getReferenceType(reader2.getReferenceSort(index2)));
        }
        nError++;
    }
    if ( options.fullCheck() && (reader1.getReferenceDocumentation(index1) != reader2.getReferenceDocumentation(index2)) )
    {
        if ( options.forceOutput() && !options.unoTypeCheck() )
        {
            dumpTypeClass (bDump, typeClass, keyName);
            fprintf(stdout, "  Reference %d: Doku1 = %s\n                 Doku2 = %s\n", index1,
                    U2S(reader1.getReferenceDocumentation(index1)),
                    U2S(reader2.getReferenceDocumentation(index2)));
        }
        nError++;
    }
    if ( reader1.getReferenceFlags(index1) != reader2.getReferenceFlags(index2) )
    {
        if ( options.forceOutput() && !options.unoTypeCheck() )
        {
            dumpTypeClass (bDump, typeClass, keyName);
            fprintf(stdout, "  Reference %d: Access1 = %s  !=  Access2 = %s\n", index1,
                    getFieldAccess(reader1.getReferenceFlags(index1)).getStr(),
                    getFieldAccess(reader1.getReferenceFlags(index2)).getStr());
        }
        nError++;
    }
    return nError;
}

static sal_uInt32 checkFieldsWithoutOrder(Options_Impl const & options,
                                          const OUString& keyName,
                                          RTTypeClass typeClass,
                                          bool & bDump,
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
            dumpTypeClass (bDump, typeClass, keyName);
            fprintf(stdout, "  %s1 contains %d more properties as %s2\n",
                    getTypeClass(typeClass), nFields1-nFields2, getTypeClass(typeClass));
        }
    }

    bool bFound = false;
    ::std::set< sal_uInt16 > moreProps;

    for (i=0; i < nFields1; i++)
    {
        for (j=0; j < nFields2; j++)
        {
            if (!checkField(options, keyName, typeClass, bDump, reader1, reader2, i, j, SILENT))
            {
                bFound =  true;
                moreProps.insert(j);
                break;
            }
        }
        if (!bFound)
        {
            if (options.forceOutput())
            {
                dumpTypeClass (bDump, typeClass, keyName);
                fprintf(stdout, "  incompatible change: Field %d ('%s') of r1 is not longer a property of this %s in r2\n",
                        i, U2S(shortName(reader1.getFieldName(i))), getTypeClass(typeClass));
            }
            nError++;
        }
        else
        {
            bFound = false;
        }
    }

    if ( typeClass == RT_TYPE_SERVICE && !moreProps.empty() )
    {
        for (j=0; j < nFields2; j++)
        {
            if ( moreProps.find(j) == moreProps.end() )
            {
                if ( (reader2.getFieldFlags(j) & RTFieldAccess::OPTIONAL) != RTFieldAccess::OPTIONAL )
                {
                    if ( options.forceOutput() )
                    {
                        dumpTypeClass (bDump, typeClass, keyName);
                        fprintf(stdout,
                                "  incompatible change: Field %d ('%s') of r2 is a new property"
                                " compared to this %s in r1 and is not 'optional'\n",
                                j, U2S(shortName(reader2.getFieldName(j))), getTypeClass(typeClass));
                    }
                    nError++;
                }
            }
        }
    }

    return nError;
}

static sal_uInt32 checkBlob(
    Options_Impl const & options,
    const OUString& keyName,
    typereg::Reader& reader1, sal_uInt32 size1,
    typereg::Reader& reader2, sal_uInt32 size2)
{
    sal_uInt32 nError = 0;
    bool bDump = true;

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
    if (reader1.isPublished())
    {
        if (!reader2.isPublished())
        {
            if (options.forceOutput())
            {
                dumpTypeClass(bDump, /*"?"*/ reader1.getTypeClass(), keyName);
                fprintf(stdout, "    published in 1 but unpublished in 2\n");
            }
            ++nError;
        }
    }
    else if (!options.checkUnpublished())
    {
        return nError;
    }
    if ( reader1.getTypeClass() != reader2.getTypeClass() )
    {
        if ( options.forceOutput() )
        {
            dumpTypeClass(bDump, /*"?"*/ reader1.getTypeClass(), keyName);
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
            dumpTypeClass(bDump, typeClass, keyName);
            fprintf(stdout, "    TypeName1 = %s  !=  TypeName2 = %s\n",
                    U2S(reader1.getTypeName()), U2S(reader2.getTypeName()));
        }
        nError++;
    }
    if ( (typeClass == RT_TYPE_INTERFACE ||
          typeClass == RT_TYPE_STRUCT ||
          typeClass == RT_TYPE_EXCEPTION) )
    {
        if (reader1.getSuperTypeCount() != reader2.getSuperTypeCount())
        {
            dumpTypeClass(bDump, typeClass, keyName);
            fprintf(
                stdout, "    SuperTypeCount1 = %d  !=  SuperTypeCount2 = %d\n",
                static_cast< int >(reader1.getSuperTypeCount()),
                static_cast< int >(reader2.getSuperTypeCount()));
            ++nError;
        } else
        {
            for (sal_Int16 i = 0; i < reader1.getSuperTypeCount(); ++i)
            {
                if (reader1.getSuperTypeName(i) != reader2.getSuperTypeName(i))
                {
                    if ( options.forceOutput() )
                    {
                        dumpTypeClass(bDump, typeClass, keyName);
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
    bool bCheckNormal = true;

    if ( (typeClass == RT_TYPE_SERVICE ||
          typeClass == RT_TYPE_MODULE ||
          typeClass == RT_TYPE_CONSTANTS) && options.unoTypeCheck() )
    {
        bCheckNormal = false;
    }

    if ( bCheckNormal )
    {
        if ( nFields1 != nFields2 )
        {
            if ( options.forceOutput() )
            {
                dumpTypeClass(bDump, typeClass, keyName);
                fprintf(stdout, "    nFields1 = %d  !=  nFields2 = %d\n", nFields1, nFields2);
            }
            nError++;
        }

        sal_uInt16 i;
        for (i=0; i < nFields1 && i < nFields2; i++)
        {
            nError += checkField(options, keyName, typeClass, bDump, reader1, reader2, i, i, REPORT);
        }
        if ( i < nFields1 && options.forceOutput() )
        {
            dumpTypeClass(bDump, typeClass, keyName);
            fprintf(stdout, "    Registry1 contains %d more fields\n", nFields1 - i);
        }
        if ( i < nFields2 && options.forceOutput() )
        {
            dumpTypeClass(bDump, typeClass, keyName);
            fprintf(stdout, "    Registry2 contains %d more fields\n", nFields2 - i);
        }
    }
    else
    {
        nError += checkFieldsWithoutOrder(options, keyName, typeClass, bDump, reader1, reader2);
    }

    if ( typeClass == RT_TYPE_INTERFACE )
    {
        sal_uInt16 nMethods1 = (sal_uInt16)reader1.getMethodCount();
        sal_uInt16 nMethods2 = (sal_uInt16)reader2.getMethodCount();
        if ( nMethods1 != nMethods2 )
        {
            if ( options.forceOutput() )
            {
                dumpTypeClass(bDump, typeClass, keyName);
                fprintf(stdout, "    nMethods1 = %d  !=  nMethods2 = %d\n", nMethods1, nMethods2);
            }
            nError++;
        }

        sal_uInt16 i;
        for (i=0; i < nMethods1 && i < nMethods2; i++)
        {
            nError += checkMethod(options, keyName, typeClass, bDump, reader1, reader2, i);
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
                    dumpTypeClass(bDump, typeClass, keyName);
                    fprintf(stdout, "    service1 contains %d more references as service2\n",
                            nReference1-nReference2);
                }
            }

            bool bFound = false;
            ::std::set< sal_uInt16 > moreReferences;

            for (i=0; i < nReference1; i++)
            {
                for (j=0; j < nReference2; j++)
                {
                    if (!checkReference(options, keyName, typeClass, bDump, reader1, reader2, i, j))
                    {
                        bFound =  true;
                        moreReferences.insert(j);
                        break;
                    }
                }
                if (!bFound)
                {
                    if (options.forceOutput())
                    {
                        dumpTypeClass(bDump, typeClass, keyName);
                        fprintf(stdout,
                                "  incompatible change: Reference %d ('%s') in 'r1' is not longer a reference"
                                " of this service in 'r2'\n",
                                i, U2S(shortName(reader1.getReferenceTypeName(i))));
                    }
                    nError++;
                }
                else
                {
                    bFound = false;
                }
            }

            if ( !moreReferences.empty() )
            {
                for (j=0; j < nReference2; j++)
                {
                    if ( moreReferences.find(j) == moreReferences.end() )
                    {
                        if ( (reader2.getReferenceFlags(j) & RTFieldAccess::OPTIONAL) != RTFieldAccess::OPTIONAL )
                        {
                            if ( options.forceOutput() )
                            {
                                dumpTypeClass(bDump, typeClass, keyName);
                                fprintf(stdout,
                                        "  incompatible change: Reference %d ('%s') of r2 is a new reference"
                                        " compared to this service in r1 and is not 'optional'\n",
                                        j, U2S(shortName(reader2.getReferenceTypeName(j))));
                            }
                            nError++;
                        }
                    }
                }
            }
        }
        else
        {
            if ( nReference1 != nReference2 )
            {
                if ( options.forceOutput() )
                {
                    dumpTypeClass(bDump, typeClass, keyName);
                    fprintf(stdout, "    nReferences1 = %d  !=  nReferences2 = %d\n", nReference1, nReference2);
                }
                nError++;
            }

            sal_uInt16 i;
            for (i=0; i < nReference1 && i < nReference2; i++)
            {
                nError += checkReference(options, keyName, typeClass, bDump, reader1, reader2, i, i);
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
            dumpTypeClass(bDump, typeClass, keyName);
            fprintf(stdout, "    Doku1 = %s\n    Doku2 = %s\n",
                    U2S(reader1.getDocumentation()), U2S(reader2.getDocumentation()));
        }
        nError++;
    }
    return nError;
}

static sal_uInt32 checkValueDifference(
    Options_Impl const & options,
    RegistryKey& key1, RegValueType valueType1, sal_uInt32 size1,
    RegistryKey& key2, RegValueType valueType2, sal_uInt32 size2)
{
    OUString tmpName;
    sal_uInt32 nError = 0;

    if ( valueType1 == valueType2 )
    {
        bool bEqual = true;
        switch (valueType1)
        {
        case RegValueType::LONGLIST:
            {
                RegistryValueList<sal_Int32> valueList1;
                RegistryValueList<sal_Int32> valueList2;
                key1.getLongListValue(tmpName, valueList1);
                key2.getLongListValue(tmpName, valueList2);
                sal_uInt32 length1 = valueList1.getLength();
                sal_uInt32 length2 = valueList1.getLength();
                if ( length1 != length2 )
                {
                    bEqual = false;
                    break;
                }
                for (sal_uInt32 i=0; i<length1; i++)
                {
                    if ( valueList1.getElement(i) != valueList2.getElement(i) )
                    {
                        bEqual = false;
                        break;
                    }
                }
            }
            break;
        case RegValueType::STRINGLIST:
            {
                RegistryValueList<sal_Char*> valueList1;
                RegistryValueList<sal_Char*> valueList2;
                key1.getStringListValue(tmpName, valueList1);
                key2.getStringListValue(tmpName, valueList2);
                sal_uInt32 length1 = valueList1.getLength();
                sal_uInt32 length2 = valueList1.getLength();
                if ( length1 != length2 )
                {
                    bEqual = false;
                    break;
                }
                for (sal_uInt32 i=0; i<length1; i++)
                {
                    if ( strcmp(valueList1.getElement(i), valueList2.getElement(i)) != 0 )
                    {
                        bEqual = false;
                        break;
                    }
                }
            }
            break;
        case RegValueType::UNICODELIST:
            {
                RegistryValueList<sal_Unicode*> valueList1;
                RegistryValueList<sal_Unicode*> valueList2;
                key1.getUnicodeListValue(tmpName, valueList1);
                key2.getUnicodeListValue(tmpName, valueList2);
                sal_uInt32 length1 = valueList1.getLength();
                sal_uInt32 length2 = valueList1.getLength();
                if ( length1 != length2 )
                {
                    bEqual = false;
                    break;
                }
                for (sal_uInt32 i=0; i<length1; i++)
                {
                    if ( rtl_ustr_compare(valueList1.getElement(i), valueList2.getElement(i)) != 0 )
                    {
                        bEqual = false;
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
            std::vector< sal_uInt8 > value1(size1);
            key1.getValue(tmpName, &value1[0]);

            std::vector< sal_uInt8 > value2(size2);
            key2.getValue(tmpName, &value2[0]);

            bEqual = (memcmp(&value1[0], &value2[0], value1.size()) == 0 );
            if ( !bEqual && valueType1 == RegValueType::BINARY && valueType2 == RegValueType::BINARY )
            {
                typereg::Reader reader1(&value1[0], value1.size(), false, TYPEREG_VERSION_1);
                typereg::Reader reader2(&value2[0], value2.size(), false, TYPEREG_VERSION_1);
                if ( reader1.isValid() && reader2.isValid() )
                {
                    return checkBlob(options, key1.getName(), reader1, size1, reader2, size2);
                }
            }
            if ( bEqual )
            {
                return 0;
            }
            else
            {
                if ( options.forceOutput() )
                {
                    fprintf(stdout, "Difference: key values of key \"%s\" are different\n", U2S(key1.getName()));
                }
                nError++;
            }
        }
    }

    if ( options.forceOutput() )
    {
        switch (valueType1)
        {
        case RegValueType::NOT_DEFINED:
            fprintf(stdout, "    Registry 1: key has no value\n");
            break;
        case RegValueType::LONG:
            {
                std::vector< sal_uInt8 > value1(size1);
                key1.getValue(tmpName, &value1[0]);

                fprintf(stdout, "    Registry 1: Value: Type = RegValueType::LONG\n");
                fprintf(
                    stdout, "                       Size = %lu\n",
                    sal::static_int_cast< unsigned long >(size1));
                fprintf(stdout, "                       Data = %p\n", &value1[0]);
            }
            break;
        case RegValueType::STRING:
            {
                std::vector< sal_uInt8 > value1(size1);
                key1.getValue(tmpName, &value1[0]);

                fprintf(stdout, "    Registry 1: Value: Type = RegValueType::STRING\n");
                fprintf(
                    stdout, "                       Size = %lu\n",
                    sal::static_int_cast< unsigned long >(size1));
                fprintf(stdout, "                       Data = \"%s\"\n", reinterpret_cast<char const*>(&value1[0]));
            }
            break;
        case RegValueType::UNICODE:
            {
                std::vector< sal_uInt8 > value1(size1);
                key1.getValue(tmpName, &value1[0]);

                OUString uStrValue(reinterpret_cast<sal_Unicode const*>(&value1[0]));
                fprintf(stdout, "    Registry 1: Value: Type = RegValueType::UNICODE\n");
                fprintf(
                    stdout, "                       Size = %lu\n",
                    sal::static_int_cast< unsigned long >(size1));
                fprintf(stdout, "                       Data = \"%s\"\n", U2S(uStrValue));
            }
            break;
        case RegValueType::BINARY:
            fprintf(stdout, "    Registry 1: Value: Type = RegValueType::BINARY\n");
            break;
        case RegValueType::LONGLIST:
            {
                RegistryValueList<sal_Int32> valueList;
                key1.getLongListValue(tmpName, valueList);
                fprintf(stdout, "    Registry 1: Value: Type = RegValueType::LONGLIST\n");
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
        case RegValueType::STRINGLIST:
            {
                RegistryValueList<sal_Char*> valueList;
                key1.getStringListValue(tmpName, valueList);
                fprintf(stdout, "    Registry 1: Value: Type = RegValueType::STRINGLIST\n");
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
        case RegValueType::UNICODELIST:
            {
                RegistryValueList<sal_Unicode*> valueList;
                key1.getUnicodeListValue(tmpName, valueList);
                fprintf(stdout, "    Registry 1: Value: Type = RegValueType::UNICODELIST\n");
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
        case RegValueType::NOT_DEFINED:
            fprintf(stdout, "    Registry 2: key has no value\n");
            break;
        case RegValueType::LONG:
            {
                std::vector< sal_uInt8 > value2(size2);
                key2.getValue(tmpName, &value2[0]);

                fprintf(stdout, "    Registry 2: Value: Type = RegValueType::LONG\n");
                fprintf(
                    stdout, "                       Size = %lu\n",
                    sal::static_int_cast< unsigned long >(size2));
                fprintf(stdout, "                       Data = %p\n", &value2[0]);
            }
            break;
        case RegValueType::STRING:
            {
                std::vector< sal_uInt8 > value2(size2);
                key2.getValue(tmpName, &value2[0]);

                fprintf(stdout, "    Registry 2: Value: Type = RegValueType::STRING\n");
                fprintf(
                    stdout, "                       Size = %lu\n",
                    sal::static_int_cast< unsigned long >(size2));
                fprintf(stdout, "                       Data = \"%s\"\n", reinterpret_cast<char const*>(&value2[0]));
            }
            break;
        case RegValueType::UNICODE:
            {
                std::vector< sal_uInt8 > value2(size2);
                key2.getValue(tmpName, &value2[0]);

                OUString uStrValue(reinterpret_cast<sal_Unicode const*>(&value2[0]));
                fprintf(stdout, "    Registry 2: Value: Type = RegValueType::UNICODE\n");
                fprintf(
                    stdout, "                       Size = %lu\n",
                    sal::static_int_cast< unsigned long >(size2));
                fprintf(stdout, "                       Data = \"%s\"\n", U2S(uStrValue));
            }
            break;
        case RegValueType::BINARY:
            fprintf(stdout, "    Registry 2: Value: Type = RegValueType::BINARY\n");
            break;
        case RegValueType::LONGLIST:
            {
                RegistryValueList<sal_Int32> valueList;
                key2.getLongListValue(tmpName, valueList);
                fprintf(stdout, "    Registry 2: Value: Type = RegValueType::LONGLIST\n");
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
        case RegValueType::STRINGLIST:
            {
                RegistryValueList<sal_Char*> valueList;
                key2.getStringListValue(tmpName, valueList);
                fprintf(stdout, "    Registry 2: Value: Type = RegValueType::STRINGLIST\n");
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
        case RegValueType::UNICODELIST:
            {
                RegistryValueList<sal_Unicode*> valueList;
                key2.getUnicodeListValue(tmpName, valueList);
                fprintf(stdout, "    Registry 2: Value: Type = RegValueType::UNICODELIST\n");
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

static bool hasPublishedChildren(Options_Impl const & options, RegistryKey & key)
{
    RegistryKeyNames subKeyNames;
    key.getKeyNames(OUString(), subKeyNames);
    for (sal_uInt32 i = 0; i < subKeyNames.getLength(); ++i)
    {
        OUString keyName(subKeyNames.getElement(i));
        if (!options.matchedWithExcludeKey(keyName))
        {
            keyName = keyName.copy(keyName.lastIndexOf('/') + 1);
            RegistryKey subKey;
            if (key.openKey(keyName, subKey) == RegError::NO_ERROR)
            {
                if (options.forceOutput())
                {
                    fprintf(
                        stdout,
                        ("WARNING: could not open key \"%s\" in registry"
                         " \"%s\"\n"),
                        U2S(subKeyNames.getElement(i)),
                        options.getRegName1().c_str());
                }
            }
            if (subKey.isValid())
            {
                RegValueType type;
                sal_uInt32 size;
                if (subKey.getValueInfo(OUString(), &type, &size) != RegError::NO_ERROR)
                {
                    if (options.forceOutput())
                    {
                        fprintf(
                            stdout,
                            ("WARNING: could not read key \"%s\" in registry"
                             " \"%s\"\n"),
                            U2S(subKeyNames.getElement(i)),
                            options.getRegName1().c_str());
                    }
                }
                else if (type == RegValueType::BINARY)
                {
                    bool published = false;
                    std::vector< sal_uInt8 > value(size);
                    if (subKey.getValue(OUString(), &value[0]) != RegError::NO_ERROR)
                    {
                        if (options.forceOutput())
                        {
                            fprintf(
                                stdout,
                                ("WARNING: could not read key \"%s\" in"
                                 " registry \"%s\"\n"),
                                U2S(subKeyNames.getElement(i)),
                                options.getRegName1().c_str());
                        }
                    }
                    else
                    {
                        published = typereg::Reader(&value[0], value.size(), false, TYPEREG_VERSION_1).isPublished();
                    }
                    if (published)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

static sal_uInt32 checkDifferences(
    Options_Impl const & options,
    RegistryKey& key, StringSet& keys,
    RegistryKeyNames& subKeyNames1,
    RegistryKeyNames& subKeyNames2)
{
    sal_uInt32 nError = 0;
    sal_uInt32 length1 = subKeyNames1.getLength();
    sal_uInt32 length2 = subKeyNames2.getLength();
    sal_uInt32 i,j;

    for (i=0; i<length1; i++)
    {
        bool bFound = false;
        for (j=0; j<length2; j++)
        {
            if ( subKeyNames1.getElement(i) == subKeyNames2.getElement(j) )
            {
                bFound = true;
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
                            U2S(subKeyNames1.getElement(i)), options.getRegName1().c_str());
                }
                nError++;
            }
            else
            {
                OUString keyName(subKeyNames1.getElement(i));
                if (!options.matchedWithExcludeKey(keyName))
                {
                    keyName = keyName.copy(keyName.lastIndexOf('/') + 1);
                    RegistryKey subKey;
                    if (key.openKey(keyName, subKey) != RegError::NO_ERROR)
                    {
                        if (options.forceOutput())
                        {
                            fprintf(
                                stdout,
                                ("ERROR: could not open key \"%s\" in registry"
                                 " \"%s\"\n"),
                                U2S(subKeyNames1.getElement(i)),
                                options.getRegName1().c_str());
                        }
                        ++nError;
                    }
                    if (subKey.isValid())
                    {
                        RegValueType type;
                        sal_uInt32 size;
                        if (subKey.getValueInfo(OUString(), &type, &size) != RegError::NO_ERROR)
                        {
                            if (options.forceOutput())
                            {
                                fprintf(
                                    stdout,
                                    ("ERROR: could not read key \"%s\" in"
                                     " registry \"%s\"\n"),
                                    U2S(subKeyNames1.getElement(i)),
                                    options.getRegName1().c_str());
                            }
                            ++nError;
                        }
                        else if (type == RegValueType::BINARY)
                        {
                            std::vector< sal_uInt8 > value(size);
                            if (subKey.getValue(OUString(), &value[0]) != RegError::NO_ERROR)
                            {
                                if (options.forceOutput())
                                {
                                    fprintf(
                                        stdout,
                                        ("ERROR: could not read key \"%s\" in"
                                         " registry \"%s\"\n"),
                                        U2S(subKeyNames1.getElement(i)),
                                        options.getRegName1().c_str());
                                }
                                ++nError;
                            }
                            else
                            {
                                typereg::Reader reader(&value[0], value.size(), false, TYPEREG_VERSION_1);
                                if (reader.getTypeClass() == RT_TYPE_MODULE)
                                {
                                    if (options.checkUnpublished() || hasPublishedChildren(options, subKey))
                                    {
                                        if (options.forceOutput())
                                        {
                                            fprintf(
                                                stdout,
                                                ("EXISTENCE: module \"%s\""
                                                 " %sexists only in registry"
                                                 " 1\n"),
                                                U2S(subKeyNames1.getElement(i)),
                                                (options.checkUnpublished()
                                                 ? ""
                                                 : "with published children "));
                                        }
                                        ++nError;
                                    }
                                }
                                else if (options.checkUnpublished() || reader.isPublished())
                                {
                                    if (options.forceOutput())
                                    {
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
                        }
                    }
                }
            }
        }
    }

    for (i=0; i<length2; i++)
    {
        bool bFound = false;
        for (j=0; j<length1; j++)
        {
            if ( subKeyNames2.getElement(i) == subKeyNames1.getElement(j) )
            {
                bFound = true;
                keys.insert(subKeyNames2.getElement(i));
                break;
            }
        }
        if ( !bFound && options.fullCheck() )
        {
            if ( options.forceOutput() )
            {
                fprintf(stdout, "EXISTENCE: key \"%s\" exists only in registry \"%s\"\n",
                        U2S(subKeyNames2.getElement(i)), options.getRegName2().c_str());
            }
            nError++;
        }
    }
    return nError;
}

static sal_uInt32 compareKeys(
    Options_Impl const & options,
    RegistryKey& key1,
    RegistryKey& key2)
{
    sal_uInt32 nError = 0;

    RegValueType valueType1 = RegValueType::NOT_DEFINED;
    RegValueType valueType2 = RegValueType::NOT_DEFINED;
    sal_uInt32 size1 = 0;
    sal_uInt32 size2 = 0;

    OUString tmpName;
    RegError e1 = key1.getValueInfo(tmpName, &valueType1, &size1);
    RegError e2 = key2.getValueInfo(tmpName, &valueType2, &size2);
    if ( (e1 == e2) && (e1 != RegError::VALUE_NOT_EXISTS) && (e1 != RegError::INVALID_VALUE) )
    {
        nError += checkValueDifference(options, key1, valueType1, size1, key2, valueType2, size2);
    }
    else
    {
        if ( (e1 != RegError::INVALID_VALUE) || (e2 != RegError::INVALID_VALUE) )
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
    nError += checkDifferences(options, key1, keys, subKeyNames1, subKeyNames2);

    StringSet::iterator iter = keys.begin();
    StringSet::iterator end = keys.end();

    while ( iter !=  end )
    {
        OUString keyName(*iter);
        if ( options.matchedWithExcludeKey(keyName) )
        {
            ++iter;
            continue;
        }

        sal_Int32 nPos = keyName.lastIndexOf( '/' );
        keyName = keyName.copy( nPos != -1 ? nPos+1 : 0 );

        RegistryKey subKey1;
        if ( key1.openKey(keyName, subKey1) != RegError::NO_ERROR )
        {
            if ( options.forceOutput() )
            {
                fprintf(stdout, "ERROR: could not open key \"%s\" in registry \"%s\"\n",
                        U2S(*iter), options.getRegName1().c_str());
            }
            nError++;
        }

        RegistryKey subKey2;
        if ( key2.openKey(keyName, subKey2) != RegError::NO_ERROR )
        {
            if ( options.forceOutput() )
            {
                fprintf(stdout, "ERROR: could not open key \"%s\" in registry \"%s\"\n",
                        U2S(*iter), options.getRegName2().c_str());
            }
            nError++;
        }

        if ( subKey1.isValid() && subKey2.isValid() )
        {
            nError += compareKeys(options, subKey1, subKey2);
        }
        ++iter;
    }

    return nError;
}

#if (defined UNX) || defined __MINGW32__
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    try
    {
        std::vector< std::string > args;

        Options_Impl options(argv[0]);
        for (int i = 1; i < argc; i++)
        {
            if (!Options::checkArgument(args, argv[i], strlen(argv[i])))
            {
                // failure.
                options.printUsage();
                return 1;
            }
        }
        if (!options.initOptions(args))
        {
            return 1;
        }

        OUString regName1( convertToFileUrl(options.getRegName1().c_str(), options.getRegName1().size()) );
        OUString regName2( convertToFileUrl(options.getRegName2().c_str(), options.getRegName2().size()) );

        Registry reg1, reg2;
        if ( reg1.open(regName1, RegAccessMode::READONLY) != RegError::NO_ERROR )
        {
            fprintf(stdout, "%s: open registry \"%s\" failed\n",
                    options.getProgramName().c_str(), options.getRegName1().c_str());
            return 2;
        }
        if ( reg2.open(regName2, RegAccessMode::READONLY) != RegError::NO_ERROR )
        {
            fprintf(stdout, "%s: open registry \"%s\" failed\n",
                    options.getProgramName().c_str(), options.getRegName2().c_str());
            return 3;
        }

        RegistryKey key1, key2;
        if ( reg1.openRootKey(key1) != RegError::NO_ERROR )
        {
            fprintf(stdout, "%s: open root key of registry \"%s\" failed\n",
                    options.getProgramName().c_str(), options.getRegName1().c_str());
            return 4;
        }
        if ( reg2.openRootKey(key2) != RegError::NO_ERROR )
        {
            fprintf(stdout, "%s: open root key of registry \"%s\" failed\n",
                    options.getProgramName().c_str(), options.getRegName2().c_str());
            return 5;
        }

        if ( options.isStartKeyValid() )
        {
            if ( options.matchedWithExcludeKey( options.getStartKey() ) )
            {
                fprintf(stdout, "%s: start key is equal to one of the exclude keys\n",
                        options.getProgramName().c_str());
                return 6;
            }
            RegistryKey sk1, sk2;
            if ( key1.openKey(options.getStartKey(), sk1) != RegError::NO_ERROR )
            {
                fprintf(stdout, "%s: open start key of registry \"%s\" failed\n",
                        options.getProgramName().c_str(), options.getRegName1().c_str());
                return 7;
            }
            if ( key2.openKey(options.getStartKey(), sk2) != RegError::NO_ERROR )
            {
                fprintf(stdout, "%s: open start key of registry \"%s\" failed\n",
                        options.getProgramName().c_str(), options.getRegName2().c_str());
                return 8;
            }

            key1 = sk1;
            key2 = sk2;
        }

        sal_uInt32 nError = compareKeys(options, key1, key2);
        if ( nError )
        {
            if ( options.unoTypeCheck() )
            {
                fprintf(stdout, "%s: registries are incompatible: %lu differences!\n",
                        options.getProgramName().c_str(),
                        sal::static_int_cast< unsigned long >(nError));
            }
            else
            {
                fprintf(stdout, "%s: registries contain %lu differences!\n",
                        options.getProgramName().c_str(),
                        sal::static_int_cast< unsigned long >(nError));
            }
        }

        key1.releaseKey();
        key2.releaseKey();
        if ( reg1.close() != RegError::NO_ERROR )
        {
            fprintf(stdout, "%s: closing registry \"%s\" failed\n",
                    options.getProgramName().c_str(), options.getRegName1().c_str());
            return 9;
        }
        if ( reg2.close() != RegError::NO_ERROR )
        {
            fprintf(stdout, "%s: closing registry \"%s\" failed\n",
                    options.getProgramName().c_str(), options.getRegName2().c_str());
            return 10;
        }

        return ((nError > 0) ? 11 : 0);
    }
    catch (std::exception& e)
    {
        fprintf(stdout, ("WARNING: \"%s\"\n"), e.what());
        return 11;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
