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
#include <svx/strings.hrc>
#include <svx/svxids.hrc>

#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/image.hxx>

#include <svx/colrctrl.hxx>

#include <svx/svdview.hxx>
#include <svx/drawitem.hxx>
#include <editeng/colritem.hxx>
#include <svx/xattr.hxx>
#include <svx/xtable.hxx>
#include <svx/dialmgr.hxx>
#include <helpids.h>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

using namespace com::sun::star;


class SvxColorValueSetData : public TransferableHelper
{
private:

    uno::Sequence<beans::NamedValue> m_Data;

protected:

    virtual void            AddSupportedFormats() override;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;

public:

    explicit SvxColorValueSetData(const uno::Sequence<beans::NamedValue>& rProps)
        : m_Data(rProps)
    {}
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
        SetAny(uno::makeAny(m_Data));
        bRet = true;
    }

    return bRet;
}

SvxColorValueSet_docking::SvxColorValueSet_docking( vcl::Window* _pParent ) :
    SvxColorValueSet( _pParent, WB_ITEMBORDER ),
    DragSourceHelper( this ),
    mbLeftButton(true)
{
    SetAccessibleName(SvxResId(STR_COLORTABLE));
}

void SvxColorValueSet_docking::MouseButtonDown( const MouseEvent& rMEvt )
{
    // For Mac still handle differently!
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
    // For Mac still handle differently!
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
        uno::Sequence<beans::NamedValue> props(2);
        XFillColorItem const color(GetItemText(nItemId), GetItemColor(nItemId));
        props[0].Name = "FillColor";
        color.QueryValue(props[0].Value, 0);
        XFillStyleItem const style((1 == nItemId)
                ? drawing::FillStyle_NONE
                : drawing::FillStyle_SOLID);
        props[1].Name = "FillStyle";
        style.QueryValue(props[1].Value, 0);

        EndSelection();
        ( new SvxColorValueSetData(props) )->StartDrag( this, DND_ACTION_COPY );
        ReleaseMouse();
    }
}

IMPL_LINK_NOARG(SvxColorValueSet_docking, ExecDragHdl, void*, void)
{
    // As a link, so that asynchronously without ImpMouseMoveMsg on the
    // stack the color bar may also be deleted
    DoDrag();
}

static constexpr sal_uInt16 gnLeftSlot = SID_ATTR_FILL_COLOR;
static constexpr sal_uInt16 gnRightSlot = SID_ATTR_LINE_COLOR;

SvxColorDockingWindow::SvxColorDockingWindow
(
    SfxBindings* _pBindings,
    SfxChildWindow* pCW,
    vcl::Window* _pParent
) :

    SfxDockingWindow( _pBindings, pCW, _pParent, WB_MOVEABLE|WB_CLOSEABLE|WB_SIZEABLE|WB_DOCKABLE ),
    pColorList      (),
    aColorSet       ( VclPtr<SvxColorValueSet_docking>::Create(this) ),
    nCols           ( 20 ),
    nLines          ( 1 ),
    nCount          ( 0 )
{
    SetText(SvxResId(STR_COLORTABLE));
    SetSizePixel(LogicToPixel(Size(150, 22), MapMode(MapUnit::MapAppFont)));
    SetHelpId(HID_CTRL_COLOR);

    aColorSet->SetSelectHdl( LINK( this, SvxColorDockingWindow, SelectHdl ) );
    aColorSet->SetHelpId(HID_COLOR_CTL_COLORS);
    aColorSet->SetPosSizePixel(LogicToPixel(Point(2, 2), MapMode(MapUnit::MapAppFont)),
                              LogicToPixel(Size(146, 18), MapMode(MapUnit::MapAppFont)));

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
    aItemSize.setWidth( aItemSize.Width() + SvxColorValueSet::getEntryEdgeLength() );
    aItemSize.setWidth( aItemSize.Width() / 2 );
    aItemSize.setHeight( aItemSize.Height() + SvxColorValueSet::getEntryEdgeLength() );
    aItemSize.setHeight( aItemSize.Height() / 2 );

    SetSize();
    aColorSet->Show();
    if (_pBindings != nullptr)
        StartListening(*_pBindings, DuplicateHandling::Prevent);
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
        // The list of colors has changed
        pColorList = static_cast<SvxColorListItem*>( pPoolItemHint->GetObject() )->GetColorList();
        FillValueSet();
    }
}

