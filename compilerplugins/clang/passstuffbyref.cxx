/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <set>

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

// Find places where various things are passed by value.
// It's not very efficient, because we generally end up copying it twice - once into the parameter and
// again into the destination.
// They should rather be passed by reference.
//
// Generally recommending lambda capture by-ref rather than by-copy is even more
// problematic than with function parameters, as a lambda instance can easily
// outlive a referrenced variable.  So once lambdas start to get used in more
// sophisticated ways than passing them into standard algorithms, this plugin's
// advice, at least for explicit captures, will need to be revisited.

namespace {

class PassStuffByRef:
    public RecursiveASTVisitor<PassStuffByRef>, public loplugin::Plugin
{
public:
    explicit PassStuffByRef(InstantiationData const & data): Plugin(data), mbInsideFunctionDecl(false), mbFoundDisqualifier(false) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    // When warning about function params of primitive type that could be passed
    // by value instead of by reference, make sure not to warn if the parameter
    // is ever bound to a reference; on the one hand, this needs scaffolding in
    // all Traverse*Decl functions (indirectly) derived from FunctionDecl; and
    // on the other hand, use a hack of ignoring just the DeclRefExprs nested in
    // LValueToRValue ImplicitCastExprs when determining whether a param is
    // bound to a reference:
    bool TraverseFunctionDecl(FunctionDecl * decl);
    bool TraverseCXXMethodDecl(CXXMethodDecl * decl);
    bool TraverseCXXConstructorDecl(CXXConstructorDecl * decl);
    bool TraverseCXXDestructorDecl(CXXDestructorDecl * decl);
    bool TraverseCXXConversionDecl(CXXConversionDecl * decl);
    bool VisitFunctionDecl(const FunctionDecl * decl);
    bool TraverseImplicitCastExpr(ImplicitCastExpr * expr);
    bool VisitDeclRefExpr(const DeclRefExpr * expr);

    bool VisitReturnStmt(const ReturnStmt * );
    bool VisitVarDecl(const VarDecl * );

private:
    template<typename T> bool traverseAnyFunctionDecl(
        T * decl, bool (RecursiveASTVisitor::* fn)(T *));
    void checkParams(const FunctionDecl * functionDecl);
    void checkReturnValue(const FunctionDecl * functionDecl, const CXXMethodDecl * methodDecl);
    bool isFat(QualType type);
    bool isPrimitiveConstRef(QualType type);
    bool isReturnExprDisqualified(const Expr* expr);

    bool mbInsideFunctionDecl;
    bool mbFoundDisqualifier;

    struct FDecl {
        std::set<ParmVarDecl const *> parms;
        bool check = false;
    };
    std::vector<FDecl> functionDecls_;
};

bool PassStuffByRef::TraverseFunctionDecl(FunctionDecl * decl) {
    return traverseAnyFunctionDecl(
        decl, &RecursiveASTVisitor::TraverseFunctionDecl);
}

bool PassStuffByRef::TraverseCXXMethodDecl(CXXMethodDecl * decl) {
    return traverseAnyFunctionDecl(
        decl, &RecursiveASTVisitor::TraverseCXXMethodDecl);
}

bool PassStuffByRef::TraverseCXXConstructorDecl(CXXConstructorDecl * decl) {
    return traverseAnyFunctionDecl(
        decl, &RecursiveASTVisitor::TraverseCXXConstructorDecl);
}

bool PassStuffByRef::TraverseCXXDestructorDecl(CXXDestructorDecl * decl) {
    return traverseAnyFunctionDecl(
        decl, &RecursiveASTVisitor::TraverseCXXDestructorDecl);
}

bool PassStuffByRef::TraverseCXXConversionDecl(CXXConversionDecl * decl) {
    return traverseAnyFunctionDecl(
        decl, &RecursiveASTVisitor::TraverseCXXConversionDecl);
}

template<typename T> bool PassStuffByRef::traverseAnyFunctionDecl(
    T * decl, bool (RecursiveASTVisitor::* fn)(T *))
{
    if (ignoreLocation(decl)) {
        return true;
    }
    if (decl->doesThisDeclarationHaveABody()) {
        functionDecls_.emplace_back();
    }
    bool ret = (this->*fn)(decl);
    if (decl->doesThisDeclarationHaveABody()) {
        assert(!functionDecls_.empty());
        if (functionDecls_.back().check) {
            for (auto d: functionDecls_.back().parms) {
                report(
                    DiagnosticsEngine::Warning,
                    ("passing primitive type param %0 by const &, rather pass"
                     " by value"),
                    d->getLocation())
                    << d->getType() << d->getSourceRange();
                auto can = decl->getCanonicalDecl();
                if (can->getLocation() != decl->getLocation()) {
                    report(
                        DiagnosticsEngine::Note, "function is declared here:",
                        can->getLocation())
                        << can->getSourceRange();
                }
            }
        }
        functionDecls_.pop_back();
    }
    return ret;
}

bool PassStuffByRef::VisitFunctionDecl(const FunctionDecl * functionDecl) {
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    if (functionDecl->isDeleted()
        || functionDecl->isFunctionTemplateSpecialization())
    {
        return true;
    }
    // only consider base declarations, not overriden ones, or we warn on methods that
    // are overriding stuff from external libraries
    const CXXMethodDecl * methodDecl = dyn_cast<CXXMethodDecl>(functionDecl);
    if (methodDecl && methodDecl->size_overridden_methods() > 0) {
            return true;
    }

    checkParams(functionDecl);
    checkReturnValue(functionDecl, methodDecl);
    return true;
}

bool PassStuffByRef::TraverseImplicitCastExpr(ImplicitCastExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    return
        (expr->getCastKind() == CK_LValueToRValue
         && (dyn_cast<DeclRefExpr>(expr->getSubExpr()->IgnoreParenImpCasts())
             != nullptr))
        || RecursiveASTVisitor::TraverseImplicitCastExpr(expr);
}

bool PassStuffByRef::VisitDeclRefExpr(const DeclRefExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto d = dyn_cast<ParmVarDecl>(expr->getDecl());
    if (d == nullptr) {
        return true;
    }
    for (auto & fd: functionDecls_) {
        if (fd.parms.erase(d) == 1) {
            break;
        }
    }
    return true;
}

void PassStuffByRef::checkParams(const FunctionDecl * functionDecl) {
    // Only warn on the definition of the function:
    if (!functionDecl->doesThisDeclarationHaveABody()) {
        return;
    }
    unsigned n = functionDecl->getNumParams();
    for (unsigned i = 0; i != n; ++i) {
        const ParmVarDecl * pvDecl = functionDecl->getParamDecl(i);
        auto const t = pvDecl->getType();
        if (isFat(t)) {
            report(
                DiagnosticsEngine::Warning,
                ("passing %0 by value, rather pass by const lvalue reference"),
                pvDecl->getLocation())
                << t << pvDecl->getSourceRange();
            auto can = functionDecl->getCanonicalDecl();
            if (can->getLocation() != functionDecl->getLocation()) {
                report(
                    DiagnosticsEngine::Note, "function is declared here:",
                    can->getLocation())
                    << can->getSourceRange();
            }
        }
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              functionDecl->getCanonicalDecl()->getNameInfo().getLoc()))) {
        return;
    }
    // these functions are passed as parameters to another function
    if (loplugin::DeclCheck(functionDecl).MemberFunction()
        .Class("ShapeAttributeLayer").Namespace("internal")
        .Namespace("slideshow").GlobalNamespace())
    {
        return;
    }
    assert(!functionDecls_.empty());
    functionDecls_.back().check = true;
    for (unsigned i = 0; i != n; ++i) {
        const ParmVarDecl * pvDecl = functionDecl->getParamDecl(i);
        auto const t = pvDecl->getType();
        if (isPrimitiveConstRef(t)) {
            functionDecls_.back().parms.insert(pvDecl);
        }
    }
}

