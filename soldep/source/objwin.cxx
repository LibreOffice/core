/*************************************************************************
 *
 *  $RCSfile: objwin.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-01 14:18:34 $
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


#include <tools/list.hxx>
#include <tools/debug.hxx>
//src680 #include <vcl/system.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
//src680 #include <vcl/gdiobj.hxx>

#include "objwin.hxx"
#include "depwin.hxx"
#include "depper.hxx"
#include "prjdep.hxx"
#include "connctr.hxx"

//src680
static Wallpaper* pDefaultWallpaper = NULL;
static Color aDefaultColor = 0L;

UINT32 aColorMap[] = {
    COL_TRANSPARENT,
    COL_GREEN,
    COL_RED,
    COL_MAGENTA,
    COL_BLUE,
    COL_LIGHTGREEN,
    COL_LIGHTRED,
    COL_LIGHTMAGENTA,
    COL_BLUE,
    COL_BLUE,
    COL_BLUE,
    COL_BLUE,
    COL_BLUE,
    COL_BLUE,
    COL_BLUE,
    COL_BLUE
};


//
// class ObjectWin
//

/*****************************************************************************/
ObjectWin::ObjectWin( Window* pParent, WinBits nWinStyle )
/*****************************************************************************/
                : Window( pParent, nWinStyle ),
                mnObjectId( 0 ),
                sBodyText( "" ),
                sTipText( "" ),
                mnRootDist( 0 ),
                mnHeadDist( 0 ),
                mbVisited( FALSE ),
                mbFixed( FALSE ),
                mnMarkMode( 0 ),
                nViewMask( 0 ),
                bVisible( FALSE ),
                bMenuExecute( FALSE )
{
    SetBackground( Wallpaper( Color( COL_WHITE )));

    aTipTimer.SetTimeout( 500 );
    aTipTimer.SetTimeoutHdl(
        LINK( this, ObjectWin, TipHdl ));

//src680    SetFont( System::_GetStandardFont( _STDFONT_SWISS ));
    SetFont( Font( GetFont() ) );
    EnableClipSiblings();
    SetZOrder( NULL, WINDOW_ZORDER_FIRST );
    mpPopup = new PopupMenu();
//  mpPopup->InsertItem( OBJWIN_EDIT_TEXT, String::CreateFromAscii( "Details" ));
    mpPopup->InsertItem( OBJWIN_ADD_CONNECTOR, String::CreateFromAscii( "New connection" ));
    mpPopup->InsertItem( OBJWIN_REMOVE_WIN, String::CreateFromAscii( "Remove object" ));
    mpPopup->InsertItem( OBJWIN_VIEW_CONTENT, String::CreateFromAscii( "View content" ));
//  mpPopup->InsertSeparator();
    mpPopup->SetSelectHdl( LINK( this, ObjectWin, PopupSelected ));
    mpPopup->SetDeactivateHdl( LINK( this, ObjectWin, PopupDeactivated ));
    mnPopupStaticItems = mpPopup->GetItemCount();

/*src680    if ( ! pDefaultBrush )
    {
        pDefaultBrush = new Brush( GetBackgroundBrush() );
        aDefaultColor = GetTextColor();
    }
*/
    Hide();
}

/*****************************************************************************/
ObjectWin::~ObjectWin()
/*****************************************************************************/
{
    while ( Connections.Count() > 0 )
    {
        delete Connections.GetObject( 0 );
    }
}

/*****************************************************************************/
void ObjectWin::SetViewMask( ULONG nMask )
/*****************************************************************************/
{
    nViewMask = nMask;
    if ( nViewMask & mpDepperDontuseme->GetViewMask()) {
        bVisible = TRUE;
        Show();
    }
    else {
        Hide();
        bVisible = FALSE;
    }
    for ( ULONG i = 0; i < Connections.Count(); i++ )
        Connections.GetObject( i )->UpdateVisibility();
}

/*****************************************************************************/
void ObjectWin::SetBodyText( const ByteString& rNewString )
/*****************************************************************************/
{
    sBodyText = rNewString;
}

