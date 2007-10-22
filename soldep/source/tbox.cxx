/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tbox.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 14:44:42 $
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
#define private public

#include <tools/ref.hxx>
#include <vcl/msgbox.hxx>
#include <soldep/soldep.hxx>
#include <soldep/tbox.hxx>
#include <soldep/soldlg.hrc>



SoldepToolBox::SoldepToolBox( Window* pParent, const ResId& aId, BOOL bDAble ) :
                ToolBox( pParent, aId ),
                bDockable( bDAble ),
                bCloseMode( FALSE ),
                bBoxIsVisible( TRUE ),
                bPinable( TRUE ),
                aPinedBitmap( DtSodResId( TID_SMALL_PIN_PINED )),     // BMP_SMALL_PIN_PINED
                aUnpinedBitmap( DtSodResId( TID_SMALL_PIN_UNPINED ))   // BMP_SMALL_PIN_UNPINED
{
    bPinable = TRUE;

    SetAlign( WINDOWALIGN_TOP );
    HideItem(TID_SOLDEP_BACK);    //Inital hide this item, activated in project view.
    HideItem(TID_SOLDEP_SAVE);    // Hide this item now, not used yet
    HideItem(TID_SOLDEP_OPEN);    // Hide this item now, not used yet

    //SetMouseDownHdl(LINK(this,SoldepToolBox,MouseDownHdl));
    //SetFloatingLines( nFloatingLines );
    //SetFloatingMode( bFloatingMode );
    BOOL bFloatingMode = FALSE;
    if ( bFloatingMode )
    {
        //Point aPos( sToolBoxInfo.GetToken( 4, ',' ).ToInt64(), sToolBoxInfo.GetToken( 5, ',' ).ToInt64());
        //SetPosPixel( aPos );
    }

    if ( bDockable ) InsertSeparator( 0 );  // Freiraum für Abreißdödel schaffen
                                            // (ein Separator an 1er Pos wird
                                            // nicht gezeichnet,
                                            // schafft aber Platz.)
    bBoxIsVisible = FALSE;
//  if ( sToolBoxInfo.GetToken( 3, ',' ) == "Visible" ) {
        for ( USHORT i = 0; i < GetItemCount() && !bBoxIsVisible; i++ )
        {
//          USHORT nItemId = GetItemId( i );

            switch ( GetItemType( i ))
            {
                case TOOLBOXITEM_SPACE:
                    break;
                case TOOLBOXITEM_SEPARATOR :
                    break;
                default:
                    bBoxIsVisible = TRUE;
                    break;
            }
        }
//  }

    if ( bBoxIsVisible ) {
        if (!bDockable)
            Show();
        else
            Hide();
    }
    else
        Hide();

    bOldFloatMode = IsFloatingMode();
}

/*************************************************************************
|* SoldepToolBox::~SoldepToolBox()
|************************************************************************/

SoldepToolBox::~SoldepToolBox()
{
}

/*************************************************************************
|* SoldepToolBox::Paint()
|************************************************************************/

