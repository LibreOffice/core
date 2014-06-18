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
//     public X* getX() { return &x; }
//   }
// which can be:
//     public X& getX() { return x; }

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
    // only consider base declarations, not overriden ones, or we warn on methods that
    // are overriding stuff from external libraries
    if (functionDecl->size_overridden_methods() > 0) {
        return true;
    }
    QualType t1 { functionDecl->getResultType() };
    if (!t1->isPointerType()) {
        return true;
    }

    /*
      The chain here looks like
    -CompoundStmt 0x2b811dc1c1a0 <col:31, col:48>
     `-ReturnStmt 0x2b811dc1c180 <col:33, col:41>
        `-UnaryOperator 0x2b811dc1c160 <col:40, col:41> 'struct SbxValues *' prefix '&'
          `-MemberExpr 0x2b811dc1c130 <col:41> 'struct SbxValues' lvalue ->aData 0x2b811dc120d0
            `-CXXThisExpr 0x2b811dc1c118 <col:41> 'class SbxValue *' this
    */

    const CompoundStmt* compoundStmt = dyn_cast< CompoundStmt >( functionDecl->getBody() );
    if (compoundStmt == nullptr) {
        return true;
    }
    if (compoundStmt->body_begin() == compoundStmt->body_end()) {
        return true;
    }
    const ReturnStmt* returnStmt = dyn_cast<ReturnStmt>(*compoundStmt->child_begin());
    if (returnStmt == nullptr) {
        return true;
    }
    const UnaryOperator* unaryOperator = dyn_cast<UnaryOperator>(*returnStmt->child_begin());
    if (unaryOperator == nullptr || unaryOperator->child_begin() == unaryOperator->child_end()) {
        return true;
    }
    const MemberExpr* memberExpr = dyn_cast<MemberExpr>(*unaryOperator->child_begin());
    if (memberExpr == nullptr || memberExpr->child_begin() == memberExpr->child_end()) {
        return true;
    }
    const CXXThisExpr* cXXThisExpr = dyn_cast<CXXThisExpr>(*memberExpr->child_begin());
    if (cXXThisExpr == nullptr) {
        return true;
    }

    report(
        DiagnosticsEngine::Warning,
        "passing ",
        functionDecl->getSourceRange().getBegin())
      << functionDecl->getSourceRange();
    // display the location of the class member declaration so I don't have to search for it by hand
    report(
        DiagnosticsEngine::Note,
        "passing",
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
