/*************************************************************************
 *
 *  $RCSfile: cpputype.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-13 12:04:24 $
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

#include    "cpputype.hxx"
#include    "cppuoptions.hxx"

using namespace rtl;

//*************************************************************************
// CppuType
//*************************************************************************
CppuType::CppuType(TypeReader& typeReader,
                   const OString& typeName,
                   const TypeManager& typeMgr,
                   const TypeDependency& typeDependencies)
    : m_inheritedMemberCount(0)
    , m_cppuTypeLeak(sal_False)
    , m_cppuTypeDynamic(sal_True)
    , m_cppuTypeStatic(sal_False)
    , m_indentLength(0)
    , m_typeName(typeName)
    , m_name(typeName.getToken(typeName.getTokenCount('/') - 1, '/'))
    , m_reader(typeReader)
    , m_typeMgr((TypeManager&)typeMgr)
    , m_dependencies(typeDependencies)
    , m_bIsNestedType(sal_False)
{
    // check if this type is nested
    sal_Int32 i = typeName.lastIndexOf('/');

    if (i >= 0)
    {
        OString outerTypeName(typeName.copy(0, i));
        m_bIsNestedType = (m_typeMgr.getTypeClass(outerTypeName) == RT_TYPE_INTERFACE);
    }

    // check if this type has nested types
    RegistryKey key = m_typeMgr.getTypeKey(typeName);

    key.getKeyNames(OUString(), m_nestedTypeNames);
}

CppuType::~CppuType()
{

}

sal_Bool CppuType::isNestedTypeByName(const ::rtl::OString& type)
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

sal_Bool CppuType::hasNestedType(const ::rtl::OString& type)
{
    sal_Bool ret = sal_False;

    if (m_nestedTypeNames.getLength() > 0)
    {
        OUString typeName(OStringToOUString(type, RTL_TEXTENCODING_UTF8));

        for (sal_uInt32 i = 0; !ret && (i < m_nestedTypeNames.getLength()); i++)
            ret = typeName.equals(m_nestedTypeNames.getElement(i).copy(5));
    }

    return ret;
}

sal_Bool CppuType::dump(CppuOptions* pOptions)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_False;

    if (isNestedType())
        return sal_True;

    if (pOptions->isValid("-L"))
        m_cppuTypeLeak = sal_True;
    if (pOptions->isValid("-C"))
        m_cppuTypeDynamic = sal_False;
    if (pOptions->isValid("-CS"))
    {
        m_cppuTypeDynamic = sal_False;
        m_cppuTypeStatic = sal_True;
    }

    OString outPath;
    if (pOptions->isValid("-O"))
        outPath = pOptions->getOption("-O");

    OString tmpFileName;
    OString hFileName = createFileNameFromType(outPath, m_typeName, ".hdl");

    sal_Bool bFileExists = sal_False;
    sal_Bool bFileCheck = sal_False;

    if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
    {
        bFileExists = fileExists( hFileName );
        ret = sal_True;
    }

    if ( bFileExists && pOptions->isValid("-Gc") )
    {
        tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tml");
        bFileCheck = sal_True;
    }

    if ( !bFileExists || bFileCheck )
    {
        FileStream hFile;

        if ( bFileCheck )
            hFile.openFile(tmpFileName);
        else
            hFile.openFile(hFileName);

        if(!hFile.isValid())
        {
            OString message("cannot open ");
            message += hFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpHFile(hFile);

        hFile.closeFile();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(hFileName, tmpFileName);
        }
    }

    OString hxxFileName = createFileNameFromType(outPath, m_typeName, ".hpp");

    if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
    {
        bFileExists = fileExists( hFileName );
        ret = sal_True;
    }

    if ( bFileExists && pOptions->isValid("-Gc") )
    {
        tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tmp");
        bFileCheck = sal_True;
    }

    if ( !bFileExists || bFileCheck )
    {
        FileStream hxxFile;

        if ( bFileCheck )
            hxxFile.openFile(tmpFileName);
        else
            hxxFile.openFile(hxxFileName);

        if(!hxxFile.isValid())
        {
            OString message("cannot open ");
            message += hxxFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpHxxFile(hxxFile);

        hxxFile.closeFile();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(hFileName, tmpFileName);
        }
    }

    return ret;
}
sal_Bool CppuType::dumpDependedTypes(CppuOptions* pOptions)
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

OString CppuType::dumpHeaderDefine(FileStream& o, sal_Char* prefix, sal_Bool bExtended)
{
    if (m_typeName.equals("/"))
    {
        bExtended = sal_False;
        m_typeName = "global";
    }

    sal_uInt32 length = 3 + m_typeName.getLength() + strlen(prefix);

    if (bExtended)
        length += m_name.getLength() + 1;

    OStringBuffer tmpBuf(length);

    tmpBuf.append('_');
    tmpBuf.append(m_typeName);
    tmpBuf.append('_');
    if (bExtended)
    {
        tmpBuf.append(m_name);
        tmpBuf.append('_');
    }
    tmpBuf.append(prefix);
    tmpBuf.append('_');

    OString tmp(tmpBuf.makeStringAndClear().replace('/', '_').toUpperCase());

    o << "#ifndef " << tmp << "\n#define " << tmp << endl;

    return tmp;
}

void CppuType::dumpDefaultHIncludes(FileStream& o)
{
    o << "#ifndef _CPPU_MACROS_HXX_\n"
      << "#include <cppu/macros.hxx>\n"
      << "#endif\n";

    if (m_typeMgr.getTypeClass(m_typeName) == RT_TYPE_INTERFACE)
    {
        o << "#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_\n"
          << "#include <com/sun/star/uno/Reference.h>\n"
          << "#endif\n";
    }
}

void CppuType::dumpDefaultHxxIncludes(FileStream& o)
{
    o << "#ifndef _OSL_MUTEX_HXX_\n"
      << "#include <osl/mutex.hxx>\n"
      << "#endif\n\n";

    o << "#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_\n"
      << "#include <com/sun/star/uno/Type.hxx>\n"
      << "#endif\n";

    if (m_typeMgr.getTypeClass(m_typeName) == RT_TYPE_INTERFACE)
    {
        o << "#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_\n"
          << "#include <com/sun/star/uno/Reference.hxx>\n"
          << "#endif\n";
    }
}

void CppuType::dumpInclude(FileStream& o, const OString& typeName, sal_Char* prefix, sal_Bool bExtended, sal_Bool bCaseSensitive)
{
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

    OString tmp(tmpBuf.makeStringAndClear().replace('/', '_').toUpperCase());

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
    o << ">\n#endif\n";
}

void CppuType::dumpDepIncludes(FileStream& o, const OString& typeName, sal_Char* prefix)
{
    TypeUsingSet usingSet(m_dependencies.getDependencies(typeName));

    TypeUsingSet::const_iterator iter = usingSet.begin();

    OString     sPrefix(OString(prefix).toUpperCase());
    sal_Bool    bSequenceDumped = sal_False;
    sal_Bool    bInterfaceDumped = sal_False;
    sal_uInt32  index = 0;
    sal_uInt32  seqNum = 0;
    OString     relType;
    while (iter != usingSet.end())
    {
        sal_Bool bDumpThisType = sal_True;
        index = (*iter).m_type.lastIndexOf(']');
        seqNum = (index > 0 ? ((index+1) / 2) : 0);

        relType = (*iter).m_type;
        if (index > 0)
            relType = relType.copy(index+1);

        if (isNestedTypeByName(relType) && hasNestedType(relType))
            bDumpThisType = sal_False;

        if (bDumpThisType)
        {
            OString defPrefix("HXX");
            if (sPrefix.equals("HDL"))
                defPrefix = "H";

            if (seqNum > 0 && !bSequenceDumped)
            {
                bSequenceDumped = sal_True;
                o << "#ifndef _COM_SUN_STAR_UNO_SEQUENCE_" << defPrefix
                  << "_\n#include <com/sun/star/uno/Sequence." << defPrefix.toLowerCase()
                  << ">\n#endif\n";
            }

            if (getBaseType(relType).getLength() == 0 &&
                m_typeName != relType)
            {
                if (m_typeMgr.getTypeClass(relType) == RT_TYPE_INTERFACE
                    && sPrefix.equals("HDL"))
                {
                    if (!bInterfaceDumped)
                    {
                        bInterfaceDumped = sal_True;
                        o << "#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_\n"
                          << "#include <com/sun/star/uno/Reference.h>\n"
                          << "#endif\n";
                    }

                    if (!((*iter).m_use & TYPEUSE_SUPER))
                    {
                        if (isNestedTypeByName(relType))
                        {
                            sal_Int32 iLastS = relType.lastIndexOf('/');

                            OString outerNamespace(relType.copy(0,iLastS));
                            OString innerClass(relType.copy(iLastS+1));

                            iLastS = outerNamespace.lastIndexOf('/');
                            OString outerClass(outerNamespace.copy(iLastS+1));

                            o << endl;
                            dumpNameSpace(o, sal_True, sal_False, outerNamespace);
                            o << "\nclass " << outerClass << "::" << innerClass << ";\n";
                            dumpNameSpace(o, sal_False, sal_False, outerNamespace);
                            o << "\n\n";
                        }
                        else
                        {
                            o << endl;
                            dumpNameSpace(o, sal_True, sal_False, relType);
                            o << "\nclass " << scopedName(m_typeName, relType, sal_True) << ";\n";
                            dumpNameSpace(o, sal_False, sal_False, relType);
                            o << "\n\n";
                        }
                    } else
                    {
                        if (isNestedTypeByName(relType))
                        {
                            sal_Int32 iLastS = relType.lastIndexOf('/');

                            OString outerNamespace(relType.copy(0,iLastS));

                            dumpInclude(o, outerNamespace, prefix);
                        }
                        else
                            dumpInclude(o, relType, prefix);
                    }
                } else
                {
                    if (isNestedTypeByName(relType))
                    {
                        sal_Int32 iLastS = relType.lastIndexOf('/');

                        OString outerNamespace(relType.copy(0,iLastS));

                        dumpInclude(o, outerNamespace, prefix);
                    }
                    else
                        dumpInclude(o, relType, prefix);
                }
            } else
            if (relType == "any")
            {
                o << "#ifndef _COM_SUN_STAR_UNO_ANY_" << defPrefix
                  << "_\n#include <com/sun/star/uno/Any." << defPrefix.toLowerCase()
                  << ">\n#endif\n";
            } else
            if (relType == "type")
            {
                o << "#ifndef _COM_SUN_STAR_UNO_TYPE_" << defPrefix
                  << "_\n#include <com/sun/star/uno/Type." << defPrefix.toLowerCase()
                  << ">\n#endif\n";
            } else
            if (relType == "string" && sPrefix.equals("HDL"))
            {
                o << "#ifndef _RTL_USTRING_HXX_\n"
                  << "#include <rtl/ustring.hxx>\n"
                  << "#endif\n";
            }
        }

        iter++;
    }
    if (m_typeName.equals(typeName) && (getNestedTypeNames().getLength() > 0))
    {
        o << "// includes for nested types\n\n";

        for (sal_uInt32 i = 0; i < getNestedTypeNames().getLength(); i++)
        {
            OUString s(getNestedTypeNames().getElement(i));

            OString nestedName(s.getStr(), s.getLength(), RTL_TEXTENCODING_DONTKNOW);

            dumpDepIncludes(o, nestedName, prefix);
        }
    }
}

void CppuType::dumpNameSpace(FileStream& o, sal_Bool bOpen, sal_Bool bFull, const OString& type)
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

    sal_uInt32 count = typeName.getTokenCount('/');

    if (count == 1 && !bFull)
        return;

    if (!bFull) count--;

    if (bOpen)
    {
        for (sal_uInt32 i=0; i < count; i++)
        {
            o << "namespace " << typeName.getToken(i, '/');
            if (bOneLine)
                o << " { ";
            else
                 o << "\n{\n";
        }
    } else
    {
        for (int i=count-1; i >= 0; i--)
        {
            o << "}";
            if (bOneLine)
                o << " ";
            else
                 o << " // " << typeName.getToken(i, '/') << "\n";
        }
    }
}

void CppuType::dumpLGetCppuType(FileStream& o)
{
    OString typeName(m_typeName.replace('/', '_'));

    o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    if (m_reader.getTypeClass() == RT_TYPE_TYPEDEF)
    {
        o << "inline const ::com::sun::star::uno::Type& SAL_CALL get_" << typeName << "_Type( ) SAL_THROW( () )\n{\n";
    } else
    {
        o << "inline const ::com::sun::star::uno::Type& SAL_CALL getCppuType( ";
        dumpType(o, m_typeName, sal_True, sal_False);
        o << "* ) SAL_THROW( () )\n{\n";
    }
    inc();

    o << indent() << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << indent() << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << indent() << "#endif\n\n";

    o << indent() << "if ( !s_pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "typelib_static_type_init( &s_pType_" << typeName << ", "
      << getTypeClass(m_typeName, sal_True) << ", \"" << m_typeName.replace('/', '.') << "\" );\n";
    dec();
    o << indent() << "}\n";
    o << indent() << "return * reinterpret_cast< ::com::sun::star::uno::Type * >( &s_pType_"
      << typeName <<" );\n";
    dec();
    o << indent() << "}\n";

    return;
}

void CppuType::dumpGetCppuType(FileStream& o)
{
    OString typeName(m_typeName.replace('/', '_'));

    if ( m_cppuTypeLeak )
    {
        dumpLGetCppuType(o);
        return;
    }
    if ( !m_cppuTypeDynamic )
    {
        dumpCGetCppuType(o);
        return;
    }

    if ( !m_typeName.equals("com/sun/star/uno/Exception") )
    {
        o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
          << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
          << "#endif\n\n";
    }

    o << "inline const ::com::sun::star::uno::Type& SAL_CALL getCppuType( ";
    dumpType(o, m_typeName, sal_True, sal_False);
    o << "* ) SAL_THROW( () )\n{\n";
    inc();

    if ( m_typeName.equals("com/sun/star/uno/Exception") )
    {
        o << indent() << "return * reinterpret_cast< const ::com::sun::star::uno::Type * >( ::typelib_static_type_getByTypeClass("
          << " typelib_TypeClass_EXCEPTION ) );\n";
    } else
    {
        o << indent() << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
          << indent() << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
          << indent() << "#endif\n\n";

        o << indent() << "if ( !s_pType_" << typeName << " )\n" << indent() << "{\n";
        inc();

        OString superType(m_reader.getSuperTypeName());
        sal_Bool bIsBaseException = sal_False;
        if (superType.getLength() > 0)
        {
            if ( superType.equals("com/sun/star/uno/Exception") )
            {
                bIsBaseException = sal_True;
            } else
            {
                o << indent() << "const ::com::sun::star::uno::Type& rBaseType = getCppuType( ( ";
                dumpType(o, superType, sal_True, sal_False);
                o << " *)0 );\n\n";
            }
        }

        sal_uInt32 count = getMemberCount();
        if (count)
        {
            o << indent() << "typelib_TypeDescriptionReference * aMemberRefs[" << count << "];\n";
//            << indent() << "const sal_Char * aMemberNames[" << count << "];\n";

            sal_uInt32      fieldCount = m_reader.getFieldCount();
            RTFieldAccess   access = RT_ACCESS_INVALID;
            OString         fieldType, fieldName;
            OString         scope = m_typeName.replace('/', '.');
            sal_Bool        bWithScope = sal_True;
            OString         modFieldType;
            StringSet       generatedTypeSet;
            StringSet::iterator findIter;

            for (sal_uInt16 i=0; i < fieldCount; i++)
            {
                access = m_reader.getFieldAccess(i);

                if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                    continue;

                fieldName = m_reader.getFieldName(i);
                fieldType = checkRealBaseType(m_reader.getFieldType(i), sal_True);

                modFieldType = typeToIdentifier(fieldType);

                findIter = generatedTypeSet.find(fieldType);
                if ( findIter == generatedTypeSet.end() )
                {
                    generatedTypeSet.insert(fieldType);
                    o << indent() << "const ::com::sun::star::uno::Type& rMemberType_"
                      << modFieldType/*i*/ << " = getCppuType( ( ";
                    dumpType(o, fieldType, sal_True, sal_False);
                    o << " *)0 );\n";
                }

                o << indent() << "aMemberRefs[" << i << "] = rMemberType_"
                  << modFieldType/*i*/ << ".getTypeLibType();\n";
            }
            o << endl;
        }

        o << indent() << "typelib_static_compound_type_init( &s_pType_" << typeName << ", "
          << getTypeClass(m_typeName, sal_True) << ", \"" << m_typeName.replace('/', '.') << "\", ";
        if ( superType.getLength() > 0 || bIsBaseException )
        {
            if ( bIsBaseException )
            {
                o << "* ::typelib_static_type_getByTypeClass( typelib_TypeClass_EXCEPTION ), "
                  << count << ", ";
            } else
            {
                o << "rBaseType.getTypeLibType(), " << count << ", ";
            }
        } else
        {
            o << "0, " << count << ", ";
        }

        if (count)
        {
            o << " aMemberRefs );\n";
        } else
        {
            o << " 0 );\n";
        }
        dec();
        o << indent() << "}\n";
        o << indent() << "return * reinterpret_cast< const ::com::sun::star::uno::Type * >( &s_pType_"
          << typeName <<" );\n";
    }
    dec();
    o << indent() << "}\n";

    return;
}

