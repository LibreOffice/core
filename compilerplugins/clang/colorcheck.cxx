/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include <memory>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>

#include "check.hxx"
#include "plugin.hxx"

/**
*/

namespace
{
class ColorCheck : public loplugin::FilteringPlugin<ColorCheck>
{
public:
    explicit ColorCheck(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXConstructExpr(const CXXConstructExpr*);
};

bool ColorCheck::VisitCXXConstructExpr(const CXXConstructExpr* constructExpr)
{
    if (ignoreLocation(constructExpr))
        return true;

    if (constructExpr->getNumArgs() != 1)
        return true;

    auto tc = loplugin::TypeCheck(constructExpr->getType());
    if (!tc.Class("Color").GlobalNamespace())
        return true;

    StringRef aFileName = getFilenameOfLocation(
        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(constructExpr)));
    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/tools/color.hxx"))
        return true;

    const CXXConstructorDecl* constructorDecl = constructExpr->getConstructor();
    constructorDecl = constructorDecl->getCanonicalDecl();

    if (!loplugin::TypeCheck(constructorDecl->getParamDecl(0)->getType())
             .Typedef("sal_uInt32")
             .GlobalNamespace())
        return true;

    auto arg0 = constructExpr->getArg(0);
    if (arg0->isCXX11ConstantExpr(compiler.getASTContext()))
    {
        if (!arg0->isValueDependent())
        {
            llvm::Optional<llvm::APSInt> xVal
                = compat::getIntegerConstantExpr(arg0, compiler.getASTContext());
            if (xVal && *xVal > 0xffffff)
                report(DiagnosticsEngine::Warning,
                       "Rather use the ColorTransparency or ColorAlpha version of this constructor",
                       arg0->getExprLoc());
            return true;
        }
    }
    report(DiagnosticsEngine::Warning,
           "Rather use the ColorTransparency or ColorAlpha version of this constructor",
           arg0->getExprLoc());

    return true;
}

loplugin::Plugin::Registration<ColorCheck> colorcheck("colorcheck");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
