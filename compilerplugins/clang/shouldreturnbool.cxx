/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <set>
#include <iostream>

#include "check.hxx"
#include "plugin.hxx"
#include "functionaddress.hxx"

/*
 Look for functions that only return 1 and/or 0, which sometimes indicates that the
 function should be returning bool.

 Note that is partly a question of taste and code style, which is why this plugin is off by default.
*/

namespace
{
class ShouldReturnBool
    : public loplugin::FunctionAddress<loplugin::FilteringPlugin<ShouldReturnBool>>
{
public:
    explicit ShouldReturnBool(loplugin::InstantiationData const& data)
        : FunctionAddress(data)
    {
    }

    virtual void run() override
    {
        if (!compiler.getLangOpts().CPlusPlus)
            return;
        StringRef fn(handler.getMainFileName());
        // functions used as function pointers
        if (loplugin::isSamePathname(fn, SRCDIR "/sal/rtl/alloc_cache.cxx"))
            return;
        // false +, slightly odd usage, but not wrong
        if (loplugin::isSamePathname(fn, SRCDIR "/libreofficekit/qa/tilebench/tilebench.cxx"))
            return;
        // uses the Unix convention of "non-zero return indicates error"
        if (loplugin::isSamePathname(fn, SRCDIR "/idlc/source/idlcproduce.cxx"))
            return;
        // template magic
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/gdi/bmpfast.cxx"))
            return;
        // fine
        if (loplugin::isSamePathname(fn, SRCDIR "/svl/unx/source/svdde/ddedummy.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/opengl/OpenGLHelper.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svtools/source/misc/imap2.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/dialog/docrecovery.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/hwpfilter/source/lexer.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/hwpfilter/source/grammar.cxx"))
            return;
        if (loplugin::isSamePathname(
                fn, SRCDIR "/connectivity/source/drivers/odbc/ODatabaseMetaDataResultSet.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/dbaccess/source/ui/browser/dsEntriesNoExp.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/explode.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/filter/source/graphicfilter/ipict/ipict.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/core/data/dptabsrc.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/docshell/docsh3.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/dlg/masterlayoutdlg.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/filter/ppt/pptinanimations.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/unx/generic/app/i18n_im.cxx"))
            return;

        // callback
        if (loplugin::isSamePathname(fn, SRCDIR "/sax/source/expatwrap/sax_expat.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/xmlsecurity/source/xmlsec/xmlstreamio.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/filter/ww8/ww8par.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/filter/ww8/ww8par2.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/filter/ww8/ww8par5.cxx"))
            return;
        // SaxWriterHelper::writeSequence a little weird
        if (loplugin::isSamePathname(fn, SRCDIR "/sax/source/expatwrap/saxwriter.cxx"))
            return;
        // main function
        if (loplugin::isSamePathname(fn, SRCDIR "/xmlsecurity/workben/pdfverify.cxx"))
            return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        for (auto functionDecl : problemFunctions)
        {
            auto canonicalDecl = functionDecl->getCanonicalDecl();
            if (getFunctionsWithAddressTaken().find(canonicalDecl)
                != getFunctionsWithAddressTaken().end())
                continue;
            report(DiagnosticsEngine::Warning,
                   "only returning one or zero is an indication you want to return bool",
                   compat::getBeginLoc(functionDecl))
                << functionDecl->getSourceRange();
            if (canonicalDecl->getLocation() != functionDecl->getLocation())
            {
                report(DiagnosticsEngine::Note, "canonical function declaration here",
                       compat::getBeginLoc(canonicalDecl))
                    << canonicalDecl->getSourceRange();
            }
        }
    }

    bool TraverseFunctionDecl(FunctionDecl*);
    bool TraverseCXXMethodDecl(CXXMethodDecl*);
    bool VisitReturnStmt(ReturnStmt const*);

private:
    bool mbInsideFunction = false;
    bool mbFunctionOnlyReturningOneOrZero = false;
    std::unordered_set<FunctionDecl const*> problemFunctions;

    bool IsInteresting(FunctionDecl const*);
    void Report(FunctionDecl const*) const;
    bool isExprOneOrZero(Expr const*) const;
};

bool ShouldReturnBool::TraverseFunctionDecl(FunctionDecl* functionDecl)
{
    bool ret;
    if (IsInteresting(functionDecl))
    {
        mbInsideFunction = true;
        mbFunctionOnlyReturningOneOrZero = true;
        ret = FunctionAddress::TraverseFunctionDecl(functionDecl);
        mbInsideFunction = false;
        if (mbFunctionOnlyReturningOneOrZero)
            problemFunctions.insert(functionDecl);
    }
    else
        ret = FunctionAddress::TraverseFunctionDecl(functionDecl);
    return ret;
}

bool ShouldReturnBool::TraverseCXXMethodDecl(CXXMethodDecl* methodDecl)
{
    bool ret;
    if (IsInteresting(methodDecl))
    {
        mbInsideFunction = true;
        mbFunctionOnlyReturningOneOrZero = true;
        ret = FunctionAddress::TraverseCXXMethodDecl(methodDecl);
        mbInsideFunction = false;
        if (mbFunctionOnlyReturningOneOrZero)
            problemFunctions.insert(methodDecl);
    }
    else
        ret = FunctionAddress::TraverseCXXMethodDecl(methodDecl);
    return ret;
}

bool ShouldReturnBool::IsInteresting(FunctionDecl const* functionDecl)
{
    if (ignoreLocation(functionDecl))
        return false;
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(functionDecl))
        return false;
    if (functionDecl->getTemplatedKind() != FunctionDecl::TK_NonTemplate)
        return false;
    if (!functionDecl->isThisDeclarationADefinition())
        return false;
    if (functionDecl->isMain())
        return false;
    if (functionDecl->isExternC() || functionDecl->isInExternCContext())
        return false;
    auto methodDecl = dyn_cast<CXXMethodDecl>(functionDecl);
    if (methodDecl && methodDecl->isVirtual())
        return false;
    auto tc = loplugin::TypeCheck(functionDecl->getReturnType());
    if (tc.AnyBoolean() || tc.Void())
        return false;
    auto returnType = functionDecl->getReturnType();
    if (returnType->isEnumeralType() || !returnType->getUnqualifiedDesugaredType()->isIntegerType())
        return false;
    // Ignore functions that contains #ifdef-ery
    if (containsPreprocessingConditionalInclusion(functionDecl->getSourceRange()))
        return false;

