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

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

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
    explicit PassStuffByRef(InstantiationData const & data): Plugin(data), mbInsideFunctionDecl(false), mbFoundDisqualifier(false) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFunctionDecl(const FunctionDecl * decl);
    bool VisitCallExpr(const CallExpr * ) { if (mbInsideFunctionDecl) mbFoundDisqualifier = true; return true; }
    bool VisitMaterializeTemporaryExpr(const MaterializeTemporaryExpr * ) { if (mbInsideFunctionDecl) mbFoundDisqualifier = true; return true; }
    bool VisitDeclStmt(const DeclStmt * ) { if (mbInsideFunctionDecl) mbFoundDisqualifier = true; return true; }
    bool VisitLambdaExpr(const LambdaExpr * expr);

private:
    void checkParams(const FunctionDecl * functionDecl);
    void checkReturnValue(const FunctionDecl * functionDecl, const CXXMethodDecl * methodDecl);
    bool isFat(QualType type);
    bool isPrimitiveConstRef(QualType type);
    bool mbInsideFunctionDecl;
    bool mbFoundDisqualifier;
};

bool startswith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

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

    checkParams(functionDecl);
    checkReturnValue(functionDecl, methodDecl);
    return true;
}

void PassStuffByRef::checkParams(const FunctionDecl * functionDecl) {
    // only warn on the definition/prototype of the function,
    // not on the function implementation
    if (functionDecl->isThisDeclarationADefinition()) {
        return;
    }
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
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              functionDecl->getCanonicalDecl()->getNameInfo().getLoc()))) {
        return;
    }
    // these functions are passed as parameters to another function
    std::string aFunctionName = functionDecl->getQualifiedNameAsString();
    if (startswith(aFunctionName, "slideshow::internal::ShapeAttributeLayer")) {
        return;
    }
    for (unsigned i = 0; i != n; ++i) {
        const ParmVarDecl * pvDecl = functionDecl->getParamDecl(i);
        auto const t = pvDecl->getType();
        if (isPrimitiveConstRef(t)) {
            report(
                DiagnosticsEngine::Warning,
                ("passing primitive type param %0 by const &, rather pass by value"),
                pvDecl->getLocation())
                << t << pvDecl->getSourceRange();
        }
    }
}

void PassStuffByRef::checkReturnValue(const FunctionDecl * functionDecl, const CXXMethodDecl * methodDecl) {

    if (methodDecl && methodDecl->isVirtual()) {
        return;
    }
    if( !functionDecl->hasBody()) {
        return;
    }

    const QualType type = compat::getReturnType(*functionDecl).getDesugaredType(compiler.getASTContext());
    if (type->isReferenceType() || type->isIntegralOrEnumerationType() || type->isPointerType()
        || type->isTemplateTypeParmType() || type->isDependentType() || type->isBuiltinType()
        || type->isScalarType())
    {
        return;
    }

    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              functionDecl->getCanonicalDecl()->getNameInfo().getLoc()))) {
        return;
    }
    loplugin::DeclCheck dc(functionDecl);
    std::string aFunctionName = functionDecl->getQualifiedNameAsString();
    // function is passed as parameter to another function
    if (dc.Function("ImplColMonoFnc").Class("GDIMetaFile").GlobalNamespace()
        || (dc.Function("darkColor").Class("SvxBorderLine").Namespace("editeng")
            .GlobalNamespace())
        || aFunctionName.compare(0, 8, "xforms::") == 0)
        return;
    // not sure how to exclude this yet, returns copy of one of it's params
    if (dc.Function("sameDistColor").GlobalNamespace()
        || dc.Function("sameColor").GlobalNamespace()
        || (dc.Operator(OO_Call).Struct("StringIdentity").AnonymousNamespace()
            .Namespace("pcr").GlobalNamespace())
        || aFunctionName == "matop::COp<type-parameter-0-0, svl::SharedString>::operator()"
        || (dc.Function("accumulate").Namespace("internal")
            .Namespace("slideshow").GlobalNamespace())
        || (dc.Function("lerp").Namespace("internal").Namespace("slideshow")
            .GlobalNamespace()))
        return;
    // depends on a define
    if (dc.Function("GetSharedFileURL").Class("SfxObjectShell")
        .GlobalNamespace())
    {
        return;
    }
    mbInsideFunctionDecl = true;
    mbFoundDisqualifier = false;
    TraverseStmt(functionDecl->getBody());
    mbInsideFunctionDecl = false;

    if (mbFoundDisqualifier)
        return;

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

bool PassStuffByRef::isPrimitiveConstRef(QualType type) {
    if (type->isIncompleteType()) {
        return false;
    }
    if (!type->isReferenceType()) {
        return false;
    }
    const clang::ReferenceType* referenceType = dyn_cast<ReferenceType>(type);
    QualType pointeeQT = referenceType->getPointeeType();
    if (!pointeeQT.isConstQualified()) {
        return false;
    }
    if (!pointeeQT->isFundamentalType()) {
        return false;
    }
    // ignore double for now, some of our code seems to believe it is cheaper to pass by ref
    const BuiltinType* builtinType = dyn_cast<BuiltinType>(pointeeQT);
    return builtinType->getKind() != BuiltinType::Kind::Double;
}


loplugin::Plugin::Registration< PassStuffByRef > X("passstuffbyref");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
