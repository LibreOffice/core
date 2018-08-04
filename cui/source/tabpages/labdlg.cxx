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

#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <swpossizetabpage.hxx>
#include <vcl/settings.hxx>

#include <svx/dialogs.hrc>
#include <svx/svddef.hxx>
#include <svx/sxcaitm.hxx>
#include <svx/sxcecitm.hxx>
#include <svx/sxcgitm.hxx>
#include <svx/sxcllitm.hxx>
#include <svx/sxctitm.hxx>

#include <svx/dlgutil.hxx>
#include <labdlg.hxx>
#include <transfrm.hxx>

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

const sal_uInt16 SvxCaptionTabPage::pCaptionRanges[] =
{
    SDRATTR_CAPTIONTYPE,
    SDRATTR_CAPTIONFIXEDANGLE,
    SDRATTR_CAPTIONANGLE,
    SDRATTR_CAPTIONGAP,
    SDRATTR_CAPTIONESCDIR,
    SDRATTR_CAPTIONESCISREL,
    SDRATTR_CAPTIONESCREL,
    SDRATTR_CAPTIONESCABS,
    SDRATTR_CAPTIONLINELEN,
    SDRATTR_CAPTIONFITLINELEN,
    0
};

SvxCaptionTabPage::SvxCaptionTabPage(vcl::Window* pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent, "CalloutPage", "cui/ui/calloutpage.ui", &rInAttrs)
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
{
    get(m_pCT_CAPTTYPE, "valueset");

    Size aSize(m_pCT_CAPTTYPE->LogicToPixel(Size(187, 38), MapMode(MapUnit::MapAppFont)));
    m_pCT_CAPTTYPE->set_width_request(aSize.Width());
    m_pCT_CAPTTYPE->set_height_request(aSize.Height());

    get(m_pMF_SPACING, "spacing");
    get(m_pLB_EXTENSION, "extension");
    get(m_pFT_BYFT, "byft");
    get(m_pMF_BY, "by");
    get(m_pFT_POSITIONFT, "positionft");
    get(m_pLB_POSITION, "position");

    assert(m_pLB_POSITION->GetEntryCount() == 6);
    for (int i = 0;  i < 3; ++i)
        m_aStrHorzList.push_back(m_pLB_POSITION->GetEntry(i));
    for (int i = 3;  i < 6; ++i)
        m_aStrVertList.push_back(m_pLB_POSITION->GetEntry(i));
    m_pLB_POSITION->Clear();

    ListBox *pLineTypes = get<ListBox>("linetypes");
    assert(pLineTypes->GetEntryCount() == 3);
    std::vector<OUString> aLineTypes;
    for (int i = 0;  i < 3; ++i)
        aLineTypes.push_back(pLineTypes->GetEntry(i));

    get(m_pFT_LENGTHFT, "lengthft");
    get(m_pMF_LENGTH, "length");
    get(m_pCB_OPTIMAL, "optimal");

    for(sal_uInt16 nBitmap = 0; nBitmap < CAPTYPE_BITMAPS_COUNT; ++nBitmap)
    {
        FixedImage *pImage = get<FixedImage>(OString("legtyp") + OString::number(nBitmap+1));
        m_aBmpCapTypes[nBitmap] = pImage->GetImage();
    }

    //------------install ValueSet--------------------------
    m_pCT_CAPTTYPE->SetStyle( m_pCT_CAPTTYPE->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER | WB_NAMEFIELD );
    m_pCT_CAPTTYPE->SetColCount(5);//XXX
    m_pCT_CAPTTYPE->SetLineCount(1);
    m_pCT_CAPTTYPE->SetSelectHdl(LINK( this, SvxCaptionTabPage, SelectCaptTypeHdl_Impl));

    Image aImage;
    m_pCT_CAPTTYPE->InsertItem(BMP_CAPTTYPE_1, aImage, aLineTypes[0]);
    m_pCT_CAPTTYPE->InsertItem(BMP_CAPTTYPE_2, aImage, aLineTypes[1]);
    m_pCT_CAPTTYPE->InsertItem(BMP_CAPTTYPE_3, aImage, aLineTypes[2]);

    FillValueSet();

    m_pLB_EXTENSION->SetSelectHdl(LINK(this, SvxCaptionTabPage, ExtensionSelectHdl_Impl));
    m_pLB_POSITION->SetSelectHdl(LINK(this, SvxCaptionTabPage, PositionSelectHdl_Impl));
    m_pCB_OPTIMAL->SetClickHdl(LINK(this, SvxCaptionTabPage, LineOptHdl_Impl));
}

