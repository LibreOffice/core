/*************************************************************************
 *
 *  $RCSfile: cunotype.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 15:53:42 $
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

#include    "cunotype.hxx"
#include    "cunooptions.hxx"

using namespace rtl;

//*************************************************************************
// CunoType
//*************************************************************************
CunoType::CunoType(TypeReader& typeReader,
                   const OString& typeName,
                   const TypeManager& typeMgr,
                   const TypeDependency& typeDependencies)
    : m_inheritedMemberCount(0)
    , m_cunoTypeLib(sal_False)
    , m_cunoTypeLeak(sal_False)
    , m_cunoTypeDynamic(sal_True)
    , m_indentLength(0)
    , m_typeName(typeName)
//  , m_name(typeName.getToken(typeName.getTokenCount('/') - 1, '/'))
    , m_name(typeName.replace('/', '_'))
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

CunoType::~CunoType()
{

}

sal_Bool CunoType::isNestedTypeByName(const ::rtl::OString& type)
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

sal_Bool CunoType::hasNestedType(const ::rtl::OString& type)
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

sal_Bool CunoType::dump(CunoOptions* pOptions)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_False;

    if (isNestedType())
        return sal_True;

    if (pOptions->isValid("-U"))
        m_cunoTypeLib = sal_True;
    if (pOptions->isValid("-L"))
        m_cunoTypeLeak = sal_True;
    if (pOptions->isValid("-C"))
        m_cunoTypeDynamic = sal_False;

    OString outPath;
    if (pOptions->isValid("-O"))
        outPath = pOptions->getOption("-O");

    OString tmpFileName;
    OString hFileName = createFileNameFromType(outPath, m_typeName, ".h");

    sal_Bool bFileExists = sal_False;
    sal_Bool bFileCheck = sal_False;

    if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
    {
        bFileExists = fileExists( hFileName );
        ret = sal_True;
    }

    if ( bFileExists && pOptions->isValid("-Gc") )
    {
        tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tmh");
        bFileCheck = sal_True;
    }

    if ( !bFileExists || bFileCheck )
    {
        FileStream hFile;

        if ( bFileCheck )
            hFile.open(tmpFileName);
        else
            hFile.open(hFileName);

        if(!hFile.isValid())
        {
            OString message("cannot open ");
            message += hFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpHFile(hFile);

        hFile.close();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(hFileName, tmpFileName);
        }
    }

    if ( m_cunoTypeLib )
    {
        bFileExists = sal_False;
        bFileCheck = sal_False;

        if (pOptions->isValid("-OC"))
            outPath = pOptions->getOption("-OC");
        else
            outPath = OString();

        OString cFileName = createFileNameFromType(outPath, m_typeName, ".c");

        if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
        {
            bFileExists = fileExists( cFileName );
            ret = sal_True;
        }

        if ( bFileExists && pOptions->isValid("-Gc") )
        {
            tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tmc");
            bFileCheck = sal_True;
        }

        if ( !bFileExists || bFileCheck )
        {
            FileStream cFile;

            if ( bFileCheck )
                cFile.open(tmpFileName);
            else
                cFile.open(cFileName);

            if(!cFile.isValid())
            {
                OString message("cannot open ");
                message += cFileName + " for writing";
                throw CannotDumpException(message);
            }

            ret = dumpCFile(cFile);

            cFile.close();
            if (ret && bFileCheck)
            {
                ret = checkFileContent(cFileName, tmpFileName);
            }
        }
    }
    return ret;
}
sal_Bool CunoType::dumpDependedTypes(CunoOptions* pOptions)
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
        ++iter;
    }

    return ret;
}

OString CunoType::dumpHeaderDefine(FileStream& o, sal_Char* prefix, sal_Bool bExtended)
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

    OString tmp(tmpBuf.makeStringAndClear().replace('/', '_').toAsciiUpperCase());

    o << "#ifndef " << tmp << "\n#define " << tmp << "\n";

    return tmp;
}

void CunoType::dumpDefaultHIncludes(FileStream& o)
{
    o << "#ifndef _UNO_CUNO_H_\n"
      << "#include <uno/cuno.h>\n"
      << "#endif\n";
/*
    if (m_typeMgr.getTypeClass(m_typeName) == RT_TYPE_INTERFACE &&
        !m_typeName.equals("com/sun/star/uno/XInterface") )
    {
        o << "#ifndef _COM_SUN_STAR_UNO_XINTERFACE_H_\n"
          << "#include <com/sun/star/uno/XInterface.h>\n"
          << "#endif\n";
    }
*/
}

void CunoType::dumpDefaultCIncludes(FileStream& o)
{
    o << "#ifndef _OSL_MUTEX_H_\n"
      << "#include <osl/mutex.h>\n"
      << "#endif\n\n";
}

void CunoType::dumpInclude(FileStream& o, const OString& typeName, sal_Char* prefix, sal_Bool bExtended, sal_Bool bCaseSensitive)
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
    o << ">\n#endif\n";
}

void CunoType::dumpDepIncludes(FileStream& o, const OString& typeName, sal_Char* prefix)
{
    TypeUsingSet usingSet(m_dependencies.getDependencies(typeName));

    TypeUsingSet::const_iterator iter = usingSet.begin();

    OString     sPrefix(OString(prefix).toAsciiUpperCase());
    sal_Bool    bSequenceDumped = sal_False;
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
            OString defPrefix("H");
            if (sPrefix.equals("H"))
                defPrefix = "H";

            if (seqNum > 0 && !bSequenceDumped)
            {
                bSequenceDumped = sal_True;
                o << "#ifndef _UNO_SEQUENCE2_" << defPrefix
                  << "_\n#include <uno/sequence2." << defPrefix.toAsciiLowerCase()
                  << ">\n#endif\n";
            }

            if (getBaseType(relType).getLength() == 0 &&
                m_typeName != relType)
            {
                if (m_typeMgr.getTypeClass(relType) == RT_TYPE_INTERFACE
                    && sPrefix.equals("H"))
                {
                    if (!((*iter).m_use & TYPEUSE_SUPER))
                    {
                        if (isNestedTypeByName(relType))
                        {
                            sal_Int32 iLastS = relType.lastIndexOf('/');

                            OString outerNamespace(relType.copy(0,iLastS));
                            OString innerClass(relType.copy(iLastS+1));

                            iLastS = outerNamespace.lastIndexOf('/');
                            OString outerClass(outerNamespace.copy(iLastS+1));

//                          o << "\n";
//                          dumpNameSpace(o, sal_True, sal_False, outerNamespace);
//                          o << "\nclass " << outerClass << "::" << innerClass << ";\n";
//                          dumpNameSpace(o, sal_False, sal_False, outerNamespace);
//                          o << "\n\n";
                        }
                        else
                        {
//                          dumpInclude(o, relType, prefix);
                            OString type(relType.replace('/', '_'));
                            o << "\n#ifndef " << type.toAsciiUpperCase() << "\n";
                            o << "#define " << type.toAsciiUpperCase() << "\n";
                            o << "struct _" << type << ";\n"
                              << "typedef struct _" << type << "_ftab * " << type << ";\n";
                            o << "#endif\n\n";
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
                o << "#ifndef _UNO_ANY2_H_\n"
                  << "#include <uno/any2.h>\n"
                  << "#endif\n";
            } else
            if (relType == "type")
            {
                o << "#ifndef _TYPELIB_TYPEDESCRIPTION_H_\n"
                  << "#include <typelib/typedescription.h>\n"
                  << "#endif\n";
            } else
            if (relType == "string" && sPrefix.equals("H"))
            {
                o << "#ifndef _RTL_USTRING_H_\n"
                  << "#include <rtl/ustring.h>\n"
                  << "#endif\n";
            }
        }
        ++iter;
    }
    if (m_typeName.equals(typeName) && (getNestedTypeNames().getLength() > 0))
    {
        o << "// includes for nested types\n\n";

        for (sal_uInt32 i = 0; i < getNestedTypeNames().getLength(); i++)
        {
            OUString s(getNestedTypeNames().getElement(i));

            OString nestedName(s.getStr(), s.getLength(), RTL_TEXTENCODING_UTF8);

            dumpDepIncludes(o, nestedName, prefix);
        }
    }
}

void CunoType::dumpOpenExternC(FileStream& o)
{
    o << "#ifdef __cplusplus\n"
      << "extern \"C\" {\n"
      << "#endif\n\n";
}

void CunoType::dumpCloseExternC(FileStream& o)
{
    o << "#ifdef __cplusplus\n"
      << "}\n"
      << "#endif\n\n";
}

void CunoType::dumpLGetCunoType(FileStream& o)
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
        o << "inline const ::com::sun::star::uno::Type& SAL_CALL getCunoType( ";
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

void CunoType::dumpGetCunoType(FileStream& o)
{
    OString typeName(m_typeName.replace('/', '_'));

    if ( m_cunoTypeLeak )
    {
        dumpLGetCunoType(o);
        return;
    }
    if ( !m_cunoTypeDynamic )
    {
        dumpCGetCunoType(o);
        return;
    }

    dumpOpenExternC(o);

    if ( !m_typeName.equals("com/sun/star/uno/Exception") )
    {
        o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
          << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
          << "#endif\n\n";
    }

    o << "typelib_TypeDescriptionReference ** SAL_CALL getCUnoType_" << m_name << "() SAL_THROW_EXTERN_C( () )\n{\n";
    inc();

    if ( m_typeName.equals("com/sun/star/uno/Exception") )
    {
        o << indent() << "return typelib_static_type_getByTypeClass( typelib_TypeClass_EXCEPTION );\n";
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
                o << indent() << "typelib_TypeDescriptionReference * pBaseType = 0;\n";
            }
        }

        sal_uInt32 count = getMemberCount();
        if (count)
        {
            o << indent() << "typelib_TypeDescriptionReference * aMemberRefs[" << count << "];\n";
        }

        if ( !bIsBaseException )
        {
            o << indent() << "typelib_typedescriptionreference_newByAsciiName(&pBaseType, typelib_TypeClass_INTERFACE, \""
              << superType.replace('/', '.') << "\" );\n";
        }

        if (count)
        {
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

//              modFieldType = typeToIdentifier(fieldType);

                findIter = generatedTypeSet.find(fieldType);
                if ( findIter == generatedTypeSet.end() )
                {
                    generatedTypeSet.insert(fieldType);
                    o << indent() << "typelib_typedescriptionreference_newByAsciiName(&aMemberRefs["
                      << i << "], " << getTypeClass(fieldType, sal_True);
                    o << "  , \"" << fieldType.replace('/', '.') << "\" );\n";
                }
            }
            o << "\n";
        }

        o << indent() << "typelib_static_compound_type_init( &s_pType_" << typeName << ", "
          << getTypeClass(m_typeName, sal_True) << ", \"" << m_typeName.replace('/', '.') << "\", ";
        if ( superType.getLength() > 0 || bIsBaseException )
        {
            if ( bIsBaseException )
            {
                o << "* typelib_static_type_getByTypeClass( typelib_TypeClass_EXCEPTION ), "
                  << count << ", ";
            } else
            {
                o << "pBaseType, " << count << ", ";
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
        o << indent() << "}\n"
          << indent() << "typelib_typedescriptionreference_acquire( s_pType_" << typeName << " );\n"
          << indent() << "return &_pType_" << typeName <<" );\n";
    }
    dec();
    o << indent() << "}\n";

    dumpCloseExternC(o);
}

