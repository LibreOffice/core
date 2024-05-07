/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <sal/config.h>

#include <utility>
#include <vector>


#include <optional>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/deployment/XUpdateInformationProvider.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/xml/dom/XElement.hpp>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <salhelper/thread.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/svapp.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

#include <dp_dependencies.hxx>
#include <dp_descriptioninfoset.hxx>
#include <dp_identifier.hxx>
#include <dp_misc.h>
#include <dp_update.hxx>

#include <strings.hrc>
#include "dp_gui_updatedata.hxx"
#include "dp_gui_updatedialog.hxx"
#include <dp_shared.hxx>

class KeyEvent;
class MouseEvent;
namespace com::sun::star::uno {
    class XComponentContext;
}

using namespace ::com::sun::star;
using dp_gui::UpdateDialog;

namespace {

sal_Unicode const LF = 0x000A;
sal_Unicode const CR = 0x000D;

constexpr OUStringLiteral IGNORED_UPDATES = u"/org.openoffice.Office.ExtensionManager/ExtensionUpdateData/IgnoredUpdates";
constexpr OUStringLiteral PROPERTY_VERSION = u"Version";

enum Kind { ENABLED_UPDATE, DISABLED_UPDATE, SPECIFIC_ERROR };

OUString confineToParagraph(OUString const & text) {
    // Confine arbitrary text to a single paragraph in a VclMultiLineEdit
    // This assumes that U+000A and U+000D are the only paragraph separators in
    // a VclMultiLineEdit, and that replacing them with a single space
    // each is acceptable:
    return text.replace(LF, ' ').replace(CR, ' ');
}
}

struct UpdateDialog::DisabledUpdate {
    OUString                  name;
    uno::Sequence< OUString > unsatisfiedDependencies;
    // We also want to show release notes and publisher for disabled updates
    css::uno::Reference< css::xml::dom::XNode > aUpdateInfo;
};

struct UpdateDialog::SpecificError {
    OUString name;
    OUString message;
};


struct UpdateDialog::IgnoredUpdate {
    OUString sExtensionID;
    OUString sVersion;

    IgnoredUpdate( OUString aExtensionID, OUString aVersion );
};


UpdateDialog::IgnoredUpdate::IgnoredUpdate( OUString aExtensionID, OUString aVersion ):
    sExtensionID(std::move( aExtensionID )),
    sVersion(std::move( aVersion ))
{}


struct UpdateDialog::Index
{
    Kind          m_eKind;
    bool          m_bIgnored;
    sal_uInt16    m_nIndex;
    OUString      m_aName;

    Index( Kind theKind, sal_uInt16 nIndex, OUString aName ) :
        m_eKind( theKind ),
        m_bIgnored( false ),
        m_nIndex( nIndex ),
        m_aName(std::move( aName )) {}
};


class UpdateDialog::Thread: public salhelper::Thread {
public:
    Thread(
        uno::Reference< uno::XComponentContext > const & context,
        UpdateDialog & dialog,
        std::vector< uno::Reference< deployment::XPackage >  > && vExtensionList);

    void stop();

private:
    virtual ~Thread() override;

    virtual void execute() override;

    void handleSpecificError(
        uno::Reference< deployment::XPackage > const & package,
        uno::Any const & exception) const;

    OUString getUpdateDisplayString(
        dp_gui::UpdateData const & data, std::u16string_view version = std::u16string_view()) const;

    void prepareUpdateData(
        css::uno::Reference< css::xml::dom::XNode > const & updateInfo,
        UpdateDialog::DisabledUpdate & out_du,
        dp_gui::UpdateData & out_data) const;

    bool update(
        UpdateDialog::DisabledUpdate const & du,
        dp_gui::UpdateData const & data) const;

    uno::Reference< uno::XComponentContext > m_context;
    UpdateDialog & m_dialog;
    std::vector< uno::Reference< deployment::XPackage > > m_vExtensionList;
    uno::Reference< deployment::XUpdateInformationProvider > m_updateInformation;
    uno::Reference< task::XInteractionHandler > m_xInteractionHdl;

    // guarded by Application::GetSolarMutex():
    bool m_stop;
};

