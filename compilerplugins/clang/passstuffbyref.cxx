/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>

#include "plugin.hxx"

// Find places where various things are passed by value.
// It's not very efficient, because we generally end up copying it twice - once into the parameter and
// again into the destination.
// They should rather be passed by reference.

namespace {

class PassStuffByRef:
    public RecursiveASTVisitor<PassStuffByRef>, public loplugin::Plugin
{
public:
    explicit PassStuffByRef(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFunctionDecl(const FunctionDecl * decl);
};

bool PassStuffByRef::VisitFunctionDecl(const FunctionDecl * functionDecl) {
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    // only warn on the definition/prototype of the function,
    // not on the function implementation
    if (functionDecl->isThisDeclarationADefinition() && functionDecl->getPreviousDecl() != nullptr) {
        return true;
    }
    unsigned n = functionDecl->getNumParams();
    for (unsigned i = 0; i != n; ++i) {
        const ParmVarDecl * pvDecl = functionDecl->getParamDecl(i);
        QualType t1 { pvDecl->getType() };
        if (!t1->isClassType()) {
            continue;
        }
        string typeName = t1.getUnqualifiedType().getCanonicalType().getAsString();
        if (typeName == "class rtl::OUString") {
            report(
                DiagnosticsEngine::Warning,
                "passing OUString by value, rather pass by reference .e.g. 'const OUString&'",
                pvDecl->getSourceRange().getBegin())
              << pvDecl->getSourceRange();
        }
        else if (typeName == "class rtl::OString") {
            report(
                DiagnosticsEngine::Warning,
                "passing OString by value, rather pass by reference .e.g. 'const OString&'",
                pvDecl->getSourceRange().getBegin())
              << pvDecl->getSourceRange();
        }
        else if (typeName.find("class com::sun::star::uno::Sequence") == 0) {
            report(
                DiagnosticsEngine::Warning,
                "passing css::uno::Sequence by value, rather pass by reference .e.g. 'const css::uno::Sequence&' " + typeName,
                pvDecl->getSourceRange().getBegin())
              << pvDecl->getSourceRange();
        }
    }
    return true;
}

loplugin::Plugin::Registration< PassStuffByRef > X("passstuffbyref");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
