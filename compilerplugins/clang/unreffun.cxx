/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>

#include "clang/AST/Attr.h"
#include "clang/Sema/SemaInternal.h" // warn_unused_function

#include "plugin.hxx"

namespace {

// It appears that, given a function declaration, there is no way to determine
// the language linkage of the function's type, only of the function's name
// (via FunctionDecl::isExternC); however, in a case like
//
//   extern "C" { static void f(); }
//
// the function's name does not have C language linkage while the function's
// type does (as clarified in C++11 [decl.link]); cf. <http://clang-developers.
// 42468.n3.nabble.com/Language-linkage-of-function-type-tt4037248.html>
// "Language linkage of function type":
bool hasCLanguageLinkageType(FunctionDecl const * decl) {
    assert(decl != nullptr);
    if (decl->isExternC()) {
        return true;
    }
    if (decl->isInExternCContext()) {
        return true;
    }
    return false;
}

bool isFriendDecl(Decl const * decl) {
    return decl->getFriendObjectKind() != Decl::FOK_None;
}

Decl const * getPreviousNonFriendDecl(Decl const * decl) {
    for (;;) {
        decl = decl->getPreviousDecl();
        if (decl == nullptr || !isFriendDecl(decl)) {
            return decl;
        }
    }
}

class UnrefFun: public RecursiveASTVisitor<UnrefFun>, public loplugin::Plugin {
public:
    explicit UnrefFun(InstantiationData const & data): Plugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFunctionDecl(FunctionDecl const * decl);
};

bool UnrefFun::VisitFunctionDecl(FunctionDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }

    //TODO, filtering out any functions relating to (class) templates for now:
    CXXRecordDecl const * r = dyn_cast<CXXRecordDecl>(decl->getDeclContext());
    if (r != nullptr
        && (r->getTemplateSpecializationKind() != TSK_Undeclared
            || r->isDependentContext()))
    {
        return true;
    }

    if (!(decl->isThisDeclarationADefinition() || isFriendDecl(decl)
          || decl->isFunctionTemplateSpecialization()))
    {
        Decl const * prev = getPreviousNonFriendDecl(decl);
        if (prev != nullptr/* && prev != decl->getPrimaryTemplate()*/) {
            // Workaround for redeclarations that introduce visibility attributes
            // (as is done with
            //
            //  SAL_DLLPUBLIC_EXPORT GType lok_doc_view_get_type();
            //
            // in libreofficekit/source/gtk/lokdocview.cxx):
            if (decl->getAttr<VisibilityAttr>() != nullptr
                && prev->getAttr<VisibilityAttr>() == nullptr)
            {
                return true;
            }
            report(
                DiagnosticsEngine::Warning,
                "redundant function%0 redeclaration", decl->getLocation())
                << ((decl->getTemplatedKind()
                     == FunctionDecl::TK_FunctionTemplate)
                    ? " template" : "")
                << decl->getSourceRange();
            report(
                DiagnosticsEngine::Note, "previous declaration is here",
                prev->getLocation())
                << prev->getSourceRange();
            return true;
        }
    }

    FunctionDecl const * canon = decl->getCanonicalDecl();
        //TODO: is that the first?
    if (canon->isDeleted() || canon->isReferenced()
        || !(canon->isDefined()
             ? decl->isThisDeclarationADefinition() : decl->isFirstDecl())
        || !compiler.getSourceManager().isInMainFile(canon->getLocation())
        || isInUnoIncludeFile(canon)
        || canon->isMain() || canon->isMSVCRTEntryPoint()
        || (decl->getTemplatedKind() == FunctionDecl::TK_FunctionTemplate
            && (decl->getDescribedFunctionTemplate()->spec_begin()
                != decl->getDescribedFunctionTemplate()->spec_end()))
        || (compiler.getDiagnostics().getDiagnosticLevel(
                diag::warn_unused_function, decl->getLocation())
            < DiagnosticsEngine::Warning))
    {
        return true;
    }
    LinkageInfo info(canon->getLinkageAndVisibility());
    if (info.getLinkage() == ExternalLinkage
        && hasCLanguageLinkageType(canon) && canon->isDefined()
        && ((decl == canon && info.getVisibility() == DefaultVisibility)
            || ((canon->hasAttr<ConstructorAttr>()
                 || canon->hasAttr<DestructorAttr>())
                && info.getVisibility() == HiddenVisibility)))
    {
        return true;
    }
    auto loc = decl->getLocation();
    if (compiler.getSourceManager().isMacroBodyExpansion(loc)
        && (Lexer::getImmediateMacroName(
                loc, compiler.getSourceManager(), compiler.getLangOpts())
            == "MDDS_MTV_DEFINE_ELEMENT_CALLBACKS"))
    {
        return true;
    }
    report(
        DiagnosticsEngine::Warning,
        (canon->isDefined()
         ? (canon->isExternallyVisible()
            ? "Unreferenced externally visible function%0 definition"
            : "Unreferenced externally invisible function%0 definition")
         : "Unreferenced function%0 declaration"),
        decl->getLocation())
        << (decl->getTemplatedKind() == FunctionDecl::TK_FunctionTemplate
            ? " template" : "")
        << decl->getSourceRange();
    if (canon->isDefined() && !decl->isFirstDecl()) {
        report(
            DiagnosticsEngine::Note, "first declaration is here",
            canon->getLocation())
            << canon->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<UnrefFun> X("unreffun");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
