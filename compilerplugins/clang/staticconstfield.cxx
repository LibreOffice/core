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
class StaticConstField : public loplugin::FilteringPlugin<StaticConstField>
{
public:
    explicit StaticConstField(loplugin::InstantiationData const& data)
        : loplugin::FilteringPlugin<StaticConstField>(data)
    {
    }

    void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool TraverseConstructorInitializer(CXXCtorInitializer* init);
};

bool StaticConstField::TraverseConstructorInitializer(CXXCtorInitializer* init)
{
    if (!init->getSourceLocation().isValid() || ignoreLocation(init->getSourceLocation()))
        return true;
    if (!init->getMember())
        return true;
    auto type = init->getMember()->getType();
    auto tc = loplugin::TypeCheck(type);
    bool found = false;
    if (!tc.Const())
        return true;
    if (tc.Const().Class("OUString").Namespace("rtl").GlobalNamespace()
        || tc.Const().Class("OString").Namespace("rtl").GlobalNamespace())
    {
        if (auto constructExpr = dyn_cast<CXXConstructExpr>(init->getInit()))
        {
            if (constructExpr->getNumArgs() >= 1
                && isa<clang::StringLiteral>(constructExpr->getArg(0)))
                found = true;
        }
    }
    else if (type->isIntegerType())
    {
        if (isa<IntegerLiteral>(init->getInit()->IgnoreParenImpCasts()))
            found = true;
        // isIntegerType includes bool
        else if (isa<CXXBoolLiteralExpr>(init->getInit()->IgnoreParenImpCasts()))
            found = true;
    }
    else if (type->isFloatingType())
    {
        if (isa<FloatingLiteral>(init->getInit()->IgnoreParenImpCasts()))
            found = true;
    }
    else if (type->isEnumeralType())
    {
        if (auto declRefExpr = dyn_cast<DeclRefExpr>(init->getInit()->IgnoreParenImpCasts()))
        {
            if (isa<EnumConstantDecl>(declRefExpr->getDecl()))
                found = true;
        }
    }

    // If we find more than one non-copy-move constructor, we can't say for sure if a member can be static
    // because it could be initialised differently in each constructor.
    if (auto cxxRecordDecl = dyn_cast<CXXRecordDecl>(init->getMember()->getParent()))
    {
        int cnt = 0;
        for (auto it = cxxRecordDecl->ctor_begin(); it != cxxRecordDecl->ctor_end(); ++it)
        {
            if (!it->isCopyOrMoveConstructor())
                cnt++;
        }
        if (cnt > 1)
            return true;
    }

    if (!found)
        return true;

    std::string fn = handler.getMainFileName();
    loplugin::normalizeDotDotInFilePath(fn);

    // unusual case where a user constructor sets a field to one value, and a copy constructor sets it to a different value
    if (fn == SRCDIR "/sw/source/core/attr/hints.cxx")
        return true;

    report(DiagnosticsEngine::Warning, "field can be static const", init->getSourceLocation())
        << init->getSourceRange();
    report(DiagnosticsEngine::Note, "field here", init->getMember()->getLocation())
        << init->getMember()->getSourceRange();

    return true;
}

loplugin::Plugin::Registration<StaticConstField> X("staticconstfield", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
