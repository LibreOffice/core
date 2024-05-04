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

#include <swpossizetabpage.hxx>
#include <svtools/unitconv.hxx>

#include <svx/svddef.hxx>
#include <svx/sxcecitm.hxx>
#include <svx/sxcgitm.hxx>
#include <svx/sxcllitm.hxx>
#include <svx/sxctitm.hxx>

#include <svx/dlgutil.hxx>
#include <labdlg.hxx>
#include <transfrm.hxx>
#include <bitmaps.hlst>

// define ----------------------------------------------------------------

#define EXT_OPTIMAL     0
#define EXT_FROM_TOP    1
#define EXT_FROM_LEFT   2
#define EXT_HORIZONTAL  3
#define EXT_VERTICAL    4

#define POS_TOP         0
#define POS_MIDDLE      1
#define POS_BOTTOM      2

#define BMP_CAPTTYPE_1  1
#define BMP_CAPTTYPE_2  2
#define BMP_CAPTTYPE_3  3

// static ----------------------------------------------------------------

const WhichRangesContainer SvxCaptionTabPage::pCaptionRanges(
    svl::Items<
    SDRATTR_CAPTIONTYPE, SDRATTR_CAPTIONFIXEDANGLE,
    SDRATTR_CAPTIONANGLE, SDRATTR_CAPTIONGAP,
    SDRATTR_CAPTIONESCDIR, SDRATTR_CAPTIONESCISREL,
    SDRATTR_CAPTIONESCREL, SDRATTR_CAPTIONESCABS,
    SDRATTR_CAPTIONLINELEN, SDRATTR_CAPTIONFITLINELEN>);

SvxCaptionTabPage::SvxCaptionTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "cui/ui/calloutpage.ui", "CalloutPage", &rInAttrs)
    , nCaptionType(SdrCaptionType::Type1)
    , nGap(0)
    , nEscDir(SdrCaptionEscDir::Horizontal)
    , bEscRel(false)
    , nEscAbs(0)
    , nEscRel(0)
    , nLineLen(0)
    , bFitLineLen(false)
    , nPosition(0)
    , nExtension(0)
    , rOutAttrs(rInAttrs)
    , pView(nullptr)
    , m_xMF_SPACING(m_xBuilder->weld_metric_spin_button("spacing", FieldUnit::MM))
    , m_xLB_EXTENSION(m_xBuilder->weld_combo_box("extension"))
    , m_xFT_BYFT(m_xBuilder->weld_label("byft"))
    , m_xMF_BY(m_xBuilder->weld_metric_spin_button("by", FieldUnit::MM))
    , m_xFT_POSITIONFT(m_xBuilder->weld_label("positionft"))
    , m_xLB_POSITION(m_xBuilder->weld_combo_box("position"))
    , m_xLineTypes(m_xBuilder->weld_combo_box("linetypes"))
    , m_xFT_LENGTHFT(m_xBuilder->weld_label("lengthft"))
    , m_xMF_LENGTH(m_xBuilder->weld_metric_spin_button("length", FieldUnit::MM))
    , m_xCB_OPTIMAL(m_xBuilder->weld_check_button("optimal"))
    , m_xCT_CAPTTYPE(new ValueSet(m_xBuilder->weld_scrolled_window("valuesetwin", true)))
    , m_xCT_CAPTTYPEWin(new weld::CustomWeld(*m_xBuilder, "valueset", *m_xCT_CAPTTYPE))
{
    Size aSize(m_xCT_CAPTTYPE->GetDrawingArea()->get_ref_device().LogicToPixel(Size(187, 38), MapMode(MapUnit::MapAppFont)));
    m_xCT_CAPTTYPEWin->set_size_request(aSize.Width(), aSize.Height());

    assert(m_xLB_POSITION->get_count() == 6);
    for (int i = 0;  i < 3; ++i)
        m_aStrHorzList.push_back(m_xLB_POSITION->get_text(i));
    for (int i = 3;  i < 6; ++i)
        m_aStrVertList.push_back(m_xLB_POSITION->get_text(i));
    m_xLB_POSITION->clear();

    assert(m_xLineTypes->get_count() == 3);
    std::vector<OUString> aLineTypes;
    aLineTypes.reserve(3);
    for (int i = 0; i < 3; ++i)
        aLineTypes.push_back(m_xLineTypes->get_text(i));

    static_assert(CAPTYPE_BITMAPS_COUNT == 3, "unexpected");
    m_aBmpCapTypes[0] = Image(StockImage::Yes, RID_SVXBMP_LEGTYP1);
    m_aBmpCapTypes[1] = Image(StockImage::Yes, RID_SVXBMP_LEGTYP2);
    m_aBmpCapTypes[2] = Image(StockImage::Yes, RID_SVXBMP_LEGTYP3);

    //------------install ValueSet--------------------------
    m_xCT_CAPTTYPE->SetStyle( m_xCT_CAPTTYPE->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER | WB_NAMEFIELD );
    m_xCT_CAPTTYPE->SetColCount(5);//XXX
    m_xCT_CAPTTYPE->SetLineCount(1);
    m_xCT_CAPTTYPE->SetSelectHdl(LINK( this, SvxCaptionTabPage, SelectCaptTypeHdl_Impl));

    Image aImage;
    m_xCT_CAPTTYPE->InsertItem(BMP_CAPTTYPE_1, aImage, aLineTypes[0]);
    m_xCT_CAPTTYPE->InsertItem(BMP_CAPTTYPE_2, aImage, aLineTypes[1]);
    m_xCT_CAPTTYPE->InsertItem(BMP_CAPTTYPE_3, aImage, aLineTypes[2]);

    FillValueSet();

    m_xLB_EXTENSION->connect_changed(LINK(this, SvxCaptionTabPage, ExtensionSelectHdl_Impl));
    m_xLB_POSITION->connect_changed(LINK(this, SvxCaptionTabPage, PositionSelectHdl_Impl));
    m_xCB_OPTIMAL->connect_toggled(LINK(this, SvxCaptionTabPage, LineOptHdl_Impl));
}