/*****************************************************************************/
ByteString& ObjectWin::GetBodyText()
/*****************************************************************************/
{
    return sBodyText;
}

/*****************************************************************************/
void ObjectWin::SetTipText( const ByteString& rNewString )
/*****************************************************************************/
{
    sTipText = rNewString;
}

/*****************************************************************************/
ByteString& ObjectWin::GetTipText()
/*****************************************************************************/
{
    return sTipText;
}

/*****************************************************************************/
Point ObjectWin::GetFixPoint( const Point& rRefPoint, BOOL bUseRealPos )
/*****************************************************************************/
{
    Point aLocalPoint;
    if ( bUseRealPos )
        aLocalPoint = GetPosPixel();
    else
        aLocalPoint = GetCalcPosPixel();

    Size aLocalSize = GetSizePixel();
    Point aRetPoint;

    USHORT nRefX = aLocalPoint.X() + aLocalSize.Width() / 2 ;
    USHORT nRefY = aLocalPoint.Y() + aLocalSize.Height() / 2 ;

    if ( nRefX < 0 ) nRefX = 0;
    if ( nRefY < 0 ) nRefY = 0;

    if ( rRefPoint.X() > nRefX )
    {
        if ( rRefPoint.Y() > nRefY )
        {
            if  ( Abs( rRefPoint.X() - nRefX ) > Abs( rRefPoint.Y() - nRefY ))
            {
                aRetPoint.X() = aLocalPoint.X() + aLocalSize.Width();
                aRetPoint.Y() = nRefY;
            }
            else
            {
                aRetPoint.X() = nRefX;
                aRetPoint.Y() = aLocalPoint.Y() + aLocalSize.Height();
            }
        }
        else
        {
            if  ( Abs( rRefPoint.X() - nRefX ) > Abs( rRefPoint.Y() - nRefY ))
            {
                aRetPoint.X() = aLocalPoint.X() + aLocalSize.Width();
                aRetPoint.Y() = nRefY;
            }
            else
            {
                aRetPoint.X() = nRefX;
                aRetPoint.Y() = aLocalPoint.Y();
            }
        }
    }
    else
    {
        if ( rRefPoint.Y() > nRefY )
        {
            if  ( Abs( rRefPoint.X() - nRefX ) > Abs( rRefPoint.Y() - nRefY ))
            {
                aRetPoint.X() = aLocalPoint.X();
                aRetPoint.Y() = nRefY;
            }
            else
            {
                aRetPoint.X() = nRefX;
                aRetPoint.Y() = aLocalPoint.Y() + aLocalSize.Height();
            }
        }
        else
        {
            if  ( Abs( rRefPoint.X() - nRefX ) > Abs( rRefPoint.Y() - nRefY ))
            {
                aRetPoint.X() = aLocalPoint.X();
                aRetPoint.Y() = nRefY;
            }
            else
            {
                aRetPoint.X() = nRefX;
                aRetPoint.Y() = aLocalPoint.Y();
            }
        }
    }

    return PixelToLogic(aRetPoint);

}

/*****************************************************************************/
void ObjectWin::AddConnector( Connector* pNewCon )
/*****************************************************************************/
{
    Connections.Insert( pNewCon );
}

/*****************************************************************************/
BOOL ObjectWin::ConnectionExistsInAnyDirection( ObjectWin *pWin )
/*****************************************************************************/
{
    for ( ULONG i = 0; i < Connections.Count(); i++ )
        if ( Connections.GetObject( i )->GetOtherWin( this ) == pWin )
            return TRUE;

    return FALSE;
}

/*****************************************************************************/
void ObjectWin::RemoveConnector( Connector* pOldCon )
/*****************************************************************************/
{
    Connections.Remove( pOldCon );
}

/*****************************************************************************/
Connector* ObjectWin::GetConnector( ULONG nIndex )
/*****************************************************************************/
{
    ULONG nConCount = Connections.Count();

    if ( nIndex < nConCount )
        return Connections.GetObject( nIndex );
    return NULL;
}