void SvxColorDockingWindow::FillValueSet()
{
    if( !pColorList.is() )
        return;

    nCount = pColorList->Count();
    aColorSet->Clear();

    // create the first entry for 'invisible/none'
    const Size aColorSize(SvxColorValueSet::getEntryEdgeLength(), SvxColorValueSet::getEntryEdgeLength());
    long nPtX = aColorSize.Width() - 1;
    long nPtY = aColorSize.Height() - 1;
    ScopedVclPtrInstance< VirtualDevice > pVD;

    pVD->SetOutputSizePixel( aColorSize );
    pVD->SetLineColor( COL_BLACK );
    pVD->SetBackground( Wallpaper( COL_WHITE ) );
    pVD->DrawLine( Point(), Point( nPtX, nPtY ) );
    pVD->DrawLine( Point( 0, nPtY ), Point( nPtX, 0 ) );

    BitmapEx aBmp( pVD->GetBitmapEx( Point(), aColorSize ) );

    aColorSet->InsertItem( sal_uInt16(1), Image(aBmp), SvxResId( RID_SVXSTR_INVISIBLE ) );

    aColorSet->addEntriesForXColorList(*pColorList, 2);
}

void SvxColorDockingWindow::SetSize()
{
    // calculate the size for ValueSet
    Size aSize = GetOutputSizePixel();
    aSize.AdjustWidth( -4 );
    aSize.AdjustHeight( -4 );

    // calculate rows and columns
    nCols = static_cast<sal_uInt16>( aSize.Width() / aItemSize.Width() );
    nLines = static_cast<sal_uInt16>( static_cast<float>(aSize.Height()) / static_cast<float>(aItemSize.Height()) /*+ 0.35*/ );
    if( nLines == 0 )
        nLines++;

    // set/remove scroll bar
    WinBits nBits = aColorSet->GetStyle();
    if ( static_cast<long>(nLines) * nCols >= nCount )
        nBits &= ~WB_VSCROLL;
    else
        nBits |= WB_VSCROLL;
    aColorSet->SetStyle( nBits );

    // scroll bar?
    long nScrollWidth = aColorSet->GetScrollWidth();
    if( nScrollWidth > 0 )
    {
        // calculate columns with scroll bar
        nCols = static_cast<sal_uInt16>( ( aSize.Width() - nScrollWidth ) / aItemSize.Width() );
    }
    aColorSet->SetColCount( nCols );

    if( IsFloatingMode() )
        aColorSet->SetLineCount( nLines );
    else
    {
        aColorSet->SetLineCount(); // otherwise line height is ignored
        aColorSet->SetItemHeight( aItemSize.Height() );
    }

    aColorSet->SetPosSizePixel( Point( 2, 2 ), aSize );
}

bool SvxColorDockingWindow::Close()
{
    SfxBoolItem aItem( SID_COLOR_CONTROL, false );
    GetBindings().GetDispatcher()->ExecuteList(SID_COLOR_CONTROL,
            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, { &aItem });
    SfxDockingWindow::Close();
    return true;
}

