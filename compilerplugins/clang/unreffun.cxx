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

#include "compat.hxx"
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
#if (__clang_major__ == 3 && __clang_minor__ >= 3) || __clang_major__ > 3
    if (decl->isInExternCContext()) {
        return true;
    }
#else
    if (decl->getCanonicalDecl()->getDeclContext()->isExternCContext()) {
        return true;
    }
#endif
    return false;
}

class UnrefFun: public RecursiveASTVisitor<UnrefFun>, public loplugin::Plugin {
public:
    explicit UnrefFun(InstantiationData const & data): Plugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFunctionDecl(FunctionDecl const * decl);

private:
    bool isInUnoIncludeFile(SourceLocation spellingLocation) const;
};

bool UnrefFun::VisitFunctionDecl(FunctionDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }

    //TODO, filtering out anything template for now:
    if (decl->isDependentContext()) {
        return true;
    }
    CXXRecordDecl const * r = dyn_cast<CXXRecordDecl>(decl->getDeclContext());;
    if (r != nullptr && r->getTemplateSpecializationKind() != TSK_Undeclared) {
        return true;
    }

    FunctionDecl const * canon = decl->getCanonicalDecl();
        //TODO: is that the first?
    if (canon->isDeleted() || canon->isReferenced()
        || !(canon->isDefined()
             ? decl->isThisDeclarationADefinition()
             : compat::isFirstDecl(*decl))
        || !compat::isInMainFile(
            compiler.getSourceManager(), canon->getLocation())
        || isInUnoIncludeFile(
            compiler.getSourceManager().getSpellingLoc(
                canon->getNameInfo().getLoc()))
        || canon->isMain()
        || (compiler.getDiagnostics().getDiagnosticLevel(
                diag::warn_unused_function, decl->getLocation())
            < DiagnosticsEngine::Warning))
    {
        return true;
    }
    compat::LinkageInfo info(canon->getLinkageAndVisibility());
    if (compat::getLinkage(info) == ExternalLinkage
        && hasCLanguageLinkageType(canon) && canon->isDefined()
        && ((decl == canon && compat::getVisibility(info) == DefaultVisibility)
            || ((canon->hasAttr<ConstructorAttr>()
                 || canon->hasAttr<DestructorAttr>())
                && compat::getVisibility(info) == HiddenVisibility)))
    {
        return true;
    }
    report(
        DiagnosticsEngine::Warning,
        (canon->isDefined()
#if (__clang_major__ == 3 && __clang_minor__ >= 4) || __clang_major__ > 3
         ? (canon->isExternallyVisible()
            ? "Unreferenced externally visible function definition"
            : "Unreferenced externally invisible function definition")
#else
         ? "Unreferenced function definition"
#endif
         : "Unreferenced function declaration"),
        decl->getLocation())
        << decl->getSourceRange();
    if (canon->isDefined() && !compat::isFirstDecl(*decl)) {
        report(
            DiagnosticsEngine::Note, "first declaration is here",
            canon->getLocation())
            << canon->getSourceRange();
    }
    return true;
}

bool UnrefFun::isInUnoIncludeFile(SourceLocation spellingLocation) const {
    StringRef name {
        compiler.getSourceManager().getFilename(spellingLocation) };
    return compat::isInMainFile(compiler.getSourceManager(), spellingLocation)
        ? (name == SRCDIR "/cppu/source/cppu/compat.cxx"
           || name == SRCDIR "/cppuhelper/source/compat.cxx"
           || name == SRCDIR "/sal/osl/all/compat.cxx")
        : (name.startswith(SRCDIR "/include/com/")
           || name.startswith(SRCDIR "/include/cppu/")
           || name.startswith(SRCDIR "/include/cppuhelper/")
           || name.startswith(SRCDIR "/include/osl/")
           || name.startswith(SRCDIR "/include/rtl/")
           || name.startswith(SRCDIR "/include/sal/")
           || name.startswith(SRCDIR "/include/salhelper/")
           || name.startswith(SRCDIR "/include/systools/")
           || name.startswith(SRCDIR "/include/typelib/")
           || name.startswith(SRCDIR "/include/uno/"));
}

loplugin::Plugin::Registration<UnrefFun> X("unreffun");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
