/*************************************************************************
 *
 *  $RCSfile: javatype.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: rt $ $Date: 2004-03-30 16:53:25 $
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

#ifndef _RTL_ALLOC_H_
#include    <rtl/alloc.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include    <rtl/ustring.hxx>
#endif

#ifndef _RTL_STRBUF_HXX_
#include    <rtl/strbuf.hxx>
#endif

#include "registry/reader.hxx"
#include "registry/version.h"

#include "codemaker/exceptiontree.hxx"
#include "codemaker/unotypesort.hxx"

#include    "javatype.hxx"
#include    "javaoptions.hxx"

using namespace rtl;

namespace {

rtl::OString translateSimpleUnoType(rtl::OString const & unoType) {
    static rtl::OString const trans[codemaker::UNO_TYPE_SORT_COMPLEX + 1] = {
        "void", "boolean", "byte", "short", "short", "int", "int", "long",
        "long", "float", "double", "char", "java.lang.String",
        "com.sun.star.uno.Type", "java.lang.Object", rtl::OString() };
    return trans[codemaker::getUnoTypeSort(unoType)];
}

rtl::OString translateUnoSequenceRank(sal_Int32 rank) {
    rtl::OStringBuffer buf;
    for (sal_Int32 i = 0; i < rank; ++i) {
        buf.append(RTL_CONSTASCII_STRINGPARAM("[]"));
    }
    return buf.makeStringAndClear();
}

rtl::OString translateIdentifier(
    rtl::OString const & unoIdentifier, rtl::OString const & prefix)
{
    if (unoIdentifier == "abstract"
        || unoIdentifier == "assert" // since Java 1.4
        || unoIdentifier == "boolean"
        || unoIdentifier == "break"
        || unoIdentifier == "byte"
        || unoIdentifier == "case"
        || unoIdentifier == "catch"
        || unoIdentifier == "char"
        || unoIdentifier == "class"
        || unoIdentifier == "const"
        || unoIdentifier == "continue"
        || unoIdentifier == "default"
        || unoIdentifier == "do"
        || unoIdentifier == "double"
        || unoIdentifier == "else"
        || unoIdentifier == "enum" // probable addition in Java 1.5
        || unoIdentifier == "extends"
        || unoIdentifier == "final"
        || unoIdentifier == "finally"
        || unoIdentifier == "float"
        || unoIdentifier == "for"
        || unoIdentifier == "goto"
        || unoIdentifier == "if"
        || unoIdentifier == "implements"
        || unoIdentifier == "import"
        || unoIdentifier == "instanceof"
        || unoIdentifier == "int"
        || unoIdentifier == "interface"
        || unoIdentifier == "long"
        || unoIdentifier == "native"
        || unoIdentifier == "new"
        || unoIdentifier == "package"
        || unoIdentifier == "private"
        || unoIdentifier == "protected"
        || unoIdentifier == "public"
        || unoIdentifier == "return"
        || unoIdentifier == "short"
        || unoIdentifier == "static"
        || unoIdentifier == "strictfp"
        || unoIdentifier == "super"
        || unoIdentifier == "switch"
        || unoIdentifier == "synchronized"
        || unoIdentifier == "this"
        || unoIdentifier == "throw"
        || unoIdentifier == "throws"
        || unoIdentifier == "transient"
        || unoIdentifier == "try"
        || unoIdentifier == "void"
        || unoIdentifier == "volatile"
        || unoIdentifier == "while")
    {
        rtl::OStringBuffer buf(prefix);
        buf.append('_');
        buf.append(unoIdentifier);
        return buf.makeStringAndClear();
    } else {
        return unoIdentifier;
    }
}

}

//*************************************************************************
// JavaType
//*************************************************************************
JavaType::JavaType(typereg::Reader& typeReader,
                   const OString& typeName,
                   const TypeManager& typeMgr,
                   const TypeDependency& typeDependencies)
    : m_indentLength(0)
    , m_typeName(typeName)
    , m_reader(typeReader)
    , m_typeMgr((TypeManager&)typeMgr)
    , m_dependencies(typeDependencies)
{
    sal_Int32 nPos = typeName.lastIndexOf( '/' );
    m_name = typeName.copy( nPos+1 );
}

JavaType::~JavaType()
{

}

sal_Bool JavaType::dump(JavaOptions* pOptions)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_False;

    OString outPath;
    if (pOptions->isValid("-O"))
        outPath = pOptions->getOption("-O");

    OString sFileName(
        createFileNameFromType(outPath, translateTypeName(), ".java"));
    sal_Bool bFileExists = fileExists( sFileName );
    sal_Bool bFileCheck = sal_False;

    if ( bFileExists && pOptions->isValid("-G") )
        return sal_True;

    if ( bFileExists && pOptions->isValid("-Gc") )
        bFileCheck = sal_True;

    OString sTmpDir = getTempDir(sFileName);
    FileStream javaFile;
    javaFile.createTempFile(sTmpDir);
    OString sTmpFileName;

    if(!javaFile.isValid())
    {
        OString message("cannot open ");
        message += sFileName + " for writing";
        throw CannotDumpException(message);
    } else
        sTmpFileName = javaFile.getName();

    ret = dumpFile(javaFile);

    javaFile.close();

    if (ret) {
        ret = makeValidTypeFile(sFileName, sTmpFileName, bFileCheck);
    } else {
        // remove existing type file if something goes wrong to ensure consistency
        if (fileExists(sFileName))
            removeTypeFile(sFileName);

        // remove tmp file if something goes wrong
        removeTypeFile(sTmpFileName);
    }

    return ret;
}

void JavaType::dumpDependedTypes(JavaOptions * options) {
    TypeUsingSet set(m_dependencies.getDependencies(m_typeName));
    for (TypeUsingSet::const_iterator i(set.begin()); i != set.end(); ++i) {
        rtl::OString type(unfoldType(i->m_type));
        if (codemaker::getUnoTypeSort(type) == codemaker::UNO_TYPE_SORT_COMPLEX
            && !produceType(type, m_typeMgr, m_dependencies, options))
        {
            fprintf(
                stderr, "%s ERROR: cannot dump Type '%s'\n",
                options->getProgramName().getStr(), type.getStr());
            exit(99);
        }
    }
}

void JavaType::dumpPackage(FileStream& o, sal_Bool bFullScope)
{
    if (m_typeName.equals("/"))
        return;

    if (bFullScope)
    {
        o << "package " << m_typeName.replace('/', '.') << ";\n\n";
    } else
    {
        if (m_typeName.lastIndexOf('/') > 0)
            o << "package " << m_typeName.copy(0, m_typeName.lastIndexOf('/')).replace('/', '.') << ";\n\n";
    }
}

void JavaType::dumpType(FileStream& o, const OString& type)
    throw( CannotDumpException )
{
    sal_Int32 rank;
    rtl::OString unoType(unfoldType(type, &rank));
    switch (m_typeMgr.getTypeClass(unoType)) {
    case RT_TYPE_INVALID:
        {
            rtl::OString javaType(translateSimpleUnoType(unoType));
            if (javaType.getLength() == 0) {
                throw CannotDumpException(
                    "unknown type " + unoType + ", incomplete type library");
            }
            o << javaType;
            break;
        }

    case RT_TYPE_INTERFACE:
        if (unoType == "com/sun/star/uno/XInterface") {
            o << "java.lang.Object";
            break;
        }
    case RT_TYPE_ENUM:
    case RT_TYPE_STRUCT:
    case RT_TYPE_EXCEPTION:
        o << scopedName(m_typeName, unoType);
        break;

    default:
        OSL_ASSERT(false);
        break;
    }
    o << translateUnoSequenceRank(rank);
}

sal_Bool JavaType::isUnsigned(const OString& typeName)
{
    switch (codemaker::getUnoTypeSort(unfoldType(typeName))) {
    case codemaker::UNO_TYPE_SORT_UNSIGNED_SHORT:
    case codemaker::UNO_TYPE_SORT_UNSIGNED_LONG:
    case codemaker::UNO_TYPE_SORT_UNSIGNED_HYPER:
        return true;

    default:
        return false;
    }
}

sal_Bool JavaType::isAny(const OString& typeName)
{
    return codemaker::getUnoTypeSort(unfoldType(typeName))
        == codemaker::UNO_TYPE_SORT_ANY;
}

sal_Bool JavaType::isInterface(const OString& typeName)
{
    return m_typeMgr.getTypeClass(unfoldType(typeName)) == RT_TYPE_INTERFACE;
}

void JavaType::dumpTypeInit(FileStream& o, const OString& name, const OString& typeName)
{
    OString type(resolveTypedefs(typeName));

    sal_Int32 index = type.lastIndexOf(']');
    sal_Int32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    OString relType = (index > 0 ? ((OString)type).copy(index+1) : type);

    if (seqNum > 0)
    {
        o << indent() << name << " = _static_seq_" << name << ";\n";
        return;
    }

    switch (codemaker::getUnoTypeSort(relType))
    {
        case codemaker::UNO_TYPE_SORT_STRING:
//          o << "new String()";
            o << indent() << name << " = \"\";\n";
            return;
        case codemaker::UNO_TYPE_SORT_TYPE:
            o << indent() << name << " = com.sun.star.uno.Type.VOID;\n";
            return;
        case codemaker::UNO_TYPE_SORT_ANY:
            o << indent() << name << " = com.sun.star.uno.Any.VOID;\n";
            return;
        case codemaker::UNO_TYPE_SORT_BOOLEAN:
        case codemaker::UNO_TYPE_SORT_CHAR:
        case codemaker::UNO_TYPE_SORT_FLOAT:
        case codemaker::UNO_TYPE_SORT_DOUBLE:
        case codemaker::UNO_TYPE_SORT_BYTE:
        case codemaker::UNO_TYPE_SORT_SHORT:
        case codemaker::UNO_TYPE_SORT_LONG:
        case codemaker::UNO_TYPE_SORT_HYPER:
        case codemaker::UNO_TYPE_SORT_UNSIGNED_SHORT:
        case codemaker::UNO_TYPE_SORT_UNSIGNED_LONG:
        case codemaker::UNO_TYPE_SORT_UNSIGNED_HYPER:
//          o << "0";
            return;
    }

    RTTypeClass typeClass = m_typeMgr.getTypeClass(type);

    if (typeClass == RT_TYPE_ENUM)
    {
        o << indent() << name << " = " << type.replace('/', '.') << ".getDefault();\n";
        return;
    }

    if (typeClass == RT_TYPE_INTERFACE)
    {
//      o << "null";
        return;
    }

    o << indent() << name << " = new " << type.replace('/', '.') << "();\n";
}

OString JavaType::resolveTypedefs(const OString& unoType)
{
    OString baseType(unoType);

    RegistryKey     key;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    sal_uInt8*      pBuffer=NULL;
    RTTypeClass     typeClass;
    sal_Bool        isTypeDef = (m_typeMgr.getTypeClass(baseType) == RT_TYPE_TYPEDEF);

    while (isTypeDef)
    {
        key = m_typeMgr.getTypeKey(baseType);

        if (key.isValid())
        {
            if (!key.getValueInfo(OUString(), &valueType, &valueSize))
            {
                pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
                if (!key.getValue(OUString(), pBuffer))
                {
                    typereg::Reader reader(
                        pBuffer, valueSize, false, TYPEREG_VERSION_1);

                    typeClass = reader.getTypeClass();

                    if (typeClass == RT_TYPE_TYPEDEF)
                        baseType = rtl::OUStringToOString(
                            reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
                    else
                        isTypeDef = sal_False;
                }
                rtl_freeMemory(pBuffer);
            }
            key.closeKey();
        } else
        {
            break;
        }
    }

    return baseType;
}

OString JavaType::unfoldType(OString const & unoType, sal_Int32 * rank) {
    OString type(unoType);
    sal_Int32 n = 0;
    for (;;) {
        type = resolveTypedefs(type);
        sal_Int32 i = type.lastIndexOf(']');
        if (i < 0) {
            break;
        }
        type = type.copy(i + 1);
        n += i / 2 + 1;
    }
    if (rank != 0) {
        *rank = n;
    }
    return type;
}

OString JavaType::checkRealBaseType(const OString& type)
{
    sal_Int32 index = type.lastIndexOf(']');
    OString baseType = (index > 0 ? ((OString)type).copy(index+1) : type);

    RegistryKey     key;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    sal_uInt8*      pBuffer=NULL;
    RTTypeClass     typeClass;
    sal_Bool        mustBeChecked = (m_typeMgr.getTypeClass(baseType) == RT_TYPE_TYPEDEF);

    while (mustBeChecked)
    {
        key = m_typeMgr.getTypeKey(baseType);

        if (key.isValid())
        {
            if (!key.getValueInfo(OUString(), &valueType, &valueSize))
            {
                pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
                if (!key.getValue(OUString(), pBuffer))
                {
                    typereg::Reader reader(
                        pBuffer, valueSize, false, TYPEREG_VERSION_1);

                    typeClass = reader.getTypeClass();

                    if (typeClass == RT_TYPE_TYPEDEF)
                    {
                        baseType = rtl::OUStringToOString(
                            reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
                        index = baseType.lastIndexOf(']');
                          if (index > 0)
                        {
                            baseType = baseType.copy(index+1);
                        }

                    } else
                        mustBeChecked = sal_False;
                }
                rtl_freeMemory(pBuffer);
            }
            key.closeKey();
        } else
        {
            break;
        }
    }

    return baseType;
}

void JavaType::dumpConstantValue(FileStream& o, sal_uInt16 index)
{
    RTConstValue constValue = m_reader.getFieldValue(index);

    switch (constValue.m_type)
    {
        case RT_TYPE_BOOL:
            o << "(boolean)" << constValue.m_value.aBool;
            break;
        case RT_TYPE_BYTE:
            o << "(byte)" << (sal_Int16) constValue.m_value.aByte;
            break;
        case RT_TYPE_INT16:
            o << "(short)" << constValue.m_value.aShort << "L";
            break;
        case RT_TYPE_UINT16:
            o << "(short)" << constValue.m_value.aUShort << "L";
            break;
        case RT_TYPE_INT32:
            o << "(int)" << constValue.m_value.aLong << "L";
            break;
        case RT_TYPE_UINT32:
            o << "(int)" << constValue.m_value.aULong << "L";
            break;
        case RT_TYPE_INT64:
            {
                ::rtl::OString tmp( OString::valueOf(constValue.m_value.aHyper) );
                o << "(long)" << tmp.getStr() << "L";
            }
            break;
        case RT_TYPE_UINT64:
            {
                ::rtl::OString tmp( OString::valueOf((sal_Int64)constValue.m_value.aUHyper) );
                o << "(long)" << tmp.getStr() << "L";;
            }
            break;
        case RT_TYPE_FLOAT:
            {
                ::rtl::OString tmp( OString::valueOf(constValue.m_value.aFloat) );
                o << "(float)" << tmp.getStr();
            }
            break;
        case RT_TYPE_DOUBLE:
            {
                ::rtl::OString tmp( OString::valueOf(constValue.m_value.aDouble) );
                o << "(double)" << tmp.getStr();
            }
            break;
        case RT_TYPE_STRING:
            {
                ::rtl::OUString aUStr(constValue.m_value.aString);
                ::rtl::OString aStr = ::rtl::OUStringToOString(aUStr, RTL_TEXTENCODING_ASCII_US);
                o << "L\"" << aStr.getStr() << "\"";
            }
            break;
    }
}

sal_Bool JavaType::dumpMemberConstructor(FileStream& o)
{
    o << indent() << "public " << m_name << "( ";
    inc(9 + m_name.getLength());

    OString     superType;
    if (m_reader.getSuperTypeCount() >= 1) {
        superType = rtl::OUStringToOString(
            m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
    }
    sal_Bool    withIndent = sal_False;
    if (superType.getLength() > 0)
        withIndent = dumpInheritedMembers(o, superType, sal_True);

    sal_uInt16      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    sal_Bool        first = withIndent;

    sal_uInt16 i;
    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);

        if (withIndent)
        {
            if (first)
            {
                first = sal_False;
                o << ",\n";
            }
            o << indent();
        } else
            withIndent = sal_True;

        dumpType(
            o,
            rtl::OUStringToOString(
                m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8));
        o << " _" << fieldName;

        if (i+1 < fieldCount)
            o << ",\n";
    }

    o << " )\n";
    dec(9 + m_name.getLength());
    o << indent() << "{\n";
    inc();

    if (m_typeName.equals("com/sun/star/uno/RuntimeException"))
    {
        o << indent() << "super( _Message );\n";
        o << indent() << "Context" << " = _Context;\n";
    } else
    {
        if (superType.getLength() > 0)
        {
            o << indent() << "super( ";
            inc(7);
            dumpInheritedMembers(o, superType, sal_True, sal_False);
            o << " );\n";
            dec(7);
        } else
        {
            if (m_typeName.equals("com/sun/star/uno/Exception"))
                o << indent() << "super( _Message );\n";
        }
    }

    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
        fieldType = rtl::OUStringToOString(
            m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

        if (m_typeName.equals("com/sun/star/uno/Exception") && fieldName.equals("Message"))
            continue;

        o << indent() << fieldName << " = _" << fieldName << ";\n";
    }

    dec();
    o << indent() << "}\n";

    return sal_True;
}

sal_Bool JavaType::dumpInheritedMembers(FileStream& o, const OString& type, sal_Bool first, sal_Bool withType)
{
    RegistryKey     key = m_typeMgr.getTypeKey(type);
    RegValueType    valueType;
    sal_uInt32      valueSize;
    sal_uInt8*      pBuffer=NULL;
    sal_Bool        withIndent = sal_False;

    if (key.isValid())
    {
        if (!key.getValueInfo(OUString(), &valueType, &valueSize))
        {
            pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
            if (!key.getValue(OUString(), pBuffer))
            {
                typereg::Reader reader(
                    pBuffer, valueSize, false, TYPEREG_VERSION_1);

                if (reader.getSuperTypeCount() >= 1) {
                    withIndent = dumpInheritedMembers(
                        o,
                        rtl::OUStringToOString(
                            reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8),
                        first, withType);
                }

                first = withIndent;

                sal_uInt16      fieldCount = reader.getFieldCount();
                RTFieldAccess   access = RT_ACCESS_INVALID;
                for (sal_uInt16 i=0; i < fieldCount; i++)
                {
                    access = reader.getFieldFlags(i);

                    if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                        continue;

                    if (withIndent)
                    {
                        if (first)
                        {
                            first = sal_False;
                            o << ",\n";
                        }
                        o << indent();
                    }else
                        withIndent = sal_True;

                    if (withType)
                    {
                        dumpType(
                            o,
                            rtl::OUStringToOString(
                                reader.getFieldTypeName(i),
                                RTL_TEXTENCODING_UTF8));
                        o << " ";
                    }

                    o << "_"
                      << rtl::OUStringToOString(
                          reader.getFieldName(i), RTL_TEXTENCODING_UTF8);

                    if (i+1 < fieldCount)
                        o << ",\n";
                }
            }
            rtl_freeMemory(pBuffer);
        }
        key.closeKey();
    }

    return withIndent;
}

void JavaType::dumpSeqStaticMember(FileStream& o, const ::rtl::OString& typeName,
                                   const ::rtl::OString& name)
{
    OString type(resolveTypedefs(typeName));

    sal_Int32 index = type.lastIndexOf(']');
    sal_Int32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    if (seqNum > 0)
    {
        OString relType = (index > 0 ? ((OString)type).copy(index+1) : type);

        o << indent() << "public static final ";
        dumpType(o, relType);
        o << translateUnoSequenceRank(seqNum) << " _static_seq_" << name
          << " = new ";
        dumpType(o, relType);
        for (sal_Int32 i=0; i < seqNum; i++)
        {
            o << "[0]";
        }
        o << ";\n";
        return;
    }
}


void JavaType::inc(sal_uInt32 num)
{
    m_indentLength += num;
}

void JavaType::dec(sal_uInt32 num)
{
    if (m_indentLength - num < 0)
        m_indentLength = 0;
    else
        m_indentLength -= num;
}

OString JavaType::indent()
{
    OStringBuffer tmp(m_indentLength);

    for (sal_uInt32 i=0; i < m_indentLength; i++)
    {
        tmp.append(' ');
    }
    return tmp.makeStringAndClear();
}

OString JavaType::indent(sal_uInt32 num)
{
    OStringBuffer tmp(m_indentLength + num);

    for (sal_uInt32 i=0; i < m_indentLength + num; i++)
    {
        tmp.append(' ');
    }
    return tmp.makeStringAndClear();
}

//*************************************************************************
// InterfaceType
//*************************************************************************
InterfaceType::InterfaceType(typereg::Reader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr,
                             const TypeDependency& typeDependencies)
    : JavaType(typeReader, typeName, typeMgr, typeDependencies)
{
}

InterfaceType::~InterfaceType()
{

}

sal_Bool InterfaceType::dumpFile(FileStream& o)
    throw( CannotDumpException )
{
    dumpPackage(o);

    o << "public interface " << m_name;

    sal_uInt16 superTypeCount = m_reader.getSuperTypeCount();
    for (sal_uInt16 i = 0; i < superTypeCount; ++i) {
        OString superType(
            rtl::OUStringToOString(
                m_reader.getSuperTypeName(i), RTL_TEXTENCODING_UTF8));
        o << (i == 0 ? " extends " : ", ") << scopedName(m_typeName, superType);
    }

    o << "\n{\n";
    inc();

    UnoInfoList aUnoTypeInfos;

    dumpAttributes(o, &aUnoTypeInfos);
    dumpMethods(o, &aUnoTypeInfos);

    o << indent() << "// static Member\n";
/*
    o << indent() << "public static com.sun.star.uno.Uik UIK = new com.sun.star.uno.Uik( ";

    RTUik uik;
    m_reader.getUik(uik);
    sal_Char buffer[67];
    snprintf(buffer, sizeof(buffer), "0x%.8x, (short)0x%.4x, (short)0x%.4x, 0x%.8x, 0x%.8x",
            uik.m_Data1, uik.m_Data2, uik.m_Data3, uik.m_Data4, uik.m_Data5);
    o << buffer << " );\n\n";
*/
    o << indent() << "public static final com.sun.star.lib.uno.typeinfo.TypeInfo UNOTYPEINFO[] = ";

    if (!aUnoTypeInfos.empty())
    {
        o << "{ \n";
        inc();

        sal_Int32 index = 0;
        UnoInfoList::const_iterator iter = aUnoTypeInfos.begin();
        while (iter != aUnoTypeInfos.end())
        {
            o << indent();

            dumpUnoInfo(o, *iter, &index);
            if (++iter != aUnoTypeInfos.end())
                o << ",";

            o << "\n";
        }

        dec();
        o << indent() << " };\n";
    }
    else
        o << "null;\n";

