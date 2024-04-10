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
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "config_clang.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

// This checker aims to pull buried assignments out of complex expressions,
// where they are quite hard to notice amidst the other conditional logic.

namespace
{
class BuriedAssign : public loplugin::FilteringPlugin<BuriedAssign>
{
public:
    explicit BuriedAssign(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);

        // code where I don't have a better alternative
        if (fn == SRCDIR "/sal/osl/unx/profile.cxx")
            return;
        if (fn == SRCDIR "/sal/rtl/uri.cxx")
            return;
        if (fn == SRCDIR "/sal/osl/unx/process.cxx")
            return;
        if (fn == SRCDIR "/sal/rtl/bootstrap.cxx")
            return;
        if (fn == SRCDIR "/i18npool/source/textconversion/genconv_dict.cxx")
            return;
        if (fn == SRCDIR "/soltools/cpp/_macro.c")
            return;
        if (fn == SRCDIR "/stoc/source/inspect/introspection.cxx")
            return;
        if (fn == SRCDIR "/tools/source/fsys/urlobj.cxx")
            return;
        if (fn == SRCDIR "/sax/source/tools/fastserializer.cxx")
            return;
        if (fn == SRCDIR "/svl/source/crypto/cryptosign.cxx")
            return;
        if (fn == SRCDIR "/svl/source/numbers/zforfind.cxx")
            return;
        if (fn == SRCDIR "/svl/source/numbers/zformat.cxx")
            return;
        if (fn == SRCDIR "/svl/source/numbers/zforscan.cxx")
            return;
        if (fn == SRCDIR "/svl/source/numbers/zforlist.cxx")
            return;
        if (fn == SRCDIR "/vcl/source/window/debugevent.cxx")
            return;
        if (fn == SRCDIR "/vcl/source/control/scrbar.cxx")
            return;
        if (fn == SRCDIR "/vcl/source/gdi/bitmap3.cxx")
            return;
        if (fn == SRCDIR "/vcl/source/window/menu.cxx")
            return;
        if (fn == SRCDIR "/vcl/source/fontsubset/sft.cxx")
            return;
        if (fn == SRCDIR "/vcl/unx/generic/print/prtsetup.cxx")
            return;
        if (fn == SRCDIR "/svtools/source/brwbox/brwbox1.cxx")
            return;
        if (fn == SRCDIR "/svtools/source/control/valueset.cxx")
            return;
        if (fn == SRCDIR "/basic/source/runtime/iosys.cxx")
            return;
        if (fn == SRCDIR "/basic/source/runtime/runtime.cxx")
            return;
        if (fn == SRCDIR "/basic/source/sbx/sbxvalue.cxx")
            return;
        if (fn == SRCDIR "/basic/source/sbx/sbxvalue.cxx")
            return;
        if (fn == SRCDIR "/sfx2/source/dialog/templdlg.cxx")
            return;
        if (fn == SRCDIR "/sfx2/source/view/viewfrm.cxx")
            return;
        if (fn == SRCDIR "/connectivity/source/commontools/dbtools.cxx")
            return;
        if (fn == SRCDIR "/xmloff/source/style/xmlnumfi.cxx")
            return;
        if (fn == SRCDIR "/xmloff/source/style/xmlnumfe .cxx")
            return;
        if (fn == SRCDIR "/editeng/source/items/textitem.cxx")
            return;
        if (fn == SRCDIR "/editeng/source/rtf/rtfitem.cxx")
            return;
        if (fn == SRCDIR "/editeng/source/rtf/svxrtf.cxx")
            return;
        if (fn == SRCDIR "/editeng/source/misc/svxacorr.cxx")
            return;
        if (fn == SRCDIR "/svx/source/items/numfmtsh.cxx")
            return;
        if (fn == SRCDIR "/svx/source/dialog/hdft.cxx")
            return;
        if (fn == SRCDIR "/cui/source/dialogs/insdlg.cxx")
            return;
        if (fn == SRCDIR "/cui/source/tabpages/paragrph.cxx")
            return;
        if (fn == SRCDIR "/cui/source/tabpages/page.cxx")
            return;
        if (fn == SRCDIR "/cui/source/tabpages/border.cxx")
            return;
        if (fn == SRCDIR "/cui/source/tabpages/chardlg.cxx")
            return;
        if (fn == SRCDIR "/cui/source/tabpages/numpages.cxx")
            return;
        if (fn == SRCDIR "/cui/source/dialogs/SpellDialog.cxx")
            return;
        if (fn == SRCDIR "/oox/source/drawingml/diagram/diagramlayoutatoms.cxx")
            return;
        if (fn == SRCDIR "/dbaccess/source/core/dataaccess/intercept.cxx")
            return;
        if (fn == SRCDIR "/sw/writerfilter/dmapper/DomainMapper.cxx")
            return;
        if (fn == SRCDIR "/sw/writerfilter/dmapper/DomainMapper_Impl.cxx")
            return;
        if (fn == SRCDIR "/lotuswordpro/source/filter/lwptablelayout.cxx")
            return;
        if (fn == SRCDIR "/i18npool/source/characterclassification/cclass_unicode_parser.cxx")
            return;
        if (fn == SRCDIR "/sd/source/filter/eppt/pptx-animations.cxx")
            return;
        if (fn == SRCDIR "/sc/source/core/tool/address.cxx")
            return;
        if (fn == SRCDIR "/sc/source/core/tool/interpr1.cxx")
            return;
        if (fn == SRCDIR "/sc/source/core/tool/interpr4.cxx")
            return;
        if (fn == SRCDIR "/sc/source/core/tool/interpr5.cxx")
            return;
        if (fn == SRCDIR "/sc/source/core/tool/compiler.cxx")
            return;
        if (fn == SRCDIR "/sc/source/core/data/table4.cxx")
            return;
        if (fn == SRCDIR "/sc/source/ui/drawfunc/fudraw.cxx")
            return;
        if (fn == SRCDIR "/sc/source/filter/xml/xmlcelli.cxx")
            return;
        if (fn == SRCDIR "/sc/source/ui/miscdlgs/crnrdlg.cxx")
            return;
        if (fn == SRCDIR "/sc/source/ui/app/inputwin.cxx")
            return;
        if (fn == SRCDIR "/sc/source/ui/view/viewfun2.cxx")
            return;
        if (fn == SRCDIR "/sc/source/ui/unoobj/docuno.cxx")
            return;
        if (fn == SRCDIR "/sc/source/ui/view/gridwin.cxx")
            return;
        if (fn == SRCDIR "/sw/source/core/crsr/callnk.cxx")
            return;
        if (fn == SRCDIR "/sw/source/core/crsr/findtxt.cxx")
            return;
        if (fn == SRCDIR "/sw/source/core/crsr/crsrsh.cxx")
            return;
        if (fn == SRCDIR "/sw/source/core/crsr/crstrvl.cxx")
            return;
        if (fn == SRCDIR "/sw/source/core/doc/doccomp.cxx")
            return;
        if (fn == SRCDIR "/sw/source/core/doc/docedt.cxx")
            return;
        if (fn == SRCDIR "/sw/source/core/doc/docfly.cxx")
            return;
        if (fn == SRCDIR "/sw/source/core/doc/DocumentRedlineManager.cxx")
            return;
        if (fn == SRCDIR "/sw/source/core/doc/notxtfrm.cxx")
            return;
        if (fn == SRCDIR "/sw/source/core/docnode/node.cxx")
            return;
        if (fn == SRCDIR "/sw/source/core/layout/ftnfrm.cxx")
            return;
        if (fn == SRCDIR "/sw/source/core/table/swtable.cxx")
            return;
        if (fn == SRCDIR "/sw/source/core/unocore/unoframe.cxx")
            return;
        if (fn == SRCDIR "/sw/source/filter/xml/xmlimp.cxx")
            return;
        if (fn == SRCDIR "/sw/source/uibase/docvw/edtwin.cxx")
            return;
        if (fn == SRCDIR "/sw/source/uibase/shells/langhelper.cxx")
            return;
        if (fn == SRCDIR "/sw/source/uibase/shells/tabsh.cxx")
            return;
        if (fn == SRCDIR "/sw/source/uibase/shells/textsh1.cxx")
            return;
        if (fn == SRCDIR "/sw/source/uibase/uiview/view2.cxx")
            return;
        if (fn == SRCDIR "/starmath/source/mathtype.cxx")
            return;
        if (fn == SRCDIR "/starmath/source/mathmlexport.cxx")
            return;
        if (fn == SRCDIR "/starmath/source/view.cxx")
            return;
        if (fn == SRCDIR "/xmlhelp/source/treeview/tvread.cxx")
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitBinaryOperator(BinaryOperator const*);
    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const*);
    bool VisitCompoundStmt(CompoundStmt const*);
    bool VisitIfStmt(IfStmt const*);
    bool VisitLabelStmt(LabelStmt const*);
    bool VisitForStmt(ForStmt const*);
    bool VisitCXXForRangeStmt(CXXForRangeStmt const*);
    bool VisitWhileStmt(WhileStmt const*);
    bool VisitDoStmt(DoStmt const*);
    bool VisitCaseStmt(CaseStmt const*);
    bool VisitDefaultStmt(DefaultStmt const*);
    bool VisitVarDecl(VarDecl const*);
    bool VisitCXXFoldExpr(CXXFoldExpr const*);

private:
    void MarkIfAssignment(Stmt const*);
    void MarkAll(Stmt const*);
    void MarkConditionForControlLoops(Expr const*);

