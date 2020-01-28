/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include "check.hxx"
#include "plugin.hxx"

namespace
{
class StdFunction final : public loplugin::FilteringPlugin<StdFunction>
{
public:
    explicit StdFunction(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool VisitFunctionDecl(FunctionDecl const* decl)
    {
        if (ignoreLocation(decl))
        {
            return true;
        }
        if (!loplugin::ContextCheck(decl->getDeclContext()).StdOrNestedNamespace())
        {
            return true;
        }
        report(DiagnosticsEngine::Warning,
               ("Do not declare a %select{function|function template|member function template"
                " specialization|function template specialization}0 in namespace 'std'"),
               decl->getLocation())
            << decl->getTemplatedKind() << decl->getSourceRange();
        return true;
    }

private:
    void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }
};

static loplugin::Plugin::Registration<StdFunction> X("stdfunction");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
