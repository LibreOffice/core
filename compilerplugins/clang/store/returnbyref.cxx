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
    std::string getFilename(SourceLocation loc);
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
    if (isInUnoIncludeFile(functionDecl)) {
        return true;
    }
    QualType t1 { functionDecl->getReturnType() };
    if (!t1->isPointerType()) {
        return true;
    }
    // Ignore stuff like:
    //   operator vcl::Window *(){ return &m_rWindow; }
    if (dyn_cast< CXXConversionDecl >( functionDecl ) != nullptr) {
        return true;
    }

    std::string aFilename = getFilename(functionDecl->getCanonicalDecl()->getLocStart());
    if (aFilename == SRCDIR "/include/o3tl/cow_wrapper.hxx")
    {
        return true;
    }
    if ( functionDecl->getNameAsString() == "operator->") {
        return true;
    }
    std::string aFunctionName = functionDecl->getQualifiedNameAsString();
    if (aFunctionName == "SbxValue::data") {
        return true;
    }
    /*
    std::string aParentName = functionDecl->getParent()->getQualifiedNameAsString();
    std::string fqn = aParentName + "::" + functionDecl->getNameAsString();
    if (aFilename == "TextCharAttribList::GetAttrib") {
        return true;
    }*/

    /*
      The AST here looks like:
      -CompoundStmt
       `-ReturnStmt
          `-UnaryOperator
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
nextStmt->dump();
    report(
        DiagnosticsEngine::Warning,
        "rather return by reference ",
        functionDecl->getSourceRange().getBegin())
      << functionDecl->getSourceRange();

    // display the location of the class member declaration so I don't have to search for it by hand
    auto otherLoc = functionDecl->getCanonicalDecl()->getSourceRange().getBegin();
    if (otherLoc != functionDecl->getSourceRange().getBegin())
    {
        report(
            DiagnosticsEngine::Note,
            "rather return by reference",
            functionDecl->getCanonicalDecl()->getSourceRange().getBegin())
          << functionDecl->getCanonicalDecl()->getSourceRange();
    }

    return true;
}

std::string ReturnByRef::getFilename(SourceLocation loc)
{
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(loc);
    return compiler.getSourceManager().getFilename(spellingLocation);
}

loplugin::Plugin::Registration< ReturnByRef > X("returnbyref");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
