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

#include <sal/config.h>

#include <sot/exchange.hxx>
#include <sot/storage.hxx>
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
#include "helpid.hrc"
#include <vcl/svapp.hxx>

using namespace com::sun::star;

// - SvxColorValueSetData -

class SvxColorValueSetData : public TransferableHelper
{
private:

    XFillExchangeData       maData;

protected:

    virtual void            AddSupportedFormats() override;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
    virtual bool            WriteObject( tools::SvRef<SotStorageStream>& rxOStm, void* pUserObject, SotClipboardFormatId nUserObjectId, const css::datatransfer::DataFlavor& rFlavor ) override;

public:

    explicit SvxColorValueSetData( const XFillAttrSetItem& rSetItem ) :
        maData( rSetItem ) {}
};

void SvxColorValueSetData::AddSupportedFormats()
{
    AddFormat( SotClipboardFormatId::XFA );
}

bool SvxColorValueSetData::GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& /*rDestDoc*/ )
{
    bool bRet = false;

    if( SotExchange::GetFormat( rFlavor ) == SotClipboardFormatId::XFA )
    {
        SetObject( &maData, SotClipboardFormatId::NONE, rFlavor );
        bRet = true;
    }

    return bRet;
}

bool SvxColorValueSetData::WriteObject( tools::SvRef<SotStorageStream>& rxOStm, void*, SotClipboardFormatId, const css::datatransfer::DataFlavor&  )
{
    WriteXFillExchangeData( *rxOStm, maData );
    return( rxOStm->GetError() == ERRCODE_NONE );
}

SvxColorValueSet_docking::SvxColorValueSet_docking( vcl::Window* _pParent, WinBits nWinStyle ) :
    SvxColorValueSet( _pParent, nWinStyle ),
    DragSourceHelper( this ),
    mbLeftButton(true)
{
    SetAccessibleName(SVX_RESSTR(STR_COLORTABLE));
}

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

void SvxColorValueSet_docking::Command(const CommandEvent& rCEvt)
{
    // Basisklasse
    SvxColorValueSet::Command(rCEvt);
}

void SvxColorValueSet_docking::StartDrag( sal_Int8 , const Point&  )
{
    Application::PostUserEvent(LINK(this, SvxColorValueSet_docking, ExecDragHdl), nullptr, true);
}

void SvxColorValueSet_docking::DoDrag()
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    sal_uInt16          nItemId = GetItemId( aDragPosPixel );

    if( pDocSh && nItemId )
    {
        XFillAttrSetItem    aXFillSetItem( &pDocSh->GetPool() );
        SfxItemSet&         rSet = aXFillSetItem.GetItemSet();

        rSet.Put( XFillColorItem( GetItemText( nItemId ), GetItemColor( nItemId ) ) );
        rSet.Put(XFillStyleItem( ( 1 == nItemId ) ? drawing::FillStyle_NONE : drawing::FillStyle_SOLID ) );

        EndSelection();
        ( new SvxColorValueSetData( aXFillSetItem ) )->StartDrag( this, DND_ACTION_COPY );
        ReleaseMouse();
    }
}

IMPL_LINK_NOARG_TYPED(SvxColorValueSet_docking, ExecDragHdl, void*, void)
{
    // Als Link, damit asynchron ohne ImpMouseMoveMsg auf dem Stack auch die
    // Farbleiste geloescht werden darf
    DoDrag();
}