    std::unordered_set<const Stmt*> m_handled;
};

static bool isAssignmentOp(clang::BinaryOperatorKind op)
{
    // We ignore BO_ShrAssign i.e. >>= because we use that everywhere for
    // extracting data from css::uno::Any
    return op == BO_Assign || op == BO_MulAssign || op == BO_DivAssign || op == BO_RemAssign
           || op == BO_AddAssign || op == BO_SubAssign || op == BO_ShlAssign || op == BO_AndAssign
           || op == BO_XorAssign || op == BO_OrAssign;
}

static bool isAssignmentOp(clang::OverloadedOperatorKind Opc)
{
    // Same logic as CXXOperatorCallExpr::isAssignmentOp(), which our supported clang
    // doesn't have yet.
    // Except that we ignore OO_GreaterGreaterEqual i.e. >>= because we use that everywhere for
    // extracting data from css::uno::Any
    return Opc == OO_Equal || Opc == OO_StarEqual || Opc == OO_SlashEqual || Opc == OO_PercentEqual
           || Opc == OO_PlusEqual || Opc == OO_MinusEqual || Opc == OO_LessLessEqual
           || Opc == OO_AmpEqual || Opc == OO_CaretEqual || Opc == OO_PipeEqual;
}

static const Expr* IgnoreImplicitAndConversionOperator(const Expr* expr)
{
    expr = expr->IgnoreImplicit();
    if (auto memberCall = dyn_cast<CXXMemberCallExpr>(expr))
    {
        if (auto conversionDecl = dyn_cast_or_null<CXXConversionDecl>(memberCall->getMethodDecl()))
        {
            if (!conversionDecl->isExplicit())
                expr = memberCall->getImplicitObjectArgument()->IgnoreImplicit();
        }
    }
    return expr;
}