SvxCaptionTabPage::~SvxCaptionTabPage()
{
    disposeOnce();
}

void SvxCaptionTabPage::dispose()
{
    m_pCT_CAPTTYPE.clear();
    m_pMF_SPACING.clear();
    m_pLB_EXTENSION.clear();
    m_pFT_BYFT.clear();
    m_pMF_BY.clear();
    m_pFT_POSITIONFT.clear();
    m_pLB_POSITION.clear();
    m_pFT_LENGTHFT.clear();
    m_pMF_LENGTH.clear();
    m_pCB_OPTIMAL.clear();
    SfxTabPage::dispose();
}

void SvxCaptionTabPage::Construct()
{
    // set rectangle and working area
    DBG_ASSERT( pView, "No valid View transferred!" );
}


bool SvxCaptionTabPage::FillItemSet( SfxItemSet*  _rOutAttrs)
{
    SfxItemPool*    pPool = _rOutAttrs->GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );

    MapUnit      eUnit;

    nCaptionType = static_cast<SdrCaptionType>(m_pCT_CAPTTYPE->GetSelectedItemId()-1);

    _rOutAttrs->Put( SdrCaptionTypeItem( nCaptionType ) );

    if( m_pMF_SPACING->IsValueModified() )
    {
        eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONGAP ) );
        _rOutAttrs->Put( SdrCaptionGapItem( GetCoreValue(*m_pMF_SPACING, eUnit ) ) );
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

    bEscRel = m_pLB_POSITION->IsVisible();
    _rOutAttrs->Put( SdrCaptionEscIsRelItem( bEscRel ) );

    if( bEscRel )
    {
        long    nVal = 0;

        switch( m_pLB_POSITION->GetSelectedEntryPos() )
        {
            case POS_TOP:     nVal=0;break;
            case POS_MIDDLE:  nVal=5000;break;
            case POS_BOTTOM:  nVal=10000;break;
        }
        _rOutAttrs->Put( SdrCaptionEscRelItem( nVal ) );
    }
    else
    {
        if( m_pMF_BY->IsValueModified() )
        {
            eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONESCABS ) );
            _rOutAttrs->Put( SdrCaptionEscAbsItem( GetCoreValue(*m_pMF_BY, eUnit ) ) );
        }
    }

    bFitLineLen = m_pCB_OPTIMAL->IsChecked();
    _rOutAttrs->Put( SdrCaptionFitLineLenItem( bFitLineLen ) );

    if( ! bFitLineLen )
    {
        if( m_pMF_LENGTH->IsValueModified() )
        {
            eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONLINELEN ) );
            _rOutAttrs->Put( SdrCaptionLineLenItem( GetCoreValue(*m_pMF_LENGTH, eUnit ) ) );
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
        case FUNIT_CM:
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            break;
        default: ;//prevent warning
    }
    SetFieldUnit( *m_pMF_SPACING, eFUnit );
    SetFieldUnit( *m_pMF_BY, eFUnit );
    SetFieldUnit( *m_pMF_LENGTH, eFUnit );

    SfxItemPool*    pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );

    sal_uInt16   nWhich;
    MapUnit      eUnit;

    nWhich = GetWhich( SDRATTR_CAPTIONESCABS );
    eUnit = pPool->GetMetric( nWhich );
    nEscAbs = static_cast<const SdrCaptionEscAbsItem&>( rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( *m_pMF_BY, nEscAbs, eUnit );
    nEscAbs = static_cast<long>(m_pMF_BY->GetValue());

    nWhich = GetWhich( SDRATTR_CAPTIONESCREL );
    nEscRel = static_cast<long>(static_cast<const SdrCaptionEscRelItem&>( rOutAttrs.Get( nWhich ) ).GetValue());

    //------- line length ----------
    nWhich = GetWhich( SDRATTR_CAPTIONLINELEN );
    eUnit = pPool->GetMetric( nWhich );
    nLineLen = static_cast<const SdrCaptionLineLenItem&>( rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( *m_pMF_LENGTH, nLineLen, eUnit );
    nLineLen = static_cast<long>(m_pMF_LENGTH->GetValue());

    //------- distance to box ----------
    nWhich = GetWhich( SDRATTR_CAPTIONGAP );
    eUnit = pPool->GetMetric( nWhich );
    nGap = static_cast<const SdrCaptionGapItem&>( rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( *m_pMF_SPACING, nGap, eUnit );
    nGap = static_cast<long>(m_pMF_SPACING->GetValue());

    nCaptionType = static_cast<const SdrCaptionTypeItem&>( rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONTYPE ) ) ).GetValue();
    bFitLineLen = static_cast<const SfxBoolItem&>( rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONFITLINELEN ) ) ).GetValue();
    nEscDir = static_cast<const SdrCaptionEscDirItem&>( rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONESCDIR ) ) ).GetValue();
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

    m_pMF_SPACING->SetValue( nGap );

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
            m_pMF_BY->SetValue( nEscAbs );
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
            m_pMF_BY->SetValue( nEscAbs );
        }
    }
    else if( nEscDir == SdrCaptionEscDir::BestFit )
    {
        nExtension = EXT_OPTIMAL;
    }

    m_pCB_OPTIMAL->Check( bFitLineLen );
    m_pMF_LENGTH->SetValue( nLineLen );

    m_pLB_EXTENSION->SelectEntryPos( nExtension );

    SetupExtension_Impl( nExtension );
    m_pCT_CAPTTYPE->SelectItem( static_cast<int>(nCaptionType)+1 ); // Enum starts at 0!
    SetupType_Impl( nCaptionType );
}


