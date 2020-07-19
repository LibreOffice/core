/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include <string>
#include <unordered_set>

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

// Find places where parameters are passed by value
// It's not very efficient, because we generally end up copying it twice - once into the parameter and
// again into the destination.
// They should rather be passed by reference.
//
// Generally recommending lambda capture by-ref rather than by-copy is even more
// problematic than with function parameters, as a lambda instance can easily
// outlive a referenced variable. So once lambdas start to get used in more
// sophisticated ways than passing them into standard algorithms, this plugin's
// advice, at least for explicit captures, will need to be revisited.

namespace {

class PassParamsByRef:
    public loplugin::FilteringPlugin<PassParamsByRef>
{
public:
    explicit PassParamsByRef(loplugin::InstantiationData const & data): FilteringPlugin(data), mbInsideFunctionDecl(false) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    // When warning about function params of primitive type that could be passed
    // by value instead of by reference, make sure not to warn if the parameter
    // is ever bound to a reference; on the one hand, this needs scaffolding in
    // all Traverse*Decl functions (indirectly) derived from FunctionDecl; and
    // on the other hand, use a hack of ignoring just the DeclRefExprs nested in
    // LValueToRValue ImplicitCastExprs when determining whether a param is
    // bound to a reference:
    bool PreTraverseFunctionDecl(FunctionDecl *);
    bool PostTraverseFunctionDecl(FunctionDecl *, bool);
    bool TraverseFunctionDecl(FunctionDecl *);
    bool PreTraverseImplicitCastExpr(ImplicitCastExpr *);
    bool TraverseImplicitCastExpr(ImplicitCastExpr *);

    bool VisitBinaryOperator(BinaryOperator const *);
    bool VisitCXXOperatorCallExpr(const CXXOperatorCallExpr *);

private:
    bool isFat(QualType type);

    bool mbInsideFunctionDecl;
    std::unordered_set<ParmVarDecl const *> mParamExclusions;
};

bool PassParamsByRef::PreTraverseFunctionDecl(FunctionDecl* functionDecl)
{
    if (ignoreLocation(functionDecl))
        return false;
    if (functionDecl->isDeleted()
        || functionDecl->isFunctionTemplateSpecialization())
    {
        return false;
    }
    // only consider base declarations, not overridden ones, or we warn on methods that
    // are overriding stuff from external libraries
    const CXXMethodDecl * methodDecl = dyn_cast<CXXMethodDecl>(functionDecl);
    if (methodDecl && methodDecl->size_overridden_methods() > 0)
        return false;

    // Only warn on the definition of the function:
    if (!functionDecl->doesThisDeclarationHaveABody())
        return false;

    mbInsideFunctionDecl = true;
    mParamExclusions.clear();
    return true;
}

bool PassParamsByRef::PostTraverseFunctionDecl(FunctionDecl* functionDecl, bool)
{
    mbInsideFunctionDecl = false;

    auto cxxConstructorDecl = dyn_cast<CXXConstructorDecl>(functionDecl);
    unsigned n = functionDecl->getNumParams();
    for (unsigned i = 0; i != n; ++i) {
        const ParmVarDecl * pvDecl = functionDecl->getParamDecl(i);
        auto const t = pvDecl->getType();
        if (!isFat(t))
            continue;
        if (mParamExclusions.find(pvDecl) != mParamExclusions.end())
            continue;
        // Ignore cases where the parameter is std::move'd.
        // This is a fairly simple check, might need some more complexity if the parameter is std::move'd
        // somewhere else in the constructor.
        bool bFoundMove = false;
        if (cxxConstructorDecl) {
            for (CXXCtorInitializer const * cxxCtorInitializer : cxxConstructorDecl->inits()) {
                if (cxxCtorInitializer->isMemberInitializer())
                {
                    auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(cxxCtorInitializer->getInit()->IgnoreParenImpCasts());
                    if (cxxConstructExpr && cxxConstructExpr->getNumArgs() == 1)
                    {
                        if (auto callExpr = dyn_cast<CallExpr>(cxxConstructExpr->getArg(0)->IgnoreParenImpCasts())) {
                            if (loplugin::DeclCheck(callExpr->getCalleeDecl()).Function("move").StdNamespace()) {
                                bFoundMove = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (bFoundMove)
            continue;
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
    return true;
}

bool PassParamsByRef::TraverseFunctionDecl(FunctionDecl* functionDecl)
{
    bool ret = true;
    if (PreTraverseFunctionDecl(functionDecl))
    {
        ret = RecursiveASTVisitor::TraverseFunctionDecl(functionDecl);
        PostTraverseFunctionDecl(functionDecl, ret);
    }
    return ret;
}

bool PassParamsByRef::PreTraverseImplicitCastExpr(ImplicitCastExpr * expr)
{
    if (ignoreLocation(expr))
        return false;
    if (expr->getCastKind() == CK_LValueToRValue
         && isa<DeclRefExpr>(expr->getSubExpr()->IgnoreParenImpCasts()))
        return false;
    return true;
}

bool PassParamsByRef::TraverseImplicitCastExpr(ImplicitCastExpr * expr)
{
    bool ret = true;
    if (PreTraverseImplicitCastExpr(expr))
    {
        ret = RecursiveASTVisitor::TraverseImplicitCastExpr(expr);
    }
    return ret;
}

bool PassParamsByRef::VisitBinaryOperator(const BinaryOperator * binaryOperator)
{
    if (binaryOperator->getOpcode() != BO_Assign) {
        return true;
    }
    if (!mbInsideFunctionDecl)
        return true;
    // if we are assigning to a parameter, it can be inconvenient to make the param pass-by-ref
    if (auto declRefExpr = dyn_cast<DeclRefExpr>(binaryOperator->getLHS()))
    {
        if (auto parmVarDecl = dyn_cast<ParmVarDecl>(declRefExpr->getDecl()))
            mParamExclusions.emplace(parmVarDecl);
    }
    return true;
}

bool PassParamsByRef::VisitCXXOperatorCallExpr(const CXXOperatorCallExpr * cxxOperatorCallExpr )
{
    if (!mbInsideFunctionDecl)
        return true;
    // if we are assigning to a parameter, it can be inconvenient to make the param pass-by-ref
    auto op = cxxOperatorCallExpr->getOperator();
    if ( op != clang::OverloadedOperatorKind::OO_Equal
         && op != clang::OverloadedOperatorKind::OO_SlashEqual
         && op != clang::OverloadedOperatorKind::OO_StarEqual
         && op != clang::OverloadedOperatorKind::OO_MinusEqual
         && op != clang::OverloadedOperatorKind::OO_PlusEqual)
        return true;
    auto declRefExpr = dyn_cast<DeclRefExpr>(cxxOperatorCallExpr->getArg(0));
    if (!declRefExpr)
        return true;
    if (auto parmVarDecl = dyn_cast<ParmVarDecl>(declRefExpr->getDecl()))
        mParamExclusions.emplace(parmVarDecl);
    return true;
}

bool PassParamsByRef::isFat(QualType type) {
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
    clang::Type const * t2 = type.getTypePtrOrNull();
    return t2 != nullptr
        && compiler.getASTContext().getTypeSizeInChars(t2).getQuantity() > 64;
}

loplugin::Plugin::Registration< PassParamsByRef > passparamsbyref("passparamsbyref");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
