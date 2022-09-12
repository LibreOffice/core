/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include <string>
#include <iostream>
#include <map>
#include <set>

#include "plugin.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"

/**
 * Make sure a class does not have multiple reference-counting base classes
 */
namespace
{
class RefCountingBase : public loplugin::FilteringPlugin<RefCountingBase>
{
public:
    explicit RefCountingBase(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitCXXRecordDecl(CXXRecordDecl const*);
};

bool RefCountingBase::VisitCXXRecordDecl(CXXRecordDecl const* recordDecl)
{
    if (ignoreLocation(recordDecl))
        return true;
    if (!recordDecl->isThisDeclarationADefinition())
        return true;

    int virtualWeakBase = 0;
    int virtualOWeakObject = 0;
    int virtualSimpleReferenceObject = 0;
    int virtualSvRefBase = 0;
    int virtualXmlImportContenxt = 0;
    int virtualVclReferenceBase = 0;
    int noRefCountingBases = 0;
    std::string basePaths;
    auto BaseMatchesCallback = [&](const CXXBaseSpecifier* cxxBaseSpecifier, CXXBasePath& Paths) {
        if (!cxxBaseSpecifier->getType().getTypePtr())
            return false;
        const CXXRecordDecl* baseCXXRecordDecl = cxxBaseSpecifier->getType()->getAsCXXRecordDecl();
        if (!baseCXXRecordDecl)
            return false;
        if (baseCXXRecordDecl->isInvalidDecl())
            return false;

        if (baseCXXRecordDecl->getName() != "WeakBase" // tools::WeakBase
            && baseCXXRecordDecl->getName() != "OWeakObject" // cppu::WeakBase
            && baseCXXRecordDecl->getName()
                   != "SimpleReferenceObject" // salhelper::SimpleReferenceObject
            && baseCXXRecordDecl->getName() != "SvRefBase" // tool::SvRefBase
            && baseCXXRecordDecl->getName() != "SvXMLImportContext" // in xmloff
            && baseCXXRecordDecl->getName() != "VclReferenceBase") // in vcl
            return false;
        if (cxxBaseSpecifier->isVirtual())
        {
            if (baseCXXRecordDecl->getName() == "WeakBase")
                virtualWeakBase = 1;
            else if (baseCXXRecordDecl->getName() != "OWeakObject")
                virtualOWeakObject = 1;
            else if (baseCXXRecordDecl->getName() != "SimpleReferenceObject")
                virtualSimpleReferenceObject = 1;
            else if (baseCXXRecordDecl->getName() != "SvRefBase")
                virtualSvRefBase = 1;
            else if (baseCXXRecordDecl->getName() != "SvXMLImportContext")
                virtualXmlImportContenxt = 1;
            else if (baseCXXRecordDecl->getName() != "VclReferenceBase")
                virtualVclReferenceBase = 1;
            else
                assert(false);
        }
        else
            ++noRefCountingBases;
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

    int total = virtualWeakBase + virtualOWeakObject + virtualSimpleReferenceObject
                + virtualSvRefBase + virtualXmlImportContenxt + virtualVclReferenceBase
                + noRefCountingBases;
    if (total > 1)
    {
        report(DiagnosticsEngine::Warning,
               "this class has multiple copies of a reference-counting base class, through "
               "inheritance paths %0",
               recordDecl->getBeginLoc())
            << basePaths << recordDecl->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<RefCountingBase> refcountingbase("refcountingbase", true);

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
