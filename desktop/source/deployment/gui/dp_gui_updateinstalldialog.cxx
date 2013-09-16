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


#include "dp_gui_updatedata.hxx"

#include "sal/config.h"
#include "osl/file.hxx"
#include "osl/conditn.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "tools/resid.hxx"
#include "tools/solar.h"
#include "tools/string.hxx"
#include "vcl/dialog.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/svapp.hxx"
#include "osl/mutex.hxx"
#include "cppuhelper/implbase3.hxx"

#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/xml/dom/XElement.hpp"
#include "com/sun/star/xml/dom/XNode.hpp"
#include "com/sun/star/xml/dom/XNodeList.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/ucb/InteractiveAugmentedIOException.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/ucb/XProgressHandler.hpp"
#include "com/sun/star/deployment/XExtensionManager.hpp"
#include "com/sun/star/deployment/ExtensionManager.hpp"
#include "com/sun/star/deployment/XUpdateInformationProvider.hpp"
#include "com/sun/star/deployment/DependencyException.hpp"
#include "com/sun/star/deployment/LicenseException.hpp"
#include "com/sun/star/deployment/VersionException.hpp"
#include "com/sun/star/deployment/ui/LicenseDialog.hpp"
#include "com/sun/star/task/XInteractionHandler.hpp"
#include "com/sun/star/ui/dialogs/XExecutableDialog.hpp"
#include "com/sun/star/ui/dialogs/ExecutableDialogResults.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"

#include "dp_descriptioninfoset.hxx"
#include "dp_gui.hrc"
#include "dp_gui_updateinstalldialog.hxx"
#include "dp_gui_shared.hxx"
#include "dp_ucb.h"
#include "dp_misc.h"
#include "dp_version.hxx"
#include "dp_gui_extensioncmdqueue.hxx"
#include "ucbhelper/content.hxx"
#include "rtl/ref.hxx"
#include "salhelper/thread.hxx"
#include "com/sun/star/uno/Sequence.h"
#include "comphelper/anytostring.hxx"
#include "toolkit/helper/vclunohelper.hxx"

#include <vector>

class Window;

namespace cssu = ::com::sun::star::uno;

using dp_misc::StrTitle;

namespace dp_gui {

class UpdateInstallDialog::Thread: public salhelper::Thread {
    friend class UpdateCommandEnv;
public:
    Thread(cssu::Reference< cssu::XComponentContext > ctx,
        UpdateInstallDialog & dialog, std::vector< dp_gui::UpdateData > & aVecUpdateData);

    void stop();



private:
    virtual ~Thread();

    virtual void execute();
    void downloadExtensions();
    void download(OUString const & aUrls, UpdateData & aUpdatData);
    void installExtensions();
    void removeTempDownloads();

    UpdateInstallDialog & m_dialog;
    cssu::Reference< css::deployment::XUpdateInformationProvider >
        m_updateInformation;

    // guarded by Application::GetSolarMutex():
    cssu::Reference< css::task::XAbortChannel > m_abort;
    cssu::Reference< cssu::XComponentContext > m_xComponentContext;
    std::vector< dp_gui::UpdateData > & m_aVecUpdateData;
    ::rtl::Reference<UpdateCommandEnv> m_updateCmdEnv;

    //A folder which is created in the temp directory in which then the updates are downloaded
    OUString m_sDownloadFolder;

