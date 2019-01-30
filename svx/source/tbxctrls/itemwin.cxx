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

#include <com/sun/star/drawing/FillStyle.hpp>

#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/module.hxx>
#include <tools/urlobj.hxx>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

#include <svx/svxids.hrc>

#define DELAY_TIMEOUT           100

#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <svx/dlgutil.hxx>
#include <svx/itemwin.hxx>
#include <svx/linectrl.hxx>
#include <svtools/colorcfg.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

SvxLineBox::SvxLineBox( vcl::Window* pParent, const Reference< XFrame >& rFrame ) :
    ListBox(pParent, WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL),
    nCurPos     ( 0 ),
    aLogicalSize(40,140),
    bRelease    ( true ),
    mpSh        ( nullptr ),
    mxFrame     ( rFrame )
{
    SetSizePixel(LogicToPixel(aLogicalSize, MapMode(MapUnit::MapAppFont)));
    Show();

    aDelayTimer.SetTimeout( DELAY_TIMEOUT );
    aDelayTimer.SetInvokeHandler( LINK( this, SvxLineBox, DelayHdl_Impl ) );
    aDelayTimer.Start();
}


// Fills the listbox (provisional) with strings

void SvxLineBox::Fill( const XDashListRef &pList )
{
    Clear();

    if( !pList.is() )
        return;

    // entry for 'none'
    InsertEntry(pList->GetStringForUiNoLine());

    // entry for solid line
    InsertEntry(pList->GetStringForUiSolidLine(),
            Image(pList->GetBitmapForUISolidLine()));

    // entries for dashed lines

    long nCount = pList->Count();
    SetUpdateMode( false );

    for( long i = 0; i < nCount; i++ )
    {
        const XDashEntry* pEntry = pList->GetDash(i);
        const BitmapEx aBitmap = pList->GetUiBitmap( i );
        if( !aBitmap.IsEmpty() )
        {
            InsertEntry(pEntry->GetName(), Image(aBitmap));
        }
        else
            InsertEntry( pEntry->GetName() );
    }

    AdaptDropDownLineCountToMaximum();
    SetUpdateMode( true );
}

IMPL_LINK_NOARG(SvxLineBox, DelayHdl_Impl, Timer *, void)
{
    if ( GetEntryCount() == 0 )
    {
        mpSh = SfxObjectShell::Current();
        FillControl();
    }
}


void SvxLineBox::Select()
{
    // Call the parent's Select() member to trigger accessibility events.
    ListBox::Select();

    if ( IsTravelSelect() )
        return;

    drawing::LineStyle eXLS;
    sal_Int32 nPos = GetSelectedEntryPos();

    switch ( nPos )
    {
        case 0:
            eXLS = drawing::LineStyle_NONE;
            break;

        case 1:
            eXLS = drawing::LineStyle_SOLID;
            break;

        default:
        {
            eXLS = drawing::LineStyle_DASH;

            if ( nPos != LISTBOX_ENTRY_NOTFOUND &&
                 SfxObjectShell::Current()  &&
                 SfxObjectShell::Current()->GetItem( SID_DASH_LIST ) )
            {
                // LineDashItem will only be sent if it also has a dash.
                // Notify cares!
                SvxDashListItem const * pItem = SfxObjectShell::Current()->GetItem( SID_DASH_LIST );
                XLineDashItem aLineDashItem( GetSelectedEntry(),
                    pItem->GetDashList()->GetDash( nPos - 2 )->GetDash() );

                Any a;
                Sequence< PropertyValue > aArgs( 1 );
                aArgs[0].Name = "LineDash";
                aLineDashItem.QueryValue ( a );
                aArgs[0].Value = a;
                SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                             ".uno:LineDash",
                                             aArgs );
            }
        }
        break;
    }

    XLineStyleItem aLineStyleItem( eXLS );
    Any a;
    Sequence< PropertyValue > aArgs( 1 );
    aArgs[0].Name = "XLineStyle";
    aLineStyleItem.QueryValue ( a );
    aArgs[0].Value = a;
    SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                 ".uno:XLineStyle",
                                 aArgs );

    nCurPos = GetSelectedEntryPos();
    ReleaseFocus_Impl();
}