void PassStuffByRef::checkReturnValue(const FunctionDecl * functionDecl, const CXXMethodDecl * methodDecl) {

    if (methodDecl && (methodDecl->isVirtual() || methodDecl->hasAttr<OverrideAttr>())) {
        return;
    }
    if( !functionDecl->doesThisDeclarationHaveABody()) {
        return;
    }

    const QualType type = compat::getReturnType(*functionDecl).getDesugaredType(compiler.getASTContext());
    if (type->isReferenceType() || type->isIntegralOrEnumerationType() || type->isPointerType()
        || type->isTemplateTypeParmType() || type->isDependentType() || type->isBuiltinType()
        || type->isScalarType())
    {
        return;
    }

    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              functionDecl->getCanonicalDecl()->getNameInfo().getLoc()))) {
        return;
    }
    loplugin::DeclCheck dc(functionDecl);
    // function is passed as parameter to another function
    if (dc.Function("ImplColMonoFnc").Class("GDIMetaFile").GlobalNamespace()
        || (dc.Function("darkColor").Class("SvxBorderLine").Namespace("editeng")
            .GlobalNamespace())
        || (dc.MemberFunction().Class("Binding").Namespace("xforms")
            .GlobalNamespace())
        || (dc.MemberFunction().Class("Model").Namespace("xforms")
            .GlobalNamespace())
        || (dc.MemberFunction().Class("Submission").Namespace("xforms")
            .GlobalNamespace())
        || (dc.Function("TopLeft").Class("SwRect").GlobalNamespace()))
    {
        return;
    }
    // not sure how to exclude this yet, returns copy of one of it's params
    if (dc.Function("sameDistColor").GlobalNamespace()
        || dc.Function("sameColor").GlobalNamespace()
        || (dc.Operator(OO_Call).Struct("StringIdentity").AnonymousNamespace()
            .Namespace("pcr").GlobalNamespace())
        || (dc.Function("accumulate").Namespace("internal")
            .Namespace("slideshow").GlobalNamespace())
        || (dc.Function("lerp").Namespace("internal").Namespace("slideshow")
            .GlobalNamespace()))
        return;
    // depends on a define
    if (dc.Function("GetSharedFileURL").Class("SfxObjectShell")
        .GlobalNamespace()) {
        return;
    }
    mbInsideFunctionDecl = true;
    mbFoundDisqualifier = false;
    TraverseStmt(functionDecl->getBody());
    mbInsideFunctionDecl = false;

    if (mbFoundDisqualifier)
        return;

    report(
            DiagnosticsEngine::Warning,
            "rather return %0 from function %1 %2 by const& than by value, to avoid unnecessary copying",
            functionDecl->getSourceRange().getBegin())
        << type.getAsString() << functionDecl->getQualifiedNameAsString() << type->getTypeClassName() << functionDecl->getSourceRange();

    // display the location of the class member declaration so I don't have to search for it by hand
    if (functionDecl->getSourceRange().getBegin() != functionDecl->getCanonicalDecl()->getSourceRange().getBegin())
    {
        report(
                DiagnosticsEngine::Note,
                "decl here",
                functionDecl->getCanonicalDecl()->getSourceRange().getBegin())
            << functionDecl->getCanonicalDecl()->getSourceRange();
    }
}

