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

#ifndef INCLUDED_DESKTOP_SOURCE_APP_DISPATCHWATCHER_HXX
#define INCLUDED_DESKTOP_SOURCE_APP_DISPATCHWATCHER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>

#include "officeipcthread.hxx"
#include <unordered_map>
#include <vector>

namespace desktop
{

/*
    Class for controls dispatching of command URL through office command line. There
    are "dangerous" command URLs, that can result in a running office without UI. To prevent
    this situation the implementation surveille all dispatches and looks for an open task if
    there is arose a problem. If there is none the office will be shutdown to prevent a
    running office without UI.
*/

class DispatchWatcherHashMap : public std::unordered_map< OUString, sal_Int32, OUStringHash, std::equal_to< OUString >  >
{
};

class DispatchWatcher : public ::cppu::WeakImplHelper< css::frame::XDispatchResultListener >
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
            REQUEST_BATCHPRINT,
            REQUEST_CAT
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
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
            throw(css::uno::RuntimeException, std::exception) override;

        // XDispachResultListener
        virtual void SAL_CALL dispatchFinished( const css::frame::DispatchResultEvent& aEvent ) throw( css::uno::RuntimeException, std::exception ) override;

        // Access function to get a dispatcher watcher reference. There must be a global reference holder
        static DispatchWatcher* GetDispatchWatcher();

        // execute new dispatch request
        bool executeDispatchRequests( const DispatchList& aDispatches, bool bNoTerminate = false );

    private:
        DispatchWatcher();

        static ::osl::Mutex&        GetMutex();

        DispatchWatcherHashMap      m_aRequestContainer;

        sal_Int16                   m_nRequestCount;
};

}

#endif // INCLUDED_DESKTOP_SOURCE_APP_DISPATCHWATCHER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
