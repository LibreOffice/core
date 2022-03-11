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

// Look for declared constructors that can be trivial (and therefore don't need to be declared)

namespace
{
class TrivialConstructor : public loplugin::FilteringPlugin<TrivialConstructor>
{
public:
    explicit TrivialConstructor(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXConstructorDecl(CXXConstructorDecl const*);

private:
    bool HasTrivialConstructorBody(const CXXRecordDecl* BaseClassDecl,
                                   const CXXRecordDecl* MostDerivedClassDecl);
    bool FieldHasTrivialConstructorBody(const FieldDecl* Field);
};

bool TrivialConstructor::VisitCXXConstructorDecl(CXXConstructorDecl const* constructorDecl)
{
    if (ignoreLocation(constructorDecl))
        return true;
    if (!constructorDecl->hasTrivialBody())
        return true;
    if (constructorDecl->isExplicit())
        return true;
    if (!constructorDecl->isDefaultConstructor())
        return true;
    if (!constructorDecl->inits().empty())
        return true;
    if (constructorDecl->getExceptionSpecType() != EST_None)
        return true;
    if (constructorDecl->getAccess() != AS_public)
        return true;
    if (!constructorDecl->isThisDeclarationADefinition())
        return true;
    if (isInUnoIncludeFile(
            compiler.getSourceManager().getSpellingLoc(constructorDecl->getLocation())))
        return true;
    const CXXRecordDecl* recordDecl = constructorDecl->getParent();
    if (std::distance(recordDecl->ctor_begin(), recordDecl->ctor_end()) != 1)
        return true;
    if (!HasTrivialConstructorBody(recordDecl, recordDecl))
        return true;

    // template magic in sc/inc/stlalgorithm.hxx
    if (recordDecl->getIdentifier() && recordDecl->getName() == "AlignedAllocator")
        return true;

    report(DiagnosticsEngine::Warning, "no need for explicit constructor decl",
           constructorDecl->getLocation())
        << constructorDecl->getSourceRange();
    if (constructorDecl->getCanonicalDecl() != constructorDecl)
    {
        constructorDecl = constructorDecl->getCanonicalDecl();
        report(DiagnosticsEngine::Warning, "no need for explicit constructor decl",
               constructorDecl->getLocation())
            << constructorDecl->getSourceRange();
    }
    return true;
}

bool TrivialConstructor::HasTrivialConstructorBody(const CXXRecordDecl* BaseClassDecl,
                                                   const CXXRecordDecl* MostDerivedClassDecl)
{
    if (BaseClassDecl != MostDerivedClassDecl && !BaseClassDecl->hasTrivialDefaultConstructor())
        return false;

    // Check fields.
    for (const auto* field : BaseClassDecl->fields())
        if (!FieldHasTrivialConstructorBody(field))
            return false;

    // Check non-virtual bases.
    for (const auto& I : BaseClassDecl->bases())
    {
        if (I.isVirtual())
            continue;
        if (!I.getType()->isRecordType())
            continue;
        const CXXRecordDecl* NonVirtualBase = I.getType()->getAsCXXRecordDecl();
        if (NonVirtualBase && !HasTrivialConstructorBody(NonVirtualBase, MostDerivedClassDecl))
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
            if (VirtualBase && !HasTrivialConstructorBody(VirtualBase, MostDerivedClassDecl))
                return false;
        }
    }
    return true;
}

bool TrivialConstructor::FieldHasTrivialConstructorBody(const FieldDecl* Field)
{
    QualType FieldBaseElementType = compiler.getASTContext().getBaseElementType(Field->getType());

    const RecordType* RT = FieldBaseElementType->getAs<RecordType>();
    if (!RT)
        return true;

    CXXRecordDecl* FieldClassDecl = cast<CXXRecordDecl>(RT->getDecl());

    // The constructor for an implicit anonymous union member is never invoked.
    if (FieldClassDecl->isUnion() && FieldClassDecl->isAnonymousStructOrUnion())
        return false;

    return FieldClassDecl->hasTrivialDefaultConstructor();
}

loplugin::Plugin::Registration<TrivialConstructor> X("trivialconstructor", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
