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
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <rtl/uuid.h>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <tools/debug.hxx>
#include <helper/scrollabledialog.hxx>
#include <toolkit/helper/property.hxx>


//  class VCLXContainer


void VCLXContainer::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    VCLXWindow::ImplGetPropertyIds( rIds );
}

VCLXContainer::VCLXContainer()
{
}

VCLXContainer::~VCLXContainer()
{
}

// css::uno::XInterface
css::uno::Any VCLXContainer::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XVclContainer* >(this),
                                        static_cast< css::awt::XVclContainerPeer* >(this) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXContainer )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXContainer::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XVclContainer>::get(),
        cppu::UnoType<css::awt::XVclContainerPeer>::get(),
        VCLXWindow::getTypes()
    );
    return aTypeList.getTypes();
}


// css::awt::XVclContainer
void VCLXContainer::addVclContainerListener( const css::uno::Reference< css::awt::XVclContainerListener >& rxListener )
{
    SolarMutexGuard aGuard;

    GetContainerListeners().addInterface( rxListener );
}

void VCLXContainer::removeVclContainerListener( const css::uno::Reference< css::awt::XVclContainerListener >& rxListener )
{
    SolarMutexGuard aGuard;

    GetContainerListeners().removeInterface( rxListener );
}

css::uno::Sequence< css::uno::Reference< css::awt::XWindow > > VCLXContainer::getWindows(  )
{
    SolarMutexGuard aGuard;

    // Request container interface from all children
    css::uno::Sequence< css::uno::Reference< css::awt::XWindow > > aSeq;
    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
    {
        sal_uInt16 nChildren = pWindow->GetChildCount();
        if ( nChildren )
        {
            aSeq = css::uno::Sequence< css::uno::Reference< css::awt::XWindow > >( nChildren );
            css::uno::Reference< css::awt::XWindow > * pChildRefs = aSeq.getArray();
            for ( sal_uInt16 n = 0; n < nChildren; n++ )
            {
                vcl::Window* pChild = pWindow->GetChild( n );
                css::uno::Reference< css::awt::XWindowPeer >  xWP = pChild->GetComponentInterface();
                css::uno::Reference< css::awt::XWindow > xW( xWP, css::uno::UNO_QUERY );
                pChildRefs[n] = xW;
            }
        }
    }
    return aSeq;
}


// css::awt::XVclContainerPeer
void VCLXContainer::enableDialogControl( sal_Bool bEnable )
{
    SolarMutexGuard aGuard;

    VclPtr<vcl::Window> pWindow = GetWindow();
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

void VCLXContainer::setTabOrder( const css::uno::Sequence< css::uno::Reference< css::awt::XWindow > >& Components, const css::uno::Sequence< css::uno::Any >& Tabs, sal_Bool bGroupControl )
{
    SolarMutexGuard aGuard;

    sal_uInt32 nCount = Components.getLength();
    DBG_ASSERT( nCount == static_cast<sal_uInt32>(Tabs.getLength()), "setTabOrder: TabCount != ComponentCount" );
    const css::uno::Reference< css::awt::XWindow > * pComps = Components.getConstArray();
    const css::uno::Any* pTabs = Tabs.getConstArray();

    vcl::Window* pPrevWin = nullptr;
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        // css::style::TabStop
        VclPtr<vcl::Window> pWin = VCLUnoHelper::GetWindow( pComps[n] );
        // May be NULL if a css::uno::Sequence is originated from TabController and is missing a peer!
        if ( pWin )
        {
            // Order windows before manipulating their style, because elements such as the
            // RadioButton considers the PREV-window in StateChanged.
            if ( pPrevWin )
                pWin->SetZOrder( pPrevWin, ZOrderFlags::Behind );

            WinBits nStyle = pWin->GetStyle();
            nStyle &= ~(WB_TABSTOP|WB_NOTABSTOP|WB_GROUP);
            if ( pTabs[n].getValueType().getTypeClass() == css::uno::TypeClass_BOOLEAN )
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

void VCLXContainer::setGroup( const css::uno::Sequence< css::uno::Reference< css::awt::XWindow > >& Components )
{
    SolarMutexGuard aGuard;

    sal_uInt32 nCount = Components.getLength();
    const css::uno::Reference< css::awt::XWindow > * pComps = Components.getConstArray();

    vcl::Window* pPrevWin = nullptr;
    vcl::Window* pPrevRadio = nullptr;
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        VclPtr<vcl::Window> pWin = VCLUnoHelper::GetWindow( pComps[n] );
        if ( pWin )
        {
            vcl::Window* pSortBehind = pPrevWin;
            // #57096# Sort all radios consecutively
            bool bNewPrevWin = true;
            if ( pWin->GetType() == WindowType::RADIOBUTTON )
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
                pWin->SetZOrder( pSortBehind, ZOrderFlags::Behind );

            WinBits nStyle = pWin->GetStyle();
            if ( n == 0 )
                nStyle |= WB_GROUP;
            else
                nStyle &= (~WB_GROUP);
            pWin->SetStyle( nStyle );

            // Add WB_GROUP after the last group
            if ( n == ( nCount - 1 ) )
            {
                vcl::Window* pBehindLast = pWin->GetWindow( GetWindowType::Next );
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
    const css::uno::Any& Value )
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
            VclPtr<vcl::Window> pWindow = GetWindow();
            MapMode aMode( MapUnit::MapAppFont );
            toolkit::ScrollableDialog* pScrollable = dynamic_cast< toolkit::ScrollableDialog* >( pWindow.get() );
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
