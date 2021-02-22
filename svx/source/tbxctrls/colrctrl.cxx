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
#include <svx/strings.hrc>
#include <svx/svxids.hrc>

#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/image.hxx>
#include <vcl/transfer.hxx>

#include <colrctrl.hxx>

#include <svx/svdview.hxx>
#include <svx/drawitem.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <editeng/colritem.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xtable.hxx>
#include <svx/dialmgr.hxx>
#include <helpids.h>
#include <vcl/virdev.hxx>

#include <com/sun/star/beans/NamedValue.hpp>

using namespace com::sun::star;

class SvxColorValueSetData final : public TransferDataContainer
{
private:
    uno::Sequence<beans::NamedValue> m_Data;

    virtual void AddSupportedFormats() override;
    virtual bool GetData(const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc) override;

public:
    SvxColorValueSetData()
    {
    }

    void SetData(const uno::Sequence<beans::NamedValue>& rData)
    {
        m_Data = rData;
        ClearFormats(); // invalidate m_aAny so new data will take effect
    }
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

void SvxColorValueSet_docking::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    SvxColorValueSet::SetDrawingArea(pDrawingArea);
    SetAccessibleName(SvxResId(STR_COLORTABLE));
    SetStyle(GetStyle() | WB_ITEMBORDER);

    m_xHelper.set(new SvxColorValueSetData);
    rtl::Reference<TransferDataContainer> xHelper(m_xHelper);
    SetDragDataTransferrable(xHelper, DND_ACTION_COPY);
}

SvxColorValueSet_docking::SvxColorValueSet_docking(std::unique_ptr<weld::ScrolledWindow> xWindow)
    : SvxColorValueSet(std::move(xWindow))
    , mbLeftButton(true)
{
}

bool SvxColorValueSet_docking::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bRet;

    // For Mac still handle differently!
    if( rMEvt.IsLeft() )
    {
        mbLeftButton = true;
        bRet = SvxColorValueSet::MouseButtonDown( rMEvt );
    }
    else
    {
        mbLeftButton = false;
        MouseEvent aMEvt( rMEvt.GetPosPixel(),
                          rMEvt.GetClicks(),
                          rMEvt.GetMode(),
                          MOUSE_LEFT,
                          rMEvt.GetModifier() );
        bRet = SvxColorValueSet::MouseButtonDown( aMEvt );
    }

    return bRet;
}

bool SvxColorValueSet_docking::MouseButtonUp( const MouseEvent& rMEvt )
{
    bool bRet;

    // For Mac still handle differently!
    if( rMEvt.IsLeft() )
    {
        mbLeftButton = true;
        bRet = SvxColorValueSet::MouseButtonUp( rMEvt );
    }
    else
    {
        mbLeftButton = false;
        MouseEvent aMEvt( rMEvt.GetPosPixel(),
                          rMEvt.GetClicks(),
                          rMEvt.GetMode(),
                          MOUSE_LEFT,
                          rMEvt.GetModifier() );
        bRet = SvxColorValueSet::MouseButtonUp( aMEvt );
    }
    SetNoSelection();

    return bRet;
}

bool SvxColorValueSet_docking::StartDrag()
{
    sal_uInt16 nPos = GetSelectedItemId();
    Color aItemColor( GetItemColor( nPos ) );
    OUString sItemText( GetItemText( nPos ) );

    drawing::FillStyle eStyle = ((1 == nPos)
                            ? drawing::FillStyle_NONE
                            : drawing::FillStyle_SOLID);

    uno::Sequence<beans::NamedValue> props(2);
    XFillColorItem const color(sItemText, aItemColor);
    props[0].Name = "FillColor";
    color.QueryValue(props[0].Value, 0);
    XFillStyleItem const style(eStyle);
    props[1].Name = "FillStyle";
    style.QueryValue(props[1].Value, 0);

    m_xHelper->SetData(props);

    return false;
}

constexpr sal_uInt16 gnLeftSlot = SID_ATTR_FILL_COLOR;
constexpr sal_uInt16 gnRightSlot = SID_ATTR_LINE_COLOR;

