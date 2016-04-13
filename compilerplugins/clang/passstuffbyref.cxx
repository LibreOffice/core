/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <set>

#include "plugin.hxx"
#include "compat.hxx"
#include "typecheck.hxx"

// Find places where various things are passed by value.
// It's not very efficient, because we generally end up copying it twice - once into the parameter and
// again into the destination.
// They should rather be passed by reference.
//
// Generally recommending lambda capture by-ref rather than by-copy is even more
// problematic than with function parameters, as a lambda instance can easily
// outlive a referrenced variable.  So once lambdas start to get used in more
// sophisticated ways than passing them into standard algorithms, this plugin's
// advice, at least for explicit captures, will need to be revisited.

namespace {

class PassStuffByRef:
    public RecursiveASTVisitor<PassStuffByRef>, public loplugin::Plugin
{
public:
    explicit PassStuffByRef(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFunctionDecl(const FunctionDecl * decl);
    bool VisitCallExpr(const CallExpr * ) { if (mbInsideFunctionDecl) mbFoundDisqualifier = true; return true; }
    bool VisitMaterializeTemporaryExpr(const MaterializeTemporaryExpr * ) { if (mbInsideFunctionDecl) mbFoundDisqualifier = true; return true; }
    bool VisitDeclStmt(const DeclStmt * ) { if (mbInsideFunctionDecl) mbFoundDisqualifier = true; return true; }
    bool VisitLambdaExpr(const LambdaExpr * expr);

private:
    bool isFat(QualType type);
    bool mbInsideFunctionDecl;
    bool mbFoundDisqualifier;
};

bool PassStuffByRef::VisitFunctionDecl(const FunctionDecl * functionDecl) {
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    if (functionDecl->isDeleted())
        return true;
    // only consider base declarations, not overriden ones, or we warn on methods that
    // are overriding stuff from external libraries
    const CXXMethodDecl * methodDecl = dyn_cast<CXXMethodDecl>(functionDecl);
    if (methodDecl && methodDecl->size_overridden_methods() > 0) {
            return true;
    }
    // only warn on the definition/prototype of the function,
    // not on the function implementation
    if (!functionDecl->isThisDeclarationADefinition())
    {
        unsigned n = functionDecl->getNumParams();
        for (unsigned i = 0; i != n; ++i) {
            const ParmVarDecl * pvDecl = functionDecl->getParamDecl(i);
            auto const t = pvDecl->getType();
            if (isFat(t)) {
                report(
                    DiagnosticsEngine::Warning,
                    ("passing %0 by value, rather pass by const lvalue reference"),
                    pvDecl->getLocation())
                    << t << pvDecl->getSourceRange();
            }
        }
        return true;
    }


    if (methodDecl && methodDecl->isVirtual()) {
        return true;
    }
    if( !functionDecl->hasBody()) {
        return true;
    }

    const QualType type = functionDecl->getReturnType().getDesugaredType(compiler.getASTContext());
    if (type->isReferenceType() || type->isIntegralOrEnumerationType() || type->isPointerType()
        || type->isTemplateTypeParmType() || type->isDependentType() || type->isBuiltinType()
        || type->isScalarType())
    {
        return true;
    }

    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              functionDecl->getCanonicalDecl()->getNameInfo().getLoc()))) {
        return true;
    }
    std::string aFunctionName = functionDecl->getQualifiedNameAsString();
    // function is passed as parameter to another function
    if (aFunctionName == "GDIMetaFile::ImplColMonoFnc"
        || aFunctionName == "editeng::SvxBorderLine::darkColor"
        || aFunctionName.compare(0, 8, "xforms::") == 0)
        return true;
    // not sure how to exclude this yet, returns copy of one of it's params
    if (aFunctionName == "sameDistColor" || aFunctionName == "sameColor"
        || aFunctionName == "pcr::(anonymous namespace)::StringIdentity::operator()"
        || aFunctionName == "matop::COp<type-parameter-0-0, svl::SharedString>::operator()"
        || aFunctionName == "slideshow::internal::accumulate"
        || aFunctionName == "slideshow::internal::lerp")
        return true;
    // depends on a define
    if (aFunctionName == "SfxObjectShell::GetSharedFileURL")
        return true;
    mbInsideFunctionDecl = true;
    mbFoundDisqualifier = false;
    TraverseStmt(functionDecl->getBody());
    mbInsideFunctionDecl = false;

    if (mbFoundDisqualifier)
        return true;

    report(
            DiagnosticsEngine::Warning,
            "rather return %0 from function %1 %2 by const& than by value, to avoid unnecessary copying",
            functionDecl->getSourceRange().getBegin())
        << type.getAsString() << aFunctionName << type->getTypeClassName() << functionDecl->getSourceRange();

    // display the location of the class member declaration so I don't have to search for it by hand
    if (functionDecl->getSourceRange().getBegin() != functionDecl->getCanonicalDecl()->getSourceRange().getBegin())
    {
        report(
                DiagnosticsEngine::Note,
                "rather return by const& than by value",
                functionDecl->getCanonicalDecl()->getSourceRange().getBegin())
            << functionDecl->getCanonicalDecl()->getSourceRange();
    }
    //functionDecl->dump();
    return true;
}

bool PassStuffByRef::VisitLambdaExpr(const LambdaExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    for (auto i(expr->capture_begin()); i != expr->capture_end(); ++i) {
        if (i->getCaptureKind() == LambdaCaptureKind::LCK_ByCopy) {
            auto const t = i->getCapturedVar()->getType();
            if (isFat(t)) {
                report(
                    DiagnosticsEngine::Warning,
                    ("%0 capture of %1 variable by copy, rather use capture"
                     " by reference---UNLESS THE LAMBDA OUTLIVES THE VARIABLE"),
                    i->getLocation())
                    << (i->isImplicit() ? "implicit" : "explicit") << t
                    << expr->getSourceRange();
            }
        }
    }
    return true;
}

bool PassStuffByRef::isFat(QualType type) {
    if (!type->isRecordType()) {
        return false;
    }
    if ((loplugin::TypeCheck(type).Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        || (loplugin::TypeCheck(type).Class("OString").Namespace("rtl")
            .GlobalNamespace())
        || (loplugin::TypeCheck(type).Class("Sequence").Namespace("uno")
            .Namespace("star").Namespace("sun").Namespace("com")
            .GlobalNamespace()))
    {
        return true;
    }
    if (type->isIncompleteType()) {
        return false;
    }
    Type const * t2 = type.getTypePtrOrNull();
    return t2 != nullptr
        && compiler.getASTContext().getTypeSizeInChars(t2).getQuantity() > 64;
}

loplugin::Plugin::Registration< PassStuffByRef > X("passstuffbyref", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
