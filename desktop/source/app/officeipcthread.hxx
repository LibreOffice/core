/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _DESKTOP_OFFICEIPCTHREAD_HXX_
#define _DESKTOP_OFFICEIPCTHREAD_HXX_

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <vos/pipe.hxx>
#include <vos/security.hxx>
#include <vos/thread.hxx>
#include <vos/signal.hxx>
#include <rtl/ustring.hxx>
#ifndef _CPPUHELPER_WEAKBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#include <osl/conditn.hxx>
#include "boost/optional.hpp"

namespace desktop
{

class SalMainPipeExchangeSignalHandler : public vos::OSignalHandler
{
    virtual TSignalAction SAL_CALL signal(TSignalInfo *pInfo);
};

// A request for the current office
// that was given by command line or by IPC pipe communication.
struct ProcessDocumentsRequest
{
    ProcessDocumentsRequest(boost::optional< rtl::OUString > const & cwdUrl):
        aCwdUrl(cwdUrl), pcProcessed( NULL ) {}

    boost::optional< ::rtl::OUString > aCwdUrl;
    ::rtl::OUString aModule;
    ::rtl::OUString aOpenList;      // Documents that should be opened in the default way
    ::rtl::OUString aViewList;      // Documents that should be opened in viewmode
    ::rtl::OUString aStartList;     // Documents/Presentations that should be started
    ::rtl::OUString aPrintList;     // Documents that should be printed on default printer
    ::rtl::OUString aForceOpenList; // Documents that should be forced to open for editing (even templates)
    ::rtl::OUString aForceNewList;  // Documents that should be forced to create a new document
    ::rtl::OUString aPrinterName;   // The printer name that should be used for printing
    ::rtl::OUString aPrintToList;   // Documents that should be printed on the given printer
    ::osl::Condition *pcProcessed;  // pointer condition to be set when the request has been processed
};

class DispatchWatcher;
class OfficeIPCThread : public vos::OThread
{
  private:
    static OfficeIPCThread*     pGlobalOfficeIPCThread;
    static ::osl::Mutex*        pOfficeIPCThreadMutex;

    vos::OPipe                  maPipe;
    vos::OStreamPipe            maStreamPipe;
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

    OfficeIPCThread();

  protected:
    /// Working method which should be overridden
    virtual void SAL_CALL run();

  public:
    enum Status
    {
        IPC_STATUS_OK,
        IPC_STATUS_2ND_OFFICE,
        IPC_STATUS_BOOTSTRAP_ERROR
    };

    virtual ~OfficeIPCThread();

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
    static void                 SetReady(OfficeIPCThread* pThread = NULL);

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