void CppuType::dumpCGetCppuType(FileStream& o)
{
    OString typeName(m_typeName.replace('/', '_'));

    o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << "static ::com::sun::star::uno::Type * pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    if (m_cppuTypeStatic)
        o << "static";
    else
        o << "inline";
    o << " const ::com::sun::star::uno::Type& SAL_CALL getCppuType( ";
    dumpType(o, m_typeName, sal_True, sal_False);
    o << "* ) SAL_THROW( () )\n{\n";
    inc();

    o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << indent() << "static ::com::sun::star::uno::Type * pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << indent() << "if ( !pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );\n";

    o << indent() << "if ( !pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "::rtl::OUString sTypeName( RTL_CONSTASCII_USTRINGPARAM(\""
      << m_typeName.replace('/', '.') << "\") );\n\n";

    if ( m_cppuTypeDynamic )
    {
        o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n";
        o << indent() << "static ::com::sun::star::uno::Type aType_" << typeName << "( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << indent() << "pType_" << typeName << " = &aType_" << typeName << ";\n";
        o << "#else\n";
        o << indent() << "pType_" << typeName << " = new ::com::sun::star::uno::Type( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << "#endif\n";
    } else
    {
        o << indent() << "// Start inline typedescription generation\n"
          << indent() << "typelib_TypeDescription * pTD = 0;\n";

        OString superType(m_reader.getSuperTypeName());
        if (superType.getLength() > 0)
        {
            o << indent() << "const ::com::sun::star::uno::Type& rSuperType = getCppuType( ( ";
            dumpType(o, superType, sal_True, sal_False);
            o << " *)0 );\n";
        }

        dumpCppuGetTypeMemberDecl(o, CPPUTYPEDECL_ALLTYPES);

        sal_uInt32 count = getMemberCount();
        if (count)
        {
            o << "\n" << indent() << "typelib_CompoundMember_Init aMembers["
              << count << "];\n";

            sal_uInt32      fieldCount = m_reader.getFieldCount();
            RTFieldAccess   access = RT_ACCESS_INVALID;
            OString         fieldType, fieldName;
            OString         scope = m_typeName.replace('/', '.');
            sal_Bool        bWithScope = sal_True;

            if ( m_reader.getTypeClass() == RT_TYPE_STRUCT ||
                 m_reader.getTypeClass() == RT_TYPE_ENUM )
            {
                bWithScope = sal_False;
            }

            for (sal_uInt16 i=0; i < fieldCount; i++)
            {
                access = m_reader.getFieldAccess(i);

                if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                    continue;

                fieldName = m_reader.getFieldName(i);
                fieldType = checkRealBaseType(m_reader.getFieldType(i), sal_True);

                o << indent() << "::rtl::OUString sMemberType" << i << "( RTL_CONSTASCII_USTRINGPARAM(\""
                  << fieldType.replace('/', '.') << "\") );\n";
                o << indent() << "::rtl::OUString sMemberName" << i << "( RTL_CONSTASCII_USTRINGPARAM(\"";
                if (bWithScope)
                  o << scope << "::";
                o << fieldName << "\") );\n";
                o << indent() << "aMembers[" << i << "].eTypeClass = "
                  << "(typelib_TypeClass)" << getTypeClass(fieldType) << ";\n"
                  << indent() << "aMembers[" << i << "].pTypeName = sMemberType" << i << ".pData;\n"
                  << indent() << "aMembers[" << i << "].pMemberName = sMemberName" << i << ".pData;\n";
            }
        }

        o << "\n" << indent() << "typelib_typedescription_new(\n";
        inc();
        o << indent() << "&pTD,\n" << indent() << "(typelib_TypeClass)"
          << getTypeClass() << ", sTypeName.pData,\n";

        if (superType.getLength() > 0)
            o << indent() << "rSuperType.getTypeLibType(),\n";
        else
            o << indent() << "0,\n";

        if ( count )
        {
            o << indent() << count << ",\n" << indent() << "aMembers );\n\n";
        } else
        {
            o << indent() << count << ",\n" << indent() << "0 );\n\n";
        }

        dec();
        o << indent() << "typelib_typedescription_register( (typelib_TypeDescription**)&pTD );\n\n";

        o << indent() << "typelib_typedescription_release( pTD );\n"
          << indent() << "// End inline typedescription generation\n\n";

        o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n";
        o << indent() << "static ::com::sun::star::uno::Type aType_" << typeName << "( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << indent() << "pType_" << typeName << " = &aType_" << typeName << ";\n";
        o << "#else\n";
        o << indent() << "pType_" << typeName << " = new ::com::sun::star::uno::Type( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << "#endif\n";
    }

    dec();
    o << indent() << "}\n";
    dec();
    o << indent() << "}\n\n";
    o << indent() << "return *pType_" << typeName << ";\n";
    dec();
    o << "}\n";
}

void CppuType::dumpCppuGetTypeMemberDecl(FileStream& o, CppuTypeDecl eDeclFlag)
{
    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;

    StringSet aFinishedTypes;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        if (aFinishedTypes.count(m_reader.getFieldType(i)) == 0)
        {
            aFinishedTypes.insert(m_reader.getFieldType(i));
            dumpCppuGetType(o, m_reader.getFieldType(i), sal_True, eDeclFlag);
        }
    }
}

sal_uInt32 CppuType::getMemberCount()
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

sal_uInt32 CppuType::checkInheritedMemberCount(const TypeReader* pReader)
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
            {
                count++;
            }
        }
    }

    return count;
}

