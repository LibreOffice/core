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
    StringRef fn(handler.getMainFileName());
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/bridges/"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/cppuhelper/source/shlib.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/registry/source/regimpl.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/l10ntools/source/lngmerge.cxx"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/tools/qa/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/jvmfwk/"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/svl/source/passwordcontainer/passwordcontainer.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/svl/source/numbers/zformat.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/svl/source/numbers/zforscan.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/control/combobox.cxx"))
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
        report(DiagnosticsEngine::Warning,
               "appending to OUString in loop, rather use OUStringBuffer",
               operatorCallExpr->getLocStart())
            << operatorCallExpr->getSourceRange();
        auto fieldDecl = dyn_cast<FieldDecl>(memberExpr->getMemberDecl());
        report(DiagnosticsEngine::Note, "field here", fieldDecl->getLocStart())
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
                   operatorCallExpr->getLocStart())
                << operatorCallExpr->getSourceRange();
            report(DiagnosticsEngine::Note, "var here", varDecl->getLocStart())
                << varDecl->getSourceRange();
        }
    }
    return true;
}

loplugin::Plugin::Registration<StringLoop> X("stringloop", false);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
