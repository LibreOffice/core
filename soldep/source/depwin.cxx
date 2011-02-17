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

#include <tools/debug.hxx>

#include <soldep/depwin.hxx>
#include <soldep/depper.hxx>
#include <soldep/connctr.hxx>
#include <soldep/objwin.hxx>

Bitmap* pWinCopy;

DepWin::DepWin( Window* pParent, WinBits nWinStyle ) :
    Window( pParent, nWinStyle ),
    mbStartNewCon( sal_False ),
    maNewConStart( 0, 0 ),
    maNewConEnd( 0, 0 )
//    mpSelectedProject( NULL ) unbekannt
//  mpCapturer( NULL )
{
    if ( !pParent->IsChildNotify() )
        pParent->EnableChildNotify( sal_True );
//  if ( !pParent->IsAllResizeEnabled())
//      pParent->EnableAllResize( sal_True );
    SetUpdateMode( sal_True );
    SetPosSizePixel( Point(0,0), Size( 2000, 2000 )); //Size of the scrollable Window
    mpPopup = new PopupMenu();
}

DepWin::~DepWin()
{
    Hide();
    while( ConList.Count() > 0 )
    {
        delete ConList.GetObject( 0 );
    }
//  if ( mpPopup )
///     delete mpPopup;
}

void DepWin::AddConnector( Connector* pNewCon )
{
    ConList.Insert( pNewCon );
}

void DepWin::RemoveConnector( Connector* pOldCon )
{
    ConList.Remove( pOldCon );
}

void DepWin::NewConnector( ObjectWin* pWin )
{
    if ( !mbStartNewCon )
    {
        mpNewConWin = pWin;
        mbStartNewCon = sal_True;
        maNewConStart = pWin->GetFixPoint(Point(0,0));
    }
    else
    {
        Invalidate( Rectangle( maNewConStart, maNewConEnd ));
        if ( pWin != mpNewConWin )
        {
//          Connector* pConctr;
//          pConctr = new Connector( this, WB_NOBORDER );
//          pConctr->Initialize( mpNewConWin, pWin );

// AddConnector has been moved to soldep
//          mpDepperDontuseme->AddConnector( mpNewConWin, pWin );
        }
        mpNewConWin = 0L;
        mbStartNewCon = sal_False;
    }

}

void DepWin::Paint( const Rectangle& rRect )
{
    sal_uIntPtr i = 0;
    sal_uIntPtr nListCount = ConList.Count();

    for ( i=0 ; i < nListCount ; i++ )
    {
        ConList.GetObject( i )->Paint( aEmptyRect );
    }
    if ( mbStartNewCon )
    {
        DrawLine( maNewConStart, maNewConEnd );
    }
}

void DepWin::DrawOutput( OutputDevice* pDevice, const Point& rOffset  )
{
    sal_uIntPtr i = 0;
    sal_uIntPtr nListCount = ConList.Count();

    for ( i=0 ; i < nListCount ; i++ )
    {
        ConList.GetObject( i )->DrawOutput( pDevice, rOffset );
    }
    if ( mbStartNewCon )
    {
        pDevice->DrawLine( maNewConStart, maNewConEnd );
    }
}

void DepWin::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsRight() )
    {
        mpPopup->Execute( this, rMEvt.GetPosPixel());
    }
}

void DepWin::MouseMove( const MouseEvent& rMEvt )
{
    if ( mbStartNewCon )
    {
        Invalidate( Rectangle( maNewConStart, maNewConEnd ));
        maNewConEnd = PixelToLogic(rMEvt.GetPosPixel());
        maNewConStart = mpNewConWin->GetFixPoint( maNewConEnd );
    }
}


ConnectorList*  DepWin::GetConnectorList()
{
    return &ConList;
}

void DepWin::SetPopupHdl( void* pHdl )
{
    mpPopup->SetSelectHdl( LINK( pHdl, Depper, PopupSelected ));
}

void DepWin::Command( const CommandEvent& rEvent)
{
    //mpDepperDontuseme->GetGraphWin()->Command( rEvent );
    GetParent()->Command( rEvent );
}
