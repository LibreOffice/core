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

#include <tools/list.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/wall.hxx>

#include <soldep/objwin.hxx>
#include <soldep/depwin.hxx>
//#include "depapp.hxx"
#include <soldep/depper.hxx>
//#include "prjdep.hxx"
#include <soldep/connctr.hxx>
#include <stdio.h>

static Color aDefaultColor = 0L;
static Wallpaper* pDefaultWallpaper = 0L;

// Initialize static class member
sal_Bool ObjectWin::msbHideMode = sal_False;
sal_uIntPtr ObjectWin::msnGlobalViewMask = 0;


sal_uInt32 aColorMap[] = {
    RGB_COLORDATA( 0xFF, 0xFF, 0x80 ),             //MARKMODE_DEFAULT    0
    COL_GREEN,              //MARKMODE_DEPENDING  1
    COL_RED,                //MARKMODE_NEEDED     2
    COL_MAGENTA,            //1+2
    COL_GRAY,               //MARKMODE_ACTIVATED  4
    COL_LIGHTGREEN,         //1+4
    COL_LIGHTRED,           //2+4
    COL_LIGHTMAGENTA,       //1+2+4
    COL_BLUE,               //MARKMODE_SELECTED   8
    COL_LIGHTGRAY,          //1+8
    COL_CYAN,               //2+8
    COL_LIGHTCYAN,          //1+2+8
    COL_LIGHTBLUE,          //4+8
    COL_BROWN,              //1+4+8
    COL_BLACK,              //2+4+8
    COL_BLUE                //1+2+4+8
};


//
// class ObjectWin
//

/*****************************************************************************/
ObjectWin::ObjectWin( Window* pParent, WinBits nWinStyle )
/*****************************************************************************/
                : Window( pParent, nWinStyle ),
                msBodyText( "" ),
                msTipText( "" ),
                mnObjectId( 0 ),
                mnMarkMode( 0 ),
                mnViewMask( 0 ),
                mbVisible( sal_False ),
                mbMenuExecute( sal_False ),
                mbVisited( sal_False ),
                mnRootDist( 0 ),
                mnHeadDist( 0 ),
                mbFixed( sal_False )
{
    SetBackground( Wallpaper( aColorMap[0] ));

    aTipTimer.SetTimeout( 500 );
    aTipTimer.SetTimeoutHdl(
    LINK( this, ObjectWin, TipHdl ));

    SetFont( Font( GetFont() ) );
    Font aFont( GetFont() );
    Size aSize = aFont.GetSize();
    aFont.SetSize( aSize );
    SetFont( aFont );

    EnableClipSiblings();
    SetZOrder( NULL, WINDOW_ZORDER_FIRST );
    mpPopup = new PopupMenu();
    mpPopup->InsertItem( OBJWIN_EDIT_TEXT, String::CreateFromAscii( "Details" ));
    mpPopup->InsertItem( OBJWIN_ADD_CONNECTOR, String::CreateFromAscii( "New connection" ));
    mpPopup->InsertItem( OBJWIN_REMOVE_WIN, String::CreateFromAscii( "Remove object" ));
    mpPopup->InsertItem( OBJWIN_VIEW_CONTENT, String::CreateFromAscii( "View content" ));
//  mpPopup->InsertSeparator();
    mpPopup->SetSelectHdl( LINK( this, ObjectWin, PopupSelected ));
    mpPopup->SetDeactivateHdl( LINK( this, ObjectWin, PopupDeactivated ));
    mnPopupStaticItems = mpPopup->GetItemCount();

    if ( ! pDefaultWallpaper )
    {
        pDefaultWallpaper = new Wallpaper( GetBackground() );
        aDefaultColor = GetTextColor();
    }
    Hide();
}

/*****************************************************************************/
ObjectWin::~ObjectWin()
/*****************************************************************************/
{
    while ( mConnections.Count() > 0 )
    {
        delete mConnections.GetObject( 0 );
    }
}