void CunoType::dumpCGetCunoType(FileStream& o)
{
    OString typeName(m_typeName.replace('/', '_'));

    dumpOpenExternC(o);

    o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << "typelib_TypeDescriptionReference ** SAL_CALL getCUnoType_" << m_name << "() SAL_THROW_EXTERN_C( () )\n{\n";
    inc();

    o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << indent() << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << indent() << "if ( !s_pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "oslMutex * pMutex = osl_getGlobalMutex();\n"
      << indent() << "osl_acquireMutex( pMutex );\n";

    o << indent() << "if ( !s_pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "rtl_uString * pTypeName = 0;\n"
       << indent() << "typelib_TypeDescription * pTD = 0;\n";

    OString superType(m_reader.getSuperTypeName());
    if (superType.getLength() > 0)
        o << indent() << "typelib_TypeDescriptionReference * pSuperType = 0;\n";

    sal_uInt32      count = getMemberCount();
    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    if (count)
    {
        o << indent() << "typelib_CompoundMember_Init aMembers["
          << count << "];\n";

        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldAccess(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            o << indent() << "rtl_uString * pMemberName" << i << " = 0;\n"
              << indent() << "rtl_uString * pMemberType" << i << " = 0;\n";
        }
    }

    o << indent() << "rtl_uString_newFromAscii( &pTypeName, \"" << m_typeName.replace('/', '.') << "\" );\n";

    if (superType.getLength() > 0)
    {
        o << indent() << "typelib_typedescriptionreference_newByAsciiName(&pSuperType, typelib_TypeClass_INTERFACE, \""
          << superType.replace('/', '.') << "\" );\n";
    }

    dumpCppuGetTypeMemberDecl(o, CUNOTYPEDECL_ALLTYPES);

    if (count)
    {
        OString         fieldType, fieldName;
        OString         scope = m_typeName.replace('/', '.');

        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldAccess(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            fieldName = m_reader.getFieldName(i);
            fieldType = checkRealBaseType(m_reader.getFieldType(i), sal_True);

            o << indent() << "rtl_uString_newFromAscii( &pMemberType" << i << ", \""
              << fieldType.replace('/', '.') << "\") );\n";
            o << indent() << "rtl_uString_newFromAscii( &pMemberName" << i << ", \"";
            o << fieldName << "\") );\n";
            o << indent() << "aMembers[" << i << "].eTypeClass = "
              << getTypeClass(fieldType, sal_True) << ";\n"
              << indent() << "aMembers[" << i << "].pTypeName = pMemberType" << i << ";\n"
              << indent() << "aMembers[" << i << "].pMemberName = pMemberName" << i << ";\n";
        }

        o << "\n" << indent() << "typelib_typedescription_new(\n";
        inc();
        o << indent() << "&pTD,\n" << indent()
          << getTypeClass(OString(), sal_True) << ", pTypeName,\n";

        if (superType.getLength() > 0)
            o << indent() << "pSuperType,\n";
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
        o << indent() << "typelib_typedescription_register( &pTD );\n\n";

        o << indent() << "typelib_typedescriptionreference_new( &s_pType_ " << typeName
          << getTypeClass(OString(), sal_True) << ", pTD);\n\n";

        o << indent() << "typelib_typedescription_release( pTD );\n"
          << indent() << "typelib_typedescriptionreference_release( pSuperType );\n"
          << indent() << "rtl_uString_release( pTypeName );\n";

        for (i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldAccess(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            o << indent() << "rtl_uString_release( pMemberName" << i << " );\n"
              << indent() << "rtl_uString_release( pMemberType" << i << " );\n";
        }
    }

    dec();
    o << indent() << "}\n";
    o << indent() << "osl_releaseMutex( pMutex );\n";
    dec();
    o << indent() << "}\n"
      << indent() << "typelib_typedescriptionreference_acquire( s_pType_" << typeName << " );\n"
      << indent() << "return &s_pType_" << typeName <<" );\n";
    dec();
    o << "}\n";

    dumpCloseExternC(o);
}

void CunoType::dumpCppuGetTypeMemberDecl(FileStream& o, CunoTypeDecl eDeclFlag)
{
    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;

    if ( fieldCount )
    {
        o << indent() << "{\n" << indent() << "typelib_TypeDescriptionReference ** ppTypeRef = 0;\n";

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
        o << indent() << "}\n";
    }
}

sal_uInt32 CunoType::getMemberCount()
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

sal_uInt32 CunoType::checkInheritedMemberCount(const TypeReader* pReader)
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

sal_uInt32 CunoType::getInheritedMemberCount()
{
    if (m_inheritedMemberCount == 0)
    {
        m_inheritedMemberCount = checkInheritedMemberCount(0);
    }

    return m_inheritedMemberCount;
}

void CunoType::dumpInheritedMembers(FileStream& o, rtl::OString& superType)
{
    TypeReader aSuperReader(m_typeMgr.getTypeReader(superType));

    OString baseType(aSuperReader.getSuperTypeName());
    if (baseType.getLength() > 0)
    {
        dumpInheritedMembers(o, baseType);
    }

    sal_uInt32 fieldCount = aSuperReader.getFieldCount();
    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldName;
    OString fieldType;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = aSuperReader.getFieldAccess(i);

        if (access != RT_ACCESS_CONST && access != RT_ACCESS_INVALID)
        {
            fieldName = aSuperReader.getFieldName(i);
            fieldType = aSuperReader.getFieldType(i);

            o << indent();
            dumpType(o, fieldType);
            o << " " << fieldName << ";\n";
        }
    }
}

