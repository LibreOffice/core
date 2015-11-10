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

#include <com/sun/star/embed/EmbedStates.hpp>
#include <cppuhelper/weak.hxx>

#include "intercept.hxx"
#include "docholder.hxx"
#include "commonembobj.hxx"

using namespace ::com::sun::star;

#define IUL 6

uno::Sequence< OUString > Interceptor::m_aInterceptedURL(IUL);

class StatusChangeListenerContainer
    : public cppu::OMultiTypeInterfaceContainerHelperVar<OUString>
{
public:
    explicit StatusChangeListenerContainer(osl::Mutex& aMutex)
        :  cppu::OMultiTypeInterfaceContainerHelperVar<OUString>(aMutex)
    {
    }
};

void Interceptor::DisconnectDocHolder()
{
    osl::MutexGuard aGuard( m_aMutex );
    m_pDocHolder = nullptr;
}

Interceptor::Interceptor( DocumentHolder* pDocHolder )
    : m_pDocHolder( pDocHolder ),
      m_pDisposeEventListeners(nullptr),
      m_pStatCL(nullptr)
{
    m_aInterceptedURL[0] = ".uno:Save";
    m_aInterceptedURL[1] = ".uno:SaveAll";
    m_aInterceptedURL[2] = ".uno:CloseDoc";
    m_aInterceptedURL[3] = ".uno:CloseWin";
    m_aInterceptedURL[4] = ".uno:CloseFrame";
    m_aInterceptedURL[5] = ".uno:SaveAs";

}

Interceptor::~Interceptor()
{
    delete m_pDisposeEventListeners;
    delete m_pStatCL;
}

//XDispatch
void SAL_CALL
Interceptor::dispatch(
    const util::URL& URL,
    const uno::Sequence<
    beans::PropertyValue >& Arguments )
    throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard aGuard(m_aMutex);
    if( m_pDocHolder )
    {
        if(URL.Complete == m_aInterceptedURL[0])
            m_pDocHolder->GetEmbedObject()->SaveObject_Impl();
        else if(URL.Complete == m_aInterceptedURL[2] ||
                URL.Complete == m_aInterceptedURL[3] ||
                URL.Complete == m_aInterceptedURL[4])
        {
            try {
                m_pDocHolder->GetEmbedObject()->changeState( embed::EmbedStates::RUNNING );
            }
            catch( const uno::Exception& )
            {
            }
        }
        else if ( URL.Complete == m_aInterceptedURL[5] )
        {
            uno::Sequence< beans::PropertyValue > aNewArgs = Arguments;
            sal_Int32 nInd = 0;

            while( nInd < aNewArgs.getLength() )
            {
                if ( aNewArgs[nInd].Name == "SaveTo" )
                {
                    aNewArgs[nInd].Value <<= sal_True;
                    break;
                }
                nInd++;
            }

            if ( nInd == aNewArgs.getLength() )
            {
                aNewArgs.realloc( nInd + 1 );
                aNewArgs[nInd].Name = "SaveTo";
                aNewArgs[nInd].Value <<= sal_True;
            }

            uno::Reference< frame::XDispatch > xDispatch = m_xSlaveDispatchProvider->queryDispatch(
                URL, "_self", 0 );
            if ( xDispatch.is() )
                xDispatch->dispatch( URL, aNewArgs );
        }
    }
}

void SAL_CALL
Interceptor::addStatusListener(
    const uno::Reference<
    frame::XStatusListener >& Control,
    const util::URL& URL )
    throw (
        uno::RuntimeException, std::exception
    )
{
    if(!Control.is())
        return;

    if(URL.Complete == m_aInterceptedURL[0])
    {   // Save
        frame::FeatureStateEvent aStateEvent;
        aStateEvent.FeatureURL.Complete = m_aInterceptedURL[0];
        aStateEvent.FeatureDescriptor = "Update";
        aStateEvent.IsEnabled = sal_True;
        aStateEvent.Requery = sal_False;
        aStateEvent.State <<= ( "($1) " + m_pDocHolder->GetTitle() );
        Control->statusChanged(aStateEvent);

        {
            osl::MutexGuard aGuard(m_aMutex);
            if(!m_pStatCL)
                m_pStatCL =
                    new StatusChangeListenerContainer(m_aMutex);
        }

        m_pStatCL->addInterface(URL.Complete,Control);
        return;
    }

    sal_Int32 i = 2;
    if(URL.Complete == m_aInterceptedURL[i] ||
       URL.Complete == m_aInterceptedURL[++i] ||
       URL.Complete == m_aInterceptedURL[++i] )
    {   // Close and return
        frame::FeatureStateEvent aStateEvent;
        aStateEvent.FeatureURL.Complete = m_aInterceptedURL[i];
        aStateEvent.FeatureDescriptor = "Close and Return";
        aStateEvent.IsEnabled = sal_True;
        aStateEvent.Requery = sal_False;
        aStateEvent.State <<= ( "($2) " + m_pDocHolder->GetTitle() );
        Control->statusChanged(aStateEvent);


        {
            osl::MutexGuard aGuard(m_aMutex);
            if(!m_pStatCL)
                m_pStatCL =
                    new StatusChangeListenerContainer(m_aMutex);
        }

        m_pStatCL->addInterface(URL.Complete,Control);
        return;
    }

    if(URL.Complete == m_aInterceptedURL[5])
    {   // SaveAs
        frame::FeatureStateEvent aStateEvent;
        aStateEvent.FeatureURL.Complete = m_aInterceptedURL[5];
        aStateEvent.FeatureDescriptor = "SaveCopyTo";
        aStateEvent.IsEnabled = sal_True;
        aStateEvent.Requery = sal_False;
        aStateEvent.State <<= (OUString( "($3)"));
        Control->statusChanged(aStateEvent);

        {
            osl::MutexGuard aGuard(m_aMutex);
            if(!m_pStatCL)
                m_pStatCL =
                    new StatusChangeListenerContainer(m_aMutex);
        }

        m_pStatCL->addInterface(URL.Complete,Control);
        return;
    }

}


