/*************************************************************************
 *
 *  $RCSfile: linectrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:27 $
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

// include ---------------------------------------------------------------

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif

#include "dialogs.hrc"
#include "helpid.hrc"

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST

#include "drawitem.hxx"
#include "xattr.hxx"
#include "xtable.hxx"
#include "linectrl.hxx"
#include "itemwin.hxx"
#include "dialmgr.hxx"

// Fuer Linienenden-Controller
#define MAX_LINES 12

// STATIC DATA -----------------------------------------------------------

#define RESIZE_VALUE_POPUP(value_set)   \
{                                                       \
    Size aSize = GetOutputSizePixel();                  \
    aSize.Width()  -= 4;                                \
    aSize.Height() -= 4;                                \
    (value_set).SetPosSizePixel( Point(2,2), aSize );   \
}

#define CALCSIZE_VALUE_POPUP(value_set,item_size) \
{                                                                   \
    Size aSize = (value_set).CalcWindowSizePixel( (item_size) );    \
    aSize.Width()  += 4;                                            \
    aSize.Height() += 4;                                            \
    SetOutputSizePixel( aSize );                                    \
}


SFX_IMPL_TOOLBOX_CONTROL( SvxLineStyleToolBoxControl, XLineStyleItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxLineWidthToolBoxControl, XLineWidthItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxLineColorToolBoxControl, XLineColorItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxLineEndToolBoxControl,   SfxBoolItem );

/*************************************************************************
|*
|* SvxLineStyleToolBoxControl
|*
\************************************************************************/

SvxLineStyleToolBoxControl::SvxLineStyleToolBoxControl( USHORT nId,
                                                        ToolBox& rTbx,
                                                        SfxBindings& rBind ) :
    SfxToolBoxControl( nId, rTbx, rBind ),
    aDashForwarder  ( SID_ATTR_LINE_DASH, *this ),
    pStyleItem      ( NULL ),
    pDashItem       ( NULL ),
    bUpdate         ( FALSE )
{
    StartListening( rBind, TRUE );
}

//========================================================================

SvxLineStyleToolBoxControl::~SvxLineStyleToolBoxControl()
{
    delete pStyleItem;
    delete pDashItem;
}

//========================================================================

void SvxLineStyleToolBoxControl::StateChanged (

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    SvxLineBox* pBox = (SvxLineBox*)GetToolBox().GetItemWindow( GetId() );
    DBG_ASSERT( pBox, "Window not found!" );

    if( eState == SFX_ITEM_DISABLED )
    {
        pBox->Disable();
        pBox->SetNoSelection();
    }
    else
    {
        pBox->Enable();

        if ( eState == SFX_ITEM_AVAILABLE )
        {
            if( nSID == SID_ATTR_LINE_STYLE )
            {
                delete pStyleItem;
                pStyleItem = (XLineStyleItem*)pState->Clone();
            }
            else if( nSID == SID_ATTR_LINE_DASH )
            {
                delete pDashItem;
                pDashItem = (XLineDashItem*)pState->Clone();
            }
            else
                DBG_ERROR( "Huch wer kommt nach drinnen rein?");

            bUpdate = TRUE;
        }
        else
            // kein oder uneindeutiger Status
            pBox->SetNoSelection();
    }
}

//========================================================================

