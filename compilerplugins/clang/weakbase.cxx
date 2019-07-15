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
#include <map>
#include <set>

#include "plugin.hxx"
#include "clang/AST/CXXInheritance.h"

/**
 * Check for multiple copies of WeakBase in base classes
 */
namespace
{
class WeakBase : public loplugin::FilteringPlugin<WeakBase>
{
public:
    explicit WeakBase(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXRecordDecl(CXXRecordDecl const*);
};

bool WeakBase::VisitCXXRecordDecl(CXXRecordDecl const* recordDecl)
{
    if (ignoreLocation(recordDecl))
    {
        return true;
    }
    //    StringRef aFileName = getFileNameOfSpellingLoc(
    //        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(fieldDecl)));

    //    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/chart2/source/"))
    //        return true;
    //    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/sfx2/recentdocsview.hxx"))
    //        return true;
    //    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/sfx2/templatelocalview.hxx"))
    //        return true;
    //    if (loplugin::isSamePathname(aFileName, SRCDIR "/store/source/stortree.hxx")
    //        || loplugin::isSamePathname(aFileName, SRCDIR "/store/source/stordata.hxx"))
    //        return true;
    //    if (loplugin::isSamePathname(aFileName, SRCDIR "/sw/source/uibase/inc/dbtree.hxx"))
    //        return true;

    recordDecl = recordDecl->getCanonicalDecl();
    if (!recordDecl->hasDefinition())
        return true;

    int noWeakBases = 0;
    std::string basePaths;
    auto BaseMatchesCallback = [&](const CXXBaseSpecifier* cxxBaseSpecifier, CXXBasePath& Paths) {
        if (!cxxBaseSpecifier->getType().getTypePtr())
            return false;
        const CXXRecordDecl* baseCXXRecordDecl = cxxBaseSpecifier->getType()->getAsCXXRecordDecl();
        if (!baseCXXRecordDecl)
            return false;
        if (baseCXXRecordDecl->isInvalidDecl())
            return false;
        if (baseCXXRecordDecl->getName() != "WeakBase")
            return false;
        ++noWeakBases;
        std::string sPath;
        for (CXXBasePathElement const& pathElement : Paths)
        {
            if (!sPath.empty())
            {
                sPath += "->";
            }
            if (pathElement.Class->hasDefinition())
                sPath += pathElement.Class->getNameAsString();
            else
                sPath += "???";
        }
        sPath += "->";
        sPath += baseCXXRecordDecl->getNameAsString();
        if (!basePaths.empty())
            basePaths += ", ";
        basePaths += sPath;
        return false;
    };

    CXXBasePaths aPaths;
    recordDecl->lookupInBases(BaseMatchesCallback, aPaths);

    if (noWeakBases > 1)
    {
        report(DiagnosticsEngine::Warning,
               "multiple copies of WeakBase, through inheritance paths %0",
               compat::getBeginLoc(recordDecl))
            << basePaths << recordDecl->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<WeakBase> WeakBase("weakbase", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
