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
#include <stack>

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

// Rewrite some uses of O[U]String to use ""_ostr/u""_ustr literals.

namespace
{
class Ostr : public loplugin::FilteringRewritePlugin<Ostr>
{
public:
    explicit Ostr(loplugin::InstantiationData const& data)
        : FilteringRewritePlugin(data)
    {
    }

    // Needed so that e.g.
    //
    //   struct S { OUString s; };
    //   S s = {u"foo"};
    //
    // is caught:
    bool shouldVisitImplicitCode() const { return true; }

    void run() override
    {
        if (compiler.getLangOpts().CPlusPlus
            && TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()))
        {
            for (auto const& i : vars_)
            {
                auto const utf16
                    = bool(loplugin::TypeCheck(i.first->getType()).Class("OUStringLiteral"));
                if (i.second.singleUse == nullptr)
                {
                    if (rewriter != nullptr)
                    {
                        auto e = i.first->getInit()->IgnoreParenImpCasts();
                        if (auto const e2 = dyn_cast<ConstantExpr>(e))
                        {
                            e = e2->getSubExpr()->IgnoreParenImpCasts();
                        }
                        if (auto const e2 = dyn_cast<CXXConstructExpr>(e))
                        {
                            assert(e2->getNumArgs() == 1);
                            e = e2->getArg(0)->IgnoreParenImpCasts();
                        }
                        e = dyn_cast<clang::StringLiteral>(e);
                        // e is null when this OUStringLiteral is initialized with another
                        // OUStringLiteral:
                        if (e == nullptr
                            || insertTextAfterToken(e->getEndLoc(), utf16 ? "_ustr" : "_ostr"))
                        {
                            auto ok = true;
                            for (auto d = i.first->getMostRecentDecl(); d != nullptr;
                                 d = d->getPreviousDecl())
                            {
                                auto const l1 = d->getTypeSpecStartLoc();
                                auto l2 = d->getTypeSpecEndLoc();
                                l2 = l2.getLocWithOffset(Lexer::MeasureTokenLength(
                                    l2, compiler.getSourceManager(), compiler.getLangOpts()));
                                if (!replaceText(l1, delta(l1, l2), utf16 ? "OUString" : "OString"))
                                {
                                    ok = false;
                                }
                            }
                            for (auto const i : i.second.explicitConversions)
                            {
                                auto const e2 = i->getArg(0);
                                auto l1 = i->getBeginLoc();
                                auto l2 = e2->getBeginLoc();
                                auto l3 = e2->getEndLoc();
                                auto l4 = i->getEndLoc();
                                while (compiler.getSourceManager().isMacroArgExpansion(l1)
                                       && compiler.getSourceManager().isMacroArgExpansion(l2)
                                       && compiler.getSourceManager().isMacroArgExpansion(l3)
                                       && compiler.getSourceManager().isMacroArgExpansion(l4))
                                //TODO: check all four locations are part of the same macro argument
                                // expansion
                                {
                                    l1 = compiler.getSourceManager().getImmediateMacroCallerLoc(l1);
                                    l2 = compiler.getSourceManager().getImmediateMacroCallerLoc(l2);
                                    l3 = compiler.getSourceManager().getImmediateMacroCallerLoc(l3);
                                    l4 = compiler.getSourceManager().getImmediateMacroCallerLoc(l4);
                                }
                                l3 = l3.getLocWithOffset(Lexer::MeasureTokenLength(
                                    l3, compiler.getSourceManager(), compiler.getLangOpts()));
                                l4 = l4.getLocWithOffset(Lexer::MeasureTokenLength(
                                    l4, compiler.getSourceManager(), compiler.getLangOpts()));
                                removeText(l1, delta(l1, l2));
                                removeText(l3, delta(l3, l4));
                            }
                            if (ok)
                            {
                                continue;
                            }
                        }
                    }
                    report(DiagnosticsEngine::Warning,
                           "use '%select{OString|OUString}0', created from a %select{_ostr|_ustr}0 "
                           "user-defined string literal, instead of "
                           "'%select{OStringLiteral|OUStringLiteral}0' for the variable %1",
                           i.first->getLocation())
                        << utf16 << i.first->getName() << i.first->getSourceRange();
                    for (auto d = i.first->getMostRecentDecl(); d != nullptr;
                         d = d->getPreviousDecl())
                    {
                        if (d != i.first)
                        {
                            report(DiagnosticsEngine::Note, "variable %0 declared here",
                                   d->getLocation())
                                << d->getName() << d->getSourceRange();
                        }
                    }
                }
                else
                {
                    if (!compiler.getDiagnosticOpts().VerifyDiagnostics)
                    {
                        //TODO, left for later:
                        continue;
                    }
                    report(DiagnosticsEngine::Warning,
                           "directly use a %select{_ostr|_ustr}0 user-defined string literal "
                           "instead of introducing the intermediary "
                           "'%select{OStringLiteral|OUStringLiteral}0' variable %1",
                           i.second.singleUse->getExprLoc())
                        << utf16 << i.first->getName() << i.second.singleUse->getSourceRange();
                    for (auto d = i.first->getMostRecentDecl(); d != nullptr;
                         d = d->getPreviousDecl())
                    {
                        report(DiagnosticsEngine::Note, "intermediary variable %0 declared here",
                               d->getLocation())
                            << d->getName() << d->getSourceRange();
                    }
                }
            }
        }
    }

