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
#include <iostream>
#include <fstream>
#include <set>

#include "check.hxx"
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

    void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFunctionDecl(FunctionDecl const*);
    bool VisitCXXMethodDecl(CXXMethodDecl const*);
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
#if defined _WIN32
            // SAL_IMPLEMENT_MAIN (include/sal/main.h) declares the third parameter of WinMain to be
            // of type 'char *' rather than 'LPSTR', but using that typedef there would require
            // including windows.h, which would require including include/prewin.h and
            // include/postwin.h (to undo macros like Yield defined in windows.h) but which (unlike
            // include/sal/main.h) are not part of the stable URE interface; so just ignore that
            // here:
            if (loplugin::DeclCheck(functionDecl).Function("WinMain").GlobalNamespace())
            {
                continue;
            }
#endif
            report(DiagnosticsEngine::Warning,
                   "function param %0 at definition site does not match function param at "
                   "declaration site, %1 vs %2",
                   thisParam->getLocation())
                << (i + 1) << thisParam->getType() << canonicalParam->getType()
                << functionDecl->getSourceRange();
            report(DiagnosticsEngine::Note, "declaration site here", canonicalParam->getLocation())
                << canonicalDecl->getSourceRange();
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
    }
    return true;
}

bool TypedefParam::VisitCXXMethodDecl(CXXMethodDecl const* methodDecl)
{
    if (ignoreLocation(methodDecl))
        return true;
    if (methodDecl->isFunctionTemplateSpecialization())
        return true;
    // only warn on the declaration site
    if (methodDecl->getCanonicalDecl() != methodDecl)
        return true;

    StringRef aFileName = getFilenameOfLocation(
        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(methodDecl)));
    // seems to be using typedefs as a form of documentation for method params
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sw/source/filter/ww8/ww8scan.hxx"))
        return true;
    // necessary to work around changes in popplers API
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR
                                    "/sdext/source/pdfimport/xpdfwrapper/pdfioutdev_gpl.hxx"))
        return true;

    for (auto superMethodIt = methodDecl->begin_overridden_methods();
         superMethodIt != methodDecl->end_overridden_methods(); ++superMethodIt)
    {
        const CXXMethodDecl* superMethodDecl = *superMethodIt;
        int i = 0;
        for (const ParmVarDecl* superParmVarDecl : superMethodDecl->parameters())
        {
            const ParmVarDecl* parmVarDecl = methodDecl->getParamDecl(i);
            if (!areTypesEqual(parmVarDecl->getType(), superParmVarDecl->getType()))
            {
                report(DiagnosticsEngine::Warning,
                       "method param %0 does not match overridden method param "
                       "%1 vs %2",
                       parmVarDecl->getLocation())
                    << (i + 1) << parmVarDecl->getType() << superParmVarDecl->getType()
                    << methodDecl->getSourceRange();
                report(DiagnosticsEngine::Note, "super-class method here",
                       superParmVarDecl->getLocation())
                    << superMethodDecl->getSourceRange();
            }
            ++i;
        }

        // it is quite normal to override a method and return a more specific pointer/reference type
        auto returnType = methodDecl->getReturnType();
        if (!returnType->isPointerType() && !returnType->isReferenceType())
            if (!areTypesEqual(returnType, superMethodDecl->getReturnType()))
            {
                report(DiagnosticsEngine::Warning,
                       "method return type does not match overridden method "
                       "%0 vs %1",
                       compat::getBeginLoc(methodDecl))
                    << methodDecl->getReturnType() << superMethodDecl->getReturnType()
                    << methodDecl->getSourceRange();
                report(DiagnosticsEngine::Note, "super-class method here",
                       compat::getBeginLoc(superMethodDecl))
                    << superMethodDecl->getSourceRange();
            }
    }
    return true;
}