VclPtr<SfxTabPage> SvxCaptionTabPage::Create( TabPageParent pWindow,
                                              const SfxItemSet* rOutAttrs )
{
    return VclPtr<SvxCaptionTabPage>::Create( pWindow.pParent, *rOutAttrs );
}


void SvxCaptionTabPage::SetupExtension_Impl( sal_uInt16 nType )
{
    switch( nType )
    {
        case EXT_OPTIMAL:
        m_pMF_BY->Show();
        m_pFT_BYFT->Show();
        m_pFT_POSITIONFT->Hide();
        m_pLB_POSITION->Hide();
        nEscDir = SdrCaptionEscDir::BestFit;
        break;

        case EXT_FROM_TOP:
        m_pMF_BY->Show();
        m_pFT_BYFT->Show();
        m_pFT_POSITIONFT->Hide();
        m_pLB_POSITION->Hide();
        nEscDir = SdrCaptionEscDir::Horizontal;
        break;

        case EXT_FROM_LEFT:
        m_pMF_BY->Show();
        m_pFT_BYFT->Show();
        m_pFT_POSITIONFT->Hide();
        m_pLB_POSITION->Hide();
        nEscDir = SdrCaptionEscDir::Vertical;
        break;

        case EXT_HORIZONTAL:
        m_pLB_POSITION->Clear();
        for (OUString & i : m_aStrHorzList)
            m_pLB_POSITION->InsertEntry(i);
        m_pLB_POSITION->SelectEntryPos(nPosition);

        m_pMF_BY->Hide();
        m_pFT_BYFT->Hide();
        m_pFT_POSITIONFT->Show();
        m_pLB_POSITION->Show();
        nEscDir = SdrCaptionEscDir::Horizontal;
        break;

        case EXT_VERTICAL:
        m_pLB_POSITION->Clear();
        for (OUString & i : m_aStrVertList)
            m_pLB_POSITION->InsertEntry(i);
        m_pLB_POSITION->SelectEntryPos(nPosition);

        m_pMF_BY->Hide();
        m_pFT_BYFT->Hide();
        m_pFT_POSITIONFT->Show();
        m_pLB_POSITION->Show();
        nEscDir = SdrCaptionEscDir::Vertical;
        break;
    }
}


IMPL_LINK( SvxCaptionTabPage, ExtensionSelectHdl_Impl, ListBox&, rListBox, void )
{
    if (&rListBox == m_pLB_EXTENSION)
    {
        SetupExtension_Impl( m_pLB_EXTENSION->GetSelectedEntryPos() );
    }
}

IMPL_LINK( SvxCaptionTabPage, PositionSelectHdl_Impl, ListBox&, rListBox, void )
{
    if (&rListBox == m_pLB_POSITION)
    {
        nPosition = m_pLB_POSITION->GetSelectedEntryPos();
    }
}