void SvxLineStyleToolBoxControl::SFX_NOTIFY( SfxBroadcaster& rBC,
                                         const TypeId& rBCType,
                                         const SfxHint& rHint,
                                         const TypeId& rHintType )
{
    const SfxSimpleHint *pSimpleHint = PTR_CAST(SfxSimpleHint, &rHint);

    if ( pSimpleHint &&
        ( pSimpleHint->GetId() == SFX_HINT_UPDATEDONE ) &&
          bUpdate )
    {
        bUpdate = FALSE;

        SvxLineBox* pBox = (SvxLineBox*)GetToolBox().GetItemWindow( GetId() );
        DBG_ASSERT( pBox, "Window not found!" );

        // Da der Timer unerwartet zuschlagen kann, kann es vorkommen, dass
        // die LB noch nicht gefuellt ist. Ein ClearCache() am Control im
        // DelayHdl() blieb ohne Erfolg.
        if( pBox->GetEntryCount() == 0 )
        {
            SfxObjectShell* pSh = SfxObjectShell::Current();
            if ( pSh )
            {
                pBox->InsertEntry( SVX_RESSTR(RID_SVXSTR_INVISIBLE) );
                pBox->InsertEntry( SVX_RESSTR(RID_SVXSTR_SOLID) );
                const SvxDashListItem *pItem =
                    (const SvxDashListItem*)( pSh->GetItem( SID_DASH_LIST ) );
                if(pItem)
                    pBox->Fill( pItem->GetDashList() );
            }
        }

        XLineStyle eXLS;

        if ( pStyleItem )
            eXLS = ( XLineStyle )pStyleItem->GetValue();
        else
            eXLS = XLINE_NONE;

        switch( eXLS )
        {
            case XLINE_NONE:
                pBox->SelectEntryPos( 0 );
                break;

            case XLINE_SOLID:
                pBox->SelectEntryPos( 1 );
                break;

            case XLINE_DASH:
            {
                if( pDashItem )
                {
                    String aString( pDashItem->GetName() );
                    pBox->SelectEntry( aString );
                }
                else
                    pBox->SetNoSelection();
            }
            break;

            default:
                DBG_ERROR( "Nicht unterstuetzter Linientyp" );
                break;
        }
    }

    const SfxPoolItemHint *pPoolItemHint = PTR_CAST(SfxPoolItemHint, &rHint);

    if ( pPoolItemHint
         && ( pPoolItemHint->GetObject()->ISA( SvxDashListItem ) ) )
    {
        // Die Liste der Linienstile hat sich geaendert
        SvxLineBox* pBox = (SvxLineBox*)GetToolBox().GetItemWindow( GetId() );
        DBG_ASSERT( pBox, "Window not found!" );

        String aString( pBox->GetSelectEntry() );
        pBox->Clear();
        pBox->InsertEntry( SVX_RESSTR(RID_SVXSTR_INVISIBLE) );
        pBox->InsertEntry( SVX_RESSTR(RID_SVXSTR_SOLID) );
        pBox->Fill( ( (SvxDashListItem*) pPoolItemHint->GetObject() )->GetDashList() );
        pBox->SelectEntry( aString );
    }
}

//========================================================================

Window* SvxLineStyleToolBoxControl::CreateItemWindow( Window *pParent )
{
    return new SvxLineBox( pParent, GetBindings() );
}

/*************************************************************************
|*
|* SvxLineWidthToolBoxControl
|*
\************************************************************************/

SvxLineWidthToolBoxControl::SvxLineWidthToolBoxControl( USHORT nId,
                                                        ToolBox& rTbx,
                                                        SfxBindings& rBind ) :
    SfxToolBoxControl( nId, rTbx, rBind )
{
    SfxApplication* pSfxApp = SFX_APP();
    SfxModule* pModule = pSfxApp->GetActiveModule();

    if( pModule )
        StartListening( rBind, TRUE );
    else
        StartListening( *pSfxApp , TRUE );

}

//========================================================================

SvxLineWidthToolBoxControl::~SvxLineWidthToolBoxControl()
{
}

//========================================================================

void SvxLineWidthToolBoxControl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    SvxMetricField* pFld = (SvxMetricField*)
                           GetToolBox().GetItemWindow( GetId() );
    DBG_ASSERT( pFld, "Window not found" );

    if ( eState == SFX_ITEM_DISABLED )
    {
        pFld->Disable();
        pFld->SetText( String() );
    }
    else
    {
        pFld->Enable();

        if ( eState == SFX_ITEM_AVAILABLE )
        {
            DBG_ASSERT( pState->ISA(XLineWidthItem), "falscher ItemType" )

            // Core-Unit an MetricField uebergeben
            // Darf nicht in CreateItemWin() geschehen!
            SfxMapUnit eUnit = GetCoreMetric();
            pFld->SetCoreUnit( eUnit );

            pFld->Update( (const XLineWidthItem*)pState );
        }
        else
            pFld->Update( NULL );
    }
}

