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

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"
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
    bool VisitFunctionDecl(const FunctionDecl *);

    // Creation of temporaries with one argument are represented by
    // CXXFunctionalCastExpr, while any other number of arguments are
    // represented by CXXTemporaryObjectExpr:
    bool VisitCXXTemporaryObjectExpr(CXXTemporaryObjectExpr const * expr)
    { return visitTemporaryObjectExpr(expr); }
    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr const * expr)
    { return visitTemporaryObjectExpr(expr); }

    bool WalkUpFromObjCIvarDecl(ObjCIvarDecl * decl) {
        // Don't recurse into WalkUpFromFieldDecl, as VisitFieldDecl calls
        // FieldDecl::getParent, which triggers an assertion at least with
        // current trunk towards Clang 3.7 when the FieldDecl is actually an
        // ObjCIvarDecl.
        return VisitObjCIvarDecl(decl);
    }
private:
    void checkUnoReference(QualType qt, const Decl* decl,
                           const std::string& rParentName, const std::string& rDeclName);

    bool visitTemporaryObjectExpr(Expr const * expr);
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
    if (!compat::forallBases(
            *decl,
#if CLANG_VERSION < 30800
            BaseCheckNotSubclass,
#else
            [pString](const CXXRecordDecl *BaseDefinition) -> bool
                { return BaseCheckNotSubclass(
                        BaseDefinition, const_cast<char *>(pString)); },
#endif
            static_cast<void*>(const_cast<char*>(pString)), true))
    {
        return true;
    }
    return false;
}


bool containsXInterfaceSubclass(const Type* pType0);

bool containsXInterfaceSubclass(const QualType& qType) {
    return containsXInterfaceSubclass(qType.getTypePtr());
}