void ObjectWin::SetHideMode(sal_Bool bHide)
{
    msbHideMode = bHide;
    mConnections.GetObject(0)->SetHideMode(msbHideMode);
}

sal_Bool ObjectWin::ToggleHideMode()
{
    msbHideMode = !msbHideMode;
    mConnections.GetObject(0)->SetHideMode(msbHideMode);
    return msbHideMode;
}

/*****************************************************************************/
void ObjectWin::SetViewMask( sal_uIntPtr nMask )
/*****************************************************************************/
{
    mnViewMask = nMask;
    // Compares
    if ( mnViewMask & msnGlobalViewMask) {
        mbVisible = sal_True;
        Show();
    }
    else {
        Hide();
        mbVisible = sal_False;
    }
    for ( sal_uIntPtr i = 0; i < mConnections.Count(); i++ )
        mConnections.GetObject( i )->UpdateVisibility();
}

/*****************************************************************************/
void ObjectWin::SetBodyText( const ByteString& rNewString )
/*****************************************************************************/
{
    msBodyText = rNewString;
}

/*****************************************************************************/
ByteString& ObjectWin::GetBodyText()
/*****************************************************************************/
{
    return msBodyText;
}

/*****************************************************************************/
void ObjectWin::SetTipText( const ByteString& rNewString )
/*****************************************************************************/
{
    msTipText = rNewString;
}

/*****************************************************************************/
ByteString& ObjectWin::GetTipText()
/*****************************************************************************/
{
    return msTipText;
}

/*****************************************************************************/
Point ObjectWin::GetFixPoint( const Point& rRefPoint, sal_Bool bUseRealPos )
/*****************************************************************************/
{
    Point aLocalPoint;
    if ( bUseRealPos )
        aLocalPoint = GetPosPixel();
    else
        aLocalPoint = GetCalcPosPixel();

    Size aLocalSize = GetSizePixel();
    Point aRetPoint;

    sal_uInt16 nRefX = aLocalPoint.X() + aLocalSize.Width() / 2 ;
    sal_uInt16 nRefY = aLocalPoint.Y() + aLocalSize.Height() / 2 ;

    // always false...
    //if ( nRefX < 0 ) nRefX = 0;
    //if ( nRefY < 0 ) nRefY = 0;

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
    mConnections.Insert( pNewCon );
}

/*****************************************************************************/
sal_Bool ObjectWin::ConnectionExistsInAnyDirection( ObjectWin *pWin )
/*****************************************************************************/
{
    for ( sal_uIntPtr i = 0; i < mConnections.Count(); i++ )
        if ( mConnections.GetObject( i )->GetOtherWin( this ) == pWin )
            return sal_True;

    return sal_False;
}

/*****************************************************************************/
void ObjectWin::RemoveConnector( Connector* pOldCon )
/*****************************************************************************/
{
    mConnections.Remove( pOldCon );
}

/*****************************************************************************/
Connector* ObjectWin::GetConnector( sal_uIntPtr nIndex )
/*****************************************************************************/
{
    sal_uIntPtr nConCount = mConnections.Count();

    if ( nIndex < nConCount )
        return mConnections.GetObject( nIndex );
    return NULL;
}

/*****************************************************************************/
Connector* ObjectWin::GetConnector( sal_uIntPtr nStartId, sal_uIntPtr nEndId )
/*****************************************************************************/
{
    if ( mnObjectId != nStartId )
        return NULL;

    sal_uInt16 i;
    Connector* pCon;
    sal_uIntPtr nConCount = mConnections.Count();

    for ( i = 0; i < nConCount; i++ )
    {
        pCon = mConnections.GetObject( i );
        if ( pCon->GetOtherWin( this )->GetId() == nEndId )
            return pCon;
    }
    return NULL;
}

void ObjectWin::SetAllConnectorsUnvisible()
{
    Connector* pCon;
    sal_uIntPtr nConCount = mConnections.Count();
    for ( sal_uIntPtr i = 0; i < nConCount; i++ )
    {
        pCon = mConnections.GetObject( i );
        if (pCon) pCon->SetVisibility( sal_False );
    }
}

