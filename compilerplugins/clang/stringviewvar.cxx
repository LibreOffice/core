/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <set>
#include <vector>

#include "config_clang.h"

#include "check.hxx"
#include "functionaddress.hxx"
#include "plugin.hxx"

// Find various of type  rtl::O[U]String that can be converted to std::[u16]string_view instead.

// This can generate false positives e.g.
//   OUString sSave( aToken );
//   aToken.append("foo");
//   aToken = sSave;
// where the data that is backing the view is modified and then the view is used to assign to
// the same source data.

namespace
{
enum class StringType
{
    None,
    RtlOstring,
    RtlOustring
};

StringType relevantStringType(QualType type)
{
    loplugin::TypeCheck const c(type);
    if (c.Class("OString").Namespace("rtl"))
    {
        return StringType::RtlOstring;
    }
    else if (c.Class("OUString").Namespace("rtl"))
    {
        return StringType::RtlOustring;
    }
    else
    {
        return StringType::None;
    }
}

bool relevantVarDecl(VarDecl const* decl)
{
    auto const t1 = decl->getType();
    if (relevantStringType(t1.getNonReferenceType()) == StringType::None)
    {
        return false;
    }
    if (isa<ParmVarDecl>(decl))
    {
        return false;
    }
    return true;
}

DeclRefExpr const* relevantDeclRefExpr(Expr const* expr)
{
    //TODO: Look through BO_Comma and AbstractConditionalOperator
    auto const e = dyn_cast<DeclRefExpr>(expr->IgnoreParenImpCasts());
    if (e == nullptr)
    {
        return nullptr;
    }
    auto const d = dyn_cast<VarDecl>(e->getDecl());
    if (d == nullptr)
    {
        return nullptr;
    }
    if (!relevantVarDecl(d))
    {
        return nullptr;
    }
    return e;
}

bool isStringView(QualType qt)
{
    return bool(loplugin::TypeCheck(qt).ClassOrStruct("basic_string_view").StdNamespace());
}

DeclRefExpr const* relevantImplicitCastExpr(ImplicitCastExpr const* expr)
{
    if (!isStringView(expr->getType()))
    {
        return nullptr;
    }
    return relevantDeclRefExpr(expr->getSubExprAsWritten());
}

DeclRefExpr const* relevantCStyleCastExpr(CStyleCastExpr const* expr)
{
    if (expr->getCastKind() != CK_ToVoid)
    {
        return nullptr;
    }
    return relevantDeclRefExpr(expr->getSubExprAsWritten());
}

DeclRefExpr const* relevantCXXMemberCallExpr(CXXMemberCallExpr const* expr)
{
    StringType t = relevantStringType(expr->getObjectType());
    if (t == StringType::None)
    {
        return nullptr;
    }
    bool good = false;
    auto const d = expr->getMethodDecl();
    if (d->getOverloadedOperator() == OO_Subscript)
    {
        good = true;
    }
    else if (auto const i = d->getIdentifier())
    {
        auto const n = i->getName();
        if (n == "endsWith" || n == "isEmpty" || n == "startsWith" || n == "subView"
            || n == "indexOf" || n == "lastIndexOf" || n == "compareTo" || n == "match"
            || n == "trim" || n == "toInt32" || n == "toInt64" || n == "toDouble"
            || n == "equalsIgnoreAsciiCase" || n == "compareToIgnoreAsciiCase" || n == "getToken"
            || n == "copy" || n == "iterateCodePoints")
        {
            good = true;
        }
#if 0
        //TODO: rtl::O[U]String::getLength would be awkward to replace with
        // std::[u16]string_view::length/size due to the sal_Int32 vs. std::size_t return type
        // mismatch (C++20 ssize might make that easier, though); and while rtl::OString::getStr is
        // documented to be NUL-terminated (so not eligible for replacement with
        // std::string_view::data in general), rtl::OUString::getStr is not (so should be eligible
        // for replacement with std::u16string_view::data, but some call sites might nevertheless
        // incorrectly rely on NUL termination, so any replacement would need careful review):
        if (n == "getLength" || (t == StringType::RtlOustring && n == "getStr"))
        {
            good = true;
        }
#endif
    }
    if (!good)
    {
        return nullptr;
    }
    return relevantDeclRefExpr(expr->getImplicitObjectArgument());
}

SmallVector<DeclRefExpr const*, 2> wrap(DeclRefExpr const* expr)
{
    if (expr == nullptr)
    {
        return {};
    }
    return { expr };
}

SmallVector<DeclRefExpr const*, 2> relevantCXXOperatorCallExpr(CXXOperatorCallExpr const* expr)
{
    auto const op = expr->getOperator();
    if (op == OO_Subscript)
    {
        auto const e = expr->getArg(0);
        if (relevantStringType(e->getType()) == StringType::None)
        {
            return {};
        }
        return wrap(relevantDeclRefExpr(e));
    }
    if (expr->isComparisonOp() || (op == OO_Plus && expr->getNumArgs() == 2))
    {
        SmallVector<DeclRefExpr const*, 2> v;
        if (auto const e = relevantDeclRefExpr(expr->getArg(0)))
        {
            v.push_back(e);
        }
        if (auto const e = relevantDeclRefExpr(expr->getArg(1)))
        {
            v.push_back(e);
        }
        return v;
    }
    if (op == OO_PlusEqual)
    {
        if (relevantStringType(expr->getArg(0)->getType()) != StringType::RtlOustring)
        {
            return {};
        }
        return wrap(relevantDeclRefExpr(expr->getArg(1)));
    }
    if (op == OO_Equal)
    {
        if (!isStringView(expr->getArg(1)->getType()))
        {
            return {};
        }
        return wrap(relevantDeclRefExpr(expr->getArg(0)));
    }
    return {};
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

class StringViewVar final
    : public loplugin::FunctionAddress<loplugin::FilteringPlugin<StringViewVar>>
{
public:
    explicit StringViewVar(loplugin::InstantiationData const& data)
        : FunctionAddress(data)
    {
    }

    bool VisitVarDecl(VarDecl* decl)
    {
        if (ignoreLocation(decl))
        {
            return true;
        }
        if (decl->hasGlobalStorage())
        {
            return true;
        }
        if (!decl->isThisDeclarationADefinition())
        {
            return true;
        }
        if (!relevantVarDecl(decl))
        {
            return true;
        }
        if (decl->getInit())
        {
            auto expr = IgnoreImplicitAndConversionOperator(decl->getInit());
            if (auto castExpr = dyn_cast<CXXFunctionalCastExpr>(expr))
            {
                expr = IgnoreImplicitAndConversionOperator(castExpr->getSubExpr());
            }
            if (auto cxxConstruct = dyn_cast<CXXConstructExpr>(expr))
            {
                if (cxxConstruct->getNumArgs() == 0)
                    currentVars_.insert(decl); // default constructor
                else if (cxxConstruct->getNumArgs() == 1
                         && isStringView(cxxConstruct->getArg(0)->getType()))
                    currentVars_.insert(decl);
            }
        }
        return true;
    }

    bool TraverseImplicitCastExpr(ImplicitCastExpr* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const e = relevantImplicitCastExpr(expr);
        if (e == nullptr)
        {
            return FunctionAddress::TraverseImplicitCastExpr(expr);
        }
        currentGoodUses_.insert(e);
        auto const ret = FunctionAddress::TraverseImplicitCastExpr(expr);
        currentGoodUses_.erase(e);
        return ret;
    }

    bool TraverseCStyleCastExpr(CStyleCastExpr* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const e = relevantCStyleCastExpr(expr);
        if (e == nullptr)
        {
            return FunctionAddress::TraverseCStyleCastExpr(expr);
        }
        currentGoodUses_.insert(e);
        auto const ret = FunctionAddress::TraverseCStyleCastExpr(expr);
        currentGoodUses_.erase(e);
        return ret;
    }

    bool TraverseCXXMemberCallExpr(CXXMemberCallExpr* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const e = relevantCXXMemberCallExpr(expr);
        if (e == nullptr)
        {
            return FunctionAddress::TraverseCXXMemberCallExpr(expr);
        }
        currentGoodUses_.insert(e);
        auto const ret = FunctionAddress::TraverseCXXMemberCallExpr(expr);
        currentGoodUses_.erase(e);
        return ret;
    }

    bool TraverseCXXOperatorCallExpr(CXXOperatorCallExpr* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const es = relevantCXXOperatorCallExpr(expr);
        if (es.empty())
        {
            return FunctionAddress::TraverseCXXOperatorCallExpr(expr);
        }
        currentGoodUses_.insert(es.begin(), es.end());
        auto const ret = FunctionAddress::TraverseCXXOperatorCallExpr(expr);
        for (auto const i : es)
        {
            currentGoodUses_.erase(i);
        }
        return ret;
    }

    bool VisitDeclRefExpr(DeclRefExpr* expr)
    {
        if (!FunctionAddress::VisitDeclRefExpr(expr))
        {
            return false;
        }
        if (ignoreLocation(expr))
        {
            return true;
        }
        if (currentGoodUses_.find(expr) != currentGoodUses_.end())
        {
            return true;
        }
        if (auto const d = dyn_cast<VarDecl>(expr->getDecl()))
        {
            currentVars_.erase(d);
        }
        return true;
    }

private:
    void run() override
    {
        if (!compiler.getLangOpts().CPlusPlus)
        {
            return;
        }
        if (compiler.getPreprocessor().getIdentifierInfo("NDEBUG")->hasMacroDefinition())
        {
            return;
        }
        StringRef fn(handler.getMainFileName());
        // leave the string QA tests alone
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sal/qa/"))
        {
            return;
        }
        // false +
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/svtools/source/svrtf/parrtf.cxx"))
        {
            return;
        }
        if (!TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()))
        {
            return;
        }
        for (auto const i : currentVars_)
        {
            auto const t = relevantStringType(i->getType().getNonReferenceType());
            report(DiagnosticsEngine::Warning,
                   "replace var of type %0 with "
                   "'%select{std::string_view|std::u16string_view}1'",
                   i->getLocation())
                << i->getType() << (int(t) - 1) << i->getSourceRange();
        }
    }

    std::set<VarDecl const*> currentVars_;
    std::set<DeclRefExpr const*> currentGoodUses_;
};

static loplugin::Plugin::Registration<StringViewVar> reg("stringviewvar",
                                                         false); // might generate false+
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
