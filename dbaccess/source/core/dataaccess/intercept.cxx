/*************************************************************************
 *
 *  $RCSfile: intercept.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:10:59 $
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

#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef DBA_INTERCEPT_HXX
#include "intercept.hxx"
#endif
#include "dbastrings.hrc"


namespace dbaccess
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::container;
using namespace ::comphelper;
using namespace ::cppu;

#define DISPATCH_SAVEAS     0
#define DISPATCH_SAVE       1
#define DISPATCH_CLOSEDOC   2
#define DISPATCH_CLOSEWIN   3
#define DISPATCH_CLOSEFRAME 4
#define DISPATCH_EDITDOC    5
#define DISPATCH_RELOAD     6
// the OSL_ENSURE in CTOR has to be changed too, when adding new defines

void OInterceptor::DisconnectContentHolder()
{
    osl::MutexGuard aGuard( m_aMutex );
    m_pContentHolder = NULL;
}

void SAL_CALL
OInterceptor::addEventListener(
    const Reference< com::sun::star::lang::XEventListener >& Listener )
    throw( RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( ! m_pDisposeEventListeners )
        m_pDisposeEventListeners =
            new cppu::OInterfaceContainerHelper( m_aMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}


void SAL_CALL
OInterceptor::removeEventListener(
    const Reference< com::sun::star::lang::XEventListener >& Listener )
    throw( RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}


void SAL_CALL OInterceptor::dispose()
    throw( RuntimeException )
{
    EventObject aEvt;
    aEvt.Source = static_cast< XDispatch* >( this );

    osl::MutexGuard aGuard(m_aMutex);

    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
        m_pDisposeEventListeners->disposeAndClear( aEvt );

    if(m_pStatCL)
        m_pStatCL->disposeAndClear( aEvt );

    m_xSlaveDispatchProvider = 0;
    m_xMasterDispatchProvider = 0;
}



OInterceptor::OInterceptor( ODocumentDefinition* _pContentHolder,sal_Bool _bAllowEditDoc )
    : m_pContentHolder( _pContentHolder )
      ,m_pStatCL(0)
      ,m_pDisposeEventListeners(0)
      ,m_aInterceptedURL(7)
      ,m_bAllowEditDoc(_bAllowEditDoc)
{
    OSL_ENSURE(DISPATCH_EDITDOC < m_aInterceptedURL.getLength(),"Illegal size.");

    m_aInterceptedURL[DISPATCH_SAVEAS]      = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:SaveAs"));
    m_aInterceptedURL[DISPATCH_SAVE]        = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Save"));
    m_aInterceptedURL[DISPATCH_CLOSEDOC]    = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:CloseDoc"));
    m_aInterceptedURL[DISPATCH_CLOSEWIN]    = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:CloseWin"));
    m_aInterceptedURL[DISPATCH_CLOSEFRAME]  = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:CloseFrame"));
    m_aInterceptedURL[DISPATCH_EDITDOC]     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:EditDoc"));
    m_aInterceptedURL[DISPATCH_RELOAD]      = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Reload"));
}


OInterceptor::~OInterceptor()
{
    if( m_pDisposeEventListeners )
        delete m_pDisposeEventListeners;

    if(m_pStatCL)
        delete m_pStatCL;
}



//XDispatch
void SAL_CALL
OInterceptor::dispatch(
    const URL& _URL,
    const Sequence<
    PropertyValue >& Arguments )
    throw (RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);
    if( m_pContentHolder )
        if( _URL.Complete == m_aInterceptedURL[DISPATCH_SAVE] )
        {
            m_pContentHolder->save(sal_False);
        }
        else if( _URL.Complete == m_aInterceptedURL[DISPATCH_RELOAD] )
        {
            m_pContentHolder->fillReportData();
        }
        else if( _URL.Complete == m_aInterceptedURL[DISPATCH_SAVEAS] )
        {
            Sequence< PropertyValue > aNewArgs = Arguments;
            sal_Int32 nInd = 0;

            while( nInd < aNewArgs.getLength() )
            {
                if ( aNewArgs[nInd].Name.equalsAscii( "SaveTo" ) )
                {
                    aNewArgs[nInd].Value <<= sal_True;
                    break;
                }
                nInd++;
            }

            if ( nInd == aNewArgs.getLength() )
            {
                aNewArgs.realloc( nInd + 1 );
                aNewArgs[nInd].Name = ::rtl::OUString::createFromAscii( "SaveTo" );
                aNewArgs[nInd].Value <<= sal_True;
            }

            Reference< XDispatch > xDispatch = m_xSlaveDispatchProvider->queryDispatch(
                _URL, ::rtl::OUString::createFromAscii( "_self" ), 0 );
            if ( xDispatch.is() )
                xDispatch->dispatch( _URL, aNewArgs );
        }
        else if (  _URL.Complete == m_aInterceptedURL[DISPATCH_CLOSEDOC]
                || _URL.Complete == m_aInterceptedURL[DISPATCH_CLOSEWIN]
                || _URL.Complete == m_aInterceptedURL[DISPATCH_CLOSEFRAME]
                )
        {
            if ( m_pContentHolder->isModified() )
            {
                if ( !m_pContentHolder->save(sal_True) )
                    return;
            }

            Reference< XDispatch > xDispatch = m_xSlaveDispatchProvider->queryDispatch(
                _URL, ::rtl::OUString::createFromAscii( "_self" ), 0 );
            if ( xDispatch.is() )
                xDispatch->dispatch( _URL, Arguments );
        }
        else if ( _URL.Complete == m_aInterceptedURL[DISPATCH_EDITDOC] )
        {
            Reference< XDispatch > xDispatch = m_xSlaveDispatchProvider->queryDispatch(
                _URL, ::rtl::OUString::createFromAscii( "_self" ), 0 );
            if ( xDispatch.is() )
                xDispatch->dispatch( _URL, Arguments );
        }
}

void SAL_CALL
OInterceptor::addStatusListener(
    const Reference<
    XStatusListener >& Control,
    const URL& _URL )
    throw (
        RuntimeException
    )
{
    if(!Control.is())
        return;

    if ( m_pContentHolder && _URL.Complete == m_aInterceptedURL[DISPATCH_SAVEAS] )
    {   // SaveAs
        FeatureStateEvent aStateEvent;
        aStateEvent.FeatureURL.Complete = m_aInterceptedURL[DISPATCH_SAVEAS];
        aStateEvent.FeatureDescriptor = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SaveCopyTo"));
        aStateEvent.IsEnabled = sal_True;
        aStateEvent.Requery = sal_False;
        aStateEvent.State <<= (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("($3)")));
        Control->statusChanged(aStateEvent);

        {
            osl::MutexGuard aGuard(m_aMutex);
            if(!m_pStatCL)
                m_pStatCL = new PropertyChangeListenerContainer(m_aMutex);
        }

        m_pStatCL->addInterface(_URL.Complete,Control);
    }
    else if ( m_pContentHolder && _URL.Complete == m_aInterceptedURL[DISPATCH_SAVE] )
    {   // Save
        FeatureStateEvent aStateEvent;
        aStateEvent.FeatureURL.Complete = m_aInterceptedURL[DISPATCH_SAVE];
        aStateEvent.FeatureDescriptor = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Update"));
        aStateEvent.IsEnabled = m_pContentHolder != NULL && m_pContentHolder->isModified();
        aStateEvent.Requery = sal_False;

        Control->statusChanged(aStateEvent);
        {
            osl::MutexGuard aGuard(m_aMutex);
            if(!m_pStatCL)
                m_pStatCL = new PropertyChangeListenerContainer(m_aMutex);
        }

        m_pStatCL->addInterface(_URL.Complete,Control);
        Reference< ::com::sun::star::document::XEventBroadcaster> xEvtB(m_pContentHolder->getComponent(),UNO_QUERY);
        if ( xEvtB.is() )
            xEvtB->addEventListener(this);
    }
    else if ( m_pContentHolder && _URL.Complete == m_aInterceptedURL[DISPATCH_EDITDOC] )
    {   // EditDoc
        FeatureStateEvent aStateEvent;
        aStateEvent.FeatureURL.Complete = m_aInterceptedURL[DISPATCH_EDITDOC];
        aStateEvent.FeatureDescriptor = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Update"));
        aStateEvent.IsEnabled = m_bAllowEditDoc;
        aStateEvent.Requery = sal_False;

        Control->statusChanged(aStateEvent);
        {
            osl::MutexGuard aGuard(m_aMutex);
            if(!m_pStatCL)
                m_pStatCL = new PropertyChangeListenerContainer(m_aMutex);
        }

        m_pStatCL->addInterface(_URL.Complete,Control);
    }
    else
    {
        sal_Int32 i = 2;
        if(_URL.Complete == m_aInterceptedURL[i] ||
            _URL.Complete == m_aInterceptedURL[++i] ||
            _URL.Complete == m_aInterceptedURL[++i] ||
            _URL.Complete == m_aInterceptedURL[i = DISPATCH_RELOAD] )
        {   // Close and return
            FeatureStateEvent aStateEvent;
            aStateEvent.FeatureURL.Complete = m_aInterceptedURL[i];
            aStateEvent.FeatureDescriptor = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("Close and Return"));
            aStateEvent.IsEnabled = sal_True;
            aStateEvent.Requery = sal_False;
            Control->statusChanged(aStateEvent);


            {
                osl::MutexGuard aGuard(m_aMutex);
                if(!m_pStatCL)
                    m_pStatCL = new PropertyChangeListenerContainer(m_aMutex);
            }

            m_pStatCL->addInterface(_URL.Complete,Control);
            return;
        }
    }
}


void SAL_CALL
OInterceptor::removeStatusListener(
    const Reference<
    XStatusListener >& Control,
    const URL& _URL )
    throw (
        RuntimeException
    )
{
    if(!(Control.is() && m_pStatCL))
        return;
    else
    {
        m_pStatCL->removeInterface(_URL.Complete,Control);
        return;
    }
}


//XInterceptorInfo
Sequence< ::rtl::OUString >
SAL_CALL
OInterceptor::getInterceptedURLs(  )
    throw (
        RuntimeException
    )
{
    // now implemented as update

    return m_aInterceptedURL;
}


// XDispatchProvider

Reference< XDispatch > SAL_CALL
OInterceptor::queryDispatch(
    const URL& _URL,
    const ::rtl::OUString& TargetFrameName,
    sal_Int32 SearchFlags )
    throw (
        RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    const ::rtl::OUString* pIter = m_aInterceptedURL.getConstArray();
    const ::rtl::OUString* pEnd   = pIter + m_aInterceptedURL.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if ( _URL.Complete == *pIter )
            return (XDispatch*)this;
    }

    if(m_xSlaveDispatchProvider.is())
        return m_xSlaveDispatchProvider->queryDispatch(_URL,TargetFrameName,SearchFlags);
    else
        return Reference<XDispatch>();
}

Sequence< Reference< XDispatch > > SAL_CALL
OInterceptor::queryDispatches(
    const Sequence<DispatchDescriptor >& Requests )
    throw (
        RuntimeException
    )
{
    Sequence< Reference< XDispatch > > aRet;
    osl::MutexGuard aGuard(m_aMutex);
    if(m_xSlaveDispatchProvider.is())
        aRet = m_xSlaveDispatchProvider->queryDispatches(Requests);
    else
        aRet.realloc(Requests.getLength());

    for(sal_Int32 i = 0; i < Requests.getLength(); ++i)
    {
        const ::rtl::OUString* pIter = m_aInterceptedURL.getConstArray();
        const ::rtl::OUString* pEnd   = pIter + m_aInterceptedURL.getLength();
        for(;pIter != pEnd;++pIter)
        {
            if ( Requests[i].FeatureURL.Complete == *pIter )
            {
                aRet[i] = (XDispatch*) this;
                break;
            }
        }
    }

    return aRet;
}



//XDispatchProviderInterceptor

Reference< XDispatchProvider > SAL_CALL
OInterceptor::getSlaveDispatchProvider(  )
    throw (
        RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_xSlaveDispatchProvider;
}

void SAL_CALL
OInterceptor::setSlaveDispatchProvider(
    const Reference< XDispatchProvider >& NewDispatchProvider )
    throw (
        RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xSlaveDispatchProvider = NewDispatchProvider;
}


Reference< XDispatchProvider > SAL_CALL
OInterceptor::getMasterDispatchProvider(  )
    throw (
        RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_xMasterDispatchProvider;
}


void SAL_CALL
OInterceptor::setMasterDispatchProvider(
    const Reference< XDispatchProvider >& NewSupplier )
    throw (
        RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xMasterDispatchProvider = NewSupplier;
}
// -----------------------------------------------------------------------------
void SAL_CALL OInterceptor::notifyEvent( const ::com::sun::star::document::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    osl::ResettableMutexGuard _rGuard(m_aMutex);
    if ( m_pStatCL &&   Event.EventName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnModifyChanged")) )
    {
        OInterfaceContainerHelper* pListener = m_pStatCL->getContainer(m_aInterceptedURL[DISPATCH_SAVE]);
        if ( pListener )
        {
            FeatureStateEvent aEvt;
            aEvt.FeatureURL.Complete = m_aInterceptedURL[DISPATCH_SAVE];
            aEvt.FeatureDescriptor = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Update"));
            Reference<XModifiable> xModel(Event.Source,UNO_QUERY);
            aEvt.IsEnabled = xModel.is() && xModel->isModified();
            aEvt.Requery = sal_False;

            NOTIFY_LISTERNERS((*pListener),XStatusListener,statusChanged)
        }
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OInterceptor::disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException)
{
}

//........................................................................
}   // namespace dbaccess
//........................................................................

