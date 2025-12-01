/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "check.hxx"
#include "plugin.hxx"
#include "config_clang.h"
#include <vector>

/** Look for OUString/OString being appended to inside a loop, where OUStringBuffer/OStringBuffer would be a better idea
 */
namespace
{
class StringLoop : public clang::RecursiveASTVisitor<StringLoop>, public loplugin::Plugin
{
public:
    explicit StringLoop(loplugin::InstantiationData const& rData)
        : Plugin(rData)
    {
    }

    void run() override;
    bool TraverseForStmt(ForStmt*);
    bool TraverseCXXForRangeStmt(CXXForRangeStmt*);
    bool TraverseDoStmt(DoStmt*);
    bool TraverseWhileStmt(WhileStmt*);
    bool VisitVarDecl(VarDecl const*);
    bool VisitCallExpr(CallExpr const*);

private:
    int m_insideLoop = 0;
    using VarDeclList = std::vector<VarDecl const*>;
    std::vector<VarDeclList> m_varsPerLoopLevel;
};

void StringLoop::run()
{
    // Various places are not worth changing, the code becomes too awkward
    // Just exclude stuff as I go
    StringRef fn(handler.getMainFileName());
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/bridges/"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/cppuhelper/source/shlib.cxx"))
        return;
    //    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/tools/qa/"))
    //        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/jvmfwk/plugins/sunmajor/pluginlib/vendorbase.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/svl/source/numbers/zformat.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/svl/source/numbers/zforscan.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/control/combobox.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/control/calendar.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/treelist/svtabbx.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/pdf/pdfwriter_impl.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/vcl/jsdialog/jsdialogregister.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/idl/source/prj/command.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/framework/source/accelerators/storageholder.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/basic/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sfx2/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/connectivity/source/drivers/odbc/OConnection.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/connectivity/source/parse/sqliterator.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/editeng/source/editeng/eertfpar.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/editeng/source/rtf/svxrtf.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/svx/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/basctl/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/filter/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/hwpfilter/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/cui/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/dbaccess/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/desktop/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/extensions/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/lingucomponent/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/i18npool/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/embeddedobj/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sd/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/xmloff/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/xmlhelp/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/forms/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/core/tool/address.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/ui/view/gridwin2.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/core/tool/compiler.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/core/tool/interpr1.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/ui/docshell/impex.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/ui/miscdlgs/acredlin.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/ui/pagedlg/areasdlg.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/ui/dbgui/scuiasciiopt.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/core/data/SolverSettings.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/ui/unoobj/solveruno.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/filter/html/htmlpars.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sdext/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/layout/dbg_lay.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/edit/edattr.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/tox/ToxTextGenerator.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/filter/html/parcss1.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/filter/html/svxcss1.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/fields/authfld.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/filter/ascii/ascatr.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/unocore/unosect.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/unocore/unoobj.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/edit/acorrect.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/filter/html/swhtml.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/uibase/utlui/gloslst.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/doc/doctxm.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/uibase/docvw/edtwin.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/filter/ww8/ww8atr.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/filter/ww8/ww8scan.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/filter/ww8/ww8par5.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/ui/fldui/fldfunc.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/ui/dbui/dbinsdlg.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/ui/index/cnttab.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/ui/dbui/mmlayoutpage.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/ui/dbui/mmresultdialogs.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/uibase/shells/textsh1.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/uibase/utlui/content.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn,
                                    SRCDIR "/sw/source/writerfilter/dmapper/NumberingManager.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn,
                                    SRCDIR "/sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/ucb/source/ucp/file/bc.cxx"))
        return;

    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
}

bool StringLoop::TraverseForStmt(ForStmt* stmt)
{
    ++m_insideLoop;
    m_varsPerLoopLevel.push_back({});
    auto const ret = RecursiveASTVisitor::TraverseForStmt(stmt);
    m_varsPerLoopLevel.pop_back();
    --m_insideLoop;
    return ret;
}

bool StringLoop::TraverseCXXForRangeStmt(CXXForRangeStmt* stmt)
{
    ++m_insideLoop;
    m_varsPerLoopLevel.push_back({});
    auto const ret = RecursiveASTVisitor::TraverseCXXForRangeStmt(stmt);
    m_varsPerLoopLevel.pop_back();
    --m_insideLoop;
    return ret;
}

bool StringLoop::TraverseDoStmt(DoStmt* stmt)
{
    ++m_insideLoop;
    m_varsPerLoopLevel.push_back({});
    auto const ret = RecursiveASTVisitor::TraverseDoStmt(stmt);
    m_varsPerLoopLevel.pop_back();
    --m_insideLoop;
    return ret;
}

bool StringLoop::TraverseWhileStmt(WhileStmt* stmt)
{
    ++m_insideLoop;
    m_varsPerLoopLevel.push_back({});
    auto const ret = RecursiveASTVisitor::TraverseWhileStmt(stmt);
    m_varsPerLoopLevel.pop_back();
    --m_insideLoop;
    return ret;
}

bool StringLoop::VisitVarDecl(VarDecl const* varDecl)
{
    if (ignoreLocation(varDecl))
        return true;
    if (!m_insideLoop)
        return true;
    m_varsPerLoopLevel.back().push_back(varDecl);
    return true;
}

bool StringLoop::VisitCallExpr(CallExpr const* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;
    if (!m_insideLoop)
        return true;
    auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(callExpr);
    if (!operatorCallExpr)
        return true;
    if (operatorCallExpr->getOperator() != OO_PlusEqual)
        return true;

    if (auto memberExpr = dyn_cast<MemberExpr>(callExpr->getArg(0)))
    {
        auto tc = loplugin::TypeCheck(memberExpr->getType());
        if (!tc.Class("OUString").Namespace("rtl").GlobalNamespace()
            && !tc.Class("OString").Namespace("rtl").GlobalNamespace())
            return true;
        auto fieldDecl = dyn_cast<FieldDecl>(memberExpr->getMemberDecl());
        if (isInUnoIncludeFile(
                compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation())))
            return true;
        if (ignoreLocation(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation())))
            return true;
        report(DiagnosticsEngine::Warning,
               "appending to OUString in loop, rather use OUStringBuffer",
               operatorCallExpr->getBeginLoc())
            << operatorCallExpr->getSourceRange();
        report(DiagnosticsEngine::Note, "field here", fieldDecl->getBeginLoc())
            << fieldDecl->getSourceRange();
    }
    else if (auto declRefExpr = dyn_cast<DeclRefExpr>(callExpr->getArg(0)))
    {
        if (auto varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl()))
        {
            auto tc = loplugin::TypeCheck(varDecl->getType());
            if (!tc.Class("OUString").Namespace("rtl").GlobalNamespace()
                && !tc.Class("OString").Namespace("rtl").GlobalNamespace())
                return true;
            // if the var is at the same block scope as the +=, not interesting
            auto vars = m_varsPerLoopLevel.back();
            if (std::find(vars.begin(), vars.end(), varDecl) != vars.end())
                return true;
            report(DiagnosticsEngine::Warning,
                   "appending to OUString in loop, rather use OUStringBuffer",
                   operatorCallExpr->getBeginLoc())
                << operatorCallExpr->getSourceRange();
            report(DiagnosticsEngine::Note, "var here", varDecl->getBeginLoc())
                << varDecl->getSourceRange();
        }
    }
    return true;
}

/** off by default because it needs hand-holding */
loplugin::Plugin::Registration<StringLoop> X("stringloop", false);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