    bool m_stop;

};

class UpdateCommandEnv
    : public ::cppu::WeakImplHelper3< css::ucb::XCommandEnvironment,
                                      css::task::XInteractionHandler,
                                      css::ucb::XProgressHandler >
{
    friend class UpdateInstallDialog::Thread;

    ::rtl::Reference<UpdateInstallDialog::Thread> m_installThread;
    cssu::Reference< cssu::XComponentContext > m_xContext;

public:
    virtual ~UpdateCommandEnv();
    UpdateCommandEnv( cssu::Reference< cssu::XComponentContext > const & xCtx,
        ::rtl::Reference<UpdateInstallDialog::Thread>const & thread);

    // XCommandEnvironment
    virtual cssu::Reference<css::task::XInteractionHandler > SAL_CALL
    getInteractionHandler() throw (cssu::RuntimeException);
    virtual cssu::Reference<css::ucb::XProgressHandler >
    SAL_CALL getProgressHandler() throw (cssu::RuntimeException);

    // XInteractionHandler
    virtual void SAL_CALL handle(
        cssu::Reference<css::task::XInteractionRequest > const & xRequest )
        throw (cssu::RuntimeException);

    // XProgressHandler
    virtual void SAL_CALL push( cssu::Any const & Status )
        throw (cssu::RuntimeException);
    virtual void SAL_CALL update( cssu::Any const & Status )
        throw (cssu::RuntimeException);
    virtual void SAL_CALL pop() throw (cssu::RuntimeException);
};


UpdateInstallDialog::Thread::Thread(
    cssu::Reference< cssu::XComponentContext> xCtx,
    UpdateInstallDialog & dialog,
    std::vector< dp_gui::UpdateData > & aVecUpdateData):
    salhelper::Thread("dp_gui_updateinstalldialog"),
    m_dialog(dialog),
    m_xComponentContext(xCtx),
    m_aVecUpdateData(aVecUpdateData),
    m_updateCmdEnv(new UpdateCommandEnv(xCtx, this)),
    m_stop(false)
{}

void UpdateInstallDialog::Thread::stop() {
    cssu::Reference< css::task::XAbortChannel > abort;
    {
        SolarMutexGuard g;
        abort = m_abort;
        m_stop = true;
    }
    if (abort.is()) {
        abort->sendAbort();
    }
}

UpdateInstallDialog::Thread::~Thread() {}

void UpdateInstallDialog::Thread::execute()
{
    try {
        downloadExtensions();
        installExtensions();
    }
    catch (...)
    {
    }

    //clean up the temp directories
    try {
        removeTempDownloads();
    } catch( ... ) {
    }

    {
        //make sure m_dialog is still alive
        SolarMutexGuard g;
        if (! m_stop)
             m_dialog.updateDone();
    }
    //UpdateCommandEnv keeps a reference to Thread and prevents destruction. Therefore remove it.
    m_updateCmdEnv->m_installThread.clear();
}


UpdateInstallDialog::UpdateInstallDialog(
    Window * parent,
    std::vector<dp_gui::UpdateData> & aVecUpdateData,
    cssu::Reference< cssu::XComponentContext > const & xCtx):
    ModalDialog(
        parent,
        DpGuiResId(RID_DLG_UPDATEINSTALL)),

