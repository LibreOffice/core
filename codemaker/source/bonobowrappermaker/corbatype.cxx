/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_codemaker.hxx"

#include <stdio.h>
#include    <rtl/alloc.h>
#include    <rtl/ustring.hxx>
#include    <rtl/strbuf.hxx>

#include    "corbatype.hxx"
#include    "corbaoptions.hxx"

#include <hash_set>
#include <list>

using namespace rtl;


//*************************************************************************
// CorbaType
//*************************************************************************
CorbaType::CorbaType(TypeReader& typeReader,
        const OString& typeName,
        const TypeManager& typeMgr,
        const TypeDependency& typeDependencies,
        TypeSet* generatedConversions)
    : m_inheritedMemberCount(0)
    , m_indentLength(0)
    , m_typeName(typeName)
    , m_reader(typeReader)
    , m_typeMgr((TypeManager&)typeMgr)
    , m_dependencies(typeDependencies)
    , m_generatedConversions(generatedConversions)
{
    sal_Int32 i = typeName.lastIndexOf('/');
    m_name = typeName.copy( i != -1 ? i+1 : 0 );
}

CorbaType::~CorbaType()
{

}

sal_Bool CorbaType::isNestedTypeByName(const ::rtl::OString& type)
{
    sal_Bool ret = sal_False;

    sal_Int32 i = type.lastIndexOf('/');

    if (i >= 0)
    {
        OString outerTypeName(type.copy(0, i));
        ret = (m_typeMgr.getTypeClass(outerTypeName) == RT_TYPE_INTERFACE);
    }

    return ret;
}

sal_Bool CorbaType::dump(CorbaOptions* pOptions, FileStream& o, TypeSet* allreadyDumped)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_False;

    ret = dumpConversionFunctions(o, allreadyDumped);

    return ret;
}

sal_Bool CorbaType::dumpDependedTypes(CorbaOptions* pOptions, FileStream& o, TypeSet* allreadyDumped)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_True;

    TypeUsingSet usingSet(m_dependencies.getDependencies(m_typeName));

    TypeUsingSet::const_iterator iter = usingSet.begin();
    OString typeName;
    sal_uInt32 index = 0;
    while (iter != usingSet.end())
    {
        typeName = (*iter).m_type;
        if ((index = typeName.lastIndexOf(']')) > 0)
            typeName = typeName.copy(index + 1);

        if (getUnoBaseType(typeName).getLength() == 0)
        {
            if (!produceType(typeName,
                m_typeMgr,
                m_dependencies,
                pOptions,
                o, allreadyDumped, m_generatedConversions))
            {
    fprintf(stderr, "%s ERROR: %s\n",
            pOptions->getProgramName().getStr(),
            OString("cannot dump Type '" + typeName + "'").getStr());
    exit(99);
            }
        }
        ++iter;
    }

    return ret;
}

sal_Bool CorbaType::dumpConversionFunctions(FileStream& o, TypeSet* allreadyDumped)
    throw( CannotDumpException )
{
    if (m_typeName.lastIndexOf(']') < 0)
    {
        dumpInclude(o, allreadyDumped, m_typeName, "hpp", sal_False);
        dumpDepIncludes(o, allreadyDumped, m_typeName, "hpp");

        dumpFunctions(o);
    }

    return sal_True;
}


void CorbaType::dumpDefaultHxxIncludes(FileStream& o)
{
    o << "#ifndef _OSL_MUTEX_HXX_\n"
      << "#include <osl/mutex.hxx>\n"
      << "#endif\n\n";

    o << "#ifndef _RTL_USTRING_HXX_\n"
      << "#include <rtl/ustring.hxx>\n"
      << "#endif\n\n";

    o << "#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_\n"
      << "#include <com/sun/star/uno/Type.hxx>\n"
      << "#endif\n";

    o << "#ifndef _COM_SUN_STAR_UNO_ANY_HXX_\n"
      << "#include <com/sun/star/uno/Any.hxx>\n"
      << "#endif\n";

    o << "#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_\n"
      << "#include <com/sun/star/uno/Reference.hxx>\n"
      << "#endif\n";

    o << "#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_\n"
      << "#include <com/sun/star/uno/XInterface.hpp>\n"
      << "#endif\n";

    o << "#ifndef _BONOBO_NULLINTERFACE_HPP_\n"
      << "#include <Bonobo/NullInterface.hpp>\n"
      << "#endif\n";

    o << "#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_\n"
      << "#include <com/sun/star/uno/Exception.hpp>\n"
      << "#endif\n";

    o << "#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_\n"
      << "#include <com/sun/star/uno/RuntimeException.hpp>\n"
      << "#endif\n";

    o << "#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_\n"
      << "#include <com/sun/star/uno/Sequence.hxx>\n"
      << "#endif\n";
}


void CorbaType::dumpInclude(FileStream& o, TypeSet* allreadyDumped, const OString& typeName, sal_Char* prefix, sal_Bool bExtended, sal_Bool bCaseSensitive)
{
    OString realTypeName = checkRealBaseType( typeName );

    if (!isNestedTypeByName(typeName) &&
        (BT_INVALID == isBaseType(realTypeName)) &&
        !realTypeName.equals("Bonobo/NullInterface") &&
        !realTypeName.equals("com/sun/star/uno/XInterface") &&
        !realTypeName.equals("com/sun/star/uno/TypeClass") &&
        !realTypeName.equals("com/sun/star/uno/Type") &&
        !realTypeName.equals("com/sun/star/uno/Exception") &&
        !realTypeName.equals("com/sun/star/uno/RuntimeException"))
    {
        TypeSet::const_iterator iter = allreadyDumped->find(realTypeName);

        if (iter == allreadyDumped->end())
        {
            allreadyDumped->insert(realTypeName);

            sal_uInt32 length = 3+ m_typeName.getLength() + strlen(prefix);

            if (bExtended)
                length += m_name.getLength() + 1;

            OStringBuffer tmpBuf(length);

            tmpBuf.append('_');
            tmpBuf.append(typeName);
            tmpBuf.append('_');
            if (bExtended)
            {
                tmpBuf.append(m_name);
                tmpBuf.append('_');
            }
            tmpBuf.append(prefix);
            tmpBuf.append('_');

            OString tmp(tmpBuf.makeStringAndClear().replace('/', '_').toAsciiUpperCase());

            length = 1 + typeName.getLength() + strlen(prefix);
            if (bExtended)
                length += m_name.getLength() + 1;

            tmpBuf.ensureCapacity(length);
            tmpBuf.append(typeName);
            if (bExtended)
            {
                tmpBuf.append('/');
                tmpBuf.append(m_name);
            }
            tmpBuf.append('.');
            tmpBuf.append(prefix);

            o << "#ifndef " << tmp << "\n#include <";
            if (bCaseSensitive)
            {
                o << tmpBuf.makeStringAndClear();
            } else
            {
                o << tmpBuf.makeStringAndClear();
            }

            o << ">\n";
            o << "#endif\n";

            o << "namespace bonobobridge {\n\n";

            std::list<OString> nestedTypes;

            do
            {
                if ((realTypeName.lastIndexOf(']') < 0) &&
                    (BT_INVALID == isBaseType(realTypeName)) &&
                    !realTypeName.equals("Bonobo/NullInterface") &&
                    !realTypeName.equals("com/sun/star/uno/XInterface") &&
                    !realTypeName.equals("com/sun/star/uno/TypeClass") &&
                    !realTypeName.equals("com/sun/star/uno/Type") &&
                    !realTypeName.equals("com/sun/star/uno/Exception") &&
                    !realTypeName.equals("com/sun/star/uno/RuntimeException") &&
                    !realTypeName.equals("com/sun/star/uno/TypeClass"))
                {
                    o << "inline sal_Bool cpp_convert_b2u(";
                    dumpUnoType(o, realTypeName, sal_False, sal_True);
                    o << " u, ";
                    dumpCorbaType(o, realTypeName, sal_True, sal_True);
                    o << " b, const ::vos::ORef< ::bonobobridge::Bridge >& bridge);\n";
                    o << "inline sal_Bool cpp_convert_u2b(";
                    dumpCorbaType(o, realTypeName, sal_False, sal_True);
                    o << " b, ";
                    dumpUnoType(o, realTypeName, sal_True, sal_True);
                    o << " u, const ::vos::ORef< ::bonobobridge::Bridge >& bridge);\n";
                }

                RegistryKey key = m_typeMgr.getTypeKey(realTypeName);
                RegistryKeyNames nestedTypeNames;
                key.getKeyNames(OUString(), nestedTypeNames);
                for (sal_uInt32 i = 0; i < nestedTypeNames.getLength(); i++)
                {
                    OString nTypeName(OUStringToOString(nestedTypeNames.getElement(i), RTL_TEXTENCODING_UTF8));

                    nTypeName = checkRealBaseType(nTypeName.copy(5));

                    if (BT_INVALID == isBaseType(nTypeName))
                    {
                        allreadyDumped->insert(nTypeName);
                        nestedTypes.push_back(nTypeName);
                    }
                }

                if (nestedTypes.size() > 0)
                {
                    realTypeName = nestedTypes.front();
                    nestedTypes.pop_front();
                }
                else
                {
                    realTypeName = "";
                }
            }
            while (realTypeName.getLength() > 0);

            o << "}; // namespace bonobobridge\n";
        }
    }
}

void CorbaType::dumpDepIncludes(FileStream& o, TypeSet* allreadyDumped, const OString& typeName, sal_Char* prefix)
{
    TypeUsingSet usingSet(m_dependencies.getDependencies(typeName));

    TypeUsingSet::const_iterator iter = usingSet.begin();

    OString     sPrefix(OString(prefix).toAsciiUpperCase());
    sal_Bool    bSequenceDumped = sal_False;
    sal_Bool    bInterfaceDumped = sal_False;
    sal_uInt32  index = 0;
    sal_uInt32  seqNum = 0;
    OString     relType;
    while (iter != usingSet.end())
    {
        index = (*iter).m_type.lastIndexOf(']');
        seqNum = (index > 0 ? ((index+1) / 2) : 0);

        relType = (*iter).m_type;
        if (index > 0)
            relType = relType.copy(index+1);

        if (!isNestedTypeByName(relType))
        {
            OString defPrefix("HXX");
            if (sPrefix.equals("HDL"))
                defPrefix = "H";

            if (seqNum > 0 && !bSequenceDumped)
            {
                bSequenceDumped = sal_True;
            }

            if (getUnoBaseType(relType).getLength() == 0 &&
                m_typeName != relType)
            {
                if (m_typeMgr.getTypeClass(relType) == RT_TYPE_INTERFACE
                    && sPrefix.equals("HDL"))
                {
                    if (!bInterfaceDumped)
                    {
                        bInterfaceDumped = sal_True;
                    }

                    if (!((*iter).m_use & TYPEUSE_SUPER))
                    {
                        o << "\n";
                        dumpNameSpace(o, sal_True, sal_False, relType);
                        o << "\nclass " << scopedName(m_typeName, relType, sal_True) << ";\n";
                        dumpNameSpace(o, sal_False, sal_False, relType);
                        o << "\n\n";
                    } else
                    {
                        dumpInclude(o, allreadyDumped, relType, prefix);
                    }
                } else
                {
                    dumpInclude(o, allreadyDumped, relType, prefix);
                }
            }
        }
        ++iter;
    }
}

