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

// Find places where we are returning a pointer to something, where we can be returning a reference.
// e.g.
//   class A {
//     struct X x;
//     public:
//     X* getX() { return &x; }
//   }
// which can be:
//     X& getX() { return x; }

namespace {

class ReturnByRef:
    public RecursiveASTVisitor<ReturnByRef>, public loplugin::Plugin
{
public:
    explicit ReturnByRef(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXMethodDecl(const CXXMethodDecl * decl);
private:
    bool isInUnoIncludeFile(SourceLocation spellingLocation) const;
    bool isInMainFile(SourceLocation spellingLocation) const;
};

bool ReturnByRef::VisitCXXMethodDecl(const CXXMethodDecl * functionDecl) {
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    if (functionDecl->isVirtual()) {
        return true;
    }
    if (!functionDecl->isInstance()) {
        return true;
    }
    if (!functionDecl->hasBody()) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              functionDecl->getCanonicalDecl()->getNameInfo().getLoc()))) {
        return true;
    }
    QualType t1 { functionDecl->getResultType() };
    if (!t1->isPointerType()) {
        return true;
    }
    // Ignore stuff like:
    //   operator Window *(){ return &m_rWindow; }
    if (dyn_cast< CXXConversionDecl >( functionDecl ) != nullptr) {
        return true;
    }

    /*
      The AST here looks like:
      -CompoundStmt
       `-ReturnStmt
          `-UnaryOperator
            `-MemberExpr
              `-CXXThisExpr
    */

    const CompoundStmt* compoundStmt = dyn_cast< CompoundStmt >( functionDecl->getBody() );
    if (compoundStmt == nullptr || compoundStmt->body_begin() == compoundStmt->body_end()) {
        return true;
    }
    const ReturnStmt* returnStmt = dyn_cast<ReturnStmt>(*compoundStmt->child_begin());
    if (returnStmt == nullptr) {
        return true;
    }

    const Stmt* nextStmt = dyn_cast<Expr>(*returnStmt->child_begin())->IgnoreParens();
    const UnaryOperator* unaryOperator = dyn_cast<UnaryOperator>(nextStmt);
    if (unaryOperator == nullptr || unaryOperator->getOpcode() != UO_AddrOf) {
        return true;
    }

    nextStmt = dyn_cast<Expr>(*unaryOperator->child_begin())->IgnoreParens();
    const MemberExpr* memberExpr = dyn_cast<MemberExpr>(nextStmt);
    if (memberExpr == nullptr) {
        return true;
    }

    nextStmt = dyn_cast<Expr>(*memberExpr->child_begin())->IgnoreParens();
    const CXXThisExpr* cXXThisExpr = dyn_cast<CXXThisExpr>(nextStmt);
    if (cXXThisExpr == nullptr) {
        return true;
    }

    report(
        DiagnosticsEngine::Warning,
        "rather return by reference",
        functionDecl->getSourceRange().getBegin())
      << functionDecl->getSourceRange();
    // display the location of the class member declaration so I don't have to search for it by hand
    report(
        DiagnosticsEngine::Note,
        "rather return by reference",
        functionDecl->getCanonicalDecl()->getSourceRange().getBegin())
      << functionDecl->getCanonicalDecl()->getSourceRange();

    return true;
}

bool ReturnByRef::isInUnoIncludeFile(SourceLocation spellingLocation) const {
    StringRef name {
        compiler.getSourceManager().getFilename(spellingLocation) };
    return isInMainFile(spellingLocation)
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
           || name.startswith(SRCDIR "/include/uno/")
           || name.startswith(SRCDIR "/workdir/")
           || name == SRCDIR "/include/comphelper/implbase_var.hxx");
}

bool ReturnByRef::isInMainFile(SourceLocation spellingLocation) const {
#if (__clang_major__ == 3 && __clang_minor__ >= 4) || __clang_major__ > 3
    return compiler.getSourceManager().isInMainFile(spellingLocation);
#else
    return compiler.getSourceManager().isFromMainFile(spellingLocation);
#endif
}

loplugin::Plugin::Registration< ReturnByRef > X("returnbyref");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
