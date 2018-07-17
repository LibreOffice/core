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

#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/awt/vclxcontainer.hxx>
#include <toolkit/awt/vclxtopwindow.hxx>
#include <toolkit/awt/vclxgraphics.hxx>

#include <toolkit/dllapi.h>
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>

#include <tools/debug.hxx>

#include <helper/unowrapper.hxx>

using namespace ::com::sun::star;

css::uno::Reference< css::awt::XWindowPeer > CreateXWindow( vcl::Window const * pWindow )
{
    switch ( pWindow->GetType() )
    {
        case WindowType::IMAGEBUTTON:
        case WindowType::SPINBUTTON:
        case WindowType::MENUBUTTON:
        case WindowType::MOREBUTTON:
        case WindowType::PUSHBUTTON:
        case WindowType::HELPBUTTON:
        case WindowType::OKBUTTON:
        case WindowType::CANCELBUTTON:   return new VCLXButton;
        case WindowType::CHECKBOX:       return new VCLXCheckBox;
        // #i95042#
        // A Window of type <MetricBox> is inherited from type <ComboBox>.
        // Thus, it does make more sense to return a <VCLXComboBox> instance
        // instead of only a <VCLXWindow> instance, especially regarding its
        // corresponding accessibility API.
        case WindowType::METRICBOX:
        case WindowType::COMBOBOX:       return new VCLXComboBox;
        case WindowType::SPINFIELD:
        case WindowType::NUMERICFIELD:
        case WindowType::CURRENCYFIELD:  return new VCLXNumericField;
        case WindowType::DATEFIELD:      return new VCLXDateField;
        case WindowType::MULTILINEEDIT:
        case WindowType::EDIT:           return new VCLXEdit;
        case WindowType::METRICFIELD:    return new VCLXSpinField;
        case WindowType::MESSBOX:
        case WindowType::INFOBOX:
        case WindowType::WARNINGBOX:
        case WindowType::QUERYBOX:
        case WindowType::ERRORBOX:       return new VCLXMessageBox;
        case WindowType::FIXEDIMAGE:     return new VCLXImageControl;
        case WindowType::FIXEDTEXT:      return new VCLXFixedText;
        case WindowType::MULTILISTBOX:
        case WindowType::LISTBOX:        return new VCLXListBox;
        case WindowType::LONGCURRENCYFIELD:  return new VCLXCurrencyField;
        case WindowType::DIALOG:
        case WindowType::MODALDIALOG:
        case WindowType::TABDIALOG:
        case WindowType::BUTTONDIALOG:
        case WindowType::MODELESSDIALOG: return new VCLXDialog;
        case WindowType::PATTERNFIELD:   return new VCLXPatternField;
        case WindowType::RADIOBUTTON:    return new VCLXRadioButton;
        case WindowType::SCROLLBAR:      return new VCLXScrollBar;
        case WindowType::TIMEFIELD:      return new VCLXTimeField;

        case WindowType::WORKWINDOW:
        case WindowType::DOCKINGWINDOW:
        case WindowType::FLOATINGWINDOW:
        case WindowType::HELPTEXTWINDOW: return new VCLXTopWindow;

        case WindowType::WINDOW:
        case WindowType::TABPAGE:        return new VCLXContainer;

        case WindowType::TOOLBOX:        return new VCLXToolBox;
        case WindowType::TABCONTROL:     return new VCLXMultiPage;

        // case WindowType::FIXEDLINE:
        // case WindowType::FIXEDBITMAP:
        // case WindowType::DATEBOX:
        // case WindowType::GROUPBOX:
        // case WindowType::LONGCURRENCYBOX:
        // case WindowType::SPLITTER:
        // case WindowType::STATUSBAR:
        // case WindowType::TABCONTROL:
        // case WindowType::NUMERICBOX:
        // case WindowType::TRISTATEBOX:
        // case WindowType::TIMEBOX:
        // case WindowType::SPLITWINDOW:
        // case WindowType::SCROLLBARBOX:
        // case WindowType::PATTERNBOX:
        // case WindowType::CURRENCYBOX:
        default:                    return new VCLXWindow( true );
    }
}


//  class UnoWrapper


extern "C" {

TOOLKIT_DLLPUBLIC UnoWrapperBase* CreateUnoWrapper()
{
    return new UnoWrapper( nullptr );
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

css::uno::Reference< css::awt::XWindowPeer> UnoWrapper::GetWindowInterface( vcl::Window* pWindow )
{
    css::uno::Reference< css::awt::XWindowPeer> xPeer = pWindow->GetWindowPeer();
    if ( !xPeer.is() )
    {
        xPeer = CreateXWindow( pWindow );
        SetWindowInterface( pWindow, xPeer );
    }
    return xPeer;
}

VclPtr<vcl::Window> UnoWrapper::GetWindow(const css::uno::Reference<css::awt::XWindow>& rWindow)
{
    return VCLUnoHelper::GetWindow(rWindow);
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
        for (VCLXGraphics* pGrf : *pLst)
        {
            pGrf->SetOutputDevice( nullptr );
        }
    }

}

static bool lcl_ImplIsParent( vcl::Window const * pParentWindow, vcl::Window* pPossibleChild )
{
    vcl::Window* pWindow = ( pPossibleChild != pParentWindow ) ? pPossibleChild : nullptr;
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

    // find system windows...
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
    OSL_ENSURE( ( pWindowPeer != nullptr ) == xWindowPeerComp.is(),
        "UnoWrapper::WindowDestroyed: inconsistency in the window's peers!" );
    if ( pWindowPeer )
    {
        pWindowPeer->SetWindow( nullptr );
        pWindow->SetWindowPeer( nullptr, nullptr );
    }
    if ( xWindowPeerComp.is() )
        xWindowPeerComp->dispose();

    // #102132# Iterate over frames after setting Window peer to NULL,
    // because while destroying other frames, we get into the method again and try
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
