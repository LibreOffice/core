/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"

namespace {

class BadStatics
    : public clang::RecursiveASTVisitor<BadStatics>
    , public loplugin::Plugin
{

public:
    explicit BadStatics(InstantiationData const& rData) : Plugin(rData) {}

    void run() override {
        if (compiler.getLangOpts().CPlusPlus) { // no non-trivial dtors in C
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitVarDecl(VarDecl const*const pVarDecl)
    {
        if (ignoreLocation(pVarDecl)) {
            return true;
        }

        if (pVarDecl->hasGlobalStorage()) {
            auto const type(pVarDecl->getType().getUnqualifiedType().getCanonicalType().getAsString());
            if (   type == "class Image"
                || type == "class Bitmap"
                || type == "class BitmapEx"
               )
            {
                report(DiagnosticsEngine::Warning,
                        "bad static variable causes crash on shutdown",
                        pVarDecl->getLocation())
                    << pVarDecl->getSourceRange();
            }
        }

        return true;
    }

};

loplugin::Plugin::Registration<BadStatics> X("badstatics");

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