void SoldepToolBox::Paint( const Rectangle& rRect )
{
        SetOutStyle(  TOOLBOX_STYLE_FLAT );

    // Wenn wir eine Dockable ToolBox haben, ...

    if ( bDockable )
    {
        USHORT nItemId = GetItemId( 0 );

        // ... zeichnen wir den wunderbaren Abreiád”del ber das erste Item (Seperator)

        if ( !IsFloatingMode() && ( mnCurLine == 1 )) {
            BOOL bIsCloseable = (( mnWinStyle & WB_CLOSEABLE ) != 0 );

            ShowItem( nItemId );
            Rectangle aRect = GetItemRect( nItemId );

            ToolBox::Paint( rRect );

            if ( bIsCloseable )
            {
                // Paint small cross button left/top of toolbar
                SetLineColor( Color( COL_WHITE ));

                DrawLine( Point( aRect.Left(), aRect.Top()), Point( aRect.Left()+6, aRect.Top()));
                DrawLine( Point( aRect.Left(), aRect.Top()), Point( aRect.Left(), aRect.Top()+6));

                SetLineColor( Color( COL_GRAY ));

                DrawLine( Point( aRect.Left(), aRect.Top()+6), Point( aRect.Left()+6, aRect.Top()+6));
                DrawLine( Point( aRect.Left()+6, aRect.Top()), Point( aRect.Left()+6, aRect.Top()+6));

                SetLineColor( Color( COL_BLACK ));

                DrawLine( Point( aRect.Left()+2, aRect.Top()+2), Point( aRect.Left()+4, aRect.Top()+4));
                DrawLine( Point( aRect.Left()+2, aRect.Top()+4), Point( aRect.Left()+4, aRect.Top()+2));
            }
            else if ( bPinable )
            {
                if ( GetPin())
                    DrawBitmap( Point( aRect.Left(), aRect.Top()), aPinedBitmap );
                else
                    DrawBitmap( Point( aRect.Left(), aRect.Top()), aUnpinedBitmap );
            }

            SetLineColor( Color( COL_WHITE ));

            if ( IsHorizontal())
            {
                if ( bIsCloseable || bPinable ) aRect = Rectangle( Point( aRect.Left(), aRect.Top()+8), Point( aRect.Right(), aRect.Bottom()));

                DrawLine( Point(aRect.Left(), aRect.Top()), Point(aRect.Left(), aRect.Bottom()));
                DrawLine( Point(aRect.Left(), aRect.Top()), Point(aRect.Left()+2, aRect.Top()));

                SetLineColor( Color( COL_WHITE ));

                DrawLine( Point(aRect.Left()+4, aRect.Top()), Point(aRect.Left()+4, aRect.Bottom()));
                DrawLine( Point(aRect.Left()+4, aRect.Top()), Point(aRect.Left()+6, aRect.Top()));

                SetLineColor( Color( COL_GRAY ));//GRAY

                DrawLine( Point(aRect.Left()+2, aRect.Top()), Point(aRect.Left()+2, aRect.Bottom()));
                DrawLine( Point(aRect.Left(), aRect.Bottom()), Point(aRect.Left()+2, aRect.Bottom()));

                SetLineColor( Color( COL_BLACK ));

                DrawLine( Point(aRect.Left()+6, aRect.Top()), Point(aRect.Left()+6, aRect.Bottom()));
                DrawLine( Point(aRect.Left()+4, aRect.Bottom()), Point(aRect.Left()+6, aRect.Bottom()));

            }
            else
            {
                if ( bIsCloseable || bPinable ) aRect = Rectangle( Point( aRect.Left()+8, aRect.Top()), Point( aRect.Right(), aRect.Bottom()));

                DrawLine( Point( aRect.Left(), aRect.Top()), Point( aRect.Left(), aRect.Top()+2));
                DrawLine( Point( aRect.Left(), aRect.Top()), Point( aRect.Right(), aRect.Top()));

                DrawLine( Point( aRect.Left(), aRect.Top()+4), Point( aRect.Left(), aRect.Top()+6));
                DrawLine( Point( aRect.Left(), aRect.Top()+4), Point( aRect.Right(), aRect.Top()+4));

                SetLineColor( Color( COL_GRAY ));

                DrawLine( Point( aRect.Left(), aRect.Top()+2), Point( aRect.Right(),aRect.Top()+2));
                DrawLine( Point( aRect.Right(), aRect.Top()), Point( aRect.Right(), aRect.Top()+2));

                DrawLine( Point( aRect.Left(), aRect.Top()+6), Point( aRect.Right(),aRect.Top()+6));
                DrawLine( Point( aRect.Right(), aRect.Top()+4), Point( aRect.Right(), aRect.Top()+6));

            }
        }
        else
        {
            // Sind wir im FloatingMode, dann wollen wir keinen Abreißdödel haben
            // und hiden somit das erste Item.

            //Hiden löscht leider das erste Object der nächsten Zeile, daher nicht
            //löschen
            HideItem( nItemId );
            ToolBox::Paint( rRect );
        }
    }
    else
    {
        ToolBox::Paint( rRect );
    }
}

/*************************************************************************
|* SoldepToolBox::MouseButtonDown()
|************************************************************************/

