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
#include "check.hxx"

/**
  Find destructors that only contain a single call to delete of a field. In which
  case that field should really be managed by unique_ptr.
*/

namespace {

class UseUniquePtr:
    public RecursiveASTVisitor<UseUniquePtr>, public loplugin::Plugin
{
public:
    explicit UseUniquePtr(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXDestructorDecl(const CXXDestructorDecl* );
    bool VisitCompoundStmt(const CompoundStmt* );
private:
    void CheckForUnconditionalDelete(const CXXDestructorDecl*, const CompoundStmt* );
    void CheckForSimpleDelete(const CXXDestructorDecl*, const CompoundStmt* );
    void CheckRangedLoopDelete(const CXXDestructorDecl*, const CXXForRangeStmt* );
    void CheckLoopDelete(const CXXDestructorDecl*, const Stmt* );
    void CheckLoopDelete(const CXXDestructorDecl*, const CXXDeleteExpr* );
    void CheckDeleteExpr(const CXXDestructorDecl*, const CXXDeleteExpr*);
    void CheckDeleteExpr(const CXXDestructorDecl*, const CXXDeleteExpr*,
        const MemberExpr*, StringRef message);
};

bool UseUniquePtr::VisitCXXDestructorDecl(const CXXDestructorDecl* destructorDecl)
{
    if (ignoreLocation(destructorDecl))
        return true;
    if (isInUnoIncludeFile(destructorDecl))
        return true;

    const CompoundStmt* compoundStmt = dyn_cast_or_null< CompoundStmt >( destructorDecl->getBody() );
    if (!compoundStmt || compoundStmt->size() == 0)
        return true;

    CheckForSimpleDelete(destructorDecl, compoundStmt);

    for (auto i = compoundStmt->body_begin(); i != compoundStmt->body_end(); ++i)
    {
        if (auto cxxForRangeStmt = dyn_cast<CXXForRangeStmt>(*i))
            CheckRangedLoopDelete(destructorDecl, cxxForRangeStmt);
        else if (auto forStmt = dyn_cast<ForStmt>(*i))
            CheckLoopDelete(destructorDecl, forStmt->getBody());
        else if (auto whileStmt = dyn_cast<WhileStmt>(*i))
            CheckLoopDelete(destructorDecl, whileStmt->getBody());
    }

    return true;
}

/**
 * check for simple call to delete in a destructor i.e. direct unconditional call, or if-guarded call
 */
void UseUniquePtr::CheckForSimpleDelete(const CXXDestructorDecl* destructorDecl, const CompoundStmt* compoundStmt)
{
    for (auto i = compoundStmt->body_begin(); i != compoundStmt->body_end(); ++i)
    {
        auto deleteExpr = dyn_cast<CXXDeleteExpr>(*i);
        if (deleteExpr)
        {
            CheckDeleteExpr(destructorDecl, deleteExpr);
            continue;
        }
        // Check for conditional deletes like:
        //     if (m_pField != nullptr) delete m_pField;
        auto ifStmt = dyn_cast<IfStmt>(*i);
        if (!ifStmt)
            continue;
        auto cond = ifStmt->getCond()->IgnoreImpCasts();
        if (auto ifCondMemberExpr = dyn_cast<MemberExpr>(cond))
        {
            // ignore "if (bMine)"
            if (!loplugin::TypeCheck(ifCondMemberExpr->getType()).Pointer())
                continue;
            // good
        }
        else if (auto binaryOp = dyn_cast<BinaryOperator>(cond))
        {
            if (!isa<MemberExpr>(binaryOp->getLHS()->IgnoreImpCasts()))
                continue;
            if (!isa<CXXNullPtrLiteralExpr>(binaryOp->getRHS()->IgnoreImpCasts()))
                continue;
            // good
        }
        else // ignore anything more complicated
            continue;

        deleteExpr = dyn_cast<CXXDeleteExpr>(ifStmt->getThen());
        if (deleteExpr)
        {
            CheckDeleteExpr(destructorDecl, deleteExpr);
            continue;
        }

        auto ifThenCompoundStmt = dyn_cast<CompoundStmt>(ifStmt->getThen());
        if (!ifThenCompoundStmt)
            continue;
        for (auto j = ifThenCompoundStmt->body_begin(); j != ifThenCompoundStmt->body_end(); ++j)
        {
            auto ifDeleteExpr = dyn_cast<CXXDeleteExpr>(*j);
            if (ifDeleteExpr)
                CheckDeleteExpr(destructorDecl, ifDeleteExpr);
        }
    }
}

/**
 * Check the delete expression in a destructor.
 */
void UseUniquePtr::CheckDeleteExpr(const CXXDestructorDecl* destructorDecl, const CXXDeleteExpr* deleteExpr)
{
    const ImplicitCastExpr* castExpr = dyn_cast<ImplicitCastExpr>(deleteExpr->getArgument());
    if (!castExpr)
        return;
    const MemberExpr* memberExpr = dyn_cast<MemberExpr>(castExpr->getSubExpr());
    if (!memberExpr)
        return;
    CheckDeleteExpr(destructorDecl, deleteExpr, memberExpr,
        "unconditional call to delete on a member, should be using std::unique_ptr");
}

/**
 * Check the delete expression in a destructor.
 */
void UseUniquePtr::CheckDeleteExpr(const CXXDestructorDecl* destructorDecl, const CXXDeleteExpr* deleteExpr,
    const MemberExpr* memberExpr, StringRef message)
{
    // ignore union games
    const FieldDecl* fieldDecl = dyn_cast<FieldDecl>(memberExpr->getMemberDecl());
    if (!fieldDecl)
        return;
    TagDecl const * td = dyn_cast<TagDecl>(fieldDecl->getDeclContext());
    if (td->isUnion())
        return;

    // ignore calling delete on someone else's field
    if (fieldDecl->getParent() != destructorDecl->getParent() )
        return;

    if (ignoreLocation(fieldDecl))
        return;
    // to ignore things like the CPPUNIT macros
    StringRef aFileName = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocStart()));
    if (loplugin::hasPathnamePrefix(aFileName, WORKDIR))
        return;
    // passes and stores pointers to member fields
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sot/source/sdstor/stgdir.hxx"))
        return;
    // something platform-specific
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/hwpfilter/source/htags.h"))
        return;
    // passes pointers to member fields
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sd/inc/sdpptwrp.hxx"))
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
    // some weird stuff going on here around struct Entity
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sax/"))
        return;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/sax/"))
        return;
    // manipulation of tree structures ie. StgAvlNode, don't lend themselves to std::unique_ptr
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sot/"))
        return;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/sot/"))
        return;
    // the std::vector is being passed to another class
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sfx2/source/explorer/nochaos.cxx"))
        return;
    // ignore std::map and std::unordered_map, MSVC 2015 has problems with mixing these with std::unique_ptr
    auto tc = loplugin::TypeCheck(fieldDecl->getType());
    if (tc.Class("map").StdNamespace() || tc.Class("unordered_map").StdNamespace())
        return;
    // there is a loop in ~ImplPrnQueueList deleting stuff on a global data structure
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/vcl/inc/print.h"))
        return;
    // painful linked list
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/basic/source/inc/runtime.hxx"))
        return;