SvxColorDockingWindow::SvxColorDockingWindow(SfxBindings* _pBindings, SfxChildWindow* pCW, vcl::Window* _pParent)
    : SfxDockingWindow(_pBindings, pCW, _pParent,
        "DockingColorWindow", "svx/ui/dockingcolorwindow.ui")
    , pColorList()
    , xColorSet(new SvxColorValueSet_docking(m_xBuilder->weld_scrolled_window("valuesetwin", true)))
    , xColorSetWin(new weld::CustomWeld(*m_xBuilder, "valueset", *xColorSet))
{
    SetText(SvxResId(STR_COLORTABLE));
    SetQuickHelpText(SvxResId(RID_SVXSTR_COLORBAR));
    SetSizePixel(LogicToPixel(Size(150, 22), MapMode(MapUnit::MapAppFont)));
    SetHelpId(HID_CTRL_COLOR);

    xColorSet->SetSelectHdl( LINK( this, SvxColorDockingWindow, SelectHdl ) );
    xColorSet->SetHelpId(HID_COLOR_CTL_COLORS);

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

    Size aItemSize = xColorSet->CalcItemSizePixel(Size(SvxColorValueSet::getEntryEdgeLength(), SvxColorValueSet::getEntryEdgeLength()));
    aItemSize.setWidth( aItemSize.Width() + SvxColorValueSet::getEntryEdgeLength() );
    aItemSize.setWidth( aItemSize.Width() / 2 );
    aItemSize.setHeight( aItemSize.Height() + SvxColorValueSet::getEntryEdgeLength() );
    aItemSize.setHeight( aItemSize.Height() / 2 );

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
    xColorSetWin.reset();
    xColorSet.reset();
    SfxDockingWindow::dispose();
}

void SvxColorDockingWindow::Notify( SfxBroadcaster& , const SfxHint& rHint )
{
    const SfxPoolItemHint* pPoolItemHint = dynamic_cast<const SfxPoolItemHint*>(&rHint);
    if ( pPoolItemHint )
        if (auto pColorListItem = dynamic_cast<const SvxColorListItem*>(pPoolItemHint->GetObject()))
        {
            // The list of colors has changed
            pColorList = pColorListItem->GetColorList();
            FillValueSet();
        }
}

void SvxColorDockingWindow::FillValueSet()
{
    if( !pColorList.is() )
        return;

    xColorSet->Clear();

    xColorSet->addEntriesForXColorList(*pColorList, 2);

    // create the last entry for 'invisible/none'
    const Size aColorSize(SvxColorValueSet::getEntryEdgeLength(), SvxColorValueSet::getEntryEdgeLength());
    tools::Long nPtX = aColorSize.Width() - 1;
    tools::Long nPtY = aColorSize.Height() - 1;
    ScopedVclPtrInstance< VirtualDevice > pVD;

    pVD->SetOutputSizePixel( aColorSize );
    pVD->SetLineColor( COL_BLACK );
    pVD->SetBackground( Wallpaper( COL_WHITE ) );
    pVD->DrawLine( Point(), Point( nPtX, nPtY ) );
    pVD->DrawLine( Point( 0, nPtY ), Point( nPtX, 0 ) );

    BitmapEx aBmp( pVD->GetBitmapEx( Point(), aColorSize ) );

    xColorSet->InsertItem( sal_uInt16(1), Image(aBmp), SvxResId( RID_SVXSTR_INVISIBLE ) );
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
    sal_uInt16 nPos = xColorSet->GetSelectedItemId();
    Color  aColor( xColorSet->GetItemColor( nPos ) );
    OUString aStr( xColorSet->GetItemText( nPos ) );

    if (xColorSet->IsLeftButton())
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

void SvxColorDockingWindow::GetFocus()
{
    SfxDockingWindow::GetFocus();
    if (xColorSet)
    {
        // Grab the focus to the color value set so that it can be controlled
        // with the keyboard.
        xColorSet->GrabFocus();
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