OString CunoType::getTypeClass(const OString& type, sal_Bool bCStyle)
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

void CunoType::dumpType(FileStream& o, const OString& type,
                        sal_Bool bConst, sal_Bool bPointer, sal_Bool bParam)
    throw( CannotDumpException )
{
    OString sType(checkRealBaseType(type, sal_True));
    sal_uInt32 index = sType.lastIndexOf(']');
    sal_uInt32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    OString relType = (index > 0 ? (sType).copy(index+1) : type);

    RTTypeClass typeClass = m_typeMgr.getTypeClass(relType);

//  if (bConst) o << "const ";

    if ( seqNum )
    {
        o << "/*";
        sal_uInt32 i;
        for (i=0; i < seqNum; i++)
        {
            o << "sequence< ";
        }
        o << relType.replace( '/', '.');
        for (i=0; i < seqNum; i++)
        {
            o << " >";
        }
        o << "*/ uno_Sequence *";
        if (bPointer) o << "*";
        return;
    }
    switch (typeClass)
    {
        case RT_TYPE_INTERFACE:
            o << relType.replace('/', '_') << " *";
            break;
        case RT_TYPE_INVALID:
            {
                OString tmp(getBaseType(relType));
                if (tmp.getLength() > 0)
                {
                    o << tmp.getStr();
                    if ( bParam && !bPointer && relType.equals("any") )
                        o << " *";
                } else
                    throw CannotDumpException("Unknown type '" + relType + "', incomplete type library.");
            }
            break;
        case RT_TYPE_STRUCT:
        case RT_TYPE_EXCEPTION:
            o << relType.replace('/', '_');
            if ( bParam && !bPointer ) o << " *";
            break;
        case RT_TYPE_ENUM:
        case RT_TYPE_TYPEDEF:
            o << relType.replace('/', '_');
            break;
    }

    if (bPointer) o << "*";
}

OString CunoType::getBaseType(const OString& type)
{
    if (type.equals("long"))
        return "sal_Int32";
    if (type.equals("short"))
        return "sal_Int16";
    if (type.equals("hyper"))
        return "sal_Int64";
    if (type.equals("string"))
        return "rtl_uString *";
    if (type.equals("boolean"))
        return "sal_Bool";
    if (type.equals("char"))
        return "sal_Unicode";
    if (type.equals("byte"))
        return "sal_Int8";
    if (type.equals("any"))
        return "uno_Any";
    if (type.equals("type"))
        return "typelib_TypeDescriptionReference *";
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

void CunoType::dumpCppuGetType(FileStream& o, const OString& type, sal_Bool bDecl, CunoTypeDecl eDeclFlag)
{
    OString sType( checkRealBaseType(type, sal_True) );
    sal_uInt32 index = sType.lastIndexOf(']');
    OString relType = (index > 0 ? (sType).copy(index+1) : type);

    if (eDeclFlag == CUNOTYPEDECL_ONLYINTERFACES)
    {
         if (m_typeMgr.getTypeClass(relType) == RT_TYPE_INTERFACE)
        {
            if (bDecl)
                o << indent() << "ppTypeRef = ";
            else
                o << indent();

            o << "getCUnoType_" << type.replace('/', '_') << "()";

            if (bDecl)
                o << ";\n" << indent() << "typelib_typedescriptionreference_release( *ppTypeRef );\n";
        }
    } else
    {
        if (isBaseType(type))
        {
            return;
        } else
        {
            if (eDeclFlag == CUNOTYPEDECL_NOINTERFACES &&
                m_typeMgr.getTypeClass(relType) == RT_TYPE_INTERFACE)
                return;

            if ( type.equals("type") )
                return;

            if (bDecl)
                o << indent() << "ppTypeRef = ";
            else
                o << indent();

            o << indent() << "getCUnoType_" << type.replace('/', '_') << "()";
        }
        if (bDecl)
            o << ";\n" << indent() << "typelib_typedescriptionreference_release( *ppTypeRef );\n";
    }
}

void CunoType::dumpTypeInit(FileStream& o, const OString& typeName)
{
    OString type(checkSpecialCunoType(typeName));

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
                sal_Int32 i = type.lastIndexOf('/');
                o << "(" << shortScopedName("", type, sal_False)
                  << "::" << type.copy( i != -1 ? i+1 :0 )
                  << "_" << reader.getFieldName(0) << ")";
                return;
            }
        }
    }

    o << "()";
}

BASETYPE CunoType::isBaseType(const OString& type)
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

OString CunoType::typeToIdentifier(const OString& type)
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

OString CunoType::checkSpecialCunoType(const OString& type)
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

sal_Bool CunoType::isSeqType(const OString& type, OString& baseType, OString& seqPrefix)
{
    if ( type.getStr()[0] == '[' )
    {
        sal_uInt32 index = type.lastIndexOf(']');
        baseType = ((OString)type).copy(index+1);
        seqPrefix = ((OString)type).copy(0, index+1);
        return sal_True;
    } else
    {
        baseType = type;
        seqPrefix = OString();
    }
    return sal_False;
}

sal_Bool CunoType::isArrayType(const OString& type, OString& baseType, OString& arrayPrefix)
{
    if ( type.getStr()[type.getLength()-1] == ']' )
    {
        sal_uInt32 index = type.indexOf('[');
        baseType = ((OString)type).copy(0, index-1);
        arrayPrefix = ((OString)type).copy(index);
        return sal_True;
    } else
    {
        baseType = type;
        arrayPrefix = OString();
    }
    return sal_False;
}

OString CunoType::checkRealBaseType(const OString& type, sal_Bool bResolveTypeOnly)
{
    OString realType;
    OString baseType;
    OString completePrefix;
    OString prefix;
    sal_Bool bSeqType = sal_True;

    if ( !isSeqType(type, baseType, completePrefix) )
        isArrayType(type, baseType, completePrefix);

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
                realType = reader.getSuperTypeName();
                if ( isSeqType(realType, baseType, prefix) ||
                     isArrayType(realType, baseType, prefix) )
                {
                    completePrefix += prefix;
                }
            } else
                mustBeChecked = sal_False;
        } else
        {
            break;
        }
    }

    if ( bResolveTypeOnly )
    {
        if ( completePrefix.getLength() > 0 )
        {
            baseType = bSeqType ? (completePrefix + baseType) : ( baseType + completePrefix);
        }
    }
    return baseType;
}

void CunoType::dumpConstantValue(FileStream& o, sal_uInt16 index)
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
                snprintf(tmp, sizeof(tmp), "0x%x", (sal_Int8)constValue.m_value.aByte);
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
        case RT_TYPE_INT64:
            {
                ::rtl::OString tmp( OString::valueOf(constValue.m_value.aHyper) );
                o << "(sal_Int64)" << tmp.getStr() << "L";
            }
            break;
        case RT_TYPE_UINT64:
            {
                ::rtl::OString tmp( OString::valueOf((sal_Int64)constValue.m_value.aUHyper) );
                o << "(sal_uInt64)" << tmp.getStr() << "L";
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
                o << "::rtl::OUString::createFromAscii(\"" << aStr.getStr() << "\")";
            }
            break;
    }
}

void CunoType::inc(sal_uInt32 num)
{
    m_indentLength += num;
}

void CunoType::dec(sal_uInt32 num)
{
    if (m_indentLength - num < 0)
        m_indentLength = 0;
    else
        m_indentLength -= num;
}

OString CunoType::indent()
{
    OStringBuffer tmp(m_indentLength);

    for (sal_uInt32 i=0; i < m_indentLength; i++)
    {
        tmp.append(' ');
    }
    return tmp.makeStringAndClear();
}