SvxCaptionTabPage::~SvxCaptionTabPage()
{
    m_xCT_CAPTTYPEWin.reset();
    m_xCT_CAPTTYPE.reset();
}

void SvxCaptionTabPage::Construct()
{
    // set rectangle and working area
    DBG_ASSERT( pView, "No valid View transferred!" );
}

bool SvxCaptionTabPage::FillItemSet( SfxItemSet*  _rOutAttrs)
{
    SfxItemPool* pPool = _rOutAttrs->GetPool();
    assert(pPool && "Where is the pool?");

    MapUnit      eUnit;

    nCaptionType = static_cast<SdrCaptionType>(m_xCT_CAPTTYPE->GetSelectedItemId()-1);

    _rOutAttrs->Put( SdrCaptionTypeItem( nCaptionType ) );

    if (m_xMF_SPACING->get_value_changed_from_saved())
    {
        eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONGAP ) );
        _rOutAttrs->Put( SdrCaptionGapItem( GetCoreValue(*m_xMF_SPACING, eUnit ) ) );
    }

    // special treatment!!! XXX
    if( nCaptionType==SdrCaptionType::Type1 )
    {
        switch( nEscDir )
        {
            case SdrCaptionEscDir::Horizontal:     nEscDir=SdrCaptionEscDir::Vertical;break;
            case SdrCaptionEscDir::Vertical:       nEscDir=SdrCaptionEscDir::Horizontal;break;
            default: break;
        }
    }

    _rOutAttrs->Put( SdrCaptionEscDirItem( nEscDir ) );

    bEscRel = m_xLB_POSITION->get_visible();
    _rOutAttrs->Put( SdrCaptionEscIsRelItem( bEscRel ) );

    if( bEscRel )
    {
        tools::Long    nVal = 0;

        switch (m_xLB_POSITION->get_active())
        {
            case POS_TOP:     nVal=0;break;
            case POS_MIDDLE:  nVal=5000;break;
            case POS_BOTTOM:  nVal=10000;break;
        }
        _rOutAttrs->Put( SdrCaptionEscRelItem( nVal ) );
    }
    else
    {
        if (m_xMF_BY->get_value_changed_from_saved())
        {
            eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONESCABS ) );
            _rOutAttrs->Put( SdrCaptionEscAbsItem( GetCoreValue(*m_xMF_BY, eUnit ) ) );
        }
    }

    bFitLineLen = m_xCB_OPTIMAL->get_active();
    _rOutAttrs->Put( SdrCaptionFitLineLenItem( bFitLineLen ) );

    if( ! bFitLineLen )
    {
        if (m_xMF_LENGTH->get_value_changed_from_saved())
        {
            eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONLINELEN ) );
            _rOutAttrs->Put( SdrCaptionLineLenItem( GetCoreValue(*m_xMF_LENGTH, eUnit ) ) );
        }
    }