    bool TraverseParmVarDecl(ParmVarDecl* decl)
    {
        // Otherwise,
        //
        //   struct S { void f(int = 0); };
        //   void S::f(int) {}
        //
        // would visit the default argument twice:
        if (decl->hasDefaultArg() && !decl->hasUninstantiatedDefaultArg()
            && !decl->hasUnparsedDefaultArg() && !defaultArgs_.insert(decl->getDefaultArg()).second)
        {
            return true;
        }
        return RecursiveASTVisitor::TraverseParmVarDecl(decl);
    }

    bool TraverseCXXFunctionalCastExpr(CXXFunctionalCastExpr* expr)
    {
        functionalCasts_.push(expr);
        auto const ret = RecursiveASTVisitor::TraverseCXXFunctionalCastExpr(expr);
        functionalCasts_.pop();
        return ret;
    }

    bool VisitVarDecl(VarDecl const* decl)
    {
        if (ignoreLocation(decl))
        {
            return true;
        }
        if (!decl->isThisDeclarationADefinition())
        {
            return true;
        }
        loplugin::TypeCheck const tc(decl->getType());
        if (!(tc.Class("OStringLiteral").Namespace("rtl").GlobalNamespace()
              || tc.Class("OUStringLiteral").Namespace("rtl").GlobalNamespace()))
        {
            return true;
        }
        if (suppressWarningAt(decl->getLocation()))
        {
            return true;
        }
        vars_[decl].multipleUses
            = decl->getDeclContext()->isFileContext()
                  ? !compiler.getSourceManager().isInMainFile(decl->getLocation())
                  : decl->isExternallyVisible();
        return true;
    }

    bool VisitDeclRefExpr(DeclRefExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const d1 = dyn_cast<VarDecl>(expr->getDecl());
        if (d1 == nullptr)
        {
            return true;
        }
        auto const d2 = d1->getDefinition();
        if (d2 == nullptr)
        {
            return true;
        }
        auto const i = vars_.find(d2);
        if (i == vars_.end())
        {
            return true;
        }
        if (!i->second.multipleUses)
        {
            if (i->second.singleUse == nullptr)
            {
                i->second.singleUse = expr;
            }
            else
            {
                i->second.multipleUses = true;
                i->second.singleUse = nullptr;
            }
        }
        return true;
    }

