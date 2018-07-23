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
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        // can't change these because we pass them down to the SfxItemPool stuff
        if (fn == SRCDIR "/sc/source/core/data/docpool.cxx")
            return;
        // this just too clever for me
        if (fn == SRCDIR "/sc/source/core/tool/chgtrack.cxx")
            return;
        // too clever
        if (fn == SRCDIR "/pyuno/source/module/pyuno_runtime.cxx")
            return;
        // m_pExampleSet here is very badly managed. sometimes it is owning, sometimes not,
        // and the logic depends on overriding methods.
        if (fn == SRCDIR "/sfx2/source/dialog/tabdlg.cxx")
            return;
        // pLongArr is being deleted here because we temporarily overwrite a pointer to someone else's buffer, with a pointer
        // to our own buffer
        if (fn == SRCDIR "/editeng/source/misc/txtrange.cxx")
            return;
        // can't use std::set<std::unique_ptr<>> until C++14
        if (fn == SRCDIR "/editeng/source/misc/svxacorr.cxx")
            return;
        // horrible horrible spawn of evil ownership and deletion here
        if (fn == SRCDIR "/sfx2/source/view/ipclient.cxx")
            return;
        // sometimes it owns, sometimes it doesn't
        if (fn == SRCDIR "/editeng/source/misc/svxacorr.cxx")
            return;
        // SwDoc::m_PageDescs has weird handling
        if (fn == SRCDIR "/sw/source/core/doc/docnew.cxx")
            return;
        // SwRedlineData::pNext and pExtraData have complex handling
        if (fn == SRCDIR "/sw/source/core/doc/docredln.cxx")
            return;
        // ScTempDocSource::pTempDoc
        if (fn == SRCDIR "/sc/source/ui/unoobj/funcuno.cxx")
            return;
        // SwAttrIter::m_pFont
        if (fn == SRCDIR "/sw/source/core/text/itratr.cxx"
            || fn == SRCDIR "/sw/source/core/text/redlnitr.cxx")
            return;
        // SwWrongList
        if (fn == SRCDIR "/sw/source/core/text/wrong.cxx")
            return;
        // SwLineLayout::m_pNext
        if (fn == SRCDIR "/sw/source/core/text/porlay.cxx")
            return;
        // ODatabaseExport::m_aDestColumns
        if (fn == SRCDIR "/dbaccess/source/ui/misc/DExport.cxx")
            return;
        // ScTabView::pDrawActual and pDrawOld
        if (fn == SRCDIR "/sc/source/ui/view/tabview5.cxx")
            return;
        // SwHTMLParser::m_pPendStack
        if (fn == SRCDIR "/sw/source/filter/html/htmlcss1.cxx")
            return;
        // Visual Studio 2017 has trouble with these
        if (fn == SRCDIR "/comphelper/source/property/MasterPropertySet.cxx"
            || fn == SRCDIR "/comphelper/source/property/MasterPropertySetInfo.cxx")
            return;
        // SwTableLine::m_aBoxes
        if (fn == SRCDIR "/sw/source/core/table/swtable.cxx")
            return;
        // SwHTMLParser::m_pFormImpl
        if (fn == SRCDIR "/sw/source/filter/html/htmlform.cxx")
            return;
        // SwHTMLParser::m_pPendStack, pNext
        if (fn == SRCDIR "/sw/source/filter/html/htmltab.cxx")
            return;
        // SaveLine::pBox, pNext
        if (fn == SRCDIR "/sw/source/core/undo/untbl.cxx")
            return;
        // RedlineInfo::pNextRedline
        if (fn == SRCDIR "/sw/source/filter/xml/XMLRedlineImportHelper.cxx")
            return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXMethodDecl(const CXXMethodDecl* );
    bool VisitCompoundStmt(const CompoundStmt* );
    bool VisitCXXDeleteExpr(const CXXDeleteExpr* );
    bool TraverseFunctionDecl(FunctionDecl* );
