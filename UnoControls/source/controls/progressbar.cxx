/*************************************************************************
 *
 *  $RCSfile: progressbar.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:11:17 $
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

//____________________________________________________________________________________________________________
//  my own includes
//____________________________________________________________________________________________________________

#ifndef _UNOCONTROLS_PROGRESSBAR_CTRL_HXX
#include "progressbar.hxx"
#endif

//____________________________________________________________________________________________________________
//  includes of other projects
//____________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_GRADIENTSTYLE_HPP_
#include <com/sun/star/awt/GradientStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_RASTEROPERATION_HPP_
#include <com/sun/star/awt/RasterOperation.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XGRAPHICS_HPP_
#include <com/sun/star/awt/XGraphics.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#include <math.h>

//____________________________________________________________________________________________________________
//  includes of my project
//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________
//  namespace
//____________________________________________________________________________________________________________

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
    , m_bHorizontal         (    DEFAULT_HORIZONTAL         )
    , m_aBlockSize          (    DEFAULT_BLOCKDIMENSION     )
    , m_nBackgroundColor    (    DEFAULT_BACKGROUNDCOLOR    )
    , m_nForegroundColor    (    DEFAULT_FOREGROUNDCOLOR    )
    , m_nMinRange           (    DEFAULT_MINRANGE           )
    , m_nMaxRange           (    DEFAULT_MAXRANGE           )
    , m_nBlockValue         (    DEFAULT_BLOCKVALUE         )
    , m_nValue              (    DEFAULT_VALUE              )
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
    Reference< XInterface > xDelegator = BaseControl::impl_getDelegator();
    if ( xDelegator.is() == sal_True )
    {
        // If an delegator exist, forward question to his queryInterface.
        // Delegator will ask his own queryAggregation!
        aReturn = xDelegator->queryInterface( rType );
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

void SAL_CALL ProgressBar::acquire() throw( RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::acquire();
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL ProgressBar::release() throw( RuntimeException )
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
            static OTypeCollection aTypeCollection  (   ::getCppuType(( const Reference< XControlModel  >*)NULL )   ,
                                                          ::getCppuType(( const Reference< XProgressBar >*)NULL )   ,
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
                                               static_cast< XControlModel*  > ( this )  ,
                                               static_cast< XProgressBar*   > ( this )
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

void SAL_CALL ProgressBar::setPosSize ( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nFlags ) throw( RuntimeException )
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

sal_Bool SAL_CALL ProgressBar::setModel( const Reference< XControlModel >& xModel ) throw( RuntimeException )
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
    seqServiceNames.getArray() [0] = OUString::createFromAscii( SERVICENAME_PROGRESSBAR );
    return seqServiceNames ;
}

//____________________________________________________________________________________________________________
//  impl but public method to register service
//____________________________________________________________________________________________________________

const OUString ProgressBar::impl_getStaticImplementationName()
{
    return OUString::createFromAscii( IMPLEMENTATIONNAME_PROGRESSBAR );
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

        sal_Int32   nBlockStart     =   0   ;   // = left site of new block
        sal_Int32   nBlockCount     =   0   ;   // = number of next block

        // same color for line and fill for blocks
        rGraphics->setFillColor ( m_nForegroundColor ) ;
        rGraphics->setLineColor ( m_nForegroundColor ) ;

        sal_Int32 nDifference = labs ( m_nValue - m_nMinRange ) ;

        if ( m_nBlockValue == 0 )
        {
            // Prevent "division by zero"
            m_nBlockValue = 1 ;
        }
        // number of blocks for current value
        nBlockCount = nDifference / m_nBlockValue ;

        // Round to next valid block number
        if ( fmod ( (float)nDifference, (float)m_nBlockValue ) != 0 )
        {
            ++nBlockCount ;
        }

        // Draw horizontal progressbar
        // decision in "recalcRange()"
        if (m_bHorizontal)
        {
            // Step to left side of window
            nBlockStart = nX ;

            for ( sal_Int16 i=1; i<=nBlockCount; ++i )
            {
                // step free field
                nBlockStart +=  FREESPACE   ;
                // paint block
                rGraphics->drawRect (nBlockStart, nY+FREESPACE, m_aBlockSize.Width, m_aBlockSize.Height) ;
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
                nBlockStart -=  FREESPACE   ;
                // paint block
                rGraphics->drawRect (nX+FREESPACE, nBlockStart, m_aBlockSize.Width, m_aBlockSize.Height) ;
                // step next free field
                nBlockStart -=  m_aBlockSize.Height;
            }
        }

        // Paint shadow border around the progressbar
        rGraphics->setLineColor ( LINECOLOR_SHADOW                          ) ;
        rGraphics->drawLine     ( nX, nY, impl_getWidth(), nY               ) ;
        rGraphics->drawLine     ( nX, nY, nX             , impl_getHeight() ) ;

        rGraphics->setLineColor ( LINECOLOR_BRIGHT                                                              ) ;
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

    // Calculate size of block in progressbar
    // and switch the orientation of the control
    sal_Int32   nMaxBlock   = 0                 ;
    sal_Int32   nWidth      = impl_getWidth  () ;   // it's better for debug !
    sal_Int32   nHeight     = impl_getHeight () ;   // it's better for debug !

    // Default = horizontal progressbar ...
    m_bHorizontal = sal_True ;

    // ... but, if programmer say "NO IT's VERTICAL ..."
    if ( nWidth > nHeight )
    {
        // Don't forget to save this state! Used in "ProgressBar::paint()"
        m_bHorizontal = sal_True ;

        // Calculate count of blocks for actual size
        // (prevent error "division by zero")
        if ( nHeight == 0 )
        {
            nHeight = 1 ;
        }

        nMaxBlock    =  nWidth / nHeight    ;
        nMaxBlock   *=  2                   ;

        // prevent error "division by zero"
        if ( nMaxBlock == 0 )
        {
            nMaxBlock = 1 ;
        }

        // Calculate new value and new size for ONE block.

        // Do not a calculation like this: "m_nBlockValue=(m_nMaxRange-m_nMinRange)/nMaxBlock"  !
        // If difference between m_nMaxRange and m_nMinRange to large, it give an overflow and a
        // following error "division by zero" in method "paint() ... nBlockCount=nDifference/m_nBlockValue ..."

        // Overflow ? => example: _I32_MAX - _I32_MIN = -1 and not _UI32_MAX !!!

        m_nBlockValue       =   ( m_nMaxRange / nMaxBlock ) - ( m_nMinRange / nMaxBlock ) ;
        m_aBlockSize.Height =   ( nHeight - ( FREESPACE * 2 )                           ) ;
        m_aBlockSize.Width  =   ( ( nWidth / nMaxBlock ) - FREESPACE                    ) ;
    }
    else
    {
        // Don't forget to save this state! Used in "ProgressBar::paint()"
        m_bHorizontal = sal_False ;

        // Calculate count of blocks for actual size
        // (prevent error "division by zero")
        if ( nWidth == 0 )
        {
            nWidth = 1 ;
        }

        nMaxBlock    =  nHeight / nWidth    ;
        nMaxBlock   *=  2                   ;

        // prevent error "division by zero"
        if ( nMaxBlock == 0 )
        {
            nMaxBlock = 1 ;
        }

        // Calculate new value and new size for ONE block.

        // Do not a calculation like this: "m_nBlockValue=(m_nMaxRange-m_nMinRange)/nMaxBlock"  !
        // If difference between m_nMaxRange and m_nMinRange to large, it give an overflow and a
        // following error "division by zero" in method "paint() ... nBlockCount=nDifference/m_nBlockValue ..."

        // Overflow ? => example: _I32_MAX - _I32_MIN = -1 and not _UI32_MAX !!!

        m_nBlockValue       =   ( m_nMaxRange / nMaxBlock ) - ( m_nMinRange / nMaxBlock ) ;
        m_aBlockSize.Height =   ( ( nHeight / nMaxBlock ) - FREESPACE                   ) ;
        m_aBlockSize.Width  =   ( nWidth - ( FREESPACE * 2 )                            ) ;
    }
}

}   // namespace unocontrols
