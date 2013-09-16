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


#include "sal/config.h"

#include <cstddef>
#include <limits>
#include <map>
#include <memory>
#include <utility>
#include <vector>


#include "boost/optional.hpp"
#include "com/sun/star/awt/Rectangle.hpp"
#include "com/sun/star/awt/WindowAttribute.hpp"
#include "com/sun/star/awt/WindowClass.hpp"
#include "com/sun/star/awt/WindowDescriptor.hpp"
#include "com/sun/star/awt/Toolkit.hpp"
#include "com/sun/star/awt/XWindow.hpp"
#include "com/sun/star/awt/XWindowPeer.hpp"
#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/configuration/theDefaultProvider.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/deployment/DeploymentException.hpp"
#include "com/sun/star/deployment/UpdateInformationProvider.hpp"
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/deployment/XExtensionManager.hpp"
#include "com/sun/star/deployment/ExtensionManager.hpp"
#include "com/sun/star/deployment/XUpdateInformationProvider.hpp"
#include "com/sun/star/frame/Desktop.hpp"
#include "com/sun/star/frame/XDispatch.hpp"
#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/system/SystemShellExecuteFlags.hpp"
#include "com/sun/star/system/SystemShellExecute.hpp"
#include "com/sun/star/task/InteractionHandler.hpp"
#include "com/sun/star/task/XAbortChannel.hpp"
#include "com/sun/star/task/XJob.hpp"
#include "com/sun/star/ucb/CommandAbortedException.hpp"
#include "com/sun/star/ucb/CommandFailedException.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/URL.hpp"
#include "com/sun/star/util/XChangesBatch.hpp"
#include "com/sun/star/util/URLTransformer.hpp"
#include "com/sun/star/util/XURLTransformer.hpp"
#include "com/sun/star/xml/dom/XElement.hpp"
#include "com/sun/star/xml/dom/XNode.hpp"
#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "salhelper/thread.hxx"
#include "svtools/svlbitm.hxx"
#include "svtools/treelistbox.hxx"
#include <svtools/controldims.hrc>
#include "svx/checklbx.hxx"
#include "tools/gen.hxx"
#include "tools/link.hxx"
#include "tools/resid.hxx"
#include "tools/solar.h"
#include "unotools/configmgr.hxx"
#include "vcl/button.hxx"
#include "vcl/dialog.hxx"
#include "vcl/fixed.hxx"
#include "vcl/image.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/svapp.hxx"
#include "osl/mutex.hxx"

#include "comphelper/processfactory.hxx"

#include "dp_dependencies.hxx"
#include "dp_descriptioninfoset.hxx"
#include "dp_identifier.hxx"
#include "dp_version.hxx"
#include "dp_misc.h"
#include "dp_update.hxx"

#include "dp_gui.h"
#include "dp_gui.hrc"
#include "dp_gui_updatedata.hxx"
#include "dp_gui_updatedialog.hxx"
#include "dp_gui_shared.hxx"

class KeyEvent;
class MouseEvent;
class Window;
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
    // Confine arbitrary text to a single paragraph in a dp_gui::AutoScrollEdit.
    // This assumes that U+000A and U+000D are the only paragraph separators in
    // a dp_gui::AutoScrollEdit, and that replacing them with a single space
    // each is acceptable:
    return text.replace(LF, ' ').replace(CR, ' ');
}
}

struct UpdateDialog::DisabledUpdate {
    OUString name;
    uno::Sequence< OUString > unsatisfiedDependencies;
    // We also want to show release notes and publisher for disabled updates
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::dom::XNode > aUpdateInfo;
    sal_uInt16 m_nID;
};

struct UpdateDialog::SpecificError {
    OUString name;
    OUString message;
    sal_uInt16 m_nID;
};

//------------------------------------------------------------------------------
struct UpdateDialog::IgnoredUpdate {
    OUString sExtensionID;
    OUString sVersion;
    bool          bRemoved;

    IgnoredUpdate( const OUString &rExtensionID, const OUString &rVersion );
};

//------------------------------------------------------------------------------
UpdateDialog::IgnoredUpdate::IgnoredUpdate( const OUString &rExtensionID, const OUString &rVersion ):
    sExtensionID( rExtensionID ),
    sVersion( rVersion ),
    bRemoved( false )
{}