bool BuriedAssign::VisitBinaryOperator(BinaryOperator const* binaryOp)
{
    if (ignoreLocation(binaryOp))
        return true;
    if (binaryOp->getBeginLoc().isMacroID())
        return true;
    if (!isAssignmentOp(binaryOp->getOpcode()))
        return true;
    auto expr = IgnoreImplicitAndConversionOperator(binaryOp->getRHS());
    if (auto rhs = dyn_cast<BinaryOperator>(expr))
    {
        // Ignore chained assignment.
        // TODO limit this to only ordinary assignment
        if (isAssignmentOp(rhs->getOpcode()))
            m_handled.insert(rhs);
    }
    else if (auto rhs = dyn_cast<CXXOperatorCallExpr>(expr))
    {
        // Ignore chained assignment.
        // TODO limit this to only ordinary assignment
        if (isAssignmentOp(rhs->getOperator()))
            m_handled.insert(rhs);
    }
    else if (auto cxxConstruct = dyn_cast<CXXConstructExpr>(expr))
    {
        if (cxxConstruct->getNumArgs() == 1)
            MarkIfAssignment(cxxConstruct->getArg(0));
    }
    if (!m_handled.insert(binaryOp).second)
        return true;

    // assignment in constructor
    StringRef aFileName = getFilenameOfLocation(
        compiler.getSourceManager().getSpellingLoc(binaryOp->getBeginLoc()));
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/comphelper/flagguard.hxx"))
        return true;

    report(DiagnosticsEngine::Warning, "buried assignment, rather put on own line",
           binaryOp->getBeginLoc())
        << binaryOp->getSourceRange();
    //getParentStmt(getParentStmt(getParentStmt(getParentStmt(getParentStmt(getParentStmt(binaryOp))))))->dump();
    return true;
}