OString CunoType::indent(sal_uInt32 num)
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
    : CunoType(typeReader, typeName, typeMgr, typeDependencies)
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
    OString headerDefine(dumpHeaderDefine(o, "H"));
    o << "\n";

    dumpDefaultHIncludes(o);
    o << "\n";
    dumpDepIncludes(o, m_typeName, "h");
    o << "\n";
    dumpOpenExternC(o);

    o << "#ifndef " << m_name.toAsciiUpperCase() << "\n";
    o << "#define " << m_name.toAsciiUpperCase() << "\n";
    o << "struct _" << m_name << "_ftab;\n"
      << "typedef struct _" << m_name << "_ftab * " << m_name << ";\n";
    o << "#endif\n\n";

    dumpDeclaration(o);

    if ( m_cunoTypeLib )
    {
        o << "#ifdef CUNO_TYPELIB\n"
          << "typelib_TypeDescriptionReference * SAL_CALL getCUnoType_" << m_name << "() SAL_THROW( () );\n"
          << "#endif\n\n";
    }

/*
    if (getNestedTypeNames().getLength() > 0)
    {
        o << indent() << "// nested types\n\n";
        for (sal_uInt32 i = 0; i < getNestedTypeNames().getLength(); i++)
        {
            OUString s(getNestedTypeNames().getElement(i));

            OString nestedName(s.getStr(), s.getLength(), RTL_TEXTENCODING_UTF8);

            nestedName = checkRealBaseType(nestedName.copy(5));

            if (nestedName.lastIndexOf(']') < 0)
            {
                o << "inline const ::com::sun::star::uno::Type& SAL_CALL getCunoType( ";
                dumpType(o, nestedName, sal_True, sal_False);
                o << "* ) SAL_THROW( () );\n\n";
            }
        }
    }
*/
    dumpCloseExternC(o);

    o << "#endif /* "<< headerDefine << " */\n";
    return sal_True;
}

void InterfaceType::dumpInheritedFunctions(FileStream& o, rtl::OString& superType)
{
    TypeReader aSuperReader(m_typeMgr.getTypeReader(superType));

    OString baseType(aSuperReader.getSuperTypeName());
    if (baseType.getLength() > 0)
    {
        dumpInheritedFunctions(o, baseType);
    }

    dumpAttributes(o, superType.replace('/', '_'), aSuperReader);
    dumpMethods(o, superType.replace('/', '_'), aSuperReader);
}

sal_Bool InterfaceType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    o << "typedef struct _" << m_name << "_ftab\n" << indent() << "{";
    inc();

    OString superType(m_reader.getSuperTypeName());
    if (superType.getLength() > 0)
        dumpInheritedFunctions(o, superType);
/*
    if (getNestedTypeNames().getLength() > 0)
    {
        inc();
        o << indent() << "// nested types\n\n";
        for (sal_uInt32 i = 0; i < getNestedTypeNames().getLength(); i++)
        {
            OUString s(getNestedTypeNames().getElement(i));

            OString nestedName(s.getStr(), s.getLength(), RTL_TEXTENCODING_UTF8);

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
*/
    dumpAttributes(o, m_name, m_reader);
    dumpMethods(o, m_name, m_reader);

    dec();
    o << "} " << m_name << "_ftab;\n\n";

    return sal_True;
}

sal_Bool InterfaceType::dumpCFile(FileStream& o)
    throw( CannotDumpException )
{
    dumpInclude(o, m_typeName, "h");
    o << "\n";
    dumpDefaultCIncludes(o);
    o << "\n";
    dumpDepIncludes(o, m_typeName, "h");
    o << "\n";
    dumpGetCunoType(o);
/*
    if (getNestedTypeNames().getLength() > 0)
    {
        o << indent() << "// nested types\n\n";
        for (sal_uInt32 i = 0; i < getNestedTypeNames().getLength(); i++)
        {
            OUString s(getNestedTypeNames().getElement(i));

            OString nestedName(s.getStr(), s.getLength(), RTL_TEXTENCODING_UTF8);

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
                            iType.dumpGetCunoType(o);
                        }
                        break;
                    case RT_TYPE_STRUCT:
                        {
                            StructureType sType(reader, nestedName, m_typeMgr, m_dependencies);
                            sType.dumpGetCunoType(o);
                        }
                        break;
                    case RT_TYPE_ENUM:
                        {
                            EnumType enType(reader, nestedName, m_typeMgr, m_dependencies);
                            enType.dumpGetCunoType(o);
                        }
                        break;
                    case RT_TYPE_EXCEPTION:
                        {
                            ExceptionType eType(reader, nestedName, m_typeMgr, m_dependencies);
                            eType.dumpGetCunoType(o);
                        }
                        break;
                    case RT_TYPE_TYPEDEF:
                        {
                            TypeDefType tdType(reader, nestedName, m_typeMgr, m_dependencies);
                            tdType.dumpGetCunoType(o);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
*/
    return sal_True;
}

void InterfaceType::dumpAttributes(FileStream& o, const OString& interfaceType, TypeReader& reader )
{
    sal_uInt32 fieldCount = reader.getFieldCount();
    sal_Bool first=sal_True;

    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldName;
    OString fieldType;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = reader.getFieldName(i);
        fieldType = reader.getFieldType(i);

        if (first)
        {
            first = sal_False;
            o << "\n" << indent() << "/* Attributes of " << interfaceType << " */\n";
        }

        o << indent() << "cuno_ErrorCode (SAL_CALL *get" << fieldName << ")( "
          << interfaceType << " *, uno_Any *, ";
        dumpType(o, fieldType, sal_False, sal_True);
        o << " );\n";

        if (access != RT_ACCESS_READONLY)
        {
            OString relType = checkSpecialCunoType(fieldType);
            sal_Bool bParam = sal_False;

            if ( m_typeMgr.getTypeClass(relType) == RT_TYPE_STRUCT ||
                 m_typeMgr.getTypeClass(relType) == RT_TYPE_EXCEPTION ||
                (isBaseType(relType) && relType.equals("any")))
            {
                bParam = sal_True;
            } else
            {
                bParam = sal_False;
            }

            o << indent() << "cuno_ErrorCode (SAL_CALL *set" << fieldName << ")( "
                << interfaceType << " *, uno_Any *, ";
            dumpType(o, fieldType, sal_False, sal_False, bParam);
            o << " );\n";
        }
    }
}

void InterfaceType::dumpMethods(FileStream& o, const OString& interfaceType, TypeReader& reader )
{
    sal_uInt32 methodCount = reader.getMethodCount();
    sal_Bool first=sal_True;

    OString methodName, returnType, paramType, paramName;
    sal_uInt32 paramCount = 0;
    sal_uInt32 excCount = 0;
    RTMethodMode methodMode = RT_MODE_INVALID;
    RTParamMode  paramMode = RT_PARAM_INVALID;

    sal_Bool bPointer = sal_False;
    sal_Bool bParam = sal_False;
    sal_Bool bWithRunTimeExcp = sal_True;

    for (sal_uInt16 i=0; i < methodCount; i++)
    {
        methodName = reader.getMethodName(i);
        returnType = reader.getMethodReturnType(i);
        paramCount = reader.getMethodParamCount(i);
        excCount = reader.getMethodExcCount(i);
        methodMode = reader.getMethodMode(i);

        if ( methodName.equals("queryInterface") )
        {
            first = sal_False;
            o << "\n" << indent() << "/* Methods of " << interfaceType << " */\n";
            o << indent() << "cuno_ErrorCode (SAL_CALL *queryInterface)( com_sun_star_uno_XInterface *, "
              << "uno_Any *, com_sun_star_uno_XInterface **, typelib_TypeDescriptionReference * );\n";
            continue;
        }

        if ( methodName.equals("acquire") || methodName.equals("release") )
        {
            bWithRunTimeExcp = sal_False;
        }

        if (first)
        {
            first = sal_False;
            o << "\n" << indent() << "/* Methods of " << interfaceType << " */\n";
        }

        o << indent() << "cuno_ErrorCode (SAL_CALL *" << methodName << ")( "
          << interfaceType << " *";
        if ( excCount || bWithRunTimeExcp )
        {
            o << ", uno_Any *";
        }
        if ( !isVoid(returnType) )
        {
            o << ", ";
            dumpType(o, returnType, sal_False, sal_True);
        }

        sal_uInt16 j;
        for (j=0; j < paramCount; j++)
        {
            paramName = reader.getMethodParamName(i, j);
            paramType = reader.getMethodParamType(i, j);
            paramMode = reader.getMethodParamMode(i, j);

            if (j < (sal_uInt16)paramCount) o << ", ";

            switch (paramMode)
            {
                case RT_PARAM_IN:
                {
                    OString relType = checkSpecialCunoType(paramType);
                    if (m_typeMgr.getTypeClass(relType) == RT_TYPE_STRUCT ||
                        m_typeMgr.getTypeClass(relType) == RT_TYPE_EXCEPTION ||
                        (isBaseType(relType) && relType.equals("any")))
                    {
                        bParam = sal_True;
                    } else
                    {
                        bParam = sal_False;
                    }
                    break;
                }
                case RT_PARAM_OUT:
                case RT_PARAM_INOUT:
                    bPointer = sal_True;
                    break;
            }

            dumpType(o, paramType, sal_False, bPointer, bParam);
        }
        o << " );\n";
    }
}

