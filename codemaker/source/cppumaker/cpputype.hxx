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

#ifndef INCLUDED_CODEMAKER_SOURCE_CPPUMAKER_CPPUTYPE_HXX
#define INCLUDED_CODEMAKER_SOURCE_CPPUMAKER_CPPUTYPE_HXX

#include "codemaker/dependencies.hxx"
#include "codemaker/options.hxx"
#include "codemaker/typemanager.hxx"
#include "codemaker/commoncpp.hxx"
#include "registry/reader.hxx"
#include "registry/types.h"
#include "rtl/string.hxx"

namespace rtl { class OUString; }
namespace codemaker {
    namespace cppumaker { class Includes; }
    struct ExceptionTreeNode;
    class GeneratedTypeSet;
}

enum CppuTypeDecl
{
    CPPUTYPEDECL_ALLTYPES,
    CPPUTYPEDECL_NOINTERFACES,
    CPPUTYPEDECL_ONLYINTERFACES
};

class CppuOptions;
class FileStream;

class CppuType
{
public:
    CppuType(typereg::Reader& typeReader,
             const ::rtl::OString& typeName,
             const TypeManager& typeMgr);

    virtual ~CppuType();

    virtual sal_Bool dump(CppuOptions* pOptions) throw( CannotDumpException );
    sal_Bool dumpFile(CppuOptions* pOptions,
                      const ::rtl::OString& sExtension,
                      const ::rtl::OString& sName,
                      const ::rtl::OString& sOutPath )
        throw( CannotDumpException );
    void dumpDependedTypes(
        codemaker::GeneratedTypeSet & generated, CppuOptions * options);
    virtual sal_Bool dumpHFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException ) = 0;
    virtual sal_Bool dumpHxxFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException ) = 0;

    ::rtl::OString dumpHeaderDefine(
        FileStream& o, char const * prefix, sal_Bool bExtended=sal_False);

    void dumpGetCppuType(FileStream & out);
    virtual void dumpLightGetCppuType(FileStream& o);
    virtual void dumpNormalGetCppuType(FileStream& o);
    virtual void dumpComprehensiveGetCppuType(FileStream& o);

    virtual void dumpType(FileStream& o, const ::rtl::OString& type, bool bConst=false,
                          bool bRef=false, bool bNative=false, bool cppuUnoType=false)
        const throw( CannotDumpException );
    ::rtl::OString  getTypeClass(const ::rtl::OString& type="", sal_Bool bCStyle=sal_False);
    void    dumpCppuGetType(FileStream& o, const ::rtl::OString& type, sal_Bool bDecl=sal_False, CppuTypeDecl eDeclFlag=CPPUTYPEDECL_ALLTYPES);

    ::rtl::OString typeToIdentifier(const ::rtl::OString& type);

    void    dumpConstantValue(FileStream& o, sal_uInt16 index);

    virtual sal_uInt32  getMemberCount();
    virtual sal_uInt32  getInheritedMemberCount();

    void            inc(sal_Int32 num=4);
    void            dec(sal_Int32 num=4);
    ::rtl::OString  indent() const;
protected:
    virtual sal_uInt32 checkInheritedMemberCount(
        const typereg::Reader* pReader);

    bool passByReference(rtl::OString const & unoType);

    ::rtl::OString  resolveTypedefs(const ::rtl::OString& type) const;
    ::rtl::OString  checkRealBaseType(const ::rtl::OString& type, sal_Bool bResolveTypeOnly = sal_False) const;
    void    dumpCppuGetTypeMemberDecl(FileStream& o, CppuTypeDecl eDeclFlag);

    codemaker::cpp::IdentifierTranslationMode isGlobal() const;

    virtual void dumpDeclaration(FileStream & out) throw (CannotDumpException);

    virtual void addSpecialDependencies() {}

    virtual bool dumpFiles(CppuOptions * options, rtl::OString const & outPath);

    virtual void addLightGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual void addNormalGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual void addComprehensiveGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual bool isPolymorphic() const;

    virtual void dumpTemplateHead(FileStream & out) const;

    virtual void dumpTemplateParameters(FileStream & out) const;

    void dumpGetCppuTypePreamble(FileStream & out);

    void dumpGetCppuTypePostamble(FileStream & out);

    void addDefaultHIncludes(codemaker::cppumaker::Includes & includes) const;
    void addDefaultHxxIncludes(codemaker::cppumaker::Includes & includes) const;

    void dumpInitializer(
        FileStream & out, bool parameterized, rtl::OUString const & type) const;

    void dumpHFileContent(
        FileStream & out, codemaker::cppumaker::Includes & includes);