bool PassStuffByRef::VisitReturnStmt(const ReturnStmt * returnStmt)
{
    if (!mbInsideFunctionDecl)
        return true;
    const Expr* expr = dyn_cast<Expr>(*returnStmt->child_begin())->IgnoreParenCasts();

    if (isReturnExprDisqualified(expr)) {
        mbFoundDisqualifier = true;
        return true;
    }
    return true;
}

bool PassStuffByRef::isReturnExprDisqualified(const Expr* expr)
{
    if (isa<ExprWithCleanups>(expr)) {
        return true;
    }
    if (const CXXConstructExpr* constructExpr = dyn_cast<CXXConstructExpr>(expr)) {
        if (constructExpr->getNumArgs()==1) {
            expr = constructExpr->getArg(0)->IgnoreParenCasts();
        }
    }
    if (isa<CXXConstructExpr>(expr)) {
        return true;
    }
    if (const ArraySubscriptExpr* childExpr = dyn_cast<ArraySubscriptExpr>(expr)) {
        expr = childExpr->getLHS();
    }
    if (const MemberExpr* memberExpr = dyn_cast<MemberExpr>(expr)) {
        expr = memberExpr->getBase();
    }
    if (const DeclRefExpr* declRef = dyn_cast<DeclRefExpr>(expr)) {
        const VarDecl* varDecl = dyn_cast<VarDecl>(declRef->getDecl());
        if (varDecl) {
            if (varDecl->getStorageDuration() == SD_Automatic
                || varDecl->getStorageDuration() == SD_FullExpression ) {
                return true;
            }
            return false;
        }
    }
    if (const ConditionalOperator* condOper = dyn_cast<ConditionalOperator>(expr)) {
        return isReturnExprDisqualified(condOper->getTrueExpr())
            || isReturnExprDisqualified(condOper->getFalseExpr());
    }
    if (const CallExpr* callExpr = dyn_cast<CallExpr>(expr)) {
        return !loplugin::TypeCheck(callExpr->getType()).Const().LvalueReference();
    }
    return true;
}

