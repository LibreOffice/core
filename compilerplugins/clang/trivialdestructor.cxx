/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "config_clang.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

// Look for explicit destructors that can be trivial (and therefore don't need to be declared)

namespace
{
class TrivialDestructor : public loplugin::FilteringPlugin<TrivialDestructor>
{
public:
    explicit TrivialDestructor(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXDestructorDecl(CXXDestructorDecl const*);

private:
    bool HasTrivialDestructorBody(const CXXRecordDecl* ClassDecl);
    bool FieldHasTrivialDestructor(const FieldDecl* Field);
};

bool TrivialDestructor::VisitCXXDestructorDecl(CXXDestructorDecl const* destructorDecl)
{
    if (ignoreLocation(destructorDecl))
        return true;
    if (!destructorDecl->isThisDeclarationADefinition())
        return true;
    if (!destructorDecl->hasTrivialBody())
        return true;
    if (destructorDecl->isVirtual())
        return true;
    if (destructorDecl->getExceptionSpecType() != EST_None)
        return true;
    if (destructorDecl->getAccess() != AS_public)
        return true;
    if (isInUnoIncludeFile(
            compiler.getSourceManager().getSpellingLoc(destructorDecl->getLocation())))
        return true;
    if (!HasTrivialDestructorBody(destructorDecl->getParent()))
        return true;

    report(DiagnosticsEngine::Warning, "no need for explicit destructor decl",
           destructorDecl->getLocation())
        << destructorDecl->getSourceRange();
    for (FunctionDecl const* d2 = destructorDecl;;)
    {
        d2 = d2->getPreviousDecl();
        if (d2 == nullptr)
        {
            break;
        }
        report(DiagnosticsEngine::Note, "previous declaration is here", d2->getLocation())
            << d2->getSourceRange();
    }
    return true;
}

bool TrivialDestructor::HasTrivialDestructorBody(const CXXRecordDecl* ClassDecl)
{
    // Check fields.
    for (const auto* field : ClassDecl->fields())
        if (!FieldHasTrivialDestructor(field))
            return false;

    // Check bases.
    for (const auto& I : ClassDecl->bases())
    {
        const CXXRecordDecl* Base = I.getType()->getAsCXXRecordDecl();
        if (!Base->hasTrivialDestructor())
            return false;
    }

    return true;
}

bool TrivialDestructor::FieldHasTrivialDestructor(const FieldDecl* Field)
{
    QualType FieldBaseElementType = compiler.getASTContext().getBaseElementType(Field->getType());

    const RecordType* RT = FieldBaseElementType->getAs<RecordType>();
    if (!RT)
        return true;

    CXXRecordDecl* FieldClassDecl = cast<CXXRecordDecl>(RT->getDecl());

    return FieldClassDecl->hasTrivialDestructor();
}

loplugin::Plugin::Registration<TrivialDestructor> X("trivialdestructor", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