//NYI-------------the angles have to be added here!!! XXX----------------------

    return true;
}

void SvxCaptionTabPage::Reset( const SfxItemSet*  )
{

    //------------set metric-----------------------------

    FieldUnit eFUnit = GetModuleFieldUnit( rOutAttrs );

    switch ( eFUnit )
    {
        case FieldUnit::CM:
        case FieldUnit::M:
        case FieldUnit::KM:
            eFUnit = FieldUnit::MM;
            break;
        default: ;//prevent warning
    }
    SetFieldUnit( *m_xMF_SPACING, eFUnit );
    SetFieldUnit( *m_xMF_BY, eFUnit );
    SetFieldUnit( *m_xMF_LENGTH, eFUnit );

    SfxItemPool*    pPool = rOutAttrs.GetPool();
    assert(pPool && "Where is the pool?");

    sal_uInt16   nWhich;
    MapUnit      eUnit;

    nWhich = GetWhich( SDRATTR_CAPTIONESCABS );
    eUnit = pPool->GetMetric( nWhich );
    nEscAbs = static_cast<const SdrCaptionEscAbsItem&>( rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( *m_xMF_BY, nEscAbs, eUnit );
    nEscAbs = m_xMF_BY->get_value(FieldUnit::NONE);

    nWhich = GetWhich( SDRATTR_CAPTIONESCREL );
    nEscRel = static_cast<tools::Long>(static_cast<const SdrCaptionEscRelItem&>( rOutAttrs.Get( nWhich ) ).GetValue());

    //------- line length ----------
    nWhich = GetWhich( SDRATTR_CAPTIONLINELEN );
    eUnit = pPool->GetMetric( nWhich );
    nLineLen = static_cast<const SdrCaptionLineLenItem&>( rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( *m_xMF_LENGTH, nLineLen, eUnit );
    nLineLen = m_xMF_LENGTH->get_value(FieldUnit::NONE);

    //------- distance to box ----------
    nWhich = GetWhich( SDRATTR_CAPTIONGAP );
    eUnit = pPool->GetMetric( nWhich );
    nGap = static_cast<const SdrCaptionGapItem&>( rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( *m_xMF_SPACING, nGap, eUnit );
    nGap = m_xMF_SPACING->get_value(FieldUnit::NONE);

    nCaptionType = rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONTYPE ) ).GetValue();
    bFitLineLen = static_cast<const SfxBoolItem&>( rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONFITLINELEN ) ) ).GetValue();
    nEscDir = rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONESCDIR ) ).GetValue();
    bEscRel = static_cast<const SfxBoolItem&>( rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONESCISREL ) ) ).GetValue();

    // special treatment!!! XXX
    if( nCaptionType==SdrCaptionType::Type1 )
    {
        switch( nEscDir )
        {
            case SdrCaptionEscDir::Horizontal:     nEscDir=SdrCaptionEscDir::Vertical;break;
            case SdrCaptionEscDir::Vertical:       nEscDir=SdrCaptionEscDir::Horizontal;break;
            default: break;
        }
    }

    nPosition = POS_MIDDLE;
    nExtension = EXT_OPTIMAL;

    m_xMF_SPACING->set_value(nGap, FieldUnit::NONE);

    if( nEscDir == SdrCaptionEscDir::Horizontal )
    {
        if( bEscRel )
        {
            if( nEscRel < 3333 )
                nPosition = POS_TOP;
            if( nEscRel > 6666 )
                nPosition = POS_BOTTOM;
            nExtension = EXT_HORIZONTAL;
        }
        else
        {
            nExtension = EXT_FROM_TOP;
            m_xMF_BY->set_value(nEscAbs, FieldUnit::NONE);
        }
    }
    else if( nEscDir == SdrCaptionEscDir::Vertical )
    {
        if( bEscRel )
        {
            if( nEscRel < 3333 )
                nPosition = POS_TOP;
            if( nEscRel > 6666 )
                nPosition = POS_BOTTOM;
            nExtension = EXT_VERTICAL;
        }
        else
        {
            nExtension = EXT_FROM_LEFT;
            m_xMF_BY->set_value(nEscAbs, FieldUnit::NONE);
        }
    }
    else if( nEscDir == SdrCaptionEscDir::BestFit )
    {
        nExtension = EXT_OPTIMAL;
    }

    m_xCB_OPTIMAL->set_active(bFitLineLen);
    m_xMF_LENGTH->set_value(nLineLen, FieldUnit::NONE);

    m_xLB_EXTENSION->set_active(nExtension);

    SetupExtension_Impl( nExtension );
    m_xCT_CAPTTYPE->SelectItem( static_cast<int>(nCaptionType)+1 ); // Enum starts at 0!
    SetupType_Impl( nCaptionType );
}