sal_uInt32 CppuType::getInheritedMemberCount()
{
    if (m_inheritedMemberCount == 0)
    {
        m_inheritedMemberCount = checkInheritedMemberCount(0);
    }

    return m_inheritedMemberCount;
}

OString CppuType::getTypeClass(const OString& type, sal_Bool bCStyle)
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

void CppuType::dumpType(FileStream& o, const OString& type,
                        sal_Bool bConst, sal_Bool bRef, sal_Bool bNative)
    throw( CannotDumpException )
{
    OString sType(checkRealBaseType(type, sal_True));
    sal_uInt32 index = sType.lastIndexOf(']');
    sal_uInt32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    OString relType = (index > 0 ? (sType).copy(index+1) : type);

    RTTypeClass typeClass = m_typeMgr.getTypeClass(relType);

    if (bConst) o << "const ";

    sal_uInt32 i;
    for (i=0; i < seqNum; i++)
    {
        o << "::com::sun::star::uno::Sequence< ";
    }

    switch (typeClass)
    {
        case RT_TYPE_INTERFACE:
            if (bNative)
                o << scopedName(m_typeName, relType);
            else
                o << "::com::sun::star::uno::Reference< " << scopedName(m_typeName, relType) << " >";
            break;
        case RT_TYPE_INVALID:
            {
                OString tmp(getBaseType(relType));
                if (tmp.getLength() > 0)
                {
                    o << getBaseType(relType);
                } else
                    throw CannotDumpException("Unknown type '" + relType + "', incomplete type library.");
            }
            break;
        case RT_TYPE_STRUCT:
        case RT_TYPE_ENUM:
        case RT_TYPE_TYPEDEF:
        case RT_TYPE_EXCEPTION:
            o << scopedName(m_typeName, relType);
            break;
    }

    for (i=0; i < seqNum; i++)
    {
        o << " >";
    }

    if (bRef) o << "&";
}

OString CppuType::getBaseType(const OString& type)
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

void CppuType::dumpCppuGetType(FileStream& o, const OString& type, sal_Bool bDecl, CppuTypeDecl eDeclFlag)
{
    OString sType( checkRealBaseType(type, sal_True) );
    sal_uInt32 index = sType.lastIndexOf(']');
    OString relType = (index > 0 ? (sType).copy(index+1) : type);

    if (eDeclFlag == CPPUTYPEDECL_ONLYINTERFACES)
    {
         if (m_typeMgr.getTypeClass(relType) == RT_TYPE_INTERFACE)
        {
            o << indent() << "getCppuType( (";
            dumpType(o, type, sal_True, sal_False);
            o << "*)0 )";

            if (bDecl)
                o << ";\n";
        }
    } else
    {
        if (isBaseType(type))
        {
            return;
        } else
        {
            if (eDeclFlag == CPPUTYPEDECL_NOINTERFACES &&
                m_typeMgr.getTypeClass(relType) == RT_TYPE_INTERFACE)
                return;

//          if (m_typeMgr.getTypeClass(type) == RT_TYPE_TYPEDEF)
//          {
//              o << indent() << "get_" << type.replace('/', '_') << "_Type()";
//          } else
//          {
                o << indent() << "getCppuType( (";
                dumpType(o, type, sal_True, sal_False);
                o << "*)0 )";
//          }
        }
        if (bDecl)
            o << ";\n";
    }
}

void CppuType::dumpTypeInit(FileStream& o, const OString& typeName)
{
    OString type(checkSpecialCppuType(typeName));

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
            dumpType(o, type);
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
                o << "(" << shortScopedName("", type, sal_False)
                  << "::" << type.getToken(type.getTokenCount('/') - 1, '/')
                  << "_" << reader.getFieldName(0) << ")";
                return;
            }
        }
    }

    o << "()";
}

BASETYPE CppuType::isBaseType(const OString& type)
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

OString CppuType::typeToIdentifier(const OString& type)
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

OString CppuType::checkSpecialCppuType(const OString& type)
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
        {
            break;
        }
    }

    return baseType;
}

OString CppuType::checkRealBaseType(const OString& type, sal_Bool bResolveTypeOnly)
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
        {
            break;
        }
    }

    if ( bResolveTypeOnly )
        baseType = seqPrefix + baseType;

    return baseType;
}

void CppuType::dumpConstantValue(FileStream& o, sal_uInt16 index)
{
    RTConstValue constValue = m_reader.getFieldConstValue(index);

    switch (constValue.m_type)
    {
        case RT_TYPE_BOOL:
            if (constValue.m_value.aBool)
                o << "sal_True";
            else
                o << "sal_False";
            break;
        case RT_TYPE_BYTE:
            {
                char tmp[16];
                sprintf(tmp, "0x%x", (sal_Int8)constValue.m_value.aByte);
                o << "(sal_Int8)" << tmp;
            }
            break;
        case RT_TYPE_INT16:
            o << "(sal_Int16)" << constValue.m_value.aShort;
            break;
        case RT_TYPE_UINT16:
            o << "(sal_uInt16)" << constValue.m_value.aUShort;
            break;
        case RT_TYPE_INT32:
            o << "(sal_Int32)" << constValue.m_value.aLong;
            break;
        case RT_TYPE_UINT32:
            o << "(sal_uInt32)" << constValue.m_value.aULong;
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
                o << "::rtl::OUString::createFromAscii(\"" << aStr.getStr() << "\")";
            }
            break;
    }
}

void CppuType::inc(sal_uInt32 num)
{
    m_indentLength += num;
}

void CppuType::dec(sal_uInt32 num)
{
    if (m_indentLength - num < 0)
        m_indentLength = 0;
    else
        m_indentLength -= num;
}

OString CppuType::indent()
{
    OStringBuffer tmp(m_indentLength);

    for (sal_uInt32 i=0; i < m_indentLength; i++)
    {
        tmp.append(' ');
    }
    return tmp.makeStringAndClear();
}

OString CppuType::indent(sal_uInt32 num)
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
                             const TypeDependency& typeDependencies)
    : CppuType(typeReader, typeName, typeMgr, typeDependencies)
{
    m_inheritedMemberCount = 0;
    m_hasAttributes = sal_False;
    m_hasMethods = sal_False;
}

InterfaceType::~InterfaceType()
{

}

sal_Bool InterfaceType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HDL"));
    o << endl;

    dumpDefaultHIncludes(o);
    o << endl;
    dumpDepIncludes(o, m_typeName, "hdl");
    o << endl;
    dumpNameSpace(o);
    dumpDeclaration(o);
    dumpNameSpace(o, sal_False);

    o << "\nnamespace com { namespace sun { namespace star { namespace uno {\n"
      << "class Type;\n} } } }\n\n";

    if (m_cppuTypeStatic)
        o << "static";
    else
        o << "inline";
    o << " const ::com::sun::star::uno::Type& SAL_CALL getCppuType( ";
    dumpType(o, m_typeName, sal_True, sal_False);
    o << "* ) SAL_THROW( () );\n\n";

    if (getNestedTypeNames().getLength() > 0)
    {
        o << indent() << "// nested types\n\n";
        for (sal_uInt32 i = 0; i < getNestedTypeNames().getLength(); i++)
        {
            OUString s(getNestedTypeNames().getElement(i));

            OString nestedName(s.getStr(), s.getLength(), RTL_TEXTENCODING_DONTKNOW);

            nestedName = checkRealBaseType(nestedName.copy(5));

            if (nestedName.lastIndexOf(']') < 0)
            {
                if (m_cppuTypeStatic)
                    o << "static";
                else
                    o << "inline";

                o << " const ::com::sun::star::uno::Type& SAL_CALL getCppuType( ";
                dumpType(o, nestedName, sal_True, sal_False);
                o << "* ) SAL_THROW( () );\n\n";
            }
        }
    }

    o << "#endif // "<< headerDefine << endl;
    return sal_True;
}

