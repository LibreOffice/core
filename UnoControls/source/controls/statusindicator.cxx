/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "statusindicator.hxx"

#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/typeprovider.hxx>

#include "progressbar.hxx"

using namespace ::cppu                  ;
using namespace ::osl                   ;
using namespace ::rtl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::lang  ;
using namespace ::com::sun::star::awt   ;
using namespace ::com::sun::star::task  ;

namespace unocontrols{





StatusIndicator::StatusIndicator( const css::uno::Reference< XComponentContext >& rxContext )
    : BaseContainerControl  ( rxContext  )
{
    
    
    ++m_refCount ;

    
    m_xText         = css::uno::Reference< XFixedText >   ( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_SERVICENAME, rxContext ), UNO_QUERY );
    m_xProgressBar = new ProgressBar(rxContext);
    
    
    css::uno::Reference< XControl > xTextControl      ( m_xText       , UNO_QUERY );
    xTextControl->setModel( css::uno::Reference< XControlModel >( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_MODELNAME, rxContext ), UNO_QUERY ) );
    
    addControl( CONTROLNAME_TEXT, xTextControl    );
    addControl( CONTROLNAME_PROGRESSBAR, m_xProgressBar.get() );
    
    
    m_xProgressBar->setVisible( sal_True );
    
    
    m_xText->setText( STATUSINDICATOR_DEFAULT_TEXT );

    --m_refCount ;
}

StatusIndicator::~StatusIndicator() {}





Any SAL_CALL StatusIndicator::queryInterface( const Type& rType ) throw( RuntimeException )
{
    
    
    Any aReturn ;
    css::uno::Reference< XInterface > xDel = BaseContainerControl::impl_getDelegator();
    if ( xDel.is() )
    {
        
        
        aReturn = xDel->queryInterface( rType );
    }
    else
    {
        
        aReturn = queryAggregation( rType );
    }

    return aReturn ;
}





void SAL_CALL StatusIndicator::acquire() throw()
{
    
    

    
    BaseControl::acquire();
}





void SAL_CALL StatusIndicator::release() throw()
{
    
    

    
    BaseControl::release();
}





Sequence< Type > SAL_CALL StatusIndicator::getTypes() throw( RuntimeException )
{
    
    
    
    static OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        
        MutexGuard aGuard( Mutex::getGlobalMutex() );

        
        if ( pTypeCollection == NULL )
        {
            
            static OTypeCollection aTypeCollection  ( ::getCppuType(( const css::uno::Reference< XLayoutConstrains    >*)NULL )   ,
                                                      ::getCppuType(( const css::uno::Reference< XStatusIndicator >*)NULL )   ,
                                                      BaseContainerControl::getTypes()
                                                    );
            
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes();
}





Any SAL_CALL StatusIndicator::queryAggregation( const Type& aType ) throw( RuntimeException )
{
    
    
    Any aReturn ( ::cppu::queryInterface( aType                                     ,
                                          static_cast< XLayoutConstrains*   > ( this )  ,
                                          static_cast< XStatusIndicator*    > ( this )
                                        )
                );

    
    if ( !aReturn.hasValue() )
    {
        
        aReturn = BaseControl::queryAggregation( aType );
    }

    return aReturn ;
}





void SAL_CALL StatusIndicator::start( const OUString& sText, sal_Int32 nRange ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    
    m_xText->setText( sText );
    m_xProgressBar->setRange( 0, nRange );
    
    impl_recalcLayout ( WindowEvent(static_cast< OWeakObject* >(this),0,0,impl_getWidth(),impl_getHeight(),0,0,0,0) ) ;
}





void SAL_CALL StatusIndicator::end() throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    
    m_xText->setText( OUString() );
    m_xProgressBar->setValue( 0 );
    setVisible( sal_False );
}





void SAL_CALL StatusIndicator::setText( const OUString& sText ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    
    m_xText->setText( sText );
}





void SAL_CALL StatusIndicator::setValue( sal_Int32 nValue ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    
    m_xProgressBar->setValue( nValue );
}





void SAL_CALL StatusIndicator::reset() throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    
    
    m_xText->setText( OUString() );
    m_xProgressBar->setValue( 0 );
}





