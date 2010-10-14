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

#ifndef _CORBAMAKER_CPPUTYPE_HXX_
#define _CORBAMAKER_CPPUTYPE_HXX_

#include <codemaker/typemanager.hxx>
#include <codemaker/dependency.hxx>

#include <hash_set>

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


class CorbaOptions;
class FileStream;

struct OStringEqual
{
    sal_Bool operator() (const ::rtl::OString& lhs, const ::rtl::OString& rhs) const
  { return lhs.equals( rhs );}
};

struct OStringHash : public std::unary_function< const ::rtl::OString &, size_t >
{
    size_t operator()( const ::rtl::OString & rStr ) const
        { return rStr.hashCode(); }
};

typedef std::hash_set< ::rtl::OString, OStringHash, OStringEqual > TypeSet;

class CorbaType
{
public:
    CorbaType(TypeReader& typeReader,
              const ::rtl::OString& typeName,
              const TypeManager& typeMgr,
              const TypeDependency& typeDependencies,
              TypeSet* generatedConversion);

    virtual ~CorbaType();

    virtual sal_Bool dump(CorbaOptions* pOptions, FileStream& o, TypeSet* allreadyDumped) throw( CannotDumpException );
    virtual sal_Bool dumpDependedTypes(CorbaOptions* pOptions, FileStream& o, TypeSet* allreadyDumped)  throw( CannotDumpException );
    virtual sal_Bool dumpConversionFunctions(FileStream& o, TypeSet* allreadyDumped) throw( CannotDumpException );

    static  void dumpDefaultHxxIncludes(FileStream& o);

    virtual void dumpInclude(FileStream& o, TypeSet* allreadyDumped, const ::rtl::OString& typeName, sal_Char* prefix, sal_Bool bExtended=sal_False, sal_Bool bCaseSensitive=sal_False);
    virtual void dumpDepIncludes(FileStream& o, TypeSet* allreadyDumped, const ::rtl::OString& typeName, sal_Char* prefix);

    virtual void dumpNameSpace(FileStream& o, sal_Bool bOpen = sal_True, sal_Bool bFull = sal_False, const ::rtl::OString& type="");
    virtual void dumpFunctions(FileStream& o) = 0;

    virtual ::rtl::OString printUnoType( const ::rtl::OString& type,
                                        sal_Bool bConst=sal_False,
                                        sal_Bool bRef=sal_False,
                                        sal_Bool bNative=sal_False)
        throw( CannotDumpException );

    virtual void dumpUnoType(FileStream& o,
                            const ::rtl::OString& type,
                            sal_Bool bConst=sal_False,
                            sal_Bool bRef=sal_False,
                            sal_Bool bNative=sal_False)
        throw( CannotDumpException );

     virtual ::rtl::OString printCorbaType(const ::rtl::OString& type,
                                            sal_Bool bConst,
                                            sal_Bool bRef)
        throw( CannotDumpException );

    virtual void dumpCorbaType(FileStream& o,
                                const ::rtl::OString& type,
                                sal_Bool bConst=sal_False,
                                sal_Bool bRef=sal_False)
        throw( CannotDumpException );

    sal_Bool isPassedAsPointer(const ::rtl::OString& type);
    sal_Bool isArray(const ::rtl::OString& type);

    sal_Bool isDerivedFromUnknown(const ::rtl::OString& typeName);


    ::rtl::OString printCorbaParameter(const ::rtl::OString& type,
                                        sal_Bool bOut = sal_False)
                                throw( CannotDumpException );

    ::rtl::OString  getTypeClass(const ::rtl::OString& type="",
                                sal_Bool bCStyle=sal_False);
    ::rtl::OString  getUnoBaseType(const ::rtl::OString& type);
    ::rtl::OString  getCorbaBaseType(const ::rtl::OString& type);

    void    dumpTypeInit(FileStream& o, const ::rtl::OString& type);
    BASETYPE isBaseType(const ::rtl::OString& type);

    ::rtl::OString typeToIdentifier(const ::rtl::OString& type);

    virtual sal_uInt32  getMemberCount();
    virtual sal_uInt32  getInheritedMemberCount();

    sal_Bool isNestedTypeByName(const ::rtl::OString& type);

    void            inc(sal_uInt32 num=4);
    void            dec(sal_uInt32 num=4);
    ::rtl::OString  indent();
    ::rtl::OString  indent(sal_uInt32 num);
protected:
    virtual sal_uInt32  checkInheritedMemberCount(const TypeReader* pReader);

