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

#include <cstddef>
#include <limits>
#include <map>
#include <utility>
#include <vector>


#include <boost/optional.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/WindowClass.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>
#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/deployment/XExtensionManager.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/deployment/XUpdateInformationProvider.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XAbortChannel.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/xml/dom/XElement.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>
#include <rtl/string.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <salhelper/thread.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/treelistbox.hxx>
#include <svtools/controldims.hxx>
#include <svx/checklbx.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <tools/solar.h>
#include <unotools/configmgr.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/image.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <comphelper/processfactory.hxx>

#include "dp_dependencies.hxx"
#include "dp_descriptioninfoset.hxx"
#include "dp_identifier.hxx"
#include "dp_version.hxx"
#include "dp_misc.h"
#include "dp_update.hxx"

#include "dp_gui.h"
#include "strings.hrc"
#include "bitmaps.hlst"
#include "dp_gui_updatedata.hxx"
#include "dp_gui_updatedialog.hxx"
#include "dp_shared.hxx"

class KeyEvent;
class MouseEvent;
namespace vcl { class Window; }
namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

using namespace ::com::sun::star;
using dp_gui::UpdateDialog;

namespace {

static sal_Unicode const LF = 0x000A;
static sal_Unicode const CR = 0x000D;
static const sal_uInt16 CMD_ENABLE_UPDATE = 1;
static const sal_uInt16 CMD_IGNORE_UPDATE = 2;
static const sal_uInt16 CMD_IGNORE_ALL_UPDATES = 3;

#define IGNORED_UPDATES     OUString("/org.openoffice.Office.ExtensionManager/ExtensionUpdateData/IgnoredUpdates")
#define PROPERTY_VERSION    "Version"

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
    bool     bRemoved;

    IgnoredUpdate( const OUString &rExtensionID, const OUString &rVersion );
};


UpdateDialog::IgnoredUpdate::IgnoredUpdate( const OUString &rExtensionID, const OUString &rVersion ):
    sExtensionID( rExtensionID ),
    sVersion( rVersion ),
    bRemoved( false )
{}


struct UpdateDialog::Index
{
    Kind          m_eKind;
    bool          m_bIgnored;
    sal_uInt16    m_nIndex;
    OUString      m_aName;

    Index( Kind theKind, sal_uInt16 nIndex, const OUString &rName ) :
        m_eKind( theKind ),
        m_bIgnored( false ),
        m_nIndex( nIndex ),
        m_aName( rName ) {}
};


class UpdateDialog::Thread: public salhelper::Thread {
public:
    Thread(
        uno::Reference< uno::XComponentContext > const & context,
        UpdateDialog & dialog,
        const std::vector< uno::Reference< deployment::XPackage >  > & vExtensionList);

    void stop();

private:
    virtual ~Thread() override;

    virtual void execute() override;

    void handleSpecificError(
        uno::Reference< deployment::XPackage > const & package,
        uno::Any const & exception) const;

