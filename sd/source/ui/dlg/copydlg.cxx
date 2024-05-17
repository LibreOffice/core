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

#include <copydlg.hxx>
#include <svx/colorbox.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdangitm.hxx>
#include <sfx2/module.hxx>
#include <svx/xcolit.hxx>
#include <svl/intitem.hxx>

#include <unotools/viewoptions.hxx>
#include <svtools/unitconv.hxx>
#include <o3tl/string_view.hxx>

#include <sdattr.hrc>
#include <View.hxx>
#include <drawdoc.hxx>


namespace sd {

constexpr char TOKEN = ';';

CopyDlg::CopyDlg(weld::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pInView)
    : SfxDialogController(pWindow, u"modules/sdraw/ui/copydlg.ui"_ustr, u"DuplicateDialog"_ustr)
    , mrOutAttrs(rInAttrs)
    , maUIScale(pInView->GetDoc().GetUIScale())
    , mpView(pInView)
    , m_xNumFldCopies(m_xBuilder->weld_spin_button(u"copies"_ustr))
    , m_xBtnSetViewData(m_xBuilder->weld_button(u"viewdata"_ustr))
    , m_xMtrFldMoveX(m_xBuilder->weld_metric_spin_button(u"x"_ustr, FieldUnit::CM))
    , m_xMtrFldMoveY(m_xBuilder->weld_metric_spin_button(u"y"_ustr, FieldUnit::CM))
    , m_xMtrFldAngle(m_xBuilder->weld_metric_spin_button(u"angle"_ustr, FieldUnit::DEGREE))
    , m_xMtrFldWidth(m_xBuilder->weld_metric_spin_button(u"width"_ustr, FieldUnit::CM))
    , m_xMtrFldHeight(m_xBuilder->weld_metric_spin_button(u"height"_ustr, FieldUnit::CM))
    , m_xFtEndColor(m_xBuilder->weld_label(u"endlabel"_ustr))
    , m_xBtnSetDefault(m_xBuilder->weld_button(u"default"_ustr))
    , m_xLbStartColor(new ColorListBox(m_xBuilder->weld_menu_button(u"start"_ustr), [this]{ return m_xDialog.get(); } ))
    , m_xLbEndColor(new ColorListBox(m_xBuilder->weld_menu_button(u"end"_ustr), [this]{ return m_xDialog.get(); } ))
{
    m_xLbStartColor->SetSelectHdl( LINK( this, CopyDlg, SelectColorHdl ) );
    m_xBtnSetViewData->connect_clicked( LINK( this, CopyDlg, SetViewData ) );
    m_xBtnSetDefault->connect_clicked( LINK( this, CopyDlg, SetDefault ) );

    FieldUnit eFUnit( SfxModule::GetCurrentFieldUnit() );

    SetFieldUnit( *m_xMtrFldMoveX, eFUnit, true );
    SetFieldUnit( *m_xMtrFldMoveY, eFUnit, true );
    SetFieldUnit( *m_xMtrFldWidth, eFUnit, true );
    SetFieldUnit( *m_xMtrFldHeight, eFUnit, true );

    Reset();
}

CopyDlg::~CopyDlg()
{
    SvtViewOptions aDlgOpt(EViewType::Dialog, m_xDialog->get_help_id());
    OUString sStr =
        OUString::number(m_xNumFldCopies->get_value()) + OUStringChar(TOKEN) +
        OUString::number(m_xMtrFldMoveX->get_value(FieldUnit::NONE)) + OUStringChar(TOKEN) +
        OUString::number(m_xMtrFldMoveY->get_value(FieldUnit::NONE)) + OUStringChar(TOKEN) +
        OUString::number(m_xMtrFldAngle->get_value(FieldUnit::NONE)) + OUStringChar(TOKEN) +
        OUString::number(m_xMtrFldWidth->get_value(FieldUnit::NONE)) + OUStringChar(TOKEN) +
        OUString::number(m_xMtrFldHeight->get_value(FieldUnit::NONE)) + OUStringChar(TOKEN) +
        OUString::number(static_cast<sal_uInt32>(m_xLbStartColor->GetSelectEntryColor())) + OUStringChar(TOKEN) +
        OUString::number(static_cast<sal_uInt32>(m_xLbEndColor->GetSelectEntryColor()));
    aDlgOpt.SetUserItem(u"UserItem"_ustr, css::uno::Any(sStr));
}

/**
 * reads provided item set or evaluate ini string
 */
void CopyDlg::Reset()
{
    // Set Min/Max values
    ::tools::Rectangle aRect = mpView->GetAllMarkedRect();
    Size aPageSize = mpView->GetSdrPageView()->GetPage()->GetSize();

    // tdf#125011 draw/impress sizes are in mm_100th already, "normalize" to
    // decimal shift by number of decimal places the widgets are using (2) then
    // scale by the ui scaling factor
    auto nPageWidth = tools::Long(m_xMtrFldMoveX->normalize(aPageSize.Width()) / maUIScale);
    auto nPageHeight = tools::Long(m_xMtrFldMoveX->normalize(aPageSize.Height()) / maUIScale);
    auto nRectWidth = tools::Long(m_xMtrFldMoveX->normalize(aRect.GetWidth()) / maUIScale);
    auto nRectHeight = tools::Long(m_xMtrFldMoveX->normalize(aRect.GetHeight()) / maUIScale);

    m_xMtrFldMoveX->set_range(-nPageWidth, nPageWidth, FieldUnit::MM_100TH);
    m_xMtrFldMoveY->set_range(-nPageHeight, nPageHeight, FieldUnit::MM_100TH);
    m_xMtrFldWidth->set_range(-nRectWidth, nPageWidth, FieldUnit::MM_100TH);
    m_xMtrFldHeight->set_range(-nRectHeight, nPageHeight, FieldUnit::MM_100TH);

    OUString aStr;
    SvtViewOptions aDlgOpt(EViewType::Dialog, m_xDialog->get_help_id());
    if (aDlgOpt.Exists())
    {
        css::uno::Any aUserItem = aDlgOpt.GetUserItem(u"UserItem"_ustr);
        aUserItem >>= aStr;
    }

    if (aStr.isEmpty())
    {
        if( const SfxUInt16Item* pPoolItem = mrOutAttrs.GetItemIfSet( ATTR_COPY_NUMBER ) )
            m_xNumFldCopies->set_value(pPoolItem->GetValue());
        else
            m_xNumFldCopies->set_value(1);

        tools::Long nMoveX = 500;
        if( const SfxInt32Item* pPoolItem = mrOutAttrs.GetItemIfSet( ATTR_COPY_MOVE_X ) )
            nMoveX = pPoolItem->GetValue();
        SetMetricValue( *m_xMtrFldMoveX, tools::Long(nMoveX / maUIScale), MapUnit::Map100thMM);

        tools::Long nMoveY = 500;
        if( const SfxInt32Item* pPoolItem = mrOutAttrs.GetItemIfSet( ATTR_COPY_MOVE_Y ) )
            nMoveY = pPoolItem->GetValue();
        SetMetricValue( *m_xMtrFldMoveY, tools::Long(nMoveY / maUIScale), MapUnit::Map100thMM);

        if( const SdrAngleItem* pPoolItem = mrOutAttrs.GetItemIfSet( ATTR_COPY_ANGLE ) )
            m_xMtrFldAngle->set_value( pPoolItem->GetValue().get(), FieldUnit::NONE);
        else
            m_xMtrFldAngle->set_value(0, FieldUnit::NONE);

        tools::Long nWidth = 0;
        if( const SfxInt32Item* pPoolItem = mrOutAttrs.GetItemIfSet( ATTR_COPY_WIDTH ) )
            nWidth = pPoolItem->GetValue();
        SetMetricValue( *m_xMtrFldWidth, tools::Long(nWidth / maUIScale), MapUnit::Map100thMM);

        tools::Long nHeight = 0;
        if( const SfxInt32Item* pPoolItem = mrOutAttrs.GetItemIfSet( ATTR_COPY_HEIGHT ) )
            nHeight = pPoolItem->GetValue();
        SetMetricValue( *m_xMtrFldHeight, tools::Long(nHeight / maUIScale), MapUnit::Map100thMM);

        if( const XColorItem* pPoolItem = mrOutAttrs.GetItemIfSet( ATTR_COPY_START_COLOR ) )
        {
            Color aColor = pPoolItem->GetColorValue();
            m_xLbStartColor->SelectEntry( aColor );
            m_xLbEndColor->SelectEntry( aColor );
        }
        else
        {
            m_xLbStartColor->SetNoSelection();
            m_xLbEndColor->SetNoSelection();
            m_xLbEndColor->set_sensitive(false);
            m_xFtEndColor->set_sensitive(false);
        }
    }
    else
    {
        sal_Int32 nIdx {0};
        m_xNumFldCopies->set_value(o3tl::toInt64(o3tl::getToken(aStr, 0, TOKEN, nIdx)));
        m_xMtrFldMoveX->set_value(o3tl::toInt64(o3tl::getToken(aStr, 0, TOKEN, nIdx)), FieldUnit::NONE);
        m_xMtrFldMoveY->set_value(o3tl::toInt64(o3tl::getToken(aStr, 0, TOKEN, nIdx)), FieldUnit::NONE);
        m_xMtrFldAngle->set_value(o3tl::toInt64(o3tl::getToken(aStr, 0, TOKEN, nIdx)), FieldUnit::NONE);
        m_xMtrFldWidth->set_value(o3tl::toInt64(o3tl::getToken(aStr, 0, TOKEN, nIdx)), FieldUnit::NONE);
        m_xMtrFldHeight->set_value(o3tl::toInt64(o3tl::getToken(aStr, 0, TOKEN, nIdx)), FieldUnit::NONE);
        m_xLbStartColor->SelectEntry( Color( ColorTransparency, o3tl::toUInt32(o3tl::getToken(aStr, 0, TOKEN, nIdx)) ) );
        m_xLbEndColor->SelectEntry( Color( ColorTransparency, o3tl::toUInt32(o3tl::getToken(aStr, 0, TOKEN, nIdx)) ) );
    }

}

/**
 * fills provided item set with dialog box attributes
 */
void CopyDlg::GetAttr( SfxItemSet& rOutAttrs )
{
    tools::Long nMoveX = tools::Long( GetCoreValue( *m_xMtrFldMoveX, MapUnit::Map100thMM) * maUIScale);
    tools::Long nMoveY = tools::Long( GetCoreValue( *m_xMtrFldMoveY, MapUnit::Map100thMM) * maUIScale);
    tools::Long nHeight = tools::Long( GetCoreValue( *m_xMtrFldHeight, MapUnit::Map100thMM) * maUIScale);
    tools::Long nWidth  = tools::Long( GetCoreValue( *m_xMtrFldWidth, MapUnit::Map100thMM) * maUIScale);

    rOutAttrs.Put( SfxUInt16Item( ATTR_COPY_NUMBER, static_cast<sal_uInt16>(m_xNumFldCopies->get_value()) ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_MOVE_X, nMoveX ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_MOVE_Y, nMoveY ) );
    rOutAttrs.Put( SdrAngleItem( ATTR_COPY_ANGLE, Degree100(static_cast<sal_Int32>(m_xMtrFldAngle->get_value(FieldUnit::DEGREE))) ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_WIDTH, nWidth ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_HEIGHT, nHeight ) );

    NamedColor aColor = m_xLbStartColor->GetSelectedEntry();
    rOutAttrs.Put(XColorItem(ATTR_COPY_START_COLOR, aColor.m_aName, aColor.m_aColor));
    aColor = m_xLbEndColor->GetSelectedEntry();
    rOutAttrs.Put(XColorItem(ATTR_COPY_END_COLOR, aColor.m_aName, aColor.m_aColor));
}

/**
 * enables and selects end color LB
 */
IMPL_LINK_NOARG(CopyDlg, SelectColorHdl, ColorListBox&, void)
{
    const Color aColor = m_xLbStartColor->GetSelectEntryColor();

    if (!m_xLbEndColor->get_sensitive())
    {
        m_xLbEndColor->SelectEntry(aColor);
        m_xLbEndColor->set_sensitive(true);
        m_xFtEndColor->set_sensitive(true);
    }
}

/**
 * sets values of selection
 */
IMPL_LINK_NOARG(CopyDlg, SetViewData, weld::Button&, void)
{
    ::tools::Rectangle aRect = mpView->GetAllMarkedRect();

    SetMetricValue( *m_xMtrFldMoveX, tools::Long( aRect.GetWidth() /
                                    maUIScale ), MapUnit::Map100thMM);
    SetMetricValue( *m_xMtrFldMoveY, tools::Long( aRect.GetHeight() /
                                    maUIScale ), MapUnit::Map100thMM);

    // sets color attribute
    if( const XColorItem* pPoolItem = mrOutAttrs.GetItemIfSet( ATTR_COPY_START_COLOR ) )
    {
        Color aColor = pPoolItem->GetColorValue();
        m_xLbStartColor->SelectEntry( aColor );
    }
}

/**
 * resets values to default
 */
IMPL_LINK_NOARG(CopyDlg, SetDefault, weld::Button&, void)
{
    m_xNumFldCopies->set_value(1);

    tools::Long nValue = 500;
    SetMetricValue( *m_xMtrFldMoveX, tools::Long(nValue / maUIScale), MapUnit::Map100thMM);
    SetMetricValue( *m_xMtrFldMoveY, tools::Long(nValue / maUIScale), MapUnit::Map100thMM);

    nValue = 0;
    m_xMtrFldAngle->set_value(nValue, FieldUnit::DEGREE);
    SetMetricValue( *m_xMtrFldWidth, tools::Long(nValue / maUIScale), MapUnit::Map100thMM);
    SetMetricValue( *m_xMtrFldHeight, tools::Long(nValue / maUIScale), MapUnit::Map100thMM);

    // set color attribute
    if( const XColorItem* pPoolItem = mrOutAttrs.GetItemIfSet( ATTR_COPY_START_COLOR ) )
    {
        Color aColor = pPoolItem->GetColorValue();
        m_xLbStartColor->SelectEntry( aColor );
        m_xLbEndColor->SelectEntry( aColor );
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
