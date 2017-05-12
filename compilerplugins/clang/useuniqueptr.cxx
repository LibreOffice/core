/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include "plugin.hxx"

/**
  Find destructors that only contain a single call to delete of a field. In which
  case that field should really be managed by unique_ptr.
*/

namespace {

class UseUniquePtr:
    public RecursiveASTVisitor<UseUniquePtr>, public loplugin::Plugin
{
public:
    explicit UseUniquePtr(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXDestructorDecl(const CXXDestructorDecl * );
    bool VisitCompoundStmt(const CompoundStmt * );
};

bool UseUniquePtr::VisitCXXDestructorDecl(const CXXDestructorDecl* destructorDecl)
{
    if (ignoreLocation(destructorDecl))
        return true;
    if (isInUnoIncludeFile(destructorDecl))
        return true;

    if (destructorDecl->getBody() == nullptr)
        return true;
    const CompoundStmt* compoundStmt = dyn_cast< CompoundStmt >( destructorDecl->getBody() );
    if (compoundStmt == nullptr) {
        return true;
    }

    const CXXDeleteExpr* deleteExpr;
    if (compoundStmt->size() == 1) {
        deleteExpr = dyn_cast<CXXDeleteExpr>(compoundStmt->body_front());
    }
    else if (compoundStmt->size() == 2) {
        // ignore SAL_INFO type stuff
        // TODO should probably be a little more specific here
        if (!isa<DoStmt>(compoundStmt->body_front())) {
            return true;
        }
        deleteExpr = dyn_cast<CXXDeleteExpr>(compoundStmt->body_back());
    } else {
        return true;
    }
    if (deleteExpr == nullptr) {
        return true;
    }

    const ImplicitCastExpr* pCastExpr = dyn_cast<ImplicitCastExpr>(deleteExpr->getArgument());
    if (!pCastExpr)
        return true;
    const MemberExpr* pMemberExpr = dyn_cast<MemberExpr>(pCastExpr->getSubExpr());
    if (!pMemberExpr)
        return true;

    // ignore union games
    const FieldDecl* pFieldDecl = dyn_cast<FieldDecl>(pMemberExpr->getMemberDecl());
    if (!pFieldDecl)
        return true;
    TagDecl const * td = dyn_cast<TagDecl>(pFieldDecl->getDeclContext());
    if (td->isUnion())
        return true;

    // ignore calling delete on someone else's field
    if (pFieldDecl->getParent() != destructorDecl->getParent() )
        return true;

    if (ignoreLocation(pFieldDecl))
        return true;
    // to ignore things like the CPPUNIT macros
    StringRef aFileName = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(pFieldDecl->getLocStart()));
    if (aFileName.startswith(WORKDIR))
        return true;
    // passes and stores pointers to member fields
    if (aFileName.startswith(SRCDIR "/sot/source/sdstor/stgdir.hxx"))
        return true;
    // something platform-specific
    if (aFileName.startswith(SRCDIR "/hwpfilter/source/htags.h"))
        return true;
    // @TODO there is clearly a bug in the ownership here, the operator= method cannot be right
    if (aFileName.startswith(SRCDIR "/include/formula/formdata.hxx"))
        return true;
    // passes pointers to member fields
    if (aFileName.startswith(SRCDIR "/sd/inc/sdpptwrp.hxx"))
        return true;
    // @TODO there is clearly a bug in the ownership here, the ScJumpMatrixToken copy constructor cannot be right
    if (aFileName.startswith(SRCDIR "/sc/inc/token.hxx"))
        return true;
    // @TODO intrusive linked-lists here, with some trickiness
    if (aFileName.startswith(SRCDIR "/sw/source/filter/html/parcss1.hxx"))
        return true;
    // @TODO SwDoc has some weird ref-counting going on
    if (aFileName.startswith(SRCDIR "/sw/inc/shellio.hxx"))
        return true;
    // @TODO it's sharing pointers with another class
    if (aFileName.startswith(SRCDIR "/sc/inc/formulacell.hxx"))
        return true;

    report(
        DiagnosticsEngine::Warning,
        "a destructor with only a single unconditional call to delete on a member, is a sure sign it should be using std::unique_ptr for that field",
        deleteExpr->getLocStart())
        << deleteExpr->getSourceRange();
    report(
        DiagnosticsEngine::Note,
        "member is here",
        pFieldDecl->getLocStart())
        << pFieldDecl->getSourceRange();
    return true;
}

bool UseUniquePtr::VisitCompoundStmt(const CompoundStmt* compoundStmt)
{
    if (ignoreLocation(compoundStmt))
        return true;
    if (isInUnoIncludeFile(compoundStmt->getLocStart()))
        return true;
    if (compoundStmt->size() == 0) {
        return true;
    }

    auto lastStmt = compoundStmt->body_back();
    if (compoundStmt->size() > 1) {
        if (isa<ReturnStmt>(lastStmt))
            lastStmt = *(++compoundStmt->body_rbegin());
    }
    auto deleteExpr = dyn_cast<CXXDeleteExpr>(lastStmt);
    if (deleteExpr == nullptr) {
        return true;
    }

    auto pCastExpr = dyn_cast<ImplicitCastExpr>(deleteExpr->getArgument());
    if (!pCastExpr)
        return true;
    auto declRefExpr = dyn_cast<DeclRefExpr>(pCastExpr->getSubExpr());
    if (!declRefExpr)
        return true;
    auto varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl());
    if (!varDecl)
        return true;
    if (!varDecl->hasInit()
        || !isa<CXXNewExpr>(
            varDecl->getInit()->IgnoreImplicit()->IgnoreParenImpCasts()))
        return true;
    // determine if the var is declared inside the same block as the delete.
    // @TODO there should surely be a better way to do this
    if (varDecl->getLocStart() < compoundStmt->getLocStart())
        return true;

    report(
        DiagnosticsEngine::Warning,
        "deleting a local variable at the end of a block, is a sure sign it should be using std::unique_ptr for that var",
        deleteExpr->getLocStart())
        << deleteExpr->getSourceRange();
    report(
        DiagnosticsEngine::Note,
        "var is here",
        varDecl->getLocStart())
        << varDecl->getSourceRange();
    return true;
}

loplugin::Plugin::Registration< UseUniquePtr > X("useuniqueptr");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
