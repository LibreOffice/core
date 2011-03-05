/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

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
#include "com/sun/star/awt/XThrobber.hpp"
#include "com/sun/star/awt/XToolkit.hpp"
#include "com/sun/star/awt/XWindow.hpp"
#include "com/sun/star/awt/XWindowPeer.hpp"
#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/deployment/DeploymentException.hpp"
#include "com/sun/star/deployment/UpdateInformationProvider.hpp"
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/deployment/XExtensionManager.hpp"
#include "com/sun/star/deployment/ExtensionManager.hpp"
#include "com/sun/star/deployment/XUpdateInformationProvider.hpp"
#include "com/sun/star/frame/XDesktop.hpp"
#include "com/sun/star/frame/XDispatch.hpp"
#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/system/SystemShellExecuteFlags.hpp"
#include "com/sun/star/system/XSystemShellExecute.hpp"
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
#include "com/sun/star/util/XURLTransformer.hpp"
#include "com/sun/star/xml/dom/XElement.hpp"
#include "com/sun/star/xml/dom/XNode.hpp"
#include "osl/diagnose.h"
#include "rtl/bootstrap.hxx"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "svtools/svlbitm.hxx"
#include "svtools/svlbox.hxx"
#include <svtools/controldims.hrc>
#include "svx/checklbx.hxx"
#include "tools/gen.hxx"
#include "tools/link.hxx"
#include "tools/resid.hxx"
#include "tools/resmgr.hxx"
#include "tools/solar.h"
#include "tools/string.hxx"
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
#include "dp_gui_thread.hxx"
#include "dp_gui_updatedata.hxx"
#include "dp_gui_updatedialog.hxx"
#include "dp_gui_shared.hxx"

class KeyEvent;
class MouseEvent;
class Window;
namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace css = ::com::sun::star;

using dp_gui::UpdateDialog;

namespace {

static sal_Unicode const LF = 0x000A;
static sal_Unicode const CR = 0x000D;

enum Kind { ENABLED_UPDATE, DISABLED_UPDATE, GENERAL_ERROR, SPECIFIC_ERROR };

rtl::OUString confineToParagraph(rtl::OUString const & text) {
    // Confine arbitrary text to a single paragraph in a dp_gui::AutoScrollEdit.
    // This assumes that U+000A and U+000D are the only paragraph separators in
    // a dp_gui::AutoScrollEdit, and that replacing them with a single space
    // each is acceptable:
    return text.replace(LF, ' ').replace(CR, ' ');
}
}

struct UpdateDialog::DisabledUpdate {
    rtl::OUString name;
    css::uno::Sequence< rtl::OUString > unsatisfiedDependencies;
    // We also want to show release notes and publisher for disabled updates
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::dom::XNode > aUpdateInfo;
};

struct UpdateDialog::SpecificError {
    rtl::OUString name;
    rtl::OUString message;
};

union UpdateDialog::IndexUnion{
    std::vector< dp_gui::UpdateData >::size_type enabledUpdate;
    std::vector< UpdateDialog::DisabledUpdate >::size_type disabledUpdate;
    std::vector< rtl::OUString >::size_type generalError;
    std::vector< UpdateDialog::SpecificError >::size_type specificError;
};

struct UpdateDialog::Index {
    static std::auto_ptr< UpdateDialog::Index const > newEnabledUpdate(
        std::vector< dp_gui::UpdateData >::size_type n);

    static std::auto_ptr< UpdateDialog::Index const > newDisabledUpdate(
        std::vector< UpdateDialog::DisabledUpdate >::size_type n);

    static std::auto_ptr< UpdateDialog::Index const > newGeneralError(
        std::vector< rtl::OUString >::size_type n);

    static std::auto_ptr< UpdateDialog::Index const > newSpecificError(
        std::vector< UpdateDialog::SpecificError >::size_type n);

    Kind kind;
    IndexUnion index;

private:
    explicit Index(Kind theKind);
};

std::auto_ptr< UpdateDialog::Index const >
UpdateDialog::Index::newEnabledUpdate(
    std::vector< dp_gui::UpdateData >::size_type n)
{
    UpdateDialog::Index * p = new UpdateDialog::Index(ENABLED_UPDATE);
    p->index.enabledUpdate = n;
    return std::auto_ptr< UpdateDialog::Index const >(p);
}