    // not sure what basegfx is doing here
    StringRef fileName{ getFilenameOfLocation(functionDecl->getLocation()) };
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/basegfx/range/basicrange.hxx"))
        return false;
    // false +
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/svl/macitem.hxx"))
        return false;
    if (loplugin::isSamePathname(fileName, SRCDIR "/lotuswordpro/source/filter/lwpcharsetmgr.hxx"))
        return false;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sc/inc/dptabsrc.hxx"))
        return false;

    return true;
}

bool ShouldReturnBool::VisitReturnStmt(const ReturnStmt* returnStmt)
{
    if (!mbInsideFunction)
        return true;
    if (!returnStmt->getRetValue())
        return true;
    if (loplugin::TypeCheck(returnStmt->getRetValue()->getType()).AnyBoolean())
        return true;
    if (!isExprOneOrZero(returnStmt->getRetValue()))
        mbFunctionOnlyReturningOneOrZero = false;
    return true;
}

bool ShouldReturnBool::isExprOneOrZero(const Expr* arg) const
{
    arg = arg->IgnoreParenCasts();
    // ignore this, it seems to trigger an infinite recursion
    if (isa<UnaryExprOrTypeTraitExpr>(arg))
    {
        return false;
    }
    APSInt x1;
    if (compat::EvaluateAsInt(arg, x1, compiler.getASTContext()))
    {
        return x1 == 1 || x1 == 0;
    }
    return false;
}

loplugin::Plugin::Registration<ShouldReturnBool> X("shouldreturnbool", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