    OUString getUpdateDisplayString(
        dp_gui::UpdateData const & data, OUString const & version = OUString()) const;

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
    const std::vector< uno::Reference< deployment::XPackage > > &vExtensionList):
    salhelper::Thread("dp_gui_updatedialog"),
    m_context(context),
    m_dialog(dialog),
    m_vExtensionList(vExtensionList),
    m_updateInformation(
        deployment::UpdateInformationProvider::create(context)),
    m_stop(false)
{
    if( m_context.is() )
    {
        m_xInteractionHdl.set(
            task::InteractionHandler::createWithParent(m_context, nullptr),
            uno::UNO_QUERY );
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

    typedef std::vector<std::pair<uno::Reference<deployment::XPackage>,
        uno::Any> >::const_iterator ITERROR;
    for (ITERROR ite = errors.begin(); ite != errors.end(); ++ite )
        handleSpecificError(ite->first, ite->second);

    for (dp_misc::UpdateInfoMap::iterator i(updateInfoMap.begin()); i != updateInfoMap.end(); ++i)
    {
        dp_misc::UpdateInfo const & info = i->second;
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

        bool bSharedReadOnly = extMgr->isReadOnlyRepository("shared");

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
    dp_gui::UpdateData const & data, OUString const & version) const
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
    if (!version.isEmpty())
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
    for (sal_Int32 i = 0; i < ds.getLength(); ++i) {
        out_du.unsatisfiedDependencies[i] = dp_misc::Dependencies::getErrorText(ds[i]);
    }

    const ::boost::optional< OUString> updateWebsiteURL(infoset.getLocalizedUpdateWebsiteURL());

    out_du.name = getUpdateDisplayString(out_data, infoset.getVersion());

    if (out_du.unsatisfiedDependencies.getLength() == 0)
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
    if (du.unsatisfiedDependencies.getLength() == 0)
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
    vcl::Window * parent,
                           const std::vector<uno::Reference< deployment::XPackage > > &vExtensionList,
    std::vector< dp_gui::UpdateData > * updateData):
    ModalDialog(parent, "UpdateDialog", "desktop/ui/updatedialog.ui"),
    m_context(context),
    m_none(DpResId(RID_DLG_UPDATE_NONE)),
    m_noInstallable(DpResId(RID_DLG_UPDATE_NOINSTALLABLE)),
    m_failure(DpResId(RID_DLG_UPDATE_FAILURE)),
    m_unknownError(DpResId(RID_DLG_UPDATE_UNKNOWNERROR)),
    m_noDescription(DpResId(RID_DLG_UPDATE_NODESCRIPTION)),
    m_noInstall(DpResId(RID_DLG_UPDATE_NOINSTALL)),
    m_noDependency(DpResId(RID_DLG_UPDATE_NODEPENDENCY)),
    m_noDependencyCurVer(DpResId(RID_DLG_UPDATE_NODEPENDENCY_CUR_VER)),
    m_browserbased(DpResId(RID_DLG_UPDATE_BROWSERBASED)),
    m_version(DpResId(RID_DLG_UPDATE_VERSION)),
    m_ignoredUpdate(DpResId(RID_DLG_UPDATE_IGNORED_UPDATE)),
    m_updateData(*updateData),
    m_thread(
        new UpdateDialog::Thread(
            context, *this, vExtensionList)),
    m_bModified( false )
    // TODO: check!
//    ,
//    m_extensionManagerDialog(extensionManagerDialog)
{
    get(m_pchecking, "UPDATE_CHECKING");
    get(m_pthrobber, "THROBBER");
    get(m_pUpdate, "UPDATE_LABEL");
    get(m_pContainer, "UPDATES_CONTAINER");
    m_pUpdates = VclPtr<UpdateDialog::CheckListBox>::Create(m_pContainer, *this);
    Size aSize(LogicToPixel(Size(240, 51), MapUnit::MapAppFont));
    m_pUpdates->set_width_request(aSize.Width());
    m_pUpdates->set_height_request(aSize.Height());
    m_pUpdates->Show();
    get(m_pAll, "UPDATE_ALL");
    get(m_pDescription, "DESCRIPTION_LABEL");
    get(m_pPublisherLabel, "PUBLISHER_LABEL");
    get(m_pPublisherLink, "PUBLISHER_LINK");
    get(m_pReleaseNotesLabel, "RELEASE_NOTES_LABEL");
    get(m_pReleaseNotesLink, "RELEASE_NOTES_LINK");
    get(m_pDescriptions, "DESCRIPTIONS");
    aSize = LogicToPixel(Size(240, 59), MapUnit::MapAppFont);
    m_pDescriptions->set_width_request(aSize.Width());
    m_pDescriptions->set_height_request(aSize.Height());
    get(m_pOk, "INSTALL");
    get(m_pClose, "gtk-close");
    get(m_pHelp, "gtk-help");
    OSL_ASSERT(updateData != nullptr);

    m_xExtensionManager = deployment::ExtensionManager::get( context );

    uno::Reference< awt::XToolkit2 > toolkit;
    try {
        toolkit = awt::Toolkit::create(m_context);
    } catch (const uno::RuntimeException &) {
        throw;
    } catch (const uno::Exception & e) {
        throw uno::RuntimeException(e.Message, e.Context);
    }
    m_pUpdates->SetSelectHdl(LINK(this, UpdateDialog, selectionHandler));
    m_pAll->SetToggleHdl(LINK(this, UpdateDialog, allHandler));
    m_pOk->SetClickHdl(LINK(this, UpdateDialog, okHandler));
    m_pClose->SetClickHdl(LINK(this, UpdateDialog, closeHandler));
    if ( ! dp_misc::office_is_running())
        m_pHelp->Disable();

    initDescription();
    getIgnoredUpdates();
}


UpdateDialog::~UpdateDialog()
{
    disposeOnce();
}

void UpdateDialog::dispose()
{
    storeIgnoredUpdates();

    for ( std::vector< UpdateDialog::Index* >::iterator i( m_ListboxEntries.begin() ); i != m_ListboxEntries.end(); ++i )
    {
        delete (*i);
    }
    for ( std::vector< UpdateDialog::IgnoredUpdate* >::iterator i( m_ignoredUpdates.begin() ); i != m_ignoredUpdates.end(); ++i )
    {
        delete (*i);
    }
    m_pUpdates.disposeAndClear();
    m_pchecking.clear();
    m_pthrobber.clear();
    m_pUpdate.clear();
    m_pContainer.clear();
    m_pAll.clear();
    m_pDescription.clear();
    m_pPublisherLabel.clear();
    m_pPublisherLink.clear();
    m_pReleaseNotesLabel.clear();
    m_pReleaseNotesLink.clear();
    m_pDescriptions.clear();
    m_pHelp.clear();
    m_pOk.clear();
    m_pClose.clear();
    ModalDialog::dispose();
}


bool UpdateDialog::Close() {
    m_thread->stop();
    return ModalDialog::Close();
}

short UpdateDialog::Execute() {
    m_pthrobber->start();
    m_thread->launch();
    return ModalDialog::Execute();
}

UpdateDialog::CheckListBox::CheckListBox( vcl::Window* pParent, UpdateDialog & dialog):
    SvxCheckListBox( pParent, WinBits(WB_BORDER) ),
    m_ignoreUpdate( DpResId( RID_DLG_UPDATE_IGNORE ) ),
    m_ignoreAllUpdates( DpResId( RID_DLG_UPDATE_IGNORE_ALL ) ),
    m_enableUpdate( DpResId( RID_DLG_UPDATE_ENABLE ) ),
    m_dialog(dialog)
{
    SetNormalStaticImage(Image(BitmapEx(RID_DLG_UPDATE_NORMALALERT)));
}

sal_uInt16 UpdateDialog::CheckListBox::getItemCount() const {
    sal_uLong i = GetEntryCount();
    OSL_ASSERT(i <= std::numeric_limits< sal_uInt16 >::max());
    return sal::static_int_cast< sal_uInt16 >(i);
}


void UpdateDialog::CheckListBox::MouseButtonDown( MouseEvent const & event )
{
    // When clicking on a selected entry in an SvxCheckListBox, the entry's
    // checkbox is toggled on mouse button down:
    SvxCheckListBox::MouseButtonDown( event );

    if ( event.IsRight() )
    {
        handlePopupMenu( event.GetPosPixel() );
    }

    m_dialog.enableOk();
}


void UpdateDialog::CheckListBox::MouseButtonUp(MouseEvent const & event) {
    // When clicking on an entry's checkbox in an SvxCheckListBox, the entry's
    // checkbox is toggled on mouse button up:
    SvxCheckListBox::MouseButtonUp(event);
    m_dialog.enableOk();
}

void UpdateDialog::CheckListBox::KeyInput(KeyEvent const & event) {
    SvxCheckListBox::KeyInput(event);
    m_dialog.enableOk();
}


void UpdateDialog::CheckListBox::handlePopupMenu( const Point &rPos )
{
    SvTreeListEntry *pData = GetEntry( rPos );

    if ( pData )
    {
        sal_uLong nEntryPos = GetSelectEntryPos();
        UpdateDialog::Index * p = static_cast< UpdateDialog::Index * >( GetEntryData( nEntryPos ) );

        if ( ( p->m_eKind == ENABLED_UPDATE ) || ( p->m_eKind == DISABLED_UPDATE ) )
        {
            ScopedVclPtrInstance<PopupMenu> aPopup;

            if ( p->m_bIgnored )
                aPopup->InsertItem( CMD_ENABLE_UPDATE, m_enableUpdate );
            else
            {
                aPopup->InsertItem( CMD_IGNORE_UPDATE, m_ignoreUpdate );
                aPopup->InsertItem( CMD_IGNORE_ALL_UPDATES, m_ignoreAllUpdates );
            }

            sal_uInt16 aCmd = aPopup->Execute( this, rPos );
            if ( ( aCmd == CMD_IGNORE_UPDATE ) || ( aCmd == CMD_IGNORE_ALL_UPDATES ) )
            {
                p->m_bIgnored = true;
                if ( p->m_eKind == ENABLED_UPDATE )
                {
                    RemoveEntry( nEntryPos );
                    m_dialog.addAdditional( p, SvLBoxButtonKind::DisabledCheckbox );
                }
                if ( aCmd == CMD_IGNORE_UPDATE )
                    m_dialog.setIgnoredUpdate( p, true, false );
                else
                    m_dialog.setIgnoredUpdate( p, true, true );
                // TODO: reselect entry to display new description!
            }
            else if ( aCmd == CMD_ENABLE_UPDATE )
            {
                p->m_bIgnored = false;
                if ( p->m_eKind == ENABLED_UPDATE )
                {
                    RemoveEntry( nEntryPos );
                    m_dialog.insertItem( p, SvLBoxButtonKind::EnabledCheckbox );
                }
                m_dialog.setIgnoredUpdate( p, false, false );
            }
        }
    }
}


sal_uInt16 UpdateDialog::insertItem( UpdateDialog::Index *pEntry, SvLBoxButtonKind kind )
{
    m_pUpdates->InsertEntry( pEntry->m_aName, TREELIST_APPEND, static_cast< void * >( pEntry ), kind );

    for ( sal_uInt16 i = m_pUpdates->getItemCount(); i != 0 ; )
    {
        i -= 1;
        UpdateDialog::Index const * p = static_cast< UpdateDialog::Index const * >( m_pUpdates->GetEntryData( i ) );
        if ( p == pEntry )
            return i;
    }
    OSL_ASSERT(false);
    return 0;
}


void UpdateDialog::addAdditional( UpdateDialog::Index * index, SvLBoxButtonKind kind )
{
    m_pAll->Enable();
    if (m_pAll->IsChecked())
    {
        insertItem( index, kind );
        m_pUpdate->Enable();
        m_pUpdates->Enable();
        m_pDescription->Enable();
        m_pDescriptions->Enable();
    }
}


void UpdateDialog::addEnabledUpdate( OUString const & name,
                                     dp_gui::UpdateData const & data )
{
    sal_uInt16 nIndex = sal::static_int_cast< sal_uInt16 >( m_enabledUpdates.size() );
    UpdateDialog::Index *pEntry = new UpdateDialog::Index( ENABLED_UPDATE, nIndex, name );

    m_enabledUpdates.push_back( data );
    m_ListboxEntries.push_back( pEntry );

    if ( ! isIgnoredUpdate( pEntry ) )
    {
        sal_uInt16 nPos = insertItem( pEntry, SvLBoxButtonKind::EnabledCheckbox );
        m_pUpdates->CheckEntryPos( nPos );
    }
    else
        addAdditional( pEntry, SvLBoxButtonKind::DisabledCheckbox );

    m_pUpdate->Enable();
    m_pUpdates->Enable();
    m_pDescription->Enable();
    m_pDescriptions->Enable();
}


void UpdateDialog::addDisabledUpdate( UpdateDialog::DisabledUpdate const & data )
{
    sal_uInt16 nIndex = sal::static_int_cast< sal_uInt16 >( m_disabledUpdates.size() );
    UpdateDialog::Index *pEntry = new UpdateDialog::Index( DISABLED_UPDATE, nIndex, data.name );

    m_disabledUpdates.push_back( data );
    m_ListboxEntries.push_back( pEntry );

    isIgnoredUpdate( pEntry );
    addAdditional( pEntry, SvLBoxButtonKind::DisabledCheckbox );
}


void UpdateDialog::addSpecificError( UpdateDialog::SpecificError const & data )
{
    sal_uInt16 nIndex = sal::static_int_cast< sal_uInt16 >( m_specificErrors.size() );
    UpdateDialog::Index *pEntry = new UpdateDialog::Index( SPECIFIC_ERROR, nIndex, data.name );

    m_specificErrors.push_back( data );
    m_ListboxEntries.push_back( pEntry );

    addAdditional( pEntry, SvLBoxButtonKind::StaticImage);
}

void UpdateDialog::checkingDone() {
    m_pchecking->Hide();
    m_pthrobber->stop();
    m_pthrobber->Hide();
    if (m_pUpdates->getItemCount() == 0)
    {
        clearDescription();
        m_pDescription->Enable();
        m_pDescriptions->Enable();

        if ( m_disabledUpdates.empty() && m_specificErrors.empty() && m_ignoredUpdates.empty() )
            showDescription( m_none );
        else
            showDescription( m_noInstallable );
    }

    enableOk();
}

void UpdateDialog::enableOk() {
    if (!m_pchecking->IsVisible()) {
        m_pOk->Enable(m_pUpdates->GetCheckedEntryCount() != 0);
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

        beans::PropertyValue aProperty;
        aProperty.Name  = "nodepath";
        aProperty.Value <<= OUString("org.openoffice.Office.Addons/AddonUI/OfficeHelp/UpdateCheckJob");

        uno::Sequence< uno::Any > aArgumentList( 1 );
        aArgumentList[0] <<= aProperty;

        uno::Reference< container::XNameAccess > xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess", aArgumentList ),
            uno::UNO_QUERY_THROW );

        util::URL aURL;
        xNameAccess->getByName("URL") >>= aURL.Complete;

        uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        uno::Reference < util::XURLTransformer > xTransformer = util::URLTransformer::create(xContext);

        xTransformer->parseStrict(aURL);

        uno::Reference < frame::XDesktop2 > xDesktop = frame::Desktop::create( xContext );
        uno::Reference< frame::XDispatchProvider > xDispatchProvider( xDesktop->getCurrentFrame(),
            uno::UNO_QUERY_THROW );
        uno::Reference< frame::XDispatch > xDispatch = xDispatchProvider->queryDispatch(aURL, OUString(), 0);

        if( xDispatch.is() )
        {
            uno::Sequence< beans::PropertyValue > aPropList(2);
            aProperty.Name  = "updateList";
            aProperty.Value <<= rItemList;
            aPropList[0] = aProperty;
            aProperty.Name  = "prepareOnly";
            aProperty.Value <<= bPrepareOnly;
            aPropList[1] = aProperty;

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
        for ( sal_Int16 i = 0; i < m_pUpdates->getItemCount(); ++i )
        {
            uno::Sequence< OUString > aItem(2);

            UpdateDialog::Index const * p = static_cast< UpdateDialog::Index const * >(m_pUpdates->GetEntryData(i));

            if ( p->m_eKind == ENABLED_UPDATE )
            {
                dp_gui::UpdateData aUpdData = m_enabledUpdates[ p->m_nIndex ];
                aItem[0] = dp_misc::getIdentifier( aUpdData.aInstalledPackage );

                dp_misc::DescriptionInfoset aInfoset( m_context, aUpdData.aUpdateInfo );
                aItem[1] = aInfoset.getVersion();
            }
            else
                continue;

            aItemList.realloc( nCount + 1 );
            aItemList[ nCount ] = aItem;
            nCount += 1;
        }
    }

    storeIgnoredUpdates();
    createNotifyJob( bPrepareOnly, aItemList );
}

// *********************************************************************************

void UpdateDialog::initDescription()
{
    m_pPublisherLabel->Hide();
    m_pPublisherLink->Hide();
    m_pReleaseNotesLabel->Hide();
    m_pReleaseNotesLink->Hide();
}

void UpdateDialog::clearDescription()
{
    m_pPublisherLabel->Hide();
    m_pPublisherLink->Hide();
    m_pPublisherLink->SetText( "" );
    m_pPublisherLink->SetURL( "" );
    m_pReleaseNotesLabel->Hide();
    m_pReleaseNotesLink->Hide();
    m_pReleaseNotesLink->SetURL( "" );
    m_pDescriptions->SetText("");
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
                           "");
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
        m_pPublisherLabel->Show();
        m_pPublisherLink->Show();
        m_pPublisherLink->SetText( sPub );
        m_pPublisherLink->SetURL( sURL );
    }

    if ( !sReleaseNotes.isEmpty() )
    {
        m_pReleaseNotesLabel->Show();
        m_pReleaseNotesLink->Show();
        m_pReleaseNotesLink->SetURL( sReleaseNotes );
    }
    return true;
}

