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

#ifndef INCLUDED_CODEMAKER_SOURCE_CUNOMAKER_CUNOTYPE_HXX_
#define INCLUDED_CODEMAKER_SOURCE_CUNOMAKER_CUNOTYPE_HXX

#include    <codemaker/typemanager.hxx>
#include    <codemaker/dependency.hxx>

enum BASETYPE
{
    BT_INVALID,
    BT_VOID,
    BT_ANY,
    BT_TYPE,
    BT_BOOLEAN,
    BT_CHAR,
    BT_STRING,
    BT_FLOAT,
    BT_DOUBLE,
    BT_OCTET,
    BT_BYTE,
    BT_SHORT,
    BT_LONG,
    BT_HYPER,
    BT_UNSIGNED_SHORT,
    BT_UNSIGNED_LONG,
    BT_UNSIGNED_HYPER
};


enum CunoTypeDecl
{
    CUNOTYPEDECL_ALLTYPES,
    CUNOTYPEDECL_NOINTERFACES,
    CUNOTYPEDECL_ONLYINTERFACES
};

class CunoOptions;
class FileStream;

class CunoType
{
public:
    CunoType(TypeReader& typeReader,
             const ::rtl::OString& typeName,
             const TypeManager& typeMgr,
             const TypeDependency& typeDependencies);

    virtual ~CunoType();

    virtual sal_Bool dump(CunoOptions* pOptions) throw( CannotDumpException );
    virtual sal_Bool dumpDependedTypes(CunoOptions* pOptions)  throw( CannotDumpException );
    virtual sal_Bool dumpHFile(FileStream& o) throw( CannotDumpException ) = 0;
    virtual sal_Bool dumpCFile(FileStream& o) throw( CannotDumpException ) = 0;

    virtual ::rtl::OString dumpHeaderDefine(FileStream& o, sal_Char* prefix, sal_Bool bExtended=sal_False);
    virtual void dumpDefaultHIncludes(FileStream& o);
    virtual void dumpDefaultCIncludes(FileStream& o);
    virtual void dumpInclude(FileStream& o, const ::rtl::OString& typeName, sal_Char* prefix, sal_Bool bExtended=sal_False, sal_Bool bCaseSensitive=sal_False);
    virtual void dumpDepIncludes(FileStream& o, const ::rtl::OString& typeName, sal_Char* prefix);

    virtual void dumpOpenExternC(FileStream& o);
    virtual void dumpCloseExternC(FileStream& o);

    virtual void dumpGetCunoType(FileStream& o);
    virtual void dumpCGetCunoType(FileStream& o);
    virtual void dumpLGetCunoType(FileStream& o);

    virtual void dumpType(FileStream& o, const ::rtl::OString& type, sal_Bool bConst=sal_False,
                          sal_Bool bPointer=sal_False, sal_Bool bParam=sal_False)
                    throw( CannotDumpException );
    ::rtl::OString  getTypeClass(const ::rtl::OString& type="", sal_Bool bCStyle=sal_False);
    ::rtl::OString  getBaseType(const ::rtl::OString& type);
    void    dumpCppuGetType(FileStream& o, const ::rtl::OString& type, sal_Bool bDecl=sal_False, CunoTypeDecl eDeclFlag=CUNOTYPEDECL_ALLTYPES);
    void    dumpTypeInit(FileStream& o, const ::rtl::OString& type);
    BASETYPE isBaseType(const ::rtl::OString& type);

    ::rtl::OString typeToIdentifier(const ::rtl::OString& type);

    void    dumpConstantValue(FileStream& o, sal_uInt16 index);

    virtual sal_uInt32  getMemberCount();
    virtual sal_uInt32  getInheritedMemberCount();
    void    dumpInheritedMembers(FileStream& o, rtl::OString& superType);

    sal_Bool isSeqType(const ::rtl::OString& type, ::rtl::OString& baseType, ::rtl::OString& seqPrefix);
    sal_Bool isArrayType(const ::rtl::OString& type, ::rtl::OString& baseType, ::rtl::OString& arrayPrefix);
    sal_Bool isVoid(const ::rtl::OString& type)
        { return type.equals("void"); }
    void            inc(sal_uInt32 num=4);
    void            dec(sal_uInt32 num=4);
    ::rtl::OString  indent();
    ::rtl::OString  indent(sal_uInt32 num);
protected:
    virtual sal_uInt32  checkInheritedMemberCount(const TypeReader* pReader);

    ::rtl::OString  checkSpecialCunoType(const ::rtl::OString& type);
    ::rtl::OString  checkRealBaseType(const ::rtl::OString& type, sal_Bool bResolveTypeOnly = sal_False);
    void    dumpCppuGetTypeMemberDecl(FileStream& o, CunoTypeDecl eDeclFlag);

     sal_Bool isNestedType() const
        { return m_bIsNestedType; };

     RegistryKeyNames& getNestedTypeNames()
        { return  m_nestedTypeNames; };

     sal_Bool isNestedTypeByName(const ::rtl::OString& type);
     sal_Bool hasNestedType(const ::rtl::OString& type);

protected:
    sal_uInt32          m_inheritedMemberCount;

    sal_Bool            m_cunoTypeLib;
    sal_Bool            m_cunoTypeLeak;
    sal_Bool            m_cunoTypeDynamic;
    sal_uInt32          m_indentLength;
    ::rtl::OString      m_typeName;
    ::rtl::OString      m_name;
    TypeReader          m_reader;
    TypeManager&        m_typeMgr;
    TypeDependency      m_dependencies;
    sal_Bool            m_bIsNestedType;
    RegistryKeyNames    m_nestedTypeNames;
};

