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
#include "clang/AST/CXXInheritance.h"

/**
 * Check for data member being shadowed.
 *
 * @TODO check for any members in superclass hierarchy with duplicate names,
 *       regardless of their visibility,
 *       more specific names will make the code easier to read
 */
namespace
{

class DataMemberShadow:
    public loplugin::FilteringPlugin<DataMemberShadow>
{
public:
    explicit DataMemberShadow(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    virtual void run() override {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitFieldDecl(FieldDecl const *);
};

bool DataMemberShadow::VisitFieldDecl(FieldDecl const * fieldDecl)
{
    if (ignoreLocation(fieldDecl)) {
        return true;
    }
    StringRef aFileName = getFileNameOfSpellingLoc(
        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(fieldDecl)));

    // FIXME complex stuff to fix later

    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/chart2/source/"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/sfx2/recentdocsview.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/sfx2/templatelocalview.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/store/source/stortree.hxx")
        || loplugin::isSamePathname(aFileName, SRCDIR "/store/source/stordata.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sw/source/uibase/inc/dbtree.hxx"))
        return true;

    const CXXRecordDecl* parentCXXRecordDecl = dyn_cast<CXXRecordDecl>(fieldDecl->getDeclContext());
    if (!parentCXXRecordDecl) {
        return true;
    }

    fieldDecl = fieldDecl->getCanonicalDecl();

    auto BaseMatchesCallback = [&](const CXXBaseSpecifier *cxxBaseSpecifier, CXXBasePath& Paths)
    {
        if (!cxxBaseSpecifier->getType().getTypePtr())
            return false;
        const CXXRecordDecl* baseCXXRecordDecl = cxxBaseSpecifier->getType()->getAsCXXRecordDecl();
        if (!baseCXXRecordDecl)
            return false;
        if (baseCXXRecordDecl->isInvalidDecl())
            return false;
        for (const FieldDecl* baseFieldDecl : baseCXXRecordDecl->fields())
        {
            // TODO look for overlaps even with private fields

            if (baseFieldDecl->getAccess() == AS_private
                || !baseFieldDecl->getDeclName().isIdentifier()
                || fieldDecl->getName() != baseFieldDecl->getName()) {
                continue;
            }
            std::string sPath;
            for (CXXBasePathElement const & pathElement : Paths) {
                if (!sPath.empty()) {
                    sPath += "->";
                }
                sPath += pathElement.Class->getNameAsString();
            }
            sPath += "->";
            sPath += baseCXXRecordDecl->getNameAsString();
            report(DiagnosticsEngine::Warning,
                    "data member %0 is shadowing member in superclass, through inheritance path %1",
                    compat::getBeginLoc(fieldDecl))
                << fieldDecl->getName()
                << sPath
                << fieldDecl->getSourceRange();
            report(DiagnosticsEngine::Note,
                    "superclass member here",
                    compat::getBeginLoc(baseFieldDecl))
                << baseFieldDecl->getSourceRange();
        }
        return false;
    };

    CXXBasePaths aPaths;
    parentCXXRecordDecl->lookupInBases(BaseMatchesCallback, aPaths);
    return true;
}

loplugin::Plugin::Registration< DataMemberShadow > datamembershadow("datamembershadow", true);

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
