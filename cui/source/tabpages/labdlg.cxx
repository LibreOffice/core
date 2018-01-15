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

#include <comphelper/string.hxx>
#include <dialmgr.hxx>
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

#define AZ_OPTIMAL      0
#define AZ_VON_OBEN     1
#define AZ_VON_LINKS    2
#define AZ_HORIZONTAL   3
#define AZ_VERTIKAL     4

#define AT_OBEN         0
#define AT_MITTE        1
#define AT_UNTEN        2

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
    , nAnsatzRelPos(0)
    , nAnsatzTypePos(0)
    , rOutAttrs(rInAttrs)
    , pView(nullptr)
{
    get(m_pCT_CAPTTYPE, "valueset");

    Size aSize(m_pCT_CAPTTYPE->LogicToPixel(Size(187, 38), MapMode(MapUnit::MapAppFont)));
    m_pCT_CAPTTYPE->set_width_request(aSize.Width());
    m_pCT_CAPTTYPE->set_height_request(aSize.Height());

    get(m_pMF_ABSTAND, "spacing");
    get(m_pLB_ANSATZ, "extension");
    get(m_pFT_UM, "byft");
    get(m_pMF_ANSATZ, "by");
    get(m_pFT_ANSATZ_REL, "positionft");
    get(m_pLB_ANSATZ_REL, "position");

    assert(m_pLB_ANSATZ_REL->GetEntryCount() == 6);
    for (int i = 0;  i < 3; ++i)
        m_aStrHorzList.push_back(m_pLB_ANSATZ_REL->GetEntry(i));
    for (int i = 3;  i < 6; ++i)
        m_aStrVertList.push_back(m_pLB_ANSATZ_REL->GetEntry(i));
    m_pLB_ANSATZ_REL->Clear();

    ListBox *pLineTypes = get<ListBox>("linetypes");
    assert(pLineTypes->GetEntryCount() == 3);
    for (int i = 0;  i < 3; ++i)
        m_aLineTypes.push_back(pLineTypes->GetEntry(i));

    get(m_pFT_LAENGE, "lengthft");
    get(m_pMF_LAENGE, "length");
    get(m_pCB_LAENGE, "optimal");

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
    m_pCT_CAPTTYPE->InsertItem(BMP_CAPTTYPE_1, aImage, m_aLineTypes[0]);
    m_pCT_CAPTTYPE->InsertItem(BMP_CAPTTYPE_2, aImage, m_aLineTypes[1]);
    m_pCT_CAPTTYPE->InsertItem(BMP_CAPTTYPE_3, aImage, m_aLineTypes[2]);

    FillValueSet();

    m_pLB_ANSATZ->SetSelectHdl(LINK(this,SvxCaptionTabPage,AnsatzSelectHdl_Impl));
    m_pLB_ANSATZ_REL->SetSelectHdl(LINK(this,SvxCaptionTabPage,AnsatzRelSelectHdl_Impl));
    m_pCB_LAENGE->SetClickHdl(LINK(this,SvxCaptionTabPage,LineOptHdl_Impl));
}

SvxCaptionTabPage::~SvxCaptionTabPage()
{
    disposeOnce();
}