std::auto_ptr< UpdateDialog::Index const >
UpdateDialog::Index::newDisabledUpdate(
    std::vector< UpdateDialog::DisabledUpdate >::size_type n)
{
    UpdateDialog::Index * p = new UpdateDialog::Index(DISABLED_UPDATE);
    p->index.disabledUpdate = n;
    return std::auto_ptr< UpdateDialog::Index const >(p);
}

std::auto_ptr< UpdateDialog::Index const > UpdateDialog::Index::newGeneralError(
    std::vector< rtl::OUString >::size_type n)
{
    UpdateDialog::Index * p = new UpdateDialog::Index(GENERAL_ERROR);
    p->index.generalError = n;
    return std::auto_ptr< UpdateDialog::Index const >(p);
}

std::auto_ptr< UpdateDialog::Index const >
UpdateDialog::Index::newSpecificError(
    std::vector< UpdateDialog::SpecificError >::size_type n)
{
    UpdateDialog::Index * p = new UpdateDialog::Index(SPECIFIC_ERROR);
    p->index.specificError = n;
    return std::auto_ptr< UpdateDialog::Index const >(p);
}

UpdateDialog::Index::Index(Kind theKind): kind(theKind) {}

class UpdateDialog::Thread: public dp_gui::Thread {
public:
    Thread(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        UpdateDialog & dialog,
        const std::vector< css::uno::Reference< css::deployment::XPackage >  > & vExtensionList);

    void stop();

private:
    Thread(UpdateDialog::Thread &); // not defined
    void operator =(UpdateDialog::Thread &); // not defined

    virtual ~Thread();

    virtual void execute();

    void handleSpecificError(
        css::uno::Reference< css::deployment::XPackage > const & package,
        css::uno::Any const & exception) const;

    css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >
    getUpdateInformation(
        css::uno::Reference< css::deployment::XPackage > const & package,
        css::uno::Sequence< rtl::OUString > const & urls,
        rtl::OUString const & identifier) const;

    ::rtl::OUString getUpdateDisplayString(
        dp_gui::UpdateData const & data, ::rtl::OUString const & version = ::rtl::OUString()) const;

    void prepareUpdateData(
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::dom::XNode > const & updateInfo,
        UpdateDialog::DisabledUpdate & out_du,
        dp_gui::UpdateData & out_data) const;

    bool update(
    UpdateDialog::DisabledUpdate const & du,
    dp_gui::UpdateData const & data) const;

    css::uno::Reference< css::uno::XComponentContext > m_context;
    UpdateDialog & m_dialog;
    std::vector< css::uno::Reference< css::deployment::XPackage > > m_vExtensionList;
    css::uno::Reference< css::deployment::XUpdateInformationProvider > m_updateInformation;
    css::uno::Reference< css::task::XInteractionHandler > m_xInteractionHdl;

    // guarded by Application::GetSolarMutex():
    css::uno::Reference< css::task::XAbortChannel > m_abort;
    bool m_stop;
};

UpdateDialog::Thread::Thread(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    UpdateDialog & dialog,
    const std::vector< css::uno::Reference< css::deployment::XPackage > > &vExtensionList):
    m_context(context),
    m_dialog(dialog),
    m_vExtensionList(vExtensionList),
    m_updateInformation(
        css::deployment::UpdateInformationProvider::create(context)),
    m_stop(false)
{
    if( m_context.is() )
    {
        css::uno::Reference< css::lang::XMultiComponentFactory > xServiceManager( m_context->getServiceManager() );

        if( xServiceManager.is() )
        {
            m_xInteractionHdl = css::uno::Reference< css::task::XInteractionHandler > (
                                xServiceManager->createInstanceWithContext( OUSTR( "com.sun.star.task.InteractionHandler" ), m_context),
                                css::uno::UNO_QUERY );
            if ( m_xInteractionHdl.is() )
                m_updateInformation->setInteractionHandler( m_xInteractionHdl );
        }
    }
}

void UpdateDialog::Thread::stop() {
    css::uno::Reference< css::task::XAbortChannel > abort;
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
        m_updateInformation->setInteractionHandler( css::uno::Reference< css::task::XInteractionHandler > () );
}

