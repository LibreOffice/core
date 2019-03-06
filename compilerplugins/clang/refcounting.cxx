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

*/

namespace loplugin {

class RefCounting:
    public loplugin::FilteringPlugin<RefCounting>
{
public:
    explicit RefCounting(loplugin::InstantiationData const & data): FilteringPlugin(data)
    {}

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
                           const RecordDecl* parent, const std::string& rDeclName);

    bool visitTemporaryObjectExpr(Expr const * expr);
};

bool containsXInterfaceSubclass(const clang::Type* pType0);

bool containsXInterfaceSubclass(const QualType& qType) {
    return containsXInterfaceSubclass(qType.getTypePtr());
}

bool containsXInterfaceSubclass(const clang::Type* pType0) {
    if (!pType0)
        return false;
    const clang::Type* pType = pType0->getUnqualifiedDesugaredType();
    if (!pType)
        return false;
    const CXXRecordDecl* pRecordDecl = pType->getAsCXXRecordDecl();
    if (pRecordDecl) {
        pRecordDecl = pRecordDecl->getCanonicalDecl();
        // these classes override acquire/release and forwards to its parent
        if (isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("ListenerMultiplexerBase").GlobalNamespace()); })) { // module UnoTools
            return false;
        }
        if (isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("GridEventForwarder").Namespace("toolkit").GlobalNamespace()); })) { // module toolkit
            return false;
        }
        if (isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("OWeakSubObject").GlobalNamespace()); })) { // module svx
            return false;
        }
        if (isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("OSbaWeakSubObject").Namespace("dbaui").GlobalNamespace()); })) { // module dbaccess
            return false;
        }
        // FIXME This class has private operator new, and I cannot figure out how it can be dynamically instantiated
        if (isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("XPropertyList").GlobalNamespace()); })) { // module svx
            return false;
        }
        // tdf#114596
        if (isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("OBookmarkContainer").Namespace("dbaccess").GlobalNamespace()); })) { // module dbaccess
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
        const clang::ArrayType* pArrayType = dyn_cast<clang::ArrayType>(pType);
        QualType elementType = pArrayType->getElementType();
        return containsXInterfaceSubclass(elementType);
    } else {
        return isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("XInterface").Namespace("uno").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace()); });
    }
}

bool containsSvRefBaseSubclass(const clang::Type* pType0) {
    if (!pType0)
        return false;
    const clang::Type* pType = pType0->getUnqualifiedDesugaredType();
    if (!pType)
        return false;
    const CXXRecordDecl* pRecordDecl = pType->getAsCXXRecordDecl();
    if (pRecordDecl) {
        pRecordDecl = pRecordDecl->getCanonicalDecl();
    }
    if (pRecordDecl) {
        const ClassTemplateSpecializationDecl* pTemplate = dyn_cast<ClassTemplateSpecializationDecl>(pRecordDecl);
        if (pTemplate) {
            if (loplugin::DeclCheck(pTemplate).Class("SvRef")
                .Namespace("tools").GlobalNamespace())
            {
                return false;
            }
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
        const clang::ArrayType* pArrayType = dyn_cast<clang::ArrayType>(pType);
        QualType elementType = pArrayType->getElementType();
        return containsSvRefBaseSubclass(elementType.getTypePtr());
    } else {
        return isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("SvRefBase").Namespace("tools").GlobalNamespace()); });
    }
}

bool containsSalhelperReferenceObjectSubclass(const clang::Type* pType0) {
    if (!pType0)
        return false;
    const clang::Type* pType = pType0->getUnqualifiedDesugaredType();
    if (!pType)
        return false;
    const CXXRecordDecl* pRecordDecl = pType->getAsCXXRecordDecl();
    if (pRecordDecl) {
        pRecordDecl = pRecordDecl->getCanonicalDecl();
    }
    if (pRecordDecl) {
        const ClassTemplateSpecializationDecl* pTemplate = dyn_cast<ClassTemplateSpecializationDecl>(pRecordDecl);
        if (pTemplate) {
            auto const dc = loplugin::DeclCheck(pTemplate);
            if (dc.Class("Reference").Namespace("rtl").GlobalNamespace()
                || (dc.Class("OStoreHandle").Namespace("store")
                    .GlobalNamespace()))
            {
                return false;
            }
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
        const clang::ArrayType* pArrayType = dyn_cast<clang::ArrayType>(pType);
        QualType elementType = pArrayType->getElementType();
        return containsSalhelperReferenceObjectSubclass(elementType.getTypePtr());
    } else {
        return isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("SimpleReferenceObject").Namespace("salhelper").GlobalNamespace()); });
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

