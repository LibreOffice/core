/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>
#include <string>

#include "clang/AST/Attr.h"
#include "clang/Sema/SemaInternal.h" // warn_unused_function

#include "plugin.hxx"

namespace {

class InlineVisible:
    public loplugin::FilteringPlugin<InlineVisible>
{
public:
    explicit InlineVisible(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFunctionDecl(FunctionDecl const * decl);
};

bool InlineVisible::VisitFunctionDecl(FunctionDecl const * decl) {
    if (!ignoreLocation(decl) && decl->isInlineSpecified()) {
        VisibilityAttr * attr = decl->getAttr<VisibilityAttr>();
        if (attr != nullptr && attr->getVisibility() == VisibilityAttr::Default)
        {
            report(
                DiagnosticsEngine::Warning,
                ("Function explicitly declared both inline and "
                 " visibility=default"),
                decl->getLocation())
                << decl->getSourceRange();
        }
    }
    return true;
}

loplugin::Plugin::Registration<InlineVisible> inlinevisible("inlinevisible");

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
