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
#include <cppuhelper/weak.hxx>
#include <comphelper/multiinterfacecontainer2.hxx>

#include <embeddoc.hxx>
#include <docholder.hxx>
#include <intercept.hxx>

using namespace ::com::sun::star;


#define IUL 6

constexpr OUStringLiteral IU0 = u".uno:Save";
constexpr OUStringLiteral IU1 = u".uno:SaveAll";
constexpr OUStringLiteral IU2 = u".uno:CloseDoc";
constexpr OUStringLiteral IU3 = u".uno:CloseWin";
constexpr OUStringLiteral IU4 = u".uno:CloseFrame";
constexpr OUStringLiteral IU5 = u".uno:SaveAs";
const uno::Sequence< OUString > Interceptor::m_aInterceptedURL({ IU0, IU1, IU2, IU3, IU4, IU5});

class StatusChangeListenerContainer
    : public comphelper::OMultiTypeInterfaceContainerHelperVar2<OUString>
{
public:
    explicit StatusChangeListenerContainer(osl::Mutex& aMutex)
        :  comphelper::OMultiTypeInterfaceContainerHelperVar2<OUString>(aMutex)
    {
    }
};


void
Interceptor::addEventListener(
    const uno::Reference<lang::XEventListener >& Listener )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( ! m_pDisposeEventListeners )
        m_pDisposeEventListeners =
            new comphelper::OInterfaceContainerHelper2( m_aMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}


void
Interceptor::removeEventListener(
    const uno::Reference< lang::XEventListener >& Listener )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}


void Interceptor::dispose()
{
    lang::EventObject aEvt;
    aEvt.Source = static_cast< frame::XDispatch* >( this );

    osl::MutexGuard aGuard(m_aMutex);

    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
        m_pDisposeEventListeners->disposeAndClear( aEvt );

    if(m_pStatCL)
        m_pStatCL->disposeAndClear( aEvt );

    m_xSlaveDispatchProvider = nullptr;
    m_xMasterDispatchProvider = nullptr;
}


Interceptor::Interceptor(
    const ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl >& xOleAccess,
    DocumentHolder* pDocH,
    bool bLink )
    : m_xOleAccess( xOleAccess ),
      m_xDocHLocker( static_cast< ::cppu::OWeakObject* >( pDocH ) ),
      m_pDocH(pDocH),
      m_pDisposeEventListeners(nullptr),
      m_pStatCL(nullptr),
      m_bLink( bLink )
{
}


Interceptor::~Interceptor()
{
    delete m_pDisposeEventListeners;
    delete m_pStatCL;

    DocumentHolder* pTmpDocH = nullptr;
    uno::Reference< uno::XInterface > xLock;
    {
        osl::MutexGuard aGuard(m_aMutex);
        xLock = m_xDocHLocker.get();
        if ( xLock.is() )
            pTmpDocH = m_pDocH;
    }

    if ( pTmpDocH )
        pTmpDocH->ClearInterceptor();
}

void Interceptor::DisconnectDocHolder()
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xDocHLocker.clear();
    m_pDocH = nullptr;
    m_xOleAccess = nullptr;
}

//XDispatch
void SAL_CALL
Interceptor::dispatch(
    const util::URL& URL,
    const uno::Sequence<
    beans::PropertyValue >& Arguments )
{
    ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl > xOleAccess;
    {
        osl::MutexGuard aGuard(m_aMutex);
        xOleAccess = m_xOleAccess;
    }

    if ( xOleAccess.is() )
    {
        LockedEmbedDocument_Impl aDocLock = xOleAccess->GetEmbedDocument();
        if ( aDocLock.GetEmbedDocument() )
        {
            if( !m_bLink && URL.Complete == m_aInterceptedURL[0])
                aDocLock.GetEmbedDocument()->SaveObject();
            else if(!m_bLink
                 && ( URL.Complete == m_aInterceptedURL[2] ||
                      URL.Complete == m_aInterceptedURL[3] ||
                      URL.Complete == m_aInterceptedURL[4] ) )
                aDocLock.GetEmbedDocument()->Close( 0 );
            else if ( URL.Complete == m_aInterceptedURL[5] )
            {
                uno::Sequence< beans::PropertyValue > aNewArgs = Arguments;
                sal_Int32 nInd = 0;

                while( nInd < aNewArgs.getLength() )
                {
                    if ( aNewArgs[nInd].Name == "SaveTo" )
                        break;
                    nInd++;
                }

                const bool bNew = nInd == aNewArgs.getLength();
                if (bNew)
                    aNewArgs.realloc( nInd + 1 );
                auto pNewArgs = aNewArgs.getArray();
                if (bNew)
                    pNewArgs[nInd].Name = "SaveTo";
                pNewArgs[nInd].Value <<= true;

                uno::Reference< frame::XDispatch > xDispatch = m_xSlaveDispatchProvider->queryDispatch(
                    URL, "_self", 0 );
                if ( xDispatch.is() )
                    xDispatch->dispatch( URL, aNewArgs );
            }
        }
    }
}


