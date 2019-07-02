/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include "plugin.hxx"
#include <clang/AST/Attr.h>
#include "clang/AST/CXXInheritance.h"
#include <iostream>

/*
This is a compile-time checker.

Check for cases where we have
 - two IDL interfaces A and B,
 - B extends A
 - we are converting a Reference<B> to a Reference<A> using UNO_QUERY

This makes the code simpler and cheaper, because UNO_QUERY can be surprisingly expensive if used a lot.

*/

class ReferenceCasting : public loplugin::FilteringPlugin<ReferenceCasting>
{
public:
    explicit ReferenceCasting(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }
    void run() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        // macros
        if (fn == SRCDIR "/dbaccess/source/ui/browser/formadapter.cxx")
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }
    bool VisitCXXConstructExpr(const CXXConstructExpr* cce);
    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr* mce);
};

static const RecordType* extractTemplateType(const clang::Type*);
static bool isDerivedFrom(const CXXRecordDecl* subtypeRecord, const CXXRecordDecl* baseRecord);

bool ReferenceCasting::VisitCXXConstructExpr(const CXXConstructExpr* cce)
{
    // don't bother processing anything in the Reference.h file. Makes my life easier when debugging this.
    StringRef aFileName = getFileNameOfSpellingLoc(
        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(cce)));
    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/com/sun/star/uno/Reference.h"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/com/sun/star/uno/Reference.hxx"))
        return true;

    // look for calls to the Reference<T>(x, UNO_something) constructor
    auto constructorClass = cce->getConstructor()->getParent();
    if (!constructorClass->getIdentifier() || constructorClass->getName() != "Reference")
        return true;

    if (cce->getNumArgs() != 2)
        return true;

    // ignore the up-casting constructor
    if (!isa<EnumType>(cce->getConstructor()->getParamDecl(1)->getType()))
        return true;

    // extract the type parameter passed to the template
    const RecordType* templateParamType = extractTemplateType(cce->getType().getTypePtr());
    if (!templateParamType)
        return true;

    // extract the type of the first parameter passed to the constructor
    const Expr* constructorArg0 = cce->getArg(0);
    if (!constructorArg0)
        return true;

    // ignore the Reference(XInterface*,...) constructor
    if (constructorArg0->getType()->isPointerType())
        return true;

    // drill down the expression tree till we hit the bottom, because at the top, the type is BaseReference
    const clang::Type* argType;
    for (;;)
    {
        if (auto castExpr = dyn_cast<CastExpr>(constructorArg0))
        {
            constructorArg0 = castExpr->getSubExpr();
            continue;
        }
        if (auto matTempExpr = dyn_cast<MaterializeTemporaryExpr>(constructorArg0))
        {
            constructorArg0 = matTempExpr->GetTemporaryExpr();
            continue;
        }
        if (auto bindTempExpr = dyn_cast<CXXBindTemporaryExpr>(constructorArg0))
        {
            constructorArg0 = bindTempExpr->getSubExpr();
            continue;
        }
        if (auto tempObjExpr = dyn_cast<CXXTemporaryObjectExpr>(constructorArg0))
        {
            constructorArg0 = tempObjExpr->getArg(0);
            continue;
        }
        if (auto parenExpr = dyn_cast<ParenExpr>(constructorArg0))
        {
            constructorArg0 = parenExpr->getSubExpr();
            continue;
        }
        argType = constructorArg0->getType().getTypePtr();
        break;
    }

    const RecordType* argTemplateType = extractTemplateType(argType);
    if (!argTemplateType)
        return true;

    CXXRecordDecl* templateParamRD = dyn_cast<CXXRecordDecl>(templateParamType->getDecl());
    CXXRecordDecl* constructorArgRD = dyn_cast<CXXRecordDecl>(argTemplateType->getDecl());

    if (auto declRefExpr = dyn_cast<DeclRefExpr>(cce->getArg(1)))
    {
        // no warning expected, used to reject null references
        if (auto enumConstantDecl = dyn_cast<EnumConstantDecl>(declRefExpr->getDecl()))
        {
            if (enumConstantDecl->getName() == "UNO_SET_THROW")
                return true;
            if (enumConstantDecl->getName() == "UNO_QUERY_THROW")
                return true;
        }
    }

    if (constructorArgRD->Equals(templateParamRD)
        || isDerivedFrom(constructorArgRD, templateParamRD))
    {
        report(DiagnosticsEngine::Warning,
               "the source reference is already a subtype of the destination reference, just use =",
               cce->getBeginLoc())
            << cce->getSourceRange();
    }
    return true;
}

