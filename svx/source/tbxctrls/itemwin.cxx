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
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/module.hxx>
#include <tools/urlobj.hxx>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>

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
#include <svtools/unitconv.hxx>
#include <svtools/valueset.hxx>

#include <boost/property_tree/json_parser.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

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
    ListBox( pParent, WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL | WB_TABSTOP ),
    nCurPos ( 0 ),
    bSelect ( false )
{
    Fill();
    SetSizePixel(get_preferred_size());
    SelectEntryPos( sal_Int32(drawing::FillStyle_SOLID) );
    Show();
}

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

    return ListBox::PreNotify( rNEvt );
}

bool SvxFillTypeBox::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = ListBox::EventNotify( rNEvt );

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

boost::property_tree::ptree SvxFillTypeBox::DumpAsPropertyTree()
{
    boost::property_tree::ptree aTree = ListBox::DumpAsPropertyTree();
    aTree.put("command", ".uno:FillStyle");
    return aTree;
}

void SvxFillTypeBox::Fill()
{
    SetUpdateMode( false );

    InsertEntry( SvxResId(RID_SVXSTR_INVISIBLE) );
    InsertEntry( SvxResId(RID_SVXSTR_COLOR) );
    InsertEntry( SvxResId(RID_SVXSTR_GRADIENT) );
    InsertEntry( SvxResId(RID_SVXSTR_HATCH) );
    InsertEntry( SvxResId(RID_SVXSTR_BITMAP) );
    InsertEntry( SvxResId(RID_SVXSTR_PATTERN) );

    AdaptDropDownLineCountToMaximum();
    SetUpdateMode( true );
}

void SvxFillTypeBox::Fill(weld::ComboBox& rListBox)
{
    rListBox.freeze();

    rListBox.append_text(SvxResId(RID_SVXSTR_INVISIBLE));
    rListBox.append_text(SvxResId(RID_SVXSTR_COLOR));
    rListBox.append_text(SvxResId(RID_SVXSTR_GRADIENT));
    rListBox.append_text(SvxResId(RID_SVXSTR_HATCH));
    rListBox.append_text(SvxResId(RID_SVXSTR_BITMAP));
    rListBox.append_text(SvxResId(RID_SVXSTR_PATTERN));

    rListBox.thaw();

    rListBox.set_active(1); // solid color
}

SvxFillAttrBox::SvxFillAttrBox( vcl::Window* pParent ) :
    ListBox(pParent, WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL | WB_TABSTOP),
    nCurPos( 0 )
{
    SetPosPixel( Point( 90, 0 ) );
    SetSizePixel(LogicToPixel(Size(50, 80), MapMode(MapUnit::MapAppFont)));
    Show();
}

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

void SvxFillAttrBox::Fill(weld::ComboBox& rBox, const XHatchListRef &pList)
{
    if( !pList.is() )
        return;

    long nCount = pList->Count();
    ScopedVclPtrInstance< VirtualDevice > pVD;
    rBox.freeze();

    for( long i = 0; i < nCount; i++ )
    {
        const XHatchEntry* pEntry = pList->GetHatch(i);
        const BitmapEx aBitmapEx = pList->GetUiBitmap( i );
        if( !aBitmapEx.IsEmpty() )
        {
            const Size aBmpSize(aBitmapEx.GetSizePixel());
            pVD->SetOutputSizePixel(aBmpSize, false);
            pVD->DrawBitmapEx(Point(), aBitmapEx);
            rBox.append("", pEntry->GetName(), *pVD);
        }
        else
            rBox.append_text(pEntry->GetName());
    }

    rBox.thaw();
}

void SvxFillAttrBox::Fill(weld::ComboBox& rBox, const XGradientListRef &pList)
{
    if( !pList.is() )
        return;

    long nCount = pList->Count();
    ScopedVclPtrInstance< VirtualDevice > pVD;
    rBox.freeze();

    for( long i = 0; i < nCount; i++ )
    {
        const XGradientEntry* pEntry = pList->GetGradient(i);
        const BitmapEx aBitmapEx = pList->GetUiBitmap( i );
        if( !aBitmapEx.IsEmpty() )
        {
            const Size aBmpSize(aBitmapEx.GetSizePixel());
            pVD->SetOutputSizePixel(aBmpSize, false);
            pVD->DrawBitmapEx(Point(), aBitmapEx);
            rBox.append("", pEntry->GetName(), *pVD);
        }
        else
            rBox.append_text(pEntry->GetName());
    }

    rBox.thaw();
}

void SvxFillAttrBox::Fill(weld::ComboBox& rBox, const XBitmapListRef &pList)
{
    if( !pList.is() )
        return;

    long nCount = pList->Count();
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Size aSize(rStyleSettings.GetListBoxPreviewDefaultPixelSize());
    ScopedVclPtrInstance< VirtualDevice > pVD;
    pVD->SetOutputSizePixel(aSize, false);
    rBox.freeze();

    for( long i = 0; i < nCount; i++ )
    {
        const XBitmapEntry* pEntry = pList->GetBitmap( i );
        BitmapEx aBitmapEx = pEntry->GetGraphicObject().GetGraphic().GetBitmapEx();
        formatBitmapExToSize(aBitmapEx, aSize);
        pVD->DrawBitmapEx(Point(), aBitmapEx);
        rBox.append("", pEntry->GetName(), *pVD);
    }

    rBox.thaw();
}

void SvxFillAttrBox::Fill(weld::ComboBox& rBox, const XPatternListRef &pList)
{
    if( !pList.is() )
        return;

    long nCount = pList->Count();
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Size aSize(rStyleSettings.GetListBoxPreviewDefaultPixelSize());
    ScopedVclPtrInstance< VirtualDevice > pVD;
    pVD->SetOutputSizePixel(aSize, false);
    rBox.freeze();

    for( long i = 0; i < nCount; i++ )
    {
        const XBitmapEntry* pEntry = pList->GetBitmap( i );
        BitmapEx aBitmapEx = pEntry->GetGraphicObject().GetGraphic().GetBitmapEx();
        formatBitmapExToSize(aBitmapEx, aSize);
        pVD->DrawBitmapEx(Point(), aBitmapEx);
        rBox.append("", pEntry->GetName(), *pVD);
    }

    rBox.thaw();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