//========================================================================

void SvxLineWidthToolBoxControl::SFX_NOTIFY( SfxBroadcaster& rBC,
                                         const TypeId& rBCType,
                                         const SfxHint& rHint,
                                         const TypeId& rHintType )
{
    // SfxItemSetHint funktioniert nicht mehr und kann laut MI
    // auch nie funktioniert haben!
    //const SfxItemSetHint *pOptionsHint = PTR_CAST(SfxItemSetHint, &rHint);

    const SfxPoolItemHint *pOptionsHint = PTR_CAST(SfxPoolItemHint, &rHint);
    if( pOptionsHint
        && ( pOptionsHint->GetObject()->Which() == SID_ATTR_METRIC ) )
    {
        SvxMetricField* pFld = (SvxMetricField*)
                               GetToolBox().GetItemWindow( GetId() );
        DBG_ASSERT( pFld, "Window not found" );

        pFld->RefreshDlgUnit();
    }
}

//========================================================================

Window* SvxLineWidthToolBoxControl::CreateItemWindow( Window *pParent )
{
    return( new SvxMetricField( pParent, GetBindings() ) );
}

/*************************************************************************
|*
|* SvxLineColorToolBoxControl
|*
\************************************************************************/

SvxLineColorToolBoxControl::SvxLineColorToolBoxControl( USHORT nId,
                                                        ToolBox& rTbx,
                                                        SfxBindings& rBind ) :
    SfxToolBoxControl( nId, rTbx, rBind )
{
    StartListening( GetBindings() );
}

//========================================================================

SvxLineColorToolBoxControl::~SvxLineColorToolBoxControl()
{
}

//========================================================================

void SvxLineColorToolBoxControl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    SvxColorBox* pBox = (SvxColorBox*)GetToolBox().GetItemWindow( GetId() );
    DBG_ASSERT( pBox, "Window not found" );

    if ( eState == SFX_ITEM_DISABLED )
    {
        pBox->Disable();
        pBox->SetNoSelection();
    }
    else
    {
        pBox->Enable();

        if ( eState == SFX_ITEM_AVAILABLE )
        {
            DBG_ASSERT( pState->ISA(XLineColorItem), "falscher ItemTyoe" );
            pBox->Update( (const XLineColorItem*) pState );
        }
        else
            pBox->Update( NULL );
    }
}

//========================================================================

void SvxLineColorToolBoxControl::SFX_NOTIFY( SfxBroadcaster& rBC,
                                    const TypeId& rBCType,
                                    const SfxHint& rHint,
                                    const TypeId& rHintType )
{
    const SfxPoolItemHint *pPoolItemHint = PTR_CAST(SfxPoolItemHint, &rHint);

    if ( pPoolItemHint
         && ( pPoolItemHint->GetObject()->ISA( SvxColorTableItem ) ) )
    {
        SvxColorBox* pBox = (SvxColorBox*)GetToolBox().GetItemWindow( GetId() );

        DBG_ASSERT( pBox, "Window not found" );

        // Die Liste der Farben (ColorTable) hat sich geaendert:
        Color aTmpColor( pBox->GetSelectEntryColor() );
        pBox->Clear();
        pBox->Fill( ( (SvxColorTableItem*) pPoolItemHint->GetObject() )->GetColorTable() );
        pBox->SelectEntry( aTmpColor );
    }
}

//========================================================================

Window* SvxLineColorToolBoxControl::CreateItemWindow( Window *pParent )
{
    return new SvxColorBox( pParent, GetId(), GetBindings() );
}

/*************************************************************************
|*
|* SvxLineEndWindow
|*
\************************************************************************/