void CorbaType::dumpNameSpace(FileStream& o, sal_Bool bOpen, sal_Bool bFull, const OString& type)
{
    OString typeName(type);
    sal_Bool bOneLine = sal_True;
    if (typeName.getLength() == 0)
    {
        typeName = m_typeName;
        bOneLine = sal_False;
    }

    if (typeName == "/")
        return;

    if (typeName.indexOf( '/' ) == -1 && !bFull)
        return;

    if (!bFull)
        typeName = typeName.copy( 0, typeName.lastIndexOf( '/' ) );

    if (bOpen)
    {
        sal_Int32 nIndex = 0;
        do
        {
            o << "namespace " << typeName.getToken(0, '/', nIndex);
            if (bOneLine)
                o << " { ";
            else
                 o << "\n{\n";
        } while( nIndex != -1 );
    } else
    {
        sal_Int32 nPos = 0;
        do
        {
            nPos = typeName.lastIndexOf( '/' );
            o << "}";
            if( bOneLine )
                o << " ";
            else
                o << " // " << typeName.copy( nPos+1 ) << "\n";
            if( nPos != -1 )
                typeName = typeName.copy( 0, nPos );
        } while( nPos != -1 );
    }
}


sal_uInt32 CorbaType::getMemberCount()
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

sal_uInt32 CorbaType::checkInheritedMemberCount(const TypeReader* pReader)
{
    sal_Bool bSelfCheck = sal_True;
    if (!pReader)
    {
        bSelfCheck = sal_False;
        pReader = &m_reader;
    }

    sal_uInt32 count = 0;
    OString superType(pReader->getSuperTypeName());
    if (superType.getLength() > 0)
    {
        TypeReader aSuperReader(m_typeMgr.getTypeReader(superType));
        if ( aSuperReader.isValid() )
        {
            count = checkInheritedMemberCount(&aSuperReader);
        }
    }

    if (bSelfCheck)
    {
        count += pReader->getMethodCount();
        sal_uInt32 fieldCount = pReader->getFieldCount();
        RTFieldAccess access = RT_ACCESS_INVALID;
        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = pReader->getFieldAccess(i);

            if (access != RT_ACCESS_CONST && access != RT_ACCESS_INVALID)
                count++;
        }
    }

    return count;
}

sal_uInt32 CorbaType::getInheritedMemberCount()
{
    if (m_inheritedMemberCount == 0)
    {
        m_inheritedMemberCount = checkInheritedMemberCount(0);
    }

    return m_inheritedMemberCount;
}

OString CorbaType::getTypeClass(const OString& type, sal_Bool bCStyle)
{
    OString     typeName = (type.getLength() > 0 ? type : m_typeName);
    RTTypeClass rtTypeClass = RT_TYPE_INVALID;

    if (type.getLength() > 0)
    {
        typeName = type;
        rtTypeClass = m_typeMgr.getTypeClass(typeName);
    } else
    {
        typeName = m_typeName;
        rtTypeClass = m_reader.getTypeClass();
    }

    if (typeName.lastIndexOf(']') > 0)
        return bCStyle ? "typelib_TypeClass_SEQUENCE" : "::com::sun::star::uno::TypeClass_SEQUENCE";

    switch (rtTypeClass)
    {
        case RT_TYPE_INTERFACE:
            return bCStyle ? "typelib_TypeClass_INTERFACE" : "::com::sun::star::uno::TypeClass_INTERFACE";
            break;
        case RT_TYPE_MODULE:
            return bCStyle ? "typelib_TypeClass_MODULE" : "::com::sun::star::uno::TypeClass_MODULE";
            break;
        case RT_TYPE_STRUCT:
            return bCStyle ? "typelib_TypeClass_STRUCT" : "::com::sun::star::uno::TypeClass_STRUCT";
            break;
        case RT_TYPE_ENUM:
            return bCStyle ? "typelib_TypeClass_ENUM" : "::com::sun::star::uno::TypeClass_ENUM";
            break;
        case RT_TYPE_EXCEPTION:
            return bCStyle ? "typelib_TypeClass_EXCEPTION" : "::com::sun::star::uno::TypeClass_EXCEPTION";
            break;
        case RT_TYPE_TYPEDEF:
            {
    OString realType = checkRealBaseType( typeName );
    return getTypeClass( realType, bCStyle );
            }
//          return bCStyle ? "typelib_TypeClass_TYPEDEF" : "::com::sun::star::uno::TypeClass_TYPEDEF";
            break;
        case RT_TYPE_SERVICE:
            return bCStyle ? "typelib_TypeClass_SERVICE" : "::com::sun::star::uno::TypeClass_SERVICE";
            break;
        case RT_TYPE_INVALID:
            {
                if (type.equals("long"))
                    return bCStyle ? "typelib_TypeClass_LONG" : "::com::sun::star::uno::TypeClass_LONG";
                if (type.equals("short"))
                    return bCStyle ? "typelib_TypeClass_SHORT" : "::com::sun::star::uno::TypeClass_SHORT";
                if (type.equals("hyper"))
                    return bCStyle ? "typelib_TypeClass_HYPER" : "::com::sun::star::uno::TypeClass_HYPER";
                if (type.equals("string"))
                    return bCStyle ? "typelib_TypeClass_STRING" : "::com::sun::star::uno::TypeClass_STRING";
                if (type.equals("boolean"))
                    return bCStyle ? "typelib_TypeClass_BOOLEAN" : "::com::sun::star::uno::TypeClass_BOOLEAN";
                if (type.equals("char"))
                    return bCStyle ? "typelib_TypeClass_CHAR" : "::com::sun::star::uno::TypeClass_CHAR";
                if (type.equals("byte"))
                    return bCStyle ? "typelib_TypeClass_BYTE" : "::com::sun::star::uno::TypeClass_BYTE";
                if (type.equals("any"))
                    return bCStyle ? "typelib_TypeClass_ANY" : "::com::sun::star::uno::TypeClass_ANY";
                if (type.equals("type"))
                    return bCStyle ? "typelib_TypeClass_TYPE" : "::com::sun::star::uno::TypeClass_TYPE";
                if (type.equals("float"))
                    return bCStyle ? "typelib_TypeClass_FLOAT" : "::com::sun::star::uno::TypeClass_FLOAT";
                if (type.equals("double"))
                    return bCStyle ? "typelib_TypeClass_DOUBLE" : "::com::sun::star::uno::TypeClass_DOUBLE";
                if (type.equals("void"))
                    return bCStyle ? "typelib_TypeClass_VOID" : "::com::sun::star::uno::TypeClass_VOID";
                if (type.equals("unsigned long"))
                    return bCStyle ? "typelib_TypeClass_UNSIGNED_LONG" : "::com::sun::star::uno::TypeClass_UNSIGNED_LONG";
                if (type.equals("unsigned short"))
                    return bCStyle ? "typelib_TypeClass_UNSIGNED_SHORT" : "::com::sun::star::uno::TypeClass_UNSIGNED_SHORT";
                if (type.equals("unsigned hyper"))
                    return bCStyle ? "typelib_TypeClass_UNSIGNED_HYPER" : "::com::sun::star::uno::TypeClass_UNSIGNED_HYPER";
            }
            break;
    }

    return bCStyle ? "typelib_TypeClass_UNKNOWN" : "::com::sun::star::uno::TypeClass_UNKNOWN";
}

OString CorbaType::printUnoType(const OString& type, sal_Bool bConst, sal_Bool bRef, sal_Bool bNative)
    throw( CannotDumpException )
{
    OStringBuffer ret(1024);
    OString sType(checkRealBaseType(type, sal_True));
    sal_uInt32 index = sType.lastIndexOf(']');
    sal_uInt32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    OString relType = (index > 0 ? (sType).copy(index+1) : type);

    RTTypeClass typeClass = m_typeMgr.getTypeClass(relType);

    if (bConst) ret.append("const ");

    sal_uInt32 i;
    for (i=0; i < seqNum; i++)
    {
        ret.append("::com::sun::star::uno::Sequence< ");
    }

    switch (typeClass)
    {
    case RT_TYPE_INTERFACE:
        if (bNative)
            ret.append(scopedName(m_typeName, relType));
        else
            ret.append("::com::sun::star::uno::Reference< ").append(scopedName(m_typeName, relType)).append(" >");
        break;
    case RT_TYPE_INVALID:
    {
        OString tmp(getUnoBaseType(relType));
        if (tmp.getLength() > 0)
        {
            ret.append(getUnoBaseType(relType));
        } else
            throw CannotDumpException("Unknown type '" + relType + "', incomplete type library. ("+type+")");
    }
    break;
    case RT_TYPE_STRUCT:
    case RT_TYPE_ENUM:
    case RT_TYPE_TYPEDEF:
    case RT_TYPE_EXCEPTION:
        ret.append(scopedName(m_typeName, relType));
        break;
    }

    for (i=0; i < seqNum; i++)
    {
        ret.append(" >");
    }

    if (bRef) ret.append("&");
    return ret.makeStringAndClear();
}

void CorbaType::dumpUnoType(FileStream& o, const OString& type,
            sal_Bool bConst, sal_Bool bRef, sal_Bool bNative)
    throw( CannotDumpException )
{
    OString ret = printUnoType(type, bConst, bRef, bNative);
    o << ret;
}