void UpdateDialog::Thread::execute()
{
    {
        SolarMutexGuard g;
        if ( m_stop ) {
            return;
        }
    }
    css::uno::Reference<css::deployment::XExtensionManager> extMgr =
        css::deployment::ExtensionManager::get(m_context);

    std::vector<std::pair<css::uno::Reference<css::deployment::XPackage>, css::uno::Any > > errors;

    dp_misc::UpdateInfoMap updateInfoMap = dp_misc::getOnlineUpdateInfos(
        m_context, extMgr, m_updateInformation, &m_vExtensionList, errors);

    typedef std::vector<std::pair<css::uno::Reference<css::deployment::XPackage>,
        css::uno::Any> >::const_iterator ITERROR;
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
        rtl::OUString sOnlineVersion;
        if (info.info.is())
            sOnlineVersion = info.version;
        rtl::OUString sVersionUser;
        rtl::OUString sVersionShared;
        rtl::OUString sVersionBundled;
        css::uno::Sequence< css::uno::Reference< css::deployment::XPackage> > extensions;
        try {
            extensions = extMgr->getExtensionsWithSameIdentifier(
                dp_misc::getIdentifier(info.extension), info.extension->getName(),
                css::uno::Reference<css::ucb::XCommandEnvironment>());
        } catch (css::lang::IllegalArgumentException& ) {
            OSL_ASSERT(0);
        }
        OSL_ASSERT(extensions.getLength() == 3);
        if (extensions[0].is() )
            sVersionUser = extensions[0]->getVersion();
        if (extensions[1].is() )
            sVersionShared = extensions[1]->getVersion();
        if (extensions[2].is() )
            sVersionBundled = extensions[2]->getVersion();

        bool bSharedReadOnly = extMgr->isReadOnlyRepository(OUSTR("shared"));

        dp_misc::UPDATE_SOURCE sourceUser = dp_misc::isUpdateUserExtension(
            bSharedReadOnly, sVersionUser, sVersionShared, sVersionBundled, sOnlineVersion);
        dp_misc::UPDATE_SOURCE sourceShared = dp_misc::isUpdateSharedExtension(
            bSharedReadOnly, sVersionShared, sVersionBundled, sOnlineVersion);

        css::uno::Reference<css::deployment::XPackage> updateSource;
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
    css::uno::Reference< css::deployment::XPackage > const & package,
    css::uno::Any const & exception) const
{
    UpdateDialog::SpecificError data;
    if (package.is())
        data.name = package->getDisplayName();
    css::uno::Exception e;
    if (exception >>= e) {
        data.message = e.Message;
    }
    SolarMutexGuard g;
    if (!m_stop) {
        m_dialog.addSpecificError(data);
    }
}

::rtl::OUString UpdateDialog::Thread::getUpdateDisplayString(
    dp_gui::UpdateData const & data, ::rtl::OUString const & version) const
{
    OSL_ASSERT(data.aInstalledPackage.is());
    rtl::OUStringBuffer b(data.aInstalledPackage->getDisplayName());
    b.append(static_cast< sal_Unicode >(' '));
    {
        SolarMutexGuard g;
        if(!m_stop)
            b.append(m_dialog.m_version);
    }
    b.append(static_cast< sal_Unicode >(' '));
    if (version.getLength())
        b.append(version);
    else
        b.append(data.updateVersion);

    if (data.sWebsiteURL.getLength())
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
    css::uno::Reference< css::xml::dom::XNode > const & updateInfo,
    UpdateDialog::DisabledUpdate & out_du,
    dp_gui::UpdateData & out_data) const
{
    if (!updateInfo.is())
        return;
    dp_misc::DescriptionInfoset infoset(m_context, updateInfo);
    OSL_ASSERT(infoset.getVersion().getLength() != 0);
    css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > > ds(
        dp_misc::Dependencies::check(infoset));

    out_du.aUpdateInfo = updateInfo;
    out_du.unsatisfiedDependencies.realloc(ds.getLength());
    for (sal_Int32 i = 0; i < ds.getLength(); ++i) {
        out_du.unsatisfiedDependencies[i] = dp_misc::Dependencies::getErrorText(ds[i]);
    }

    const ::boost::optional< ::rtl::OUString> updateWebsiteURL(infoset.getLocalizedUpdateWebsiteURL());

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
    css::uno::Reference< css::uno::XComponentContext > const & context,
    Window * parent,
    const std::vector<css::uno::Reference< css::deployment::XPackage > > &vExtensionList,
    std::vector< dp_gui::UpdateData > * updateData):
    ModalDialog(parent,DpGuiResId(RID_DLG_UPDATE)),
    m_context(context),
    m_checking(this, DpGuiResId(RID_DLG_UPDATE_CHECKING)),
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
    m_cancel(this, DpGuiResId(RID_DLG_UPDATE_CANCEL)),
    m_error(String(DpGuiResId(RID_DLG_UPDATE_ERROR))),
    m_none(String(DpGuiResId(RID_DLG_UPDATE_NONE))),
    m_noInstallable(String(DpGuiResId(RID_DLG_UPDATE_NOINSTALLABLE))),
    m_failure(String(DpGuiResId(RID_DLG_UPDATE_FAILURE))),
    m_unknownError(String(DpGuiResId(RID_DLG_UPDATE_UNKNOWNERROR))),
    m_noDescription(String(DpGuiResId(RID_DLG_UPDATE_NODESCRIPTION))),
    m_noInstall(String(DpGuiResId(RID_DLG_UPDATE_NOINSTALL))),
    m_noDependency(String(DpGuiResId(RID_DLG_UPDATE_NODEPENDENCY))),
    m_noDependencyCurVer(String(DpGuiResId(RID_DLG_UPDATE_NODEPENDENCY_CUR_VER))),
    m_browserbased(String(DpGuiResId(RID_DLG_UPDATE_BROWSERBASED))),
    m_version(String(DpGuiResId(RID_DLG_UPDATE_VERSION))),
    m_updateData(*updateData),
    m_thread(
        new UpdateDialog::Thread(
            context, *this, vExtensionList)),
    m_nFirstLineDelta(0),
    m_nOneLineMissing(0)
    // TODO: check!