bool ReferenceCasting::VisitCXXMemberCallExpr(const CXXMemberCallExpr* mce)
{
    // don't bother processing anything in the Reference.h file. Makes my life easier when debugging this.
    StringRef aFileName = getFileNameOfSpellingLoc(
        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(mce)));
    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/com/sun/star/uno/Reference.h"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/com/sun/star/uno/Reference.hxx"))
        return true;

    // look for calls to the Reference<T>.set(x, UNO_QUERY) constructor
    auto method = mce->getMethodDecl();
    if (!method || !method->getIdentifier() || method->getName() != "set")
        return true;
    if (mce->getNumArgs() != 2)
        return true;

    auto methodRecordDecl = dyn_cast<ClassTemplateSpecializationDecl>(mce->getRecordDecl());
    if (!methodRecordDecl || !methodRecordDecl->getIdentifier()
        || methodRecordDecl->getName() != "Reference")
        return true;

    // extract the type parameter passed to the template
    const RecordType* templateParamType
        = dyn_cast<RecordType>(methodRecordDecl->getTemplateArgs()[0].getAsType());
    if (!templateParamType)
        return true;

    // extract the type of the first parameter passed to the method
    const Expr* arg0 = mce->getArg(0);
    if (!arg0)
        return true;

    // ignore the Reference(XInterface*,...) constructor
    if (arg0->getType()->isPointerType())
        return true;

    // drill down the expression tree till we hit the bottom, because at the top, the type is BaseReference
    const clang::Type* argType;
    for (;;)
    {
        if (auto castExpr = dyn_cast<CastExpr>(arg0))
        {
            arg0 = castExpr->getSubExpr();
            continue;
        }
        if (auto matTempExpr = dyn_cast<MaterializeTemporaryExpr>(arg0))
        {
            arg0 = matTempExpr->GetTemporaryExpr();
            continue;
        }
        if (auto bindTempExpr = dyn_cast<CXXBindTemporaryExpr>(arg0))
        {
            arg0 = bindTempExpr->getSubExpr();
            continue;
        }
        if (auto tempObjExpr = dyn_cast<CXXTemporaryObjectExpr>(arg0))
        {
            arg0 = tempObjExpr->getArg(0);
            continue;
        }
        if (auto parenExpr = dyn_cast<ParenExpr>(arg0))
        {
            arg0 = parenExpr->getSubExpr();
            continue;
        }
        argType = arg0->getType().getTypePtr();
        break;
    }

    const RecordType* argTemplateType = extractTemplateType(argType);
    if (!argTemplateType)
        return true;

    CXXRecordDecl* templateParamRD = dyn_cast<CXXRecordDecl>(templateParamType->getDecl());
    CXXRecordDecl* methodArgRD = dyn_cast<CXXRecordDecl>(argTemplateType->getDecl());

    if (auto declRefExpr = dyn_cast<DeclRefExpr>(mce->getArg(1)))
    {
        // no warning expected, used to reject null references
        if (auto enumConstantDecl = dyn_cast<EnumConstantDecl>(declRefExpr->getDecl()))
        {
            if (enumConstantDecl->getName() == "UNO_SET_THROW")
                return true;
            if (enumConstantDecl->getName() == "UNO_QUERY_THROW")
                return true;
        }
    }

    if (methodArgRD->Equals(templateParamRD) || isDerivedFrom(methodArgRD, templateParamRD))
    {
        report(DiagnosticsEngine::Warning,
               "the source reference is already a subtype of the destination reference, just use =",
               mce->getBeginLoc())
            << mce->getSourceRange();
    }
    return true;
}

static const RecordType* extractTemplateType(const clang::Type* cceType)
{
    if (auto elaboratedType = dyn_cast<ElaboratedType>(cceType))
        cceType = elaboratedType->desugar().getTypePtr();
    auto cceTST = dyn_cast<TemplateSpecializationType>(cceType);
    if (!cceTST)
        return NULL;
    if (cceTST->getNumArgs() != 1)
        return NULL;
    const TemplateArgument& cceTA = cceTST->getArg(0);
    const clang::Type* templateParamType = cceTA.getAsType().getTypePtr();
    if (auto elaboratedType = dyn_cast<ElaboratedType>(templateParamType))
        templateParamType = elaboratedType->desugar().getTypePtr();
    return dyn_cast<RecordType>(templateParamType);
}

/**
  Implement my own isDerived because we can't always see all the definitions of the classes involved.
  which will cause an assert with the normal clang isDerivedFrom code.
*/
static bool isDerivedFrom(const CXXRecordDecl* subtypeRecord, const CXXRecordDecl* baseRecord)
{
    if (!subtypeRecord->hasDefinition())
        return false;
    for (auto it = subtypeRecord->bases_begin(); it != subtypeRecord->bases_end(); ++it)
    {
        QualType qt = it->getType();
        auto type = qt.getTypePtr();
        if (auto elaboratedType = dyn_cast<ElaboratedType>(type))
            type = elaboratedType->desugar().getTypePtr();
        if (auto recordType = dyn_cast<RecordType>(type))
        {
            if (auto cxxRecordDecl = dyn_cast<CXXRecordDecl>(recordType->getDecl()))
            {
                if (cxxRecordDecl->Equals(baseRecord))
                    return true;
                if (isDerivedFrom(cxxRecordDecl, baseRecord))
                    return true;
            }
        }
    }
    return false;
}

loplugin::Plugin::Registration<ReferenceCasting> X2("referencecasting");

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