void SoldepToolBox::MouseButtonDown(const MouseEvent& rEvent)
{
    // Sind wir im DockingMode, ...

    if ( !IsFloatingMode() && ((( mnWinStyle & WB_CLOSEABLE ) != 0 ) || bPinable ))
    {
        // ... dann testen wir, ob am Abreiád”del der Close-Button gedrckt wurde ...

        Rectangle aRect = GetItemRect( GetItemId( 0 ));
        aRect = Rectangle( aRect.TopLeft(), Point( aRect.Left()+6, aRect.Top()+6 ));
        if ( rEvent.IsLeft() && aRect.IsInside( rEvent.GetPosPixel()))
        {
            if ( bPinable ) {
                TogglePin();
                Invalidate();
            }
            else {
                bCloseMode = TRUE;

                // ... und zeichnen ggf. den Button gedrckt.

                SetLineColor( Color( COL_WHITE ));

                DrawLine( Point( aRect.Left(), aRect.Bottom()), Point( aRect.Right(), aRect.Bottom()));
                DrawLine( Point( aRect.Right(), aRect.Bottom()), Point( aRect.Right(), aRect.Top()));

                SetLineColor( Color( COL_GRAY ));

                DrawLine( Point( aRect.Left(), aRect.Top()), Point( aRect.Right(), aRect.Top()));
                DrawLine( Point( aRect.Left(), aRect.Top()), Point( aRect.Left(), aRect.Bottom()));
            }
        }
        else
        {
            ToolBox::MouseButtonDown( rEvent );
        }
    }
    else
    {
        ToolBox::MouseButtonDown( rEvent );
    }

    aMouseDownHdl.Call(this);
}

/*************************************************************************
|* SoldepToolBox::MouseButtonUp()
|************************************************************************/

void SoldepToolBox::MouseButtonUp(const MouseEvent& rEvent)
{
    // Wenn der Close-Button am Abreiád”del zuvor gedrckt wurde, ...

    if ( bCloseMode )
    {
        bCloseMode = FALSE;

        // ... so zeichen wir diesen wieder normal ...

        Rectangle aRect = GetItemRect( GetItemId( 0 ));
        aRect = Rectangle( aRect.TopLeft(), Point( aRect.Left()+6, aRect.Top()+6 ));

        SetLineColor( Color( COL_WHITE ));

        DrawLine( Point( aRect.Left(), aRect.Top()), Point( aRect.Right(), aRect.Top()));
        DrawLine( Point( aRect.Left(), aRect.Top()), Point( aRect.Left(), aRect.Bottom()));

        SetLineColor( Color( COL_GRAY ));

        DrawLine( Point( aRect.Left(), aRect.Bottom()), Point( aRect.Right(), aRect.Bottom()));
        DrawLine( Point( aRect.Right(), aRect.Bottom()), Point( aRect.Right(), aRect.Top()));

        if ( rEvent.IsLeft() && aRect.IsInside( rEvent.GetPosPixel()))
        {
            // ... und schlieáen die Box durch Aufruf der virtuellen Methode
            // CloseDockingMode().

            CloseDockingMode();
        }
        else
        {
            ToolBox::MouseButtonUp( rEvent );
        }
    }
    else
    {
        ToolBox::MouseButtonUp( rEvent );
    }
}

/*************************************************************************
|* SoldepToolBox::MouseMove()
|************************************************************************/

void SoldepToolBox::MouseMove(const MouseEvent& rEvent)
{
    // Wenn der Close-Button am Abreiád”del zuvor gedrckt wurde, ...

    if ( bCloseMode )
    {
        Rectangle aRect = GetItemRect( GetItemId( 0 ));
        aRect = Rectangle( aRect.TopLeft(), Point( aRect.Left()+6, aRect.Top()+6 ));

        // ... der Mouse-Zeiger jedoch den Close-Button verl„át, ...

        if ( !aRect.IsInside( rEvent.GetPosPixel()))
        {
            SetLineColor( Color( COL_WHITE ));

            // ... zeichnen wir diesen halt wieder normal.

            DrawLine( Point( aRect.Left(), aRect.Top()), Point( aRect.Right(), aRect.Top()));
            DrawLine( Point( aRect.Left(), aRect.Top()), Point( aRect.Left(), aRect.Bottom()));

            SetLineColor( Color( COL_GRAY ));

                DrawLine( Point( aRect.Left(), aRect.Bottom()), Point( aRect.Right(), aRect.Bottom()));
            DrawLine( Point( aRect.Right(), aRect.Bottom()), Point( aRect.Right(), aRect.Top()));

            bCloseMode = FALSE;
        }
        else
        {
            ToolBox::MouseMove( rEvent );
        }
    }
    else
    {
        ToolBox::MouseMove( rEvent );
    }
}

