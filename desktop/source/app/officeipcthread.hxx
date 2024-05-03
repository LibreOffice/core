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

#include <utility>
#include <vector>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <osl/signal.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <cppuhelper/implbase.hxx>
#include <osl/conditn.hxx>
#include <optional>

namespace desktop
{

oslSignalAction SalMainPipeExchangeSignal_impl(void* /*pData*/, oslSignalInfo* pInfo);

// A request for the current office
// that was given by command line or by IPC pipe communication.
struct ProcessDocumentsRequest
{
    explicit ProcessDocumentsRequest(std::optional< OUString > cwdUrl):
        aCwdUrl(std::move(cwdUrl)), pcProcessed( nullptr ), bTextCat( false ), bScriptCat( false ) {}

    std::optional< OUString > aCwdUrl;
    OUString aModule;
    std::vector< OUString > aOpenList; // Documents that should be opened in the default way
    std::vector< OUString > aViewList; // Documents that should be opened in viewmode
    std::vector< OUString > aStartList; // Documents/Presentations that should be started
    std::vector< OUString > aPrintList; // Documents that should be printed on default printer
    std::vector< OUString > aForceOpenList; // Documents that should be forced to open for editing (even templates)
    std::vector< OUString > aForceNewList; // Documents that should be forced to create a new document
    OUString aPrinterName;   // The printer name that should be used for printing
    std::vector< OUString > aPrintToList; // Documents that should be printed on the given printer
    std::vector< OUString > aConversionList;
    OUString aConversionParams;
    OUString aConversionOut;
    OUString aImageConversionType;
    OUString aStartListParams;
    std::vector< OUString > aInFilter;
    ::osl::Condition *pcProcessed;  // pointer condition to be set when the request has been processed
    bool* mpbSuccess = nullptr; // pointer to boolean receiving if the processing was successful
    bool bTextCat; // boolean flag indicating whether to dump text content to console
    bool bScriptCat; // boolean flag indicating whether to dump script content to console
};

class DispatchWatcher;
class IpcThread;
class PipeIpcThread;
class DbusIpcThread;

class RequestHandler: public salhelper::SimpleReferenceObject
{
    friend IpcThread;
    friend PipeIpcThread;
    friend DbusIpcThread;

  private:
    static rtl::Reference< RequestHandler > pGlobal;

    enum class State { Starting, RequestsEnabled, Downing };

    State                       mState;
    int                         mnPendingRequests;
    rtl::Reference<DispatchWatcher> mpDispatchWatcher;
    rtl::Reference<IpcThread> mIpcThread;

    /* condition to be set when the request has been processed */
    ::osl::Condition cProcessed;
    /* receives if the processing was successful (may be false e.g. when shutting down) */
    bool mbSuccess = false;

    /* condition to be set when the main event loop is ready
       otherwise an error dialogs event loop could eat away
       requests from a 2nd office */
    ::osl::Condition cReady;

    static ::osl::Mutex&        GetMutex();

    RequestHandler();

    virtual ~RequestHandler() override;

  public:
    enum Status
    {
        IPC_STATUS_OK,
        IPC_STATUS_2ND_OFFICE,
        IPC_STATUS_PIPE_ERROR,
        IPC_STATUS_BOOTSTRAP_ERROR
    };

    // controlling pipe communication during shutdown
    static void                 SetDowning();
    static void                 EnableRequests();
    static bool                 AreRequestsPending();
    static void                 RequestsCompleted();
    static bool                 ExecuteCmdLineRequests(
        ProcessDocumentsRequest&, bool noTerminate);

    // return sal_False if second office
    static Status               Enable(bool ipc);
    static void                 Disable();
    // start dispatching events...
    static void                 SetReady(bool bIsReady);
    static void                 WaitForReady();

    bool                        AreRequestsEnabled() const { return mState == State::RequestsEnabled; }
};


class RequestHandlerController : public ::cppu::WeakImplHelper<
                                            css::lang::XServiceInfo,
                                            css::frame::XTerminateListener >
{
    public:
        RequestHandlerController() {}

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XTerminateListener
        virtual void SAL_CALL queryTermination( const css::lang::EventObject& aEvent ) override;
        virtual void SAL_CALL notifyTermination( const css::lang::EventObject& aEvent ) override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