//    ,
//    m_extensionManagerDialog(extensionManagerDialog)
{
    OSL_ASSERT(updateData != NULL);

    m_xExtensionManager = css::deployment::ExtensionManager::get( context );

    css::uno::Reference< css::awt::XToolkit > toolkit;
    try {
        toolkit = css::uno::Reference< css::awt::XToolkit >(
            (css::uno::Reference< css::lang::XMultiComponentFactory >(
                m_context->getServiceManager(),
                css::uno::UNO_QUERY_THROW)->
             createInstanceWithContext(
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.Toolkit")),
                 m_context)),
            css::uno::UNO_QUERY_THROW);
    } catch (css::uno::RuntimeException &) {
        throw;
    } catch (css::uno::Exception & e) {
        throw css::uno::RuntimeException(e.Message, e.Context);
    }
    Control c(this, DpGuiResId(RID_DLG_UPDATE_THROBBER));
    Point pos(c.GetPosPixel());
    Size size(c.GetSizePixel());
    try {
        m_throbber = css::uno::Reference< css::awt::XThrobber >(
            toolkit->createWindow(
                css::awt::WindowDescriptor(
                    css::awt::WindowClass_SIMPLE,
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Throbber")),
                    GetComponentInterface(), 0,
                    css::awt::Rectangle(
                        pos.X(), pos.Y(), size.Width(), size.Height()),
                    css::awt::WindowAttribute::SHOW)),
            css::uno::UNO_QUERY_THROW);
    } catch (css::lang::IllegalArgumentException & e) {
        throw css::uno::RuntimeException(e.Message, e.Context);
    }
    m_updates.SetSelectHdl(LINK(this, UpdateDialog, selectionHandler));
    m_all.SetToggleHdl(LINK(this, UpdateDialog, allHandler));
    m_ok.SetClickHdl(LINK(this, UpdateDialog, okHandler));
    m_cancel.SetClickHdl(LINK(this, UpdateDialog, cancelHandler));
    if ( ! dp_misc::office_is_running())
        m_help.Disable();
    FreeResource();

    initDescription();
}

UpdateDialog::~UpdateDialog() {
    for (USHORT i = 0; i < m_updates.getItemCount(); ++i) {
        delete static_cast< UpdateDialog::Index const * >(
            m_updates.GetEntryData(i));
    }
}

BOOL UpdateDialog::Close() {
    m_thread->stop();
    return ModalDialog::Close();
}

short UpdateDialog::Execute() {
    m_throbber->start();
    m_thread->launch();
    return ModalDialog::Execute();
}

UpdateDialog::CheckListBox::CheckListBox(
    UpdateDialog & dialog, ResId const & resource,
    Image const & normalStaticImage):
    SvxCheckListBox(
        &dialog, resource, normalStaticImage),
    m_dialog(dialog)
{}

UpdateDialog::CheckListBox::~CheckListBox() {}

USHORT UpdateDialog::CheckListBox::getItemCount() const {
    ULONG i = GetEntryCount();
    OSL_ASSERT(i <= std::numeric_limits< USHORT >::max());
    return sal::static_int_cast< USHORT >(i);
}