void RefCounting::checkUnoReference(QualType qt, const Decl* decl, const RecordDecl* parent, const std::string& rDeclName)
{
    if (loplugin::TypeCheck(qt).Class("Reference").Namespace("uno").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace()) {
        const CXXRecordDecl* pRecordDecl = qt->getAsCXXRecordDecl();
        const ClassTemplateSpecializationDecl* pTemplate = dyn_cast<ClassTemplateSpecializationDecl>(pRecordDecl);
        const TemplateArgument& rArg = pTemplate->getTemplateArgs()[0];
        const CXXRecordDecl* templateParam = rArg.getAsType()->getAsCXXRecordDecl()->getDefinition();
        if (templateParam && !containsStaticTypeMethod(templateParam)) {
            report(
                DiagnosticsEngine::Warning,
                ("uno::Reference %0 with template parameter that does not"
                 " contain ::static_type() %1%select{|, parent is %3,}2 should"
                 " probably be using rtl::Reference instead"),
                decl->getLocation())
              << rDeclName << qt << (parent != nullptr)
              << (parent != nullptr
                  ? parent->getQualifiedNameAsString() : std::string())
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
            compat::getBeginLoc(expr))
            << t.getUnqualifiedType() << expr->getSourceRange();
    } else if (containsSalhelperReferenceObjectSubclass(t.getTypePtr())) {
        report(
            DiagnosticsEngine::Warning,
            ("Temporary object of salhelper::SimpleReferenceObject subclass %0"
             " being directly stack managed, should be managed via"
             " rtl::Reference"),
            compat::getBeginLoc(expr))
            << t.getUnqualifiedType() << expr->getSourceRange();
    } else if (containsXInterfaceSubclass(t)) {
        report(
            DiagnosticsEngine::Warning,
            ("Temporary object of css::uno::XInterface subclass %0 being"
             " directly stack managed, should be managed via"
             " css::uno::Reference"),
            compat::getBeginLoc(expr))
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

    // check for dodgy code managing ref-counted stuff with shared_ptr or unique_ptr or similar stuff
    QualType firstTemplateParamType;
    if (auto recordType = fieldDecl->getType()->getUnqualifiedDesugaredType()->getAs<RecordType>()) {
        auto const tc = loplugin::TypeCheck(fieldDecl->getType());
        if (tc.Class("unique_ptr").StdNamespace()
            || tc.Class("shared_ptr").StdNamespace()
            || tc.Class("intrusive_ptr").Namespace("boost").GlobalNamespace())
        {
            auto templateDecl = dyn_cast<ClassTemplateSpecializationDecl>(recordType->getDecl());
            if (templateDecl && templateDecl->getTemplateArgs().size() > 0)
                firstTemplateParamType = templateDecl->getTemplateArgs()[0].getAsType();
        }
    }

    if (containsSvRefBaseSubclass(fieldDecl->getType().getTypePtr())) {
        report(
            DiagnosticsEngine::Warning,
            "SvRefBase subclass %0 being directly heap managed, should be managed via tools::SvRef, "
            ", parent is %1",
            fieldDecl->getLocation())
            << fieldDecl->getType()
            << fieldDecl->getParent()
            << fieldDecl->getSourceRange();
    }

    if (!firstTemplateParamType.isNull() && containsSvRefBaseSubclass(firstTemplateParamType.getTypePtr()))
    {
        report(
            DiagnosticsEngine::Warning,
            "SvRefBase subclass %0 being managed via smart pointer, should be managed via tools::SvRef, "
            "parent is %1",
            fieldDecl->getLocation())
            << firstTemplateParamType
            << fieldDecl->getParent()
            << fieldDecl->getSourceRange();
    }

    if (containsSalhelperReferenceObjectSubclass(fieldDecl->getType().getTypePtr())) {
        report(
            DiagnosticsEngine::Warning,
            "salhelper::SimpleReferenceObject subclass %0 being directly heap managed, should be managed via rtl::Reference, "
            "parent is %1",
            fieldDecl->getLocation())
            << fieldDecl->getType()
            << fieldDecl->getParent()
            << fieldDecl->getSourceRange();
    }

    if (!firstTemplateParamType.isNull() && containsSalhelperReferenceObjectSubclass(firstTemplateParamType.getTypePtr()))
    {
        report(
            DiagnosticsEngine::Warning,
            "salhelper::SimpleReferenceObject subclass %0 being managed via smart pointer, should be managed via rtl::Reference, "
            "parent is %1",
            fieldDecl->getLocation())
            << firstTemplateParamType
            << fieldDecl->getParent()
            << fieldDecl->getSourceRange();
    }

    auto const dc = loplugin::DeclCheck(fieldDecl->getParent());
    if ( (dc.Class("BaseReference").Namespace("uno").Namespace("star")
          .Namespace("sun").Namespace("com").GlobalNamespace())
         || (dc.Union("element_alias").Namespace("detail").Namespace("cppu")
             .GlobalNamespace())
         // this is playing some kind of game to avoid circular references
         || (dc.Class("ResultSetDataSupplier").Namespace("ucbhelper")
             .GlobalNamespace()))
    {
        return true;
    }

    if (containsXInterfaceSubclass(fieldDecl->getType())) {
        report(
            DiagnosticsEngine::Warning,
            "XInterface subclass %0 being directly heap managed, should be managed via uno::Reference, "
            "parent is %1",
            fieldDecl->getLocation())
            << fieldDecl->getType()
            << fieldDecl->getParent()
            << fieldDecl->getSourceRange();
    }

// Not in general (dbaccess::DocumentEvents, dbaccess/source/core/dataaccess/databasedocument.hxx):
#if 0
    if (!firstTemplateParamType.isNull() && containsXInterfaceSubclass(firstTemplateParamType))
    {
        report(
            DiagnosticsEngine::Warning,
            "XInterface subclass %0 being managed via smart pointer, should be managed via uno::Reference, "
            "parent is %1",
            fieldDecl->getLocation())
            << firstTemplateParamType
            << fieldDecl->getParent()
            << fieldDecl->getSourceRange();
    }
#endif

    checkUnoReference(
        fieldDecl->getType(), fieldDecl,
        fieldDecl->getParent(), "field");

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
            StringRef name { getFileNameOfSpellingLoc(
                compiler.getSourceManager().getSpellingLoc(varDecl->getLocation())) };
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
    checkUnoReference(varDecl->getType(), varDecl, nullptr, "var");
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
    checkUnoReference(functionDecl->getReturnType(), functionDecl, nullptr, "return");
    return true;
}

loplugin::Plugin::Registration< RefCounting > X("refcounting");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