SvxLineEndWindow::SvxLineEndWindow( USHORT nId, const String& rWndTitle, SfxBindings& rBindings ) :

    SfxPopupWindow( nId, WinBits( WB_BORDER | WB_STDFLOATWIN | WB_SIZEABLE | WB_3DLOOK ), rBindings ),

    pLineEndList    ( NULL ),
    aLineEndSet     ( this, WinBits( WB_ITEMBORDER | WB_3DLOOK ) ),
    nCols           ( 2 ),
    nLines          ( 12 ),
    nLineEndWidth   ( 400 ),
    bPopupMode      ( TRUE )

{
    SfxObjectShell*     pDocSh  = SfxObjectShell::Current();
    const SfxPoolItem*  pItem   = NULL;

    SetHelpId( HID_POPUP_LINEEND );
    aLineEndSet.SetHelpId( HID_POPUP_LINEEND_CTRL );

    if ( pDocSh )
    {
        if( pItem = pDocSh->GetItem( SID_LINEEND_LIST ) )
            pLineEndList = ( (SvxLineEndListItem*) pItem )->GetLineEndList();

        if( pItem = pDocSh->GetItem( SID_ATTR_LINEEND_WIDTH_DEFAULT ) )
            nLineEndWidth = ( (SfxUInt16Item*) pItem )->GetValue();
    }
    DBG_ASSERT( pLineEndList, "LineEndList wurde nicht gefunden" );

    aLineEndSet.SetSelectHdl( LINK( this, SvxLineEndWindow, SelectHdl ) );
    aLineEndSet.SetColCount( nCols );

    // ValueSet mit Eintraegen der LineEndList fuellen
    FillValueSet();

    //ChangeHelpId( HID_POPUP_LINEENDSTYLE );
    SetText( rWndTitle );
    aLineEndSet.Show();
    StartListening( rBindings );
}

SfxPopupWindow* SvxLineEndWindow::Clone() const
{
    return new SvxLineEndWindow( GetId(), GetText(), (SfxBindings&)GetBindings() );
}

// -----------------------------------------------------------------------

SvxLineEndWindow::~SvxLineEndWindow()
{
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLineEndWindow, SelectHdl, void *, EMPTYARG )
{
    XLineEndItem*           pLineEndItem = NULL;
    XLineStartItem*         pLineStartItem = NULL;
    USHORT                  nId = aLineEndSet.GetSelectItemId();

    if( nId == 1 )
    {
        pLineStartItem  = new XLineStartItem();
    }
    else if( nId == 2 )
    {
        pLineEndItem    = new XLineEndItem();
    }
    else if( nId % 2 ) // LinienAnfang
    {
        XLineEndEntry* pEntry = pLineEndList->Get( ( nId - 1 ) / 2 - 1 );
        pLineStartItem  = new XLineStartItem( pEntry->GetName(), pEntry->GetLineEnd() );
    }
    else // LinienEnde
    {
        XLineEndEntry* pEntry = pLineEndList->Get( nId / 2 - 2 );
        pLineEndItem    = new XLineEndItem( pEntry->GetName(), pEntry->GetLineEnd() );
    }

    if ( IsInPopupMode() )
        EndPopupMode();

    SfxDispatcher* pDisp = GetBindings().GetDispatcher();
    DBG_ASSERT( pDisp, "invalid Dispatcher" );
    if ( pLineStartItem )
        pDisp->Execute( SID_ATTR_LINEEND_STYLE, SFX_CALLMODE_RECORD, pLineStartItem, 0L , 0L );
    else
        pDisp->Execute( SID_ATTR_LINEEND_STYLE, SFX_CALLMODE_RECORD, pLineEndItem, 0L , 0L );

    delete pLineEndItem;
    delete pLineStartItem;
    aLineEndSet.SetNoSelection();

    return 0;
}

// -----------------------------------------------------------------------