    bool VisitCXXConstructExpr(CXXConstructExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const dc = expr->getConstructor()->getParent();
        auto const utf16
            = bool(loplugin::DeclCheck(dc).Class("OUString").Namespace("rtl").GlobalNamespace());
        if (!(utf16 || loplugin::DeclCheck(dc).Class("OString").Namespace("rtl").GlobalNamespace()))
        {
            return true;
        }
        if (expr->getNumArgs() == 1
            && loplugin::TypeCheck(expr->getArg(0)->getType())
                   .Class(utf16 ? "OUStringLiteral" : "OStringLiteral")
                   .Namespace("rtl")
                   .GlobalNamespace())
        {
            if (functionalCasts_.empty()
                || functionalCasts_.top()->getSubExpr()->IgnoreImplicit() != expr)
            {
                return true;
            }
            auto const e = dyn_cast<DeclRefExpr>(expr->getArg(0)->IgnoreParenImpCasts());
            if (e == nullptr)
            {
                return true;
            }
            auto const d1 = dyn_cast<VarDecl>(e->getDecl());
            if (d1 == nullptr)
            {
                return true;
            }
            auto const d2 = d1->getDefinition();
            if (d2 == nullptr)
            {
                return true;
            }
            auto const i = vars_.find(d2);
            if (i == vars_.end())
            {
                return true;
            }
            i->second.explicitConversions.insert(expr);
            return true;
        }
        if (expr->getNumArgs() != 2)
        {
            return true;
        }
        if (!loplugin::TypeCheck(expr->getArg(1)->getType())
                 .Struct("Dummy")
                 .Namespace("libreoffice_internal")
                 .Namespace("rtl")
                 .GlobalNamespace())
        {
            return true;
        }
        auto const e2 = dyn_cast<clang::StringLiteral>(expr->getArg(0)->IgnoreParenImpCasts());
        if (e2 == nullptr)
        {
            return true;
        }
        if (!(compat::isOrdinary(e2) || e2->isUTF8()))
        {
            return true;
        }
        auto const temp = isa<CXXTemporaryObjectExpr>(expr)
                          || (!functionalCasts_.empty()
                              && functionalCasts_.top()->getSubExpr()->IgnoreImplicit() == expr);
        auto const e1 = temp ? static_cast<Expr const*>(expr) : static_cast<Expr const*>(e2);
        auto l1 = e1->getBeginLoc();
        auto l2 = e2->getBeginLoc();
        auto l3 = e2->getEndLoc();
        auto l4 = e1->getEndLoc();
        while (compiler.getSourceManager().isMacroArgExpansion(l1)
               && compiler.getSourceManager().isMacroArgExpansion(l2)
               && compiler.getSourceManager().isMacroArgExpansion(l3)
               && compiler.getSourceManager().isMacroArgExpansion(l4))
        //TODO: check all four locations are part of the same macro argument expansion
        {
            l1 = compiler.getSourceManager().getImmediateMacroCallerLoc(l1);
            l2 = compiler.getSourceManager().getImmediateMacroCallerLoc(l2);
            l3 = compiler.getSourceManager().getImmediateMacroCallerLoc(l3);
            l4 = compiler.getSourceManager().getImmediateMacroCallerLoc(l4);
        }
        if (!locs_.insert(l1).second)
        {
            return true;
        }
        auto const macroBegin = l2.isMacroID()
                                && Lexer::isAtStartOfMacroExpansion(l2, compiler.getSourceManager(),
                                                                    compiler.getLangOpts());
        if (macroBegin)
        {
            l2 = compiler.getSourceManager().getImmediateMacroCallerLoc(l2);
        }
        auto const macroEnd = l3.isMacroID()
                              && Lexer::isAtEndOfMacroExpansion(l3, compiler.getSourceManager(),
                                                                compiler.getLangOpts());
        if (macroEnd)
        {
            l3 = compiler.getSourceManager().getImmediateMacroCallerLoc(l3);
        }
        if (!temp)
        {
            l1 = l2;
            l4 = l3;
        }
        if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(l1)))
        {
            return true;
        }
        if (!compiler.getDiagnosticOpts().VerifyDiagnostics && utf16)
        {
            //TODO: Leave rewriting these uses of ordinary string literals for later (but already
            // cover them when verifying CompilerTest_compilerplugins_clang):
            return true;
        }
        if (rewriter != nullptr && isSpellingRange(l1, l2) && isSpellingRange(l3, l4))
        {
            l3 = l3.getLocWithOffset(
                Lexer::MeasureTokenLength(l3, compiler.getSourceManager(), compiler.getLangOpts()));
            l4 = l4.getLocWithOffset(
                Lexer::MeasureTokenLength(l4, compiler.getSourceManager(), compiler.getLangOpts()));
            if (replaceText(l1, delta(l1, l2), utf16 ? (macroBegin ? "u\"\" " : "u") : "")
                && replaceText(l3, delta(l3, l4),
                               utf16 ? (macroEnd ? " \"\"_ustr" : "_ustr")
                                     : (macroEnd ? " \"\"_ostr" : "_ostr")))
            {
                return true;
            }
        }
        report(DiagnosticsEngine::Warning,
               "use a %select{_ostr|_ustr}0 user-defined string literal instead of constructing an"
               " instance of %1 from an ordinary string literal",
               expr->getExprLoc())
            << utf16 << expr->getType().getLocalUnqualifiedType() << expr->getSourceRange();
        return true;
    }

    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        if (expr->getOperator() != OO_Equal)
        {
            return true;
        }
        if (!loplugin::TypeCheck(expr->getArg(0)->getType())
                 .Class("OString")
                 .Namespace("rtl")
                 .GlobalNamespace())
        {
            return true;
        }
        auto const e2 = dyn_cast<clang::StringLiteral>(expr->getArg(1)->IgnoreParenImpCasts());
        if (e2 == nullptr)
        {
            return true;
        }
        if (rewriter != nullptr)
        {
            auto loc = e2->getEndLoc();
            auto const macroEnd = loc.isMacroID()
                                  && Lexer::isAtEndOfMacroExpansion(
                                         loc, compiler.getSourceManager(), compiler.getLangOpts());
            if (macroEnd)
            {
                loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
            }
            if (insertTextAfterToken(loc, macroEnd ? " \"\"_ostr" : "_ostr"))
            {
                return true;
            }
        }
        report(DiagnosticsEngine::Warning,
               "use a _ostr user-defined string literal instead of assigning from an ordinary"
               " string literal",
               expr->getExprLoc())
            << expr->getSourceRange();
        return true;
    }

    bool VisitCXXMemberCallExpr(CXXMemberCallExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        if (!loplugin::DeclCheck(expr->getMethodDecl()).Operator(OO_Equal))
        {
            return true;
        }
        if (!loplugin::TypeCheck(expr->getObjectType())
                 .Class("OString")
                 .Namespace("rtl")
                 .GlobalNamespace())
        {
            return true;
        }
        auto const e2 = dyn_cast<clang::StringLiteral>(expr->getArg(0)->IgnoreParenImpCasts());
        if (e2 == nullptr)
        {
            return true;
        }
        if (rewriter != nullptr)
        {
            //TODO
        }
        report(DiagnosticsEngine::Warning,
               "use a _ostr user-defined string literal instead of assigning from an ordinary"
               " string literal",
               expr->getExprLoc())
            << expr->getSourceRange();
        return true;
    }

    bool VisitCastExpr(CastExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const t1 = expr->getType().getNonReferenceType();
        auto const tc1 = loplugin::TypeCheck(t1);
        if (!(tc1.ClassOrStruct("basic_string").StdNamespace()
              || tc1.ClassOrStruct("basic_string_view").StdNamespace()))
        {
            return true;
        }
        auto const e2 = dyn_cast<UserDefinedLiteral>(expr->getSubExprAsWritten());
        if (e2 == nullptr)
        {
            return true;
        }
        auto const tc2 = loplugin::TypeCheck(e2->getType());
        if (!(tc2.Class("OString").Namespace("rtl").GlobalNamespace()
              || tc2.Class("OUString").Namespace("rtl").GlobalNamespace()))
        {
            return true;
        }
        report(DiagnosticsEngine::Warning,
               "directly use a %0 value instead of a %select{_ostr|_ustr}1 user-defined string"
               " literal",
               expr->getExprLoc())
            << t1.getUnqualifiedType() << bool(tc2.Class("OUString")) << expr->getSourceRange();
        return true;
    }

private:
    bool isSpellingRange(SourceLocation loc1, SourceLocation loc2)
    {
        if (!SourceLocation::isPairOfFileLocations(loc1, loc2))
        {
            return false;
        }
        if (compiler.getSourceManager().getFileID(loc1)
            != compiler.getSourceManager().getFileID(loc2))
        {
            return false;
        }
        return loc1 <= loc2;
    }

    unsigned delta(SourceLocation loc1, SourceLocation loc2)
    {
        return compiler.getSourceManager().getDecomposedLoc(loc2).second
               - compiler.getSourceManager().getDecomposedLoc(loc1).second;
    }

    struct Var
    {
        bool multipleUses = false;
        DeclRefExpr const* singleUse = nullptr;
        std::set<CXXConstructExpr const*> explicitConversions;
    };

    std::set<Expr const*> defaultArgs_;
    std::stack<CXXFunctionalCastExpr const*> functionalCasts_;
    std::set<SourceLocation> locs_;
    std::map<VarDecl const*, Var> vars_;
};

loplugin::Plugin::Registration<Ostr> X("ostr", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
