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
#include "config_clang.h"
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

namespace {

class RefCounting:
    public loplugin::FilteringPlugin<RefCounting>
{
public:
    explicit RefCounting(loplugin::InstantiationData const & data): FilteringPlugin(data)
    {}

    virtual bool preRun() override { return true; }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitFieldDecl(const FieldDecl *);
    bool VisitVarDecl(const VarDecl *);
    bool VisitFunctionDecl(const FunctionDecl *);
    bool VisitTypeLoc(clang::TypeLoc typeLoc);
    bool VisitCXXDeleteExpr(const CXXDeleteExpr *);
    bool VisitBinaryOperator(const BinaryOperator *);
    bool VisitReturnStmt(const ReturnStmt *);

    // Creation of temporaries with one argument are represented by
    // CXXFunctionalCastExpr, while any other number of arguments are
    // represented by CXXTemporaryObjectExpr:
    bool VisitCXXTemporaryObjectExpr(CXXTemporaryObjectExpr const * expr)
    { return visitTemporaryObjectExpr(expr); }
    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr const * expr)
    { return visitTemporaryObjectExpr(expr); }

private:
    void checkUnoReference(QualType qt, const Decl* decl,
                           const RecordDecl* parent, const std::string& rDeclName);

    bool visitTemporaryObjectExpr(Expr const * expr);
    bool isCastingReference(const Expr* expr);
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
        if (loplugin::isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("ListenerMultiplexerBase").GlobalNamespace()); })) { // module UnoTools
            return false;
        }
        if (loplugin::isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("GridEventForwarder").Namespace("toolkit").GlobalNamespace()); })) { // module toolkit
            return false;
        }
        if (loplugin::isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("OWeakSubObject").GlobalNamespace()); })) { // module svx
            return false;
        }
        if (loplugin::isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("OSbaWeakSubObject").Namespace("dbaui").GlobalNamespace()); })) { // module dbaccess
            return false;
        }
        // FIXME This class has private operator new, and I cannot figure out how it can be dynamically instantiated
        if (loplugin::isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("XPropertyList").GlobalNamespace()); })) { // module svx
            return false;
        }
        // tdf#114596
        if (loplugin::isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("OBookmarkContainer").Namespace("dbaccess").GlobalNamespace()); })) { // module dbaccess
            return false;
        }

        // tdf#114596
        if (loplugin::isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("OCollection").Namespace("dbaccess").GlobalNamespace()); })) { // module dbaccess
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
        return loplugin::isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("XInterface").Namespace("uno").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace()); });
    }
}

bool containsOWeakObjectSubclass(const clang::Type* pType0);

bool containsOWeakObjectSubclass(const QualType& qType) {
    return containsOWeakObjectSubclass(qType.getTypePtr());
}