//      o << "\n" << indent() << "public static Object UNORUNTIMEDATA = null;\n";

    dec();
    o << "}\n\n";

    return sal_True;
}

void InterfaceType::dumpAttributes(FileStream& o, UnoInfoList* pUnoInfos)
{
    sal_uInt16 fieldCount = m_reader.getFieldCount();
    sal_Bool first=sal_True;

    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldName;
    OString fieldType;
    sal_Int32 flags;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        flags = 0;
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        rtl::OUString name(m_reader.getFieldName(i));
        fieldName = rtl::OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        fieldType = rtl::OUStringToOString(
            m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

        if (first)
        {
            first = sal_False;
            o << indent() << "// Attributes\n";
        }

        o << indent() << "public ";
        dumpType(o, fieldType);
        o << " get" << fieldName << "()";
        dumpAttributeExceptionSpecification(o, name, RT_MODE_ATTRIBUTE_GET);
        o << ";\n";

        if ((access & RT_ACCESS_READONLY) == 0)
        {
            o << indent() << "public void set" << fieldName << "( ";
            dumpType(o, fieldType);
            o << " _" << fieldName.toAsciiLowerCase() << " )";
            dumpAttributeExceptionSpecification(o, name, RT_MODE_ATTRIBUTE_SET);
            o << ";\n";
        }

        if ((access & RT_ACCESS_BOUND) != 0) {
            flags |= UIT_BOUND;
        }
        if ((access & RT_ACCESS_READONLY) != 0) {
            flags |= UIT_READONLY;
        }
        if (isUnsigned(fieldType))
            flags = flags | UIT_UNSIGNED;
        if (isAny(fieldType))
            flags = flags | UIT_ANY;
        if (isInterface(fieldType))
            flags = flags | UIT_INTERFACE;

          pUnoInfos->push_back(UnoInfo(fieldName, "", UNOTYPEINFO_ATTIRBUTE, i, flags));
    }

    if (!first)
        o << "\n";
}

