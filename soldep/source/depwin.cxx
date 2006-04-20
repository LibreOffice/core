/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: depwin.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-04-20 15:15:01 $
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

#include <tools/debug.hxx>

#include "depwin.hxx"
#include "depper.hxx"
#include "connctr.hxx"
#include "objwin.hxx"

Bitmap* pWinCopy;

DepWin::DepWin( Window* pParent, WinBits nWinStyle ) :
    Window( pParent, nWinStyle ),
    mbStartNewCon( FALSE ),
    maNewConStart( 0, 0 ),
    maNewConEnd( 0, 0 )
//    mpSelectedProject( NULL ) unbekannt
//  mpCapturer( NULL )
{
    if ( !pParent->IsChildNotify() )
        pParent->EnableChildNotify( TRUE );
//  if ( !pParent->IsAllResizeEnabled())
//      pParent->EnableAllResize( TRUE );
    SetUpdateMode( TRUE );
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
        mbStartNewCon = TRUE;
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
        mbStartNewCon = FALSE;
    }

}

void DepWin::Paint( const Rectangle& rRect )
{
    ULONG i = 0;
    ULONG nListCount = ConList.Count();

    for ( i=0 ; i < nListCount ; i++ )
    {
        ConList.GetObject( i )->Paint( aEmptyRect );
    }
    if ( mbStartNewCon )
    {
        DrawLine( maNewConStart, maNewConEnd );
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
