/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>

#include "plugin.hxx"

namespace {

class RedundantInline:
    public RecursiveASTVisitor<RedundantInline>, public loplugin::RewritePlugin
{
public:
    explicit RedundantInline(loplugin::InstantiationData const & data):
        RewritePlugin(data) {}

    void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitFunctionDecl(FunctionDecl const * decl) {
        if (ignoreLocation(decl) || !decl->isInlineSpecified()
            || !(decl->doesThisDeclarationHaveABody()
                 || decl->isExplicitlyDefaulted())
            || !(decl->getLexicalDeclContext()->isRecord()
                 || decl->isConstexpr()))
        {
            return true;
        }
        auto l1 = unwindToQObject(decl->getLocStart());
        if (l1.isValid() && l1 == unwindToQObject(decl->getLocEnd())) {
            return true;
        }
        SourceLocation inlineLoc;
        unsigned n;
        auto end = Lexer::getLocForEndOfToken(
            compiler.getSourceManager().getExpansionLoc(decl->getLocEnd()), 0,
            compiler.getSourceManager(), compiler.getLangOpts());
        assert(end.isValid());
        for (auto loc = compiler.getSourceManager().getExpansionLoc(
                 decl->getLocStart());
             loc != end; loc = loc.getLocWithOffset(std::max<unsigned>(n, 1)))
        {
            n = Lexer::MeasureTokenLength(
                loc, compiler.getSourceManager(), compiler.getLangOpts());
            StringRef s(compiler.getSourceManager().getCharacterData(loc), n);
            //TODO: see compilerplugins/clang/override.cxx:
            if (s.startswith("\\\n")) {
                s = s.drop_front(2);
            }
            if (s == "inline") {
                if (!compiler.getSourceManager().isMacroArgExpansion(loc)) {
                    inlineLoc = loc;
                }
                break;
            } else if (s == "#") {
                // Hard to pick the right 'inline' in code like
                //
                //   #if 1
                //     static
                //   #else
                //     inline
                //   #endif
                //   inline void f() {}
                //
                // so just give up once a preprocessing directive is seen:
                break;
            }
        }
        if (rewriter != nullptr && inlineLoc.isValid()) {
            for (auto loc = inlineLoc.getLocWithOffset(
                     std::max<unsigned>(n, 1));;)
            {
                assert(loc != end);
                unsigned n2 = Lexer::MeasureTokenLength(
                    loc, compiler.getSourceManager(), compiler.getLangOpts());
                StringRef s(
                    compiler.getSourceManager().getCharacterData(loc), n2);
                //TODO: see compilerplugins/clang/override.cxx:
                if (s.startswith("\\\n")) {
                    s = s.drop_front(2);
                }
                if (!s.empty()) {
                    break;
                }
                n2 = std::max<unsigned>(n2, 1);
                n += n2;
                loc = loc.getLocWithOffset(n2);
            }
            if (removeText(inlineLoc, n, RewriteOptions(RemoveLineIfEmpty))) {
                return true;
            }
        }
        report(
            DiagnosticsEngine::Warning,
            "function definition redundantly declared 'inline'",
            inlineLoc.isValid() ? inlineLoc : decl->getLocStart())
            << decl->getSourceRange();
        return true;
    }

private:
    SourceLocation unwindToQObject(SourceLocation const & loc) {
        if (!loc.isMacroID()) {
            return SourceLocation();
        }
        auto l = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
        return
            (Lexer::getImmediateMacroName(
                loc, compiler.getSourceManager(), compiler.getLangOpts())
             == "Q_OBJECT")
            ? l : unwindToQObject(l);
    }
};

loplugin::Plugin::Registration<RedundantInline> reg("redundantinline", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