bool SvxLineBox::PreNotify( NotifyEvent& rNEvt )
{
    MouseNotifyEvent nType = rNEvt.GetType();

    switch(nType)
    {
        case MouseNotifyEvent::MOUSEBUTTONDOWN:
        case MouseNotifyEvent::GETFOCUS:
            nCurPos = GetSelectedEntryPos();
        break;
        case MouseNotifyEvent::LOSEFOCUS:
            SelectEntryPos(nCurPos);
        break;
        case MouseNotifyEvent::KEYINPUT:
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
            if( pKEvt->GetKeyCode().GetCode() == KEY_TAB)
            {
                bRelease = false;
                Select();
            }
        }
        break;
        default:
        break;
    }
    return ListBox::PreNotify( rNEvt );
}


bool SvxLineBox::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = ListBox::EventNotify( rNEvt );

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

        switch ( pKEvt->GetKeyCode().GetCode() )
        {
            case KEY_RETURN:
                Select();
                bHandled = true;
                break;

            case KEY_ESCAPE:
                SelectEntryPos( nCurPos );
                ReleaseFocus_Impl();
                bHandled = true;
                break;
        }
    }
    return bHandled;
}


void SvxLineBox::ReleaseFocus_Impl()
{
    if(!bRelease)
    {
        bRelease = true;
        return;
    }

    if( SfxViewShell::Current() )
    {
        vcl::Window* pShellWnd = SfxViewShell::Current()->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

void SvxLineBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        SetSizePixel(LogicToPixel(aLogicalSize, MapMode(MapUnit::MapAppFont)));
    }

    ListBox::DataChanged( rDCEvt );
}

void SvxLineBox::FillControl()
{
    // FillStyles();
    if ( !mpSh )
        mpSh = SfxObjectShell::Current();

    if( mpSh )
    {
        const SvxDashListItem* pItem = mpSh->GetItem( SID_DASH_LIST );
        if ( pItem )
            Fill( pItem->GetDashList() );
    }
}

SvxMetricField::SvxMetricField(
    vcl::Window* pParent, const Reference< XFrame >& rFrame )
    : MetricField(pParent, WB_BORDER | WB_SPIN | WB_REPEAT)
    , aCurTxt()
    , ePoolUnit(MapUnit::MapCM)
    , mxFrame(rFrame)
{
    Size aSize( CalcMinimumSize() );
    SetSizePixel( aSize );
    aLogicalSize = PixelToLogic(aSize, MapMode(MapUnit::MapAppFont));
    SetUnit( FieldUnit::MM );
    SetDecimalDigits( 2 );
    SetMax( 5000 );
    SetMin( 0 );
    SetLast( 5000 );
    SetFirst( 0 );

    eDlgUnit = SfxModule::GetModuleFieldUnit( mxFrame );
    SetFieldUnit( *this, eDlgUnit );
    Show();
}


void SvxMetricField::Update( const XLineWidthItem* pItem )
{
    if ( pItem )
    {
        if ( pItem->GetValue() != GetCoreValue( *this, ePoolUnit ) )
            SetMetricValue( *this, pItem->GetValue(), ePoolUnit );
    }
    else
        SetText( "" );
}


void SvxMetricField::Modify()
{
    MetricField::Modify();
    long nTmp = GetCoreValue( *this, ePoolUnit );
    XLineWidthItem aLineWidthItem( nTmp );

    Any a;
    Sequence< PropertyValue > aArgs( 1 );
    aArgs[0].Name = "LineWidth";
    aLineWidthItem.QueryValue( a );
    aArgs[0].Value = a;
    SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                 ".uno:LineWidth",
                                 aArgs );
}


void SvxMetricField::ReleaseFocus_Impl()
{
    if( SfxViewShell::Current() )
    {
        vcl::Window* pShellWnd = SfxViewShell::Current()->GetWindow();
        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

void SvxMetricField::SetCoreUnit( MapUnit eUnit )
{
    ePoolUnit = eUnit;
}

void SvxMetricField::RefreshDlgUnit()
{
    FieldUnit eTmpUnit = SfxModule::GetModuleFieldUnit( mxFrame );
    if ( eDlgUnit != eTmpUnit )
    {
        eDlgUnit = eTmpUnit;
        SetFieldUnit( *this, eDlgUnit );
    }
}

bool SvxMetricField::PreNotify( NotifyEvent& rNEvt )
{
    MouseNotifyEvent nType = rNEvt.GetType();

    if ( MouseNotifyEvent::MOUSEBUTTONDOWN == nType || MouseNotifyEvent::GETFOCUS == nType )
        aCurTxt = GetText();

    return MetricField::PreNotify( rNEvt );
}


bool SvxMetricField::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = MetricField::EventNotify( rNEvt );

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const vcl::KeyCode& rKey = pKEvt->GetKeyCode();
        SfxViewShell* pSh = SfxViewShell::Current();

        if ( rKey.GetModifier() && rKey.GetGroup() != KEYGROUP_CURSOR && pSh )
            (void)pSh->KeyInput( *pKEvt );
        else
        {
            bool bHandledInside = false;

            switch ( rKey.GetCode() )
            {
                case KEY_RETURN:
                    Reformat();
                    bHandledInside = true;
                    break;

                case KEY_ESCAPE:
                    SetText( aCurTxt );
                    bHandled = true;
                    break;
            }

            if ( bHandledInside )
            {
                bHandled = true;
                Modify();
                ReleaseFocus_Impl();
            }
        }
    }
    return bHandled;
}