class InterfaceType : public CunoType
{
public:
    InterfaceType(TypeReader& typeReader,
                 const ::rtl::OString& typeName,
                 const TypeManager& typeMgr,
                 const TypeDependency& typeDependencies);

    virtual ~InterfaceType();

    sal_Bool    dumpDeclaration(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpHFile(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpCFile(FileStream& o) throw( CannotDumpException );

    void        dumpInheritedFunctions(FileStream& o, rtl::OString& superType);
    void        dumpAttributes(FileStream& o, const ::rtl::OString& interfaceType, TypeReader& reader );
    void        dumpMethods(FileStream& o, const ::rtl::OString& interfaceType, TypeReader& reader );
    void        dumpGetCunoType(FileStream& o);
    void        dumpCGetCunoType(FileStream& o);
    void        dumpCUnoAttributeTypeNames(FileStream& o, sal_Bool bRelease=sal_False);
    void        dumpCUnoMethodTypeNames(FileStream& o, sal_Bool bRelease=sal_False);
    void        dumpCUnoAttributeRefs(FileStream& o, sal_uInt32& index);
    void        dumpCUnoMethodRefs(FileStream& o, sal_uInt32& index);
    void        dumpCUnoAttributes(FileStream& o, sal_uInt32& index);
    void        dumpCUnoMethods(FileStream& o, sal_uInt32& index);
    void        dumpAttributesCppuDecl(FileStream& o, StringSet* pFinishedTypes, CunoTypeDecl eDeclFlag);
    void        dumpMethodsCppuDecl(FileStream& o, StringSet* pFinishedTypes, CunoTypeDecl eDeclFlag );

    sal_uInt32  getMemberCount();
    sal_uInt32  getInheritedMemberCount();

protected:
    sal_uInt32  checkInheritedMemberCount(const TypeReader* pReader);

protected:
    sal_uInt32  m_inheritedMemberCount;
    sal_Bool    m_hasAttributes;
    sal_Bool    m_hasMethods;
};

class ModuleType : public CunoType
{
public:
    ModuleType(TypeReader& typeReader,
                  const ::rtl::OString& typeName,
               const TypeManager& typeMgr,
               const TypeDependency& typeDependencies);

    virtual ~ModuleType();

    virtual sal_Bool    dump(CunoOptions* pOptions) throw( CannotDumpException );

    sal_Bool            dumpDeclaration(FileStream& o) throw( CannotDumpException );
    sal_Bool            dumpHFile(FileStream& o) throw( CannotDumpException );
    sal_Bool            dumpCFile(FileStream& o) throw( CannotDumpException );
    sal_Bool            hasConstants();
};

class ConstantsType : public ModuleType
{
public:
    ConstantsType(TypeReader& typeReader,
                  const ::rtl::OString& typeName,
               const TypeManager& typeMgr,
               const TypeDependency& typeDependencies);

    virtual ~ConstantsType();

    virtual sal_Bool    dump(CunoOptions* pOptions) throw( CannotDumpException );
};

class StructureType : public CunoType
{
public:
    StructureType(TypeReader& typeReader,
                  const ::rtl::OString& typeName,
                  const TypeManager& typeMgr,
                  const TypeDependency& typeDependencies);

    virtual ~StructureType();

    sal_Bool    dumpDeclaration(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpHFile(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpCFile(FileStream& o) throw( CannotDumpException );
};

class ExceptionType : public CunoType
{
public:
    ExceptionType(TypeReader& typeReader,
                  const ::rtl::OString& typeName,
                  const TypeManager& typeMgr,
                  const TypeDependency& typeDependencies);

    virtual ~ExceptionType();

    sal_Bool    dumpDeclaration(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpHFile(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpCFile(FileStream& o) throw( CannotDumpException );
};

class EnumType : public CunoType
{
public:
    EnumType(TypeReader& typeReader,
              const ::rtl::OString& typeName,
              const TypeManager& typeMgr,
              const TypeDependency& typeDependencies);

    virtual ~EnumType();

    sal_Bool    dumpDeclaration(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpHFile(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpCFile(FileStream& o) throw( CannotDumpException );

    void        dumpGetCunoType(FileStream& o);
    void        dumpCGetCunoType(FileStream& o);
};

class TypeDefType : public CunoType
{
public:
    TypeDefType(TypeReader& typeReader,
              const ::rtl::OString& typeName,
              const TypeManager& typeMgr,
              const TypeDependency& typeDependencies);

    virtual ~TypeDefType();

    sal_Bool    dumpDeclaration(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpHFile(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpCFile(FileStream& o) throw( CannotDumpException );

    void        dumpGetCunoType(FileStream& o);
    void        dumpLGetCunoType(FileStream& o);
    void        dumpCGetCunoType(FileStream& o);
};


sal_Bool produceType(const ::rtl::OString& typeName,
                     TypeManager& typeMgr,
                     TypeDependency& typeDependencies,
                     CunoOptions* pOptions)
                 throw( CannotDumpException );

/**
 * This function returns a C++ scoped name, represents the namespace
 * scoping of this type, e.g. com:.sun::star::uno::XInterface. If the scope of
 * the type is equal scope, the relativ name will be used.
 */
::rtl::OString scopedName(const ::rtl::OString& scope, const ::rtl::OString& type,
                   sal_Bool bNoNameSpace=sal_False);

::rtl::OString shortScopedName(const ::rtl::OString& scope, const ::rtl::OString& type,
                           sal_Bool bNoNameSpace=sal_False);


#endif // INCLUDED_CODEMAKER_SOURCE_CUNOMAKER_CUNOTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