bool UpdateDialog::showDescription( const OUString& rDescription)
{
    if ( rDescription.isEmpty() )
        // nothing to show
        return false;

    m_pDescriptions->SetText( rDescription );
    return true;
}


void UpdateDialog::getIgnoredUpdates()
{
    uno::Reference< lang::XMultiServiceFactory > xConfig(
        configuration::theDefaultProvider::get(m_context));
    beans::NamedValue aValue( "nodepath", uno::Any( IGNORED_UPDATES ) );
    uno::Sequence< uno::Any > args(1);
    args[0] <<= aValue;

    uno::Reference< container::XNameAccess > xNameAccess( xConfig->createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess", args), uno::UNO_QUERY_THROW );
    uno::Sequence< OUString > aElementNames = xNameAccess->getElementNames();

    for ( sal_Int32 i = 0; i < aElementNames.getLength(); i++ )
    {
        OUString aIdentifier = aElementNames[i];
        OUString aVersion;

        uno::Any aPropValue( uno::Reference< beans::XPropertySet >( xNameAccess->getByName( aIdentifier ), uno::UNO_QUERY_THROW )->getPropertyValue( PROPERTY_VERSION ) );
        aPropValue >>= aVersion;
        IgnoredUpdate *pData = new IgnoredUpdate( aIdentifier, aVersion );
        m_ignoredUpdates.push_back( pData );
    }
}


void UpdateDialog::storeIgnoredUpdates()
{
    if ( m_bModified && ( !m_ignoredUpdates.empty() ) )
    {
        uno::Reference< lang::XMultiServiceFactory > xConfig(
            configuration::theDefaultProvider::get(m_context));
        beans::NamedValue aValue( "nodepath", uno::Any( IGNORED_UPDATES ) );
        uno::Sequence< uno::Any > args(1);
        args[0] <<= aValue;

        uno::Reference< container::XNameContainer > xNameContainer( xConfig->createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationUpdateAccess", args ), uno::UNO_QUERY_THROW );

        for ( std::vector< UpdateDialog::IgnoredUpdate* >::iterator i( m_ignoredUpdates.begin() ); i != m_ignoredUpdates.end(); ++i )
        {
            if ( xNameContainer->hasByName( (*i)->sExtensionID ) )
            {
                if ( (*i)->bRemoved )
                    xNameContainer->removeByName( (*i)->sExtensionID );
                else
                    uno::Reference< beans::XPropertySet >( xNameContainer->getByName( (*i)->sExtensionID ), uno::UNO_QUERY_THROW )->setPropertyValue( PROPERTY_VERSION, uno::Any( (*i)->sVersion ) );
            }
            else if ( ! (*i)->bRemoved )
            {
                uno::Reference< beans::XPropertySet > elem( uno::Reference< lang::XSingleServiceFactory >( xNameContainer, uno::UNO_QUERY_THROW )->createInstance(), uno::UNO_QUERY_THROW );
                elem->setPropertyValue( PROPERTY_VERSION, uno::Any( (*i)->sVersion ) );
                xNameContainer->insertByName( (*i)->sExtensionID, uno::Any( elem ) );
            }
        }

        uno::Reference< util::XChangesBatch > xChangesBatch( xNameContainer, uno::UNO_QUERY );
        if ( xChangesBatch.is() && xChangesBatch->hasPendingChanges() )
            xChangesBatch->commitChanges();
    }

    m_bModified = false;
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
            ::boost::optional< OUString > aID( aInfoset.getIdentifier() );
            if ( aID )
                aExtensionID = *aID;
            aVersion = aInfoset.getVersion();
        }

        for ( std::vector< UpdateDialog::IgnoredUpdate* >::iterator i( m_ignoredUpdates.begin() ); i != m_ignoredUpdates.end(); ++i )
        {
            if ( (*i)->sExtensionID == aExtensionID )
            {
                if ( ( !(*i)->sVersion.isEmpty() ) || ( (*i)->sVersion == aVersion ) )
                {
                    bIsIgnored = true;
                    index->m_bIgnored = true;
                }
                else // when we find another update of an ignored version, we will remove the old one to keep the ignored list small
                    (*i)->bRemoved = true;
                break;
            }
        }
    }

    return bIsIgnored;
}