OString CorbaType::printCorbaType(const OString& type, sal_Bool bConst, sal_Bool bRef)
    throw( CannotDumpException )
{
    OStringBuffer ret(1024);

    OString sType(type);

    sal_uInt32 index = sType.lastIndexOf(']');
    sal_uInt32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    OString relType = (index > 0 ? (sType).copy(index+1) : type);

    RTTypeClass typeClass = m_typeMgr.getTypeClass(relType);

    if (relType.equals("com/sun/star/uno/XInterface"))
        relType = "Bonobo/Unknown";

    if (relType.equals("com/sun/star/uno/TypeClass"))
        relType = "CORBA_TypeCode";

    if (relType.equals("com/sun/star/uno/RuntimeException"))
        relType = "CORBA_SystemException";

    if (relType.equals("com/sun/star/uno/Exception"))
        relType = "CORBA_any";

    if (bConst) ret.append("const ");


    sal_uInt32 i;
    for (i=0; i < seqNum; i++)
    {
        ret.append("CORBA_sequence_");
    }

    switch (typeClass)
    {
    case RT_TYPE_INTERFACE:
        ret.append(relType.replace('/', '_'));
        break;
    case RT_TYPE_INVALID:
    {
        OString tmp(getUnoBaseType(relType));
        if (tmp.getLength() > 0)
            ret.append(getCorbaBaseType(relType));
        else
            throw CannotDumpException("Unknown type '" + relType + "', incomplete type library. ("+type+")");
    }
    break;
    case RT_TYPE_STRUCT:
    case RT_TYPE_ENUM:
    case RT_TYPE_TYPEDEF:
    case RT_TYPE_EXCEPTION:
        ret.append(relType.replace('/', '_'));
        break;
    }

    if (bRef) ret.append("&");

    return ret.makeStringAndClear();
}

sal_Bool CorbaType::isPassedAsPointer(const OString& type)
{
    sal_Bool ret = sal_False;

    OString sType(checkSpecialCorbaType(type));

    sal_Int32 index = sType.lastIndexOf(']');
    sal_Int32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    OString relType = (index > 0 ? (sType).copy(index+1) : type);

    if (index > 0)
    {
        OString fakeTest;

        sal_Int32 j = type.lastIndexOf('/');
        if (j >= 0)
            fakeTest = type.copy(0, j+1)+"_faked_array_"+type.copy(j+1);
        else
            fakeTest = "_faked_array_"+sType;

        TypeReader fakeTestReader = m_typeMgr.getTypeReader(fakeTest);

        if (fakeTestReader.isValid())
            ret = sal_False;
        else
            ret = sal_True;
    }
    else
    {
        RTTypeClass typeClass = m_typeMgr.getTypeClass(sType);

        switch (typeClass)
        {
        case RT_TYPE_STRUCT:
        case RT_TYPE_EXCEPTION:
            ret = sal_True;
            break;

        case RT_TYPE_INTERFACE:
        case RT_TYPE_ENUM:
        case RT_TYPE_INVALID:
            if (sType.equals("any"))
                ret = sal_True;
            else
                ret = sal_False;
            break;
        }
    }

    return ret;
}

sal_Bool CorbaType::isDerivedFromUnknown(const ::rtl::OString& typeName)
{
    sal_Bool ret = sal_True;
    if (typeName.getLength() == 0)
        ret = sal_False;
    else if (typeName.equals("Bonobo/NullInterface"))
        ret = sal_False;
    else if (typeName.equals("com/sun/star/uno/XInterface"))
        ret = sal_True;
    else
    {
        TypeReader reader(m_typeMgr.getTypeReader(typeName));
        if (reader.isValid())
            ret = isDerivedFromUnknown(reader.getSuperTypeName());
        else
            ret = sal_False;
    }
    return ret;
}


sal_Bool CorbaType::isArray(const OString& type)
{
    sal_Bool ret = sal_False;

    OString sType(checkSpecialCorbaType(type));

    sal_Int32 index = sType.lastIndexOf(']');
    sal_Int32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    OString relType = (index > 0 ? (sType).copy(index+1) : type);

    if (index > 0)
    {
        OString fakeTest;

        sal_Int32 j = type.lastIndexOf('/');
        if (j >= 0)
            fakeTest = type.copy(0, j+1)+"_faked_array_"+type.copy(j+1);
        else
            fakeTest = "_faked_array_"+sType;

        TypeReader fakeTestReader = m_typeMgr.getTypeReader(fakeTest);

        if (fakeTestReader.isValid())
            ret = sal_True;
    }

    return ret;
}

OString CorbaType::printCorbaParameter(const OString& type, sal_Bool bOut)
    throw( CannotDumpException )
{
    OStringBuffer ret(1024);

    OString sType(type);
    sal_Int32 index = sType.lastIndexOf(']');
    sal_Int32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    OString relType = (index > 0 ? (sType).copy(index+1) : type);

    RTTypeClass typeClass = m_typeMgr.getTypeClass(relType);

    if (relType.equals("Bonobo/NullInterface"))
        relType = "CORBA_Object";

    if (relType.equals("com/sun/star/uno/XInterface"))
        relType = "Bonobo/Unknown";

    if (relType.equals("com/sun/star/uno/TypeClass"))
        relType = "CORBA_TypeCode";

    if (relType.equals("com/sun/star/uno/RuntimeException"))
        relType = "CORBA_SystemException";

    if (relType.equals("com/sun/star/uno/Exception"))
        relType = "CORBA_any";

    int i;
    for (i=0; i < seqNum; i++)
    {
        ret.append("CORBA_sequence_");
    }

    switch (typeClass)
    {
    case RT_TYPE_INTERFACE:
        ret.append(relType.replace('/', '_'));
        break;
    case RT_TYPE_INVALID:
    {
        OString tmp(getUnoBaseType(relType));
        if (tmp.getLength() > 0)
        {
            ret.append(getCorbaBaseType(relType));
        } else
            throw CannotDumpException("Unknown type '" + relType + "', incomplete type library. ("+type+")");
    }
    break;
    case RT_TYPE_STRUCT:
    case RT_TYPE_EXCEPTION:
    case RT_TYPE_ENUM:
    case RT_TYPE_TYPEDEF:
        ret.append(relType.replace('/', '_'));
        break;
    }

    return ret.makeStringAndClear();
}

void CorbaType::dumpCorbaType(FileStream& o, const OString& type,
            sal_Bool bConst, sal_Bool bRef)
    throw( CannotDumpException )
{
    OString ret = printCorbaType(type, bConst, bRef);
    o << ret;
}

OString CorbaType::getUnoBaseType(const OString& type)
{
    if (type.equals("long"))
        return "sal_Int32";
    if (type.equals("short"))
        return "sal_Int16";
    if (type.equals("hyper"))
        return "sal_Int64";
    if (type.equals("string"))
        return "::rtl::OUString";
    if (type.equals("boolean"))
        return "sal_Bool";
    if (type.equals("char"))
        return "sal_Unicode";
    if (type.equals("byte"))
        return "sal_Int8";
    if (type.equals("any"))
        return "::com::sun::star::uno::Any";
    if (type.equals("type"))
        return "::com::sun::star::uno::Type";
    if (type.equals("float"))
        return "float";
    if (type.equals("double"))
        return "double";
    if (type.equals("octet"))
        return "sal_Int8";
    if (type.equals("void"))
        return type;
    if (type.equals("unsigned long"))
        return "sal_uInt32";
    if (type.equals("unsigned short"))
        return "sal_uInt16";
    if (type.equals("unsigned hyper"))
        return "sal_uInt64";

    return OString();
}

OString CorbaType::getCorbaBaseType(const OString& type)
{
    if (type.equals("long"))
        return "CORBA_long";
    if (type.equals("short"))
        return "CORBA_short";
    if (type.equals("hyper"))
        return "CORBA_long_long";
    if (type.equals("string"))
        return "CORBA_char*";
    if (type.equals("boolean"))
        return "CORBA_boolean";
    if (type.equals("char"))
        return "CORBA_char";
    if (type.equals("byte"))
        return "CORBA_octet";
    if (type.equals("any"))
        return "CORBA_any";
    if (type.equals("type"))
        return "CORBA_TypeCode";
    if (type.equals("float"))
        return "CORBA_float";
    if (type.equals("double"))
        return "CORBA_double";
    if (type.equals("octet"))
        return "CORBA_octet";
    if (type.equals("void"))
        return type;
    if (type.equals("unsigned long"))
        return "CORBA_unsigned_long";
    if (type.equals("unsigned short"))
        return "CORBA_unsigned_short";
    if (type.equals("unsigned hyper"))
        return "CORBA_unsigned_long_long";

    return OString();
}


void CorbaType::dumpTypeInit(FileStream& o, const OString& typeName)
{
    OString type(checkSpecialCorbaType(typeName));

    BASETYPE baseType = isBaseType(type);

    switch (baseType)
    {
        case BT_BOOLEAN:
            o << "(sal_False)";
            return;
            break;
        case BT_ANY:
        case BT_STRING:
            o << "()";
            return;
            break;
        case BT_INVALID:
            break;
        default:
            o << "((";
            dumpUnoType(o, type);
            o << ")" << "0)";
            return;
    }

    RTTypeClass typeClass = m_typeMgr.getTypeClass(type);

    if (typeClass == RT_TYPE_ENUM)
    {
        RegistryTypeReaderLoader aReaderLoader;

        if (aReaderLoader.isLoaded())
        {
            TypeReader reader(m_typeMgr.getTypeReader(type));

            if ( reader.isValid() )
            {
                sal_Int32 nPos = type.lastIndexOf( '/' );
                o << "(" << shortScopedName("", type, sal_False)
                  << "::" << type.copy( nPos != -1 ? nPos+1 : 0 )
                  << "_" << reader.getFieldName(0) << ")";
                return;
            }
        }
    }

    o << "()";
}

BASETYPE CorbaType::isBaseType(const OString& type)
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
    if (type.equals("unsigned long"))
        return BT_UNSIGNED_LONG;
    if (type.equals("unsigned short"))
        return BT_UNSIGNED_SHORT;
    if (type.equals("unsigned hyper"))
        return BT_UNSIGNED_HYPER;

    return BT_INVALID;
}

