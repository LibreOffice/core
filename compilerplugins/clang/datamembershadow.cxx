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
 * Check for data member being shadowed.
 *
 * @TODO check for any members in superclass hierarchy with duplicate names,
 *       regardless of their visibility,
 *       more specific names will make the code easier to read
 */
namespace
{

class DataMemberShadow:
    public RecursiveASTVisitor<DataMemberShadow>, public loplugin::Plugin
{
public:
    explicit DataMemberShadow(loplugin::InstantiationData const & data):
        Plugin(data) {}

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
    StringRef aFileName = compiler.getSourceManager().getFilename(
            compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocStart()));

    // FIXME complex stuff to fix later

    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/chart2/source/"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/cppcanvas/source/mtfrenderer/emfplus.cxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/cui/source/customize/eventdlg.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/sfx2/recentdocsview.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/sfx2/templatelocalview.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/filter/source/graphicfilter/idxf/dxfentrd.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/framework/source/uielement/popuptoolbarcontroller.cxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/lotuswordpro/inc/xfilter/xfcellstyle.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/lotuswordpro/inc/xfilter/xfdrawobj.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sc/source/ui/vba/vbastyles.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sd/inc/Outliner.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sd/source/ui/annotations/annotationtag.cxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sd/source/ui/inc/FrameView.hxx")
        || loplugin::isSamePathname(aFileName, SRCDIR "/sd/source/filter/ppt/../../ui/inc/FrameView.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sd/source/ui/inc/unopage.hxx"))
        return true;
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sd/source/ui/view/viewoverlaymanager.cxx"))
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
                    fieldDecl->getLocStart())
                << fieldDecl->getName()
                << sPath
                << fieldDecl->getSourceRange();
            report(DiagnosticsEngine::Note,
                    "superclass member here",
                    baseFieldDecl->getLocStart())
                << baseFieldDecl->getSourceRange();
        }
        return false;
    };

    CXXBasePaths aPaths;
    parentCXXRecordDecl->lookupInBases(BaseMatchesCallback, aPaths);
    return true;
}

loplugin::Plugin::Registration< DataMemberShadow > X("datamembershadow", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
