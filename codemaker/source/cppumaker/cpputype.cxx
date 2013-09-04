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

#include "sal/config.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <vector>

#include "boost/noncopyable.hpp"
#include "rtl/alloc.h"
#include "rtl/ref.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "rtl/strbuf.hxx"
#include "unoidl/unoidl.hxx"

#include "codemaker/commoncpp.hxx"
#include "codemaker/exceptiontree.hxx"
#include "codemaker/generatedtypeset.hxx"
#include "codemaker/typemanager.hxx"
#include "codemaker/unotype.hxx"

#include "cpputype.hxx"
#include "cppuoptions.hxx"
#include "dependencies.hxx"
#include "dumputils.hxx"
#include "includes.hxx"

namespace {

bool isBootstrapType(OUString const & name) {
    static char const * const names[] = {
        "com.sun.star.beans.PropertyAttribute",
        "com.sun.star.beans.PropertyState",
        "com.sun.star.beans.PropertyValue",
        "com.sun.star.beans.XFastPropertySet",
        "com.sun.star.beans.XMultiPropertySet",
        "com.sun.star.beans.XPropertyAccess",
        "com.sun.star.beans.XPropertySet",
        "com.sun.star.beans.XPropertySetOption",
        "com.sun.star.bridge.UnoUrlResolver",
        "com.sun.star.bridge.XUnoUrlResolver",
        "com.sun.star.connection.SocketPermission",
        "com.sun.star.container.XElementAccess",
        "com.sun.star.container.XEnumerationAccess",
        "com.sun.star.container.XHierarchicalNameAccess",
        "com.sun.star.container.XNameAccess",
        "com.sun.star.container.XNameContainer",
        "com.sun.star.container.XNameReplace",
        "com.sun.star.container.XSet",
        "com.sun.star.io.FilePermission",
        "com.sun.star.io.IOException",
        "com.sun.star.lang.DisposedException",
        "com.sun.star.lang.WrappedTargetRuntimeException",
        "com.sun.star.lang.XComponent",
        "com.sun.star.lang.XEventListener",
        "com.sun.star.lang.XInitialization",
        "com.sun.star.lang.XMultiComponentFactory",
        "com.sun.star.lang.XMultiServiceFactory",
        "com.sun.star.lang.XServiceInfo",
        "com.sun.star.lang.XSingleComponentFactory",
        "com.sun.star.lang.XSingleServiceFactory",
        "com.sun.star.lang.XTypeProvider",
        "com.sun.star.loader.XImplementationLoader",
        "com.sun.star.reflection.XArrayTypeDescription",
        "com.sun.star.reflection.XCompoundTypeDescription",
        "com.sun.star.reflection.XEnumTypeDescription",
        "com.sun.star.reflection.XIdlClass",
        "com.sun.star.reflection.XIdlField2",
        "com.sun.star.reflection.XIdlReflection",
        "com.sun.star.reflection.XIndirectTypeDescription",
        "com.sun.star.reflection.XInterfaceAttributeTypeDescription",
        "com.sun.star.reflection.XInterfaceAttributeTypeDescription2",
        "com.sun.star.reflection.XInterfaceMemberTypeDescription",
        "com.sun.star.reflection.XInterfaceMethodTypeDescription",
        "com.sun.star.reflection.XInterfaceTypeDescription",
        "com.sun.star.reflection.XInterfaceTypeDescription2",
        "com.sun.star.reflection.XMethodParameter",
        "com.sun.star.reflection.XStructTypeDescription",
        "com.sun.star.reflection.XTypeDescription",
        "com.sun.star.reflection.XTypeDescriptionEnumerationAccess",
        "com.sun.star.reflection.XUnionTypeDescription",
        "com.sun.star.registry.XImplementationRegistration",
        "com.sun.star.registry.XRegistryKey",
        "com.sun.star.registry.XSimpleRegistry",
        "com.sun.star.security.RuntimePermission",
        "com.sun.star.security.XAccessController",
        "com.sun.star.uno.DeploymentException",
        "com.sun.star.uno.RuntimeException",
        "com.sun.star.uno.XAggregation",
        "com.sun.star.uno.XComponentContext",
        "com.sun.star.uno.XCurrentContext",
        "com.sun.star.uno.XUnloadingPreference",
        "com.sun.star.uno.XWeak",
        "com.sun.star.util.XMacroExpander" };
        // cf. cppuhelper/unotypes/Makefile UNOTYPES (plus missing dependencies)
    for (std::size_t i = 0; i < SAL_N_ELEMENTS(names); ++i) {
        if (name.equalsAscii(names[i])) {
            return true;
        }
    }
    return false;
}

class CppuType: private boost::noncopyable {
public:
    CppuType(
        OUString const & name, rtl::Reference< TypeManager > const & typeMgr);

    virtual ~CppuType() {}

    void dump(CppuOptions const & options);

    void dumpFile(
        OUString const & uri, OUString const & name, bool hpp,
        CppuOptions const & options);

    void dumpDependedTypes(
        codemaker::GeneratedTypeSet & generated, CppuOptions const & options);

    virtual void dumpHFile(
        FileStream & out, codemaker::cppumaker::Includes & includes)
    { dumpHFileContent(out, includes); }

    virtual void dumpHxxFile(FileStream& o, codemaker::cppumaker::Includes & includes) = 0;

    OUString dumpHeaderDefine(FileStream& o, OUString const & extension);

    void dumpGetCppuType(FileStream & out);

    virtual void dumpLightGetCppuType(FileStream & out);

    virtual void dumpNormalGetCppuType(FileStream &)
    { assert(false); } // this cannot happen

    virtual void dumpComprehensiveGetCppuType(FileStream &)
    { assert(false); } // this cannot happen

    void dumpType(
        FileStream & out, OUString const & name, bool isConst = false,
        bool isRef = false, bool native = false, bool cppuUnoType = false)
        const;

    OUString getTypeClass(OUString const & name, bool cStyle = false);

    void dumpCppuGetType(
        FileStream & out, OUString const & name, OUString const * ownName = 0);

    sal_uInt32 getInheritedMemberCount();

    void            inc(sal_Int32 num=4);
    void            dec(sal_Int32 num=4);
    OUString indent() const;
protected:
    virtual sal_uInt32 checkInheritedMemberCount() const
    { assert(false); return 0; } // this cannot happen

    bool passByReference(OUString const & name) const;

    OUString resolveOuterTypedefs(OUString const & name) const;

    OUString resolveAllTypedefs(OUString const & name) const;

    codemaker::cpp::IdentifierTranslationMode isGlobal() const;

    virtual void dumpDeclaration(FileStream &)
    { assert(false); } // this cannot happen

    virtual void dumpFiles(OUString const & uri, CppuOptions const & options);

    virtual void addLightGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual void addNormalGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual void addComprehensiveGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual bool isPolymorphic() const;

    virtual void dumpTemplateHead(FileStream &) const {}

    virtual void dumpTemplateParameters(FileStream &) const {}

    void dumpGetCppuTypePreamble(FileStream & out);

    void dumpGetCppuTypePostamble(FileStream & out);

    void addDefaultHIncludes(codemaker::cppumaker::Includes & includes) const;
    void addDefaultHxxIncludes(codemaker::cppumaker::Includes & includes) const;

    void dumpInitializer(
        FileStream & out, bool parameterized, OUString const & name) const;

    void dumpHFileContent(
        FileStream & out, codemaker::cppumaker::Includes & includes);

protected:
    sal_uInt32          m_inheritedMemberCount;

    bool                m_cppuTypeLeak;
    bool                m_cppuTypeDynamic;
    sal_Int32           m_indentLength;
    OUString name_;
    OUString id_;
    rtl::Reference< TypeManager > m_typeMgr;
    codemaker::cppumaker::Dependencies m_dependencies;

private:
    void addGetCppuTypeIncludes(codemaker::cppumaker::Includes & includes)
        const;
};

CppuType::CppuType(
    OUString const & name, rtl::Reference< TypeManager > const & typeMgr):
    m_inheritedMemberCount(0)
    , m_cppuTypeLeak(false)
    , m_cppuTypeDynamic(true)
    , m_indentLength(0)
    , name_(name)
    , id_(name_.copy(name_.lastIndexOf('.') + 1))
    , m_typeMgr(typeMgr)
    , m_dependencies(typeMgr, name_)
{}

void CppuType::addGetCppuTypeIncludes(codemaker::cppumaker::Includes & includes)
    const
{
    if (name_ == "com.sun.star.uno.XInterface"
        || name_ == "com.sun.star.uno.Exception")
    {
        includes.addType();
        includes.addCppuUnotypeHxx();
        includes.addSalTypesH();
        includes.addTypelibTypeclassH();
        includes.addTypelibTypedescriptionH();
    } else if (m_cppuTypeLeak) {
        addLightGetCppuTypeIncludes(includes);
    } else if (m_cppuTypeDynamic) {
        addNormalGetCppuTypeIncludes(includes);
    } else {
        addComprehensiveGetCppuTypeIncludes(includes);
    }
}

void CppuType::dumpFiles(OUString const & uri, CppuOptions const & options) {
    dumpFile(uri, name_, false, options);
    dumpFile(uri, name_, true, options);
}

void CppuType::addLightGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    //TODO: Determine what is really needed, instead of relying on
    // addDefaultHxxIncludes
    includes.addCppuUnotypeHxx();
}

void CppuType::addNormalGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    //TODO: Determine what is really needed, instead of relying on
    // addDefaultHxxIncludes
    includes.addCppuUnotypeHxx();
}

void CppuType::addComprehensiveGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    //TODO: Determine what is really needed, instead of relying on
    // addDefaultHxxIncludes
    includes.addCppuUnotypeHxx();
}

bool CppuType::isPolymorphic() const { return false; }

void CppuType::dumpGetCppuTypePreamble(FileStream & out) {
    if (isPolymorphic()) {
        out << "namespace cppu {\n\n";
        dumpTemplateHead(out);
        out << "class UnoType< ";
        dumpType(out, name_);
        dumpTemplateParameters(out);
        out << " > {\npublic:\n";
        inc();
        out << indent()
            << "static inline ::com::sun::star::uno::Type const & get() {\n";
    } else {
        if (codemaker::cppumaker::dumpNamespaceOpen(out, name_, false)) {
            out << "\n\n";
        }
        out << ("inline ::com::sun::star::uno::Type const &"
                " cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ");
        dumpType(out, name_, false, false, true);
        out << " const *) {\n";
    }
    inc();
}

void CppuType::dumpGetCppuTypePostamble(FileStream & out) {
    dec();
    if (isPolymorphic()) {
        out << indent() << "}\n\nprivate:\n"
            << indent() << "UnoType(UnoType &); // not defined\n"
            << indent() << "~UnoType(); // not defined\n"
            << indent()
            << "void operator =(UnoType); // not defined\n};\n\n}\n\n";
    } else {
        out << "}\n\n";
        if (codemaker::cppumaker::dumpNamespaceClose(out, name_, false)) {
            out << "\n\n";
        }
    }
    dumpTemplateHead(out);
    out << ("inline ::com::sun::star::uno::Type const & SAL_CALL"
            " getCppuType(SAL_UNUSED_PARAMETER ");
    dumpType(out, name_);
    dumpTemplateParameters(out);
    out << " const *) SAL_THROW(()) {\n";
    inc();
    out << indent() << "return ::cppu::UnoType< ";
    dumpType(out, name_);
    dumpTemplateParameters(out);
    out << " >::get();\n";
    dec();
    out << indent() << "}\n";
}

void CppuType::dump(CppuOptions const & options) {
    if (isBootstrapType(name_)) {
        m_cppuTypeDynamic = false;
    } else {
        // -CS was used as an undocumented option to generate static getCppuType
        // functions; since the introduction of cppu::UnoType this no longer is
        // meaningful (getCppuType is just a forward to cppu::UnoType::get now),
        // and -CS is handled the same way as -C now:
        if (options.isValid("-L"))
            m_cppuTypeLeak = true;
        if (options.isValid("-C") || options.isValid("-CS"))
            m_cppuTypeDynamic = false;
    }
    dumpFiles(
        options.isValid("-O") ? b2u(options.getOption("-O")) : "", options);
}

void CppuType::dumpFile(
    OUString const & uri, OUString const & name, bool hpp,
    CppuOptions const & options)
{
    OUString fileUri(
        b2u(createFileNameFromType(
                u2b(uri), u2b(name), hpp ? ".hpp" : ".hdl")));
    if (fileUri.isEmpty()) {
        throw CannotDumpException("empty target URI for entity " + name);
    }
    bool exists = fileExists(u2b(fileUri));
    if (exists && options.isValid("-G")) {
        return;
    }
    FileStream out;
    out.createTempFile(getTempDir(u2b(fileUri)));
    OUString tmpUri(b2u(out.getName()));
    if(!out.isValid()) {
        throw CannotDumpException("cannot open " + tmpUri + " for writing");
    }
    codemaker::cppumaker::Includes includes(m_typeMgr, m_dependencies, hpp);
    try {
        if (hpp) {
            addGetCppuTypeIncludes(includes);
            dumpHxxFile(out, includes);
        } else {
            dumpHFile(out, includes);
        }
    } catch (...) {
        out.close();
        // Remove existing type file if something goes wrong to ensure
        // consistency:
        if (fileExists(u2b(fileUri))) {
            removeTypeFile(u2b(fileUri));
        }
        removeTypeFile(u2b(tmpUri));
        throw;
    }
    out.close();
    makeValidTypeFile(
        u2b(fileUri), u2b(tmpUri), exists && options.isValid("-Gc"));
}

void CppuType::dumpDependedTypes(
    codemaker::GeneratedTypeSet & generated, CppuOptions const & options)
{
    codemaker::cppumaker::Dependencies::Map const & map
        = m_dependencies.getMap();
    for (codemaker::cppumaker::Dependencies::Map::const_iterator i(map.begin());
         i != map.end(); ++i)
    {
        produce(i->first, m_typeMgr, generated, options);
    }
}

OUString CppuType::dumpHeaderDefine(
    FileStream & out, OUString const & extension)
{
    OUString def(
        "INCLUDED_" + name_.replace('.', '_').toAsciiUpperCase() + "_"
        + extension);
    out << "#ifndef " << def << "\n#define " << def << "\n";
    return def;
}

void CppuType::addDefaultHIncludes(codemaker::cppumaker::Includes & includes)
    const
{
    //TODO: Only include what is really needed
    includes.addCppuMacrosHxx();
    if (m_typeMgr->getSort(name_)
        == codemaker::UnoType::SORT_INTERFACE_TYPE)
    {
        includes.addReference();
    }
}

void CppuType::addDefaultHxxIncludes(codemaker::cppumaker::Includes & includes)
    const
{
    //TODO: Only include what is really needed
    includes.addRtlInstanceHxx();
    includes.addOslMutexHxx();
    includes.addType();
    if (m_typeMgr->getSort(name_)
        == codemaker::UnoType::SORT_INTERFACE_TYPE)
    {
        includes.addReference();
    }
}

void CppuType::dumpInitializer(
    FileStream & out, bool parameterized, OUString const & name) const
{
    out << "(";
    if (!parameterized) {
        sal_Int32 k;
        std::vector< OString > args;
        OUString n(
            b2u(codemaker::UnoType::decompose(
                    u2b(resolveAllTypedefs(name)), &k, &args)));
        if (k == 0) {
            rtl::Reference< unoidl::Entity > ent;
            switch (m_typeMgr->getSort(n, &ent)) {
            case codemaker::UnoType::SORT_BOOLEAN:
                out << "false";
                break;
            case codemaker::UnoType::SORT_BYTE:
            case codemaker::UnoType::SORT_SHORT:
            case codemaker::UnoType::SORT_UNSIGNED_SHORT:
            case codemaker::UnoType::SORT_LONG:
            case codemaker::UnoType::SORT_UNSIGNED_LONG:
            case codemaker::UnoType::SORT_HYPER:
            case codemaker::UnoType::SORT_UNSIGNED_HYPER:
            case codemaker::UnoType::SORT_FLOAT:
            case codemaker::UnoType::SORT_DOUBLE:
            case codemaker::UnoType::SORT_CHAR:
                out << "0";
                break;
            case codemaker::UnoType::SORT_ENUM_TYPE:
                out << codemaker::cpp::scopedCppName(u2b(n)) << "_"
                    << (dynamic_cast< unoidl::EnumTypeEntity * >(ent.get())->
                        getMembers()[0].name);
                break;
            case codemaker::UnoType::SORT_STRING:
            case codemaker::UnoType::SORT_TYPE:
            case codemaker::UnoType::SORT_ANY:
            case codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE:
            case codemaker::UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
            case codemaker::UnoType::SORT_INTERFACE_TYPE:
                break;
            default:
                throw CannotDumpException(
                    "unexpected entity \"" + name
                    + "\" in call to CppuType::dumpInitializer");
            }
        }
    }
    out << ")";
}

