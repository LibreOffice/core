/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include "check.hxx"
#include "plugin.hxx"

/** Look for static O*String and O*String[] which can be constepxr.
 */
namespace {

class StringStatic
    : public loplugin::FilteringPlugin<StringStatic>
{

public:
    explicit StringStatic(loplugin::InstantiationData const& rData):
        FilteringPlugin(rData) {}

    void run() override;
    bool VisitVarDecl(VarDecl const*);
};

void StringStatic::run()
{
    if( preRun())
        if( TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()))
            postRun();
}

bool StringStatic::VisitVarDecl(VarDecl const* varDecl)
{
    if (ignoreLocation(varDecl))
        return true;
    QualType qt = varDecl->getType();
    if (!varDecl->isThisDeclarationADefinition()
        || !qt.isConstQualified() || varDecl->isConstexpr())
        return true;

    if (varDecl->hasGlobalStorage())
    {
        if (qt->isArrayType())
            qt = qt->getAsArrayTypeUnsafe()->getElementType();

        auto tc = loplugin::TypeCheck(qt);
        if (!tc.Class("OUString").Namespace("rtl").GlobalNamespace()
            && !tc.Class("OString").Namespace("rtl").GlobalNamespace())
            return true;
        if (varDecl->hasInit())
        {
            Expr const * expr = varDecl->getInit();
            while (true) {
                if (ExprWithCleanups const * exprWithCleanups = dyn_cast<ExprWithCleanups>(expr)) {
                    expr = exprWithCleanups->getSubExpr();
                }
                else if (CastExpr const * castExpr = dyn_cast<CastExpr>(expr)) {
                    expr = castExpr->getSubExpr();
                }
                else if (MaterializeTemporaryExpr const * materializeExpr = dyn_cast<MaterializeTemporaryExpr>(expr)) {
                    expr = materializeExpr->getSubExpr();
                }
                else if (CXXBindTemporaryExpr const * bindExpr = dyn_cast<CXXBindTemporaryExpr>(expr)) {
                    expr = bindExpr->getSubExpr();
                }
                else if (CXXConstructExpr const * constructExpr = dyn_cast<CXXConstructExpr>(expr)) {
                    if (constructExpr->getNumArgs() == 0) {
                        return true;
                    }
                    expr = constructExpr->getArg(0);
                } else if (isa<CallExpr>(expr)) {
                    return true;
                } else {
                    break;
                }
            }
        }
    }
    else
    {
        if (isa<ParmVarDecl>(varDecl))
            return true;
        loplugin::TypeCheck const tc(varDecl->getType());
        if (!(tc.Class("OString").Namespace("rtl").GlobalNamespace()
              || tc.Class("OUString").Namespace("rtl").GlobalNamespace()))
        {
            return true;
        }
        if (varDecl->hasInit())
        {
            auto cxxConstruct = dyn_cast<CXXConstructExpr>(varDecl->getInit()->IgnoreImplicit());
            if (!cxxConstruct || cxxConstruct->getNumArgs() == 0)
                return true;
            if (!isa<clang::StringLiteral>(cxxConstruct->getArg(0)))
                return true;
        }
    }
    // cannot make something constexpr if it is also declared in a header file as extern
    if (loplugin::hasExternalLinkage(varDecl))
        return true;
    report(DiagnosticsEngine::Warning,
           "rather declare this as constexpr",
           varDecl->getLocation())
        << varDecl->getSourceRange();

    return true;
}

loplugin::Plugin::Registration<StringStatic> stringstatic("stringstatic");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