    ::rtl::OString  checkSpecialCorbaType(const ::rtl::OString& type);
    ::rtl::OString  checkRealBaseType(const ::rtl::OString& type, sal_Bool bResolveTypeOnly = sal_False);

protected:
    sal_uInt32          m_inheritedMemberCount;
    sal_uInt32          m_indentLength;
    ::rtl::OString      m_typeName;
    ::rtl::OString      m_name;
    TypeReader          m_reader;
    TypeManager&        m_typeMgr;
    TypeDependency      m_dependencies;
    TypeSet* m_generatedConversions;
};

class InterfaceType : public CorbaType
{
public:
    InterfaceType(TypeReader& typeReader,
                const ::rtl::OString& typeName,
                const TypeManager& typeMgr,
                const TypeDependency& typeDependencies,
                TypeSet* generatedConversion);

    virtual ~InterfaceType();

    void        dumpFunctions(FileStream& o);

    void        dumpUnoMethods(FileStream& o, sal_Bool bDeclOnly, sal_Bool bDelegateToSuper);
    void        dumpCorbaMethods(FileStream& o, sal_Bool bDeclOnly);

    sal_uInt32  getMemberCount();
    sal_uInt32  getInheritedMemberCount();

protected:
    sal_uInt32  checkInheritedMemberCount(const TypeReader* pReader);

protected:
    sal_uInt32  m_inheritedMemberCount;
    sal_Bool    m_hasAttributes;
    sal_Bool    m_hasMethods;
};

class ModuleType : public CorbaType
{
public:
    ModuleType(TypeReader& typeReader,
                const ::rtl::OString& typeName,
                const TypeManager& typeMgr,
                const TypeDependency& typeDependencies,
                TypeSet* generatedConversion);

    virtual ~ModuleType();

    sal_Bool    dumpConversionFunctions(FileStream& o, TypeSet* allreadyDumped) throw( CannotDumpException );
    void        dumpFunctions(FileStream& o);
    sal_Bool    hasConstants();
};

class ConstantsType : public ModuleType
{
public:
    ConstantsType(TypeReader& typeReader,
                const ::rtl::OString& typeName,
                const TypeManager& typeMgr,
                const TypeDependency& typeDependencies,
                TypeSet* generatedConversion);

    virtual ~ConstantsType();

    void dumpFunctions(FileStream& o);
};

class StructureType : public CorbaType
{
public:
    StructureType(TypeReader& typeReader,
                const ::rtl::OString& typeName,
                const TypeManager& typeMgr,
                const TypeDependency& typeDependencies,
                TypeSet* generatedConversion);

    virtual ~StructureType();

    void        dumpFunctions(FileStream& o);

    sal_Bool    dumpSuperMember(FileStream& o,  const ::rtl::OString& super, sal_Bool bWithType);
};

class ExceptionType : public CorbaType
{
public:
    ExceptionType(TypeReader& typeReader,
                const ::rtl::OString& typeName,
                const TypeManager& typeMgr,
                const TypeDependency& typeDependencies,
                TypeSet* generatedConversions);

    virtual ~ExceptionType();

    void        dumpFunctions(FileStream& o);

    sal_Bool    dumpSuperMember(FileStream& o, const ::rtl::OString& super, sal_Bool bWithType);
};

class EnumType : public CorbaType
{
public:
    EnumType(TypeReader& typeReader,
            const ::rtl::OString& typeName,
            const TypeManager& typeMgr,
            const TypeDependency& typeDependencies,
            TypeSet* generatedConversion);

    virtual ~EnumType();

    void        dumpFunctions(FileStream& o);
};

class TypeDefType : public CorbaType
{
public:
    TypeDefType(TypeReader& typeReader,
                const ::rtl::OString& typeName,
                const TypeManager& typeMgr,
                const TypeDependency& typeDependencies,
                TypeSet* generatedConversion);

    virtual ~TypeDefType();

    void dumpFunctions(FileStream& o);
};


sal_Bool produceType(const ::rtl::OString& typeName,
                    TypeManager& typeMgr,
                    TypeDependency& typeDependencies,
                    CorbaOptions* pOptions,
                    FileStream& o, TypeSet* allreadyDumped,
                    TypeSet* generatedConversions)
    throw( CannotDumpException );

/**
 * This function returns a C++ scoped name, represents the namespace
 * scoping of this type, e.g. com:.sun::star::uno::XInterface. If the scope of
 * the type is equal scope, the relativ name will be used.
 */
::rtl::OString scopedName(const ::rtl::OString& scope,
                          const ::rtl::OString& type,
                             sal_Bool bNoNameSpace=sal_False);

::rtl::OString shortScopedName(const ::rtl::OString& scope,
                               const ::rtl::OString& type,
                                  sal_Bool bNoNameSpace=sal_False);


#endif // _CORBAMAKER_CPPUTYPE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
