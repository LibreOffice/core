/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <iostream>

#include "plugin.hxx"
#include "compat.hxx"
#include "clang/AST/CXXInheritance.h"

/**

If you have:

    class Foo : public css::foo::XBaa {
    };

Then XBaa has acquire and release methods inherited from XInterface.
These are hard lifecycle controls.

If you see another class:

    class Baz {
        Foo aFooMember;
    };

this is a bug =) since aFooMember assumes heap allocated lifecycle and
not delete on last 'release'.

TODO check that things that extend SvRefBase are managed by SvRef
TODO fix the SvXMLImportContext class (mentioned below)
TODO fix the slideshow::internal::SlideView class (mentioned below)
*/

namespace {

class RefCounting:
    public RecursiveASTVisitor<RefCounting>, public loplugin::Plugin
{
public:
    explicit RefCounting(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFieldDecl(const FieldDecl *);
    bool VisitVarDecl(const VarDecl *);
};

bool BaseCheckNotSubclass(const CXXRecordDecl *BaseDefinition, void *p) {
    if (!BaseDefinition)
        return true;
    const char *pString = static_cast<const char *>(p);
    if (BaseDefinition->getQualifiedNameAsString() == pString) {
        return false;
    }
    return true;
}

bool isDerivedFrom(const CXXRecordDecl *decl, const char *pString) {
    if (!decl)
        return false;
    if (decl->getQualifiedNameAsString() == pString)
        return true;
    if (!decl->hasDefinition()) {
        return false;
    }
    if (// not sure what hasAnyDependentBases() does,
        // but it avoids classes we don't want, e.g. WeakAggComponentImplHelper1
        !decl->hasAnyDependentBases() &&
        !decl->forallBases(BaseCheckNotSubclass, static_cast<void*>(const_cast<char*>(pString)), true)) {
        return true;
    }
    return false;
}


bool containsXInterfaceSubclass(const Type* pType0);

bool containsXInterfaceSubclass(const QualType& qType) {
    return containsXInterfaceSubclass(qType.getTypePtr());
}

static std::vector<std::string> PROBABLY_GOOD_TEMPLATES = {
    "(anonymous namespace)::FindUnoInstanceHint",
    "abp::OMultiInstanceAutoRegistration",
    "com::sun::star::uno::Reference",
    "com::sun::star::uno::WeakReference",
    "com::sun::star::uno::Sequence",
    "accessibility::HardCppRef",
    "accessibility::WeakCppRef",
    "dba::OAutoRegistration",
    "dba::OSingletonRegistration",
    "dbp::OMultiInstanceAutoRegistration",
    "dbaui::OMultiInstanceAutoRegistration",
    "dbaxml::OMultiInstanceAutoRegistration",
    "io_acceptor::ReferenceEqual",
    "io_acceptor::ReferenceHash",
    "comphelper::OAutoRegistration",
    "comphelper::OInterfaceCompare",
    "comphelper::module::OSingletonRegistration",
    "comphelper::WeakBag",
    "comphelper::service_decl::class_",
    "comphelper::service_decl::vba_service_class_",
    "comphelper::service_decl::inheritingClass_",
    "comphelper::module::OAutoRegistration",
    "comphelper::mem_fun1_t",
    "comphelper::OSimpleListenerContainer",
    "dbmm::OAutoRegistration",
    "pcr::OAutoRegistration",
    "logging::ComponentMethodGuard",
    "logging::OSingletonRegistration",
    "logging::OAutoRegistration",
    "rtl::Reference",
    "sdbtools::OAutoRegistration",
    "stoc_connector::ReferenceEqual",
    "stoc_connector::ReferenceHash",
    "std::mem_fun_t",
    "std::mem_fun1_t",
    "SwIterator",
    "toolkit::InitGuard",
    "utl::SharedUNOComponent",
    "utl::OAutoRegistration",
    "vcl::DeleteUnoReferenceOnDeinit",
    "xmloff::OInterfaceCompare",
};

bool containsXInterfaceSubclass(const Type* pType0) {
    if (!pType0)
        return false;
    const Type* pType = pType0->getUnqualifiedDesugaredType();
    if (!pType)
        return false;
    const CXXRecordDecl* pRecordDecl = pType->getAsCXXRecordDecl();
    if (pRecordDecl) {
        pRecordDecl = pRecordDecl->getCanonicalDecl();
        // these classes override acquire/release and forwards to it's parent
        if (isDerivedFrom(pRecordDecl, "ListenerMultiplexerBase")) { // module UnoTools
            return false;
        }
        if (isDerivedFrom(pRecordDecl, "toolkit::GridEventForwarder")) { // module toolkit
            return false;
        }
        if (isDerivedFrom(pRecordDecl, "OWeakSubObject")) { // module svx
            return false;
        }
        if (isDerivedFrom(pRecordDecl, "dbaui::OSbaWeakSubObject")) { // module dbaccess
            return false;
        }
        // FIXME this class extends 2 different ref-counting bases, SvRefBase and XInterface (via. cppu::WeakImplHelper)
        // I have no idea how to fix it
        if (isDerivedFrom(pRecordDecl, "SvXMLImportContext")) { // module xmloff
            return false;
        }
        // The actual problem child is SlideView, of which this is the parent.
        // Everything in the hierarchy above this wants to be managed via boost::shared_ptr
        if (isDerivedFrom(pRecordDecl, "slideshow::internal::UnoView")) { // module slideshow
            return false;
        }
        // FIXME This class has private operator new, and I cannot figure out how it can be dynamically instantiated
        if (isDerivedFrom(pRecordDecl, "XPropertyList")) { // module svx
            return false;
        }
    }
    if (pRecordDecl) {
        const ClassTemplateSpecializationDecl* pTemplate = dyn_cast<ClassTemplateSpecializationDecl>(pRecordDecl);
        if (pTemplate) {
            std::string aName = pTemplate->getQualifiedNameAsString();
            if (std::find(PROBABLY_GOOD_TEMPLATES.begin(), PROBABLY_GOOD_TEMPLATES.end(), aName) != PROBABLY_GOOD_TEMPLATES.end())
                return false;
            for(unsigned i=0; i<pTemplate->getTemplateArgs().size(); ++i) {
                const TemplateArgument& rArg = pTemplate->getTemplateArgs()[i];
                if (rArg.getKind() == TemplateArgument::ArgKind::Type &&
                    containsXInterfaceSubclass(rArg.getAsType()))
                {
                    return true;
                }
            }
        }
    }
    if (pType->isPointerType()) {
        // ignore
        return false;
    } else if (pType->isArrayType()) {
        const ArrayType* pArrayType = dyn_cast<ArrayType>(pType);
        QualType elementType = pArrayType->getElementType();
        return containsXInterfaceSubclass(elementType);
    } else {
        return isDerivedFrom(pRecordDecl, "com::sun::star::uno::XInterface");
    }
}



bool RefCounting::VisitFieldDecl(const FieldDecl * fieldDecl) {
    if (ignoreLocation(fieldDecl)) {
        return true;
    }
    if (fieldDecl->isBitField()) {
        return true;
    }
    std::string aParentName = fieldDecl->getParent()->getQualifiedNameAsString();
    if ( aParentName == "com::sun::star::uno::BaseReference"
         || aParentName == "cppu::detail::element_alias"
         // this is playing some kind of game to avoid circular references
         || aParentName == "ucbhelper::ResultSetDataSupplier")
    {
        return true;
    }

    if (containsXInterfaceSubclass(fieldDecl->getType())) {
        report(
            DiagnosticsEngine::Warning,
            "XInterface subclass being directly heap managed, should be managed via uno::Reference, "
            + fieldDecl->getType().getAsString()
            + ", parent is " + aParentName,
            fieldDecl->getLocation())
          << fieldDecl->getSourceRange();
        return true;
    }
    return true;
}


bool RefCounting::VisitVarDecl(const VarDecl * varDecl) {
    if (ignoreLocation(varDecl)) {
        return true;
    }
    if (containsXInterfaceSubclass(varDecl->getType())) {
        report(
            DiagnosticsEngine::Warning,
            "XInterface subclass being directly stack managed, should be managed via uno::Reference, "
            + varDecl->getType().getAsString(),
            varDecl->getLocation())
          << varDecl->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration< RefCounting > X("refcounting");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
