/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <comphelper/propertyvalue.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/module.hxx>

#include <vcl/event.hxx>
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
using namespace ::cpo::uno;
using namespace cpo::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

SvxMetricField::SvxMetricField(
    vcl::Window* pParent, const Reference< XFrame >& rFrame )
    : InterimItemWindow(pParent, u"svx/ui/metricfieldbox.ui"_ustr, u"MetricFieldBox"_ustr)
    , m_xWidget(m_xBuilder->weld_metric_spin_button(u"metricfield"_ustr, FieldUnit::MM))
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
        m_xWidget->set_text(u""_ustr);
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
        m_xWidget->set_text(u""_ustr);
}

IMPL_LINK_NOARG(SvxMetricField, ModifyHdl, weld::MetricSpinButton&, void)
{
    auto nTmp = GetCoreValue(*m_xWidget, eDestPoolUnit);
    XLineWidthItem aLineWidthItem( nTmp );

    Any a;
    aLineWidthItem.QueryValue( a );
    Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(u"LineWidth"_ustr, a) };
    SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                 u".uno:LineWidth"_ustr,
                                 aArgs );
}

void SvxMetricField::ReleaseFocus_Impl()
{
    if (const SfxViewShell* pViewShell = SfxViewShell::Current())
    {
        vcl::Window* pShellWnd = pViewShell->GetWindow();
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

namespace
{
// Appends each entry of the list with its name and its thumbnail, or with its name only when it
// has no thumbnail.
void FillWithThumbnails(weld::ComboBox& rBox, const XPropertyList& rList)
{
    const tools::Long nCount = rList.Count();
    ScopedVclPtrInstance<VirtualDevice> pVD;
    rBox.freeze();

    for (tools::Long i = 0; i < nCount; i++)
    {
        const OUString& rName = rList.Get(i)->GetName();
        const Bitmap aBitmap = rList.GetUiBitmap(i);
        if (!aBitmap.IsEmpty())
        {
            pVD->SetOutputSizePixel(aBitmap.GetSizePixel(), false);
            pVD->DrawBitmap(Point(), aBitmap);
            rBox.append(u""_ustr, rName, *pVD);
        }
        else
            rBox.append_text(rName);
    }

    rBox.thaw();
}
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
    rListBox.append_text(SvxResId(RID_SVXSTR_USE_BACKGROUND));

    rListBox.thaw();

    rListBox.set_active(1); // solid color
}

void SvxFillAttrBox::Fill(weld::ComboBox& rBox, const XHatchListRef &pList)
{
    if( !pList.is() )
        return;

    FillWithThumbnails(rBox, *pList);
}

void SvxFillAttrBox::Fill(weld::ComboBox& rBox, const XGradientListRef &pList)
{
    if( !pList.is() )
        return;

    FillWithThumbnails(rBox, *pList);
}

void SvxFillAttrBox::Fill(weld::ComboBox& rBox, const XBitmapListRef &pList)
{
    if( !pList.is() )
        return;

    FillWithThumbnails(rBox, *pList);
}

void SvxFillAttrBox::Fill(weld::ComboBox& rBox, const XPatternListRef &pList)
{
    if( !pList.is() )
        return;

    FillWithThumbnails(rBox, *pList);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