/*****************************************************************************/
void SoldepToolBox::ToggleFloatingMode()
/*****************************************************************************/
{
    ToolBox::ToggleFloatingMode();
    CallEventListeners( VCLEVENT_USER_TBOX_RESIZE_APP, this);
}

/*****************************************************************************/
void SoldepToolBox::EndDocking( const Rectangle& rRect, BOOL bFloatMode )
/*****************************************************************************/
{
    ToolBox::EndDocking( rRect, bFloatMode );
    if ( aResizeHdl.IsSet())
        CallEventListeners( VCLEVENT_USER_TBOX_RESIZE_APP, this);
    else
        {
           GetParent()->Resize();
        }
};

/*****************************************************************************/
BOOL SoldepToolBox::Close()
/*****************************************************************************/
{
    if (!ToolBox::Close()) return FALSE;

    return TRUE;
}

/*****************************************************************************/
void SoldepToolBox::Move()
/*****************************************************************************/
{
}

/*************************************************************************
|* SoldepToolBox::CloseDockingMode()
|*
|* virtuelle Methode zur Reaktion auf den Close-Button im DockinMode
|************************************************************************/

void SoldepToolBox::CloseDockingMode()
{
    Hide();
    bBoxIsVisible = FALSE;
    CallEventListeners( VCLEVENT_USER_TBOX_RESIZE_APP, this);
}

/*************************************************************************
|* SoldepToolBox::Command()
|************************************************************************/

void SoldepToolBox::Command( const CommandEvent& rCEvt)
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        if( !IsFloatingMode() && bDockable
            && ( GetItemRect(GetItemId( 0 )).IsInside(rCEvt.GetMousePosPixel())))
        {
            // Hier das Context-Menue fuer Abreißdödel dynamisch erzeugen...

            PopupMenu aPopupMenu;

            String sText( String::CreateFromAscii( "Floating mode" ));
            aPopupMenu.InsertItem( 5, sText );
            aPopupMenu.InsertSeparator();

            sText = String::CreateFromAscii( "Top" );
            aPopupMenu.InsertItem( 1, sText );
            sText = String::CreateFromAscii( "Bottom" );
            aPopupMenu.InsertItem( 2, sText );
            sText = String::CreateFromAscii( "Left" );
            aPopupMenu.InsertItem( 3, sText );
            sText = String::CreateFromAscii( "Right" );
            aPopupMenu.InsertItem( 4, sText );

            switch( GetAlign())
            {
                case WINDOWALIGN_TOP : aPopupMenu.EnableItem( 1, FALSE ); break;
                case WINDOWALIGN_BOTTOM : aPopupMenu.EnableItem( 2, FALSE ); break;
                case WINDOWALIGN_LEFT : aPopupMenu.EnableItem( 3, FALSE ); break;
                case WINDOWALIGN_RIGHT : aPopupMenu.EnableItem( 4, FALSE ); break;
            }

            if (( mnWinStyle & WB_CLOSEABLE ) != 0 )
            {
                aPopupMenu.InsertSeparator();
                sText = String::CreateFromAscii( "Hide" );
                aPopupMenu.InsertItem( 6, sText );
            }
            else if ( bPinable ) {
                aPopupMenu.InsertSeparator();
                sText = String::CreateFromAscii( "Pin" );
                aPopupMenu.InsertItem( 6, sText );
                aPopupMenu.CheckItem( 6, GetPin());
            }

            // ... und rufen.

            aPopupMenu.SetSelectHdl ( LINK ( this, SoldepToolBox, MenuSelectHdl ));
            aPopupMenu.Execute( this, rCEvt.GetMousePosPixel());
        }
        else if ( !GetItemId(rCEvt.GetMousePosPixel()))
        {
            CallContextMenu( this, rCEvt.GetMousePosPixel());
        }
    }
    else ToolBox::Command( rCEvt );
}