void SvxCaptionTabPage::dispose()
{
    m_pCT_CAPTTYPE.clear();
    m_pMF_ABSTAND.clear();
    m_pLB_ANSATZ.clear();
    m_pFT_UM.clear();
    m_pMF_ANSATZ.clear();
    m_pFT_ANSATZ_REL.clear();
    m_pLB_ANSATZ_REL.clear();
    m_pFT_LAENGE.clear();
    m_pMF_LAENGE.clear();
    m_pCB_LAENGE.clear();
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

    nCaptionType = static_cast<SdrCaptionType>(m_pCT_CAPTTYPE->GetSelectItemId()-1);

    _rOutAttrs->Put( SdrCaptionTypeItem( nCaptionType ) );

    if( m_pMF_ABSTAND->IsValueModified() )
    {
        eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONGAP ) );
        _rOutAttrs->Put( SdrCaptionGapItem( GetCoreValue(*m_pMF_ABSTAND, eUnit ) ) );
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

    bEscRel = m_pLB_ANSATZ_REL->IsVisible();
    _rOutAttrs->Put( SdrCaptionEscIsRelItem( bEscRel ) );

    if( bEscRel )
    {
        long    nVal = 0;

        switch( m_pLB_ANSATZ_REL->GetSelectedEntryPos() )
        {
            case AT_OBEN:   nVal=0;break;
            case AT_MITTE:  nVal=5000;break;
            case AT_UNTEN:  nVal=10000;break;
        }
        _rOutAttrs->Put( SdrCaptionEscRelItem( nVal ) );
    }
    else
    {
        if( m_pMF_ANSATZ->IsValueModified() )
        {
            eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONESCABS ) );
            _rOutAttrs->Put( SdrCaptionEscAbsItem( GetCoreValue(*m_pMF_ANSATZ, eUnit ) ) );
        }
    }

    bFitLineLen = m_pCB_LAENGE->IsChecked();
    _rOutAttrs->Put( SdrCaptionFitLineLenItem( bFitLineLen ) );

    if( ! bFitLineLen )
    {
        if( m_pMF_LAENGE->IsValueModified() )
        {
            eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONLINELEN ) );
            _rOutAttrs->Put( SdrCaptionLineLenItem( GetCoreValue(*m_pMF_LAENGE, eUnit ) ) );
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
    SetFieldUnit( *m_pMF_ABSTAND, eFUnit );
    SetFieldUnit( *m_pMF_ANSATZ, eFUnit );
    SetFieldUnit( *m_pMF_LAENGE, eFUnit );

    SfxItemPool*    pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );

    sal_uInt16   nWhich;
    MapUnit      eUnit;

    nWhich = GetWhich( SDRATTR_CAPTIONESCABS );
    eUnit = pPool->GetMetric( nWhich );
    nEscAbs = static_cast<const SdrCaptionEscAbsItem&>( rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( *m_pMF_ANSATZ, nEscAbs, eUnit );
    nEscAbs = static_cast<long>(m_pMF_ANSATZ->GetValue());

    nWhich = GetWhich( SDRATTR_CAPTIONESCREL );
    nEscRel = static_cast<long>(static_cast<const SdrCaptionEscRelItem&>( rOutAttrs.Get( nWhich ) ).GetValue());

    //------- line length ----------
    nWhich = GetWhich( SDRATTR_CAPTIONLINELEN );
    eUnit = pPool->GetMetric( nWhich );
    nLineLen = static_cast<const SdrCaptionLineLenItem&>( rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( *m_pMF_LAENGE, nLineLen, eUnit );
    nLineLen = static_cast<long>(m_pMF_LAENGE->GetValue());

    //------- distance to box ----------
    nWhich = GetWhich( SDRATTR_CAPTIONGAP );
    eUnit = pPool->GetMetric( nWhich );
    nGap = static_cast<const SdrCaptionGapItem&>( rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( *m_pMF_ABSTAND, nGap, eUnit );
    nGap = static_cast<long>(m_pMF_ABSTAND->GetValue());

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

    nAnsatzRelPos=AT_MITTE;
    nAnsatzTypePos=AZ_OPTIMAL;

    m_pMF_ABSTAND->SetValue( nGap );

    if( nEscDir == SdrCaptionEscDir::Horizontal )
    {
        if( bEscRel )
        {
            if( nEscRel < 3333 )
                nAnsatzRelPos = AT_OBEN;
            if( nEscRel > 6666 )
                nAnsatzRelPos = AT_UNTEN;
            nAnsatzTypePos = AZ_HORIZONTAL;
        }
        else
        {
            nAnsatzTypePos = AZ_VON_OBEN;
            m_pMF_ANSATZ->SetValue( nEscAbs );
        }
    }
    else if( nEscDir == SdrCaptionEscDir::Vertical )
    {
        if( bEscRel )
        {
            if( nEscRel < 3333 )
                nAnsatzRelPos = AT_OBEN;
            if( nEscRel > 6666 )
                nAnsatzRelPos = AT_UNTEN;
            nAnsatzTypePos = AZ_VERTIKAL;
        }
        else
        {
            nAnsatzTypePos = AZ_VON_LINKS;
            m_pMF_ANSATZ->SetValue( nEscAbs );
        }
    }
    else if( nEscDir == SdrCaptionEscDir::BestFit )
    {
        nAnsatzTypePos = AZ_OPTIMAL;
    }

    m_pCB_LAENGE->Check( bFitLineLen );
    m_pMF_LAENGE->SetValue( nLineLen );

    m_pLB_ANSATZ->SelectEntryPos( nAnsatzTypePos );

    SetupAnsatz_Impl( nAnsatzTypePos );
    m_pCT_CAPTTYPE->SelectItem( static_cast<int>(nCaptionType)+1 ); // Enum starts at 0!
    SetupType_Impl( nCaptionType );
}


VclPtr<SfxTabPage> SvxCaptionTabPage::Create( vcl::Window* pWindow,
                                              const SfxItemSet* rOutAttrs )
{
    return VclPtr<SvxCaptionTabPage>::Create( pWindow, *rOutAttrs );
}


void SvxCaptionTabPage::SetupAnsatz_Impl( sal_uInt16 nType )
{
    switch( nType )
    {
        case AZ_OPTIMAL:
        m_pMF_ANSATZ->Show();
        m_pFT_UM->Show();
        m_pFT_ANSATZ_REL->Hide();
        m_pLB_ANSATZ_REL->Hide();
        nEscDir = SdrCaptionEscDir::BestFit;
        break;

        case AZ_VON_OBEN:
        m_pMF_ANSATZ->Show();
        m_pFT_UM->Show();
        m_pFT_ANSATZ_REL->Hide();
        m_pLB_ANSATZ_REL->Hide();
        nEscDir = SdrCaptionEscDir::Horizontal;
        break;

        case AZ_VON_LINKS:
        m_pMF_ANSATZ->Show();
        m_pFT_UM->Show();
        m_pFT_ANSATZ_REL->Hide();
        m_pLB_ANSATZ_REL->Hide();
        nEscDir = SdrCaptionEscDir::Vertical;
        break;

        case AZ_HORIZONTAL:
        m_pLB_ANSATZ_REL->Clear();
        for (OUString & i : m_aStrHorzList)
            m_pLB_ANSATZ_REL->InsertEntry(i);
        m_pLB_ANSATZ_REL->SelectEntryPos(nAnsatzRelPos);

        m_pMF_ANSATZ->Hide();
        m_pFT_UM->Hide();
        m_pFT_ANSATZ_REL->Show();
        m_pLB_ANSATZ_REL->Show();
        nEscDir = SdrCaptionEscDir::Horizontal;
        break;

        case AZ_VERTIKAL:
        m_pLB_ANSATZ_REL->Clear();
        for (OUString & i : m_aStrVertList)
            m_pLB_ANSATZ_REL->InsertEntry(i);
        m_pLB_ANSATZ_REL->SelectEntryPos(nAnsatzRelPos);

        m_pMF_ANSATZ->Hide();
        m_pFT_UM->Hide();
        m_pFT_ANSATZ_REL->Show();
        m_pLB_ANSATZ_REL->Show();
        nEscDir = SdrCaptionEscDir::Vertical;
        break;
    }
}


IMPL_LINK( SvxCaptionTabPage, AnsatzSelectHdl_Impl, ListBox&, rListBox, void )
{
    if (&rListBox == m_pLB_ANSATZ)
    {
        SetupAnsatz_Impl( m_pLB_ANSATZ->GetSelectedEntryPos() );
    }
}

IMPL_LINK( SvxCaptionTabPage, AnsatzRelSelectHdl_Impl, ListBox&, rListBox, void )
{
    if (&rListBox == m_pLB_ANSATZ_REL)
    {
        nAnsatzRelPos = m_pLB_ANSATZ_REL->GetSelectedEntryPos();
    }
}

IMPL_LINK( SvxCaptionTabPage, LineOptHdl_Impl, Button *, pButton, void )
{
    if (pButton == m_pCB_LAENGE)
    {
        if( m_pCB_LAENGE->IsChecked() || ! m_pCB_LAENGE->IsEnabled() )
        {
            m_pFT_LAENGE->Disable();
            m_pMF_LAENGE->Disable();
        }
        else
        {
            m_pFT_LAENGE->Enable();
            m_pMF_LAENGE->Enable();
        }
    }
}


IMPL_LINK_NOARG(SvxCaptionTabPage, SelectCaptTypeHdl_Impl, ValueSet*, void)
{
    SetupType_Impl( static_cast<SdrCaptionType>(m_pCT_CAPTTYPE->GetSelectItemId()) );
}

void SvxCaptionTabPage::SetupType_Impl( SdrCaptionType nType )
{
    switch( nType )
    {
        case SdrCaptionType::Type1:
        m_pFT_LAENGE->Disable();
        m_pCB_LAENGE->Disable();
        LineOptHdl_Impl( m_pCB_LAENGE );
        break;

        case SdrCaptionType::Type2:
        m_pFT_LAENGE->Disable();
        m_pCB_LAENGE->Disable();
        LineOptHdl_Impl( m_pCB_LAENGE );
        break;

        case SdrCaptionType::Type3:
        m_pFT_LAENGE->Enable();
        m_pCB_LAENGE->Enable();
        LineOptHdl_Impl( m_pCB_LAENGE );
        break;

        case SdrCaptionType::Type4:
        m_pFT_LAENGE->Enable();
        m_pCB_LAENGE->Enable();
        LineOptHdl_Impl( m_pCB_LAENGE );
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
