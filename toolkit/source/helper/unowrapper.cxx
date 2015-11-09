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

#include <com/sun/star/awt/WindowEvent.hpp>
#include <comphelper/processfactory.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/awt/vclxcontainer.hxx>
#include <toolkit/awt/vclxtopwindow.hxx>
#include <toolkit/awt/vclxgraphics.hxx>

#include "toolkit/dllapi.h"
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>

#include <tools/debug.hxx>

#include "helper/unowrapper.hxx"

using namespace ::com::sun::star;

css::uno::Reference< css::awt::XWindowPeer > CreateXWindow( vcl::Window* pWindow )
{
    switch ( pWindow->GetType() )
    {
        case WINDOW_IMAGEBUTTON:
        case WINDOW_SPINBUTTON:
        case WINDOW_MENUBUTTON:
        case WINDOW_MOREBUTTON:
        case WINDOW_PUSHBUTTON:
        case WINDOW_HELPBUTTON:
        case WINDOW_OKBUTTON:
        case WINDOW_CANCELBUTTON:   return new VCLXButton;
        case WINDOW_CHECKBOX:       return new VCLXCheckBox;
        // #i95042#
        // A Window of type <MetricBox> is inherited from type <ComboBox>.
        // Thus, it does make more sense to return a <VCLXComboBox> instance
        // instead of only a <VCLXWindow> instance, especially regarding its
        // corresponding accessibility API.
        case WINDOW_METRICBOX:
        case WINDOW_COMBOBOX:       return new VCLXComboBox;
        case WINDOW_SPINFIELD:
        case WINDOW_NUMERICFIELD:
        case WINDOW_CURRENCYFIELD:  return new VCLXNumericField;
        case WINDOW_DATEFIELD:      return new VCLXDateField;
        case WINDOW_MULTILINEEDIT:
        case WINDOW_EDIT:           return new VCLXEdit;
        case WINDOW_METRICFIELD:    return new VCLXSpinField;
        case WINDOW_MESSBOX:
        case WINDOW_INFOBOX:
        case WINDOW_WARNINGBOX:
        case WINDOW_QUERYBOX:
        case WINDOW_ERRORBOX:       return new VCLXMessageBox;
        case WINDOW_FIXEDIMAGE:     return new VCLXImageControl;
        case WINDOW_FIXEDTEXT:      return new VCLXFixedText;
        case WINDOW_MULTILISTBOX:
        case WINDOW_LISTBOX:        return new VCLXListBox;
        case WINDOW_LONGCURRENCYFIELD:  return new VCLXCurrencyField;
        case WINDOW_DIALOG:
        case WINDOW_MODALDIALOG:
        case WINDOW_TABDIALOG:
        case WINDOW_BUTTONDIALOG:
        case WINDOW_MODELESSDIALOG: return new VCLXDialog;
        case WINDOW_PATTERNFIELD:   return new VCLXPatternField;
        case WINDOW_RADIOBUTTON:    return new VCLXRadioButton;
        case WINDOW_SCROLLBAR:      return new VCLXScrollBar;
        case WINDOW_TIMEFIELD:      return new VCLXTimeField;

        case WINDOW_SYSWINDOW:
        case WINDOW_WORKWINDOW:
        case WINDOW_DOCKINGWINDOW:
        case WINDOW_FLOATINGWINDOW:
        case WINDOW_HELPTEXTWINDOW: return new VCLXTopWindow;

        case WINDOW_WINDOW:
        case WINDOW_TABPAGE:        return new VCLXContainer;

        case WINDOW_TOOLBOX:        return new VCLXToolBox;
        case WINDOW_TABCONTROL:     return new VCLXMultiPage;

        // case WINDOW_FIXEDLINE:
        // case WINDOW_FIXEDBITMAP:
        // case WINDOW_DATEBOX:
        // case WINDOW_GROUPBOX:
        // case WINDOW_LONGCURRENCYBOX:
        // case WINDOW_SPLITTER:
        // case WINDOW_STATUSBAR:
        // case WINDOW_TABCONTROL:
        // case WINDOW_NUMERICBOX:
        // case WINDOW_TRISTATEBOX:
        // case WINDOW_TIMEBOX:
        // case WINDOW_SPLITWINDOW:
        // case WINDOW_SCROLLBARBOX:
        // case WINDOW_PATTERNBOX:
        // case WINDOW_CURRENCYBOX:
        default:                    return new VCLXWindow( true );
    }
}