/*****************************************************************************/
void ObjectWin::SetMarkMode( sal_uIntPtr nMarkMode )
/*****************************************************************************/
{
    //Wallpaper aWallpaper;

    if ( nMarkMode == MARKMODE_DEFAULT )
    {
        if ( pDefaultWallpaper )
        {
            maObjWallpaper = GetBackground();
            maObjWallpaper.SetColor( pDefaultWallpaper->GetColor() );
            SetBackground( maObjWallpaper );
            SetTextColor( aDefaultColor );
        }
    }
    else
    {
        mnMarkMode |= nMarkMode;
        maObjWallpaper = GetBackground();
        maObjWallpaper.SetColor( aColorMap[ mnMarkMode ] );
        SetBackground( maObjWallpaper );
        SetTextColor( COL_WHITE );
    }

    Invalidate();
}

/*****************************************************************************/
void ObjectWin::UnsetMarkMode( sal_uIntPtr nMarkMode )
/*****************************************************************************/
{
    //Wallpaper aWallpaper;

    sal_uIntPtr nOldMode = mnMarkMode;
    mnMarkMode &= ( !nMarkMode );

    if ( nOldMode != mnMarkMode ) {
        if ( mnMarkMode == MARKMODE_DEFAULT )
        {
            if ( pDefaultWallpaper )
            {
                maObjWallpaper = GetBackground();
                maObjWallpaper.SetColor( pDefaultWallpaper->GetColor() );
                SetBackground( maObjWallpaper );
                SetTextColor( aDefaultColor );
            }
        }
        else
        {
            maObjWallpaper = GetBackground();
            maObjWallpaper.SetColor( aColorMap[ mnMarkMode ] ); //mnMarkMode
            SetBackground( maObjWallpaper );
            SetTextColor( COL_WHITE );
        }
        Invalidate();
    }
}

/*****************************************************************************/
void ObjectWin::MarkNeeded( sal_Bool bReset )
/*****************************************************************************/
{
    Connector* pCon;
    ObjectWin* pWin;

    sal_uIntPtr nConCount = mConnections.Count();
    sal_uIntPtr i;

    for ( i = 0; i < nConCount; i++ )
    {
        pCon = mConnections.GetObject( i );
        if ( pCon && !pCon->IsStart( this))
        {
            pWin = pCon->GetOtherWin( this );
            if ( pWin )
            {
                if ( bReset )
                    pWin->UnsetMarkMode( MARKMODE_NEEDED );
                else
                    pWin->SetMarkMode( MARKMODE_NEEDED );
                pWin->MarkNeeded( bReset );                    // recursive call
            }
        }
    }
}

