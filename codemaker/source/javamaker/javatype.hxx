/*************************************************************************
 *
 *  $RCSfile: javatype.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-03-30 16:53:39 $
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

#ifndef _CPPUMAKER_CPPUTYPE_HXX_
#define _CPPUMAKER_CPPUTYPE_HXX_

#ifndef _CODEMAKER_TYPEMANAGER_HXX_
#include    <codemaker/typemanager.hxx>
#endif

#ifndef _CODEMAKER_DEPENDENCY_HXX_
#include    <codemaker/dependency.hxx>
#endif

#include "codemaker/options.hxx"

#include "registry/reader.hxx"
#include "registry/types.h"

namespace codemaker { struct ExceptionTreeNode; }

enum JavaTypeDecl
{
    CPPUTYPEDECL_ALLTYPES,
    CPPUTYPEDECL_NOINTERFACES,
    CPPUTYPEDECL_ONLYINTERFACES
};

static const sal_Int32 UIT_IN        = 0x00000001;
static const sal_Int32 UIT_OUT       = 0x00000002;
static const sal_Int32 UIT_UNSIGNED  = 0x00000004;
static const sal_Int32 UIT_READONLY  = 0x00000008;
static const sal_Int32 UIT_ONEWAY    = 0x00000010;
static const sal_Int32 UIT_CONST     = 0x00000020;
static const sal_Int32 UIT_ANY       = 0x00000040;
static const sal_Int32 UIT_INTERFACE = 0x00000080;
static const sal_Int32 UIT_BOUND     = 0x00000100;

enum UnoTypeInfo
{
  UNOTYPEINFO_INVALID,
  UNOTYPEINFO_METHOD,
  UNOTYPEINFO_PARAMETER,
  UNOTYPEINFO_ATTIRBUTE,
  UNOTYPEINFO_MEMBER
};

struct UnoInfo
{
    UnoInfo()
        : m_unoTypeInfo(UNOTYPEINFO_INVALID)
        , m_index(-1)
        , m_flags(0)
        {}

    UnoInfo(const ::rtl::OString& name, const ::rtl::OString& methodName,
            UnoTypeInfo unoTypeInfo, sal_Int32 index, sal_Int32 flags)
        : m_name(name)
        , m_methodName(methodName)
        , m_unoTypeInfo(unoTypeInfo)
        , m_index(index)
        , m_flags(flags)
        {}

    ::rtl::OString  m_name;
    ::rtl::OString  m_methodName;
    UnoTypeInfo     m_unoTypeInfo;
    sal_Int32       m_index;
    sal_Int32       m_flags;
};

inline int operator == (const UnoInfo& u1, const UnoInfo& u2)
{
    return ((u1.m_name == u2.m_name) && (u1.m_methodName == u2.m_methodName));
}

inline int operator < (const UnoInfo& u1, const UnoInfo& u2)
{
    if (u1.m_name == u2.m_name)
        return (u1.m_methodName < u2.m_methodName);
    else
        return (u1.m_name < u2.m_name);
}

typedef ::std::list< UnoInfo > UnoInfoList;

class JavaOptions;
class FileStream;

class JavaType
{
public:
    JavaType(typereg::Reader& typeReader,
             const ::rtl::OString& typeName,
             const TypeManager& typeMgr,
             const TypeDependency& typeDependencies);

    virtual ~JavaType();

    virtual sal_Bool dump(JavaOptions* pOptions) throw( CannotDumpException );
    void dumpDependedTypes(JavaOptions * options);
    virtual sal_Bool dumpFile(FileStream& o) throw( CannotDumpException ) { return sal_True; }

    void dumpPackage(FileStream& o, sal_Bool bFullScope = sal_False);

    virtual void dumpType(FileStream& o, const ::rtl::OString& type) throw( CannotDumpException );
    void dumpTypeInit(FileStream& o, const ::rtl::OString& name, const ::rtl::OString& type);
    sal_Bool isUnsigned(const ::rtl::OString& type);
    sal_Bool isAny(const ::rtl::OString& type);
    sal_Bool isInterface(const ::rtl::OString& type);

    void    dumpConstantValue(FileStream& o, sal_uInt16 index);

    // only used for structs and exceptions
    sal_Bool    dumpMemberConstructor(FileStream& o);
    sal_Bool    dumpInheritedMembers( FileStream& o, const ::rtl::OString& type,
                                      sal_Bool first, sal_Bool withType= sal_True );
    void        dumpSeqStaticMember(FileStream& o, const ::rtl::OString& type,
                                    const ::rtl::OString& name);

    void            inc(sal_uInt32 num=4);
    void            dec(sal_uInt32 num=4);
    ::rtl::OString  indent();
    ::rtl::OString  indent(sal_uInt32 num);
protected:
    rtl::OString resolveTypedefs(rtl::OString const & unoType);

    rtl::OString unfoldType(rtl::OString const & unoType, sal_Int32 * rank = 0);

    ::rtl::OString  checkRealBaseType(const ::rtl::OString& type);

    virtual rtl::OString translateTypeName() { return m_typeName; }

protected:
    sal_uInt32          m_indentLength;
    ::rtl::OString      m_typeName;
    ::rtl::OString      m_name;
    typereg::Reader     m_reader;
    TypeManager&        m_typeMgr;
    TypeDependency      m_dependencies;
};

class InterfaceType : public JavaType
{
public:
    InterfaceType(typereg::Reader& typeReader,
                 const ::rtl::OString& typeName,
                 const TypeManager& typeMgr,
                 const TypeDependency& typeDependencies);

    virtual ~InterfaceType();

    sal_Bool    dumpFile(FileStream& o) throw( CannotDumpException );

    void        dumpAttributes(FileStream& o, UnoInfoList* pUnoInfos);
    void        dumpMethods(FileStream& o, UnoInfoList* pUnoInfos);

    void        dumpUnoInfo(FileStream& o, const UnoInfo& unoInfo, sal_Int32 * index);

private:
    void dumpExceptionSpecification(FileStream & out, sal_uInt16 methodIndex);

    void dumpAttributeExceptionSpecification(
        FileStream & out, rtl::OUString const & name, RTMethodMode sort);
};

class ModuleType : public JavaType
{
public:
    ModuleType(typereg::Reader& typeReader,
                  const ::rtl::OString& typeName,
               const TypeManager& typeMgr,
               const TypeDependency& typeDependencies);

    virtual ~ModuleType();

    sal_Bool    dump(JavaOptions* pOptions) throw( CannotDumpException );
    sal_Bool    hasConstants();
};

class ConstantsType : public JavaType
{
public:
    ConstantsType(typereg::Reader& typeReader,
                  const ::rtl::OString& typeName,
               const TypeManager& typeMgr,
               const TypeDependency& typeDependencies);

    virtual ~ConstantsType();

    sal_Bool    dumpFile(FileStream& o) throw( CannotDumpException );
};

class StructureType : public JavaType
{
public:
    StructureType(typereg::Reader& typeReader,
                  const ::rtl::OString& typeName,
                  const TypeManager& typeMgr,
                  const TypeDependency& typeDependencies);

    virtual ~StructureType();

    sal_Bool    dumpFile(FileStream& o) throw( CannotDumpException );
};

class ExceptionType : public JavaType
{
public:
    ExceptionType(typereg::Reader& typeReader,
                  const ::rtl::OString& typeName,
                  const TypeManager& typeMgr,
                  const TypeDependency& typeDependencies);

    virtual ~ExceptionType();

    sal_Bool    dumpFile(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpSimpleMemberConstructor(FileStream& o);
};

class EnumType : public JavaType
{
public:
    EnumType(typereg::Reader& typeReader,
              const ::rtl::OString& typeName,
              const TypeManager& typeMgr,
              const TypeDependency& typeDependencies);

    virtual ~EnumType();

    sal_Bool    dumpFile(FileStream& o) throw( CannotDumpException );
};

class TypeDefType : public JavaType
{
public:
    TypeDefType(typereg::Reader& typeReader,
              const ::rtl::OString& typeName,
              const TypeManager& typeMgr,
              const TypeDependency& typeDependencies);

    virtual ~TypeDefType();

    sal_Bool    dump(JavaOptions* pOptions) throw( CannotDumpException );
};

class ServiceType: public JavaType {
public:
    ServiceType(
        typereg::Reader & reader, rtl::OString const & name,
        TypeManager const & manager, TypeDependency const & dependencies):
        JavaType(reader, name, manager, dependencies) {}

    bool isSingleInterfaceBased();

    virtual sal_Bool dumpFile(FileStream & out) throw (CannotDumpException);

private:
    virtual rtl::OString translateTypeName();

    void dumpCatchClauses(
        FileStream & out, codemaker::ExceptionTreeNode const * node);

    void dumpAny(
        FileStream & out, rtl::OString const & javaExpression,
        rtl::OString const & unoType);
};

class SingletonType: public JavaType {
public:
    SingletonType(
        typereg::Reader & reader, rtl::OString const & name,
        TypeManager const & manager, TypeDependency const & dependencies):
        JavaType(reader, name, manager, dependencies) {}

    bool isInterfaceBased();

    virtual sal_Bool dumpFile(FileStream & out) throw (CannotDumpException);

private:
    virtual rtl::OString translateTypeName();
};

sal_Bool produceType(const ::rtl::OString& typeName,
                     TypeManager& typeMgr,
                     TypeDependency& typeDependencies,
                     JavaOptions* pOptions)
                 throw( CannotDumpException );

/**
 * This function returns a Java scoped name, represents the package
 * scoping of this type, e.g. com.sun.star.uno.XInterface. If the scope of
 * the type is equal scope, the relativ name will be used.
 */
::rtl::OString scopedName(const ::rtl::OString& scope, const ::rtl::OString& type,
                   sal_Bool bNoNameSpace=sal_False);


#endif // _CPPUMAKER_CPPUTYPE_HXX_