private:
    void CheckCompoundStmt(const CXXMethodDecl*, const CompoundStmt* );
    void CheckForUnconditionalDelete(const CXXMethodDecl*, const CompoundStmt* );
    void CheckForSimpleDelete(const CXXMethodDecl*, const CompoundStmt* );
    void CheckRangedLoopDelete(const CXXMethodDecl*, const CXXForRangeStmt* );
    void CheckLoopDelete(const CXXMethodDecl*, const Stmt* );
    void CheckLoopDelete(const CXXMethodDecl*, const CXXDeleteExpr* );
    void CheckDeleteExpr(const CXXMethodDecl*, const CXXDeleteExpr*);
    void CheckParenExpr(const CXXMethodDecl*, const ParenExpr*);
    void CheckDeleteExpr(const CXXMethodDecl*, const CXXDeleteExpr*,
        const MemberExpr*, StringRef message);
    FunctionDecl const * mpCurrentFunctionDecl = nullptr;
};

bool UseUniquePtr::VisitCXXMethodDecl(const CXXMethodDecl* methodDecl)
{
    if (ignoreLocation(methodDecl))
        return true;
    if (isInUnoIncludeFile(methodDecl))
        return true;

    const CompoundStmt* compoundStmt = dyn_cast_or_null< CompoundStmt >( methodDecl->getBody() );
    if (!compoundStmt || compoundStmt->size() == 0)
        return true;

    CheckCompoundStmt(methodDecl, compoundStmt);

    return true;
}

void UseUniquePtr::CheckCompoundStmt(const CXXMethodDecl* methodDecl, const CompoundStmt* compoundStmt)
{
    CheckForSimpleDelete(methodDecl, compoundStmt);

    for (auto i = compoundStmt->body_begin(); i != compoundStmt->body_end(); ++i)
    {
        if (auto cxxForRangeStmt = dyn_cast<CXXForRangeStmt>(*i))
            CheckRangedLoopDelete(methodDecl, cxxForRangeStmt);
        else if (auto forStmt = dyn_cast<ForStmt>(*i))
            CheckLoopDelete(methodDecl, forStmt->getBody());
        else if (auto whileStmt = dyn_cast<WhileStmt>(*i))
            CheckLoopDelete(methodDecl, whileStmt->getBody());
        // check for unconditional inner compound statements
        else if (auto innerCompoundStmt = dyn_cast<CompoundStmt>(*i))
            CheckCompoundStmt(methodDecl, innerCompoundStmt);
    }
}

/**
 * check for simple call to delete in a destructor i.e. direct unconditional call, or if-guarded call
 */
void UseUniquePtr::CheckForSimpleDelete(const CXXMethodDecl* methodDecl, const CompoundStmt* compoundStmt)
{
    for (auto i = compoundStmt->body_begin(); i != compoundStmt->body_end(); ++i)
    {
        auto deleteExpr = dyn_cast<CXXDeleteExpr>(*i);
        if (deleteExpr)
        {
            CheckDeleteExpr(methodDecl, deleteExpr);
            continue;
        }
        auto parenExpr = dyn_cast<ParenExpr>(*i);
        if (parenExpr)
        {
            CheckParenExpr(methodDecl, parenExpr);
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
            CheckDeleteExpr(methodDecl, deleteExpr);
            continue;
        }

        parenExpr = dyn_cast<ParenExpr>(ifStmt->getThen());
        if (parenExpr)
        {
            CheckParenExpr(methodDecl, parenExpr);
            continue;
        }

        auto ifThenCompoundStmt = dyn_cast<CompoundStmt>(ifStmt->getThen());
        if (!ifThenCompoundStmt)
            continue;
        for (auto j = ifThenCompoundStmt->body_begin(); j != ifThenCompoundStmt->body_end(); ++j)
        {
            auto ifDeleteExpr = dyn_cast<CXXDeleteExpr>(*j);
            if (ifDeleteExpr)
                CheckDeleteExpr(methodDecl, ifDeleteExpr);
            ParenExpr const * parenExpr = dyn_cast<ParenExpr>(*j);
            if (parenExpr)
                CheckParenExpr(methodDecl, parenExpr);
        }
    }
}