bool containsOWeakObjectSubclass(const clang::Type* pType0) {
    if (!pType0)
        return false;
    if (pType0->isDependentType()) {
         return false;
    }
    const clang::Type* pType = pType0->getUnqualifiedDesugaredType();
    if (!pType)
        return false;
    const CXXRecordDecl* pRecordDecl = pType->getAsCXXRecordDecl();
    if (pRecordDecl) {
        // because dbaccess just has to be special...
        loplugin::DeclCheck dc(pRecordDecl);
        if (dc.Class("DocumentEvents").Namespace("dbaccess")
                .GlobalNamespace() ||
            dc.Class("OBookmarkContainer").Namespace("dbaccess")
                .GlobalNamespace())
            return false;
        // TODO not sure about these ones, just avoiding dbaccess in general for now
        if (dc.Class("SbaXPropertiesChangeMultiplexer").Namespace("dbaui").GlobalNamespace() ||
            dc.Class("SbaXSubmitMultiplexer").Namespace("dbaui").GlobalNamespace() ||
            dc.Class("SbaXResetMultiplexer").Namespace("dbaui").GlobalNamespace() ||
            dc.Class("SbaXPropertyChangeMultiplexer").Namespace("dbaui").GlobalNamespace() ||
            dc.Class("SbaXSQLErrorMultiplexer").Namespace("dbaui").GlobalNamespace() ||
            dc.Class("SbaXParameterMultiplexer").Namespace("dbaui").GlobalNamespace() ||
            dc.Class("SbaXRowSetApproveMultiplexer").Namespace("dbaui").GlobalNamespace() ||
            dc.Class("SbaXRowSetMultiplexer").Namespace("dbaui").GlobalNamespace() ||
            dc.Class("SbaXLoadMultiplexer").Namespace("dbaui").GlobalNamespace() ||
            dc.Class("SbaXVetoableChangeMultiplexer").Namespace("dbaui").GlobalNamespace())
            return false;
        // slideshow playing games here
        if (dc.Class("SlideView").AnonymousNamespace().Namespace("internal").Namespace("slideshow").GlobalNamespace())
            return false;
        // svx playing acquire/release games here in OWeakSubObject
        if (dc.Class("FmXUpdateMultiplexer").GlobalNamespace() ||
            dc.Class("FmXContainerMultiplexer").GlobalNamespace() ||
            dc.Class("FmXSelectionMultiplexer").GlobalNamespace() ||
            dc.Class("FmXGridControlMultiplexer").GlobalNamespace() ||
            dc.Class("FmXModifyMultiplexer").GlobalNamespace())
            return false;
    }
    if (pType->isPointerType()) {
        // ignore
        return false;
    } else if (pType->isArrayType()) {
        const clang::ArrayType* pArrayType = dyn_cast<clang::ArrayType>(pType);
        QualType elementType = pArrayType->getElementType();
        return containsOWeakObjectSubclass(elementType);
    } else {
        return loplugin::isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("OWeakObject").Namespace("cppu").GlobalNamespace()); });
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
        return loplugin::isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("SvRefBase").Namespace("tools").GlobalNamespace()); });
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
        // for performance reasons we sometimes allocate temporaries on the stack
        if (loplugin::DeclCheck(pRecordDecl).Struct("ScSheetLimits").GlobalNamespace())
            return false;

        // the calc excel filter likes storing lots of classes either by reference or by value
        if (loplugin::isDerivedFrom(pRecordDecl,
                [](Decl const * decl) -> bool
                {
                    return
                        bool(loplugin::DeclCheck(decl).Class("XclExpRecordBase").GlobalNamespace())
                        || bool(loplugin::DeclCheck(decl).Class("XclImpChLineFormat").GlobalNamespace());
                }))
            return false;

        const ClassTemplateSpecializationDecl* pTemplate = dyn_cast<ClassTemplateSpecializationDecl>(pRecordDecl);
        if (pTemplate) {
            auto const dc = loplugin::DeclCheck(pTemplate);
            if (dc.Class("Reference").Namespace("rtl").GlobalNamespace()
                || (dc.Class("OStoreHandle").AnonymousNamespace().Namespace("store")
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
        return loplugin::isDerivedFrom(pRecordDecl, [](Decl const * decl) -> bool { return bool(loplugin::DeclCheck(decl).Class("SimpleReferenceObject").Namespace("salhelper").GlobalNamespace()); });
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
            expr->getBeginLoc())
            << t.getUnqualifiedType() << expr->getSourceRange();
    } else if (containsSalhelperReferenceObjectSubclass(t.getTypePtr())) {
        report(
            DiagnosticsEngine::Warning,
            ("Temporary object of salhelper::SimpleReferenceObject subclass %0"
             " being directly stack managed, should be managed via"
             " rtl::Reference"),
            expr->getBeginLoc())
            << t.getUnqualifiedType() << expr->getSourceRange();
    } else if (containsXInterfaceSubclass(t)) {
        report(
            DiagnosticsEngine::Warning,
            ("Temporary object of css::uno::XInterface subclass %0 being"
             " directly stack managed, should be managed via"
             " css::uno::Reference"),
            expr->getBeginLoc())
            << t.getUnqualifiedType() << expr->getSourceRange();
    } else if (containsOWeakObjectSubclass(t)) {
        report(
            DiagnosticsEngine::Warning,
            ("Temporary object of cppu::OWeakObject subclass %0 being"
             " directly stack managed, should be managed via"
             " css::uno::Reference"),
            expr->getBeginLoc())
            << t.getUnqualifiedType() << expr->getSourceRange();
    }
    return true;
}

