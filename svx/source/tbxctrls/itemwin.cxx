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

#include <sfx2/tbxctrl.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/module.hxx>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>

#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>

#include <svx/xlnwtit.hxx>
#include <svx/xtable.hxx>
#include <svx/itemwin.hxx>
#include <svtools/unitconv.hxx>
#include "linemetricbox.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

SvxMetricField::SvxMetricField(
    vcl::Window* pParent, const Reference< XFrame >& rFrame )
    : InterimItemWindow(pParent, "svx/ui/metricfieldbox.ui", "MetricFieldBox")
    , m_xWidget(m_xBuilder->weld_metric_spin_button("metricfield", FieldUnit::MM))
    , nCurValue(0)
    , eDestPoolUnit(MapUnit::Map100thMM)
    , eDlgUnit(SfxModule::GetModuleFieldUnit(rFrame))
    , mxFrame(rFrame)
{
    InitControlBase(&m_xWidget->get_widget());

    m_xWidget->set_range(0, 5000, FieldUnit::NONE);
    m_xWidget->connect_value_changed(LINK(this, SvxMetricField, ModifyHdl));
    m_xWidget->connect_focus_in(LINK(this, SvxMetricField, FocusInHdl));
    m_xWidget->get_widget().connect_key_press(LINK(this, SvxMetricField, KeyInputHdl));

    SetFieldUnit(*m_xWidget, eDlgUnit);

    SetSizePixel(m_xWidget->get_preferred_size());
}

void SvxMetricField::dispose()
{
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

SvxMetricField::~SvxMetricField()
{
    disposeOnce();
}

void SvxMetricField::set_sensitive(bool bSensitive)
{
    Enable(bSensitive);
    m_xWidget->set_sensitive(bSensitive);
    if (!bSensitive)
        m_xWidget->set_text("");
}

void SvxMetricField::Update( const XLineWidthItem* pItem )
{
    if ( pItem )
    {
        // tdf#132169 we always get the value in MapUnit::Map100thMM but have
        // to set it in the core metric of the target application
        if (pItem->GetValue() != GetCoreValue(*m_xWidget, MapUnit::Map100thMM))
            SetMetricValue(*m_xWidget, pItem->GetValue(), MapUnit::Map100thMM);
    }
    else
        m_xWidget->set_text("");
}

IMPL_LINK_NOARG(SvxMetricField, ModifyHdl, weld::MetricSpinButton&, void)
{
    auto nTmp = GetCoreValue(*m_xWidget, eDestPoolUnit);
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

void SvxMetricField::SetDestCoreUnit( MapUnit eUnit )
{
    eDestPoolUnit = eUnit;
}

void SvxMetricField::RefreshDlgUnit()
{
    FieldUnit eTmpUnit = SfxModule::GetModuleFieldUnit( mxFrame );
    if ( eDlgUnit != eTmpUnit )
    {
        eDlgUnit = eTmpUnit;
        SetFieldUnit(*m_xWidget, eDlgUnit);
    }
}

IMPL_LINK_NOARG(SvxMetricField, FocusInHdl, weld::Widget&, void)
{
    nCurValue = m_xWidget->get_value(FieldUnit::NONE);
}

IMPL_LINK(SvxMetricField, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bHandled = false;

    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

    if (nCode == KEY_ESCAPE)
    {
        m_xWidget->set_value(nCurValue, FieldUnit::NONE);
        ModifyHdl(*m_xWidget);
        ReleaseFocus_Impl();
        bHandled = true;
    }

    return bHandled || ChildKeyInput(rKEvt);
}

void SvxMetricField::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        SetSizePixel(m_xWidget->get_preferred_size());
    }

    InterimItemWindow::DataChanged( rDCEvt );
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

namespace
{
    void formatBitmapExToSize(BitmapEx& rBitmapEx, const Size& rSize)
    {
        if(rBitmapEx.IsEmpty() || rSize.IsEmpty())
            return;

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
} // end of anonymous namespace

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