/**
 * Check the delete expression in a destructor.
 */
void UseUniquePtr::CheckDeleteExpr(const CXXMethodDecl* methodDecl, const CXXDeleteExpr* deleteExpr)
{
    const ImplicitCastExpr* castExpr = dyn_cast<ImplicitCastExpr>(deleteExpr->getArgument());
    if (!castExpr)
        return;
    const MemberExpr* memberExpr = dyn_cast<MemberExpr>(castExpr->getSubExpr());
    if (!memberExpr)
        return;
    CheckDeleteExpr(methodDecl, deleteExpr, memberExpr,
        "unconditional call to delete on a member, should be using std::unique_ptr");
}

/**
 * Look for DELETEZ expressions.
 */
void UseUniquePtr::CheckParenExpr(const CXXMethodDecl* methodDecl, const ParenExpr* parenExpr)
{
    auto binaryOp = dyn_cast<BinaryOperator>(parenExpr->getSubExpr());
    if (!binaryOp || binaryOp->getOpcode() != BO_Comma)
        return;
    auto deleteExpr = dyn_cast<CXXDeleteExpr>(binaryOp->getLHS());
    if (!deleteExpr)
        return;
    CheckDeleteExpr(methodDecl, deleteExpr);
}

/**
 * Check the delete expression in a destructor.
 */
void UseUniquePtr::CheckDeleteExpr(const CXXMethodDecl* methodDecl, const CXXDeleteExpr* deleteExpr,
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
    if (fieldDecl->getParent() != methodDecl->getParent() )
        return;

    if (ignoreLocation(fieldDecl))
        return;
    // to ignore things like the CPPUNIT macros
    StringRef aFileName = getFileNameOfSpellingLoc(
        compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocStart()));
    if (loplugin::hasPathnamePrefix(aFileName, WORKDIR "/"))
        return;
    // passes and stores pointers to member fields
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sot/source/sdstor/stgdir.hxx"))
        return;
    // something platform-specific
    if (loplugin::isSamePathname(aFileName, SRCDIR "/hwpfilter/source/htags.h"))
        return;
    // passes pointers to member fields
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sd/inc/sdpptwrp.hxx"))
        return;
    // @TODO intrusive linked-lists here, with some trickiness
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sw/source/filter/html/parcss1.hxx"))
        return;
    // @TODO SwDoc has some weird ref-counting going on
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sw/inc/shellio.hxx"))
        return;
    // @TODO it's sharing pointers with another class
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sc/inc/formulacell.hxx"))
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
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sfx2/source/explorer/nochaos.cxx"))
        return;
    auto tc = loplugin::TypeCheck(fieldDecl->getType());
    // these sw::Ring based classes do not lend themselves to std::unique_ptr management
    if (tc.Pointer().Class("SwNodeIndex").GlobalNamespace() || tc.Pointer().Class("SwShellTableCursor").GlobalNamespace()
        || tc.Pointer().Class("SwBlockCursor").GlobalNamespace() || tc.Pointer().Class("SwVisibleCursor").GlobalNamespace()
        || tc.Pointer().Class("SwShellCursor").GlobalNamespace())
        return;
    // there is a loop in ~ImplPrnQueueList deleting stuff on a global data structure
    if (loplugin::isSamePathname(aFileName, SRCDIR "/vcl/inc/print.h"))
        return;
    // painful linked list
    if (loplugin::isSamePathname(aFileName, SRCDIR "/basic/source/inc/runtime.hxx"))
        return;
    // not sure how the node management is working here
    if (loplugin::isSamePathname(aFileName, SRCDIR "/i18npool/source/localedata/saxparser.cxx"))
        return;
    // has a pointer that it only sometimes owns
    if (loplugin::isSamePathname(aFileName, SRCDIR "/editeng/source/editeng/impedit.hxx"))
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