SvxColorDockingWindow::SvxColorDockingWindow
(
    SfxBindings* _pBindings,
    SfxChildWindow* pCW,
    vcl::Window* _pParent
) :

    SfxDockingWindow( _pBindings, pCW, _pParent, WB_MOVEABLE|WB_CLOSEABLE|WB_SIZEABLE|WB_DOCKABLE ),
    pColorList      (),
    aColorSet       ( VclPtr<SvxColorValueSet_docking>::Create(this) ),
    nLeftSlot       ( SID_ATTR_FILL_COLOR ),
    nRightSlot      ( SID_ATTR_LINE_COLOR ),
    nCols           ( 20 ),
    nLines          ( 1 ),
    nCount          ( 0 )
{
    SetText(SVX_RESSTR(STR_COLORTABLE));
    SetSizePixel(LogicToPixel(Size(150, 22), MapMode(MAP_APPFONT)));
    SetHelpId(HID_CTRL_COLOR);

    aColorSet->SetSelectHdl( LINK( this, SvxColorDockingWindow, SelectHdl ) );
    aColorSet->SetHelpId(HID_COLOR_CTL_COLORS);
    aColorSet->SetPosSizePixel(LogicToPixel(Point(2, 2), MapMode(MAP_APPFONT)),
                              LogicToPixel(Size(146, 18), MapMode(MAP_APPFONT)));

    // Get the model from the view shell.  Using SfxObjectShell::Current()
    // is unreliable when called at the wrong times.
    SfxObjectShell* pDocSh = nullptr;
    if (_pBindings != nullptr)
    {
        SfxDispatcher* pDispatcher = _pBindings->GetDispatcher();
        if (pDispatcher != nullptr)
        {
            SfxViewFrame* pFrame = pDispatcher->GetFrame();
            if (pFrame != nullptr)
            {
                SfxViewShell* pViewShell = pFrame->GetViewShell();
                if (pViewShell != nullptr)
                    pDocSh = pViewShell->GetObjectShell();
            }
        }
    }

    if ( pDocSh )
    {
        const SfxPoolItem*  pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if( pItem )
        {
            pColorList = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
            FillValueSet();
        }
    }

    aItemSize = aColorSet->CalcItemSizePixel(Size(SvxColorValueSet::getEntryEdgeLength(), SvxColorValueSet::getEntryEdgeLength()));
    aItemSize.Width() = aItemSize.Width() + SvxColorValueSet::getEntryEdgeLength();
    aItemSize.Width() /= 2;
    aItemSize.Height() = aItemSize.Height() + SvxColorValueSet::getEntryEdgeLength();
    aItemSize.Height() /= 2;

    SetSize();
    aColorSet->Show();
    if (_pBindings != nullptr)
        StartListening( *_pBindings, true );
}

SvxColorDockingWindow::~SvxColorDockingWindow()
{
    disposeOnce();
}

void SvxColorDockingWindow::dispose()
{
    EndListening( GetBindings() );
    aColorSet.disposeAndClear();
    SfxDockingWindow::dispose();
}

void SvxColorDockingWindow::Notify( SfxBroadcaster& , const SfxHint& rHint )
{
    const SfxPoolItemHint* pPoolItemHint = dynamic_cast<const SfxPoolItemHint*>(&rHint);
    if ( pPoolItemHint
         && ( dynamic_cast<const SvxColorListItem*>(pPoolItemHint->GetObject()) != nullptr ) )
    {
        // Die Liste der Farben hat sich geaendert
        pColorList = static_cast<SvxColorListItem*>( pPoolItemHint->GetObject() )->GetColorList();
        FillValueSet();
    }
}

void SvxColorDockingWindow::FillValueSet()
{
    if( pColorList.is() )
    {
        nCount = pColorList->Count();
        aColorSet->Clear();

        // create the first entry for 'invisible/none'
        const Size aColorSize(SvxColorValueSet::getEntryEdgeLength(), SvxColorValueSet::getEntryEdgeLength());
        long nPtX = aColorSize.Width() - 1;
        long nPtY = aColorSize.Height() - 1;
        ScopedVclPtrInstance< VirtualDevice > pVD;

        pVD->SetOutputSizePixel( aColorSize );
        pVD->SetLineColor( Color( COL_BLACK ) );
        pVD->SetBackground( Wallpaper( Color( COL_WHITE ) ) );
        pVD->DrawLine( Point(), Point( nPtX, nPtY ) );
        pVD->DrawLine( Point( 0, nPtY ), Point( nPtX, 0 ) );

        Bitmap aBmp( pVD->GetBitmap( Point(), aColorSize ) );

        aColorSet->InsertItem( (sal_uInt16)1, Image(aBmp), SVX_RESSTR( RID_SVXSTR_INVISIBLE ) );

        aColorSet->addEntriesForXColorList(*pColorList, 2);
    }
}

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
    WinBits nBits = aColorSet->GetStyle();
    if ( static_cast<long>(nLines) * nCols >= nCount )
        nBits &= ~WB_VSCROLL;
    else
        nBits |= WB_VSCROLL;
    aColorSet->SetStyle( nBits );

    // ScrollBar ?
    long nScrollWidth = aColorSet->GetScrollWidth();
    if( nScrollWidth > 0 )
    {
        // Spalten mit ScrollBar berechnen
        nCols = (sal_uInt16) ( ( aSize.Width() - nScrollWidth ) / aItemSize.Width() );
    }
    aColorSet->SetColCount( nCols );

    if( IsFloatingMode() )
        aColorSet->SetLineCount( nLines );
    else
    {
        aColorSet->SetLineCount(); // sonst wird LineHeight ignoriert
        aColorSet->SetItemHeight( aItemSize.Height() );
    }

    aColorSet->SetPosSizePixel( Point( 2, 2 ), aSize );
}