// check for dodgy code managing ref-counted stuff with shared_ptr or unique_ptr or similar stuff
bool RefCounting::VisitTypeLoc(clang::TypeLoc typeLoc)
{
    QualType firstTemplateParamType;
    if (auto recordType = typeLoc.getType()->getUnqualifiedDesugaredType()->getAs<RecordType>()) {
        auto const tc = loplugin::TypeCheck(recordType);
        if (tc.ClassOrStruct("unique_ptr").StdNamespace()
            || tc.ClassOrStruct("weak_ptr").StdNamespace()
            || tc.ClassOrStruct("shared_ptr").StdNamespace()
            || tc.ClassOrStruct("intrusive_ptr").Namespace("boost").GlobalNamespace())
        {
            auto templateDecl = dyn_cast<ClassTemplateSpecializationDecl>(recordType->getDecl());
            if (templateDecl && templateDecl->getTemplateArgs().size() > 0)
                firstTemplateParamType = templateDecl->getTemplateArgs()[0].getAsType();
        }
    }
    if (firstTemplateParamType.isNull())
        return true;
    if (containsSvRefBaseSubclass(firstTemplateParamType.getTypePtr()))
    {
        report(
            DiagnosticsEngine::Warning,
            "SvRefBase subclass %0 being managed via smart pointer, should be managed via tools::SvRef",
            typeLoc.getBeginLoc())
            << firstTemplateParamType
            << typeLoc.getSourceRange();
    }
    if (containsSalhelperReferenceObjectSubclass(firstTemplateParamType.getTypePtr()))
    {
        report(
            DiagnosticsEngine::Warning,
            "salhelper::SimpleReferenceObject subclass %0 being managed via smart pointer, should be managed via rtl::Reference",
            typeLoc.getBeginLoc())
            << firstTemplateParamType
            << typeLoc.getSourceRange();
    }
// Not in general (dbaccess::DocumentEvents, dbaccess/source/core/dataaccess/databasedocument.hxx):
#if 0
    if (containsXInterfaceSubclass(firstTemplateParamType))
    {
        report(
            DiagnosticsEngine::Warning,
            "XInterface subclass %0 being managed via smart pointer, should be managed via uno::Reference",
            typeLoc.getBeginLoc())
            << firstTemplateParamType
            << typeLoc.getSourceRange();
    }
#endif
    if (containsOWeakObjectSubclass(firstTemplateParamType.getTypePtr()))
    {
        report(
            DiagnosticsEngine::Warning,
            "cppu::OWeakObject subclass %0 being managed via smart pointer, should be managed via rtl::Reference",
            typeLoc.getBeginLoc())
            << firstTemplateParamType
            << typeLoc.getSourceRange();
    }
    return true;
}

