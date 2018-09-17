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
    public loplugin::FilteringPlugin<UseUniquePtr>
{
public:
    explicit UseUniquePtr(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    virtual void run() override
    {
        fn = handler.getMainFileName();
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
        // SfxObjectShell::pMedium
        if (fn == SRCDIR "/sfx2/source/doc/objxtor.cxx")
            return;
        // various
        if (fn == SRCDIR "/sw/source/filter/ww8/wrtww8.cxx")
            return;
        // WW8TabBandDesc
        if (fn == SRCDIR "/sw/source/filter/ww8/ww8par2.cxx")
            return;
        // ZipOutputStream, ownership of ZipEntry is horribly complicated here
        if (fn == SRCDIR "/package/source/zipapi/ZipOutputStream.cxx")
            return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitFunctionDecl(const FunctionDecl* );
    bool VisitCompoundStmt(const CompoundStmt* );
    bool VisitCXXDeleteExpr(const CXXDeleteExpr* );
    bool TraverseFunctionDecl(FunctionDecl* );
    bool TraverseCXXMethodDecl(CXXMethodDecl* );
    bool TraverseCXXConstructorDecl(CXXConstructorDecl* );
    bool TraverseConstructorInitializer(CXXCtorInitializer*);

private:
    void CheckCompoundStmt(const FunctionDecl*, const CompoundStmt* );
    void CheckIfStmt(const FunctionDecl*, const IfStmt* );
    void CheckCXXForRangeStmt(const FunctionDecl*, const CXXForRangeStmt* );
    void CheckLoopDelete(const FunctionDecl*, const Stmt* );
    void CheckLoopDelete(const FunctionDecl*, const CXXDeleteExpr* );
    void CheckDeleteExpr(const FunctionDecl*, const CXXDeleteExpr*);
    void CheckParenExpr(const FunctionDecl*, const ParenExpr*);
    void CheckDeleteExpr(const FunctionDecl*, const CXXDeleteExpr*,
        const MemberExpr*, StringRef message);
    FunctionDecl const * mpCurrentFunctionDecl = nullptr;
    std::string fn;
};

static bool startswith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

bool UseUniquePtr::VisitFunctionDecl(const FunctionDecl* functionDecl)
{
    if (ignoreLocation(functionDecl))
        return true;
    if (isInUnoIncludeFile(functionDecl))
        return true;

    const CompoundStmt* compoundStmt = dyn_cast_or_null< CompoundStmt >( functionDecl->getBody() );
    if (!compoundStmt || compoundStmt->size() == 0)
        return true;

    CheckCompoundStmt(functionDecl, compoundStmt);

    return true;
}

/**
 * check for simple call to delete i.e. direct unconditional call, or if-guarded call
 */
void UseUniquePtr::CheckCompoundStmt(const FunctionDecl* functionDecl, const CompoundStmt* compoundStmt)
{
    for (auto i = compoundStmt->body_begin(); i != compoundStmt->body_end(); ++i)
    {
        if (auto cxxForRangeStmt = dyn_cast<CXXForRangeStmt>(*i))
            CheckCXXForRangeStmt(functionDecl, cxxForRangeStmt);
        else if (auto forStmt = dyn_cast<ForStmt>(*i))
            CheckLoopDelete(functionDecl, forStmt->getBody());
        else if (auto whileStmt = dyn_cast<WhileStmt>(*i))
            CheckLoopDelete(functionDecl, whileStmt->getBody());
        // check for unconditional inner compound statements
        else if (auto innerCompoundStmt = dyn_cast<CompoundStmt>(*i))
            CheckCompoundStmt(functionDecl, innerCompoundStmt);
        else if (auto deleteExpr = dyn_cast<CXXDeleteExpr>(*i))
            CheckDeleteExpr(functionDecl, deleteExpr);
        else if (auto parenExpr = dyn_cast<ParenExpr>(*i))
            CheckParenExpr(functionDecl, parenExpr);
        else if (auto ifStmt = dyn_cast<IfStmt>(*i))
            CheckIfStmt(functionDecl, ifStmt);
    }
}

// Check for conditional deletes like:
//     if (m_pField != nullptr) delete m_pField;
void UseUniquePtr::CheckIfStmt(const FunctionDecl* functionDecl, const IfStmt* ifStmt)
{
    auto cond = ifStmt->getCond()->IgnoreImpCasts();
    if (auto ifCondMemberExpr = dyn_cast<MemberExpr>(cond))
    {
        // ignore "if (bMine)"
        if (!loplugin::TypeCheck(ifCondMemberExpr->getType()).Pointer())
            return;
        // good
    }
    else if (auto binaryOp = dyn_cast<BinaryOperator>(cond))
    {
        if (!isa<MemberExpr>(binaryOp->getLHS()->IgnoreImpCasts()))
            return;
        if (!isa<CXXNullPtrLiteralExpr>(binaryOp->getRHS()->IgnoreImpCasts()))
            return;
        // good
    }
    else // ignore anything more complicated
        return;

    auto deleteExpr = dyn_cast<CXXDeleteExpr>(ifStmt->getThen());
    if (deleteExpr)
    {
        CheckDeleteExpr(functionDecl, deleteExpr);
        return;
    }

    auto parenExpr = dyn_cast<ParenExpr>(ifStmt->getThen());
    if (parenExpr)
    {
        CheckParenExpr(functionDecl, parenExpr);
        return;
    }

    auto ifThenCompoundStmt = dyn_cast<CompoundStmt>(ifStmt->getThen());
    if (!ifThenCompoundStmt)
        return;
    for (auto j = ifThenCompoundStmt->body_begin(); j != ifThenCompoundStmt->body_end(); ++j)
    {
        auto ifDeleteExpr = dyn_cast<CXXDeleteExpr>(*j);
        if (ifDeleteExpr)
            CheckDeleteExpr(functionDecl, ifDeleteExpr);
        ParenExpr const * parenExpr = dyn_cast<ParenExpr>(*j);
        if (parenExpr)
            CheckParenExpr(functionDecl, parenExpr);
    }
}

void UseUniquePtr::CheckDeleteExpr(const FunctionDecl* functionDecl, const CXXDeleteExpr* deleteExpr)
{
    auto deleteExprArg = deleteExpr->getArgument()->IgnoreParenImpCasts();


    if (const MemberExpr* memberExpr = dyn_cast<MemberExpr>(deleteExprArg))
    {
        // ignore delete static_cast<T>(p)->other;
        if (!isa<CXXThisExpr>(memberExpr->getBase()->IgnoreCasts()))
            return;
        // don't always own this
        if (fn == SRCDIR "/editeng/source/editeng/impedit2.cxx")
            return;
        // this member needs to get passed via a extern "C" API
        if (fn == SRCDIR "/sd/source/filter/sdpptwrp.cxx")
            return;
        // ownership complicated between this and the group
        if (fn == SRCDIR "/sc/source/core/data/formulacell.cxx")
            return;
        // linked list
        if (fn == SRCDIR "/sw/source/filter/html/parcss1.cxx")
            return;
        // linked list
        if (fn == SRCDIR "/sw/source/filter/writer/writer.cxx")
            return;
        // complicated
        if (fn == SRCDIR "/sc/source/filter/html/htmlpars.cxx")
            return;

        CheckDeleteExpr(functionDecl, deleteExpr, memberExpr,
            "unconditional call to delete on a member, should be using std::unique_ptr");
        return;
    }

    if (auto declRefExpr = dyn_cast<DeclRefExpr>(deleteExprArg))
    {
        if (isa<ParmVarDecl>(declRefExpr->getDecl()))
            ;// handled in VisitDeleteExpr
        else if (auto varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl()))
        {
            // ignore globals for now
            if (varDecl->hasGlobalStorage())
                return;
            // Ignore times when we are casting to init the var, normally indicates
            // some complex memory management.
            if (varDecl->getInit() && isa<ExplicitCastExpr>(varDecl->getInit()))
                return;

            if (startswith(fn, SRCDIR "/sal/qa/"))
                return;
            if (startswith(fn, SRCDIR "/comphelper/qa/"))
                return;
            if (startswith(fn, SRCDIR "/cppuhelper/qa/"))
                return;
            if (startswith(fn, SRCDIR "/libreofficekit/qa/"))
                return;
            if (startswith(fn, SRCDIR "/vcl/qa/"))
                return;
            if (startswith(fn, SRCDIR "/sc/qa/"))
                return;
            if (startswith(fn, SRCDIR "/sfx2/qa/"))
                return;
            if (startswith(fn, SRCDIR "/smoketest/"))
                return;
            if (startswith(fn, WORKDIR))
                return;
            // linked lists
            if (fn == SRCDIR "/vcl/source/gdi/regband.cxx")
                return;
            // this thing relies on explicit delete
            if (loplugin::TypeCheck(varDecl->getType()).Pointer().Class("VersionCompat").GlobalNamespace())
                return;
            if (loplugin::TypeCheck(varDecl->getType()).Pointer().Class("IMapCompat").GlobalNamespace())
                return;
            // passing data to gtk API and I can't figure out the types
            if (fn == SRCDIR "/vcl/unx/gtk3/gtk3gtkdata.cxx"
                || fn == SRCDIR "/vcl/unx/gtk/gtkdata.cxx")
                return;
            // sometimes this stuff is held by tools::SvRef, sometimes by std::unique_ptr .....
            if (fn == SRCDIR "/sot/source/unoolestorage/xolesimplestorage.cxx")
                return;
            // don't feel like messing with this chunk of sfx2
            if (fn == SRCDIR "/sfx2/source/appl/appinit.cxx")
                return;
            if (fn == SRCDIR "/svx/source/svdraw/svdobj.cxx")
                return;
            if (fn == SRCDIR "/svx/source/svdraw/svdmodel.cxx")
                return;
            // linked list
            if (fn == SRCDIR "/basic/source/comp/parser.cxx")
                return;
            if (fn == SRCDIR "/basic/source/runtime/runtime.cxx")
                return;
            // just horrible
            if (fn == SRCDIR "/svx/source/form/filtnav.cxx")
                return;
            // using clucene macros
            if (fn == SRCDIR "/helpcompiler/source/HelpSearch.cxx")
                return;
            // linked list
            if (fn == SRCDIR "/filter/source/graphicfilter/ios2met/ios2met.cxx")
                return;
            // no idea what this is trying to do
            if (fn == SRCDIR "/cui/source/customize/SvxMenuConfigPage.cxx")
                return;
            // I cannot follow the ownership of OSQLParseNode's
            if (fn == SRCDIR "/dbaccess/source/core/api/SingleSelectQueryComposer.cxx")
                return;
            if (fn == SRCDIR "/dbaccess/source/ui/querydesign/SelectionBrowseBox.cxx")
                return;
            // linked list
            if (fn == SRCDIR "/formula/source/core/api/FormulaCompiler.cxx")
                return;
            // smuggling data around via SvxFontListItem
            if (fn == SRCDIR "/extensions/source/propctrlr/fontdialog.cxx")
                return;
            // atomics
            if (fn == SRCDIR "/sc/source/ui/docshell/documentlinkmgr.cxx")
                return;
            // finicky
            if (fn == SRCDIR "/sc/source/core/data/stlpool.cxx")
                return;
            // macros
            if (fn == SRCDIR "/sc/source/core/tool/autoform.cxx")
                return;
            // unsure about ownership
            if (fn == SRCDIR "/xmlsecurity/source/framework/saxeventkeeperimpl.cxx")
                return;
            // ScTokenArray ownership complicated between this and the group
            if (fn == SRCDIR "/sc/source/core/data/formulacell.cxx")
                return;
            // macros
            if (fn == SRCDIR "/sw/source/core/doc/tblafmt.cxx")
                return;
            // more ScTokenArray
            if (fn == SRCDIR "/sc/source/ui/unoobj/tokenuno.cxx")
                return;
            // SwDoc::DelTextFormatColl
            if (fn == SRCDIR "/sw/source/core/doc/docfmt.cxx")
                return;
            // SwRootFrame::CalcFrameRects
            if (fn == SRCDIR "/sw/source/core/layout/trvlfrm.cxx")
                return;
            // crazy code
            if (fn == SRCDIR "/sw/source/core/undo/SwUndoPageDesc.cxx")
                return;
            // unsure about the SwLinePortion ownership
            if (fn == SRCDIR "/sw/source/core/text/itrform2.cxx")
                return;
            // can't follow the ownership
            if (fn == SRCDIR "/sw/source/filter/html/htmlatr.cxx")
                return;
            // SwTextFormatter::BuildMultiPortion complicated
            if (fn == SRCDIR "/sw/source/core/text/pormulti.cxx")
                return;
            // SwXMLExport::ExportTableLines
            if (fn == SRCDIR "/sw/source/filter/xml/xmltble.cxx")
                return;
            // SwPagePreview::~SwPagePreview
            if (fn == SRCDIR "/sw/source/uibase/uiview/pview.cxx")
                return;

            report(
                DiagnosticsEngine::Warning,
                "unconditional call to delete on a var, should be using std::unique_ptr",
                compat::getBeginLoc(deleteExpr))
                << deleteExpr->getSourceRange();
            report(
                DiagnosticsEngine::Note,
                "var is here",
                compat::getBeginLoc(varDecl))
                << varDecl->getSourceRange();
            return;
        }
    }

    const ArraySubscriptExpr* arrayExpr = dyn_cast<ArraySubscriptExpr>(deleteExprArg);
    if (arrayExpr)
    {
       auto baseMemberExpr = dyn_cast<MemberExpr>(arrayExpr->getBase()->IgnoreParenImpCasts());
       if (baseMemberExpr)
            CheckDeleteExpr(functionDecl, deleteExpr, baseMemberExpr,
                "unconditional call to delete on an array member, should be using std::unique_ptr");
    }
}

/**
 * Look for DELETEZ expressions.
 */
void UseUniquePtr::CheckParenExpr(const FunctionDecl* functionDecl, const ParenExpr* parenExpr)
{
    auto binaryOp = dyn_cast<BinaryOperator>(parenExpr->getSubExpr());
    if (!binaryOp || binaryOp->getOpcode() != BO_Comma)
        return;
    auto deleteExpr = dyn_cast<CXXDeleteExpr>(binaryOp->getLHS());
    if (!deleteExpr)
        return;
    CheckDeleteExpr(functionDecl, deleteExpr);
}

void UseUniquePtr::CheckDeleteExpr(const FunctionDecl* functionDecl, const CXXDeleteExpr* deleteExpr,
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
    if (auto methodDecl = dyn_cast<CXXMethodDecl>(functionDecl))
        if (fieldDecl->getParent() != methodDecl->getParent() )
            return;

    if (ignoreLocation(fieldDecl))
        return;
    // to ignore things like the CPPUNIT macros
    if (startswith(fn, WORKDIR "/"))
        return;
    // passes and stores pointers to member fields
    if (fn == SRCDIR "/sot/source/sdstor/stgdir.hxx")
        return;
    // something platform-specific
    if (fn == SRCDIR "/hwpfilter/source/htags.h")
        return;
    // passes pointers to member fields
    if (fn == SRCDIR "/sd/inc/sdpptwrp.hxx")
        return;
    // @TODO intrusive linked-lists here, with some trickiness
    if (fn == SRCDIR "/sw/source/filter/html/parcss1.hxx")
        return;
    // @TODO SwDoc has some weird ref-counting going on
    if (fn == SRCDIR "/sw/inc/shellio.hxx")
        return;
    // @TODO it's sharing pointers with another class
    if (fn == SRCDIR "/sc/inc/formulacell.hxx")
        return;
    // some weird stuff going on here around struct Entity
    if (startswith(fn, SRCDIR "/sax/"))
        return;
    if (startswith(fn, SRCDIR "/include/sax/"))
        return;
    // manipulation of tree structures ie. StgAvlNode, don't lend themselves to std::unique_ptr
    if (startswith(fn, SRCDIR "/sot/"))
        return;
    if (startswith(fn, SRCDIR "/include/sot/"))
        return;
    // the std::vector is being passed to another class
    if (fn == SRCDIR "/sfx2/source/explorer/nochaos.cxx")
        return;
    auto tc = loplugin::TypeCheck(fieldDecl->getType());
    // these sw::Ring based classes do not lend themselves to std::unique_ptr management
    if (tc.Pointer().Class("SwNodeIndex").GlobalNamespace() || tc.Pointer().Class("SwShellTableCursor").GlobalNamespace()
        || tc.Pointer().Class("SwBlockCursor").GlobalNamespace() || tc.Pointer().Class("SwVisibleCursor").GlobalNamespace()
        || tc.Pointer().Class("SwShellCursor").GlobalNamespace())
        return;
    // there is a loop in ~ImplPrnQueueList deleting stuff on a global data structure
    if (fn == SRCDIR "/vcl/inc/print.h")
        return;
    // painful linked list
    if (fn == SRCDIR "/basic/source/inc/runtime.hxx")
        return;
    // not sure how the node management is working here
    if (fn == SRCDIR "/i18npool/source/localedata/saxparser.cxx")
        return;
    // has a pointer that it only sometimes owns
    if (fn == SRCDIR "/editeng/source/editeng/impedit.hxx")
        return;

    report(
        DiagnosticsEngine::Warning,
        message,
        compat::getBeginLoc(deleteExpr))
        << deleteExpr->getSourceRange();
    report(
        DiagnosticsEngine::Note,
        "member is here",
        compat::getBeginLoc(fieldDecl))
        << fieldDecl->getSourceRange();
}

void UseUniquePtr::CheckLoopDelete(const FunctionDecl* functionDecl, const Stmt* bodyStmt)
{
    if (auto deleteExpr = dyn_cast<CXXDeleteExpr>(bodyStmt))
        CheckLoopDelete(functionDecl, deleteExpr);
    else if (auto compoundStmt = dyn_cast<CompoundStmt>(bodyStmt))
    {
        for (auto i = compoundStmt->body_begin(); i != compoundStmt->body_end(); ++i)
        {
            if (auto deleteExpr = dyn_cast<CXXDeleteExpr>(*i))
                CheckLoopDelete(functionDecl, deleteExpr);
        }
    }
}

void UseUniquePtr::CheckLoopDelete(const FunctionDecl* functionDecl, const CXXDeleteExpr* deleteExpr)
{
    const MemberExpr* memberExpr = nullptr;
    const Expr* subExpr = deleteExpr->getArgument();
    // drill down looking for a MemberExpr
    for (;;)
    {
        subExpr = subExpr->IgnoreImpCasts();
        if ((memberExpr = dyn_cast<MemberExpr>(subExpr)))
            break;
        else if (auto arraySubscriptExpr = dyn_cast<ArraySubscriptExpr>(subExpr))
            subExpr = arraySubscriptExpr->getBase();
        else if (auto cxxOperatorCallExpr = dyn_cast<CXXOperatorCallExpr>(subExpr))
        {
            // look for deletes of an iterator object where the iterator is over a member field
            if (auto declRefExpr = dyn_cast<DeclRefExpr>(cxxOperatorCallExpr->getArg(0)->IgnoreImpCasts()))
            {
                if (auto varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl()))
                {
                    auto init = varDecl->getInit();
                    if (init)
                    {
                        if (auto x = dyn_cast<ExprWithCleanups>(init))
                            init = x->getSubExpr();
                        if (auto x = dyn_cast<CXXBindTemporaryExpr>(init))
                            init = x->getSubExpr();
                        if (auto x = dyn_cast<CXXMemberCallExpr>(init))
                            init = x->getImplicitObjectArgument();
                        memberExpr = dyn_cast<MemberExpr>(init);
                    }
                    break;
                }
            }
            // look for deletes like "delete m_pField[0]"
            if (cxxOperatorCallExpr->getOperator() == OO_Subscript)
            {
                memberExpr = dyn_cast<MemberExpr>(cxxOperatorCallExpr->getArg(0));
            }
            break;
        }
        else
           break;
    }
    if (!memberExpr)
        return;

    // OStorage_Impl::Commit very complicated ownership passing going on
    if (fn == SRCDIR "/package/source/xstor/xstorage.cxx")
        return;
    // complicated
    if (fn == SRCDIR "/vcl/source/gdi/print.cxx")
        return;
    // linked list
    if (fn == SRCDIR "/basic/source/runtime/runtime.cxx")
        return;

    CheckDeleteExpr(functionDecl, deleteExpr, memberExpr, "rather manage with std::some_container<std::unique_ptr<T>>");
}