bool BuriedAssign::VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* cxxOper)
{
    if (ignoreLocation(cxxOper))
        return true;
    if (cxxOper->getBeginLoc().isMacroID())
        return true;
    if (!isAssignmentOp(cxxOper->getOperator()))
        return true;
    auto expr = IgnoreImplicitAndConversionOperator(cxxOper->getArg(1));
    if (auto rhs = dyn_cast<BinaryOperator>(expr))
    {
        // Ignore chained assignment.
        // TODO limit this to only ordinary assignment
        if (isAssignmentOp(rhs->getOpcode()))
            m_handled.insert(rhs);
    }
    else if (auto rhs = dyn_cast<CXXOperatorCallExpr>(expr))
    {
        // Ignore chained assignment.
        // TODO limit this to only ordinary assignment
        if (isAssignmentOp(rhs->getOperator()))
            m_handled.insert(rhs);
    }
    else if (auto cxxConstruct = dyn_cast<CXXConstructExpr>(expr))
    {
        if (cxxConstruct->getNumArgs() == 1)
            MarkIfAssignment(cxxConstruct->getArg(0));
    }
    if (!m_handled.insert(cxxOper).second)
        return true;
    report(DiagnosticsEngine::Warning, "buried assignment, rather put on own line",
           cxxOper->getBeginLoc())
        << cxxOper->getSourceRange();
    //getParentStmt(getParentStmt(getParentStmt(getParentStmt(getParentStmt(cxxOper)))))->dump();
    return true;
}

bool BuriedAssign::VisitCompoundStmt(CompoundStmt const* compoundStmt)
{
    if (ignoreLocation(compoundStmt))
        return true;
    for (auto i = compoundStmt->child_begin(); i != compoundStmt->child_end(); ++i)
    {
        if (auto expr = dyn_cast<Expr>(*i))
        {
            expr = expr->IgnoreImplicit();
            if (auto binaryOp = dyn_cast<BinaryOperator>(expr))
            {
                // ignore comma-chained statements at this level
                if (binaryOp->getOpcode() == BO_Comma)
                {
                    MarkIfAssignment(binaryOp->getLHS());
                    MarkIfAssignment(binaryOp->getRHS());
                    continue;
                }
            }
            MarkIfAssignment(expr);
        }
    }
    return true;
}

void BuriedAssign::MarkIfAssignment(Stmt const* stmt)
{
    if (auto expr = dyn_cast_or_null<Expr>(stmt))
    {
        expr = expr->IgnoreImplicit();
        if (auto binaryOp = dyn_cast<BinaryOperator>(expr))
        {
            if (isAssignmentOp(binaryOp->getOpcode()))
            {
                m_handled.insert(expr);
                MarkIfAssignment(binaryOp->getRHS()); // in case it is chained
            }
            else if (binaryOp->getOpcode() == BO_Comma)
            {
                MarkIfAssignment(binaryOp->getLHS());
                MarkIfAssignment(binaryOp->getRHS());
            }
        }
        else if (auto cxxOper = dyn_cast<CXXOperatorCallExpr>(expr))
        {
            if (isAssignmentOp(cxxOper->getOperator()))
            {
                m_handled.insert(expr);
                MarkIfAssignment(cxxOper->getArg(1)); // in case it is chained
            }
        }
    }
}

