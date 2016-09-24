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
#include <iostream>
#include <fstream>
#include <set>

#include "compat.hxx"
#include "plugin.hxx"

/**
look for methods where all they do is call their superclass method
*/

namespace {

class UnnecessaryOverride:
    public RecursiveASTVisitor<UnnecessaryOverride>, public loplugin::Plugin
{
public:
    explicit UnnecessaryOverride(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        // ignore some files with problematic macros
        StringRef fn( compiler.getSourceManager().getFileEntryForID(
                          compiler.getSourceManager().getMainFileID())->getName() );
        if (fn == SRCDIR "/sd/source/ui/framework/factories/ChildWindowPane.cxx")
             return;
        if (fn == SRCDIR "/forms/source/component/Date.cxx")
             return;
        if (fn == SRCDIR "/forms/source/component/Time.cxx")
            return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXMethodDecl(const CXXMethodDecl *);
};

bool UnnecessaryOverride::VisitCXXMethodDecl(const CXXMethodDecl* methodDecl)
{
    if (ignoreLocation(methodDecl->getCanonicalDecl()) || !methodDecl->doesThisDeclarationHaveABody()) {
        return true;
    }
    // if we are overriding more than one method, then this is a disambiguating override
    if (!methodDecl->isVirtual() || methodDecl->size_overridden_methods() != 1
        || (*methodDecl->begin_overridden_methods())->isPure()) {
        return true;
    }
    if (dyn_cast<CXXDestructorDecl>(methodDecl)) {
        return true;
    }
    // sometimes the disambiguation happens in a base class
    StringRef aFileName = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(methodDecl->getLocStart()));
    if (aFileName == SRCDIR "/comphelper/source/property/propertycontainer.cxx")
        return true;
    // not sure what is happening here
    if (aFileName == SRCDIR "/extensions/source/bibliography/datman.cxx")
        return true;
    // some very creative method hiding going on here
    if (aFileName == SRCDIR "/svx/source/dialog/checklbx.cxx")
        return true;
    // entertaining template magic
    if (aFileName == SRCDIR "/sc/source/ui/vba/vbaformatcondition.cxx")
        return true;

    const CXXMethodDecl* overriddenMethodDecl = *methodDecl->begin_overridden_methods();

    if (compat::getReturnType(*methodDecl).getCanonicalType()
        != compat::getReturnType(*overriddenMethodDecl).getCanonicalType())
    {
        return true;
    }
    //TODO: check for identical exception specifications

    const CompoundStmt* compoundStmt = dyn_cast<CompoundStmt>(methodDecl->getBody());
    if (!compoundStmt || compoundStmt->size() != 1)
        return true;
    auto returnStmt = dyn_cast<ReturnStmt>(*compoundStmt->body_begin());
    if (returnStmt == nullptr) {
        return true;
    }
    auto returnExpr = returnStmt->getRetValue();
    if (returnExpr == nullptr) {
        return true;
    }
    returnExpr = returnExpr->IgnoreImplicit();

    // In something like
    //
    //  Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery(
    //      const rtl::OUString& sql)
    //      throw(SQLException, RuntimeException, std::exception)
    //  {
    //      return OCommonStatement::executeQuery( sql );
    //  }
    //
    // look down through all the
    //
    //   ReturnStmt
    //   `-ExprWithCleanups
    //     `-CXXConstructExpr
    //      `-MaterializeTemporaryExpr
    //       `-ImplicitCastExpr
    //        `-CXXBindTemporaryExpr
    //         `-CXXMemberCallExpr
    //
    // where the fact that the overriding and overridden function have identical
    // return types makes us confident that all we need to check here is whether
    // there's an (arbitrary, one-argument) CXXConstructorExpr and
    // CXXBindTemporaryExpr in between:
    if (auto ctorExpr = dyn_cast<CXXConstructExpr>(returnExpr)) {
        if (ctorExpr->getNumArgs() == 1) {
            if (auto tempExpr = dyn_cast<CXXBindTemporaryExpr>(
                    ctorExpr->getArg(0)->IgnoreImplicit()))
            {
                returnExpr = tempExpr->getSubExpr();
            }
        }
    }

    const CXXMemberCallExpr* callExpr = dyn_cast<CXXMemberCallExpr>(
        returnExpr->IgnoreParenImpCasts());
    if (!callExpr || callExpr->getMethodDecl() != overriddenMethodDecl)
        return true;
    const ImplicitCastExpr* expr1 = dyn_cast_or_null<ImplicitCastExpr>(callExpr->getImplicitObjectArgument());
    if (!expr1)
        return true;
    const CXXThisExpr* expr2 = dyn_cast_or_null<CXXThisExpr>(expr1->getSubExpr());
    if (!expr2)
        return true;
    for (unsigned i = 0; i<callExpr->getNumArgs(); ++i) {
        // ignore ImplicitCastExpr
        const DeclRefExpr * declRefExpr = dyn_cast<DeclRefExpr>(callExpr->getArg(i)->IgnoreImplicit());
        if (!declRefExpr || declRefExpr->getDecl() != methodDecl->getParamDecl(i))
            return true;
    }

    report(
        DiagnosticsEngine::Warning, "%0 virtual function just calls %1 parent",
        methodDecl->getSourceRange().getBegin())
        << methodDecl->getAccess() << overriddenMethodDecl->getAccess()
        << methodDecl->getSourceRange();
    if (methodDecl->getCanonicalDecl()->getLocation() != methodDecl->getLocation()) {
        const CXXMethodDecl* pOther = methodDecl->getCanonicalDecl();
        report(
            DiagnosticsEngine::Note,
            "method declaration here",
            pOther->getLocStart())
          << pOther->getSourceRange();
    }
    return true;
}


loplugin::Plugin::Registration< UnnecessaryOverride > X("unnecessaryoverride", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
