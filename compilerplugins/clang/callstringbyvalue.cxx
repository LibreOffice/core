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

// Find places where OUString and OString are passed by value.
// It's not very efficient, because we generally end up copying it twice - once into the parameter and
// again into the destination OUString.

namespace {

class CallStringByValue:
    public RecursiveASTVisitor<CallStringByValue>, public loplugin::Plugin
{
public:
    explicit CallStringByValue(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFunctionDecl(const FunctionDecl * decl);
};

bool CallStringByValue::VisitFunctionDecl(const FunctionDecl * functionDecl) {
    if (ignoreLocation(functionDecl)) {
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
                "passing OUString by value, rather pass by reference",
                pvDecl->getSourceRange().getBegin())
              << pvDecl->getSourceRange();
        }
        else if (typeName == "class rtl::OString") {
            report(
                DiagnosticsEngine::Warning,
                "passing OString by value, rather pass by reference",
                pvDecl->getSourceRange().getBegin())
              << pvDecl->getSourceRange();
        }
    }
    return true;
}

loplugin::Plugin::Registration< CallStringByValue > X("callstringbyvalue");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
