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

#pragma once

#include <sal/config.h>

#include <condition_variable>
#include <mutex>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <osl/conditn.hxx>
#include <osl/thread.hxx>
#include <rtl/instance.hxx>

#include "updateinfo.hxx"
#include "updatecheckconfiglistener.hxx"
#include "actionlistener.hxx"
#include "updatehdl.hxx"
#include "download.hxx"


class UpdateCheck;

class UpdateCheckInitData {

public:
    inline rtl::Reference< UpdateCheck > operator() () const;
};

class WorkerThread : public osl::Thread
{
public:
    virtual void cancel() = 0;
};

class UpdateCheck :
    public UpdateCheckConfigListener,
    public IActionListener,
    public DownloadInteractionHandler,
    public rtl::StaticWithInit< rtl::Reference< UpdateCheck >, UpdateCheckInitData >
{
    UpdateCheck();

    virtual ~UpdateCheck() override;

public:
    operator rtl::Reference< UpdateCheckConfigListener > ()
        { return static_cast< UpdateCheckConfigListener * > (this); }

    void initialize(const css::uno::Sequence<css::beans::NamedValue>& rValues,
                    const css::uno::Reference<css::uno::XComponentContext>& xContext);

    // Update internal update info member
    void setUpdateInfo(const UpdateInfo& aInfo);

    /* This method turns on the menubar icon, triggers the bubble window or
     * updates the dialog text when appropriate
     */
    void setUIState(UpdateState eState, bool suppressBubble = false);

    // Returns the UI state that matches rInfo best
    static UpdateState getUIState(const UpdateInfo& rInfo);

    // Check for updates failed
    void setCheckFailedState();

    // Executes the update check dialog for manual checks and downloads interaction
    void showDialog(bool forceCheck = false);

    // Returns true if the update dialog is currently showing
    bool isDialogShowing() const;
    bool shouldShowExtUpdDlg() const { return ( m_bShowExtUpdDlg && m_bHasExtensionUpdate ); }
    void showExtensionDialog();
    void setHasExtensionUpdates( bool bHasUpdates ) { m_bHasExtensionUpdate = bHasUpdates; }
    bool hasOfficeUpdate() const { return (m_aUpdateInfo.BuildId.getLength() > 0); }

    // DownloadInteractionHandler
    virtual bool downloadTargetExists(const OUString& rFileName) override;
    virtual void downloadStalled(const OUString& rErrorMessage) override;
    virtual void downloadProgressAt(sal_Int8 nProcent) override;
    virtual void downloadStarted(const OUString& rLocalFileName, sal_Int64 nFileSize) override;
    virtual void downloadFinished(const OUString& rLocalFileName) override;
    // checks if the download target already exists and asks user what to do next
    virtual bool checkDownloadDestination( const OUString& rFile ) override;

    // Cancels the download action (and resumes checking if enabled)
    void cancelDownload();

    // Returns the XInteractionHandler of the UpdateHandler instance if present (and visible)
    css::uno::Reference< css::task::XInteractionHandler > getInteractionHandler() const;

    // UpdateCheckConfigListener
    virtual void autoCheckStatusChanged(bool enabled) override;
    virtual void autoCheckIntervalChanged() override;

    // IActionListener
    void cancel() override;
    void download() override;
    void install() override;
    void pause() override;
    void resume() override;
    void closeAfterFailure() override;

    void notifyUpdateCheckFinished();

    void waitForUpdateCheckFinished();

private:

    // Schedules or cancels next automatic check for updates
    void enableAutoCheck(bool enable);

    // Starts/resumes or stops a download
    void enableDownload(bool enable, bool paused=false);

    // Shuts down the currently running thread
    void shutdownThread(bool join);

    // Returns the update handler instance
    rtl::Reference<UpdateHandler> getUpdateHandler();

    // Open the given URL in a browser
    void showReleaseNote(const OUString& rURL) const;

    // stores the release note url on disk to be used by setup app
    static bool storeReleaseNote(sal_Int8 nNum, const OUString &rURL);

    /* This method turns on the menubar icon and triggers the bubble window
     */
    void handleMenuBarUI( const rtl::Reference< UpdateHandler >& rUpdateHandler,
                          UpdateState& eState, bool suppressBubble );
    enum State {
        NOT_INITIALIZED,
        DISABLED,
        CHECK_SCHEDULED,
        DOWNLOADING,
        DOWNLOAD_PAUSED
    };

    State m_eState;
    UpdateState m_eUpdateState;

    mutable std::recursive_mutex m_aMutex;
    WorkerThread *m_pThread;
    osl::Condition m_aCondition;
    osl::Condition m_NotInWaitState;

    UpdateInfo m_aUpdateInfo;
    OUString m_aImageName;
    bool m_bHasExtensionUpdate;
    bool m_bShowExtUpdDlg;

    rtl::Reference<UpdateHandler> m_aUpdateHandler;
    css::uno::Reference<css::beans::XPropertySet> m_xMenuBarUI;
    css::uno::Reference<css::uno::XComponentContext> m_xContext;

    bool m_updateCheckRunning = false;
    std::condition_variable_any m_updateCheckFinished;

    friend class UpdateCheckInitData;
};

inline rtl::Reference< UpdateCheck >
UpdateCheckInitData::operator() () const
{
    return rtl::Reference< UpdateCheck > (new UpdateCheck());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
