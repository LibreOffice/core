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

// Methods that purely return a local field should be declared in the header and be declared inline.
// So that the compiler can elide the function call and turn it into a simple fixed-offset-load instruction.

namespace {

class InlineSimpleAccessMethods:
    public RecursiveASTVisitor<InlineSimpleAccessMethods>, public loplugin::Plugin
{
public:
    explicit InlineSimpleAccessMethods(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXMethodDecl(const CXXMethodDecl * decl);
};

bool InlineSimpleAccessMethods::VisitCXXMethodDecl(const CXXMethodDecl * functionDecl) {
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    if (functionDecl->isVirtual()) {
        return true;
    }
    if (functionDecl->getTemplatedKind() != FunctionDecl::TK_NonTemplate) {
        return true;
    }
    if (!functionDecl->isInstance()) {
        return true;
    }
    if (!functionDecl->isOutOfLine()) {
        return true;
    }
    if( !functionDecl->hasBody()) {
        return true;
    }
    if( functionDecl->isInlineSpecified()) {
        return true;
    }
    // ignore stuff like:
    //   template<class E> E * Sequence<E>::begin() { return getArray(); }
    if( functionDecl->getParent()->getDescribedClassTemplate() != nullptr ) {
        return true;
    }

    /*
      The chain here looks like
      CompoundStmt
        -ReturnStmt
          - other stuff -
           -CXXThisExpr

      although sometimes the second ImplicitCastExpr is omitted.
    */

    const CompoundStmt* compoundStmt = dyn_cast< CompoundStmt >( functionDecl->getBody() );
    if (compoundStmt == nullptr) {
        return true;
    }
    if (compoundStmt->body_begin() == compoundStmt->body_end()) {
        return true;
    }

    const Stmt* childStmt = *compoundStmt->child_begin();

    if (dyn_cast<ReturnStmt>( childStmt ) == nullptr) {
        return true;
    }

    // look for a chain that ends with a "CXXThisExpr"

    int count = 0;
    while(1) {
        if (childStmt->child_begin() == childStmt->child_end()) {
           return true;
        }

        childStmt = *childStmt->child_begin();
        if (dyn_cast<CXXThisExpr>( childStmt ) != nullptr) {
           if (childStmt->child_begin() == childStmt->child_end()) {
               break;
           }
        }
        // We want simple expressions, but they seem to contain a variable number
        // of interleading nodes. But I don't want complex expressions, so this is a
        // hueristic.
        count++;
        if (count > 2) {
           return true;
        }
    }

    report(
        DiagnosticsEngine::Warning,
        "inlinesimpleaccessmethods",
        functionDecl->getSourceRange().getBegin())
      << functionDecl->getSourceRange();
    return true;
}

loplugin::Plugin::Registration< InlineSimpleAccessMethods > X("inlinesimpleaccessmethods");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