bool SvxColorDockingWindow::Close()
{
    SfxBoolItem aItem( SID_COLOR_CONTROL, false );
    GetBindings().GetDispatcher()->Execute(
        SID_COLOR_CONTROL, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, &aItem, 0L );
    SfxDockingWindow::Close();
    return true;
}

IMPL_LINK_NOARG_TYPED(SvxColorDockingWindow, SelectHdl, ValueSet*, void)
{
    SfxDispatcher* pDispatcher = GetBindings().GetDispatcher();
    sal_uInt16 nPos = aColorSet->GetSelectItemId();
    Color  aColor( aColorSet->GetItemColor( nPos ) );
    OUString aStr( aColorSet->GetItemText( nPos ) );

    if (aColorSet->IsLeftButton())
    {
        if ( nLeftSlot == SID_ATTR_FILL_COLOR )
        {
            if ( nPos == 1 )        // unsichtbar
            {
                XFillStyleItem aXFillStyleItem( drawing::FillStyle_NONE );
                pDispatcher->Execute( nLeftSlot, SfxCallMode::RECORD, &aXFillStyleItem, 0L );
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
                            SID_ATTR_CHAR_COLOR, SfxCallMode::RECORD, &aTextColorItem, 0L );
                        bDone = true;
                    }
                }
                if ( !bDone )
                {
                    XFillStyleItem aXFillStyleItem( drawing::FillStyle_SOLID );
                    XFillColorItem aXFillColorItem( aStr, aColor );
                    pDispatcher->Execute(
                        nLeftSlot, SfxCallMode::RECORD, &aXFillColorItem, &aXFillStyleItem, 0L );
                }
            }
        }
        else if ( nPos != 1 )       // unsichtbar
        {
            SvxColorItem aLeftColorItem( aColor, nLeftSlot );
            pDispatcher->Execute( nLeftSlot, SfxCallMode::RECORD, &aLeftColorItem, 0L );
        }
    }
    else
    {
        if ( nRightSlot == SID_ATTR_LINE_COLOR )
        {
            if( nPos == 1 )     // unsichtbar
            {
                XLineStyleItem aXLineStyleItem( drawing::LineStyle_NONE );
                pDispatcher->Execute( nRightSlot, SfxCallMode::RECORD, &aXLineStyleItem, 0L );
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
                        if ( aAttrSet.GetItemState( XATTR_LINESTYLE ) != SfxItemState::DONTCARE )
                        {
                            drawing::LineStyle eXLS = (drawing::LineStyle)
                                static_cast<const XLineStyleItem&>(aAttrSet.Get( XATTR_LINESTYLE ) ).GetValue();
                            if ( eXLS == drawing::LineStyle_NONE )
                            {
                                XLineStyleItem aXLineStyleItem( drawing::LineStyle_SOLID );
                                pDispatcher->Execute( nRightSlot, SfxCallMode::RECORD, &aXLineStyleItem, 0L );
                            }
                        }
                    }
                }

                XLineColorItem aXLineColorItem( aStr, aColor );
                pDispatcher->Execute( nRightSlot, SfxCallMode::RECORD, &aXLineColorItem, 0L );
            }
        }
        else if ( nPos != 1 )       // unsichtbar
        {
            SvxColorItem aRightColorItem( aColor, nRightSlot );
            pDispatcher->Execute( nRightSlot, SfxCallMode::RECORD, &aRightColorItem, 0L );
        }
    }
}

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
    WinBits nBits = aColorSet->GetStyle();
    if ( static_cast<long>(nLines) * nCols >= nCount )
        nBits &= ~WB_VSCROLL;
    else
        nBits |= WB_VSCROLL;
    aColorSet->SetStyle( nBits );

    // ScrollBar ?
    long nScrollWidth = aColorSet->GetScrollWidth();
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

void SvxColorDockingWindow::Resize()
{
    if ( !IsFloatingMode() || !GetFloatingWindow()->IsRollUp() )
        SetSize();
    SfxDockingWindow::Resize();
}

void SvxColorDockingWindow::GetFocus()
{
    SfxDockingWindow::GetFocus();
    if (aColorSet)
    {
        // Grab the focus to the color value set so that it can be controlled
        // with the keyboard.
        aColorSet->GrabFocus();
    }
}

bool SvxColorDockingWindow::Notify( NotifyEvent& rNEvt )
{
    bool bRet = false;
    if( ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT ) )
    {
        KeyEvent aKeyEvt = *rNEvt.GetKeyEvent();
        sal_uInt16   nKeyCode = aKeyEvt.GetKeyCode().GetCode();
        switch( nKeyCode )
        {
            case KEY_ESCAPE:
                GrabFocusToDocument();
                bRet = true;
                break;
        }
    }

    return bRet || SfxDockingWindow::Notify( rNEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
