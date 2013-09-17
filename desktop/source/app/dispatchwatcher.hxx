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

#ifndef _DESKTOP_DISPATCHWATCHER_HXX
#define _DESKTOP_DISPATCHWATCHER_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>

#include "officeipcthread.hxx"
#include <boost/unordered_map.hpp>
#include <vector>

namespace desktop
{

/*
    Class for controlls dispatching of command URL through office command line. There
    are "dangerous" command URLs, that can result in a running office without UI. To prevent
    this situation the implementation surveile all dispatches and looks for an open task if
    there is arose a problem. If there is none the office will be shutdown to prevent a
    running office without UI.
*/

class DispatchWatcherHashMap : public ::boost::unordered_map< OUString, sal_Int32, OUStringHash, ::std::equal_to< OUString >  >
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
            REQUEST_FORCENEW,
            REQUEST_CONVERSION,
            REQUEST_INFILTER,
            REQUEST_BATCHPRINT
        };

        struct DispatchRequest
        {
            DispatchRequest( RequestType aType, const OUString& aFile, boost::optional< OUString > const & cwdUrl, const OUString& aPrinter, const OUString& aFact ) :
                aRequestType( aType ), aURL( aFile ), aCwdUrl( cwdUrl ), aPrinterName( aPrinter ), aPreselectedFactory( aFact ) {}

            RequestType     aRequestType;
            OUString   aURL;
            boost::optional< OUString > aCwdUrl;
            OUString   aPrinterName;  // also conversion params
            OUString   aPreselectedFactory;
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

        sal_Int16                   m_nRequestCount;
};

}

#endif // _DESKTOP_DISPATCHWATCHER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
