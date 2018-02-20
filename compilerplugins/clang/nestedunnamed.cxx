/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>

#include "plugin.hxx"

// Warn about unnamed namespaces nested (directly) within unnamed namespaces.  (It can be hard to
// keep track whether a certain spot in a source file is already in an unnamed namespace, so it
// happens that additions to the source add redundant, nested unnamed namespaces.)

namespace
{
class NestedUnnamed : public RecursiveASTVisitor<NestedUnnamed>, public loplugin::Plugin
{
public:
    explicit NestedUnnamed(loplugin::InstantiationData const& data)
        : Plugin(data)
    {
    }

    void run() override
    {
        if (compiler.getLangOpts().CPlusPlus)
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitNamespaceDecl(NamespaceDecl const* decl)
    {
        if (ignoreLocation(decl))
        {
            return true;
        }
        if (!decl->isAnonymousNamespace())
        {
            return true;
        }
        NamespaceDecl const* outer;
        for (auto p = decl->getLexicalParent();; p = p->getLexicalParent())
        {
            outer = dyn_cast<NamespaceDecl>(p);
            if (outer != nullptr)
            {
                break;
            }
            if (isa<TranslationUnitDecl>(p))
            {
                return true;
            }
        }
        if (!outer->isAnonymousNamespace())
        {
            return true;
        }
        report(DiagnosticsEngine::Warning, "unnamed namespace directly nested in unnamed namespace",
               decl->getLocation())
            << decl->getSourceRange();
        report(DiagnosticsEngine::Note, "outer namespace declared here", outer->getLocation())
            << outer->getSourceRange();
        return true;
    }
};

loplugin::Plugin::Registration<NestedUnnamed> X("nestedunnamed");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