void InterfaceType::dumpMethods(FileStream& o, UnoInfoList* pUnoInfos)
{
    sal_uInt16 methodCount = m_reader.getMethodCount();
    sal_Bool first=sal_True;

    OString methodName, returnType, paramType, paramName;
    sal_uInt16 paramCount = 0;
    RTMethodMode methodMode = RT_MODE_INVALID;
    RTParamMode  paramMode = RT_PARAM_INVALID;
    sal_Int32 flags;

    for (sal_uInt16 i=0; i < methodCount; i++)
    {
        methodMode = m_reader.getMethodFlags(i);
        if (methodMode == RT_MODE_ATTRIBUTE_GET
            || methodMode == RT_MODE_ATTRIBUTE_SET)
        {
            continue;
        }

        flags = 0;

        methodName = rtl::OUStringToOString(
            m_reader.getMethodName(i), RTL_TEXTENCODING_UTF8);
        returnType = rtl::OUStringToOString(
            m_reader.getMethodReturnTypeName(i), RTL_TEXTENCODING_UTF8);
        paramCount = m_reader.getMethodParameterCount(i);

        if ( m_typeName.equals("com/sun/star/uno/XInterface") &&
             ( methodName.equals("queryInterface") ||
               methodName.equals("acquire") ||
               methodName.equals("release") ) )
        {
            continue;
        }

        if (isUnsigned(returnType))
            flags = flags | UIT_UNSIGNED;
        if (isAny(returnType))
            flags = flags | UIT_ANY;
        if (isInterface(returnType))
            flags = flags | UIT_INTERFACE;
        if (methodMode == RT_MODE_ONEWAY || methodMode == RT_MODE_ONEWAY_CONST)
            flags = flags | UIT_ONEWAY;
        if (methodMode == RT_MODE_ONEWAY_CONST || methodMode == RT_MODE_TWOWAY_CONST)
            flags = flags | UIT_CONST;

        if (sal_True || flags) // while unsorted, add always method type info
            pUnoInfos->push_back(UnoInfo(methodName, "", UNOTYPEINFO_METHOD, i, flags));

        if (first)
        {
            first = sal_False;
            o << indent() << "// Methods\n";
        }

        o << indent() << "public ";
        dumpType(o, returnType);
        o << " " << methodName << "( ";

        for (sal_uInt16 j=0; j < paramCount; j++)
        {
            flags = 0;
            paramName = rtl::OUStringToOString(
                m_reader.getMethodParameterName(i, j), RTL_TEXTENCODING_UTF8);
            paramType = rtl::OUStringToOString(
                m_reader.getMethodParameterTypeName(i, j),
                RTL_TEXTENCODING_UTF8);
            paramMode = m_reader.getMethodParameterFlags(i, j);

            switch (paramMode)
            {
                case RT_PARAM_IN:
                    flags = flags | UIT_IN;
                    o << "/*IN*/";
                    break;
                case RT_PARAM_OUT:
                    flags = flags | UIT_OUT;
                    o << "/*OUT*/";
                    break;
                case RT_PARAM_INOUT:
                    flags = flags | UIT_IN | UIT_OUT;
                    o << "/*INOUT*/";
                    break;
            }
            if (isUnsigned(paramType))
                flags = flags | UIT_UNSIGNED;
            if (isAny(paramType))
                flags = flags | UIT_ANY;
            if (isInterface(paramType))
                flags = flags | UIT_INTERFACE;
            if (flags && flags != 1)
                pUnoInfos->push_back(UnoInfo(paramName, methodName, UNOTYPEINFO_PARAMETER, j, flags));

            dumpType(o, paramType);
            if (paramMode != RT_PARAM_IN)
                o << "[]";

            o << " " << paramName;

            if (j+1 < paramCount) o << ", ";
        }
        o << " )";

        dumpExceptionSpecification(o, i);
        o << ";\n";
    }

    if (!first)
        o << "\n";
}

