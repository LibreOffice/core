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
    bool HasTrivialDestructorBody(const CXXRecordDecl* BaseClassDecl,
                                  const CXXRecordDecl* MostDerivedClassDecl);
    bool FieldHasTrivialDestructorBody(const FieldDecl* Field);
};

bool TrivialDestructor::VisitCXXDestructorDecl(CXXDestructorDecl const* destructorDecl)
{
    if (ignoreLocation(destructorDecl))
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
    if (!HasTrivialDestructorBody(destructorDecl->getParent(), destructorDecl->getParent()))
        return true;

    report(DiagnosticsEngine::Warning, "no need for explicit destructor decl",
           destructorDecl->getLocation())
        << destructorDecl->getSourceRange();
    if (destructorDecl->getCanonicalDecl() != destructorDecl)
    {
        destructorDecl = destructorDecl->getCanonicalDecl();
        report(DiagnosticsEngine::Warning, "no need for explicit destructor decl",
               destructorDecl->getLocation())
            << destructorDecl->getSourceRange();
    }
    return true;
}

bool TrivialDestructor::HasTrivialDestructorBody(const CXXRecordDecl* BaseClassDecl,
                                                 const CXXRecordDecl* MostDerivedClassDecl)
{
    if (BaseClassDecl != MostDerivedClassDecl && !BaseClassDecl->hasTrivialDestructor())
        return false;

    // Check fields.
    for (const auto* field : BaseClassDecl->fields())
        if (!FieldHasTrivialDestructorBody(field))
            return false;

    // Check non-virtual bases.
    for (const auto& I : BaseClassDecl->bases())
    {
        if (I.isVirtual())
            continue;
        if (!I.getType()->isRecordType())
            continue;
        const CXXRecordDecl* NonVirtualBase = I.getType()->getAsCXXRecordDecl();
        if (NonVirtualBase && !HasTrivialDestructorBody(NonVirtualBase, MostDerivedClassDecl))
            return false;
    }

    if (BaseClassDecl == MostDerivedClassDecl)
    {
        // Check virtual bases.
        for (const auto& I : BaseClassDecl->vbases())
        {
            if (!I.getType()->isRecordType())
                continue;
            const CXXRecordDecl* VirtualBase = I.getType()->getAsCXXRecordDecl();
            if (VirtualBase && !HasTrivialDestructorBody(VirtualBase, MostDerivedClassDecl))
                return false;
        }
    }
    return true;
}

bool TrivialDestructor::FieldHasTrivialDestructorBody(const FieldDecl* Field)
{
    QualType FieldBaseElementType = compiler.getASTContext().getBaseElementType(Field->getType());

    const RecordType* RT = FieldBaseElementType->getAs<RecordType>();
    if (!RT)
        return true;

    CXXRecordDecl* FieldClassDecl = cast<CXXRecordDecl>(RT->getDecl());

    // The destructor for an implicit anonymous union member is never invoked.
    if (FieldClassDecl->isUnion() && FieldClassDecl->isAnonymousStructOrUnion())
        return false;

    return FieldClassDecl->hasTrivialDestructor();
}

loplugin::Plugin::Registration<TrivialDestructor> X("trivialdestructor", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
