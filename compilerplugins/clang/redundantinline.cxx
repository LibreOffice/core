/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>

#include "plugin.hxx"

namespace {

class RedundantInline:
    public loplugin::FilteringRewritePlugin<RedundantInline>
{
public:
    explicit RedundantInline(loplugin::InstantiationData const & data):
        FilteringRewritePlugin(data) {}

    void run() override {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitFunctionDecl(FunctionDecl const * decl) {
        if (ignoreLocation(decl)) {
            return true;
        }
        if (!decl->isInlineSpecified()) {
            return true;
        }
        handleImplicitInline(decl) || handleNonExternalLinkage(decl);
        return true;
    }

private:
    bool removeInline(FunctionDecl const * decl, SourceLocation * inlineLoc) {
        assert(inlineLoc != nullptr);
        assert(inlineLoc->isInvalid());
        unsigned n = {}; // avoid -Werror=maybe-uninitialized
        auto end = Lexer::getLocForEndOfToken(
            compiler.getSourceManager().getExpansionLoc(compat::getEndLoc(decl)), 0,
            compiler.getSourceManager(), compiler.getLangOpts());
        assert(end.isValid());
        for (auto loc = compiler.getSourceManager().getExpansionLoc(
                 compat::getBeginLoc(decl));
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
                    *inlineLoc = loc;
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
        if (rewriter != nullptr && inlineLoc->isValid()) {
            for (auto loc = inlineLoc->getLocWithOffset(
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
            if (removeText(*inlineLoc, n, RewriteOptions(RemoveLineIfEmpty))) {
                return true;
            }
        }
        return false;
    }

    SourceLocation unwindTo(SourceLocation const & loc, StringRef name) {
        if (!loc.isMacroID()) {
            return SourceLocation();
        }
        auto l = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
        return
            (Lexer::getImmediateMacroName(
                loc, compiler.getSourceManager(), compiler.getLangOpts())
             == name)
            ? l : unwindTo(l, name);
    }

    bool isInMacroExpansion(FunctionDecl const * decl, StringRef name) {
        auto loc = unwindTo(compat::getBeginLoc(decl), name);
        return loc.isValid() && loc == unwindTo(compat::getEndLoc(decl), name);
    }

    bool handleImplicitInline(FunctionDecl const * decl) {
        if (!(decl->doesThisDeclarationHaveABody() || decl->isExplicitlyDefaulted())
            || !(decl->getLexicalDeclContext()->isRecord() || decl->isConstexpr()))
        {
            return false;
        }
        if (isInMacroExpansion(decl, "Q_OBJECT")) {
            return true;
        }
        SourceLocation inlineLoc;
        if (!removeInline(decl, &inlineLoc)) {
            report(
                DiagnosticsEngine::Warning,
                "function definition redundantly declared 'inline'",
                inlineLoc.isValid() ? inlineLoc : compat::getBeginLoc(decl))
                << decl->getSourceRange();
        }
        return true;
    }

    bool handleNonExternalLinkage(FunctionDecl const * decl) {
        if (decl->getLinkageInternal() >= ModuleLinkage) {
            return false;
        }
        if (!compiler.getSourceManager().isInMainFile(decl->getLocation())) {
            // There *may* be benefit to "static inline" in include files (esp. in C code, where an
            // inline function with external linkage still requires an external definition), so
            // just ignore those for now:
            return true;
        }
        if (isInMacroExpansion(decl, "G_DEFINE_TYPE")
            || isInMacroExpansion(decl, "G_DEFINE_TYPE_WITH_CODE")
            || isInMacroExpansion(decl, "G_DEFINE_TYPE_WITH_PRIVATE"))
        {
            return true;
        }
        SourceLocation inlineLoc;
        if (!removeInline(decl, &inlineLoc)) {
            report(
                DiagnosticsEngine::Warning,
                "function has no external linkage but is explicitly declared 'inline'",
                inlineLoc.isValid() ? inlineLoc : compat::getBeginLoc(decl))
                << decl->getSourceRange();
        }
        return true;
    }
};

loplugin::Plugin::Registration<RedundantInline> redundantinline("redundantinline");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
