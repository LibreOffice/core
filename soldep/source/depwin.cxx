
/*************************************************************************
 *
 *  $RCSfile: depwin.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obo $ $Date: 2004-02-26 14:48:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    maNewConEnd( 0, 0 ),
    mpCapturer( NULL )
{
    if ( !pParent->IsChildNotify() )
        pParent->EnableChildNotify( TRUE );
//  if ( !pParent->IsAllResizeEnabled())
//      pParent->EnableAllResize( TRUE );
    SetUpdateMode( TRUE );
    SetPosSizePixel( Point(0,0), Size( 2000, 2000 ));

    mpPopup = new PopupMenu();
    mpPopup->InsertItem( DEPPOPUP_NEW, String::CreateFromAscii("New object") );
    mpPopup->InsertItem( DEPPOPUP_ZOOMIN, String::CreateFromAscii("Zoom in") );
    mpPopup->InsertItem( DEPPOPUP_ZOOMOUT, String::CreateFromAscii("Zoom out") );
    mpPopup->InsertSeparator();
    mpPopup->InsertItem( DEPPOPUP_CLEAR, String::CreateFromAscii("Clear") );
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
            mpDepperDontuseme->AddConnector( mpNewConWin, pWin );
        }
        mpNewConWin = 0L;
        mbStartNewCon = FALSE;
    }

}

void    DepWin::Paint( const Rectangle& rRect )
{
    ULONG i = 0;

    if ( ! mpCapturer )
    {
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
    else
    {
        Connector* pCon;

        Point aPoint( 0, 0 );
        DrawBitmap( aPoint, *pWinCopy );
        while( pCon = mpCapturer->GetConnector( i ))
        {
            pCon->Paint( aEmptyRect );
            i++;
        }
    }
}

/*void  DepWin::Resize()
{
} */

void DepWin::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsRight() )
    {
//#ifdef DEBUG
        mpPopup->Execute( this, rMEvt.GetPosPixel());
//#endif
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


void DepWin::SetCapturer( ObjectWin* pWin )
{
    mpCapturer = pWin;
    if ( pWin )
        pWinCopy = new Bitmap( GetBitmap( Point( 0, 0 ), GetParent()->GetSizePixel()));
    else
    {
        Invalidate();
        delete pWinCopy;
        pWinCopy = NULL;
    }
}

void DepWin::Command( const CommandEvent& rEvent)
{
    mpDepperDontuseme->GetGraphWin()->Command( rEvent );
}

