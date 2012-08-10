/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <com/sun/star/awt/WindowEvent.hpp>
#include <comphelper/processfactory.hxx>

#include <toolkit/helper/unowrapper.hxx>
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

using namespace ::com::sun::star;

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > CreateXWindow( Window* pWindow )
{
    switch ( pWindow->GetType() )
    {
        case WINDOW_IMAGERADIOBUTTON:
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

//  ----------------------------------------------------
//  class UnoWrapper
//  ----------------------------------------------------

extern "C" {

TOOLKIT_DLLPUBLIC UnoWrapperBase* CreateUnoWrapper()
{
    return new UnoWrapper( NULL );
}

}   // extern "C"


UnoWrapper::UnoWrapper( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>& rxToolkit )
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

::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit> UnoWrapper::GetVCLToolkit()
{
    if ( !mxToolkit.is() )
        mxToolkit = VCLUnoHelper::CreateToolkit();
    return mxToolkit.get();
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> UnoWrapper::GetWindowInterface( Window* pWindow, sal_Bool bCreate )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> xPeer = pWindow->GetWindowPeer();
    if ( !xPeer.is() && bCreate )
    {
        xPeer = CreateXWindow( pWindow );
        SetWindowInterface( pWindow, xPeer );
    }
    return xPeer;
}

void UnoWrapper::SetWindowInterface( Window* pWindow, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> xIFace )
{
    VCLXWindow* pVCLXWindow = VCLXWindow::GetImplementation( xIFace );

    DBG_ASSERT( pVCLXWindow, "SetComponentInterface - unsupported type" );
    if ( pVCLXWindow )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> xPeer = pWindow->GetWindowPeer();
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

::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics> UnoWrapper::CreateGraphics( OutputDevice* pOutDev )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics> xGrf;
    VCLXGraphics* pGrf = new VCLXGraphics;
    xGrf = pGrf;
    pGrf->Init( pOutDev );
    return xGrf;
}

void UnoWrapper::ReleaseAllGraphics( OutputDevice* pOutDev )
{
    VCLXGraphicsList_impl* pLst = pOutDev->GetUnoGraphicsList();
    if ( pLst )
    {
        for ( size_t n = 0; n < pLst->size(); n++ )
        {
            VCLXGraphics* pGrf = (*pLst)[ n ];
            pGrf->SetOutputDevice( NULL );
        }
    }

}

// It was once called in the Window-CTOR to make listeners of the Container
// to react. This didn't really work, as the interface within the Window-CTOR
// was not ready.
// => Call only the listener, when created through ::com::sun::star::awt::Toolkit

/*
void ImplSmartWindowCreated( Window* pNewWindow )
{
    UNOWindowData* pParentUNOData = pNewWindow->GetParent() ?
                            pNewWindow->GetParent()->GetUNOData() : NULL;

    if ( pParentUNOData && pParentUNOData->GetListeners( EL_CONTAINER ) )
    {
        UNOWindowData* pUNOData = pNewWindow->GetUNOData();
        if ( !pUNOData )
            pUNOData = ImplSmartCreateUNOData( pNewWindow );

        ::com::sun::star::awt::VclContainerEvent aEvent;
        aEvent.Source = (UsrObject*)pParentUNOData->GetWindowPeer();
        aEvent.Id = VCLCOMPONENT_ADDED;
        aEvent.Child = (UsrObject*)pUNOData->GetWindowPeer();

        EventList* pLst = pParentUNOData->GetListeners( EL_CONTAINER );
        for ( sal_uInt32 n = 0; n < pLst->Count(); n++ )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > * pRef = pLst->GetObject( n );
            ((::com::sun::star::awt::XVclContainerListener*)(::com::sun::star::lang::XEventListener*)*pRef)->windowAdded( aEvent );
        }
    }
}
*/

sal_Bool lcl_ImplIsParent( Window* pParentWindow, Window* pPossibleChild )
{
    Window* pWindow = ( pPossibleChild != pParentWindow ) ? pPossibleChild : NULL;
    while ( pWindow && ( pWindow != pParentWindow ) )
        pWindow = pWindow->GetParent();

    return pWindow ? sal_True : sal_False;
}

void UnoWrapper::WindowDestroyed( Window* pWindow )
{
    // their still might be some children created with ::com::sun::star::loader::Java
    // that would otherwise not be destroyed until the garbage collector cleans up
    Window* pChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    while ( pChild )
    {
        Window* pNextChild = pChild->GetWindow( WINDOW_NEXT );

        Window* pClient = pChild->GetWindow( WINDOW_CLIENT );
        if ( pClient->GetWindowPeer() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComp( pClient->GetComponentInterface( sal_False ), ::com::sun::star::uno::UNO_QUERY );
            xComp->dispose();
        }

        pChild = pNextChild;
    }

    // ::com::sun::star::chaos::System-Windows suchen...
    Window* pOverlap = pWindow->GetWindow( WINDOW_OVERLAP );
    pOverlap = pOverlap->GetWindow( WINDOW_FIRSTOVERLAP );
    while ( pOverlap )
    {
        Window* pNextOverlap = pOverlap->GetWindow( WINDOW_NEXT );
        Window* pClient = pOverlap->GetWindow( WINDOW_CLIENT );

        if ( pClient->GetWindowPeer() && lcl_ImplIsParent( pWindow, pClient ) )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComp( pClient->GetComponentInterface( sal_False ), ::com::sun::star::uno::UNO_QUERY );
            xComp->dispose();
        }

        pOverlap = pNextOverlap;
    }

    Window* pParent = pWindow->GetParent();
    if ( pParent && pParent->GetWindowPeer() )
        pParent->GetWindowPeer()->notifyWindowRemoved( *pWindow );

    VCLXWindow* pWindowPeer = pWindow->GetWindowPeer();
    uno::Reference< lang::XComponent > xWindowPeerComp( pWindow->GetComponentInterface( sal_False ), uno::UNO_QUERY );
    OSL_ENSURE( ( pWindowPeer != NULL ) == ( xWindowPeerComp.is() == sal_True ),
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
    if ( pWindow )
    {
        Window* pTopWindowChild = pWindow->GetWindow( WINDOW_FIRSTTOPWINDOWCHILD );
        while ( pTopWindowChild )
        {
            OSL_ENSURE( pTopWindowChild->GetParent() == pWindow,
                        "UnoWrapper::WindowDestroyed: inconsistency in the SystemWindow relationship!" );

            Window* pNextTopChild = pTopWindowChild->GetWindow( WINDOW_NEXTTOPWINDOWSIBLING );

            //the window still could be on the stack, so we have to
            // use lazy delete ( it will automatically
            // disconnect from the currently destroyed parent window )
            pTopWindowChild->doLazyDelete();

            pTopWindowChild = pNextTopChild;
        }
    }
}

// ----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > UnoWrapper::CreateAccessible( Menu* pMenu, sal_Bool bIsMenuBar )
{
    return maAccessibleFactoryAccess.getFactory().createAccessible( pMenu, bIsMenuBar );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