void UpdateDialog::CheckListBox::MouseButtonDown(MouseEvent const & event) {
    // When clicking on a selected entry in an SvxCheckListBox, the entry's
    // checkbox is toggled on mouse button down:
    SvxCheckListBox::MouseButtonDown(event);
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

void UpdateDialog::insertItem(
    rtl::OUString const & name, USHORT position,
    std::auto_ptr< UpdateDialog::Index const > index, SvLBoxButtonKind kind)
{
    m_updates.InsertEntry(
        name, position,
        const_cast< void * >(static_cast< void const * >(index.release())),
        kind);
        //TODO #i72487#: UpdateDialog::Index potentially leaks as the exception
        // behavior of SvxCheckListBox::InsertEntry is unspecified
}

void UpdateDialog::addAdditional(
    rtl::OUString const & name, USHORT position,
    std::auto_ptr< UpdateDialog::Index const > index, SvLBoxButtonKind kind)
{
    m_all.Enable();
    if (m_all.IsChecked()) {
        insertItem(name, position, index, kind);
        m_update.Enable();
        m_updates.Enable();
        m_description.Enable();
        m_descriptions.Enable();
    }
}

void UpdateDialog::addEnabledUpdate(
    rtl::OUString const & name, dp_gui::UpdateData const & data)
{
    std::vector< dp_gui::UpdateData >::size_type n = m_enabledUpdates.size();
    m_enabledUpdates.push_back(data);
    insertItem(
        name, sal::static_int_cast< USHORT >(n),
        UpdateDialog::Index::newEnabledUpdate(n),
        SvLBoxButtonKind_enabledCheckbox);
        // position overflow is rather harmless
    m_updates.CheckEntryPos(sal::static_int_cast< USHORT >(n));
        //TODO #i72487#: fragile computation; insertItem should instead return
        // pos
    m_update.Enable();
    m_updates.Enable();
    m_description.Enable();
    m_descriptions.Enable();
}

void UpdateDialog::addDisabledUpdate(UpdateDialog::DisabledUpdate const & data)
{
    std::vector< UpdateDialog::DisabledUpdate >::size_type n =
        m_disabledUpdates.size();
    m_disabledUpdates.push_back(data);
    addAdditional(
        data.name, sal::static_int_cast< USHORT >(m_enabledUpdates.size() + n),
        UpdateDialog::Index::newDisabledUpdate(n),
        SvLBoxButtonKind_disabledCheckbox);
        // position overflow is rather harmless
}

void UpdateDialog::addSpecificError(UpdateDialog::SpecificError const & data) {
    std::vector< UpdateDialog::SpecificError >::size_type n =
        m_specificErrors.size();
    m_specificErrors.push_back(data);
    addAdditional(
        data.name, LISTBOX_APPEND, UpdateDialog::Index::newSpecificError(n),
        SvLBoxButtonKind_staticImage);
}

void UpdateDialog::checkingDone() {
    m_checking.Hide();
    m_throbber->stop();
    css::uno::Reference< css::awt::XWindow >(
        m_throbber, css::uno::UNO_QUERY_THROW)->setVisible(false);
    if (m_updates.getItemCount() == 0)
    {
        clearDescription();
        m_description.Enable();
        m_descriptions.Enable();
        showDescription(
            ( m_disabledUpdates.empty() && m_generalErrors.empty() && m_specificErrors.empty() )
                ? m_none : m_noInstallable, false );
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
    css::uno::Sequence< css::uno::Sequence< rtl::OUString > > &rItemList )
{
    if ( !dp_misc::office_is_running() )
        return;

    // notify update check job
    try
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
        css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider(
            xFactory->createInstance( OUSTR( "com.sun.star.configuration.ConfigurationProvider" )),
            css::uno::UNO_QUERY_THROW);

        css::beans::PropertyValue aProperty;
        aProperty.Name  = OUSTR( "nodepath" );
        aProperty.Value = css::uno::makeAny( OUSTR("org.openoffice.Office.Addons/AddonUI/OfficeHelp/UpdateCheckJob") );

        css::uno::Sequence< css::uno::Any > aArgumentList( 1 );
        aArgumentList[0] = css::uno::makeAny( aProperty );

        css::uno::Reference< css::container::XNameAccess > xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                OUSTR("com.sun.star.configuration.ConfigurationAccess"), aArgumentList ),
            css::uno::UNO_QUERY_THROW );

        css::util::URL aURL;
        xNameAccess->getByName(OUSTR("URL")) >>= aURL.Complete;

        css::uno::Reference < css::util::XURLTransformer > xTransformer( xFactory->createInstance( OUSTR( "com.sun.star.util.URLTransformer" ) ),
            css::uno::UNO_QUERY_THROW );

        xTransformer->parseStrict(aURL);

        css::uno::Reference < css::frame::XDesktop > xDesktop( xFactory->createInstance( OUSTR( "com.sun.star.frame.Desktop" ) ),
            css::uno::UNO_QUERY_THROW );
        css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( xDesktop->getCurrentFrame(),
            css::uno::UNO_QUERY_THROW );
        css::uno::Reference< css::frame::XDispatch > xDispatch = xDispatchProvider->queryDispatch(aURL, rtl::OUString(), 0);

        if( xDispatch.is() )
        {
            css::uno::Sequence< css::beans::PropertyValue > aPropList(2);
            aProperty.Name  = OUSTR( "updateList" );
            aProperty.Value = css::uno::makeAny( rItemList );
            aPropList[0] = aProperty;
            aProperty.Name  = OUSTR( "prepareOnly" );
            aProperty.Value = css::uno::makeAny( bPrepareOnly );
            aPropList[1] = aProperty;

            xDispatch->dispatch(aURL, aPropList );
        }
    }
    catch( const css::uno::Exception& e )
    {
        dp_misc::TRACE( OUSTR("Caught exception: ")
            + e.Message + OUSTR("\n thread terminated.\n\n"));
    }
}

