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
#include <sfx2/module.hxx>
#include <svx/xcolit.hxx>

#include <unotools/viewoptions.hxx>
#include <svtools/unitconv.hxx>

#include <sdattr.hrc>
#include <View.hxx>
#include <drawdoc.hxx>


namespace sd {

#define TOKEN ';'

CopyDlg::CopyDlg(weld::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pInView)
    : SfxDialogController(pWindow, "modules/sdraw/ui/copydlg.ui", "DuplicateDialog")
    , mrOutAttrs(rInAttrs)
    , maUIScale(pInView->GetDoc().GetUIScale())
    , mpView(pInView)
    , m_xNumFldCopies(m_xBuilder->weld_spin_button("copies"))
    , m_xBtnSetViewData(m_xBuilder->weld_button("viewdata"))
    , m_xMtrFldMoveX(m_xBuilder->weld_metric_spin_button("x", FieldUnit::CM))
    , m_xMtrFldMoveY(m_xBuilder->weld_metric_spin_button("y", FieldUnit::CM))
    , m_xMtrFldAngle(m_xBuilder->weld_metric_spin_button("angle", FieldUnit::DEGREE))
    , m_xMtrFldWidth(m_xBuilder->weld_metric_spin_button("width", FieldUnit::CM))
    , m_xMtrFldHeight(m_xBuilder->weld_metric_spin_button("height", FieldUnit::CM))
    , m_xFtEndColor(m_xBuilder->weld_label("endlabel"))
    , m_xBtnSetDefault(m_xBuilder->weld_button("default"))
    , m_xLbStartColor(new ColorListBox(m_xBuilder->weld_menu_button("start"), pWindow))
    , m_xLbEndColor(new ColorListBox(m_xBuilder->weld_menu_button("end"), pWindow))
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
    SvtViewOptions aDlgOpt(EViewType::Dialog, OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
    OUString sStr =
        OUString::number(m_xNumFldCopies->get_value()) + OUString(TOKEN) +
        OUString::number(m_xMtrFldMoveX->get_value(FieldUnit::NONE)) + OUString(TOKEN) +
        OUString::number(m_xMtrFldMoveY->get_value(FieldUnit::NONE)) + OUString(TOKEN) +
        OUString::number(m_xMtrFldAngle->get_value(FieldUnit::NONE)) + OUString(TOKEN) +
        OUString::number(m_xMtrFldWidth->get_value(FieldUnit::NONE)) + OUString(TOKEN) +
        OUString::number(m_xMtrFldHeight->get_value(FieldUnit::NONE)) + OUString(TOKEN) +
        OUString::number(static_cast<sal_uInt32>(m_xLbStartColor->GetSelectEntryColor())) + OUString(TOKEN) +
        OUString::number(static_cast<sal_uInt32>(m_xLbEndColor->GetSelectEntryColor()));
    aDlgOpt.SetUserItem("UserItem", css::uno::makeAny(sStr));
}

/**
 * reads provided item set or evaluate ini string
 */
void CopyDlg::Reset()
{
    // Set Min/Max values
    ::tools::Rectangle aRect = mpView->GetAllMarkedRect();
    Size aPageSize = mpView->GetSdrPageView()->GetPage()->GetSize();
    auto const n1 = m_xMtrFldMoveX->normalize(long(1000000 / maUIScale));
    auto const n2 = m_xMtrFldMoveX->convert_value_from(n1, FieldUnit::MM_100TH);
    double fScaleFactor = m_xMtrFldMoveX->convert_value_to(n2, FieldUnit::NONE)/1000000.0;

    long nPageWidth  = aPageSize.Width()  * fScaleFactor;
    long nPageHeight = aPageSize.Height() * fScaleFactor;
    long nRectWidth  = aRect.GetWidth()   * fScaleFactor;
    long nRectHeight = aRect.GetHeight()  * fScaleFactor;
    m_xMtrFldMoveX->set_range(-nPageWidth, nPageWidth, FieldUnit::NONE);
    m_xMtrFldMoveY->set_range(-nPageHeight, nPageHeight, FieldUnit::NONE);
    m_xMtrFldWidth->set_range(-nRectWidth, nPageWidth, FieldUnit::NONE);
    m_xMtrFldHeight->set_range(-nRectHeight, nPageHeight, FieldUnit::NONE);

    const SfxPoolItem* pPoolItem = nullptr;
    OUString aStr;
    SvtViewOptions aDlgOpt(EViewType::Dialog, OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
    if (aDlgOpt.Exists())
    {
        css::uno::Any aUserItem = aDlgOpt.GetUserItem("UserItem");
        aUserItem >>= aStr;
    }

    if (aStr.isEmpty())
    {
        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_NUMBER, true, &pPoolItem ) )
            m_xNumFldCopies->set_value(static_cast<const SfxUInt16Item*>(pPoolItem)->GetValue());
        else
            m_xNumFldCopies->set_value(1);

        long nMoveX = 500;
        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_MOVE_X, true, &pPoolItem ) )
            nMoveX = static_cast<const SfxInt32Item*>( pPoolItem )->GetValue();
        SetMetricValue( *m_xMtrFldMoveX, long(nMoveX / maUIScale), MapUnit::Map100thMM);

        long nMoveY = 500;
        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_MOVE_Y, true, &pPoolItem ) )
            nMoveY = static_cast<const SfxInt32Item*>( pPoolItem )->GetValue();
        SetMetricValue( *m_xMtrFldMoveY, long(nMoveY / maUIScale), MapUnit::Map100thMM);

        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_ANGLE, true, &pPoolItem ) )
            m_xMtrFldAngle->set_value(static_cast<const SfxInt32Item*>( pPoolItem )->GetValue(), FieldUnit::NONE);
        else
            m_xMtrFldAngle->set_value(0, FieldUnit::NONE);

        long nWidth = 0;
        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_WIDTH, true, &pPoolItem ) )
            nWidth = static_cast<const SfxInt32Item*>( pPoolItem )->GetValue();
        SetMetricValue( *m_xMtrFldWidth, long(nWidth / maUIScale), MapUnit::Map100thMM);

        long nHeight = 0;
        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_HEIGHT, true, &pPoolItem ) )
            nHeight = static_cast<const SfxInt32Item*>( pPoolItem )->GetValue();
        SetMetricValue( *m_xMtrFldHeight, long(nHeight / maUIScale), MapUnit::Map100thMM);

        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_START_COLOR, true, &pPoolItem ) )
        {
            Color aColor = static_cast<const XColorItem*>( pPoolItem )->GetColorValue();
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
        m_xNumFldCopies->set_value(aStr.getToken(0, TOKEN, nIdx).toInt64());
        m_xMtrFldMoveX->set_value(aStr.getToken(0, TOKEN, nIdx).toInt64(), FieldUnit::NONE);
        m_xMtrFldMoveY->set_value(aStr.getToken(0, TOKEN, nIdx).toInt64(), FieldUnit::NONE);
        m_xMtrFldAngle->set_value(aStr.getToken(0, TOKEN, nIdx).toInt64(), FieldUnit::NONE);
        m_xMtrFldWidth->set_value(aStr.getToken(0, TOKEN, nIdx).toInt64(), FieldUnit::NONE);
        m_xMtrFldHeight->set_value(aStr.getToken(0, TOKEN, nIdx).toInt64(), FieldUnit::NONE);
        m_xLbStartColor->SelectEntry( Color( aStr.getToken(0, TOKEN, nIdx).toUInt32() ) );
        m_xLbEndColor->SelectEntry( Color( aStr.getToken(0, TOKEN, nIdx).toUInt32() ) );
    }

}

