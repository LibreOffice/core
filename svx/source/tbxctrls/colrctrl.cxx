/*************************************************************************
 *
 *  $RCSfile: colrctrl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-20 20:01:38 $
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

#pragma hdrstop

#include "dialogs.hrc"

#define ITEMID_COLOR            0
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE

#include <tools/list.hxx>

#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#include "colrctrl.hxx"

#include "svdview.hxx"
#include "drawitem.hxx"
#include "colritem.hxx"
#include "xattr.hxx"
#include "xtable.hxx"
#include "dialmgr.hxx"
#include "xexch.hxx"

SFX_IMPL_DOCKINGWINDOW( SvxColorChildWindow, SID_COLOR_CONTROL )

// ------------------------
// - SvxColorValueSetData -
// ------------------------

class SvxColorValueSetData : public TransferableHelper
{
private:

    XFillExchangeData       maData;

protected:

    virtual void            AddSupportedFormats();
    virtual sal_Bool        GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual sal_Bool        WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );

public:

                            SvxColorValueSetData( const XFillAttrSetItem& rSetItem ) :
                                maData( rSetItem ) {}
};

// -----------------------------------------------------------------------------

void SvxColorValueSetData::AddSupportedFormats()
{
    AddFormat( SOT_FORMATSTR_ID_XFA );
}

// -----------------------------------------------------------------------------

sal_Bool SvxColorValueSetData::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    sal_Bool bRet = sal_False;

    if( SotExchange::GetFormat( rFlavor ) == SOT_FORMATSTR_ID_XFA )
    {
        SetObject( &maData, 0, rFlavor );
        bRet = sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SvxColorValueSetData::WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    *rxOStm << maData;
    return( rxOStm->GetError() == ERRCODE_NONE );
}

/*************************************************************************
|*
|* SvxColorValueSet: Ctor
|*
\************************************************************************/

SvxColorValueSet::SvxColorValueSet( Window* pParent, WinBits nWinStyle ) :
    ValueSet( pParent, nWinStyle ),
    DragSourceHelper( this )
{
}

/*************************************************************************
|*
|* SvxColorValueSet: Ctor
|*
\************************************************************************/

SvxColorValueSet::SvxColorValueSet( Window* pParent, const ResId& rResId ) :
    ValueSet( pParent, rResId ),
    DragSourceHelper( this )
{
}

/*************************************************************************
|*
|* SvxColorValueSet: MouseButtonDown
|*
\************************************************************************/

void SvxColorValueSet::MouseButtonDown( const MouseEvent& rMEvt )
{
    // Fuer Mac noch anders handlen !
    if( rMEvt.IsLeft() )
    {
        bLeft = TRUE;
        ValueSet::MouseButtonDown( rMEvt );
    }
    else
    {
        bLeft = FALSE;
        MouseEvent aMEvt( rMEvt.GetPosPixel(),
                          rMEvt.GetClicks(),
                          rMEvt.GetMode(),
                          MOUSE_LEFT,
                          rMEvt.GetModifier() );
        ValueSet::MouseButtonDown( aMEvt );
    }

    aDragPosPixel = GetPointerPosPixel();
}

/*************************************************************************
|*
|* SvxColorValueSet: MouseButtonUp
|*
\************************************************************************/

void SvxColorValueSet::MouseButtonUp( const MouseEvent& rMEvt )
{
    // Fuer Mac noch anders handlen !
    if( rMEvt.IsLeft() )
    {
        bLeft = TRUE;
        ValueSet::MouseButtonUp( rMEvt );
    }
    else
    {
        bLeft = FALSE;
        MouseEvent aMEvt( rMEvt.GetPosPixel(),
                          rMEvt.GetClicks(),
                          rMEvt.GetMode(),
                          MOUSE_LEFT,
                          rMEvt.GetModifier() );
        ValueSet::MouseButtonUp( aMEvt );
    }
    SetNoSelection();
}

/*************************************************************************
|*
|* Command-Event
|*
\************************************************************************/

void SvxColorValueSet::Command(const CommandEvent& rCEvt)
{
    // Basisklasse
    ValueSet::Command(rCEvt);
}

/*************************************************************************
|*
|* StartDrag
|*
\************************************************************************/

void SvxColorValueSet::StartDrag( sal_Int8 nAction, const Point& rPtPixel )
{
    Application::PostUserEvent(STATIC_LINK(this, SvxColorValueSet, ExecDragHdl));
}

/*************************************************************************
|*
|* Drag&Drop asynchron ausfuehren
|*
\************************************************************************/

