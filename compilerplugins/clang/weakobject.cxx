
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

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <unordered_set>
#include "plugin.hxx"
#include "check.hxx"

/*
Check for places where we end up with more than one copy of cppu::OweakObject in a class, which
really should not happen - we should be using one of the AggImplInheritanceHelper classes then
to inherit.
*/

namespace
{
class WeakObject : public loplugin::FilteringPlugin<WeakObject>
{
public:
    explicit WeakObject(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override
    {
        return true;
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXRecordDecl( const CXXRecordDecl* decl);

};

bool WeakObject::VisitCXXRecordDecl(const CXXRecordDecl* decl)
{
    if (ignoreLocation(decl))
        return true;
    if (!decl->hasDefinition())
        return true;
    if (decl->hasAnyDependentBases())
        return true;
    int cnt = 0;
    decl->forallBases(
        [&cnt] (const CXXRecordDecl *BaseDefinition) -> bool
        {
            if (loplugin::DeclCheck(BaseDefinition).Class("OWeakObject").Namespace("cppu").GlobalNamespace())
                ++cnt;
            return true;
        });
    if (cnt < 2)
        return true;

    report(DiagnosticsEngine::Warning, "more than one copy of cppu::OWeakObject inherited",
            decl->getBeginLoc())
        << decl->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<WeakObject> weakobject("weakobject", false);

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
