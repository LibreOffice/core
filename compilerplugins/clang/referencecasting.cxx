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
#ifndef LO_CLANG_SHARED_PLUGINS

#include "compat.hxx"
#include "plugin.hxx"
#include "check.hxx"
#include <iostream>

/*
This is a compile-time checker.

Check for cases where we have
 - two IDL interfaces A and B,
 - B extends A
 - we are converting a Reference<B> to a Reference<A> using UNO_QUERY

This makes the code simpler and cheaper, because UNO_QUERY can be surprisingly expensive if used a lot.

*/

namespace
{
class ReferenceCasting : public loplugin::FilteringPlugin<ReferenceCasting>
{
public:
    explicit ReferenceCasting(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        // macros
        if (fn == SRCDIR "/dbaccess/source/ui/browser/formadapter.cxx")
            return false;
        // UNO aggregation
        if (fn == SRCDIR "/toolkit/source/controls/stdtabcontroller.cxx")
            return false;
        return true;
    }

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitCXXConstructExpr(const CXXConstructExpr* cce);
    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr* mce);

private:
    bool CheckForUnnecessaryGet(const Expr*);
};

static const RecordType* extractTemplateType(const clang::Type*);
static bool isDerivedFrom(const CXXRecordDecl* subtypeRecord, const CXXRecordDecl* baseRecord);

