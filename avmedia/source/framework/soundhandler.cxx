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

#include "soundhandler.hxx"

#include <comphelper/mediadescriptor.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <rtl/ustrbuf.hxx>

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/factory.hxx>

namespace avmedia{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************

void SAL_CALL SoundHandler::acquire() throw()
{
       /* Don't use mutex in methods of XInterface! */
       OWeakObject::acquire();
}

void SAL_CALL SoundHandler::release() throw()
{
       /* Don't use mutex in methods of XInterface! */
       OWeakObject::release();
}

css::uno::Any SAL_CALL SoundHandler::queryInterface( const css::uno::Type& aType ) throw( css::uno::RuntimeException )
{
       /* Attention: Don't use mutex or guard in this method!!! Is a method of XInterface.     */
        /* Ask for my own supported interfaces ...*/
       css::uno::Any aReturn( ::cppu::queryInterface( aType,
               static_cast< css::lang::XTypeProvider* >(this),
               static_cast< css::lang::XServiceInfo* >(this),
               static_cast< css::frame::XNotifyingDispatch* >(this),
               static_cast< css::frame::XDispatch* >(this),
               static_cast< css::document::XExtendedFilterDetection* >(this)));
       /* If searched interface not supported by this class ... */
       if ( aReturn.hasValue() == sal_False )
       {
               /* ... ask baseclass for interfaces! */
               aReturn = OWeakObject::queryInterface( aType );
       }
        /* Return result of this search. */
       return aReturn;
}

css::uno::Sequence< sal_Int8 > SAL_CALL SoundHandler::getImplementationId() throw( css::uno::RuntimeException )
{
    /* Create one Id for all instances of this class.                                               */
    /* Use ethernet address to do this! (sal_True)                                                  */
    /* Optimize this method                                                                         */
    /* We initialize a static variable only one time. And we don't must use a mutex at every call!  */
    /* For the first call; pID is NULL - for the second call pID is different from NULL!            */
    static ::cppu::OImplementationId* pID = NULL ;
    if ( pID == NULL )
    {
        /* Ready for multithreading; get global mutex for first call of this method only! see before   */
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        /* Control these pointer again ... it can be, that another instance will be faster then these! */
        if ( pID == NULL )
        {
            /* Create a new static ID ... */
            static ::cppu::OImplementationId aID( sal_False );
            /* ... and set his address to static pointer! */
            pID = &aID ;
        }
    }
    return pID->getImplementationId();
}

css::uno::Sequence< css::uno::Type > SAL_CALL SoundHandler::getTypes() throw( css::uno::RuntimeException )
{
    /* Optimize this method !                                       */
    /* We initialize a static variable only one time.               */
    /* And we don't must use a mutex at every call!                 */
    /* For the first call; pTypeCollection is NULL -                */
    /* for the second call pTypeCollection is different from NULL!  */
    static ::cppu::OTypeCollection* pTypeCollection = NULL ;
    if ( pTypeCollection == NULL )
    {
        /* Ready for multithreading; get global mutex for first call of this method only! see before   */
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        /* Control these pointer again ... it can be, that another instance will be faster then these! */
        if ( pTypeCollection == NULL )
        {
            /* Create a static typecollection ...           */
            static ::cppu::OTypeCollection aTypeCollection
                (
                    ::getCppuType(( const ::com::sun::star::uno::Reference< css::lang::XTypeProvider >*)NULL ),
                    ::getCppuType(( const ::com::sun::star::uno::Reference< css::lang::XServiceInfo >*)NULL ),
                    ::getCppuType(( const ::com::sun::star::uno::Reference< css::frame::XNotifyingDispatch >*)NULL ),
                    ::getCppuType(( const ::com::sun::star::uno::Reference< css::frame::XDispatch >*)NULL ),
                    ::getCppuType(( const ::com::sun::star::uno::Reference< css::document::XExtendedFilterDetection >*)NULL )
                );
            /* ... and set his address to static pointer! */
            pTypeCollection = &aTypeCollection ;
        }
    }
    return pTypeCollection->getTypes();
}

#define DECLARE_ASCII( SASCIIVALUE ) \
        ::rtl::OUString( SASCIIVALUE  )

#define IMPLEMENTATIONNAME_SOUNDHANDLER DECLARE_ASCII("com.sun.star.comp.framework.SoundHandler")
#define SERVICENAME_CONTENTHANDLER DECLARE_ASCII("com.sun.star.frame.ContentHandler")

/*===========================================================================================================*/
/* XServiceInfo */
/*===========================================================================================================*/
::rtl::OUString SAL_CALL SoundHandler::getImplementationName() throw( css::uno::RuntimeException )
{
    return impl_getStaticImplementationName();
}

/*===========================================================================================================*/
/* XServiceInfo */
/*===========================================================================================================*/
sal_Bool SAL_CALL SoundHandler::supportsService( const ::rtl::OUString& sServiceName ) throw( css::uno::RuntimeException )
{
    /* Set default return value. */
    sal_Bool bReturn = sal_False ;
    /* Get names of all supported servicenames. */
    css::uno::Sequence< ::rtl::OUString >  seqServiceNames =   getSupportedServiceNames();
    const ::rtl::OUString*                 pArray          =   seqServiceNames.getConstArray();
    sal_Int32                              nCounter        =   0;
    sal_Int32                              nLength         =   seqServiceNames.getLength();
    /* Search for right name in list. */
    while   (
              ( nCounter      <       nLength         )       &&
              ( bReturn       ==      sal_False       )
            )
    {
        /* Is name was found, say "YES, SERVICE IS SUPPORTED." and break loop. */
        if ( pArray[nCounter] == sServiceName )
        {
            bReturn = sal_True ;
        }
        /* Else step to next element in list. */
        ++nCounter;
    }
    /* Return state of search. */
    return bReturn;
}

/*===========================================================================================================*/
/* XServiceInfo */
/*===========================================================================================================*/
css::uno::Sequence< ::rtl::OUString > SAL_CALL SoundHandler::getSupportedServiceNames() throw( css::uno::RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

/*===========================================================================================================*/
/* Helper for XServiceInfo                                                                                   */
/*===========================================================================================================*/
css::uno::Sequence< ::rtl::OUString > SoundHandler::impl_getStaticSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = SERVICENAME_CONTENTHANDLER;
    return seqServiceNames;
}

/*===========================================================================================================*/
/* Helper for XServiceInfo */
/*===========================================================================================================*/
::rtl::OUString SoundHandler::impl_getStaticImplementationName()
{
    return IMPLEMENTATIONNAME_SOUNDHANDLER;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL SoundHandler::impl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager ) throw( css::uno::Exception )
{
    /* create new instance of service */
    SoundHandler* pClass = new SoundHandler( xServiceManager );
    /* hold it alive by increasing his ref count!!! */
    css::uno::Reference< css::uno::XInterface > xService( static_cast< ::cppu::OWeakObject* >(pClass), css::uno::UNO_QUERY );
    /* initialize new service instance ... he can use his own refcount ... we hold it! */
    pClass->impl_initService();
    /* return new created service as reference */
    return xService;
}

css::uno::Reference< css::lang::XSingleServiceFactory > SoundHandler::impl_createFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager )
{
    css::uno::Reference< css::lang::XSingleServiceFactory > xReturn ( cppu::createSingleFactory (
       xServiceManager,
        SoundHandler::impl_getStaticImplementationName(),
        SoundHandler::impl_createInstance,
        SoundHandler::impl_getStaticSupportedServiceNames()
        )
    );
    return xReturn;
}