sal_Bool InterfaceType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    o << "\nclass " << m_name;

    OString superType(m_reader.getSuperTypeName());
    if (superType.getLength() > 0)
        o << " : public " << scopedName(m_typeName, superType);

    o << "\n{\npublic:\n";

    if (getNestedTypeNames().getLength() > 0)
    {
        inc();
        o << indent() << "// nested types\n\n";
        for (sal_uInt32 i = 0; i < getNestedTypeNames().getLength(); i++)
        {
            OUString s(getNestedTypeNames().getElement(i));

            OString nestedName(s.getStr(), s.getLength(), RTL_TEXTENCODING_DONTKNOW);

            nestedName = nestedName.copy(5);

            o << indent() << "// " << nestedName.getStr() << "\n";

            TypeReader reader(m_typeMgr.getTypeReader(nestedName));

            if (reader.isValid())
            {
                RTTypeClass typeClass = reader.getTypeClass();
                switch (typeClass) {
                    case RT_TYPE_INTERFACE:
                        {
                            InterfaceType iType(reader, nestedName, m_typeMgr, m_dependencies);
                            iType.dumpDeclaration(o);
                        }
                        break;
                    case RT_TYPE_STRUCT:
                        {
                            StructureType sType(reader, nestedName, m_typeMgr, m_dependencies);
                            sType.dumpDeclaration(o);
                        }
                        break;
                    case RT_TYPE_ENUM:
                        {
                            EnumType enType(reader, nestedName, m_typeMgr, m_dependencies);
                            enType.dumpDeclaration(o);
                        }
                        break;
                    case RT_TYPE_EXCEPTION:
                        {
                            ExceptionType eType(reader, nestedName, m_typeMgr, m_dependencies);
                            eType.dumpDeclaration(o);
                        }
                        break;
                    case RT_TYPE_TYPEDEF:
                        {
                            TypeDefType tdType(reader, nestedName, m_typeMgr, m_dependencies);
                            tdType.dumpDeclaration(o);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        dec();
    }

    inc();

    dumpAttributes(o);
    dumpMethods(o);

    dec();
    o << "};\n\n";

    return sal_True;
}

sal_Bool InterfaceType::dumpHxxFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << endl;

    dumpInclude(o, m_typeName, "hdl");
    o << endl;

    dumpDefaultHxxIncludes(o);
    o << endl;

    dumpDepIncludes(o, m_typeName, "hpp");
    o << endl;

    dumpGetCppuType(o);

    if (getNestedTypeNames().getLength() > 0)
    {
        o << indent() << "// nested types\n\n";
        for (sal_uInt32 i = 0; i < getNestedTypeNames().getLength(); i++)
        {
            OUString s(getNestedTypeNames().getElement(i));

            OString nestedName(s.getStr(), s.getLength(), RTL_TEXTENCODING_DONTKNOW);

            nestedName = nestedName.copy(5);

            o << indent() << "// " << nestedName.getStr() << "\n";

            TypeReader reader(m_typeMgr.getTypeReader(nestedName));

            if (reader.isValid())
            {
                RTTypeClass typeClass = reader.getTypeClass();
                switch (typeClass) {
                    case RT_TYPE_INTERFACE:
                        {
                            InterfaceType iType(reader, nestedName, m_typeMgr, m_dependencies);
                            iType.dumpGetCppuType(o);
                        }
                        break;
                    case RT_TYPE_STRUCT:
                        {
                            StructureType sType(reader, nestedName, m_typeMgr, m_dependencies);
                            sType.dumpGetCppuType(o);
                        }
                        break;
                    case RT_TYPE_ENUM:
                        {
                            EnumType enType(reader, nestedName, m_typeMgr, m_dependencies);
                            enType.dumpGetCppuType(o);
                        }
                        break;
                    case RT_TYPE_EXCEPTION:
                        {
                            ExceptionType eType(reader, nestedName, m_typeMgr, m_dependencies);
                            eType.dumpGetCppuType(o);
                        }
                        break;
                    case RT_TYPE_TYPEDEF:
                        {
                            TypeDefType tdType(reader, nestedName, m_typeMgr, m_dependencies);
                            tdType.dumpGetCppuType(o);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }

    o << "\n#endif // "<< headerDefine << endl;
    return sal_True;
}

void InterfaceType::dumpAttributes(FileStream& o)
{
    sal_uInt32 fieldCount = m_reader.getFieldCount();
    sal_Bool first=sal_True;

    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldName;
    OString fieldType;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

        if (first)
        {
            first = sal_False;
            o << "\n" << indent() << "// Attributes\n";
        }

        o << indent() << "virtual ";
        dumpType(o, fieldType);
        o << " SAL_CALL get" << fieldName << "() throw (::com::sun::star::uno::RuntimeException) = 0;\n";

        if (access != RT_ACCESS_READONLY)
        {
            OString relType = checkSpecialCppuType(fieldType);
            sal_Bool bRef = sal_False;
            sal_Bool bConst = sal_False;

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

            o << indent() << "virtual void SAL_CALL set" << fieldName << "( ";
            dumpType(o, fieldType, bConst, bRef);
            o << " _" << fieldName.toLowerCase() << " ) throw (::com::sun::star::uno::RuntimeException) = 0;\n";
        }
    }
}

void InterfaceType::dumpMethods(FileStream& o)
{
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
        {
            bWithRunTimeExcp = sal_False;
        }

        if (first)
        {
            first = sal_False;
            o << "\n" << indent() << "// Methods\n";
        }

        o << indent() << "virtual ";
        dumpType(o, returnType);
        o << " SAL_CALL " << methodName << "( ";
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
                    OString relType = checkSpecialCppuType(paramType);
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

            dumpType(o, paramType, bConst, bRef);
            o << " " << paramName;

            if (j+1 < (sal_uInt16)paramCount) o << ", ";
        }
        o << " )";

        o << " throw (";
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
            o << "::com::sun::star::uno::RuntimeException";
        }

        o << ") = 0;\n";
    }
}

void InterfaceType::dumpGetCppuType(FileStream& o)
{
    OString typeName(m_typeName.replace('/', '_'));

    if ( m_cppuTypeLeak )
    {
        dumpLGetCppuType(o);
        return;
    }
    if ( !m_cppuTypeDynamic )
    {
        dumpCGetCppuType(o);
        return;
    }

    if ( !m_typeName.equals("com/sun/star/uno/XInterface") )
    {
        o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
          << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
          << "#endif\n\n";
    }

    o << "inline const ::com::sun::star::uno::Type& SAL_CALL getCppuType( ";
    dumpType(o, m_typeName, sal_True, sal_False);
    o << "* ) SAL_THROW( () )\n{\n";
    inc();

    if ( m_typeName.equals("com/sun/star/uno/XInterface") )
    {
        o << indent() << "return * reinterpret_cast< const ::com::sun::star::uno::Type * >( ::typelib_static_type_getByTypeClass("
          << " typelib_TypeClass_INTERFACE ) );\n";
    } else
    {
        o << indent() << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
          << indent() << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
          << indent() << "#endif\n\n";

        o << indent() << "if ( !s_pType_" << typeName << " )\n" << indent() << "{\n";
        inc();
        OString superType(m_reader.getSuperTypeName());
        sal_Bool bWithBase = sal_False;
        if (superType.getLength() > 0 && !superType.equals("com/sun/star/uno/XInterface"))
        {
            bWithBase = sal_True;
            o << indent() << "const ::com::sun::star::uno::Type& rSuperType = getCppuType( ( ";
            dumpType(o, superType, sal_True, sal_False);
            o << " *)0 );\n";
        }

        o << indent() << "typelib_static_interface_type_init( &s_pType_" << typeName
          << ", \"" << m_typeName.replace('/', '.') << "\", ";

        if ( bWithBase )
        {
            o << "rSuperType.getTypeLibType() );\n";
        } else
        {
            o << "0 );\n";
        }

        dec();
        o << indent() << "}\n";
        o << indent() << "return * reinterpret_cast< ::com::sun::star::uno::Type * >( &s_pType_"
          << typeName <<" );\n";
    }
    dec();
    o << indent() << "}\n";

    return;
}

void InterfaceType::dumpCGetCppuType(FileStream& o)
{
    OString typeName(m_typeName.replace('/', '_'));

    o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      <<  "static ::com::sun::star::uno::Type * pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    if (m_cppuTypeStatic)
        o << "static";
    else
        o << "inline";
    o << " const ::com::sun::star::uno::Type& SAL_CALL getCppuType( ";
    dumpType(o, m_typeName, sal_True, sal_False);
    o << "* ) SAL_THROW( () )\n{\n";
    inc();

    o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << indent() << "static ::com::sun::star::uno::Type * pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << indent() << "if ( !pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );\n";

    o << indent() << "if ( !pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "::rtl::OUString sTypeName( RTL_CONSTASCII_USTRINGPARAM(\""
      << m_typeName.replace('/', '.') << "\") );\n\n";

    if ( m_cppuTypeDynamic )
    {
        o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n";
        o << indent() << "static ::com::sun::star::uno::Type aType_" << typeName << "( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << indent() << "pType_" << typeName << " = &aType_" << typeName << ";\n";
        o << "#else\n";
        o << indent() << "pType_" << typeName << " = new ::com::sun::star::uno::Type( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << "#endif\n";
    } else
    {
         o << indent() << "// Start inline typedescription generation\n"
          << indent() << "typelib_InterfaceTypeDescription * pTD = 0;\n\n";

        OString superType(m_reader.getSuperTypeName());
        if (superType.getLength() > 0)
        {
            o << indent() << "const ::com::sun::star::uno::Type& rSuperType = getCppuType( ( ";
            dumpType(o, superType, sal_True, sal_False);
            o << " *)0 );\n";
        }

        sal_uInt32 count = getMemberCount();
        if (count)
        {
            o << indent() << "typelib_TypeDescriptionReference * pMembers[" << count << "] = { ";
            for (sal_uInt32 i = 0; i < count; i++)
            {
                o << "0";
                if (i+1 < count)
                    o << ",";
                else
                    o << " };\n";
            }

            sal_uInt32 index = 0;
            dumpCppuAttributeRefs(o, index);
            dumpCppuMethodRefs(o, index);
        }

        o << "\n" << indent() << "typelib_typedescription_newInterface(\n";
        inc();
        o << indent() << "&pTD,\n"
          << indent() << "sTypeName.pData, ";

        RTUik uik;
        m_reader.getUik(uik);
        sal_Char buffer[53];
        sprintf(buffer, "0x%.8x, 0x%.4x, 0x%.4x, 0x%.8x, 0x%.8x,\n",
                uik.m_Data1, uik.m_Data2, uik.m_Data3, uik.m_Data4, uik.m_Data5);
        o << buffer;

        if (superType.getLength() > 0)
            o << indent() << "rSuperType.getTypeLibType(),\n";
        else
            o << indent() << "0,\n";

        if ( count )
        {
            o << indent() << count << ",\n" << indent() << "pMembers );\n\n";
        } else
        {
            o << indent() << count << ",\n" << indent() << "0 );\n\n";
        }
        dec();

        o << indent() << "typelib_typedescription_register( (typelib_TypeDescription**)&pTD );\n";
        if ( count )
        {
            for (sal_uInt16 i=0; i < count; i++)
            {
                o << indent() << "typelib_typedescriptionreference_release( pMembers["
                  << i << "] );\n";
            }
        }
        o << indent() << "typelib_typedescription_release( (typelib_TypeDescription*)pTD );\n\n";

//      if (superType.getLength() > 0)
//          o << indent() << "typelib_typedescription_release( pSuperTD );\n\n";
//      else
//          o << endl;

        o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n";
        o << indent() << "static ::com::sun::star::uno::Type aType_" << typeName << "( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << indent() << "pType_" << typeName << " = &aType_" << typeName << ";\n";
        o << "#else\n";
        o << indent() << "pType_" << typeName << " = new ::com::sun::star::uno::Type( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << "#endif\n";

        StringSet   aTypes;
        // type for RuntimeException is always needed
        OString     sRunTimeExceptionType("com/sun/star/uno/RuntimeException");
        aTypes.insert(sRunTimeExceptionType);
        dumpCppuGetType(o, sRunTimeExceptionType, sal_True, CPPUTYPEDECL_ALLTYPES);

        dumpAttributesCppuDecl(o, &aTypes, CPPUTYPEDECL_ALLTYPES);
        dumpMethodsCppuDecl(o, &aTypes, CPPUTYPEDECL_ALLTYPES);

        if (count)
        {
            sal_uInt32 index = getInheritedMemberCount();
            dumpCppuAttributes(o, index);
            dumpCppuMethods(o, index);
        }

        o << indent() << "// End inline typedescription generation\n";
    }

    dec();
    o << indent() << "}\n";
    dec();
    o << indent() << "}\n\n"
      << indent() << "return *pType_" << typeName << ";\n";

    dec();
    o << "}\n";
}

void InterfaceType::dumpCppuAttributeRefs(FileStream& o, sal_uInt32& index)
{
    sal_uInt32 fieldCount = m_reader.getFieldCount();

    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldName;
    OString scope = m_typeName.replace('/', '.');

    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);

        o << indent() << "::rtl::OUString sAttributeName" << i << "( RTL_CONSTASCII_USTRINGPARAM(\""
          << scope.replace('/', '.') << "::" << fieldName << "\") );\n";
        o << indent() << "typelib_typedescriptionreference_new( &pMembers["
          << index << "],\n";
        inc(38);
        o << indent() << "(typelib_TypeClass)::com::sun::star::uno::TypeClass_INTERFACE_ATTRIBUTE,\n"
          << indent() << "sAttributeName" << i << ".pData );\n";
        dec(38);
        index++;
    }
}

void InterfaceType::dumpCppuMethodRefs(FileStream& o, sal_uInt32& index)
{
    sal_uInt32  methodCount = m_reader.getMethodCount();
    OString     methodName; //, returnType, paramType, paramName;
    OString     scope = m_typeName.replace('/', '.');

    for (sal_uInt16 i = 0; i < methodCount; i++)
    {
        methodName = m_reader.getMethodName(i);

        o << indent() << "::rtl::OUString sMethodName" << i << "( RTL_CONSTASCII_USTRINGPARAM(\""
          << scope.replace('/', '.') << "::" << methodName << "\") );\n";
        o << indent() << "typelib_typedescriptionreference_new( &pMembers["
          << index << "],\n";
        inc(38);
        o << indent() << "(typelib_TypeClass)::com::sun::star::uno::TypeClass_INTERFACE_METHOD,\n"
          << indent() << "sMethodName" << i << ".pData );\n";
        dec(38);
        index++;
    }
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
            {
                count++;
            }
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

void InterfaceType::dumpCppuAttributes(FileStream& o, sal_uInt32& index)
{
    sal_uInt32 fieldCount = m_reader.getFieldCount();

    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldName;
    OString fieldType;
    OString scope = m_typeName.replace('/', '.');

    sal_uInt32 absoluteIndex = index;

    if (m_hasAttributes)
    {
        o << "\n" << indent() << "typelib_InterfaceAttributeTypeDescription * pAttribute = 0;\n";

        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldAccess(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            fieldName = m_reader.getFieldName(i);
            fieldType = checkRealBaseType(m_reader.getFieldType(i), sal_True);

            o << indent() << "::rtl::OUString sAttributeType" << i << "( RTL_CONSTASCII_USTRINGPARAM(\""
              << fieldType.replace('/', '.') << "\") );\n";
            o << indent() << "typelib_typedescription_newInterfaceAttribute( &pAttribute,\n";
            inc();
            o << indent() << absoluteIndex++ << ", sAttributeName" << i << ".pData,\n";
            o << indent() << "(typelib_TypeClass)" << getTypeClass(fieldType)
              << ", sAttributeType" << i << ".pData,\n";
            if (access == RT_ACCESS_READONLY)
                o << indent() << "sal_True );\n";
            else
                o << indent() << "sal_False );\n";
            dec();
            o << indent() << "typelib_typedescription_register( (typelib_TypeDescription**)&pAttribute );\n\n";
        }
         o << indent() << "typelib_typedescription_release( (typelib_TypeDescription*)pAttribute );\n";

        index = absoluteIndex;
    }
}

void InterfaceType::dumpCppuMethods(FileStream& o, sal_uInt32& index)
{
    sal_uInt32      methodCount = m_reader.getMethodCount();
    OString         methodName, returnType, paramType, paramName;
    OString         scope = m_typeName.replace('/', '.');
    sal_uInt32      paramCount = 0;
    sal_uInt32      excCount = 0;
    RTMethodMode    methodMode = RT_MODE_INVALID;
    RTParamMode     paramMode = RT_PARAM_INVALID;

    sal_uInt32 absoluteIndex = index;

    if (m_hasMethods)
    {
        o << "\n" << indent() << "typelib_InterfaceMethodTypeDescription * pMethod = 0;\n";

        for (sal_uInt16 i=0; i < methodCount; i++)
        {
            methodName = m_reader.getMethodName(i);
            returnType = checkRealBaseType(m_reader.getMethodReturnType(i), sal_True);
            paramCount = m_reader.getMethodParamCount(i);
            excCount = m_reader.getMethodExcCount(i);
            methodMode = m_reader.getMethodMode(i);

            o << indent() << "{\n";
            inc();

            if (paramCount)
            {
                o << indent() << "typelib_Parameter_Init aParameters[" << paramCount << "];\n";
            }

            sal_uInt16 j;
            for (j=0; j < paramCount; j++)
            {
                paramName = m_reader.getMethodParamName(i, j);
                paramType = checkRealBaseType(m_reader.getMethodParamType(i, j), sal_True);
                paramMode = m_reader.getMethodParamMode(i, j);

                o << indent() << "::rtl::OUString sParamName" << j << "( RTL_CONSTASCII_USTRINGPARAM(\""
                  << scope << "::" << paramName << "\") );\n";
                o << indent() << "::rtl::OUString sParamType" << j << "( RTL_CONSTASCII_USTRINGPARAM(\""
                  << paramType.replace('/', '.') << "\") );\n";
                o << indent() << "aParameters[" << j << "].pParamName = sParamName" << j << ".pData;\n";
                o << indent() << "aParameters[" << j << "].eTypeClass = (typelib_TypeClass)"
                  << getTypeClass(paramType) << ";\n";
                o << indent() << "aParameters[" << j << "].pTypeName = sParamType" << j << ".pData;\n";

                if (paramMode == RT_PARAM_IN || paramMode == RT_PARAM_INOUT)
                    o << indent() << "aParameters[" << j << "].bIn = sal_True;\n";
                else
                    o << indent() << "aParameters[" << j << "].bIn = sal_False;\n";

                if (paramMode == RT_PARAM_OUT || paramMode == RT_PARAM_INOUT)
                    o << indent() << "aParameters[" << j << "].bOut = sal_True;\n";
                else
                    o << indent() << "aParameters[" << j << "].bOut = sal_False;\n";
            }

            o << indent() << "rtl_uString * pExceptions[" << excCount + 1 << "];\n";
              for (j=0; j < excCount; j++)
            {
                if (!m_reader.getMethodExcType(i, j).equals("com/sun/star/uno/RuntimeException"))
                {
                    o << indent() << "::rtl::OUString sExceptionName" << j << "( RTL_CONSTASCII_USTRINGPARAM(\""
                      << OString(m_reader.getMethodExcType(i, j)).replace('/', '.') << "\") );\n";
                    o << indent() << "pExceptions[" << j << "] = sExceptionName" << j << ".pData;\n";
                }
            }
            o << indent() << "::rtl::OUString sExceptionName" << excCount << "( RTL_CONSTASCII_USTRINGPARAM("
              << "\"com.sun.star.uno.RuntimeException\") );\n";

            o << indent() << "pExceptions[" << excCount << "] = sExceptionName" << excCount << ".pData;\n";

            o << indent() << "::rtl::OUString sReturnType" << i << "( RTL_CONSTASCII_USTRINGPARAM(\""
              << returnType.replace('/', '.') << "\") );\n";
            o << indent() << "typelib_typedescription_newInterfaceMethod( &pMethod,\n";
            inc();
            o << indent() << absoluteIndex++ << ", ";
            if (methodMode == RT_MODE_ONEWAY || methodMode == RT_MODE_ONEWAY_CONST)
                o << "sal_True,\n";
            else
                o << "sal_False,\n";
            o << indent() << "sMethodName" << i << ".pData,\n";
            o << indent() << "(typelib_TypeClass)" << getTypeClass(returnType)
              << ", sReturnType" << i << ".pData,\n";
            if (paramCount)
                o << indent() << paramCount << ", aParameters,\n";
            else
                o << indent() << "0, 0,\n";

            o << indent() << excCount + 1 << ", pExceptions );\n";
            dec();
            o << indent() << "typelib_typedescription_register( (typelib_TypeDescription**)&pMethod );\n";

            dec();
            o << indent() << "}\n";
        }
        o << indent() << "typelib_typedescription_release( (typelib_TypeDescription*)pMethod );\n";

        index = absoluteIndex;
    }
}

void InterfaceType::dumpAttributesCppuDecl(FileStream& o, StringSet* pFinishedTypes, CppuTypeDecl eDeclFlag)
{
    sal_uInt32 fieldCount = m_reader.getFieldCount();

    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldName;
    OString fieldType;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

        if (pFinishedTypes->count(fieldType) == 0)
        {
            pFinishedTypes->insert(fieldType);
            dumpCppuGetType(o, fieldType, sal_True, eDeclFlag);
        }
    }
}

void InterfaceType::dumpMethodsCppuDecl(FileStream& o, StringSet* pFinishedTypes, CppuTypeDecl eDeclFlag)
{
    sal_uInt32      methodCount = m_reader.getMethodCount();
    OString         returnType, paramType, excType;
    sal_uInt32      paramCount = 0;
    sal_uInt32      excCount = 0;

    for (sal_uInt16 i=0; i < methodCount; i++)
    {
        returnType = m_reader.getMethodReturnType(i);
        paramCount = m_reader.getMethodParamCount(i);
        excCount = m_reader.getMethodExcCount(i);

        if (pFinishedTypes->count(returnType) == 0)
        {
            pFinishedTypes->insert(returnType);
            dumpCppuGetType(o, returnType, sal_True, eDeclFlag);
        }
        sal_uInt16 j;
        for (j=0; j < paramCount; j++)
        {
            paramType = m_reader.getMethodParamType(i, j);

            if (pFinishedTypes->count(paramType) == 0)
            {
                pFinishedTypes->insert(paramType);
                dumpCppuGetType(o, paramType, sal_True, eDeclFlag);
            }
        }

        for (j=0; j < excCount; j++)
        {
            excType = m_reader.getMethodExcType(i, j);
            if (pFinishedTypes->count(excType) == 0)
            {
                pFinishedTypes->insert(excType);
                dumpCppuGetType(o, excType, sal_True, eDeclFlag);
            }
        }
    }
}

//*************************************************************************
// ModuleType
//*************************************************************************
ModuleType::ModuleType(TypeReader& typeReader,
                        const OString& typeName,
                       const TypeManager& typeMgr,
                       const TypeDependency& typeDependencies)
    : CppuType(typeReader, typeName, typeMgr, typeDependencies)
{
}

ModuleType::~ModuleType()
{

}

sal_Bool ModuleType::dump(CppuOptions* pOptions)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_False;

    if (pOptions->isValid("-U"))
        m_cppuTypeDynamic = sal_True;

    OString outPath;
    if (pOptions->isValid("-O"))
        outPath = pOptions->getOption("-O");

    OString tmpName(m_typeName);

    if (tmpName.equals("/"))
        tmpName = "global";
    else
        tmpName += "/" + m_typeName.getToken(m_typeName.getTokenCount('/') - 1, '/');

    OString tmpFileName;
    OString hFileName = createFileNameFromType(outPath, tmpName, ".hdl");

    sal_Bool bFileExists = sal_False;
    sal_Bool bFileCheck = sal_False;

    if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
    {
        bFileExists = fileExists( hFileName );
        ret = sal_True;
    }

    if ( bFileExists && pOptions->isValid("-Gc") )
    {
        tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tml");
        bFileCheck = sal_True;
    }

    if ( !bFileExists || bFileCheck )
    {
        FileStream hFile;

        if ( bFileCheck )
            hFile.openFile(tmpFileName);
        else
            hFile.openFile(hFileName);

        if(!hFile.isValid())
        {
            OString message("cannot open ");
            message += hFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpHFile(hFile);

        hFile.closeFile();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(hFileName, tmpFileName);
        }
    }

    OString hxxFileName = createFileNameFromType(outPath, tmpName, ".hpp");

    if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
    {
        bFileExists = fileExists( hFileName );
        ret = sal_True;
    }

    if ( bFileExists && pOptions->isValid("-Gc") )
    {
        tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tmp");
        bFileCheck = sal_True;
    }


    if ( !bFileExists || bFileCheck )
    {
        FileStream hxxFile;

        if ( bFileCheck )
            hxxFile.openFile(tmpFileName);
        else
            hxxFile.openFile(hxxFileName);

        if(!hxxFile.isValid())
        {
            OString message("cannot open ");
            message += hxxFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpHxxFile(hxxFile);

        hxxFile.closeFile();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(hFileName, tmpFileName);
        }
    }

    return ret;
}

sal_Bool ModuleType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    sal_Bool bSpecialDefine = sal_True;

    if (m_reader.getTypeClass() == RT_TYPE_CONSTANTS)
    {
        bSpecialDefine = sal_False;
    }

    OString headerDefine(dumpHeaderDefine(o, "HDL", bSpecialDefine));
    o << endl;

    dumpDefaultHIncludes(o);
    o << endl;
    dumpDepIncludes(o, m_typeName, "hdl");
    o << endl;

    dumpNameSpace(o, sal_True, sal_True);
    o << endl;

    dumpDeclaration(o);
    o << endl;

    dumpNameSpace(o, sal_False, sal_True);
    o << "\n#endif // "<< headerDefine << endl;

    return sal_True;
}

