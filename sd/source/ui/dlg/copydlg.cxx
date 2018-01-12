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
#include <comphelper/string.hxx>
#include <svx/colorbox.hxx>
#include <svx/dlgutil.hxx>
#include <sfx2/module.hxx>
#include <svx/xcolit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xenum.hxx>

#include <sfx2/app.hxx>

#include <sdattr.hxx>

#include <View.hxx>
#include <sdresid.hxx>
#include <drawdoc.hxx>


namespace sd {

#define TOKEN ';'

CopyDlg::CopyDlg(vcl::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pInView)
    : SfxModalDialog(pWindow, "DuplicateDialog", "modules/sdraw/ui/copydlg.ui")
    , mrOutAttrs(rInAttrs)
    , maUIScale(pInView->GetDoc().GetUIScale())
    , mpView(pInView)
{
    get(m_pNumFldCopies, "copies");
    get(m_pBtnSetViewData, "viewdata");
    get(m_pMtrFldMoveX, "x");
    get(m_pMtrFldMoveY, "y");
    get(m_pMtrFldAngle, "angle");
    get(m_pMtrFldWidth, "width");
    get(m_pMtrFldHeight, "height");
    get(m_pLbStartColor, "start");
    get(m_pFtEndColor, "endlabel");
    get(m_pLbEndColor, "end");
    get(m_pBtnSetDefault, "default");

    m_pLbStartColor->SetSelectHdl( LINK( this, CopyDlg, SelectColorHdl ) );
    m_pBtnSetViewData->SetClickHdl( LINK( this, CopyDlg, SetViewData ) );
    m_pBtnSetDefault->SetClickHdl( LINK( this, CopyDlg, SetDefault ) );

    FieldUnit eFUnit( SfxModule::GetCurrentFieldUnit() );

    SetFieldUnit( *m_pMtrFldMoveX, eFUnit, true );
    SetFieldUnit( *m_pMtrFldMoveY, eFUnit, true );
    SetFieldUnit( *m_pMtrFldWidth, eFUnit, true );
    SetFieldUnit( *m_pMtrFldHeight, eFUnit, true );

    Reset();
}

CopyDlg::~CopyDlg()
{
    disposeOnce();
}

void CopyDlg::dispose()
{
    OUString& rStr = GetExtraData();

    rStr = OUString::number(m_pNumFldCopies->GetValue());
    rStr += OUString(TOKEN);

    rStr += OUString::number(m_pMtrFldMoveX->GetValue());
    rStr += OUString( TOKEN );

    rStr += OUString::number(m_pMtrFldMoveY->GetValue());
    rStr += OUString( TOKEN );

    rStr += OUString::number(m_pMtrFldAngle->GetValue());
    rStr += OUString( TOKEN );

    rStr += OUString::number(m_pMtrFldWidth->GetValue());
    rStr += OUString( TOKEN );

    rStr += OUString::number(m_pMtrFldHeight->GetValue());
    rStr += OUString( TOKEN );

    rStr += OUString::number( m_pLbStartColor->GetSelectEntryColor().GetColor() );
    rStr += OUString( TOKEN );

    rStr += OUString::number( m_pLbEndColor->GetSelectEntryColor().GetColor() );

    m_pNumFldCopies.clear();
    m_pBtnSetViewData.clear();
    m_pMtrFldMoveX.clear();
    m_pMtrFldMoveY.clear();
    m_pMtrFldAngle.clear();
    m_pMtrFldWidth.clear();
    m_pMtrFldHeight.clear();
    m_pLbStartColor.clear();
    m_pFtEndColor.clear();
    m_pLbEndColor.clear();
    m_pBtnSetDefault.clear();
    SfxModalDialog::dispose();
}

/**
 * reads provided item set or evaluate ini string
 */
void CopyDlg::Reset()
{
    // Set Min/Max values
    ::tools::Rectangle aRect = mpView->GetAllMarkedRect();
    Size aPageSize = mpView->GetSdrPageView()->GetPage()->GetSize();
    SetMetricValue( *m_pMtrFldMoveX, long(1000000 / maUIScale), MapUnit::Map100thMM);
    double fScaleFactor = m_pMtrFldMoveX->GetValue()/1000000.0;

    long nPageWidth  = aPageSize.Width()  * fScaleFactor;
    long nPageHeight = aPageSize.Height() * fScaleFactor;
    long nRectWidth  = aRect.GetWidth()   * fScaleFactor;
    long nRectHeight = aRect.GetHeight()  * fScaleFactor;
    m_pMtrFldMoveX->SetMin( -nPageWidth );
    m_pMtrFldMoveX->SetMax(  nPageWidth );
    m_pMtrFldMoveY->SetMin( -nPageHeight );
    m_pMtrFldMoveY->SetMax(  nPageHeight );
    m_pMtrFldWidth->SetMin( -nRectWidth );
    m_pMtrFldWidth->SetMax(  nPageWidth );
    m_pMtrFldHeight->SetMin( -nRectHeight );
    m_pMtrFldHeight->SetMax(  nPageHeight );

    const SfxPoolItem* pPoolItem = nullptr;
    OUString aStr( GetExtraData() );

    if (comphelper::string::getTokenCount(aStr, TOKEN) < 8)
    {
        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_NUMBER, true, &pPoolItem ) )
            m_pNumFldCopies->SetValue( static_cast<const SfxUInt16Item*>( pPoolItem )->GetValue() );
        else
            m_pNumFldCopies->SetValue( 1 );

