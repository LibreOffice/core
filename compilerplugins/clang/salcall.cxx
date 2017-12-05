/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include "check.hxx"
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>

// The SAL_CALL function annotation is only necessary on our outward
// facing C++ ABI, anywhere else it is just cargo-cult.
//

namespace
{
//static bool startswith(const std::string& rStr, const char* pSubStr)
//{
//    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
//}

class SalCall final : public RecursiveASTVisitor<SalCall>, public loplugin::RewritePlugin
{
public:
    explicit SalCall(loplugin::InstantiationData const& data)
        : RewritePlugin(data)
    {
    }

    virtual void run() override
    {
        std::string fn(compiler.getSourceManager()
                           .getFileEntryForID(compiler.getSourceManager().getMainFileID())
                           ->getName());
        loplugin::normalizeDotDotInFilePath(fn);
        // ignore this one. I can't get accurate source code from getCharacterData() for it.
        if (fn == SRCDIR "/sal/rtl/string.cxx")
            return;
        m_phase = PluginPhase::FindAddressOf;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        m_phase = PluginPhase::Warning;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitFunctionDecl(FunctionDecl const*);
    bool VisitUnaryAddrOf(UnaryOperator const*);
    bool VisitInitListExpr(InitListExpr const*);
    bool VisitCallExpr(CallExpr const*);
    bool VisitBinAssign(BinaryOperator const*);
    bool VisitCXXConstructExpr(CXXConstructExpr const*);

private:
    void checkForFunctionDecl(Expr const*, bool bCheckOnly = false);
    bool rewrite(SourceLocation);
    bool checkOverlap(SourceRange);
    bool isSalCallFunction(FunctionDecl const* functionDecl, SourceLocation* pLoc = nullptr);

    std::set<FunctionDecl const*> m_addressOfSet;
    enum class PluginPhase
    {
        FindAddressOf,
        Warning
    };
    PluginPhase m_phase;
    std::vector<std::pair<char const*, char const*>> mvModifiedRanges;
};

bool SalCall::VisitUnaryAddrOf(UnaryOperator const* op)
{
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    checkForFunctionDecl(op->getSubExpr());
    return true;
}

bool SalCall::VisitBinAssign(BinaryOperator const* binaryOperator)
{
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    checkForFunctionDecl(binaryOperator->getRHS());
    return true;
}

bool SalCall::VisitCallExpr(CallExpr const* callExpr)
{
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    for (auto arg : callExpr->arguments())
        checkForFunctionDecl(arg);
    return true;
}

bool SalCall::VisitCXXConstructExpr(CXXConstructExpr const* constructExpr)
{
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    for (auto arg : constructExpr->arguments())
        checkForFunctionDecl(arg);
    return true;
}

bool SalCall::VisitInitListExpr(InitListExpr const* initListExpr)
{
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    for (auto subStmt : *initListExpr)
        checkForFunctionDecl(dyn_cast<Expr>(subStmt));
    return true;
}

void SalCall::checkForFunctionDecl(Expr const* expr, bool bCheckOnly)
{
    auto e1 = expr->IgnoreParenCasts();
    auto declRef = dyn_cast<DeclRefExpr>(e1);
    if (!declRef)
        return;
    auto functionDecl = dyn_cast<FunctionDecl>(declRef->getDecl());
    if (!functionDecl)
        return;
    if (bCheckOnly)
        getParentStmt(expr)->dump();
    else
        m_addressOfSet.insert(functionDecl->getCanonicalDecl());
}

bool SalCall::VisitFunctionDecl(FunctionDecl const* decl)
{
    if (m_phase != PluginPhase::Warning)
        return true;
    if (ignoreLocation(decl))
        return true;

    // ignore template stuff
    if (decl->getTemplatedKind() != clang::FunctionDecl::TK_NonTemplate)
        return true;
    auto recordDecl = dyn_cast<CXXRecordDecl>(decl->getDeclContext());
    if (recordDecl
        && (recordDecl->getTemplateSpecializationKind() != TSK_Undeclared
            || recordDecl->isDependentContext()))
    {
        return true;
    }

    auto canonicalDecl = decl->getCanonicalDecl();

    // ignore UNO implementations
    if (isInUnoIncludeFile(
            compiler.getSourceManager().getSpellingLoc(canonicalDecl->getLocation())))
        return true;

    // macros make getCharacterData() extremely unreliable
    if (compiler.getSourceManager().isMacroArgExpansion(decl->getLocation())
        || compiler.getSourceManager().isMacroBodyExpansion(decl->getLocation()))
        return true;

    SourceLocation rewriteLoc;
    SourceLocation rewriteCanonicalLoc;
    bool bDeclIsSalCall = isSalCallFunction(decl, &rewriteLoc);
    bool bCanonicalDeclIsSalCall = isSalCallFunction(canonicalDecl, &rewriteCanonicalLoc);

    // first, check for consistency, so we don't trip ourselves up on Linux, where we normally run the plugin
    if (canonicalDecl != decl)
    {
        if (bCanonicalDeclIsSalCall)
            ; // this is fine, the actual definition have or not have SAL_CALL, and MSVC is fine with it
        else if (bDeclIsSalCall)
        {
            // not fine
            report(DiagnosticsEngine::Warning, "SAL_CALL inconsistency",
                   canonicalDecl->getLocation())
                << canonicalDecl->getSourceRange();
            report(DiagnosticsEngine::Note, "SAL_CALL inconsistency", decl->getLocation())
                << decl->getSourceRange();
            return true;
        }
    }
    auto methodDecl = dyn_cast<CXXMethodDecl>(canonicalDecl);
    if (methodDecl)
    {
        for (auto iter = methodDecl->begin_overridden_methods();
             iter != methodDecl->end_overridden_methods(); ++iter)
        {
            const CXXMethodDecl* overriddenMethod = (*iter)->getCanonicalDecl();
            if (bCanonicalDeclIsSalCall != isSalCallFunction(overriddenMethod))
            {
                report(DiagnosticsEngine::Warning, "SAL_CALL inconsistency",
                       methodDecl->getLocation())
                    << methodDecl->getSourceRange();
                report(DiagnosticsEngine::Note, "SAL_CALL inconsistency",
                       overriddenMethod->getLocation())
                    << overriddenMethod->getSourceRange();
                return true;
            }
        }
    }

    if (!bCanonicalDeclIsSalCall)
        return true;

    // @TODO For now, I am ignore free functions, since those are most likely to have their address taken.
    // I'll do them later. They are harder to verify since MSVC does not verify when assigning to function pointers
    // that the calling convention of the function matches the calling convention of the function pointer!
    if (!methodDecl || methodDecl->isStatic())
        return true;

    // can only check when we have a definition since this is the most likely time
    // when the address of the method will be taken
    if (!(methodDecl && methodDecl->isPure()) && !decl->isThisDeclarationADefinition())
        return true;
    if (m_addressOfSet.find(decl->getCanonicalDecl()) != m_addressOfSet.end())
        return true;

    // ignore extern "C" UNO factory constructor functions
    if (decl->isExternC())
    {
        if (loplugin::TypeCheck(decl->getReturnType())
                .Pointer()
                .Class("XInterface")
                .Namespace("uno")
                .Namespace("star")
                .Namespace("sun")
                .Namespace("com")
                .GlobalNamespace())
            return true;
        if (loplugin::TypeCheck(decl->getReturnType()).Pointer().Void())
            return true;
    }

    // some base classes are overridden by sub-classes which override both the base-class and an UNO class
    if (recordDecl)
    {
        auto dc = loplugin::DeclCheck(recordDecl);
        if (dc.Class("OProxyAggregation").Namespace("comphelper").GlobalNamespace()
            || dc.Class("OComponentProxyAggregationHelper")
                   .Namespace("comphelper")
                   .GlobalNamespace()
            || dc.Class("SvxShapeMaster").GlobalNamespace()
            || dc.Class("ListBoxAccessibleBase").Namespace("accessibility").GlobalNamespace()
            || dc.Class("AsyncEventNotifierBase").Namespace("comphelper").GlobalNamespace()
            || dc.Class("ODescriptor")
                   .Namespace("sdbcx")
                   .Namespace("connectivity")
                   .GlobalNamespace()
            || dc.Class("IController").Namespace("dbaui").GlobalNamespace()
            || dc.Class("ORowSetBase").Namespace("dbaccess").GlobalNamespace()
            || dc.Class("OComponentAdapterBase").Namespace("bib").GlobalNamespace()
            || dc.Class("IEventProcessor").Namespace("comphelper").GlobalNamespace()
            || dc.Class("SvxUnoTextBase").GlobalNamespace()
            || dc.Class("OInterfaceContainer").Namespace("frm").GlobalNamespace()
            || dc.Class("AccessibleComponentBase").Namespace("accessibility").GlobalNamespace()
            || dc.Class("ContextHandler2Helper")
                   .Namespace("core")
                   .Namespace("oox")
                   .GlobalNamespace()
            || dc.Class("AccessibleStaticTextBase").Namespace("accessibility").GlobalNamespace()
            || dc.Class("OCommonPicker").Namespace("svt").GlobalNamespace()
            || dc.Class("VbaDocumentBase").GlobalNamespace()
            || dc.Class("VbaPageSetupBase").GlobalNamespace()
            || dc.Class("ScVbaControl").GlobalNamespace()

        )
            return true;
    }

    if (methodDecl)
    {
        for (auto iter = methodDecl->begin_overridden_methods();
             iter != methodDecl->end_overridden_methods(); ++iter)
        {
            const CXXMethodDecl* overriddenMethod = (*iter)->getCanonicalDecl();
            if (isSalCallFunction(overriddenMethod))
                return true;
        }
    }

    bool bOK = rewrite(rewriteLoc);
    if (bOK && canonicalDecl != decl)
    {
        bOK = rewrite(rewriteCanonicalLoc);
    }
    if (bOK)
        return true;

    report(DiagnosticsEngine::Warning, "SAL_CALL unnecessary here", rewriteLoc)
        << decl->getSourceRange();
    if (canonicalDecl != decl)
        report(DiagnosticsEngine::Warning, "SAL_CALL unnecessary here", rewriteCanonicalLoc)
            << canonicalDecl->getSourceRange();

    return true;
}

bool SalCall::isSalCallFunction(FunctionDecl const* functionDecl, SourceLocation* pLoc)
{
    // In certain situations, in header files, clang will return bogus range data
    // from decl->getSourceRange().
    // Specifically, for the
    //     LNG_DLLPUBLIC CapType SAL_CALL capitalType(const OUString&, CharClass const *);
    // declaration in
    //     include/linguistic/misc.hxx
    // it looks like it is returning data from definition of the LNG_DLLPUBLIC macro.
    // I suspect something inside clang is calling getSpellingLoc() once too often.
    //
    // So I use getReturnTypeSourceRange() for the start of the range
    // instead, and search for the "(" in the function decl.

    SourceRange range = functionDecl->getSourceRange();
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = functionDecl->getReturnTypeSourceRange().getBegin();
    SourceLocation endLoc = range.getEnd();
    if (!startLoc.isValid() || !endLoc.isValid())
        return false;
    char const* p1 = SM.getCharacterData(startLoc);
    char const* p2 = SM.getCharacterData(endLoc);

    //    if (functionDecl->getIdentifier() && functionDecl->getName() == "capitalType")
    //    {
    //        std::cout << "xxxx " << (long)p1 << " " << (long)p2 << " " << (int)(p2-p1) << std::endl;
    //        std::cout << "  " << std::string(p1, 80) << std::endl;
    //        report(DiagnosticsEngine::Warning, "jhjkahdashdkash", functionDecl->getLocation())
    //            << functionDecl->getSourceRange();
    //    }
    //
    static const char* SAL_CALL = "SAL_CALL";

    char const* leftBracket = static_cast<char const*>(memchr(p1, '(', p2 - p1));
    if (!leftBracket)
        return false;

    char const* found = std::search(p1, leftBracket, SAL_CALL, SAL_CALL + strlen(SAL_CALL));

    if (found >= leftBracket)
        return false;

    if (pLoc)
        // the -1 is to remove the space before the SAL_CALL
        *pLoc = startLoc.getLocWithOffset(found - p1 - 1);

    return true;
}

bool SalCall::rewrite(SourceLocation locBegin)
{
    if (!rewriter)
        return false;
    if (!locBegin.isValid())
        return false;

    auto locEnd = locBegin.getLocWithOffset(8);
    if (!locEnd.isValid())
        return false;

    SourceRange range(locBegin, locEnd);

    // If we overlap with a previous area we modified, we cannot perform this change
    // without corrupting the source
    if (!checkOverlap(range))
        return false;

    if (!replaceText(locBegin, 9, ""))
        return false;

    return true;
}

// If we overlap with a previous area we modified, we cannot perform this change
// without corrupting the source
bool SalCall::checkOverlap(SourceRange range)
{
    SourceManager& SM = compiler.getSourceManager();
    char const* p1 = SM.getCharacterData(range.getBegin());
    char const* p2 = SM.getCharacterData(range.getEnd());
    for (std::pair<char const*, char const*> const& rPair : mvModifiedRanges)
    {
        if (rPair.first <= p1 && p1 <= rPair.second)
            return false;
        if (p1 <= rPair.second && rPair.first <= p2)
            return false;
    }
    mvModifiedRanges.emplace_back(p1, p2);
    return true;
}

static loplugin::Plugin::Registration<SalCall> reg("salcall", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