//------------------------------------------------------------------------------
struct UpdateDialog::Index
{
    Kind          m_eKind;
    bool          m_bIgnored;
    sal_uInt16        m_nID;
    sal_uInt16        m_nIndex;
    OUString m_aName;

    Index( Kind theKind, sal_uInt16 nID, sal_uInt16 nIndex, const OUString &rName );
};

//------------------------------------------------------------------------------
UpdateDialog::Index::Index( Kind theKind, sal_uInt16 nID, sal_uInt16 nIndex, const OUString &rName ):
    m_eKind( theKind ),
    m_bIgnored( false ),
    m_nID( nID ),
    m_nIndex( nIndex ),
    m_aName( rName )
{}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class UpdateDialog::Thread: public salhelper::Thread {
public:
    Thread(
        uno::Reference< uno::XComponentContext > const & context,
        UpdateDialog & dialog,
        const std::vector< uno::Reference< deployment::XPackage >  > & vExtensionList);

    void stop();

private:
    virtual ~Thread();

    virtual void execute();

    void handleSpecificError(
        uno::Reference< deployment::XPackage > const & package,
        uno::Any const & exception) const;

    uno::Sequence< uno::Reference< xml::dom::XElement > >
    getUpdateInformation(
        uno::Reference< deployment::XPackage > const & package,
        uno::Sequence< OUString > const & urls,
        OUString const & identifier) const;

    OUString getUpdateDisplayString(
        dp_gui::UpdateData const & data, OUString const & version = OUString()) const;

    void prepareUpdateData(
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::dom::XNode > const & updateInfo,
        UpdateDialog::DisabledUpdate & out_du,
        dp_gui::UpdateData & out_data) const;

    bool update(
        UpdateDialog::DisabledUpdate & du,
        dp_gui::UpdateData & data) const;

    uno::Reference< uno::XComponentContext > m_context;
    UpdateDialog & m_dialog;
    std::vector< uno::Reference< deployment::XPackage > > m_vExtensionList;
    uno::Reference< deployment::XUpdateInformationProvider > m_updateInformation;
    uno::Reference< task::XInteractionHandler > m_xInteractionHdl;

    // guarded by Application::GetSolarMutex():
    uno::Reference< task::XAbortChannel > m_abort;
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
            task::InteractionHandler::createWithParent(m_context, 0),
            uno::UNO_QUERY );
        m_updateInformation->setInteractionHandler( m_xInteractionHdl );
    }
}

