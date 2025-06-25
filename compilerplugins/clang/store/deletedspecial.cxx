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
    public loplugin::FilteringPlugin<DeletedSpecial>
{
public:
    explicit DeletedSpecial(InstantiationData const & data): FilteringPlugin(data) {}

    virtual void run() override;

    bool VisitCXXMethodDecl(CXXMethodDecl const * decl);

private:
    bool allowlist(
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
    if (ignoreLocation(decl) || !decl->isFirstDecl() || decl->isDefined()
        || decl->isDefaulted() || decl->getAccess() != AS_private)
    {
        return true;
    }
    std::string desc;
    if (decl->isCopyAssignmentOperator()) {
        if (allowlist(decl, "ImpGraphic", "vcl/inc/impgraph.hxx")
            || allowlist(decl, "SwSubFont", "sw/source/core/inc/swfont.hxx"))
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
            if (allowlist(decl, "ImpGraphic", "vcl/inc/impgraph.hxx")
                || allowlist(decl, "SbMethod", "include/basic/sbmeth.hxx")
                || allowlist(decl, "ScDBCollection::NamedDBs", "sc/inc/dbdata.hxx")
                || allowlist(decl, "ScDrawPage", "sc/inc/drawpage.hxx")
                || allowlist(decl, "SmEditSource", "starmath/source/accessibility.hxx")
                || allowlist(decl, "SwChartDataSequence", "sw/inc/unochart.hxx")
                || allowlist(decl, "SwDPage", "sw/inc/dpage.hxx")
                || allowlist(decl, "SwRedlineExtraData_Format", "sw/inc/redline.hxx")
                || allowlist(decl, "SwRedlineExtraData_FormattingChanges", "sw/inc/redline.hxx")
                || allowlist(decl, "SwTextAPIEditSource", "sw/source/core/inc/textapi.hxx")
                || allowlist(decl, "XclImpBiff5Decrypter", "sc/source/filter/inc/xistream.hxx")
                || allowlist(decl, "XclImpBiff8Decrypter", "sc/source/filter/inc/xistream.hxx")
                || allowlist(decl, "configmgr::LocalizedPropertyNode", "configmgr/source/localizedpropertynode.hxx")
                || allowlist(decl, "configmgr::LocalizedValueNode", "configmgr/source/localizedvaluenode.hxx")
                || allowlist(decl, "configmgr::PropertyNode", "configmgr/source/propertynode.hxx")
                || allowlist(decl, "oox::xls::BiffDecoder_RCF", "sc/source/filter/inc/biffcodec.hxx")
                || allowlist(decl, "oox::xls::BiffDecoder_XOR", "sc/source/filter/inc/biffcodec.hxx")
                || allowlist(decl, "rptui::OReportPage", "reportdesign/inc/RptPage.hxx"))
            {
                return true;
            }
            desc = "copy constructor";
        } else if (ctor != nullptr && ctor->isMoveConstructor()) {
            desc = "move constructor";
        } else if (ctor != nullptr && ctor->isDefaultConstructor()
                   && std::distance(cls->ctor_begin(), cls->ctor_end()) == 1)
        {
            if (allowlist(decl, "AquaA11yFocusListener", "vcl/osx/a11yfocuslistener.hxx")
                || allowlist(decl, "DocTemplLocaleHelper", "sfx2/source/doc/doctemplateslocal.hxx")
                || allowlist(decl, "ScViewDataTable", "sc/source/filter/excel/../../ui/inc/viewdata.hxx")
                || allowlist(decl, "ScViewDataTable", "sc/source/ui/inc/viewdata.hxx")
                || allowlist(decl, "SwLineInfo", "sw/source/core/text/inftxt.hxx")
                || allowlist(decl, "XRenderPeer", "vcl/unx/generic/gdi/xrender_peer.hxx")
                || allowlist(decl, "desktop::DispatchWatcher", "desktop/source/app/dispatchwatcher.hxx")
                || allowlist(decl, "desktop::RequestHandler", "desktop/source/app/officeipcthread.hxx")
                || allowlist(decl, "desktop::RequestHandler", "desktop/source/lib/../app/officeipcthread.hxx")
                || allowlist(decl, "sd::DiscoveryService", "sd/source/ui/remotecontrol/DiscoveryService.hxx")
                || allowlist(decl, "sd::IconCache", "sd/source/ui/inc/tools/IconCache.hxx")
                || allowlist(decl, "sd::RemoteServer", "sd/source/ui/inc/RemoteServer.hxx")
                || allowlist(decl, "sd::slidesorter::cache::PageCacheManager", "sd/source/ui/slidesorter/inc/cache/SlsPageCacheManager.hxx")
                || allowlist(decl, "framework::CommandInfoProvider", "include/framework/commandinfoprovider.hxx")
                || allowlist(decl, "writerfilter::ooxml::OOXMLFactory", "writerfilter/source/ooxml/OOXMLFactory.hxx"))
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

bool DeletedSpecial::allowlist(
    CXXMethodDecl const * decl, std::string const & name,
    std::string const & path)
{
    return getClass(decl)->getQualifiedNameAsString() == name
        && (getFilenameOfLocation(
                compiler.getSourceManager().getSpellingLoc(decl->getLocation()))
            == SRCDIR "/" + path);
}

loplugin::Plugin::Registration<DeletedSpecial> X("deletedspecial", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