void BuriedAssign::MarkAll(Stmt const* stmt)
{
    m_handled.insert(stmt);
    for (auto it = stmt->child_begin(); it != stmt->child_end(); ++it)
        MarkAll(*it);
}

/**
 * Restrict this to cases where the buried assignment is part of the first
 * condition inside the if condition. Other cases tend to be too hard
 * too extract (notably in sw/)
 */
bool BuriedAssign::VisitIfStmt(IfStmt const* ifStmt)
{
    if (ignoreLocation(ifStmt))
        return true;
    MarkIfAssignment(ifStmt->getThen());
    MarkIfAssignment(ifStmt->getElse());

    auto expr = ifStmt->getCond();
    expr = IgnoreImplicitAndConversionOperator(expr);
    expr = expr->IgnoreParens();
    expr = IgnoreImplicitAndConversionOperator(expr);
    MarkAll(expr);

    if (auto binaryOp = dyn_cast<BinaryOperator>(expr))
    {
        if (isAssignmentOp(binaryOp->getOpcode()))
        {
            report(DiagnosticsEngine::Warning, "buried assignment, rather put on own line",
                   expr->getBeginLoc())
                << expr->getSourceRange();
        }
        else if (binaryOp->isComparisonOp())
        {
            if (auto binaryOp2
                = dyn_cast<BinaryOperator>(binaryOp->getLHS()->IgnoreParenImpCasts()))
            {
                if (!binaryOp->getRHS()->isValueDependent()
                    && binaryOp->getRHS()->isCXX11ConstantExpr(compiler.getASTContext())
                    && isAssignmentOp(binaryOp2->getOpcode()))
                    report(DiagnosticsEngine::Warning, "buried assignment, rather put on own line",
                           expr->getBeginLoc())
                        << expr->getSourceRange();
            }
            if (auto binaryOp2
                = dyn_cast<BinaryOperator>(binaryOp->getRHS()->IgnoreParenImpCasts()))
            {
                if (!binaryOp->getLHS()->isValueDependent()
                    && binaryOp->getLHS()->isCXX11ConstantExpr(compiler.getASTContext())
                    && isAssignmentOp(binaryOp2->getOpcode()))
                    report(DiagnosticsEngine::Warning, "buried assignment, rather put on own line",
                           expr->getBeginLoc())
                        << expr->getSourceRange();
            }
        }
        else if (binaryOp->isLogicalOp())
        {
            if (auto binaryOp2
                = dyn_cast<BinaryOperator>(binaryOp->getLHS()->IgnoreParenImpCasts()))
            {
                if (isAssignmentOp(binaryOp2->getOpcode()))
                    report(DiagnosticsEngine::Warning, "buried assignment, rather put on own line",
                           expr->getBeginLoc())
                        << expr->getSourceRange();
            }
        }
    }
    else if (auto operCall = dyn_cast<CXXOperatorCallExpr>(expr))
    {
        // Ignore chained assignment.
        // TODO limit this to only ordinary assignment
        if (isAssignmentOp(operCall->getOperator()))
        {
            report(DiagnosticsEngine::Warning, "buried assignment, rather put on own line",
                   expr->getBeginLoc())
                << expr->getSourceRange();
        }
    }

    return true;
}

bool BuriedAssign::VisitCaseStmt(CaseStmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkIfAssignment(stmt->getSubStmt());
    return true;
}

bool BuriedAssign::VisitDefaultStmt(DefaultStmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkIfAssignment(stmt->getSubStmt());
    return true;
}

bool BuriedAssign::VisitWhileStmt(WhileStmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkConditionForControlLoops(stmt->getCond());
    MarkIfAssignment(stmt->getBody());
    return true;
}

bool BuriedAssign::VisitDoStmt(DoStmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkConditionForControlLoops(stmt->getCond());
    MarkIfAssignment(stmt->getBody());
    return true;
}