void CppuType::dumpHFileContent(
    FileStream & out, codemaker::cppumaker::Includes & includes)
{
    addDefaultHIncludes(includes);
    dumpHeaderDefine(out, "HDL");
    out << "\n";
    includes.dump(out, 0);
    out << ("\nnamespace com { namespace sun { namespace star { namespace uno"
            " { class Type; } } } }\n\n");
    if (codemaker::cppumaker::dumpNamespaceOpen(out, name_, false)) {
        out << "\n";
    }
    dumpDeclaration(out);
    if (!(name_ == "com.sun.star.uno.XInterface"
          || name_ == "com.sun.star.uno.Exception"
          || isPolymorphic()))
    {
        out << "\n" << indent()
            << ("inline ::com::sun::star::uno::Type const &"
                " cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ");
        dumpType(out, name_, false, false, true);
        out << " const *);\n";
    }
    if (codemaker::cppumaker::dumpNamespaceClose(out, name_, false)) {
        out << "\n";
    }
    out << "\n";
    dumpTemplateHead(out);
    out << "inline ::com::sun::star::uno::Type const & SAL_CALL getCppuType(";
    dumpType(out, name_, true);
    dumpTemplateParameters(out);
    out << " *) SAL_THROW(());\n\n#endif\n";
}

void CppuType::dumpGetCppuType(FileStream & out) {
    if (name_ == "com.sun.star.uno.XInterface") {
        out << indent()
            << ("inline ::com::sun::star::uno::Type const & SAL_CALL"
                " getCppuType(SAL_UNUSED_PARAMETER ");
        dumpType(out, name_, true);
        out << " *) SAL_THROW(()) {\n";
        inc();
        out << indent()
            << ("return ::cppu::UnoType< ::com::sun::star::uno::XInterface"
                " >::get();\n");
        dec();
        out << indent() << "}\n";
    } else if (name_ == "com.sun.star.uno.Exception") {
        out << indent()
            << ("inline ::com::sun::star::uno::Type const & SAL_CALL"
                " getCppuType(SAL_UNUSED_PARAMETER ");
        dumpType(out, name_, true);
        out << " *) SAL_THROW(()) {\n";
        inc();
        out << indent()
            << ("return ::cppu::UnoType< ::com::sun::star::uno::Exception"
                " >::get();\n");
        dec();
        out << indent() << "}\n";
    } else if (m_cppuTypeLeak) {
        dumpLightGetCppuType(out);
    } else if (m_cppuTypeDynamic) {
        dumpNormalGetCppuType(out);
    } else {
        dumpComprehensiveGetCppuType(out);
    }
}

void CppuType::dumpLightGetCppuType(FileStream & out) {
    dumpGetCppuTypePreamble(out);
    out << indent()
        << "static typelib_TypeDescriptionReference * the_type = 0;\n"
        << indent() << "if ( !the_type )\n" << indent() << "{\n";
    inc();
    out << indent() << "typelib_static_type_init( &the_type, "
        << getTypeClass(name_, true) << ", \"" << name_ << "\" );\n";
    dec();
    out << indent() << "}\n" << indent()
        << ("return * reinterpret_cast< ::com::sun::star::uno::Type * >("
            " &the_type );\n");
    dumpGetCppuTypePostamble(out);
}

codemaker::cpp::IdentifierTranslationMode CppuType::isGlobal() const {
    return name_.indexOf('.') == -1
        ? codemaker::cpp::ITM_GLOBAL : codemaker::cpp::ITM_NONGLOBAL;
}

sal_uInt32 CppuType::getInheritedMemberCount()
{
    if (m_inheritedMemberCount == 0)
    {
        m_inheritedMemberCount = checkInheritedMemberCount();
    }

    return m_inheritedMemberCount;
}

OUString CppuType::getTypeClass(OUString const & name, bool cStyle) {
    rtl::Reference< unoidl::Entity > ent;
    switch (m_typeMgr->getSort(name, &ent)) {
    case codemaker::UnoType::SORT_VOID:
        return cStyle
            ? OUString("typelib_TypeClass_VOID")
            : OUString("::com::sun::star::uno::TypeClass_VOID");
    case codemaker::UnoType::SORT_BOOLEAN:
        return cStyle
            ? OUString("typelib_TypeClass_BOOLEAN")
            : OUString("::com::sun::star::uno::TypeClass_BOOLEAN");
    case codemaker::UnoType::SORT_BYTE:
        return cStyle
            ? OUString("typelib_TypeClass_BYTE")
            : OUString("::com::sun::star::uno::TypeClass_BYTE");
    case codemaker::UnoType::SORT_SHORT:
        return cStyle
            ? OUString("typelib_TypeClass_SHORT")
            : OUString("::com::sun::star::uno::TypeClass_SHORT");
    case codemaker::UnoType::SORT_UNSIGNED_SHORT:
        return cStyle
            ? OUString("typelib_TypeClass_UNSIGNED_SHORT")
            : OUString("::com::sun::star::uno::TypeClass_UNSIGNED_SHORT");
    case codemaker::UnoType::SORT_LONG:
        return cStyle
            ? OUString("typelib_TypeClass_LONG")
            : OUString("::com::sun::star::uno::TypeClass_LONG");
    case codemaker::UnoType::SORT_UNSIGNED_LONG:
        return cStyle
            ? OUString("typelib_TypeClass_UNSIGNED_LONG")
            : OUString("::com::sun::star::uno::TypeClass_UNSIGNED_LONG");
    case codemaker::UnoType::SORT_HYPER:
        return cStyle
            ? OUString("typelib_TypeClass_HYPER")
            : OUString("::com::sun::star::uno::TypeClass_HYPER");
    case codemaker::UnoType::SORT_UNSIGNED_HYPER:
        return cStyle
            ? OUString("typelib_TypeClass_UNSIGNED_HYPER")
            : OUString("::com::sun::star::uno::TypeClass_UNSIGNED_HYPER");
    case codemaker::UnoType::SORT_FLOAT:
        return cStyle
            ? OUString("typelib_TypeClass_FLOAT")
            : OUString("::com::sun::star::uno::TypeClass_FLOAT");
    case codemaker::UnoType::SORT_DOUBLE:
        return cStyle
            ? OUString("typelib_TypeClass_DOUBLE")
            : OUString("::com::sun::star::uno::TypeClass_DOUBLE");
    case codemaker::UnoType::SORT_CHAR:
        return cStyle
            ? OUString("typelib_TypeClass_CHAR")
            : OUString("::com::sun::star::uno::TypeClass_CHAR");
    case codemaker::UnoType::SORT_STRING:
        return cStyle
            ? OUString("typelib_TypeClass_STRING")
            : OUString("::com::sun::star::uno::TypeClass_STRING");
    case codemaker::UnoType::SORT_TYPE:
        return cStyle
            ? OUString("typelib_TypeClass_TYPE")
            : OUString("::com::sun::star::uno::TypeClass_TYPE");
    case codemaker::UnoType::SORT_ANY:
        return cStyle
            ? OUString("typelib_TypeClass_ANY")
            : OUString("::com::sun::star::uno::TypeClass_ANY");
    case codemaker::UnoType::SORT_SEQUENCE_TYPE:
        return cStyle
            ? OUString("typelib_TypeClass_SEQUENCE")
            : OUString("::com::sun::star::uno::TypeClass_SEQUENCE");
    case codemaker::UnoType::SORT_ENUM_TYPE:
        return cStyle
            ? OUString("typelib_TypeClass_ENUM")
            : OUString("::com::sun::star::uno::TypeClass_ENUM");
    case codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE:
    case codemaker::UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
    case codemaker::UnoType::SORT_INSTANTIATED_POLYMORPHIC_STRUCT_TYPE:
        return cStyle
            ? OUString("typelib_TypeClass_STRUCT")
            : OUString("::com::sun::star::uno::TypeClass_STRUCT");
    case codemaker::UnoType::SORT_EXCEPTION_TYPE:
        return cStyle
            ? OUString("typelib_TypeClass_EXCEPTION")
            : OUString("::com::sun::star::uno::TypeClass_EXCEPTION");
    case codemaker::UnoType::SORT_INTERFACE_TYPE:
        return cStyle
            ? OUString("typelib_TypeClass_INTERFACE")
            : OUString("::com::sun::star::uno::TypeClass_INTERFACE");
    case codemaker::UnoType::SORT_TYPEDEF:
        return getTypeClass(
            dynamic_cast< unoidl::TypedefEntity * >(ent.get())->getType(),
            cStyle);
    default:
        for (;;) { std::abort(); }
    }
}

void CppuType::dumpType(
    FileStream & out, OUString const & name, bool isConst, bool isRef,
    bool native, bool cppuUnoType) const
{
    sal_Int32 k;
    std::vector< OString > args;
    OUString n(
        b2u(codemaker::UnoType::decompose(
                u2b(resolveAllTypedefs(name)), &k, &args)));
    if (isConst) {
        out << "const ";
    }
    for (sal_Int32 i = 0; i != k; ++i) {
        out << (cppuUnoType
                ? "::cppu::UnoSequenceType" : "::com::sun::star::uno::Sequence")
            << "< ";
    }
    switch (m_typeMgr->getSort(n)) {
    case codemaker::UnoType::SORT_VOID:
        out << "void";
        break;
    case codemaker::UnoType::SORT_BOOLEAN:
        out << "::sal_Bool";
        break;
    case codemaker::UnoType::SORT_BYTE:
        out << "::sal_Int8";
        break;
    case codemaker::UnoType::SORT_SHORT:
        out << "::sal_Int16";
        break;
    case codemaker::UnoType::SORT_UNSIGNED_SHORT:
        out << (cppuUnoType ? "::cppu::UnoUnsignedShortType" : "::sal_uInt16");
        break;
    case codemaker::UnoType::SORT_LONG:
        out << "::sal_Int32";
        break;
    case codemaker::UnoType::SORT_UNSIGNED_LONG:
        out << "::sal_uInt32";
        break;
    case codemaker::UnoType::SORT_HYPER:
        out << "::sal_Int64";
        break;
    case codemaker::UnoType::SORT_UNSIGNED_HYPER:
        out << "::sal_uInt64";
        break;
    case codemaker::UnoType::SORT_FLOAT:
        out << "float";
        break;
    case codemaker::UnoType::SORT_DOUBLE:
        out << "double";
        break;
    case codemaker::UnoType::SORT_CHAR:
        out << (cppuUnoType ? "::cppu::UnoCharType" : "::sal_Unicode");
        break;
    case codemaker::UnoType::SORT_STRING:
        out << "::rtl::OUString";
        break;
    case codemaker::UnoType::SORT_TYPE:
        out << "::com::sun::star::uno::Type";
        break;
    case codemaker::UnoType::SORT_ANY:
        out << "::com::sun::star::uno::Any";
        break;
    case codemaker::UnoType::SORT_ENUM_TYPE:
    case codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE:
    case codemaker::UnoType::SORT_EXCEPTION_TYPE:
        out << codemaker::cpp::scopedCppName(u2b(n));
        break;
    case codemaker::UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
        out << codemaker::cpp::scopedCppName(u2b(n));
        if (!args.empty()) {
            out << "< ";
            for (std::vector< OString >::iterator i(args.begin());
                 i != args.end(); ++i)
            {
                if (i != args.begin()) {
                    out << ", ";
                }
                dumpType(out, b2u(*i));
            }
            out << " >";
        }
        break;
    case codemaker::UnoType::SORT_INTERFACE_TYPE:
        if (!native) {
            out << "::com::sun::star::uno::Reference< ";
        }
        out << codemaker::cpp::scopedCppName(u2b(n));
        if (!native) {
            out << " >";
        }
        break;
    default:
        throw CannotDumpException(
            "unexpected entity \"" + name + "\" in call to CppuType::dumpType");
    }
    for (sal_Int32 i = 0; i != k; ++i) {
        out << " >";
    }
    if (isRef) {
        out << "&";
    }
}

void CppuType::dumpCppuGetType(
    FileStream & out, OUString const & name, OUString const * ownName)
{
    //TODO: What are these calls good for?
    OUString nucleus;
    sal_Int32 rank;
    codemaker::UnoType::Sort sort = m_typeMgr->decompose(
        name, true, &nucleus, &rank, 0, 0);
    switch (rank == 0 ? sort : codemaker::UnoType::SORT_SEQUENCE_TYPE) {
    case codemaker::UnoType::SORT_VOID:
    case codemaker::UnoType::SORT_BOOLEAN:
    case codemaker::UnoType::SORT_BYTE:
    case codemaker::UnoType::SORT_SHORT:
    case codemaker::UnoType::SORT_UNSIGNED_SHORT:
    case codemaker::UnoType::SORT_LONG:
    case codemaker::UnoType::SORT_UNSIGNED_LONG:
    case codemaker::UnoType::SORT_HYPER:
    case codemaker::UnoType::SORT_UNSIGNED_HYPER:
    case codemaker::UnoType::SORT_FLOAT:
    case codemaker::UnoType::SORT_DOUBLE:
    case codemaker::UnoType::SORT_CHAR:
    case codemaker::UnoType::SORT_STRING:
    case codemaker::UnoType::SORT_TYPE:
    case codemaker::UnoType::SORT_ANY:
        break;
    case codemaker::UnoType::SORT_SEQUENCE_TYPE:
    case codemaker::UnoType::SORT_ENUM_TYPE:
    case codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE:
    case codemaker::UnoType::SORT_INSTANTIATED_POLYMORPHIC_STRUCT_TYPE:
    case codemaker::UnoType::SORT_EXCEPTION_TYPE:
    case codemaker::UnoType::SORT_INTERFACE_TYPE:
        // Take care of recursion like struct S { sequence<S> x; }:
        if (ownName == 0 || nucleus != *ownName) {
            out << indent() << "::cppu::UnoType< ";
            dumpType(out, name, false, false, false, true);
            out << " >::get();\n";
        }
        break;
    case codemaker::UnoType::SORT_TYPEDEF:
        assert(false); // this cannot happen
        // fall through
    default:
        throw CannotDumpException(
            "unexpected entity \"" + name
            + "\" in call to CppuType::dumpCppuGetType");
    }
}

bool CppuType::passByReference(OUString const & name) const {
    switch (m_typeMgr->getSort(resolveOuterTypedefs(name))) {
    case codemaker::UnoType::SORT_BOOLEAN:
    case codemaker::UnoType::SORT_BYTE:
    case codemaker::UnoType::SORT_SHORT:
    case codemaker::UnoType::SORT_UNSIGNED_SHORT:
    case codemaker::UnoType::SORT_LONG:
    case codemaker::UnoType::SORT_UNSIGNED_LONG:
    case codemaker::UnoType::SORT_HYPER:
    case codemaker::UnoType::SORT_UNSIGNED_HYPER:
    case codemaker::UnoType::SORT_FLOAT:
    case codemaker::UnoType::SORT_DOUBLE:
    case codemaker::UnoType::SORT_CHAR:
    case codemaker::UnoType::SORT_ENUM_TYPE:
        return false;
    case codemaker::UnoType::SORT_STRING:
    case codemaker::UnoType::SORT_TYPE:
    case codemaker::UnoType::SORT_ANY:
    case codemaker::UnoType::SORT_SEQUENCE_TYPE:
    case codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE:
    case codemaker::UnoType::SORT_INSTANTIATED_POLYMORPHIC_STRUCT_TYPE:
    case codemaker::UnoType::SORT_INTERFACE_TYPE:
        return true;
    default:
        throw CannotDumpException(
            "unexpected entity \"" + name
            + "\" in call to CppuType::passByReference");
    }
}

OUString CppuType::resolveOuterTypedefs(OUString const & name) const {
    for (OUString n(name);;) {
        rtl::Reference< unoidl::Entity > ent;
        if (m_typeMgr->getSort(n, &ent) != codemaker::UnoType::SORT_TYPEDEF) {
            return n;
        }
        n = dynamic_cast< unoidl::TypedefEntity * >(ent.get())->getType();
    }
}

OUString CppuType::resolveAllTypedefs(OUString const & name) const {
    sal_Int32 k1;
    OUString n(b2u(codemaker::UnoType::decompose(u2b(name), &k1)));
    for (;;) {
        rtl::Reference< unoidl::Entity > ent;
        if (m_typeMgr->getSort(n, &ent) != codemaker::UnoType::SORT_TYPEDEF) {
            break;
        }
        sal_Int32 k2;
        n = b2u(
            codemaker::UnoType::decompose(
                u2b(dynamic_cast< unoidl::TypedefEntity * >(ent.get())->
                    getType()),
                &k2));
        k1 += k2; //TODO: overflow
    }
    OUStringBuffer b;
    for (sal_Int32 i = 0; i != k1; ++i) {
        b.append("[]");
    }
    b.append(n);
    return b.makeStringAndClear();
}

void CppuType::inc(sal_Int32 num)
{
    m_indentLength += num;
}

void CppuType::dec(sal_Int32 num)
{
    m_indentLength = std::max< sal_Int32 >(m_indentLength - num, 0);
}

OUString CppuType::indent() const {
    OUStringBuffer buf(m_indentLength);
    for (sal_Int32 i = 0; i != m_indentLength; ++i) {
        buf.append(' ');
    }
    return buf.makeStringAndClear();
}

bool isDeprecated(std::vector< OUString > const & annotations) {
    for (std::vector< OUString >::const_iterator i(annotations.begin());
         i != annotations.end(); ++i)
    {
        if (*i == "deprecated") {
            return true;
        }
    }
    return false;
}

void dumpDeprecation(FileStream & out, bool deprecated) {
    if (deprecated) {
        out << "SAL_DEPRECATED_INTERNAL(\"marked @deprecated in UNOIDL\") ";
    }
}

class BaseOffset: private boost::noncopyable {
public:
    BaseOffset(
        rtl::Reference< TypeManager > const & manager,
        rtl::Reference< unoidl::InterfaceTypeEntity > const & entity):
        manager_(manager), offset_(0) { calculateBases(entity); }

    sal_Int32 get() const { return offset_; }

private:
    void calculateBases(
        rtl::Reference< unoidl::InterfaceTypeEntity > const & entity);