/**
 * fills provided item set with dialog box attributes
 */
void CopyDlg::GetAttr( SfxItemSet& rOutAttrs )
{
    long nMoveX = long( GetCoreValue( *m_xMtrFldMoveX, MapUnit::Map100thMM) * maUIScale);
    long nMoveY = long( GetCoreValue( *m_xMtrFldMoveY, MapUnit::Map100thMM) * maUIScale);
    long nHeight = long( GetCoreValue( *m_xMtrFldHeight, MapUnit::Map100thMM) * maUIScale);
    long nWidth  = long( GetCoreValue( *m_xMtrFldWidth, MapUnit::Map100thMM) * maUIScale);

    rOutAttrs.Put( SfxUInt16Item( ATTR_COPY_NUMBER, static_cast<sal_uInt16>(m_xNumFldCopies->get_value()) ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_MOVE_X, nMoveX ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_MOVE_Y, nMoveY ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_ANGLE, static_cast<sal_Int32>(m_xMtrFldAngle->get_value(FieldUnit::DEGREE)) ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_WIDTH, nWidth ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_HEIGHT, nHeight ) );

    NamedColor aColor = m_xLbStartColor->GetSelectedEntry();
    rOutAttrs.Put(XColorItem(ATTR_COPY_START_COLOR, aColor.second, aColor.first));
    aColor = m_xLbEndColor->GetSelectedEntry();
    rOutAttrs.Put(XColorItem(ATTR_COPY_END_COLOR, aColor.second, aColor.first));
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

    SetMetricValue( *m_xMtrFldMoveX, long( aRect.GetWidth() /
                                    maUIScale ), MapUnit::Map100thMM);
    SetMetricValue( *m_xMtrFldMoveY, long( aRect.GetHeight() /
                                    maUIScale ), MapUnit::Map100thMM);

    // sets color attribute
    const SfxPoolItem*  pPoolItem = nullptr;
    if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_START_COLOR, true, &pPoolItem ) )
    {
        Color aColor = static_cast<const XColorItem*>( pPoolItem )->GetColorValue();
        m_xLbStartColor->SelectEntry( aColor );
    }
}

/**
 * resets values to default
 */
IMPL_LINK_NOARG(CopyDlg, SetDefault, weld::Button&, void)
{
    m_xNumFldCopies->set_value(1);

    long nValue = 500;
    SetMetricValue( *m_xMtrFldMoveX, long(nValue / maUIScale), MapUnit::Map100thMM);
    SetMetricValue( *m_xMtrFldMoveY, long(nValue / maUIScale), MapUnit::Map100thMM);

    nValue = 0;
    m_xMtrFldAngle->set_value(nValue, FieldUnit::DEGREE);
    SetMetricValue( *m_xMtrFldWidth, long(nValue / maUIScale), MapUnit::Map100thMM);
    SetMetricValue( *m_xMtrFldHeight, long(nValue / maUIScale), MapUnit::Map100thMM);

    // set color attribute
    const SfxPoolItem*  pPoolItem = nullptr;
    if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_START_COLOR, true, &pPoolItem ) )
    {
        Color aColor = static_cast<const XColorItem*>( pPoolItem )->GetColorValue();
        m_xLbStartColor->SelectEntry( aColor );
        m_xLbEndColor->SelectEntry( aColor );
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