IMPL_LINK( SvxCaptionTabPage, LineOptHdl_Impl, Button *, pButton, void )
{
    if (pButton == m_pCB_OPTIMAL)
    {
        if( m_pCB_OPTIMAL->IsChecked() || ! m_pCB_OPTIMAL->IsEnabled() )
        {
            m_pFT_LENGTHFT->Disable();
            m_pMF_LENGTH->Disable();
        }
        else
        {
            m_pFT_LENGTHFT->Enable();
            m_pMF_LENGTH->Enable();
        }
    }
}


IMPL_LINK_NOARG(SvxCaptionTabPage, SelectCaptTypeHdl_Impl, ValueSet*, void)
{
    SetupType_Impl( static_cast<SdrCaptionType>(m_pCT_CAPTTYPE->GetSelectedItemId()) );
}

void SvxCaptionTabPage::SetupType_Impl( SdrCaptionType nType )
{
    switch( nType )
    {
        case SdrCaptionType::Type1:
        case SdrCaptionType::Type2:
        m_pFT_LENGTHFT->Disable();
        m_pCB_OPTIMAL->Disable();
        LineOptHdl_Impl( m_pCB_OPTIMAL );
        break;

        case SdrCaptionType::Type3:
        case SdrCaptionType::Type4:
        m_pFT_LENGTHFT->Enable();
        m_pCB_OPTIMAL->Enable();
        LineOptHdl_Impl( m_pCB_OPTIMAL );
        break;
    }
}


void SvxCaptionTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
            FillValueSet();
}


void SvxCaptionTabPage::FillValueSet()
{
    m_pCT_CAPTTYPE->SetItemImage(BMP_CAPTTYPE_1, m_aBmpCapTypes[0] );
    m_pCT_CAPTTYPE->SetItemImage(BMP_CAPTTYPE_2, m_aBmpCapTypes[1] );
    m_pCT_CAPTTYPE->SetItemImage(BMP_CAPTTYPE_3, m_aBmpCapTypes[2] );
}


SvxCaptionTabDialog::SvxCaptionTabDialog(vcl::Window* pParent, const SdrView* pSdrView,
    SvxAnchorIds nAnchorTypes)
    : SfxTabDialog( pParent, "CalloutDialog", "cui/ui/calloutdialog.ui")
    , pView(pSdrView)
    , nAnchorCtrls(nAnchorTypes)
    , m_nSwPosSizePageId(0)
    , m_nPositionSizePageId(0)
    , m_nCaptionPageId(0)
{
    assert(pView); // No valid View transferred!

    //different positioning page in Writer
    if (nAnchorCtrls & (SvxAnchorIds::Paragraph | SvxAnchorIds::Character | SvxAnchorIds::Page | SvxAnchorIds::Fly))
    {
        m_nSwPosSizePageId = AddTabPage("RID_SVXPAGE_SWPOSSIZE", SvxSwPosSizeTabPage::Create,
            SvxSwPosSizeTabPage::GetRanges );
        RemoveTabPage("RID_SVXPAGE_POSITION_SIZE");
    }
    else
    {
        m_nPositionSizePageId = AddTabPage("RID_SVXPAGE_POSITION_SIZE", SvxPositionSizeTabPage::Create,
            SvxPositionSizeTabPage::GetRanges );
        RemoveTabPage("RID_SVXPAGE_SWPOSSIZE");
    }
    m_nCaptionPageId = AddTabPage("RID_SVXPAGE_CAPTION", SvxCaptionTabPage::Create,
        SvxCaptionTabPage::GetRanges );
}

void SvxCaptionTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if (nId == m_nPositionSizePageId)
    {
        static_cast<SvxPositionSizeTabPage&>( rPage ).SetView( pView );
        static_cast<SvxPositionSizeTabPage&>( rPage ).Construct();
        if( nAnchorCtrls & SvxAnchorIds::NoResize )
            static_cast<SvxPositionSizeTabPage&>( rPage ).DisableResize();

        if( nAnchorCtrls & SvxAnchorIds::NoProtect )
            static_cast<SvxPositionSizeTabPage&>( rPage ).DisableProtect();
    }
    else if (nId == m_nSwPosSizePageId)
    {
        SvxSwPosSizeTabPage& rSwPage = static_cast<SvxSwPosSizeTabPage&>(rPage);
        rSwPage.EnableAnchorTypes(nAnchorCtrls);
        rSwPage.SetValidateFramePosLink( aValidateLink );
    }
    else if (nId == m_nCaptionPageId)
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
