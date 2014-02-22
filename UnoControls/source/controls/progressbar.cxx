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

#include "progressbar.hxx"

#include <com/sun/star/awt/GradientStyle.hpp>
#include <com/sun/star/awt/RasterOperation.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <tools/debug.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <math.h>
#include <limits.h>

using namespace ::cppu                  ;
using namespace ::osl                   ;
using namespace ::rtl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::lang  ;
using namespace ::com::sun::star::awt   ;

namespace unocontrols{





ProgressBar::ProgressBar( const Reference< XComponentContext >& rxContext )
    : BaseControl           (    rxContext                   )
    , m_bHorizontal         (    PROGRESSBAR_DEFAULT_HORIZONTAL         )
    , m_aBlockSize          (    PROGRESSBAR_DEFAULT_BLOCKDIMENSION     )
    , m_nForegroundColor    (    PROGRESSBAR_DEFAULT_FOREGROUNDCOLOR    )
    , m_nBackgroundColor    (    PROGRESSBAR_DEFAULT_BACKGROUNDCOLOR    )
    , m_nMinRange           (    PROGRESSBAR_DEFAULT_MINRANGE           )
    , m_nMaxRange           (    PROGRESSBAR_DEFAULT_MAXRANGE           )
    , m_nBlockValue         (    PROGRESSBAR_DEFAULT_BLOCKVALUE         )
    , m_nValue              (    PROGRESSBAR_DEFAULT_VALUE              )
{
}

ProgressBar::~ProgressBar()
{
}





Any SAL_CALL ProgressBar::queryInterface( const Type& rType ) throw( RuntimeException )
{
    
    
    Any aReturn ;
    Reference< XInterface > xDel = BaseControl::impl_getDelegator();
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





void SAL_CALL ProgressBar::acquire() throw()
{
    
    

    
    BaseControl::acquire();
}





void SAL_CALL ProgressBar::release() throw()
{
    
    

    
    BaseControl::release();
}





Sequence< Type > SAL_CALL ProgressBar::getTypes() throw( RuntimeException )
{
    
    
    
    static OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        
        MutexGuard aGuard( Mutex::getGlobalMutex() );

        
        if ( pTypeCollection == NULL )
        {
            
            static OTypeCollection aTypeCollection  ( ::getCppuType(( const Reference< XControlModel >*) NULL ) ,
                                                      ::getCppuType(( const Reference< XProgressBar  >*) NULL ) ,
                                                      BaseControl::getTypes()
                                                    );
            
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes();
}





Any SAL_CALL ProgressBar::queryAggregation( const Type& aType ) throw( RuntimeException )
{
    
    
    Any aReturn ( ::cppu::queryInterface(   aType                                   ,
                                            static_cast< XControlModel* > ( this )  ,
                                            static_cast< XProgressBar*  > ( this )
                                        )
                );

    
    if ( !aReturn.hasValue() )
    {
        
        aReturn = BaseControl::queryAggregation( aType );
    }

    return aReturn ;
}





void SAL_CALL ProgressBar::setForegroundColor( sal_Int32 nColor ) throw( RuntimeException )
{
    
    MutexGuard  aGuard (m_aMutex) ;

    
    m_nForegroundColor = nColor ;

    
    impl_paint ( 0, 0, impl_getGraphicsPeer() ) ;
}





void SAL_CALL ProgressBar::setBackgroundColor ( sal_Int32 nColor ) throw( RuntimeException )
{
    
    MutexGuard  aGuard (m_aMutex) ;

    
    m_nBackgroundColor = nColor ;

    
    impl_paint ( 0, 0, impl_getGraphicsPeer() ) ;
}





void SAL_CALL ProgressBar::setValue ( sal_Int32 nValue ) throw( RuntimeException )
{
    
    
    

    
    MutexGuard aGuard (m_aMutex) ;

    
    
    DBG_ASSERT ( (( nValue >= m_nMinRange ) && ( nValue <= m_nMaxRange )), "ProgressBar::setValue()\nNot valid value.\n" ) ;

    
    if (
        ( nValue >= m_nMinRange ) &&
        ( nValue <= m_nMaxRange )
       )
    {
        
        m_nValue = nValue ;

        
        impl_paint ( 0, 0, impl_getGraphicsPeer() ) ;
    }
}





void SAL_CALL ProgressBar::setRange ( sal_Int32 nMin, sal_Int32 nMax ) throw( RuntimeException )
{
    
    
    
    

    
    
    
    DBG_ASSERT ( ( nMin != nMax ) , "ProgressBar::setRange()\nValues for MIN and MAX are the same. This is not allowed!\n" ) ;

    
    MutexGuard  aGuard (m_aMutex) ;

    
    if ( nMin < nMax )
    {
        
        m_nMinRange = nMin  ;
        m_nMaxRange = nMax  ;
    }
    else
    {
        
        m_nMinRange = nMax  ;
        m_nMaxRange = nMin  ;
    }

    
    if (!(m_nMinRange < m_nValue  &&  m_nValue < m_nMaxRange))
        m_nValue = m_nMinRange;

    impl_recalcRange () ;

    
    
    
}





sal_Int32 SAL_CALL ProgressBar::getValue () throw( RuntimeException )
{
    
    MutexGuard aGuard (m_aMutex) ;

    return ( m_nValue ) ;
}





void SAL_CALL ProgressBar::setPosSize (
    sal_Int32 nX,
    sal_Int32 nY,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Int16 nFlags
) throw( RuntimeException )
{
    
    
    Rectangle aBasePosSize = getPosSize () ;
    BaseControl::setPosSize (nX, nY, nWidth, nHeight, nFlags) ;

    
    if (
        ( nWidth  != aBasePosSize.Width     ) ||
        ( nHeight != aBasePosSize.Height    )
       )
    {
        impl_recalcRange    (                           ) ;
        impl_paint          ( 0, 0, impl_getGraphicsPeer () ) ;
    }
}





sal_Bool SAL_CALL ProgressBar::setModel( const Reference< XControlModel >& /*xModel*/ ) throw( RuntimeException )
{
    
    return sal_False ;
}





Reference< XControlModel > SAL_CALL ProgressBar::getModel() throw( RuntimeException )
{
    
    return Reference< XControlModel >();
}





const Sequence< OUString > ProgressBar::impl_getStaticSupportedServiceNames()
{
    return css::uno::Sequence<OUString>();
}





const OUString ProgressBar::impl_getStaticImplementationName()
{
    return OUString("stardiv.UnoControls.ProgressBar");
}





void ProgressBar::impl_paint ( sal_Int32 nX, sal_Int32 nY, const Reference< XGraphics > & rGraphics )
{
    
    DBG_ASSERT ( rGraphics.is(), "ProgressBar::paint()\nCalled with invalid Reference< XGraphics > ." ) ;

    
    
     if ( rGraphics.is () )
    {
        MutexGuard  aGuard (m_aMutex) ;

        
        
        rGraphics->setFillColor ( m_nBackgroundColor                        ) ;
        rGraphics->setLineColor ( m_nBackgroundColor                        ) ;
        rGraphics->drawRect     ( nX, nY, impl_getWidth(), impl_getHeight() ) ;

        
        rGraphics->setFillColor ( m_nForegroundColor ) ;
        rGraphics->setLineColor ( m_nForegroundColor ) ;

        sal_Int32   nBlockStart     =   0                                                                           ;   
        sal_Int32   nBlockCount     =   m_nBlockValue!=0.00 ? (sal_Int32)((m_nValue-m_nMinRange)/m_nBlockValue) : 0 ;   

        
        
        if (m_bHorizontal)
        {
            
            nBlockStart = nX ;

            for ( sal_Int16 i=1; i<=nBlockCount; ++i )
            {
                
                nBlockStart +=  PROGRESSBAR_FREESPACE   ;
                
                rGraphics->drawRect (nBlockStart, nY+PROGRESSBAR_FREESPACE, m_aBlockSize.Width, m_aBlockSize.Height) ;
                
                nBlockStart +=  m_aBlockSize.Width ;
            }
        }
        
        
        else
        {
            
            nBlockStart  =  nY+impl_getHeight() ;
            nBlockStart -=  m_aBlockSize.Height ;

            for ( sal_Int16 i=1; i<=nBlockCount; ++i )
            {
                
                nBlockStart -=  PROGRESSBAR_FREESPACE   ;
                
                rGraphics->drawRect (nX+PROGRESSBAR_FREESPACE, nBlockStart, m_aBlockSize.Width, m_aBlockSize.Height) ;
                
                nBlockStart -=  m_aBlockSize.Height;
            }
        }

        
        rGraphics->setLineColor ( PROGRESSBAR_LINECOLOR_SHADOW                          ) ;
        rGraphics->drawLine     ( nX, nY, impl_getWidth(), nY               ) ;
        rGraphics->drawLine     ( nX, nY, nX             , impl_getHeight() ) ;

        rGraphics->setLineColor ( PROGRESSBAR_LINECOLOR_BRIGHT                                                              ) ;
        rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, impl_getWidth()-1, nY                  ) ;
        rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, nX               , impl_getHeight()-1  ) ;
    }
}





void ProgressBar::impl_recalcRange ()
{
    MutexGuard  aGuard (m_aMutex) ;

    sal_Int32 nWindowWidth  = impl_getWidth()  ;
    sal_Int32 nWindowHeight = impl_getHeight() ;
    double    fBlockHeight                     ;
    double    fBlockWidth                      ;
    double    fMaxBlocks                       ;

    if( nWindowWidth > nWindowHeight )
    {
        m_bHorizontal = true                            ;
        fBlockHeight  = (nWindowHeight-(2*PROGRESSBAR_FREESPACE))       ;
        fBlockWidth   = fBlockHeight                        ;
        fMaxBlocks    = nWindowWidth/(fBlockWidth+PROGRESSBAR_FREESPACE);
    }
    else
    {
        m_bHorizontal = false                             ;
        fBlockWidth   = (nWindowWidth-(2*PROGRESSBAR_FREESPACE))          ;
        fBlockHeight  = fBlockWidth                           ;
        fMaxBlocks    = nWindowHeight/(fBlockHeight+PROGRESSBAR_FREESPACE);
    }

    double fRange       = m_nMaxRange-m_nMinRange    ;
    double fBlockValue  = fRange/fMaxBlocks          ;

    m_nBlockValue       = fBlockValue            ;
    m_aBlockSize.Height = (sal_Int32)fBlockHeight;
    m_aBlockSize.Width  = (sal_Int32)fBlockWidth ;
}

}   

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