/*************************************************************************
|* SoldepToolBox::GetContextMenu()
|************************************************************************/

PopupMenu *SoldepToolBox::GetContextMenu()
{
    InitContextMenu();
    return &aMenu;
}

/*************************************************************************
|* SoldepToolBox::InitContextMenu()
|************************************************************************/

void SoldepToolBox::InitContextMenu()
{
    USHORT nStart = 0;
    if ( bDockable )
        nStart = 1;

    aMenu.Clear();

    for ( USHORT i = nStart; i < GetItemCount(); i++ )
    {
        USHORT nItemId = GetItemId( i );

        switch ( GetItemType( i ))
        {
            case TOOLBOXITEM_SPACE:
                break;
            case TOOLBOXITEM_SEPARATOR :
                if (!(( i < GetItemCount() - 1 ) && ( GetItemType( i + 1 ) == TOOLBOXITEM_DONTKNOW )))
                    aMenu.InsertSeparator();
                break;
            default:
                aMenu.InsertItem( nItemId, GetItemText( nItemId ), GetItemImage( nItemId ));
                if (IsItemChecked( nItemId )) aMenu.CheckItem( nItemId, TRUE );
                if (!IsItemEnabled( nItemId )) aMenu.EnableItem( nItemId, FALSE );
                if (!IsItemVisible( nItemId )) aMenu.HideItem(nItemId);
                break;
        }
    }
}

/*************************************************************************
|* SoldepToolBox::CallContextMenu()
|************************************************************************/

void SoldepToolBox::CallContextMenu( Window *pWin, Point aPos )
{
    if ( aMenu.IsInExecute())
        return;

    InitContextMenu();

    // ... und rufen.

    aMenu.SetSelectHdl ( LINK ( this, SoldepToolBox, MenuSelectHdl ));
    aMenu.Execute( pWin, aPos );
}

void SoldepToolBox::SetPosSizePixel( const Point& rNewPos,
                                         const Size& rNewSize )
{
//  if(rNewPos != GetPosPixel() || rNewSize != GetSizePixel())
//  {
        ToolBox::SetPosSizePixel(rNewPos,rNewSize);
//      Resize();
//  }
}

void SoldepToolBox::SetDockingRects( const Rectangle& rOutRect,
                                         const Rectangle& rInRect )
{
    if(rOutRect != aOutRect || rInRect != aInRect)
    {
        ToolBox::SetDockingRects(rOutRect,rInRect);
        Invalidate();

        aOutRect = rOutRect;
        aInRect  = rInRect;
    }
}

/*****************************************************************************/
void SoldepToolBox::StartDocking()
/*****************************************************************************/
{
    ToolBox::StartDocking();
    CallEventListeners( VCLEVENT_USER_TBOX_RESIZE_APP, this);
};

BOOL SoldepToolBox::GetPin()

{
    return bPin;
}

void SoldepToolBox::TogglePin()
{
     bPin = !bPin;
}

void SoldepToolBox::SetPin( BOOL bP )
{
    bPin = bP;
}

/*************************************************************************
|* SoldepToolBox::MenuSelectHdl()
|*
|* Handler, der beim Execute der Context-Mens gerufen wird
|************************************************************************/

IMPL_LINK ( SoldepToolBox, MenuSelectHdl, Menu*, pMenu )
{
    // Wenn die CurItemId <= 6 ist, dann wurde das Abreiád”del-Context-Men ausgefhrt ...
    ULONG itemid;
    itemid = pMenu->GetCurItemId();

    if ( pMenu == &aMenu )
    {
        // ... Andernfalls wurde das Default-Context-Men ausgefhrt ...
        USHORT nTmpId = mnCurItemId;
        mnCurItemId = pMenu->GetCurItemId();

        // ... und der gesetzte Handler (in soldep.cxx) wird gerufen.
        maSelectHdl.Call( this );
        mnCurItemId = nTmpId;
    }
    return 0;
}