Size SAL_CALL StatusIndicator::getMinimumSize () throw( RuntimeException )
{
    return Size (STATUSINDICATOR_DEFAULT_WIDTH, STATUSINDICATOR_DEFAULT_HEIGHT) ;
}





Size SAL_CALL StatusIndicator::getPreferredSize () throw( RuntimeException )
{
    
    ClearableMutexGuard aGuard ( m_aMutex ) ;

    
    css::uno::Reference< XLayoutConstrains >  xTextLayout ( m_xText, UNO_QUERY );
    Size                            aTextSize   = xTextLayout->getPreferredSize();

    aGuard.clear () ;

    
    sal_Int32 nWidth  = impl_getWidth()                 ;
    sal_Int32 nHeight = (2*STATUSINDICATOR_FREEBORDER)+aTextSize.Height ;

    
    if ( nWidth<STATUSINDICATOR_DEFAULT_WIDTH )
    {
        nWidth = STATUSINDICATOR_DEFAULT_WIDTH ;
    }
    if ( nHeight<STATUSINDICATOR_DEFAULT_HEIGHT )
    {
        nHeight = STATUSINDICATOR_DEFAULT_HEIGHT ;
    }

    
    return Size ( nWidth, nHeight ) ;
}





Size SAL_CALL StatusIndicator::calcAdjustedSize ( const Size& /*rNewSize*/ ) throw( RuntimeException )
{
    return getPreferredSize () ;
}





void SAL_CALL StatusIndicator::createPeer (
    const css::uno::Reference< XToolkit > & rToolkit,
    const css::uno::Reference< XWindowPeer > & rParent
) throw( RuntimeException )
{
    if( !getPeer().is() )
    {
        BaseContainerControl::createPeer( rToolkit, rParent );

        
        
        
        Size aDefaultSize = getMinimumSize () ;
        setPosSize ( 0, 0, aDefaultSize.Width, aDefaultSize.Height, PosSize::SIZE ) ;
    }
}





sal_Bool SAL_CALL StatusIndicator::setModel ( const css::uno::Reference< XControlModel > & /*rModel*/ ) throw( RuntimeException )
{
    
    return sal_False ;
}





css::uno::Reference< XControlModel > SAL_CALL StatusIndicator::getModel () throw( RuntimeException )
{
    
    
    return css::uno::Reference< XControlModel >  () ;
}





void SAL_CALL StatusIndicator::dispose () throw( RuntimeException )
{
    
    MutexGuard aGuard ( m_aMutex ) ;

    
    css::uno::Reference< XControl >  xTextControl     ( m_xText       , UNO_QUERY );

    removeControl( xTextControl     );
    removeControl( m_xProgressBar.get() );

    
    
    xTextControl->dispose();
    m_xProgressBar->dispose();
    BaseContainerControl::dispose();
}





void SAL_CALL StatusIndicator::setPosSize (
    sal_Int32 nX,
    sal_Int32 nY,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Int16 nFlags
) throw( RuntimeException )
{
    Rectangle   aBasePosSize = getPosSize () ;
    BaseContainerControl::setPosSize (nX, nY, nWidth, nHeight, nFlags) ;

    
    if (
        ( nWidth  != aBasePosSize.Width ) ||
        ( nHeight != aBasePosSize.Height)
       )
    {
        
        impl_recalcLayout ( WindowEvent(static_cast< OWeakObject* >(this),0,0,nWidth,nHeight,0,0,0,0) ) ;
        
        
        getPeer()->invalidate(2);
        
        impl_paint ( 0, 0, impl_getGraphicsPeer() ) ;
    }
}





const Sequence< OUString > StatusIndicator::impl_getStaticSupportedServiceNames()
{
    return css::uno::Sequence<OUString>();
}





const OUString StatusIndicator::impl_getStaticImplementationName()
{
    return OUString("stardiv.UnoControls.StatusIndicator");
}