void UseUniquePtr::CheckCXXForRangeStmt(const FunctionDecl* functionDecl, const CXXForRangeStmt* cxxForRangeStmt)
{
    CXXDeleteExpr const * deleteExpr = nullptr;
    if (auto compoundStmt = dyn_cast<CompoundStmt>(cxxForRangeStmt->getBody()))
    {
        for (auto i = compoundStmt->body_begin(); i != compoundStmt->body_end(); ++i)
            if ((deleteExpr = dyn_cast<CXXDeleteExpr>(*i)))
                break;
    }
    else
        deleteExpr = dyn_cast<CXXDeleteExpr>(cxxForRangeStmt->getBody());
    if (!deleteExpr)
        return;

    if (auto memberExpr = dyn_cast<MemberExpr>(cxxForRangeStmt->getRangeInit()))
    {
        auto fieldDecl = dyn_cast<FieldDecl>(memberExpr->getMemberDecl());
        if (!fieldDecl)
            return;

        // appears to just randomly leak stuff, and it involves some lex/yacc stuff
        if (fn == SRCDIR "/idlc/source/aststack.cxx")
            return;
        // complicated
        if (fn == SRCDIR "/vcl/source/gdi/print.cxx")
            return;

        CheckDeleteExpr(functionDecl, deleteExpr, memberExpr, "rather manage this with std::some_container<std::unique_ptr<T>>");
    }

    if (auto declRefExpr = dyn_cast<DeclRefExpr>(cxxForRangeStmt->getRangeInit()))
    {
        auto varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl());
        if (!varDecl)
            return;

        // don't feel like messing with this part of sfx2
        if (fn == SRCDIR "/sfx2/source/control/msgpool.cxx")
            return;
        if (fn == SRCDIR "/sfx2/source/doc/doctemplates.cxx")
            return;
        // lex/yacc
        if (fn == SRCDIR "/hwpfilter/source/grammar.cxx")
            return;
        if (fn == SRCDIR "/hwpfilter/source/formula.cxx")
            return;
        // no idea why, but ui tests crash afterwards in weird ways
        if (fn == SRCDIR "/svtools/source/control/roadmap.cxx")
            return;
        // sometimes it owns it, sometimes it does not
        if (fn == SRCDIR "/dbaccess/source/ui/misc/WCopyTable.cxx")
            return;
        // SfxPoolItem array
        if (fn == SRCDIR "/dbaccess/source/ui/misc/UITools.cxx")
            return;
        // SfxPoolItem array
        if (fn == SRCDIR "/sw/source/core/bastyp/init.cxx")
            return;
        // SfxPoolItem array
        if (fn == SRCDIR "/reportdesign/source/ui/misc/UITools.cxx")
            return;
        // SfxPoolItem array
        if (fn == SRCDIR "/reportdesign/source/ui/report/ReportController.cxx")
            return;

        report(
            DiagnosticsEngine::Warning,
            "rather manage this var with std::some_container<std::unique_ptr<T>>",
            compat::getBeginLoc(deleteExpr))
            << deleteExpr->getSourceRange();
        report(
            DiagnosticsEngine::Note,
            "var is here",
            compat::getBeginLoc(varDecl))
            << varDecl->getSourceRange();
    }
}