void InterfaceType::dumpUnoInfo(FileStream& o, const UnoInfo& unoInfo, sal_Int32 * index)
{
    switch (unoInfo.m_unoTypeInfo)
    {
        case UNOTYPEINFO_ATTIRBUTE:
            {
                sal_Bool hasFlags = sal_False;
                o << "new com.sun.star.lib.uno.typeinfo.AttributeTypeInfo( \"" << unoInfo.m_name << "\", " << (*index) << ", ";
                if ((unoInfo.m_flags & UIT_BOUND) != 0) {
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.BOUND";
                    hasFlags = true;
                }
                if ((unoInfo.m_flags & UIT_READONLY) != 0) {
                    if (hasFlags) {
                        o << " | ";
                    }
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.READONLY";
                    hasFlags = sal_True;
                }
                else // mutable attributes have also a set method (in addition to the get method)
                    (*index) ++;

                ++ (*index);

                if (unoInfo.m_flags & UIT_UNSIGNED)
                {
                    if (hasFlags)
                        o << "|";
                    else
                        hasFlags = sal_True;
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.UNSIGNED";
                }
                if (unoInfo.m_flags & UIT_ANY)
                {
                    if (hasFlags)
                        o << "|";
                    else
                        hasFlags = sal_True;
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.ANY";
                }
                if (unoInfo.m_flags & UIT_INTERFACE)
                {
                    if (hasFlags)
                        o << "|";
                    else
                        hasFlags = sal_True;
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.INTERFACE";
                }
                if (!hasFlags)
                {
                    o << "0";
                }
                o << " )";
            }
            break;
        case UNOTYPEINFO_METHOD:
            {
                sal_Bool hasFlags = sal_False;
                o << "new com.sun.star.lib.uno.typeinfo.MethodTypeInfo( \"" << unoInfo.m_name << "\", " << (*index) << ", ";
                ++ (*index);
                if (unoInfo.m_flags & UIT_UNSIGNED)
                {
                    if (hasFlags)
                        o << "|";
                    else
                        hasFlags = sal_True;
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.UNSIGNED";
                }
                if (unoInfo.m_flags & UIT_ONEWAY)
                {
                    if (hasFlags)
                        o << "|";
                    else
                        hasFlags = sal_True;
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.ONEWAY";
                }
                if (unoInfo.m_flags & UIT_CONST)
                {
                    if (hasFlags)
                        o << "|";
                    else
                        hasFlags = sal_True;
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.CONST";
                }
                if (unoInfo.m_flags & UIT_ANY)
                {
                    if (hasFlags)
                        o << "|";
                    else
                        hasFlags = sal_True;
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.ANY";
                }
                if (unoInfo.m_flags & UIT_INTERFACE)
                {
                    if (hasFlags)
                        o << "|";
                    else
                        hasFlags = sal_True;
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.INTERFACE";
                }
                if (!hasFlags)
                {
                    o << "0";
                }
                o << " )";
            }
            break;
        case UNOTYPEINFO_PARAMETER:
            {
                sal_Bool hasFlags = sal_False;
                o << "new com.sun.star.lib.uno.typeinfo.ParameterTypeInfo( \"" << unoInfo.m_name
                  << "\", \"" << unoInfo.m_methodName << "\", " << unoInfo.m_index
                  << ", ";
                if (unoInfo.m_flags & UIT_OUT)
                {
                    if (unoInfo.m_flags & UIT_IN)
                    {
                        hasFlags = sal_True;
                          o << "com.sun.star.lib.uno.typeinfo.TypeInfo.IN";
                    }

                    if (hasFlags)
                        o << "|";
                    else
                        hasFlags = sal_True;
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.OUT";
                }
                if (unoInfo.m_flags & UIT_ANY)
                {
                    if (hasFlags)
                        o << "|";
                    else
                        hasFlags = sal_True;
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.ANY";
                }
                if (unoInfo.m_flags & UIT_INTERFACE)
                {
                    if (hasFlags)
                        o << "|";
                    else
                        hasFlags = sal_True;
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.INTERFACE";
                }
                if (unoInfo.m_flags & UIT_UNSIGNED)
                {
                    if (hasFlags)
                        o << "|";
                    else
                        hasFlags = sal_True;
                    o << "com.sun.star.lib.uno.typeinfo.TypeInfo.UNSIGNED";
                }
                if (!hasFlags)
                {
                    o << "0";
                }
                o << " )";
            }
            break;
    }
}

void InterfaceType::dumpExceptionSpecification(
    FileStream & out, sal_uInt16 methodIndex)
{
    bool first = true;
    sal_uInt16 count = m_reader.getMethodExceptionCount(methodIndex);
    for (sal_uInt16 i = 0; i < count; ++i) {
        rtl::OUString name(m_reader.getMethodExceptionTypeName(methodIndex, i));
        if (!name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM(
                    "com/sun/star/uno/RuntimeException")))
        {
            out << (first ? " throws " : ", ")
                << scopedName(
                    m_typeName,
                    rtl::OUStringToOString(name, RTL_TEXTENCODING_UTF8));
            first = false;
        }
    }
}

void InterfaceType::dumpAttributeExceptionSpecification(
    FileStream & out, rtl::OUString const & name, RTMethodMode sort)
{
    sal_uInt16 methodCount = m_reader.getMethodCount();
    for (sal_uInt16 i = 0; i < methodCount; ++i) {
        if (m_reader.getMethodFlags(i) == sort
            && m_reader.getMethodName(i) == name)
        {
            dumpExceptionSpecification(out, i);
            break;
        }
    }
}

//*************************************************************************
// ModuleType
//*************************************************************************
ModuleType::ModuleType(typereg::Reader& typeReader,
                        const OString& typeName,
                       const TypeManager& typeMgr,
                       const TypeDependency& typeDependencies)
    : JavaType(typeReader, typeName, typeMgr, typeDependencies)
{
}

ModuleType::~ModuleType()
{

}

sal_Bool ModuleType::dump(JavaOptions* pOptions)
    throw( CannotDumpException )
{
    OString outPath;
    if (pOptions->isValid("-O"))
        outPath = pOptions->getOption("-O");

    sal_uInt16    fieldCount = m_reader.getFieldCount();
    RTFieldAccess access = RT_ACCESS_INVALID;
    OString       fieldName;
    OString       fieldType;
    OString       fileName;
    OString       sTmpDir;
    OString       sTmpFileName;
    sal_Bool      ret;
    sal_Bool      bFileExists;
    sal_Bool      bFileCheck;

    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        ret = sal_False;
        bFileCheck = sal_False;
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST)
        {
            fieldName = rtl::OUStringToOString(
                m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
            fieldType = rtl::OUStringToOString(
                m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

            fileName = createFileNameFromType(outPath, m_typeName + "/" + fieldName, ".java");
            bFileExists = fileExists(fileName);
            if ( bFileExists  && pOptions->isValid("-G") )
                break;

            if ( bFileExists && pOptions->isValid("-Gc") )
                bFileCheck = sal_True;

            sTmpDir = getTempDir(fileName);
            FileStream o;
            o.createTempFile(sTmpDir);

            if(!o.isValid())
            {
                OString message("cannot open ");
                message += fileName + " for writing";
                throw CannotDumpException(message);
            } else
                sTmpFileName = o.getName();

            dumpPackage(o, sal_True);
            o << indent() << "public interface " << fieldName << "\n{\n";
            inc();
            o << indent() << "public static final ";
            dumpType(o, fieldType);
            o << " value = ";
            dumpConstantValue(o, i);
            o << ";\n\n";

            if (isUnsigned(fieldType))
            {
                o << indent() << "public static final com.sun.star.lib.uno.typeinfo.TypeInfo UNOTYPEINFO[] = { new com.sun.star.lib.uno.typeinfo.ConstantTypeInfo( \""
                  << fieldName << "\", com.sun.star.lib.uno.typeinfo.TypeInfo.UNSIGNED ) };\n";
            }

            dec();
            o << "}\n";

            makeValidTypeFile(fileName, sTmpFileName, bFileCheck);
        }
    }

    return sal_True;
}