void Interceptor::generateFeatureStateEvent()
{
    if( m_pStatCL )
    {
        DocumentHolder* pTmpDocH = nullptr;
        uno::Reference< uno::XInterface > xLock;
        {
            osl::MutexGuard aGuard(m_aMutex);
            xLock = m_xDocHLocker.get();
            if ( xLock.is() )
                pTmpDocH = m_pDocH;
        }

        OUString aTitle;
        if ( pTmpDocH )
            aTitle = pTmpDocH->getTitle();

        for(int i = 0; i < IUL; ++i)
        {
            if( i == 1 || (m_bLink && i != 5) )
                continue;

            comphelper::OInterfaceContainerHelper2* pICH =
                m_pStatCL->getContainer(m_aInterceptedURL[i]);
            if(!pICH)
                continue;
            std::vector<uno::Reference<uno::XInterface> > aSeq = pICH->getElements();
            if(aSeq.empty())
                continue;

            frame::FeatureStateEvent aStateEvent;
            aStateEvent.IsEnabled = true;
            aStateEvent.Requery = false;
            if(i == 0)
            {

                aStateEvent.FeatureURL.Complete = m_aInterceptedURL[0];
                aStateEvent.FeatureDescriptor = "Update";
                aStateEvent.State <<= "($1) " + aTitle;

            }
            else if ( i == 5 )
            {
                aStateEvent.FeatureURL.Complete = m_aInterceptedURL[5];
                aStateEvent.FeatureDescriptor = "SaveCopyTo";
                aStateEvent.State <<= OUString("($3)");
            }
            else
            {
                aStateEvent.FeatureURL.Complete = m_aInterceptedURL[i];
                aStateEvent.FeatureDescriptor = "Close and Return";
                aStateEvent.State <<= "($2) " + aTitle;

            }

            for(uno::Reference<uno::XInterface> const & k : std::as_const(aSeq))
            {
                uno::Reference<frame::XStatusListener> Control(k,uno::UNO_QUERY);
                if(Control.is())
                    Control->statusChanged(aStateEvent);

            }
        }
    }
}