void UseUniquePtr::CheckLoopDelete(const CXXMethodDecl* methodDecl, const Stmt* bodyStmt)
{
    if (auto deleteExpr = dyn_cast<CXXDeleteExpr>(bodyStmt))
        CheckLoopDelete(methodDecl, deleteExpr);
    else if (auto compoundStmt = dyn_cast<CompoundStmt>(bodyStmt))
    {
        for (auto i = compoundStmt->body_begin(); i != compoundStmt->body_end(); ++i)
        {
            if (auto deleteExpr = dyn_cast<CXXDeleteExpr>(*i))
                CheckLoopDelete(methodDecl, deleteExpr);
        }
    }
}

void UseUniquePtr::CheckLoopDelete(const CXXMethodDecl* methodDecl, const CXXDeleteExpr* deleteExpr)
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

    CheckDeleteExpr(methodDecl, deleteExpr, memberExpr, "rather manage with std::some_container<std::unique_ptr<T>>");
}

void UseUniquePtr::CheckRangedLoopDelete(const CXXMethodDecl* methodDecl, const CXXForRangeStmt* cxxForRangeStmt)
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

    CheckDeleteExpr(methodDecl, deleteExpr, memberExpr, "rather manage with std::some_container<std::unique_ptr<T>>");
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

bool UseUniquePtr::TraverseFunctionDecl(FunctionDecl* functionDecl)
{
    if (ignoreLocation(functionDecl))
        return true;

    auto oldCurrent = mpCurrentFunctionDecl;
    mpCurrentFunctionDecl = functionDecl;
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(functionDecl);
    mpCurrentFunctionDecl = oldCurrent;

    return ret;
}

bool UseUniquePtr::VisitCXXDeleteExpr(const CXXDeleteExpr* deleteExpr)
{
    if (!mpCurrentFunctionDecl)
        return true;
    if (ignoreLocation(mpCurrentFunctionDecl))
        return true;
    if (isInUnoIncludeFile(mpCurrentFunctionDecl->getCanonicalDecl()->getLocStart()))
        return true;
    if (mpCurrentFunctionDecl->getIdentifier())
    {
        auto name = mpCurrentFunctionDecl->getName();
        if (name == "delete_IncludesCollection" || name == "convertName"
            || name == "createNamedType"
            || name == "typelib_typedescriptionreference_release" || name == "deleteExceptions"
            || name == "uno_threadpool_destroy"
            || name == "AddRanges_Impl"
            || name == "DestroySalInstance"
            || name == "ImplHandleUserEvent"
            || name == "releaseDecimalPtr" // TODO, basic
            || name == "replaceAndReset" // TODO, connectivity
            || name == "intrusive_ptr_release"
            || name == "FreeParaList"
            || name == "DeleteSdrUndoAction" // TODO, sc
            || name == "lcl_MergeGCBox" || name == "lcl_MergeGCLine" || name == "lcl_DelHFFormat")
            return true;
    }

    auto declRefExpr = dyn_cast<DeclRefExpr>(deleteExpr->getArgument()->IgnoreParenImpCasts());
    if (!declRefExpr)
        return true;
    auto varDecl = dyn_cast<ParmVarDecl>(declRefExpr->getDecl());
    if (!varDecl)
        return true;

    /*
    Sometimes we can pass the param as std::unique_ptr<T>& or std::unique_ptr, sometimes the method
    just needs to be inlined, which normally exposes more simplification.
    */
    report(
        DiagnosticsEngine::Warning,
        "calling delete on a pointer param, should be either whitelisted here or simplified",
        deleteExpr->getLocStart())
        << deleteExpr->getSourceRange();
    return true;
}

loplugin::Plugin::Registration< UseUniquePtr > X("useuniqueptr", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