void SvxLineEndWindow::FillValueSet()
{
    if( pLineEndList )
    {
        XLineEndEntry*      pEntry  = NULL;
        Bitmap*             pBmp    = NULL;
        VirtualDevice       aVD;

        long nCount = pLineEndList->Count();

        // Erster Eintrag: kein LinienEnde
        // Temporaer wird ein Eintrag hinzugefuegt, um die UI-Bitmap zu erhalten
        XPolygon aNothing( 0 );
        pLineEndList->Insert( new XLineEndEntry( aNothing, SVX_RESSTR( RID_SVXSTR_NONE ) ) );
        pEntry = pLineEndList->Get( nCount );
        pBmp = pLineEndList->GetBitmap( nCount );
        DBG_ASSERT( pBmp, "UI-Bitmap wurde nicht erzeugt" );

        aBmpSize = pBmp->GetSizePixel();
        aVD.SetOutputSizePixel( aBmpSize, FALSE );
        aBmpSize.Width() = aBmpSize.Width() / 2;
        Point aPt0( 0, 0 );
        Point aPt1( aBmpSize.Width(), 0 );

        aVD.DrawBitmap( Point(), *pBmp );
        aLineEndSet.InsertItem( 1, aVD.GetBitmap( aPt0, aBmpSize ), pEntry->GetName() );
        aLineEndSet.InsertItem( 2, aVD.GetBitmap( aPt1, aBmpSize ), pEntry->GetName() );

        delete pLineEndList->Remove( nCount );

        for( long i = 0; i < nCount; i++ )
        {
            pEntry = pLineEndList->Get( i );
            DBG_ASSERT( pEntry, "Konnte auf LineEndEntry nicht zugreifen" );
            pBmp = pLineEndList->GetBitmap( i );
            DBG_ASSERT( pBmp, "UI-Bitmap wurde nicht erzeugt" );

            aVD.DrawBitmap( aPt0, *pBmp );
            aLineEndSet.InsertItem( (USHORT)((i+1L)*2L+1L), aVD.GetBitmap( aPt0, aBmpSize ), pEntry->GetName() );
            aLineEndSet.InsertItem( (USHORT)((i+2L)*2L),    aVD.GetBitmap( aPt1, aBmpSize ), pEntry->GetName() );
        }
        nLines = Min( (USHORT)(nCount + 1), (USHORT) MAX_LINES );
        aLineEndSet.SetLineCount( nLines );

        SetSize();
    }
}

// -----------------------------------------------------------------------

void SvxLineEndWindow::Resize()
{
    if ( !IsRollUp() )
    {
        aLineEndSet.SetColCount( nCols );
        aLineEndSet.SetLineCount( nLines );

        SetSize();

        Size aSize = GetOutputSizePixel();
        aSize.Width()  -= 4;
        aSize.Height() -= 4;
        aLineEndSet.SetPosSizePixel( Point( 2, 2 ), aSize );
    }
    //SfxPopupWindow::Resize();
}

// -----------------------------------------------------------------------

void __EXPORT SvxLineEndWindow::Resizing( Size& rNewSize )
{
    Size aBitmapSize = aBmpSize; // -> Member
    aBitmapSize.Width()  += 6; //
    aBitmapSize.Height() += 6; //

    Size aItemSize = aLineEndSet.CalcItemSizePixel( aBitmapSize );  // -> Member
    //Size aOldSize = GetOutputSizePixel(); // fuer Breite

    USHORT nItemCount = aLineEndSet.GetItemCount(); // -> Member

    // Spalten ermitteln
    long nItemW = aItemSize.Width();
    long nW = rNewSize.Width();
    nCols = (USHORT) Max( ( (ULONG)(( nW + nItemW ) / ( nItemW * 2 ) )),
                                            (ULONG) 1L );
    nCols *= 2;

    // Reihen ermitteln
    long nItemH = aItemSize.Height();
    long nH = rNewSize.Height();
    nLines = (USHORT) Max( ( ( nH + nItemH / 2 ) / nItemH ), 1L );

    USHORT nMaxCols  = nItemCount / nLines;
    if( nItemCount % nLines )
        nMaxCols++;
    if( nCols > nMaxCols )
        nCols = nMaxCols;
    nW = nItemW * nCols;

    // Keine ungerade Anzahl von Spalten
    if( nCols % 2 )
        nCols--;
    nCols = Max( nCols, (USHORT) 2 );

    USHORT nMaxLines  = nItemCount / nCols;
    if( nItemCount % nCols )
        nMaxLines++;
    if( nLines > nMaxLines )
        nLines = nMaxLines;
    nH = nItemH * nLines;

    rNewSize.Width() = nW;
    rNewSize.Height() = nH;
}
// -----------------------------------------------------------------------