void SAL_CALL
Interceptor::removeStatusListener(
    const uno::Reference<
    frame::XStatusListener >& Control,
    const util::URL& URL )
    throw (
        uno::RuntimeException, std::exception
    )
{
    if(!(Control.is() && m_pStatCL))
        return;
    else {
        m_pStatCL->removeInterface(URL.Complete,Control);
        return;
    }
}


//XInterceptorInfo
uno::Sequence< OUString >
SAL_CALL
Interceptor::getInterceptedURLs(  )
    throw (
        uno::RuntimeException, std::exception
    )
{
    // now implemented as update

    return m_aInterceptedURL;
}


// XDispatchProvider

uno::Reference< frame::XDispatch > SAL_CALL
Interceptor::queryDispatch(
    const util::URL& URL,
    const OUString& TargetFrameName,
    sal_Int32 SearchFlags )
    throw (
        uno::RuntimeException, std::exception
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    if(URL.Complete == m_aInterceptedURL[0])
        return static_cast<frame::XDispatch*>(this);
    else if(URL.Complete == m_aInterceptedURL[1])
        return nullptr   ;
    else if(URL.Complete == m_aInterceptedURL[2])
        return static_cast<frame::XDispatch*>(this);
    else if(URL.Complete == m_aInterceptedURL[3])
        return static_cast<frame::XDispatch*>(this);
    else if(URL.Complete == m_aInterceptedURL[4])
        return static_cast<frame::XDispatch*>(this);
    else if(URL.Complete == m_aInterceptedURL[5])
        return static_cast<frame::XDispatch*>(this);
    else {
        if(m_xSlaveDispatchProvider.is())
            return m_xSlaveDispatchProvider->queryDispatch(
                URL,TargetFrameName,SearchFlags);
        else
            return uno::Reference<frame::XDispatch>(nullptr);
    }
}

uno::Sequence< uno::Reference< frame::XDispatch > > SAL_CALL
Interceptor::queryDispatches(
    const uno::Sequence<frame::DispatchDescriptor >& Requests )
    throw (
        uno::RuntimeException, std::exception
    )
{
    uno::Sequence< uno::Reference< frame::XDispatch > > aRet;
    osl::MutexGuard aGuard(m_aMutex);
    if(m_xSlaveDispatchProvider.is())
        aRet = m_xSlaveDispatchProvider->queryDispatches(Requests);
    else
        aRet.realloc(Requests.getLength());

    for(sal_Int32 i = 0; i < Requests.getLength(); ++i)
        if(m_aInterceptedURL[0] == Requests[i].FeatureURL.Complete)
            aRet[i] = static_cast<frame::XDispatch*>(this);
        else if(m_aInterceptedURL[1] == Requests[i].FeatureURL.Complete)
            aRet[i] = nullptr;
        else if(m_aInterceptedURL[2] == Requests[i].FeatureURL.Complete)
            aRet[i] = static_cast<frame::XDispatch*>(this);
        else if(m_aInterceptedURL[3] == Requests[i].FeatureURL.Complete)
            aRet[i] = static_cast<frame::XDispatch*>(this);
        else if(m_aInterceptedURL[4] == Requests[i].FeatureURL.Complete)
            aRet[i] = static_cast<frame::XDispatch*>(this);
        else if(m_aInterceptedURL[5] == Requests[i].FeatureURL.Complete)
            aRet[i] = static_cast<frame::XDispatch*>(this);

    return aRet;
}



//XDispatchProviderInterceptor

uno::Reference< frame::XDispatchProvider > SAL_CALL
Interceptor::getSlaveDispatchProvider(  )
    throw (
        uno::RuntimeException, std::exception
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_xSlaveDispatchProvider;
}

void SAL_CALL
Interceptor::setSlaveDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& NewDispatchProvider )
    throw (
        uno::RuntimeException, std::exception
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xSlaveDispatchProvider = NewDispatchProvider;
}


uno::Reference< frame::XDispatchProvider > SAL_CALL
Interceptor::getMasterDispatchProvider(  )
    throw (
        uno::RuntimeException, std::exception
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_xMasterDispatchProvider;
}


void SAL_CALL
Interceptor::setMasterDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& NewSupplier )
    throw (
        uno::RuntimeException, std::exception
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xMasterDispatchProvider = NewSupplier;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