void UpdateDialog::setIgnoredUpdate( UpdateDialog::Index *pIndex, bool bIgnore, bool bIgnoreAll )
{
    OUString aExtensionID;
    OUString aVersion;

    m_bModified = true;

    if ( pIndex->m_eKind == ENABLED_UPDATE )
    {
        dp_gui::UpdateData aUpdData = m_enabledUpdates[ pIndex->m_nIndex ];
        aExtensionID = dp_misc::getIdentifier( aUpdData.aInstalledPackage );
        if ( !bIgnoreAll )
            aVersion = aUpdData.updateVersion;
    }
    else if ( pIndex->m_eKind == DISABLED_UPDATE )
    {
        DisabledUpdate &rData = m_disabledUpdates[ pIndex->m_nIndex ];
        dp_misc::DescriptionInfoset aInfoset( m_context, rData.aUpdateInfo );
        ::boost::optional< OUString > aID( aInfoset.getIdentifier() );
        if ( aID )
            aExtensionID = *aID;
        if ( !bIgnoreAll )
            aVersion = aInfoset.getVersion();
    }

    if ( !aExtensionID.isEmpty() )
    {
        bool bFound = false;
        for ( std::vector< UpdateDialog::IgnoredUpdate* >::iterator i( m_ignoredUpdates.begin() ); i != m_ignoredUpdates.end(); ++i )
        {
            if ( (*i)->sExtensionID == aExtensionID )
            {
                (*i)->sVersion = aVersion;
                (*i)->bRemoved = !bIgnore;
                bFound = true;
                break;
            }
        }
        if ( bIgnore && !bFound )
        {
            IgnoredUpdate *pData = new IgnoredUpdate( aExtensionID, aVersion );
            m_ignoredUpdates.push_back( pData );
        }
    }
}


