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

    bool VisitLambdaExpr(const LambdaExpr * expr);

private:
    bool isFat(QualType type, std::string * name);
};

bool PassStuffByRef::VisitFunctionDecl(const FunctionDecl * functionDecl) {
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    // only warn on the definition/prototype of the function,
    // not on the function implementation
    if ((functionDecl->isThisDeclarationADefinition()
         && functionDecl->getPreviousDecl() != nullptr)
        || functionDecl->isDeleted())
    {
        return true;
    }
    // only consider base declarations, not overriden ones, or we warn on methods that
    // are overriding stuff from external libraries
    if (isa<CXXMethodDecl>(functionDecl)) {
        CXXMethodDecl const * m = dyn_cast<CXXMethodDecl>(functionDecl);
        if (m->size_overridden_methods() > 0)
            return true;
    }
    unsigned n = functionDecl->getNumParams();
    for (unsigned i = 0; i != n; ++i) {
        const ParmVarDecl * pvDecl = functionDecl->getParamDecl(i);
        std::string name;
        if (isFat(pvDecl->getType(), &name)) {
            report(
                DiagnosticsEngine::Warning,
                ("passing '%0' by value, rather pass by reference, e.g., '%0"
                 " const &'"),
                pvDecl->getLocation())
                << name << pvDecl->getSourceRange();
        }
    }
    return true;
}

bool PassStuffByRef::VisitLambdaExpr(const LambdaExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    for (auto i(expr->capture_begin()); i != expr->capture_end(); ++i) {
        if (i->getCaptureKind() == LambdaCaptureKind::LCK_ByCopy) {
            std::string name;
            if (isFat(i->getCapturedVar()->getType(), &name)) {
                report(
                    DiagnosticsEngine::Warning,
                    ("%0 capture of '%1' variable by copy, rather use capture"
                     " by reference---UNLESS THE LAMBDA OUTLIVES THE VARIABLE"),
                    i->getLocation())
                    << (i->isImplicit() ? "implicit" : "explicit") << name
                    << expr->getSourceRange();
            }
        }
    }
    return true;
}

bool PassStuffByRef::isFat(QualType type, std::string * name) {
    if (!type->isRecordType()) {
        return false;
    }
    *name = type.getUnqualifiedType().getCanonicalType().getAsString();
    if (*name == "class rtl::OUString" || *name == "class rtl::OString"
        || name->compare("class com::sun::star::uno::Sequence") == 0)
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

loplugin::Plugin::Registration< PassStuffByRef > X("passstuffbyref");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