bool UseUniquePtr::VisitCompoundStmt(const CompoundStmt* compoundStmt)
{
    if (ignoreLocation(compoundStmt))
        return true;
    if (isInUnoIncludeFile(compat::getBeginLoc(compoundStmt)))
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

    auto declRefExpr = dyn_cast<DeclRefExpr>(deleteExpr->getArgument()->IgnoreParenImpCasts());
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
    if (compat::getBeginLoc(varDecl) < compat::getBeginLoc(compoundStmt))
        return true;

    report(
        DiagnosticsEngine::Warning,
        "deleting a local variable at the end of a block, is a sure sign it should be using std::unique_ptr for that var",
        compat::getBeginLoc(deleteExpr))
        << deleteExpr->getSourceRange();
    report(
        DiagnosticsEngine::Note,
        "var is here",
        compat::getBeginLoc(varDecl))
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

bool UseUniquePtr::TraverseCXXMethodDecl(CXXMethodDecl* methodDecl)
{
    if (ignoreLocation(methodDecl))
        return true;

    auto oldCurrent = mpCurrentFunctionDecl;
    mpCurrentFunctionDecl = methodDecl;
    bool ret = RecursiveASTVisitor::TraverseCXXMethodDecl(methodDecl);
    mpCurrentFunctionDecl = oldCurrent;

    return ret;
}

bool UseUniquePtr::TraverseCXXConstructorDecl(CXXConstructorDecl* methodDecl)
{
    if (ignoreLocation(methodDecl))
        return true;

    auto oldCurrent = mpCurrentFunctionDecl;
    mpCurrentFunctionDecl = methodDecl;
    bool ret = RecursiveASTVisitor::TraverseCXXConstructorDecl(methodDecl);
    mpCurrentFunctionDecl = oldCurrent;

    return ret;
}

bool UseUniquePtr::VisitCXXDeleteExpr(const CXXDeleteExpr* deleteExpr)
{
    if (!mpCurrentFunctionDecl)
        return true;
    if (ignoreLocation(mpCurrentFunctionDecl))
        return true;
    if (isInUnoIncludeFile(compat::getBeginLoc(mpCurrentFunctionDecl->getCanonicalDecl())))
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
    if (auto cxxMethodDecl = dyn_cast<CXXMethodDecl>(mpCurrentFunctionDecl))
    {
        // include/o3tl/deleter.hxx
        if (cxxMethodDecl->getParent()->getName() == "default_delete")
            return true;
        // TODO Bitmap::ReleaseAccess
        // Tricky because it reverberates through other code and requires that BitmapWriteAccess move into /include again
        if (cxxMethodDecl->getParent()->getName() == "Bitmap")
            return true;
        // TODO virtual ones are much trickier, leave for later
        if (cxxMethodDecl->isVirtual())
            return true;
        // sw/inc/unobaseclass.hxx holds SolarMutex while deleting
        if (cxxMethodDecl->getParent()->getName() == "UnoImplPtrDeleter")
            return true;
    }

    auto declRefExpr = dyn_cast<DeclRefExpr>(deleteExpr->getArgument()->IgnoreParenImpCasts());
    if (!declRefExpr)
        return true;
    auto varDecl = dyn_cast<ParmVarDecl>(declRefExpr->getDecl());
    if (!varDecl)
        return true;

    // StgAvlNode::Remove
    if (fn == SRCDIR "/sot/source/sdstor/stgavl.cxx")
        return true;
    // SfxItemPool::ReleaseDefaults and SfxItemPool::Free
    if (fn == SRCDIR "/svl/source/items/itempool.cxx")
        return true;
    // SwContourCache
    if (fn == SRCDIR "/sw/source/core/text/txtfly.cxx")
        return true;
    // too messy to cope with the SQL parser
    if (fn == SRCDIR "/connectivity/source/parse/sqlnode.cxx")
        return true;
    // I can't figure out the ownership of the SfxMedium in the call site(s)
    if (fn == SRCDIR "/sfx2/source/doc/sfxbasemodel.cxx")
        return true;
    // pointer passed via IMPL_LINK
    if (fn == SRCDIR "/sfx2/source/control/dispatch.cxx")
        return true;
    // NavigatorTreeModel::Remove
    if (fn == SRCDIR "/svx/source/form/navigatortreemodel.cxx")
        return true;
    // SdrModel::AddUndo
    if (fn == SRCDIR "/svx/source/svdraw/svdmodel.cxx")
        return true;
    // undo callback
    if (fn == SRCDIR "/basctl/source/basicide/baside3.cxx")
        return true;
    // ActualizeProgress::TimeoutHdl
    if (fn == SRCDIR "/cui/source/dialogs/cuigaldlg.cxx")
        return true;
    // ToolbarSaveInData::RemoveToolbar
    if (fn == SRCDIR "/cui/source/customize/cfg.cxx")
        return true;
    // OStorage_Impl::RemoveElement very complicated ownership passing going on
    if (fn == SRCDIR "/package/source/xstor/xstorage.cxx")
        return true;
    // actually held via shared_ptr, uses protected deleter object
    if (fn == SRCDIR "/sd/source/ui/framework/tools/FrameworkHelper.cxx")
        return true;
    // actually held via shared_ptr, uses protected deleter object
    if (fn == SRCDIR "/sd/source/ui/presenter/CanvasUpdateRequester.cxx")
        return true;
    // actually held via shared_ptr, uses protected deleter object
    if (fn == SRCDIR "/sd/source/ui/slidesorter/cache/SlsPageCacheManager.cxx")
        return true;
    // actually held via shared_ptr, uses protected deleter object
    if (fn == SRCDIR "/sd/source/ui/sidebar/MasterPageContainer.cxx")
        return true;
    // actually held via shared_ptr, uses protected deleter object
    if (fn == SRCDIR "/sd/source/ui/tools/TimerBasedTaskExecution.cxx")
        return true;
    // actually held via shared_ptr, uses protected deleter object
    if (fn == SRCDIR "/sd/source/ui/view/ViewShellImplementation.cxx")
        return true;
    // ScBroadcastAreaSlot::StartListeningArea manual ref-counting of ScBroadcastArea
    if (fn == SRCDIR "/sc/source/core/data/bcaslot.cxx")
        return true;
    // ScDrawLayer::AddCalcUndo undo stuff
    if (fn == SRCDIR "/sc/source/core/data/drwlayer.cxx")
        return true;
    // ScTable::SetFormulaCell
    if (fn == SRCDIR "/sc/source/core/data/table2.cxx")
        return true;
    // ScDocument::SetFormulaCell
    if (fn == SRCDIR "/sc/source/core/data/documen2.cxx")
        return true;
    // RemoveEditAttribsHandler, stored in mdds block
    if (fn == SRCDIR "/sc/source/core/data/column2.cxx")
        return true;
    // just turns into a mess
    if (fn == SRCDIR "/sc/source/ui/Accessibility/AccessibleDocument.cxx")
        return true;
    // SwCache::DeleteObj, linked list
    if (fn == SRCDIR "/sw/source/core/bastyp/swcache.cxx")
        return true;
    // SAXEventKeeperImpl::smashBufferNode
    if (fn == SRCDIR "/xmlsecurity/source/framework/saxeventkeeperimpl.cxx")
        return true;
    // SwDoc::DeleteExtTextInput
    if (fn == SRCDIR "/sw/source/core/doc/extinput.cxx")
        return true;
    // SwDoc::DelSectionFormat
    if (fn == SRCDIR "/sw/source/core/docnode/ndsect.cxx")
        return true;
    // SwFrame::DestroyFrame
    if (fn == SRCDIR "/sw/source/core/layout/ssfrm.cxx")
        return true;
    // SwGluePortion::Join
    if (fn == SRCDIR "/sw/source/core/text/porglue.cxx")
        return true;
    // SwDoc::DelFrameFormat
    if (fn == SRCDIR "/sw/source/core/doc/docfmt.cxx")
        return true;
    // SwTextAttr::Destroy
    if (fn == SRCDIR "/sw/source/core/txtnode/txatbase.cxx")
        return true;
    // IMPL_LINK( SwDoc, AddDrawUndo, SdrUndoAction *, pUndo, void )
    if (fn == SRCDIR "/sw/source/core/undo/undraw.cxx")
        return true;
    // SwHTMLParser::EndAttr
    if (fn == SRCDIR "/sw/source/filter/html/swhtml.cxx")
        return true;
    // SwGlossaryHdl::Expand sometimes the pointer is owned, sometimes it is not
    if (fn == SRCDIR "/sw/source/uibase/dochdl/gloshdl.cxx")
        return true;
    // SwWrtShell::Insert only owned sometimes
    if (fn == SRCDIR "/sw/source/uibase/wrtsh/wrtsh1.cxx")
        return true;
    // NodeArrayDeleter
    if (fn == SRCDIR "/unoxml/source/rdf/librdf_repository.cxx")
        return true;
    // SmCursor::LineToList ran out of enthusiasm to rework the node handling
    if (fn == SRCDIR "/starmath/source/cursor.cxx")
        return true;
    // XMLEventOASISTransformerContext::FlushEventMap
    if (fn == SRCDIR "/xmloff/source/transform/EventOASISTContext.cxx")
        return true;
    // XMLEventOOoTransformerContext::FlushEventMap
    if (fn == SRCDIR "/xmloff/source/transform/EventOOoTContext.cxx")
        return true;

    /*
    Sometimes we can pass the param as std::unique_ptr<T>& or std::unique_ptr, sometimes the method
    just needs to be inlined, which normally exposes more simplification.
    */
    report(
        DiagnosticsEngine::Warning,
        "calling delete on a pointer param, should be either whitelisted or simplified",
        compat::getBeginLoc(deleteExpr))
        << deleteExpr->getSourceRange();
    return true;
}

bool UseUniquePtr::TraverseConstructorInitializer(CXXCtorInitializer * ctorInit)
{
    if (!ctorInit->getSourceLocation().isValid() || ignoreLocation(ctorInit->getSourceLocation()))
        return true;
    if (!ctorInit->getMember())
        return true;
    if (!loplugin::TypeCheck(ctorInit->getMember()->getType()).Class("unique_ptr").StdNamespace())
        return true;
    auto constructExpr = dyn_cast_or_null<CXXConstructExpr>(ctorInit->getInit());
    if (!constructExpr || constructExpr->getNumArgs() == 0)
        return true;
    auto init = constructExpr->getArg(0)->IgnoreImpCasts();
    if (!isa<DeclRefExpr>(init))
        return true;

    StringRef fn = getFileNameOfSpellingLoc(compiler.getSourceManager().getSpellingLoc(ctorInit->getSourceLocation()));
    // don't feel like fiddling with the yacc parser
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/idlc/"))
        return true;
    // cannot change URE
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/cppu/source/helper/purpenv/helper_purpenv_Environment.cxx"))
        return true;


    report(
        DiagnosticsEngine::Warning,
        "should be passing via std::unique_ptr param",
        ctorInit->getSourceLocation())
        << ctorInit->getSourceRange();
    return RecursiveASTVisitor<UseUniquePtr>::TraverseConstructorInitializer(ctorInit);
}

loplugin::Plugin::Registration< UseUniquePtr > X("useuniqueptr", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