void SvxColorValueSet::DoDrag()
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    USHORT          nItemId = GetItemId( aDragPosPixel );

    if( pDocSh && nItemId )
    {
        XFillAttrSetItem    aXFillSetItem( &pDocSh->GetPool() );
        SfxItemSet&         rSet = aXFillSetItem.GetItemSet();

        rSet.Put( XFillColorItem( GetItemText( nItemId ), GetItemColor( nItemId ) ) );
        rSet.Put(XFillStyleItem( ( 1 == nItemId ) ? XFILL_NONE : XFILL_SOLID ) );

        EndSelection();
        ( new SvxColorValueSetData( aXFillSetItem ) )->StartDrag( this, DND_ACTION_COPY );
        ReleaseMouse();
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_STATIC_LINK(SvxColorValueSet, ExecDragHdl, void*, EMPTYARG)
{
    // Als Link, damit asynchron ohne ImpMouseMoveMsg auf dem Stack auch die
    // Farbleiste geloescht werden darf
    pThis->DoDrag();
    return(0);
}

/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Animator
|*
\************************************************************************/

__EXPORT SvxColorChildWindow::SvxColorChildWindow( Window* pParent,
                                                   USHORT nId,
                                                   SfxBindings* pBindings,
                                                   SfxChildWinInfo* pInfo ) :
    SfxChildWindow( pParent, nId )
{
    SvxColorDockingWindow* pWin = new SvxColorDockingWindow( pBindings, this,
                                        pParent, SVX_RES( RID_SVXCTRL_COLOR ) );
    pWindow = pWin;

    eChildAlignment = SFX_ALIGN_BOTTOM;

    pWin->Initialize( pInfo );
}



/*************************************************************************
|*
|* Ctor: SvxColorDockingWindow
|*
\************************************************************************/

__EXPORT SvxColorDockingWindow::SvxColorDockingWindow
(
    SfxBindings* pBindings,
    SfxChildWindow* pCW,
    Window* pParent,
    const ResId& rResId
) :

    SfxDockingWindow( pBindings, pCW, pParent, rResId ),

    aColorSet       ( this, ResId( 1 ) ),
    pColorTable     ( NULL ),
    nLeftSlot       ( SID_ATTR_FILL_COLOR ),
    nRightSlot      ( SID_ATTR_LINE_COLOR ),
    nCols           ( 20 ),
    nLines          ( 1 ),
    aColorSize      ( 14, 14 )

{
    FreeResource();

    aColorSet.SetStyle( aColorSet.GetStyle() | WB_ITEMBORDER );
    aColorSet.SetSelectHdl( LINK( this, SvxColorDockingWindow, SelectHdl ) );

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if ( pDocSh )
    {
        const SfxPoolItem*  pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if( pItem )
        {
            pColorTable = ( (SvxColorTableItem*) pItem )->GetColorTable();
            FillValueSet();
        }
    }
    aItemSize = aColorSet.CalcItemSizePixel( aColorSize );
    aItemSize.Width() = aItemSize.Width() + aColorSize.Width();
    aItemSize.Width() /= 2;
    aItemSize.Height() = aItemSize.Height() + aColorSize.Height();
    aItemSize.Height() /= 2;

    SetSize();
    aColorSet.Show();
    StartListening( *pBindings, TRUE );
}


/*************************************************************************
|*
|* Dtor: SvxColorDockingWindow
|*
\************************************************************************/

__EXPORT SvxColorDockingWindow::~SvxColorDockingWindow()
{
    EndListening( GetBindings() );
}

/*************************************************************************
|*
|* Notify
|*
\************************************************************************/

void SvxColorDockingWindow::SFX_NOTIFY( SfxBroadcaster& rBC,
                                         const TypeId& rBCType,
                                         const SfxHint& rHint,
                                         const TypeId& rHintType )
{
    const SfxPoolItemHint *pPoolItemHint = PTR_CAST(SfxPoolItemHint, &rHint);
    if ( pPoolItemHint
         && ( pPoolItemHint->GetObject()->ISA( SvxColorTableItem ) ) )
    {
        // Die Liste der Farben hat sich geaendert
        pColorTable = ( (SvxColorTableItem*) pPoolItemHint->GetObject() )->GetColorTable();
        FillValueSet();
    }
}

/*************************************************************************
|*
|* FillValueSet
|*
\************************************************************************/

void SvxColorDockingWindow::FillValueSet()
{
    if( pColorTable )
    {
        aColorSet.Clear();

        // Erster Eintrag: unsichtbar
        long nPtX = aColorSize.Width() - 1;
        long nPtY = aColorSize.Height() - 1;
        VirtualDevice aVD;
        aVD.SetOutputSizePixel( aColorSize );
        aVD.SetLineColor( Color( COL_BLACK ) );
        aVD.SetBackground( Wallpaper( Color( COL_WHITE ) ) );
        aVD.DrawLine( Point(), Point( nPtX, nPtY ) );
        aVD.DrawLine( Point( 0, nPtY ), Point( nPtX, 0 ) );

        Bitmap aBmp( aVD.GetBitmap( Point(), aColorSize ) );

        aColorSet.InsertItem( (USHORT)1, aBmp, SVX_RESSTR( RID_SVXSTR_INVISIBLE ) );

        XColorEntry* pEntry;
        nCount = pColorTable->Count();

        for( long i = 0; i < nCount; i++ )
        {
            pEntry = pColorTable->Get( i );
            aColorSet.InsertItem( (USHORT)i+2,
                            pEntry->GetColor(), pEntry->GetName() );
        }
    }
}

/*************************************************************************
|*
|* SetSize
|*
\************************************************************************/

void SvxColorDockingWindow::SetSize()
{
    // Groesse fuer ValueSet berechnen
    Size aSize = GetOutputSizePixel();
    aSize.Width()  -= 4;
    aSize.Height() -= 4;

    // Zeilen und Spalten berechnen
    nCols = (USHORT) ( aSize.Width() / aItemSize.Width() );
    nLines = (USHORT) ( (float) aSize.Height() / (float) aItemSize.Height() + 0.35 );
    if( nLines == 0 )
        nLines++;

    // Scrollbar setzen/entfernen
    WinBits nBits = aColorSet.GetStyle();
    if ( nLines * nCols >= nCount )
        nBits &= ~WB_VSCROLL;
    else
        nBits |= WB_VSCROLL;
    aColorSet.SetStyle( nBits );

    // ScrollBar ?
    long nScrollWidth = aColorSet.GetScrollWidth();
    if( nScrollWidth > 0 )
    {
        // Spalten mit ScrollBar berechnen
        nCols = (USHORT) ( ( aSize.Width() - nScrollWidth ) / aItemSize.Width() );
    }
    aColorSet.SetColCount( nCols );

    if( IsFloatingMode() )
        aColorSet.SetLineCount( nLines );
    else
    {
        aColorSet.SetLineCount( 0 ); // sonst wird LineHeight ignoriert
        aColorSet.SetItemHeight( aItemSize.Height() );
    }

    aColorSet.SetPosSizePixel( Point( 2, 2 ), aSize );
}

/*************************************************************************
|*
|* SvxColorDockingWindow: Close
|*
\************************************************************************/

BOOL __EXPORT SvxColorDockingWindow::Close()
{
    SfxBoolItem aItem( SID_COLOR_CONTROL, FALSE );
    GetBindings().GetDispatcher()->Execute(
        SID_COLOR_CONTROL, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
    SfxDockingWindow::Close();
    return( TRUE );
}

/*************************************************************************
|*
|* SelectHdl
|*
\************************************************************************/

IMPL_LINK( SvxColorDockingWindow, SelectHdl, void *, EMPTYARG )
{
    SfxDispatcher* pDispatcher = GetBindings().GetDispatcher();
    USHORT nPos = aColorSet.GetSelectItemId();
    Color  aColor( aColorSet.GetItemColor( nPos ) );
    String aStr( aColorSet.GetItemText( nPos ) );

    if ( aColorSet.IsLeftButton() )
    {
        if ( nLeftSlot == SID_ATTR_FILL_COLOR )
        {
            if ( nPos == 1 )        // unsichtbar
            {
                XFillStyleItem aXFillStyleItem( XFILL_NONE );
                pDispatcher->Execute( nLeftSlot, SFX_CALLMODE_RECORD, &aXFillStyleItem, 0L );
            }
            else
            {
                BOOL bDone = FALSE;

                // Wenn wir eine DrawView haben und uns im TextEdit-Modus befinden,
                // wird nicht die Flaechen-, sondern die Textfarbe zugewiesen
                SfxViewShell* pViewSh = SfxViewShell::Current();
                if ( pViewSh )
                {
                    SdrView* pView = pViewSh->GetDrawView();
                    if ( pView && pView->IsTextEdit() )
                    {
                        SvxColorItem aTextColorItem( aColor, SID_ATTR_CHAR_COLOR );
                        pDispatcher->Execute(
                            SID_ATTR_CHAR_COLOR, SFX_CALLMODE_RECORD, &aTextColorItem, 0L );
                        bDone = TRUE;
                    }
                }
                if ( !bDone )
                {
                    XFillStyleItem aXFillStyleItem( XFILL_SOLID );
                    XFillColorItem aXFillColorItem( aStr, aColor );
                    pDispatcher->Execute(
                        nLeftSlot, SFX_CALLMODE_RECORD, &aXFillColorItem, &aXFillStyleItem, 0L );
                }
            }
        }
        else if ( nPos != 1 )       // unsichtbar
        {
            SvxColorItem aLeftColorItem( aColor, nLeftSlot );
            pDispatcher->Execute( nLeftSlot, SFX_CALLMODE_RECORD, &aLeftColorItem, 0L );
        }
    }
    else
    {
        if ( nRightSlot == SID_ATTR_LINE_COLOR )
        {
            if( nPos == 1 )     // unsichtbar
            {
                XLineStyleItem aXLineStyleItem( XLINE_NONE );
                pDispatcher->Execute( nRightSlot, SFX_CALLMODE_RECORD, &aXLineStyleItem, 0L );
            }
            else
            {
                // Sollte der LineStyle unsichtbar sein, so wird er auf SOLID gesetzt
                SfxViewShell* pViewSh = SfxViewShell::Current();
                if ( pViewSh )
                {
                    SdrView* pView = pViewSh->GetDrawView();
                    if ( pView )
                    {
                        SfxItemSet aAttrSet( pView->GetModel()->GetItemPool() );
                        pView->GetAttributes( aAttrSet );
                        if ( aAttrSet.GetItemState( XATTR_LINESTYLE ) != SFX_ITEM_DONTCARE )
                        {
                            XLineStyle eXLS = (XLineStyle)
                                ( (const XLineStyleItem&)aAttrSet.Get( XATTR_LINESTYLE ) ).GetValue();
                            if ( eXLS == XLINE_NONE )
                            {
                                XLineStyleItem aXLineStyleItem( XLINE_SOLID );
                                pDispatcher->Execute( nRightSlot, SFX_CALLMODE_RECORD, &aXLineStyleItem, 0L );
                            }
                        }
                    }
                }

                XLineColorItem aXLineColorItem( aStr, aColor );
                pDispatcher->Execute( nRightSlot, SFX_CALLMODE_RECORD, &aXLineColorItem, 0L );
            }
        }
        else if ( nPos != 1 )       // unsichtbar
        {
            SvxColorItem aRightColorItem( aColor, nRightSlot );
            pDispatcher->Execute( nRightSlot, SFX_CALLMODE_RECORD, &aRightColorItem, 0L );
        }
    }

    return 0;
}

/*************************************************************************
|*
|* Resizing
|*
\************************************************************************/


void __EXPORT SvxColorDockingWindow::Resizing( Size& rNewSize )
{
    rNewSize.Width()  -= 4;
    rNewSize.Height() -= 4;

    // Spalten und Reihen ermitteln
    nCols = (USHORT) ( (float) rNewSize.Width() / (float) aItemSize.Width() + 0.5 );
    nLines = (USHORT) ( (float) rNewSize.Height() / (float) aItemSize.Height() + 0.5 );
    if( nLines == 0 )
        nLines = 1;

    // Scrollbar setzen/entfernen
    WinBits nBits = aColorSet.GetStyle();
    if ( nLines * nCols >= nCount )
        nBits &= ~WB_VSCROLL;
    else
        nBits |= WB_VSCROLL;
    aColorSet.SetStyle( nBits );

    // ScrollBar ?
    long nScrollWidth = aColorSet.GetScrollWidth();
    if( nScrollWidth > 0 )
    {
        // Spalten mit ScrollBar berechnen
        nCols = (USHORT) ( ( ( (float) rNewSize.Width() - (float) nScrollWidth ) )
                            / (float) aItemSize.Width() + 0.5 );
    }
    if( nCols <= 1 )
        nCols = 2;

    // Max. Reihen anhand der gegebenen Spalten berechnen
    USHORT nMaxLines = nCount / nCols;
    if( nCount %  nCols )
        nMaxLines++;

    nLines = Min( nLines, nMaxLines );

    // Groesse des Windows setzen
    rNewSize.Width()  = nCols * aItemSize.Width() + nScrollWidth + 4;
    rNewSize.Height() = nLines * aItemSize.Height() + 4;
}

/*************************************************************************
|*
|* Resize
|*
\************************************************************************/

void __EXPORT SvxColorDockingWindow::Resize()
{
    if ( !IsFloatingMode() || !GetFloatingWindow()->IsRollUp() )
        SetSize();
    SfxDockingWindow::Resize();
}