void InterfaceType::dumpGetCunoType(FileStream& o)
{
    OString typeName(m_typeName.replace('/', '_'));

    if ( m_cunoTypeLeak )
    {
        dumpLGetCunoType(o);
        return;
    }
    if ( !m_cunoTypeDynamic )
    {
        dumpCGetCunoType(o);
        return;
    }

    dumpOpenExternC(o);

    if ( !m_typeName.equals("com/sun/star/uno/XInterface") )
    {
        o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
          << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
          << "#endif\n\n";
    }

    o << "typelib_TypeDescriptionReference ** SAL_CALL getCUnoType_" << m_name << "() SAL_THROW_EXTERN_C( () )\n{\n";
    inc();

    if ( m_typeName.equals("com/sun/star/uno/XInterface") )
    {
        o << indent() << "return typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE );\n";
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
            o << indent() << "typelib_TypeDescriptionReference * pSuperType = 0;\n"
              << indent() << "typelib_typedescriptionreference_newByAsciiName(&pSuperType, typelib_TypeClass_INTERFACE, \""
              << superType.replace('/', '.') << "\" );\n";
        }

        o << indent() << "typelib_static_interface_type_init( &s_pType_" << typeName
          << ", \"" << m_typeName.replace('/', '.') << "\", ";

        if ( bWithBase )
        {
            o << "pSuperType );\n";
        } else
        {
            o << "0 );\n";
        }

        dec();
        o << indent() << "}\n"
          << indent() << "typelib_typedescriptionreference_acquire( s_pType_" << typeName << " );\n"
          << indent() << "return &s_pType_" << typeName <<" );\n";
    }
    dec();
    o << indent() << "}\n";

    dumpCloseExternC(o);
}

void InterfaceType::dumpCGetCunoType(FileStream& o)
{
    OString typeName(m_typeName.replace('/', '_'));

    dumpOpenExternC(o);

    o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      <<  "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << "typelib_TypeDescriptionReference ** SAL_CALL getCUnoType_" << m_name << "() SAL_THROW_EXTERN_C( () )\n{\n";
    inc();

    o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << indent() << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << indent() << "if ( !s_pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "oslMutex * pMutex = osl_getGlobalMutex();\n"
      << indent() << "osl_acquireMutex( pMutex );\n";

    o << indent() << "if ( !s_pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "rtl_uString * pTypeName = 0;\n"
       << indent() << "typelib_InterfaceTypeDescription * pTD = 0;\n";

    OString superType(m_reader.getSuperTypeName());
    sal_uInt32 count = getMemberCount();

    if (superType.getLength() > 0)
        o << indent() << "typelib_TypeDescriptionReference * pSuperType = 0;\n";

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

        dumpCUnoAttributeTypeNames(o);
        dumpCUnoMethodTypeNames(o);
    }

    o << indent() << "rtl_uString_newFromAscii( &pTypeName, \"" << m_typeName.replace('/', '.') << "\" );\n";

    if (superType.getLength() > 0)
    {
        o << indent() << "typelib_typedescriptionreference_newByAsciiName(&pSuperType, typelib_TypeClass_INTERFACE, \""
          << superType.replace('/', '.') << "\" );\n";
    }

    if (count)
    {
        sal_uInt32 index = 0;
        dumpCUnoAttributeRefs(o, index);
        dumpCUnoMethodRefs(o, index);
    }

    o << "\n" << indent() << "typelib_typedescription_newInterface(\n";
    inc();
    o << indent() << "&pTD,\n"
      << indent() << "pTypeName, ";

    RTUik uik;
    m_reader.getUik(uik);
    sal_Char buffer[53];
    snprintf(buffer, sizeof(buffer), "0x%.8x, 0x%.4x, 0x%.4x, 0x%.8x, 0x%.8x,\n",
            uik.m_Data1, uik.m_Data2, uik.m_Data3, uik.m_Data4, uik.m_Data5);
    o << buffer;

    if (superType.getLength() > 0)
        o << indent() << "pSuperType,\n";
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
    o << indent() << "typelib_typedescription_release( (typelib_TypeDescription*)pTD );\n";

    if (superType.getLength() > 0)
        o << indent() << "typelib_typedescription_release( pSuperType );\n\n";
    else
        o << "\n";

    o << indent() << "typelib_typedescriptionreference_new( &s_pType_ " << typeName
      << "typelib_TypeClass_INTERFACE, (typelib_TypeDescription*)pTD);\n\n";

    o << indent() << "typelib_TypeDescriptionReference ** ppTypeRef = 0;\n";
    StringSet   aTypes;
    // type for RuntimeException is always needed
    OString     sRunTimeExceptionType("com/sun/star/uno/RuntimeException");
    aTypes.insert(sRunTimeExceptionType);
    dumpCppuGetType(o, sRunTimeExceptionType, sal_True, CUNOTYPEDECL_ALLTYPES);

    dumpAttributesCppuDecl(o, &aTypes, CUNOTYPEDECL_ALLTYPES);
    dumpMethodsCppuDecl(o, &aTypes, CUNOTYPEDECL_ALLTYPES);

    if (count)
    {
        sal_uInt32 index = getInheritedMemberCount();
        dumpCUnoAttributes(o, index);
        dumpCUnoMethods(o, index);
    }

    // release strings for names
    dumpCUnoAttributeTypeNames(o, sal_True);
    dumpCUnoMethodTypeNames(o, sal_True);

    dec();
    o << indent() << "}\n";
    o << indent() << "osl_releaseMutex( pMutex );\n";
    dec();
    o << indent() << "}\n\n"
      << indent() << "typelib_typedescriptionreference_acquire( s_pType_" << typeName << " );\n"
      << indent() << "return &s_pType_" << typeName << ";\n";

    dec();
    o << "}\n";

    dumpCloseExternC(o);
}

void InterfaceType::dumpCUnoAttributeTypeNames(FileStream&o, sal_Bool bRelease)
{
    sal_uInt32 fieldCount = m_reader.getFieldCount();
    RTFieldAccess access = RT_ACCESS_INVALID;

    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);
        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;
        if ( bRelease )
        {
            o << indent() << "rtl_uString_release( pAttributeName" << i << " );\n";
        } else
        {
            o << indent() << "rtl_uString * pAttributeName" << i << " = 0;\n";
        }
    }
}

void InterfaceType::dumpCUnoMethodTypeNames(FileStream&o, sal_Bool bRelease)
{
    sal_uInt32  methodCount = m_reader.getMethodCount();

    for (sal_uInt16 i = 0; i < methodCount; i++)
    {
        if ( bRelease )
        {
            o << indent() << "rtl_uString_release( pMethodName" << i << " );\n";
        } else
        {
            o << indent() << "rtl_uString * pMethodName" << i << " = 0;\n";
        }
    }
}

void InterfaceType::dumpCUnoAttributeRefs(FileStream& o, sal_uInt32& index)
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

        o << indent() << "rtl_uString_newFromAscii( &pAttributeName" << i << ", \""
          << scope << "::" << fieldName << "\" );\n";
        o << indent() << "typelib_typedescriptionreference_new( &pMembers["
          << index << "],\n";
        inc(38);
        o << indent() << "typelib_TypeClass_INTERFACE_ATTRIBUTE,\n"
          << indent() << "pAttributeName" << i << " );\n";
        dec(38);
        index++;
    }
}