    rtl::Reference< TypeManager > manager_;
    std::set< OUString > set_;
    sal_Int32 offset_;
};

void BaseOffset::calculateBases(
    rtl::Reference< unoidl::InterfaceTypeEntity > const & entity)
{
    assert(entity.is());
    for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
             entity->getDirectMandatoryBases().begin());
         i != entity->getDirectMandatoryBases().end(); ++i)
    {
        if (set_.insert(i->name).second) {
            rtl::Reference< unoidl::Entity > ent;
            codemaker::UnoType::Sort sort = manager_->getSort(i->name, &ent);
            if (sort != codemaker::UnoType::SORT_INTERFACE_TYPE) {
                throw CannotDumpException(
                    "interface type base " + i->name
                    + " is not an interface type");
            }
            rtl::Reference< unoidl::InterfaceTypeEntity > ent2(
                dynamic_cast< unoidl::InterfaceTypeEntity * >(ent.get()));
            assert(ent2.is());
            calculateBases(ent2);
            offset_ += ent2->getDirectAttributes().size()
                + ent2->getDirectMethods().size(); //TODO: overflow
        }
    }
}

class InterfaceType: public CppuType {
public:
    InterfaceType(
        rtl::Reference< unoidl::InterfaceTypeEntity > const & entity,
        OUString const & name, rtl::Reference< TypeManager > const & typeMgr);

    virtual void dumpDeclaration(FileStream& o);
    void dumpHxxFile(FileStream& o, codemaker::cppumaker::Includes & includes);

    void        dumpAttributes(FileStream& o);
    void        dumpMethods(FileStream& o);
    void        dumpNormalGetCppuType(FileStream& o);
    void        dumpComprehensiveGetCppuType(FileStream& o);
    void        dumpCppuAttributeRefs(FileStream& o, sal_uInt32& index);
    void        dumpCppuMethodRefs(FileStream& o, sal_uInt32& index);
    void        dumpCppuAttributes(FileStream& o, sal_uInt32& index);
    void        dumpCppuMethods(FileStream& o, sal_uInt32& index);
    void dumpAttributesCppuDecl(FileStream & out, std::set< OUString > * seen);
    void dumpMethodsCppuDecl(FileStream & out, std::set< OUString > * seen);

protected:
    virtual void addComprehensiveGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual sal_uInt32 checkInheritedMemberCount() const
    { return BaseOffset(m_typeMgr, entity_).get(); }

    sal_uInt32  m_inheritedMemberCount;
    bool        m_isDeprecated;

private:
    void dumpExceptionSpecification(
        FileStream & out, std::vector< OUString > const & exceptions,
        bool runtimeException);

    void dumpExceptionTypeName(
        FileStream & out, OUString const & prefix, sal_uInt32 index,
        OUString const & name);

    sal_Int32 dumpExceptionTypeNames(
        FileStream & out, OUString const & prefix,
        std::vector< OUString > const & exceptions, bool runtimeException);

    rtl::Reference< unoidl::InterfaceTypeEntity > entity_;
};

InterfaceType::InterfaceType(
    rtl::Reference< unoidl::InterfaceTypeEntity > const & entity,
    OUString const & name, rtl::Reference< TypeManager > const & typeMgr):
    CppuType(name, typeMgr), entity_(entity)
{
    assert(entity.is());
    m_inheritedMemberCount = 0;
    m_isDeprecated = isDeprecated(entity->getAnnotations());
}

void InterfaceType::dumpDeclaration(FileStream & out) {
    out << "\nclass SAL_NO_VTABLE " << id_;
    for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
             entity_->getDirectMandatoryBases().begin());
         i != entity_->getDirectMandatoryBases().end(); ++i)
    {
        out << (i == entity_->getDirectMandatoryBases().begin() ? " :" : ",")
            << " public " << codemaker::cpp::scopedCppName(u2b(i->name));
    }
    out << "\n{\npublic:\n";
    inc();
    dumpAttributes(out);
    dumpMethods(out);
    out << "\n" << indent()
        << ("static inline ::com::sun::star::uno::Type const & SAL_CALL"
            " static_type(void * = 0);\n\n");
    dec();
    out << "protected:\n";
    inc();
    out << indent() << "~" << id_
        << ("() throw () {} // avoid warnings about virtual members and"
            " non-virtual dtor\n");
    dec();
    out << "};\n\n";
}

void InterfaceType::dumpHxxFile(
    FileStream & out, codemaker::cppumaker::Includes & includes)
{
    OUString headerDefine(dumpHeaderDefine(out, "HPP"));
    out << "\n";
    addDefaultHxxIncludes(includes);
    includes.dump(out, &name_);
    out << "\n";
    dumpGetCppuType(out);
    out << "\n::com::sun::star::uno::Type const & "
        << codemaker::cpp::scopedCppName(u2b(name_))
        << "::static_type(SAL_UNUSED_PARAMETER void *) {\n";
    inc();
    out << indent() << "return ::getCppuType(static_cast< ";
    dumpType(out, name_);
    out << " * >(0));\n";
    dec();
    out << "}\n\n#endif // "<< headerDefine << "\n";
}

void InterfaceType::dumpAttributes(FileStream & out) {
    if (!entity_->getDirectAttributes().empty()) {
        out << "\n" << indent() << "// Attributes\n";
    }
    for (std::vector< unoidl::InterfaceTypeEntity::Attribute >::const_iterator
             i(entity_->getDirectAttributes().begin());
         i != entity_->getDirectAttributes().end(); ++i)
    {
        bool depr = m_isDeprecated || isDeprecated(i->annotations);
        out << indent();
        dumpDeprecation(out, depr);
        out << "virtual ";
        dumpType(out, i->type);
        out << " SAL_CALL get" << i->name << "()";
        dumpExceptionSpecification(out, i->getExceptions, true);
        out << " = 0;\n";
        if (!i->readOnly) {
            bool byRef = passByReference(i->type);
            out << indent();
            dumpDeprecation(out, depr);
            out << "virtual void SAL_CALL set" << i->name << "( ";
            dumpType(out, i->type, byRef, byRef);
            out << " _" << i->name.toAsciiLowerCase() << " )";
            dumpExceptionSpecification(out, i->setExceptions, true);
            out << " = 0;\n";
        }
    }
}

void InterfaceType::dumpMethods(FileStream & out) {
    if (!entity_->getDirectMethods().empty()) {
        out << "\n" << indent() << "// Methods\n";
    }
    for (std::vector< unoidl::InterfaceTypeEntity::Method >::const_iterator i(
             entity_->getDirectMethods().begin());
         i != entity_->getDirectMethods().end(); ++i)
    {
        out << indent();
        dumpDeprecation(out, m_isDeprecated || isDeprecated(i->annotations));
        out << "virtual ";
        dumpType(out, i->returnType);
        out << " SAL_CALL " << i->name << "(";
        if (i->parameters.empty()) {
            out << ")";
        } else {
            out << " ";
            for (std::vector< unoidl::InterfaceTypeEntity::Method::Parameter >::
                     const_iterator j(i->parameters.begin());
                 j != i->parameters.end();)
            {
                bool isConst;
                bool isRef;
                if (j->direction
                    == (unoidl::InterfaceTypeEntity::Method::Parameter::
                        DIRECTION_IN))
                {
                    isConst = passByReference(j->type);
                    isRef = isConst;
                } else {
                    isConst = false;
                    isRef = true;
                }
                dumpType(out, j->type, isConst, isRef);
                out << " " << j->name;
                ++j;
                if (j != i->parameters.end()) {
                    out << ", ";
                }
            }
            out << " )";
        }
        dumpExceptionSpecification(
            out, i->exceptions, i->name != "acquire" && i->name != "release");
        out << " = 0;\n";
    }
}

void InterfaceType::dumpNormalGetCppuType(FileStream & out) {
    dumpGetCppuTypePreamble(out);
    out << indent()
        << "static typelib_TypeDescriptionReference * the_type = 0;\n"
        << indent() << "if ( !the_type )\n" << indent() << "{\n";
    inc();
    std::vector< unoidl::AnnotatedReference >::size_type bases(
        entity_->getDirectMandatoryBases().size());
    if (bases == 1
        && (entity_->getDirectMandatoryBases()[0].name
            == "com.sun.star.uno.XInterface"))
    {
        bases = 0;
    }
    if (bases != 0) {
        out << indent() << "typelib_TypeDescriptionReference * aSuperTypes["
            << entity_->getDirectMandatoryBases().size() << "];\n";
        std::vector< unoidl::AnnotatedReference >::size_type n = 0;
        for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
                 entity_->getDirectMandatoryBases().begin());
             i != entity_->getDirectMandatoryBases().end(); ++i)
        {
            out << indent() << "aSuperTypes[" << n++ << "] = ::cppu::UnoType< ";
            dumpType(out, i->name, true, false, false, true);
            out << " >::get().getTypeLibType();\n";
        }
    }
    out << indent() << "typelib_static_mi_interface_type_init( &the_type, \""
        << name_ << "\", " << bases << ", "
        << (bases == 0 ? "0" : "aSuperTypes") << " );\n";
    dec();
    out << indent() << "}\n" << indent()
        << ("return * reinterpret_cast< ::com::sun::star::uno::Type * >("
            " &the_type );\n");
    dumpGetCppuTypePostamble(out);
}

void InterfaceType::dumpComprehensiveGetCppuType(FileStream & out) {
    codemaker::cppumaker::dumpNamespaceOpen(out, name_, false);
    OUString staticTypeClass("the" + id_ + "Type");
    out << " namespace detail {\n\n" << indent() << "struct " << staticTypeClass
        << " : public rtl::StaticWithInit< ::com::sun::star::uno::Type *, "
        << staticTypeClass << " >\n" << indent() << "{\n";
    inc();
    out << indent() << "::com::sun::star::uno::Type * operator()() const\n"
        << indent() << "{\n";
    inc();
    out << indent() << "::rtl::OUString sTypeName( \"" << name_ << "\" );\n\n"
        << indent() << "// Start inline typedescription generation\n"
        << indent() << "typelib_InterfaceTypeDescription * pTD = 0;\n\n";
    out << indent() << "typelib_TypeDescriptionReference * aSuperTypes["
        << entity_->getDirectMandatoryBases().size() << "];\n";
    std::vector< unoidl::AnnotatedReference >::size_type n = 0;
    for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
             entity_->getDirectMandatoryBases().begin());
         i != entity_->getDirectMandatoryBases().end(); ++i)
    {
        out << indent() << "aSuperTypes[" << n++ << "] = ::cppu::UnoType< ";
        dumpType(out, i->name, false, false, false, true);
        out << " >::get().getTypeLibType();\n";
    }
    std::size_t count = entity_->getDirectAttributes().size()
        + entity_->getDirectMethods().size(); //TODO: overflow
    if (count != 0) {
        out << indent() << "typelib_TypeDescriptionReference * pMembers["
            << count << "] = { ";
        for (std::size_t i = 0; i != count; ++i) {
            out << "0";
            if (i + 1 != count) {
                out << ",";
            }
        }
        out << " };\n";
        sal_uInt32 index = 0;
        dumpCppuAttributeRefs(out, index);
        dumpCppuMethodRefs(out, index);
    }
    out << "\n" << indent() << "typelib_typedescription_newMIInterface(\n";
    inc();
    out << indent() << "&pTD,\n" << indent()
        << "sTypeName.pData, 0, 0, 0, 0, 0,\n" << indent()
        << entity_->getDirectMandatoryBases().size() << ", aSuperTypes,\n"
        << indent() << count << ",\n" << indent()
        << (count == 0 ? "0" : "pMembers") << " );\n\n";
    dec();
    out << indent()
        << ("typelib_typedescription_register( (typelib_TypeDescription**)&pTD"
          " );\n");
    for (std::size_t i = 0; i != count; ++i) {
        out << indent() << "typelib_typedescriptionreference_release( pMembers["
            << i << "] );\n";
    }
    out << indent()
        << ("typelib_typedescription_release( (typelib_TypeDescription*)pTD"
            " );\n\n")
        << indent() << "return new ::com::sun::star::uno::Type( "
        << getTypeClass(name_) << ", sTypeName ); // leaked\n";
    dec();
    out << indent() << "}\n";
    dec();
    out << indent() << "};\n\n";
    codemaker::cppumaker::dumpNamespaceClose(out, name_, false);
    out << " }\n\n";
    dumpGetCppuTypePreamble(out);
    out << indent() << "const ::com::sun::star::uno::Type &rRet = *detail::"
        << staticTypeClass << "::get();\n" << indent()
        << "// End inline typedescription generation\n" << indent()
        << "static bool bInitStarted = false;\n" << indent()
        << "if (!bInitStarted)\n" << indent() << "{\n";
    inc();
    out << indent()
        << "::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );\n"
        << indent() << "if (!bInitStarted)\n" << indent() << "{\n";
    inc();
    out << indent() << "OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();\n"
        << indent() << "bInitStarted = true;\n";
    std::set< OUString > seen;
    // Type for RuntimeException is always needed:
    seen.insert("com.sun.star.uno.RuntimeException");
    dumpCppuGetType(out, "com.sun.star.uno.RuntimeException");
    dumpAttributesCppuDecl(out, &seen);
    dumpMethodsCppuDecl(out, &seen);
    if (count != 0) {
        sal_uInt32 index = getInheritedMemberCount();
        dumpCppuAttributes(out, index);
        dumpCppuMethods(out, index);
    }
    dec();
    out << indent() << "}\n";
    dec();
    out << indent() << "}\n" << indent() << "else\n" << indent() << "{\n";
    inc();
    out << indent() << "OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();\n";
    dec();
    out << indent() << "}\n" << indent() << "return rRet;\n";
    dumpGetCppuTypePostamble(out);
}

void InterfaceType::dumpCppuAttributeRefs(FileStream & out, sal_uInt32 & index)
{
    std::vector< unoidl::InterfaceTypeEntity::Attribute >::size_type n = 0;
    for (std::vector< unoidl::InterfaceTypeEntity::Attribute >::const_iterator
             i(entity_->getDirectAttributes().begin());
         i != entity_->getDirectAttributes().end(); ++i)
    {
        out << indent() << "::rtl::OUString sAttributeName" << n << "( \""
            << name_ << "::" << i->name << "\" );\n" << indent()
            << "typelib_typedescriptionreference_new( &pMembers[" << index++
            << "],\n";
        inc(38);
        out << indent()
            << ("(typelib_TypeClass)"
                "::com::sun::star::uno::TypeClass_INTERFACE_ATTRIBUTE,\n")
            << indent() << "sAttributeName" << n << ".pData );\n";
        dec(38);
        ++n;
    }
}

void InterfaceType::dumpCppuMethodRefs(FileStream & out, sal_uInt32 & index) {
    std::vector< unoidl::InterfaceTypeEntity::Method >::size_type n = 0;
    for (std::vector< unoidl::InterfaceTypeEntity::Method >::const_iterator i(
             entity_->getDirectMethods().begin());
         i != entity_->getDirectMethods().end(); ++i)
    {
        out << indent() << "::rtl::OUString sMethodName" << n << "( \"" << name_
            << "::" << i->name << "\" );\n" << indent()
            << "typelib_typedescriptionreference_new( &pMembers[" << index++
            << "],\n";
        inc(38);
        out << indent()
            << ("(typelib_TypeClass)"
                "::com::sun::star::uno::TypeClass_INTERFACE_METHOD,\n")
            << indent() << "sMethodName" << n << ".pData );\n";
        dec(38);
        ++n;
    }
}

void InterfaceType::addComprehensiveGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    // The comprehensive getCppuType method always includes a line
    // "getCppuType( (const ::com::sun::star::uno::RuntimeException*)0 );":
    includes.addCppuUnotypeHxx();
    includes.add("com.sun.star.uno.RuntimeException");
}

void InterfaceType::dumpCppuAttributes(FileStream & out, sal_uInt32 & index) {
    if (!entity_->getDirectAttributes().empty()) {
        out << "\n" << indent()
            << "typelib_InterfaceAttributeTypeDescription * pAttribute = 0;\n";
        std::vector< unoidl::InterfaceTypeEntity::Attribute >::size_type n = 0;
        for (std::vector< unoidl::InterfaceTypeEntity::Attribute >::
                 const_iterator i(entity_->getDirectAttributes().begin());
             i != entity_->getDirectAttributes().end(); ++i)
        {
            OUString type(resolveAllTypedefs(i->type));
            out << indent() << "{\n";
            inc();
            out << indent() << "::rtl::OUString sAttributeType" << n << "( \""
                << type << "\" );\n" << indent()
                << "::rtl::OUString sAttributeName" << n << "( \"" << name_
                << "::" << i->name << "\" );\n";
            sal_Int32 getExcn = dumpExceptionTypeNames(
                out, "get", i->getExceptions, false);
            sal_Int32 setExcn = dumpExceptionTypeNames(
                out, "set", i->setExceptions, false);
            out << indent()
                << ("typelib_typedescription_newExtendedInterfaceAttribute("
                    " &pAttribute,\n");
            inc();
            out << indent() << index++ << ", sAttributeName" << n
                << ".pData,\n" << indent() << "(typelib_TypeClass)"
                << getTypeClass(type) << ", sAttributeType" << n << ".pData,\n"
                << indent() << "sal_" << (i->readOnly ? "True" : "False")
                << ", " << getExcn << ", "
                << (getExcn == 0 ? "0" : "the_getExceptions") << ", " << setExcn
                << ", " << (setExcn == 0 ? "0" : "the_setExceptions")
                << " );\n";
            dec();
            out << indent()
                << ("typelib_typedescription_register("
                    " (typelib_TypeDescription**)&pAttribute );\n");
            dec();
            out << indent() << "}\n";
            ++n;
        }
        out << indent()
            << ("typelib_typedescription_release("
                " (typelib_TypeDescription*)pAttribute );\n");
    }
}

