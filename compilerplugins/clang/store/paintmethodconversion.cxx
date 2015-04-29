/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "clang/Lex/Lexer.h"

#include "compat.hxx"
#include "plugin.hxx"
#include <iostream>
#include <fstream>

/**
 * Rewrites all Paint method on subclasses of vcl::Window to include RenderContext& as parameter.
 *
 * run as: make COMPILER_PLUGIN_TOOL=paintmethodconversion UPDATE_FILES=all FORCE_COMPILE_ALL=1
 */

namespace
{

bool baseCheckNotWindowSubclass(const CXXRecordDecl* aBaseDefinition, void* /*pInput*/)
{
    if (aBaseDefinition && aBaseDefinition->getQualifiedNameAsString() == "vcl::Window")
    {
        return false;
    }
    return true;
}

bool isDerivedFromWindow(const CXXRecordDecl* decl) {
    if (!decl)
        return false;
    // skip vcl::Window
    if (decl->getQualifiedNameAsString() == "vcl::Window")
        return false;
    if (!decl->forallBases(baseCheckNotWindowSubclass, nullptr, true))
        return true;

    return false;
}

class PaintMethodConversion: public RecursiveASTVisitor<PaintMethodConversion>, public loplugin::RewritePlugin
{
public:
    explicit PaintMethodConversion(InstantiationData const& data):
        RewritePlugin(data)
    {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool TraverseCXXMethodDecl(const CXXMethodDecl* methodDeclaration)
    {
        if (!rewriter)
            return true;

        if (methodDeclaration->getNameAsString() != "Paint")
            return true;

        if (!isDerivedFromWindow(methodDeclaration->getParent()))
        {
            return true;
        }

        unsigned int nNoOfParameters = methodDeclaration->getNumParams();

        if (nNoOfParameters == 1) // we expect only one parameter Paint(Rect&)
        {
            const ParmVarDecl* parameterDecl = methodDeclaration->getParamDecl(0);
            if (methodDeclaration->hasBody())
            {
                rewriter->InsertText(parameterDecl->getLocStart(), "vcl::RenderContext& /*rRenderContext*/, ", true, true);
            }
            else
            {
                rewriter->InsertText(parameterDecl->getLocStart(), "vcl::RenderContext& rRenderContext, ", true, true);
            }
        }
        return true;
    }

};

loplugin::Plugin::Registration<PaintMethodConversion> X("paintmethodconversion", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
