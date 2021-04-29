/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"

namespace
{
class Noexcept : public loplugin::FilteringRewritePlugin<Noexcept>
{
public:
    explicit Noexcept(loplugin::InstantiationData const& data)
        : FilteringRewritePlugin(data)
    {
    }

    void run() override
    {
        // Don't execute for < C++11, so we don't accidentally rewrite the legacy definitions of
        // SAL_THROW_EXTERN_C and SAL_NOEXCEPT in include/sal/types.h e.g. when building
        // CppunitTest_odk_checkapi which explicitly uses gb_CXX03FLAGS:
        if (compiler.getLangOpts().CPlusPlus11)
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitFunctionProtoTypeLoc(FunctionProtoTypeLoc tloc)
    {
        if (ignoreLocation(tloc))
        {
            return true;
        }
        if (tloc.getTypePtr()->getExceptionSpecType() != EST_DynamicNone)
        {
            return true;
        }
        auto const r = tloc.getExceptionSpecRange();
        auto const repl = isInUnoIncludeFile(r.getBegin()) ? "SAL_NOEXCEPT" : "noexcept";
        if (rewriter != nullptr)
        {
            auto r2 = r;
            auto l1 = r.getBegin();
            while (compiler.getSourceManager().isMacroArgExpansion(l1))
            {
                l1 = compiler.getSourceManager().getImmediateMacroCallerLoc(l1);
            }
            if (compiler.getSourceManager().isMacroBodyExpansion(l1))
            {
                auto l2 = r.getEnd();
                while (compiler.getSourceManager().isMacroArgExpansion(l2))
                {
                    l2 = compiler.getSourceManager().getImmediateMacroCallerLoc(l2);
                }
                if (compiler.getSourceManager().isMacroBodyExpansion(l2))
                {
                    //TODO: check l1, l2 are in same macro body expansion
                    r2 = { compiler.getSourceManager().getSpellingLoc(l1),
                           compiler.getSourceManager().getSpellingLoc(l2) };
                }
            }
            if (replaceText(r2, repl))
            {
                return true;
            }
        }
        report(DiagnosticsEngine::Warning,
               "Replace legacy dynamic 'throw ()' exception specification with '%0'", r.getBegin())
            << repl << r;
        return true;
    }
};

loplugin::Plugin::Registration<Noexcept> X("noexcept", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
