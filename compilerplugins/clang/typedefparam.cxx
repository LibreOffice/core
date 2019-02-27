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
#include <fstream>
#include <set>
#include "plugin.hxx"

/**
  Check that parameters at the declaration site and the definition site are the same.
  This can be important when refactoring, and can sometimes make a difference when compiling
  for platforms with different pointer-sizes (e.g. 32 vs 64 bit)
 */
namespace
{
class TypedefParam : public loplugin::FilteringRewritePlugin<TypedefParam>
{
public:
    explicit TypedefParam(loplugin::InstantiationData const& data)
        : FilteringRewritePlugin(data)
    {
    }

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFunctionDecl(FunctionDecl const*);
};

static bool areTypesEqual(QualType lhs, QualType rhs);

bool TypedefParam::VisitFunctionDecl(FunctionDecl const* functionDecl)
{
    if (ignoreLocation(functionDecl))
        return true;
    if (functionDecl->isFunctionTemplateSpecialization())
        return true;
    auto canonicalDecl = functionDecl->getCanonicalDecl();
    if (canonicalDecl == functionDecl)
        return true;

    for (unsigned i = 0; i < functionDecl->getNumParams(); ++i)
    {
        ParmVarDecl const* thisParam = functionDecl->getParamDecl(i);
        ParmVarDecl const* canonicalParam = canonicalDecl->getParamDecl(i);
        if (!areTypesEqual(thisParam->getType(), canonicalParam->getType()))
        {
            report(DiagnosticsEngine::Warning,
                   "function param %0 at definition site does not match function param at "
                   "declaration site, %1 vs %2",
                   compat::getBeginLoc(functionDecl))
                << i << thisParam->getType() << canonicalParam->getType()
                << functionDecl->getSourceRange();
            report(DiagnosticsEngine::Note, "declaration site here",
                   compat::getBeginLoc(canonicalDecl))
                << canonicalDecl->getSourceRange();
            thisParam->getType()->dump();
            canonicalParam->getType()->dump();
        }
    }

    if (!areTypesEqual(functionDecl->getReturnType(), canonicalDecl->getReturnType()))
    {
        report(DiagnosticsEngine::Warning,
               "function return type at definition site does not match function param at "
               "declaration site, %0 vs %1",
               compat::getBeginLoc(functionDecl))
            << functionDecl->getReturnType() << canonicalDecl->getReturnType()
            << functionDecl->getSourceRange();
        report(DiagnosticsEngine::Note, "declaration site here", compat::getBeginLoc(canonicalDecl))
            << canonicalDecl->getSourceRange();
        functionDecl->getReturnType()->dump();
        canonicalDecl->getReturnType()->dump();
    }
    return true;
}

static bool areTypesEqual(QualType lhs, QualType rhs)
{
    if (lhs == rhs)
        return true;
    if (lhs->isDependentType() || rhs->isDependentType()) // ignore
        return true;
    auto lhsType = lhs.getTypePtr();
    auto rhsType = rhs.getTypePtr();
    if (auto lhsElaborated = dyn_cast<ElaboratedType>(lhsType))
    {
        return areTypesEqual(lhsElaborated->getNamedType(), rhs);
    }
    if (auto rhsElaborated = dyn_cast<ElaboratedType>(rhsType))
    {
        return areTypesEqual(lhs, rhsElaborated->getNamedType());
    }
    if (auto lhsLValue = dyn_cast<LValueReferenceType>(lhsType))
    {
        auto rhsLValue = dyn_cast<LValueReferenceType>(rhsType);
        if (!rhsLValue)
            return false;
        return areTypesEqual(lhsLValue->getPointeeType(), rhsLValue->getPointeeType());
    }
    if (auto lhsRValue = dyn_cast<RValueReferenceType>(lhsType))
    {
        auto rhsRValue = dyn_cast<RValueReferenceType>(rhsType);
        if (!rhsRValue)
            return false;
        return areTypesEqual(lhsRValue->getPointeeType(), rhsRValue->getPointeeType());
    }
    if (auto lhsPointer = dyn_cast<clang::PointerType>(lhsType))
    {
        auto rhsPointer = dyn_cast<clang::PointerType>(rhsType);
        if (!rhsPointer)
            return false;
        return areTypesEqual(lhsPointer->getPointeeType(), rhsPointer->getPointeeType());
    }
    if (auto lhsTypedef = dyn_cast<TypedefType>(lhsType))
    {
        auto rhsTypedef = dyn_cast<TypedefType>(rhsType);
        if (!rhsTypedef)
            return false;
        // comparing the underlying Decl's here doesn't work, they are not uniqued
        if (lhsTypedef->getDecl()->getName() != rhsTypedef->getDecl()->getName())
            return false;
        return areTypesEqual(lhsTypedef->desugar(), rhsTypedef->desugar());
    }
    if (auto lhsTemplate = dyn_cast<TemplateSpecializationType>(lhsType))
    {
        auto rhsTemplate = dyn_cast<TemplateSpecializationType>(rhsType);
        if (!rhsTemplate)
            return false;
        return areTypesEqual(lhsTemplate->desugar(), rhsTemplate->desugar());
    }
    if (auto lhsMember = dyn_cast<MemberPointerType>(lhsType))
    {
        auto rhsMember = dyn_cast<MemberPointerType>(rhsType);
        if (!rhsMember)
            return false;
        if (lhsMember->getClass() != rhsMember->getClass())
            return true;
        return areTypesEqual(lhsMember->getPointeeType(), rhsMember->getPointeeType());
    }
    if (auto lhsParen = dyn_cast<ParenType>(lhsType))
    {
        auto rhsParen = dyn_cast<ParenType>(rhsType);
        if (!rhsParen)
            return false;
        return areTypesEqual(lhsParen->getInnerType(), rhsParen->getInnerType());
    }
    if (dyn_cast<FunctionProtoType>(lhsType))
    {
        auto rhsFunction = dyn_cast<FunctionProtoType>(rhsType);
        if (!rhsFunction)
            return false;
        return true; // TODO
    }
    if (auto lhsDecayed = dyn_cast<DecayedType>(lhsType))
    {
        auto rhsDecayed = dyn_cast<DecayedType>(rhsType);
        if (!rhsDecayed)
            return false;
        return areTypesEqual(lhsDecayed->getAdjustedType(), rhsDecayed->getAdjustedType());
    }
    return lhsType == rhsType;
}

loplugin::Plugin::Registration<TypedefParam> X("typedefparam", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