void InterfaceType::dumpCppuMethods(FileStream & out, sal_uInt32 & index) {
    if (!entity_->getDirectMethods().empty()) {
        out << "\n" << indent()
            << "typelib_InterfaceMethodTypeDescription * pMethod = 0;\n";
        std::vector< unoidl::InterfaceTypeEntity::Method >::size_type n = 0;
        for (std::vector< unoidl::InterfaceTypeEntity::Method >::const_iterator
                 i(entity_->getDirectMethods().begin());
             i != entity_->getDirectMethods().end(); ++i)
        {
            OUString returnType(resolveAllTypedefs(i->returnType));
            out << indent() << "{\n";
            inc();
            if (!i->parameters.empty()) {
                out << indent() << "typelib_Parameter_Init aParameters["
                    << i->parameters.size() << "];\n";
            }
            std::vector< unoidl::InterfaceTypeEntity::Method::Parameter >::
                size_type m = 0;
            for (std::vector< unoidl::InterfaceTypeEntity::Method::Parameter >::
                     const_iterator j(i->parameters.begin());
                 j != i->parameters.end(); ++j)
            {
                OUString type(resolveAllTypedefs(j->type));
                out << indent() << "::rtl::OUString sParamName" << m << "( \""
                    << j->name << "\" );\n" << indent()
                    << "::rtl::OUString sParamType" << m << "( \"" << type
                    << "\" );\n" << indent() << "aParameters[" << m
                    << "].pParamName = sParamName" << m << ".pData;\n"
                    << indent() << "aParameters[" << m
                    << "].eTypeClass = (typelib_TypeClass)"
                    << getTypeClass(type) << ";\n" << indent() << "aParameters["
                    << m << "].pTypeName = sParamType" << m << ".pData;\n"
                    << indent() << "aParameters[" << m << "].bIn = "
                    << ((j->direction
                         == (unoidl::InterfaceTypeEntity::Method::Parameter::
                             DIRECTION_OUT))
                        ? "sal_False" : "sal_True")
                    << ";\n" << indent() << "aParameters[" << m << "].bOut = "
                    << ((j->direction
                         == (unoidl::InterfaceTypeEntity::Method::Parameter::
                             DIRECTION_IN))
                        ? "sal_False" : "sal_True")
                    << ";\n";
                ++m;
            }
            sal_Int32 excn = dumpExceptionTypeNames(
                out, "", i->exceptions,
                i->name != "acquire" && i->name != "release");
            out << indent() << "::rtl::OUString sReturnType" << n << "( \""
                << returnType << "\" );\n" << indent()
                << "::rtl::OUString sMethodName" << n << "( \"" << name_ << "::"
                << i->name << "\" );\n" << indent()
                << "typelib_typedescription_newInterfaceMethod( &pMethod,\n";
            inc();
            out << indent() << index++ << ", sal_False,\n" << indent()
                << "sMethodName" << n << ".pData,\n" << indent()
                << "(typelib_TypeClass)" << getTypeClass(returnType)
                << ", sReturnType" << n << ".pData,\n" << indent()
                << i->parameters.size() << ", "
                << (i->parameters.empty() ? "0" : "aParameters") << ",\n"
                << indent() << excn << ", "
                << (excn == 0 ? "0" : "the_Exceptions") << " );\n";
            dec();
            out << indent()
                << ("typelib_typedescription_register("
                    " (typelib_TypeDescription**)&pMethod );\n");
            dec();
            out << indent() << "}\n";
            ++n;
        }
        out << indent()
            << ("typelib_typedescription_release("
                " (typelib_TypeDescription*)pMethod );\n");
    }
}

void InterfaceType::dumpAttributesCppuDecl(
    FileStream & out, std::set< OUString > * seen)
{
    assert(seen != 0);
    for (std::vector< unoidl::InterfaceTypeEntity::Attribute >::const_iterator
             i(entity_->getDirectAttributes().begin());
         i != entity_->getDirectAttributes().end(); ++i)
    {
        if (seen->insert(i->type).second) {
            dumpCppuGetType(out, i->type);
        }
        for (std::vector< OUString >::const_iterator j(
                 i->getExceptions.begin());
             j != i->getExceptions.end(); ++j)
        {
            if (seen->insert(*j).second) {
                dumpCppuGetType(out, *j);
            }
        }
        for (std::vector< OUString >::const_iterator j(
                 i->setExceptions.begin());
             j != i->setExceptions.end(); ++j)
        {
            if (seen->insert(*j).second) {
                dumpCppuGetType(out, *j);
            }
        }
    }
}

void InterfaceType::dumpMethodsCppuDecl(
    FileStream & out, std::set< OUString > * seen)
{
    assert(seen != 0);
    for (std::vector< unoidl::InterfaceTypeEntity::Method >::const_iterator i(
             entity_->getDirectMethods().begin());
         i != entity_->getDirectMethods().end(); ++i)
    {
        if (seen->insert(i->returnType).second) {
            dumpCppuGetType(out, i->returnType);
        }
        for (std::vector< unoidl::InterfaceTypeEntity::Method::Parameter >::
                 const_iterator j(i->parameters.begin());
             j != i->parameters.end(); ++j)
        {
            if (seen->insert(j->type).second) {
                dumpCppuGetType(out, j->type);
            }
        }
        for (std::vector< OUString >::const_iterator j(i->exceptions.begin());
             j != i->exceptions.end(); ++j)
        {
            if (seen->insert(*j).second) {
                dumpCppuGetType(out, *j);
            }
        }
    }
}

void InterfaceType::dumpExceptionSpecification(
    FileStream & out, std::vector< OUString > const & exceptions,
    bool runtimeException)
{
    // Exception specifications are undesirable in production code, but make
    // for useful assertions in debug builds (on platforms where they are
    // enforced at runtime):
#if !defined DBG_UTIL
    out << " /*";
#endif
    out << " throw (";
    bool first = true;
    for (std::vector< OUString >::const_iterator i(exceptions.begin());
         i != exceptions.end(); ++i)
    {
        if (*i != "com.sun.star.uno.RuntimeException") {
            if (!first) {
                out << ", ";
            }
            out << codemaker::cpp::scopedCppName(u2b(*i));
            first = false;
        }
    }
    if (runtimeException) {
        if (!first) {
            out << ", ";
        }
        out << "::com::sun::star::uno::RuntimeException";
    }
    out << ")";
#if !defined DBG_UTIL
    out << " */";
#endif
}

void InterfaceType::dumpExceptionTypeName(
    FileStream & out, OUString const & prefix, sal_uInt32 index,
    OUString const & name)
{
    out << indent() << "::rtl::OUString the_" << prefix << "ExceptionName"
        << index << "( \"" << name << "\" );\n";
}

sal_Int32 InterfaceType::dumpExceptionTypeNames(
    FileStream & out, OUString const & prefix,
    std::vector< OUString > const & exceptions, bool runtimeException)
{
    sal_Int32 count = 0;
    for (std::vector< OUString >::const_iterator i(exceptions.begin());
         i != exceptions.end(); ++i)
    {
        if (*i != "com.sun.star.uno.RuntimeException") {
            dumpExceptionTypeName(out, prefix, count++, *i);
        }
    }
    if (runtimeException) {
        dumpExceptionTypeName(
            out, prefix, count++, "com.sun.star.uno.RuntimeException");
    }
    if (count != 0) {
        out << indent() << "rtl_uString * the_" << prefix << "Exceptions[] = {";
        for (sal_Int32 i = 0; i != count; ++i) {
            out << (i == 0 ? " " : ", ") << "the_" << prefix << "ExceptionName"
                << i << ".pData";
        }
        out << " };\n";
    }
    return count;
}

class ConstantGroup: public CppuType {
public:
    ConstantGroup(
        rtl::Reference< unoidl::ConstantGroupEntity > const & entity,
        OUString const & name, rtl::Reference< TypeManager > const & typeMgr):
        CppuType(name, typeMgr), entity_(entity)
    { assert(entity.is()); }

    bool hasConstants() const { return !entity_->getMembers().empty(); }

private:
    virtual void dumpHFile(
        FileStream & out, codemaker::cppumaker::Includes & includes);

    virtual void dumpHxxFile(
        FileStream & out, codemaker::cppumaker::Includes & includes);

    virtual void dumpDeclaration(FileStream & out);

    rtl::Reference< unoidl::ConstantGroupEntity > entity_;
};

void ConstantGroup::dumpHFile(
    FileStream & out, codemaker::cppumaker::Includes & includes)
{
    OUString headerDefine(dumpHeaderDefine(out, "HDL"));
    out << "\n";
    addDefaultHIncludes(includes);
    includes.dump(out, 0);
    out << "\n";
    if (codemaker::cppumaker::dumpNamespaceOpen(out, name_, true)) {
        out << "\n";
    }
    out << "\n";
    dumpDeclaration(out);
    out << "\n";
    if (codemaker::cppumaker::dumpNamespaceClose(out, name_, true)) {
        out << "\n";
    }
    out << "\n#endif // "<< headerDefine << "\n";
}

void ConstantGroup::dumpHxxFile(
    FileStream & out, codemaker::cppumaker::Includes &)
{
    OUString headerDefine(dumpHeaderDefine(out, "HPP"));
    out << "\n";
    codemaker::cppumaker::Includes::dumpInclude(out, u2b(name_), false);
    out << "\n#endif // "<< headerDefine << "\n";
}

void ConstantGroup::dumpDeclaration(FileStream & out) {
    for (std::vector< unoidl::ConstantGroupEntity::Member >::const_iterator i(
             entity_->getMembers().begin());
         i != entity_->getMembers().end(); ++i)
    {
        out << "static const ";
        switch (i->value.type) {
        case unoidl::ConstantValue::TYPE_BOOLEAN:
            out << "::sal_Bool";
            break;
        case unoidl::ConstantValue::TYPE_BYTE:
            out << "::sal_Int8";
            break;
        case unoidl::ConstantValue::TYPE_SHORT:
            out << "::sal_Int16";
            break;
        case unoidl::ConstantValue::TYPE_UNSIGNED_SHORT:
            out << "::sal_uInt16";
            break;
        case unoidl::ConstantValue::TYPE_LONG:
            out << "::sal_Int32";
            break;
        case unoidl::ConstantValue::TYPE_UNSIGNED_LONG:
            out << "::sal_uInt32";
            break;
        case unoidl::ConstantValue::TYPE_HYPER:
            out << "::sal_Int64";
            break;
        case unoidl::ConstantValue::TYPE_UNSIGNED_HYPER:
            out << "::sal_uInt64";
            break;
        case unoidl::ConstantValue::TYPE_FLOAT:
            out << "float";
            break;
        case unoidl::ConstantValue::TYPE_DOUBLE:
            out << "double";
            break;
        }
        out << " " << i->name << " = ";
        switch (i->value.type) {
        case unoidl::ConstantValue::TYPE_BOOLEAN:
            out << (i->value.booleanValue ? "sal_True" : "sal_False");
            break;
        case unoidl::ConstantValue::TYPE_BYTE:
            out << "(sal_Int8)" << OUString::number(i->value.byteValue);
            break;
        case unoidl::ConstantValue::TYPE_SHORT:
            out << "(sal_Int16)" << OUString::number(i->value.shortValue);
            break;
        case unoidl::ConstantValue::TYPE_UNSIGNED_SHORT:
            out << "(sal_uInt16)"
                << OUString::number(i->value.unsignedShortValue);
            break;
        case unoidl::ConstantValue::TYPE_LONG:
            // Avoid C++ compiler warnings about (un)signedness of literal
            // -2^31:
            if (i->value.longValue == SAL_MIN_INT32) {
                out << "SAL_MIN_INT32";
            } else {
                out << "(sal_Int32)" << OUString::number(i->value.longValue);
            }
            break;
        case unoidl::ConstantValue::TYPE_UNSIGNED_LONG:
            out << "(sal_uInt32)"
                << OUString::number(i->value.unsignedLongValue) << "U";
            break;
        case unoidl::ConstantValue::TYPE_HYPER:
            // Avoid C++ compiler warnings about (un)signedness of literal
            // -2^63:
            if (i->value.hyperValue == SAL_MIN_INT64) {
                out << "SAL_MIN_INT64";
            } else {
                out << "(sal_Int64) SAL_CONST_INT64("
                    << OUString::number(i->value.hyperValue) << ")";
            }
            break;
        case unoidl::ConstantValue::TYPE_UNSIGNED_HYPER:
            out << "SAL_CONST_UINT64("
                << OUString::number(i->value.unsignedHyperValue) << ")";
            break;
        case unoidl::ConstantValue::TYPE_FLOAT:
            out << "(float)" << OUString::number(i->value.floatValue);
            break;
        case unoidl::ConstantValue::TYPE_DOUBLE:
            out << "(double)" << OUString::number(i->value.doubleValue);
            break;
        }
        out << ";\n";
    }
}

void dumpTypeParameterName(FileStream & out, OUString const & name) {
    // Prefix all type parameters with "typeparam_" to avoid problems when a
    // struct member has the same name as a type parameter, as in
    // struct<T> { T T; };
    out << "typeparam_" << name;
}

class PlainStructType: public CppuType {
public:
    PlainStructType(
        rtl::Reference< unoidl::PlainStructTypeEntity > const & entity,
        OUString const & name, rtl::Reference< TypeManager > const & typeMgr):
        CppuType(name, typeMgr), entity_(entity)
    { assert(entity.is()); }

private:
    virtual sal_uInt32 checkInheritedMemberCount() const
    { return getTotalMemberCount(entity_->getDirectBase()); }

    virtual void dumpDeclaration(FileStream& o);

    void dumpHxxFile(FileStream& o, codemaker::cppumaker::Includes & includes);

    virtual void dumpLightGetCppuType(FileStream & out);

    virtual void dumpNormalGetCppuType(FileStream & out);

    virtual void dumpComprehensiveGetCppuType(FileStream & out);

    virtual void addLightGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual void addNormalGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual void addComprehensiveGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    bool dumpBaseMembers(
        FileStream & out, OUString const & base, bool withType);

    sal_uInt32 getTotalMemberCount(OUString const & base) const;

    rtl::Reference< unoidl::PlainStructTypeEntity > entity_;
};

void PlainStructType::dumpDeclaration(FileStream & out) {
    out << "\n#ifdef SAL_W32\n#   pragma pack(push, 8)\n#endif\n\n" << indent()
        << "struct " << id_;
    OUString base(entity_->getDirectBase());
    if (!base.isEmpty()) {
        out << ": public " << codemaker::cpp::scopedCppName(u2b(base));
    }
    out << " {\n";
    inc();
    out << indent() << "inline " << id_ << "() SAL_THROW(());\n";
    if (!entity_->getDirectMembers().empty() || getInheritedMemberCount() > 0) {
        out << "\n" << indent() << "inline " << id_ << "(";
        bool first = !dumpBaseMembers(out, base, true);
        for (std::vector< unoidl::PlainStructTypeEntity::Member >::
                 const_iterator i(entity_->getDirectMembers().begin());
             i != entity_->getDirectMembers().end(); ++i)
        {
            if (!first) {
                out << ", ";
            }
            dumpType(out, i->type, true, true);
            out << " " << i->name << "_";
            first = false;
        }
        out << ") SAL_THROW(());\n";
    }
    if (!entity_->getDirectMembers().empty()) {
        out << "\n";
        for (std::vector< unoidl::PlainStructTypeEntity::Member >::
                 const_iterator i(entity_->getDirectMembers().begin());
             i != entity_->getDirectMembers().end(); ++i)
        {
            out << indent();
            dumpType(out, i->type);
            out << " " << i->name;
            if (i == entity_->getDirectMembers().begin() && !base.isEmpty()
                && i->type != "hyper" && i->type != "unsigned hyper"
                && i->type != "double")
            {
                out << " CPPU_GCC3_ALIGN("
                    << codemaker::cpp::scopedCppName(u2b(base)) << ")";
            }
            out << ";\n";
        }
    }
    dec();
    out << "};\n\n#ifdef SAL_W32\n#   pragma pack(pop)\n#endif\n\n";
}

