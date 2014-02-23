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

#include "framecontrol.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>


//  namespaces


using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::cppu                  ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::lang  ;
using namespace ::com::sun::star::beans ;
using namespace ::com::sun::star::awt   ;
using namespace ::com::sun::star::frame ;
using namespace ::com::sun::star::util  ;

namespace unocontrols{


//  construct/destruct


FrameControl::FrameControl( const Reference< XComponentContext >& rxContext)
    : BaseControl                   ( rxContext                                                                             )
    , OBroadcastHelper              ( m_aMutex                                                                              )
    , OPropertySetHelper            ( *(static_cast< OBroadcastHelper * >(this))  )
    , m_aInterfaceContainer         ( m_aMutex                                                                              )
    , m_aConnectionPointContainer   ( m_aMutex                                                                              )
{
}

FrameControl::~FrameControl()
{
}


//  XInterface


Any SAL_CALL FrameControl::queryInterface( const Type& rType ) throw( RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.
    Any aReturn ;
    Reference< XInterface > xDel = BaseControl::impl_getDelegator();
    if ( xDel.is() )
    {
        // If an delegator exist, forward question to his queryInterface.
        // Delegator will ask his own queryAggregation!
        aReturn = xDel->queryInterface( rType );
    }
    else
    {
        // If an delegator unknown, forward question to own queryAggregation.
        aReturn = queryAggregation( rType );
    }

    return aReturn ;
}


//  XInterface


void SAL_CALL FrameControl::acquire() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::acquire();
}


//  XInterface


void SAL_CALL FrameControl::release() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::release();
}


//  XTypeProvider


Sequence< Type > SAL_CALL FrameControl::getTypes() throw( RuntimeException )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
    static OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MutexGuard aGuard( Mutex::getGlobalMutex() );

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == NULL )
        {
            // Create a static typecollection ...
            static OTypeCollection aTypeCollection  (   ::getCppuType(( const Reference< XControlModel              >*)NULL )   ,
                                                          ::getCppuType(( const Reference< XControlContainer            >*)NULL )   ,
                                                          ::getCppuType(( const Reference< XConnectionPointContainer    >*)NULL )   ,
                                                        BaseControl::getTypes()
                                                    );
            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes();
}


//  XAggregation


Any SAL_CALL FrameControl::queryAggregation( const Type& aType ) throw( RuntimeException )
{
    // Ask for my own supported interfaces ...
    // Attention: XTypeProvider and XInterface are supported by OComponentHelper!
    Any aReturn ( ::cppu::queryInterface(   aType                                               ,
                                               static_cast< XControlModel*              > ( this )  ,
                                               static_cast< XConnectionPointContainer*  > ( this )
                                        )
                );

    // If searched interface not supported by this class ...
    if ( !aReturn.hasValue() )
    {
        // ... ask baseclasses.
        aReturn = OPropertySetHelper::queryInterface( aType );
        if ( !aReturn.hasValue() )
        {
            aReturn = BaseControl::queryAggregation( aType );
        }
    }

    return aReturn ;
}


//  XControl


void SAL_CALL FrameControl::createPeer( const   Reference< XToolkit >&      xToolkit    ,
                                        const   Reference< XWindowPeer >&   xParentPeer ) throw( RuntimeException )
{
    BaseControl::createPeer( xToolkit, xParentPeer );
    if ( impl_getPeerWindow().is() )
    {
        if( !m_sComponentURL.isEmpty() )
        {
            impl_createFrame( getPeer(), m_sComponentURL, m_seqLoaderArguments );
        }
    }
}


//  XControl


sal_Bool SAL_CALL FrameControl::setModel( const Reference< XControlModel >& /*xModel*/ ) throw( RuntimeException )
{
    // We have no model.
    return sal_False ;
}


//  XControl


Reference< XControlModel > SAL_CALL FrameControl::getModel() throw( RuntimeException )
{
    // We have no model.
    return Reference< XControlModel >();
}


//  XControl


void SAL_CALL FrameControl::dispose() throw( RuntimeException )
{
    impl_deleteFrame();
    BaseControl::dispose();
}


//  XView


sal_Bool SAL_CALL FrameControl::setGraphics( const Reference< XGraphics >& /*xDevice*/ ) throw( RuntimeException )
{
    // it is not possible to print this control
    return sal_False ;
}


//  XView


Reference< XGraphics > SAL_CALL FrameControl::getGraphics() throw( RuntimeException )
{
    // when its not posible to set graphics ! then its possible to return null
    return Reference< XGraphics >();
}


//  XConnectionPointContainer


Sequence< Type > SAL_CALL FrameControl::getConnectionPointTypes() throw( RuntimeException )
{
    // Forwarded to helper class
    return m_aConnectionPointContainer.getConnectionPointTypes();
}


//  XConnectionPointContainer