/*****************************************************************************/
Connector* ObjectWin::GetConnector( ULONG nStartId, ULONG nEndId )
/*****************************************************************************/
{
    if ( mnObjectId != nStartId )
        return NULL;

    USHORT i;
    Connector* pCon;
    ULONG nConCount = Connections.Count();

    for ( i = 0; i < nConCount; i++ )
    {
        pCon = Connections.GetObject( i );
        if ( pCon->GetOtherWin( this )->GetId() == nEndId )
            return pCon;
    }
    return NULL;
}

/*****************************************************************************/
void ObjectWin::SetMarkMode( ULONG nMarkMode )
/*****************************************************************************/
{
//src680    Brush aBrush;
    Wallpaper aWallpaper;

    if ( nMarkMode == MARKMODE_DEFAULT )
    {
//src680
        if ( pDefaultWallpaper )
        {
            aWallpaper = GetBackground();
            aWallpaper.SetColor( pDefaultWallpaper->GetColor() );
            SetBackground( aWallpaper );
            SetTextColor( aDefaultColor );
        }
    }
    else
    {
        mnMarkMode |= nMarkMode;
//src680
        aWallpaper = GetBackground();
//src680
        aWallpaper.SetColor( aColorMap[ mnMarkMode ] );
//src680
        SetBackground( aWallpaper );
        SetTextColor( COL_WHITE );
    }
    Invalidate();
}

/*****************************************************************************/
void ObjectWin::UnsetMarkMode( ULONG nMarkMode )
/*****************************************************************************/
{
//src680
    Wallpaper aWallpaper;

    ULONG nOldMode = mnMarkMode;
    mnMarkMode &= ( !nMarkMode );

    if ( nOldMode != mnMarkMode ) {
        if ( mnMarkMode == MARKMODE_DEFAULT )
        {
//src680
            if ( pDefaultWallpaper )
            {
                aWallpaper = GetBackground();
                aWallpaper.SetColor( pDefaultWallpaper->GetColor() );
                SetBackground( aWallpaper );
                SetTextColor( aDefaultColor );
            }
        }
        else
        {
//src680
            aWallpaper = GetBackground();
            aWallpaper.SetColor( aColorMap[ mnMarkMode ] );
            SetBackground( aWallpaper );
            SetTextColor( COL_WHITE );
        }
        Invalidate();
    }
}

/*****************************************************************************/
void ObjectWin::MarkNeeded( BOOL bReset )
/*****************************************************************************/
{
    Connector* pCon;
    ObjectWin* pWin;

    ULONG nConCount = Connections.Count();
    ULONG i;

    for ( i = 0; i < nConCount; i++ )
    {
        pCon = Connections.GetObject( i );
        if ( pCon && !pCon->IsStart( this) )
        {
            pWin = pCon->GetOtherWin( this );
            if ( pWin )
            {
                if ( bReset )
                    pWin->UnsetMarkMode( MARKMODE_NEEDED );
                else
                    pWin->SetMarkMode( MARKMODE_NEEDED );
                pWin->MarkNeeded( bReset );
            }
        }
    }
}

/*****************************************************************************/
void ObjectWin::MarkDepending( BOOL bReset )
/*****************************************************************************/
{
    Connector* pCon;
    ObjectWin* pWin;

    ULONG nConCount = Connections.Count();
    ULONG i;

    for ( i = 0; i < nConCount; i++ )
    {
        pCon = Connections.GetObject( i );
        if ( pCon && pCon->IsStart( this) )
        {
            pWin = pCon->GetOtherWin( this );
            if ( pWin )
            {
                if ( bReset )
                    pWin->UnsetMarkMode( MARKMODE_DEPENDING );
                else
                    pWin->SetMarkMode( MARKMODE_DEPENDING );
                pWin->MarkDepending( bReset );
            }
        }
    }
}