void PlainStructType::dumpHxxFile(
    FileStream & out, codemaker::cppumaker::Includes & includes)
{
    OUString headerDefine(dumpHeaderDefine(out, "HPP"));
    out << "\n";
    includes.dump(out, &name_);
    out << "\n";
    if (codemaker::cppumaker::dumpNamespaceOpen(out, name_, false)) {
        out << "\n";
    }
    out << "\ninline " << id_ << "::" << id_ << "() SAL_THROW(())\n";
    inc();
    OUString base(entity_->getDirectBase());
    bool first = true;
    if (!base.isEmpty()) {
        out << indent() << ": " << codemaker::cpp::scopedCppName(u2b(base))
            << "()\n";
        first = false;
    }
    for (std::vector< unoidl::PlainStructTypeEntity::Member >::const_iterator i(
             entity_->getDirectMembers().begin());
         i != entity_->getDirectMembers().end(); ++i)
    {
        out << indent() << (first ? ":" : ",") << " " << i->name;
        dumpInitializer(out, false, i->type);
        out << "\n";
        first = false;
    }
    dec();
    out << "{\n}\n\n";
    if (!entity_->getDirectMembers().empty() || getInheritedMemberCount() > 0) {
        out << "inline " << id_;
        out << "::" << id_ << "(";
        first = !dumpBaseMembers(out, base, true);
        for (std::vector< unoidl::PlainStructTypeEntity::Member >::
                 const_iterator i(entity_->getDirectMembers().begin());
             i != entity_->getDirectMembers().end(); ++i)
        {
            if (!first) {
                out << ", ";
            }
            dumpType(out, i->type, true, true);
            out << " " << i->name << "_";
            first = false;
        }
        out << ") SAL_THROW(())\n";
        inc();
        first = true;
        if (!base.isEmpty()) {
            out << indent() << ": " << codemaker::cpp::scopedCppName(u2b(base))
                << "(";
            dumpBaseMembers(out, base, false);
            out << ")\n";
            first = false;
        }
        for (std::vector< unoidl::PlainStructTypeEntity::Member >::
                 const_iterator i(entity_->getDirectMembers().begin());
             i != entity_->getDirectMembers().end(); ++i)
        {
            out << indent() << (first ? ":" : ",") << " " << i->name << "("
                << i->name << "_)\n";
            first = false;
        }
        dec();
        out << "{\n}\n\n";
    }
    if (codemaker::cppumaker::dumpNamespaceClose(out, name_, false)) {
        out << "\n";
    }
    out << "\n";
    dumpGetCppuType(out);
    out << "\n#endif // "<< headerDefine << "\n";
}

void PlainStructType::dumpLightGetCppuType(FileStream & out) {
    dumpGetCppuTypePreamble(out);
    out << indent()
        << ("//TODO: On certain platforms with weak memory models, the"
            " following code can result in some threads observing that the_type"
            " points to garbage\n")
        << indent()
        << "static ::typelib_TypeDescriptionReference * the_type = 0;\n"
        << indent() << "if (the_type == 0) {\n";
    inc();
    out << indent() << "::typelib_static_type_init(&the_type, "
        << getTypeClass(name_, true) << ", \"" << name_ << "\");\n";
    dec();
    out << indent() << "}\n" << indent()
        << ("return *reinterpret_cast< ::com::sun::star::uno::Type * >("
            "&the_type);\n");
    dumpGetCppuTypePostamble(out);
}

void PlainStructType::dumpNormalGetCppuType(FileStream & out) {
    dumpGetCppuTypePreamble(out);
    out << indent()
        << ("//TODO: On certain platforms with weak memory models, the"
            " following code can result in some threads observing that the_type"
            " points to garbage\n")
        << indent()
        << "static ::typelib_TypeDescriptionReference * the_type = 0;\n"
        << indent() << "if (the_type == 0) {\n";
    inc();
    out << indent()
        << "::typelib_TypeDescriptionReference * the_members[] = {\n";
    inc();
    for (std::vector< unoidl::PlainStructTypeEntity::Member >::const_iterator i(
             entity_->getDirectMembers().begin());
         i != entity_->getDirectMembers().end();)
    {
        out << indent() << "::cppu::UnoType< ";
        dumpType(out, i->type, false, false, false, true);
        ++i;
        out << " >::get().getTypeLibType()"
            << (i == entity_->getDirectMembers().end() ? " };" : ",") << "\n";
    }
    dec();
    out << indent() << "::typelib_static_struct_type_init(&the_type, \""
        << name_ << "\", ";
    if (entity_->getDirectBase().isEmpty()) {
        out << "0";
    } else {
        out << "::cppu::UnoType< ";
        dumpType(out, entity_->getDirectBase(), false, false, false, true);
        out << " >::get().getTypeLibType()";
    }
    out << ", " << entity_->getDirectMembers().size() << ", the_members, 0);\n";
    dec();
    out << indent() << "}\n" << indent()
        << ("return *reinterpret_cast< ::com::sun::star::uno::Type * >("
            "&the_type);\n");
    dumpGetCppuTypePostamble(out);
}

void PlainStructType::dumpComprehensiveGetCppuType(FileStream & out) {
    OUString staticTypeClass("the" + id_ + "Type");
    codemaker::cppumaker::dumpNamespaceOpen(out, name_, false);
    out << " namespace detail {\n\n" << indent() << "struct "
        << staticTypeClass
        << " : public rtl::StaticWithInit< ::com::sun::star::uno::Type *, "
        << staticTypeClass << " >\n" << indent() << "{\n";
    inc();
    out << indent() << "::com::sun::star::uno::Type * operator()() const\n"
        << indent() << "{\n";
    inc();
    out << indent() << "::rtl::OUString the_name( \"" << name_ << "\" );\n";
    std::map< OUString, sal_uInt32 > types;
    std::vector< unoidl::PlainStructTypeEntity::Member >::size_type n = 0;
    for (std::vector< unoidl::PlainStructTypeEntity::Member >::const_iterator i(
             entity_->getDirectMembers().begin());
         i != entity_->getDirectMembers().end(); ++i)
    {
        if (types.insert(
                std::map< OUString, sal_uInt32 >::value_type(
                    i->type, static_cast< sal_uInt32 >(types.size()))).
            second)
        {
            dumpCppuGetType(out, i->type, &name_);
            // For typedefs, use the resolved type name, as there will be no
            // information available about the typedef itself at runtime (the
            // above getCppuType call will make available information about the
            // resolved type); no extra #include for the resolved type is
            // needed, as the header for the typedef includes it already:
            out << indent() << "::rtl::OUString the_tname"
                << static_cast< sal_uInt32 >(types.size() - 1) << "( \""
                << resolveAllTypedefs(i->type) << "\" );\n";
        }
        out << indent() << "::rtl::OUString the_name" << n++ << "( \""
            << i->name << "\" );\n";
    }
    out << indent() << "::typelib_StructMember_Init the_members[] = {\n";
    inc();
    n = 0;
    for (std::vector< unoidl::PlainStructTypeEntity::Member >::const_iterator i(
             entity_->getDirectMembers().begin());
         i != entity_->getDirectMembers().end();)
    {
        out << indent() << "{ { " << getTypeClass(i->type, true)
            << ", the_tname" << types.find(i->type)->second
            << ".pData, the_name" << n++ << ".pData }, false }";
        ++i;
        out << (i == entity_->getDirectMembers().end() ? " };" : ",") << "\n";
    }
    dec();
    out << indent() << "::typelib_TypeDescription * the_newType = 0;\n"
        << indent()
        << "::typelib_typedescription_newStruct(&the_newType, the_name.pData, ";
    if (entity_->getDirectBase().isEmpty()) {
        out << "0";
    } else {
        out << "::cppu::UnoType< ";
        dumpType(out, entity_->getDirectBase(), false, false, false, true);
        out << " >::get().getTypeLibType()";
    }
    out << ", " << entity_->getDirectMembers().size() << ", the_members);\n"
        << indent() << "::typelib_typedescription_register(&the_newType);\n"
        << indent() << "::typelib_typedescription_release(the_newType);\n"
        << indent() << "return new ::com::sun::star::uno::Type("
        << getTypeClass(name_) << ", the_name); // leaked\n";
    dec();
    out << indent() << "}\n";
    dec();
    out << indent() << "};\n";
    codemaker::cppumaker::dumpNamespaceClose(out, name_, false);
    out << " }\n\n";
    dumpGetCppuTypePreamble(out);
    out << indent() << "return *detail::" << staticTypeClass << "::get();\n";
    dumpGetCppuTypePostamble(out);
}

bool PlainStructType::dumpBaseMembers(
    FileStream & out, OUString const & base, bool withType)
{
    bool hasMember = false;
    if (!base.isEmpty()) {
        rtl::Reference< unoidl::Entity > ent;
        codemaker::UnoType::Sort sort = m_typeMgr->getSort(base, &ent);
        if (sort != codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE) {
            throw CannotDumpException(
                "plain struct type base " + base
                + " is not a plain struct type");
        }
        rtl::Reference< unoidl::PlainStructTypeEntity > ent2(
            dynamic_cast< unoidl::PlainStructTypeEntity * >(ent.get()));
        assert(ent2.is());
        hasMember = dumpBaseMembers(out, ent2->getDirectBase(), withType);
        for (std::vector< unoidl::PlainStructTypeEntity::Member >::
                 const_iterator i(ent2->getDirectMembers().begin());
             i != ent2->getDirectMembers().end(); ++i)
        {
            if (hasMember) {
                out << ", ";
            }
            if (withType) {
                dumpType(out, i->type, true, true);
                out << " ";
            }
            out << i->name << "_";
            hasMember = true;
        }
    }
    return hasMember;
}

void PlainStructType::addLightGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    includes.addType();
    includes.addCppuUnotypeHxx();
    includes.addSalTypesH();
    includes.addTypelibTypeclassH();
    includes.addTypelibTypedescriptionH();
}

void PlainStructType::addNormalGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    includes.addType();
    includes.addCppuUnotypeHxx();
    includes.addSalTypesH();
    includes.addTypelibTypeclassH();
    includes.addTypelibTypedescriptionH();
}

void PlainStructType::addComprehensiveGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    includes.addType();
    includes.addCppuUnotypeHxx();
    includes.addRtlInstanceHxx();
    includes.addOslMutexHxx();
    includes.addRtlUstringH();
    includes.addRtlUstringHxx();
    includes.addSalTypesH();
    includes.addTypelibTypeclassH();
    includes.addTypelibTypedescriptionH();
}

sal_uInt32 PlainStructType::getTotalMemberCount(OUString const & base) const {
    if (base.isEmpty()) {
        return 0;
    }
    rtl::Reference< unoidl::Entity > ent;
    codemaker::UnoType::Sort sort = m_typeMgr->getSort(base, &ent);
    if (sort != codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE) {
        throw CannotDumpException(
            "plain struct type base " + base + " is not a plain struct type");
    }
    rtl::Reference< unoidl::PlainStructTypeEntity > ent2(
        dynamic_cast< unoidl::PlainStructTypeEntity * >(ent.get()));
    assert(ent2.is());
    return getTotalMemberCount(ent2->getDirectBase())
        + ent2->getDirectMembers().size(); //TODO: overflow
}

class PolyStructType: public CppuType {
public:
    PolyStructType(
        rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > const &
            entity,
        OUString const & name, rtl::Reference< TypeManager > const & typeMgr):
        CppuType(name, typeMgr), entity_(entity)
    { assert(entity.is()); }

private:
    virtual void dumpDeclaration(FileStream& o);

    void dumpHxxFile(FileStream& o, codemaker::cppumaker::Includes & includes);

    virtual void dumpLightGetCppuType(FileStream & out);

    virtual void dumpNormalGetCppuType(FileStream & out);

    virtual void dumpComprehensiveGetCppuType(FileStream & out);

    virtual void addLightGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual void addNormalGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual void addComprehensiveGetCppuTypeIncludes(
        codemaker::cppumaker::Includes & includes) const;

    virtual bool isPolymorphic() const { return true; }

    virtual void dumpTemplateHead(FileStream & out) const;

    virtual void dumpTemplateParameters(FileStream & out) const;

    rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > entity_;
};

void PolyStructType::dumpDeclaration(FileStream & out) {
    out << "\n#ifdef SAL_W32\n#   pragma pack(push, 8)\n#endif\n\n" << indent();
    dumpTemplateHead(out);
    out << "struct " << id_ << " {\n";
    inc();
    out << indent() << "inline " << id_ << "() SAL_THROW(());\n";
    if (!entity_->getMembers().empty()) {
        out << "\n" << indent() << "inline " << id_ << "(";
        for (std::vector<
                 unoidl::PolymorphicStructTypeTemplateEntity::Member >::
                 const_iterator i(entity_->getMembers().begin());
             i != entity_->getMembers().end(); ++i)
        {
            if (i != entity_->getMembers().begin()) {
                out << ", ";
            }
            if (i->parameterized) {
                dumpTypeParameterName(out, i->type);
                out << " const &";
            } else {
                dumpType(out, i->type, true, true);
            }
            out << " " << i->name << "_";
        }
        out << ") SAL_THROW(());\n\n";
        for (std::vector<
                 unoidl::PolymorphicStructTypeTemplateEntity::Member >::
                 const_iterator i(entity_->getMembers().begin());
             i != entity_->getMembers().end(); ++i)
        {
            out << indent();
            if (i->parameterized) {
                dumpTypeParameterName(out, i->type);
            } else {
                dumpType(out, i->type);
            }
            out << " " << i->name << ";\n";
        }
    }
    dec();
    out << "};\n\n#ifdef SAL_W32\n#   pragma pack(pop)\n#endif\n\n";
}

void PolyStructType::dumpHxxFile(
    FileStream & out, codemaker::cppumaker::Includes & includes)
{
    OUString headerDefine(dumpHeaderDefine(out, "HPP"));
    out << "\n";
    includes.dump(out, &name_);
    out << "\n";
    if (codemaker::cppumaker::dumpNamespaceOpen(out, name_, false)) {
        out << "\n";
    }
    out << "\n";
    dumpTemplateHead(out);
    out << "inline " << id_;
    dumpTemplateParameters(out);
    out << "::" << id_ << "() SAL_THROW(())\n";
    inc();
    for (std::vector< unoidl::PolymorphicStructTypeTemplateEntity::Member >::
             const_iterator i(entity_->getMembers().begin());
         i != entity_->getMembers().end(); ++i)
    {
        out << indent() << (i == entity_->getMembers().begin() ? ":" : ",")
            << " " << i->name;
        dumpInitializer(out, i->parameterized, i->type);
        out << "\n";
    }
    dec();
    out << "{\n}\n\n";
    if (!entity_->getMembers().empty()) {
        dumpTemplateHead(out);
        out << "inline " << id_;
        dumpTemplateParameters(out);
        out << "::" << id_ << "(";
        for (std::vector<
                 unoidl::PolymorphicStructTypeTemplateEntity::Member >::
                 const_iterator i(entity_->getMembers().begin());
             i != entity_->getMembers().end(); ++i)
        {
            if (i != entity_->getMembers().begin()) {
                out << ", ";
            }
            if (i->parameterized) {
                dumpTypeParameterName(out, i->type);
                out << " const &";
            } else {
                dumpType(out, i->type, true, true);
            }
            out << " " << i->name << "_";
        }
        out << ") SAL_THROW(())\n";
        inc();
        for (std::vector<
                 unoidl::PolymorphicStructTypeTemplateEntity::Member >::
                 const_iterator i(entity_->getMembers().begin());
             i != entity_->getMembers().end(); ++i)
        {
            out << indent() << (i == entity_->getMembers().begin() ? ":" : ",")
                << " " << i->name << "(" << i->name << "_)\n";
        }
        dec();
        out << "{\n}\n\n" << indent();
        dumpTemplateHead(out);
        out << "\n" << indent() << "inline " << id_;
        dumpTemplateParameters(out);
        out << "\n" << indent() << "make_" << id_ << "(";
        for (std::vector<
                 unoidl::PolymorphicStructTypeTemplateEntity::Member >::
                 const_iterator i(entity_->getMembers().begin());
             i != entity_->getMembers().end(); ++i)
        {
            if (i != entity_->getMembers().begin()) {
                out << ", ";
            }
            if (i->parameterized) {
                dumpTypeParameterName(out, i->type);
                out << " const &";
            } else {
                dumpType(out, i->type, true, true);
            }
            out << " " << i->name << "_";
        }
        out << ") SAL_THROW(())\n" << indent() << "{\n";
        inc();
        out << indent() << "return " << id_;
        dumpTemplateParameters(out);
        out << "(";
        for (std::vector<
                 unoidl::PolymorphicStructTypeTemplateEntity::Member >::
                 const_iterator i(entity_->getMembers().begin());
             i != entity_->getMembers().end(); ++i)
        {
            if (i != entity_->getMembers().begin()) {
                out << ", ";
            }
            out << i->name << "_";
        }
        out << ");\n";
        dec();
        out << indent() << "}\n\n";
    }
    if (codemaker::cppumaker::dumpNamespaceClose(out, name_, false)) {
        out << "\n";
    }
    out << "\n";
    dumpGetCppuType(out);
    out << "\n#endif // "<< headerDefine << "\n";
}

void PolyStructType::dumpLightGetCppuType(FileStream & out) {
    dumpGetCppuTypePreamble(out);
    out << indent()
        << ("//TODO: On certain platforms with weak memory models, the"
            " following code can result in some threads observing that the_type"
            " points to garbage\n")
        << indent()
        << "static ::typelib_TypeDescriptionReference * the_type = 0;\n"
        << indent() << "if (the_type == 0) {\n";
    inc();
    out << indent() << "::rtl::OStringBuffer the_buffer(\"" << name_
        << "<\");\n";
    for (std::vector< OUString >::const_iterator i(
             entity_->getTypeParameters().begin());
         i != entity_->getTypeParameters().end();)
    {
        out << indent()
            << ("the_buffer.append(::rtl::OUStringToOString("
                "::cppu::getTypeFavourChar(static_cast< ");
        dumpTypeParameterName(out, *i);
        out << " * >(0)).getTypeName(), RTL_TEXTENCODING_UTF8));\n";
        ++i;
        if (i != entity_->getTypeParameters().end()) {
            out << indent() << "the_buffer.append(',');\n";
        }
    }
    out << indent() << "the_buffer.append('>');\n" << indent()
        << "::typelib_static_type_init(&the_type, " << getTypeClass(name_, true)
        << ", the_buffer.getStr());\n";
    dec();
    out << indent() << "}\n" << indent()
        << ("return *reinterpret_cast< ::com::sun::star::uno::Type * >("
            "&the_type);\n");
    dumpGetCppuTypePostamble(out);
}

