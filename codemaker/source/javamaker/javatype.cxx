/*************************************************************************
 *
 *  $RCSfile: javatype.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:27 $
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

#include    "javatype.hxx"
#include    "javaoptions.hxx"

using namespace rtl;

//*************************************************************************
// JavaType
//*************************************************************************
JavaType::JavaType(TypeReader& typeReader,
                   const OString& typeName,
                   const TypeManager& typeMgr,
                   const TypeDependency& typeDependencies)
    : m_indentLength(0)
    , m_typeName(typeName)
    , m_name(typeName.getToken(typeName.getTokenCount('/') - 1, '/'))
    , m_reader(typeReader)
    , m_typeMgr((TypeManager&)typeMgr)
    , m_dependencies(typeDependencies)
{
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

    OString fileName = createFileNameFromType(outPath, m_typeName, ".java");

    FileStream javaFile(fileName);

    if(!javaFile.isValid())
    {
        OString message("cannot open ");
        message += fileName + " for writing";
        throw CannotDumpException(message);
    }

    return dumpFile(javaFile);
}

sal_Bool JavaType::dumpDependedTypes(JavaOptions* pOptions)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_True;

    TypeUsingSet usingSet(m_dependencies.getDependencies(m_typeName));

    TypeUsingSet::const_iterator iter = usingSet.begin();
    OString typeName;
    sal_Int32 index = 0;
    while (iter != usingSet.end())
    {
        typeName = (*iter).m_type;
        if ((index = typeName.lastIndexOf(']')) > 0)
            typeName = typeName.copy(index + 1);

        if (getBaseType(typeName).getLength() == 0)
        {
            if (!produceType(typeName,
                                m_typeMgr,
                             m_dependencies,
                             pOptions))
            {
                fprintf(stderr, "%s ERROR: %s\n",
                        pOptions->getProgramName().getStr(),
                        OString("cannot dump Type '" + typeName + "'").getStr());
                exit(99);
            }
        }
        iter++;
    }

    return ret;
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

void JavaType::dumpDepImports(FileStream& o, const OString& typeName)
{
    TypeUsingSet usingSet(m_dependencies.getDependencies(typeName));

    TypeUsingSet::const_iterator iter = usingSet.begin();

    sal_Int32   index = 0;
    sal_Int32   seqNum = 0;
    OString     relType;
    while (iter != usingSet.end())
    {
        index = (*iter).m_type.lastIndexOf(']');
        seqNum = (index > 0 ? ((index+1) / 2) : 0);

        relType = (*iter).m_type;
        if (index > 0)
            relType = relType.copy(index+1);

        if (getBaseType(relType).getLength() == 0 &&
            m_typeName != relType)
        {
            /// dump import;
        }

        iter++;
    }
}

sal_uInt32 JavaType::getMemberCount()
{
    sal_uInt32 count = m_reader.getMethodCount();

    sal_uInt32 fieldCount = m_reader.getFieldCount();
    RTFieldAccess access = RT_ACCESS_INVALID;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access != RT_ACCESS_CONST && access != RT_ACCESS_INVALID)
            count++;
    }
    return count;
}

void JavaType::dumpType(FileStream& o, const OString& type)
    throw( CannotDumpException )
{
    sal_Int32 index = type.lastIndexOf(']');
    sal_Int32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    OString relType = (index > 0 ? ((OString)type).copy(index+1) : type);

    RTTypeClass typeClass = m_typeMgr.getTypeClass(relType);

    switch (typeClass)
    {
        case RT_TYPE_INVALID:
            {
                OString tmp(getBaseType(relType));
                if (tmp.getLength() > 0)
                    o << getBaseType(relType);
                else
                    throw CannotDumpException("Unknown type '" + relType + "', incomplete type library.");
            }
            break;
        case RT_TYPE_TYPEDEF:
            {
                OString baseType = checkSpecialJavaType(relType);
                dumpType(o, baseType);
            }
            break;
        case RT_TYPE_INTERFACE:
            if (relType.equals("com/sun/star/uno/XInterface"))
            {
                o << "java.lang.Object";
            } else
            {
                o << scopedName(m_typeName, relType);
            }
            break;
        case RT_TYPE_STRUCT:
        case RT_TYPE_ENUM:
        case RT_TYPE_EXCEPTION:
            o << scopedName(m_typeName, relType);
            break;
    }

    for (sal_Int32 i=0; i < seqNum; i++)
    {
        o << "[]";
    }
}

OString JavaType::getBaseType(const OString& type)
{
    if (type.equals("long"))
        return "int";
    if (type.equals("short"))
        return "short";
    if (type.equals("hyper"))
        return "long";
    if (type.equals("string"))
        return "String";
    if (type.equals("boolean"))
        return type;
    if (type.equals("char"))
        return type;
    if (type.equals("byte"))
        return type;
    if (type.equals("any"))
        return "java.lang.Object";
    if (type.equals("type"))
        return "com.sun.star.uno.Type";
    if (type.equals("float"))
        return type;
    if (type.equals("double"))
        return type;
    if (type.equals("octet"))
        return "byte";
    if (type.equals("void"))
        return type;
    if (type.equals("unsigned long"))
        return "int";
    if (type.equals("unsigned short"))
        return "short";
    if (type.equals("unsigned hyper"))
        return "long";

    return OString();
}

sal_Bool JavaType::isUnsigned(const OString& typeName)
{
    OString type(checkSpecialJavaType(typeName));

    sal_Int32 index = type.lastIndexOf(']');

    OString relType = (index > 0 ? ((OString)type).copy(index+1) : type);

    if ( relType.equals("unsigned long") ||
         relType.equals("unsigned short") ||
         relType.equals("unsigned hyper") )
        return sal_True;

    return sal_False;
}

sal_Bool JavaType::isAny(const OString& typeName)
{
    OString type(checkSpecialJavaType(typeName));

    sal_Int32 index = type.lastIndexOf(']');

    OString relType = (index > 0 ? ((OString)type).copy(index+1) : type);

    if ( relType.equals("any") )
        return sal_True;

    return sal_False;
}

sal_Bool JavaType::isInterface(const OString& typeName)
{
    OString type(checkSpecialJavaType(typeName));

    sal_Int32 index = type.lastIndexOf(']');

    OString relType = (index > 0 ? ((OString)type).copy(index+1) : type);

    RTTypeClass typeClass = m_typeMgr.getTypeClass(relType);

    if ( typeClass == RT_TYPE_INTERFACE)
        return sal_True;

    return sal_False;
}

void JavaType::dumpTypeInit(FileStream& o, const OString& name, const OString& typeName)
{
    OString type(checkSpecialJavaType(typeName));

    sal_Int32 index = type.lastIndexOf(']');
    sal_Int32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    OString relType = (index > 0 ? ((OString)type).copy(index+1) : type);

    if (seqNum > 0)
    {
        o << indent() << name << " = _static_seq_" << name << ";\n";
        return;
    }

    BASETYPE baseType = isBaseType(relType);

    switch (baseType)
    {
        case BT_STRING:
//          o << "new String()";
            o << indent() << name << " = \"\";\n";
            return;
        case BT_TYPE:
            o << indent() << name << " = new com.sun.star.uno.Type();\n";
            return;
        case BT_ANY:
//          o << "new java.lang.Object()";
//          return;
        case BT_BOOLEAN:
        case BT_CHAR:
        case BT_FLOAT:
        case BT_DOUBLE:
        case BT_BYTE:
        case BT_SHORT:
        case BT_LONG:
        case BT_HYPER:
        case BT_UNSIGNED_SHORT:
        case BT_UNSIGNED_LONG:
        case BT_UNSIGNED_HYPER:
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

BASETYPE JavaType::isBaseType(const OString& type)
{
    if (type.equals("long"))
        return BT_LONG;
    if (type.equals("short"))
        return BT_SHORT;
    if (type.equals("hyper"))
        return BT_HYPER;
    if (type.equals("string"))
        return BT_STRING;
    if (type.equals("boolean"))
        return BT_BOOLEAN;
    if (type.equals("char"))
        return BT_CHAR;
    if (type.equals("byte"))
        return BT_BYTE;
    if (type.equals("any"))
        return BT_ANY;
    if (type.equals("float"))
        return BT_FLOAT;
    if (type.equals("double"))
        return BT_DOUBLE;
    if (type.equals("void"))
        return BT_VOID;
    if (type.equals("type"))
        return BT_TYPE;
    if (type.equals("unsigned long"))
        return BT_UNSIGNED_LONG;
    if (type.equals("unsigned short"))
        return BT_UNSIGNED_SHORT;
    if (type.equals("unsigned hyper"))
        return BT_UNSIGNED_HYPER;

    return BT_INVALID;
}

OString JavaType::checkSpecialJavaType(const OString& type)
{
    OString baseType(type);

    RegistryTypeReaderLoader & rReaderLoader = getRegistryTypeReaderLoader();

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
                    TypeReader reader(rReaderLoader, pBuffer, valueSize, sal_False);

                    typeClass = reader.getTypeClass();

                    if (typeClass == RT_TYPE_TYPEDEF)
                        baseType = reader.getSuperTypeName();
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

OString JavaType::checkRealBaseType(const OString& type)
{
    sal_Int32 index = type.lastIndexOf(']');
    OString baseType = (index > 0 ? ((OString)type).copy(index+1) : type);

    RegistryTypeReaderLoader & rReaderLoader = getRegistryTypeReaderLoader();

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
                    TypeReader reader(rReaderLoader, pBuffer, valueSize, sal_False);

                    typeClass = reader.getTypeClass();

                    if (typeClass == RT_TYPE_TYPEDEF)
                    {
                        baseType = reader.getSuperTypeName();
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
    RTConstValue constValue = m_reader.getFieldConstValue(index);

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
        case RT_TYPE_FLOAT:
            o << "(float)" << constValue.m_value.aFloat;
            break;
        case RT_TYPE_DOUBLE:
            o << "(double)" << constValue.m_value.aDouble;
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

    OString     superType(m_reader.getSuperTypeName());
    sal_Bool    withIndent = sal_False;
    if (superType.getLength() > 0)
        withIndent = dumpInheritedMembers(o, superType, sal_True);

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    sal_Bool        first = withIndent;

    sal_uInt16 i;
    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);

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

        dumpType(o, m_reader.getFieldType(i));
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
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

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
                RegistryTypeReaderLoader & rReaderLoader = getRegistryTypeReaderLoader();

                TypeReader reader(rReaderLoader, pBuffer, valueSize, sal_False);

                OString superType(reader.getSuperTypeName());
                if (superType.getLength() > 0)
                    withIndent = dumpInheritedMembers(o, superType, first, withType);

                first = withIndent;

                sal_uInt32      fieldCount = reader.getFieldCount();
                RTFieldAccess   access = RT_ACCESS_INVALID;
                for (sal_uInt16 i=0; i < fieldCount; i++)
                {
                    access = reader.getFieldAccess(i);

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
                        dumpType(o, reader.getFieldType(i));
                        o << " ";
                    }

                    o << "_" << reader.getFieldName(i);

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
    OString type(checkSpecialJavaType(typeName));

    sal_Int32 index = type.lastIndexOf(']');
    sal_Int32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    if (seqNum > 0)
    {
        OString relType = (index > 0 ? ((OString)type).copy(index+1) : type);

        o << indent() << "public static final ";
        dumpType(o, relType);
        for (sal_Int32 i=0; i < seqNum; i++)
        {
            o << "[]";
        }
        o << " _static_seq_" << name << " = new ";
        dumpType(o, relType);
        for (i=0; i < seqNum; i++)
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

    for (int i=0; i < m_indentLength; i++)
    {
        tmp.append(' ');
    }
    return tmp.makeStringAndClear();
}

OString JavaType::indent(sal_uInt32 num)
{
    OStringBuffer tmp(m_indentLength + num);

    for (int i=0; i < m_indentLength + num; i++)
    {
        tmp.append(' ');
    }
    return tmp.makeStringAndClear();
}

//*************************************************************************
// InterfaceType
//*************************************************************************
InterfaceType::InterfaceType(TypeReader& typeReader,
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

    OString superType(m_reader.getSuperTypeName());
    if (superType.getLength() > 0)
        o << " extends " << scopedName(m_typeName, superType);

    o << "\n{\n";
    inc();

    UnoInfoList aUnoTypeInfos;

    dumpAttributes(o, &aUnoTypeInfos);
    dumpMethods(o, &aUnoTypeInfos);

    o << indent() << "// static Member\n" << indent() << "public static "
      << "com.sun.star.uno.Uik UIK = new com.sun.star.uno.Uik( ";

    RTUik uik;
    m_reader.getUik(uik);
    sal_Char buffer[67];
    sprintf(buffer, "0x%.8x, (short)0x%.4x, (short)0x%.4x, 0x%.8x, 0x%.8x",
            uik.m_Data1, uik.m_Data2, uik.m_Data3, uik.m_Data4, uik.m_Data5);
    o << buffer << " );\n";

    if (!aUnoTypeInfos.empty())
    {
        o << "\n" << indent() << "public static final com.sun.star.lib.uno.typeinfo.TypeInfo UNOTYPEINFO[] = { \n";

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

    o << "\n" << indent() << "public static Object UNORUNTIMEDATA = null;\n";

    dec();
    o << "}\n\n";

    return sal_True;
}

void InterfaceType::dumpAttributes(FileStream& o, UnoInfoList* pUnoInfos)
{
    sal_uInt32 fieldCount = m_reader.getFieldCount();
    sal_Bool first=sal_True;

    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldName;
    OString fieldType;
    sal_Int32 flags;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        flags = 0;
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

        if (first)
        {
            first = sal_False;
            o << indent() << "// Attributes\n";
        }

        o << indent() << "public ";
        dumpType(o, fieldType);
        o << " get" << fieldName << "() throws com.sun.star.uno.RuntimeException;\n";

        if (access != RT_ACCESS_READONLY)
        {
            o << indent() << "public void set" << fieldName << "( ";
            dumpType(o, fieldType);
            o << " _" << fieldName.toLowerCase() << " ) throws com.sun.star.uno.RuntimeException;\n";
        }

        if (access == RT_ACCESS_READONLY)
            flags = flags | UIT_READONLY;
        if (isUnsigned(fieldType))
            flags = flags | UIT_UNSIGNED;
        if (isAny(fieldType))
            flags = flags | UIT_ANY;
        if (isInterface(fieldType))
            flags = flags | UIT_INTERFACE;

          pUnoInfos->push_back(UnoInfo(fieldName, "", UNOTYPEINFO_ATTIRBUTE, i, flags));
    }

    if (!first)
        o << endl;
}

void InterfaceType::dumpMethods(FileStream& o, UnoInfoList* pUnoInfos)
{
    sal_uInt32 methodCount = m_reader.getMethodCount();
    sal_Bool first=sal_True;

    OString methodName, returnType, paramType, paramName;
    sal_uInt32 paramCount = 0;
    sal_uInt32 excCount = 0;
    RTMethodMode methodMode = RT_MODE_INVALID;
    RTParamMode  paramMode = RT_PARAM_INVALID;
    sal_Int32 flags;

    for (sal_uInt16 i=0; i < methodCount; i++)
    {
        flags = 0;

        methodName = m_reader.getMethodName(i);
        returnType = m_reader.getMethodReturnType(i);
        paramCount = m_reader.getMethodParamCount(i);
        excCount = m_reader.getMethodExcCount(i);
        methodMode = m_reader.getMethodMode(i);

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

        sal_uInt16 j;
        for (j=0; j < paramCount; j++)
        {
            flags = 0;
            paramName = m_reader.getMethodParamName(i, j);
            paramType = m_reader.getMethodParamType(i, j);
            paramMode = m_reader.getMethodParamMode(i, j);

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

        o << " throws ";
        OString excpName;
        for (j=0; j < excCount; j++)
        {
            excpName = m_reader.getMethodExcType(i, j);
            if (excpName != "com/sun/star/uno/RuntimeException")
                o << scopedName(m_typeName, excpName) << ", ";
        }
        o << "com.sun.star.uno.RuntimeException;\n";
    }

    if (!first)
        o << endl;
}

void InterfaceType::dumpUnoInfo(FileStream& o, const UnoInfo& unoInfo, sal_Int32 * index)
{
    switch (unoInfo.m_unoTypeInfo)
    {
        case UNOTYPEINFO_ATTIRBUTE:
            {
                sal_Bool hasFlags = sal_False;
                o << "new com.sun.star.lib.uno.typeinfo.AttributeTypeInfo( \"" << unoInfo.m_name << "\", " << (*index) << ", ";
                if (unoInfo.m_flags & UIT_READONLY)
                {
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

//*************************************************************************
// ModuleType
//*************************************************************************
ModuleType::ModuleType(TypeReader& typeReader,
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

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    OString         fileName;

    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST)
        {
            fieldName = m_reader.getFieldName(i);
            fieldType = m_reader.getFieldType(i);

            fileName = createFileNameFromType(outPath, m_typeName + "/" + fieldName, ".java");
            FileStream o(fileName);

            if(!o.isValid())
            {
                OString message("cannot open ");
                message += fileName + " for writing";
                throw CannotDumpException(message);
            }

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
        }
    }

    return sal_True;
}

sal_Bool ModuleType::hasConstants()
{
    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;

    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST)
            return sal_True;
    }

    return sal_False;
}

//*************************************************************************
// ConstantsType
//*************************************************************************
ConstantsType::ConstantsType(TypeReader& typeReader,
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

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    StringSet       aTypeInfos;

    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST)
        {
            fieldName = m_reader.getFieldName(i);
            fieldType = m_reader.getFieldType(i);

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
StructureType::StructureType(TypeReader& typeReader,
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

    OString superType(m_reader.getSuperTypeName());
    if (superType.getLength() > 0)
        o << " extends " << scopedName(m_typeName, superType);

    o << "\n{\n";
    inc();
    o << indent() << "//instance variables\n";

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
//  StringSet       aTypeInfos;
    UnoInfoList     aUnoTypeInfos;
    sal_Int32       flags = 0;

    sal_uInt16 i;
    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

        flags = 0;
        if (isUnsigned(fieldType))
            flags = flags | UIT_UNSIGNED;
        if (isAny(fieldType))
            flags = flags | UIT_ANY;
        if (isInterface(fieldType))
            flags = flags | UIT_INTERFACE;
        if (flags)
              aUnoTypeInfos.push_back(UnoInfo(fieldName, "", UNOTYPEINFO_MEMBER, i, flags));

        dumpSeqStaticMember(o, fieldType, fieldName);
        o << indent() << "public ";
        dumpType(o, fieldType);
        o << " " << fieldName << ";\n";
    }

    o << endl << indent() << "//constructors\n";
    o << indent() << "public " << m_name << "()\n" << indent() << "{\n";
    inc();
    OString relType;
    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

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

            if (++iter != aUnoTypeInfos.end())
                o << ",\n";
        }

        dec(63);
        o << " };\n";
    }

    o << "\n" << indent() << "public static Object UNORUNTIMEDATA = null;\n";

    dec();
    o << "}\n";

    return sal_True;
}


//*************************************************************************
// ExceptionType
//*************************************************************************
ExceptionType::ExceptionType(TypeReader& typeReader,
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
    } else
    {
        OString superType(m_reader.getSuperTypeName());
        if (superType.getLength() > 0)
            o << " extends " << scopedName(m_typeName, superType);
        else
            o << " extends java.lang.Exception\n";
    }

    o << "\n{\n";
    inc();
    o << indent() << "//instance variables\n";

    sal_uInt32      fieldCount = m_reader.getFieldCount();
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
    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

        flags = 0;

        if (isUnsigned(fieldType))
            flags = flags | UIT_UNSIGNED;
        if (isAny(fieldType))
            flags = flags | UIT_ANY;
        if (isInterface(fieldType))
            flags = flags | UIT_INTERFACE;
        if (flags)
              aUnoTypeInfos.push_back(UnoInfo(fieldName, "", UNOTYPEINFO_MEMBER, i, flags));

        if (m_typeName.equals("com/sun/star/uno/Exception") && fieldName.equals("Message"))
            continue;

        dumpSeqStaticMember(o, fieldType, fieldName);
        o << indent() << "public ";
        dumpType(o, fieldType);
        o << " " << fieldName << ";\n";
    }

    o << endl << indent() << "//constructors\n";
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
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

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
          << "new com.sun.star.lib.uno.typeinfo.MemberTypeInfo( \"Context\", com.sun.star.lib.uno.typeinfo.TypeInfo.INTERFACE ) };\n";
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

            if (++iter != aUnoTypeInfos.end())
                o << ",\n";
        }

        dec(63);
        o << " };\n";
    }

    o << "\n" << indent() << "public static Object UNORUNTIMEDATA = null;\n";

    dec();
    o << "}\n";

    return sal_True;
}

sal_Bool ExceptionType::dumpSimpleMemberConstructor(FileStream& o)
{
    o << indent() << "public " << m_name << "( String _Message )\n";

    OString         superType(m_reader.getSuperTypeName());
    sal_uInt32      fieldCount = m_reader.getFieldCount();
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
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

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
EnumType::EnumType(TypeReader& typeReader,
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
    o << indent() << "return " << m_reader.getFieldName(0) << ";\n";
    dec();
    o << indent() << "}\n\n";

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    RTConstValue    constValue;
    OString         fieldName;
    sal_uInt32      value=0;

    sal_uInt16 i;
    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access != RT_ACCESS_CONST)
            continue;

        fieldName = m_reader.getFieldName(i);
        constValue = m_reader.getFieldConstValue(i);

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
    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access != RT_ACCESS_CONST)
            continue;

        fieldName = m_reader.getFieldName(i);
        constValue = m_reader.getFieldConstValue(i);

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

    o << "\n" << indent() << "public static Object UNORUNTIMEDATA = null;\n";

    dec();
    o << "}\n";

    return sal_True;
}

//*************************************************************************
// TypeDefType
//*************************************************************************
TypeDefType::TypeDefType(TypeReader& typeReader,
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

    return produceType(relBaseType, m_typeMgr, m_dependencies, pOptions);
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

    RegistryKey     typeKey = typeMgr.getTypeKey(typeName);

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

    RTTypeClass typeClass = reader.getTypeClass();
    sal_Bool    ret = sal_False;
    switch (typeClass)
    {
        case RT_TYPE_INTERFACE:
            {
                InterfaceType iType(reader, typeName, typeMgr, typeDependencies);
                ret = iType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                ret = iType.dumpDependedTypes(pOptions);
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
                ret = sType.dumpDependedTypes(pOptions);
            }
            break;
        case RT_TYPE_ENUM:
            {
                EnumType enType(reader, typeName, typeMgr, typeDependencies);
                ret = enType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                ret = enType.dumpDependedTypes(pOptions);
            }
            break;
        case RT_TYPE_EXCEPTION:
            {
                ExceptionType eType(reader, typeName, typeMgr, typeDependencies);
                ret = eType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                ret = eType.dumpDependedTypes(pOptions);
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
                ret = tdType.dumpDependedTypes(pOptions);
            }
            break;
        case RT_TYPE_SERVICE:
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
        return type.getToken(type.getTokenCount('/') - 1, '/');

    // scoped name only if the namespace is not equal
    if (scope.lastIndexOf('/') > 0)
    {
        OString tmpScp(scope.copy(0, scope.lastIndexOf('/')));
        OString tmpScp2(type.copy(0, type.lastIndexOf('/')));

        if (tmpScp == tmpScp2)
            return type.getToken(type.getTokenCount('/') - 1, '/');
    }

    return type.replace('/', '.');
}