void InterfaceType::dumpCUnoMethodRefs(FileStream& o, sal_uInt32& index)
{
    sal_uInt32  methodCount = m_reader.getMethodCount();
    OString     methodName; //, returnType, paramType, paramName;
    OString     scope = m_typeName.replace('/', '.');

    for (sal_uInt16 i = 0; i < methodCount; i++)
    {
        methodName = m_reader.getMethodName(i);

        o << indent() << "rtl_uString_newFromAscii( &pMethodName" << i << ", \""
          << scope.replace('/', '.') << "::" << methodName << "\" );\n";
        o << indent() << "typelib_typedescriptionreference_new( &pMembers["
          << index << "],\n";
        inc(38);
        o << indent() << "typelib_TypeClass_INTERFACE_METHOD,\n"
          << indent() << "pMethodName" << i << " );\n";
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

void InterfaceType::dumpCUnoAttributes(FileStream& o, sal_uInt32& index)
{
    sal_uInt32 fieldCount = m_reader.getFieldCount();

    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldType;

    sal_uInt32 absoluteIndex = index;

    if (m_hasAttributes)
    {
        o << "\n" << indent() << "{\n" << indent() << "typelib_InterfaceAttributeTypeDescription * pAttribute = 0;\n";

        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldAccess(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            fieldType = checkRealBaseType(m_reader.getFieldType(i), sal_True);
            o << indent() << "{\n";
            o << indent() << "rtl_uString * pAttributeType" << i << " = 0;\n";
            o << indent() << "rtl_uString_newFromAscii( &pAttributeType" << i << ", \""
              << fieldType.replace('/', '.') << "\" );\n";
            o << indent() << "typelib_typedescription_newInterfaceAttribute( &pAttribute,\n";
            inc();
            o << indent() << absoluteIndex++ << ", pAttributeName" << i << ",\n";
            o << indent() << getTypeClass(fieldType, sal_True) << ", pAttributeType" << i << ",\n";
            if (access == RT_ACCESS_READONLY)
                o << indent() << "sal_True );\n";
            else
                o << indent() << "sal_False );\n";
            dec();
            o << indent() << "typelib_typedescription_register( (typelib_TypeDescription**)&pAttribute );\n\n";
            o << indent() << "}\n";
        }
         o << indent() << "typelib_typedescription_release( (typelib_TypeDescription*)pAttribute );\n";
        o << indent() << "}\n";
        index = absoluteIndex;
    }
}

void InterfaceType::dumpCUnoMethods(FileStream& o, sal_uInt32& index)
{
    sal_uInt32      methodCount = m_reader.getMethodCount();
    OString         methodName, returnType, paramType, paramName;
    sal_uInt32      paramCount = 0;
    sal_uInt32      excCount = 0;
    RTMethodMode    methodMode = RT_MODE_INVALID;
    RTParamMode     paramMode = RT_PARAM_INVALID;
    sal_Bool        bWithRuntimeException = sal_True;

    sal_uInt32 absoluteIndex = index;

    if (m_hasMethods)
    {
        o << "\n" << indent() << "{\n" << indent() << "typelib_InterfaceMethodTypeDescription * pMethod = 0;\n";

        for (sal_uInt16 i=0; i < methodCount; i++)
        {
            methodName = m_reader.getMethodName(i);
            returnType = checkRealBaseType(m_reader.getMethodReturnType(i), sal_True);
            paramCount = m_reader.getMethodParamCount(i);
            excCount = m_reader.getMethodExcCount(i);
            methodMode = m_reader.getMethodMode(i);

            if ( methodName.equals("acquire") || methodName.equals("release") )
            {
                bWithRuntimeException = sal_False;
            }
            o << indent() << "{\n";
            inc();

            if (paramCount)
            {
                o << indent() << "typelib_Parameter_Init pParameters[" << paramCount << "];\n";
            }
            if ( excCount || bWithRuntimeException )
            {
                o << indent() << "rtl_uString * pExceptions[" << excCount + 1 << "];\n";
            }
            o << indent() << "rtl_uString * pReturnType" << i << " = 0;\n";

            sal_uInt16 j;
            for (j=0; j < paramCount; j++)
            {
                o << indent() << "rtl_uString * pParamName" << j << " = 0;\n"
                  << indent() << "rtl_uString * pParamType" << j << " = 0;\n";
            }

              for (j=0; j < excCount; j++)
            {
                o << indent() << "rtl_uString * pExceptionName" << j << " = 0;\n";
            }
            if ( excCount || bWithRuntimeException )
            {
                o << indent() << "rtl_uString * pExceptionName" << excCount << " = 0;\n";
            }
            for (j=0; j < paramCount; j++)
            {
                paramName = m_reader.getMethodParamName(i, j);
                paramType = checkRealBaseType(m_reader.getMethodParamType(i, j), sal_True);
                paramMode = m_reader.getMethodParamMode(i, j);
                o << indent() << "rtl_uString_newFromAscii( &pParamName" << j << ", \""
                  << paramName << "\" );\n";
                o << indent() << "rtl_uString_newFromAscii( &pParamType" << j << ", \""
                  << paramType.replace('/', '.') << "\" );\n";
                o << indent() << "pParameters[" << j << "].pParamName = pParamName" << j << ";\n";
                o << indent() << "pParameters[" << j << "].eTypeClass = "
                  << getTypeClass(paramType, sal_True) << ";\n";
                o << indent() << "pParameters[" << j << "].pTypeName = sParamType" << j << ";\n";

                if (paramMode == RT_PARAM_IN || paramMode == RT_PARAM_INOUT)
                    o << indent() << "pParameters[" << j << "].bIn = sal_True;\n";
                else
                    o << indent() << "pParameters[" << j << "].bIn = sal_False;\n";

                if (paramMode == RT_PARAM_OUT || paramMode == RT_PARAM_INOUT)
                    o << indent() << "pParameters[" << j << "].bOut = sal_True;\n";
                else
                    o << indent() << "pParameters[" << j << "].bOut = sal_False;\n";
            }

              for (j=0; j < excCount; j++)
            {
                if (!m_reader.getMethodExcType(i, j).equals("com/sun/star/uno/RuntimeException"))
                {
                    o << indent() << "rtl_uString_newFromAscii( & pExceptionName" << j << ", \""
                      << OString(m_reader.getMethodExcType(i, j)).replace('/', '.') << "\" );\n";
                    o << indent() << "pExceptions[" << j << "] = pExceptionName" << j << ";\n";
                }
            }
            if ( excCount || bWithRuntimeException )
            {
                  o << indent() << "rtl_uString_newFromAscii( & pExceptionName" << excCount
                  << ", \"com.sun.star.uno.RuntimeException\") );\n";
                o << indent() << "pExceptions[" << excCount << "] = pExceptionName" << excCount << ";\n";
            }
            o << indent() << "rtl_uString_newFromAscii( &pReturnType" << i << ", \""
              << returnType.replace('/', '.') << "\" );\n";
            o << indent() << "typelib_typedescription_newInterfaceMethod( &pMethod,\n";
            inc();
            o << indent() << absoluteIndex++ << ", ";
            if (methodMode == RT_MODE_ONEWAY || methodMode == RT_MODE_ONEWAY_CONST)
                o << "sal_True,\n";
            else
                o << "sal_False,\n";
            o << indent() << "pMethodName" << i << ",\n";
            o << indent() << getTypeClass(returnType, sal_True) << ", pReturnType" << i << ",\n";
            if (paramCount)
                o << indent() << paramCount << ", pParameters,\n";
            else
                o << indent() << "0, 0,\n";

            if ( excCount || bWithRuntimeException )
            {
                o << indent() << excCount + 1 << ", pExceptions );\n";
            } else
            {
                o << indent() << "0, 0 );\n";
            }

            dec();
            o << indent() << "typelib_typedescription_register( (typelib_TypeDescription**)&pMethod );\n";

            o << indent() << "rtl_uString_release( pReturnType );\n";
            for (j=0; j < paramCount; j++)
            {
                o << indent() << "rtl_uString_release( pParamName" << j << " );\n"
                  << indent() << "rtl_uString_release( pParamType" << j << " );\n";
            }

              for (j=0; j < excCount; j++)
            {
                o << indent() << "rtl_uString_release( pExceptionName" << j << " );\n";
            }
            if ( excCount || bWithRuntimeException )
            {
                o << indent() << "rtl_uString_release( pExceptionName" << excCount << " );\n";
            }
            dec();
            o << indent() << "}\n";
        }
        o << indent() << "typelib_typedescription_release( (typelib_TypeDescription*)pMethod );\n";

        o << indent() << "}\n";
        index = absoluteIndex;
    }
}

void InterfaceType::dumpAttributesCppuDecl(FileStream& o, StringSet* pFinishedTypes, CunoTypeDecl eDeclFlag)
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

void InterfaceType::dumpMethodsCppuDecl(FileStream& o, StringSet* pFinishedTypes, CunoTypeDecl eDeclFlag)
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
    : CunoType(typeReader, typeName, typeMgr, typeDependencies)
{
}

ModuleType::~ModuleType()
{

}

sal_Bool ModuleType::dump(CunoOptions* pOptions)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_False;

    if (pOptions->isValid("-U"))
        m_cunoTypeDynamic = sal_True;

    OString outPath;
    if (pOptions->isValid("-O"))
        outPath = pOptions->getOption("-O");

    OString tmpName(m_typeName);

    if (tmpName.equals("/"))
        tmpName = "global";
    else
//      tmpName += "/" + m_typeName.getToken(m_typeName.getTokenCount('/') - 1, '/');
        tmpName += "/" + m_name;

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
            hFile.open(tmpFileName);
        else
            hFile.open(hFileName);

        if(!hFile.isValid())
        {
            OString message("cannot open ");
            message += hFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpHFile(hFile);

        hFile.close();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(hFileName, tmpFileName);
        }
    }
/*
    bFileExists = sal_False;
    bFileCheck = sal_False;
    OString cFileName = createFileNameFromType(outPath, tmpName, ".c");

    if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
    {
        bFileExists = fileExists( cFileName );
        ret = sal_True;
    }

    if ( bFileExists && pOptions->isValid("-Gc") )
    {
        tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tmc");
        bFileCheck = sal_True;
    }


    if ( !bFileExists || bFileCheck )
    {
        FileStream hxxFile;

        if ( bFileCheck )
            cFile.open(tmpFileName);
        else
            cFile.open(cFileName);

        if(!cFile.isValid())
        {
            OString message("cannot open ");
            message += cFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpCFile(cFile);

        cFile.close();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(cFileName, tmpFileName);
        }
    }
*/
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

    OString headerDefine(dumpHeaderDefine(o, "H", bSpecialDefine));
    o << "\n";

    dumpDefaultHIncludes(o);
    o << "\n";
    dumpDepIncludes(o, m_typeName, "h");
    o << "\n";

    dumpOpenExternC(o);
    dumpDeclaration(o);
    o << "\n";
    dumpCloseExternC(o);

    o << "\n#endif /* "<< headerDefine << " */\n";

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
            o << " " << m_name << "_" << fieldName << " = ";
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

sal_Bool ModuleType::dumpCFile(FileStream& o)
    throw( CannotDumpException )
{
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

sal_Bool ConstantsType::dump(CunoOptions* pOptions)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_False;

    if (pOptions->isValid("-U"))
        m_cunoTypeDynamic = sal_True;

    OString outPath;
    if (pOptions->isValid("-O"))
        outPath = pOptions->getOption("-O");

    OString tmpFileName;
    OString hFileName = createFileNameFromType(outPath, m_typeName, ".h");

    sal_Bool bFileExists = sal_False;
    sal_Bool bFileCheck = sal_False;

    if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
    {
        bFileExists = fileExists( hFileName );
        ret = sal_True;
    }

    if ( bFileExists && pOptions->isValid("-Gc") )
    {
        tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tmh");
        bFileCheck = sal_True;
    }

    if ( !bFileExists || bFileCheck )
    {
        FileStream hFile;

        if ( bFileCheck )
            hFile.open(tmpFileName);
        else
            hFile.open(hFileName);

        if(!hFile.isValid())
        {
            OString message("cannot open ");
            message += hFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpHFile(hFile);

        hFile.close();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(hFileName, tmpFileName);
        }
    }
/*
    bFileExists = sal_False;
    bFileCheck = sal_False;
    OString cFileName = createFileNameFromType(outPath, m_typeName, ".c");

    if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
    {
        bFileExists = fileExists( cFileName );
        ret = sal_True;
    }

    if ( bFileExists && pOptions->isValid("-Gc") )
    {
        tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tmc");
        bFileCheck = sal_True;
    }

    if ( !bFileExists || bFileCheck )
    {
        FileStream cFile;

        if ( bFileCheck )
            cFile.open(tmpFileName);
        else
            cFile.open(cFileName);

        if(!cFile.isValid())
        {
            OString message("cannot open ");
            message += cFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpCFile(cFile);

        cFile.close();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(cFileName, tmpFileName);
        }
    }
*/
    return ret;
}

//*************************************************************************
// StructureType
//*************************************************************************
StructureType::StructureType(TypeReader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr,
                             const TypeDependency& typeDependencies)
    : CunoType(typeReader, typeName, typeMgr, typeDependencies)
{
}

StructureType::~StructureType()
{

}

sal_Bool StructureType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "H"));
    o << "\n";

    dumpDefaultHIncludes(o);
    o << "\n";
    dumpDepIncludes(o, m_typeName, "h");
    o << "\n";

    dumpOpenExternC(o);

    dumpDeclaration(o);

    if ( m_cunoTypeLib )
    {
        o << "#ifdef CUNO_TYPELIB\n"
          << "typelib_TypeDescriptionReference ** SAL_CALL getCUnoType_" << m_name << "() SAL_THROW_EXTERN_C( () );\n"
          << "#endif\n\n";
    }

    dumpCloseExternC(o);

    o << "#endif /* "<< headerDefine << " */\n";

    return sal_True;
}

