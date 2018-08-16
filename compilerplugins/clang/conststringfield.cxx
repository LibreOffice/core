/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include "check.hxx"
#include "compat.hxx"
#include <iostream>

namespace
{
class ConstStringField : public loplugin::FilteringPlugin<ConstStringField>
{
public:
    explicit ConstStringField(loplugin::InstantiationData const& data)
        : loplugin::FilteringPlugin<ConstStringField>(data)
    {
    }

    void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool TraverseConstructorInitializer(CXXCtorInitializer* init);
};

bool ConstStringField::TraverseConstructorInitializer(CXXCtorInitializer* init)
{
    if (!init->getSourceLocation().isValid() || ignoreLocation(init->getSourceLocation()))
        return true;
    if (!init->getMember())
        return true;
    auto tc = loplugin::TypeCheck(init->getMember()->getType());
    if (!tc.Const().Class("OUString").Namespace("rtl").GlobalNamespace()
        && !tc.Const().Class("OString").Namespace("rtl").GlobalNamespace())
        return true;
    if (auto constructExpr = dyn_cast<CXXConstructExpr>(init->getInit()))
    {
        if (constructExpr->getNumArgs() >= 1 && isa<clang::StringLiteral>(constructExpr->getArg(0)))
        {
            report(DiagnosticsEngine::Warning, "string field can be static const",
                   init->getSourceLocation())
                << init->getSourceRange();
            report(DiagnosticsEngine::Note, "field here", init->getMember()->getLocation())
                << init->getMember()->getSourceRange();
        }
    }
    return RecursiveASTVisitor::TraverseConstructorInitializer(init);
}

loplugin::Plugin::Registration<ConstStringField> X("conststringfield", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