//  class UnoWrapper


extern "C" {

TOOLKIT_DLLPUBLIC UnoWrapperBase* CreateUnoWrapper()
{
    return new UnoWrapper( NULL );
}

}   // extern "C"


UnoWrapper::UnoWrapper( const css::uno::Reference< css::awt::XToolkit>& rxToolkit )
{
    mxToolkit = rxToolkit;
}

void UnoWrapper::Destroy()
{
    delete this;
}

UnoWrapper::~UnoWrapper()
{
}

css::uno::Reference< css::awt::XToolkit> UnoWrapper::GetVCLToolkit()
{
    if ( !mxToolkit.is() )
        mxToolkit = VCLUnoHelper::CreateToolkit();
    return mxToolkit.get();
}

css::uno::Reference< css::awt::XWindowPeer> UnoWrapper::GetWindowInterface( vcl::Window* pWindow, bool bCreate )
{
    css::uno::Reference< css::awt::XWindowPeer> xPeer = pWindow->GetWindowPeer();
    if ( !xPeer.is() && bCreate )
    {
        xPeer = CreateXWindow( pWindow );
        SetWindowInterface( pWindow, xPeer );
    }
    return xPeer;
}

void UnoWrapper::SetWindowInterface( vcl::Window* pWindow, css::uno::Reference< css::awt::XWindowPeer> xIFace )
{
    VCLXWindow* pVCLXWindow = VCLXWindow::GetImplementation( xIFace );

    DBG_ASSERT( pVCLXWindow, "SetComponentInterface - unsupported type" );
    if ( pVCLXWindow )
    {
        css::uno::Reference< css::awt::XWindowPeer> xPeer = pWindow->GetWindowPeer();
        if( xPeer.is() )
        {
            bool bSameInstance( pVCLXWindow == dynamic_cast< VCLXWindow* >( xPeer.get() ));
            DBG_ASSERT( bSameInstance, "UnoWrapper::SetWindowInterface: there already *is* a WindowInterface for this window!" );
            if ( bSameInstance )
                return;
        }
        pVCLXWindow->SetWindow( pWindow );
        pWindow->SetWindowPeer( xIFace, pVCLXWindow );
    }
}

css::uno::Reference< css::awt::XGraphics> UnoWrapper::CreateGraphics( OutputDevice* pOutDev )
{
    css::uno::Reference< css::awt::XGraphics> xGrf;
    VCLXGraphics* pGrf = new VCLXGraphics;
    xGrf = pGrf;
    pGrf->Init( pOutDev );
    return xGrf;
}

void UnoWrapper::ReleaseAllGraphics( OutputDevice* pOutDev )
{
    std::vector< VCLXGraphics* > *pLst = pOutDev->GetUnoGraphicsList();
    if ( pLst )
    {
        for ( size_t n = 0; n < pLst->size(); n++ )
        {
            VCLXGraphics* pGrf = (*pLst)[ n ];
            pGrf->SetOutputDevice( NULL );
        }
    }

}

static bool lcl_ImplIsParent( vcl::Window* pParentWindow, vcl::Window* pPossibleChild )
{
    vcl::Window* pWindow = ( pPossibleChild != pParentWindow ) ? pPossibleChild : NULL;
    while ( pWindow && ( pWindow != pParentWindow ) )
        pWindow = pWindow->GetParent();

    return pWindow != nullptr;
}

