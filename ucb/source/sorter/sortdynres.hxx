/*************************************************************************
 *
 *  $RCSfile: sortdynres.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:53:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SORTDYNRES_HXX
#define _SORTDYNRES_HXX

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NUMBEREDSORTINGINFO_HPP_
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_XDYNAMICRESULTSETLISTENER_HPP_
#include <com/sun/star/ucb/XDynamicResultSetListener.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_LISTENERALREADYSETEXCEPTION_HPP_
#include <com/sun/star/ucb/ListenerAlreadySetException.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_XSORTEDDYNAMICRESULTSETFACTORY_HPP_
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include <tools/list.hxx>

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

#ifndef _SORTRESULT_HXX
#include "sortresult.hxx"
#endif

namespace cppu {
    class OInterfaceContainerHelper;
}

//-----------------------------------------------------------------------------

#define NUMBERED_SORTINGINFO        com::sun::star::ucb::NumberedSortingInfo
#define RUNTIMEEXCEPTION            com::sun::star::uno::RuntimeException
#define REFERENCE                   com::sun::star::uno::Reference
#define SEQUENCE                    com::sun::star::uno::Sequence
#define EVENTOBJECT                 com::sun::star::lang::EventObject
#define XEVENTLISTENER              com::sun::star::lang::XEventListener
#define XMULTISERVICEFACTORY        com::sun::star::lang::XMultiServiceFactory
#define XRESULTSET                  com::sun::star::sdbc::XResultSet
#define SQLEXCEPTION                com::sun::star::sdbc::SQLException
#define XANYCOMPAREFACTORY          com::sun::star::ucb::XAnyCompareFactory
#define XDYNAMICRESULTSET           com::sun::star::ucb::XDynamicResultSet
#define XDYNAMICRESULTSETLISTENER   com::sun::star::ucb::XDynamicResultSetListener
#define LISTENERALREADYSETEXCEPTION com::sun::star::ucb::ListenerAlreadySetException

#define DYNAMIC_RESULTSET_SERVICE_NAME  "com.sun.star.ucb.SortedDynamicResultSet"
#define DYNAMIC_RESULTSET_FACTORY_NAME  "com.sun.star.ucb.SortedDynamicResultSetFactory"

//-----------------------------------------------------------------------------
class SortedDynamicResultSetListener;

class SortedDynamicResultSet:
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::ucb::XDynamicResultSet
{
    cppu::OInterfaceContainerHelper *mpDisposeEventListeners;

    REFERENCE < XDYNAMICRESULTSETLISTENER > mxListener;
    REFERENCE < XDYNAMICRESULTSETLISTENER > mxOwnListener;

    REFERENCE < XRESULTSET >            mxOne;
    REFERENCE < XRESULTSET >            mxTwo;
    REFERENCE < XDYNAMICRESULTSET >     mxOriginal;
    SEQUENCE  < NUMBERED_SORTINGINFO >  maOptions;
    REFERENCE < XANYCOMPAREFACTORY >    mxCompFac;
    REFERENCE < XMULTISERVICEFACTORY >  mxSMgr;

    SortedResultSet*                    mpOne;
    SortedResultSet*                    mpTwo;
    SortedDynamicResultSetListener*     mpOwnListener;

    EventList                           maActions;
    ::vos::OMutex                       maMutex;
    BOOL                                mbGotWelcome    :1;
    BOOL                                mbUseOne        :1;
    BOOL                                mbStatic        :1;

private:

    void                SendNotify();

public:
    SortedDynamicResultSet( const REFERENCE < XDYNAMICRESULTSET >    &xOriginal,
                            const SEQUENCE  < NUMBERED_SORTINGINFO > &aOptions,
                            const REFERENCE < XANYCOMPAREFACTORY >   &xCompFac,
                            const REFERENCE < XMULTISERVICEFACTORY > &xSMgr );

    ~SortedDynamicResultSet();

    //-----------------------------------------------------------------
    // XInterface
    //-----------------------------------------------------------------
    XINTERFACE_DECL()

    //-----------------------------------------------------------------
    // XTypeProvider
    //-----------------------------------------------------------------
    XTYPEPROVIDER_DECL()

    //-----------------------------------------------------------------
    // XServiceInfo
    //-----------------------------------------------------------------
    XSERVICEINFO_NOFACTORY_DECL()

    //-----------------------------------------------------------------
    // XComponent
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    dispose() throw( RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    addEventListener( const REFERENCE< XEVENTLISTENER >& Listener )
        throw( RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    removeEventListener( const REFERENCE< XEVENTLISTENER >& Listener )
        throw( RUNTIME_EXCEPTION );

    //-----------------------------------------------------------------
    // XDynamicResultSet
    //-----------------------------------------------------------------
    virtual REFERENCE< XRESULTSET > SAL_CALL
    getStaticResultSet(  )
        throw( LISTENERALREADYSETEXCEPTION, RUNTIMEEXCEPTION );

    virtual void SAL_CALL
    setListener( const REFERENCE< XDYNAMICRESULTSETLISTENER >& Listener )
        throw( LISTENERALREADYSETEXCEPTION, RUNTIMEEXCEPTION );

    virtual void SAL_CALL
    connectToCache( const REFERENCE< XDYNAMICRESULTSET > & xCache )
        throw( LISTENERALREADYSETEXCEPTION,
               com::sun::star::ucb::AlreadyInitializedException,
               com::sun::star::ucb::ServiceNotFoundException,
               RUNTIMEEXCEPTION );

    virtual sal_Int16 SAL_CALL
    getCapabilities()
        throw( RUNTIMEEXCEPTION );

    //-----------------------------------------------------------------
    // own methods:
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    impl_disposing( const EVENTOBJECT& Source )
        throw( RUNTIMEEXCEPTION );

    virtual void SAL_CALL
    impl_notify( const ::com::sun::star::ucb::ListEvent& Changes )
        throw( RUNTIMEEXCEPTION );
};

//-----------------------------------------------------------------------------

class SortedDynamicResultSetListener:
                public cppu::OWeakObject,
                public com::sun::star::ucb::XDynamicResultSetListener
{
    SortedDynamicResultSet  *mpOwner;
    ::vos::OMutex            maMutex;

public:
     SortedDynamicResultSetListener( SortedDynamicResultSet *mOwner );
    ~SortedDynamicResultSetListener();

    //-----------------------------------------------------------------
    // XInterface
    //-----------------------------------------------------------------
    XINTERFACE_DECL()

    //-----------------------------------------------------------------
    // XEventListener ( base of XDynamicResultSetListener )
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    disposing( const EVENTOBJECT& Source )
        throw( RUNTIMEEXCEPTION );

    //-----------------------------------------------------------------
    // XDynamicResultSetListener
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    notify( const ::com::sun::star::ucb::ListEvent& Changes )
        throw( RUNTIMEEXCEPTION );

    //-----------------------------------------------------------------
    // own methods:
    //-----------------------------------------------------------------
    void SAL_CALL impl_OwnerDies();
};

//-----------------------------------------------------------------------------

class SortedDynamicResultSetFactory:
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::ucb::XSortedDynamicResultSetFactory
{

    REFERENCE< XMULTISERVICEFACTORY >   mxSMgr;

public:

    SortedDynamicResultSetFactory(
        const REFERENCE< XMULTISERVICEFACTORY > & rSMgr);

    ~SortedDynamicResultSetFactory();

    //-----------------------------------------------------------------
    // XInterface
    //-----------------------------------------------------------------
    XINTERFACE_DECL()

    //-----------------------------------------------------------------
    // XTypeProvider
    //-----------------------------------------------------------------
    XTYPEPROVIDER_DECL()

    //-----------------------------------------------------------------
    // XServiceInfo
    //-----------------------------------------------------------------
    XSERVICEINFO_DECL()

    //-----------------------------------------------------------------
    // XSortedDynamicResultSetFactory

    virtual REFERENCE< XDYNAMICRESULTSET > SAL_CALL
    createSortedDynamicResultSet(
                const REFERENCE< XDYNAMICRESULTSET > & Source,
                const SEQUENCE< NUMBERED_SORTINGINFO > & Info,
                const REFERENCE< XANYCOMPAREFACTORY > & CompareFactory )
        throw( RUNTIMEEXCEPTION );
};

#endif
