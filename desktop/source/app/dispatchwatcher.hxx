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

struct OUStringHashCode
{
    size_t operator()( const ::rtl::OUString& sString ) const
    {
        return sString.hashCode();
    }
};

class DispatchWatcherHashMap : public ::boost::unordered_map< ::rtl::OUString, sal_Int32, OUStringHashCode, ::std::equal_to< ::rtl::OUString >  >
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
            DispatchRequest( RequestType aType, const ::rtl::OUString& aFile, boost::optional< rtl::OUString > const & cwdUrl, const ::rtl::OUString& aPrinter, const ::rtl::OUString& aFact ) :
                aRequestType( aType ), aURL( aFile ), aCwdUrl( cwdUrl ), aPrinterName( aPrinter ), aPreselectedFactory( aFact ) {}

            RequestType     aRequestType;
            rtl::OUString   aURL;
            boost::optional< rtl::OUString > aCwdUrl;
            rtl::OUString   aPrinterName;  // also conversion params
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