sal_Bool ModuleType::hasConstants()
{
    sal_uInt16      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;

    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST)
            return sal_True;
    }

    return sal_False;
}

//*************************************************************************
// ConstantsType
//*************************************************************************
ConstantsType::ConstantsType(typereg::Reader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr,
                             const TypeDependency& typeDependencies)
    : JavaType(typeReader, typeName, typeMgr, typeDependencies)
{
}

ConstantsType::~ConstantsType()
{

}

sal_Bool ConstantsType::dumpFile(FileStream& o)
    throw( CannotDumpException )
{
    dumpPackage(o);

    o << "public interface " << m_name << "\n{\n";
    inc();

    sal_uInt16      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    StringSet       aTypeInfos;

    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST)
        {
            fieldName = rtl::OUStringToOString(
                m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
            fieldType = rtl::OUStringToOString(
                m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

            if (isUnsigned(fieldType))
                aTypeInfos.insert(fieldName);

            o << indent() << "public static final ";
            dumpType(o, fieldType);
            o << " " << fieldName << " = ";
            dumpConstantValue(o, i);
            o << ";\n";
        }
    }

    if (!aTypeInfos.empty())
    {
        o << "\n" << indent() << "public static final com.sun.star.lib.uno.typeinfo.TypeInfo UNOTYPEINFO[] = { ";
        inc(63);

        StringSet::const_iterator iter = aTypeInfos.begin();
        while (iter != aTypeInfos.end())
        {
            if (iter != aTypeInfos.begin())
                o << indent();

            o << "new com.sun.star.lib.uno.typeinfo.ConstantTypeInfo( \"" << *iter << "\", com.sun.star.lib.uno.typeinfo.TypeInfo.UNSIGNED )";

            if (++iter != aTypeInfos.end())
                o << ",\n";
        }

        dec(63);
        o << " };\n";
    }

    dec();
    o << "}\n";

    return sal_True;
}


//*************************************************************************
// StructureType
//*************************************************************************
StructureType::StructureType(typereg::Reader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr,
                             const TypeDependency& typeDependencies)
    : JavaType(typeReader, typeName, typeMgr, typeDependencies)
{
}

StructureType::~StructureType()
{

}

sal_Bool StructureType::dumpFile(FileStream& o)
    throw( CannotDumpException )
{
    dumpPackage(o);

    o << "public class " << m_name;

    if (m_reader.getSuperTypeCount() == 1) {
        o << " extends "
          << scopedName(
              m_typeName,
              rtl::OUStringToOString(
                  m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8));
    }

    o << "\n{\n";
    inc();
    o << indent() << "//instance variables\n";

    sal_uInt16      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
//  StringSet       aTypeInfos;
    UnoInfoList     aUnoTypeInfos;
    sal_Int32       flags = 0;

    sal_uInt16 i;
    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
        fieldType = rtl::OUStringToOString(
            m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

        flags = 0;
        if (isUnsigned(fieldType))
            flags = flags | UIT_UNSIGNED;
        if (isAny(fieldType))
            flags = flags | UIT_ANY;
        if (isInterface(fieldType))
            flags = flags | UIT_INTERFACE;
        aUnoTypeInfos.push_back(UnoInfo(fieldName, "", UNOTYPEINFO_MEMBER, i, flags));

        dumpSeqStaticMember(o, fieldType, fieldName);
        o << indent() << "public ";
        dumpType(o, fieldType);
        o << " " << fieldName << ";\n";
    }

    o << "\n" << indent() << "// constructors\n";
    o << indent() << "public " << m_name << "()\n" << indent() << "{\n";
    inc();
    OString relType;
    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
        fieldType = rtl::OUStringToOString(
            m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

        dumpTypeInit(o, fieldName, fieldType);
    }
    dec();
    o << indent() << "}\n\n";

    dumpMemberConstructor(o);

    if (!aUnoTypeInfos.empty())
    {
        o << "\n" << indent() << "public static final com.sun.star.lib.uno.typeinfo.TypeInfo UNOTYPEINFO[] = { ";
        inc(63);

        UnoInfoList::const_iterator iter = aUnoTypeInfos.begin();
        while (iter != aUnoTypeInfos.end())
        {
            if (iter != aUnoTypeInfos.begin())
                o << indent();

            o << "new com.sun.star.lib.uno.typeinfo.MemberTypeInfo( \"" << (*iter).m_name << "\", ";
            o << (*iter).m_index << ", ";

            if ((*iter).m_flags & UIT_UNSIGNED)
            {
                o << "com.sun.star.lib.uno.typeinfo.TypeInfo.UNSIGNED )";
            }
            else if ((*iter).m_flags & UIT_ANY)
            {
                o << "com.sun.star.lib.uno.typeinfo.TypeInfo.ANY )";
            }
            else if ((*iter).m_flags & UIT_INTERFACE)
            {
                o << "com.sun.star.lib.uno.typeinfo.TypeInfo.INTERFACE )";
            }
            else
            {
                o << "0 )";
            }

            if (++iter != aUnoTypeInfos.end())
                o << ",\n";
        }

        dec(63);
        o << " };\n";
    }

//      o << "\n" << indent() << "public static Object UNORUNTIMEDATA = null;\n";

    dec();
    o << "}\n";

    return sal_True;
}


//*************************************************************************
// ExceptionType
//*************************************************************************
ExceptionType::ExceptionType(typereg::Reader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr,
                             const TypeDependency& typeDependencies)
    : JavaType(typeReader, typeName, typeMgr, typeDependencies)
{
}

ExceptionType::~ExceptionType()
{

}

sal_Bool ExceptionType::dumpFile(FileStream& o)
    throw( CannotDumpException )
{
    dumpPackage(o);

    o << "public class " << m_name;

    if (m_typeName.equals("com/sun/star/uno/RuntimeException"))
    {
            o << " extends java.lang.RuntimeException\n";
    } else if (m_reader.getSuperTypeCount() == 1) {
        o << " extends "
          << scopedName(
              m_typeName,
              rtl::OUStringToOString(
                  m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8));
    } else {
        o << " extends java.lang.Exception\n";
    }

    o << "\n{\n";
    inc();
    o << indent() << "//instance variables\n";

    sal_uInt16      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
//  StringSet       aTypeInfos;
    UnoInfoList     aUnoTypeInfos;
    sal_Int32       flags = 0;

    if (m_typeName.equals("com/sun/star/uno/RuntimeException"))
    {
        o << indent() << "public java.lang.Object Context;\n";
    }

    sal_uInt16 i;
    sal_Int32 nOffset = 0;
    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
        fieldType = rtl::OUStringToOString(
            m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

        flags = 0;

        if (m_typeName.equals("com/sun/star/uno/Exception") && fieldName.equals("Message"))
        {
            nOffset ++;
            continue;
        }

        if (isUnsigned(fieldType))
            flags = flags | UIT_UNSIGNED;
        if (isAny(fieldType))
            flags = flags | UIT_ANY;
        if (isInterface(fieldType))
            flags = flags | UIT_INTERFACE;
        aUnoTypeInfos.push_back(UnoInfo(fieldName, "", UNOTYPEINFO_MEMBER, i-nOffset, flags));

        dumpSeqStaticMember(o, fieldType, fieldName);
        o << indent() << "public ";
        dumpType(o, fieldType);
        o << " " << fieldName << ";\n";
    }

    o << "\n" << indent() << "//constructors\n";
    o << indent() << "public " << m_name << "()\n" << indent() << "{\n";
    inc();
/*
    if (m_typeName.equals("com/sun/star/uno/RuntimeException"))
    {
        o << indent() << "Context = null;\n";
    }
*/
    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
        fieldType = rtl::OUStringToOString(
            m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

        if (m_typeName.equals("com/sun/star/uno/Exception") && fieldName.equals("Message"))
            continue;

           dumpTypeInit(o, fieldName, fieldType);
    }
    dec();

    o << indent() << "}\n\n";

    dumpSimpleMemberConstructor(o);
    dumpMemberConstructor(o);

    if (m_typeName.equals("com/sun/star/uno/RuntimeException"))
    {
        o << "\n" << indent() << "public static final com.sun.star.lib.uno.typeinfo.TypeInfo UNOTYPEINFO[] = { "
          << "new com.sun.star.lib.uno.typeinfo.MemberTypeInfo( \"Context\", 0, com.sun.star.lib.uno.typeinfo.TypeInfo.INTERFACE ) };\n";
    } else
    if (!aUnoTypeInfos.empty())
    {
        o << "\n" << indent() << "public static final com.sun.star.lib.uno.typeinfo.TypeInfo UNOTYPEINFO[] = { ";
        inc(63);

        UnoInfoList::const_iterator iter = aUnoTypeInfos.begin();
        while (iter != aUnoTypeInfos.end())
        {
            if (iter != aUnoTypeInfos.begin())
                o << indent();

            o << "new com.sun.star.lib.uno.typeinfo.MemberTypeInfo( \"" << (*iter).m_name << "\", ";
            o << (*iter).m_index << ", ";

            if ((*iter).m_flags & UIT_UNSIGNED)
            {
                o << "com.sun.star.lib.uno.typeinfo.TypeInfo.UNSIGNED )";
            }
            else if ((*iter).m_flags & UIT_ANY)
            {
                o << "com.sun.star.lib.uno.typeinfo.TypeInfo.ANY )";
            }
            else if ((*iter).m_flags & UIT_INTERFACE)
            {
                o << "com.sun.star.lib.uno.typeinfo.TypeInfo.INTERFACE )";
            }
            else
            {
                o << "0 )";
            }

            if (++iter != aUnoTypeInfos.end())
                o << ",\n";
        }

        dec(63);
        o << " };\n";
    }

//      o << "\n" << indent() << "public static Object UNORUNTIMEDATA = null;\n";

    dec();
    o << "}\n";

    return sal_True;
}