UpdateDialog::Thread::Thread(
    uno::Reference< uno::XComponentContext > const & context,
    UpdateDialog & dialog,
    std::vector< uno::Reference< deployment::XPackage > >&& vExtensionList):
    salhelper::Thread("dp_gui_updatedialog"),
    m_context(context),
    m_dialog(dialog),
    m_vExtensionList(std::move(vExtensionList)),
    m_updateInformation(
        deployment::UpdateInformationProvider::create(context)),
    m_stop(false)
{
    if( m_context.is() )
    {
        m_xInteractionHdl =
            task::InteractionHandler::createWithParent(m_context, dialog.getDialog()->GetXWindow());
        m_updateInformation->setInteractionHandler( m_xInteractionHdl );
    }
}

void UpdateDialog::Thread::stop() {
    {
        SolarMutexGuard g;
        m_stop = true;
    }
    m_updateInformation->cancel();
}

UpdateDialog::Thread::~Thread()
{
    if ( m_xInteractionHdl.is() )
        m_updateInformation->setInteractionHandler( uno::Reference< task::XInteractionHandler > () );
}

void UpdateDialog::Thread::execute()
{
    {
        SolarMutexGuard g;
        if ( m_stop ) {
            return;
        }
    }
    uno::Reference<deployment::XExtensionManager> extMgr =
        deployment::ExtensionManager::get(m_context);

    std::vector<std::pair<uno::Reference<deployment::XPackage>, uno::Any > > errors;

    dp_misc::UpdateInfoMap updateInfoMap = dp_misc::getOnlineUpdateInfos(
        m_context, extMgr, m_updateInformation, &m_vExtensionList, errors);

    for (auto const& elem : errors)
        handleSpecificError(elem.first, elem.second);

    for (auto const& updateInfo : updateInfoMap)
    {
        dp_misc::UpdateInfo const & info = updateInfo.second;
        UpdateData updateData(info.extension);
        DisabledUpdate disableUpdate;
        //determine if online updates meet the requirements
        prepareUpdateData(info.info, disableUpdate, updateData);

        //determine if the update is installed in the user or shared repository
        OUString sOnlineVersion;
        if (info.info.is())
            sOnlineVersion = info.version;
        OUString sVersionUser;
        OUString sVersionShared;
        OUString sVersionBundled;
        uno::Sequence< uno::Reference< deployment::XPackage> > extensions;
        try {
            extensions = extMgr->getExtensionsWithSameIdentifier(
                dp_misc::getIdentifier(info.extension), info.extension->getName(),
                uno::Reference<ucb::XCommandEnvironment>());
        } catch ( const lang::IllegalArgumentException& ) {
            OSL_ASSERT(false);
            continue;
        } catch ( const css::ucb::CommandFailedException& ) {
            OSL_ASSERT(false);
            continue;
        }
        OSL_ASSERT(extensions.getLength() == 3);
        if (extensions[0].is() )
            sVersionUser = extensions[0]->getVersion();
        if (extensions[1].is() )
            sVersionShared = extensions[1]->getVersion();
        if (extensions[2].is() )
            sVersionBundled = extensions[2]->getVersion();

        bool bSharedReadOnly = extMgr->isReadOnlyRepository(u"shared"_ustr);

        dp_misc::UPDATE_SOURCE sourceUser = dp_misc::isUpdateUserExtension(
            bSharedReadOnly, sVersionUser, sVersionShared, sVersionBundled, sOnlineVersion);
        dp_misc::UPDATE_SOURCE sourceShared = dp_misc::isUpdateSharedExtension(
            bSharedReadOnly, sVersionShared, sVersionBundled, sOnlineVersion);

        if (sourceUser != dp_misc::UPDATE_SOURCE_NONE)
        {
            if (sourceUser == dp_misc::UPDATE_SOURCE_SHARED)
            {
                updateData.aUpdateSource = extensions[1];
                updateData.updateVersion = extensions[1]->getVersion();
            }
            else if (sourceUser == dp_misc::UPDATE_SOURCE_BUNDLED)
            {
                updateData.aUpdateSource = extensions[2];
                updateData.updateVersion = extensions[2]->getVersion();
            }
            if (!update(disableUpdate, updateData))
                return;
        }

        if (sourceShared != dp_misc::UPDATE_SOURCE_NONE)
        {
            if (sourceShared == dp_misc::UPDATE_SOURCE_BUNDLED)
            {
                updateData.aUpdateSource = extensions[2];
                updateData.updateVersion = extensions[2]->getVersion();
            }
            updateData.bIsShared = true;
            if (!update(disableUpdate, updateData))
                return;
        }
    }


    SolarMutexGuard g;
    if (!m_stop) {
        m_dialog.checkingDone();
    }
}