bool ReferenceCasting::VisitCXXConstructExpr(const CXXConstructExpr* cce)
{
    // don't bother processing anything in the Reference.h file. Makes my life easier when debugging this.
    StringRef aFileName = getFilenameOfLocation(
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

    if (CheckForUnnecessaryGet(cce->getArg(0)))
        report(DiagnosticsEngine::Warning, "unnecessary get() call", compat::getBeginLoc(cce))
            << cce->getSourceRange();

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
            constructorArg0 = compat::getSubExpr(matTempExpr);
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

    // querying for XInterface (instead of doing an upcast) has special semantics,
    // to check for UNO object equivalence.
    if (templateParamRD->getName() == "XInterface")
        return true;

    // XShape is used in UNO aggregates in very "entertaining" ways, which means an UNO_QUERY
    // can return a completely different object, e.g. see SwXShape::queryInterface
    if (templateParamRD->getName() == "XShape")
        return true;

    if (auto declRefExpr = dyn_cast<DeclRefExpr>(cce->getArg(1)))
    {
        // no warning expected, used to reject null references
        if (auto enumConstantDecl = dyn_cast<EnumConstantDecl>(declRefExpr->getDecl()))
        {
            if (enumConstantDecl->getName() == "UNO_SET_THROW")
                return true;
            if (enumConstantDecl->getName() == "UNO_QUERY_THROW")
                return true;
            if (enumConstantDecl->getName() == "SAL_NO_ACQUIRE")
                return true;
        }
    }

    if (constructorArgRD->Equals(templateParamRD)
        || isDerivedFrom(constructorArgRD, templateParamRD))
    {
        report(DiagnosticsEngine::Warning,
               "the source reference is already a subtype of the destination reference, just use =",
               compat::getBeginLoc(cce))
            << cce->getSourceRange();
    }
    return true;
}

bool ReferenceCasting::VisitCXXMemberCallExpr(const CXXMemberCallExpr* mce)
{
    // don't bother processing anything in the Reference.h file. Makes my life easier when debugging this.
    StringRef aFileName = getFilenameOfLocation(
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

    if (CheckForUnnecessaryGet(mce->getArg(0)))
        report(DiagnosticsEngine::Warning, "unnecessary get() call", compat::getBeginLoc(mce))
            << mce->getSourceRange();

    // extract the type parameter passed to the template
    const RecordType* templateParamType
        = dyn_cast<RecordType>(methodRecordDecl->getTemplateArgs()[0].getAsType());
    if (!templateParamType)
        return true;

    // extract the type of the first parameter passed to the method
    const Expr* arg0 = mce->getArg(0);
    if (!arg0)
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
            arg0 = compat::getSubExpr(matTempExpr);
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

    // querying for XInterface (instead of doing an upcast) has special semantics,
    // to check for UNO object equivalence.
    if (templateParamRD->getName() == "XInterface")
        return true;

    // XShape is used in UNO aggregates in very "entertaining" ways, which means an UNO_QUERY
    // can return a completely different object, e.g. see SwXShape::queryInterface
    if (templateParamRD->getName() == "XShape")
        return true;

    if (auto declRefExpr = dyn_cast<DeclRefExpr>(mce->getArg(1)))
    {
        // no warning expected, used to reject null references
        if (auto enumConstantDecl = dyn_cast<EnumConstantDecl>(declRefExpr->getDecl()))
        {
            if (enumConstantDecl->getName() == "UNO_SET_THROW")
                return true;
            if (enumConstantDecl->getName() == "UNO_QUERY_THROW")
                return true;
            if (enumConstantDecl->getName() == "SAL_NO_ACQUIRE")
                return true;
        }
    }

    if (methodArgRD->Equals(templateParamRD) || isDerivedFrom(methodArgRD, templateParamRD))
    {
        report(DiagnosticsEngine::Warning,
               "the source reference is already a subtype of the destination reference, just use =",
               compat::getBeginLoc(mce))
            << mce->getSourceRange();
    }
    return true;
}

/**
    Check for
        Reference<T>(x.get(), UNO_QUERY)
    because sometimes simplifying that means the main purpose of this plugin can kick in.
 */
bool ReferenceCasting::CheckForUnnecessaryGet(const Expr* expr)
{
    expr = expr->IgnoreImplicit();
    auto cxxMemberCallExpr = dyn_cast<CXXMemberCallExpr>(expr);
    if (!cxxMemberCallExpr)
        return false;
    auto methodDecl = cxxMemberCallExpr->getMethodDecl();
    if (!methodDecl)
        return false;
    if (!methodDecl->getIdentifier() || methodDecl->getName() != "get")
        return false;

    if (!loplugin::TypeCheck(expr->getType()).Pointer())
        return false;
    if (!loplugin::DeclCheck(methodDecl->getParent()).Class("Reference").Namespace("uno"))
        return false;

    StringRef aFileName = getFilenameOfLocation(
        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(expr)));
    if (loplugin::isSamePathname(aFileName, SRCDIR "/cppu/qa/test_reference.cxx"))
        return false;

    return true;
}

static const RecordType* extractTemplateType(const clang::Type* cceType)
{
    // check for passing raw pointer to interface case
    if (cceType->isPointerType())
    {
        auto pointeeType = cceType->getPointeeType();
        if (auto elaboratedType = dyn_cast<ElaboratedType>(pointeeType))
            pointeeType = elaboratedType->desugar();
        if (auto recordType = dyn_cast<RecordType>(pointeeType))
            return recordType;
    }

    // extract Foo from Reference<Foo>
    if (auto subst = dyn_cast<SubstTemplateTypeParmType>(cceType))
    {
        if (auto recType = dyn_cast<RecordType>(subst->desugar().getTypePtr()))
        {
            if (auto ctsd = dyn_cast<ClassTemplateSpecializationDecl>(recType->getDecl()))
            {
                auto const& args = ctsd->getTemplateArgs();
                if (args.size() > 0 && args[0].getKind() == TemplateArgument::ArgKind::Type)
                    return dyn_cast_or_null<RecordType>(args[0].getAsType().getTypePtr());
            }
        }
    }

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
    // if there is more than one case, then we have an ambiguous conversion, and we can't change the code
    // to use the upcasting constructor.
    return loplugin::derivedFromCount(subtypeRecord, baseRecord) == 1;
}

loplugin::Plugin::Registration<ReferenceCasting> referencecasting("referencecasting");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