    report(
        DiagnosticsEngine::Warning,
        message,
        deleteExpr->getLocStart())
        << deleteExpr->getSourceRange();
    report(
        DiagnosticsEngine::Note,
        "member is here",
        fieldDecl->getLocStart())
        << fieldDecl->getSourceRange();
}

void UseUniquePtr::CheckLoopDelete(const CXXDestructorDecl* destructorDecl, const Stmt* bodyStmt)
{
    if (auto deleteExpr = dyn_cast<CXXDeleteExpr>(bodyStmt))
        CheckLoopDelete(destructorDecl, deleteExpr);
    else if (auto compoundStmt = dyn_cast<CompoundStmt>(bodyStmt))
    {
        for (auto i = compoundStmt->body_begin(); i != compoundStmt->body_end(); ++i)
        {
            if (auto deleteExpr = dyn_cast<CXXDeleteExpr>(*i))
                CheckLoopDelete(destructorDecl, deleteExpr);
        }
    }
}

void UseUniquePtr::CheckLoopDelete(const CXXDestructorDecl* destructorDecl, const CXXDeleteExpr* deleteExpr)
{
    const MemberExpr* memberExpr = nullptr;
    const Expr* subExpr = deleteExpr->getArgument();
    for (;;)
    {
        subExpr = subExpr->IgnoreImpCasts();
        if ((memberExpr = dyn_cast<MemberExpr>(subExpr)))
            break;
        else if (auto arraySubscriptExpr = dyn_cast<ArraySubscriptExpr>(subExpr))
            subExpr = arraySubscriptExpr->getBase();
        else if (auto cxxOperatorCallExpr = dyn_cast<CXXOperatorCallExpr>(subExpr))
        {
            if (cxxOperatorCallExpr->getOperator() == OO_Subscript)
            {
                memberExpr = dyn_cast<MemberExpr>(cxxOperatorCallExpr->getArg(0));
                break;
            }
            break;
        }
        else
           break;
    }
    if (!memberExpr)
        return;

    CheckDeleteExpr(destructorDecl, deleteExpr, memberExpr, "rather manage with std::some_container<std::unique_ptr<T>>");
}

void UseUniquePtr::CheckRangedLoopDelete(const CXXDestructorDecl* destructorDecl, const CXXForRangeStmt* cxxForRangeStmt)
{
    CXXDeleteExpr const * deleteExpr = nullptr;
    if (auto compoundStmt = dyn_cast<CompoundStmt>(cxxForRangeStmt->getBody()))
        deleteExpr = dyn_cast<CXXDeleteExpr>(*compoundStmt->body_begin());
    else
        deleteExpr = dyn_cast<CXXDeleteExpr>(cxxForRangeStmt->getBody());
    if (!deleteExpr)
        return;
    auto memberExpr = dyn_cast<MemberExpr>(cxxForRangeStmt->getRangeInit());
    if (!memberExpr)
        return;
    auto fieldDecl = dyn_cast<FieldDecl>(memberExpr->getMemberDecl());
    if (!fieldDecl)
        return;

    CheckDeleteExpr(destructorDecl, deleteExpr, memberExpr, "rather manage with std::some_container<std::unique_ptr<T>>");
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

loplugin::Plugin::Registration< UseUniquePtr > X("useuniqueptr", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
