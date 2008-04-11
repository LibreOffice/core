/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: intercept.cxx,v $
 * $Revision: 1.14 $
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
#include <cppuhelper/weak.hxx>

#include <embeddoc.hxx>
#include <docholder.hxx>
#include <intercept.hxx>

using namespace ::com::sun::star;



#define IUL 6



uno::Sequence< ::rtl::OUString > Interceptor::m_aInterceptedURL(IUL);




struct equalOUString
{
    bool operator()(
        const rtl::OUString& rKey1,
        const rtl::OUString& rKey2 ) const
    {
        return !!( rKey1 == rKey2 );
    }
};


struct hashOUString
{
    size_t operator()( const rtl::OUString& rName ) const
    {
        return rName.hashCode();
    }
};



class StatusChangeListenerContainer
    : public ::cppu::OMultiTypeInterfaceContainerHelperVar<
rtl::OUString,hashOUString,equalOUString>
{
public:
    StatusChangeListenerContainer( ::osl::Mutex& aMutex )
        :  cppu::OMultiTypeInterfaceContainerHelperVar<
    rtl::OUString,hashOUString,equalOUString>(aMutex)
    {
    }
};


void SAL_CALL
Interceptor::addEventListener(
    const uno::Reference<lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( ! m_pDisposeEventListeners )
        m_pDisposeEventListeners =
            new cppu::OInterfaceContainerHelper( m_aMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}


void SAL_CALL
Interceptor::removeEventListener(
    const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}


void SAL_CALL Interceptor::dispose()
    throw(::com::sun::star::uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = static_cast< frame::XDispatch* >( this );

    osl::MutexGuard aGuard(m_aMutex);

    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
        m_pDisposeEventListeners->disposeAndClear( aEvt );

    if(m_pStatCL)
        m_pStatCL->disposeAndClear( aEvt );

    m_xSlaveDispatchProvider = 0;
    m_xMasterDispatchProvider = 0;
}



Interceptor::Interceptor(
    const ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl >& xOleAccess,
    DocumentHolder* pDocH,
    sal_Bool bLink )
    : m_xOleAccess( xOleAccess ),
      m_xDocHLocker( static_cast< ::cppu::OWeakObject* >( pDocH ) ),
      m_pDocH(pDocH),
      m_pStatCL(0),
      m_pDisposeEventListeners(0),
      m_bLink( bLink )
{
    m_aInterceptedURL[0] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(".uno:Save"));
    m_aInterceptedURL[1] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(".uno:SaveAll"));
    m_aInterceptedURL[2] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(".uno:CloseDoc"));
    m_aInterceptedURL[3] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(".uno:CloseWin"));
    m_aInterceptedURL[4] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(".uno:CloseFrame"));
    m_aInterceptedURL[5] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(".uno:SaveAs"));
}


Interceptor::~Interceptor()
{
    if( m_pDisposeEventListeners )
        delete m_pDisposeEventListeners;

    if(m_pStatCL)
        delete m_pStatCL;

    DocumentHolder* pTmpDocH = NULL;
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
    m_xDocHLocker = uno::Reference< uno::XInterface >();
    m_pDocH = NULL;
    m_xOleAccess = NULL;
}