/*****************************************************************************/
void ObjectWin::Paint( const Rectangle& rRect )
/*****************************************************************************/
{

    Size  aWinSize  = PixelToLogic( GetOutputSizePixel() );
    Size  aTextSize;
    aTextSize.Width() = GetTextWidth( String( sBodyText, RTL_TEXTENCODING_UTF8 ));
    aTextSize.Height() = GetTextHeight();
    Point aPos( aWinSize.Width() / 2  - aTextSize.Width() / 2,
                aWinSize.Height() / 2 - aTextSize.Height() / 2 );

    DrawText( aPos , String( sBodyText, RTL_TEXTENCODING_UTF8 ));
}

/*****************************************************************************/
void ObjectWin::MouseButtonDown( const MouseEvent& rMEvt )
/*****************************************************************************/
{
    SetZOrder( NULL, WINDOW_ZORDER_FIRST );
    GrabFocus();

    // workaround fuer vcl-bug
//  GetWindow( WINDOW_REALPARENT)->Invalidate();

    aMouseOffset = rMEvt.GetPosPixel();
    if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.IsMod1() )    //IsMod2
        {
            CaptureMouse();
            SetMarkMode( MARKMODE_ACTIVATED );
            MarkNeeded();
            MarkDepending();
        }
        else {
            MarkNeeded( TRUE );
            MarkDepending( TRUE );
        }
        if( rMEvt.GetClicks() == 2 )
            DoubleClick();
        else if ( !rMEvt.IsShift() && !((DepWin*)GetParent())->IsStartNewCon())
            CaptureMouse();
    }
}

/*****************************************************************************/
void ObjectWin::MouseButtonUp( const MouseEvent& rMEvt )
/*****************************************************************************/
{
    if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.IsShift() || ((DepWin*)GetParent())->IsStartNewCon())
            ((DepWin*)GetParent())->NewConnector( this );
        else
        {
            ReleaseMouse();
//          ((DepWin*)GetParent())->SetCapturer( NULL );
            // workaround fuer vcl-bug

// try without
//          GetParent()->Invalidate();
        }
        if ( GetMarkMode() &  MARKMODE_ACTIVATED )
        {
//          UnsetMarkMode( MARKMODE_ACTIVATED );
//          MarkNeeded( TRUE );
//          MarkDepending( TRUE );
            SetMarkMode( MARKMODE_SELECTED );
        }
    }
    else if ( rMEvt.IsRight() )
    {
        USHORT i;

        while ( mnPopupStaticItems < mpPopup->GetItemCount() )
        {
            mpPopup->RemoveItem( mnPopupStaticItems );
        }

        if ( Connections.Count()) {
            mpPopup->InsertSeparator();

            for( i = 0; i < Connections.Count() ; i++ )
            {
                mpPopup->InsertItem( mnPopupStaticItems + i, String( ((Connections.GetObject( i ))->GetOtherWin( this ))->GetBodyText(), RTL_TEXTENCODING_UTF8 ));
            }
        }
        bMenuExecute = TRUE;
        mpPopup->Execute( GetParent(), rMEvt.GetPosPixel() + GetPosPixel());
    }
}

/*****************************************************************************/
void ObjectWin::MouseMove( const MouseEvent& rMEvt )
/*****************************************************************************/
{
    if ( IsMouseCaptured() )
    {
        USHORT i;

        Point aNewWinPos( GetPosPixel() + rMEvt.GetPosPixel() - aMouseOffset );

        aNewWinPos.X() = Max( 0L, aNewWinPos.X());
        aNewWinPos.Y() = Max( 0L, aNewWinPos.Y());
        SetPosPixel( aNewWinPos );

        for ( i=0; i < Connections.Count();i++)
        {
            Connections.GetObject( i )->UpdatePosition( this );
        }
    }
    else
    {
        if ( rMEvt.IsLeaveWindow() )
            aTipTimer.Stop();
        else
            aTipTimer.Start();

        MouseEvent aNewMEvt( rMEvt.GetPosPixel() + GetPosPixel());

        GetParent()->MouseMove( aNewMEvt );
    }
}