//Parameter package can be null
void UpdateDialog::Thread::handleSpecificError(
    uno::Reference< deployment::XPackage > const & package,
    uno::Any const & exception) const
{
    UpdateDialog::SpecificError data;
    if (package.is())
        data.name = package->getDisplayName();
    uno::Exception e;
    if (exception >>= e) {
        data.message = e.Message;
    }
    SolarMutexGuard g;
    if (!m_stop) {
        m_dialog.addSpecificError(data);
    }
}

OUString UpdateDialog::Thread::getUpdateDisplayString(
    dp_gui::UpdateData const & data, std::u16string_view version) const
{
    OSL_ASSERT(data.aInstalledPackage.is());
    OUStringBuffer b(data.aInstalledPackage->getDisplayName());
    b.append(' ');
    {
        SolarMutexGuard g;
        if(!m_stop)
            b.append(m_dialog.m_version);
    }
    b.append(' ');
    if (!version.empty())
        b.append(version);
    else
        b.append(data.updateVersion);

    if (!data.sWebsiteURL.isEmpty())
    {
        b.append(' ');
        {
            SolarMutexGuard g;
            if(!m_stop)
                b.append(m_dialog.m_browserbased);
        }
    }
    return  b.makeStringAndClear();
}

/** out_data will only be filled if all dependencies are ok.
 */
void UpdateDialog::Thread::prepareUpdateData(
    uno::Reference< xml::dom::XNode > const & updateInfo,
    UpdateDialog::DisabledUpdate & out_du,
    dp_gui::UpdateData & out_data) const
{
    if (!updateInfo.is())
        return;
    dp_misc::DescriptionInfoset infoset(m_context, updateInfo);
    OSL_ASSERT(!infoset.getVersion().isEmpty());
    uno::Sequence< uno::Reference< xml::dom::XElement > > ds(
        dp_misc::Dependencies::check(infoset));

    out_du.aUpdateInfo = updateInfo;
    out_du.unsatisfiedDependencies.realloc(ds.getLength());
    auto p_unsatisfiedDependencies = out_du.unsatisfiedDependencies.getArray();
    for (sal_Int32 i = 0; i < ds.getLength(); ++i) {
        p_unsatisfiedDependencies[i] = dp_misc::Dependencies::getErrorText(ds[i]);
    }

    const ::std::optional< OUString> updateWebsiteURL(infoset.getLocalizedUpdateWebsiteURL());

    out_du.name = getUpdateDisplayString(out_data, infoset.getVersion());

    if (!out_du.unsatisfiedDependencies.hasElements())
    {
        out_data.aUpdateInfo = updateInfo;
        out_data.updateVersion = infoset.getVersion();
        if (updateWebsiteURL)
            out_data.sWebsiteURL = *updateWebsiteURL;
    }
}

bool UpdateDialog::Thread::update(
    UpdateDialog::DisabledUpdate const & du,
    dp_gui::UpdateData const & data) const
{
    bool ret = false;
    if (!du.unsatisfiedDependencies.hasElements())
    {
        SolarMutexGuard g;
        if (!m_stop) {
            m_dialog.addEnabledUpdate(getUpdateDisplayString(data), data);
        }
        ret = !m_stop;
    } else {
        SolarMutexGuard g;
        if (!m_stop) {
                m_dialog.addDisabledUpdate(du);
        }
        ret = !m_stop;
    }
    return ret;
}

