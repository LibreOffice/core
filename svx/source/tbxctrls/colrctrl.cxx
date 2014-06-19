/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svx/dialogs.hrc>

#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/image.hxx>

#include <svx/colrctrl.hxx>

#include <svx/svdview.hxx>
#include "svx/drawitem.hxx"
#include <editeng/colritem.hxx>
#include "svx/xattr.hxx"
#include <svx/xtable.hxx>
#include <svx/dialmgr.hxx>
#include "svx/xexch.hxx"
#include <vcl/svapp.hxx>


// - SvxColorValueSetData -


class SvxColorValueSetData : public TransferableHelper
{
private:

    XFillExchangeData       maData;

protected:

    virtual void            AddSupportedFormats() SAL_OVERRIDE;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) SAL_OVERRIDE;
    virtual bool            WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) SAL_OVERRIDE;

public:

                            SvxColorValueSetData( const XFillAttrSetItem& rSetItem ) :
                                maData( rSetItem ) {}
};



void SvxColorValueSetData::AddSupportedFormats()
{
    AddFormat( SOT_FORMATSTR_ID_XFA );
}



bool SvxColorValueSetData::GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& /*rDestDoc*/ )
{
    bool bRet = false;

    if( SotExchange::GetFormat( rFlavor ) == SOT_FORMATSTR_ID_XFA )
    {
        SetObject( &maData, 0, rFlavor );
        bRet = true;
    }

    return bRet;
}



bool SvxColorValueSetData::WriteObject( SotStorageStreamRef& rxOStm, void*, sal_uInt32 , const ::com::sun::star::datatransfer::DataFlavor&  )
{
    WriteXFillExchangeData( *rxOStm, maData );
    return( rxOStm->GetError() == ERRCODE_NONE );
}

/*************************************************************************
|*
|* SvxColorValueSet_docking: Ctor
|*
\************************************************************************/

SvxColorValueSet_docking::SvxColorValueSet_docking( Window* _pParent, const ResId& rResId ) :
    SvxColorValueSet( _pParent, rResId ),
    DragSourceHelper( this ),
    mbLeftButton(true)
{
    SetAccessibleName(SVX_RESSTR(STR_COLORTABLE));
}

/*************************************************************************
|*
|* SvxColorValueSet_docking: MouseButtonDown
|*
\************************************************************************/

void SvxColorValueSet_docking::MouseButtonDown( const MouseEvent& rMEvt )
{
    // Fuer Mac noch anders handlen !
    if( rMEvt.IsLeft() )
    {
        mbLeftButton = true;
        SvxColorValueSet::MouseButtonDown( rMEvt );
    }
    else
    {
        mbLeftButton = false;
        MouseEvent aMEvt( rMEvt.GetPosPixel(),
                          rMEvt.GetClicks(),
                          rMEvt.GetMode(),
                          MOUSE_LEFT,
                          rMEvt.GetModifier() );
        SvxColorValueSet::MouseButtonDown( aMEvt );
    }

    aDragPosPixel = GetPointerPosPixel();
}

/*************************************************************************
|*
|* SvxColorValueSet_docking: MouseButtonUp
|*
\************************************************************************/

void SvxColorValueSet_docking::MouseButtonUp( const MouseEvent& rMEvt )
{
    // Fuer Mac noch anders handlen !
    if( rMEvt.IsLeft() )
    {
        mbLeftButton = true;
        SvxColorValueSet::MouseButtonUp( rMEvt );
    }
    else
    {
        mbLeftButton = false;
        MouseEvent aMEvt( rMEvt.GetPosPixel(),
                          rMEvt.GetClicks(),
                          rMEvt.GetMode(),
                          MOUSE_LEFT,
                          rMEvt.GetModifier() );
        SvxColorValueSet::MouseButtonUp( aMEvt );
    }
    SetNoSelection();
}

/*************************************************************************
|*
|* Command-Event
|*
\************************************************************************/

void SvxColorValueSet_docking::Command(const CommandEvent& rCEvt)
{
    // Basisklasse
    SvxColorValueSet::Command(rCEvt);
}

/*************************************************************************
|*
|* StartDrag
|*
\************************************************************************/