/** stuff like
 *    while ((x = foo())
 * and
 *    while ((x = foo() < 0)
 * is considered idiomatic.
 */
void BuriedAssign::MarkConditionForControlLoops(Expr const* expr)
{
    if (!expr)
        return;
    expr = expr->IgnoreImplicit();

    if (auto binaryOp = dyn_cast<BinaryOperator>(expr))
    {
        // ignore comma-chained statements at this level
        if (binaryOp->getOpcode() == BO_Comma)
        {
            MarkConditionForControlLoops(binaryOp->getLHS());
            MarkConditionForControlLoops(binaryOp->getRHS());
            return;
        }
    }

    // unwrap conversion to bool
    if (auto memberCall = dyn_cast<CXXMemberCallExpr>(expr))
    {
        if (memberCall->getMethodDecl() && isa<CXXConversionDecl>(memberCall->getMethodDecl()))
        {
            // TODO check that the conversion is converting to bool
            expr = memberCall->getImplicitObjectArgument()->IgnoreImplicit();
        }
    }

    if (auto binaryOp = dyn_cast<BinaryOperator>(expr))
    {
        // handle: ((xxx = foo()) != error)
        if (binaryOp->isComparisonOp())
        {
            MarkIfAssignment(binaryOp->getLHS()->IgnoreImplicit()->IgnoreParens());
            MarkIfAssignment(binaryOp->getRHS()->IgnoreImplicit()->IgnoreParens());
        }
    }
    else if (auto cxxOper = dyn_cast<CXXOperatorCallExpr>(expr))
    {
        // handle: ((xxx = foo()) != error)
        if (cxxOper->isComparisonOp())
        {
            MarkIfAssignment(cxxOper->getArg(0)->IgnoreImplicit()->IgnoreParens());
            MarkIfAssignment(cxxOper->getArg(1)->IgnoreImplicit()->IgnoreParens());
        }
        // handle: (!(xxx = foo()))
        else if (cxxOper->getOperator() == OO_Exclaim)
            MarkIfAssignment(cxxOper->getArg(0)->IgnoreImplicit()->IgnoreParens());
    }
    else if (auto parenExpr = dyn_cast<ParenExpr>(expr))
    {
        // handle: ((xxx = foo()))
        MarkIfAssignment(parenExpr->getSubExpr()->IgnoreImplicit());
    }
    else if (auto unaryOp = dyn_cast<UnaryOperator>(expr))
    {
        // handle: (!(xxx = foo()))
        MarkIfAssignment(unaryOp->getSubExpr()->IgnoreImplicit()->IgnoreParens());
    }
    else
        MarkIfAssignment(expr);
}

bool BuriedAssign::VisitForStmt(ForStmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkIfAssignment(stmt->getInit());
    MarkConditionForControlLoops(stmt->getCond());
    MarkIfAssignment(stmt->getInc());
    MarkIfAssignment(stmt->getBody());
    return true;
}

bool BuriedAssign::VisitCXXForRangeStmt(CXXForRangeStmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkIfAssignment(stmt->getBody());
    return true;
}

bool BuriedAssign::VisitLabelStmt(LabelStmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkIfAssignment(stmt->getSubStmt());
    return true;
}

bool BuriedAssign::VisitVarDecl(VarDecl const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    if (stmt->getInit())
    {
        auto expr = IgnoreImplicitAndConversionOperator(stmt->getInit());
        MarkIfAssignment(expr);
        if (auto cxxConstruct = dyn_cast<CXXConstructExpr>(expr))
        {
            if (cxxConstruct->getNumArgs() == 1)
                MarkIfAssignment(cxxConstruct->getArg(0));
        }
    }
    return true;
}

bool BuriedAssign::VisitCXXFoldExpr(CXXFoldExpr const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkConditionForControlLoops(stmt->getLHS());
    MarkConditionForControlLoops(stmt->getRHS());
    return true;
}

// off by default because it uses getParentStmt so it's more expensive to run
loplugin::Plugin::Registration<BuriedAssign> X("buriedassign", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