// UpdateDialog ----------------------------------------------------------
UpdateDialog::UpdateDialog(
    uno::Reference< uno::XComponentContext > const & context,
    weld::Window * parent, std::vector<uno::Reference< deployment::XPackage > > && vExtensionList,
    std::vector< dp_gui::UpdateData > * updateData)
    : GenericDialogController(parent, u"desktop/ui/updatedialog.ui"_ustr, u"UpdateDialog"_ustr)
    , m_context(context)
    , m_none(DpResId(RID_DLG_UPDATE_NONE))
    , m_noInstallable(DpResId(RID_DLG_UPDATE_NOINSTALLABLE))
    , m_failure(DpResId(RID_DLG_UPDATE_FAILURE))
    , m_unknownError(DpResId(RID_DLG_UPDATE_UNKNOWNERROR))
    , m_noDescription(DpResId(RID_DLG_UPDATE_NODESCRIPTION))
    , m_noInstall(DpResId(RID_DLG_UPDATE_NOINSTALL))
    , m_noDependency(DpResId(RID_DLG_UPDATE_NODEPENDENCY))
    , m_noDependencyCurVer(DpResId(RID_DLG_UPDATE_NODEPENDENCY_CUR_VER))
    , m_browserbased(DpResId(RID_DLG_UPDATE_BROWSERBASED))
    , m_version(DpResId(RID_DLG_UPDATE_VERSION))
    , m_ignoredUpdate(DpResId(RID_DLG_UPDATE_IGNORED_UPDATE))
    , m_updateData(*updateData)
    , m_thread(new UpdateDialog::Thread(context, *this, std::move(vExtensionList)))
    , m_xChecking(m_xBuilder->weld_label(u"UPDATE_CHECKING"_ustr))
    , m_xThrobber(m_xBuilder->weld_spinner(u"THROBBER"_ustr))
    , m_xUpdate(m_xBuilder->weld_label(u"UPDATE_LABEL"_ustr))
    , m_xUpdates(m_xBuilder->weld_tree_view(u"checklist"_ustr))
    , m_xAll(m_xBuilder->weld_check_button(u"UPDATE_ALL"_ustr))
    , m_xDescription(m_xBuilder->weld_label(u"DESCRIPTION_LABEL"_ustr))
    , m_xPublisherLabel(m_xBuilder->weld_label(u"PUBLISHER_LABEL"_ustr))
    , m_xPublisherLink(m_xBuilder->weld_link_button(u"PUBLISHER_LINK"_ustr))
    , m_xReleaseNotesLabel(m_xBuilder->weld_label(u"RELEASE_NOTES_LABEL"_ustr))
    , m_xReleaseNotesLink(m_xBuilder->weld_link_button(u"RELEASE_NOTES_LINK"_ustr))
    , m_xDescriptions(m_xBuilder->weld_text_view(u"DESCRIPTIONS"_ustr))
    , m_xOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xClose(m_xBuilder->weld_button(u"close"_ustr))
    , m_xHelp(m_xBuilder->weld_button(u"help"_ustr))
{
    auto nWidth = m_xDescriptions->get_approximate_digit_width() * 62;
    auto nHeight = m_xDescriptions->get_height_rows(8);
    m_xDescriptions->set_size_request(nWidth, nHeight);
    m_xUpdates->set_size_request(nWidth, nHeight);

    m_xUpdates->enable_toggle_buttons(weld::ColumnToggleType::Check);

    OSL_ASSERT(updateData != nullptr);

    m_xExtensionManager = deployment::ExtensionManager::get( context );

    m_xUpdates->connect_changed(LINK(this, UpdateDialog, selectionHandler));
    m_xUpdates->connect_toggled(LINK(this, UpdateDialog, entryToggled));
    m_xAll->connect_toggled(LINK(this, UpdateDialog, allHandler));
    m_xOk->connect_clicked(LINK(this, UpdateDialog, okHandler));
    m_xClose->connect_clicked(LINK(this, UpdateDialog, closeHandler));
    if (!dp_misc::office_is_running())
        m_xHelp->set_sensitive(false);

    initDescription();
    getIgnoredUpdates();
}

UpdateDialog::~UpdateDialog()
{
}

short UpdateDialog::run() {
    m_xThrobber->start();
    m_thread->launch();
    short nRet = GenericDialogController::run();
    m_thread->stop();
    return nRet;
}

IMPL_LINK(UpdateDialog, entryToggled, const weld::TreeView::iter_col&, rRowCol, void)
{
    // error's can't be enabled
    const UpdateDialog::Index* p = weld::fromId<UpdateDialog::Index const *>(m_xUpdates->get_id(rRowCol.first));
    if (p->m_eKind == SPECIFIC_ERROR)
        m_xUpdates->set_toggle(rRowCol.first, TRISTATE_FALSE);

    enableOk();
}

void UpdateDialog::insertItem(const UpdateDialog::Index *pEntry, bool bEnabledCheckBox)
{
    int nEntry = m_xUpdates->n_children();
    m_xUpdates->append();
    m_xUpdates->set_toggle(nEntry, bEnabledCheckBox ? TRISTATE_TRUE : TRISTATE_FALSE);
    m_xUpdates->set_text(nEntry, pEntry->m_aName, 0);
    m_xUpdates->set_id(nEntry, weld::toId(pEntry));
}

