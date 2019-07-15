/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "check.hxx"
#include "plugin.hxx"

// TODO it would be better if we were running some kind of nullability analysis here, where we marked
// the results of expressions like Reference(..UNO_QUERY) as being nullable, and then looked for
// places where we unconditionally deference the results of that expression.

namespace
{
class UnoQuery : public loplugin::FilteringPlugin<UnoQuery>
{
public:
    explicit UnoQuery(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }
    bool VisitCXXMemberCallExpr(CXXMemberCallExpr const*);
};

bool UnoQuery::VisitCXXMemberCallExpr(CXXMemberCallExpr const* memberCallExpr)
{
    if (ignoreLocation(memberCallExpr))
        return true;

    auto isXInterface = [](Decl const* decl) -> bool {
        return bool(loplugin::DeclCheck(decl)
                        .Class("XInterface")
                        .Namespace("uno")
                        .Namespace("star")
                        .Namespace("sun")
                        .Namespace("com")
                        .GlobalNamespace());
    };
    if (!loplugin::isDerivedFrom(memberCallExpr->getRecordDecl(), isXInterface))
        return true;
    auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(
        memberCallExpr->getImplicitObjectArgument()->IgnoreImplicit());
    if (!operatorCallExpr)
        return true;

    Expr const* expr = operatorCallExpr->getArg(0)->IgnoreImplicit();
    // depending on the version of clang, the IgnoreImplicit may or may not look through these nodes
    if (auto matTemp = dyn_cast<MaterializeTemporaryExpr>(expr))
        expr = matTemp->GetTemporaryExpr();
    if (auto bindTemp = dyn_cast<CXXBindTemporaryExpr>(expr))
        expr = bindTemp->getSubExpr();

    auto temporaryExpr = dyn_cast<CXXTemporaryObjectExpr>(expr);
    if (!temporaryExpr)
        return true;
    if (temporaryExpr->getNumArgs() < 2)
        return true;
    auto declRefExpr = dyn_cast<DeclRefExpr>(temporaryExpr->getArg(1)->IgnoreImplicit());
    if (!declRefExpr)
        return true;
    auto enumConstant = dyn_cast<EnumConstantDecl>(declRefExpr->getDecl());
    if (!enumConstant)
        return true;
    if (enumConstant->getName() != "UNO_QUERY")
        return true;

    report(DiagnosticsEngine::Warning,
           "calling UNO_QUERY followed by unconditional method call might result in SIGSEGV, "
           "rather use UNO_QUERY_THROW",
           memberCallExpr->getExprLoc())
        << memberCallExpr->getSourceRange();

    return true;
}

loplugin::Plugin::Registration<UnoQuery> unoquery("unoquery", true);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
