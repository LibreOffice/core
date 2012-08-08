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

//____________________________________________________________________________________________________________
//  construct/destruct
//____________________________________________________________________________________________________________

ProgressBar::ProgressBar( const Reference< XMultiServiceFactory >& xFactory )
    : BaseControl           (    xFactory                   )
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

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

Any SAL_CALL ProgressBar::queryInterface( const Type& rType ) throw( RuntimeException )
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

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL ProgressBar::acquire() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::acquire();
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL ProgressBar::release() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::release();
}

//____________________________________________________________________________________________________________
//  XTypeProvider
//____________________________________________________________________________________________________________

Sequence< Type > SAL_CALL ProgressBar::getTypes() throw( RuntimeException )
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
            static OTypeCollection aTypeCollection  ( ::getCppuType(( const Reference< XControlModel >*) NULL ) ,
                                                      ::getCppuType(( const Reference< XProgressBar  >*) NULL ) ,
                                                      BaseControl::getTypes()
                                                    );
            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes();
}

//____________________________________________________________________________________________________________
//  XAggregation
//____________________________________________________________________________________________________________

Any SAL_CALL ProgressBar::queryAggregation( const Type& aType ) throw( RuntimeException )
{
    // Ask for my own supported interfaces ...
    // Attention: XTypeProvider and XInterface are supported by OComponentHelper!
    Any aReturn ( ::cppu::queryInterface(   aType                                   ,
                                            static_cast< XControlModel* > ( this )  ,
                                            static_cast< XProgressBar*  > ( this )
                                        )
                );

    // If searched interface not supported by this class ...
    if ( aReturn.hasValue() == sal_False )
    {
        // ... ask baseclasses.
        aReturn = BaseControl::queryAggregation( aType );
    }

    return aReturn ;
}

//____________________________________________________________________________________________________________
//  XProgressBar
//____________________________________________________________________________________________________________

void SAL_CALL ProgressBar::setForegroundColor( sal_Int32 nColor ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard  aGuard (m_aMutex) ;

    // Safe color for later use.
    m_nForegroundColor = nColor ;

    // Repaint control
    impl_paint ( 0, 0, impl_getGraphicsPeer() ) ;
}

//____________________________________________________________________________________________________________
//  XProgressBar
//____________________________________________________________________________________________________________

void SAL_CALL ProgressBar::setBackgroundColor ( sal_Int32 nColor ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard  aGuard (m_aMutex) ;

    // Safe color for later use.
    m_nBackgroundColor = nColor ;

    // Repaint control
    impl_paint ( 0, 0, impl_getGraphicsPeer() ) ;
}

//____________________________________________________________________________________________________________
//  XProgressBar
//____________________________________________________________________________________________________________

void SAL_CALL ProgressBar::setValue ( sal_Int32 nValue ) throw( RuntimeException )
{
    // This method is defined for follow things:
    //      1) Values >= _nMinRange
    //      2) Values <= _nMaxRange

    // Ready for multithreading
    MutexGuard aGuard (m_aMutex) ;

    // save impossible cases
    // This method is only defined for valid values
    DBG_ASSERT ( (( nValue >= m_nMinRange ) && ( nValue <= m_nMaxRange )), "ProgressBar::setValue()\nNot valid value.\n" ) ;

    // If new value not valid ... do nothing in release version!
    if (
        ( nValue >= m_nMinRange ) &&
        ( nValue <= m_nMaxRange )
       )
    {
        // New value is ok => save this
        m_nValue = nValue ;

        // Repaint to display changes
        impl_paint ( 0, 0, impl_getGraphicsPeer() ) ;
    }
}

//____________________________________________________________________________________________________________
//  XProgressBar
//____________________________________________________________________________________________________________

void SAL_CALL ProgressBar::setRange ( sal_Int32 nMin, sal_Int32 nMax ) throw( RuntimeException )
{
    // This method is defined for follow things:
    //      1) All values of sal_Int32
    //      2) Min < Max
    //      3) Min > Max

    // save impossible cases
    // This method is only defined for valid values
    // If you ignore this, the release version wil produce an error "division by zero" in "ProgressBar::setValue()"!
    DBG_ASSERT ( ( nMin != nMax ) , "ProgressBar::setRange()\nValues for MIN and MAX are the same. This is not allowed!\n" ) ;

    // Ready for multithreading
    MutexGuard  aGuard (m_aMutex) ;

    // control the values for min and max
    if ( nMin < nMax )
    {
        // Take correct Min and Max
        m_nMinRange = nMin  ;
        m_nMaxRange = nMax  ;
    }
    else
    {
        // Change Min and Max automaticly
        m_nMinRange = nMax  ;
        m_nMaxRange = nMin  ;
    }

    // assure that m_nValue is within the range
    if (!(m_nMinRange < m_nValue  &&  m_nValue < m_nMaxRange))
        m_nValue = m_nMinRange;

    impl_recalcRange () ;

    // Do not repaint the control at this place!!!
    // An old "m_nValue" is set and can not be correct for this new range.
    // Next call of "ProgressBar::setValue()" do this.
}

//____________________________________________________________________________________________________________
//  XProgressBar
//____________________________________________________________________________________________________________

