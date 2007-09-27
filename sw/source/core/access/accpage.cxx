 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accpage.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:22:53 $
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
#include "precompiled_sw.hxx"


#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif

#ifndef _ACCPAGE_HXX
#include "accpage.hxx"
#endif

#ifndef _ACCESS_HRC
#include "access.hrc"
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using uno::Reference;
using uno::RuntimeException;
using uno::Sequence;
using ::rtl::OUString;


const sal_Char sServiceName[] = "com.sun.star.text.AccessiblePageView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessiblePageView";

sal_Bool SwAccessiblePage::IsSelected()
{
    return GetMap()->IsPageSelected( static_cast < const SwPageFrm * >( GetFrm() ) );
}

void SwAccessiblePage::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // FOCUSABLE
    rStateSet.AddState( AccessibleStateType::FOCUSABLE );

    // FOCUSED
    if( IsSelected() )
    {
        ASSERT( bIsSelected, "bSelected out of sync" );
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );

        Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() )
            rStateSet.AddState( AccessibleStateType::FOCUSED );
    }
}

void SwAccessiblePage::_InvalidateCursorPos()
{
    sal_Bool bNewSelected = IsSelected();
    sal_Bool bOldSelected;

    {
        vos::OGuard aGuard( aMutex );
        bOldSelected = bIsSelected;
        bIsSelected = bNewSelected;
    }

    if( bNewSelected )
    {
        // remember that object as the one that has the caret. This is
        // neccessary to notify that object if the cursor leaves it.
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }

    if( bOldSelected != bNewSelected )
    {
        Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() )
            FireStateChangedEvent( AccessibleStateType::FOCUSED, bNewSelected );
    }
}

void SwAccessiblePage::_InvalidateFocus()
{
    Window *pWin = GetWindow();
    if( pWin )
    {
        sal_Bool bSelected;

        {
            vos::OGuard aGuard( aMutex );
            bSelected = bIsSelected;
        }
        ASSERT( bSelected, "focus object should be selected" );

        FireStateChangedEvent( AccessibleStateType::FOCUSED,
                               pWin->HasFocus() && bSelected );
    }
}

SwAccessiblePage::SwAccessiblePage( SwAccessibleMap* pInitMap,
                                    const SwFrm* pFrame ) :
    SwAccessibleContext( pInitMap, AccessibleRole::PANEL, pFrame )
{
    DBG_ASSERT( pFrame != NULL, "need frame" );
    DBG_ASSERT( pInitMap != NULL, "need map" );
    DBG_ASSERT( pFrame->IsPageFrm(), "need page frame" );

    vos::OGuard aGuard(Application::GetSolarMutex());

    OUString sPage = OUString::valueOf(
        static_cast<sal_Int32>(
            static_cast<const SwPageFrm*>( GetFrm() )->GetPhyPageNum() ) );
    SetName( GetResource( STR_ACCESS_PAGE_NAME, &sPage ) );
}

SwAccessiblePage::~SwAccessiblePage()
{
}

sal_Bool SwAccessiblePage::HasCursor()
{
    vos::OGuard aGuard( aMutex );
    return bIsSelected;
}

OUString SwAccessiblePage::getImplementationName( )
    throw( RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationName));
}

sal_Bool SwAccessiblePage::supportsService( const OUString& rServiceName)
    throw( RuntimeException )
{
    return rServiceName.equalsAsciiL( sServiceName, sizeof(sServiceName)-1 ) ||
           rServiceName.equalsAsciiL( sAccessibleServiceName,
                                   sizeof(sAccessibleServiceName)-1 );
}

Sequence<OUString> SwAccessiblePage::getSupportedServiceNames( )
    throw( RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

Sequence< sal_Int8 > SAL_CALL SwAccessiblePage::getImplementationId()
        throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

OUString SwAccessiblePage::getAccessibleDescription( )
    throw( RuntimeException )
{
    CHECK_FOR_DEFUNC( ::com::sun::star::accessibility::XAccessibleContext );

    OUString sArg( GetFormattedPageNumber() );
    return GetResource( STR_ACCESS_PAGE_DESC, &sArg );
}