sal_Bool ModuleType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST)
        {
            fieldName = m_reader.getFieldName(i);
            fieldType = m_reader.getFieldType(i);

            o << "static const ";
            dumpType(o, fieldType);
            o << " " << fieldName << " = ";
            dumpConstantValue(o, i);
            o << ";\n";
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

sal_Bool ModuleType::dumpHxxFile(FileStream& o)
    throw( CannotDumpException )
{
    sal_Bool bSpecialDefine = sal_True;

    if (m_reader.getTypeClass() == RT_TYPE_CONSTANTS)
    {
        bSpecialDefine = sal_False;
    }

    OString headerDefine(dumpHeaderDefine(o, "HPP", bSpecialDefine));
    o << endl;

    dumpInclude(o, m_typeName, "hdl", bSpecialDefine);

    o << "\n#endif // "<< headerDefine << endl;

    return sal_True;
}

//*************************************************************************
// ConstantsType
//*************************************************************************
ConstantsType::ConstantsType(TypeReader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr,
                             const TypeDependency& typeDependencies)
    : ModuleType(typeReader, typeName, typeMgr, typeDependencies)
{
}

ConstantsType::~ConstantsType()
{

}

sal_Bool ConstantsType::dump(CppuOptions* pOptions)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_False;

    if (pOptions->isValid("-U"))
        m_cppuTypeDynamic = sal_True;

    OString outPath;
    if (pOptions->isValid("-O"))
        outPath = pOptions->getOption("-O");

    OString tmpFileName;
    OString hFileName = createFileNameFromType(outPath, m_typeName, ".hdl");

    sal_Bool bFileExists = sal_False;
    sal_Bool bFileCheck = sal_False;

    if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
    {
        bFileExists = fileExists( hFileName );
        ret = sal_True;
    }

    if ( bFileExists && pOptions->isValid("-Gc") )
    {
        tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tml");
        bFileCheck = sal_True;
    }

    if ( !bFileExists || bFileCheck )
    {
        FileStream hFile;

        if ( bFileCheck )
            hFile.openFile(tmpFileName);
        else
            hFile.openFile(hFileName);

        if(!hFile.isValid())
        {
            OString message("cannot open ");
            message += hFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpHFile(hFile);

        hFile.closeFile();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(hFileName, tmpFileName);
        }
    }

    OString hxxFileName = createFileNameFromType(outPath, m_typeName, ".hpp");

    if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
    {
        bFileExists = fileExists( hFileName );
        ret = sal_True;
    }

    if ( bFileExists && pOptions->isValid("-Gc") )
    {
        tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tmp");
        bFileCheck = sal_True;
    }

    if ( !bFileExists || bFileCheck )
    {
        FileStream hxxFile;

        if ( bFileCheck )
            hxxFile.openFile(tmpFileName);
        else
            hxxFile.openFile(hxxFileName);

        if(!hxxFile.isValid())
        {
            OString message("cannot open ");
            message += hxxFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpHxxFile(hxxFile);

        hxxFile.closeFile();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(hFileName, tmpFileName);
        }
    }

    return ret;
}