void SAL_CALL SoundHandler::impl_initService()
{
}

/*-************************************************************************************************************//**
    @short      standard ctor
    @descr      These initialize a new instance of this class with needed informations for work.

    @seealso    using at owner

    @param      "xFactory", reference to service manager for creation of new services
    @return     -

    @onerror    Show an assertion and do nothing else.
    @threadsafe yes
*//*-*************************************************************************************************************/
SoundHandler::SoundHandler( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        :   ThreadHelpBase      (          )
        ,   ::cppu::OWeakObject (          )
        // Init member
    ,   m_bError        ( false    )
        ,   m_xFactory          ( xFactory )
{
    m_aUpdateTimer.SetTimeoutHdl(LINK(this, SoundHandler, implts_PlayerNotify));
}

/*-************************************************************************************************************//**
    @short      standard dtor
    @descr      -

    @seealso    -

    @param      -
    @return     -

    @onerror    -
    @threadsafe -
*//*-*************************************************************************************************************/
SoundHandler::~SoundHandler()
{
    if (m_xListener.is())
    {
        css::frame::DispatchResultEvent aEvent;
        aEvent.State = css::frame::DispatchResultState::FAILURE;
        m_xListener->dispatchFinished(aEvent);
        m_xListener = css::uno::Reference< css::frame::XDispatchResultListener >();
    }
}

/*-************************************************************************************************************//**
    @interface  ::com::sun::star::frame::XDispatch

    @short      try to load audio file
    @descr      This method try to load given audio file by URL and play it. We use vcl/Sound class to do that.
                Playing of sound is asynchron everytime.

    @attention  We must hold us alive by ourself ... because we use async. vcl sound player ... but playing is started
                in async interface call "dispatch()" too. And caller forget us imediatly. But then our uno ref count
                will decreased to 0 and will die. The only solution is to use own reference to our implementation.
                But we do it for realy started jobs only and release it during call back of vcl.

    @seealso    class vcl/Sound
    @seealso    method implts_PlayerNotify()

    @param      "aURL"      , URL to dispatch.
    @param      "lArguments", list of optional arguments.
    @return     -

    @onerror    We do nothing.
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL SoundHandler::dispatchWithNotification(const css::util::URL&                                             aURL      ,
                                                     const css::uno::Sequence< css::beans::PropertyValue >&            lDescriptor,
                                                     const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw(css::uno::RuntimeException)
{
    // SAFE {
    const ::osl::MutexGuard aLock( m_aLock );

    {
    //close streams otherwise on windows we can't reopen the file in the
    //media player when we pass the url to directx as it'll already be open
        ::comphelper::MediaDescriptor aDescriptor(lDescriptor);

    css::uno::Reference< css::io::XInputStream > xInputStream =
        aDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_INPUTSTREAM(),
        css::uno::Reference< css::io::XInputStream >());
    if (xInputStream.is()) xInputStream->closeInput();
    }

    // If player currently used for other dispatch() requests ...
    // cancel it by calling stop()!
    m_aUpdateTimer.Stop();
    if (m_xPlayer.is())
    {
        if (m_xPlayer->isPlaying())
            m_xPlayer->stop();
        m_xPlayer.clear();
    }

    // Try to initialize player.
    m_xListener = xListener;
    try
    {
        m_bError = false;
        m_xPlayer.set( avmedia::MediaWindow::createPlayer( aURL.Complete ), css::uno::UNO_QUERY_THROW );
        // OK- we can start async playing ...
        // Count this request and initialize self-holder against dieing by uno ref count ...
        m_xSelfHold = css::uno::Reference< css::uno::XInterface >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
        m_xPlayer->start();
        m_aUpdateTimer.SetTimeout( 200 );
        m_aUpdateTimer.Start();
    }
    catch( css::uno::Exception& e )
    {
        m_bError = true;
        (void)e;
        m_xPlayer.clear();
    }

    // } SAFE
}

void SAL_CALL SoundHandler::dispatch( const css::util::URL&                                  aURL       ,
                                      const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    dispatchWithNotification(aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >());
}

/*-************************************************************************************************************//**
    @interface  ::com::sun::star::document::XExtendedFilterDetection

    @short      try to detect file (given as argument included in "lDescriptor")
    @descr      We try to detect, if given file could be handled by this class and is a well known one.
                If it is - we return right internal type name - otherwise we return nothing!
                So call can search for another detect service and ask him too.

    @attention  a) We don't need any mutex here ... because we don't use any member!
                b) Dont' use internal player instance "m_pPlayer" to detect given sound file!
                   It's not neccessary to do that ... and we can use temp. variable to do the same.
                   This way is easy - we don't must synchronize it with currently played sounds!
                   Another reason to do so ... We are a listener on our internal ma_Player object.
                   If you would call "IsSoundFile()" on this instance, he would call us back and
                   we make some uneccssary things ...

    @seealso    -

    @param      "lDescriptor", description of file to detect
    @return     Internal type name which match this file ... or nothing if it is unknown.

    @onerror    We return nothing.
    @threadsafe yes
*//*-*************************************************************************************************************/
::rtl::OUString SAL_CALL SoundHandler::detect( css::uno::Sequence< css::beans::PropertyValue >& lDescriptor ) throw( css::uno::RuntimeException )
{
    // Our default is "nothing". So we can return it, if detection failed or fily type is realy unknown.
    ::rtl::OUString sTypeName;

    // Analyze given descriptor to find filename or input stream or ...
    ::comphelper::MediaDescriptor aDescriptor(lDescriptor);
    ::rtl::OUString               sURL       = aDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_URL(), ::rtl::OUString());

    if (
        (sURL.getLength()           ) &&
        (avmedia::MediaWindow::isMediaURL(sURL))
       )
    {
        // If the file type is supported depends on the OS, so...
        // I think we can the following ones:
        //  a) look for given extension of url to map our type decision HARD CODED!!!
        //  b) return preferred type every time... it's easy :-)
        sTypeName = ::rtl::OUString("wav_Wave_Audio_File");
        aDescriptor[::comphelper::MediaDescriptor::PROP_TYPENAME()] <<= sTypeName;
        aDescriptor >> lDescriptor;
    }

    // Return our decision.
    return sTypeName;
}