/*****************************************************************************/
void ObjectWin::MarkDepending( sal_Bool bReset )
/*****************************************************************************/
{
    //if ( !bReset )
    //  return;

    Connector* pCon;
    ObjectWin* pWin;

    sal_uIntPtr nConCount = mConnections.Count();
    sal_uIntPtr i;

    for ( i = 0; i < nConCount; i++ )
    {
        pCon = mConnections.GetObject( i );
        if ( pCon && pCon->IsStart( this) )
        {
            pWin = pCon->GetOtherWin( this );
            if ( pWin )
            {
                if ( bReset )
                    pWin->UnsetMarkMode( MARKMODE_DEPENDING );
                else
                    pWin->SetMarkMode( MARKMODE_DEPENDING );
                pWin->MarkDepending( bReset );                  // recursive call
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
    ByteString sbt = msBodyText;                         //debug
    //sbt += " ";                                         //debug
    //sbt += ByteString::CreateFromInt32(mnMarkMode);     //debug
    aTextSize.Width() = GetTextWidth( String( msBodyText, RTL_TEXTENCODING_UTF8 ));
    aTextSize.Height() = GetTextHeight();
    Point aPos( aWinSize.Width() / 2  - aTextSize.Width() / 2,
                aWinSize.Height() / 2 - aTextSize.Height() / 2 );

    //DrawText( aPos , String( sBodyText, RTL_TEXTENCODING_UTF8 ));
    if (msBodyText =="null") //don't paint this "window"
    {
        Hide();
        Invalidate();
    } else
        DrawText( aPos , String( sbt, RTL_TEXTENCODING_UTF8 )); //debug
}

void ObjectWin::DrawOutput( OutputDevice* pDevice, const Point& rOffset )
/*****************************************************************************/
{
    Size aWinSize  = PixelToLogic( GetSizePixel() );
    Size aTextSize;
    ByteString sbt = msBodyText;
    aTextSize.Width() = GetTextWidth( String( msBodyText, RTL_TEXTENCODING_UTF8 ));
    aTextSize.Height() = GetTextHeight();
    Point aPos = GetPosPixel();
    Point aTextPos( aWinSize.Width() / 2  - aTextSize.Width() / 2,
                aWinSize.Height() / 2 - aTextSize.Height() / 2 );
    aTextPos += aPos;
    aPos = pDevice->PixelToLogic( aPos ) - rOffset;
    aTextPos = pDevice->PixelToLogic( aTextPos ) - rOffset;
    if ( msBodyText !="null" )
    {
        pDevice->SetFillColor( GetBackground().GetColor() );
        pDevice->DrawRect( Rectangle( aPos, pDevice->PixelToLogic( GetSizePixel() ) ) );
        Font aFont( GetFont() );
        Size aSize = aFont.GetSize();
        aSize = pDevice->PixelToLogic( aSize );
        aFont.SetSize( aSize );
        pDevice->SetFont( aFont );
        pDevice->SetTextColor( GetTextColor() );
        pDevice->DrawText( aTextPos, String( sbt, RTL_TEXTENCODING_UTF8 ) );
    }
}

/*****************************************************************************/
void ObjectWin::MouseButtonDown( const MouseEvent& rMEvt )
/*****************************************************************************/
{
    //Notify Soldep to clear ObjectList
    SetZOrder( NULL, WINDOW_ZORDER_FIRST );
    GrabFocus();

    // workaround fuer vcl-bug
//  GetWindow( WINDOW_REALPARENT)->Invalidate();
//    MyApp *pApp = (MyApp*)GetpApp();
//    SolDep *pSoldep = pApp->GetSolDep();

    maMouseOffset = rMEvt.GetPosPixel();
    if ( rMEvt.IsLeft() )
    {

        if ( rMEvt.IsMod2() )               // alt + mouse click left
        {
            CallEventListeners( VCLEVENT_USER_MOUSEBUTTON_DOWN_ALT, this );
        }
        else {
            CallEventListeners( VCLEVENT_USER_MOUSEBUTTON_DOWN, this );
        }
        if( rMEvt.GetClicks() == 2 )
            CallEventListeners( VCLEVENT_USER_MOUSEBUTTON_DOWN_DBLCLICK, this );
        else if ( !rMEvt.IsShift() && !((DepWin*)GetParent())->IsStartNewCon())
        {
            //((DepWin*)GetParent())->SaveSelectedObjWin(&this);
            CaptureMouse();
        }
    }
}

/*****************************************************************************/
void ObjectWin::MouseButtonUp( const MouseEvent& rMEvt )
/*****************************************************************************/
{
    fprintf(stdout,"ObjectWin::MouseButtonUp\n");
    if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.IsShift() || ((DepWin*)GetParent())->IsStartNewCon())
            CallEventListeners( VCLEVENT_USER_MOUSEBUTTON_UP_SHFT, this );
//          ((DepWin*)GetParent())->NewConnector( this );
        else
        {
            CallEventListeners( VCLEVENT_USER_MOUSEBUTTON_UP, this );
            if ( IsMouseCaptured() ) ReleaseMouse();
        }
    }
    else if ( rMEvt.IsRight() )
    {
        sal_uInt16 i;

        while ( mnPopupStaticItems < mpPopup->GetItemCount() )
        {
            mpPopup->RemoveItem( mnPopupStaticItems );
        }

        if ( mConnections.Count()) {
            mpPopup->InsertSeparator();

            for( i = 0; i < mConnections.Count() ; i++ )
            {
                mpPopup->InsertItem( mnPopupStaticItems + i + 1, String( ((mConnections.GetObject( i ))->GetOtherWin( this ))->GetBodyText(), RTL_TEXTENCODING_UTF8 ));
            }
        }
        mbMenuExecute = sal_True;
        mpPopup->Execute( GetParent(), rMEvt.GetPosPixel() + GetPosPixel());
    }
}

/*****************************************************************************/
void ObjectWin::MouseMove( const MouseEvent& rMEvt )
/*****************************************************************************/
{
    if ( IsMouseCaptured() )
    {
        sal_uInt16 i;

        Point aNewWinPos( GetPosPixel() + rMEvt.GetPosPixel() - maMouseOffset );

        aNewWinPos.X() = Max( 0L, aNewWinPos.X());
        aNewWinPos.Y() = Max( 0L, aNewWinPos.Y());
        SetPosPixel( aNewWinPos );
        //int t = mConnections.Count();

        for ( i=0; i < mConnections.Count();i++)
        {
            mConnections.GetObject( i )->UpdatePosition( this );
        }
    }
    else // !IsMouseCaptured()
    {
        if ( rMEvt.IsLeaveWindow() )
            aTipTimer.Stop();
        else
            aTipTimer.Start();

        MouseEvent aNewMEvt( rMEvt.GetPosPixel() + GetPosPixel());

        GetParent()->MouseMove( aNewMEvt ); //call to DepWin::MouseMove
    }
}

/*****************************************************************************/
sal_uInt16 ObjectWin::Save( SvFileStream& rOutFile )
/*****************************************************************************/
{
    return 0;
}

/*****************************************************************************/
sal_uInt16 ObjectWin::Load( SvFileStream& rInFile )
/*****************************************************************************/
{
    return 0;
}

/*****************************************************************************/
void ObjectWin::SetId( sal_uIntPtr nId )
/*****************************************************************************/
{
    mnObjectId = nId;
}

/*****************************************************************************/
sal_uIntPtr ObjectWin::GetId()
/*****************************************************************************/
{
    return mnObjectId;
}

/*****************************************************************************/
void ObjectWin::UpdateConnectors()
/*****************************************************************************/
{
    sal_uInt16 i;

    for ( i = 0; i < mConnections.Count(); i++ )
    {
        mConnections.GetObject( i )->UpdatePosition( this );
    }
}

IMPL_LINK( ObjectWin, PopupSelected, PopupMenu*, mpPopup_l )
{
    sal_uInt16 nItemId = mpPopup_l->GetCurItemId();

    switch( nItemId )
    {
        case OBJWIN_EDIT_TEXT :
            DBG_ASSERT( sal_False,"edit");
                break;
        case OBJWIN_REMOVE_WIN :
//          DBG_ASSERT( FALSE,"remove");
//          DBG_ASSERT( mpDepperDontuseme,"remove");
            //mpDepperDontuseme->RemoveObject(mpDepperDontuseme->mpObjectList, ( sal_uInt16 ) GetId());
                break;
        case OBJWIN_ADD_CONNECTOR :
//          DBG_ASSERT( FALSE,"add con");
            ((DepWin*)GetParent())->NewConnector( this );
                break;
        case OBJWIN_VIEW_CONTENT :
//          DBG_ASSERT( FALSE,"view cnt");
//          mpDepperDontuseme->ViewContent( msBodyText );
//          TBD: CallEventListener
                break;
        default :
//          DBG_ASSERT( sal_False, String (nItemId) );
            Connector* pCon = mConnections.GetObject( nItemId - mnPopupStaticItems - 1);
            pCon = 0;
//          delete pCon;
//          mpDepperDontuseme->RemoveConnector( pCon->GetStartId(), pCon->GetEndId());
//          TBD: CallEventListener

                break;
    }
    return 0;
}

/*****************************************************************************/
IMPL_LINK( ObjectWin, TipHdl, void *, EMTY_ARG )
/*****************************************************************************/
{
    aTipTimer.Stop();

    if ( msTipText.Len()) {
        Point aPos( GetpApp()->GetAppWindow()->GetPointerPosPixel());
        Help::ShowBalloon( GetpApp()->GetAppWindow(),
            Point( aPos.X(), aPos.Y()),
            String( msTipText, RTL_TEXTENCODING_UTF8 ));
    }
    return 0;
}

/*****************************************************************************/
//void ObjectWin::GetFocus()
/*****************************************************************************/
//{
    //SetMarkMode( MARKMODE_SELECTED );
//}

/*****************************************************************************/
void ObjectWin::LoseFocus()
/*****************************************************************************/
{
    if ( !mbMenuExecute && !msbHideMode ) {
        UnsetMarkMode( MARKMODE_SELECTED );
        UnsetMarkMode( MARKMODE_ACTIVATED );
        MarkNeeded( sal_True );
        MarkDepending( sal_True );
    }
    else
        mbMenuExecute = sal_False;
}

/*****************************************************************************/
IMPL_LINK( ObjectWin, PopupDeactivated, PopupMenu*, mpPopup_l )
/*****************************************************************************/
{
    mbMenuExecute = sal_False;

    if ( !HasFocus()) {
        UnsetMarkMode( MARKMODE_SELECTED );
        UnsetMarkMode( MARKMODE_ACTIVATED );
           MarkNeeded( sal_True );
        MarkDepending( sal_True );
    }

    return 0;
}

/*****************************************************************************/
void ObjectWin::Command( const CommandEvent& rEvent)
/*****************************************************************************/
{
    fprintf(stdout, "ObjectWin::Command");
//  mpDepperDontuseme->GetGraphWin()->Command( rEvent );
//          TBD: CallEventListener

}

/*****************************************************************************/
/*****************************************************************************/

ObjectList::ObjectList() : ObjWinList()
{
}

/*****************************************************************************/
void ObjectList::ResetSelectedObject()
/*****************************************************************************/
{
//    return;

    sal_uIntPtr nCount_l = Count();
    ObjectWin* pObjectWin = NULL;
    for (sal_uIntPtr i=0; i < nCount_l; i++ )
    {
        pObjectWin = GetObject( i );
        pObjectWin->UnsetMarkMode( MARKMODE_SELECTED );
        pObjectWin->UnsetMarkMode( MARKMODE_NEEDED );
        pObjectWin->UnsetMarkMode( MARKMODE_DEPENDING );
        pObjectWin->SetActualWallpaper(*pDefaultWallpaper);
        pObjectWin->SetAllConnectorsUnvisible();
    }
    return;
}

/*****************************************************************************/
ObjectWin* ObjectList::GetPtrByName( const ByteString& rText )
/*****************************************************************************/
{
    sal_uIntPtr i = 0;
    sal_uIntPtr nCount_l = Count();
    ObjectWin* pObjectWin = NULL;
    while ( i < nCount_l )
    {
       pObjectWin = GetObject( i );
       ByteString sPrj = pObjectWin->GetBodyText();
       if (sPrj == rText) return pObjectWin;
       i++;
    }
    return 0;
}

ObjectList* ObjectList::FindTopLevelModules()
{
    ObjectList* pList = new ObjectList;
    for ( sal_uInt16 i=0; i<Count(); i++ )
    {
       ObjectWin* pObjectWin = GetObject( i );
       if ( pObjectWin->IsTop() )
           pList->Insert( pObjectWin );
    }

    return pList;
}

sal_Bool ObjectWin::IsTop()
{
    sal_uIntPtr nConCount = mConnections.Count();
    for ( sal_uIntPtr i = 0; i < nConCount; i++ )
    {
        Connector* pCon = mConnections.GetObject( i );
        if ( pCon && pCon->IsStart( this) )
            return sal_False;
    }

    return sal_True;
}