        long nMoveX = 500;
        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_MOVE_X, true, &pPoolItem ) )
            nMoveX = static_cast<const SfxInt32Item*>( pPoolItem )->GetValue();
        SetMetricValue( *m_pMtrFldMoveX, long(nMoveX / maUIScale), MapUnit::Map100thMM);

        long nMoveY = 500;
        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_MOVE_Y, true, &pPoolItem ) )
            nMoveY = static_cast<const SfxInt32Item*>( pPoolItem )->GetValue();
        SetMetricValue( *m_pMtrFldMoveY, long(nMoveY / maUIScale), MapUnit::Map100thMM);

        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_ANGLE, true, &pPoolItem ) )
            m_pMtrFldAngle->SetValue( static_cast<const SfxInt32Item*>( pPoolItem )->GetValue() );
        else
            m_pMtrFldAngle->SetValue( 0 );

        long nWidth = 0;
        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_WIDTH, true, &pPoolItem ) )
            nWidth = static_cast<const SfxInt32Item*>( pPoolItem )->GetValue();
        SetMetricValue( *m_pMtrFldWidth, long(nWidth / maUIScale), MapUnit::Map100thMM);

        long nHeight = 0;
        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_HEIGHT, true, &pPoolItem ) )
            nHeight = static_cast<const SfxInt32Item*>( pPoolItem )->GetValue();
        SetMetricValue( *m_pMtrFldHeight, long(nHeight / maUIScale), MapUnit::Map100thMM);

        if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_START_COLOR, true, &pPoolItem ) )
        {
            Color aColor = static_cast<const XColorItem*>( pPoolItem )->GetColorValue();
            m_pLbStartColor->SelectEntry( aColor );
            m_pLbEndColor->SelectEntry( aColor );
        }
        else
        {
            m_pLbStartColor->SetNoSelection();
            m_pLbEndColor->SetNoSelection();
            m_pLbEndColor->Disable();
            m_pFtEndColor->Disable();
        }
    }
    else
    {
        long nTmp;
        nTmp = static_cast<long>(aStr.getToken( 0, TOKEN ).toInt32());
        m_pNumFldCopies->SetValue( nTmp );

        nTmp = static_cast<long>(aStr.getToken( 1, TOKEN ).toInt32());
        m_pMtrFldMoveX->SetValue( nTmp );

        nTmp = static_cast<long>(aStr.getToken( 2, TOKEN ).toInt32());
        m_pMtrFldMoveY->SetValue( nTmp );

        nTmp = static_cast<long>(aStr.getToken( 3, TOKEN ).toInt32());
        m_pMtrFldAngle->SetValue( nTmp );

        nTmp = static_cast<long>(aStr.getToken( 4, TOKEN ).toInt32());
        m_pMtrFldWidth->SetValue( nTmp );

        nTmp = static_cast<long>(aStr.getToken( 5, TOKEN ).toInt32());
        m_pMtrFldHeight->SetValue( nTmp );

        nTmp = static_cast<long>(aStr.getToken( 6, TOKEN ).toInt32());
        m_pLbStartColor->SelectEntry( Color( nTmp ) );

        nTmp = static_cast<long>(aStr.getToken( 7, TOKEN ).toInt32());
        m_pLbEndColor->SelectEntry( Color( nTmp ) );
    }

}