void UpdateDialog::addAdditional(const UpdateDialog::Index * index, bool bEnabledCheckBox)
{
    m_xAll->set_sensitive(true);
    if (m_xAll->get_active())
    {
        insertItem(index, bEnabledCheckBox);
        m_xUpdate->set_sensitive(true);
        m_xUpdates->set_sensitive(true);
        m_xDescription->set_sensitive(true);
        m_xDescriptions->set_sensitive(true);
    }
}

void UpdateDialog::addEnabledUpdate( OUString const & name,
                                     dp_gui::UpdateData const & data )
{
    sal_uInt16 nIndex = sal::static_int_cast< sal_uInt16 >( m_enabledUpdates.size() );
    UpdateDialog::Index *pEntry = new UpdateDialog::Index( ENABLED_UPDATE, nIndex, name );

    m_enabledUpdates.push_back( data );
    m_ListboxEntries.emplace_back( pEntry );

    if (!isIgnoredUpdate(pEntry))
    {
        insertItem(pEntry, true);
    }
    else
        addAdditional(pEntry, false);

    m_xUpdate->set_sensitive(true);
    m_xUpdates->set_sensitive(true);
    m_xDescription->set_sensitive(true);
    m_xDescriptions->set_sensitive(true);
}

void UpdateDialog::addDisabledUpdate( UpdateDialog::DisabledUpdate const & data )
{
    sal_uInt16 nIndex = sal::static_int_cast< sal_uInt16 >( m_disabledUpdates.size() );
    UpdateDialog::Index *pEntry = new UpdateDialog::Index( DISABLED_UPDATE, nIndex, data.name );

    m_disabledUpdates.push_back( data );
    m_ListboxEntries.emplace_back( pEntry );

    isIgnoredUpdate( pEntry );
    addAdditional(pEntry, false);
}

void UpdateDialog::addSpecificError( UpdateDialog::SpecificError const & data )
{
    sal_uInt16 nIndex = sal::static_int_cast< sal_uInt16 >( m_specificErrors.size() );
    UpdateDialog::Index *pEntry = new UpdateDialog::Index( SPECIFIC_ERROR, nIndex, data.name );

    m_specificErrors.push_back( data );
    m_ListboxEntries.emplace_back( pEntry );

    addAdditional(pEntry, false);
}

void UpdateDialog::checkingDone() {
    m_xChecking->hide();
    m_xThrobber->stop();
    m_xThrobber->hide();
    if (m_xUpdates->n_children() == 0)
    {
        clearDescription();
        m_xDescription->set_sensitive(true);
        m_xDescriptions->set_sensitive(true);

        if ( m_disabledUpdates.empty() && m_specificErrors.empty() && m_ignoredUpdates.empty() )
            showDescription( m_none );
        else
            showDescription( m_noInstallable );
    }

    enableOk();
}

void UpdateDialog::enableOk() {
    if (!m_xChecking->get_visible()) {
        int nChecked = 0;
        for (int i = 0, nCount = m_xUpdates->n_children(); i < nCount; ++i) {
            if (m_xUpdates->get_toggle(i) == TRISTATE_TRUE)
                ++nChecked;
        }
        m_xOk->set_sensitive(nChecked != 0);
    }
}

// *********************************************************************************
void UpdateDialog::createNotifyJob( bool bPrepareOnly,
    uno::Sequence< uno::Sequence< OUString > > const &rItemList )
{
    if ( !dp_misc::office_is_running() )
        return;

    // notify update check job
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
            configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext()));

        uno::Sequence< uno::Any > aArgumentList{ uno::Any(comphelper::makePropertyValue(
            u"nodepath"_ustr,
            u"org.openoffice.Office.Addons/AddonUI/OfficeHelp/UpdateCheckJob"_ustr)) };

        uno::Reference< container::XNameAccess > xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                u"com.sun.star.configuration.ConfigurationAccess"_ustr, aArgumentList ),
            uno::UNO_QUERY_THROW );

        util::URL aURL;
        xNameAccess->getByName(u"URL"_ustr) >>= aURL.Complete;

        uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        uno::Reference < util::XURLTransformer > xTransformer = util::URLTransformer::create(xContext);

        xTransformer->parseStrict(aURL);

        uno::Reference < frame::XDesktop2 > xDesktop = frame::Desktop::create( xContext );
        uno::Reference< frame::XDispatchProvider > xDispatchProvider( xDesktop->getCurrentFrame(),
            uno::UNO_QUERY_THROW );
        uno::Reference< frame::XDispatch > xDispatch = xDispatchProvider->queryDispatch(aURL, OUString(), 0);

        if( xDispatch.is() )
        {
            uno::Sequence aPropList{ comphelper::makePropertyValue(u"updateList"_ustr, rItemList),
                                     comphelper::makePropertyValue(u"prepareOnly"_ustr, bPrepareOnly) };

            xDispatch->dispatch(aURL, aPropList );
        }
    }
    catch( const uno::Exception& e )
    {
        dp_misc::TRACE( "Caught exception: "
            + e.Message + "\n thread terminated.\n\n");
    }
}