//XDispatch
void SAL_CALL
Interceptor::dispatch(
    const util::URL& URL,
    const uno::Sequence<
    beans::PropertyValue >& Arguments )
    throw (uno::RuntimeException)
{
    ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl > xOleAccess;
    {
        osl::MutexGuard aGuard(m_aMutex);
        xOleAccess = m_xOleAccess;
    }

    if ( xOleAccess.is() )
    {
        LockedEmbedDocument_Impl aDocLock = xOleAccess->GetEmbedDocument();
        if ( aDocLock.m_pEmbedDocument )
        {
            if( !m_bLink && URL.Complete == m_aInterceptedURL[0])
                aDocLock.m_pEmbedDocument->SaveObject();
            else if(!m_bLink
                    && ( URL.Complete == m_aInterceptedURL[2] ||
                         URL.Complete == m_aInterceptedURL[3] ||
                         URL.Complete == m_aInterceptedURL[4] ) )
                aDocLock.m_pEmbedDocument->Close( 0 );
            else if ( URL.Complete == m_aInterceptedURL[5] )
            {
                uno::Sequence< beans::PropertyValue > aNewArgs = Arguments;
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

                uno::Reference< frame::XDispatch > xDispatch = m_xSlaveDispatchProvider->queryDispatch(
                    URL, ::rtl::OUString::createFromAscii( "_self" ), 0 );
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
        DocumentHolder* pTmpDocH = NULL;
        uno::Reference< uno::XInterface > xLock;
        {
            osl::MutexGuard aGuard(m_aMutex);
            xLock = m_xDocHLocker.get();
            if ( xLock.is() )
                pTmpDocH = m_pDocH;
        }

        ::rtl::OUString aTitle;
        if ( pTmpDocH )
            aTitle = pTmpDocH->getTitle();

        for(int i = 0; i < IUL; ++i)
        {
            if( i == 1 || m_bLink && i != 5 )
                continue;

            cppu::OInterfaceContainerHelper* pICH =
                m_pStatCL->getContainer(m_aInterceptedURL[i]);
            uno::Sequence<uno::Reference<uno::XInterface> > aSeq;
            if(pICH)
                aSeq = pICH->getElements();
            if(!aSeq.getLength())
                continue;

            frame::FeatureStateEvent aStateEvent;
            aStateEvent.IsEnabled = sal_True;
            aStateEvent.Requery = sal_False;
            if(i == 0)
            {

                aStateEvent.FeatureURL.Complete = m_aInterceptedURL[0];
                aStateEvent.FeatureDescriptor = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("Update"));
                aStateEvent.State <<= (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("($1) ")) +
                                       aTitle);

            }
            else if ( i == 5 )
            {
                aStateEvent.FeatureURL.Complete = m_aInterceptedURL[5];
                aStateEvent.FeatureDescriptor = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("SaveCopyTo"));
                aStateEvent.State <<= (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("($3)")));
            }
            else
            {
                aStateEvent.FeatureURL.Complete = m_aInterceptedURL[i];
                aStateEvent.FeatureDescriptor = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("Close and Return"));
                aStateEvent.State <<= (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("($2) ")) +
                                       aTitle);

            }

            for(sal_Int32 k = 0; k < aSeq.getLength(); ++k)
            {
                uno::Reference<frame::XStatusListener>
                    Control(aSeq[k],uno::UNO_QUERY);
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
    throw (
        uno::RuntimeException
    )
{
    if(!Control.is())
        return;

    if( !m_bLink && URL.Complete == m_aInterceptedURL[0] )
    {   // Save
        DocumentHolder* pTmpDocH = NULL;
        uno::Reference< uno::XInterface > xLock;
        {
            osl::MutexGuard aGuard(m_aMutex);
            xLock = m_xDocHLocker.get();
            if ( xLock.is() )
                pTmpDocH = m_pDocH;
        }

        ::rtl::OUString aTitle;
        if ( pTmpDocH )
            aTitle = pTmpDocH->getTitle();

        frame::FeatureStateEvent aStateEvent;
        aStateEvent.FeatureURL.Complete = m_aInterceptedURL[0];
        aStateEvent.FeatureDescriptor = rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("Update"));
        aStateEvent.IsEnabled = sal_True;
        aStateEvent.Requery = sal_False;
        aStateEvent.State <<= (rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("($1) ")) +
                               aTitle );
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
        DocumentHolder* pTmpDocH = NULL;
        uno::Reference< uno::XInterface > xLock;
        {
            osl::MutexGuard aGuard(m_aMutex);
            xLock = m_xDocHLocker.get();
            if ( xLock.is() )
                pTmpDocH = m_pDocH;
        }

        ::rtl::OUString aTitle;
        if ( pTmpDocH )
            aTitle = pTmpDocH->getTitle();

        frame::FeatureStateEvent aStateEvent;
        aStateEvent.FeatureURL.Complete = m_aInterceptedURL[i];
        aStateEvent.FeatureDescriptor = rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("Close and Return"));
        aStateEvent.IsEnabled = sal_True;
        aStateEvent.Requery = sal_False;
        aStateEvent.State <<= (rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("($2) ")) +
                               aTitle );
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
        aStateEvent.FeatureDescriptor = rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("SaveCopyTo"));
        aStateEvent.IsEnabled = sal_True;
        aStateEvent.Requery = sal_False;
        aStateEvent.State <<= (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("($3)")));
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
        uno::RuntimeException
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
uno::Sequence< ::rtl::OUString >
SAL_CALL
Interceptor::getInterceptedURLs(  )
    throw (
        uno::RuntimeException
    )
{
    // now implemented as update
    if ( m_bLink )
    {
        uno::Sequence< ::rtl::OUString > aResult( 2 );
        aResult[0] = m_aInterceptedURL[1];
        aResult[1] = m_aInterceptedURL[5];

        return aResult;
    }

    return m_aInterceptedURL;
}