/*-************************************************************************************************************//**
    @short      call back of sound player
    @descr      Our player call us back to give us some informations.
                We use this informations to callback our might existing listener.

    @seealso    method dispatchWithNotification()

    @param      -
    @return     0 everytime ... it doesnt matter for us.

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
IMPL_LINK_NOARG(SoundHandler, implts_PlayerNotify)
{
    // SAFE {
    ::osl::ClearableMutexGuard aLock( m_aLock );

    if (m_xPlayer.is() && m_xPlayer->isPlaying() && m_xPlayer->getMediaTime() < m_xPlayer->getDuration())
    {
        m_aUpdateTimer.Start();
        return 0L;
    }
    m_xPlayer.clear();

    // We use m_xSelfHold to let us die ... but we must live till real finishing of this method too!!!
    // So we SHOULD use another "self-holder" temp. to provide that ...
    css::uno::Reference< css::uno::XInterface > xOperationHold = m_xSelfHold;
    m_xSelfHold = css::uno::Reference< css::uno::XInterface >();

    // notify might existing listener
    // And forget this listener!
    // Because the corresponding dispatch was finished.
    if (m_xListener.is())
    {
        css::frame::DispatchResultEvent aEvent;
        if (!m_bError)
            aEvent.State = css::frame::DispatchResultState::SUCCESS;
        else
            aEvent.State = css::frame::DispatchResultState::FAILURE;
        m_xListener->dispatchFinished(aEvent);
        m_xListener = css::uno::Reference< css::frame::XDispatchResultListener >();
    }

    // } SAFE
    //release aLock before end of method at which point xOperationHold goes out of scope and pThis dies
    aLock.clear();
    return 0;
}

} // namespace framework

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory(const sal_Char* pImplementationName, void* pServiceManager, void* /*pRegistryKey*/ )
{
    void* pReturn = NULL;
    if  (pServiceManager !=  NULL )
    {
        /* Define variables which are used in following macros. */
        css::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > xFactory;
        css::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager;
            xServiceManager = reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pServiceManager )  ;

        if ( avmedia::SoundHandler::impl_getStaticImplementationName().equals( ::rtl::OUString::createFromAscii( pImplementationName ) ) )
            xFactory = avmedia::SoundHandler::impl_createFactory( xServiceManager );

        if ( xFactory.is() == sal_True )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }
    /* Return with result of this operation. */
    return pReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