OString CorbaType::typeToIdentifier(const OString& type)
{
    sal_uInt32 index = type.lastIndexOf(']');
    sal_uInt32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    OString relType = (index > 0 ? ((OString)type).copy(index+1) : type);
    OString sIdentifier;

    while( seqNum > 0 )
    {
        sIdentifier += OString("seq");

        if ( --seqNum == 0 )
        {
            sIdentifier += OString("_");
        }
    }

    if ( isBaseType(relType) )
    {
        sIdentifier += relType.replace(' ', '_');
    } else
    {
        sIdentifier += relType.replace('/', '_');
    }


    return sIdentifier;
}

OString CorbaType::checkSpecialCorbaType(const OString& type)
{
    OString baseType(type);

    RegistryTypeReaderLoader & rReaderLoader = getRegistryTypeReaderLoader();

    RegistryKey     key;
    sal_uInt8*      pBuffer=NULL;
    RTTypeClass     typeClass;
    sal_Bool        isTypeDef = (m_typeMgr.getTypeClass(baseType) == RT_TYPE_TYPEDEF);
    TypeReader      reader;

    while (isTypeDef)
    {
        reader = m_typeMgr.getTypeReader(baseType);

        if (reader.isValid())
        {
            typeClass = reader.getTypeClass();

            if (typeClass == RT_TYPE_TYPEDEF)
                baseType = reader.getSuperTypeName();
            else
                isTypeDef = sal_False;
        } else
            break;
    }

    return baseType;
}

OString CorbaType::checkRealBaseType(const OString& type, sal_Bool bResolveTypeOnly)
{
    sal_uInt32 index = type.lastIndexOf(']');
    OString baseType = (index > 0 ? ((OString)type).copy(index+1) : type);
    OString seqPrefix = (index > 0 ? ((OString)type).copy(0, index+1) : OString());

    RegistryTypeReaderLoader & rReaderLoader = getRegistryTypeReaderLoader();

    RegistryKey     key;
    sal_uInt8*      pBuffer=NULL;
    RTTypeClass     typeClass;
    sal_Bool        mustBeChecked = (m_typeMgr.getTypeClass(baseType) == RT_TYPE_TYPEDEF);
    TypeReader      reader;

    while (mustBeChecked)
    {
        reader = m_typeMgr.getTypeReader(baseType);

        if (reader.isValid())
        {
            typeClass = reader.getTypeClass();

            if (typeClass == RT_TYPE_TYPEDEF)
            {
                baseType = reader.getSuperTypeName();
                index = baseType.lastIndexOf(']');
                  if (index > 0)
                {
                    seqPrefix += baseType.copy(0, index+1);
                    baseType = baseType.copy(index+1);
                }
            } else
                mustBeChecked = sal_False;
        } else
            break;
    }

    if ( bResolveTypeOnly )
        baseType = seqPrefix + baseType;

    return baseType;
}


void CorbaType::inc(sal_uInt32 num)
{
    m_indentLength += num;
}

void CorbaType::dec(sal_uInt32 num)
{
    if (m_indentLength - num < 0)
        m_indentLength = 0;
    else
        m_indentLength -= num;
}

OString CorbaType::indent()
{
    OStringBuffer tmp(m_indentLength);

    for (sal_uInt32 i=0; i < m_indentLength; i++)
    {
        tmp.append(' ');
    }
    return tmp.makeStringAndClear();
}

OString CorbaType::indent(sal_uInt32 num)
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
InterfaceType::InterfaceType(TypeReader& typeReader,
                                const OString& typeName,
                                const TypeManager& typeMgr,
                                const TypeDependency& typeDependencies,
                                TypeSet* generatedConversions)
    : CorbaType(typeReader, typeName, typeMgr, typeDependencies, generatedConversions)
{
    m_inheritedMemberCount = 0;
    m_hasAttributes = sal_False;
    m_hasMethods = sal_False;
}

InterfaceType::~InterfaceType()
{

}


void InterfaceType::dumpUnoMethods(FileStream& o, sal_Bool bDeclOnly, sal_Bool bDelegateToSuper)
{
    OString superName(m_reader.getSuperTypeName());
    if (bDeclOnly &&
        !superName.equals("Bonobo/NullInterface") &&
        !superName.equals("com/sun/star/uno/XInterface"))
    {
        TypeReader reader(m_typeMgr.getTypeReader(superName));
        InterfaceType iType(reader, superName, m_typeMgr, TypeDependency(), m_generatedConversions);
        iType.inc();
        iType.dumpUnoMethods(o, bDeclOnly, sal_True);
    }

    sal_uInt32 methodCount = m_reader.getMethodCount();
    sal_Bool first=sal_True;

    OString methodName, returnType, paramType, paramName;
    sal_uInt32 paramCount = 0;
    sal_uInt32 excCount = 0;
    RTMethodMode methodMode = RT_MODE_INVALID;
    RTParamMode  paramMode = RT_PARAM_INVALID;

    sal_Bool bRef = sal_False;
    sal_Bool bConst = sal_False;
    sal_Bool bWithRunTimeExcp = sal_True;

    for (sal_uInt16 i=0; i < methodCount; i++)
    {
        methodName = m_reader.getMethodName(i);
        returnType = m_reader.getMethodReturnType(i);
        paramCount = m_reader.getMethodParamCount(i);
        excCount = m_reader.getMethodExcCount(i);
        methodMode = m_reader.getMethodMode(i);

        if ( methodName.equals("acquire") || methodName.equals("release") )
            bWithRunTimeExcp = sal_False;

        if (first)
        {
            first = sal_False;
            o << "\n" << indent() << "// Methods\n";
        }

        o << indent();
        if (bDeclOnly)
            o << "virtual ";
        dumpUnoType(o, returnType);
        o << " SAL_CALL ";
        if (!bDeclOnly)
        {
            o << "bonobobridge::BonoboWrapper_";
            dumpCorbaType(o, m_typeName, sal_False, sal_False);
            o << "::";
        }

        o << methodName << "( ";

        sal_uInt16 j;
        for (j=0; j < paramCount; j++)
        {
            paramName = m_reader.getMethodParamName(i, j);
            paramType = m_reader.getMethodParamType(i, j);
            paramMode = m_reader.getMethodParamMode(i, j);

            switch (paramMode)
            {
            case RT_PARAM_IN:
            {
                OString relType = checkSpecialCorbaType(paramType);
                if (m_typeMgr.getTypeClass(relType) == RT_TYPE_ENUM ||
                    (isBaseType(relType) && !relType.equals("string") && !relType.equals("any")))
                {
                    bConst = sal_False;
                    bRef = sal_False;
                } else
                {
                    bConst = sal_True;
                    bRef = sal_True;
                }
                break;
            }
            case RT_PARAM_OUT:
            case RT_PARAM_INOUT:
                bConst = sal_False;
                bRef = sal_True;
                break;
            }

            dumpUnoType(o, paramType, bConst, bRef);
            o << " " << paramName;

            if (j+1 < paramCount) o << ", ";
        }
        o << "  )";

        o << "   throw(";
        OString excpName;
        for (j=0; j < excCount; j++)
        {
            excpName = m_reader.getMethodExcType(i, j);
            if (excpName != "com/sun/star/uno/RuntimeException")
                o << scopedName(m_typeName, excpName);
            if (bWithRunTimeExcp)
                o << ", ";
        }

        if ( bWithRunTimeExcp )
        {
            o << "  ::com::sun::star::uno::RuntimeException";
        }

        if (bDeclOnly && bDelegateToSuper)
        {
            o << " ) {\n";
            if (returnType.equals("void"))
                o << indent() << "  ";
            else
                o << indent() << "  return ";
            o << "BonoboWrapper_";
            dumpCorbaType(o, m_typeName, sal_False, sal_False);
            o << "::" << methodName << "( ";
            for (j = 0; j < paramCount; j++)
            {
                paramName = m_reader.getMethodParamName(i, j);
                o << paramName;
                if (j < (paramCount-1))
                    o << ", ";
            }
            o << " );\n"
                    << indent() << "}\n";
        }
        else if (!bDeclOnly)
        {
            o << " ) {\n";

            OStringBuffer preBuffer(1024);
            OStringBuffer callBuffer(1024);
            OStringBuffer postBuffer(1024);

            callBuffer.append("    ");

            if (!returnType.equals("void"))
            {
                preBuffer.append("    ");
                preBuffer.append(printCorbaParameter(returnType));
                if (isPassedAsPointer(returnType))
                    preBuffer.append("*");
                preBuffer.append(" _b_ret;\n");
                preBuffer.append("    ");
                preBuffer.append(printUnoType(returnType, sal_False, sal_False));
                preBuffer.append(" _u_ret;\n");

                callBuffer.append("_b_ret = ");
            }

            callBuffer.append(printCorbaType(m_typeName, sal_False, sal_False));
            callBuffer.append("_");
            if (methodName.indexOf("_reserved_identifier_") == 0)
                callBuffer.append(methodName.copy(OString("_reserved_identifier_").getLength()));
            else
                callBuffer.append(methodName);

            callBuffer.append("( m_corbaObject");

            for (j=0; j < paramCount; j++)
            {
                paramName = m_reader.getMethodParamName(i, j);
                paramType = m_reader.getMethodParamType(i, j);
                paramMode = m_reader.getMethodParamMode(i, j);

                preBuffer.append("    ");
                preBuffer.append(printCorbaParameter(paramType));
                if (isPassedAsPointer(paramType) && (paramMode == RT_PARAM_OUT))
                {
                    preBuffer.append("* _b_");
                    preBuffer.append(paramName);
                    preBuffer.append(";\n");
                }
                else
                {
                    preBuffer.append(" _b_");
                    preBuffer.append(paramName);
                    preBuffer.append(";\n");
                }

                switch (paramMode) {
                case RT_PARAM_IN:

                    if (isArray(paramType))
                        preBuffer.append("    // fix me: conversion of array types!\n");
                    else
                    {
                        preBuffer.append("    cpp_convert_u2b(");
                        preBuffer.append("_b_");
                        preBuffer.append(paramName);
                        preBuffer.append(", ");
                        preBuffer.append(paramName);
                        preBuffer.append(", m_bridge);\n");
                    }

                    if (isPassedAsPointer(paramType))
                        callBuffer.append(", &_b_");
                    else
                        callBuffer.append(", _b_");
                    break;
                case RT_PARAM_INOUT:

                    if (isArray(paramType))
                        preBuffer.append("    // fix me: conversion of array types!\n");
                    else
                    {
                        preBuffer.append("    cpp_convert_u2b(");
                        if (isPassedAsPointer(paramType))
                            preBuffer.append("_b_");
                        else
                            preBuffer.append("_b_");
                        preBuffer.append(paramName);
                        preBuffer.append(", ");
                        preBuffer.append(paramName);
                        preBuffer.append(", m_bridge);\n");
                    }

                    callBuffer.append(", &_b_");

                    if (isArray(paramType))
                        postBuffer.append("    // fix me: conversion of array types!\n");
                    else
                    {
                        postBuffer.append("    cpp_convert_b2u(");
                        postBuffer.append(paramName);
                        postBuffer.append(", _b_");
                        postBuffer.append(paramName);
                        postBuffer.append(", m_bridge);\n");
                    }

                    break;
                case RT_PARAM_OUT:

                    callBuffer.append(", &_b_");

                    if (isArray(paramType))
                        postBuffer.append("    // fix me: conversion of array types!\n");
                    else
                    {
                        postBuffer.append("    cpp_convert_b2u(");
                        postBuffer.append(paramName);
                        if (isPassedAsPointer(paramType))
                            postBuffer.append(", *_b_");
                        else
                            postBuffer.append(", _b_");
                        postBuffer.append(paramName);
                        postBuffer.append(", m_bridge);\n");
                    }

                    break;
                }


                callBuffer.append(paramName);
            }

            callBuffer.append(", &_ev );\n");

            if (!returnType.equals("void"))
            {
                if (isArray(returnType))
                {
                    postBuffer.append("    // fix me: conversion of array types!\n");
                }
                else
                {
                    if (isPassedAsPointer(returnType))
                        postBuffer.append("    cpp_convert_b2u(_u_ret, *_b_ret, m_bridge);\n");
                    else
                        postBuffer.append("    cpp_convert_b2u(_u_ret, _b_ret, m_bridge);\n");
                }
                postBuffer.append("    return _u_ret;\n");
            }

            o << "    ::osl::MutexGuard guard(m_bridge->getORBLock());\n"
                    << "    CORBA_Environment _ev;\n"
                    << "    CORBA_exception_init (&_ev);\n";

            o << preBuffer;
            o << callBuffer;

            o << "    if (_ev._major != CORBA_NO_EXCEPTION) {\n"
              << "        ::com::sun::star::uno::RuntimeException _ex(::rtl::OUString::createFromAscii(\"exception raised in bonobobridge\"), NULL);\n"
              << "        CORBA_exception_free (&_ev);\n"
              << "        throw _ex;\n"
              << "    }\n"
              << "    CORBA_exception_free (&_ev);\n";

            o << postBuffer;

            o << indent() << "}\n";
        }
        else
            o << "  );\n";
    }
}