Reference< XConnectionPoint > SAL_CALL FrameControl::queryConnectionPoint( const Type& aType ) throw( RuntimeException )
{
    // Forwarded to helper class
    return m_aConnectionPointContainer.queryConnectionPoint( aType );
}


//  XConnectionPointContainer


void SAL_CALL FrameControl::advise( const   Type&                       aType       ,
                                    const   Reference< XInterface >&    xListener   ) throw( RuntimeException )
{
    // Forwarded to helper class
    m_aConnectionPointContainer.advise( aType, xListener );
}


//  XConnectionPointContainer


void SAL_CALL FrameControl::unadvise(   const   Type&                       aType       ,
                                        const   Reference< XInterface >&    xListener   ) throw( RuntimeException )
{
    // Forwarded to helper class
    m_aConnectionPointContainer.unadvise( aType, xListener );
}


//  impl but public method to register service


const Sequence< OUString > FrameControl::impl_getStaticSupportedServiceNames()
{
    MutexGuard aGuard( Mutex::getGlobalMutex() );
    Sequence< OUString > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = SERVICENAME_FRAMECONTROL;
    return seqServiceNames ;
}


//  impl but public method to register service


const OUString FrameControl::impl_getStaticImplementationName()
{
    return OUString(IMPLEMENTATIONNAME_FRAMECONTROL);
}


//  OPropertySetHelper


sal_Bool FrameControl::convertFastPropertyValue(        Any&        rConvertedValue ,
                                                        Any&        rOldValue       ,
                                                        sal_Int32   nHandle         ,
                                                const   Any&        rValue          ) throw( IllegalArgumentException )
{
    bool bReturn = false ;
    switch (nHandle)
    {
        case PROPERTYHANDLE_COMPONENTURL        :       rConvertedValue   = rValue                  ;
                                                        rOldValue       <<= m_sComponentURL         ;
                                                        bReturn           = true                ;
                                                        break ;

        case PROPERTYHANDLE_LOADERARGUMENTS     :       rConvertedValue   = rValue                  ;
                                                        rOldValue       <<= m_seqLoaderArguments    ;
                                                        bReturn           = true                ;
                                                        break ;
    }

    if ( !bReturn )
    {
        throw IllegalArgumentException();
    }

    return bReturn ;
}


//  OPropertySetHelper


void FrameControl::setFastPropertyValue_NoBroadcast(            sal_Int32   nHandle ,
                                                        const   Any&        rValue  )
                                                        throw ( ::com::sun::star::uno::Exception )
{
    // this method only set the value
    MutexGuard  aGuard (m_aMutex) ;
    switch (nHandle)
    {
        case PROPERTYHANDLE_COMPONENTURL        :       rValue >>= m_sComponentURL ;
                                                        if (getPeer().is())
                                                        {
                                                            impl_createFrame ( getPeer(), m_sComponentURL, m_seqLoaderArguments ) ;
                                                        }
                                                        break ;

        case PROPERTYHANDLE_LOADERARGUMENTS     :       rValue >>= m_seqLoaderArguments ;
                                                        break ;

        default :                                       OSL_ENSURE ( nHandle == -1, "This is an invalid property handle." ) ;
    }
}


//  OPropertySetHelper


void FrameControl::getFastPropertyValue(    Any&        rRet    ,
                                            sal_Int32   nHandle ) const
{
    MutexGuard  aGuard ( Mutex::getGlobalMutex() ) ;

    switch (nHandle)
    {
        case PROPERTYHANDLE_COMPONENTURL    :       rRet <<= m_sComponentURL ;
                                                    break ;

        case PROPERTYHANDLE_LOADERARGUMENTS :       rRet <<= m_seqLoaderArguments ;
                                                    break ;

        case PROPERTYHANDLE_FRAME           :       rRet <<= m_xFrame ;
                                                       break ;

        default :                                   OSL_ENSURE ( nHandle == -1, "This is an invalid property handle." ) ;
    }
}


//  OPropertySetHelper


IPropertyArrayHelper& FrameControl::getInfoHelper()
{
    // Create a table that map names to index values.
    static OPropertyArrayHelper* pInfo ;

    if (!pInfo)
    {
        // global method must be guarded
        MutexGuard  aGuard ( Mutex::getGlobalMutex() ) ;

        if (!pInfo)
        {
            pInfo = new OPropertyArrayHelper( impl_getStaticPropertyDescriptor(), sal_True );
        }
    }

    return *pInfo ;
}


//  OPropertySetHelper