sal_Bool ExceptionType::dumpSimpleMemberConstructor(FileStream& o)
{
    o << indent() << "public " << m_name << "( String _Message )\n";

    sal_uInt16      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;

    o << indent() << "{\n";
    inc();

    o << indent() << "super( _Message );\n";
/*
    if (m_typeName.equals("com/sun/star/uno/RuntimeException"))
    {
        o << indent() << "super( _Message );\n";
        o << indent() << "Context" << " = null;\n";
    } else
    {
        o << indent() << "super( _Message );\n";
    }
*/
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
        fieldType = rtl::OUStringToOString(
            m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

        if (m_typeName.equals("com/sun/star/uno/Exception") && fieldName.equals("Message"))
            continue;

        dumpTypeInit(o, fieldName, fieldType);
    }

    dec();
    o << indent() << "}\n\n";

    return sal_True;
}

//*************************************************************************
// EnumType
//*************************************************************************
EnumType::EnumType(typereg::Reader& typeReader,
                    const OString& typeName,
                   const TypeManager& typeMgr,
                   const TypeDependency& typeDependencies)
    : JavaType(typeReader, typeName, typeMgr, typeDependencies)
{
}

EnumType::~EnumType()
{

}

sal_Bool EnumType::dumpFile(FileStream& o)
    throw( CannotDumpException )
{
    dumpPackage(o);

    o << "final public class " << m_name << " extends com.sun.star.uno.Enum\n{\n";
    inc();

    o << indent() << "private " << m_name << "(int value)\n" << indent() << "{\n";
    inc();
    o << indent() << "super(value);\n";
    dec();
    o << indent() << "}\n\n";

    o << indent() << "public static " << m_name << " getDefault()\n" << indent() << "{\n";
    inc();
    o << indent() << "return "
      << rtl::OUStringToOString(m_reader.getFieldName(0), RTL_TEXTENCODING_UTF8)
      << ";\n";
    dec();
    o << indent() << "}\n\n";

    sal_uInt16      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    RTConstValue    constValue;
    OString         fieldName;
    sal_Int32       value=0;

    sal_uInt16 i;
    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access != RT_ACCESS_CONST)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
        constValue = m_reader.getFieldValue(i);

        if (constValue.m_type == RT_TYPE_INT32)
            value = constValue.m_value.aLong;
        else
            value++;

        o << indent() << "public static final " << m_name << " " << fieldName << " = "
          << "new " << m_name << "(" << value << ");\n";
        o << indent() << "public static final int " << fieldName << "_value = " << value << ";\n";
    }

    o << "\n" << indent() << "public static " << m_name << " fromInt(int value)\n" << indent() << "{\n";
    inc();
    o << indent() << "switch( value )\n" << indent() << "{\n";
    inc();

    value = 0;
    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access != RT_ACCESS_CONST)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
        constValue = m_reader.getFieldValue(i);

        if (constValue.m_type == RT_TYPE_INT32)
            value = constValue.m_value.aLong;
        else
            value++;

        o << indent() << "case " << value << ":\n";
        inc();
        o << indent() << "return " << fieldName << ";\n";
        dec();
    }
    o << indent() << "default:\n";
    inc();
    o << indent() << "return null;\n";
    dec();

    dec();
    o << indent() << "}\n";
    dec();
    o << indent() << "}\n";

//      o << "\n" << indent() << "public static Object UNORUNTIMEDATA = null;\n";

    dec();
    o << "}\n";

    return sal_True;
}

//*************************************************************************
// TypeDefType
//*************************************************************************
TypeDefType::TypeDefType(typereg::Reader& typeReader,
                             const OString& typeName,
                            const TypeManager& typeMgr,
                            const TypeDependency& typeDependencies)
    : JavaType(typeReader, typeName, typeMgr, typeDependencies)
{
}

TypeDefType::~TypeDefType()
{

}

sal_Bool TypeDefType::dump(JavaOptions* pOptions)
    throw( CannotDumpException )
{
    OString relBaseType(checkRealBaseType(m_typeName));

    if ( pOptions->isValid("-nD") )
        return sal_True;

    return produceType(relBaseType, m_typeMgr, m_dependencies, pOptions);
}

//*************************************************************************
// ServiceType
//*************************************************************************

bool ServiceType::isSingleInterfaceBased() {
    return m_reader.getSuperTypeCount() == 1;
}

sal_Bool ServiceType::dumpFile(FileStream & out) throw (CannotDumpException) {
    if (!(checkTypeDependencies(
            m_typeMgr, m_dependencies,
            "com/sun/star/lang/XMultiComponentFactory")
          && checkTypeDependencies(
              m_typeMgr, m_dependencies, "com/sun/star/uno/DeploymentException")
          && checkTypeDependencies(
              m_typeMgr, m_dependencies, "com/sun/star/uno/Exception")
          && checkTypeDependencies(
              m_typeMgr, m_dependencies, "com/sun/star/uno/XComponentContext")))
    {
        return false;
    }
    rtl::OString javaName(translateIdentifier(m_name, "service"));
    rtl::OString fullName(m_typeName.replace('/', '.'));
    rtl::OString baseName(
        rtl::OUStringToOString(
            m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8));
    rtl::OString fullBaseName(baseName.replace('/', '.'));
    rtl::OString scopedBaseName(scopedName(m_typeName, baseName));
    dumpPackage(out);
    out << "public final class " << javaName << " {\n";
    inc();
    sal_uInt16 ctors = m_reader.getMethodCount();
    if (ctors == 0) {
        out << indent() << "public static " << scopedBaseName
            << " create(com.sun.star.uno.XComponentContext the_context) {\n";
        inc();
        out << indent()
            << ("com.sun.star.lang.XMultiComponentFactory the_factory"
                " = the_context.getServiceManager();\n")
            << indent() << "if (the_factory == null) {\n";
        inc();
        out << indent()
            << ("throw new com.sun.star.uno.DeploymentException(\"component"
                " context fails to supply service manager\", the_context);\n");
        dec();
        out << indent() << "}\n" << indent() << scopedBaseName
            << " the_instance;\n" << indent() << "try {\n";
        inc();
        out << indent() << "the_instance = (" << scopedBaseName
            << ") com.sun.star.uno.UnoRuntime.queryInterface(" << scopedBaseName
            << ".class, the_factory.createInstanceWithContext(\"" << fullName
            << "\", the_context));\n";
        dec();
        out << indent()
            << "} catch (com.sun.star.uno.Exception the_exception) {\n";
        inc();
        out << indent()
            << ("throw new com.sun.star.uno.DeploymentException(\"component"
                " context fails to supply service ")
            << fullName << " of type " << fullBaseName
            << ": \" + the_exception, the_context);\n";
        dec();
        out << indent() << "}\n" << indent() << "if (the_instance == null) {\n";
        inc();
        out << indent()
            << ("throw new com.sun.star.uno.DeploymentException(\"component"
                " context fails to supply service ")
            << fullName << " of type " << fullBaseName << "\", the_context);\n";
        dec();
        out << indent() << "}\n" << indent() << "return the_instance;\n";
        dec();
        out << indent() << "}\n\n";
    } else {
        for (sal_uInt16 i = 0; i < ctors; ++i) {
            out << indent() << "public static " << scopedBaseName
                << " "
                << translateIdentifier(
                    rtl::OUStringToOString(
                        m_reader.getMethodName(i), RTL_TEXTENCODING_UTF8),
                    "method")
                << "(com.sun.star.uno.XComponentContext the_context";
            sal_uInt16 params = m_reader.getMethodParameterCount(i);
            bool rest = params == 1
                && ((m_reader.getMethodParameterFlags(i, 0) & RT_PARAM_REST)
                    != 0);
            for (sal_uInt16 j = 0; j < params; ++j) {
                out << ", ";
                dumpType(
                    out,
                    rtl::OUStringToOString(
                        m_reader.getMethodParameterTypeName(i, j),
                        RTL_TEXTENCODING_UTF8));
                if ((m_reader.getMethodParameterFlags(i, j) & RT_PARAM_REST)
                    != 0)
                {
                    out << "[]";
                }
                out << " "
                    << translateIdentifier(
                        rtl::OUStringToOString(
                            m_reader.getMethodParameterName(i, j),
                            RTL_TEXTENCODING_UTF8),
                        "param");
            }
            out << ")";
            sal_uInt16 exceptions = m_reader.getMethodExceptionCount(i);
            codemaker::ExceptionTree tree;
            if (exceptions > 0) {
                out << " throws ";
                for (sal_uInt16 j = 0; j < exceptions; ++j) {
                    if (j > 0) {
                        out << ", ";
                    }
                    rtl::OString name(
                        rtl::OUStringToOString(
                            m_reader.getMethodExceptionTypeName(i, j),
                            RTL_TEXTENCODING_UTF8));
                    dumpType(out, name);
                    tree.add(name, m_typeMgr);
                }
            }
            out << " {\n";
            inc();
            out << indent()
                << ("com.sun.star.lang.XMultiComponentFactory the_factory"
                    " = the_context.getServiceManager();\n")
                << indent() << "if (the_factory == null) {\n";
            inc();
            out << indent()
                << ("throw new com.sun.star.uno.DeploymentException(\"component"
                    " context fails to supply service manager\","
                    " the_context);\n");
            dec();
            out << indent() << "}\n" << indent() << scopedBaseName
                << " the_instance;\n";
            if (!tree.getRoot()->present) {
                out << indent() << "try {\n";
                inc();
            }
            out << indent() << "the_instance = (" << scopedBaseName
                << ") com.sun.star.uno.UnoRuntime.queryInterface("
                << scopedBaseName
                << (".class, the_factory"
                    ".createInstanceWithArgumentsAndContext(\"")
                << fullName << "\", ";
            if (rest) {
                out << translateIdentifier(
                    rtl::OUStringToOString(
                        m_reader.getMethodParameterName(i, 0),
                        RTL_TEXTENCODING_UTF8),
                    "param");
            } else {
                out << "new java.lang.Object[] {";
                for (sal_uInt16 j = 0; j < params; ++j) {
                    if (j > 0) {
                        out << ",";
                    }
                    out << " ";
                    dumpAny(
                        out,
                        translateIdentifier(
                            rtl::OUStringToOString(
                                m_reader.getMethodParameterName(i, j),
                                RTL_TEXTENCODING_UTF8),
                            "param"),
                        rtl::OUStringToOString(
                            m_reader.getMethodParameterTypeName(i, j),
                            RTL_TEXTENCODING_UTF8));
                }
                if (params > 0) {
                    out << " ";
                }
                out << "}";
            }
            out << ", the_context));\n";
            if (!tree.getRoot()->present) {
                dec();
                dumpCatchClauses(out, tree.getRoot());
                out << indent()
                    << "} catch (com.sun.star.uno.Exception the_exception) {\n";
                inc();
                out << indent()
                    << ("throw new com.sun.star.uno.DeploymentException("
                        "\"component context fails to supply service ")
                    << fullName << " of type " << fullBaseName
                    << ": \" + the_exception, the_context);\n";
                dec();
                out << indent() << "}\n";
            }
            out << indent() << "if (the_instance == null) {\n";
            inc();
            out << indent()
                << ("throw new com.sun.star.uno.DeploymentException(\"component"
                    " context fails to supply service ")
                << fullName << " of type " << fullBaseName
                << "\", the_context);\n";
            dec();
            out << indent() << "}\n" << indent() << "return the_instance;\n";
            dec();
            out << indent() << "}\n\n";
        }
    }
    out << indent() << "private " << javaName
        << "() {} // do not instantiate\n";
    dec();
    out << "}\n";
    return true;
}