void SAL_CALL
Interceptor::addStatusListener(
    const uno::Reference<
    frame::XStatusListener >& Control,
    const util::URL& URL )
{
    if(!Control.is())
        return;

    if( !m_bLink && URL.Complete == m_aInterceptedURL[0] )
    {   // Save
        DocumentHolder* pTmpDocH = nullptr;
        uno::Reference< uno::XInterface > xLock;
        {
            osl::MutexGuard aGuard(m_aMutex);
            xLock = m_xDocHLocker.get();
            if ( xLock.is() )
                pTmpDocH = m_pDocH;
        }

        OUString aTitle;
        if ( pTmpDocH )
            aTitle = pTmpDocH->getTitle();

        frame::FeatureStateEvent aStateEvent;
        aStateEvent.FeatureURL.Complete = m_aInterceptedURL[0];
        aStateEvent.FeatureDescriptor = "Update";
        aStateEvent.IsEnabled = true;
        aStateEvent.Requery = false;
        aStateEvent.State <<= "($1) " + aTitle;
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
    if ( !m_bLink
      && ( URL.Complete == m_aInterceptedURL[i] ||
           URL.Complete == m_aInterceptedURL[++i] ||
           URL.Complete == m_aInterceptedURL[++i] ) )
    {   // Close and return
        DocumentHolder* pTmpDocH = nullptr;
        uno::Reference< uno::XInterface > xLock;
        {
            osl::MutexGuard aGuard(m_aMutex);
            xLock = m_xDocHLocker.get();
            if ( xLock.is() )
                pTmpDocH = m_pDocH;
        }

        OUString aTitle;
        if ( pTmpDocH )
            aTitle = pTmpDocH->getTitle();

        frame::FeatureStateEvent aStateEvent;
        aStateEvent.FeatureURL.Complete = m_aInterceptedURL[i];
        aStateEvent.FeatureDescriptor = "Close and Return";
        aStateEvent.IsEnabled = true;
        aStateEvent.Requery = false;
        aStateEvent.State <<= "($2) " + aTitle;
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
        aStateEvent.IsEnabled = true;
        aStateEvent.Requery = false;
        aStateEvent.State <<= OUString("($3)");
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
{
    // now implemented as update
    if ( m_bLink )
        return { m_aInterceptedURL[1], m_aInterceptedURL[5] };

    return m_aInterceptedURL;
}


// XDispatchProvider

uno::Reference< frame::XDispatch > SAL_CALL
Interceptor::queryDispatch(
    const util::URL& URL,
    const OUString& TargetFrameName,
    sal_Int32 SearchFlags )
{
    osl::MutexGuard aGuard(m_aMutex);
    if( !m_bLink && URL.Complete == m_aInterceptedURL[0] )
        return static_cast<frame::XDispatch*>(this);
    else if(URL.Complete == m_aInterceptedURL[1])
        return nullptr;
    else if( !m_bLink && URL.Complete == m_aInterceptedURL[2] )
        return static_cast<frame::XDispatch*>(this);
    else if( !m_bLink && URL.Complete == m_aInterceptedURL[3] )
        return static_cast<frame::XDispatch*>(this);
    else if( !m_bLink && URL.Complete == m_aInterceptedURL[4] )
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
{
    uno::Sequence< uno::Reference< frame::XDispatch > > aRet;
    osl::MutexGuard aGuard(m_aMutex);
    if(m_xSlaveDispatchProvider.is())
        aRet = m_xSlaveDispatchProvider->queryDispatches(Requests);
    else
        aRet.realloc(Requests.getLength());
    auto aRetRange = asNonConstRange(aRet);
    for(sal_Int32 i = 0; i < Requests.getLength(); ++i)
        if ( !m_bLink && m_aInterceptedURL[0] == Requests[i].FeatureURL.Complete )
            aRetRange[i] = static_cast<frame::XDispatch*>(this);
        else if(m_aInterceptedURL[1] == Requests[i].FeatureURL.Complete)
            aRetRange[i] = nullptr;
        else if( !m_bLink && m_aInterceptedURL[2] == Requests[i].FeatureURL.Complete )
            aRetRange[i] = static_cast<frame::XDispatch*>(this);
        else if( !m_bLink && m_aInterceptedURL[3] == Requests[i].FeatureURL.Complete )
            aRetRange[i] = static_cast<frame::XDispatch*>(this);
        else if( !m_bLink && m_aInterceptedURL[4] == Requests[i].FeatureURL.Complete )
            aRetRange[i] = static_cast<frame::XDispatch*>(this);
        else if(m_aInterceptedURL[5] == Requests[i].FeatureURL.Complete)
            aRetRange[i] = static_cast<frame::XDispatch*>(this);

    return aRet;
}


//XDispatchProviderInterceptor

uno::Reference< frame::XDispatchProvider > SAL_CALL
Interceptor::getSlaveDispatchProvider(  )
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_xSlaveDispatchProvider;
}

void SAL_CALL
Interceptor::setSlaveDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& NewDispatchProvider )
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xSlaveDispatchProvider = NewDispatchProvider;
}


uno::Reference< frame::XDispatchProvider > SAL_CALL
Interceptor::getMasterDispatchProvider(  )
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_xMasterDispatchProvider;
}


void SAL_CALL
Interceptor::setMasterDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& NewSupplier )
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xMasterDispatchProvider = NewSupplier;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