bool containsXInterfaceSubclass(const Type* pType0) {
    if (!pType0)
        return false;
    const Type* pType = pType0->getUnqualifiedDesugaredType();
    if (!pType)
        return false;
    const CXXRecordDecl* pRecordDecl = pType->getAsCXXRecordDecl();
    if (pRecordDecl) {
        pRecordDecl = pRecordDecl->getCanonicalDecl();
        // these classes override acquire/release and forwards to its parent
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
            // Probably good templates:
            loplugin::DeclCheck dc(pTemplate);
            if ((dc.Struct("FindUnoInstanceHint").AnonymousNamespace()
                 .GlobalNamespace())
                || (dc.Class("OMultiInstanceAutoRegistration").Namespace("abp")
                    .GlobalNamespace())
                || (dc.Class("Reference").Namespace("uno").Namespace("star")
                    .Namespace("sun").Namespace("com").GlobalNamespace())
                || (dc.Class("WeakReference").Namespace("uno").Namespace("star")
                    .Namespace("sun").Namespace("com").GlobalNamespace())
                || (dc.Class("Sequence").Namespace("uno").Namespace("star")
                    .Namespace("sun").Namespace("com").GlobalNamespace())
                || (dc.Class("WeakCppRef").Namespace("accessibility")
                    .GlobalNamespace())
                || (dc.Class("OAutoRegistration").Namespace("dba")
                    .GlobalNamespace())
                || (dc.Class("OMultiInstanceAutoRegistration").Namespace("dbp")
                    .GlobalNamespace())
                || (dc.Class("OMultiInstanceAutoRegistration")
                    .Namespace("dbaui").GlobalNamespace())
                || (dc.Class("OMultiInstanceAutoRegistration")
                    .Namespace("dbaxml").GlobalNamespace())
                || (dc.Struct("ReferenceEqual").Namespace("io_acceptor")
                    .GlobalNamespace())
                || (dc.Struct("ReferenceHash").Namespace("io_acceptor")
                    .GlobalNamespace())
                || (dc.Class("OAutoRegistration").Namespace("comphelper")
                    .GlobalNamespace())
                || (dc.Struct("OInterfaceCompare").Namespace("comphelper")
                    .GlobalNamespace())
                || dc.Class("WeakBag").Namespace("comphelper").GlobalNamespace()
                || (dc.Struct("class_").Namespace("service_decl")
                    .Namespace("comphelper").GlobalNamespace())
                || (dc.Struct("vba_service_class_").Namespace("service_decl")
                    .Namespace("comphelper").GlobalNamespace())
                || (dc.Struct("inheritingClass_").Namespace("service_decl")
                    .Namespace("comphelper").GlobalNamespace())
                || (dc.Class("OAutoRegistration").Namespace("module")
                    .Namespace("comphelper").GlobalNamespace())
                || (dc.Class("mem_fun1_t").Namespace("comphelper")
                    .GlobalNamespace())
                || (dc.Class("OSimpleListenerContainer").Namespace("comphelper")
                    .GlobalNamespace())
                || (dc.Class("OAutoRegistration").Namespace("dbmm")
                    .GlobalNamespace())
                || (dc.Class("OAutoRegistration").Namespace("pcr")
                    .GlobalNamespace())
                || (dc.Class("ComponentMethodGuard").Namespace("logging")
                    .GlobalNamespace())
                || (dc.Class("OAutoRegistration").Namespace("logging")
                    .GlobalNamespace())
                || dc.Class("Reference").Namespace("rtl").GlobalNamespace()
                || (dc.Class("OAutoRegistration").Namespace("sdbtools")
                    .GlobalNamespace())
                || (dc.Struct("ReferenceEqual").Namespace("stoc_connector")
                    .GlobalNamespace())
                || (dc.Struct("ReferenceHash").Namespace("stoc_connector")
                    .GlobalNamespace())
                || dc.Class("mem_fun_t").StdNamespace()
                || dc.Class("mem_fun1_t").StdNamespace()
                || dc.Class("SwIterator").GlobalNamespace()
                || (dc.Class("SharedUNOComponent").Namespace("utl")
                    .GlobalNamespace())
                || (dc.Class("OAutoRegistration").Namespace("utl")
                    .GlobalNamespace())
                || (dc.Class("DeleteUnoReferenceOnDeinit").Namespace("vcl")
                    .GlobalNamespace())
                || (dc.Struct("OInterfaceCompare").Namespace("xmloff")
                    .GlobalNamespace()))
            {
                return false;
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

bool containsSvRefBaseSubclass(const Type* pType0) {
    if (!pType0)
        return false;
    const Type* pType = pType0->getUnqualifiedDesugaredType();
    if (!pType)
        return false;
    const CXXRecordDecl* pRecordDecl = pType->getAsCXXRecordDecl();
    if (pRecordDecl) {
        pRecordDecl = pRecordDecl->getCanonicalDecl();
    }
    if (pRecordDecl) {
        const ClassTemplateSpecializationDecl* pTemplate = dyn_cast<ClassTemplateSpecializationDecl>(pRecordDecl);
        if (pTemplate) {
            std::string aName = pTemplate->getQualifiedNameAsString();
            if (aName == "tools::SvRef")
                return false;
            for(unsigned i=0; i<pTemplate->getTemplateArgs().size(); ++i) {
                const TemplateArgument& rArg = pTemplate->getTemplateArgs()[i];
                if (rArg.getKind() == TemplateArgument::ArgKind::Type &&
                    containsSvRefBaseSubclass(rArg.getAsType().getTypePtr()))
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
        return containsSvRefBaseSubclass(elementType.getTypePtr());
    } else {
        return isDerivedFrom(pRecordDecl, "tools::SvRefBase");
    }
}

bool containsSalhelperReferenceObjectSubclass(const Type* pType0) {
    if (!pType0)
        return false;
    const Type* pType = pType0->getUnqualifiedDesugaredType();
    if (!pType)
        return false;
    const CXXRecordDecl* pRecordDecl = pType->getAsCXXRecordDecl();
    if (pRecordDecl) {
        pRecordDecl = pRecordDecl->getCanonicalDecl();
    }
    if (pRecordDecl) {
        const ClassTemplateSpecializationDecl* pTemplate = dyn_cast<ClassTemplateSpecializationDecl>(pRecordDecl);
        if (pTemplate) {
            std::string aName = pTemplate->getQualifiedNameAsString();
            if (aName == "rtl::Reference" || aName == "store::OStoreHandle")
                return false;
            for(unsigned i=0; i<pTemplate->getTemplateArgs().size(); ++i) {
                const TemplateArgument& rArg = pTemplate->getTemplateArgs()[i];
                if (rArg.getKind() == TemplateArgument::ArgKind::Type &&
                    containsSalhelperReferenceObjectSubclass(rArg.getAsType().getTypePtr()))
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
        return containsSalhelperReferenceObjectSubclass(elementType.getTypePtr());
    } else {
        return isDerivedFrom(pRecordDecl, "salhelper::SimpleReferenceObject");
    }
}

static bool containsStaticTypeMethod(const CXXRecordDecl* x)
{
    for (auto it = x->method_begin(); it != x->method_end(); ++it) {
        auto i = *it;
        if ( !i->isStatic() )
            continue;
        auto ident = i->getIdentifier();
        if ( ident && ident->isStr("static_type") ) {
            return true;
        }
    }
    return false;
}

void RefCounting::checkUnoReference(QualType qt, const Decl* decl, const std::string& rParentName, const std::string& rDeclName)
{
    if (loplugin::TypeCheck(qt).Class("Reference").Namespace("uno").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace()) {
        const CXXRecordDecl* pRecordDecl = qt->getAsCXXRecordDecl();
        const ClassTemplateSpecializationDecl* pTemplate = dyn_cast<ClassTemplateSpecializationDecl>(pRecordDecl);
        const TemplateArgument& rArg = pTemplate->getTemplateArgs()[0];
        const CXXRecordDecl* templateParam = rArg.getAsType()->getAsCXXRecordDecl()->getDefinition();
        if (templateParam && !containsStaticTypeMethod(templateParam)) {
            report(
                DiagnosticsEngine::Warning,
                "uno::Reference " + rDeclName + " with template parameter that does not contain ::static_type() "
                + qt.getAsString()
                + ", parent is " + rParentName
                + ", should probably be using rtl::Reference instead",
                decl->getLocation())
              << decl->getSourceRange();
        }
    }
}

bool RefCounting::visitTemporaryObjectExpr(Expr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto t = expr->getType();
    if (containsSvRefBaseSubclass(t.getTypePtr())) {
        report(
            DiagnosticsEngine::Warning,
            ("Temporary object of SvRefBase subclass %0 being directly stack"
             " managed, should be managed via tools::SvRef"),
            expr->getLocStart())
            << t.getUnqualifiedType() << expr->getSourceRange();
    } else if (containsSalhelperReferenceObjectSubclass(t.getTypePtr())) {
        report(
            DiagnosticsEngine::Warning,
            ("Temporary object of salhelper::SimpleReferenceObject subclass %0"
             " being directly stack managed, should be managed via"
             " rtl::Reference"),
            expr->getLocStart())
            << t.getUnqualifiedType() << expr->getSourceRange();
    } else if (containsXInterfaceSubclass(t)) {
        report(
            DiagnosticsEngine::Warning,
            ("Temporary object of css::uno::XInterface subclass %0 being"
             " directly stack managed, should be managed via"
             " css::uno::Reference"),
            expr->getLocStart())
            << t.getUnqualifiedType() << expr->getSourceRange();
    }
    return true;
}

bool RefCounting::VisitFieldDecl(const FieldDecl * fieldDecl) {
    if (ignoreLocation(fieldDecl)) {
        return true;
    }
    if (fieldDecl->isBitField()) {
        return true;
    }

    std::string aParentName = fieldDecl->getParent()->getQualifiedNameAsString();

    if (containsSvRefBaseSubclass(fieldDecl->getType().getTypePtr())) {
        report(
            DiagnosticsEngine::Warning,
            "SvRefBase subclass being directly heap managed, should be managed via tools::SvRef, "
            + fieldDecl->getType().getAsString()
            + ", parent is " + aParentName,
            fieldDecl->getLocation())
          << fieldDecl->getSourceRange();
    }

    if (containsSalhelperReferenceObjectSubclass(fieldDecl->getType().getTypePtr())) {
        report(
            DiagnosticsEngine::Warning,
            "salhelper::SimpleReferenceObject subclass being directly heap managed, should be managed via rtl::Reference, "
            + fieldDecl->getType().getAsString()
            + ", parent is " + aParentName,
            fieldDecl->getLocation())
          << fieldDecl->getSourceRange();
    }

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
    }

    checkUnoReference(fieldDecl->getType(), fieldDecl, aParentName, "field");

    return true;
}


bool RefCounting::VisitVarDecl(const VarDecl * varDecl) {
    if (ignoreLocation(varDecl)) {
        return true;
    }
    if (!isa<ParmVarDecl>(varDecl)) {
        if (containsSvRefBaseSubclass(varDecl->getType().getTypePtr())) {
            report(
                DiagnosticsEngine::Warning,
                "SvRefBase subclass being directly stack managed, should be managed via tools::SvRef, "
                + varDecl->getType().getAsString(),
                varDecl->getLocation())
              << varDecl->getSourceRange();
        }
        if (containsSalhelperReferenceObjectSubclass(varDecl->getType().getTypePtr())) {
            StringRef name { compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(varDecl->getLocation())) };
            // this is playing games that it believes is safe
            if (loplugin::isSamePathname(name, SRCDIR "/stoc/source/security/permissions.cxx"))
                return true;
            report(
                DiagnosticsEngine::Warning,
                "salhelper::SimpleReferenceObject subclass being directly stack managed, should be managed via rtl::Reference, "
                + varDecl->getType().getAsString(),
                varDecl->getLocation())
              << varDecl->getSourceRange();
        }
        if (containsXInterfaceSubclass(varDecl->getType())) {
            report(
                DiagnosticsEngine::Warning,
                "XInterface subclass being directly stack managed, should be managed via uno::Reference, "
                + varDecl->getType().getAsString(),
                varDecl->getLocation())
              << varDecl->getSourceRange();
        }
    }
    checkUnoReference(varDecl->getType(), varDecl, "", "var");
    return true;
}

bool RefCounting::VisitFunctionDecl(const FunctionDecl * functionDecl) {
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    // only consider base declarations, not overridden ones, or we warn on methods that
    // are overriding stuff from external libraries
    const CXXMethodDecl * methodDecl = dyn_cast<CXXMethodDecl>(functionDecl);
    if (methodDecl && methodDecl->size_overridden_methods() > 0) {
            return true;
    }
    checkUnoReference(compat::getReturnType(*functionDecl), functionDecl, "", "return");
    return true;
}

loplugin::Plugin::Registration< RefCounting > X("refcounting");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