void UpdateDialog::Thread::stop() {
    uno::Reference< task::XAbortChannel > abort;
    {
        SolarMutexGuard g;
        abort = m_abort;
        m_stop = true;
    }
    if (abort.is()) {
        abort->sendAbort();
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
            OSL_ASSERT(0);
            continue;
        } catch ( const css::ucb::CommandFailedException& ) {
            OSL_ASSERT(0);
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

        uno::Reference<deployment::XPackage> updateSource;
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
    b.append(static_cast< sal_Unicode >(' '));
    {
        SolarMutexGuard g;
        if(!m_stop)
            b.append(m_dialog.m_version);
    }
    b.append(static_cast< sal_Unicode >(' '));
    if (!version.isEmpty())
        b.append(version);
    else
        b.append(data.updateVersion);

    if (!data.sWebsiteURL.isEmpty())
    {
        b.append(static_cast< sal_Unicode >(' '));
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
    UpdateDialog::DisabledUpdate & du,
    dp_gui::UpdateData & data) const
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
    Window * parent,
    const std::vector<uno::Reference< deployment::XPackage > > &vExtensionList,
    std::vector< dp_gui::UpdateData > * updateData):
    ModalDialog(parent,DpGuiResId(RID_DLG_UPDATE)),
    m_context(context),
    m_checking(this, DpGuiResId(RID_DLG_UPDATE_CHECKING)),
    m_throbber(this, DpGuiResId(RID_DLG_UPDATE_THROBBER)),
    m_update(this, DpGuiResId(RID_DLG_UPDATE_UPDATE)),
    m_updates(
        *this, DpGuiResId(RID_DLG_UPDATE_UPDATES),
        Image(DpGuiResId(RID_DLG_UPDATE_NORMALALERT))),
    m_all(this, DpGuiResId(RID_DLG_UPDATE_ALL)),
    m_description(this, DpGuiResId(RID_DLG_UPDATE_DESCRIPTION)),
    m_PublisherLabel(this, DpGuiResId(RID_DLG_UPDATE_PUBLISHER_LABEL)),
    m_PublisherLink(this, DpGuiResId(RID_DLG_UPDATE_PUBLISHER_LINK)),
    m_ReleaseNotesLabel(this, DpGuiResId(RID_DLG_UPDATE_RELEASENOTES_LABEL)),
    m_ReleaseNotesLink(this, DpGuiResId(RID_DLG_UPDATE_RELEASENOTES_LINK)),
    m_descriptions(this, DpGuiResId(RID_DLG_UPDATE_DESCRIPTIONS)),
    m_line(this, DpGuiResId(RID_DLG_UPDATE_LINE)),
    m_help(this, DpGuiResId(RID_DLG_UPDATE_HELP)),
    m_ok(this, DpGuiResId(RID_DLG_UPDATE_OK)),
    m_close(this, DpGuiResId(RID_DLG_UPDATE_CLOSE)),
    m_error(DPGUI_RESSTR(RID_DLG_UPDATE_ERROR)),
    m_none(DPGUI_RESSTR(RID_DLG_UPDATE_NONE)),
    m_noInstallable(DPGUI_RESSTR(RID_DLG_UPDATE_NOINSTALLABLE)),
    m_failure(DPGUI_RESSTR(RID_DLG_UPDATE_FAILURE)),
    m_unknownError(DPGUI_RESSTR(RID_DLG_UPDATE_UNKNOWNERROR)),
    m_noDescription(DPGUI_RESSTR(RID_DLG_UPDATE_NODESCRIPTION)),
    m_noInstall(DPGUI_RESSTR(RID_DLG_UPDATE_NOINSTALL)),
    m_noDependency(DPGUI_RESSTR(RID_DLG_UPDATE_NODEPENDENCY)),
    m_noDependencyCurVer(DPGUI_RESSTR(RID_DLG_UPDATE_NODEPENDENCY_CUR_VER)),
    m_browserbased(DPGUI_RESSTR(RID_DLG_UPDATE_BROWSERBASED)),
    m_version(DPGUI_RESSTR(RID_DLG_UPDATE_VERSION)),
    m_ignoredUpdate(DPGUI_RESSTR(RID_DLG_UPDATE_IGNORED_UPDATE)),
    m_updateData(*updateData),
    m_thread(
        new UpdateDialog::Thread(
            context, *this, vExtensionList)),
    m_nFirstLineDelta(0),
    m_nOneLineMissing(0),
    m_nLastID(1),
    m_bModified( false )
    // TODO: check!
//    ,
//    m_extensionManagerDialog(extensionManagerDialog)
{
    OSL_ASSERT(updateData != NULL);

    m_xExtensionManager = deployment::ExtensionManager::get( context );

    uno::Reference< awt::XToolkit2 > toolkit;
    try {
        toolkit = awt::Toolkit::create(m_context);
    } catch (const uno::RuntimeException &) {
        throw;
    } catch (const uno::Exception & e) {
        throw uno::RuntimeException(e.Message, e.Context);
    }
    m_updates.SetSelectHdl(LINK(this, UpdateDialog, selectionHandler));
    m_all.SetToggleHdl(LINK(this, UpdateDialog, allHandler));
    m_ok.SetClickHdl(LINK(this, UpdateDialog, okHandler));
    m_close.SetClickHdl(LINK(this, UpdateDialog, closeHandler));
    if ( ! dp_misc::office_is_running())
        m_help.Disable();
    FreeResource();

    initDescription();
    getIgnoredUpdates();
}

//------------------------------------------------------------------------------
UpdateDialog::~UpdateDialog()
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
}

//------------------------------------------------------------------------------
sal_Bool UpdateDialog::Close() {
    m_thread->stop();
    return ModalDialog::Close();
}