void PolyStructType::dumpNormalGetCppuType(FileStream & out) {
    dumpGetCppuTypePreamble(out);
    out << indent()
        << ("//TODO: On certain platforms with weak memory models, the"
            " following code can result in some threads observing that the_type"
            " points to garbage\n")
        << indent()
        << "static ::typelib_TypeDescriptionReference * the_type = 0;\n"
        << indent() << "if (the_type == 0) {\n";
    inc();
    out << indent() << "::rtl::OStringBuffer the_buffer(\"" << name_
        << "<\");\n";
    for (std::vector< OUString >::const_iterator i(
             entity_->getTypeParameters().begin());
         i != entity_->getTypeParameters().end();)
    {
        out << indent()
            << ("the_buffer.append(::rtl::OUStringToOString("
                "::cppu::getTypeFavourChar(static_cast< ");
        dumpTypeParameterName(out, *i);
        out << " * >(0)).getTypeName(), RTL_TEXTENCODING_UTF8));\n";
        ++i;
        if (i != entity_->getTypeParameters().end()) {
            out << indent() << "the_buffer.append(',');\n";
        }
    }
    out << indent() << "the_buffer.append('>');\n" << indent()
        << "::typelib_TypeDescriptionReference * the_members[] = {\n";
    inc();
    for (std::vector< unoidl::PolymorphicStructTypeTemplateEntity::Member >::
             const_iterator i(entity_->getMembers().begin());
         i != entity_->getMembers().end();)
    {
        out << indent();
        if (i->parameterized) {
            out << "::cppu::getTypeFavourChar(static_cast< ";
            dumpTypeParameterName(out, i->type);
            out << " * >(0))";
        } else {
            out << "::cppu::UnoType< ";
            dumpType(out, i->type, false, false, false, true);
            out << " >::get()";
        }
        ++i;
        out << ".getTypeLibType()"
            << (i == entity_->getMembers().end() ? " };" : ",") << "\n";
    }
    dec();
    out << indent() << "static ::sal_Bool const the_parameterizedTypes[] = { ";
    for (std::vector< unoidl::PolymorphicStructTypeTemplateEntity::Member >::
             const_iterator i(entity_->getMembers().begin());
         i != entity_->getMembers().end(); ++i)
    {
        if (i != entity_->getMembers().begin()) {
            out << ", ";
        }
        out << (i->parameterized ? "true" : "false");
    }
    out << " };\n" << indent()
        << ("::typelib_static_struct_type_init(&the_type, the_buffer.getStr(),"
            " 0, ")
        << entity_->getMembers().size()
        << ", the_members, the_parameterizedTypes);\n";
    dec();
    out << indent() << "}\n" << indent()
        << ("return *reinterpret_cast< ::com::sun::star::uno::Type * >("
            "&the_type);\n");
    dumpGetCppuTypePostamble(out);
}

void PolyStructType::dumpComprehensiveGetCppuType(FileStream & out) {
    out << "namespace cppu {  namespace detail {\n\n" << indent();
    dumpTemplateHead(out);
    OUString staticTypeClass("the" + id_ + "Type");
    out << "struct " << staticTypeClass
        << " : public rtl::StaticWithInit< ::com::sun::star::uno::Type *, "
        << staticTypeClass;
    dumpTemplateParameters(out);
    out << " >\n" << indent() << "{\n";
    inc();
    out << indent() << "::com::sun::star::uno::Type * operator()() const\n"
        << indent() << "{\n";
    inc();
    out << indent() << "::rtl::OUStringBuffer the_buffer;\n" << indent()
        << "the_buffer.append(\"" << name_ << "<\");\n";
    for (std::vector< OUString >::const_iterator i(
             entity_->getTypeParameters().begin());
         i != entity_->getTypeParameters().end();)
    {
        out << indent()
            << "the_buffer.append(::cppu::getTypeFavourChar(static_cast< ";
        dumpTypeParameterName(out, *i);
        out << " * >(0)).getTypeName());\n";
        ++i;
        if (i != entity_->getTypeParameters().end()) {
            out << indent()
                << ("the_buffer.append("
                    "static_cast< ::sal_Unicode >(','));\n");
        }
    }
    out << indent() << "the_buffer.append(static_cast< ::sal_Unicode >('>'));\n"
        << indent()
        << "::rtl::OUString the_name(the_buffer.makeStringAndClear());\n";
    std::map< OUString, sal_uInt32 > parameters;
    std::map< OUString, sal_uInt32 > types;
    std::vector< unoidl::PolymorphicStructTypeTemplateEntity::Member >::
        size_type n = 0;
    for (std::vector< unoidl::PolymorphicStructTypeTemplateEntity::Member >::
             const_iterator i(entity_->getMembers().begin());
         i != entity_->getMembers().end(); ++i)
    {
        if (i->parameterized) {
            if (parameters.insert(
                    std::map< OUString, sal_uInt32 >::value_type(
                        i->type, static_cast< sal_uInt32 >(parameters.size()))).
                second)
            {
                sal_uInt32 k = static_cast< sal_uInt32 >(parameters.size() - 1);
                out << indent()
                    << "::com::sun::star::uno::Type const & the_ptype" << k
                    << " = ::cppu::getTypeFavourChar(static_cast< ";
                dumpTypeParameterName(out, i->type);
                out << " * >(0));\n" << indent()
                    << "::typelib_TypeClass the_pclass" << k
                    << " = (::typelib_TypeClass) the_ptype" << k
                    << ".getTypeClass();\n" << indent()
                    << "::rtl::OUString the_pname" << k << "(the_ptype" << k
                    << ".getTypeName());\n";
            }
        } else if (types.insert(
                       std::map< OUString, sal_uInt32 >::value_type(
                           i->type, static_cast< sal_uInt32 >(types.size()))).
                   second)
        {
            dumpCppuGetType(out, i->type, &name_);
            // For typedefs, use the resolved type name, as there will be no
            // information available about the typedef itself at runtime (the
            // above getCppuType call will make available information about the
            // resolved type); no extra #include for the resolved type is
            // needed, as the header for the typedef includes it already:
            out << indent() << "::rtl::OUString the_tname"
                << static_cast< sal_uInt32 >(types.size() - 1) << "( \""
                << resolveAllTypedefs(i->type) << "\" );\n";
        }
        out << indent() << "::rtl::OUString the_name" << n++ << "( \""
            << i->name << "\" );\n";
    }
    out << indent() << "::typelib_StructMember_Init the_members[] = {\n";
    inc();
    n = 0;
    for (std::vector< unoidl::PolymorphicStructTypeTemplateEntity::Member >::
             const_iterator i(entity_->getMembers().begin());
         i != entity_->getMembers().end();)
    {
        out << indent() << "{ { ";
        if (i->parameterized) {
            sal_uInt32 k = parameters.find(i->type)->second;
            out << "the_pclass" << k << ", the_pname" << k << ".pData";
        } else {
            out << getTypeClass(i->type, true) << ", the_tname"
                << types.find(i->type)->second << ".pData";
        }
        out << ", the_name" << n++ << ".pData }, "
            << (i->parameterized ? "true" : "false") << " }";
        ++i;
        out << (i == entity_->getMembers().end() ? " };" : ",") << "\n";
    }
    dec();
    out << indent() << "::typelib_TypeDescription * the_newType = 0;\n";
    out << indent()
        << ("::typelib_typedescription_newStruct(&the_newType, the_name.pData,"
            " 0, ")
        << entity_->getMembers().size() << ", the_members);\n" << indent()
        << "::typelib_typedescription_register(&the_newType);\n" << indent()
        << "::typelib_typedescription_release(the_newType);\n" << indent()
        << "return new ::com::sun::star::uno::Type(" << getTypeClass(name_)
        << ", the_name); // leaked\n";
    dec();
    out << indent() << "}\n";
    dec();
    out << indent() << "};\n" << " }" << " }\n\n";
    dumpGetCppuTypePreamble(out);
    out << indent() << "return *detail::" << staticTypeClass;
    dumpTemplateParameters(out);
    out << "::get();\n";
    dumpGetCppuTypePostamble(out);
}

void PolyStructType::addLightGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    includes.addType();
    includes.addCppuUnotypeHxx();
    includes.addSalTypesH();
    includes.addTypelibTypeclassH();
    includes.addTypelibTypedescriptionH();
    includes.addRtlStrbufHxx();
    includes.addRtlTextencH();
    includes.addRtlUstringHxx();
}

void PolyStructType::addNormalGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    includes.addType();
    includes.addCppuUnotypeHxx();
    includes.addSalTypesH();
    includes.addTypelibTypeclassH();
    includes.addTypelibTypedescriptionH();
    includes.addRtlStrbufHxx();
    includes.addRtlTextencH();
    includes.addRtlUstringHxx();
}

void PolyStructType::addComprehensiveGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    includes.addType();
    includes.addCppuUnotypeHxx();
    includes.addRtlInstanceHxx();
    includes.addOslMutexHxx();
    includes.addRtlUstringH();
    includes.addRtlUstringHxx();
    includes.addSalTypesH();
    includes.addTypelibTypeclassH();
    includes.addTypelibTypedescriptionH();
    includes.addRtlStringH();
    includes.addRtlUstrbufHxx();
}

void PolyStructType::dumpTemplateHead(FileStream & out) const {
    out << "template< ";
    for (std::vector< OUString >::const_iterator i(
             entity_->getTypeParameters().begin());
         i != entity_->getTypeParameters().end(); ++i)
    {
        if (i != entity_->getTypeParameters().begin()) {
            out << ", ";
        }
        out << "typename ";
        dumpTypeParameterName(out, *i);
    }
    out << " > ";
}

void PolyStructType::dumpTemplateParameters(FileStream & out) const {
    out << "< ";
    for (std::vector< OUString >::const_iterator i(
             entity_->getTypeParameters().begin());
         i != entity_->getTypeParameters().end(); ++i)
    {
        if (i != entity_->getTypeParameters().begin()) {
            out << ", ";
        }
        dumpTypeParameterName(out, *i);
    }
    out << " >";
}

OUString typeToIdentifier(OUString const & name) {
    sal_Int32 k;
    OUString n(b2u(codemaker::UnoType::decompose(u2b(name), &k)));
    OUStringBuffer b;
    for (sal_Int32 i = 0; i != k; ++i) {
        b.append("seq_");
    }
    b.append(n);
    b.replace(' ', '_');
    b.replace(',', '_');
    b.replace('.', '_');
    b.replace('<', '_');
    b.replace('>', '_');
    return b.makeStringAndClear();
}

class ExceptionType: public CppuType {
public:
    ExceptionType(
        rtl::Reference< unoidl::ExceptionTypeEntity > const & entity,
        OUString const & name, rtl::Reference< TypeManager > const & typeMgr):
        CppuType(name, typeMgr), entity_(entity)
    { assert(entity.is()); }

private:
    virtual void dumpHxxFile(
        FileStream & out, codemaker::cppumaker::Includes & includes);

    virtual void dumpLightGetCppuType(FileStream & out);

    virtual void dumpNormalGetCppuType(FileStream & out);

    virtual void dumpComprehensiveGetCppuType(FileStream & out);

    virtual sal_uInt32 checkInheritedMemberCount() const
    { return getTotalMemberCount(entity_->getDirectBase()); }

    virtual void dumpDeclaration(FileStream & out);

    bool dumpBaseMembers(
        FileStream & out, OUString const & base, bool withType);

    sal_uInt32 getTotalMemberCount(OUString const & base) const;

    rtl::Reference< unoidl::ExceptionTypeEntity > entity_;
};

void ExceptionType::dumpHxxFile(
    FileStream & out, codemaker::cppumaker::Includes & includes)
{
    OUString headerDefine(dumpHeaderDefine(out, "HPP"));
    out << "\n";
    addDefaultHxxIncludes(includes);
    includes.dump(out, &name_);
    out << "\n";
    if (codemaker::cppumaker::dumpNamespaceOpen(out, name_, false)) {
        out << "\n";
    }
    out << "\ninline " << id_ << "::" << id_ << "() SAL_THROW(())\n";
    inc();
    OUString base(entity_->getDirectBase());
    bool first = true;
    if (!base.isEmpty()) {
        out << indent() << ": " << codemaker::cpp::scopedCppName(u2b(base))
            << "()\n";
        first = false;
    }
    for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator i(
             entity_->getDirectMembers().begin());
         i != entity_->getDirectMembers().end(); ++i)
    {
        out << indent() << (first ? ":" : ",") << " ";
        out << i->name;
        dumpInitializer(out, false, i->type);
        out << "\n";
        first = false;
    }
    dec();
    out << "{";
    if (!m_cppuTypeDynamic) {
        out << "\n";
        inc();
        dumpCppuGetType(out, name_);
        dec();
    } else {
        out << " ";
    }
    out << "}\n\n";
    if (!entity_->getDirectMembers().empty() || getInheritedMemberCount() > 0) {
        out << indent() << "inline " << id_ << "::" << id_ << "(";
        first = !dumpBaseMembers(out, base, true);
        for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator
                 i(entity_->getDirectMembers().begin());
             i != entity_->getDirectMembers().end(); ++i)
        {
            if (!first) {
                out << ", ";
            }
            dumpType(out, i->type, true, true);
            out << " " << i->name << "_";
            first = false;
        }
        out << ") SAL_THROW(())\n";
        inc();
        first = true;
        if (!base.isEmpty()) {
            out << indent() << ": " << codemaker::cpp::scopedCppName(u2b(base))
                << "(";
            dumpBaseMembers(out, base, false);
            out << ")\n";
            first = false;
        }
        for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator
                 i(entity_->getDirectMembers().begin());
             i != entity_->getDirectMembers().end(); ++i)
        {
            out << indent() << (first ? ":" : ",") << " " << i->name << "("
                << i->name << "_)\n";
            first = false;
        }
        dec();
        out << "{";
        if (!m_cppuTypeDynamic) {
            out << "\n";
            inc();
            dumpCppuGetType(out, name_);
            dec();
        } else {
            out << " ";
        }
        out << "}\n\n";
    }
    out << indent() << id_ << "::" << id_ << "(" << id_
        << " const & the_other)";
    first = true;
    if (!base.isEmpty()) {
        out << ": " << codemaker::cpp::scopedCppName(u2b(base))
            << "(the_other)";
        first = false;
    }
    for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator i(
             entity_->getDirectMembers().begin());
         i != entity_->getDirectMembers().end(); ++i)
    {
        out << (first ? ":" : ",") << " " << i->name << "(the_other." << i->name
            << ")";
        first = false;
    }
    out << indent() << " {}\n\n" << indent() << id_ << "::~" << id_
        << "() {}\n\n" << indent() << id_ << " & " << id_ << "::operator =("
        << id_ << " const & the_other) {\n";
    inc();
    out << indent()
        << ("//TODO: Just like its implicitly-defined counterpart, this"
            " function definition is not exception-safe\n");
    if (!base.isEmpty()) {
        out << indent() << codemaker::cpp::scopedCppName(u2b(base))
            << "::operator =(the_other);\n";
    }
    for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator i(
             entity_->getDirectMembers().begin());
         i != entity_->getDirectMembers().end(); ++i)
    {
        out << indent() << i->name << " = the_other." << i->name << ";\n";
    }
    out << indent() << "return *this;\n";
    dec();
    out << indent() << "}\n\n";
    if (codemaker::cppumaker::dumpNamespaceClose(out, name_, false)) {
        out << "\n";
    }
    out << "\n";
    dumpGetCppuType(out);
    out << "\n#endif // "<< headerDefine << "\n";
}

void ExceptionType::dumpLightGetCppuType(FileStream & out) {
    dumpGetCppuTypePreamble(out);
    out << indent()
        << "static typelib_TypeDescriptionReference * the_type = 0;\n"
        << indent() << "if ( !the_type )\n" << indent() << "{\n";
    inc();
    out << indent() << "typelib_static_type_init( &the_type, "
        << getTypeClass(name_, true) << ", \"" << name_ << "\" );\n";
    dec();
    out << indent() << "}\n" << indent()
        << ("return * reinterpret_cast< ::com::sun::star::uno::Type * >("
            " &the_type );\n");
    dumpGetCppuTypePostamble(out);
}

void ExceptionType::dumpNormalGetCppuType(FileStream & out) {
    dumpGetCppuTypePreamble(out);
    out << indent()
        << "static typelib_TypeDescriptionReference * the_type = 0;\n"
        << indent() << "if ( !the_type )\n" << indent() << "{\n";
    inc();
    OUString base(entity_->getDirectBase());
    bool baseException = false;
    if (!base.isEmpty()) {
        if (base == "com.sun.star.uno.Exception") {
            baseException = true;
        } else {
            out << indent()
                << ("const ::com::sun::star::uno::Type& rBaseType ="
                    " ::cppu::UnoType< ");
            dumpType(out, base, true, false, false, true);
            out << " >::get();\n\n";
        }
    }
    if (!entity_->getDirectMembers().empty()) {
        out << indent() << "typelib_TypeDescriptionReference * aMemberRefs["
            << entity_->getDirectMembers().size() << "];\n";
        std::set< OUString > seen;
        std::vector< unoidl::ExceptionTypeEntity::Member >::size_type n = 0;
        for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator
                 i(entity_->getDirectMembers().begin());
             i != entity_->getDirectMembers().end(); ++i)
        {
            OUString type(resolveAllTypedefs(i->type));
            OUString modType(typeToIdentifier(type));
            if (seen.insert(type).second) {
                out << indent()
                    << "const ::com::sun::star::uno::Type& rMemberType_"
                    << modType << " = ::cppu::UnoType< ";
                dumpType(out, type, false, false, false, true);
                out << " >::get();\n";
            }
            out << indent() << "aMemberRefs[" << n++ << "] = rMemberType_"
                << modType << ".getTypeLibType();\n";
        }
        out << "\n";
    }
    out << indent() << "typelib_static_compound_type_init( &the_type, "
        << getTypeClass(name_, true) << ", \"" << name_ << "\", ";
    if (baseException) {
        out << ("* ::typelib_static_type_getByTypeClass("
                " typelib_TypeClass_EXCEPTION )");
    } else if (base.isEmpty()) {
        out << "0";
    } else {
        out << "rBaseType.getTypeLibType()";
    }
    out << ", " << entity_->getDirectMembers().size() << ",  "
        << (entity_->getDirectMembers().empty() ? "0" : "aMemberRefs")
        << " );\n";
    dec();
    out << indent() << "}\n" << indent()
        << ("return * reinterpret_cast< const ::com::sun::star::uno::Type * >("
            " &the_type );\n");
    dumpGetCppuTypePostamble(out);
}

