/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cassert>
#include <deque>
#include <string>
#include <iostream>
#include <fstream>
#include <set>

#include <clang/AST/CXXInheritance.h>
#include "plugin.hxx"
#include "check.hxx"
#include "compat.hxx"

/**
  Simplify boolean expressions involving smart pointers e.g.
    if (x.get())
  can be
    if (x)
*/
#ifndef LO_CLANG_SHARED_PLUGINS

namespace
{
class SimplifyPointerToBool : public loplugin::FilteringRewritePlugin<SimplifyPointerToBool>
{
public:
    explicit SimplifyPointerToBool(loplugin::InstantiationData const& data)
        : FilteringRewritePlugin(data)
    {
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitImplicitCastExpr(ImplicitCastExpr const*);

    bool PreTraverseUnaryLNot(UnaryOperator* expr)
    {
        contextuallyConvertedExprs_.push_back(expr->getSubExpr()->IgnoreParenImpCasts());
        return true;
    }

    bool PostTraverseUnaryLNot(UnaryOperator*, bool)
    {
        assert(!contextuallyConvertedExprs_.empty());
        contextuallyConvertedExprs_.pop_back();
        return true;
    }

    bool TraverseUnaryLNot(UnaryOperator* expr)
    {
        auto res = PreTraverseUnaryLNot(expr);
        assert(res);
        res = FilteringRewritePlugin::TraverseUnaryLNot(expr);
        PostTraverseUnaryLNot(expr, res);
        return res;
    }

    bool PreTraverseBinLAnd(BinaryOperator* expr)
    {
        contextuallyConvertedExprs_.push_back(expr->getLHS()->IgnoreParenImpCasts());
        contextuallyConvertedExprs_.push_back(expr->getRHS()->IgnoreParenImpCasts());
        return true;
    }

    bool PostTraverseBinLAnd(BinaryOperator*, bool)
    {
        assert(contextuallyConvertedExprs_.size() >= 2);
        contextuallyConvertedExprs_.pop_back();
        contextuallyConvertedExprs_.pop_back();
        return true;
    }

    bool TraverseBinLAnd(BinaryOperator* expr)
    {
        auto res = PreTraverseBinLAnd(expr);
        assert(res);
        res = FilteringRewritePlugin::TraverseBinLAnd(expr);
        PostTraverseBinLAnd(expr, res);
        return res;
    }

    bool PreTraverseBinLOr(BinaryOperator* expr)
    {
        contextuallyConvertedExprs_.push_back(expr->getLHS()->IgnoreParenImpCasts());
        contextuallyConvertedExprs_.push_back(expr->getRHS()->IgnoreParenImpCasts());
        return true;
    }

    bool PostTraverseBinLOr(BinaryOperator*, bool)
    {
        assert(contextuallyConvertedExprs_.size() >= 2);
        contextuallyConvertedExprs_.pop_back();
        contextuallyConvertedExprs_.pop_back();
        return true;
    }

    bool TraverseBinLOr(BinaryOperator* expr)
    {
        auto res = PreTraverseBinLOr(expr);
        assert(res);
        res = FilteringRewritePlugin::TraverseBinLOr(expr);
        PostTraverseBinLOr(expr, res);
        return res;
    }

    bool PreTraverseConditionalOperator(ConditionalOperator* expr)
    {
        contextuallyConvertedExprs_.push_back(expr->getCond()->IgnoreParenImpCasts());
        return true;
    }

    bool PostTraverseConditionalOperator(ConditionalOperator*, bool)
    {
        assert(!contextuallyConvertedExprs_.empty());
        contextuallyConvertedExprs_.pop_back();
        return true;
    }

    bool TraverseConditionalOperator(ConditionalOperator* expr)
    {
        auto res = PreTraverseConditionalOperator(expr);
        assert(res);
        res = FilteringRewritePlugin::TraverseConditionalOperator(expr);
        PostTraverseConditionalOperator(expr, res);
        return res;
    }

    bool PreTraverseIfStmt(IfStmt* stmt)
    {
        contextuallyConvertedExprs_.push_back(stmt->getCond()->IgnoreParenImpCasts());
        return true;
    }

    bool PostTraverseIfStmt(IfStmt*, bool)
    {
        assert(!contextuallyConvertedExprs_.empty());
        contextuallyConvertedExprs_.pop_back();
        return true;
    }

    bool TraverseIfStmt(IfStmt* stmt)
    {
        auto res = PreTraverseIfStmt(stmt);
        assert(res);
        res = FilteringRewritePlugin::TraverseIfStmt(stmt);
        PostTraverseIfStmt(stmt, res);
        return res;
    }

    bool PreTraverseWhileStmt(WhileStmt* stmt)
    {
        contextuallyConvertedExprs_.push_back(stmt->getCond()->IgnoreParenImpCasts());
        return true;
    }

    bool PostTraverseWhileStmt(WhileStmt*, bool)
    {
        assert(!contextuallyConvertedExprs_.empty());
        contextuallyConvertedExprs_.pop_back();
        return true;
    }

    bool TraverseWhileStmt(WhileStmt* stmt)
    {
        auto res = PreTraverseWhileStmt(stmt);
        assert(res);
        res = FilteringRewritePlugin::TraverseWhileStmt(stmt);
        PostTraverseWhileStmt(stmt, res);
        return res;
    }

    bool PreTraverseDoStmt(DoStmt* stmt)
    {
        contextuallyConvertedExprs_.push_back(stmt->getCond()->IgnoreParenImpCasts());
        return true;
    }

    bool PostTraverseDoStmt(DoStmt*, bool)
    {
        assert(!contextuallyConvertedExprs_.empty());
        contextuallyConvertedExprs_.pop_back();
        return true;
    }

    bool TraverseDoStmt(DoStmt* stmt)
    {
        auto res = PreTraverseDoStmt(stmt);
        assert(res);
        res = FilteringRewritePlugin::TraverseDoStmt(stmt);
        PostTraverseDoStmt(stmt, res);
        return res;
    }

    bool PreTraverseForStmt(ForStmt* stmt)
    {
        auto const e = stmt->getCond();
        if (e != nullptr)
        {
            contextuallyConvertedExprs_.push_back(e->IgnoreParenImpCasts());
        }
        return true;
    }

    bool PostTraverseForStmt(ForStmt* stmt, bool)
    {
        if (stmt->getCond() != nullptr)
        {
            assert(!contextuallyConvertedExprs_.empty());
            contextuallyConvertedExprs_.pop_back();
        }
        return true;
    }

    bool TraverseForStmt(ForStmt* stmt)
    {
        auto res = PreTraverseForStmt(stmt);
        assert(res);
        res = FilteringRewritePlugin::TraverseForStmt(stmt);
        PostTraverseForStmt(stmt, res);
        return res;
    }

private:
    bool isContextuallyConverted(Expr const* expr) const
    {
        return std::find(contextuallyConvertedExprs_.begin(), contextuallyConvertedExprs_.end(),
                         expr)
               != contextuallyConvertedExprs_.end();
    }

    // Get the source range starting at the "."or "->" (plus any preceding non-comment white space):
    SourceRange getCallSourceRange(CXXMemberCallExpr const* expr) const
    {
        if (expr->getImplicitObjectArgument() == nullptr)
        {
            //TODO: Arguably, such a call of a `get` member function from within some member
            // function (so that syntactically no caller is mentioned) should already be handled
            // differently when reporting it (just "drop the get()" does not make sense), instead of
            // being fitered here:
            return {};
        }
        // CXXMemberCallExpr::getExprLoc happens to return the location following the "." or "->":
        auto start = compiler.getSourceManager().getSpellingLoc(expr->getExprLoc());
        if (!start.isValid())
        {
            return {};
        }
        for (;;)
        {
            start = Lexer::GetBeginningOfToken(start.getLocWithOffset(-1),
                                               compiler.getSourceManager(), compiler.getLangOpts());
            auto const s = StringRef(compiler.getSourceManager().getCharacterData(start),
                                     Lexer::MeasureTokenLength(start, compiler.getSourceManager(),
                                                               compiler.getLangOpts()));
            if (s.empty() || s.startswith("\\\n"))
            {
                continue;
            }
            if (s != "." && s != "->")
            {
                return {};
            }
            break;
        }
        for (;;)
        {
            auto start1 = Lexer::GetBeginningOfToken(
                start.getLocWithOffset(-1), compiler.getSourceManager(), compiler.getLangOpts());
            auto const s = StringRef(compiler.getSourceManager().getCharacterData(start1),
                                     Lexer::MeasureTokenLength(start1, compiler.getSourceManager(),
                                                               compiler.getLangOpts()));
            if (!(s.empty() || s.startswith("\\\n")))
            {
                break;
            }
            start = start1;
        }
        return SourceRange(start,
                           compiler.getSourceManager().getSpellingLoc(compat::getEndLoc(expr)));
    }

    //TODO: There are some more places where an expression is contextually converted to bool, but
    // those are probably not relevant for our needs here.
    std::deque<Expr const*> contextuallyConvertedExprs_;
};

bool SimplifyPointerToBool::VisitImplicitCastExpr(ImplicitCastExpr const* castExpr)
{
    if (ignoreLocation(castExpr))
        return true;
    if (castExpr->getCastKind() != CK_PointerToBoolean)
        return true;
    auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(castExpr->getSubExpr()->IgnoreParens());
    if (!memberCallExpr)
        return true;
    auto methodDecl = memberCallExpr->getMethodDecl();
    if (!methodDecl || !methodDecl->getIdentifier() || methodDecl->getName() != "get")
        return true;
    //    castExpr->dump();
    //    methodDecl->getParent()->getTypeForDecl()->dump();
    if (!loplugin::isSmartPointerType(memberCallExpr->getImplicitObjectArgument()))
        return true;
    //    if (isa<CXXOperatorCallExpr>(callExpr))
    //        return true;
    //    const FunctionDecl* functionDecl;
    //    if (isa<CXXMemberCallExpr>(callExpr))
    //    {
    //        functionDecl = dyn_cast<CXXMemberCallExpr>(callExpr)->getMethodDecl();
    //    }
    //    else
    //    {
    //        functionDecl = callExpr->getDirectCallee();
    //    }
    //    if (!functionDecl)
    //        return true;
    //
    //    unsigned len = std::min(callExpr->getNumArgs(), functionDecl->getNumParams());
    //    for (unsigned i = 0; i < len; ++i)
    //    {
    //        auto param = functionDecl->getParamDecl(i);
    //        auto paramTC = loplugin::TypeCheck(param->getType());
    //        if (!paramTC.AnyBoolean())
    //            continue;
    //        auto arg = callExpr->getArg(i)->IgnoreImpCasts();
    //        auto argTC = loplugin::TypeCheck(arg->getType());
    //        if (argTC.AnyBoolean())
    //            continue;
    //        // sal_Bool is sometimes disguised
    //        if (isa<SubstTemplateTypeParmType>(arg->getType()))
    //            if (arg->getType()->getUnqualifiedDesugaredType()->isSpecificBuiltinType(
    //                    clang::BuiltinType::UChar))
    //                continue;
    //        if (arg->getType()->isDependentType())
    //            continue;
    //        if (arg->getType()->isIntegerType())
    //        {
    //            auto ret = getCallValue(arg);
    //            if (ret.hasValue() && (ret.getValue() == 1 || ret.getValue() == 0))
    //                continue;
    //            // something like: priv->m_nLOKFeatures & LOK_FEATURE_DOCUMENT_PASSWORD
    //            if (isa<BinaryOperator>(arg->IgnoreParenImpCasts()))
    //                continue;
    //            // something like: pbEmbolden ? FcTrue : FcFalse
    //            if (isa<ConditionalOperator>(arg->IgnoreParenImpCasts()))
    //                continue;
    //        }
    if (isContextuallyConverted(memberCallExpr))
    {
        if (rewriter)
        {
            auto const range = getCallSourceRange(memberCallExpr);
            if (range.isValid() && removeText(range))
            {
                return true;
            }
        }
        report(DiagnosticsEngine::Warning, "simplify, drop the get()", memberCallExpr->getExprLoc())
            << memberCallExpr->getSourceRange();
    }
    else if (isa<ParenExpr>(castExpr->getSubExpr()))
    {
        if (rewriter)
        {
            auto const loc
                = compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(memberCallExpr));
            auto const range = getCallSourceRange(memberCallExpr);
            if (loc.isValid() && range.isValid() && insertText(loc, "bool") && removeText(range))
            {
                //TODO: atomically only change both or neither
                return true;
            }
        }
        report(DiagnosticsEngine::Warning,
               "simplify, drop the get() and turn the surrounding parentheses into a functional "
               "cast to bool",
               memberCallExpr->getExprLoc())
            << memberCallExpr->getSourceRange();
        report(DiagnosticsEngine::Note, "surrounding parentheses here",
               castExpr->getSubExpr()->getExprLoc())
            << castExpr->getSubExpr()->getSourceRange();
    }
    else
    {
        if (rewriter)
        {
            auto const loc
                = compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(memberCallExpr));
            auto const range = getCallSourceRange(memberCallExpr);
            if (loc.isValid() && range.isValid() && insertText(loc, "bool(")
                && replaceText(range, ")"))
            {
                //TODO: atomically only change both or neither
                return true;
            }
        }
        report(DiagnosticsEngine::Warning,
               "simplify, drop the get() and wrap the expression in a functional cast to bool",
               memberCallExpr->getExprLoc())
            << memberCallExpr->getSourceRange();
    }
    //        report(DiagnosticsEngine::Note, "method here", param->getLocation())
    //            << param->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<SimplifyPointerToBool> simplifypointertobool("simplifypointertobool",
                                                                            true);

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