//*************************************************************************
// StructureType
//*************************************************************************
StructureType::StructureType(TypeReader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr,
                             const TypeDependency& typeDependencies)
    : CppuType(typeReader, typeName, typeMgr, typeDependencies)
{
}

StructureType::~StructureType()
{

}

sal_Bool StructureType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HDL"));
    o << endl;

    dumpDefaultHIncludes(o);
    o << endl;
    dumpDepIncludes(o, m_typeName, "hdl");
    o << endl;

    dumpNameSpace(o);

    dumpDeclaration(o);

    dumpNameSpace(o, sal_False);

    o << "\nnamespace com { namespace sun { namespace star { namespace uno {\n"
      << "class Type;\n} } } }\n\n";

    if (m_cppuTypeStatic)
        o << "static";
    else
        o << "inline";
    o << " const ::com::sun::star::uno::Type& SAL_CALL getCppuType( ";
    dumpType(o, m_typeName, sal_True, sal_False);
    o << "* ) SAL_THROW( () );\n\n";

    o << "#endif // "<< headerDefine << endl;

    return sal_True;
}

sal_Bool StructureType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    o << "\n#ifdef SAL_W32\n"
      << "#   pragma pack(push, 8)\n"
      << "#elif defined(SAL_OS2)\n"
      << "#   pragma pack(8)\n"
      << "#endif\n\n";

    o << "struct " << m_name;

    OString superType(m_reader.getSuperTypeName());
    if (superType.getLength() > 0)
        o << " : public " << scopedName(m_typeName, superType);

    o << "\n{\n";
    inc();
    o << indent() << "inline " << m_name << "() SAL_THROW( () );\n\n";

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    sal_uInt16      i=0;
    if (fieldCount > 0 || getInheritedMemberCount() > 0)
    {
        o << indent() << m_name << "(";

        sal_Bool superHasMember = dumpSuperMember(o, superType, sal_True);

        for (i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldAccess(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                            continue;

            fieldName = m_reader.getFieldName(i);
            fieldType = m_reader.getFieldType(i);

            if (superHasMember)
                o << ", ";
            else
                superHasMember = sal_True;

            dumpType(o, fieldType, sal_True, sal_True);
            o << " __" << fieldName;
        }
        o << ") SAL_THROW( () );\n\n";
    }

    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

        o << indent();
        dumpType(o, fieldType);
        o << " " << fieldName << ";\n";
    }

    dec();
    o << "};\n\n";

    o << "#ifdef SAL_W32\n"
      << "#   pragma pack(pop)\n"
      << "#elif defined(SAL_OS2)\n"
      << "#   pragma pack()\n"
      << "#endif\n\n";

    return sal_True;
}

sal_Bool StructureType::dumpHxxFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << endl;

    dumpInclude(o, m_typeName, "hdl");
    o << endl;

    dumpDefaultHxxIncludes(o);
    o << endl;

    dumpDepIncludes(o, m_typeName, "hpp");
    o << endl;

    dumpNameSpace(o);
    o << endl;

    o << "inline " << m_name << "::" << m_name << "() SAL_THROW( () )\n";
    inc();
    OString superType(m_reader.getSuperTypeName());
    sal_Bool first = sal_True;
    if (superType.getLength() > 0)
    {
        o << indent() << ": " << scopedName(m_typeName, superType) << "()\n";
        first = sal_False;
    }

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    sal_uInt16      i = 0;

    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

        if (first)
        {
            first = sal_False;
            o << indent() << ": ";
        } else
            o << indent() << ", ";

        o << fieldName;
        dumpTypeInit(o, fieldType);
        o << endl;
    }
    dec();
    o << "{\n}\n\n";

    if (fieldCount > 0 || getInheritedMemberCount() > 0)
    {
        o << indent() << "inline " << m_name << "::" << m_name << "(";

        sal_Bool superHasMember = dumpSuperMember(o, superType, sal_True);

        for (i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldAccess(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            fieldName = m_reader.getFieldName(i);
            fieldType = m_reader.getFieldType(i);

            if (superHasMember)
                o << ", ";
            else
                superHasMember = sal_True;

            dumpType(o, fieldType, sal_True, sal_True);
            o << " __" << fieldName;
        }
        o << ") SAL_THROW( () )\n";

        inc();
        sal_Bool first = sal_True;
        if (superType.getLength() > 0)
        {
            o << indent() << ": " << scopedName(m_typeName, superType) << "(";
            dumpSuperMember(o, superType, sal_False);
            o << ")\n";
            first = sal_False;
        }

        for (i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldAccess(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            fieldName = m_reader.getFieldName(i);

            if (first)
            {
                first = sal_False;
                o << indent() << ": ";
            } else
                o << indent() << ", ";

            o << fieldName << "(__" << fieldName << ")\n";
        }

        dec();
        o << "{\n}\n\n";
    }

    dumpNameSpace(o, sal_False);

    o << endl;
    dumpGetCppuType(o);

    o << "\n#endif // "<< headerDefine << endl;

    return sal_True;
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
                    dumpType(o, fieldType, sal_True, sal_True);
                    o << " ";
                }
                o << "__" << fieldName;
            }
        }
/*
        RegistryKey     superKey = m_typeMgr.getTypeKey(superType);
        RegValueType    valueType;
        sal_uInt32      valueSize;

        if (!superKey.getValueInfo(OUString(), &valueType, &valueSize))
        {
            sal_uInt8* pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
            RegistryTypeReaderLoader & rReaderLoader = getRegistryTypeReaderLoader();

            if (!superKey.getValue(OUString(), pBuffer) )
            {
                TypeReader aSuperReader(rReaderLoader, pBuffer, valueSize, sal_True);

                hasMember = dumpSuperMember(o, aSuperReader.getSuperTypeName(), bWithType);

                sal_uInt32      fieldCount = aSuperReader.getFieldCount();
                RTFieldAccess   access = RT_ACCESS_INVALID;
                OString         fieldName;
                OString         fieldType;
                for (sal_Int16 i=0; i < fieldCount; i++)
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
                        dumpType(o, fieldType, sal_True, sal_True);
                        o << " ";
                    }
                    o << "__" << fieldName;
                }
            }

            rtl_freeMemory(pBuffer);
        }
*/
    }

    return hasMember;
}

//*************************************************************************
// ExceptionType
//*************************************************************************
ExceptionType::ExceptionType(TypeReader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr,
                             const TypeDependency& typeDependencies)
    : CppuType(typeReader, typeName, typeMgr, typeDependencies)
{
}