void ExceptionType::dumpComprehensiveGetCppuType(FileStream & out) {
    codemaker::cppumaker::dumpNamespaceOpen(out, name_, false);
    out << " namespace detail {\n\n";
    OUString staticTypeClass("the" + id_ + "Type");
    out << indent() << "struct " << staticTypeClass
        << " : public rtl::StaticWithInit< ::com::sun::star::uno::Type *, "
        << staticTypeClass << " >\n" << indent() << "{\n";
    inc();
    out << indent() << "::com::sun::star::uno::Type * operator()() const\n"
        << indent() << "{\n";
    inc();
    out << indent() << "::rtl::OUString sTypeName( \"" << name_ << "\" );\n\n"
        << indent() << "// Start inline typedescription generation\n"
        << indent() << "typelib_TypeDescription * pTD = 0;\n";
    OUString base(entity_->getDirectBase());
    if (!base.isEmpty()) {
        out << indent()
            << ("const ::com::sun::star::uno::Type& rSuperType ="
                " ::cppu::UnoType< ");
        dumpType(out, base, false, false, false, true);
        out << " >::get();\n";
    }
    std::set< OUString > seen;
    for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator i(
             entity_->getDirectMembers().begin());
         i != entity_->getDirectMembers().end(); ++i)
    {
        if (seen.insert(i->type).second) {
            dumpCppuGetType(out, i->type);
        }
    }
    if (!entity_->getDirectMembers().empty()) {
        out << "\n" << indent() << "typelib_CompoundMember_Init aMembers["
            << entity_->getDirectMembers().size() << "];\n";
        std::vector< unoidl::ExceptionTypeEntity::Member >::size_type n = 0;
        for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator
                 i(entity_->getDirectMembers().begin());
             i != entity_->getDirectMembers().end(); ++i)
        {
            OUString type(resolveAllTypedefs(i->type));
            out << indent() << "::rtl::OUString sMemberType" << n << "( \""
                << type << "\" );\n" << indent()
                << "::rtl::OUString sMemberName" << n << "( \"" << i->name
                << "\" );\n" << indent() << "aMembers[" << n
                << "].eTypeClass = (typelib_TypeClass)" << getTypeClass(type)
                << ";\n" << indent() << "aMembers[" << n
                << "].pTypeName = sMemberType" << n << ".pData;\n" << indent()
                << "aMembers[" << n << "].pMemberName = sMemberName" << n
                << ".pData;\n";
            ++n;
        }
    }
    out << "\n" << indent() << "typelib_typedescription_new(\n";
    inc();
    out << indent() << "&pTD,\n" << indent() << "(typelib_TypeClass)"
        << getTypeClass(name_) << ", sTypeName.pData,\n" << indent()
        << (base.isEmpty() ? "0" : "rSuperType.getTypeLibType()") << ",\n"
        << indent() << entity_->getDirectMembers().size() << ",\n" << indent()
        << (entity_->getDirectMembers().empty() ? "0" : "aMembers")
        << " );\n\n";
    dec();
    out << indent()
        << ("typelib_typedescription_register( (typelib_TypeDescription**)&pTD"
            " );\n\n")
        << indent() << "typelib_typedescription_release( pTD );\n" << indent()
        << "// End inline typedescription generation\n\n" << indent()
        << "return new ::com::sun::star::uno::Type( " << getTypeClass(name_)
        << ", sTypeName ); // leaked\n";
    dec();
    out << indent() << "}\n";
    dec();
    out << indent() << "};\n\n";
    codemaker::cppumaker::dumpNamespaceClose(out, name_, false);
    out << " }\n\n";
    dumpGetCppuTypePreamble(out);
    out << indent() << "return *detail::" << staticTypeClass << "::get();\n";
    dumpGetCppuTypePostamble(out);
}

void ExceptionType::dumpDeclaration(FileStream & out) {
    out << "\nclass CPPU_GCC_DLLPUBLIC_EXPORT " << id_;
    OUString base(entity_->getDirectBase());
    if (!base.isEmpty()) {
        out << " : public " << codemaker::cpp::scopedCppName(u2b(base));
    }
    out << "\n{\npublic:\n";
    inc();
    out << indent() << "inline CPPU_GCC_DLLPRIVATE " << id_
        << "() SAL_THROW(());\n\n";
    if (!entity_->getDirectMembers().empty() || getInheritedMemberCount() > 0) {
        out << indent() << "inline CPPU_GCC_DLLPRIVATE " << id_ << "(";
        bool first = !dumpBaseMembers(out, base, true);
        for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator
                 i(entity_->getDirectMembers().begin());
             i != entity_->getDirectMembers().end(); ++i)
        {
            if (!first) {
                out << ", ";
            }
            dumpType(out, i->type, true, true);
            out << " " << i->name << "_";
            first = false;
        }
        out << ") SAL_THROW(());\n\n";
    }
    out << indent() << "inline CPPU_GCC_DLLPRIVATE " << id_ << "(" << id_
        << " const &);\n\n" << indent() << "inline CPPU_GCC_DLLPRIVATE ~"
        << id_ << "();\n\n" << indent() << "inline CPPU_GCC_DLLPRIVATE " << id_
        << " & operator =(" << id_ << " const &);\n\n";
    for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator i(
             entity_->getDirectMembers().begin());
         i != entity_->getDirectMembers().end(); ++i)
    {
        out << indent();
        dumpType(out, i->type);
        out << " " << i->name;
        if (i == entity_->getDirectMembers().begin() && !base.isEmpty()
            && i->type != "hyper" && i->type != "unsigned hyper"
            && i->type != "double")
        {
            out << " CPPU_GCC3_ALIGN( "
                << codemaker::cpp::scopedCppName(u2b(base)) << " )";
        }
        out << ";\n";
    }
    dec();
    out << "};\n\n";
}

bool ExceptionType::dumpBaseMembers(
    FileStream & out, OUString const & base, bool withType)
{
    bool hasMember = false;
    if (!base.isEmpty()) {
        rtl::Reference< unoidl::Entity > ent;
        codemaker::UnoType::Sort sort = m_typeMgr->getSort(base, &ent);
        if (sort != codemaker::UnoType::SORT_EXCEPTION_TYPE) {
            throw CannotDumpException(
                "exception type base " + base + " is not an exception type");
        }
        rtl::Reference< unoidl::ExceptionTypeEntity > ent2(
            dynamic_cast< unoidl::ExceptionTypeEntity * >(ent.get()));
        assert(ent2.is());
        hasMember = dumpBaseMembers(out, ent2->getDirectBase(), withType);
        for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator
                 i(ent2->getDirectMembers().begin());
             i != ent2->getDirectMembers().end(); ++i)
        {
            if (hasMember) {
                out << ", ";
            }
            if (withType) {
                dumpType(out, i->type, true, true);
                out << " ";
            }
            out << i->name << "_";
            hasMember = true;
        }
    }
    return hasMember;
}

sal_uInt32 ExceptionType::getTotalMemberCount(OUString const & base) const {
    if (base.isEmpty()) {
        return 0;
    }
    rtl::Reference< unoidl::Entity > ent;
    codemaker::UnoType::Sort sort = m_typeMgr->getSort(base, &ent);
    if (sort != codemaker::UnoType::SORT_EXCEPTION_TYPE) {
        throw CannotDumpException(
            "exception type base " + base + " is not an exception type");
    }
    rtl::Reference< unoidl::ExceptionTypeEntity > ent2(
        dynamic_cast< unoidl::ExceptionTypeEntity * >(ent.get()));
    assert(ent2.is());
    return getTotalMemberCount(ent2->getDirectBase())
        + ent2->getDirectMembers().size(); //TODO: overflow
}

class EnumType: public CppuType {
public:
    EnumType(
        rtl::Reference< unoidl::EnumTypeEntity > const & entity,
        OUString const & name, rtl::Reference< TypeManager > const & typeMgr):
        CppuType(name, typeMgr), entity_(entity)
    { assert(entity.is()); }

private:
    virtual void dumpDeclaration(FileStream& o);

    void dumpHxxFile(FileStream& o, codemaker::cppumaker::Includes & includes);

    void        dumpNormalGetCppuType(FileStream& o);
    void        dumpComprehensiveGetCppuType(FileStream& o);

    rtl::Reference< unoidl::EnumTypeEntity > entity_;
};

void EnumType::dumpDeclaration(FileStream& o)
{
    o << "\nenum " << id_ << "\n{\n";
    inc();

    for (std::vector< unoidl::EnumTypeEntity::Member >::const_iterator i(
             entity_->getMembers().begin());
         i != entity_->getMembers().end(); ++i)
    {
        o << indent() << id_ << "_" << u2b(i->name) << " = " << i->value
          << ",\n";
    }

    o << indent() << id_ << "_MAKE_FIXED_SIZE = SAL_MAX_ENUM\n";

    dec();
    o << "};\n\n";
}

void EnumType::dumpHxxFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
{
    OUString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << "\n";

    addDefaultHxxIncludes(includes);
    includes.dump(o, &name_);
    o << "\n";

    dumpGetCppuType(o);

    o << "\n#endif // "<< headerDefine << "\n";
}

void EnumType::dumpNormalGetCppuType(FileStream& o)
{
    dumpGetCppuTypePreamble(o);

    o << indent()
      << "static typelib_TypeDescriptionReference * the_type = 0;\n";

    o << indent() << "if ( !the_type )\n" << indent() << "{\n";
    inc();

    o << indent() << "typelib_static_enum_type_init( &the_type,\n";
    inc(31);
    o << indent() << "\"" << name_ << "\",\n"
      << indent() << codemaker::cpp::scopedCppName(u2b(name_)) << "_"
      << u2b(entity_->getMembers()[0].name) << " );\n";
    dec(31);
    dec();
    o << indent() << "}\n";
    o << indent()
      << ("return * reinterpret_cast< ::com::sun::star::uno::Type * >("
          " &the_type );\n");
    dumpGetCppuTypePostamble(o);
}

void EnumType::dumpComprehensiveGetCppuType(FileStream& o)
{
    if (!isPolymorphic())
        codemaker::cppumaker::dumpNamespaceOpen(o, name_, false);
    else
        o << "namespace cppu { ";
    o << " namespace detail {\n\n";

    OUString sStaticTypeClass("the" + id_ + "Type");
    o << indent() << "struct " << sStaticTypeClass << " : public rtl::StaticWithInit< ::com::sun::star::uno::Type *, " << sStaticTypeClass << " >\n";
    o << indent() << "{\n";
    inc();
    o << indent() << "::com::sun::star::uno::Type * operator()() const\n";
    o << indent() << "{\n";

    inc();
    o << indent() << "::rtl::OUString sTypeName( \"" << name_
      << "\" );\n\n";

    o << indent() << "// Start inline typedescription generation\n"
      << indent() << "typelib_TypeDescription * pTD = 0;\n\n";

    o << indent() << "rtl_uString* enumValueNames["
      << entity_->getMembers().size() << "];\n";
    std::vector< unoidl::EnumTypeEntity::Member >::size_type n = 0;
    for (std::vector< unoidl::EnumTypeEntity::Member >::const_iterator i(
             entity_->getMembers().begin());
         i != entity_->getMembers().end(); ++i)
    {
        o << indent() << "::rtl::OUString sEnumValue" << n << "( \""
          << u2b(i->name) << "\" );\n";
        o << indent() << "enumValueNames[" << n << "] = sEnumValue" << n
          << ".pData;\n";
        ++n;
    }

    o << "\n" << indent() << "sal_Int32 enumValues["
      << entity_->getMembers().size() << "];\n";
    n = 0;
    for (std::vector< unoidl::EnumTypeEntity::Member >::const_iterator i(
             entity_->getMembers().begin());
         i != entity_->getMembers().end(); ++i)
    {
        o << indent() << "enumValues[" << n++ << "] = " << i->value << ";\n";
    }

    o << "\n" << indent() << "typelib_typedescription_newEnum( &pTD,\n";
    inc();
    o << indent() << "sTypeName.pData,\n"
      << indent() << "(sal_Int32)"
      << codemaker::cpp::scopedCppName(u2b(name_), false) << "_"
      << u2b(entity_->getMembers()[0].name) << ",\n"
      << indent() << entity_->getMembers().size()
      << ", enumValueNames, enumValues );\n\n";
    dec();

    o << indent()
      << ("typelib_typedescription_register( (typelib_TypeDescription**)&pTD"
          " );\n");
    o << indent() << "typelib_typedescription_release( pTD );\n"
      << indent() << "// End inline typedescription generation\n\n";

    o << indent() << "return new ::com::sun::star::uno::Type( "
      << getTypeClass(name_) << ", sTypeName ); // leaked\n";

    dec();
    o << indent() << "}\n";
    dec();
    o << indent() << "};\n\n";

    if (!isPolymorphic())
        codemaker::cppumaker::dumpNamespaceClose(o, name_, false);
    else
        o << " }";
    o << " }\n\n";

    dumpGetCppuTypePreamble(o);
    o  << indent() << "return *detail::" << sStaticTypeClass << "::get();\n";
    dumpGetCppuTypePostamble(o);
}

class Typedef: public CppuType {
public:
    Typedef(
        rtl::Reference< unoidl::TypedefEntity > const & entity,
        OUString const & name, rtl::Reference< TypeManager > const & typeMgr):
        CppuType(name, typeMgr), entity_(entity)
    { assert(entity.is()); }

private:
    virtual void dumpDeclaration(FileStream& o);

    void dumpHFile(FileStream& o, codemaker::cppumaker::Includes & includes);

    void dumpHxxFile(FileStream& o, codemaker::cppumaker::Includes & includes);

    rtl::Reference< unoidl::TypedefEntity > entity_;
};

void Typedef::dumpHFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
{
    OUString headerDefine(dumpHeaderDefine(o, "HDL"));
    o << "\n";

    addDefaultHIncludes(includes);
    includes.dump(o, 0);
    o << "\n";

    if (codemaker::cppumaker::dumpNamespaceOpen(o, name_, false)) {
        o << "\n";
    }

    dumpDeclaration(o);

    if (codemaker::cppumaker::dumpNamespaceClose(o, name_, false)) {
        o << "\n";
    }

    o << "#endif // "<< headerDefine << "\n";
}

void Typedef::dumpDeclaration(FileStream& o)
{
    o << "\ntypedef ";
    dumpType(o, entity_->getType());
    o << " " << id_ << ";\n\n";
}

void Typedef::dumpHxxFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
{
    OUString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << "\n";

    addDefaultHxxIncludes(includes);
    includes.dump(o, &name_);
    o << "\n";

    o << "\n#endif // "<< headerDefine << "\n";
}

class ConstructiveType: public CppuType {
public:
    ConstructiveType(
        OUString const & name, rtl::Reference< TypeManager > const & manager):
        CppuType(name, manager) {}

private:
    virtual void dumpHFile(FileStream &, codemaker::cppumaker::Includes &)
    { assert(false); } // this cannot happen

    virtual void dumpFiles(OUString const & uri, CppuOptions const & options)
    { dumpFile(uri, name_, true, options); }
};

bool hasRestParameter(
    unoidl::SingleInterfaceBasedServiceEntity::Constructor const & constructor)
{
    return !constructor.parameters.empty()
        && constructor.parameters.back().rest;
}

void includeExceptions(
    codemaker::cppumaker::Includes & includes,
    codemaker::ExceptionTreeNode const * node)
{
    if (node->present) {
        includes.add(node->name);
    } else {
        for (codemaker::ExceptionTreeNode::Children::const_iterator i(
                 node->children.begin());
             i != node->children.end(); ++i)
        {
            includeExceptions(includes, *i);
        }
    }
}

class ServiceType: public ConstructiveType {
public:
    ServiceType(
        rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > const &
            entity,
        OUString const & name, rtl::Reference< TypeManager > const & manager):
        ConstructiveType(name, manager), entity_(entity)
    { assert(entity.is()); }

private:
    virtual void dumpHxxFile(
        FileStream & o, codemaker::cppumaker::Includes & includes);

    void dumpCatchClauses(
        FileStream & out, codemaker::ExceptionTreeNode const * node);

    rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > entity_;
};