Reference< XPropertySetInfo > SAL_CALL FrameControl::getPropertySetInfo() throw( RuntimeException )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static Reference< XPropertySetInfo >* pInfo = (Reference< XPropertySetInfo >*)0 ;
    if ( pInfo == (Reference< XPropertySetInfo >*)0 )
    {
        // Ready for multithreading
        MutexGuard aGuard ( Mutex::getGlobalMutex () ) ;
        // Control this pointer again, another instance can be faster then these!
        if ( pInfo == (Reference< XPropertySetInfo >*)0 )
        {
            // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
            // (Use method "getInfoHelper()".)
            static Reference< XPropertySetInfo > xInfo ( createPropertySetInfo ( getInfoHelper () ) ) ;
            pInfo = &xInfo ;
        }
    }
    return ( *pInfo ) ;
}


//  BaseControl


WindowDescriptor* FrameControl::impl_getWindowDescriptor( const Reference< XWindowPeer >& xParentPeer )
{
    WindowDescriptor* pDescriptor   = new WindowDescriptor  ;

    pDescriptor->Type               = WindowClass_CONTAINER ;
    pDescriptor->ParentIndex        = -1                    ;
    pDescriptor->Parent             = xParentPeer           ;
    pDescriptor->Bounds             = getPosSize ()         ;
    pDescriptor->WindowAttributes   = 0                     ;

    return pDescriptor ;
}


//  private method


void FrameControl::impl_createFrame(    const   Reference< XWindowPeer >&   xPeer       ,
                                        const   OUString&                   rURL        ,
                                        const   Sequence< PropertyValue >&  rArguments  )
{
    Reference< XFrame2 >     xOldFrame   ;
    Reference< XFrame2 >     xNewFrame   ;

    {
        MutexGuard  aGuard ( m_aMutex ) ;
        xOldFrame = m_xFrame ;
    }



    xNewFrame = Frame::create( impl_getComponentContext() );

    Reference< XWindow >  xWP ( xPeer, UNO_QUERY ) ;
    xNewFrame->initialize ( xWP ) ;

    //  option
    //xFrame->setName( "WhatYouWant" );

    Reference< XURLTransformer > xTrans = URLTransformer::create( impl_getComponentContext() );
    // load file
    URL aURL ;
    aURL.Complete = rURL ;
    xTrans->parseStrict( aURL ) ;

    Reference< XDispatch >  xDisp = xNewFrame->queryDispatch ( aURL, OUString (), FrameSearchFlag::SELF ) ;
    if (xDisp.is())
    {
        xDisp->dispatch ( aURL, rArguments ) ;
    }

    // set the frame
    {
        MutexGuard aGuard ( m_aMutex ) ;
        m_xFrame = xNewFrame ;
    }

    // notify the listeners
    sal_Int32   nFrameId = PROPERTYHANDLE_FRAME ;
    Any aNewFrame ( &xNewFrame, ::getCppuType((const Reference< XFrame >*)0) ) ;
    Any aOldFrame ( &xOldFrame, ::getCppuType((const Reference< XFrame >*)0) ) ;

    fire ( &nFrameId, &aNewFrame, &aOldFrame, 1, sal_False ) ;

    if (xOldFrame.is())
    {
        xOldFrame->dispose () ;
    }
}


//  private method


void FrameControl::impl_deleteFrame()
{
    Reference< XFrame2 >  xOldFrame;
    Reference< XFrame2 >  xNullFrame;

    {
        // do not dispose the frame in this guarded section (deadlock?)
        MutexGuard aGuard( m_aMutex );
        xOldFrame = m_xFrame;
        m_xFrame = Reference< XFrame2 > ();
    }

    // notify the listeners
    sal_Int32 nFrameId = PROPERTYHANDLE_FRAME;
    Any aNewFrame( &xNullFrame, ::getCppuType((const Reference< XFrame2 >*)0) );
    Any aOldFrame( &xOldFrame, ::getCppuType((const Reference< XFrame2 >*)0) );
    fire( &nFrameId, &aNewFrame, &aOldFrame, 1, sal_False );

    // dispose the frame
    if( xOldFrame.is() )
        xOldFrame->dispose();
}


//  private method


const Sequence< Property > FrameControl::impl_getStaticPropertyDescriptor()
{
    // All Properties of this implementation. The array must be sorted!
    static const Property pPropertys[PROPERTY_COUNT] =
    {
        Property( PROPERTYNAME_COMPONENTURL, PROPERTYHANDLE_COMPONENTURL, ::getCppuType((const OUString*)0), PropertyAttribute::BOUND | PropertyAttribute::CONSTRAINED ),
        Property( PROPERTYNAME_FRAME, PROPERTYHANDLE_FRAME, ::getCppuType((const Reference< XFrame >*)0), PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT   ),
        Property( PROPERTYNAME_LOADERARGUMENTS, PROPERTYHANDLE_LOADERARGUMENTS, ::getCppuType((const Sequence< PropertyValue >*)0), PropertyAttribute::BOUND | PropertyAttribute::CONSTRAINED )
    };

    static const Sequence< Property > seqPropertys( pPropertys, PROPERTY_COUNT );

    return seqPropertys ;
}

}   // namespace unocontrols

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