protected:
    sal_uInt32          m_inheritedMemberCount;

    bool                m_cppuTypeLeak;
    bool                m_cppuTypeDynamic;
    sal_Int32           m_indentLength;
    ::rtl::OString      m_typeName;
    ::rtl::OString      m_name;
    typereg::Reader     m_reader;
    TypeManager const & m_typeMgr;
    codemaker::Dependencies m_dependencies;

private:
    void addGetCppuTypeIncludes(codemaker::cppumaker::Includes & includes)
        const;
};

class InterfaceType : public CppuType
{
public:
    InterfaceType(typereg::Reader& typeReader,
                 const ::rtl::OString& typeName,
                 const TypeManager& typeMgr);

    virtual ~InterfaceType();

    virtual void dumpDeclaration(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpHFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException );
    sal_Bool    dumpHxxFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException );

    void        dumpAttributes(FileStream& o);
    void        dumpMethods(FileStream& o);
    void        dumpNormalGetCppuType(FileStream& o);
    void        dumpComprehensiveGetCppuType(FileStream& o);
    void        dumpCppuAttributeRefs(FileStream& o, sal_uInt32& index);
    void        dumpCppuMethodRefs(FileStream& o, sal_uInt32& index);
    void        dumpCppuAttributes(FileStream& o, sal_uInt32& index);
    void        dumpCppuMethods(FileStream& o, sal_uInt32& index);
    void        dumpAttributesCppuDecl(FileStream& o, StringSet* pFinishedTypes, CppuTypeDecl eDeclFlag);
    void        dumpMethodsCppuDecl(FileStream& o, StringSet* pFinishedTypes, CppuTypeDecl eDeclFlag );

    sal_uInt32  getMemberCount();
    sal_uInt32  getInheritedMemberCount();

protected:
    virtual void addSpecialDependencies();

    virtual void addComprehensiveGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    sal_uInt32  checkInheritedMemberCount(const typereg::Reader* pReader);

protected:
    sal_uInt32  m_inheritedMemberCount;
    bool        m_hasAttributes;
    bool        m_hasMethods;

private:
    void dumpExceptionSpecification(
        FileStream & out, sal_uInt32 methodIndex, bool runtimeException);

    void dumpAttributeExceptionSpecification(
        FileStream & out, rtl::OUString const & name, RTMethodMode sort);

    void dumpExceptionTypeName(
        FileStream & out, char const * prefix, sal_uInt32 index,
        rtl::OUString name);

    sal_Int32 dumpExceptionTypeNames(
        FileStream & out, char const * prefix, sal_uInt16 methodIndex,
        bool runtimeException);

    sal_Int32 dumpAttributeExceptionTypeNames(
        FileStream & out, char const * prefix, rtl::OUString const & name,
        RTMethodMode sort);
};

class ConstantsType : public CppuType
{
public:
    ConstantsType(typereg::Reader& typeReader,
                  const ::rtl::OString& typeName,
                  const TypeManager& typeMgr);

    virtual ~ConstantsType();

    virtual sal_Bool    dump(CppuOptions* pOptions) throw( CannotDumpException );

    virtual void dumpDeclaration(FileStream& o) throw( CannotDumpException );
    sal_Bool            dumpHFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException );
    sal_Bool            dumpHxxFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException );
    sal_Bool            hasConstants();
};

class ModuleType : public ConstantsType
{
public:
    ModuleType(typereg::Reader& typeReader,
                  const ::rtl::OString& typeName,
               const TypeManager& typeMgr);

    virtual ~ModuleType();

protected:
    virtual bool dumpFiles(CppuOptions * options, rtl::OString const & outPath);
};

class StructureType : public CppuType
{
public:
    StructureType(typereg::Reader& typeReader,
                  const ::rtl::OString& typeName,
                  const TypeManager& typeMgr);

    virtual ~StructureType();