WindowDescriptor* StatusIndicator::impl_getWindowDescriptor( const css::uno::Reference< XWindowPeer >& xParentPeer )
{
    
    
    

    WindowDescriptor* pDescriptor = new WindowDescriptor ;

    pDescriptor->Type               =   WindowClass_SIMPLE                              ;
    pDescriptor->WindowServiceName  =   "floatingwindow"                                ;
    pDescriptor->ParentIndex        =   -1                                              ;
    pDescriptor->Parent             =   xParentPeer                                     ;
    pDescriptor->Bounds             =   getPosSize ()                                   ;

    return pDescriptor ;
}





void StatusIndicator::impl_paint ( sal_Int32 nX, sal_Int32 nY, const css::uno::Reference< XGraphics > & rGraphics )
{
    
    
     if ( rGraphics.is () )
    {
        MutexGuard  aGuard (m_aMutex) ;

        
        css::uno::Reference< XWindowPeer > xPeer( impl_getPeerWindow(), UNO_QUERY );
        if( xPeer.is() )
            xPeer->setBackground( STATUSINDICATOR_BACKGROUNDCOLOR );

        
        css::uno::Reference< XControl > xTextControl( m_xText, UNO_QUERY );
        xPeer = xTextControl->getPeer();
        if( xPeer.is() )
            xPeer->setBackground( STATUSINDICATOR_BACKGROUNDCOLOR );

        
        xPeer = m_xProgressBar->getPeer();
        if( xPeer.is() )
            xPeer->setBackground( STATUSINDICATOR_BACKGROUNDCOLOR );

        
        rGraphics->setLineColor ( STATUSINDICATOR_LINECOLOR_BRIGHT                          );
        rGraphics->drawLine     ( nX, nY, impl_getWidth(), nY               );
        rGraphics->drawLine     ( nX, nY, nX             , impl_getHeight() );

        rGraphics->setLineColor ( STATUSINDICATOR_LINECOLOR_SHADOW                                                              );
        rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, impl_getWidth()-1, nY                  );
        rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, nX               , impl_getHeight()-1  );
    }
}





void StatusIndicator::impl_recalcLayout ( const WindowEvent& aEvent )
{
    sal_Int32   nX_ProgressBar          ;
    sal_Int32   nY_ProgressBar          ;
    sal_Int32   nWidth_ProgressBar      ;
    sal_Int32   nHeight_ProgressBar     ;
    sal_Int32   nX_Text                 ;
    sal_Int32   nY_Text                 ;
    sal_Int32   nWidth_Text             ;
    sal_Int32   nHeight_Text            ;

    
    MutexGuard aGuard ( m_aMutex ) ;

    
    Size                            aWindowSize     ( aEvent.Width, aEvent.Height );
    css::uno::Reference< XLayoutConstrains >  xTextLayout     ( m_xText, UNO_QUERY );
    Size                            aTextSize       = xTextLayout->getPreferredSize();

    if( aWindowSize.Width < STATUSINDICATOR_DEFAULT_WIDTH )
    {
        aWindowSize.Width = STATUSINDICATOR_DEFAULT_WIDTH;
    }
    if( aWindowSize.Height < STATUSINDICATOR_DEFAULT_HEIGHT )
    {
        aWindowSize.Height = STATUSINDICATOR_DEFAULT_HEIGHT;
    }

    
    nX_Text             = STATUSINDICATOR_FREEBORDER                                    ;
    nY_Text             = STATUSINDICATOR_FREEBORDER                                    ;
    nWidth_Text         = aTextSize.Width                               ;
    nHeight_Text        = aTextSize.Height                              ;

    nX_ProgressBar      = nX_Text+nWidth_Text+STATUSINDICATOR_FREEBORDER                ;
    nY_ProgressBar      = nY_Text                                       ;
    nWidth_ProgressBar  = aWindowSize.Width-nWidth_Text-(3*STATUSINDICATOR_FREEBORDER)  ;
    nHeight_ProgressBar = nHeight_Text                                  ;

    
    css::uno::Reference< XWindow >  xTextWindow       ( m_xText       , UNO_QUERY );

    xTextWindow->setPosSize     ( nX_Text       , nY_Text       , nWidth_Text       , nHeight_Text          , 15 ) ;
    m_xProgressBar->setPosSize( nX_ProgressBar, nY_ProgressBar, nWidth_ProgressBar, nHeight_ProgressBar, 15 );
}

}   

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