void InterfaceType::dumpCorbaMethods(FileStream& o, sal_Bool bDeclOnly)
{
    OString superName(m_reader.getSuperTypeName());

    sal_uInt32 methodCount = m_reader.getMethodCount();

    OString methodName, returnType, paramType, paramName;
    sal_uInt32 paramCount = 0;
    sal_uInt32 excCount = 0;
    RTMethodMode methodMode = RT_MODE_INVALID;
    RTParamMode  paramMode = RT_PARAM_INVALID;

    sal_Bool bRef = sal_False;
    sal_Bool bConst = sal_False;
    sal_Bool bWithRunTimeExcp = sal_True;

    for (sal_uInt16 i=0; i < methodCount; i++)
    {
        methodName = m_reader.getMethodName(i);
        returnType = m_reader.getMethodReturnType(i);
        paramCount = m_reader.getMethodParamCount(i);
        excCount = m_reader.getMethodExcCount(i);
        methodMode = m_reader.getMethodMode(i);

        o << indent()
          << "extern \"C\" "
          << printCorbaParameter(returnType);

        if (isPassedAsPointer(returnType))
            o << "*";

        o << " bonobobridge_";
        dumpCorbaType(o, m_typeName, sal_False, sal_False);
        o << "_" << methodName << "( PortableServer_Servant _servant, ";

        sal_uInt16 j;
        for (j=0; j < paramCount; j++)
        {
            paramName = m_reader.getMethodParamName(i, j);
            paramType = m_reader.getMethodParamType(i, j);
            paramMode = m_reader.getMethodParamMode(i, j);

            if ((isPassedAsPointer(paramType) || paramType.equals("string") || isArray(paramType)) &&
                (paramMode == RT_PARAM_IN))
                o << "const ";

            o << printCorbaParameter(paramType);

            if (isPassedAsPointer(paramType))
            {
                if (paramMode == RT_PARAM_OUT)
                    o << "**";
                else
                    o << "*";
            }
            else
            {
                if (paramMode != RT_PARAM_IN)
                    o << "*";
            }


            o << " " << paramName << ", ";
        }

        o << "CORBA_Environment * _ev)";

        if (bDeclOnly)
            o << ";\n";
        else
        {
            o << " {\n";
            OStringBuffer preBuffer(1024);
            OStringBuffer callBuffer(1024);
            OStringBuffer postBuffer(1024);

            callBuffer.append("    ");

            preBuffer.append("    ");
            preBuffer.append(printUnoType(m_typeName, sal_False, sal_False));
            preBuffer.append(" rThis((");
            preBuffer.append(printUnoType(m_typeName, sal_False, sal_False, sal_True));
            preBuffer.append(" *)((bonobobridge::UNO_POA_com_sun_star_uno_XInterface*)_servant)->pThis->getUnoObject());\n");

            if (!returnType.equals("void"))
            {
                preBuffer.append("    ");
                preBuffer.append(printCorbaParameter(returnType));
                if (isPassedAsPointer(returnType))
                {
                    preBuffer.append("* _b_ret = ");
                    preBuffer.append(printCorbaType(returnType, sal_False, sal_False));
                    preBuffer.append("__alloc();\n");

                }
                else
                {
                    preBuffer.append(" _b_ret;\n");
                }
                preBuffer.append("    ");
                preBuffer.append(printUnoType(returnType, sal_False, sal_False));
                preBuffer.append(" _u_ret;\n");

                callBuffer.append("_u_ret = ");
            }

            callBuffer.append("rThis->");
            callBuffer.append(methodName);
            callBuffer.append("( ");

            for (j=0; j < paramCount; j++)
            {
                paramName = m_reader.getMethodParamName(i, j);
                paramType = m_reader.getMethodParamType(i, j);
                paramMode = m_reader.getMethodParamMode(i, j);

                preBuffer.append("    ");
                preBuffer.append(printUnoType(paramType, sal_False, sal_False));
                preBuffer.append(" _u_");
                preBuffer.append(paramName);
                preBuffer.append(";\n");

                callBuffer.append("_u_");
                callBuffer.append(paramName);

                if (j < (paramCount-1))
                    callBuffer.append(", ");

                switch (paramMode) {
                case RT_PARAM_IN:

                    if (isArray(paramType))
                        preBuffer.append("    // fix me: conversion of array types!\n");
                    else
                    {
                        preBuffer.append("    cpp_convert_b2u(");
                        preBuffer.append("_u_");
                        preBuffer.append(paramName);
                        preBuffer.append(", ");
                        if (isPassedAsPointer(paramType))
                            preBuffer.append("*");
                        preBuffer.append(paramName);
                        preBuffer.append(", ((bonobobridge::UNO_POA_com_sun_star_uno_XInterface*)_servant)->pThis->getBridge());\n");
                    }
                    break;
                case RT_PARAM_INOUT:

                    if (isArray(paramType))
                    {
                        preBuffer.append("    // fix me: conversion of array types!\n");
                        postBuffer.append("    // fix me: conversion of array types!\n");
                    }
                    else
                    {
                        preBuffer.append("    cpp_convert_b2u(");
                        preBuffer.append("_u_");
                        preBuffer.append(paramName);
                        preBuffer.append(", ");
                        if (isPassedAsPointer(paramType))
                            preBuffer.append("*");
                        preBuffer.append(paramName);
                        preBuffer.append(", ((bonobobridge::UNO_POA_com_sun_star_uno_XInterface*)_servant)->pThis->getBridge());\n");

                        postBuffer.append("    cpp_convert_u2b(");
                        if (isPassedAsPointer(paramType))
                            postBuffer.append("*");
                        postBuffer.append(paramName);
                        postBuffer.append(", _u_");
                        postBuffer.append(paramName);
                        postBuffer.append(", ((bonobobridge::UNO_POA_com_sun_star_uno_XInterface*)_servant)->pThis->getBridge());\n");
                    }
                    break;
                case RT_PARAM_OUT:

                    if (isArray(paramType))
                        preBuffer.append("    // fix me: conversion of array types!\n");
                    else
                    {
                        postBuffer.append("    cpp_convert_u2b(");
                        if (isPassedAsPointer(paramType))
                            postBuffer.append("**");
                        else
                            postBuffer.append("*");
                        postBuffer.append(paramName);
                        postBuffer.append(", _u_");
                        postBuffer.append(paramName);
                        postBuffer.append(", ((bonobobridge::UNO_POA_com_sun_star_uno_XInterface*)_servant)->pThis->getBridge());\n");
                    }
                    break;
                }
            }

            callBuffer.append(" );\n");

            if (!returnType.equals("void"))
            {
                if (isArray(returnType))
                    postBuffer.append("    // fix me: conversion of array types!\n");
                else
                {
                    if (isPassedAsPointer(returnType))
                        postBuffer.append("    cpp_convert_u2b(*_b_ret, _u_ret, ((bonobobridge::UNO_POA_com_sun_star_uno_XInterface*)_servant)->pThis->getBridge());\n");
                    else
                        postBuffer.append("    cpp_convert_u2b(_b_ret, _u_ret, ((bonobobridge::UNO_POA_com_sun_star_uno_XInterface*)_servant)->pThis->getBridge());\n");
                }

                postBuffer.append("    return _b_ret;\n");
            }

            o << preBuffer;
            o << callBuffer;
            o << postBuffer;

            o << "}\n";
        }
    }
}