bool RefCounting::VisitCXXDeleteExpr(const CXXDeleteExpr * cxxDeleteExpr)
{
    if (ignoreLocation(cxxDeleteExpr))
        return true;
    StringRef aFileName = getFilenameOfLocation(
        compiler.getSourceManager().getSpellingLoc(cxxDeleteExpr->getBeginLoc()));
    if (loplugin::isSamePathname(aFileName, SRCDIR "/cppuhelper/source/weak.cxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/svx/svdobj.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/svx/source/svdraw/svdobj.cxx"))
        return true;

    if (!cxxDeleteExpr->getArgument())
        return true;
    auto argType = cxxDeleteExpr->getArgument()->getType();
    if (argType.isNull() || !argType->isPointerType())
        return true;
    auto pointeeType = argType->getPointeeType();
    if (containsOWeakObjectSubclass(pointeeType))
    {
        report(
            DiagnosticsEngine::Warning,
            "cppu::OWeakObject subclass %0 being deleted via delete, should be managed via rtl::Reference",
            cxxDeleteExpr->getBeginLoc())
            << pointeeType
            << cxxDeleteExpr->getSourceRange();
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

    // We can't call FieldDecl::getParent, which triggers an assertion at least with
    // current trunk towards Clang 3.7 when the FieldDecl is actually an
    // ObjCIvarDecl.
    if (isa<ObjCIvarDecl>(fieldDecl)) {
        return true;
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

    auto const dc = loplugin::DeclCheck(fieldDecl->getParent());
    if ( (dc.Class("BaseReference").Namespace("uno").Namespace("star")
          .Namespace("sun").Namespace("com").GlobalNamespace())
         || (dc.Class("Reference").Namespace("rtl").GlobalNamespace())
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

    if (containsOWeakObjectSubclass(fieldDecl->getType())) {
        report(
            DiagnosticsEngine::Warning,
            "cppu::OWeakObject subclass %0 being directly heap managed, should be managed via rtl::Reference, "
            "parent is %1",
            fieldDecl->getLocation())
            << fieldDecl->getType()
            << fieldDecl->getParent()
            << fieldDecl->getSourceRange();
    }

    checkUnoReference(
        fieldDecl->getType(), fieldDecl,
        fieldDecl->getParent(), "field");

    return true;
}

bool RefCounting::VisitReturnStmt(const ReturnStmt * returnStmt) {
    if (ignoreLocation(returnStmt)) {
        return true;
    }

    if (!returnStmt->getRetValue())
        return true;
    auto cxxNewExpr = dyn_cast<CXXNewExpr>(returnStmt->getRetValue()->IgnoreImplicit());
    if (!cxxNewExpr)
        return true;

    auto qt = returnStmt->getRetValue()->getType();
    if (!qt->isPointerType())
        return false;
    qt = qt->getPointeeType();

    if (containsOWeakObjectSubclass(qt)) {
        report(
            DiagnosticsEngine::Warning,
            "new object of cppu::OWeakObject subclass %0 being returned via raw pointer, should be returned by via rtl::Reference",
            returnStmt->getBeginLoc())
            << qt
            << returnStmt->getSourceRange();
    }

    return true;
}

bool RefCounting::VisitVarDecl(const VarDecl * varDecl) {
    if (ignoreLocation(varDecl))
        return true;

    checkUnoReference(varDecl->getType(), varDecl, nullptr, "var");

    if (isa<ParmVarDecl>(varDecl))
        return true;

    if (containsSvRefBaseSubclass(varDecl->getType().getTypePtr())) {
        report(
            DiagnosticsEngine::Warning,
            "SvRefBase subclass being directly stack managed, should be managed via tools::SvRef, "
            + varDecl->getType().getAsString(),
            varDecl->getLocation())
            << varDecl->getSourceRange();
    }
    if (containsSalhelperReferenceObjectSubclass(varDecl->getType().getTypePtr())) {
        StringRef name { getFilenameOfLocation(
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
    if (containsOWeakObjectSubclass(varDecl->getType())) {
        report(
            DiagnosticsEngine::Warning,
            "cppu::OWeakObject subclass being directly stack managed, should be managed via uno::Reference, "
            + varDecl->getType().getAsString(),
            varDecl->getLocation())
            << varDecl->getSourceRange();
    }

    if (varDecl->getType()->isPointerType() && varDecl->getInit())
    {
        auto newExpr = dyn_cast<CXXNewExpr>(varDecl->getInit()->IgnoreImplicit());
        if (newExpr)
        {
            StringRef fileName = getFilenameOfLocation(compiler.getSourceManager().getSpellingLoc(varDecl->getBeginLoc()));
            if (loplugin::isSamePathname(fileName, SRCDIR "/cppuhelper/source/component_context.cxx"))
                return true;
            auto pointeeType = varDecl->getType()->getPointeeType();
            if (containsOWeakObjectSubclass(pointeeType))
                report(
                    DiagnosticsEngine::Warning,
                    "cppu::OWeakObject subclass %0 being managed via raw pointer, should be managed via rtl::Reference",
                    varDecl->getLocation())
                    << pointeeType
                    << varDecl->getSourceRange();
        }
        if (isCastingReference(varDecl->getInit()))
        {
            // TODO false+ code
            StringRef fileName = getFilenameOfLocation(compiler.getSourceManager().getSpellingLoc(varDecl->getBeginLoc()));
            if (loplugin::isSamePathname(fileName, SRCDIR "/sw/source/core/unocore/unotbl.cxx"))
                return true;
            auto pointeeType = varDecl->getType()->getPointeeType();
            if (containsOWeakObjectSubclass(pointeeType))
                report(
                    DiagnosticsEngine::Warning,
                    "cppu::OWeakObject subclass %0 being managed via raw pointer, should be managed via rtl::Reference",
                    varDecl->getLocation())
                    << pointeeType
                    << varDecl->getSourceRange();
        }
    }
    return true;
}

/**
    Look for code like
        static_cast<FooChild*>(makeFoo().get());
    where makeFoo() returns a Reference<Foo>
*/
bool RefCounting::isCastingReference(const Expr* expr)
{
    expr = expr->IgnoreImplicit();
    auto castExpr = dyn_cast<CastExpr>(expr);
    if (!castExpr)
        return false;
    auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(castExpr->getSubExpr());
    if (!memberCallExpr)
        return false;
    if (!memberCallExpr->getMethodDecl()->getIdentifier() || memberCallExpr->getMethodDecl()->getName() != "get")
        return false;
    QualType objectType = memberCallExpr->getImplicitObjectArgument()->getType();
    if (!loplugin::TypeCheck(objectType).Class("Reference"))
        return false;
    // ignore "x.get()" where x is a var
    auto obj = memberCallExpr->getImplicitObjectArgument()->IgnoreImplicit();
    if (isa<DeclRefExpr>(obj) || isa<MemberExpr>(obj))
        return false;
    // if the foo in foo().get() returns "rtl::Reference<T>&" then the variable
    // we are assigning to does not __have__ to be Reference, since the method called
    // must already be holding a reference.
    if (auto callExpr = dyn_cast<CallExpr>(obj))
    {
        if (auto callMethod = callExpr->getDirectCallee())
            if (callMethod->getReturnType()->isReferenceType())
                return false;
    }
    // Ignore
    //     WeakReference x;
    //     if (x.get.get())
    // and similar stuff
    if (auto memberCall2 = dyn_cast<CXXMemberCallExpr>(obj))
    {
        if (loplugin::TypeCheck(memberCall2->getImplicitObjectArgument()->getType()).Class("WeakReference"))
                return false;
    }
    return true;
}

bool RefCounting::VisitBinaryOperator(const BinaryOperator * binaryOperator)
{
    if (ignoreLocation(binaryOperator))
        return true;
    if (binaryOperator->getOpcode() != BO_Assign)
        return true;
    if (!binaryOperator->getLHS()->getType()->isPointerType())
        return true;

    auto newExpr = dyn_cast<CXXNewExpr>(binaryOperator->getRHS()->IgnoreImplicit());
    if (newExpr)
    {
        // deliberately does not want to keep track at the allocation site
        StringRef fileName = getFilenameOfLocation(compiler.getSourceManager().getSpellingLoc(binaryOperator->getBeginLoc()));
        if (loplugin::isSamePathname(fileName, SRCDIR "/vcl/unx/generic/dtrans/X11_selection.cxx"))
            return true;

        auto pointeeType = binaryOperator->getLHS()->getType()->getPointeeType();
        if (containsOWeakObjectSubclass(pointeeType))
        {
            report(
                DiagnosticsEngine::Warning,
                "cppu::OWeakObject subclass %0 being managed via raw pointer, should be managed via rtl::Reference",
                binaryOperator->getBeginLoc())
                << pointeeType
                << binaryOperator->getSourceRange();
        }
    }
    if (isCastingReference(binaryOperator->getRHS()))
    {
        auto pointeeType = binaryOperator->getLHS()->getType()->getPointeeType();
        if (containsOWeakObjectSubclass(pointeeType))
            report(
                DiagnosticsEngine::Warning,
                "cppu::OWeakObject subclass %0 being managed via raw pointer, should be managed via rtl::Reference",
                binaryOperator->getBeginLoc())
                << pointeeType
                << binaryOperator->getSourceRange();
    }
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

loplugin::Plugin::Registration< RefCounting > refcounting("refcounting");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