sal_Bool StructureType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    o << "#ifdef SAL_W32\n"
      << "#   pragma pack(push, 8)\n"
      << "#elif defined(SAL_OS2)\n"
      << "#   pragma pack(8)\n"
      << "#endif\n\n";

    o << "typedef struct _" << m_name << "\n{\n";
    inc();

    OString superType(m_reader.getSuperTypeName());
    if (superType.getLength() > 0)
        o << indent() << superType.replace('/', '_').getStr() << " _Base;\n";
        //dumpInheritedMembers(o, superType);

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    sal_uInt16      i=0;

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
    o << "} " << m_name << ";\n\n";

    o << "#ifdef SAL_W32\n"
      << "#   pragma pack(pop)\n"
      << "#elif defined(SAL_OS2)\n"
      << "#   pragma pack()\n"
      << "#endif\n\n";

    return sal_True;
}

sal_Bool StructureType::dumpCFile(FileStream& o)
    throw( CannotDumpException )
{
    dumpInclude(o, m_typeName, "h");
    o << "\n";
    dumpDefaultCIncludes(o);
    o << "\n";
    dumpDepIncludes(o, m_typeName, "h");
    o << "\n";

    dumpGetCunoType(o);

    return sal_True;
}

//*************************************************************************
// ExceptionType
//*************************************************************************
ExceptionType::ExceptionType(TypeReader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr,
                             const TypeDependency& typeDependencies)
    : CunoType(typeReader, typeName, typeMgr, typeDependencies)
{
}

ExceptionType::~ExceptionType()
{

}

sal_Bool ExceptionType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "H"));
    o << "\n";

    dumpDefaultHIncludes(o);
    o << "\n";
    dumpDepIncludes(o, m_typeName, "h");
    o << "\n";

    dumpOpenExternC(o);

    dumpDeclaration(o);

    if ( m_cunoTypeLib )
    {
        o << "#ifdef CUNO_TYPELIB\n"
          << "typelib_TypeDescriptionReference ** SAL_CALL getCUnoType_" << m_name << "() SAL_THROW_EXTERN_C( () );\n"
          << "#endif\n\n";
    }

    dumpCloseExternC(o);

    o << "#endif /* "<< headerDefine << " */\n";

    return sal_True;
}

sal_Bool ExceptionType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    o << "#ifdef SAL_W32\n"
      << "#   pragma pack(push, 8)\n"
      << "#elif defined(SAL_OS2)\n"
      << "#   pragma pack(8)\n"
      << "#endif\n\n";

    o << "\n/* Exception type */\ntypedef struct _" << m_name << "\n{\n";
    inc();

    OString superType(m_reader.getSuperTypeName());
    if (superType.getLength() > 0)
        o << indent() << superType.replace('/', '_').getStr() << " _Base;\n";
        //dumpInheritedMembers(o, superType);

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

        o << indent();
        dumpType(o, fieldType);
        o << " " << fieldName << ";\n";
    }

    dec();
    o << "} " << m_name << ";\n\n";

    o << "#ifdef SAL_W32\n"
      << "#   pragma pack(pop)\n"
      << "#elif defined(SAL_OS2)\n"
      << "#   pragma pack()\n"
      << "#endif\n\n";

    return sal_True;
}

