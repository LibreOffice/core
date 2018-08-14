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
#include "compat.hxx"
#include "functionaddress.hxx"
#include <iostream>
#include <set>
#include <string>

/*
  Look for member functions that merely return a compile-time constant, or they are empty, and can thus
  be either removed, or converted into a constant.

  This mostly tends to happen as a side-effect of other cleanups.
*/
namespace
{
class ReturnConstant : public loplugin::FunctionAddress<ReturnConstant>
{
public:
    explicit ReturnConstant(loplugin::InstantiationData const& data)
        : loplugin::FunctionAddress<ReturnConstant>(data)
    {
    }

    void run() override
    {
        StringRef fn(handler.getMainFileName());
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/store"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/registry"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/idlc"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/include/unotools"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/xmlreader/"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/jvmfwk"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/vcl"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/svtools"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/basic"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/basctl"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/canvas")) // TODO
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sax"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/oox"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/cui"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/desktop"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/svl/unx/source/svdde/ddedummy.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sfx2/source/appl/shutdownicon.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sfx2/source/control/minfitem.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sfx2/source/doc/docmacromode.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn,
                                        SRCDIR "/ucb/source/cacher/contentresultsetwrapper.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/editeng/source/editeng/impedit2.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sfx2/source/doc/syspath.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/xmloff/source/core/SvXMLAttrCollection.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/test/source/diff/diff.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/svx/source/items/SmartTagItem.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/svx/source/dialog/rulritem.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/svx/source/svdraw/svdograf.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/filter/source/graphicfilter/ipict/ipict.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/filter/source/svg/svgwriter.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(
                fn, SRCDIR "/connectivity/source/drivers/odbc/ODatabaseMetaDataResultSet.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sd/"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/core/data/attrib.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/core/data/dptabsrc.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/core/data/formulacell.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/ui/docshell/docfunc.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/access/accpara.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR
                                        "/sw/source/core/doc/DocumentContentOperationsManager.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/doc/tblcpy.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/edit/autofmt.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/edit/edglss.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/table/swnewtable.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/filter/xml/xmlfmt.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/reportdesign/source/ui/dlg/AddField.cxx"))
            return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        for (auto& pair : problemFunctions)
        {
            auto functionDecl = pair.first;
            auto canonicalDecl = functionDecl->getCanonicalDecl();
            if (getFunctionsWithAddressTaken().find(canonicalDecl)
                != getFunctionsWithAddressTaken().end())
                continue;
            report(DiagnosticsEngine::Warning,
                   "Method only returns a single constant value %0, does it make sense?",
                   compat::getBeginLoc(functionDecl))
                << pair.second << functionDecl->getSourceRange();
            if (functionDecl != functionDecl->getCanonicalDecl())
                report(DiagnosticsEngine::Note, "decl here",
                       compat::getBeginLoc(functionDecl->getCanonicalDecl()))
                    << functionDecl->getCanonicalDecl()->getSourceRange();
        }
    }

    bool TraverseCXXMethodDecl(CXXMethodDecl*);
    bool VisitReturnStmt(ReturnStmt const*);

private:
    std::string getExprValue(Expr const* arg);

    struct Context
    {
        bool ignore = false;
        std::set<std::string> values;
    };
    std::vector<Context> m_functionStack;
    std::vector<std::pair<FunctionDecl const*, std::string>> problemFunctions;
};

