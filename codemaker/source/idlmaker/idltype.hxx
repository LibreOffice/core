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

#ifndef INCLUDED_CODEMAKER_SOURCE_IDLMAKER_IDLTYPE_HXX
#define INCLUDED_CODEMAKER_SOURCE_IDLMAKER_IDLTYPE_HXX

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


enum IdlTypeDecl
{
    CPPUTYPEDECL_ALLTYPES,
    CPPUTYPEDECL_NOINTERFACES,
    CPPUTYPEDECL_ONLYINTERFACES
};

class IdlOptions;
class FileStream;

class IdlType
{
public:
    IdlType(TypeReader& typeReader,
             const ::rtl::OString& typeName,
             const TypeManager& typeMgr,
             const TypeDependency& typeDependencies);

    virtual ~IdlType();

    virtual sal_Bool dump(IdlOptions* pOptions) throw( CannotDumpException );
    virtual sal_Bool dumpDependedTypes(IdlOptions* pOptions)  throw( CannotDumpException );
    virtual sal_Bool dumpHFile(FileStream& o) throw( CannotDumpException ) = 0;

    virtual ::rtl::OString dumpHeaderDefine(FileStream& o, sal_Char* prefix );
    virtual void dumpDefaultHIncludes(FileStream& o);
    virtual void dumpInclude(FileStream& o, const ::rtl::OString& genTypeName, const ::rtl::OString& typeName, sal_Char* prefix );

    virtual void dumpDepIncludes(FileStream& o, const ::rtl::OString& typeName, sal_Char* prefix);

    virtual void dumpNameSpace(FileStream& o, sal_Bool bOpen = sal_True, sal_Bool bFull = sal_False, const ::rtl::OString& type="");

    virtual void dumpType(FileStream& o, const ::rtl::OString& type)
                    throw( CannotDumpException );
    ::rtl::OString  getBaseType(const ::rtl::OString& type);
    void    dumpIdlGetType(FileStream& o, const ::rtl::OString& type, sal_Bool bDecl=sal_False, IdlTypeDecl eDeclFlag=CPPUTYPEDECL_ALLTYPES);
    BASETYPE isBaseType(const ::rtl::OString& type);

    void    dumpConstantValue(FileStream& o, sal_uInt16 index);

    virtual sal_uInt32  getMemberCount();
    virtual sal_uInt32  getInheritedMemberCount();

    void            inc(sal_uInt32 num=4);
    void            dec(sal_uInt32 num=4);
    ::rtl::OString  indent();
    ::rtl::OString  indent(sal_uInt32 num);
protected:
    virtual sal_uInt32  checkInheritedMemberCount(const TypeReader* pReader);

    ::rtl::OString  checkSpecialIdlType(const ::rtl::OString& type);
    ::rtl::OString  checkRealBaseType(const ::rtl::OString& type, sal_Bool bResolveTypeOnly = sal_False);

protected:
    sal_uInt32          m_inheritedMemberCount;

    sal_uInt32          m_indentLength;
    ::rtl::OString      m_typeName;
    ::rtl::OString      m_name;
    TypeReader          m_reader;
    TypeManager&        m_typeMgr;
    TypeDependency      m_dependencies;
};

class InterfaceType : public IdlType
{
public:
    InterfaceType(TypeReader& typeReader,
                 const ::rtl::OString& typeName,
                 const TypeManager& typeMgr,
                 const TypeDependency& typeDependencies);

    virtual ~InterfaceType();

    sal_Bool    dumpHFile(FileStream& o) throw( CannotDumpException );

    void        dumpAttributes(FileStream& o);
    void        dumpMethods(FileStream& o);

    sal_uInt32  getMemberCount();
    sal_uInt32  getInheritedMemberCount();

protected:
    sal_uInt32  checkInheritedMemberCount(const TypeReader* pReader);

protected:
    sal_uInt32  m_inheritedMemberCount;
    sal_Bool    m_hasAttributes;
    sal_Bool    m_hasMethods;
};

class ModuleType : public IdlType
{
public:
    ModuleType(TypeReader& typeReader,
                  const ::rtl::OString& typeName,
               const TypeManager& typeMgr,
               const TypeDependency& typeDependencies);

    virtual ~ModuleType();

    virtual sal_Bool    dump(IdlOptions* pOptions) throw( CannotDumpException );
    sal_Bool            dumpHFile(FileStream& o) throw( CannotDumpException );
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

    virtual sal_Bool    dump(IdlOptions* pOptions) throw( CannotDumpException );
};

class StructureType : public IdlType
{
public:
    StructureType(TypeReader& typeReader,
                  const ::rtl::OString& typeName,
                  const TypeManager& typeMgr,
                  const TypeDependency& typeDependencies);

    virtual ~StructureType();

    sal_Bool    dumpHFile(FileStream& o) throw( CannotDumpException );

    void        dumpSuperMember(FileStream& o, const ::rtl::OString& super);
};

class ExceptionType : public IdlType
{
public:
    ExceptionType(TypeReader& typeReader,
                  const ::rtl::OString& typeName,
                  const TypeManager& typeMgr,
                  const TypeDependency& typeDependencies);

    virtual ~ExceptionType();

    sal_Bool    dumpHFile(FileStream& o) throw( CannotDumpException );

    void        dumpSuperMember(FileStream& o, const ::rtl::OString& super);
};

class EnumType : public IdlType
{
public:
    EnumType(TypeReader& typeReader,
              const ::rtl::OString& typeName,
              const TypeManager& typeMgr,
              const TypeDependency& typeDependencies);

    virtual ~EnumType();

    sal_Bool    dumpHFile(FileStream& o) throw( CannotDumpException );
};

class TypeDefType : public IdlType
{
public:
    TypeDefType(TypeReader& typeReader,
              const ::rtl::OString& typeName,
              const TypeManager& typeMgr,
              const TypeDependency& typeDependencies);

    virtual ~TypeDefType();

    sal_Bool    dumpHFile(FileStream& o) throw( CannotDumpException );
};


sal_Bool produceType(const ::rtl::OString& typeName,
                     TypeManager& typeMgr,
                     TypeDependency& typeDependencies,
                     IdlOptions* pOptions)
                 throw( CannotDumpException );

/**
 * This function returns a C++ scoped name, represents the namespace
 * scoping of this type, e.g. com:.sun::star::uno::XInterface. If the scope of
 * the type is equal scope, the relativ name will be used.
 */
::rtl::OString scopedName(const ::rtl::OString& scope, const ::rtl::OString& type, sal_Bool bNoNameSpace = sal_False );

::rtl::OString scope(const ::rtl::OString& scope, const ::rtl::OString& type );


#endif // INCLUDED_CODEMAKER_SOURCE_IDLMAKER_IDLTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