void ServiceType::dumpHxxFile(
    FileStream & o, codemaker::cppumaker::Includes & includes)
{
    if (!entity_->getConstructors().empty()) {
        //TODO: Decide whether the types added to includes should rather be
        // added to m_dependencies (and thus be generated during
        // dumpDependedTypes):
        includes.addCassert();
        includes.addReference();
        includes.addRtlUstringH();
        includes.addRtlUstringHxx();
        includes.add("com.sun.star.uno.DeploymentException");
        includes.add("com.sun.star.uno.XComponentContext");
        for (std::vector<
                 unoidl::SingleInterfaceBasedServiceEntity::Constructor >::
                 const_iterator i(entity_->getConstructors().begin());
             i != entity_->getConstructors().end(); ++i)
        {
            if (i->defaultConstructor) {
                includes.add("com.sun.star.uno.Exception");
                includes.add("com.sun.star.uno.RuntimeException");
            } else {
                if (!hasRestParameter(*i)) {
                    includes.addAny();
                    includes.addSequence();
                    for (std::vector<
                             unoidl::SingleInterfaceBasedServiceEntity::
                             Constructor::Parameter >::const_iterator j(
                                 i->parameters.begin());
                         j != i->parameters.end(); ++j)
                    {
                        if (m_typeMgr->getSort(
                                b2u(codemaker::UnoType::decompose(
                                        u2b(j->type), 0, 0)))
                            == codemaker::UnoType::SORT_CHAR)
                        {
                            includes.addCppuUnotypeHxx();
                            break;
                        }
                    }
                }
                codemaker::ExceptionTree tree;
                for (std::vector< OUString >::const_iterator j(
                         i->exceptions.begin());
                     j != i->exceptions.end(); ++j)
                {
                    tree.add(u2b(*j), m_typeMgr);
                }
                if (!tree.getRoot()->present) {
                    includes.add("com.sun.star.uno.Exception");
                    includes.add("com.sun.star.uno.RuntimeException");
                    includeExceptions(includes, tree.getRoot());
                }
            }
        }
    }
    OString cppName(
        codemaker::cpp::translateUnoToCppIdentifier(
            u2b(id_), "service", isGlobal()));
    OUString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << "\n";
    includes.dump(o, 0);
    o << "\n";
    if (codemaker::cppumaker::dumpNamespaceOpen(o, name_, false)) {
        o << "\n";
    }
    o << "\nclass " << cppName << " {\n";
    inc();
    if (!entity_->getConstructors().empty()) {
        OString baseName(u2b(entity_->getBase()));
        OString scopedBaseName(codemaker::cpp::scopedCppName(baseName));
        o << "public:\n";
        for (std::vector<
                 unoidl::SingleInterfaceBasedServiceEntity::Constructor >::
                 const_iterator i(entity_->getConstructors().begin());
             i != entity_->getConstructors().end(); ++i)
        {
            if (i->defaultConstructor) {
                o << indent() << "static ::com::sun::star::uno::Reference< "
                  << scopedBaseName << " > "
                  << codemaker::cpp::translateUnoToCppIdentifier(
                      "create", "method", codemaker::cpp::ITM_NONGLOBAL,
                      &cppName)
                  << ("(::com::sun::star::uno::Reference<"
                      " ::com::sun::star::uno::XComponentContext > const &"
                      " the_context) {\n");
                inc();
                o << indent() << "assert(the_context.is());\n" << indent()
                  << "::com::sun::star::uno::Reference< " << scopedBaseName
                  << " > the_instance;\n" << indent() << "try {\n";
                inc();
                o << indent()
                  << "the_instance = ::com::sun::star::uno::Reference< "
                  << scopedBaseName
                  << (" >(the_context->getServiceManager()->"
                      "createInstanceWithContext(::rtl::OUString("
                      " \"")
                  << name_
                  << "\" ), the_context), ::com::sun::star::uno::UNO_QUERY);\n";
                dec();
                o << indent()
                  << "} catch (const ::com::sun::star::uno::RuntimeException &) {\n";
                inc();
                o << indent() << "throw;\n";
                dec();
                o << indent()
                  << ("} catch (const ::com::sun::star::uno::Exception &"
                      " the_exception) {\n");
                inc();
                o << indent()
                  << ("throw ::com::sun::star::uno::DeploymentException("
                      "::rtl::OUString( "
                      "\"component context fails to supply service ")
                  << name_ << " of type " << baseName
                  << ": \" ) + the_exception.Message, the_context);\n";
                dec();
                o << indent() << "}\n" << indent()
                  << "if (!the_instance.is()) {\n";
                inc();
                o << indent()
                  << ("throw ::com::sun::star::uno::DeploymentException("
                      "::rtl::OUString( "
                      "\"component context fails to supply service ")
                  << name_ << " of type " << baseName
                  << "\" ), the_context);\n";
                dec();
                o << indent() << "}\n" << indent() << "return the_instance;\n";
                dec();
                o << indent() << "}\n\n";
            } else {
                o << indent() << "static ::com::sun::star::uno::Reference< "
                  << scopedBaseName << " > "
                  << codemaker::cpp::translateUnoToCppIdentifier(
                      u2b(i->name), "method", codemaker::cpp::ITM_NONGLOBAL,
                      &cppName)
                  << ("(::com::sun::star::uno::Reference<"
                      " ::com::sun::star::uno::XComponentContext > const &"
                      " the_context");
                bool rest = hasRestParameter(*i);
                for (std::vector<
                         unoidl::SingleInterfaceBasedServiceEntity::Constructor::
                         Parameter >::const_iterator j(i->parameters.begin());
                     j != i->parameters.end(); ++j)
                {
                    o << ", ";
                    OUStringBuffer buf;
                    if (j->rest) {
                        buf.append("[]");
                    }
                    buf.append(j->type);
                    OUString type(buf.makeStringAndClear());
                    bool byRef = passByReference(type);
                    dumpType(o, type, byRef, byRef);
                    o << " "
                      << codemaker::cpp::translateUnoToCppIdentifier(
                          u2b(j->name), "param", codemaker::cpp::ITM_NONGLOBAL);
                }
                o << ") {\n";
                inc();
                o << indent() << "assert(the_context.is());\n";
                if (!rest && !i->parameters.empty()) {
                    o << indent()
                      << ("::com::sun::star::uno::Sequence<"
                          " ::com::sun::star::uno::Any > the_arguments(")
                      << i->parameters.size() << ");\n";
                    std::vector<
                        unoidl::SingleInterfaceBasedServiceEntity::Constructor::
                        Parameter >::size_type n = 0;
                    for (std::vector<
                             unoidl::SingleInterfaceBasedServiceEntity::
                             Constructor::Parameter >::const_iterator j(
                                 i->parameters.begin());
                         j != i->parameters.end(); ++j)
                    {
                        o << indent() << "the_arguments[" << n++ << "] ";
                        OString param(
                            codemaker::cpp::translateUnoToCppIdentifier(
                                u2b(j->name), "param",
                                codemaker::cpp::ITM_NONGLOBAL));
                        sal_Int32 rank;
                        if (m_typeMgr->getSort(
                                b2u(codemaker::UnoType::decompose(
                                        u2b(j->type), &rank, 0)))
                            == codemaker::UnoType::SORT_CHAR)
                        {
                            o << "= ::com::sun::star::uno::Any(&" << param
                              << ", ::cppu::UnoType< ";
                            for (sal_Int32 k = 0; k < rank; ++k) {
                                o << "::cppu::UnoSequenceType< ";
                            }
                            o << "::cppu::UnoCharType";
                            for (sal_Int32 k = 0; k < rank; ++k) {
                                o << " >";
                            }
                            o << " >::get())";
                        } else {
                            o << "<<= " << param;
                        }
                        o << ";\n";
                    }
                }
                o << indent() << "::com::sun::star::uno::Reference< "
                  << scopedBaseName << " > the_instance;\n";
                codemaker::ExceptionTree tree;
                for (std::vector< OUString >::const_iterator j(
                         i->exceptions.begin());
                     j != i->exceptions.end(); ++j)
                {
                    tree.add(u2b(*j), m_typeMgr);
                }
                if (!tree.getRoot()->present) {
                    o << indent() << "try {\n";
                    inc();
                }
                o << indent()
                  << "the_instance = ::com::sun::star::uno::Reference< "
                  << scopedBaseName
                  << (" >(the_context->getServiceManager()->"
                      "createInstanceWithArgumentsAndContext(::rtl::OUString("
                      " \"")
                  << name_ << "\" ), ";
                if (rest) {
                    o << codemaker::cpp::translateUnoToCppIdentifier(
                        u2b(i->parameters.back().name), "param",
                        codemaker::cpp::ITM_NONGLOBAL);
                } else if (i->parameters.empty()) {
                    o << ("::com::sun::star::uno::Sequence<"
                          " ::com::sun::star::uno::Any >()");
                } else {
                    o << "the_arguments";
                }
                o << ", the_context), ::com::sun::star::uno::UNO_QUERY);\n";
                if (!tree.getRoot()->present) {
                    dec();
                    o << indent()
                      << ("} catch (const ::com::sun::star::uno::RuntimeException &)"
                          " {\n");
                    inc();
                    o << indent() << "throw;\n";
                    dec();
                    dumpCatchClauses(o, tree.getRoot());
                    o << indent()
                      << ("} catch (const ::com::sun::star::uno::Exception &"
                          " the_exception) {\n");
                    inc();
                    o << indent()
                      << ("throw ::com::sun::star::uno::DeploymentException("
                          "::rtl::OUString( "
                          "\"component context fails to supply service ")
                      << name_ << " of type " << baseName
                      << ": \" ) + the_exception.Message, the_context);\n";
                    dec();
                    o << indent() << "}\n";
                }
                o << indent() << "if (!the_instance.is()) {\n";
                inc();
                o << indent()
                  << ("throw ::com::sun::star::uno::DeploymentException("
                      "::rtl::OUString( "
                      "\"component context fails to supply service ")
                  << name_ << " of type " << baseName
                  << "\" ), the_context);\n";
                dec();
                o << indent() << "}\n" << indent() << "return the_instance;\n";
                dec();
                o << indent() << "}\n\n";
            }
        }
    }
    o << "private:\n";
    o << indent() << cppName << "(); // not implemented\n"
      << indent() << cppName << "(" << cppName << " &); // not implemented\n"
      << indent() << "~" << cppName << "(); // not implemented\n"
      << indent() << "void operator =(" << cppName << "); // not implemented\n";
    dec();
    o << "};\n\n";
    if (codemaker::cppumaker::dumpNamespaceClose(o, name_, false)) {
        o << "\n";
    }
    o << "\n#endif // "<< headerDefine << "\n";
}

void ServiceType::dumpCatchClauses(
    FileStream & out, codemaker::ExceptionTreeNode const * node)
{
    if (node->present) {
        out << indent() << "} catch (const ";
        dumpType(out, b2u(node->name));
        out << " &) {\n";
        inc();
        out << indent() << "throw;\n";
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

class SingletonType: public ConstructiveType {
public:
    SingletonType(
        rtl::Reference< unoidl::InterfaceBasedSingletonEntity > const & entity,
        OUString const & name, rtl::Reference< TypeManager > const & manager):
        ConstructiveType(name, manager), entity_(entity)
    { assert(entity.is()); }

private:
    virtual void dumpHxxFile(
        FileStream & o, codemaker::cppumaker::Includes & includes);

    rtl::Reference< unoidl::InterfaceBasedSingletonEntity > entity_;
};

void SingletonType::dumpHxxFile(
    FileStream & o, codemaker::cppumaker::Includes & includes)
{
    OString cppName(
        codemaker::cpp::translateUnoToCppIdentifier(
            u2b(id_), "singleton", isGlobal()));
    OString baseName(u2b(entity_->getBase()));
    OString scopedBaseName(codemaker::cpp::scopedCppName(baseName));
    OUString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << "\n";
    //TODO: Decide whether the types added to includes should rather be added to
    // m_dependencies (and thus be generated during dumpDependedTypes):
    includes.add("com.sun.star.uno.DeploymentException");
    includes.add("com.sun.star.uno.XComponentContext");
    includes.addCassert();
    includes.addAny();
    includes.addReference();
    includes.addRtlUstringH();
    includes.addRtlUstringHxx();
    includes.dump(o, 0);
    o << "\n";
    if (codemaker::cppumaker::dumpNamespaceOpen(o, name_, false)) {
        o << "\n";
    }
    o << "\nclass " << cppName << " {\npublic:\n";
    inc();
    o << indent() << "static ::com::sun::star::uno::Reference< "
      << scopedBaseName << " > "
      << codemaker::cpp::translateUnoToCppIdentifier(
          "get", "method", codemaker::cpp::ITM_NONGLOBAL, &cppName)
      << ("(::com::sun::star::uno::Reference<"
          " ::com::sun::star::uno::XComponentContext > const & the_context)"
          " {\n");
    inc();
    o << indent() << "assert(the_context.is());\n" << indent()
      << "::com::sun::star::uno::Reference< " << scopedBaseName
      << " > instance;\n" << indent()
      << ("if (!(the_context->getValueByName("
          "::rtl::OUString( \"/singletons/")
      << name_ << "\" )) >>= instance) || !instance.is()) {\n";
    inc();
    o << indent()
      << ("throw ::com::sun::star::uno::DeploymentException("
          "::rtl::OUString( \"component context"
          " fails to supply singleton ")
      << name_ << " of type " << baseName << "\" ), the_context);\n";
    dec();
    o << indent() << "}\n" << indent() << "return instance;\n";
    dec();
    o << indent() << "}\n\n";
    o << "private:\n";
    o << indent() << cppName << "(); // not implemented\n"
      << indent() << cppName << "(" << cppName << " &); // not implemented\n"
      << indent() << "~" << cppName << "(); // not implemented\n"
      << indent() << "void operator =(" << cppName << "); // not implemented\n";
    dec();
    o << "};\n\n";
    if (codemaker::cppumaker::dumpNamespaceClose(o, name_, false)) {
        o << "\n";
    }
    o << "\n#endif // "<< headerDefine << "\n";
}

}

void produce(
    OUString const & name, rtl::Reference< TypeManager > const & manager,
    codemaker::GeneratedTypeSet & generated, CppuOptions const & options)
{
    if (generated.contains(u2b(name))) {
        return;
    }
    generated.add(u2b(name));
    if (!manager->foundAtPrimaryProvider(name)) {
        return;
    }
    rtl::Reference< unoidl::Entity > ent;
    rtl::Reference< unoidl::MapCursor > cur;
    switch (manager->getSort(name, &ent, &cur)) {
    case codemaker::UnoType::SORT_MODULE:
        {
            OUString prefix;
            if (!name.isEmpty()) {
                prefix = name + ".";
            }
            for (;;) {
                OUString mem;
                if (!cur->getNext(&mem).is()) {
                    break;
                }
                produce(prefix + mem, manager, generated, options);
            }
            break;
        }
    case codemaker::UnoType::SORT_ENUM_TYPE:
        {
            EnumType t(
                dynamic_cast< unoidl::EnumTypeEntity * >(ent.get()), name,
                manager);
            t.dump(options);
            t.dumpDependedTypes(generated, options);
            break;
        }
    case codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE:
        {
            PlainStructType t(
                dynamic_cast< unoidl::PlainStructTypeEntity * >(ent.get()),
                name, manager);
            t.dump(options);
            t.dumpDependedTypes(generated, options);
            break;
        }
    case codemaker::UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
        {
            PolyStructType t(
                dynamic_cast< unoidl::PolymorphicStructTypeTemplateEntity * >(
                    ent.get()),
                name, manager);
            t.dump(options);
            t.dumpDependedTypes(generated, options);
            break;
        }
    case codemaker::UnoType::SORT_EXCEPTION_TYPE:
        {
            ExceptionType t(
                dynamic_cast< unoidl::ExceptionTypeEntity * >(ent.get()), name,
                manager);
            t.dump(options);
            t.dumpDependedTypes(generated, options);
            break;
        }
    case codemaker::UnoType::SORT_INTERFACE_TYPE:
        {
            InterfaceType t(
                dynamic_cast< unoidl::InterfaceTypeEntity * >(ent.get()), name,
                manager);
            t.dump(options);
            t.dumpDependedTypes(generated, options);
            break;
        }
    case codemaker::UnoType::SORT_TYPEDEF:
        {
            Typedef t(
                dynamic_cast< unoidl::TypedefEntity * >(ent.get()), name,
                manager);
            t.dump(options);
            t.dumpDependedTypes(generated, options);
            break;
        }
    case codemaker::UnoType::SORT_CONSTANT_GROUP:
        {
            ConstantGroup t(
                dynamic_cast< unoidl::ConstantGroupEntity * >(ent.get()), name,
                manager);
            if (t.hasConstants()) {
                t.dump(options);
            }
            break;
        }
    case codemaker::UnoType::SORT_SINGLE_INTERFACE_BASED_SERVICE:
        {
            ServiceType t(
                dynamic_cast< unoidl::SingleInterfaceBasedServiceEntity * >(
                    ent.get()),
                name, manager);
            t.dump(options);
            t.dumpDependedTypes(generated, options);
            break;
        }
    case codemaker::UnoType::SORT_INTERFACE_BASED_SINGLETON:
        {
            SingletonType t(
                dynamic_cast< unoidl::InterfaceBasedSingletonEntity * >(
                    ent.get()),
                name, manager);
            t.dump(options);
            t.dumpDependedTypes(generated, options);
            break;
        }
    case codemaker::UnoType::SORT_ACCUMULATION_BASED_SERVICE:
    case codemaker::UnoType::SORT_SERVICE_BASED_SINGLETON:
        break;
    default:
        throw CannotDumpException(
            "unexpected entity \"" + name + "\" in call to produce");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