void SvxMetricField::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        SetSizePixel(LogicToPixel(aLogicalSize, MapMode(MapUnit::MapAppFont)));
    }

    MetricField::DataChanged( rDCEvt );
}

SvxFillTypeBox::SvxFillTypeBox( vcl::Window* pParent ) :
    FillTypeLB( pParent, WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL | WB_TABSTOP ),
    nCurPos ( 0 ),
    bSelect ( false )
{
    Fill();
    SetSizePixel(get_preferred_size());
    SelectEntryPos( sal_Int32(drawing::FillStyle_SOLID) );
    Show();
}

VCL_BUILDER_FACTORY(SvxFillTypeBox)

bool SvxFillTypeBox::PreNotify( NotifyEvent& rNEvt )
{
    MouseNotifyEvent nType = rNEvt.GetType();

    if (!isDisposed())
    {
        if ( MouseNotifyEvent::MOUSEBUTTONDOWN == nType || MouseNotifyEvent::GETFOCUS == nType )
            nCurPos = GetSelectedEntryPos();
        else if ( MouseNotifyEvent::LOSEFOCUS == nType
                  && Application::GetFocusWindow()
                  && !IsWindowOrChild( Application::GetFocusWindow(), true ) )
        {
            if ( !bSelect )
                SelectEntryPos( nCurPos );
            else
                bSelect = false;
        }
    }

    return FillTypeLB::PreNotify( rNEvt );
}


bool SvxFillTypeBox::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = FillTypeLB::EventNotify( rNEvt );

    if (isDisposed())
        return false;

    if ( !bHandled && rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        switch ( pKEvt->GetKeyCode().GetCode() )
        {
            case KEY_RETURN:
                bHandled = true;
                GetSelectHdl().Call( *this );
            break;

            case KEY_TAB:
                GetSelectHdl().Call( *this );
                break;

            case KEY_ESCAPE:
                SelectEntryPos( nCurPos );
                ReleaseFocus_Impl();
                bHandled = true;
                break;
        }
    }
    return bHandled;
}


