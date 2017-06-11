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
#include <svx/sidebar/LineWidthPopup.hxx>
#include <svx/sidebar/LinePropertyPanelBase.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/xlnwtit.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/svapp.hxx>
#include "LineWidthValueSet.hxx"
#include "bitmaps.hlst"

namespace svx { namespace sidebar {

LineWidthPopup::LineWidthPopup(LinePropertyPanelBase& rParent)
    : FloatingWindow(&rParent, "FloatingLineProperty", "svx/ui/floatinglineproperty.ui")
    , m_rParent(rParent)
    , m_pStr(nullptr)
    , m_sPt(SvxResId(RID_SVXSTR_PT))
    , m_eMapUnit(MapUnit::MapTwip)
    , m_bVSFocus(true)
    , m_bCustom(false)
    , m_nCustomWidth(0)
    , m_aIMGCus(BitmapEx(RID_SVXBMP_WIDTH_CUSTOM))
    , m_aIMGCusGray(BitmapEx(RID_SVXBMP_WIDTH_CUSTOM_GRAY))
{
    get(m_xMFWidth, "spin");

    get(m_xBox, "box");

    m_xVSWidth = VclPtr<LineWidthValueSet>::Create(m_xBox);

    m_xVSWidth->SetStyle(m_xVSWidth->GetStyle()| WB_3DLOOK |  WB_NO_DIRECTSELECT);

    m_pStr = new OUString[9];

    m_pStr[0] = "0.5";
    m_pStr[1] = "0.8";
    m_pStr[2] = "1.0";
    m_pStr[3] = "1.5";
    m_pStr[4] = "2.3";
    m_pStr[5] = "3.0";
    m_pStr[6] = "4.5";
    m_pStr[7] = "6.0";
    m_pStr[8] = SvxResId(RID_SVXSTR_WIDTH_LAST_CUSTOM);

    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    for(int i = 0; i <= 7 ; i++)
    {
        m_pStr[i] = m_pStr[i].replace('.', cSep);//Modify
        m_pStr[i] += " ";
        m_pStr[i] += m_sPt;
    }

    for (sal_uInt16 i = 1 ; i <= 9; ++i)
    {
        m_xVSWidth->InsertItem(i);
        m_xVSWidth->SetItemText(i, m_pStr[i-1]);
    }

    m_xVSWidth->SetUnit(m_pStr);
    m_xVSWidth->SetItemData(1, reinterpret_cast<void*>(5));
    m_xVSWidth->SetItemData(2, reinterpret_cast<void*>(8));
    m_xVSWidth->SetItemData(3, reinterpret_cast<void*>(10));
    m_xVSWidth->SetItemData(4, reinterpret_cast<void*>(15));
    m_xVSWidth->SetItemData(5, reinterpret_cast<void*>(23));
    m_xVSWidth->SetItemData(6, reinterpret_cast<void*>(30));
    m_xVSWidth->SetItemData(7, reinterpret_cast<void*>(45));
    m_xVSWidth->SetItemData(8, reinterpret_cast<void*>(60));
    m_xVSWidth->SetImage(m_aIMGCusGray);

    m_xVSWidth->SetSelItem(0);

    m_xVSWidth->SetSelectHdl(LINK(this, LineWidthPopup, VSSelectHdl));
    m_xMFWidth->SetModifyHdl(LINK(this, LineWidthPopup, MFModifyHdl));

    m_xVSWidth->StartSelection();
    m_xVSWidth->Show();
}

void LineWidthPopup::dispose()
{
    delete[] m_pStr;
    m_xVSWidth.disposeAndClear();
    m_xBox.clear();
    m_xMFWidth.clear();
    FloatingWindow::dispose();
}

LineWidthPopup::~LineWidthPopup()
{
    disposeOnce();
}

IMPL_LINK(LineWidthPopup, VSSelectHdl, ValueSet*, /*pControl*/, void)
{
    sal_uInt16 iPos = m_xVSWidth->GetSelectItemId();
    if (iPos >= 1 && iPos <= 8)
    {
        sal_IntPtr nVal = LogicToLogic(reinterpret_cast<sal_IntPtr>(m_xVSWidth->GetItemData( iPos )), MapUnit::MapPoint, m_eMapUnit);
        nVal = m_xMFWidth->Denormalize(nVal);
        XLineWidthItem aWidthItem( nVal );
        m_rParent.setLineWidth(aWidthItem);
        m_rParent.SetWidthIcon(iPos);
        m_rParent.SetWidth(nVal);
    }
    else if (iPos == 9)
    {//last custom
        //modified
        if (m_bCustom)
        {
            long nVal = LogicToLogic(m_nCustomWidth , MapUnit::MapPoint, m_eMapUnit);
            nVal = m_xMFWidth->Denormalize(nVal);
            XLineWidthItem aWidthItem( nVal );
            m_rParent.setLineWidth(aWidthItem);
            m_rParent.SetWidth(nVal);
        }
        else
        {
            m_xVSWidth->SetNoSelection();     //add , set no selection and keep the last select item
            m_xVSWidth->SetFormat();
            m_xVSWidth->Invalidate();
            Invalidate();
            m_xVSWidth->StartSelection();
        }
        //modify end
    }
    if ((iPos >= 1 && iPos <= 8) || (iPos == 9 && m_bCustom)) //add
    {
        EndPopupMode();
    }
}

IMPL_LINK(LineWidthPopup, MFModifyHdl, Edit&, /*rControl*/, void)
{
    if (m_xVSWidth->GetSelItem())
    {
        m_xVSWidth->SetSelItem(0);
        m_xVSWidth->SetFormat();
        m_xVSWidth->Invalidate();
        Invalidate();
        m_xVSWidth->StartSelection();
    }
    long nTmp = static_cast<long>(m_xMFWidth->GetValue());
    long nVal = LogicToLogic( nTmp, MapUnit::MapPoint, m_eMapUnit );
    sal_Int32 nNewWidth = (short)m_xMFWidth->Denormalize( nVal );
    XLineWidthItem aWidthItem(nNewWidth);
    m_rParent.setLineWidth(aWidthItem);
}

void LineWidthPopup::SetWidthSelect(long lValue, bool bValuable, MapUnit eMapUnit)
{
    m_bVSFocus = true;
    m_xVSWidth->SetSelItem(0);
    m_eMapUnit = eMapUnit;
    SvtViewOptions aWinOpt( EViewType::Window, SIDEBAR_LINE_WIDTH_GLOBAL_VALUE );
    if (aWinOpt.Exists())
    {
        css::uno::Sequence <css::beans::NamedValue> aSeq = aWinOpt.GetUserData();
        OUString aTmp;
        if ( aSeq.getLength())
            aSeq[0].Value >>= aTmp;

        OUString aWinData( aTmp );
        m_nCustomWidth = aWinData.toInt32();
        m_bCustom = true;
        m_xVSWidth->SetImage(m_aIMGCus);
        m_xVSWidth->SetCusEnable(true);

        OUString aStrTip( OUString::number( (double)m_nCustomWidth / 10));
        aStrTip += m_sPt;
        m_xVSWidth->SetItemText(9, aStrTip);
    }
    else
    {
        m_bCustom = false;
        m_xVSWidth->SetImage(m_aIMGCusGray);
        m_xVSWidth->SetCusEnable(false);
        m_xVSWidth->SetItemText(9, m_pStr[8]);
    }

    if (bValuable)
    {
        sal_Int64 nVal = OutputDevice::LogicToLogic(lValue, eMapUnit, MapUnit::Map100thMM );
        nVal = m_xMFWidth->Normalize(nVal);
        m_xMFWidth->SetValue( nVal, FUNIT_100TH_MM );
    }
    else
    {
        m_xMFWidth->SetText( "" );
    }

    OUString strCurrValue = m_xMFWidth->GetText();
    sal_uInt16 i = 0;
    for(; i < 8; i++)
    {
        if (strCurrValue == m_pStr[i])
        {
            m_xVSWidth->SetSelItem(i+1);
            break;
        }
    }

    if (i>=8)
    {
        m_bVSFocus = false;
        m_xVSWidth->SetSelItem(0);
    }

    m_xVSWidth->SetFormat();
    m_xVSWidth->Invalidate();
    m_xVSWidth->StartSelection();

    if (m_bVSFocus)
        m_xVSWidth->GrabFocus();
    else
        m_xMFWidth->GrabFocus();
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