ExceptionType::~ExceptionType()
{

}

sal_Bool ExceptionType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HDL"));
    o << endl;

    dumpDefaultHIncludes(o);
    o << endl;
    dumpDepIncludes(o, m_typeName, "hdl");
    o << endl;

    dumpNameSpace(o);

    dumpDeclaration(o);

    dumpNameSpace(o, sal_False);

    o << "\nnamespace com { namespace sun { namespace star { namespace uno {\n"
      << "class Type;\n} } } }\n\n";

    if (m_cppuTypeStatic)
        o << "static";
    else
        o << "inline";
    o << " const ::com::sun::star::uno::Type& SAL_CALL getCppuType( ";
    dumpType(o, m_typeName, sal_True, sal_False);
    o << "* ) SAL_THROW( () );\n\n";

    o << "#endif // "<< headerDefine << endl;

    return sal_True;
}

sal_Bool ExceptionType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    o << "\nclass " << m_name;

    OString superType(m_reader.getSuperTypeName());
    if (superType.getLength() > 0)
                    o << " : public " << scopedName(m_typeName, superType);

    o << "\n{\npublic:\n";
    inc();
    o << indent() << "inline " << m_name << "() SAL_THROW( () );\n\n";

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    sal_uInt16      i = 0;

    if (fieldCount > 0 || getInheritedMemberCount() > 0)
    {
        o << indent() << "inline " << m_name << "(";

        sal_Bool superHasMember = dumpSuperMember(o, superType, sal_True);

        for (i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldAccess(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                            continue;

            fieldName = m_reader.getFieldName(i);
            fieldType = m_reader.getFieldType(i);

            if (superHasMember)
                o << ", ";
            else
                superHasMember = sal_True;

            dumpType(o, fieldType, sal_True, sal_True);
            o << " __" << fieldName;
        }
        o << ") SAL_THROW( () );\n\n";
    }

    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

        o << indent();
        dumpType(o, fieldType);
        o << " " << fieldName << ";\n";
    }


    dec();
    o << "};\n\n";

    return sal_True;
}

sal_Bool ExceptionType::dumpHxxFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << endl;

    dumpInclude(o, m_typeName, "hdl");
    o << endl;

    dumpDefaultHxxIncludes(o);
    o << endl;

    dumpDepIncludes(o, m_typeName, "hpp");
    o << endl;

    dumpNameSpace(o);
    o << endl;

    o << "inline " << m_name << "::" << m_name << "() SAL_THROW( () )\n";
    inc();
    OString superType(m_reader.getSuperTypeName());
    sal_Bool first = sal_True;
    if (superType.getLength() > 0)
    {
        o << indent() << ": " << scopedName(m_typeName, superType) << "()\n";
        first = sal_False;
    }

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    sal_uInt16      i = 0;

    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

        if (first)
        {
            first = sal_False;
            o << indent() << ": ";
        } else
            o << indent() << ", ";

        o << fieldName;
        dumpTypeInit(o, fieldType);
        o << endl;
    }
    dec();
    if ( !m_cppuTypeDynamic )
    {
        o << "{\n";
        inc();
        dumpCppuGetType(o, m_typeName, sal_True);
        dec();
        o << "}\n\n";
    } else
    {
        o << "{ }\n\n";
    }

    if (fieldCount > 0 || getInheritedMemberCount() > 0)
    {
        o << indent() << "inline " << m_name << "::" << m_name << "(";

        sal_Bool superHasMember = dumpSuperMember(o, superType, sal_True);

        for (i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldAccess(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            fieldName = m_reader.getFieldName(i);
            fieldType = m_reader.getFieldType(i);

            if (superHasMember)
                o << ", ";
            else
                superHasMember = sal_True;

            dumpType(o, fieldType, sal_True, sal_True);
            o << " __" << fieldName;
        }
        o << ") SAL_THROW( () )\n";

        inc();
        sal_Bool first = sal_True;
        if (superType.getLength() > 0)
        {
            o << indent() << ": " << scopedName(m_typeName, superType) << "(";
            dumpSuperMember(o, superType, sal_False);
            o << ")\n";
            first = sal_False;
        }

        for (i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldAccess(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            fieldName = m_reader.getFieldName(i);

            if (first)
            {
                first = sal_False;
                o << indent() << ": ";
            } else
                o << indent() << ", ";

            o << fieldName << "(__" << fieldName << ")\n";
        }

        dec();
        if ( !m_cppuTypeDynamic )
        {
            o << "{\n";
            inc();
            dumpCppuGetType(o, m_typeName, sal_True);
            dec();
            o << "}\n\n";
        } else
        {
            o << "{ }\n\n";
        }
    }

    dumpNameSpace(o, sal_False);

    o << endl;
    dumpGetCppuType(o);

    o << "\n#endif // "<< headerDefine << endl;
    return sal_True;
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
                    dumpType(o, fieldType, sal_True, sal_True);
                    o << " ";
                }
                o << "__" << fieldName;
            }
        }
/*
        RegistryKey     superKey = m_typeMgr.getTypeKey(superType);
        RegValueType    valueType;
        sal_uInt32      valueSize;

        if (!superKey.getValueInfo(OUString(), &valueType, &valueSize))
        {
            sal_uInt8* pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
            RegistryTypeReaderLoader & rReaderLoader = getRegistryTypeReaderLoader();

            if (!superKey.getValue(OUString(), pBuffer) )
            {
                TypeReader aSuperReader(rReaderLoader, pBuffer, valueSize, sal_True);

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
                        dumpType(o, fieldType, sal_True, sal_True);
                        o << " ";
                    }
                    o << "__" << fieldName;
                }
            }

            rtl_freeMemory(pBuffer);
        }
*/
    }

    return hasMember;
}

//*************************************************************************
// EnumType
//*************************************************************************
EnumType::EnumType(TypeReader& typeReader,
                    const OString& typeName,
                   const TypeManager& typeMgr,
                   const TypeDependency& typeDependencies)
    : CppuType(typeReader, typeName, typeMgr, typeDependencies)
{
}

EnumType::~EnumType()
{

}

sal_Bool EnumType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HDL"));
    o << endl;

    dumpDefaultHIncludes(o);
    o << endl;

    dumpNameSpace(o);

    dumpDeclaration(o);

    dumpNameSpace(o, sal_False);

    o << "\nnamespace com { namespace sun { namespace star { namespace uno {\n"
      << "class Type;\n} } } }\n\n";

    if (m_cppuTypeStatic)
        o << "static";
    else
        o << "inline";
    o << " const ::com::sun::star::uno::Type& SAL_CALL getCppuType( ";
    dumpType(o, m_typeName, sal_True, sal_False);
    o << "* ) SAL_THROW( () );\n\n";

    o << "#endif // "<< headerDefine << endl;

    return sal_True;
}

sal_Bool EnumType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    o << "\nenum " << m_name << "\n{\n";
    inc();

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    RTConstValue    constValue;
    OString         fieldName;
    sal_uInt32      value=0;
    for (sal_uInt16 i=0; i < fieldCount; i++)
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

        o << indent() << m_name << "_" << fieldName << " = " << value << ",\n";
    }

    o << indent() << m_name << "_MAKE_FIXED_SIZE = SAL_MAX_ENUM\n";

    dec();
    o << "};\n\n";

    return sal_True;
}

sal_Bool EnumType::dumpHxxFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << endl;

    dumpInclude(o, m_typeName, "hdl");
    o << endl;

    dumpDefaultHxxIncludes(o);
    o << endl;

    dumpGetCppuType(o);

    o << "\n#endif // "<< headerDefine << endl;
    return sal_True;
}

void EnumType::dumpGetCppuType(FileStream& o)
{
    OString typeName(m_typeName.replace('/', '_'));

    if ( m_cppuTypeLeak )
    {
        dumpLGetCppuType(o);
        return;
    }
    if ( !m_cppuTypeDynamic )
    {
        dumpCGetCppuType(o);
        return;
    }

    o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << "inline const ::com::sun::star::uno::Type& SAL_CALL getCppuType( ";
    dumpType(o, m_typeName, sal_True, sal_False);
    o << "* ) SAL_THROW( () )\n{\n";
    inc();

    o << indent() << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << indent() << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << indent() << "#endif\n\n";

    o << indent() << "if ( !s_pType_" << typeName << " )\n" << indent() << "{\n";
    inc();

    o << indent() << "typelib_static_enum_type_init( &s_pType_" << typeName << ",\n";
    inc(31);
    o << indent() << "\"" << m_typeName.replace('/', '.') << "\",\n"
      << indent() << scopedName(OString(), m_typeName) << "_" << m_reader.getFieldName(0) << " );\n";
    dec(31);
    dec();
    o << indent() << "}\n";
    o << indent() << "return * reinterpret_cast< ::com::sun::star::uno::Type * >( &s_pType_"
      << typeName <<" );\n";
    dec();
    o << indent() << "}\n";

    return;
}

