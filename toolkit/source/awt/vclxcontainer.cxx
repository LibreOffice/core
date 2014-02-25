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

#include <toolkit/awt/vclxcontainer.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uuid.h>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <tools/debug.hxx>
#include "toolkit/awt/scrollabledialog.hxx"
#include <toolkit/helper/property.hxx>


//  class VCLXContainer


void VCLXContainer::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    VCLXWindow::ImplGetPropertyIds( rIds );
}

VCLXContainer::VCLXContainer()
{
}

VCLXContainer::~VCLXContainer()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXContainer::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XVclContainer* >(this)),
                                        (static_cast< ::com::sun::star::awt::XVclContainerPeer* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXContainer )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclContainer>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclContainerPeer>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END


// ::com::sun::star::awt::XVclContainer
void VCLXContainer::addVclContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclContainerListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    GetContainerListeners().addInterface( rxListener );
}

void VCLXContainer::removeVclContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclContainerListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    GetContainerListeners().removeInterface( rxListener );
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > > VCLXContainer::getWindows(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // Request container interface from all children
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > > aSeq;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        sal_uInt16 nChildren = pWindow->GetChildCount();
        if ( nChildren )
        {
            aSeq = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > >( nChildren );
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > * pChildRefs = aSeq.getArray();
            for ( sal_uInt16 n = 0; n < nChildren; n++ )
            {
                Window* pChild = pWindow->GetChild( n );
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xWP = pChild->GetComponentInterface( sal_True );
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xW( xWP, ::com::sun::star::uno::UNO_QUERY );
                pChildRefs[n] = xW;
            }
        }
    }
    return aSeq;
}


// ::com::sun::star::awt::XVclContainerPeer
void VCLXContainer::enableDialogControl( sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        if ( bEnable )
            nStyle |= WB_DIALOGCONTROL;
        else
            nStyle &= (~WB_DIALOGCONTROL);
        pWindow->SetStyle( nStyle );
    }
}

void VCLXContainer::setTabOrder( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > >& Components, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Tabs, sal_Bool bGroupControl ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_uInt32 nCount = Components.getLength();
    DBG_ASSERT( nCount == (sal_uInt32)Tabs.getLength(), "setTabOrder: TabCount != ComponentCount" );
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > * pComps = Components.getConstArray();
    const ::com::sun::star::uno::Any* pTabs = Tabs.getConstArray();

    Window* pPrevWin = NULL;
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        // ::com::sun::star::style::TabStop
        Window* pWin = VCLUnoHelper::GetWindow( pComps[n] );
        // May be NULL if a ::com::sun::star::uno::Sequence is originated from TabController and is missing a peer!
        if ( pWin )
        {
            // Order windows before manipulating their style, because elements such as the
            // RadioButton considers the PREV-window in StateChanged.
            if ( pPrevWin )
                pWin->SetZOrder( pPrevWin, WINDOW_ZORDER_BEHIND );

            WinBits nStyle = pWin->GetStyle();
            nStyle &= ~(WB_TABSTOP|WB_NOTABSTOP|WB_GROUP);
            if ( pTabs[n].getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_BOOLEAN )
            {
                bool bTab = false;
                pTabs[n] >>= bTab;
                nStyle |= ( bTab ? WB_TABSTOP : WB_NOTABSTOP );
            }
            pWin->SetStyle( nStyle );

            if ( bGroupControl )
            {
                if ( n == 0 )
                    pWin->SetDialogControlStart( true );
                else
                    pWin->SetDialogControlStart( false );
            }

            pPrevWin = pWin;
        }
    }
}

void VCLXContainer::setGroup( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > >& Components ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_uInt32 nCount = Components.getLength();
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > * pComps = Components.getConstArray();

    Window* pPrevWin = NULL;
    Window* pPrevRadio = NULL;
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        Window* pWin = VCLUnoHelper::GetWindow( pComps[n] );
        if ( pWin )
        {
            Window* pSortBehind = pPrevWin;
            // #57096# Sort all radios consecutively
            bool bNewPrevWin = true;
            if ( pWin->GetType() == WINDOW_RADIOBUTTON )
            {
                if ( pPrevRadio )
                {
                    // This RadioButton was sorted before PrevWin
                    bNewPrevWin = ( pPrevWin == pPrevRadio );
                    pSortBehind = pPrevRadio;
                }
                pPrevRadio = pWin;
            }

            // Z-Order
            if ( pSortBehind )
                pWin->SetZOrder( pSortBehind, WINDOW_ZORDER_BEHIND );

            WinBits nStyle = pWin->GetStyle();
            if ( n == 0 )
                nStyle |= WB_GROUP;
            else
                nStyle &= (~WB_GROUP);
            pWin->SetStyle( nStyle );

            // Add WB_GROUP after the last group
            if ( n == ( nCount - 1 ) )
            {
                Window* pBehindLast = pWin->GetWindow( WINDOW_NEXT );
                if ( pBehindLast )
                {
                    WinBits nLastStyle = pBehindLast->GetStyle();
                    nLastStyle |= WB_GROUP;
                    pBehindLast->SetStyle( nLastStyle );
                }
            }

            if ( bNewPrevWin )
                pPrevWin = pWin;
        }
    }
}

void SAL_CALL VCLXContainer::setProperty(
    const OUString& PropertyName,
    const ::com::sun::star::uno::Any& Value )
throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_SCROLLHEIGHT:
        case BASEPROPERTY_SCROLLWIDTH:
        case BASEPROPERTY_SCROLLTOP:
        case BASEPROPERTY_SCROLLLEFT:
        {
            sal_Int32 nVal =0;
            Value >>= nVal;
            Size aSize( nVal, nVal );
            Window* pWindow = GetWindow();
            MapMode aMode( MAP_APPFONT );
            toolkit::ScrollableInterface* pScrollable = dynamic_cast< toolkit::ScrollableInterface* >( pWindow );
            if ( pWindow && pScrollable )
            {
                OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
                if ( !pDev )
                    pDev = pWindow->GetParent();
                // shouldn't happen but it appears pDev can be NULL
                // #FIXME ( find out how/why )
                if ( !pDev )
                    break;

                aSize = pDev->LogicToPixel( aSize, aMode );
                switch ( nPropType )
                {
                    case BASEPROPERTY_SCROLLHEIGHT:
                        pScrollable->SetScrollHeight( aSize.Height() );
                        break;
                    case BASEPROPERTY_SCROLLWIDTH:
                        pScrollable->SetScrollWidth( aSize.Width() );
                        break;
                    case BASEPROPERTY_SCROLLTOP:
                        pScrollable->SetScrollTop( aSize.Height() );
                        break;
                    case BASEPROPERTY_SCROLLLEFT:
                        pScrollable->SetScrollLeft( aSize.Width() );
                        break;
                    default:
                        break;
                }
                break;
            }
            break;
        }

        default:
        {
            VCLXWindow::setProperty( PropertyName, Value );
        }
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