// *********************************************************************************
void UpdateDialog::notifyMenubar( bool bPrepareOnly, bool bRecheckOnly )
{
    if ( !dp_misc::office_is_running() )
        return;

    uno::Sequence< uno::Sequence< OUString > > aItemList;

    if ( ! bRecheckOnly )
    {
        sal_Int32 nCount = 0;
        for (sal_uInt16 i = 0, nItemCount = m_xUpdates->n_children(); i < nItemCount; ++i)
        {

            UpdateDialog::Index const * p = weld::fromId<UpdateDialog::Index const*>(m_xUpdates->get_id(i));

            if ( p->m_eKind == ENABLED_UPDATE )
            {
                dp_gui::UpdateData aUpdData = m_enabledUpdates[ p->m_nIndex ];

                dp_misc::DescriptionInfoset aInfoset( m_context, aUpdData.aUpdateInfo );
                aItemList.realloc(nCount + 1);
                aItemList.getArray()[nCount] =
                {
                    dp_misc::getIdentifier( aUpdData.aInstalledPackage ),
                    aInfoset.getVersion()
                };
                nCount += 1;
            }
            else
                continue;
        }
    }

    createNotifyJob( bPrepareOnly, aItemList );
}

// *********************************************************************************

void UpdateDialog::initDescription()
{
    m_xPublisherLabel->hide();
    m_xPublisherLink->hide();
    m_xReleaseNotesLabel->hide();
    m_xReleaseNotesLink->hide();
}

void UpdateDialog::clearDescription()
{
    m_xPublisherLabel->hide();
    m_xPublisherLink->hide();
    m_xPublisherLink->set_label(u""_ustr);
    m_xPublisherLink->set_uri(u""_ustr);
    m_xReleaseNotesLabel->hide();
    m_xReleaseNotesLink->hide();
    m_xReleaseNotesLink->set_uri( u""_ustr );
    m_xDescriptions->set_text(u""_ustr);
}

bool UpdateDialog::showDescription(uno::Reference< xml::dom::XNode > const & aUpdateInfo)
{
    dp_misc::DescriptionInfoset infoset(m_context, aUpdateInfo);
    return showDescription(infoset.getLocalizedPublisherNameAndURL(),
                           infoset.getLocalizedReleaseNotesURL());
}

bool UpdateDialog::showDescription(uno::Reference< deployment::XPackage > const & aExtension)
{
    OSL_ASSERT(aExtension.is());
    beans::StringPair pubInfo = aExtension->getPublisherInfo();
    return showDescription(std::make_pair(pubInfo.First, pubInfo.Second),
                           u""_ustr);
}

bool UpdateDialog::showDescription(std::pair< OUString, OUString > const & pairPublisher,
                                   OUString const & sReleaseNotes)
{
    OUString sPub = pairPublisher.first;
    OUString sURL = pairPublisher.second;

    if ( sPub.isEmpty() && sURL.isEmpty() && sReleaseNotes.isEmpty() )
        // nothing to show
        return false;

    if ( !sPub.isEmpty() )
    {
        m_xPublisherLabel->show();
        m_xPublisherLink->show();
        m_xPublisherLink->set_label(sPub);
        m_xPublisherLink->set_uri(sURL);
    }

    if ( !sReleaseNotes.isEmpty() )
    {
        m_xReleaseNotesLabel->show();
        m_xReleaseNotesLink->show();
        m_xReleaseNotesLink->set_uri( sReleaseNotes );
    }
    return true;
}