rtl::OString ServiceType::translateTypeName() {
    return m_typeName.copy(0, m_typeName.lastIndexOf('/') + 1)
        + translateIdentifier(m_name, "service");
}

void ServiceType::dumpCatchClauses(
    FileStream & out, codemaker::ExceptionTreeNode const * node)
{
   if (node->present) {
        out << indent() << "} catch (";
        dumpType(out, node->name);
        out << " the_exception) {\n";
        inc();
        out << indent() << "throw the_exception;\n";
        dec();
    } else {
        for (codemaker::ExceptionTreeNode::Children::const_iterator i(
                 node->children.begin());
             i != node->children.end(); ++i)
        {
            dumpCatchClauses(out, *i);
        }
    }
}

void ServiceType::dumpAny(
    FileStream & out, rtl::OString const & javaExpression,
    rtl::OString const & unoType)
{
    sal_Int32 rank;
    rtl::OString type(unfoldType(unoType, &rank));
    switch (m_typeMgr.getTypeClass(type)) {
    case RT_TYPE_INVALID:
        switch (codemaker::getUnoTypeSort(type)) {
        case codemaker::UNO_TYPE_SORT_BOOLEAN:
            out << javaExpression;
            if (rank == 0) {
                out << " ? java.lang.Boolean.TRUE : java.lang.Boolean.FALSE";
            }
            break;

        case codemaker::UNO_TYPE_SORT_BYTE:
            if (rank == 0) {
                out << "new java.lang.Byte(" << javaExpression << ")";
            } else {
                out << javaExpression;
            }
            break;

        case codemaker::UNO_TYPE_SORT_SHORT:
            if (rank == 0) {
                out << "new java.lang.Short(" << javaExpression << ")";
            } else {
                out << javaExpression;
            }
            break;

        case codemaker::UNO_TYPE_SORT_UNSIGNED_SHORT:
            out << "new com.sun.star.uno.Any(";
            if (rank == 0) {
                out << ("com.sun.star.uno.Type.UNSIGNED_SHORT,"
                        " new java.lang.Short(")
                    << javaExpression << ")";
            } else {
                out << "new com.sun.star.uno.Type(short"
                    << translateUnoSequenceRank(rank) << ".class, true), "
                    << javaExpression;
            }
            out << ")";
            break;

        case codemaker::UNO_TYPE_SORT_LONG:
            if (rank == 0) {
                out << "new java.lang.Integer(" << javaExpression << ")";
            } else {
                out << javaExpression;
            }
            break;

        case codemaker::UNO_TYPE_SORT_UNSIGNED_LONG:
            out << "new com.sun.star.uno.Any(";
            if (rank == 0) {
                out << ("com.sun.star.uno.Type.UNSIGNED_LONG,"
                        " new java.lang.Integer(")
                    << javaExpression << ")";
            } else {
                out << "new com.sun.star.uno.Type(int"
                    << translateUnoSequenceRank(rank) << ".class, true), "
                    << javaExpression;
            }
            out << ")";
            break;

        case codemaker::UNO_TYPE_SORT_HYPER:
            if (rank == 0) {
                out << "new java.lang.Long(" << javaExpression << ")";
            } else {
                out << javaExpression;
            }
            break;

        case codemaker::UNO_TYPE_SORT_UNSIGNED_HYPER:
            out << "new com.sun.star.uno.Any(";
            if (rank == 0) {
                out << ("com.sun.star.uno.Type.UNSIGNED_HYPER,"
                        " new java.lang.Long(")
                    << javaExpression << ")";
            } else {
                out << "new com.sun.star.uno.Type(long"
                    << translateUnoSequenceRank(rank) << ".class, true), "
                    << javaExpression;
            }
            out << ")";
            break;

        case codemaker::UNO_TYPE_SORT_FLOAT:
            if (rank == 0) {
                out << "new java.lang.Float(" << javaExpression << ")";
            } else {
                out << javaExpression;
            }
            break;

        case codemaker::UNO_TYPE_SORT_DOUBLE:
            if (rank == 0) {
                out << "new java.lang.Double(" << javaExpression << ")";
            } else {
                out << javaExpression;
            }
            break;

        case codemaker::UNO_TYPE_SORT_CHAR:
            if (rank == 0) {
                out << "new java.lang.Character(" << javaExpression << ")";
            } else {
                out << javaExpression;
            }
            break;

        case codemaker::UNO_TYPE_SORT_STRING:
        case codemaker::UNO_TYPE_SORT_TYPE:
            // Assuming that no Java types are derived from
            // com.sun.star.uno.Type (if rank > 0):
            out << javaExpression;
            break;

        case codemaker::UNO_TYPE_SORT_ANY:
            if (rank == 0) {
                out << javaExpression;
            } else {
                out << ("new com.sun.star.uno.Any(new com.sun.star.uno.Type("
                        "com.sun.star.uno.Any")
                    << translateUnoSequenceRank(rank) << ".class), "
                    << javaExpression << ")";
            }
            break;

        default:
            OSL_ASSERT(false);
            break;
        }
        break;

    case RT_TYPE_ENUM:
        // Assuming that no Java types are derived from Java types that are
        // directly derived from com.sun.star.uno.Enum:
        out << javaExpression;
        break;

    case RT_TYPE_INTERFACE:
        if (type == "com/sun/star/uno/XInterface") {
            if (rank == 0) {
                out << javaExpression;
            } else {
                out << ("new com.sun.star.uno.Any(new com.sun.star.uno.Type("
                        "com.sun.star.uno.XInterface")
                    << translateUnoSequenceRank(rank) << ".class), "
                    << javaExpression << ")";
            }
            break;
        }
    case RT_TYPE_STRUCT:
    case RT_TYPE_EXCEPTION:
        out << "new com.sun.star.uno.Any(new com.sun.star.uno.Type("
            << scopedName(m_typeName, type) << translateUnoSequenceRank(rank)
            << ".class), " << javaExpression << ")";
        break;

    default:
        OSL_ASSERT(false);
        break;
    }
}