void SvxLineEndWindow::StartSelection()
{
    aLineEndSet.StartSelection();
}

// -----------------------------------------------------------------------

BOOL SvxLineEndWindow::Close()
{
    return SfxPopupWindow::Close();
}

// -----------------------------------------------------------------------

void SvxLineEndWindow::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType )
{
    const SfxPoolItemHint *pPoolItemHint = PTR_CAST(SfxPoolItemHint, &rHint);

    if ( pPoolItemHint
         && ( pPoolItemHint->GetObject()->ISA( SvxLineEndListItem ) ) )
    {
        // Die Liste der LinienEnden (LineEndList) hat sich geaendert:

        pLineEndList = ( (SvxLineEndListItem*) pPoolItemHint->
                                        GetObject() )->GetLineEndList();
        DBG_ASSERT( pLineEndList, "LineEndList nicht gefunden" );

        aLineEndSet.Clear();
        FillValueSet();

        Size aSize = GetOutputSizePixel();
        Resizing( aSize );
        Resize();
    }
}

// -----------------------------------------------------------------------

void SvxLineEndWindow::PopupModeEnd()
{
    if ( IsVisible() )
    {
        bPopupMode = FALSE;
        SetSize();
    }
    SfxPopupWindow::PopupModeEnd();
}

// -----------------------------------------------------------------------

void SvxLineEndWindow::SetSize()
{
    if( !bPopupMode )
    //if( !IsInPopupMode() )
    {
        USHORT nItemCount = aLineEndSet.GetItemCount(); // -> Member
        USHORT nMaxLines  = nItemCount / nCols; // -> Member ?
        if( nItemCount % nCols )
            nMaxLines++;

        WinBits nBits = aLineEndSet.GetStyle();
        if ( nLines == nMaxLines )
            nBits &= ~WB_VSCROLL;
        else
            nBits |= WB_VSCROLL;
        aLineEndSet.SetStyle( nBits );
    }

    Size aSize( aBmpSize );
    aSize.Width()  += 6;
    aSize.Height() += 6;
    aSize = aLineEndSet.CalcWindowSizePixel( aSize );
    aSize.Width()  += 4;
    aSize.Height() += 4;
    SetOutputSizePixel( aSize );
    aSize.Height() = aBmpSize.Height();
    aSize.Height() += 14;
    //SetMinOutputSizePixel( aSize );
}

/*************************************************************************
|*
|* SvxLineEndToolBoxControl
|*
\************************************************************************/

SvxLineEndToolBoxControl::SvxLineEndToolBoxControl( USHORT  nId, ToolBox &rTbx, SfxBindings &rBindings ) :

    SfxToolBoxControl( nId, rTbx, rBindings )

{
}

// -----------------------------------------------------------------------

SvxLineEndToolBoxControl::~SvxLineEndToolBoxControl()
{
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxLineEndToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxLineEndToolBoxControl::CreatePopupWindow()
{
    SvxLineEndWindow* pLineEndWin =
        new SvxLineEndWindow( GetId(),SVX_RESSTR( RID_SVXSTR_LINEEND ), GetBindings() );
    pLineEndWin->StartPopupMode( &GetToolBox(), TRUE );
    pLineEndWin->StartSelection();
    return pLineEndWin;
}

// -----------------------------------------------------------------------

void SvxLineEndToolBoxControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

