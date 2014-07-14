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



#ifndef _DESKTOP_DISPATCHWATCHER_HXX
#define _DESKTOP_DISPATCHWATCHER_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>

#include "officeipcthread.hxx"
#include <hash_map>
#include <vector>

namespace desktop
{

/*
    Class for controls dispatching of command URL through office command line. There
    are "dangerous" command URLs, that can result in a running office without UI. To prevent
    this situation the implementation surveile all dispatches and looks for an open task if
    there is arose a problem. If there is none the office will be shutdown to prevent a
    running office without UI.
*/

struct OUStringHashCode
{
    size_t operator()( const ::rtl::OUString& sString ) const
    {
        return sString.hashCode();
    }
};

class DispatchWatcherHashMap : public ::std::hash_map< ::rtl::OUString, sal_Int32, OUStringHashCode, ::std::equal_to< ::rtl::OUString > >
{
    public:
        inline void free()
        {
            DispatchWatcherHashMap().swap( *this );
        }
};

class DispatchWatcher : public ::cppu::WeakImplHelper1< ::com::sun::star::frame::XDispatchResultListener >
{
    public:
        enum RequestType
        {
            REQUEST_OPEN,
            REQUEST_VIEW,
            REQUEST_START,
            REQUEST_PRINT,
            REQUEST_PRINTTO,
            REQUEST_FORCEOPEN,
            REQUEST_FORCENEW
        };

        struct DispatchRequest
        {
            DispatchRequest( RequestType aType, const ::rtl::OUString& aFile, boost::optional< rtl::OUString > const & cwdUrl, const ::rtl::OUString& aPrinter, const ::rtl::OUString& aFact ) :
                aRequestType( aType ), aURL( aFile ), aCwdUrl( cwdUrl ), aPrinterName( aPrinter ), aPreselectedFactory( aFact ) {}

            RequestType     aRequestType;
            rtl::OUString   aURL;
            boost::optional< rtl::OUString > aCwdUrl;
            rtl::OUString   aPrinterName;
            rtl::OUString   aPreselectedFactory;
        };

        typedef std::vector< DispatchRequest > DispatchList;

        virtual ~DispatchWatcher();

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
            throw(::com::sun::star::uno::RuntimeException);

        // XDispachResultListener
        virtual void SAL_CALL dispatchFinished( const com::sun::star::frame::DispatchResultEvent& aEvent ) throw( com::sun::star::uno::RuntimeException );

        // Access function to get a dispatcher watcher reference. There must be a global reference holder
        static DispatchWatcher* GetDispatchWatcher();

        // execute new dispatch request
        sal_Bool executeDispatchRequests( const DispatchList& aDispatches, bool bNoTerminate = false );

    private:
        DispatchWatcher();

        static ::osl::Mutex&        GetMutex();

        DispatchWatcherHashMap      m_aRequestContainer;

        static ::osl::Mutex*        pWatcherMutex;

        sal_Int16                   m_nRequestCount;
};

}

#endif // _DESKTOP_DISPATCHWATCHER_HXX
