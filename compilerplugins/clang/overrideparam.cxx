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
#include "check.hxx"

/*
  Find overridden methods that :
  (a) declare default params in different places to their super-method(s)

  Still TODO
  (b) Find places where the values of the default parameters are different
  (c) Find places where the names of the parameters differ
*/

namespace {

class OverrideParam:
    public loplugin::FilteringPlugin<OverrideParam>
{
public:
    explicit OverrideParam(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    virtual void run() override;

    bool VisitCXXMethodDecl(const CXXMethodDecl *);

private:
    bool hasSameDefaultParams(const ParmVarDecl * parmVarDecl, const ParmVarDecl * superParmVarDecl);
};

void OverrideParam::run()
{
    /*
    StringRef fn(handler.getMainFileName());
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
        for (const ParmVarDecl *superParmVarDecl : superMethodDecl->parameters()) {
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
        return true;
    }
    return
        checkIdenticalDefaultArguments(
            parmVarDecl->getDefaultArg(), superParmVarDecl->getDefaultArg())
        != IdenticalDefaultArgumentsResult::No;
        // for one, Clang 3.8 doesn't have EvaluateAsFloat; for another, since
        // <http://llvm.org/viewvc/llvm-project?view=revision&revision=291318>
        // "PR23135: Don't instantiate constexpr functions referenced in
        // unevaluated operands where possible", default args are not
        // necessarily evaluated, so the above calls to EvaluateAsInt etc. may
        // fail (as they do e.g. for SfxViewShell::SetPrinter and derived
        // classes' 'SfxPrinterChangeFlags nDiffFlags = SFX_PRINTER_ALL' arg,
        // include/sfx2/viewsh.hxx; what appears would help is to call
        // 'compiler.getSema().MarkDeclarationsReferencedInExpr()' on
        // defaultArgExpr and superDefaultArgExpr before any of the calls to
        // EvaluateAsInt etc., cf. the implementation of
        // Sema::CheckCXXDefaultArgExpr in Clang's lib/Sema/SemaExpr.cpp, but
        // that would probably have unwanted side-effects)
}

loplugin::Plugin::Registration< OverrideParam > X("overrideparam");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