static bool areTypesEqual(QualType lhs, QualType rhs)
{
    if (lhs == rhs)
        return true;

    // ignore template stuff
    if (lhs->isDependentType() || rhs->isDependentType())
        return true;
    // there are some places, e.g. chart2/source/controller/chartapiwrapper/WrappedSymbolProperties.cxx
    // where just unwrapping these types does not work, so just ignore them
    if (isa<SubstTemplateTypeParmType>(lhs) || isa<SubstTemplateTypeParmType>(rhs))
        return true;

    auto lhsType = lhs.getTypePtr();
    auto rhsType = rhs.getTypePtr();

    // this is the carve-out exception for the "typedef struct S {...} T" idiom we use in the UNO code
    if (auto lhsPointer = dyn_cast<clang::PointerType>(lhsType))
    {
        if (auto rhsPointer = dyn_cast<clang::PointerType>(rhsType))
        {
            auto extractRecordType = [](clang::QualType type) {
                auto recordType = dyn_cast<RecordType>(type);
                if (recordType)
                    return recordType;
                auto elaboratedType = dyn_cast<ElaboratedType>(type);
                if (!elaboratedType)
                    return static_cast<const clang::RecordType*>(nullptr);
                return dyn_cast<RecordType>(elaboratedType->desugar());
            };
            auto containsTypedefToRecord = [](clang::QualType type, RecordType const* recordType) {
                TypedefType const* typedefType = dyn_cast<TypedefType>(type);
                if (!typedefType)
                    return false;
                auto tmp = typedefType->desugar();
                if (auto elaboratedType = dyn_cast<ElaboratedType>(tmp))
                    tmp = elaboratedType->desugar();
                return tmp.getTypePtr() == recordType;
            };
            if (auto recordType = extractRecordType(lhsPointer->getPointeeType()))
                if (containsTypedefToRecord(rhsPointer->getPointeeType(), recordType))
                    return true;
            if (auto recordType = extractRecordType(rhsPointer->getPointeeType()))
                if (containsTypedefToRecord(lhsPointer->getPointeeType(), recordType))
                    return true;
        }
    }

    if (auto lhsElaborated = dyn_cast<ElaboratedType>(lhsType))
    {
        return areTypesEqual(lhsElaborated->getNamedType(), rhs);
    }
    if (auto rhsElaborated = dyn_cast<ElaboratedType>(rhsType))
    {
        return areTypesEqual(lhs, rhsElaborated->getNamedType());
    }
    if (auto lhsTemplate = dyn_cast<TemplateSpecializationType>(lhsType))
    {
        return areTypesEqual(lhsTemplate->desugar(), rhs);
    }
    if (auto rhsTemplate = dyn_cast<TemplateSpecializationType>(rhsType))
    {
        return areTypesEqual(lhs, rhsTemplate->desugar());
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
        // comparing the underlying Decl's here doesn't work, they are not unique
        if (lhsTypedef->getDecl()->getName() != rhsTypedef->getDecl()->getName())
            return false;
#if defined __APPLE__
        // My Clang trunk .../lib/clang/9.0.0/include/stddef.h has a
        //
        //   typedef long unsigned int size_t;
        //
        // while /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/
        // SDKs/MacOSX10.14.sdk/usr/include/sys/_types/_size_t.h has a
        //
        //   typedef __darwin_size_t size_t;
        //
        // where __darwin_size_t is a typedef for long unsigned int, too, so that, depending on the
        // order in which those two files get included, either of those two typedefs can act as a
        // redeclaration of the other one.  However, areTypesEqual would unhelpfully consider such
        // different occurrences of size_t to be non-equal, so filter them out here.  And, at least
        // with my libcxx trunk .../include/c++/v1/cstddef, std::size_t is a using declaration that
        // brings size_t from the global namespace into namespace std, so that the above checks for
        // ElaboratedType sugar will already have unwrapped those to the size_t typedefs in the
        // global namespace here.
        if (loplugin::TypeCheck(lhsTypedef).Typedef("size_t").GlobalNamespace()
            && loplugin::TypeCheck(rhsTypedef).Typedef("size_t").GlobalNamespace())
        {
            return true;
        }
#endif
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
    if (auto lhsAttr = dyn_cast<AttributedType>(lhsType))
    {
        auto rhsAttr = dyn_cast<AttributedType>(rhsType);
        if (!rhsAttr)
            return false;
        return areTypesEqual(lhsAttr->getModifiedType(), rhsAttr->getModifiedType());
    }
    return lhsType == rhsType;
}

loplugin::Plugin::Registration<TypedefParam> typedefparam("typedefparam");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