        m_thread(new Thread(xCtx, *this, aVecUpdateData)),
        m_xComponentContext(xCtx),
        m_bError(false),
        m_bNoEntry(true),
        m_bActivated(false),
        m_sInstalling(OUString(DpGuiResId(RID_DLG_UPDATE_INSTALL_INSTALLING))),
        m_sFinished(OUString(DpGuiResId(RID_DLG_UPDATE_INSTALL_FINISHED))),
        m_sNoErrors(OUString(DpGuiResId(RID_DLG_UPDATE_INSTALL_NO_ERRORS))),
        m_sErrorDownload(OUString(DpGuiResId(RID_DLG_UPDATE_INSTALL_ERROR_DOWNLOAD))),
        m_sErrorInstallation(OUString(DpGuiResId(RID_DLG_UPDATE_INSTALL_ERROR_INSTALLATION))),
        m_sErrorLicenseDeclined(OUString(DpGuiResId(RID_DLG_UPDATE_INSTALL_ERROR_LIC_DECLINED))),
        m_sNoInstall(OUString(DpGuiResId(RID_DLG_UPDATE_INSTALL_EXTENSION_NOINSTALL))),
        m_sThisErrorOccurred(OUString(DpGuiResId(RID_DLG_UPDATE_INSTALL_THIS_ERROR_OCCURRED))),
        m_ft_action(this, DpGuiResId(RID_DLG_UPDATE_INSTALL_DOWNLOADING)),
        m_statusbar(this,DpGuiResId(RID_DLG_UPDATE_INSTALL_STATUSBAR)),
        m_ft_extension_name(this, DpGuiResId(RID_DLG_UPDATE_INSTALL_EXTENSION_NAME)),
        m_ft_results(this, DpGuiResId(RID_DLG_UPDATE_INSTALL_RESULTS)),
        m_mle_info(this, DpGuiResId(RID_DLG_UPDATE_INSTALL_INFO)),
        m_line(this, DpGuiResId(RID_DLG_UPDATE_INSTALL_LINE)),
        m_help(this, DpGuiResId(RID_DLG_UPDATE_INSTALL_HELP)),
        m_ok(this, DpGuiResId(RID_DLG_UPDATE_INSTALL_OK)),
        m_cancel(this, DpGuiResId(RID_DLG_UPDATE_INSTALL_ABORT))
{
    FreeResource();

    m_xExtensionManager = css::deployment::ExtensionManager::get( xCtx );

    m_cancel.SetClickHdl(LINK(this, UpdateInstallDialog, cancelHandler));
    m_mle_info.EnableCursor(sal_False);
    if ( ! dp_misc::office_is_running())
        m_help.Disable();
}

UpdateInstallDialog::~UpdateInstallDialog() {}

sal_Bool UpdateInstallDialog::Close()
{
    m_thread->stop();
    return ModalDialog::Close();
}

short UpdateInstallDialog::Execute()
{
    m_thread->launch();
    return ModalDialog::Execute();
}


// make sure the solar mutex is locked before calling
void UpdateInstallDialog::updateDone()
{
    if (!m_bError)
        m_mle_info.InsertText(m_sNoErrors);
    m_ok.Enable();
    m_ok.GrabFocus();
    m_cancel.Disable();
}
// make sure the solar mutex is locked before calling
//sets an error message in the text area
void UpdateInstallDialog::setError(INSTALL_ERROR err, OUString const & sExtension,
    OUString const & exceptionMessage)
{
    OUString sError;
    m_bError = true;

    switch (err)
    {
    case ERROR_DOWNLOAD:
        sError = m_sErrorDownload;
        break;
    case ERROR_INSTALLATION:
        sError = m_sErrorInstallation;
        break;
    case ERROR_LICENSE_DECLINED:
        sError = m_sErrorLicenseDeclined;
        break;

    default:
        OSL_ASSERT(0);
    }

    sError = sError.replaceFirst("%NAME", sExtension);
    //We want to have an empty line between the error messages. However,
    //there shall be no empty line after the last entry.
    if (m_bNoEntry)
        m_bNoEntry = false;
    else
        m_mle_info.InsertText(OUString("\n"));
    m_mle_info.InsertText(sError);
    //Insert more information about the error
    if (!exceptionMessage.isEmpty())
        m_mle_info.InsertText(m_sThisErrorOccurred + exceptionMessage + "\n");

    m_mle_info.InsertText(m_sNoInstall);
    m_mle_info.InsertText(OUString("\n"));
}

void UpdateInstallDialog::setError(OUString const & exceptionMessage)
{
    m_bError = true;
    m_mle_info.InsertText(exceptionMessage + "\n");
}

IMPL_LINK_NOARG(UpdateInstallDialog, cancelHandler)
{
    m_thread->stop();
    EndDialog(RET_CANCEL);
    return 0;
}

//------------------------------------------------------------------------------------------------

void UpdateInstallDialog::Thread::downloadExtensions()
{
    try
    {
        //create the download directory in the temp folder
        OUString sTempDir;
        if (::osl::FileBase::getTempDirURL(sTempDir) != ::osl::FileBase::E_None)
            throw cssu::Exception("Could not get URL for the temp directory. No extensions will be installed.", 0);

        //create a unique name for the directory
        OUString tempEntry, destFolder;
        if (::osl::File::createTempFile(&sTempDir, 0, &tempEntry ) != ::osl::File::E_None)
            throw cssu::Exception("Could not create a temporary file in " + sTempDir +
             ". No extensions will be installed", 0 );

        tempEntry = tempEntry.copy( tempEntry.lastIndexOf( '/' ) + 1 );

        destFolder = dp_misc::makeURL( sTempDir, tempEntry );
        destFolder += "_";
        m_sDownloadFolder = destFolder;
        try
        {
            dp_misc::create_folder(0, destFolder, m_updateCmdEnv.get(), true );
        } catch (const cssu::Exception & e)
        {
            throw cssu::Exception(e.Message + " No extensions will be installed.", 0);
        }


        sal_uInt16 count = 0;
        typedef std::vector<UpdateData>::iterator It;
        for (It i = m_aVecUpdateData.begin(); i != m_aVecUpdateData.end(); ++i)
        {
            UpdateData & curData = *i;

            if (!curData.aUpdateInfo.is() || curData.aUpdateSource.is())
                continue;
            //We assume that m_aVecUpdateData contains only information about extensions which
            //can be downloaded directly.
            OSL_ASSERT(curData.sWebsiteURL.isEmpty());

            //update the name of the extension which is to be downloaded
            {
                SolarMutexGuard g;
                if (m_stop) {
                    return;
                }
                m_dialog.m_ft_extension_name.SetText(curData.aInstalledPackage->getDisplayName());
                sal_uInt16 prog = (sal::static_int_cast<sal_uInt16>(100) * ++count) /
                    sal::static_int_cast<sal_uInt16>(m_aVecUpdateData.size());
                m_dialog.m_statusbar.SetValue(prog);
            }
            dp_misc::DescriptionInfoset info(m_xComponentContext, curData.aUpdateInfo);
            //remember occurring exceptions in case we need to print out error information
            ::std::vector< ::std::pair<OUString, cssu::Exception> > vecExceptions;
            cssu::Sequence<OUString> seqDownloadURLs = info.getUpdateDownloadUrls();
            OSL_ENSURE(seqDownloadURLs.getLength() > 0, "No download URL provided!");
            for (sal_Int32 j = 0; j < seqDownloadURLs.getLength(); j++)
            {
                try
                {
                    OSL_ENSURE(!seqDownloadURLs[j].isEmpty(), "Download URL is empty!");
                    download(seqDownloadURLs[j], curData);
                    if (!curData.sLocalURL.isEmpty())
                        break;
                }
                catch ( cssu::Exception & e )
                {
                    vecExceptions.push_back( ::std::make_pair(seqDownloadURLs[j], e));
                    //There can be several different errors, for example, the URL is wrong, webserver cannot be reached,
                    //name cannot be resolved. The UCB helper API does not specify different special exceptions for these
                    //cases. Therefore ignore and continue.
                    continue;
                }
            }
            //update the progress and display download error
            {
                SolarMutexGuard g;
                if (m_stop) {
                    return;
                }
                if (curData.sLocalURL.isEmpty())
                {
                    //Construct a string of all messages contained in the exceptions plus the respective download URLs
                    OUStringBuffer buf(256);
                    typedef ::std::vector< ::std::pair<OUString, cssu::Exception > >::const_iterator CIT;
                    for (CIT j = vecExceptions.begin(); j != vecExceptions.end(); ++j)
                    {
                        if (j != vecExceptions.begin())
                            buf.appendAscii("\n");
                        buf.append("Could not download ");
                        buf.append(j->first);
                        buf.appendAscii(". ");
                        buf.append(j->second.Message);
                    }
                    m_dialog.setError(UpdateInstallDialog::ERROR_DOWNLOAD, curData.aInstalledPackage->getDisplayName(),
                        buf.makeStringAndClear());
                }
            }

        }
    }
    catch (const cssu::Exception & e)
    {
        SolarMutexGuard g;
        if (m_stop) {
            return;
        }
        m_dialog.setError(e.Message);
    }
}
void UpdateInstallDialog::Thread::installExtensions()
{
    //Update the fix text in the dialog to "Installing extensions..."
    {
        SolarMutexGuard g;
        if (m_stop) {
            return;
        }
        m_dialog.m_ft_action.SetText(m_dialog.m_sInstalling);
        m_dialog.m_statusbar.SetValue(0);
    }

    sal_uInt16 count = 0;
    typedef std::vector<UpdateData>::iterator It;
    for (It i = m_aVecUpdateData.begin(); i != m_aVecUpdateData.end(); ++i, ++count)
    {
        //update the name of the extension which is to be installed
        {
            SolarMutexGuard g;
            if (m_stop) {
                return;
            }
            //we only show progress after an extension has been installed.
            if (count > 0) {
                m_dialog.m_statusbar.SetValue(
                (sal::static_int_cast<sal_uInt16>(100) * count) /
                sal::static_int_cast<sal_uInt16>(m_aVecUpdateData.size()));
             }
            m_dialog.m_ft_extension_name.SetText(i->aInstalledPackage->getDisplayName());
        }
        bool bError = false;
        bool bLicenseDeclined = false;
        cssu::Reference<css::deployment::XPackage> xExtension;
        UpdateData & curData = *i;
        cssu::Exception exc;
        try
        {
            cssu::Reference< css::task::XAbortChannel > xAbortChannel(
                curData.aInstalledPackage->createAbortChannel() );
            {
                SolarMutexGuard g;
                if (m_stop) {
                    return;
                }
                m_abort = xAbortChannel;
            }
            if (!curData.aUpdateSource.is() && !curData.sLocalURL.isEmpty())
            {
                css::beans::NamedValue prop("EXTENSION_UPDATE", css::uno::makeAny(OUString("1")));
                if (!curData.bIsShared)
                    xExtension = m_dialog.getExtensionManager()->addExtension(
                        curData.sLocalURL, css::uno::Sequence<css::beans::NamedValue>(&prop, 1),
                        "user", xAbortChannel, m_updateCmdEnv.get());
                else
                    xExtension = m_dialog.getExtensionManager()->addExtension(
                        curData.sLocalURL, css::uno::Sequence<css::beans::NamedValue>(&prop, 1),
                        "shared", xAbortChannel, m_updateCmdEnv.get());
            }
            else if (curData.aUpdateSource.is())
            {
                OSL_ASSERT(curData.aUpdateSource.is());
                //I am not sure if we should obtain the install properties and pass them into
                //add extension. Currently it contains only "SUPPRESS_LICENSE". So it it could happen
                //that a license is displayed when updating from the shared repository, although the
                //shared extension was installed using "SUPPRESS_LICENSE".
                css::beans::NamedValue prop("EXTENSION_UPDATE", css::uno::makeAny(OUString("1")));
                if (!curData.bIsShared)
                    xExtension = m_dialog.getExtensionManager()->addExtension(
                        curData.aUpdateSource->getURL(), css::uno::Sequence<css::beans::NamedValue>(&prop, 1),
                        "user", xAbortChannel, m_updateCmdEnv.get());
                else
                    xExtension = m_dialog.getExtensionManager()->addExtension(
                        curData.aUpdateSource->getURL(), css::uno::Sequence<css::beans::NamedValue>(&prop, 1),
                        "shared", xAbortChannel, m_updateCmdEnv.get());
            }
        }
        catch (css::deployment::DeploymentException & de)
        {
            if (de.Cause.has<css::deployment::LicenseException>())
            {
                bLicenseDeclined = true;
            }
            else
            {
                exc = de.Cause.get<cssu::Exception>();
                bError = true;
            }
        }
        catch (cssu::Exception& e)
        {
            exc = e;
            bError = true;
        }

        if (bLicenseDeclined)
        {
            SolarMutexGuard g;
            if (m_stop) {
                return;
            }
            m_dialog.setError(UpdateInstallDialog::ERROR_LICENSE_DECLINED,
                curData.aInstalledPackage->getDisplayName(), OUString());
        }
        else if (!xExtension.is() || bError)
        {
            SolarMutexGuard g;
            if (m_stop) {
                return;
            }
            m_dialog.setError(UpdateInstallDialog::ERROR_INSTALLATION,
                curData.aInstalledPackage->getDisplayName(), exc.Message);
        }
    }
    {
        SolarMutexGuard g;
        if (m_stop) {
            return;
        }
        m_dialog.m_statusbar.SetValue(100);
        m_dialog.m_ft_extension_name.SetText(OUString());
        m_dialog.m_ft_action.SetText(m_dialog.m_sFinished);
    }
}

void UpdateInstallDialog::Thread::removeTempDownloads()
{
    if (!m_sDownloadFolder.isEmpty())
    {
        dp_misc::erase_path(m_sDownloadFolder,
            cssu::Reference<css::ucb::XCommandEnvironment>(),false /* no throw: ignore errors */ );
        //remove also the temp file which we have used to create the unique name
        OUString tempFile = m_sDownloadFolder.copy(0, m_sDownloadFolder.getLength() - 1);
        dp_misc::erase_path(tempFile, cssu::Reference<css::ucb::XCommandEnvironment>(),false);
        m_sDownloadFolder = OUString();
    }
}


void UpdateInstallDialog::Thread::download(OUString const & sDownloadURL, UpdateData & aUpdateData)
{
    {
        SolarMutexGuard g;
        if (m_stop) {
            return;
        }
    }

    OSL_ASSERT(m_sDownloadFolder.getLength());
    OUString destFolder, tempEntry;
    if (::osl::File::createTempFile(
        &m_sDownloadFolder,
        0, &tempEntry ) != ::osl::File::E_None)
    {
        //ToDo feedback in window that download of this component failed
        throw cssu::Exception("Could not create temporary file in folder " + destFolder + ".", 0);
    }
    tempEntry = tempEntry.copy( tempEntry.lastIndexOf( '/' ) + 1 );

    destFolder = dp_misc::makeURL( m_sDownloadFolder, tempEntry );
    destFolder += "_";

    ::ucbhelper::Content destFolderContent;
    dp_misc::create_folder( &destFolderContent, destFolder, m_updateCmdEnv.get() );

    ::ucbhelper::Content sourceContent;
    dp_misc::create_ucb_content( &sourceContent, sDownloadURL, m_updateCmdEnv.get() );

    const OUString sTitle( StrTitle::getTitle( sourceContent ) );

    if (destFolderContent.transferContent(
            sourceContent, ::ucbhelper::InsertOperation_COPY,
            sTitle, css::ucb::NameClash::OVERWRITE ))
    {
        //the user may have cancelled the dialog because downloading took to long
        {
            SolarMutexGuard g;
            if (m_stop) {
                return;
            }
            //all errors should be handeld by the command environment.
            aUpdateData.sLocalURL = destFolder + "/" + sTitle;
        }
    }
}


// -------------------------------------------------------------------------------------------------------

UpdateCommandEnv::UpdateCommandEnv( cssu::Reference< cssu::XComponentContext > const & xCtx,
    ::rtl::Reference<UpdateInstallDialog::Thread>const & thread)
    : m_installThread(thread),
    m_xContext(xCtx)
{
}

UpdateCommandEnv::~UpdateCommandEnv()
{
}


// XCommandEnvironment
//______________________________________________________________________________
cssu::Reference<css::task::XInteractionHandler> UpdateCommandEnv::getInteractionHandler()
throw (cssu::RuntimeException)
{
    return this;
}

//______________________________________________________________________________
cssu::Reference<css::ucb::XProgressHandler> UpdateCommandEnv::getProgressHandler()
throw (cssu::RuntimeException)
{
    return this;
}

// XInteractionHandler
void UpdateCommandEnv::handle(
    cssu::Reference< css::task::XInteractionRequest> const & xRequest )
    throw (cssu::RuntimeException)
{
    cssu::Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == cssu::TypeClass_EXCEPTION );
    dp_misc::TRACE("[dp_gui_cmdenv.cxx] incoming request:\n"
        + ::comphelper::anyToString(request) + "\n\n");

