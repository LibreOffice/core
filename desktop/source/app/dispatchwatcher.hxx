/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dispatchwatcher.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2006-11-07 15:30:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _DESKTOP_DISPATCHWATCHER_HXX
#define _DESKTOP_DISPATCHWATCHER_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_XNOTIFYINGDISPATCH_HPP_
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRESULTLISTENER_HPP_
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#endif

#include "officeipcthread.hxx"
#include <hash_map>
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
            DispatchRequest( RequestType aType, const ::rtl::OUString& aFile, const ::rtl::OUString& aPrinter, const ::rtl::OUString& aFact ) :
                aRequestType( aType ), aURL( aFile ), aPrinterName( aPrinter ), aPreselectedFactory( aFact ) {}

            RequestType     aRequestType;
            rtl::OUString   aURL;
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
        sal_Bool executeDispatchRequests( const DispatchList& aDispatches );

    private:
        DispatchWatcher();

        static ::osl::Mutex&        GetMutex();

        DispatchWatcherHashMap      m_aRequestContainer;

        static ::osl::Mutex*        pWatcherMutex;

        sal_Int16                   m_nRequestCount;
};

}

#endif // _DESKTOP_DISPATCHWATCHER_HXX
