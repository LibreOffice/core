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

#include "check.hxx"
#include "compat.hxx"
#include "functionaddress.hxx"
#include "plugin.hxx"

// Find functions that take rtl::O[U]String parameters that can be generalized to take
// std::[u16]string_view instead.

//TODO: At least theoretically, there are issues with replacing parameters that are being assigned
// to, as in
//
//  void f(OUString s) {
//    {
//      OUString t = ...;
//      s = t;
//    }
//    ... use s ... // if s is now std::u16string_view, it points into destroyed contents of t
//  }

namespace
{
bool hasSalDllpublicExportAttr(FunctionDecl const* decl)
{
    if (auto const attr = decl->getAttr<VisibilityAttr>())
    {
        return attr->getVisibility() == VisibilityAttr::Default;
    }
    return decl->hasAttr<DLLExportAttr>();
}

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

bool isRelevantStringBufferType(QualType type)
{
    loplugin::TypeCheck const c(type);
    if (c.Class("OStringBuffer").Namespace("rtl"))
    {
        return true;
    }
    else if (c.Class("OUStringBuffer").Namespace("rtl"))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool relevantParmVarDecl(ParmVarDecl const* decl)
{
    auto const t1 = decl->getType();
    if (auto const t2 = t1->getAs<LValueReferenceType>())
    {
        if (!t2->getPointeeType().isConstQualified())
        {
            return false;
        }
    }
    if (relevantStringType(t1.getNonReferenceType()) == StringType::None)
    {
        return false;
    }
    if (decl->hasAttr<UnusedAttr>())
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
    auto const d = dyn_cast<ParmVarDecl>(e->getDecl());
    if (d == nullptr)
    {
        return nullptr;
    }
    if (!relevantParmVarDecl(d))
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

DeclRefExpr const* relevantCXXMemberCallExpr(CXXMemberCallExpr const* expr)
{
    auto const d = expr->getMethodDecl();
    if (isRelevantStringBufferType(compat::getObjectType(expr)))
    {
        if (auto const i = d->getIdentifier())
        {
            auto const n = i->getName();
            if (n == "append" || n == "indexOf" || n == "lastIndexOf")
            {
                return relevantDeclRefExpr(expr->getArg(0));
            }
            else if (n == "insert")
            {
                return relevantDeclRefExpr(expr->getArg(1));
            }
        }
    }
    StringType t = relevantStringType(compat::getObjectType(expr));
    if (t == StringType::None)
    {
        return nullptr;
    }
    bool good = false;
    if (d->getOverloadedOperator() == OO_Subscript)
    {
        good = true;
    }
    else if (auto const i = d->getIdentifier())
    {
        auto const n = i->getName();
        if (n == "isEmpty" || n == "subView")
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
    if (compat::isComparisonOp(expr) || (op == OO_Plus && expr->getNumArgs() == 2))
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
    return {};
}

//TODO: current implementation is not at all general, just tests what we encounter in practice:
bool hasStringViewOverload(ParmVarDecl const* decl)
{
    auto const d1 = cast<FunctionDecl>(decl->getDeclContext());
    auto const ctx = d1->getDeclContext();
    if (!ctx->isLookupContext())
    {
        return false;
    }
    auto const res = ctx->lookup(d1->getDeclName());
    auto const idx = decl->getFunctionScopeIndex();
    auto const n = d1->getNumParams();
    assert(n > idx);
    for (auto i = res.begin(); i != res.end(); ++i)
    {
        auto const d2 = dyn_cast<FunctionDecl>(*i);
        if (d2 == nullptr)
        {
            continue;
        }
        if (d2->getNumParams() != n)
        {
            continue;
        }
        auto match = true;
        for (unsigned j = 0; j != n; ++j)
        {
            if (j == idx)
            {
                //TODO: check for exactly std::string_view or std::u16string_view:
                if (!isStringView(d2->getParamDecl(j)->getType()))
                {
                    match = false;
                    break;
                }
            }
            else if (d1->getParamDecl(j)->getType().getCanonicalType()
                     != d2->getParamDecl(j)->getType().getCanonicalType())
            {
                match = false;
                break;
            }
        }
        if (match)
        {
            return true;
        }
    }
    return false;
}

class StringViewParam final
    : public loplugin::FunctionAddress<loplugin::FilteringPlugin<StringViewParam>>
{
public:
    explicit StringViewParam(loplugin::InstantiationData const& data)
        : FunctionAddress(data)
    {
    }

    //TODO: Also check lambdas
    bool TraverseFunctionDecl(FunctionDecl* decl)
    {
        if (ignoreLocation(decl))
        {
            return true;
        }
        if (!relevantFunctionDecl(decl))
        {
            return FunctionAddress::TraverseFunctionDecl(decl);
        }
        auto const oldParams = currentParams_;
        auto const n = decl->getNumParams();
        for (unsigned i = 0; i != n; ++i)
        {
            auto const d = decl->getParamDecl(i);
            if (relevantParmVarDecl(d))
            {
                currentParams_.insert(d);
            }
        }
        auto const ret = FunctionAddress::TraverseFunctionDecl(decl);
        if (ret)
        {
            for (unsigned i = 0; i != n; ++i)
            {
                auto const d1 = decl->getParamDecl(i);
                if (currentParams_.find(d1) == currentParams_.end())
                {
                    continue;
                }
                if (containsPreprocessingConditionalInclusion(decl->getSourceRange()))
                {
                    break;
                }
                badParams_.push_back(d1);
            }
        }
        currentParams_ = oldParams;
        return ret;
    }

    bool TraverseCXXMethodDecl(CXXMethodDecl* decl)
    {
        if (ignoreLocation(decl))
        {
            return true;
        }
        if (!relevantFunctionDecl(decl))
        {
            return FunctionAddress::TraverseCXXMethodDecl(decl);
        }
        auto const oldParams = currentParams_;
        auto const n = decl->getNumParams();
        for (unsigned i = 0; i != n; ++i)
        {
            auto const d = decl->getParamDecl(i);
            if (relevantParmVarDecl(d))
            {
                currentParams_.insert(d);
            }
        }
        auto const ret = FunctionAddress::TraverseCXXMethodDecl(decl);
        if (ret)
        {
            for (unsigned i = 0; i != n; ++i)
            {
                auto const d1 = decl->getParamDecl(i);
                if (currentParams_.find(d1) == currentParams_.end())
                {
                    continue;
                }
                if (containsPreprocessingConditionalInclusion(decl->getSourceRange()))
                {
                    break;
                }
                badParams_.push_back(d1);
            }
        }
        currentParams_ = oldParams;
        return ret;
    }

    bool TraverseCXXConstructorDecl(CXXConstructorDecl* decl)
    {
        if (ignoreLocation(decl))
        {
            return true;
        }
        if (!relevantFunctionDecl(decl))
        {
            return FunctionAddress::TraverseCXXConstructorDecl(decl);
        }
        auto const oldParams = currentParams_;
        auto const n = decl->getNumParams();
        for (unsigned i = 0; i != n; ++i)
        {
            auto const d = decl->getParamDecl(i);
            if (relevantParmVarDecl(d))
            {
                currentParams_.insert(d);
            }
        }
        auto const ret = FunctionAddress::TraverseCXXConstructorDecl(decl);
        if (ret)
        {
            for (unsigned i = 0; i != n; ++i)
            {
                auto const d1 = decl->getParamDecl(i);
                if (currentParams_.find(d1) == currentParams_.end())
                {
                    continue;
                }
                if (containsPreprocessingConditionalInclusion(decl->getSourceRange()))
                {
                    break;
                }
                badParams_.push_back(d1);
            }
        }
        currentParams_ = oldParams;
        return ret;
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
        if (auto const d = dyn_cast<ParmVarDecl>(expr->getDecl()))
        {
            currentParams_.erase(d);
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
        StringRef fn(handler.getMainFileName());
        // leave the string QA tests alone
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sal/qa/"))
        {
            return;
        }
        if (!TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()))
        {
            return;
        }
        auto const ignoredFns = getFunctionsWithAddressTaken();
        for (auto const i : badParams_)
        {
            auto const d1 = cast<FunctionDecl>(i->getDeclContext());
            if (ignoredFns.find(d1) != ignoredFns.end())
            {
                continue;
            }
            if (hasStringViewOverload(i))
            {
                continue;
            }
            auto const t = relevantStringType(i->getType().getNonReferenceType());
            assert(t != StringType::None);
            report(DiagnosticsEngine::Warning,
                   "replace function parameter of type %0 with "
                   "'%select{std::string_view|std::u16string_view}1'",
                   i->getLocation())
                << i->getType() << (int(t) - 1) << i->getSourceRange();
            for (auto d2 = d1;;)
            {
                d2 = d2->getPreviousDecl();
                if (d2 == nullptr)
                {
                    break;
                }
                auto const d3 = d2->getParamDecl(i->getFunctionScopeIndex());
                report(DiagnosticsEngine::Note, "previous declaration is here", d3->getLocation())
                    << d3->getSourceRange();
            }
        }
    }

    bool relevantFunctionDecl(FunctionDecl const* decl)
    {
        if (!decl->doesThisDeclarationHaveABody())
        {
            return false;
        }
        if (decl->getBody() == nullptr) // unparsed template
        {
            return false;
        }
        if (auto const d = dyn_cast<CXXMethodDecl>(decl))
        {
            if (d->isVirtual())
            {
                return false;
            }
        }
        if (decl->isFunctionTemplateSpecialization())
        {
            return false;
        }
        if (decl->getLocation().isMacroID())
        {
            return false;
        }
        // Filter out functions that are presumably meant to be called dynamically (e.g., via
        // dlopen, or backwards compatibility stubs in cppuhelper/cppu/sal compat.cxx):
        if (decl->getPreviousDecl() == nullptr && !decl->isInlined()
            && hasSalDllpublicExportAttr(decl)
            && compiler.getSourceManager().isInMainFile(decl->getLocation()))
        {
            return false;
        }
        if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(decl->getLocation())))
        {
            return false;
        }
        return true;
    }

    std::set<ParmVarDecl const*> currentParams_;
    std::set<DeclRefExpr const*> currentGoodUses_;
    std::vector<ParmVarDecl const*> badParams_;
};

static loplugin::Plugin::Registration<StringViewParam> reg("stringviewparam");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