// XDispatchProvider

uno::Reference< frame::XDispatch > SAL_CALL
Interceptor::queryDispatch(
    const util::URL& URL,
    const ::rtl::OUString& TargetFrameName,
    sal_Int32 SearchFlags )
    throw (
        uno::RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    if( !m_bLink && URL.Complete == m_aInterceptedURL[0] )
        return (frame::XDispatch*)this;
    else if(URL.Complete == m_aInterceptedURL[1])
        return (frame::XDispatch*)0   ;
    else if( !m_bLink && URL.Complete == m_aInterceptedURL[2] )
        return (frame::XDispatch*)this;
    else if( !m_bLink && URL.Complete == m_aInterceptedURL[3] )
        return (frame::XDispatch*)this;
    else if( !m_bLink && URL.Complete == m_aInterceptedURL[4] )
        return (frame::XDispatch*)this;
    else if(URL.Complete == m_aInterceptedURL[5])
        return (frame::XDispatch*)this;
    else {
        if(m_xSlaveDispatchProvider.is())
            return m_xSlaveDispatchProvider->queryDispatch(
                URL,TargetFrameName,SearchFlags);
        else
            return uno::Reference<frame::XDispatch>(0);
    }
}

uno::Sequence< uno::Reference< frame::XDispatch > > SAL_CALL
Interceptor::queryDispatches(
    const uno::Sequence<frame::DispatchDescriptor >& Requests )
    throw (
        uno::RuntimeException
    )
{
    uno::Sequence< uno::Reference< frame::XDispatch > > aRet;
    osl::MutexGuard aGuard(m_aMutex);
    if(m_xSlaveDispatchProvider.is())
        aRet = m_xSlaveDispatchProvider->queryDispatches(Requests);
    else
        aRet.realloc(Requests.getLength());

    for(sal_Int32 i = 0; i < Requests.getLength(); ++i)
        if ( !m_bLink && m_aInterceptedURL[0] == Requests[i].FeatureURL.Complete )
            aRet[i] = (frame::XDispatch*) this;
        else if(m_aInterceptedURL[1] == Requests[i].FeatureURL.Complete)
            aRet[i] = (frame::XDispatch*) 0;
        else if( !m_bLink && m_aInterceptedURL[2] == Requests[i].FeatureURL.Complete )
            aRet[i] = (frame::XDispatch*) this;
        else if( !m_bLink && m_aInterceptedURL[3] == Requests[i].FeatureURL.Complete )
            aRet[i] = (frame::XDispatch*) this;
        else if( !m_bLink && m_aInterceptedURL[4] == Requests[i].FeatureURL.Complete )
            aRet[i] = (frame::XDispatch*) this;
        else if(m_aInterceptedURL[5] == Requests[i].FeatureURL.Complete)
            aRet[i] = (frame::XDispatch*) this;

    return aRet;
}



//XDispatchProviderInterceptor

uno::Reference< frame::XDispatchProvider > SAL_CALL
Interceptor::getSlaveDispatchProvider(  )
    throw (
        uno::RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_xSlaveDispatchProvider;
}

void SAL_CALL
Interceptor::setSlaveDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& NewDispatchProvider )
    throw (
        uno::RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xSlaveDispatchProvider = NewDispatchProvider;
}


uno::Reference< frame::XDispatchProvider > SAL_CALL
Interceptor::getMasterDispatchProvider(  )
    throw (
        uno::RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_xMasterDispatchProvider;
}


void SAL_CALL
Interceptor::setMasterDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& NewSupplier )
    throw (
        uno::RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xMasterDispatchProvider = NewSupplier;
}

// Fix strange warnings about some
// ATL::CAxHostWindow::QueryInterface|AddRef|Releae functions.
// warning C4505: 'xxx' : unreferenced local function has been removed
#if defined(_MSC_VER)
#pragma warning(disable: 4505)
#endif