// *********************************************************************************
void UpdateDialog::notifyMenubar( bool bPrepareOnly, bool bRecheckOnly )
{
    if ( !dp_misc::office_is_running() )
        return;

    css::uno::Sequence< css::uno::Sequence< rtl::OUString > > aItemList;

    if ( ! bRecheckOnly )
    {
        sal_Int32 nCount = 0;
        for ( sal_Int16 i = 0; i < m_updates.getItemCount(); ++i )
        {
            css::uno::Sequence< rtl::OUString > aItem(2);

            UpdateDialog::Index const * p = static_cast< UpdateDialog::Index const * >(m_updates.GetEntryData(i));

            if ( p->kind == ENABLED_UPDATE )
            {
                dp_gui::UpdateData aUpdData = m_enabledUpdates[ p->index.enabledUpdate ];
                aItem[0] = dp_misc::getIdentifier( aUpdData.aInstalledPackage );

                dp_misc::DescriptionInfoset aInfoset( m_context, aUpdData.aUpdateInfo );
                aItem[1] = aInfoset.getVersion();
            }
            else if ( p->kind == DISABLED_UPDATE )
                continue;
            else
                continue;

            aItemList.realloc( nCount + 1 );
            aItemList[ nCount ] = aItem;
            nCount += 1;
        }
    }
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
    String sEmpty;
    m_PublisherLabel.Hide();
    m_PublisherLink.Hide();
    m_PublisherLink.SetDescription( sEmpty );
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

bool UpdateDialog::showDescription(css::uno::Reference< css::xml::dom::XNode > const & aUpdateInfo)
{
    dp_misc::DescriptionInfoset infoset(m_context, aUpdateInfo);
    return showDescription(infoset.getLocalizedPublisherNameAndURL(),
                           infoset.getLocalizedReleaseNotesURL());
}

bool UpdateDialog::showDescription(css::uno::Reference< css::deployment::XPackage > const & aExtension)
{
    OSL_ASSERT(aExtension.is());
    css::beans::StringPair pubInfo = aExtension->getPublisherInfo();
    return showDescription(std::make_pair(pubInfo.First, pubInfo.Second),
                           OUSTR(""));
}

bool UpdateDialog::showDescription(std::pair< rtl::OUString, rtl::OUString > const & pairPublisher,
                                   rtl::OUString const & sReleaseNotes)
{
    rtl::OUString sPub = pairPublisher.first;
    rtl::OUString sURL = pairPublisher.second;

    if ( sPub.getLength() == 0 && sURL.getLength() == 0 && sReleaseNotes.getLength() == 0 )
        // nothing to show
        return false;

    bool bPublisher = false;
    if ( sPub.getLength() > 0 )
    {
        m_PublisherLabel.Show();
        m_PublisherLink.Show();
        m_PublisherLink.SetDescription( sPub );
        m_PublisherLink.SetURL( sURL );
        bPublisher = true;
    }

    if ( sReleaseNotes.getLength() > 0 )
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

bool UpdateDialog::showDescription( const String& rDescription, bool bWithPublisher )
{
    if ( rDescription.Len() == 0 )
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
    m_descriptions.SetDescription( rDescription );
    return true;
}

IMPL_LINK(UpdateDialog, selectionHandler, void *, EMPTYARG)
{
    rtl::OUStringBuffer b;
    bool bInserted = false;
    UpdateDialog::Index const * p = static_cast< UpdateDialog::Index const * >(
        m_updates.GetEntryData(m_updates.GetSelectEntryPos()));
    clearDescription();

    if (p != NULL)
    {
        //When the index is greater or equal than the amount of enabled updates then the "Show all"
        //button is probably checked. Then we show first all enabled and then the disabled
        //updates.
        USHORT pos = m_updates.GetSelectEntryPos();
        const std::vector< dp_gui::UpdateData >::size_type sizeEnabled =
            m_enabledUpdates.size();
        const std::vector< UpdateDialog::DisabledUpdate >::size_type sizeDisabled =
            m_disabledUpdates.size();
        if (pos < sizeEnabled)
        {
            if (m_enabledUpdates[pos].aUpdateSource.is())
                bInserted = showDescription(m_enabledUpdates[pos].aUpdateSource);
            else
                bInserted = showDescription(m_enabledUpdates[pos].aUpdateInfo);
        }
        else if (pos >= sizeEnabled
            && pos < (sizeEnabled + sizeDisabled))
            bInserted = showDescription(m_disabledUpdates[pos - sizeEnabled].aUpdateInfo);

        switch (p->kind)
        {
            case ENABLED_UPDATE:
            {
                b.append(m_noDescription);
                break;
            }
            case DISABLED_UPDATE:
            {
                UpdateDialog::DisabledUpdate & data = m_disabledUpdates[
                    p->index.disabledUpdate];
                if (data.unsatisfiedDependencies.getLength() != 0)
                {
                    // create error string for version mismatch
                    ::rtl::OUString sVersion( RTL_CONSTASCII_USTRINGPARAM("%VERSION") );
                    ::rtl::OUString sProductName( RTL_CONSTASCII_USTRINGPARAM("%PRODUCTNAME") );
                    sal_Int32 nPos = m_noDependencyCurVer.indexOf( sVersion );
                    if ( nPos >= 0 )
                    {
                        ::rtl::OUString sCurVersion( RTL_CONSTASCII_USTRINGPARAM( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("version") ":Version:OOOPackageVersion}"));
                        ::rtl::Bootstrap::expandMacros(sCurVersion);
                        m_noDependencyCurVer = m_noDependencyCurVer.replaceAt( nPos, sVersion.getLength(), sCurVersion );
                    }
                    nPos = m_noDependencyCurVer.indexOf( sProductName );
                    if ( nPos >= 0 )
                    {
                        m_noDependencyCurVer = m_noDependencyCurVer.replaceAt( nPos, sProductName.getLength(), BrandName::get() );
                    }
                    nPos = m_noDependency.indexOf( sProductName );
                    if ( nPos >= 0 )
                    {
                        m_noDependency = m_noDependency.replaceAt( nPos, sProductName.getLength(), BrandName::get() );
                    }

                    b.append(m_noInstall);
                    b.append(LF);
                    b.append(m_noDependency);
                    for (sal_Int32 i = 0;
                         i < data.unsatisfiedDependencies.getLength(); ++i)
                    {
                        b.append(LF);
                        b.appendAscii(RTL_CONSTASCII_STRINGPARAM("  "));
                            // U+2003 EM SPACE would be better than two spaces,
                            // but some fonts do not contain it
                        b.append(
                            confineToParagraph(
                                data.unsatisfiedDependencies[i]));
                    }
                    b.append(LF);
                    b.appendAscii(RTL_CONSTASCII_STRINGPARAM("  "));
                    b.append(m_noDependencyCurVer);
                }
                break;
            }
            case GENERAL_ERROR:
            {
                rtl::OUString & msg = m_generalErrors[p->index.generalError];
                b.append(m_failure);
                b.append(LF);
                b.append(msg.getLength() == 0 ? m_unknownError : msg);
                break;
            }
            case SPECIFIC_ERROR:
            {
                UpdateDialog::SpecificError & data = m_specificErrors[
                    p->index.specificError];
                b.append(m_failure);
                b.append(LF);
                b.append(
                    data.message.getLength() == 0
                    ? m_unknownError : data.message);
                break;
            }
            default:
                OSL_ASSERT(false);
                break;
        }
    }

    showDescription( b.makeStringAndClear(), bInserted );
    return 0;
}

IMPL_LINK(UpdateDialog, allHandler, void *, EMPTYARG) {
    if (m_all.IsChecked()) {
        m_update.Enable();
        m_updates.Enable();
        m_description.Enable();
        m_descriptions.Enable();
        std::vector< UpdateDialog::DisabledUpdate >::size_type n1 = 0;
        for (std::vector< UpdateDialog::DisabledUpdate >::iterator i(
                 m_disabledUpdates.begin());
             i != m_disabledUpdates.end(); ++i)
        {
            insertItem(
                i->name, LISTBOX_APPEND,
                UpdateDialog::Index::newDisabledUpdate(n1++),
                SvLBoxButtonKind_disabledCheckbox);
        }
        std::vector< rtl::OUString >::size_type n2 = 0;
        for (std::vector< rtl::OUString >::iterator i(m_generalErrors.begin());
             i != m_generalErrors.end(); ++i)
        {
            insertItem(
                m_error, LISTBOX_APPEND,
                UpdateDialog::Index::newGeneralError(n2++),
                SvLBoxButtonKind_staticImage);
        }
        std::vector< UpdateDialog::SpecificError >::size_type n3 = 0;
        for (std::vector< UpdateDialog::SpecificError >::iterator i(
                 m_specificErrors.begin());
             i != m_specificErrors.end(); ++i)
        {
            insertItem(
                i->name, LISTBOX_APPEND,
                UpdateDialog::Index::newSpecificError(n3++),
                SvLBoxButtonKind_staticImage);
        }
    } else {
        for (USHORT i = 0; i < m_updates.getItemCount();) {
            UpdateDialog::Index const * p =
                static_cast< UpdateDialog::Index const * >(
                    m_updates.GetEntryData(i));
            if (p->kind != ENABLED_UPDATE) {
                m_updates.RemoveEntry(i);
                    //TODO #i72487#: UpdateDialog::Index potentially leaks as
                    // SvxCheckListBox::RemoveEntry's exception behavior is
                    // unspecified
                delete p;
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

IMPL_LINK(UpdateDialog, okHandler, void *, EMPTYARG)
{
    //If users are going to update a shared extension then we need
    //to warn them
    typedef ::std::vector<UpdateData>::const_iterator CIT;
    for (CIT i = m_enabledUpdates.begin(); i < m_enabledUpdates.end(); i++)
    {
        OSL_ASSERT(i->aInstalledPackage.is());
        //If the user has no write access to the shared folder then the update
        //for a shared extension is disable, that is it cannot be in m_enabledUpdates
    }


    for (USHORT i = 0; i < m_updates.getItemCount(); ++i) {
        UpdateDialog::Index const * p =
            static_cast< UpdateDialog::Index const * >(
                m_updates.GetEntryData(i));
        if (p->kind == ENABLED_UPDATE && m_updates.IsChecked(i)) {
            m_updateData.push_back(m_enabledUpdates[p->index.enabledUpdate]);
        }
    }

    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK(UpdateDialog, cancelHandler, void *, EMPTYARG) {
    m_thread->stop();
    EndDialog(RET_CANCEL);
    return 0;
}

IMPL_LINK( UpdateDialog, hyperlink_clicked, svt::FixedHyperlink*, pHyperlink )
{
    ::rtl::OUString sURL;
    if ( pHyperlink )
        sURL = ::rtl::OUString( pHyperlink->GetURL() );
    if ( sURL.getLength() == 0 )
        return 0;

    try
    {
        css::uno::Reference< css::system::XSystemShellExecute > xSystemShellExecute(
            m_context->getServiceManager()->createInstanceWithContext(
                OUSTR( "com.sun.star.system.SystemShellExecute" ),
                m_context), css::uno::UNO_QUERY_THROW);
        //throws css::lang::IllegalArgumentException, css::system::SystemShellExecuteException
        xSystemShellExecute->execute(
            sURL, ::rtl::OUString(), css::system::SystemShellExecuteFlags::DEFAULTS);
    }
    catch (css::uno::Exception& )
    {
    }

    return 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
