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


#include "intercept.hxx"
#include "stringconstants.hxx"

#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <cppuhelper/weak.hxx>

#include <comphelper/types.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

#include <memory>

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
#define DISPATCH_RELOAD     5
// the OSL_ENSURE in CTOR has to be changed too, when adding new defines

void SAL_CALL OInterceptor::dispose()
{
    EventObject aEvt( *this );

    osl::MutexGuard aGuard(m_aMutex);

    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
        m_pDisposeEventListeners->disposeAndClear( aEvt );

    if ( m_pStatCL )
        m_pStatCL->disposeAndClear( aEvt );

    m_xSlaveDispatchProvider.clear();
    m_xMasterDispatchProvider.clear();

    m_pContentHolder = nullptr;
}


OInterceptor::OInterceptor( ODocumentDefinition* _pContentHolder )
    :m_pContentHolder( _pContentHolder )
    ,m_aInterceptedURL(7)
    ,m_pDisposeEventListeners(nullptr)
    ,m_pStatCL(nullptr)
{

    OSL_ENSURE(DISPATCH_RELOAD < m_aInterceptedURL.getLength(),"Illegal size.");

    m_aInterceptedURL[DISPATCH_SAVEAS]      = ".uno:SaveAs";
    m_aInterceptedURL[DISPATCH_SAVE]        = ".uno:Save";
    m_aInterceptedURL[DISPATCH_CLOSEDOC]    = ".uno:CloseDoc";
    m_aInterceptedURL[DISPATCH_CLOSEWIN]    = ".uno:CloseWin";
    m_aInterceptedURL[DISPATCH_CLOSEFRAME]  = ".uno:CloseFrame";
    m_aInterceptedURL[DISPATCH_RELOAD]      = ".uno:Reload";
}


OInterceptor::~OInterceptor()
{
    delete m_pDisposeEventListeners;
    delete m_pStatCL;
}

struct DispatchHelper
{
    URL aURL;
    Sequence<PropertyValue > aArguments;
};

//XDispatch
void SAL_CALL OInterceptor::dispatch( const URL& URL,const Sequence<PropertyValue >& Arguments )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_pContentHolder )
        return;

    if ( URL.Complete == m_aInterceptedURL[ DISPATCH_SAVE ] )
    {
        m_pContentHolder->save( false );
        return;
    }

    if ( URL.Complete == m_aInterceptedURL[ DISPATCH_RELOAD ] )
    {
        ODocumentDefinition::fillReportData(
            m_pContentHolder->getContext(),
            m_pContentHolder->getComponent(),
            m_pContentHolder->getConnection()
        );
        return;
    }

    if( URL.Complete == m_aInterceptedURL[ DISPATCH_SAVEAS ] )
    {
        if ( m_pContentHolder->isNewReport() )
        {
            m_pContentHolder->saveAs();
        }
        else if ( m_xSlaveDispatchProvider.is() )
        {
            Sequence< PropertyValue > aNewArgs = Arguments;
            sal_Int32 nInd = 0;

            while( nInd < aNewArgs.getLength() )
            {
                if ( aNewArgs[nInd].Name == "SaveTo" )
                {
                    aNewArgs[nInd].Value <<= true;
                    break;
                }
                nInd++;
            }

            if ( nInd == aNewArgs.getLength() )
            {
                aNewArgs.realloc( nInd + 1 );
                aNewArgs[nInd].Name = "SaveTo";
                aNewArgs[nInd].Value <<= true;
            }

            Reference< XDispatch > xDispatch = m_xSlaveDispatchProvider->queryDispatch(URL, "_self", 0 );
            if ( xDispatch.is() )
                xDispatch->dispatch( URL, aNewArgs );
        }
        return;
    }

    if  (   URL.Complete == m_aInterceptedURL[ DISPATCH_CLOSEDOC ]
        ||  URL.Complete == m_aInterceptedURL[ DISPATCH_CLOSEWIN ]
        ||  URL.Complete == m_aInterceptedURL[ DISPATCH_CLOSEFRAME ]
        )
    {
        DispatchHelper* pHelper = new DispatchHelper;
        pHelper->aArguments = Arguments;
        pHelper->aURL = URL;
        Application::PostUserEvent( LINK( this, OInterceptor, OnDispatch ), pHelper );
        return;
    }
}