void SvxColorValueSet_docking::StartDrag( sal_Int8 , const Point&  )
{
    Application::PostUserEvent(STATIC_LINK(this, SvxColorValueSet_docking, ExecDragHdl));
}

/*************************************************************************
|*
|* Drag&Drop asynchron ausfuehren
|*
\************************************************************************/

void SvxColorValueSet_docking::DoDrag()
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    sal_uInt16          nItemId = GetItemId( aDragPosPixel );

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

IMPL_STATIC_LINK(SvxColorValueSet_docking, ExecDragHdl, void*, EMPTYARG)
{
    // Als Link, damit asynchron ohne ImpMouseMoveMsg auf dem Stack auch die
    // Farbleiste geloescht werden darf
    pThis->DoDrag();
    return(0);
}

/*************************************************************************
|*
|* Ctor: SvxColorDockingWindow
|*
\************************************************************************/

SvxColorDockingWindow::SvxColorDockingWindow
(
    SfxBindings* _pBindings,
    SfxChildWindow* pCW,
    Window* _pParent,
    const ResId& rResId
) :

    SfxDockingWindow( _pBindings, pCW, _pParent, rResId ),
    pColorList      (),
    aColorSet       ( this, ResId( 1, *rResId.GetResMgr() ) ),
    nLeftSlot       ( SID_ATTR_FILL_COLOR ),
    nRightSlot      ( SID_ATTR_LINE_COLOR ),
    nCols           ( 20 ),
    nLines          ( 1 ),
    nCount          ( 0 )
{
    FreeResource();

    aColorSet.SetStyle( aColorSet.GetStyle() | WB_ITEMBORDER );
    aColorSet.SetSelectHdl( LINK( this, SvxColorDockingWindow, SelectHdl ) );

    // Get the model from the view shell.  Using SfxObjectShell::Current()
    // is unreliable when called at the wrong times.
    SfxObjectShell* pDocSh = NULL;
    if (_pBindings != NULL)
    {
        SfxDispatcher* pDispatcher = _pBindings->GetDispatcher();
        if (pDispatcher != NULL)
        {
            SfxViewFrame* pFrame = pDispatcher->GetFrame();
            if (pFrame != NULL)
            {
                SfxViewShell* pViewShell = pFrame->GetViewShell();
                if (pViewShell != NULL)
                    pDocSh = pViewShell->GetObjectShell();
            }
        }
    }

    if ( pDocSh )
    {
        const SfxPoolItem*  pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if( pItem )
        {
            pColorList = ( (SvxColorListItem*) pItem )->GetColorList();
            FillValueSet();
        }
    }

    aItemSize = aColorSet.CalcItemSizePixel(Size(aColorSet.getEntryEdgeLength(), aColorSet.getEntryEdgeLength()));
    aItemSize.Width() = aItemSize.Width() + aColorSet.getEntryEdgeLength();
    aItemSize.Width() /= 2;
    aItemSize.Height() = aItemSize.Height() + aColorSet.getEntryEdgeLength();
    aItemSize.Height() /= 2;

    SetSize();
    aColorSet.Show();
    if (_pBindings != NULL)
        StartListening( *_pBindings, true );
}


/*************************************************************************
|*
|* Dtor: SvxColorDockingWindow
|*
\************************************************************************/

SvxColorDockingWindow::~SvxColorDockingWindow()
{
    EndListening( GetBindings() );
}

/*************************************************************************
|*
|* Notify
|*
\************************************************************************/