void InterfaceType::dumpFunctions(FileStream& o)
{
    if (m_typeName.equals("com/sun/star/uno/XInterface") ||
        m_typeName.equals("Bonobo/NullInterface"))
        return;

    m_generatedConversions->insert(m_typeName);

    o << "namespace bonobobridge {\n\n";

    /* bonobo implementation class */
    o << "class BonoboWrapper_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);

    OString superName(m_reader.getSuperTypeName());
    o << " : public BonoboWrapper< BonoboWrapper_";
    dumpCorbaType(o, superName, sal_False, sal_False);
    o << ", ";
    dumpUnoType(o, m_typeName, sal_False, sal_False, sal_True);
    o << " > {\n";

    o << "public:   \n"
      << "    BonoboWrapper_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "(";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << " corbaObject,"
      << "const vos::ORef<bonobobridge::Bridge>& bridge)\n";

    o << "      : BonoboWrapper< "
      << "BonoboWrapper_";
    dumpCorbaType(o, superName, sal_False, sal_False);
    o << ", ";
    dumpUnoType(o, m_typeName, sal_False, sal_False, sal_True);
    o << " >(corbaObject, bridge) {\n";

    if (isDerivedFromUnknown(m_typeName))
    {
        o << "      m_bridge->registerObjectWrapper(::getCppuType((";
        dumpUnoType(o, m_typeName, sal_False, sal_False);
        o << "*)NULL), m_corbaObject, (";
        dumpUnoType(o, m_typeName, sal_False, sal_False, sal_True);
        o << "*)this);\n";
    }

    o << "    }\n";

    o << "    virtual ~BonoboWrapper_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "() {\n";
    if (isDerivedFromUnknown(m_typeName))
    {
        o << "      m_bridge->unregisterObjectWrapper(::getCppuType((";
        dumpUnoType(o, m_typeName, sal_False, sal_False);
        o << "*)NULL), m_corbaObject, (";
        dumpUnoType(o, m_typeName, sal_False, sal_False, sal_True);
        o << "*)this);\n";
    }
    o   << "    }\n";
    inc();
    dumpUnoMethods(o, sal_True, sal_False);
    dec();

    o  << "};\n\n";

    o << "}; // namespace bonobobridge\n";

    dumpUnoMethods(o, sal_False, sal_False);

    /* convert function bonobo to uno */
    o << "static sal_Bool convert_b2u_" << m_typeName.replace('/', '_')
      << "(void* pOut, const void* pIn, const ::com::sun::star::uno::Type& type, const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  sal_Bool ret = sal_True;\n  ";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "& _u = *(";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "*) pOut;\n  const ";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "& _b = *(const ";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "*) pIn;\n\n"
      << "  _u = new bonobobridge::BonoboWrapper_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "(_b, bridge);\n"
      << "  return ret;\n";

    o   << "}\n\n";

    /* POA implementation class */
    dumpCorbaMethods(o, sal_True);
    o << "static POA_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "__epv bonobobridge_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "_epv = {\n";
    sal_uInt32      methodCount = m_reader.getMethodCount();
    OString         methodName;

    o << "    NULL,\n";

    for (sal_uInt16 i=0; i < methodCount; i++)
    {
        methodName = m_reader.getMethodName(i);
        o << "    bonobobridge_";
        dumpCorbaType(o, m_typeName, sal_False, sal_False);
        o << "_" << methodName;

        if (i < (methodCount-1))
            o << ",\n";
        else
            o << "\n};\n";
    }

    OStringBuffer initBuffer(1024);

    initBuffer.insert(0, OString("&bonobobridge_") + printCorbaType(m_typeName, sal_False, sal_False) + OString("_epv"));

    while(superName.getLength() != 0)
    {
        if (superName.equals("Bonobo/NullInterface"))
        {
            superName = "";
        }
        else
        {
            if (superName.equals("com/sun/star/uno/XInterface"))
            {
                initBuffer.insert(0, OString("&bonobobridge_com_sun_star_uno_XInterface_epv, "));
            }
            else
            {
                initBuffer.insert(0, OString("&bonobobridge_") + printCorbaType(superName, sal_False, sal_False) + OString("_epv, "));
            }
            TypeReader reader(m_typeMgr.getTypeReader(superName));
            superName = reader.getSuperTypeName();
        }
    }

    initBuffer.insert(0, OString("NULL, "));

    o << "static POA_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "__vepv bonobobridge_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "__vepv = {";
    o << initBuffer.makeStringAndClear();
    o << " };\n";

    superName = m_reader.getSuperTypeName();

    o << "namespace bonobobridge {\n\n";

    o << "class UnoServant_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << " : public UnoServant_com_sun_star_uno_XInterface {\n";

    o << "public:\n"
            << " UnoServant_";

    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "(";
    dumpUnoType(o, m_typeName, sal_False, sal_False, sal_True);
    o << "* unoObject,"
      << " const ::vos::ORef<bonobobridge::Bridge>& bridge,"
      << " CORBA_Environment *ev,"
      << " sal_Bool bInitPoa)\n"
      << "    : UnoServant_com_sun_star_uno_XInterface(unoObject, bridge, ev, sal_False) {\n"
      << "    if (bInitPoa) {\n"
      << "        memset(&m_POAUnknown, 0, sizeof(m_POAUnknown));\n"
      << "        POA_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "__init((PortableServer_Servant) &(m_POAUnknown.poa), ev);\n"
      << "        m_POAUnknown.pThis = (UnoServant_com_sun_star_uno_XInterface*)this;\n"
      << "        m_POAUnknown.poa.vepv = (POA_Bonobo_Unknown__vepv*)&bonobobridge_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "__vepv;\n"
      << "    }\n"
      << "  }\n"
      << "};\n"
      << "}; // namespace bonobobridge\n";

    dumpCorbaMethods(o, sal_False);

    /* convert function uno to bonobo */
    o << "static sal_Bool convert_u2b_" << m_typeName.replace('/', '_')
            << "(void* pOut, const void* pIn,   const ::com::sun::star::uno::Type& type,    const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n";
    o   << "  sal_Bool ret = sal_True;\n  const ";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "& _u = *(const ";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "*) pIn;\n  ";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "& _b = *(";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "*) pOut;\n\n"
      << "  if (_u.is()) {\n"
      << "    bonobobridge::UnoServant_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << " *uno_servant;\n"
      << "    POA_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << " *poa;\n"
      << "          CORBA_Environment ev;\n"
      << "    CORBA_exception_init (&ev);\n"
      << "          uno_servant = new bonobobridge::UnoServant_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "(_u.get(), bridge, &ev, sal_True);\n"
      << "    poa = (POA_";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "*)uno_servant->getPOA();\n"
      << "    if (ev._major != CORBA_NO_EXCEPTION) {\n"
      << "      delete uno_servant;\n"
      << "          CORBA_exception_free (&ev);\n"
      << "          _b = CORBA_OBJECT_NIL;\n"
      << "          ret = sal_False;\n"
      << "    }\n"
      << "  else {\n"
      << "      CORBA_free(PortableServer_POA_activate_object(bridge->getPOA(), poa, &ev));\n"
      << "          _b = PortableServer_POA_servant_to_reference (bridge->getPOA(), poa, &ev);\n"
      << "          uno_servant->corbaObjectRegistered(_b, getCppuType((";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "*)NULL)";

    if (!isDerivedFromUnknown(m_typeName))
        o << ", sal_False";

    o << ");\n"
      << "    }\n"
      << "  CORBA_exception_free (&ev);\n"
      << "  }\n"
      << "  return ret;\n"
      << "}\n\n";

    o << "inline sal_Bool bonobobridge::cpp_convert_b2u(";
    dumpUnoType(o, m_typeName, sal_False, sal_True);
    o << " u, ";
    dumpCorbaType(o, m_typeName, sal_True, sal_True);
    o << " b, const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  return bridge->convertB2U(&u, &b, ::getCppuType(&u));\n"
      << "};\n\n";

    o << "inline sal_Bool bonobobridge::cpp_convert_u2b(";
    dumpCorbaType(o, m_typeName, sal_False, sal_True);
    o << " b, ";
    dumpUnoType(o, m_typeName, sal_True, sal_True);
    o << " u, const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  return bridge->convertU2B(&b, &u, ::getCppuType(&u));\n"
      << "};\n\n";

    return;
}




sal_uInt32 InterfaceType::getMemberCount()
{
    sal_uInt32 count = m_reader.getMethodCount();

    if (count)
        m_hasMethods = sal_True;

    sal_uInt32 fieldCount = m_reader.getFieldCount();
    RTFieldAccess access = RT_ACCESS_INVALID;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access != RT_ACCESS_CONST && access != RT_ACCESS_INVALID)
        {
            m_hasAttributes = sal_True;
            count++;
        }
    }
    return count;
}

sal_uInt32 InterfaceType::checkInheritedMemberCount(const TypeReader* pReader)
{
    sal_uInt32 cout = 0;
    sal_Bool bSelfCheck = sal_True;
    if (!pReader)
    {
        bSelfCheck = sal_False;
        pReader = &m_reader;
    }

    sal_uInt32 count = 0;
    OString superType(pReader->getSuperTypeName());
    if (superType.getLength() > 0)
    {
        TypeReader aSuperReader(m_typeMgr.getTypeReader(superType));
        if (aSuperReader.isValid())
        {
            count = checkInheritedMemberCount(&aSuperReader);
        }
    }

    if (bSelfCheck)
    {
        count += pReader->getMethodCount();
        sal_uInt32 fieldCount = pReader->getFieldCount();
        RTFieldAccess access = RT_ACCESS_INVALID;
        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = pReader->getFieldAccess(i);

            if (access != RT_ACCESS_CONST && access != RT_ACCESS_INVALID)
                count++;
        }
    }

    return count;
}

sal_uInt32 InterfaceType::getInheritedMemberCount()
{
    if (m_inheritedMemberCount == 0)
    {
        m_inheritedMemberCount = checkInheritedMemberCount(0);
    }

    return m_inheritedMemberCount;
}




//*************************************************************************
// ModuleType
//*************************************************************************
ModuleType::ModuleType(TypeReader& typeReader,
                                const OString& typeName,
                                const TypeManager& typeMgr,
                                const TypeDependency& typeDependencies,
                                TypeSet* generatedConversions)
    : CorbaType(typeReader, typeName, typeMgr, typeDependencies, generatedConversions)
{
}

