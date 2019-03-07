/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include "plugin.hxx"

namespace {

class PrivateBase:
    public loplugin::FilteringPlugin<PrivateBase>
{
public:
    explicit PrivateBase(loplugin::InstantiationData const & data): FilteringPlugin(data)
    {}

    void run() override;

    bool VisitCXXRecordDecl(CXXRecordDecl const * decl);
};

void PrivateBase::run() {
    if (compiler.getLangOpts().CPlusPlus) {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }
}

bool PrivateBase::VisitCXXRecordDecl(CXXRecordDecl const * decl) {
    if (ignoreLocation(decl) || !decl->isThisDeclarationADefinition()
        || decl->getTagKind() != TTK_Class)
    {
        return true;
    }
    for (auto i = decl->bases_begin(); i != decl->bases_end(); ++i) {
        if (i->getAccessSpecifierAsWritten() == AS_none) {
            report(
                DiagnosticsEngine::Warning,
                "base class is private by default; explicitly give an access"
                    " specifier",
                compat::getBeginLoc(i))
                << i->getSourceRange();
        }
    }
    return true;
}

loplugin::Plugin::Registration<PrivateBase> privatebase("privatebase");

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
