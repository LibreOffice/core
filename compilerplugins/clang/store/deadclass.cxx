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

class DeadClass:
    public loplugin::FilteringPlugin<DeadClass>
{
public:
    explicit DeadClass(InstantiationData const & data): FilteringPlugin(data) {}

    void run() override;

    bool VisitCXXRecordDecl(CXXRecordDecl const *);
};

void DeadClass::run() {
    if (compiler.getLangOpts().CPlusPlus) {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }
}

bool DeadClass::VisitCXXRecordDecl(CXXRecordDecl const * decl) {
    if (ignoreLocation(decl) || !decl->isThisDeclarationADefinition())
        return true;
    if (decl->needsImplicitDefaultConstructor())
        return true;
    if (decl->getDescribedClassTemplate())
        return true;
    if (isa<ClassTemplateSpecializationDecl>(decl))
        return true;
    int otherCnt = 0;
    int copyMoveCnt = 0;
    for (auto i = decl->ctor_begin(); i != decl->ctor_end(); ++i) {
        if (!i->isUserProvided())
            continue;
        if (i->isCopyOrMoveConstructor())
            copyMoveCnt++;
        else
            otherCnt++;
    }
    if (otherCnt == 0 && copyMoveCnt > 0)
    {
        report(
            DiagnosticsEngine::Warning,
            "class has only copy/move constructors, must be dead",
            decl->getLocStart())
            << decl->getSourceRange();
        for (auto i = decl->ctor_begin(); i != decl->ctor_end(); ++i) {
            if (i->isDeleted())
                continue;
        }
    }
    return true;
}

loplugin::Plugin::Registration<DeadClass> X("deadclass");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