void EnumType::dumpCGetCppuType(FileStream& o)
{
    OString typeName(m_typeName.replace('/', '_'));

    o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      <<  "static ::com::sun::star::uno::Type * pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    if (m_cppuTypeStatic)
        o << "static";
    else
        o << "inline";
    o << " const ::com::sun::star::uno::Type& SAL_CALL getCppuType( ";
    dumpType(o, m_typeName, sal_True, sal_False);
    o << "* ) SAL_THROW( () )\n{\n";
    inc();

    o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << indent() << "static ::com::sun::star::uno::Type * pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << indent() << "if ( !pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );\n";

    o << indent() << "if ( !pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "::rtl::OUString sTypeName( RTL_CONSTASCII_USTRINGPARAM(\""
      << m_typeName.replace('/', '.') << "\") );\n\n";

    if ( m_cppuTypeDynamic )
    {
        o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n";
        o << indent() << "static ::com::sun::star::uno::Type aType_" << typeName << "( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << indent() << "pType_" << typeName << " = &aType_" << typeName << ";\n";
        o << "#else\n";
        o << indent() << "pType_" << typeName << " = new ::com::sun::star::uno::Type( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << "#endif\n";
    } else
    {
        o << indent() << "// Start inline typedescription generation\n"
          << indent() << "typelib_TypeDescription * pTD = 0;\n\n";

        sal_uInt32 count = m_reader.getFieldCount();
        o << indent() << "rtl_uString* enumValueNames[" << count << "];\n";
        sal_uInt32 i;
        for (i = 0; i < count; i++)
        {
            o << indent() << "::rtl::OUString sEnumValue" << i << "( RTL_CONSTASCII_USTRINGPARAM(\""
                << m_reader.getFieldName((sal_uInt16)i) << "\") );\n";
            o << indent() << "enumValueNames[" << i << "] = sEnumValue" << i << ".pData;\n";
        }

        o << "\n" << indent() << "sal_Int32 enumValues[" << count << "];\n";
        RTConstValue    constValue;
        sal_Int32       value=0;
        for (i = 0; i < count; i++)
        {
            o << indent() << "enumValues[" << i << "] = ";
            constValue = m_reader.getFieldConstValue((sal_uInt16)i);
            if (constValue.m_type == RT_TYPE_INT32)
                value = constValue.m_value.aLong;
            else
                value++;
            o << value << ";\n";
        }

        o << "\n" << indent() << "typelib_typedescription_newEnum( &pTD,\n";
        inc();
        o << indent() << "sTypeName.pData,\n"
          << indent() << "(sal_Int32)" << scopedName("", m_typeName, sal_False)
          << "_" << m_reader.getFieldName(0) << ",\n"
          << indent() << count << ", enumValueNames, enumValues );\n\n";
        dec();

        o << indent() << "typelib_typedescription_register( (typelib_TypeDescription**)&pTD );\n";
        o << indent() << "typelib_typedescription_release( pTD );\n"
          << indent() << "// End inline typedescription generation\n\n";

        o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n";
        o << indent() << "static ::com::sun::star::uno::Type aType_" << typeName << "( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << indent() << "pType_" << typeName << " = &aType_" << typeName << ";\n";
        o << "#else\n";
        o << indent() << "pType_" << typeName << " = new ::com::sun::star::uno::Type( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << "#endif\n";
    }

    dec();
    o << indent() << "}\n";
    dec();
    o << indent() << "}\n\n"
      << indent() << "return *pType_" << typeName << ";\n";

    dec();
    o << "}\n";
}

//*************************************************************************
// TypeDefType
//*************************************************************************
TypeDefType::TypeDefType(TypeReader& typeReader,
                             const OString& typeName,
                            const TypeManager& typeMgr,
                            const TypeDependency& typeDependencies)
    : CppuType(typeReader, typeName, typeMgr, typeDependencies)
{
}

TypeDefType::~TypeDefType()
{

}

sal_Bool TypeDefType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HDL"));
    o << endl;

    dumpDefaultHIncludes(o);
    o << endl;
    dumpDepIncludes(o, m_typeName, "hdl");
    o << endl;

    dumpNameSpace(o);

    dumpDeclaration(o);

    dumpNameSpace(o, sal_False);

//  o << "\nnamespace com { namespace sun { namespace star { namespace uno {\n"
//    << "class Type;\n} } } }\n\n";
//  o << "inline const ::com::sun::star::uno::Type& SAL_CALL get_" << m_typeName.replace('/', '_')
//    <<  "_Type( ) SAL_THROW( () );\n\n";

    o << "#endif // "<< headerDefine << endl;

    return sal_True;
}

sal_Bool TypeDefType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    o << "\ntypedef ";
    dumpType(o, m_reader.getSuperTypeName());
    o << " " << m_name << ";\n\n";

    return sal_True;
}

sal_Bool TypeDefType::dumpHxxFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << endl;

    dumpInclude(o, m_typeName, "hdl");
    o << endl;
    dumpDefaultHxxIncludes(o);
    o << endl;
    dumpDepIncludes(o, m_typeName, "hpp");
    o << endl;

    dumpGetCppuType(o);

    o << "\n#endif // "<< headerDefine << endl;
    return sal_True;
}

void TypeDefType::dumpGetCppuType(FileStream& o)
{
    if ( m_cppuTypeLeak )
    {
        dumpLGetCppuType(o);
        return;
    }
    if ( !m_cppuTypeDynamic )
    {
        dumpCGetCppuType(o);
        return;
    }
}

void TypeDefType::dumpCGetCppuType(FileStream& o)
{
/*
    OString typeName(m_typeName.replace('/', '_'));

    if (m_cppuTypeLeak)
    {
        dumpLGetCppuType(o);
        return;
    }

    o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      <<  "static ::com::sun::star::uno::Type * pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << "inline const ::com::sun::star::uno::Type& SAL_CALL get_" << typeName << "_Type( ) SAL_THROW( () )\n{\n";
    inc();

    o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << indent() << "static ::com::sun::star::uno::Type * pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << indent() << "if ( !pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );\n";

    o << indent() << "if ( !pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "::rtl::OUString sTypeName( RTL_CONSTASCII_USTRINGPARAM(\""
      << m_typeName.replace('/', '.') << "\") );\n\n";

    if ( m_cppuTypeDynamic )
    {
        o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n";
        o << indent() << "static ::com::sun::star::uno::Type aType_" << typeName << "( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << indent() << "pType_" << typeName << " = &aType_" << typeName << ";\n";
        o << "#else\n";
        o << indent() << "pType_" << typeName << " = new ::com::sun::star::uno::Type( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << "#endif\n";
    } else
    {
        o << indent() << "// Start inline typedescription generation\n"
          << indent() << "typelib_TypeDescription * pTD = 0;\n";

        OString superType(m_reader.getSuperTypeName());
        o << indent() << "typelib_TypeDescription * pITD = 0;\n";

          o << indent() << "getCppuType( ( ";
        dumpType(o, superType, sal_True, sal_False);
        o << " *)0 ).getDescription( &pITD );\n\n";

        o << indent() << "typelib_typedescription_new(\n";
        inc();
        o << indent() << "&pTD,\n"
          << indent() << "(typelib_TypeClass)::com::sun::star::uno::TypeClass_TYPEDEF, "
          << "sTypeName.pData,\n";

        o << indent() << "pITD,\n" << indent() << "0, 0 );\n\n";
        dec();
        o << indent() << "typelib_typedescription_register( (typelib_TypeDescription**)&pTD );\n";
        o << indent() << "typelib_typedescription_release( pITD );\n"
          << indent() << "typelib_typedescription_release( pTD );\n"
          << indent() << "// End inline typedescription generation\n\n";

        o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n";
        o << indent() << "static ::com::sun::star::uno::Type aType_" << typeName << "( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << indent() << "pType_" << typeName << " = &aType_" << typeName << ";\n";
        o << "#else\n";
        o << indent() << "pType_" << typeName << " = new ::com::sun::star::uno::Type( "
          << getTypeClass(m_typeName) << ", sTypeName );\n";
        o << "#endif\n";
    }

    dec();
    o << indent() << "}\n";
    dec();
    o << indent() << "}\n\n"
      << indent() << "return *pType_" << typeName << ";\n";
    dec();
    o << "}\n";
*/
}

void TypeDefType::dumpLGetCppuType(FileStream& o)
{
/*
    OString typeName(m_typeName.replace('/', '_'));

    o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << "inline const ::com::sun::star::uno::Type& SAL_CALL get_" << typeName << "_Type( ) SAL_THROW( () )\n{\n";
    inc();

    o << indent() << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << indent() << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << indent() << "#endif\n\n";

    o << indent() << "if ( !s_pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "const ::com::sun::star::uno::Type& rType = getCppuType( ( ";
    dumpType(o, m_reader.getSuperTypeName(), sal_True, sal_False);
    o << " *)0 );\n\n";

    o << indent() << "::typelib_static_typedef_type_init( &s_pType_" << typeName
      << ", \"" << m_typeName.replace('/', '.') << "\", rType.getTypeLibType() );\n";
    dec();
    o << indent() << "}\n";
    o << indent() << "return * reinterpret_cast< ::com::sun::star::uno::Type * >( &s_pType_"
      << typeName <<" );\n";
    dec();
    o << indent() << "}\n";
*/
}

//*************************************************************************
// produceType
//*************************************************************************
sal_Bool produceType(const OString& typeName,
                     TypeManager& typeMgr,
                     TypeDependency& typeDependencies,
                     CppuOptions* pOptions)
    throw( CannotDumpException )
{
    if (typeDependencies.isGenerated(typeName))
        return sal_True;

    TypeReader reader(typeMgr.getTypeReader(typeName));

    if (!reader.isValid())
    {
        if (typeName.equals("/"))
            return sal_True;
        else
            return sal_False;
    }

/*  RegistryKey     typeKey = typeMgr.getTypeKey(typeName);

    if (!typeKey.isValid())
        return sal_False;
*/
    if( !checkTypeDependencies(typeMgr, typeDependencies, typeName))
        return sal_False;
/*
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
//                  ret = mType.dumpDependedTypes(pOptions);
                } else
                {
                    typeDependencies.setGenerated(typeName);
                    ret = sal_True;
                }
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
        case RT_TYPE_TYPEDEF:
            {
                TypeDefType tdType(reader, typeName, typeMgr, typeDependencies);
                ret = tdType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                ret = tdType.dumpDependedTypes(pOptions);
            }
            break;
        case RT_TYPE_CONSTANTS:
            {
                ConstantsType cType(reader, typeName, typeMgr, typeDependencies);
                if (cType.hasConstants())
                {
                    ret = cType.dump(pOptions);
                    if (ret) typeDependencies.setGenerated(typeName);
//                  ret = cType.dumpDependedTypes(pOptions);
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

    return ret;
}

//*************************************************************************
// scopedName
//*************************************************************************
OString scopedName(const OString& scope, const OString& type,
                   sal_Bool bNoNameSpace)
{
    sal_uInt32 count = type.getTokenCount('/');
    sal_uInt32 offset = 0;
/*
    if (count > 1)
        offset = count - 1;
    else
        return type;
*/
    if (count == 1)
        return type;

    if (bNoNameSpace)
        return type.getToken(count - 1, '/');

    // scoped name only if the namespace is not equal
/*
    if (scope.lastIndexOf('/') > 0)
    {
        OString tmpScp(scope.copy(0, scope.lastIndexOf('/')));
        OString tmpScp2(type.copy(0, type.lastIndexOf('/')));

        if (tmpScp == tmpScp2)
            return type.getToken(count - 1, '/');
    }
*/

//  OStringBuffer tmpBuf(type.getLength() + offset);
//  tmpBuf.append(type.getToken(0, '/'));
//  for (int i=1; i < count; i++)
    OStringBuffer tmpBuf(type.getLength() + count);
    for (sal_uInt32 i=0; i < count; i++)
    {
        tmpBuf.append("::");
        tmpBuf.append(type.getToken(i, '/'));
    }

    return tmpBuf.makeStringAndClear();
}

//*************************************************************************
// shortScopedName
//*************************************************************************
OString shortScopedName(const OString& scope, const OString& type,
                           sal_Bool bNoNameSpace)
{
    sal_uInt32 count = type.getTokenCount('/');
    sal_uInt32 offset = 0;

    if (count > 1)
//      offset = count - 2;
        offset = count - 1;
    else
        return OString();

    if (bNoNameSpace)
        return OString();

    // scoped name only if the namespace is not equal
    if (scope.lastIndexOf('/') > 0)
    {
        OString tmpScp(scope.copy(0, scope.lastIndexOf('/')));
        OString tmpScp2(type.copy(0, type.lastIndexOf('/')));

        if (tmpScp == tmpScp2)
            return OString();
    }

    OStringBuffer tmpBuf(type.lastIndexOf('/') + offset);

    for (sal_uInt32 i=0; i < count - 1; i++)
    {
        tmpBuf.append("::");
        tmpBuf.append(type.getToken(i, '/'));
    }

    return tmpBuf.makeStringAndClear();
}


