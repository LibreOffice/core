/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

// Find classes deriving from other classes with virtual member functions annotated with
// SAL_LOPLUGIN_ANNOTATE("mustoverride"), but where the derived class fails to directly override
// them.

#include <set>

#include "plugin.hxx"

namespace
{
void findMustOverride(CXXRecordDecl const* decl, std::set<CXXMethodDecl const*>* mustOverride)
{
    for (auto const& base : decl->bases())
    {
        auto const d = base.getType()->getAsCXXRecordDecl();
        if (d == nullptr)
        {
            continue;
        }
        findMustOverride(d, mustOverride);
        for (auto const meth : d->methods())
        {
            if (!meth->isVirtual())
            {
                continue;
            }
            for (auto const attr : meth->specific_attrs<AnnotateAttr>())
            {
                if (attr->getAnnotation() == "loplugin:mustoverride")
                {
                    mustOverride->insert(meth);
                    break;
                }
            }
        }
    }
}

class MustOverride : public loplugin::FilteringPlugin<MustOverride>
{
public:
    explicit MustOverride(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool VisitCXXRecordDecl(CXXRecordDecl const* decl)
    {
        if (ignoreLocation(decl))
        {
            return true;
        }
        if (!decl->isThisDeclarationADefinition())
        {
            return true;
        }
        if (!decl->isPolymorphic())
        {
            return true;
        }
        std::set<CXXMethodDecl const*> mustOverride;
        findMustOverride(decl, &mustOverride);
        for (auto const meth : mustOverride)
        {
            if (meth->getCorrespondingMethodDeclaredInClass(decl) != nullptr)
            {
                continue;
            }
            report(DiagnosticsEngine::Warning, "%0 does not override %1 from %2",
                   decl->getLocation())
                << decl << meth << meth->getParent() << decl->getSourceRange();
            report(DiagnosticsEngine::Note, "declared here", meth->getLocation())
                << meth->getSourceRange();
        }
        return true;
    }

private:
    void run() override
    {
        if (compiler.getLangOpts().CPlusPlus)
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }
};

loplugin::Plugin::Registration<MustOverride> mustoverride("mustoverride");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