void SvxFillTypeBox::ReleaseFocus_Impl()
{
    if( SfxViewShell::Current() )
    {
        vcl::Window* pShellWnd = SfxViewShell::Current()->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

SvxFillAttrBox::SvxFillAttrBox( vcl::Window* pParent ) :
    ListBox(pParent, WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL | WB_TABSTOP),
    nCurPos( 0 )
{
    SetPosPixel( Point( 90, 0 ) );
    SetSizePixel(LogicToPixel(Size(50, 80), MapMode(MapUnit::MapAppFont)));
    Show();
}

VCL_BUILDER_FACTORY(SvxFillAttrBox)

bool SvxFillAttrBox::PreNotify( NotifyEvent& rNEvt )
{
    MouseNotifyEvent nType = rNEvt.GetType();

    if ( MouseNotifyEvent::MOUSEBUTTONDOWN == nType || MouseNotifyEvent::GETFOCUS == nType )
        nCurPos = GetSelectedEntryPos();

    return ListBox::PreNotify( rNEvt );
}


bool SvxFillAttrBox::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = ListBox::EventNotify( rNEvt );

    if ( !bHandled && rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

        switch ( pKEvt->GetKeyCode().GetCode() )
        {
            case KEY_RETURN:
                GetSelectHdl().Call( *this );
                bHandled = true;
            break;
            case KEY_TAB:
                GetSelectHdl().Call( *this );
            break;
            case KEY_ESCAPE:
                SelectEntryPos( nCurPos );
                ReleaseFocus_Impl();
                bHandled = true;
                break;
        }
    }
    return bHandled;
}


void SvxFillAttrBox::ReleaseFocus_Impl()
{
    if( SfxViewShell::Current() )
    {
        vcl::Window* pShellWnd = SfxViewShell::Current()->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

// Fills the listbox (provisional) with strings

void SvxFillAttrBox::Fill( const XHatchListRef &pList )
{
    long nCount = pList->Count();
    ListBox::SetUpdateMode( false );

    for( long i = 0; i < nCount; i++ )
    {
        const XHatchEntry* pEntry = pList->GetHatch(i);
        const BitmapEx aBitmap = pList->GetUiBitmap( i );
        if( !aBitmap.IsEmpty() )
            ListBox::InsertEntry(pEntry->GetName(), Image(aBitmap));
        else
            InsertEntry( pEntry->GetName() );
    }

    AdaptDropDownLineCountToMaximum();
    ListBox::SetUpdateMode( true );
}

// Fills the listbox (provisional) with strings

void SvxFillAttrBox::Fill( const XGradientListRef &pList )
{
    long nCount = pList->Count();
    ListBox::SetUpdateMode( false );

    for( long i = 0; i < nCount; i++ )
    {
        const XGradientEntry* pEntry = pList->GetGradient(i);
        const BitmapEx aBitmap = pList->GetUiBitmap( i );
        if( !aBitmap.IsEmpty() )
            ListBox::InsertEntry(pEntry->GetName(), Image(aBitmap));
        else
            InsertEntry( pEntry->GetName() );
    }

    AdaptDropDownLineCountToMaximum();
    ListBox::SetUpdateMode( true );
}

namespace
{
    void formatBitmapExToSize(BitmapEx& rBitmapEx, const Size& rSize)
    {
        if(!rBitmapEx.IsEmpty() && rSize.Width() > 0 && rSize.Height() > 0)
        {
            ScopedVclPtrInstance< VirtualDevice > pVirtualDevice;
            pVirtualDevice->SetOutputSizePixel(rSize);

            if(rBitmapEx.IsTransparent())
            {
                const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

                if(rStyleSettings.GetPreviewUsesCheckeredBackground())
                {
                    const Point aNull(0, 0);
                    static const sal_uInt32 nLen(8);
                    static const Color aW(COL_WHITE);
                    static const Color aG(0xef, 0xef, 0xef);

                    pVirtualDevice->DrawCheckered(aNull, rSize, nLen, aW, aG);
                }
                else
                {
                    pVirtualDevice->SetBackground(rStyleSettings.GetFieldColor());
                    pVirtualDevice->Erase();
                }
            }

            if(rBitmapEx.GetSizePixel().Width() >= rSize.Width() && rBitmapEx.GetSizePixel().Height() >= rSize.Height())
            {
                rBitmapEx.Scale(rSize);
                pVirtualDevice->DrawBitmapEx(Point(0, 0), rBitmapEx);
            }
            else
            {
                const Size aBitmapSize(rBitmapEx.GetSizePixel());

                for(long y(0); y < rSize.Height(); y += aBitmapSize.Height())
                {
                    for(long x(0); x < rSize.Width(); x += aBitmapSize.Width())
                    {
                        pVirtualDevice->DrawBitmapEx(
                            Point(x, y),
                            rBitmapEx);
                    }
                }
            }

            rBitmapEx = pVirtualDevice->GetBitmapEx(Point(0, 0), rSize);
        }
    }
} // end of anonymous namespace

void SvxFillAttrBox::Fill( const XBitmapListRef &pList )
{
    const long nCount(pList->Count());
    const XBitmapEntry* pEntry;
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Size aSize(rStyleSettings.GetListBoxPreviewDefaultPixelSize());

    ListBox::SetUpdateMode(false);

    for(long i(0); i < nCount; i++)
    {
        pEntry = pList->GetBitmap( i );
        maBitmapEx = pEntry->GetGraphicObject().GetGraphic().GetBitmapEx();
        formatBitmapExToSize(maBitmapEx, aSize);
        ListBox::InsertEntry(pEntry->GetName(), Image(maBitmapEx));
    }

    AdaptDropDownLineCountToMaximum();
    ListBox::SetUpdateMode(true);
}

void SvxFillAttrBox::Fill( const XPatternListRef &pList )
{
    const long nCount(pList->Count());
    const XBitmapEntry* pEntry;
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Size aSize(rStyleSettings.GetListBoxPreviewDefaultPixelSize());

    ListBox::SetUpdateMode(false);

    for(long i(0); i < nCount; i++)
    {
        pEntry = pList->GetBitmap( i );
        maBitmapEx = pEntry->GetGraphicObject().GetGraphic().GetBitmapEx();
        formatBitmapExToSize(maBitmapEx, aSize);
        ListBox::InsertEntry(pEntry->GetName(), Image(maBitmapEx));
    }

    AdaptDropDownLineCountToMaximum();
    ListBox::SetUpdateMode(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
