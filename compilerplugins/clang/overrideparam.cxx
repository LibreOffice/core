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

#include "plugin.hxx"
#include "compat.hxx"
#include "check.hxx"

/*
  Find overriden methods that :
  (a) declare default params in different palces to their super-method(s)

  Still TODO
  (b) Find places where the values of the default parameters are different
  (c) Find places where the names of the parameters differ
*/

namespace {

class OverrideParam:
    public RecursiveASTVisitor<OverrideParam>, public loplugin::Plugin
{
public:
    explicit OverrideParam(InstantiationData const & data): Plugin(data) {}

    virtual void run() override;

    bool VisitCXXMethodDecl(const CXXMethodDecl *);

private:
    bool hasSameDefaultParams(const ParmVarDecl * parmVarDecl, const ParmVarDecl * superParmVarDecl);
};

void OverrideParam::run()
{
    /*
    StringRef fn( compiler.getSourceManager().getFileEntryForID(
                      compiler.getSourceManager().getMainFileID())->getName() );
    if (fn == SRCDIR "/include/svx/checklbx.hxx")
         return;
*/
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
}

bool OverrideParam::VisitCXXMethodDecl(const CXXMethodDecl * methodDecl) {
    if (ignoreLocation(methodDecl)) {
        return true;
    }
    if (methodDecl->isThisDeclarationADefinition() || methodDecl->size_overridden_methods() == 0) {
        return true;
    }
    loplugin::DeclCheck dc(methodDecl);
    // there is an InsertEntry override here which causes trouble if I modify it
    if (dc.Function("InsertEntry").Class("SvxCheckListBox").GlobalNamespace()) {
        return true;
    }
    // This class is overriding ShowCursor(bool) AND declaring a ShowCursor() method.
    // Adding a default param causes 'ambiguous override'.
    if (dc.Function("ShowCursor").Class("ScTabViewShell").GlobalNamespace()) {
        return true;
    }

    for(auto superMethodIt = methodDecl->begin_overridden_methods();
        superMethodIt != methodDecl->end_overridden_methods(); ++superMethodIt)
    {
        const CXXMethodDecl * superMethodDecl = *superMethodIt;
        if (ignoreLocation(superMethodDecl)) {
            continue;
        }
        int i = 0;
        for (const ParmVarDecl *superParmVarDecl : compat::parameters(*superMethodDecl)) {
            const ParmVarDecl *parmVarDecl = methodDecl->getParamDecl(i);
            if (parmVarDecl->hasDefaultArg() && !superParmVarDecl->hasDefaultArg()) {
                report(
                    DiagnosticsEngine::Warning,
                    "overridden method declaration has default arg, but super-method does not",
                    parmVarDecl->getSourceRange().getBegin())
                    << parmVarDecl->getSourceRange();
                report(
                    DiagnosticsEngine::Note,
                    "original param here",
                    superParmVarDecl->getSourceRange().getBegin())
                    << superParmVarDecl->getSourceRange();
            }
            else if (!parmVarDecl->hasDefaultArg() && superParmVarDecl->hasDefaultArg()) {
                report(
                    DiagnosticsEngine::Warning,
                    "overridden method declaration has no default arg, but super-method does",
                    parmVarDecl->getSourceRange().getBegin())
                    << parmVarDecl->getSourceRange();
                report(
                    DiagnosticsEngine::Note,
                    "original param here",
                    superParmVarDecl->getSourceRange().getBegin())
                    << superParmVarDecl->getSourceRange();
            }
            else if (parmVarDecl->hasDefaultArg() && superParmVarDecl->hasDefaultArg()
                && !hasSameDefaultParams(parmVarDecl, superParmVarDecl)) {
                /* do nothing for now, will enable this in a later commit
                report(
                    DiagnosticsEngine::Warning,
                    "overridden method declaration has different default param to super-method",
                    parmVarDecl->getSourceRange().getBegin())
                    << parmVarDecl->getSourceRange();
                report(
                    DiagnosticsEngine::Note,
                    "original param here",
                    superParmVarDecl->getSourceRange().getBegin())
                    << superParmVarDecl->getSourceRange();
                */
            }
            /* do nothing for now, will enable this in a later commit
            if (methodDecl->isThisDeclarationADefinition() && parmVarDecl->getName().empty()) {
                // ignore this - means the param is unused
            }
            else if (superParmVarDecl->getName().empty()) {
                // ignore, nothing reasonable I can do
            }
            else if (superParmVarDecl->getName() != parmVarDecl->getName()) {
                report(
                    DiagnosticsEngine::Warning,
                    "overridden method declaration uses different name for parameter",
                    parmVarDecl->getSourceRange().getBegin())
                    << parmVarDecl->getSourceRange();
                report(
                    DiagnosticsEngine::Note,
                    "original param here",
                    superParmVarDecl->getSourceRange().getBegin())
                    << superParmVarDecl->getSourceRange();
            }*/
            ++i;
        }
    }
    return true;
}

bool OverrideParam::hasSameDefaultParams(const ParmVarDecl * parmVarDecl, const ParmVarDecl * superParmVarDecl)
{
    // don't know what this means, but it prevents a clang crash
    if (parmVarDecl->hasUninstantiatedDefaultArg() || superParmVarDecl->hasUninstantiatedDefaultArg()) {
        return false;
    }
    const Expr* defaultArgExpr = parmVarDecl->getDefaultArg();
    const Expr* superDefaultArgExpr = superParmVarDecl->getDefaultArg();

    if (defaultArgExpr->isNullPointerConstant(compiler.getASTContext(), Expr::NPC_NeverValueDependent)
        && superDefaultArgExpr->isNullPointerConstant(compiler.getASTContext(), Expr::NPC_NeverValueDependent))
    {
        return true;
    }
    APSInt x1, x2;
    if (defaultArgExpr->EvaluateAsInt(x1, compiler.getASTContext())
        && superDefaultArgExpr->EvaluateAsInt(x2, compiler.getASTContext()))
    {
        return x1 == x2;
    }
    // catch params with defaults like "= OUString()"
    if (isa<MaterializeTemporaryExpr>(defaultArgExpr)
        && isa<MaterializeTemporaryExpr>(superDefaultArgExpr))
    {
        return true;
    }
    return false;
}


loplugin::Plugin::Registration< OverrideParam > X("overrideparam");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