bool UpdateDialog::showDescription( const OUString& rDescription)
{
    if ( rDescription.isEmpty() )
        // nothing to show
        return false;

    m_xDescriptions->set_text(rDescription);
    return true;
}

void UpdateDialog::getIgnoredUpdates()
{
    uno::Reference< lang::XMultiServiceFactory > xConfig(
        configuration::theDefaultProvider::get(m_context));
    beans::NamedValue aValue( u"nodepath"_ustr, uno::Any( OUString(IGNORED_UPDATES) ) );
    uno::Sequence< uno::Any > args{ uno::Any(aValue) };

    uno::Reference< container::XNameAccess > xNameAccess( xConfig->createInstanceWithArguments( u"com.sun.star.configuration.ConfigurationAccess"_ustr, args), uno::UNO_QUERY_THROW );
    const uno::Sequence< OUString > aElementNames = xNameAccess->getElementNames();

    for ( OUString const & aIdentifier : aElementNames )
    {
        OUString aVersion;

        uno::Any aPropValue( uno::Reference< beans::XPropertySet >( xNameAccess->getByName( aIdentifier ), uno::UNO_QUERY_THROW )->getPropertyValue( PROPERTY_VERSION ) );
        aPropValue >>= aVersion;
        IgnoredUpdate *pData = new IgnoredUpdate( aIdentifier, aVersion );
        m_ignoredUpdates.emplace_back( pData );
    }
}


bool UpdateDialog::isIgnoredUpdate( UpdateDialog::Index * index )
{
    bool bIsIgnored = false;

    if (! m_ignoredUpdates.empty() )
    {
        OUString aExtensionID;
        OUString aVersion;

        if ( index->m_eKind == ENABLED_UPDATE )
        {
            dp_gui::UpdateData aUpdData = m_enabledUpdates[ index->m_nIndex ];
            aExtensionID = dp_misc::getIdentifier( aUpdData.aInstalledPackage );
            aVersion = aUpdData.updateVersion;
        }
        else if ( index->m_eKind == DISABLED_UPDATE )
        {
            DisabledUpdate &rData = m_disabledUpdates[ index->m_nIndex ];
            dp_misc::DescriptionInfoset aInfoset( m_context, rData.aUpdateInfo );
            ::std::optional< OUString > aID( aInfoset.getIdentifier() );
            if ( aID )
                aExtensionID = *aID;
            aVersion = aInfoset.getVersion();
        }

        for (auto const& ignoredUpdate : m_ignoredUpdates)
        {
            if ( ignoredUpdate->sExtensionID == aExtensionID )
            {
                if ( ( !ignoredUpdate->sVersion.isEmpty() ) || ( ignoredUpdate->sVersion == aVersion ) )
                {
                    bIsIgnored = true;
                    index->m_bIgnored = true;
                }
                break;
            }
        }
    }

    return bIsIgnored;
}


