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
#include <limits>
#include <set>

#include "clang/AST/Attr.h"

#include "compat.hxx"
#include "plugin.hxx"

namespace {

class SalOverride:
    public RecursiveASTVisitor<SalOverride>, public loplugin::RewritePlugin
{
public:
    explicit SalOverride(InstantiationData const & data): RewritePlugin(data) {}

    virtual void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXMethodDecl(CXXMethodDecl const * decl);

private:
    std::set<SourceLocation> insertions_;
};

bool SalOverride::VisitCXXMethodDecl(CXXMethodDecl const * decl) {
    // As a heuristic, ignore declarations where the name is spelled out in an
    // ignored location; that e.g. handles uses of the Q_OBJECT macro from
    // external QtCore/qobjectdefs.h:
    if (ignoreLocation(decl) || !compat::isFirstDecl(*decl)
        || decl->begin_overridden_methods() == decl->end_overridden_methods()
        || decl->hasAttr<OverrideAttr>() || isa<CXXDestructorDecl>(decl)
        || ignoreLocation(
            compiler.getSourceManager().getSpellingLoc(
                decl->getNameInfo().getLoc())))
    {
        return true;
    }
#if LO_COMPILERPLUGINS_CLANG_COMPAT_HAVE_isAtEndOfImmediateMacroExpansion
    if (rewriter != nullptr) {
        // In  void MACRO(...);  getSourceRange().getEnd() would (erroneously?)
        // point at "MACRO" rather than ")", so make the loop always terminate
        // at the first ";" or "{" instead of getSourceRange().getEnd():
        unsigned parens = 0;
        bool seenSpace = false;
        //TODO: Whether to add a space after the inserted "SAL_OVERRIDE" should
        // depend on the following token at the spelling location where
        // "SAL_OVERRIDE" is inserted, not on the following token in the fully-
        // macro-expanded view:
        bool addSpace;
        SourceLocation loc;
        for (SourceLocation l(decl->getSourceRange().getBegin());;) {
            SourceLocation sl(compiler.getSourceManager().getSpellingLoc(l));
            unsigned n = Lexer::MeasureTokenLength(
                sl, compiler.getSourceManager(), compiler.getLangOpts());
            StringRef s(compiler.getSourceManager().getCharacterData(sl), n);
            //TODO: Looks like a Clang bug that in some cases like
            // (filter/source/svg/svgexport.cxx)
            //
            // #define TEXT_FIELD_GET_CLASS_NAME_METHOD( class_name ) \
            // virtual OUString getClassName() const                  \
            // {                                                      \
            //     static const char className[] = #class_name;       \
            //     return OUString( className );                      \
            // }
            //
            // TEXT_FIELD_GET_CLASS_NAME_METHOD( TextField )
            //
            // where "\<NL>" is followed directly by a real token without
            // intervening whitespace, tokens "\<NL>virtual" and "\<NL>{" are
            // reported:
            if (s.startswith("\\\n")) {
                s = s.drop_front(2);
            }
            if (parens == 0) {
                if (s == "=" || s == "{") {
                    if (!seenSpace) {
                        addSpace = true;
                    }
                    break;
                }
                if (s == ";") {
                    break;
                }
            }
            if (s == "(") {
                assert(parens < std::numeric_limits<unsigned>::max());
                ++parens;
            } else if (s == ")") {
                assert(parens != 0);
                --parens;
            }
            if (s.empty()) {
                if (!seenSpace) {
                    addSpace = false;
                }
                seenSpace = true;
            } else if (s.startswith("/*") || s.startswith("//") || s == "\\") {
                if (!seenSpace) {
                    addSpace = true;
                }
                seenSpace = true;
            } else {
                seenSpace = false;
                addSpace = false;
                loc = sl;
            }
            if (l.isMacroID()
                && compiler.getSourceManager().isAtEndOfImmediateMacroExpansion(
                    l, &l))
            {
                n = Lexer::MeasureTokenLength(
                    l, compiler.getSourceManager(), compiler.getLangOpts());
            }
            l = l.getLocWithOffset(std::max<unsigned>(n, 1));
        }
        assert(loc.isValid());
        if (!insertions_.insert(loc).second
            || insertTextAfterToken(
                loc, addSpace ? " SAL_OVERRIDE " : " SAL_OVERRIDE"))
        {
            return true;
        }
    }
#endif
    report(
        DiagnosticsEngine::Warning,
        ("overriding virtual function declaration not marked 'override' (aka"
         " 'SAL_OVERRIDE')"),
        decl->getLocation())
        << decl->getSourceRange();
    for (auto i = decl->begin_overridden_methods();
         i != decl->end_overridden_methods(); ++i)
    {
        report(
            DiagnosticsEngine::Note, "overridden declaration is here",
            (*i)->getLocation())
            << (*i)->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<SalOverride> X("saloverride", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