/**
 * fills provided item set with dialog box attributes
 */
void CopyDlg::GetAttr( SfxItemSet& rOutAttrs )
{
    long nMoveX = long( GetCoreValue( *m_pMtrFldMoveX, MapUnit::Map100thMM) * maUIScale);
    long nMoveY = long( GetCoreValue( *m_pMtrFldMoveY, MapUnit::Map100thMM) * maUIScale);
    long nHeight = long( GetCoreValue( *m_pMtrFldHeight, MapUnit::Map100thMM) * maUIScale);
    long nWidth  = long( GetCoreValue( *m_pMtrFldWidth, MapUnit::Map100thMM) * maUIScale);

    rOutAttrs.Put( SfxUInt16Item( ATTR_COPY_NUMBER, static_cast<sal_uInt16>(m_pNumFldCopies->GetValue()) ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_MOVE_X, nMoveX ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_MOVE_Y, nMoveY ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_ANGLE, static_cast<sal_Int32>(m_pMtrFldAngle->GetValue()) ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_WIDTH, nWidth ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_HEIGHT, nHeight ) );

    NamedColor aColor = m_pLbStartColor->GetSelectedEntry();
    rOutAttrs.Put(XColorItem(ATTR_COPY_START_COLOR, aColor.second, aColor.first));
    aColor = m_pLbEndColor->GetSelectedEntry();
    rOutAttrs.Put(XColorItem(ATTR_COPY_END_COLOR, aColor.second, aColor.first));
}

/**
 * enables and selects end color LB
 */
IMPL_LINK_NOARG(CopyDlg, SelectColorHdl, SvxColorListBox&, void)
{
    const Color aColor = m_pLbStartColor->GetSelectEntryColor();

    if (!m_pLbEndColor->IsEnabled())
    {
        m_pLbEndColor->SelectEntry(aColor);
        m_pLbEndColor->Enable();
        m_pFtEndColor->Enable();
    }
}

/**
 * sets values of selection
 */
IMPL_LINK_NOARG(CopyDlg, SetViewData, Button*, void)
{
    ::tools::Rectangle aRect = mpView->GetAllMarkedRect();

    SetMetricValue( *m_pMtrFldMoveX, long( aRect.GetWidth() /
                                    maUIScale ), MapUnit::Map100thMM);
    SetMetricValue( *m_pMtrFldMoveY, long( aRect.GetHeight() /
                                    maUIScale ), MapUnit::Map100thMM);

    // sets color attribute
    const SfxPoolItem*  pPoolItem = nullptr;
    if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_START_COLOR, true, &pPoolItem ) )
    {
        Color aColor = static_cast<const XColorItem*>( pPoolItem )->GetColorValue();
        m_pLbStartColor->SelectEntry( aColor );
    }
}

/**
 * resets values to default
 */
IMPL_LINK_NOARG(CopyDlg, SetDefault, Button*, void)
{
    m_pNumFldCopies->SetValue( 1 );

    long nValue = 500;
    SetMetricValue( *m_pMtrFldMoveX, long(nValue / maUIScale), MapUnit::Map100thMM);
    SetMetricValue( *m_pMtrFldMoveY, long(nValue / maUIScale), MapUnit::Map100thMM);

    nValue = 0;
    m_pMtrFldAngle->SetValue( nValue );
    SetMetricValue( *m_pMtrFldWidth, long(nValue / maUIScale), MapUnit::Map100thMM);
    SetMetricValue( *m_pMtrFldHeight, long(nValue / maUIScale), MapUnit::Map100thMM);

    // set color attribute
    const SfxPoolItem*  pPoolItem = nullptr;
    if( SfxItemState::SET == mrOutAttrs.GetItemState( ATTR_COPY_START_COLOR, true, &pPoolItem ) )
    {
        Color aColor = static_cast<const XColorItem*>( pPoolItem )->GetColorValue();
        m_pLbStartColor->SelectEntry( aColor );
        m_pLbEndColor->SelectEntry( aColor );
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
