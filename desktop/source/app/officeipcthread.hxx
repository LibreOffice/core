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

#ifndef _DESKTOP_OFFICEIPCTHREAD_HXX_
#define _DESKTOP_OFFICEIPCTHREAD_HXX_

#include "sal/config.h"

#include <vector>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <osl/pipe.hxx>
#include <osl/security.hxx>
#include <osl/signal.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase2.hxx>
#include <osl/conditn.hxx>
#include <salhelper/thread.hxx>
#include "boost/optional.hpp"

namespace desktop
{

oslSignalAction SAL_CALL SalMainPipeExchangeSignal_impl(void* /*pData*/, oslSignalInfo* pInfo);

// A request for the current office
// that was given by command line or by IPC pipe communication.
struct ProcessDocumentsRequest
{
    ProcessDocumentsRequest(boost::optional< rtl::OUString > const & cwdUrl):
        aCwdUrl(cwdUrl), pcProcessed( NULL ) {}

    boost::optional< ::rtl::OUString > aCwdUrl;
    ::rtl::OUString aModule;
    std::vector< rtl::OUString > aOpenList; // Documents that should be opened in the default way
    std::vector< rtl::OUString > aViewList; // Documents that should be opened in viewmode
    std::vector< rtl::OUString > aStartList; // Documents/Presentations that should be started
    std::vector< rtl::OUString > aPrintList; // Documents that should be printed on default printer
    std::vector< rtl::OUString > aForceOpenList; // Documents that should be forced to open for editing (even templates)
    std::vector< rtl::OUString > aForceNewList; // Documents that should be forced to create a new document
    ::rtl::OUString aPrinterName;   // The printer name that should be used for printing
    std::vector< rtl::OUString > aPrintToList; // Documents that should be printed on the given printer
    std::vector< rtl::OUString > aConversionList;
    ::rtl::OUString aConversionParams;
    ::rtl::OUString aConversionOut;
    std::vector< rtl::OUString > aInFilter;
    ::osl::Condition *pcProcessed;  // pointer condition to be set when the request has been processed
};

class DispatchWatcher;
class OfficeIPCThread : public salhelper::Thread
{
  private:
    static rtl::Reference< OfficeIPCThread > pGlobalOfficeIPCThread;

    osl::Pipe                   maPipe;
    osl::StreamPipe             maStreamPipe;
    rtl::OUString               maPipeIdent;
    bool                        mbDowning;
    bool                        mbRequestsEnabled;
    int                         mnPendingRequests;
    DispatchWatcher*            mpDispatchWatcher;

    /* condition to be set when the request has been processed */
    ::osl::Condition cProcessed;

    /* condition to be set when the main event loop is ready
       otherwise an error dialogs event loop could eat away
       requests from a 2nd office */
    ::osl::Condition cReady;

    static ::osl::Mutex&        GetMutex();
    static const char *sc_aTerminationSequence;
    static const int sc_nTSeqLength;
    static const char *sc_aShowSequence;
    static const int sc_nShSeqLength;
    static const char *sc_aConfirmationSequence;
    static const int sc_nCSeqLength;
    static const char *sc_aSendArgumentsSequence;
    static const int sc_nCSASeqLength;

    OfficeIPCThread();

    virtual ~OfficeIPCThread();

    /// Working method which should be overridden
    virtual void execute();

  public:
    enum Status
    {
        IPC_STATUS_OK,
        IPC_STATUS_2ND_OFFICE,
        IPC_STATUS_BOOTSTRAP_ERROR
    };

    // controlling pipe communication during shutdown
    static void                 SetDowning();
    static void                 EnableRequests( bool i_bEnable = true );
    static sal_Bool             AreRequestsPending();
    static void                 RequestsCompleted( int n = 1 );
    static sal_Bool             ExecuteCmdLineRequests( ProcessDocumentsRequest& );

    // return sal_False if second office
    static Status               EnableOfficeIPCThread();
    static void                 DisableOfficeIPCThread();
    // start dispatching events...
    static void                 SetReady(
        rtl::Reference< OfficeIPCThread > const & pThread =
            rtl::Reference< OfficeIPCThread >());

    bool                        AreRequestsEnabled() const { return mbRequestsEnabled && ! mbDowning; }
};


class OfficeIPCThreadController : public ::cppu::WeakImplHelper2<
                                            ::com::sun::star::lang::XServiceInfo,
                                            ::com::sun::star::frame::XTerminateListener >
{
    public:
        OfficeIPCThreadController() {}
        virtual ~OfficeIPCThreadController() {}

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName()
            throw ( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw ( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
            throw ( ::com::sun::star::uno::RuntimeException );

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
            throw( ::com::sun::star::uno::RuntimeException );

        // XTerminateListener
        virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& aEvent )
            throw( ::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& aEvent )
            throw( ::com::sun::star::uno::RuntimeException );
};

}

#endif // _DESKTOP_OFFICEIPCTHREAD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
