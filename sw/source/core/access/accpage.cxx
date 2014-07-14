/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <rtl/uuid.h>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include "accpage.hxx"

#ifndef _ACCESS_HRC
#include "access.hrc"
#endif
#include <pagefrm.hxx>

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
        // necessary to notify that object if the cursor leaves it.
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
                                    const SwFrm* pFrame )
    : SwAccessibleContext( pInitMap, AccessibleRole::PANEL, pFrame )
    , bIsSelected( sal_False )
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