void SvxColorDockingWindow::Notify( SfxBroadcaster& , const SfxHint& rHint )
{
    const SfxPoolItemHint *pPoolItemHint = PTR_CAST(SfxPoolItemHint, &rHint);
    if ( pPoolItemHint
         && ( pPoolItemHint->GetObject()->ISA( SvxColorListItem ) ) )
    {
        // Die Liste der Farben hat sich geaendert
        pColorList = ( (SvxColorListItem*) pPoolItemHint->GetObject() )->GetColorList();
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
    if( pColorList.is() )
    {
        nCount = pColorList->Count();
        aColorSet.Clear();

        // create the first entry for 'invisible/none'
        const Size aColorSize(aColorSet.getEntryEdgeLength(), aColorSet.getEntryEdgeLength());
        long nPtX = aColorSize.Width() - 1;
        long nPtY = aColorSize.Height() - 1;
        VirtualDevice aVD;

        aVD.SetOutputSizePixel( aColorSize );
        aVD.SetLineColor( Color( COL_BLACK ) );
        aVD.SetBackground( Wallpaper( Color( COL_WHITE ) ) );
        aVD.DrawLine( Point(), Point( nPtX, nPtY ) );
        aVD.DrawLine( Point( 0, nPtY ), Point( nPtX, 0 ) );

        Bitmap aBmp( aVD.GetBitmap( Point(), aColorSize ) );

        aColorSet.InsertItem( (sal_uInt16)1, Image(aBmp), SVX_RESSTR( RID_SVXSTR_INVISIBLE ) );

        aColorSet.addEntriesForXColorList(*pColorList, 2);
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
    nCols = (sal_uInt16) ( aSize.Width() / aItemSize.Width() );
    nLines = (sal_uInt16) ( (float) aSize.Height() / (float) aItemSize.Height() /*+ 0.35*/ );
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
        nCols = (sal_uInt16) ( ( aSize.Width() - nScrollWidth ) / aItemSize.Width() );
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

bool SvxColorDockingWindow::Close()
{
    SfxBoolItem aItem( SID_COLOR_CONTROL, false );
    GetBindings().GetDispatcher()->Execute(
        SID_COLOR_CONTROL, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
    SfxDockingWindow::Close();
    return true;
}

/*************************************************************************
|*
|* SelectHdl
|*
\************************************************************************/

IMPL_LINK_NOARG(SvxColorDockingWindow, SelectHdl)
{
    SfxDispatcher* pDispatcher = GetBindings().GetDispatcher();
    sal_uInt16 nPos = aColorSet.GetSelectItemId();
    Color  aColor( aColorSet.GetItemColor( nPos ) );
    OUString aStr( aColorSet.GetItemText( nPos ) );

    if (aColorSet.IsLeftButton())
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
                bool bDone = false;

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
                        bDone = true;
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


void SvxColorDockingWindow::Resizing( Size& rNewSize )
{
    rNewSize.Width()  -= 4;
    rNewSize.Height() -= 4;

    // Spalten und Reihen ermitteln
    nCols = (sal_uInt16) ( (float) rNewSize.Width() / (float) aItemSize.Width() + 0.5 );
    nLines = (sal_uInt16) ( (float) rNewSize.Height() / (float) aItemSize.Height() + 0.5 );
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
        nCols = (sal_uInt16) ( ( ( (float) rNewSize.Width() - (float) nScrollWidth ) )
                            / (float) aItemSize.Width() + 0.5 );
    }
    if( nCols <= 1 )
        nCols = 2;

    // Max. Reihen anhand der gegebenen Spalten berechnen
    long nMaxLines = nCount / nCols;
    if( nCount %  nCols )
        nMaxLines++;

    nLines = sal::static_int_cast< sal_uInt16 >(
        std::min< long >( nLines, nMaxLines ) );

    // Groesse des Windows setzen
    rNewSize.Width()  = nCols * aItemSize.Width() + nScrollWidth + 4;
    rNewSize.Height() = nLines * aItemSize.Height() + 4;
}

/*************************************************************************
|*
|* Resize
|*
\************************************************************************/

void SvxColorDockingWindow::Resize()
{
    if ( !IsFloatingMode() || !GetFloatingWindow()->IsRollUp() )
        SetSize();
    SfxDockingWindow::Resize();
}



void SvxColorDockingWindow::GetFocus (void)
{
    SfxDockingWindow::GetFocus();
    // Grab the focus to the color value set so that it can be controlled
    // with the keyboard.
    aColorSet.GrabFocus();
}

bool SvxColorDockingWindow::Notify( NotifyEvent& rNEvt )
{
    bool nRet = false;
    if( ( rNEvt.GetType() == EVENT_KEYINPUT ) )
    {
        KeyEvent aKeyEvt = *rNEvt.GetKeyEvent();
        sal_uInt16   nKeyCode = aKeyEvt.GetKeyCode().GetCode();
        switch( nKeyCode )
        {
            case KEY_ESCAPE:
                GrabFocusToDocument();
                nRet = true;
                break;
        }
    }

    return nRet || SfxDockingWindow::Notify( rNEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
