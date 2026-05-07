/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

// Find assignments from regular C/C++ string literals to OUString vars, and warn to rather use an O[U]String literal,
// which avoids heap allocation.

#include <cassert>

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

namespace
{
class StringLiteralAssign final : public loplugin::FilteringRewritePlugin<StringLiteralAssign>
{
public:
    explicit StringLiteralAssign(loplugin::InstantiationData const& data)
        : FilteringRewritePlugin(data)
    {
    }

    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* expr)
    {
        if (ignoreLocation(expr))
            return true;
        if (expr->getOperator() != OO_Equal)
            return true;
        loplugin::TypeCheck const tc(expr->getType());
        if (!(tc.Class("OString").Namespace("rtl").GlobalNamespace()
              || tc.Class("OUString").Namespace("rtl").GlobalNamespace()))
        {
            return true;
        }
        if (expr->getNumArgs() != 2)
            return true;
        auto const sl = dyn_cast<clang::StringLiteral>(expr->getArg(1)->IgnoreParenImpCasts());
        if (!sl)
            return true;
        if (sl->getBeginLoc().isMacroID())
            return true;
        if (!rewriter)
        {
            report(DiagnosticsEngine::Warning,
                   "change type of constant character array (%0) to "
                   "%select{OStringLiteral|OUStringLiteral}1",
                   sl->getBeginLoc())
                << sl->getType() << (tc.Class("OString").Namespace("rtl").GlobalNamespace() ? 0 : 1)
                << sl->getSourceRange();
        }
        else
            reportedLiterals_.insert(sl);
        return true;
    }

private:
    void run() override
    {
        if (!preRun())
            return;
        if (!compiler.getLangOpts().CPlusPlus)
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        if (!rewriter)
            return;
        for (auto const& sl : reportedLiterals_)
        {
            SourceLocation l{ compiler.getSourceManager().getExpansionLoc(sl->getBeginLoc()) };
            SourceLocation end{ compiler.getSourceManager().getExpansionLoc(sl->getEndLoc()) };
            assert(l.isFileID() && end.isFileID());
            unsigned n
                = Lexer::MeasureTokenLength(l, compiler.getSourceManager(), compiler.getLangOpts());
            std::string str(compiler.getSourceManager().getCharacterData(l), n);
            str = "u" + str + "_ustr";
            replaceText(l, n, str);
        }
    }

    std::set<clang::StringLiteral const*> reportedLiterals_;
};

static loplugin::Plugin::Registration<StringLiteralAssign> reg("stringliteralassign");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
