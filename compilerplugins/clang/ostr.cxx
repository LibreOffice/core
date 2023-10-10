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
        if (compiler.getLangOpts().CPlusPlus)
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
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

    bool VisitCXXConstructExpr(CXXConstructExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        if (!loplugin::DeclCheck(expr->getConstructor()->getParent())
                 .Class("OUString")
                 .Namespace("rtl")
                 .GlobalNamespace())
        {
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
        if (!compat::isOrdinary(e2))
        {
            assert(!e2->isUTF8()); //TODO
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
        if (!compiler.getDiagnosticOpts().VerifyDiagnostics)
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
            if (replaceText(l1, delta(l1, l2), macroBegin ? "u\"\" " : "u")
                && replaceText(l3, delta(l3, l4), macroEnd ? " \"\"_ustr" : "_ustr"))
            {
                return true;
            }
        }
        report(DiagnosticsEngine::Warning,
               "use a _ustr user-defined string literal instead of constructing an instance of %0 "
               "from an ordinary string literal",
               expr->getExprLoc())
            << expr->getType().getLocalUnqualifiedType() << expr->getSourceRange();
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

    std::set<Expr const*> defaultArgs_;
    std::stack<CXXFunctionalCastExpr const*> functionalCasts_;
    std::set<SourceLocation> locs_;
};

loplugin::Plugin::Registration<Ostr> X("ostr", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
