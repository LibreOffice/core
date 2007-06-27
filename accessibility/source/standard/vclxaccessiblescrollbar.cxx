/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessiblescrollbar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:40:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"

// includes --------------------------------------------------------------

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLESCROLLBAR_HXX
#include <accessibility/standard/vclxaccessiblescrollbar.hxx>
#endif

#ifndef _TOOLKIT_AWT_VCLXWINDOWS_HXX
#include <toolkit/awt/vclxwindows.hxx>
#endif
#ifndef ACCESSIBILITY_HELPER_TKARESMGR_HXX
#include <accessibility/helper/accresmgr.hxx>
#endif
#ifndef ACCESSIBILITY_HELPER_ACCESSIBLESTRINGS_HRC_
#include <accessibility/helper/accessiblestrings.hrc>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SCROLLBARORIENTATION_HPP_
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX
#include <comphelper/sequence.hxx>
#endif

#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


// -----------------------------------------------------------------------------
// VCLXAccessibleScrollBar
// -----------------------------------------------------------------------------

VCLXAccessibleScrollBar::VCLXAccessibleScrollBar( VCLXWindow* pVCLWindow )
    :VCLXAccessibleComponent( pVCLWindow )
{
}

// -----------------------------------------------------------------------------

VCLXAccessibleScrollBar::~VCLXAccessibleScrollBar()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleScrollBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_SCROLLBAR_SCROLL:
        {
            NotifyAccessibleEvent( AccessibleEventId::VALUE_CHANGED, Any(), Any() );
        }
        break;
        default:
            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
   }
}

// -----------------------------------------------------------------------------

void VCLXAccessibleScrollBar::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );

    VCLXScrollBar* pVCLXScrollBar = static_cast< VCLXScrollBar* >( GetVCLXWindow() );
    if ( pVCLXScrollBar )
    {
        rStateSet.AddState( AccessibleStateType::FOCUSABLE );
        if ( pVCLXScrollBar->getOrientation() == ScrollBarOrientation::HORIZONTAL )
            rStateSet.AddState( AccessibleStateType::HORIZONTAL );
        else if ( pVCLXScrollBar->getOrientation() == ScrollBarOrientation::VERTICAL )
            rStateSet.AddState( AccessibleStateType::VERTICAL );
    }
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleScrollBar, VCLXAccessibleComponent, VCLXAccessibleScrollBar_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleScrollBar, VCLXAccessibleComponent, VCLXAccessibleScrollBar_BASE )

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleScrollBar::getImplementationName() throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleScrollBar" );
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > VCLXAccessibleScrollBar::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.AccessibleScrollBar" );
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessibleAction
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleScrollBar::getAccessibleActionCount( ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 4;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleScrollBar::doAccessibleAction ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    sal_Bool bReturn = sal_False;
    ScrollBar* pScrollBar = static_cast< ScrollBar* >( GetWindow() );
    if ( pScrollBar )
    {
        ScrollType eScrollType;
        switch ( nIndex )
        {
            case 0:     eScrollType = SCROLL_LINEUP;    break;
            case 1:     eScrollType = SCROLL_LINEDOWN;  break;
            case 2:     eScrollType = SCROLL_PAGEUP;    break;
            case 3:     eScrollType = SCROLL_PAGEDOWN;  break;
            default:    eScrollType = SCROLL_DONTKNOW;  break;
        }
        if ( pScrollBar->DoScrollAction( eScrollType ) )
            bReturn = sal_True;
    }

    return bReturn;
}

// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleScrollBar::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    ::rtl::OUString sDescription;

    switch ( nIndex )
    {
        case 0:     sDescription = ::rtl::OUString( TK_RES_STRING( RID_STR_ACC_ACTION_DECLINE ) );      break;
        case 1:     sDescription = ::rtl::OUString( TK_RES_STRING( RID_STR_ACC_ACTION_INCLINE ) );      break;
        case 2:     sDescription = ::rtl::OUString( TK_RES_STRING( RID_STR_ACC_ACTION_DECBLOCK ) );     break;
        case 3:     sDescription = ::rtl::OUString( TK_RES_STRING( RID_STR_ACC_ACTION_INCBLOCK ) );     break;
        default:                                                                                        break;
    }

    return sDescription;
}

// -----------------------------------------------------------------------------

Reference< XAccessibleKeyBinding > VCLXAccessibleScrollBar::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessibleKeyBinding >();
}

// -----------------------------------------------------------------------------
// XAccessibleValue
// -----------------------------------------------------------------------------

Any VCLXAccessibleScrollBar::getCurrentValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;

    VCLXScrollBar* pVCLXScrollBar = static_cast< VCLXScrollBar* >( GetVCLXWindow() );
    if ( pVCLXScrollBar )
        aValue <<= (sal_Int32) pVCLXScrollBar->getValue();

    return aValue;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleScrollBar::setCurrentValue( const Any& aNumber ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;

    VCLXScrollBar* pVCLXScrollBar = static_cast< VCLXScrollBar* >( GetVCLXWindow() );
    if ( pVCLXScrollBar )
    {
        sal_Int32 nValue = 0, nValueMin = 0, nValueMax = 0;
        OSL_VERIFY( aNumber >>= nValue );
        OSL_VERIFY( getMinimumValue() >>= nValueMin );
        OSL_VERIFY( getMaximumValue() >>= nValueMax );

        if ( nValue < nValueMin )
            nValue = nValueMin;
        else if ( nValue > nValueMax )
            nValue = nValueMax;

        pVCLXScrollBar->setValue( nValue );
        bReturn = sal_True;
    }

    return bReturn;
}

// -----------------------------------------------------------------------------

Any VCLXAccessibleScrollBar::getMaximumValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;

    VCLXScrollBar* pVCLXScrollBar = static_cast< VCLXScrollBar* >( GetVCLXWindow() );
    if ( pVCLXScrollBar )
        aValue <<= (sal_Int32) pVCLXScrollBar->getMaximum();

    return aValue;
}

// -----------------------------------------------------------------------------

Any VCLXAccessibleScrollBar::getMinimumValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= (sal_Int32) 0;

    return aValue;
}

// -----------------------------------------------------------------------------