IMPL_LINK( OInterceptor, OnDispatch, void*, _pDispatcher, void )
{
    std::unique_ptr<DispatchHelper> pHelper( static_cast< DispatchHelper* >( _pDispatcher ) );
    try
    {
        if ( m_pContentHolder && m_pContentHolder->prepareClose() && m_xSlaveDispatchProvider.is() )
        {
            Reference< XDispatch > xDispatch = m_xSlaveDispatchProvider->queryDispatch(pHelper->aURL, "_self", 0 );
            if ( xDispatch.is() )
            {
                Reference< XInterface > xKeepContentHolderAlive( *m_pContentHolder );
                xDispatch->dispatch( pHelper->aURL,pHelper->aArguments);
            }
        }
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void SAL_CALL OInterceptor::addStatusListener(
    const Reference<
    XStatusListener >& Control,
    const URL& URL )
{
    if(!Control.is())
        return;

    if ( m_pContentHolder && URL.Complete == m_aInterceptedURL[DISPATCH_SAVEAS] )
    {   // SaveAs

        if ( !m_pContentHolder->isNewReport() )
        {
            FeatureStateEvent aStateEvent;
            aStateEvent.FeatureURL.Complete = m_aInterceptedURL[DISPATCH_SAVEAS];
            aStateEvent.FeatureDescriptor = "SaveCopyTo";
            aStateEvent.IsEnabled = true;
            aStateEvent.Requery = false;
            aStateEvent.State <<= OUString("($3)");
            Control->statusChanged(aStateEvent);
        }

        {
            osl::MutexGuard aGuard(m_aMutex);
            if(!m_pStatCL)
                m_pStatCL = new PropertyChangeListenerContainer(m_aMutex);
        }

        m_pStatCL->addInterface(URL.Complete,Control);
    }
    else if ( m_pContentHolder && URL.Complete == m_aInterceptedURL[DISPATCH_SAVE] )
    {   // Save
        FeatureStateEvent aStateEvent;
        aStateEvent.FeatureURL.Complete = m_aInterceptedURL[DISPATCH_SAVE];
        aStateEvent.FeatureDescriptor = "Update";
        aStateEvent.IsEnabled = true;
        aStateEvent.Requery = false;

        Control->statusChanged(aStateEvent);
        {
            osl::MutexGuard aGuard(m_aMutex);
            if(!m_pStatCL)
                m_pStatCL = new PropertyChangeListenerContainer(m_aMutex);
        }

        m_pStatCL->addInterface(URL.Complete,Control);
    }
    else
    {
        sal_Int32 i = 2;
        if(URL.Complete == m_aInterceptedURL[i] ||
            URL.Complete == m_aInterceptedURL[++i] ||
            URL.Complete == m_aInterceptedURL[++i] ||
            URL.Complete == m_aInterceptedURL[i = DISPATCH_RELOAD] )
        {   // Close and return
            FeatureStateEvent aStateEvent;
            aStateEvent.FeatureURL.Complete = m_aInterceptedURL[i];
            aStateEvent.FeatureDescriptor = "Close and Return";
            aStateEvent.IsEnabled = true;
            aStateEvent.Requery = false;
            Control->statusChanged(aStateEvent);


            {
                osl::MutexGuard aGuard(m_aMutex);
                if(!m_pStatCL)
                    m_pStatCL = new PropertyChangeListenerContainer(m_aMutex);
            }

            m_pStatCL->addInterface(URL.Complete,Control);
            return;
        }
    }
}


void SAL_CALL OInterceptor::removeStatusListener(
    const Reference<
    XStatusListener >& Control,
    const URL& URL )
{
    if(!(Control.is() && m_pStatCL))
        return;
    else
    {
        m_pStatCL->removeInterface(URL.Complete,Control);
        return;
    }
}


//XInterceptorInfo
Sequence< OUString > SAL_CALL OInterceptor::getInterceptedURLs(  )
{
    // now implemented as update
    return m_aInterceptedURL;
}


// XDispatchProvider

Reference< XDispatch > SAL_CALL OInterceptor::queryDispatch( const URL& URL,const OUString& TargetFrameName,sal_Int32 SearchFlags )
{
    osl::MutexGuard aGuard(m_aMutex);
    const OUString* pIter = m_aInterceptedURL.getConstArray();
    const OUString* pEnd   = pIter + m_aInterceptedURL.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if ( URL.Complete == *pIter )
            return static_cast<XDispatch*>(this);
    }

    if(m_xSlaveDispatchProvider.is())
        return m_xSlaveDispatchProvider->queryDispatch(URL,TargetFrameName,SearchFlags);
    else
        return Reference<XDispatch>();
}

Sequence< Reference< XDispatch > > SAL_CALL OInterceptor::queryDispatches(  const Sequence<DispatchDescriptor >& Requests )
{
    Sequence< Reference< XDispatch > > aRet;
    osl::MutexGuard aGuard(m_aMutex);
    if(m_xSlaveDispatchProvider.is())
        aRet = m_xSlaveDispatchProvider->queryDispatches(Requests);
    else
        aRet.realloc(Requests.getLength());

    for(sal_Int32 i = 0; i < Requests.getLength(); ++i)
    {
        const OUString* pIter = m_aInterceptedURL.getConstArray();
        const OUString* pEnd   = pIter + m_aInterceptedURL.getLength();
        for(;pIter != pEnd;++pIter)
        {
            if ( Requests[i].FeatureURL.Complete == *pIter )
            {
                aRet[i] = static_cast<XDispatch*>(this);
                break;
            }
        }
    }

    return aRet;
}


//XDispatchProviderInterceptor

Reference< XDispatchProvider > SAL_CALL OInterceptor::getSlaveDispatchProvider(  )
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_xSlaveDispatchProvider;
}

void SAL_CALL
OInterceptor::setSlaveDispatchProvider( const Reference< XDispatchProvider >& NewDispatchProvider )
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xSlaveDispatchProvider = NewDispatchProvider;
}


Reference< XDispatchProvider > SAL_CALL OInterceptor::getMasterDispatchProvider(  )
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_xMasterDispatchProvider;
}


void SAL_CALL OInterceptor::setMasterDispatchProvider(
    const Reference< XDispatchProvider >& NewSupplier )
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xMasterDispatchProvider = NewSupplier;
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