    css::deployment::VersionException verExc;
    bool approve = false;
    bool abort = false;

    if (request >>= verExc)
    {   //We must catch the version exception during the update,
        //because otherwise the user would be confronted with the dialogs, asking
        //them if they want to replace an already installed version of the same extension.
        //During an update we assume that we always want to replace the old version with the
        //new version.
        approve = true;
    }

    if (approve == false && abort == false)
    {
        //forward to interaction handler for main dialog.
        handleInteractionRequest( m_xContext, xRequest );
    }
    else
    {
        // select:
        cssu::Sequence< cssu::Reference< css::task::XInteractionContinuation > > conts(
            xRequest->getContinuations() );
        cssu::Reference< css::task::XInteractionContinuation > const * pConts =
            conts.getConstArray();
        sal_Int32 len = conts.getLength();
        for ( sal_Int32 pos = 0; pos < len; ++pos )
        {
            if (approve) {
                cssu::Reference< css::task::XInteractionApprove > xInteractionApprove(
                    pConts[ pos ], cssu::UNO_QUERY );
                if (xInteractionApprove.is()) {
                    xInteractionApprove->select();
                    // don't query again for ongoing continuations:
                    approve = false;
                }
            }
            else if (abort) {
                cssu::Reference< css::task::XInteractionAbort > xInteractionAbort(
                    pConts[ pos ], cssu::UNO_QUERY );
                if (xInteractionAbort.is()) {
                    xInteractionAbort->select();
                    // don't query again for ongoing continuations:
                    abort = false;
                }
            }
        }
    }
}

// XProgressHandler
void UpdateCommandEnv::push( cssu::Any const & /*Status*/ )
throw (cssu::RuntimeException)
{
}


void UpdateCommandEnv::update( cssu::Any const & /*Status */)
throw (cssu::RuntimeException)
{
}

void UpdateCommandEnv::pop() throw (cssu::RuntimeException)
{
}


} //end namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