std::unique_ptr<SfxTabPage> SvxCaptionTabPage::Create(weld::Container* pPage, weld::DialogController* pController,
                                             const SfxItemSet* rOutAttrs)
{
    return std::make_unique<SvxCaptionTabPage>(pPage, pController, *rOutAttrs);
}

void SvxCaptionTabPage::SetupExtension_Impl( sal_uInt16 nType )
{
    switch( nType )
    {
        case EXT_OPTIMAL:
        m_xMF_BY->show();
        m_xFT_BYFT->show();
        m_xFT_POSITIONFT->hide();
        m_xLB_POSITION->hide();
        nEscDir = SdrCaptionEscDir::BestFit;
        break;

        case EXT_FROM_TOP:
        m_xMF_BY->show();
        m_xFT_BYFT->show();
        m_xFT_POSITIONFT->hide();
        m_xLB_POSITION->hide();
        nEscDir = SdrCaptionEscDir::Horizontal;
        break;

        case EXT_FROM_LEFT:
        m_xMF_BY->show();
        m_xFT_BYFT->show();
        m_xFT_POSITIONFT->hide();
        m_xLB_POSITION->hide();
        nEscDir = SdrCaptionEscDir::Vertical;
        break;

        case EXT_HORIZONTAL:
        m_xLB_POSITION->clear();
        for (const OUString & i : m_aStrHorzList)
            m_xLB_POSITION->append_text(i);
        m_xLB_POSITION->set_active(nPosition);

        m_xMF_BY->hide();
        m_xFT_BYFT->hide();
        m_xFT_POSITIONFT->show();
        m_xLB_POSITION->show();
        nEscDir = SdrCaptionEscDir::Horizontal;
        break;

        case EXT_VERTICAL:
        m_xLB_POSITION->clear();
        for (const OUString & i : m_aStrVertList)
            m_xLB_POSITION->append_text(i);
        m_xLB_POSITION->set_active(nPosition);

        m_xMF_BY->hide();
        m_xFT_BYFT->hide();
        m_xFT_POSITIONFT->show();
        m_xLB_POSITION->show();
        nEscDir = SdrCaptionEscDir::Vertical;
        break;
    }
}

IMPL_LINK(SvxCaptionTabPage, ExtensionSelectHdl_Impl, weld::ComboBox&, rListBox, void)
{
    if (&rListBox == m_xLB_EXTENSION.get())
    {
        SetupExtension_Impl(m_xLB_EXTENSION->get_active());
    }
}

IMPL_LINK(SvxCaptionTabPage, PositionSelectHdl_Impl, weld::ComboBox&, rListBox, void)
{
    if (&rListBox == m_xLB_POSITION.get())
    {
        nPosition = m_xLB_POSITION->get_active();
    }
}

IMPL_LINK( SvxCaptionTabPage, LineOptHdl_Impl, weld::Toggleable&, rButton, void )
{
    if (&rButton != m_xCB_OPTIMAL.get())
        return;

    if (m_xCB_OPTIMAL->get_active() || !m_xCB_OPTIMAL->get_sensitive())
    {
        m_xFT_LENGTHFT->set_sensitive(false);
        m_xMF_LENGTH->set_sensitive(false);
    }
    else
    {
        m_xFT_LENGTHFT->set_sensitive(true);
        m_xMF_LENGTH->set_sensitive(true);
    }
}