ModuleType::~ModuleType()
{

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

void ModuleType::dumpFunctions(FileStream& o)
{
};

sal_Bool ModuleType::dumpConversionFunctions(FileStream& o, TypeSet* allreadyDumped)
    throw( CannotDumpException )
{
    sal_Bool bSpecialDefine = sal_True;

    if (m_reader.getTypeClass() == RT_TYPE_CONSTANTS)
    {
        bSpecialDefine = sal_False;
    }

    dumpInclude(o, allreadyDumped, m_typeName, "hpp", bSpecialDefine);

    return sal_True;
}

//*************************************************************************
// ConstantsType
//*************************************************************************
ConstantsType::ConstantsType(TypeReader& typeReader,
                                const OString& typeName,
                                const TypeManager& typeMgr,
                                const TypeDependency& typeDependencies,
                                TypeSet* generatedConversions)
    : ModuleType(typeReader, typeName, typeMgr, typeDependencies, generatedConversions)
{
}

ConstantsType::~ConstantsType()
{

}

void ConstantsType::dumpFunctions(FileStream& o)
{
};

//*************************************************************************
// StructureType
//*************************************************************************
StructureType::StructureType(TypeReader& typeReader,
                                const OString& typeName,
                                const TypeManager& typeMgr,
                                const TypeDependency& typeDependencies,
                                TypeSet* generatedConversions)
    : CorbaType(typeReader, typeName, typeMgr, typeDependencies, generatedConversions)
{
}

StructureType::~StructureType()
{

}

void StructureType::dumpFunctions(FileStream& o)
{
    m_generatedConversions->insert(m_typeName);

    OString superType(m_reader.getSuperTypeName());

    o << "static sal_Bool convert_b2u_" << m_typeName.replace('/', '_')
      << "(void* pOut, const void* pIn, const ::com::sun::star::uno::Type& type, const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  sal_Bool ret = sal_True;\n  ";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "& _u = *(";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "*) pOut;\n  const ";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "& _b = *(const ";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "*) pIn;\n\n";

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    OString         fieldName;
    OString         fieldType;
    sal_uInt16      i=0;
    sal_Int32       cIndex;
    OString         corbaFieldName;

    sal_Bool bIsUnion = sal_False;

    for (i=0; !bIsUnion && (i < fieldCount); i++)
        bIsUnion = OString("_union_fake_tag").equals(m_reader.getFieldName(i));

    if (bIsUnion)
    {
        o << "  // fix me: union !!!!\n  ret = sal_False;\n";
    }
    else
    {
        if (superType.getLength() > 0)
        {
            o << "  ret = bonobobridge::cpp_convert_b2u((";
            dumpUnoType(o, superType, sal_False, sal_False);
            o << "&) _u, (const ";
            dumpCorbaType(o, superType, sal_False, sal_False);
            o << "&) _b, bridge);\n";
        }

        for (i=0; i < fieldCount; i++)
        {
            fieldName = m_reader.getFieldName(i);
            fieldType = m_reader.getFieldType(i);
            cIndex = fieldName.indexOf("_reserved_identifier_");

            if (cIndex == 0)
                corbaFieldName = fieldName.copy(OString("_reserved_identifier_").getLength());
            else
                corbaFieldName = fieldName;

            if (isArray(fieldType))
                o << "  // fix me: no conversion of array types!\n";
            else
                o << "  if (ret)\n"
                  << "    ret = bonobobridge::cpp_convert_b2u("
                  << "_u." << fieldName.getStr()
                  << "  , _b." << corbaFieldName.getStr()
                  << ", bridge);\n";
        }
    }
    o << "  return ret;\n"
      << "}\n\n"
      << "static sal_Bool convert_u2b_" << m_typeName.replace('/', '_')
      << "(void* pOut, const void* pIn, const ::com::sun::star::uno::Type& type, const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  sal_Bool ret = sal_True;\n  const ";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "& _u = *(const ";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "*) pIn;\n  ";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "& _b = *(";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "*) pOut;\n\n";

    if (bIsUnion)
        o << "  // fix me: union !!!!\n  ret = sal_False;\n";
    else
    {
        if (superType.getLength() > 0)
        {
            o << "  ret = bonobobridge::cpp_convert_u2b((";
            dumpCorbaType(o, superType, sal_False, sal_False);
            o << "&) _u, (const ";
            dumpUnoType(o, superType, sal_False, sal_False);
            o << "&) _b, bridge);\n";
        }

        for (i=0; i < fieldCount; i++)
        {
            fieldName = m_reader.getFieldName(i);
            fieldType = m_reader.getFieldType(i);

            cIndex = fieldName.indexOf("_reserved_identifier_");

            if (cIndex == 0)
                corbaFieldName = fieldName.copy(OString("_reserved_identifier_").getLength());
            else
                corbaFieldName = fieldName;

            if (isArray(fieldType))
                o << "  // fix me: no conversion of array types!\n";
            else
                o << "  if (ret)\n"
                  << "    ret = bonobobridge::cpp_convert_u2b("
                  << "_b." << corbaFieldName.getStr()
                  << ", _u." << fieldName.getStr()
                  << ", bridge);\n";
        }
    }

    o   << "  return ret;\n"
        << "}\n\n";

    o << "inline sal_Bool bonobobridge::cpp_convert_b2u(";
    dumpUnoType(o, m_typeName, sal_False, sal_True);
    o << " u    , ";
    dumpCorbaType(o, m_typeName, sal_True, sal_True);
    o << " b,   const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  return convert_b2u_" << m_typeName.replace('/', '_')
      << "(&u, &b, ::getCppuType(&u), bridge);\n"
      << "};\n\n";

    o << "inline sal_Bool bonobobridge::cpp_convert_u2b(";
    dumpCorbaType(o, m_typeName, sal_False, sal_True);
    o << " b, ";
    dumpUnoType(o, m_typeName, sal_True, sal_True);
    o << " u,   const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  return convert_u2b_" << m_typeName.replace('/', '_')
      << "(&b, &u, ::getCppuType(&u), bridge);\n"
      << "};\n\n";
}

sal_Bool StructureType::dumpSuperMember(FileStream& o, const OString& superType, sal_Bool bWithType)
{
    sal_Bool hasMember = sal_False;

    if (superType.getLength() > 0)
    {
        TypeReader aSuperReader(m_typeMgr.getTypeReader(superType));

        if (aSuperReader.isValid())
        {
            hasMember = dumpSuperMember(o, aSuperReader.getSuperTypeName(), bWithType);

            sal_uInt32      fieldCount = aSuperReader.getFieldCount();
            RTFieldAccess   access = RT_ACCESS_INVALID;
            OString         fieldName;
            OString         fieldType;
            for (sal_uInt16 i=0; i < fieldCount; i++)
            {
                access = aSuperReader.getFieldAccess(i);

                if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                    continue;

                fieldName = aSuperReader.getFieldName(i);
                fieldType = aSuperReader.getFieldType(i);

                if (hasMember)
                {
                    o << ", ";
                } else
                {
                    hasMember = (fieldCount > 0);
                }

                if (bWithType)
                {
                    dumpUnoType(o, fieldType, sal_True, sal_True);
                    o << " ";
                }
                o << "__" << fieldName;
            }
        }
    }

    return hasMember;
}

//*************************************************************************
// ExceptionType
//*************************************************************************
ExceptionType::ExceptionType(TypeReader& typeReader,
                                const OString& typeName,
                                const TypeManager& typeMgr,
                                const TypeDependency& typeDependencies,
                                TypeSet* generatedConversions)
    : CorbaType(typeReader, typeName, typeMgr, typeDependencies, generatedConversions)
{
}

ExceptionType::~ExceptionType()
{

}

void ExceptionType::dumpFunctions(FileStream& o)
{
    if (m_typeName.equals("com/sun/star/uno/Exception"))
        return;

    if (m_typeName.equals("com/sun/star/uno/RuntimeException"))
        return;

    m_generatedConversions->insert(m_typeName);

    OString superType(m_reader.getSuperTypeName());

    o << "static sal_Bool convert_b2u_" << m_typeName.replace('/', '_')
      << "(void* pOut, const void* pIn, const ::com::sun::star::uno::Type& type, const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  sal_Bool ret = sal_True;\n";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "& _u = *(";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "*) pOut;\n  const ";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "& _b = *(const ";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "*) pIn;\n\n";

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    OString         fieldName;
    OString         fieldType;
    sal_uInt16      i=0;
    sal_Int32    cIndex;
    OString      corbaFieldName;

    if (superType.getLength() > 0)
    {
        o << "  ret = bonobobridge::cpp_convert_b2u((";
        dumpUnoType(o, superType, sal_False, sal_False);
        o << "&) _u, (const ";
        dumpCorbaType(o, superType, sal_False, sal_False);
        o << "&) _b, bridge);\n";
    }

    for (i=0; i < fieldCount; i++)
    {
        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);
        cIndex = fieldName.indexOf("_reserved_identifier_");

        if (cIndex == 0)
            corbaFieldName = fieldName.copy(OString("_reserved_identifier_").getLength());
        else
            corbaFieldName = fieldName;

        if (isArray(fieldType))
            o << "  // fix me: no conversion of array types!\n";
        else
            o << "  if (ret)\n"
              << "    ret = bonobobridge::cpp_convert_b2u("
              << "_u." << fieldName.getStr()
              << ", _b." << corbaFieldName.getStr()
              << ", bridge);\n";
    }
    o << "  return ret;\n"
      << "}\n\n";


    o << "static sal_Bool convert_u2b_" << m_typeName.replace('/', '_')
      << "(void* pOut, const void* pIn, const ::com::sun::star::uno::Type& type, const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  sal_Bool ret = sal_True;\n  const ";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "& _u = *(const ";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "*) pIn;\n  ";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "& _b = *(";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "*) pOut;\n\n";

    if (superType.getLength() > 0)
    {
        o << "  ret = bonobobridge::cpp_convert_u2b((";
        dumpCorbaType(o, superType, sal_False, sal_False);
        o << "&) _u, (const ";
        dumpUnoType(o, superType, sal_False, sal_False);
        o << "&) _b, bridge);\n";
    }

    for (i=0; i < fieldCount; i++)
    {
        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

        cIndex = fieldName.indexOf("_reserved_identifier_");

        if (cIndex == 0)
            corbaFieldName = fieldName.copy(OString("_reserved_identifier_").getLength());
        else
            corbaFieldName = fieldName;

        if (isArray(fieldType))
            o << "  // fix me: no conversion of array types!\n";
        else
            o << "  if (ret)\n"
              << "    ret = bonobobridge::cpp_convert_u2b("
              << "_b." << corbaFieldName.getStr()
              << ", _u." << fieldName.getStr()
              << ", bridge);\n";
    }

    o   << "  return ret;\n"
        << "}\n\n";

    o << "inline sal_Bool bonobobridge::cpp_convert_b2u(";
    dumpUnoType(o, m_typeName, sal_False, sal_True);
    o << " u    , ";
    dumpCorbaType(o, m_typeName, sal_True, sal_True);
    o << " b,   const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  return convert_b2u_" << m_typeName.replace('/', '_')
      << "(&u, &b, ::getCppuType(&u), bridge);\n"
      << "};\n\n";

    o << "inline sal_Bool bonobobridge::cpp_convert_u2b(";
    dumpCorbaType(o, m_typeName, sal_False, sal_True);
    o << " b, ";
    dumpUnoType(o, m_typeName, sal_True, sal_True);
    o << " u,   const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  return convert_u2b_" << m_typeName.replace('/', '_')
      << "(&b, &u, ::getCppuType(&u), bridge);\n"
      << "};\n\n";
}



