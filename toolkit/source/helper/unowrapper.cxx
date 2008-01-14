/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unowrapper.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 12:58:53 $
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
#include "precompiled_toolkit.hxx"

#ifndef _COM_SUN_STAR_AWT_WINDOWEVENT_HPP_
#include <com/sun/star/awt/WindowEvent.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <toolkit/helper/unowrapper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/awt/vclxcontainer.hxx>
#include <toolkit/awt/vclxtopwindow.hxx>
#include <toolkit/awt/vclxgraphics.hxx>

#include "toolkit/dllapi.h"

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SYSWIN_HXX
#include <vcl/syswin.hxx>
#endif

#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif

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

        // case WINDOW_FIXEDLINE:
        // case WINDOW_FIXEDBITMAP:
        // case WINDOW_DATEBOX:
        // case WINDOW_GROUPBOX:
        // case WINDOW_LONGCURRENCYBOX:
        // case WINDOW_METRICBOX:
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

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> UnoWrapper::GetWindowInterface( Window* pWindow, BOOL bCreate )
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
        if( pWindow->GetWindowPeer() )
        {
            int i = 0;
            i++;
            //          DBG_ERROR( "UnoWrapper::SetWindowInterface: there already *is* a WindowInterface for this window!" );
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
    List* pLst = pOutDev->GetUnoGraphicsList();
    if ( pLst )
    {
        for ( sal_uInt32 n = 0; n < pLst->Count(); n++ )
        {
            VCLXGraphics* pGrf = (VCLXGraphics*)pLst->GetObject( n );
            pGrf->SetOutputDevice( NULL );
        }
    }

}

// MT: Wurde im Window-CTOR gerufen, damit Container-Listener
// vom Parent reagieren, aber hat sowieso nicht richtig funktioniert,
// weil im Window-CTOR das Interface noch nicht da ist!
// => Nur Listener rufen, wenn ueber das ::com::sun::star::awt::Toolkit erzeugt

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
    // ggf. existieren noch von ::com::sun::star::loader::Java erzeugte Childs, die sonst erst
    // im Garbage-Collector zerstoert werden...
    Window* pChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    while ( pChild )
    {
        Window* pNextChild = pChild->GetWindow( WINDOW_NEXT );

        Window* pClient = pChild->GetWindow( WINDOW_CLIENT );
        if ( pClient->GetWindowPeer() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComp( pClient->GetComponentInterface( FALSE ), ::com::sun::star::uno::UNO_QUERY );
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
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComp( pClient->GetComponentInterface( FALSE ), ::com::sun::star::uno::UNO_QUERY );
            xComp->dispose();
        }

        pOverlap = pNextOverlap;
    }

    Window* pParent = pWindow->GetParent();
    if ( pParent && pParent->GetWindowPeer() && pParent->GetWindowPeer()->GetContainerListeners().getLength() )
    {
        ::com::sun::star::awt::VclContainerEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)pParent->GetWindowPeer();
        aEvent.Child = (::cppu::OWeakObject*)pWindow->GetWindowPeer();
        pParent->GetWindowPeer()->GetContainerListeners().windowRemoved( aEvent );
    }

    if ( pWindow && pWindow->GetWindowPeer() )
    {
        pWindow->GetWindowPeer()->SetWindow( NULL );
        pWindow->SetWindowPeer( NULL, NULL );
    }

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
            OSL_ENSURE( pTopWindowChild->GetParent() == pWindow, "UnoWrapper::WindowDestroyed: inconsistency in the SystemWindow relationship!" );

            uno::Reference< lang::XComponent > xComp( pTopWindowChild->GetComponentInterface( FALSE ), uno::UNO_QUERY );
            pTopWindowChild = pTopWindowChild->GetWindow( WINDOW_NEXTTOPWINDOWSIBLING );
            if  ( xComp.is() )
                xComp->dispose();
        }
    }
}

// ----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > UnoWrapper::CreateAccessible( Menu* pMenu, sal_Bool bIsMenuBar )
{
    return maAccessibleFactoryAccess.getFactory().createAccessible( pMenu, bIsMenuBar );
}