sal_Int32 SAL_CALL ProgressBar::getValue () throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard (m_aMutex) ;

    return ( m_nValue ) ;
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL ProgressBar::setPosSize (
    sal_Int32 nX,
    sal_Int32 nY,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Int16 nFlags
) throw( RuntimeException )
{
    // Take old size BEFORE you set the new values at baseclass!
    // You will control changes. At the other way, the values are the same!
    Rectangle aBasePosSize = getPosSize () ;
    BaseControl::setPosSize (nX, nY, nWidth, nHeight, nFlags) ;

    // Do only, if size has changed.
    if (
        ( nWidth  != aBasePosSize.Width     ) ||
        ( nHeight != aBasePosSize.Height    )
       )
    {
        impl_recalcRange    (                           ) ;
        impl_paint          ( 0, 0, impl_getGraphicsPeer () ) ;
    }
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

sal_Bool SAL_CALL ProgressBar::setModel( const Reference< XControlModel >& /*xModel*/ ) throw( RuntimeException )
{
    // A model is not possible for this control.
    return sal_False ;
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

Reference< XControlModel > SAL_CALL ProgressBar::getModel() throw( RuntimeException )
{
    // A model is not possible for this control.
    return Reference< XControlModel >();
}

//____________________________________________________________________________________________________________
//  impl but public method to register service
//____________________________________________________________________________________________________________

const Sequence< OUString > ProgressBar::impl_getStaticSupportedServiceNames()
{
    MutexGuard aGuard( Mutex::getGlobalMutex() );
    Sequence< OUString > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = SERVICENAME_PROGRESSBAR;
    return seqServiceNames ;
}

//____________________________________________________________________________________________________________
//  impl but public method to register service
//____________________________________________________________________________________________________________

const OUString ProgressBar::impl_getStaticImplementationName()
{
    return OUString(IMPLEMENTATIONNAME_PROGRESSBAR );
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

void ProgressBar::impl_paint ( sal_Int32 nX, sal_Int32 nY, const Reference< XGraphics > & rGraphics )
{
    // save impossible cases
    DBG_ASSERT ( rGraphics.is(), "ProgressBar::paint()\nCalled with invalid Reference< XGraphics > ." ) ;

    // This paint method ist not buffered !!
    // Every request paint the completely control. ( but only, if peer exist )
     if ( rGraphics.is () )
    {
        MutexGuard  aGuard (m_aMutex) ;

        // Clear background
        // (same color for line and fill)
        rGraphics->setFillColor ( m_nBackgroundColor                        ) ;
        rGraphics->setLineColor ( m_nBackgroundColor                        ) ;
        rGraphics->drawRect     ( nX, nY, impl_getWidth(), impl_getHeight() ) ;

        // same color for line and fill for blocks
        rGraphics->setFillColor ( m_nForegroundColor ) ;
        rGraphics->setLineColor ( m_nForegroundColor ) ;

        sal_Int32   nBlockStart     =   0                                                                           ;   // = left site of new block
        sal_Int32   nBlockCount     =   m_nBlockValue!=0.00 ? (sal_Int32)((m_nValue-m_nMinRange)/m_nBlockValue) : 0 ;   // = number of next block

        // Draw horizontal progressbar
        // decision in "recalcRange()"
        if (m_bHorizontal)
        {
            // Step to left side of window
            nBlockStart = nX ;

            for ( sal_Int16 i=1; i<=nBlockCount; ++i )
            {
                // step free field
                nBlockStart +=  PROGRESSBAR_FREESPACE   ;
                // paint block
                rGraphics->drawRect (nBlockStart, nY+PROGRESSBAR_FREESPACE, m_aBlockSize.Width, m_aBlockSize.Height) ;
                // step next free field
                nBlockStart +=  m_aBlockSize.Width ;
            }
        }
        // draw vertikal progressbar
        // decision in "recalcRange()"
        else
        {
            // step to bottom side of window
            nBlockStart  =  nY+impl_getHeight() ;
            nBlockStart -=  m_aBlockSize.Height ;

            for ( sal_Int16 i=1; i<=nBlockCount; ++i )
            {
                // step free field
                nBlockStart -=  PROGRESSBAR_FREESPACE   ;
                // paint block
                rGraphics->drawRect (nX+PROGRESSBAR_FREESPACE, nBlockStart, m_aBlockSize.Width, m_aBlockSize.Height) ;
                // step next free field
                nBlockStart -=  m_aBlockSize.Height;
            }
        }

        // Paint shadow border around the progressbar
        rGraphics->setLineColor ( PROGRESSBAR_LINECOLOR_SHADOW                          ) ;
        rGraphics->drawLine     ( nX, nY, impl_getWidth(), nY               ) ;
        rGraphics->drawLine     ( nX, nY, nX             , impl_getHeight() ) ;

        rGraphics->setLineColor ( PROGRESSBAR_LINECOLOR_BRIGHT                                                              ) ;
        rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, impl_getWidth()-1, nY                  ) ;
        rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, nX               , impl_getHeight()-1  ) ;
    }
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

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
        m_bHorizontal = sal_True                            ;
        fBlockHeight  = (nWindowHeight-(2*PROGRESSBAR_FREESPACE))       ;
        fBlockWidth   = fBlockHeight                        ;
        fMaxBlocks    = nWindowWidth/(fBlockWidth+PROGRESSBAR_FREESPACE);
    }
    else
    {
        m_bHorizontal = sal_False                             ;
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

}   // namespace unocontrols

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