IMPL_LINK_NOARG(SvxCaptionTabPage, SelectCaptTypeHdl_Impl, ValueSet*, void)
{
    SetupType_Impl( static_cast<SdrCaptionType>(m_xCT_CAPTTYPE->GetSelectedItemId()) );
}

void SvxCaptionTabPage::SetupType_Impl( SdrCaptionType nType )
{
    switch( nType )
    {
        case SdrCaptionType::Type1:
        case SdrCaptionType::Type2:
            m_xFT_LENGTHFT->set_sensitive(false);
            m_xCB_OPTIMAL->set_sensitive(false);
            LineOptHdl_Impl(*m_xCB_OPTIMAL);
            break;
        case SdrCaptionType::Type3:
        case SdrCaptionType::Type4:
            m_xFT_LENGTHFT->set_sensitive(true);
            m_xCB_OPTIMAL->set_sensitive(true);
            LineOptHdl_Impl(*m_xCB_OPTIMAL);
            break;
    }
}

void SvxCaptionTabPage::FillValueSet()
{
    m_xCT_CAPTTYPE->SetItemImage(BMP_CAPTTYPE_1, m_aBmpCapTypes[0] );
    m_xCT_CAPTTYPE->SetItemImage(BMP_CAPTTYPE_2, m_aBmpCapTypes[1] );
    m_xCT_CAPTTYPE->SetItemImage(BMP_CAPTTYPE_3, m_aBmpCapTypes[2] );
}

SvxCaptionTabDialog::SvxCaptionTabDialog(weld::Window* pParent, const SdrView* pSdrView,
    SvxAnchorIds nAnchorTypes)
    : SfxTabDialogController(pParent, "cui/ui/calloutdialog.ui", "CalloutDialog")
    , pView(pSdrView)
    , nAnchorCtrls(nAnchorTypes)
{
    assert(pView); // No valid View transferred!

    //different positioning page in Writer
    if (nAnchorCtrls & (SvxAnchorIds::Paragraph | SvxAnchorIds::Character | SvxAnchorIds::Page | SvxAnchorIds::Fly))
    {
        AddTabPage("RID_SVXPAGE_SWPOSSIZE", SvxSwPosSizeTabPage::Create,
            SvxSwPosSizeTabPage::GetRanges );
        RemoveTabPage("RID_SVXPAGE_POSITION_SIZE");
    }
    else
    {
        AddTabPage("RID_SVXPAGE_POSITION_SIZE", SvxPositionSizeTabPage::Create,
            SvxPositionSizeTabPage::GetRanges );
        RemoveTabPage("RID_SVXPAGE_SWPOSSIZE");
    }
    AddTabPage("RID_SVXPAGE_CAPTION", SvxCaptionTabPage::Create,
        SvxCaptionTabPage::GetRanges );
}

void SvxCaptionTabDialog::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    if (rId == "RID_SVXPAGE_POSITION_SIZE")
    {
        static_cast<SvxPositionSizeTabPage&>( rPage ).SetView( pView );
        static_cast<SvxPositionSizeTabPage&>( rPage ).Construct();
        if( nAnchorCtrls & SvxAnchorIds::NoResize )
            static_cast<SvxPositionSizeTabPage&>( rPage ).DisableResize();

        if( nAnchorCtrls & SvxAnchorIds::NoProtect )
            static_cast<SvxPositionSizeTabPage&>( rPage ).DisableProtect();
    }
    else if (rId == "RID_SVXPAGE_SWPOSSIZE")
    {
        SvxSwPosSizeTabPage& rSwPage = static_cast<SvxSwPosSizeTabPage&>(rPage);
        rSwPage.EnableAnchorTypes(nAnchorCtrls);
        rSwPage.SetValidateFramePosLink( aValidateLink );
    }
    else if (rId == "RID_SVXPAGE_CAPTION")
    {
        static_cast<SvxCaptionTabPage&>( rPage ).SetView( pView );
        static_cast<SvxCaptionTabPage&>( rPage ).Construct();
    }
}

void SvxCaptionTabDialog::SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink )
{
    aValidateLink = rLink;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