    virtual void dumpDeclaration(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpHFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException );
    sal_Bool    dumpHxxFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException );

    virtual void dumpLightGetCppuType(FileStream & out);

    virtual void dumpNormalGetCppuType(FileStream & out);

    virtual void dumpComprehensiveGetCppuType(FileStream & out);

    sal_Bool    dumpSuperMember(FileStream& o, const ::rtl::OString& super, sal_Bool bWithType);

protected:
    virtual void addLightGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual void addNormalGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual void addComprehensiveGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual bool isPolymorphic() const;

    virtual void dumpTemplateHead(FileStream & out) const;

    virtual void dumpTemplateParameters(FileStream & out) const;
};

class ExceptionType : public CppuType
{
public:
    ExceptionType(typereg::Reader& typeReader,
                  const ::rtl::OString& typeName,
                  const TypeManager& typeMgr);

    virtual ~ExceptionType();

    virtual void dumpDeclaration(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpHFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException );
    sal_Bool    dumpHxxFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException );

    sal_Bool    dumpSuperMember(FileStream& o, const ::rtl::OString& super, sal_Bool bWithType);
};

class EnumType : public CppuType
{
public:
    EnumType(typereg::Reader& typeReader,
              const ::rtl::OString& typeName,
              const TypeManager& typeMgr);

    virtual ~EnumType();

    virtual void dumpDeclaration(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpHFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException );
    sal_Bool    dumpHxxFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException );

    void        dumpNormalGetCppuType(FileStream& o);
    void        dumpComprehensiveGetCppuType(FileStream& o);
};

class TypeDefType : public CppuType
{
public:
    TypeDefType(typereg::Reader& typeReader,
              const ::rtl::OString& typeName,
              const TypeManager& typeMgr);

    virtual ~TypeDefType();

    virtual void dumpDeclaration(FileStream& o) throw( CannotDumpException );
    sal_Bool    dumpHFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException );
    sal_Bool    dumpHxxFile(FileStream& o, codemaker::cppumaker::Includes & includes) throw( CannotDumpException );
};

class ConstructiveType: public CppuType {
public:
    ConstructiveType(
        typereg::Reader & reader, rtl::OString const & name,
        TypeManager const & manager):
        CppuType(reader, name, manager) {}

    virtual sal_Bool dumpHFile(
        FileStream & o, codemaker::cppumaker::Includes & includes)
        throw (CannotDumpException);

protected:
    virtual bool dumpFiles(CppuOptions * options, rtl::OString const & outPath);
};

class ServiceType: public ConstructiveType {
public:
    ServiceType(
        typereg::Reader & reader, rtl::OString const & name,
        TypeManager const & manager):
        ConstructiveType(reader, name, manager) {}

    bool isSingleInterfaceBased();

    virtual sal_Bool dumpHxxFile(
        FileStream & o, codemaker::cppumaker::Includes & includes)
        throw (CannotDumpException);

private:
    virtual void addSpecialDependencies();

    bool isDefaultConstructor(sal_uInt16 ctorIndex) const;

    bool hasRestParameter(sal_uInt16 ctorIndex) const;

    void dumpCatchClauses(
        FileStream & out, codemaker::ExceptionTreeNode const * node);
};

class SingletonType: public ConstructiveType {
public:
    SingletonType(
        typereg::Reader & reader, rtl::OString const & name,
        TypeManager const & manager):
        ConstructiveType(reader, name, manager) {}

    bool isInterfaceBased();

    virtual sal_Bool dumpHxxFile(
        FileStream & o, codemaker::cppumaker::Includes & includes)
        throw (CannotDumpException);
};

bool produceType(const ::rtl::OString& typeName,
                     TypeManager const & typeMgr,
                     codemaker::GeneratedTypeSet & generated,
                     CppuOptions* pOptions)
                 throw( CannotDumpException );

bool produceType(RegistryKey& typeName, bool bIsExtraType,
                     TypeManager const & typeMgr,
                     codemaker::GeneratedTypeSet & generated,
                     CppuOptions* pOptions)
                 throw( CannotDumpException );

/**
 * This function returns a C++ scoped name, represents the namespace
 * scoping of this type, e.g. com:.sun::star::uno::XInterface.
 */
// ::rtl::OString scopedName(const ::rtl::OString& scope, const ::rtl::OString& type,
//                 sal_Bool bNoNameSpace=sal_False);

#endif // INCLUDED_CODEMAKER_SOURCE_CPPUMAKER_CPPUTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