IMPL_LINK_NOARG(UpdateDialog, selectionHandler, weld::TreeView&, void)
{
    OUStringBuffer b;
    int nSelectedPos = m_xUpdates->get_selected_index();
    clearDescription();

    const UpdateDialog::Index* p = nullptr;
    if (nSelectedPos != -1)
        p = weld::fromId<UpdateDialog::Index const*>(m_xUpdates->get_id(nSelectedPos));
    if (p != nullptr)
    {
        sal_uInt16 pos = p->m_nIndex;

        switch (p->m_eKind)
        {
            case ENABLED_UPDATE:
            {
                if ( m_enabledUpdates[ pos ].aUpdateSource.is() )
                    showDescription( m_enabledUpdates[ pos ].aUpdateSource );
                else
                    showDescription( m_enabledUpdates[ pos ].aUpdateInfo );

                if ( p->m_bIgnored )
                    b.append( m_ignoredUpdate );

                break;
            }
            case DISABLED_UPDATE:
            {
                if ( !m_disabledUpdates.empty() )
                    showDescription( m_disabledUpdates[pos].aUpdateInfo );

                if ( p->m_bIgnored )
                    b.append( m_ignoredUpdate );

                if ( m_disabledUpdates.empty() )
                    break;

                UpdateDialog::DisabledUpdate & data = m_disabledUpdates[ pos ];
                if (data.unsatisfiedDependencies.hasElements())
                {
                    // create error string for version mismatch
                    OUString sVersion( u"%VERSION"_ustr );
                    OUString sProductName( u"%PRODUCTNAME"_ustr );
                    sal_Int32 nPos = m_noDependencyCurVer.indexOf( sVersion );
                    if ( nPos >= 0 )
                    {
                        m_noDependencyCurVer = m_noDependencyCurVer.replaceAt( nPos, sVersion.getLength(), utl::ConfigManager::getAboutBoxProductVersion() );
                    }
                    nPos = m_noDependencyCurVer.indexOf( sProductName );
                    if ( nPos >= 0 )
                    {
                        m_noDependencyCurVer = m_noDependencyCurVer.replaceAt( nPos, sProductName.getLength(), utl::ConfigManager::getProductName() );
                    }
                    nPos = m_noDependency.indexOf( sProductName );
                    if ( nPos >= 0 )
                    {
                        m_noDependency = m_noDependency.replaceAt( nPos, sProductName.getLength(), utl::ConfigManager::getProductName() );
                    }

                    b.append(m_noInstall + OUStringChar(LF) + m_noDependency);
                    for (sal_Int32 i = 0;
                         i < data.unsatisfiedDependencies.getLength(); ++i)
                    {
                        b.append(OUStringChar(LF) + "  ");
                            // U+2003 EM SPACE would be better than two spaces,
                            // but some fonts do not contain it
                        b.append(
                            confineToParagraph(
                                data.unsatisfiedDependencies[i]));
                    }
                    b.append(OUStringChar(LF) + "  " + m_noDependencyCurVer);
                }
                break;
            }
            case SPECIFIC_ERROR:
            {
                UpdateDialog::SpecificError & data = m_specificErrors[ pos ];
                b.append(m_failure + OUStringChar(LF));
                b.append( data.message.isEmpty() ? m_unknownError : data.message );
                break;
            }
            default:
                OSL_ASSERT(false);
                break;
        }
    }

    if ( b.isEmpty() )
        b.append( m_noDescription );

    showDescription( b.makeStringAndClear() );
}

IMPL_LINK_NOARG(UpdateDialog, allHandler, weld::Toggleable&, void)
{
    if (m_xAll->get_active())
    {
        m_xUpdate->set_sensitive(true);
        m_xUpdates->set_sensitive(true);
        m_xDescription->set_sensitive(true);
        m_xDescriptions->set_sensitive(true);

        for (auto const& listboxEntry : m_ListboxEntries)
        {
            if ( listboxEntry->m_bIgnored || ( listboxEntry->m_eKind != ENABLED_UPDATE ) )
                insertItem(listboxEntry.get(), false);
        }
    }
    else
    {
        for (sal_uInt16 i = m_xUpdates->n_children(); i != 0 ;)
        {
            i -= 1;
            UpdateDialog::Index const * p = weld::fromId<UpdateDialog::Index const*>(m_xUpdates->get_id(i));
            if ( p->m_bIgnored || ( p->m_eKind != ENABLED_UPDATE ) )
            {
                m_xUpdates->remove(i);
            }
        }

        if (m_xUpdates->n_children() == 0)
        {
            clearDescription();
            m_xUpdate->set_sensitive(false);
            m_xUpdates->set_sensitive(false);
            if (m_xChecking->get_visible())
                m_xDescription->set_sensitive(false);
            else
                showDescription(m_noInstallable);
        }
    }
}

IMPL_LINK_NOARG(UpdateDialog, okHandler, weld::Button&, void)
{
    //If users are going to update a shared extension then we need
    //to warn them
    for (auto const& enableUpdate : m_enabledUpdates)
    {
        OSL_ASSERT(enableUpdate.aInstalledPackage.is());
        //If the user has no write access to the shared folder then the update
        //for a shared extension is disable, that is it cannot be in m_enabledUpdates
    }


    for (sal_uInt16 i = 0, nCount = m_xUpdates->n_children(); i < nCount; ++i)
    {
        UpdateDialog::Index const * p =
            weld::fromId<UpdateDialog::Index const*>(m_xUpdates->get_id(i));
        if (p->m_eKind == ENABLED_UPDATE && m_xUpdates->get_toggle(i) == TRISTATE_TRUE) {
            m_updateData.push_back( m_enabledUpdates[ p->m_nIndex ] );
        }
    }

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(UpdateDialog, closeHandler, weld::Button&, void)
{
    m_thread->stop();
    m_xDialog->response(RET_CANCEL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