//*************************************************************************
// SingletonType
//*************************************************************************

bool SingletonType::isInterfaceBased() {
    return (m_typeMgr.getTypeClass(
                rtl::OUStringToOString(
                    m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8)))
        == RT_TYPE_INTERFACE;
}

sal_Bool SingletonType::dumpFile(FileStream & out) throw (CannotDumpException) {
    if (!(checkTypeDependencies(
              m_typeMgr, m_dependencies, "com/sun/star/uno/DeploymentException")
          && checkTypeDependencies(
              m_typeMgr, m_dependencies, "com/sun/star/uno/TypeClass")
          && checkTypeDependencies(
              m_typeMgr, m_dependencies, "com/sun/star/uno/XComponentContext")))
    {
        return false;
    }
    rtl::OString javaName(translateIdentifier(m_name, "singleton"));
    rtl::OString fullName(m_typeName.replace('/', '.'));
    rtl::OString baseName(
        rtl::OUStringToOString(
            m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8));
    rtl::OString fullBaseName(baseName.replace('/', '.'));
    rtl::OString scopedBaseName(scopedName(m_typeName, baseName));
    dumpPackage(out);
    out << "public final class " << javaName << " {\n";
    inc();
    out << indent() << "public static " << scopedBaseName
        << " get(com.sun.star.uno.XComponentContext context) {\n";
    inc();
    out << indent()
        << "java.lang.Object value = context.getValueByName(\"/singletons/"
        << fullName << "\");\n" << indent()
        << "if (value instanceof com.sun.star.uno.Any) {\n";
    inc();
    out << indent()
        << "com.sun.star.uno.Any any = (com.sun.star.uno.Any) value;\n"
        << indent()
        << ("if (any.getType().getTypeClass() "
            "!= com.sun.star.uno.TypeClass.INTERFACE) {\n");
    inc();
    out << indent()
        << ("throw new com.sun.star.uno.DeploymentException(\"component context"
            " fails to supply singleton ")
        << fullName << " of type " << fullBaseName << "\", context);\n";
    dec();
    out << indent() << "}\n" << indent() << "value = any.getObject();\n";
    dec();
    out << indent() << "}\n" << indent() << scopedBaseName << " instance = ("
        << scopedBaseName << ") com.sun.star.uno.UnoRuntime.queryInterface("
        << scopedBaseName << ".class, value);\n" << indent()
        << "if (instance == null) {\n";
    inc();
    out << indent()
        << ("throw new com.sun.star.uno.DeploymentException(\"component context"
            " fails to supply singleton ")
        << fullName << " of type " << fullBaseName << "\", context);\n";
    dec();
    out << indent() << "}\n" << indent() << "return instance;\n";
    dec();
    out << indent() << "}\n\n";
    out << indent() << "private " << javaName
        << "() {} // do not instantiate\n";
    dec();
    out << "}\n";
    return true;
}

rtl::OString SingletonType::translateTypeName() {
    return m_typeName.copy(0, m_typeName.lastIndexOf('/') + 1)
        + translateIdentifier(m_name, "singleton");
}

//*************************************************************************
// produceType
//*************************************************************************
sal_Bool produceType(const OString& typeName,
                     TypeManager& typeMgr,
                     TypeDependency& typeDependencies,
                     JavaOptions* pOptions)
    throw( CannotDumpException )
{
    if (typeDependencies.isGenerated(typeName))
        return sal_True;

    sal_Bool bIsExtraType = sal_False;
    typereg::Reader reader(typeMgr.getTypeReader(typeName, &bIsExtraType));
    if (bIsExtraType)
    {
        typeDependencies.setGenerated(typeName);
        return sal_True;
    }

    if (!reader.isValid())
    {
        if (typeName.equals("/"))
            return sal_True;
        else
            return sal_False;
    }

    if( !checkTypeDependencies(typeMgr, typeDependencies, typeName))
        return sal_False;

/*
    sal_Bool bIsExtraType = sal_False;
    RegistryKey     typeKey = typeMgr.getTypeKey(typeName, &bIsExtraType);
    if (bIsExtraType)
    {
        typeDependencies.setGenerated(typeName);
        return sal_True;
    }

    if (!typeKey.isValid())
        return sal_False;

    if( !checkTypeDependencies(typeMgr, typeDependencies, typeName))
        return sal_False;

    RegValueType    valueType;
    sal_uInt32      valueSize;

    if (typeKey.getValueInfo(OUString(), &valueType, &valueSize))
    {
        if (typeName.equals("/"))
            return sal_True;
        else
            return sal_False;
    }
    sal_uInt8* pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);

    if (typeKey.getValue(OUString(), pBuffer))
    {
        rtl_freeMemory(pBuffer);
        return sal_False;
    }

    RegistryTypeReaderLoader & rReaderLoader = getRegistryTypeReaderLoader();

    TypeReader reader(rReaderLoader, pBuffer, valueSize, sal_True);

    rtl_freeMemory(pBuffer);
*/
    RTTypeClass typeClass = reader.getTypeClass();
    sal_Bool    ret = sal_False;
    switch (typeClass)
    {
        case RT_TYPE_INTERFACE:
            {
                InterfaceType iType(reader, typeName, typeMgr, typeDependencies);
                ret = iType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                if ( !pOptions->isValid("-nD") ) {
                    iType.dumpDependedTypes(pOptions);
                }
            }
            break;
        case RT_TYPE_MODULE:
            {
                ModuleType mType(reader, typeName, typeMgr, typeDependencies);
                if (mType.hasConstants())
                {
                    ret = mType.dump(pOptions);
                    if (ret) typeDependencies.setGenerated(typeName);
                } else
                    ret = sal_True;
            }
            break;
        case RT_TYPE_STRUCT:
            {
                StructureType sType(reader, typeName, typeMgr, typeDependencies);
                ret = sType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                if ( !pOptions->isValid("-nD") ) {
                    sType.dumpDependedTypes(pOptions);
                }
            }
            break;
        case RT_TYPE_ENUM:
            {
                EnumType enType(reader, typeName, typeMgr, typeDependencies);
                ret = enType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                if ( !pOptions->isValid("-nD") ) {
                    enType.dumpDependedTypes(pOptions);
                }
            }
            break;
        case RT_TYPE_EXCEPTION:
            {
                ExceptionType eType(reader, typeName, typeMgr, typeDependencies);
                ret = eType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                if ( !pOptions->isValid("-nD") ) {
                    eType.dumpDependedTypes(pOptions);
                }
            }
            break;
        case RT_TYPE_CONSTANTS:
            {
                ConstantsType cType(reader, typeName, typeMgr, typeDependencies);
                ret = cType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                return ret;
            }
            break;
        case RT_TYPE_TYPEDEF:
            {
                TypeDefType tdType(reader, typeName, typeMgr, typeDependencies);
                ret = tdType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                if ( !pOptions->isValid("-nD") ) {
                    tdType.dumpDependedTypes(pOptions);
                }
            }
            break;
        case RT_TYPE_SERVICE:
            {
                ServiceType t(reader, typeName, typeMgr, typeDependencies);
                if (t.isSingleInterfaceBased()) {
                    ret = t.dump(pOptions);
                    if (ret) {
                        typeDependencies.setGenerated(typeName);
                        if (!pOptions->isValid("-nD")) {
                            t.dumpDependedTypes(pOptions);
                        }
                    }
                } else {
                    ret = true;
                }
            }
            break;
        case RT_TYPE_SINGLETON:
            {
                SingletonType t(reader, typeName, typeMgr, typeDependencies);
                if (t.isInterfaceBased()) {
                    ret = t.dump(pOptions);
                    if (ret) {
                        typeDependencies.setGenerated(typeName);
                        if (!pOptions->isValid("-nD")) {
                            t.dumpDependedTypes(pOptions);
                        }
                    }
                } else {
                    ret = true;
                }
            }
            break;
        case RT_TYPE_OBJECT:
            ret = sal_True;
            break;
    }

    return ret;
}

//*************************************************************************
// scopedName
//*************************************************************************
OString scopedName(const OString& scope, const OString& type,
                   sal_Bool bNoNameSpace)
{
    if (type.indexOf('/') < 0)
        return type;

    if (bNoNameSpace)
        return type.copy( type.lastIndexOf( '/' )+1 );

    // scoped name only if the namespace is not equal
    if (scope.lastIndexOf('/') > 0)
    {
        OString tmpScp(scope.copy(0, scope.lastIndexOf('/')));
        OString tmpScp2(type.copy(0, type.lastIndexOf('/')));

        if (tmpScp == tmpScp2)
            return type.copy( type.lastIndexOf( '/' )+1 );
    }

    return type.replace('/', '.');
}



