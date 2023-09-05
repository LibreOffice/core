/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <iostream>

#include "check.hxx"
#include "plugin.hxx"
#include "config_clang.h"
#include "clang/AST/CXXInheritance.h"

/**

Look for places where we should be using ImplInheritanceHelper

*/

namespace
{
class ImplInheritanceHelper : public loplugin::FilteringPlugin<ImplInheritanceHelper>
{
public:
    explicit ImplInheritanceHelper(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override { return true; }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXRecordDecl(const CXXRecordDecl*);
};

bool ImplInheritanceHelper::VisitCXXRecordDecl(const CXXRecordDecl* cxxRecordDecl)
{
    if (ignoreLocation(cxxRecordDecl))
        return true;
    if (!cxxRecordDecl->isThisDeclarationADefinition())
        return true;
    if (cxxRecordDecl->isDependentContext())
        return true;

    // ignore the utility template classes
    SourceLocation spellingLocation
        = compiler.getSourceManager().getSpellingLoc(cxxRecordDecl->getBeginLoc());
    StringRef fileName = getFilenameOfLocation(spellingLocation);
    if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/include/cppu"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/comphelper/compbase.hxx"))
        return true;

    // not sure why this extends XPropertyState but does not support it in queryInterface.
    if (loplugin::DeclCheck(cxxRecordDecl)
            .Class("ChainablePropertySet")
            .Namespace("comphelper")
            .GlobalNamespace())
        return true;
    // in these cases the UNO interface is optional
    if (loplugin::DeclCheck(cxxRecordDecl).Class("OFSInputStreamContainer").GlobalNamespace())
        return true;
    if (loplugin::DeclCheck(cxxRecordDecl)
            .Class("OPropertyBrowserController")
            .Namespace("pcr")
            .GlobalNamespace())
        return true;
    if (loplugin::DeclCheck(cxxRecordDecl).Class("SdDrawPage").GlobalNamespace())
        return true;
    if (loplugin::DeclCheck(cxxRecordDecl).Class("SdMasterPage").GlobalNamespace())
        return true;

    // check if this class extends cppu::WeakImplHelper
    if (!loplugin::isDerivedFrom(cxxRecordDecl, [](Decl const* decl) -> bool {
            return bool(loplugin::DeclCheck(decl)
                            .Class("WeakImplHelper")
                            .Namespace("cppu")
                            .GlobalNamespace());
        }))
        return true;
    // check if this class directly inherits from a UNO interface class
    bool foundOne = false;
    for (auto const& i : cxxRecordDecl->bases())
    {
        auto rt = i.getType()->getAs<RecordType>();
        if (!rt)
            continue;
        auto const bd = cast<CXXRecordDecl>(rt->getDecl())->getDefinition();
        auto ctx = bd->getDeclContext();
        if (!ctx->isNamespace())
            break;
        auto ns = dyn_cast<NamespaceDecl>(ctx);
        while (ns)
        {
            if (ns->getIdentifier() && ns->getName() == "star")
            {
                foundOne = true;
                break;
            }
            ns = dyn_cast_or_null<NamespaceDecl>(ns->getParent());
        }
    }
    if (!foundOne)
        return true;
    report(DiagnosticsEngine::Warning, "can probably use ImplInheritanceHelper here",
           cxxRecordDecl->getLocation())
        << cxxRecordDecl->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<ImplInheritanceHelper>
    implinheritancehelper("implinheritancehelper");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
