/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <iterator>
#include <string>

#include "compat.hxx"
#include "plugin.hxx"

// Second-guess that certain private special member function declarations for
// which no definition can be found are left undefined to prevent them from
// being implicitly declared.  Such situations are better expressed by marking
// the function as = delete (it e.g. helps compilers determine
// whether class members are unused if all of a class's member definitions are
// seen in a compilation unit).  (Default constructors for classes with multiple
// constructors are exempted as they would not be implicitly declared.
// Destructors are exempted because it is likely that a destructor is defined
// private on purpose.)

namespace {

CXXRecordDecl const * getClass(CXXMethodDecl const * decl) {
    CXXRecordDecl const * cls = dyn_cast<CXXRecordDecl>(decl->getDeclContext());
    assert(cls != nullptr);
    return cls;
}

class DeletedSpecial:
    public RecursiveASTVisitor<DeletedSpecial>, public loplugin::Plugin
{
public:
    explicit DeletedSpecial(InstantiationData const & data): Plugin(data) {}

    virtual void run() override;

    bool VisitCXXMethodDecl(CXXMethodDecl const * decl);

private:
    bool whitelist(
        CXXMethodDecl const * decl, std::string const & name,
        std::string const & path);
};

void DeletedSpecial::run() {
    if (compiler.getLangOpts().CPlusPlus
        && compiler.getPreprocessor().getIdentifierInfo(
            "LIBO_INTERNAL_ONLY")->hasMacroDefinition())
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }
}

bool DeletedSpecial::VisitCXXMethodDecl(CXXMethodDecl const * decl) {
    if (ignoreLocation(decl) || !compat::isFirstDecl(*decl) || decl->isDefined()
        || decl->isDefaulted() || decl->getAccess() != AS_private)
    {
        return true;
    }
    std::string desc;
    if (decl->isCopyAssignmentOperator()) {
        if (whitelist(decl, "ImpGraphic", "vcl/inc/impgraph.hxx")
            || whitelist(decl, "SwSubFont", "sw/source/core/inc/swfont.hxx"))
        {
            return true;
        }
        desc = "copy assignment operator";
    } else if (decl->isMoveAssignmentOperator()) {
        desc = "move assignment operator";
    } else {
        CXXConstructorDecl const * ctor = dyn_cast<CXXConstructorDecl>(decl);
        CXXRecordDecl const * cls = getClass(decl);
        if (ctor != nullptr && ctor->isCopyConstructor()) {
            if (whitelist(decl, "ImpGraphic", "vcl/inc/impgraph.hxx")
                || whitelist(decl, "SbMethod", "include/basic/sbmeth.hxx")
                || whitelist(decl, "ScDBCollection::NamedDBs", "sc/inc/dbdata.hxx")
                || whitelist(decl, "ScDrawPage", "sc/inc/drawpage.hxx")
                || whitelist(decl, "ScFormEditData", "sc/source/ui/inc/formdata.hxx")
                || whitelist(decl, "SmEditSource", "starmath/source/accessibility.hxx")
                || whitelist(decl, "SwChartDataSequence", "sw/inc/unochart.hxx")
                || whitelist(decl, "SwDPage", "sw/inc/dpage.hxx")
                || whitelist(decl, "SwRedlineExtraData_Format", "sw/inc/redline.hxx")
                || whitelist(decl, "SwRedlineExtraData_FormattingChanges", "sw/inc/redline.hxx")
                || whitelist(decl, "SwTextAPIEditSource", "sw/source/core/inc/textapi.hxx")
                || whitelist(decl, "XclImpBiff5Decrypter", "sc/source/filter/inc/xistream.hxx")
                || whitelist(decl, "XclImpBiff8Decrypter", "sc/source/filter/inc/xistream.hxx")
                || whitelist(decl, "configmgr::LocalizedPropertyNode", "configmgr/source/localizedpropertynode.hxx")
                || whitelist(decl, "configmgr::LocalizedValueNode", "configmgr/source/localizedvaluenode.hxx")
                || whitelist(decl, "configmgr::PropertyNode", "configmgr/source/propertynode.hxx")
                || whitelist(decl, "oox::xls::BiffDecoder_RCF", "sc/source/filter/inc/biffcodec.hxx")
                || whitelist(decl, "oox::xls::BiffDecoder_XOR", "sc/source/filter/inc/biffcodec.hxx")
                || whitelist(decl, "rptui::OReportPage", "reportdesign/inc/RptPage.hxx"))
            {
                return true;
            }
            desc = "copy constructor";
        } else if (ctor != nullptr && ctor->isMoveConstructor()) {
            desc = "move constructor";
        } else if (ctor != nullptr && ctor->isDefaultConstructor()
                   && std::distance(cls->ctor_begin(), cls->ctor_end()) == 1)
        {
            if (whitelist(decl, "AquaA11yFocusListener", "vcl/osx/a11yfocuslistener.hxx")
                || whitelist(decl, "DocTemplLocaleHelper", "sfx2/source/doc/doctemplateslocal.hxx")
                || whitelist(decl, "ScViewDataTable", "sc/source/filter/excel/../../ui/inc/viewdata.hxx")
                || whitelist(decl, "ScViewDataTable", "sc/source/ui/inc/viewdata.hxx")
                || whitelist(decl, "SwLineInfo", "sw/source/core/text/inftxt.hxx")
                || whitelist(decl, "XRenderPeer", "vcl/unx/generic/gdi/xrender_peer.hxx")
                || whitelist(decl, "desktop::DispatchWatcher", "desktop/source/app/dispatchwatcher.hxx")
                || whitelist(decl, "desktop::OfficeIPCThread", "desktop/source/app/officeipcthread.hxx")
                || whitelist(decl, "desktop::OfficeIPCThread", "desktop/source/lib/../app/officeipcthread.hxx")
                || whitelist(decl, "sd::DiscoveryService", "sd/source/ui/remotecontrol/DiscoveryService.hxx")
                || whitelist(decl, "sd::IconCache", "sd/source/ui/inc/tools/IconCache.hxx")
                || whitelist(decl, "sd::RemoteServer", "sd/source/ui/inc/RemoteServer.hxx")
                || whitelist(decl, "sd::slidesorter::cache::PageCacheManager", "sd/source/ui/slidesorter/inc/cache/SlsPageCacheManager.hxx")
                || whitelist(decl, "framework::CommandInfoProvider", "include/framework/commandinfoprovider.hxx")
                || whitelist(decl, "vcl::SettingsConfigItem", "include/vcl/configsettings.hxx")
                || whitelist(decl, "writerfilter::ooxml::OOXMLFactory", "writerfilter/source/ooxml/OOXMLFactory.hxx"))
            {
                return true;
            }
            desc = "default constructor";
        } else {
            return true;
        }
    }
    report(
        DiagnosticsEngine::Warning,
        ("private %0 is not defined at least in this compilation unit, maybe it"
         " should be marked as deleted?"),
        decl->getLocation())
        << desc << decl->getSourceRange();
    return true;
}

bool DeletedSpecial::whitelist(
    CXXMethodDecl const * decl, std::string const & name,
    std::string const & path)
{
    return getClass(decl)->getQualifiedNameAsString() == name
        && (compiler.getSourceManager().getFilename(
                compiler.getSourceManager().getSpellingLoc(decl->getLocation()))
            == SRCDIR "/" + path);
}

loplugin::Plugin::Registration<DeletedSpecial> X("deletedspecial", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