void UnoWrapper::WindowDestroyed( vcl::Window* pWindow )
{
    // their still might be some children created with css::loader::Java
    // that would otherwise not be destroyed until the garbage collector cleans up
    VclPtr< vcl::Window > pChild = pWindow->GetWindow( GetWindowType::FirstChild );
    while ( pChild )
    {
        VclPtr< vcl::Window > pNextChild = pChild->GetWindow( GetWindowType::Next );

        VclPtr< vcl::Window > pClient = pChild->GetWindow( GetWindowType::Client );
        if ( pClient && pClient->GetWindowPeer() )
        {
            css::uno::Reference< css::lang::XComponent > xComp( pClient->GetComponentInterface( false ), css::uno::UNO_QUERY );
            xComp->dispose();
        }

        pChild = pNextChild;
    }

    // System-Windows suchen...
    VclPtr< vcl::Window > pOverlap = pWindow->GetWindow( GetWindowType::Overlap );
    if ( pOverlap )
    {
        pOverlap = pOverlap->GetWindow( GetWindowType::FirstOverlap );
        while ( pOverlap )
        {
            VclPtr< vcl::Window > pNextOverlap = pOverlap->GetWindow( GetWindowType::Next );
            VclPtr< vcl::Window > pClient = pOverlap->GetWindow( GetWindowType::Client );

            if ( pClient && pClient->GetWindowPeer() && lcl_ImplIsParent( pWindow, pClient ) )
            {
                css::uno::Reference< css::lang::XComponent > xComp( pClient->GetComponentInterface( false ), css::uno::UNO_QUERY );
                xComp->dispose();
            }

            pOverlap = pNextOverlap;
        }
    }

    {
        VclPtr< vcl::Window > pParent = pWindow->GetParent();
        if ( pParent && pParent->GetWindowPeer() )
            pParent->GetWindowPeer()->notifyWindowRemoved( *pWindow );
    }

    VCLXWindow* pWindowPeer = pWindow->GetWindowPeer();
    uno::Reference< lang::XComponent > xWindowPeerComp( pWindow->GetComponentInterface( false ), uno::UNO_QUERY );
    OSL_ENSURE( ( pWindowPeer != NULL ) == xWindowPeerComp.is(),
        "UnoWrapper::WindowDestroyed: inconsistency in the window's peers!" );
    if ( pWindowPeer )
    {
        pWindowPeer->SetWindow( NULL );
        pWindow->SetWindowPeer( NULL, NULL );
    }
    if ( xWindowPeerComp.is() )
        xWindowPeerComp->dispose();

    // #102132# Iterate over frames after setting Window peer to NULL,
    // because while destroying other frames, we get get into the method again and try
    // to destroy this window again...
    // #i42462#/#116855# no, don't loop: Instead, just ensure that all our top-window-children
    // are disposed, too (which should also be a valid fix for #102132#, but doesn't have the extreme
    // performance penalties)
    VclPtr< vcl::Window > pTopWindowChild = pWindow->GetWindow( GetWindowType::FirstTopWindowChild );
    while ( pTopWindowChild )
    {
        OSL_ENSURE( pTopWindowChild->GetParent() == pWindow,
                    "UnoWrapper::WindowDestroyed: inconsistency in the SystemWindow relationship!" );

        VclPtr< vcl::Window > pNextTopChild = pTopWindowChild->GetWindow( GetWindowType::NextTopWindowSibling );

        //the window still could be on the stack, so we have to
        // use lazy delete ( it will automatically
        // disconnect from the currently destroyed parent window )
        pTopWindowChild->doLazyDelete();

        pTopWindowChild = pNextTopChild;
    }
}


css::uno::Reference< css::accessibility::XAccessible > UnoWrapper::CreateAccessible( Menu* pMenu, bool bIsMenuBar )
{
    return maAccessibleFactoryAccess.getFactory().createAccessible( pMenu, bIsMenuBar );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