short UpdateDialog::Execute() {
    m_throbber.start();
    m_thread->launch();
    return ModalDialog::Execute();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
UpdateDialog::CheckListBox::CheckListBox( UpdateDialog & dialog, ResId const & resource,
                                          Image const & normalStaticImage ):
    SvxCheckListBox( &dialog, resource, normalStaticImage ),
    m_ignoreUpdate( DPGUI_RESSTR( RID_DLG_UPDATE_IGNORE ) ),
    m_ignoreAllUpdates( DPGUI_RESSTR( RID_DLG_UPDATE_IGNORE_ALL ) ),
    m_enableUpdate( DPGUI_RESSTR( RID_DLG_UPDATE_ENABLE ) ),
    m_dialog(dialog)
{}

//------------------------------------------------------------------------------
UpdateDialog::CheckListBox::~CheckListBox() {}

//------------------------------------------------------------------------------
sal_uInt16 UpdateDialog::CheckListBox::getItemCount() const {
    sal_uLong i = GetEntryCount();
    OSL_ASSERT(i <= std::numeric_limits< sal_uInt16 >::max());
    return sal::static_int_cast< sal_uInt16 >(i);
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void UpdateDialog::CheckListBox::handlePopupMenu( const Point &rPos )
{
    SvTreeListEntry *pData = GetEntry( rPos );

    if ( pData )
    {
        sal_uInt16 nEntryPos = GetSelectEntryPos();
        UpdateDialog::Index * p = static_cast< UpdateDialog::Index * >( GetEntryData( nEntryPos ) );

        if ( ( p->m_eKind == ENABLED_UPDATE ) || ( p->m_eKind == DISABLED_UPDATE ) )
        {
            PopupMenu aPopup;

            if ( p->m_bIgnored )
                aPopup.InsertItem( CMD_ENABLE_UPDATE, m_enableUpdate );
            else
            {
                aPopup.InsertItem( CMD_IGNORE_UPDATE, m_ignoreUpdate );
                aPopup.InsertItem( CMD_IGNORE_ALL_UPDATES, m_ignoreAllUpdates );
            }

            sal_uInt16 aCmd = aPopup.Execute( this, rPos );
            if ( ( aCmd == CMD_IGNORE_UPDATE ) || ( aCmd == CMD_IGNORE_ALL_UPDATES ) )
            {
                p->m_bIgnored = true;
                if ( p->m_eKind == ENABLED_UPDATE )
                {
                    RemoveEntry( nEntryPos );
                    m_dialog.addAdditional( p, SvLBoxButtonKind_disabledCheckbox );
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
                    m_dialog.insertItem( p, SvLBoxButtonKind_enabledCheckbox );
                }
                m_dialog.setIgnoredUpdate( p, false, false );
            }
        }
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
sal_uInt16 UpdateDialog::insertItem( UpdateDialog::Index *pEntry, SvLBoxButtonKind kind )
{
    m_updates.InsertEntry( pEntry->m_aName, LISTBOX_APPEND, static_cast< void * >( pEntry ), kind );

    for ( sal_uInt16 i = m_updates.getItemCount(); i != 0 ; )
    {
        i -= 1;
        UpdateDialog::Index const * p = static_cast< UpdateDialog::Index const * >( m_updates.GetEntryData( i ) );
        if ( p == pEntry )
            return i;
    }
    OSL_ASSERT(0);
    return 0;
}

//------------------------------------------------------------------------------
void UpdateDialog::addAdditional( UpdateDialog::Index * index, SvLBoxButtonKind kind )
{
    m_all.Enable();
    if (m_all.IsChecked())
    {
        insertItem( index, kind );
        m_update.Enable();
        m_updates.Enable();
        m_description.Enable();
        m_descriptions.Enable();
    }
}

//------------------------------------------------------------------------------
void UpdateDialog::addEnabledUpdate( OUString const & name,
                                     dp_gui::UpdateData & data )
{
    sal_uInt16 nIndex = sal::static_int_cast< sal_uInt16 >( m_enabledUpdates.size() );
    UpdateDialog::Index *pEntry = new UpdateDialog::Index( ENABLED_UPDATE, m_nLastID, nIndex, name );

    data.m_nID = m_nLastID;
    m_nLastID += 1;

    m_enabledUpdates.push_back( data );
    m_ListboxEntries.push_back( pEntry );

    if ( ! isIgnoredUpdate( pEntry ) )
    {
        sal_uInt16 nPos = insertItem( pEntry, SvLBoxButtonKind_enabledCheckbox );
        m_updates.CheckEntryPos( nPos );
    }
    else
        addAdditional( pEntry, SvLBoxButtonKind_disabledCheckbox );

    m_update.Enable();
    m_updates.Enable();
    m_description.Enable();
    m_descriptions.Enable();
}

//------------------------------------------------------------------------------
void UpdateDialog::addDisabledUpdate( UpdateDialog::DisabledUpdate & data )
{
    sal_uInt16 nIndex = sal::static_int_cast< sal_uInt16 >( m_disabledUpdates.size() );
    UpdateDialog::Index *pEntry = new UpdateDialog::Index( DISABLED_UPDATE, m_nLastID, nIndex, data.name );

    data.m_nID = m_nLastID;
    m_nLastID += 1;

    m_disabledUpdates.push_back( data );
    m_ListboxEntries.push_back( pEntry );

    isIgnoredUpdate( pEntry );
    addAdditional( pEntry, SvLBoxButtonKind_disabledCheckbox );
}

//------------------------------------------------------------------------------
void UpdateDialog::addSpecificError( UpdateDialog::SpecificError & data )
{
    sal_uInt16 nIndex = sal::static_int_cast< sal_uInt16 >( m_specificErrors.size() );
    UpdateDialog::Index *pEntry = new UpdateDialog::Index( SPECIFIC_ERROR, m_nLastID, nIndex, data.name );

    data.m_nID = m_nLastID;
    m_nLastID += 1;

    m_specificErrors.push_back( data );
    m_ListboxEntries.push_back( pEntry );

    addAdditional( pEntry, SvLBoxButtonKind_staticImage);
}

void UpdateDialog::checkingDone() {
    m_checking.Hide();
    m_throbber.stop();
    m_throbber.Hide();
    if (m_updates.getItemCount() == 0)
    {
        clearDescription();
        m_description.Enable();
        m_descriptions.Enable();

        if ( m_disabledUpdates.empty() && m_specificErrors.empty() && m_ignoredUpdates.empty() )
            showDescription( m_none, false );
        else
            showDescription( m_noInstallable, false );
    }

    enableOk();
}

void UpdateDialog::enableOk() {
    if (!m_checking.IsVisible()) {
        m_ok.Enable(m_updates.GetCheckedEntryCount() != 0);
    }
}

// *********************************************************************************
void UpdateDialog::createNotifyJob( bool bPrepareOnly,
    uno::Sequence< uno::Sequence< OUString > > &rItemList )
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
        aProperty.Value = uno::makeAny( OUString("org.openoffice.Office.Addons/AddonUI/OfficeHelp/UpdateCheckJob") );

        uno::Sequence< uno::Any > aArgumentList( 1 );
        aArgumentList[0] = uno::makeAny( aProperty );

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
            aProperty.Value = uno::makeAny( rItemList );
            aPropList[0] = aProperty;
            aProperty.Name  = "prepareOnly";
            aProperty.Value = uno::makeAny( bPrepareOnly );
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
        for ( sal_Int16 i = 0; i < m_updates.getItemCount(); ++i )
        {
            uno::Sequence< OUString > aItem(2);

            UpdateDialog::Index const * p = static_cast< UpdateDialog::Index const * >(m_updates.GetEntryData(i));

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
    m_PublisherLabel.Hide();
    m_PublisherLink.Hide();
    m_ReleaseNotesLabel.Hide();
    m_ReleaseNotesLink.Hide();
    m_descriptions.Hide();

    Link aLink = LINK( this, UpdateDialog, hyperlink_clicked );
    m_PublisherLink.SetClickHdl( aLink );
    m_ReleaseNotesLink.SetClickHdl( aLink );

    long nTextWidth = m_PublisherLabel.GetCtrlTextWidth( m_PublisherLabel.GetText() );
    long nTemp = m_ReleaseNotesLabel.GetTextWidth( m_ReleaseNotesLabel.GetText() );
    if ( nTemp > nTextWidth )
        nTextWidth = nTemp;
    nTextWidth = nTextWidth * 110 / 100;

    Size aNewSize = m_PublisherLabel.GetSizePixel();
    if ( nTextWidth > aNewSize.Width() )
    {
        long nDelta = nTextWidth - aNewSize.Width();
        aNewSize.Width() = nTextWidth;
        m_PublisherLabel.SetSizePixel( aNewSize );
        m_ReleaseNotesLabel.SetSizePixel( aNewSize );

        aNewSize = m_PublisherLink.GetSizePixel();
        aNewSize.Width() = aNewSize.Width() - nDelta;
        Point aNewPos = m_PublisherLink.GetPosPixel();
        aNewPos.X() = aNewPos.X() + nDelta;
        m_PublisherLink.SetPosSizePixel( aNewPos, aNewSize );
        aNewPos.Y() = m_ReleaseNotesLink.GetPosPixel().Y();
        m_ReleaseNotesLink.SetPosSizePixel( aNewPos, aNewSize );
    }

    m_aFirstLinePos = m_descriptions.GetPosPixel();
    m_aFirstLineSize = m_descriptions.GetSizePixel();
    Size aMarginSize = LogicToPixel( Size( RSC_SP_CTRL_GROUP_X, RSC_SP_CTRL_GROUP_Y ), MAP_APPFONT );
    Point aThirdLinePos = m_ReleaseNotesLabel.GetPosPixel();
    aThirdLinePos.Y() = aThirdLinePos.Y() + m_ReleaseNotesLabel.GetSizePixel().Height() + aMarginSize.Height();
    m_nFirstLineDelta = aThirdLinePos.Y() - m_aFirstLinePos.Y();
    m_nOneLineMissing = m_ReleaseNotesLabel.GetPosPixel().Y() - m_PublisherLabel.GetPosPixel().Y();
}

void UpdateDialog::clearDescription()
{
    OUString sEmpty;
    m_PublisherLabel.Hide();
    m_PublisherLink.Hide();
    m_PublisherLink.SetText( sEmpty );
    m_PublisherLink.SetURL( sEmpty );
    m_ReleaseNotesLabel.Hide();
    m_ReleaseNotesLink.Hide();
    m_ReleaseNotesLink.SetURL( sEmpty );
    if ( m_PublisherLabel.GetPosPixel().Y() == m_ReleaseNotesLabel.GetPosPixel().Y() )
    {
        Point aNewPos = m_ReleaseNotesLabel.GetPosPixel();
        aNewPos.Y() += m_nOneLineMissing;
        m_ReleaseNotesLabel.SetPosPixel( aNewPos );
        aNewPos = m_ReleaseNotesLink.GetPosPixel();
        aNewPos.Y() += m_nOneLineMissing;
        m_ReleaseNotesLink.SetPosPixel( aNewPos );
    }
    m_descriptions.Hide();
    m_descriptions.Clear();
    m_descriptions.SetPosSizePixel( m_aFirstLinePos, m_aFirstLineSize );
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

    bool bPublisher = false;
    if ( !sPub.isEmpty() )
    {
        m_PublisherLabel.Show();
        m_PublisherLink.Show();
        m_PublisherLink.SetText( sPub );
        m_PublisherLink.SetURL( sURL );
        bPublisher = true;
    }

    if ( !sReleaseNotes.isEmpty() )
    {
        if ( !bPublisher )
        {
            m_ReleaseNotesLabel.SetPosPixel( m_PublisherLabel.GetPosPixel() );
            m_ReleaseNotesLink.SetPosPixel( m_PublisherLink.GetPosPixel() );
        }
        m_ReleaseNotesLabel.Show();
        m_ReleaseNotesLink.Show();
        m_ReleaseNotesLink.SetURL( sReleaseNotes );
    }
    return true;
}

bool UpdateDialog::showDescription( const OUString& rDescription, bool bWithPublisher )
{
    if ( rDescription.isEmpty() )
        // nothing to show
        return false;

    if ( bWithPublisher )
    {
        bool bOneLineMissing = !m_ReleaseNotesLabel.IsVisible() || !m_PublisherLabel.IsVisible();
        Point aNewPos = m_aFirstLinePos;
        aNewPos.Y() += m_nFirstLineDelta;
        if ( bOneLineMissing )
            aNewPos.Y() -= m_nOneLineMissing;
        Size aNewSize = m_aFirstLineSize;
        aNewSize.Height() -= m_nFirstLineDelta;
        if ( bOneLineMissing )
            aNewSize.Height() += m_nOneLineMissing;
        m_descriptions.SetPosSizePixel( aNewPos, aNewSize );
    }
    m_descriptions.Show();
    m_descriptions.SetText( rDescription );
    return true;
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------

IMPL_LINK_NOARG(UpdateDialog, selectionHandler)
{
    OUStringBuffer b;
    bool bInserted = false;
    UpdateDialog::Index const * p = static_cast< UpdateDialog::Index const * >(
        m_updates.GetEntryData(m_updates.GetSelectEntryPos()));
    clearDescription();

    if ( p != NULL )
    {
        sal_uInt16 pos = p->m_nIndex;

        switch (p->m_eKind)
        {
            case ENABLED_UPDATE:
            {
                if ( m_enabledUpdates[ pos ].aUpdateSource.is() )
                    bInserted = showDescription( m_enabledUpdates[ pos ].aUpdateSource );
                else
                    bInserted = showDescription( m_enabledUpdates[ pos ].aUpdateInfo );

                if ( p->m_bIgnored )
                    b.append( m_ignoredUpdate );

                break;
            }
            case DISABLED_UPDATE:
            {
                if ( !m_disabledUpdates.empty() )
                    bInserted = showDescription( m_disabledUpdates[pos].aUpdateInfo );

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
                        b.appendAscii("  ");
                            // U+2003 EM SPACE would be better than two spaces,
                            // but some fonts do not contain it
                        b.append(
                            confineToParagraph(
                                data.unsatisfiedDependencies[i]));
                    }
                    b.append(LF);
                    b.appendAscii("  ");
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

    showDescription( b.makeStringAndClear(), bInserted );
    return 0;
}

IMPL_LINK_NOARG(UpdateDialog, allHandler)
{
    if (m_all.IsChecked())
    {
        m_update.Enable();
        m_updates.Enable();
        m_description.Enable();
        m_descriptions.Enable();

        for (std::vector< UpdateDialog::Index* >::iterator i( m_ListboxEntries.begin() );
             i != m_ListboxEntries.end(); ++i )
        {
            if ( (*i)->m_bIgnored || ( (*i)->m_eKind != ENABLED_UPDATE ) )
                insertItem( (*i), SvLBoxButtonKind_disabledCheckbox );
        }
    }
    else
    {
        for ( sal_uInt16 i = 0; i < m_updates.getItemCount(); )
        {
            UpdateDialog::Index const * p = static_cast< UpdateDialog::Index const * >( m_updates.GetEntryData(i) );
            if ( p->m_bIgnored || ( p->m_eKind != ENABLED_UPDATE ) )
            {
                m_updates.RemoveEntry(i);
            } else {
                ++i;
            }
        }

        if (m_updates.getItemCount() == 0)
        {
            clearDescription();
            m_update.Disable();
            m_updates.Disable();
            if (m_checking.IsVisible())
                m_description.Disable();
            else
                showDescription(m_noInstallable,false);
        }
    }
    return 0;
}

IMPL_LINK_NOARG(UpdateDialog, okHandler)
{
    //If users are going to update a shared extension then we need
    //to warn them
    typedef ::std::vector<UpdateData>::const_iterator CIT;
    for (CIT i = m_enabledUpdates.begin(); i < m_enabledUpdates.end(); ++i)
    {
        OSL_ASSERT(i->aInstalledPackage.is());
        //If the user has no write access to the shared folder then the update
        //for a shared extension is disable, that is it cannot be in m_enabledUpdates
    }


    for (sal_uInt16 i = 0; i < m_updates.getItemCount(); ++i) {
        UpdateDialog::Index const * p =
            static_cast< UpdateDialog::Index const * >(
                m_updates.GetEntryData(i));
        if (p->m_eKind == ENABLED_UPDATE && m_updates.IsChecked(i)) {
            m_updateData.push_back( m_enabledUpdates[ p->m_nIndex ] );
        }
    }

    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK_NOARG(UpdateDialog, closeHandler) {
    m_thread->stop();
    EndDialog(RET_CANCEL);
    return 0;
}

IMPL_LINK( UpdateDialog, hyperlink_clicked, FixedHyperlink*, pHyperlink )
{
    OUString sURL;
    if ( pHyperlink )
        sURL = OUString( pHyperlink->GetURL() );
    if ( sURL.isEmpty() )
        return 0;

    try
    {
        uno::Reference< com::sun::star::system::XSystemShellExecute > xSystemShellExecute(
            com::sun::star::system::SystemShellExecute::create(m_context) );
        //throws lang::IllegalArgumentException, system::SystemShellExecuteException
        xSystemShellExecute->execute( sURL, OUString(), com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY);
    }
    catch ( const uno::Exception& )
    {
    }

    return 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