IMPL_LINK_NOARG(UpdateDialog, selectionHandler, SvTreeListBox*, void)
{
    OUStringBuffer b;
    UpdateDialog::Index const * p = static_cast< UpdateDialog::Index const * >(
        m_pUpdates->GetSelectEntryData());
    clearDescription();

    if ( p != nullptr )
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
                if (data.unsatisfiedDependencies.getLength() != 0)
                {
                    // create error string for version mismatch
                    OUString sVersion( "%VERSION" );
                    OUString sProductName( "%PRODUCTNAME" );
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

                    b.append(m_noInstall);
                    b.append(LF);
                    b.append(m_noDependency);
                    for (sal_Int32 i = 0;
                         i < data.unsatisfiedDependencies.getLength(); ++i)
                    {
                        b.append(LF);
                        b.append("  ");
                            // U+2003 EM SPACE would be better than two spaces,
                            // but some fonts do not contain it
                        b.append(
                            confineToParagraph(
                                data.unsatisfiedDependencies[i]));
                    }
                    b.append(LF);
                    b.append("  ");
                    b.append(m_noDependencyCurVer);
                }
                break;
            }
            case SPECIFIC_ERROR:
            {
                UpdateDialog::SpecificError & data = m_specificErrors[ pos ];
                b.append(m_failure);
                b.append(LF);
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

IMPL_LINK_NOARG(UpdateDialog, allHandler, CheckBox&, void)
{
    if (m_pAll->IsChecked())
    {
        m_pUpdate->Enable();
        m_pUpdates->Enable();
        m_pDescription->Enable();
        m_pDescriptions->Enable();

        for (std::vector< UpdateDialog::Index* >::iterator i( m_ListboxEntries.begin() );
             i != m_ListboxEntries.end(); ++i )
        {
            if ( (*i)->m_bIgnored || ( (*i)->m_eKind != ENABLED_UPDATE ) )
                insertItem( (*i), SvLBoxButtonKind::DisabledCheckbox );
        }
    }
    else
    {
        for ( sal_uInt16 i = 0; i < m_pUpdates->getItemCount(); )
        {
            UpdateDialog::Index const * p = static_cast< UpdateDialog::Index const * >( m_pUpdates->GetEntryData(i) );
            if ( p->m_bIgnored || ( p->m_eKind != ENABLED_UPDATE ) )
            {
                m_pUpdates->RemoveEntry(i);
            } else {
                ++i;
            }
        }

        if (m_pUpdates->getItemCount() == 0)
        {
            clearDescription();
            m_pUpdate->Disable();
            m_pUpdates->Disable();
            if (m_pchecking->IsVisible())
                m_pDescription->Disable();
            else
                showDescription(m_noInstallable);
        }
    }
}

IMPL_LINK_NOARG(UpdateDialog, okHandler, Button*, void)
{
    //If users are going to update a shared extension then we need
    //to warn them
    typedef std::vector<UpdateData>::const_iterator CIT;
    for (CIT i = m_enabledUpdates.begin(); i < m_enabledUpdates.end(); ++i)
    {
        OSL_ASSERT(i->aInstalledPackage.is());
        //If the user has no write access to the shared folder then the update
        //for a shared extension is disable, that is it cannot be in m_enabledUpdates
    }


    for (sal_uInt16 i = 0; i < m_pUpdates->getItemCount(); ++i) {
        UpdateDialog::Index const * p =
            static_cast< UpdateDialog::Index const * >(
                m_pUpdates->GetEntryData(i));
        if (p->m_eKind == ENABLED_UPDATE && m_pUpdates->IsChecked(i)) {
            m_updateData.push_back( m_enabledUpdates[ p->m_nIndex ] );
        }
    }

    EndDialog(RET_OK);
}

IMPL_LINK_NOARG(UpdateDialog, closeHandler, Button*, void)
{
    m_thread->stop();
    EndDialog();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