bool ReturnConstant::TraverseCXXMethodDecl(CXXMethodDecl* functionDecl)
{
    if (ignoreLocation(functionDecl))
        return true;
    if (isInUnoIncludeFile(functionDecl))
        return true;

    if (!functionDecl->hasBody())
        return true;
    if (!functionDecl->isThisDeclarationADefinition())
        return true;
    if (functionDecl->isConstexpr())
        return true;
    if (functionDecl->getReturnType()->isVoidType())
        return true;
    if (functionDecl->isVirtual())
        return true;
    // static with inline body will be optimised at compile-time to a constant anyway
    if (functionDecl->isStatic()
        && (functionDecl->hasInlineBody() || functionDecl->isInlineSpecified()))
        return true;
    // this catches some stuff in templates
    if (functionDecl->hasAttr<OverrideAttr>())
        return true;

    // include/unotools/localedatawrapper.hxx
    if (functionDecl->getIdentifier() && functionDecl->getName() == "getCurrZeroChar")
        return true;
    // sc/inc/stlalgorithm.hxx
    if (loplugin::DeclCheck(functionDecl->getParent())
            .Class("AlignedAllocator")
            .Namespace("sc")
            .GlobalNamespace())
        return true;

    switch (functionDecl->getOverloadedOperator())
    {
        case OO_Delete:
        case OO_EqualEqual:
        case OO_Call:
            return true;
        default:
            break;
    }

    // gtk signals and slots stuff
    if (loplugin::TypeCheck(functionDecl->getReturnType()).Typedef("gboolean"))
        return true;

    // ignore LINK macro stuff
    std::string aImmediateMacro = "";
    if (compiler.getSourceManager().isMacroBodyExpansion(compat::getBeginLoc(functionDecl)))
    {
        StringRef name{ Lexer::getImmediateMacroName(compat::getBeginLoc(functionDecl),
                                                     compiler.getSourceManager(),
                                                     compiler.getLangOpts()) };
        aImmediateMacro = name;
        if (name.contains("IMPL_LINK") || name.contains("IMPL_STATIC_LINK")
            || name.contains("SFX_IMPL_POS_CHILDWINDOW_WITHID"))
            return true;
    }

    m_functionStack.emplace_back();
    bool ret = RecursiveASTVisitor<ReturnConstant>::TraverseCXXMethodDecl(functionDecl);
    Context& rContext = m_functionStack.back();
    if (!rContext.ignore && rContext.values.size() == 1
        && rContext.values.find("unknown") == rContext.values.end())
    {
        problemFunctions.push_back({ functionDecl, *rContext.values.begin() });
    }
    m_functionStack.pop_back();
    return ret;
}

bool ReturnConstant::VisitReturnStmt(ReturnStmt const* returnStmt)
{
    if (ignoreLocation(returnStmt))
        return true;
    if (m_functionStack.empty())
        return true;
    Context& rContext = m_functionStack.back();

    if (!returnStmt->getRetValue())
        return true;
    if (returnStmt->getRetValue()->isTypeDependent())
    {
        rContext.ignore = true;
        return true;
    }
    if (const UnaryOperator* unaryOp = dyn_cast<UnaryOperator>(returnStmt->getRetValue()))
    {
        if (unaryOp->getOpcode() == UO_AddrOf)
        {
            rContext.ignore = true;
            return true;
        }
    }
    rContext.values.insert(getExprValue(returnStmt->getRetValue()));
    return true;
}

std::string ReturnConstant::getExprValue(Expr const* arg)
{
    arg = arg->IgnoreParenCasts();
    if (isa<CXXDefaultArgExpr>(arg))
    {
        arg = dyn_cast<CXXDefaultArgExpr>(arg)->getExpr();
    }
    arg = arg->IgnoreParenCasts();
    // ignore this, it seems to trigger an infinite recursion
    if (isa<UnaryExprOrTypeTraitExpr>(arg))
    {
        return "unknown";
    }
    APSInt x1;
    if (arg->EvaluateAsInt(x1, compiler.getASTContext()))
    {
        return x1.toString(10);
    }
    if (isa<CXXNullPtrLiteralExpr>(arg))
    {
        return "0";
    }
    if (isa<MaterializeTemporaryExpr>(arg))
    {
        const CXXBindTemporaryExpr* strippedArg
            = dyn_cast_or_null<CXXBindTemporaryExpr>(arg->IgnoreParenCasts());
        if (strippedArg && strippedArg->getSubExpr())
        {
            auto temp = dyn_cast<CXXTemporaryObjectExpr>(strippedArg->getSubExpr());
            if (temp->getNumArgs() == 0)
            {
                if (loplugin::TypeCheck(temp->getType())
                        .Class("OUString")
                        .Namespace("rtl")
                        .GlobalNamespace())
                {
                    return "\"\"";
                }
                if (loplugin::TypeCheck(temp->getType())
                        .Class("OString")
                        .Namespace("rtl")
                        .GlobalNamespace())
                {
                    return "\"\"";
                }
            }
        }
    }
    return "unknown";
}

loplugin::Plugin::Registration<ReturnConstant> X("returnconstant", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
