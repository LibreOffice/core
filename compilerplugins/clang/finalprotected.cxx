/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <iostream>
#include <map>
#include <set>

#include "plugin.hxx"
#include "clang/AST/CXXInheritance.h"

// Check for final classes that have protected members

namespace
{

class FinalProtected:
    public RecursiveASTVisitor<FinalProtected>, public loplugin::Plugin
{
public:
    explicit FinalProtected(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXMethodDecl(CXXMethodDecl const *);
    bool VisitFieldDecl(FieldDecl const *);
};


bool FinalProtected::VisitCXXMethodDecl(CXXMethodDecl const * cxxMethodDecl)
{
    if (ignoreLocation(cxxMethodDecl)) {
        return true;
    }
    if (cxxMethodDecl->getAccess() != AS_protected) {
        return true;
    }
    if (!cxxMethodDecl->getParent()->hasAttr<FinalAttr>()) {
        return true;
    }
    cxxMethodDecl = cxxMethodDecl->getCanonicalDecl();
    report(DiagnosticsEngine::Warning,
            "final class should not have protected members - convert them to private",
            cxxMethodDecl->getLocStart())
        << cxxMethodDecl->getSourceRange();
    return true;
}

bool FinalProtected::VisitFieldDecl(FieldDecl const * fieldDecl)
{
    if (ignoreLocation(fieldDecl)) {
        return true;
    }
    if (fieldDecl->getAccess() != AS_protected) {
        return true;
    }
    if (!fieldDecl->getParent()->hasAttr<FinalAttr>()) {
        return true;
    }
    fieldDecl = fieldDecl->getCanonicalDecl();
    report(DiagnosticsEngine::Warning,
            "final class should not have protected members - convert them to private",
            fieldDecl->getLocStart())
        << fieldDecl->getSourceRange();
    return true;
}

loplugin::Plugin::Registration< FinalProtected > X("finalprotected", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