bool PassStuffByRef::VisitVarDecl(const VarDecl * varDecl)
{
    if (!mbInsideFunctionDecl)
        return true;
    // things guarded by locking are probably best left alone
    loplugin::TypeCheck dc(varDecl->getType());
    if (dc.Class("Guard").Namespace("osl").GlobalNamespace())
        mbFoundDisqualifier = true;
    if (dc.Class("ClearableGuard").Namespace("osl").GlobalNamespace())
        mbFoundDisqualifier = true;
    if (dc.Class("ResettableGuard").Namespace("osl").GlobalNamespace())
        mbFoundDisqualifier = true;
    else if (dc.Class("SolarMutexGuard").GlobalNamespace())
        mbFoundDisqualifier = true;
    else if (dc.Class("SfxModelGuard").GlobalNamespace())
        mbFoundDisqualifier = true;
    else if (dc.Class("ReadWriteGuard").Namespace("utl").GlobalNamespace())
        mbFoundDisqualifier = true;
    return true;
}

// Would produce a wrong recommendation for
//
//   PresenterFrameworkObserver::RunOnUpdateEnd(
//       xCC,
//       [pSelf](bool){ return pSelf->ShutdownPresenterScreen(); });
//
// in PresenterScreen::RequestShutdownPresenterScreen
// (sdext/source/presenter/PresenterScreen.cxx), with no obvious way to work
// around it:
//
// bool PassStuffByRef::VisitLambdaExpr(const LambdaExpr * expr) {
//     if (ignoreLocation(expr)) {
//         return true;
//     }
//     for (auto i(expr->capture_begin()); i != expr->capture_end(); ++i) {
//         if (i->getCaptureKind() == LambdaCaptureKind::LCK_ByCopy) {
//             auto const t = i->getCapturedVar()->getType();
//             if (isFat(t)) {
//                 report(
//                     DiagnosticsEngine::Warning,
//                     ("%0 capture of %1 variable by copy, rather use capture"
//                      " by reference---UNLESS THE LAMBDA OUTLIVES THE VARIABLE"),
//                     i->getLocation())
//                     << (i->isImplicit() ? "implicit" : "explicit") << t
//                     << expr->getSourceRange();
//             }
//         }
//     }
//     return true;
// }

bool PassStuffByRef::isFat(QualType type) {
    if (!type->isRecordType()) {
        return false;
    }
    loplugin::TypeCheck tc(type);
    if ((tc.Class("Reference").Namespace("uno").Namespace("star")
            .Namespace("sun").Namespace("com").GlobalNamespace())
        || (tc.Class("Sequence").Namespace("uno").Namespace("star")
            .Namespace("sun").Namespace("com").GlobalNamespace())
        || tc.Class("OString").Namespace("rtl").GlobalNamespace()
        || tc.Class("OUString").Namespace("rtl").GlobalNamespace()
        || tc.Class("Reference").Namespace("rtl").GlobalNamespace())
    {
        return true;
    }
    if (type->isIncompleteType()) {
        return false;
    }
    Type const * t2 = type.getTypePtrOrNull();
    return t2 != nullptr
        && compiler.getASTContext().getTypeSizeInChars(t2).getQuantity() > 64;
}

bool PassStuffByRef::isPrimitiveConstRef(QualType type) {
    if (type->isIncompleteType()) {
        return false;
    }
    const clang::ReferenceType* referenceType = type->getAs<ReferenceType>();
    if (referenceType == nullptr) {
        return false;
    }
    QualType pointeeQT = referenceType->getPointeeType();
    if (!pointeeQT.isConstQualified()) {
        return false;
    }
    if (!pointeeQT->isFundamentalType()) {
        return false;
    }
    // ignore double for now, some of our code seems to believe it is cheaper to pass by ref
    const BuiltinType* builtinType = pointeeQT->getAs<BuiltinType>();
    return builtinType->getKind() != BuiltinType::Kind::Double;
}


loplugin::Plugin::Registration< PassStuffByRef > X("passstuffbyref");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