sal_Bool ExceptionType::dumpSuperMember(FileStream& o, const OString& superType, sal_Bool bWithType)
{
    sal_Bool hasMember = sal_False;

    if (superType.getLength() > 0)
    {
        TypeReader aSuperReader(m_typeMgr.getTypeReader(superType));

        if (aSuperReader.isValid())
        {
            hasMember = dumpSuperMember(o, aSuperReader.getSuperTypeName(), bWithType);

            sal_uInt32      fieldCount = aSuperReader.getFieldCount();
            RTFieldAccess   access = RT_ACCESS_INVALID;
            OString         fieldName;
            OString         fieldType;
            for (sal_uInt16 i=0; i < fieldCount; i++)
            {
                access = aSuperReader.getFieldAccess(i);

                if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                    continue;

                fieldName = aSuperReader.getFieldName(i);
                fieldType = aSuperReader.getFieldType(i);

                if (hasMember)
                {
                    o << ", ";
                } else
                {
                    hasMember = (fieldCount > 0);
                }

                if (bWithType)
                {
                    dumpUnoType(o, fieldType, sal_True, sal_True);
                    o << " ";
                }
                o << "__" << fieldName;
            }
        }
    }

    return hasMember;
}

//*************************************************************************
// EnumType
//*************************************************************************
EnumType::EnumType(TypeReader& typeReader,
                            const OString& typeName,
                            const TypeManager& typeMgr,
                            const TypeDependency& typeDependencies,
                            TypeSet* generatedConversions)
    : CorbaType(typeReader, typeName, typeMgr, typeDependencies, generatedConversions)
{
}

EnumType::~EnumType()
{

}

void EnumType::dumpFunctions(FileStream& o)
{
    if (m_typeName.equals("com/sun/star/uno/TypeClass"))
        return;

    m_generatedConversions->insert(m_typeName);

    o << "static sal_Bool convert_b2u_" << m_typeName.replace('/', '_')
      << "(void* pOut, const void* pIn, const ::com::sun::star::uno::Type& type, const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  *(";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "*) pOut = (";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << ") *(const ";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "*) pIn;\n\n"
      << "  return sal_True;\n"
      << "}\n\n";

    o << "static sal_Bool convert_u2b_" << m_typeName.replace('/', '_')
      << "(void* pOut, const void* pIn, const ::com::sun::star::uno::Type& type, const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  *(";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << "*) pOut = (";
    dumpCorbaType(o, m_typeName, sal_False, sal_False);
    o << ") *(const ";
    dumpUnoType(o, m_typeName, sal_False, sal_False);
    o << "*) pIn;\n\n"
      << "  return sal_True;\n"
      << "}\n\n";

    o << "inline sal_Bool bonobobridge::cpp_convert_b2u(";
    dumpUnoType(o, m_typeName, sal_False, sal_True);
    o << " u    , ";
    dumpCorbaType(o, m_typeName, sal_True, sal_True);
    o << " b,   const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  return convert_b2u_" << m_typeName.replace('/', '_')
      << "(&u, &b, ::getCppuType(&u), bridge);\n"
      << "};\n\n";

    o << "inline sal_Bool bonobobridge::cpp_convert_u2b(";
    dumpCorbaType(o, m_typeName, sal_False, sal_True);
    o << " b, ";
    dumpUnoType(o, m_typeName, sal_True, sal_True);
    o << " u,   const ::vos::ORef< ::bonobobridge::Bridge >& bridge) {\n"
      << "  return convert_u2b_" << m_typeName.replace('/', '_')
      << "(&b, &u, ::getCppuType(&u), bridge);\n"
      << "};\n\n";

    return;
}


//*************************************************************************
// TypeDefType
//*************************************************************************
TypeDefType::TypeDefType(TypeReader& typeReader,
                            const OString& typeName,
                            const TypeManager& typeMgr,
                            const TypeDependency& typeDependencies,
                            TypeSet* generatedConversions)
    : CorbaType(typeReader, typeName, typeMgr, typeDependencies, generatedConversions)
{
}

TypeDefType::~TypeDefType()
{

}

void TypeDefType::dumpFunctions(FileStream& o)
{
}



//*************************************************************************
// produceType
//*************************************************************************
sal_Bool produceType(const OString& typeName,
                        TypeManager& typeMgr,
                        TypeDependency& typeDependencies,
                        CorbaOptions* pOptions,
                        FileStream& o, TypeSet* allreadyDumped,
                        TypeSet* generatedConversions)
    throw( CannotDumpException )
{
    sal_Bool bNewTypeSet = (allreadyDumped == NULL);
    sal_Bool ret = sal_True;

    if (bNewTypeSet)
        allreadyDumped = new TypeSet();


    if (!typeDependencies.isGenerated(typeName))
    {
       TypeReader reader(typeMgr.getTypeReader(typeName));

        if (!reader.isValid() && !typeName.equals("/"))
                ret = sal_False;

        if( ret && !checkTypeDependencies(typeMgr, typeDependencies, typeName))
            ret = sal_False;

        if (ret)
        {
            RTTypeClass typeClass = reader.getTypeClass();

            switch (typeClass)
            {
                case RT_TYPE_INTERFACE:
                {
                    InterfaceType iType(reader, typeName, typeMgr, typeDependencies, generatedConversions);
                    ret = iType.dump(pOptions, o, allreadyDumped);
                    if (ret) typeDependencies.setGenerated(typeName);
                    ret = iType.dumpDependedTypes(pOptions, o, allreadyDumped);
                }
                break;
                case RT_TYPE_MODULE:
                {
                    ModuleType mType(reader, typeName, typeMgr, typeDependencies, generatedConversions);
                    if (mType.hasConstants())
                    {
                        ret = mType.dump(pOptions, o, allreadyDumped);
                        if (ret) typeDependencies.setGenerated(typeName);
                    } else
                    {
                        typeDependencies.setGenerated(typeName);
                        ret = sal_True;
                    }
                }
                break;
                case RT_TYPE_STRUCT:
                {
                    StructureType sType(reader, typeName, typeMgr, typeDependencies, generatedConversions);
                    ret = sType.dump(pOptions, o, allreadyDumped);
                    if (ret) typeDependencies.setGenerated(typeName);
                    ret = sType.dumpDependedTypes(pOptions, o, allreadyDumped);
                }
                break;
                case RT_TYPE_ENUM:
                {
                    EnumType enType(reader, typeName, typeMgr, typeDependencies, generatedConversions);
                    ret = enType.dump(pOptions, o, allreadyDumped);
                    if (ret) typeDependencies.setGenerated(typeName);
                    ret = enType.dumpDependedTypes(pOptions, o, allreadyDumped);
                }
                break;
                case RT_TYPE_EXCEPTION:
                {
                    ExceptionType eType(reader, typeName, typeMgr, typeDependencies, generatedConversions);
                    ret = eType.dump(pOptions, o, allreadyDumped);
                    if (ret) typeDependencies.setGenerated(typeName);
                    ret = eType.dumpDependedTypes(pOptions, o, allreadyDumped);
                }
                break;
                case RT_TYPE_TYPEDEF:
                {
                    TypeDefType tdType(reader, typeName, typeMgr, typeDependencies, generatedConversions);
                    ret = tdType.dump(pOptions, o, allreadyDumped);
                    if (ret) typeDependencies.setGenerated(typeName);
                    ret = tdType.dumpDependedTypes(pOptions, o, allreadyDumped);
                }
                break;
                case RT_TYPE_CONSTANTS:
                {
                    ConstantsType cType(reader, typeName, typeMgr, typeDependencies, generatedConversions);
                    if (cType.hasConstants())
                    {
                        ret = cType.dump(pOptions, o, allreadyDumped);
                        if (ret) typeDependencies.setGenerated(typeName);
                    } else
                    {
                        typeDependencies.setGenerated(typeName);
                        ret = sal_True;
                    }
                }
                break;
                case RT_TYPE_SERVICE:
                case RT_TYPE_OBJECT:
                    ret = sal_True;
                    break;
            }
        }
    }

    if (bNewTypeSet)
        delete allreadyDumped;

    return ret;
}

//*************************************************************************
// scopedName
//*************************************************************************
OString scopedName(const OString& scope, const OString& type,
       sal_Bool bNoNameSpace)
{
    sal_Int32 nPos = type.lastIndexOf( '/' );
    if (nPos == -1)
        return type;

    if (bNoNameSpace)
        return type.copy(nPos+1);

    OStringBuffer tmpBuf(type.getLength()*2);
    nPos = 0;
    do
    {
        tmpBuf.append("::");
        tmpBuf.append(type.getToken(0, '/', nPos));
    } while( nPos != -1 );

    return tmpBuf.makeStringAndClear();
}

//*************************************************************************
// shortScopedName
//*************************************************************************
OString shortScopedName(const OString& scope, const OString& type,
               sal_Bool bNoNameSpace)
{
    sal_Int32 nPos = type.lastIndexOf( '/' );
    if( nPos == -1 )
        return OString();

    if (bNoNameSpace)
        return OString();

    // scoped name only if the namespace is not equal
    if (scope.lastIndexOf('/') > 0)
    {
        OString tmpScp(scope.copy(0, scope.lastIndexOf('/')));
        OString tmpScp2(type.copy(0, nPos));

        if (tmpScp == tmpScp2)
            return OString();
    }

    OString aScope( type.copy( 0, nPos ) );
    OStringBuffer tmpBuf(aScope.getLength()*2);

    nPos = 0;
    do
    {
        tmpBuf.append("::");
        tmpBuf.append(aScope.getToken(0, '/', nPos));
    } while( nPos != -1 );

    return tmpBuf.makeStringAndClear();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
