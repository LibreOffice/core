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
#include <unordered_map>
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

/**
    This is a kind of simplified dead-store analysis.

    We are looking for patterns like:
      Foo x = a;
      x = b;
    which can be simplified to
        x = b

    or
        Foo x = a;
        x = f(x)
    which can be simplified to
        Foo x = f(a)

   TODO Improve this plugin to make it safer. We should really be checking the following
   conditions inside the RHS of the second statement:
     If the variable is having it's address taken, or a non-const method called on it,
     on passed by non-const-ref.
*/

namespace
{
//static bool startswith(const std::string& rStr, const char* pSubStr) {
//    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
//}
class SequentialAssign : public loplugin::FilteringPlugin<SequentialAssign>
{
public:
    explicit SequentialAssign(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        // places where the existing code style just looks better
        // TODO lots of these would be unnecessary if I taught the plugin
        // to ignore vars which are assigned to repeatedly
        if (fn == SRCDIR "/vcl/source/helper/commandinfoprovider.cxx"
            || fn == SRCDIR "/basegfx/source/polygon/b2dpolygonclipper.cxx"
            || fn == SRCDIR "/i18nlangtag/source/isolang/insys.cxx"
            || fn == SRCDIR "/vcl/unx/generic/fontmanager/fontconfig.cxx"
            || fn == SRCDIR "/svtools/source/filter/exportdialog.cxx"
            || fn == SRCDIR "/svtools/source/control/ruler.cxx"
            || fn == SRCDIR "/basic/qa/cppunit/test_scanner.cxx"
            || fn == SRCDIR "/basic/source/uno/namecont.cxx"
            || fn == SRCDIR "/test/source/sheet/xnamedrange.cxx"
            || fn == SRCDIR "/i18npool/qa/cppunit/test_breakiterator.cxx"
            || fn == SRCDIR "/i18npool/source/localedata/LocaleNode.cxx"
            || fn == SRCDIR "/i18npool/source/transliteration/transliteration_Ignore.cxx"
            || fn == SRCDIR "/i18npool/qa/cppunit/test_textsearch.cxx"
            || fn == SRCDIR "/framework/source/jobs/jobdata.cxx"
            || fn == SRCDIR "/framework/source/services/pathsettings.cxx"
            || fn == SRCDIR "/xmloff/source/chart/SchXMLTools.cxx"
            || fn == SRCDIR "/svx/source/tbxctrls/Palette.cxx"
            || fn == SRCDIR "/svx/source/sdr/contact/objectcontactofpageview.cxx"
            || fn == SRCDIR "/svx/source/form/fmservs.cxx"
            || fn == SRCDIR "/svx/source/svdraw/svdograf.cxx"
            || fn == SRCDIR "/svx/source/accessibility/AccessibleShape.cxx"
            || fn == SRCDIR "/svx/source/fmcomp/fmgridcl.cxx"
            || fn == SRCDIR "/chart2/source/tools/CharacterProperties.cxx"
            || fn == SRCDIR "/svx/source/dialog/dialcontrol.cxx"
            || fn == SRCDIR "/connectivity/source/commontools/TTableHelper.cxx"
            || fn == SRCDIR "/svx/source/dialog/_bmpmask.cxx"
            || fn == SRCDIR "/media/noel/disk2/libo4/cui/source/dialogs/SignSignatureLineDialog.cxx"
            || fn == SRCDIR "/filter/source/msfilter/msdffimp.cxx"
            || fn == SRCDIR "/cui/source/dialogs/SignSignatureLineDialog.cxx"
            || fn == SRCDIR "/cui/source/dialogs/screenshotannotationdlg.cxx"
            || fn == SRCDIR "/cui/source/options/optupdt.cxx"
            || fn == SRCDIR "/dbaccess/source/ui/querydesign/querycontroller.cxx"
            || fn == SRCDIR "/dbaccess/source/ui/dlg/DbAdminImpl.cxx"
            || fn == SRCDIR "/dbaccess/source/ui/querydesign/JoinController.cxx"
            || fn == SRCDIR "/dbaccess/source/ui/misc/TokenWriter.cxx"
            || fn == SRCDIR "/dbaccess/source/ui/misc/TokenWriter.cxx"
            || fn == SRCDIR "/dbaccess/source/ui/misc/dbsubcomponentcontroller.cxx"
            || fn == SRCDIR "/dbaccess/source/core/api/querycomposer.cxx"
            || fn == SRCDIR "/desktop/source/lib/init.cxx"
            || fn == SRCDIR "/lotuswordpro/source/filter/lwpfribmark.cxx"
            || fn == SRCDIR "/tools/qa/cppunit/test_color.cxx"
            || fn == SRCDIR "/sc/qa/unit/ucalc.cxx"
            || fn == SRCDIR "/sc/source/ui/view/printfun.cxx"
            || fn == SRCDIR "/sc/source/ui/view/preview.cxx"
            || fn == SRCDIR "/sw/source/core/doc/tblafmt.cxx"
            || fn == SRCDIR "/sw/source/core/draw/dflyobj.cxx"
            || fn == SRCDIR "/sw/source/core/doc/DocumentDrawModelManager.cxx"
            || fn == SRCDIR "/sw/source/core/edit/edfcol.cxx"
            || fn == SRCDIR "/sw/source/filter/ww8/ww8toolbar.cxx"
            || fn == SRCDIR "/sw/source/ui/fldui/fldvar.cxx"
            || fn == SRCDIR "/sw/source/filter/ww8/ww8atr.cxx"
            || fn == SRCDIR "/sd/source/ui/accessibility/AccessiblePageShape.cxx"
            || fn == SRCDIR "/xmlsecurity/source/xmlsec/nss/nssinitializer.cxx"
            || fn == SRCDIR "/sd/source/ui/slidesorter/cache/SlsRequestFactory.cxx"
            || fn == SRCDIR "/sd/source/ui/framework/configuration/ResourceId.cxx"
            || fn == SRCDIR "/sd/source/filter/html/htmlex.cxx"
            || fn == SRCDIR "/starmath/source/cfgitem.cxx"
            || fn == SRCDIR "/ucb/source/ucp/ftp/ftpurl.cxx"
            || fn == SRCDIR "/starmath/source/node.cxx"
            || fn == SRCDIR "/starmath/source/starmathdatabase.cxx"
            || fn == SRCDIR "/ucb/source/ucp/cmis/certvalidation_handler.cxx"
            || fn == SRCDIR "/reportdesign/source/ui/inspection/GeometryHandler.cxx"
            || fn == SRCDIR "/reportdesign/source/core/api/ReportDefinition.cxx"
            || fn == SRCDIR "/test/source/table/tablerow.cxx"
            || fn == SRCDIR "/basegfx/test/B2DHomMatrixTest.cxx"
            || fn == SRCDIR "/comphelper/qa/unit/base64_test.cxx"
            || fn == SRCDIR "/testtools/source/bridgetest/bridgetest.cxx"
            || fn == SRCDIR "/comphelper/qa/string/test_string.cxx"
            || fn == SRCDIR "/cppu/qa/test_unotype.cxx"
            || fn == SRCDIR "/cppu/qa/cppumaker/test_cppumaker.cxx"
            || fn == SRCDIR "/o3tl/qa/test-lru_map.cxx" || fn == SRCDIR "/svl/qa/unit/svl.cxx"
            || fn == SRCDIR "/chart2/qa/extras/PivotChartTest.cxx"
            || fn == SRCDIR "/chart2/qa/extras/chart2export.cxx"
            || fn == SRCDIR "/writerfilter/qa/cppunittests/misc/misc.cxx"
            || fn == SRCDIR "/sw/qa/extras/ww8export/ww8export.cxx"
            || fn == SRCDIR "/sw/qa/extras/uiwriter/uiwriter.cxx")
            return;
        // inside ifdef
        if (fn == SRCDIR "/vcl/source/filter/png/pngread.cxx"
            || fn == SRCDIR "/vcl/source/window/syschild.cxx"
            || fn == SRCDIR "/sal/osl/unx/security.cxx"
            || fn == SRCDIR "/vcl/source/filter/png/pngwrite.cxx"
            || fn == SRCDIR "/svtools/source/control/inettbc.cxx"
            || fn == SRCDIR "/canvas/source/cairo/cairo_textlayout.cxx"
            || fn == SRCDIR "/sal/qa/osl/file/osl_File.cxx")
            return;
        // taking address of variable
        if (fn == SRCDIR "/vcl/unx/generic/dtrans/X11_selection.cxx")
            return;
        // other
        if (fn == SRCDIR "/sc/source/core/tool/scmatrix.cxx"
            || fn == SRCDIR "/sal/qa/rtl/oustringbuffer/test_oustringbuffer_assign.cxx")
            return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCompoundStmt(CompoundStmt const*);

private:
    const VarDecl* findSimpleAssign(Stmt const*);
    bool isSimpleRHS(Expr const*);
    Expr const* ignore(Expr const*);
    void checkForSecondAssign(Stmt const* stmt, VarDecl const* varDecl);
};

bool SequentialAssign::VisitCompoundStmt(CompoundStmt const* compoundStmt)
{
    if (ignoreLocation(compoundStmt))
        return true;

    auto it = compoundStmt->body_begin();
    while (true)
    {
        if (it == compoundStmt->body_end())
            break;
        auto firstStmt = *it;
        const VarDecl* foundVars = findSimpleAssign(firstStmt);
        // reference types have slightly weird behaviour
        if (!foundVars || foundVars->getType()->isReferenceType())
        {
            ++it;
            continue;
        }
        ++it;
        if (it == compoundStmt->body_end())
            break;
        checkForSecondAssign(*it, foundVars);
    }

    return true;
}

void SequentialAssign::checkForSecondAssign(Stmt const* stmt, VarDecl const* varDecl)
{
    if (auto exprCleanup = dyn_cast<ExprWithCleanups>(stmt))
        stmt = exprCleanup->getSubExpr();

    if (auto operatorCall = dyn_cast<CXXOperatorCallExpr>(stmt))
    {
        if (operatorCall->getOperator() == OO_Equal)
        {
            if (auto declRefExprLHS = dyn_cast<DeclRefExpr>(ignore(operatorCall->getArg(0))))
                if (declRefExprLHS->getDecl() == varDecl)
                {
                    report(DiagnosticsEngine::Warning,
                           "simplify by merging with the preceding assignment",
                           compat::getBeginLoc(stmt))
                        << stmt->getSourceRange();
                }
        }
        return;
    }

    if (auto binaryOp = dyn_cast<BinaryOperator>(stmt))
    {
        if (binaryOp->getOpcode() == BO_Assign)
        {
            if (auto declRefExpr = dyn_cast<DeclRefExpr>(ignore(binaryOp->getLHS())))
                if (declRefExpr->getDecl() == varDecl)
                {
                    report(DiagnosticsEngine::Warning,
                           "simplify by merging with the preceding assignment",
                           compat::getBeginLoc(stmt))
                        << stmt->getSourceRange();
                }
        }
    }
}

const VarDecl* SequentialAssign::findSimpleAssign(Stmt const* stmt)
{
    if (auto declStmt = dyn_cast<DeclStmt>(stmt))
        if (declStmt->isSingleDecl())
            if (auto varDeclLHS = dyn_cast_or_null<VarDecl>(declStmt->getSingleDecl()))
            {
                if (varDeclLHS->getStorageDuration() == SD_Static)
                    return nullptr;
                // if it's call-style init (e.g. OUString s("xxx")), we only treat
                // it as simple if it only contains a variable in the call
                // (e.g. OUString s(x))
                if (varDeclLHS->getInitStyle() == VarDecl::InitializationStyle::CallInit)
                {
                    auto cxxConstructExpr
                        = dyn_cast<CXXConstructExpr>(ignore(varDeclLHS->getInit()));
                    if (cxxConstructExpr)
                    {
                        if (cxxConstructExpr->getNumArgs() == 0)
                            return varDeclLHS;
                        if (cxxConstructExpr->getNumArgs() > 1)
                            return nullptr;
                        if (!isa<DeclRefExpr>(ignore(cxxConstructExpr->getArg(0))))
                            return nullptr;
                    }
                }
                if (auto init = varDeclLHS->getInit())
                    if (isSimpleRHS(init))
                        return varDeclLHS;
            }
    if (auto operatorCall = dyn_cast<CXXOperatorCallExpr>(stmt))
        if (operatorCall->getOperator() == OO_Equal)
            if (auto declRefExprLHS = dyn_cast<DeclRefExpr>(ignore(operatorCall->getArg(0))))
                if (auto varDeclLHS = dyn_cast<VarDecl>(declRefExprLHS->getDecl()))
                    if (isSimpleRHS(operatorCall->getArg(1)))
                        return varDeclLHS;
    if (auto binaryOp = dyn_cast<BinaryOperator>(stmt))
        if (binaryOp->getOpcode() == BO_Assign)
            if (auto declRefExprLHS = dyn_cast<DeclRefExpr>(ignore(binaryOp->getLHS())))
                if (auto varDeclLHS = dyn_cast<VarDecl>(declRefExprLHS->getDecl()))
                    if (isSimpleRHS(binaryOp->getRHS()))
                        return varDeclLHS;
    return nullptr;
}

/**
  Does the first statement have a relatively simply RHS we can inline into the second statement?
*/
bool SequentialAssign::isSimpleRHS(Expr const* expr)
{
    expr = ignore(expr);

    // code like
    //    Point aCurPos = rGlyphs
    // always has a CXXConstructExpr wrapping the RHS
    if (auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(expr))
        if (cxxConstructExpr->getNumArgs() == 1)
            expr = ignore(cxxConstructExpr->getArg(0));

    if (!expr->isValueDependent()
        && expr->isConstantInitializer(compiler.getASTContext(), false /*ForRef*/))
        return true;
    if (isa<CXXMemberCallExpr>(expr))
        return false;
    if (isa<CXXOperatorCallExpr>(expr))
        return false;
    if (isa<UserDefinedLiteral>(expr))
        return true;
    if (isa<CallExpr>(expr))
        return false;
    if (isa<CastExpr>(expr))
        return false;
    if (isa<ArraySubscriptExpr>(expr))
        return false;
    if (isa<BinaryOperator>(expr))
        return false;
    if (isa<ConditionalOperator>(expr))
        return false;
    if (isa<UnaryOperator>(expr))
        return false;
    if (isa<CXXNewExpr>(expr))
        return false;
    if (auto memberExpr = dyn_cast<MemberExpr>(expr))
        return isSimpleRHS(memberExpr->getBase());

    return true;
}

Expr const* SequentialAssign::ignore(Expr const* expr)
{
    return expr->IgnoreImplicit()->IgnoreParens()->IgnoreImplicit();
}

// Off by default because of safety concerns, see TODO at top
loplugin::Plugin::Registration<SequentialAssign> X("sequentialassign", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
