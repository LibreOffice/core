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

    bool VisitCXXDestructorDecl(const CXXDestructorDecl* );
    bool VisitCompoundStmt(const CompoundStmt* );
private:
    void CheckForSingleUnconditionalDelete(const CXXDestructorDecl*, const CompoundStmt* );
    void CheckForDeleteArrayOfPOD(const CompoundStmt* );
};

bool UseUniquePtr::VisitCXXDestructorDecl(const CXXDestructorDecl* destructorDecl)
{
    if (ignoreLocation(destructorDecl))
        return true;
    if (isInUnoIncludeFile(destructorDecl))
        return true;

    const CompoundStmt* compoundStmt = dyn_cast_or_null< CompoundStmt >( destructorDecl->getBody() );
    if (!compoundStmt)
        return true;

    CheckForSingleUnconditionalDelete(destructorDecl, compoundStmt);
    CheckForDeleteArrayOfPOD(compoundStmt);

    return true;
}

void UseUniquePtr::CheckForSingleUnconditionalDelete(const CXXDestructorDecl* destructorDecl, const CompoundStmt* compoundStmt)
{
    const CXXDeleteExpr* deleteExpr = nullptr;
    if (compoundStmt->size() == 1) {
        deleteExpr = dyn_cast<CXXDeleteExpr>(compoundStmt->body_front());
    }
    else if (compoundStmt->size() == 2) {
        // ignore SAL_INFO type stuff
        // @TODO should probably be a little more specific here
        if (isa<DoStmt>(compoundStmt->body_front())) {
            deleteExpr = dyn_cast<CXXDeleteExpr>(compoundStmt->body_back());
        }
        // look for the following pattern:
        // delete m_pbar;
        // m_pbar = nullptr;
        else if (auto binaryOp = dyn_cast<BinaryOperator>(compoundStmt->body_back())) {
            if (binaryOp->getOpcode() == BO_Assign)
                deleteExpr = dyn_cast<CXXDeleteExpr>(compoundStmt->body_front());
        }
    } else {
        return;
    }
    if (deleteExpr == nullptr)
        return;

    const ImplicitCastExpr* pCastExpr = dyn_cast<ImplicitCastExpr>(deleteExpr->getArgument());
    if (!pCastExpr)
        return;
    const MemberExpr* pMemberExpr = dyn_cast<MemberExpr>(pCastExpr->getSubExpr());
    if (!pMemberExpr)
        return;

    // ignore union games
    const FieldDecl* pFieldDecl = dyn_cast<FieldDecl>(pMemberExpr->getMemberDecl());
    if (!pFieldDecl)
        return;
    TagDecl const * td = dyn_cast<TagDecl>(pFieldDecl->getDeclContext());
    if (td->isUnion())
        return;

    // ignore calling delete on someone else's field
    if (pFieldDecl->getParent() != destructorDecl->getParent() )
        return;

    if (ignoreLocation(pFieldDecl))
        return;
    // to ignore things like the CPPUNIT macros
    StringRef aFileName = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(pFieldDecl->getLocStart()));
    if (loplugin::hasPathnamePrefix(aFileName, WORKDIR))
        return;
    // passes and stores pointers to member fields
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sot/source/sdstor/stgdir.hxx"))
        return;
    // something platform-specific
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/hwpfilter/source/htags.h"))
        return;
    // @TODO there is clearly a bug in the ownership here, the operator= method cannot be right
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/formula/formdata.hxx"))
        return;
    // passes pointers to member fields
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sd/inc/sdpptwrp.hxx"))
        return;
    // @TODO there is clearly a bug in the ownership here, the ScJumpMatrixToken copy constructor cannot be right
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sc/inc/token.hxx"))
        return;
    // @TODO intrusive linked-lists here, with some trickiness
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sw/source/filter/html/parcss1.hxx"))
        return;
    // @TODO SwDoc has some weird ref-counting going on
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sw/inc/shellio.hxx"))
        return;
    // @TODO it's sharing pointers with another class
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sc/inc/formulacell.hxx"))
        return;

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

void UseUniquePtr::CheckForDeleteArrayOfPOD(const CompoundStmt* compoundStmt)
{
    for (auto i = compoundStmt->body_begin();
              i != compoundStmt->body_end(); ++i)
    {
        auto deleteExpr = dyn_cast<CXXDeleteExpr>(*i);
        if (!deleteExpr || !deleteExpr->isArrayForm())
            continue;

        const Expr* argExpr = deleteExpr->getArgument();
        if (auto implicitCastExpr = dyn_cast<ImplicitCastExpr>(deleteExpr->getArgument()))
            argExpr = implicitCastExpr->getSubExpr();

        auto memberExpr = dyn_cast<MemberExpr>(argExpr);
        if (!memberExpr)
            continue;
        auto fieldDecl = dyn_cast<FieldDecl>(memberExpr->getMemberDecl());
        if (!fieldDecl)
            continue;
        // ignore union games
        auto * tagDecl = dyn_cast<TagDecl>(fieldDecl->getDeclContext());
        if (tagDecl->isUnion())
            continue;

        auto pointerType = dyn_cast<PointerType>(fieldDecl->getType()->getUnqualifiedDesugaredType());
        QualType elementType = pointerType->getPointeeType();
        if (!elementType.isTrivialType(compiler.getASTContext()))
            continue;

        StringRef aFileName = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocStart()));
        // TODO ignore this for now, it's just so messy to fix
        if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/tools/stream.hxx"))
            continue;
        // TODO this is very performance sensitive code
        if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/svl/itemset.hxx"))
            continue;
        // WW8TabBandDesc is playing games with copying/assigning itself
        if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sw/source/filter/ww8/ww8par.hxx"))
            continue;

        report(
            DiagnosticsEngine::Warning,
            "managing array of trival type %0 manually, rather use std::vector / std::array / std::unique_ptr",
            deleteExpr->getLocStart())
            << elementType
            << deleteExpr->getSourceRange();
        report(
            DiagnosticsEngine::Note,
            "member is here",
            fieldDecl->getLocStart())
            << fieldDecl->getSourceRange();
    }
}



loplugin::Plugin::Registration< UseUniquePtr > X("useuniqueptr");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