sal_Bool ExceptionType::dumpCFile(FileStream& o)
    throw( CannotDumpException )
{
    dumpInclude(o, m_typeName, "h");
    o << "\n";
    dumpDefaultCIncludes(o);
    o << "\n";
    dumpDepIncludes(o, m_typeName, "h");
    o << "\n";

    dumpGetCunoType(o);

    return sal_True;
}


//*************************************************************************
// EnumType
//*************************************************************************
EnumType::EnumType(TypeReader& typeReader,
                    const OString& typeName,
                   const TypeManager& typeMgr,
                   const TypeDependency& typeDependencies)
    : CunoType(typeReader, typeName, typeMgr, typeDependencies)
{
}

EnumType::~EnumType()
{

}

sal_Bool EnumType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "H"));
    o << "\n";

    dumpDefaultHIncludes(o);
    o << "\n";
    dumpOpenExternC(o);

    dumpDeclaration(o);

    if ( m_cunoTypeLib )
    {
        o << "#ifdef CUNO_TYPELIB\n"
          << "typelib_TypeDescriptionReference ** SAL_CALL getCUnoType_" << m_name << "() SAL_THROW_EXTERN_C( () );\n"
          << "#endif\n\n";
    }

    dumpCloseExternC(o);

    o << "#endif /* "<< headerDefine << " */\n";

    return sal_True;
}

sal_Bool EnumType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    o << "\ntypedef enum _" << m_name << "\n{\n";
    inc();

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    RTConstValue    constValue;
    OString         fieldName;
    sal_Int32       value=0;
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
    o << "} " << m_name << ";\n\n";

    return sal_True;
}

sal_Bool EnumType::dumpCFile(FileStream& o)
    throw( CannotDumpException )
{
    dumpInclude(o, m_typeName, "h");
    o << "\n";
    dumpDefaultCIncludes(o);
    o << "\n";
    dumpGetCunoType(o);
    return sal_True;
}

void EnumType::dumpGetCunoType(FileStream& o)
{
    OString typeName(m_typeName.replace('/', '_'));

    if ( m_cunoTypeLeak )
    {
        dumpLGetCunoType(o);
        return;
    }
    if ( !m_cunoTypeDynamic )
    {
        dumpCGetCunoType(o);
        return;
    }

    dumpOpenExternC(o);

    o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << "typelib_TypeDescriptionReference ** SAL_CALL getCUnoType_" << m_name << "() SAL_THROW_EXTERN_C( () )\n{\n";
    inc();

    o << indent() << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << indent() << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << indent() << "#endif\n\n";

    o << indent() << "if ( !s_pType_" << typeName << " )\n" << indent() << "{\n";
    inc();

    o << indent() << "typelib_static_enum_type_init( &s_pType_" << typeName << ",\n";
    inc(31);
    o << indent() << "\"" << m_typeName.replace('/', '.') << "\",\n"
      << indent() << m_name << "_" << m_reader.getFieldName(0) << " );\n";
    dec(31);
    dec();
    o << indent() << "}\n"
      << indent() << "typelib_typedescriptionreference_acquire( s_pType_" << typeName <<" );\n"
      << indent() << "return &s_pType_" << typeName <<" );\n";
    dec();
    o << indent() << "}\n";

    dumpCloseExternC(o);
}

void EnumType::dumpCGetCunoType(FileStream& o)
{
    OString typeName(m_typeName.replace('/', '_'));

    dumpOpenExternC(o);

    o << "#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << "typelib_TypeDescriptionReference ** SAL_CALL getCUnoType_" << m_name << "() SAL_THROW_EXTERN_C( () )\n{\n";
    inc();

    o << "#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))\n"
      << indent() << "static typelib_TypeDescriptionReference * s_pType_" << typeName << " = 0;\n"
      << "#endif\n\n";

    o << indent() << "if ( !s_pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "oslMutex * pMutex = osl_getGlobalMutex();\n"
      << indent() << "osl_acquireMutex( pMutex );\n";

    o << indent() << "if ( !s_pType_" << typeName << " )\n" << indent() << "{\n";
    inc();
    o << indent() << "rtl_uString * pTypeName = 0;\n"
       << indent() << "_typelib_TypeDescription * pTD = 0;\n";

    sal_uInt32 count = m_reader.getFieldCount();
    o << indent() << "rtl_uString* enumValueNames[" << count << "];\n"
      << indent() << "sal_Int32 enumValues[" << count << "];\n";
    sal_uInt32 i;
    for (i = 0; i < count; i++)
    {
        o << indent() << "rtl_uString * pEnumValue" << i << " = 0;\n";
    }

    o << indent() << "rtl_uString_newFromAscii( &pTypeName, \""
      << m_typeName.replace('/', '.') << "\") );\n\n";

    for (i = 0; i < count; i++)
    {
        o << indent() << "rtl_uString_newFromAscii( &pEnumValue" << i << ", \""
            << m_reader.getFieldName((sal_uInt16)i) << "\" );\n";
        o << indent() << "enumValueNames[" << i << "] = pEnumValue" << i << ";\n";
    }

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
    o << indent() << "pTypeName,\n"
      << indent() << "(sal_Int32)" << m_name << "_" << m_reader.getFieldName(0) << ",\n"
      << indent() << count << ", enumValueNames, enumValues );\n\n";
    dec();

    o << indent() << "typelib_typedescription_register( &pTD );\n";

    o << indent() << "typelib_typedescriptionreference_new( &s_pType_ " << typeName
      << getTypeClass(OString(), sal_True) << ", pTD);\n\n";

    o << indent() << "typelib_typedescription_release( pTD );\n"
      << indent() << "rtl_uString_release( pTypeName );\n";
    for (i = 0; i < count; i++)
    {
        o << indent() << "rtl_uString_release(  pEnumValue" << i << " );\n";
    }

    dec();
    o << indent() << "}\n";
    o << indent() << "osl_releaseMutex( pMutex );\n";
    dec();
    o << indent() << "}\n\n"
      << indent() << "typelib_typedescriptionreference_acquire( s_pType_" << typeName <<" );\n"
      << indent() << "return &s_pType_" << typeName <<" );\n";

    dec();
    o << "}\n";

    dumpCloseExternC(o);
}

//*************************************************************************
// TypeDefType
//*************************************************************************
TypeDefType::TypeDefType(TypeReader& typeReader,
                             const OString& typeName,
                            const TypeManager& typeMgr,
                            const TypeDependency& typeDependencies)
    : CunoType(typeReader, typeName, typeMgr, typeDependencies)
{
}

TypeDefType::~TypeDefType()
{

}

sal_Bool TypeDefType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "H"));
    o << "\n";

    dumpDefaultHIncludes(o);
    o << "\n";
    dumpDepIncludes(o, m_typeName, "h");
    o << "\n";

    dumpOpenExternC(o);

    dumpDeclaration(o);

    if ( m_cunoTypeLib )
    {
        o << "#ifdef CUNO_TYPELIB\n"
          << "typelib_TypeDescriptionReference ** SAL_CALL getCUnoType_" << m_name << "() SAL_THROW_EXTERN_C( () );\n"
          << "#endif\n\n";
    }

    dumpCloseExternC(o);

    o << "#endif /* "<< headerDefine << " */\n";

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

sal_Bool TypeDefType::dumpCFile(FileStream& o)
    throw( CannotDumpException )
{
    dumpInclude(o, m_typeName, "h");
    o << "\n";
    dumpDefaultCIncludes(o);
    o << "\n";
    dumpDepIncludes(o, m_typeName, "h");
    o << "\n";
    dumpGetCunoType(o);
    return sal_True;
}

void TypeDefType::dumpGetCunoType(FileStream& o)
{
    if ( m_cunoTypeLeak )
    {
        dumpLGetCunoType(o);
        return;
    }
    if ( !m_cunoTypeDynamic )
    {
        dumpCGetCunoType(o);
        return;
    }
}

void TypeDefType::dumpCGetCunoType(FileStream& o)
{
}

void TypeDefType::dumpLGetCunoType(FileStream& o)
{
}

//*************************************************************************
// produceType
//*************************************************************************
sal_Bool produceType(const OString& typeName,
                     TypeManager& typeMgr,
                     TypeDependency& typeDependencies,
                     CunoOptions* pOptions)
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

    if( !checkTypeDependencies(typeMgr, typeDependencies, typeName))
        return sal_False;

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


