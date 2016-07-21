/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "check.hxx"
#include "plugin.hxx"

/** Look for static OUString and OUString[], they can be more effeciently declared as:

        static const OUStringLiteral our_aLBEntryMap[] = {
            OUStringLiteral(" "),
            OUStringLiteral(", ")};
        static const OUStringLiteral sName("name");

    which is more efficient at startup time.
 */
namespace {

class StringStatic
    : public clang::RecursiveASTVisitor<StringStatic>
    , public loplugin::Plugin
{

public:
    explicit StringStatic(InstantiationData const& rData) : Plugin(rData) {}

    void run() override;
    bool VisitVarDecl(VarDecl const*);
    bool VisitReturnStmt(ReturnStmt const*);
private:
    std::set<VarDecl const *> potentialVars;
    std::set<VarDecl const *> excludeVars;
};

void StringStatic::run()
{
    StringRef fn( compiler.getSourceManager().getFileEntryForID(
                      compiler.getSourceManager().getMainFileID())->getName() );
    // passing around pointers to global OUString
    if (fn.startswith(SRCDIR "/filter/source/svg/"))
         return;
    // has a mix of literals and and refs to external OUStrings
    if (fn == SRCDIR "/ucb/source/ucp/webdav-neon/ContentProperties.cxx")
         return;

    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

    for (auto const & pVarDecl : excludeVars) {
        potentialVars.erase(pVarDecl);
    }
    for (auto const & varDecl : potentialVars) {
        report(DiagnosticsEngine::Warning,
                "rather declare this using OUStringLiteral or char[]",
                varDecl->getLocation())
            << varDecl->getSourceRange();
    }
}

bool StringStatic::VisitVarDecl(VarDecl const* varDecl)
{
    if (ignoreLocation(varDecl)) {
        return true;
    }

    QualType qt = varDecl->getType();
    if (!varDecl->hasGlobalStorage()
        || !varDecl->isThisDeclarationADefinition()
        || !qt.isConstQualified()) {
        return true;
    }
    if (qt->isArrayType()) {
        qt = qt->getAsArrayTypeUnsafe()->getElementType();
    }
    if (!loplugin::TypeCheck(qt).Class("OUString").Namespace("rtl").GlobalNamespace()) {
        return true;
    }
    if (varDecl->hasInit()) {
        Expr const * expr = varDecl->getInit();
        while (true) {
            if (ExprWithCleanups const * exprWithCleanups = dyn_cast<ExprWithCleanups>(expr)) {
                expr = exprWithCleanups->getSubExpr();
            }
            else if (CastExpr const * castExpr = dyn_cast<CastExpr>(expr)) {
                expr = castExpr->getSubExpr();
            }
            else if (MaterializeTemporaryExpr const * materializeExpr = dyn_cast<MaterializeTemporaryExpr>(expr)) {
                expr = materializeExpr->GetTemporaryExpr();
            }
            else if (CXXBindTemporaryExpr const * bindExpr = dyn_cast<CXXBindTemporaryExpr>(expr)) {
                expr = bindExpr->getSubExpr();
            }
            else if (CXXConstructExpr const * constructExpr = dyn_cast<CXXConstructExpr>(expr)) {
                if (constructExpr->getNumArgs() != 1) {
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
    potentialVars.insert(varDecl);

    return true;
}

bool StringStatic::VisitReturnStmt(ReturnStmt const * returnStmt)
{
    if (ignoreLocation(returnStmt)) {
        return true;
    }
    if (!returnStmt->getRetValue()) {
        return true;
    }
    DeclRefExpr const * declRef = dyn_cast<DeclRefExpr>(returnStmt->getRetValue());
    if (!declRef) {
        return true;
    }
    VarDecl const * varDecl = dyn_cast<VarDecl>(declRef->getDecl());
    if (varDecl) {
        excludeVars.insert(varDecl);
    }
    return true;
}

loplugin::Plugin::Registration<StringStatic> X("stringstatic");

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