/*****************************************************************************/
void ObjectWin::DoubleClick()
/*****************************************************************************/
{
    mpDepperDontuseme->ViewContent( sBodyText );
}

/*****************************************************************************/
USHORT ObjectWin::Save( SvFileStream& rOutFile )
/*****************************************************************************/
{
    return 0;
}

/*****************************************************************************/
USHORT ObjectWin::Load( SvFileStream& rInFile )
/*****************************************************************************/
{
    return 0;
}

/*****************************************************************************/
void ObjectWin::SetId( ULONG nId )
/*****************************************************************************/
{
    mnObjectId = nId;
}

/*****************************************************************************/
ULONG ObjectWin::GetId()
/*****************************************************************************/
{
    return mnObjectId;
}

/*****************************************************************************/
void ObjectWin::UpdateConnectors()
/*****************************************************************************/
{
    USHORT i;

    for ( i = 0; i < Connections.Count(); i++ )
    {
        Connections.GetObject( i )->UpdatePosition( this );
    }
}

IMPL_LINK( ObjectWin, PopupSelected, PopupMenu*, mpPopup )
{
    USHORT nItemId = mpPopup->GetCurItemId();

    switch( nItemId )
    {
        case OBJWIN_EDIT_TEXT :
            DBG_ASSERT( FALSE,"edit");
                break;
        case OBJWIN_REMOVE_WIN :
//          DBG_ASSERT( FALSE,"remove");
//          DBG_ASSERT( mpDepperDontuseme,"remove");
            mpDepperDontuseme->RemoveObject(( USHORT ) GetId());
                break;
        case OBJWIN_ADD_CONNECTOR :
//          DBG_ASSERT( FALSE,"add con");
            ((DepWin*)GetParent())->NewConnector( this );
                break;
        case OBJWIN_VIEW_CONTENT :
//          DBG_ASSERT( FALSE,"view cnt");
            mpDepperDontuseme->ViewContent( sBodyText );
                break;
        default :
//          DBG_ASSERT( FALSE, String (nItemId) );
            Connector* pCon = Connections.GetObject( nItemId - mnPopupStaticItems );
//          delete pCon;
            mpDepperDontuseme->RemoveConnector( pCon->GetStartId(), pCon->GetEndId());
                break;
    }
    return 0;
}

/*****************************************************************************/
IMPL_LINK( ObjectWin, TipHdl, void *, EMTY_ARG )
/*****************************************************************************/
{
    aTipTimer.Stop();

    if ( sTipText.Len()) {
        Point aPos( GetpApp()->GetAppWindow()->GetPointerPosPixel());
        Help::ShowBalloon( GetpApp()->GetAppWindow(),
            Point( aPos.X(), aPos.Y()),
            String( sTipText, RTL_TEXTENCODING_UTF8 ));
    }
    return 0;
}

/*****************************************************************************/
void ObjectWin::GetFocus()
/*****************************************************************************/
{
    SetMarkMode( MARKMODE_SELECTED );
}

/*****************************************************************************/
void ObjectWin::LoseFocus()
/*****************************************************************************/
{
    if ( !bMenuExecute ) {
        UnsetMarkMode( MARKMODE_SELECTED );
        UnsetMarkMode( MARKMODE_ACTIVATED );
        MarkNeeded( TRUE );
        MarkDepending( TRUE );
    }
    else
        bMenuExecute = FALSE;
}

/*****************************************************************************/
IMPL_LINK( ObjectWin, PopupDeactivated, PopupMenu*, mpPopup )
/*****************************************************************************/
{
    bMenuExecute = FALSE;

    if ( !HasFocus()) {
        UnsetMarkMode( MARKMODE_SELECTED );
        UnsetMarkMode( MARKMODE_ACTIVATED );
           MarkNeeded( TRUE );
        MarkDepending( TRUE );
    }

    return 0;
}

/*****************************************************************************/
void ObjectWin::Command( const CommandEvent& rEvent)
/*****************************************************************************/
{
    mpDepperDontuseme->GetGraphWin()->Command( rEvent );
}

