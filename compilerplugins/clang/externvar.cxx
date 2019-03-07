/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

// Find variable declarations at namespace scope that need not have external
// linkage.

namespace {

// It looks like Clang wrongly implements DR 4
// (<http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#4>) and treats
// a variable declared in an 'extern "..." {...}'-style linkage-specification as
// if it contained the 'extern' specifier:
bool hasExternalLinkage(VarDecl const * decl) {
    if (decl->getLinkageAndVisibility().getLinkage() != ExternalLinkage) {
        return false;
    }
    for (auto ctx = decl->getLexicalDeclContext();
         ctx->getDeclKind() != Decl::TranslationUnit;
         ctx = ctx->getLexicalParent())
    {
        if (auto ls = dyn_cast<LinkageSpecDecl>(ctx)) {
            if (!ls->hasBraces()) {
                return true;
            }
            if (auto prev = decl->getPreviousDecl()) {
                return hasExternalLinkage(prev);
            }
            return !decl->isInAnonymousNamespace();
        }
    }
    return true;
}

class ExternVar: public loplugin::FilteringPlugin<ExternVar>
{
public:
    explicit ExternVar(loplugin::InstantiationData const & data): FilteringPlugin(data)
    {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitVarDecl(VarDecl const * decl) {
        if (ignoreLocation(decl)) {
            return true;
        }
        if (decl->isStaticDataMember()) {
            return true;
        }
        if (!(decl->isFirstDecl()
              && compiler.getSourceManager().isInMainFile(decl->getLocation())
              && hasExternalLinkage(decl)))
        {
            return true;
        }
        auto def = decl->getDefinition();
        if (def == nullptr) {
            // Code like
            //
            //   namespace { extern int v; }
            //   int f() { return sizeof(v); }
            //
            // is already handled by Clang itself with an error "variable 'v' is
            // not needed and will not be emitted"
            return true;
        }
        if (loplugin::DeclCheck(def).Var("_pRawDllMain").GlobalNamespace()) {
            return true;
        }
        SourceLocation argLoc;
        if (compiler.getSourceManager().isMacroArgExpansion(def->getLocation(), &argLoc)
            && (Lexer::getImmediateMacroName(
                    argLoc, compiler.getSourceManager(), compiler.getLangOpts())
                == "DEFINE_GUID"))
        {
            return true;
        }
        report(
            DiagnosticsEngine::Warning,
            "variable with external linkage not declared in an include file",
            def->getLocation())
            << def->getSourceRange();
        report(
            DiagnosticsEngine::Note,
            ("should either have internal linkage or be declared in an include"
             " file"),
            def->getLocation())
            << def->getSourceRange();
        for (auto prev = def;;) {
            prev = prev->getPreviousDecl();
            if (prev == nullptr) {
                break;
            }
            report(
                DiagnosticsEngine::Note, "previously declared here",
                prev->getLocation())
                << prev->getSourceRange();
        }
        return true;
    }
};

loplugin::Plugin::Registration<ExternVar> externvar("externvar");

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
