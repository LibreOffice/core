/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

namespace
{
class UnusedCaptureDefault : public loplugin::FilteringPlugin<UnusedCaptureDefault>
{
public:
    explicit UnusedCaptureDefault(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

    bool VisitLambdaExpr(LambdaExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        if (expr->getCaptureDefault() == LCD_None)
        {
            return true;
        }
        if (expr->implicit_capture_begin() != expr->implicit_capture_end())
        {
            return true;
        }
        report(DiagnosticsEngine::Warning, "unused capture-default", expr->getCaptureDefaultLoc())
            << expr->getSourceRange();
        return true;
    }

private:
    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }
};

loplugin::Plugin::Registration<UnusedCaptureDefault> unusedcapturedefault("unusedcapturedefault");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