IMPL_LINK_NOARG(SvxColorDockingWindow, SelectHdl, ValueSet*, void)
{
    SfxDispatcher* pDispatcher = GetBindings().GetDispatcher();
    sal_uInt16 nPos = aColorSet->GetSelectedItemId();
    Color  aColor( aColorSet->GetItemColor( nPos ) );
    OUString aStr( aColorSet->GetItemText( nPos ) );

    if (aColorSet->IsLeftButton())
    {
        if ( gnLeftSlot == SID_ATTR_FILL_COLOR )
        {
            if ( nPos == 1 )        // invisible
            {
                XFillStyleItem aXFillStyleItem( drawing::FillStyle_NONE );
                pDispatcher->ExecuteList(gnLeftSlot, SfxCallMode::RECORD,
                        { &aXFillStyleItem });
            }
            else
            {
                bool bDone = false;

                // If we have a DrawView and we are in TextEdit mode, then
                // not the area color but the text color is assigned
                SfxViewShell* pViewSh = SfxViewShell::Current();
                if ( pViewSh )
                {
                    SdrView* pView = pViewSh->GetDrawView();
                    if ( pView && pView->IsTextEdit() )
                    {
                        SvxColorItem aTextColorItem( aColor, SID_ATTR_CHAR_COLOR );
                        pDispatcher->ExecuteList(SID_ATTR_CHAR_COLOR,
                                SfxCallMode::RECORD, { &aTextColorItem });
                        bDone = true;
                    }
                }
                if ( !bDone )
                {
                    XFillStyleItem aXFillStyleItem( drawing::FillStyle_SOLID );
                    XFillColorItem aXFillColorItem( aStr, aColor );
                    pDispatcher->ExecuteList(gnLeftSlot, SfxCallMode::RECORD,
                            { &aXFillColorItem, &aXFillStyleItem });
                }
            }
        }
        else if ( nPos != 1 )       // invisible
        {
            SvxColorItem aLeftColorItem( aColor, gnLeftSlot );
            pDispatcher->ExecuteList(gnLeftSlot, SfxCallMode::RECORD,
                    { &aLeftColorItem });
        }
    }
    else
    {
        if ( gnRightSlot == SID_ATTR_LINE_COLOR )
        {
            if( nPos == 1 )     // invisible
            {
                XLineStyleItem aXLineStyleItem( drawing::LineStyle_NONE );
                pDispatcher->ExecuteList(gnRightSlot, SfxCallMode::RECORD,
                        { &aXLineStyleItem });
            }
            else
            {
                // If the LineStyle is invisible, it is set to SOLID
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
                            drawing::LineStyle eXLS =
                                aAttrSet.Get( XATTR_LINESTYLE ).GetValue();
                            if ( eXLS == drawing::LineStyle_NONE )
                            {
                                XLineStyleItem aXLineStyleItem( drawing::LineStyle_SOLID );
                                pDispatcher->ExecuteList(gnRightSlot,
                                    SfxCallMode::RECORD, { &aXLineStyleItem });
                            }
                        }
                    }
                }

                XLineColorItem aXLineColorItem( aStr, aColor );
                pDispatcher->ExecuteList(gnRightSlot, SfxCallMode::RECORD,
                        { &aXLineColorItem });
            }
        }
        else if ( nPos != 1 )       // invisible
        {
            SvxColorItem aRightColorItem( aColor, gnRightSlot );
            pDispatcher->ExecuteList(gnRightSlot, SfxCallMode::RECORD,
                    { &aRightColorItem });
        }
    }
}

void SvxColorDockingWindow::Resizing( Size& rNewSize )
{
    rNewSize.AdjustWidth( -4 );
    rNewSize.AdjustHeight( -4 );

    // determine columns and rows
    nCols = static_cast<sal_uInt16>( static_cast<float>(rNewSize.Width()) / static_cast<float>(aItemSize.Width()) + 0.5 );
    nLines = static_cast<sal_uInt16>( static_cast<float>(rNewSize.Height()) / static_cast<float>(aItemSize.Height()) + 0.5 );
    if( nLines == 0 )
        nLines = 1;

    // set/remove scroll bar
    WinBits nBits = aColorSet->GetStyle();
    if ( static_cast<long>(nLines) * nCols >= nCount )
        nBits &= ~WB_VSCROLL;
    else
        nBits |= WB_VSCROLL;
    aColorSet->SetStyle( nBits );

    // scroll bar?
    long nScrollWidth = aColorSet->GetScrollWidth();
    if( nScrollWidth > 0 )
    {
        // calculate columns with scroll bar
        nCols = static_cast<sal_uInt16>( ( static_cast<float>(rNewSize.Width()) - static_cast<float>(nScrollWidth) )
                               / static_cast<float>(aItemSize.Width()) + 0.5 );
    }
    if( nCols <= 1 )
        nCols = 2;

    // calculate max. rows using the given columns
    long nMaxLines = nCount / nCols;
    if( nCount %  nCols )
        nMaxLines++;

    nLines = sal::static_int_cast< sal_uInt16 >(
        std::min< long >( nLines, nMaxLines ) );

    // set size of the window
    rNewSize.setWidth( nCols * aItemSize.Width() + nScrollWidth + 4 );
    rNewSize.setHeight( nLines * aItemSize.Height() + 4 );
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

bool SvxColorDockingWindow::EventNotify( NotifyEvent& rNEvt )
{
    bool bRet = false;
    if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
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

    return bRet || SfxDockingWindow::EventNotify(rNEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
