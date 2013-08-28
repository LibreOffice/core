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

#include "PageMarginControl.hxx"
#include "PagePropertyPanel.hxx"
#include "PagePropertyPanel.hrc"

#include <swtypes.hxx>

#include <svx/sidebar/ValueSetWithTextControl.hxx>

#define SWPAGE_LEFT_GVALUE      String("Sw_Page_Left", 12, RTL_TEXTENCODING_ASCII_US)
#define SWPAGE_RIGHT_GVALUE     String("Sw_Page_Right", 13, RTL_TEXTENCODING_ASCII_US)
#define SWPAGE_TOP_GVALUE       String("Sw_Page_Top", 11, RTL_TEXTENCODING_ASCII_US)
#define SWPAGE_DOWN_GVALUE      String("Sw_Page_Down", 12, RTL_TEXTENCODING_ASCII_US)
#define SWPAGE_MIRROR_GVALUE    String("Sw_Page_Mirrored", 16, RTL_TEXTENCODING_ASCII_US)


namespace sw { namespace sidebar {

PageMarginControl::PageMarginControl(
    Window* pParent,
    PagePropertyPanel& rPanel,
    const SvxLongLRSpaceItem& aPageLRMargin,
    const SvxLongULSpaceItem& aPageULMargin,
    const bool bMirrored,
    const Size aPageSize,
    const sal_Bool bLandscape,
    const FieldUnit eFUnit,
    const SfxMapUnit eUnit )
    : ::svx::sidebar::PopupControl( pParent, SW_RES(RID_POPUP_SWPAGE_MARGIN) )
    , mpMarginValueSet( new ::svx::sidebar::ValueSetWithTextControl( ::svx::sidebar::ValueSetWithTextControl::IMAGE_TEXT, this, SW_RES(VS_MARGIN) ) )
    , maCustom(this, SW_RES(FT_CUSTOM))
    , maLeft(this, SW_RES(FT_LEFT))
    , maInner(this, SW_RES(FT_INNER))
    , maLeftMarginEdit(this, SW_RES(MF_SWLEFT_MARGIN))
    , maRight(this, SW_RES(FT_RIGHT))
    , maOuter(this, SW_RES(FT_OUTER))
    , maRightMarginEdit(this, SW_RES(MF_SWRIGHT_MARGIN))
    , maTop(this, SW_RES(FT_TOP))
    , maTopMarginEdit(this, SW_RES(MF_SWTOP_MARGIN))
    , maBottom(this, SW_RES(FT_BOTTOM))
    , maBottomMarginEdit(this, SW_RES(MF_SWBOTTOM_MARGIN))
    , maWidthHeightField( this, SW_RES(FLD_WIDTH_HEIGHT) )
    , mnPageLeftMargin( aPageLRMargin.GetLeft() )
    , mnPageRightMargin( aPageLRMargin.GetRight() )
    , mnPageTopMargin( aPageULMargin.GetUpper() )
    , mnPageBottomMargin( aPageULMargin.GetLower() )
    , mbMirrored( bMirrored )
    , meUnit( eUnit )
    , mbUserCustomValuesAvailable(false)
    , mnUserCustomPageLeftMargin(0)
    , mnUserCustomPageRightMargin(0)
    , mnUserCustomPageTopMargin(0)
    , mnUserCustomPageBottomMargin(0)
    , mbUserCustomMirrored(false)
    , mbCustomValuesUsed( false )
    , mrPagePropPanel(rPanel)
{
    maWidthHeightField.Hide();
    SetFieldUnit( maWidthHeightField, eFUnit );

    mbUserCustomValuesAvailable = GetUserCustomValues();

    mpMarginValueSet->SetStyle( mpMarginValueSet->GetStyle() | WB_3DLOOK | WB_NO_DIRECTSELECT );
    mpMarginValueSet->SetColor( GetSettings().GetStyleSettings().GetMenuColor() );

    FillValueSet( bLandscape, mbUserCustomValuesAvailable );

    mpMarginValueSet->SetNoSelection();
    mpMarginValueSet->SetSelectHdl( LINK(this, PageMarginControl,ImplMarginHdl ) );
    mpMarginValueSet->Show();

    SelectValueSetItem();

    SetFieldUnit( maLeftMarginEdit, eFUnit );
    Link aLinkLR = LINK( this, PageMarginControl, ModifyLRMarginHdl );
    maLeftMarginEdit.SetModifyHdl( aLinkLR );
    SetMetricValue( maLeftMarginEdit, mnPageLeftMargin, meUnit );

    SetFieldUnit( maRightMarginEdit, eFUnit );
    maRightMarginEdit.SetModifyHdl( aLinkLR );
    SetMetricValue( maRightMarginEdit, mnPageRightMargin, meUnit );

    Link aLinkUL = LINK( this, PageMarginControl, ModifyULMarginHdl );
    SetFieldUnit( maTopMarginEdit, eFUnit );
    maTopMarginEdit.SetModifyHdl( aLinkUL );
    SetMetricValue( maTopMarginEdit, mnPageTopMargin, meUnit );

    SetFieldUnit( maBottomMarginEdit, eFUnit );
    maBottomMarginEdit.SetModifyHdl( aLinkUL );
    SetMetricValue( maBottomMarginEdit, mnPageBottomMargin, meUnit );

    SetMetricFieldMaxValues( aPageSize );

    if ( mbMirrored )
    {
        maLeft.Hide();
        maRight.Hide();
        maInner.Show();
        maOuter.Show();
    }
    else
    {
        maLeft.Show();
        maRight.Show();
        maInner.Hide();
        maOuter.Hide();
    }

    FreeResource();
}


PageMarginControl::~PageMarginControl(void)
{
    delete mpMarginValueSet;

    StoreUserCustomValues();
}


void PageMarginControl::SetMetricFieldMaxValues( const Size aPageSize )
{
    const long nML = maLeftMarginEdit.Denormalize( maLeftMarginEdit.GetValue(FUNIT_TWIP) );
    const long nMR = maRightMarginEdit.Denormalize( maRightMarginEdit.GetValue(FUNIT_TWIP) );
    const long nMT = maTopMarginEdit.Denormalize(maTopMarginEdit.GetValue(FUNIT_TWIP) );
    const long nMB = maBottomMarginEdit.Denormalize( maBottomMarginEdit.GetValue(FUNIT_TWIP) );

    const long nPH  = LogicToLogic( aPageSize.Height(), (MapUnit)meUnit, MAP_TWIP );
    const long nPW  = LogicToLogic( aPageSize.Width(),  (MapUnit)meUnit, MAP_TWIP );

    // Left
    long nMax = nPW - nMR - MINBODY;
    maLeftMarginEdit.SetMax(maLeftMarginEdit.Normalize(nMax), FUNIT_TWIP);

    // Right
    nMax = nPW - nML - MINBODY;
    maRightMarginEdit.SetMax(maRightMarginEdit.Normalize(nMax), FUNIT_TWIP);

    //Top
    nMax = nPH - nMB - MINBODY;
    maTopMarginEdit.SetMax(maTopMarginEdit.Normalize(nMax), FUNIT_TWIP);

    //Bottom
    nMax = nPH - nMT -  MINBODY;
    maBottomMarginEdit.SetMax(maTopMarginEdit.Normalize(nMax), FUNIT_TWIP);
}


void PageMarginControl::FillValueSet(
    const bool bLandscape,
    const bool bUserCustomValuesAvailable )
{
    const OUString aLeft = SW_RESSTR(STR_MARGIN_TOOLTIP_LEFT);
    const OUString aRight = SW_RESSTR(STR_MARGIN_TOOLTIP_RIGHT);
    const OUString aTop = SW_RESSTR(STR_MARGIN_TOOLTIP_TOP);
    const OUString aBottom = SW_RESSTR(STR_MARGIN_TOOLTIP_BOT);

    SetMetricValue( maWidthHeightField, SWPAGE_NARROW_VALUE, meUnit );
    const OUString aNarrowValText = maWidthHeightField.GetText();
    OUString aHelpText = aLeft;
    aHelpText += aNarrowValText;
    aHelpText += aRight;
    aHelpText += aNarrowValText;
    aHelpText += aTop;
    aHelpText += aNarrowValText;
    aHelpText += aBottom;
    aHelpText += aNarrowValText;
    mpMarginValueSet->AddItem(
        (bLandscape ? SW_RES(IMG_NARROW_L) : SW_RES(IMG_NARROW) ), 0,
        SW_RESSTR(STR_NARROW), &aHelpText );

    SetMetricValue( maWidthHeightField, SWPAGE_NORMAL_VALUE, meUnit );
    const OUString aNormalValText = maWidthHeightField.GetText();
    aHelpText = aLeft;
    aHelpText += aNormalValText;
    aHelpText += aRight;
    aHelpText += aNormalValText;
    aHelpText += aTop;
    aHelpText += aNormalValText;
    aHelpText += aBottom;
    aHelpText += aNormalValText;
    mpMarginValueSet->AddItem(
        (bLandscape ? SW_RES(IMG_NORMAL_L) : SW_RES(IMG_NORMAL) ), 0,
        SW_RESSTR(STR_NORMAL), &aHelpText );

    SetMetricValue( maWidthHeightField, SWPAGE_WIDE_VALUE1, meUnit );
    const OUString aWide1ValText = maWidthHeightField.GetText();
    SetMetricValue( maWidthHeightField, SWPAGE_WIDE_VALUE2, meUnit );
    const OUString aWide2ValText = maWidthHeightField.GetText();
    aHelpText = aLeft;
    aHelpText += aWide2ValText;
    aHelpText += aRight;
    aHelpText += aWide2ValText;
    aHelpText += aTop;
    aHelpText += aWide1ValText;
    aHelpText += aBottom;
    aHelpText += aWide1ValText;
    mpMarginValueSet->AddItem(
        (bLandscape ? SW_RES(IMG_WIDE_L) : SW_RES(IMG_WIDE) ), 0,
        SW_RESSTR(STR_WIDE), &aHelpText );

    const OUString aInner = SW_RESSTR(STR_MARGIN_TOOLTIP_INNER);
    const OUString aOuter = SW_RESSTR(STR_MARGIN_TOOLTIP_OUTER);

    SetMetricValue( maWidthHeightField, SWPAGE_WIDE_VALUE3, meUnit );
    const OUString aWide3ValText = maWidthHeightField.GetText();
    aHelpText = aInner;
    aHelpText += aWide3ValText;
    aHelpText += aOuter;
    aHelpText += aWide3ValText;
    aHelpText += aTop;
    aHelpText += aWide1ValText;
    aHelpText += aBottom;
    aHelpText += aWide1ValText;
    mpMarginValueSet->AddItem(
        (bLandscape ? SW_RES(IMG_MIRRORED_L) : SW_RES(IMG_MIRRORED) ), 0,
        SW_RESSTR(STR_MIRRORED), &aHelpText );

    if ( bUserCustomValuesAvailable )
    {
        aHelpText = mbUserCustomMirrored ? aInner : aLeft;
        SetMetricValue( maWidthHeightField, mnUserCustomPageLeftMargin, meUnit );
        aHelpText += maWidthHeightField.GetText();
        aHelpText += mbUserCustomMirrored ? aOuter : aRight;
        SetMetricValue( maWidthHeightField, mnUserCustomPageRightMargin, meUnit );
        aHelpText += maWidthHeightField.GetText();
        aHelpText += aTop;
        SetMetricValue( maWidthHeightField, mnUserCustomPageTopMargin, meUnit );
        aHelpText += maWidthHeightField.GetText();
        aHelpText += aBottom;
        SetMetricValue( maWidthHeightField, mnUserCustomPageBottomMargin, meUnit );
        aHelpText += maWidthHeightField.GetText();
    }
    else
    {
        aHelpText = OUString();
    }
    mpMarginValueSet->AddItem(
        (bUserCustomValuesAvailable ? SW_RES(IMG_CUSTOM) : SW_RES(IMG_CUSTOM_DIS) ), 0,
        SW_RESSTR(STR_LCVALUE), &aHelpText );
}


void PageMarginControl::SelectValueSetItem()
{
    const long cTolerance = 5;

    if( abs(mnPageLeftMargin - SWPAGE_NARROW_VALUE) <= cTolerance &&
        abs(mnPageRightMargin - SWPAGE_NARROW_VALUE) <= cTolerance &&
        abs(mnPageTopMargin - SWPAGE_NARROW_VALUE) <= cTolerance &&
        abs(mnPageBottomMargin - SWPAGE_NARROW_VALUE) <= cTolerance &&
        !mbMirrored )
    {
        mpMarginValueSet->SelectItem(1);
    }
    else if( abs(mnPageLeftMargin - SWPAGE_NORMAL_VALUE) <= cTolerance &&
        abs(mnPageRightMargin - SWPAGE_NORMAL_VALUE) <= cTolerance &&
        abs(mnPageTopMargin - SWPAGE_NORMAL_VALUE) <= cTolerance &&
        abs(mnPageBottomMargin - SWPAGE_NORMAL_VALUE) <= cTolerance &&
        !mbMirrored )
    {
        mpMarginValueSet->SelectItem(2);
    }
    else if( abs(mnPageLeftMargin - SWPAGE_WIDE_VALUE2) <= cTolerance &&
        abs(mnPageRightMargin - SWPAGE_WIDE_VALUE2) <= cTolerance &&
        abs(mnPageTopMargin - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        abs(mnPageBottomMargin - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        !mbMirrored )
    {
        mpMarginValueSet->SelectItem(3);
    }
    else if( abs(mnPageLeftMargin - SWPAGE_WIDE_VALUE3) <= cTolerance &&
        abs(mnPageRightMargin - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        abs(mnPageTopMargin - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        abs(mnPageBottomMargin - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        mbMirrored )
    {
        mpMarginValueSet->SelectItem(4);
    }
    else
    {
        mpMarginValueSet->SelectItem(0);
    }

    mpMarginValueSet->Format();
    mpMarginValueSet->StartSelection();
};


IMPL_LINK(PageMarginControl, ImplMarginHdl, void *, pControl)
{
    if ( pControl == mpMarginValueSet )
    {
        const sal_uInt16 iPos = mpMarginValueSet->GetSelectItemId();
        bool bMirrored = false;
        bool bApplyNewPageMargins = true;
        switch ( iPos )
        {
        case 1:
            mnPageLeftMargin = SWPAGE_NARROW_VALUE;
            mnPageRightMargin = SWPAGE_NARROW_VALUE;
            mnPageTopMargin = SWPAGE_NARROW_VALUE;
            mnPageBottomMargin = SWPAGE_NARROW_VALUE;
            bMirrored = false;
            break;
        case 2:
            mnPageLeftMargin = SWPAGE_NORMAL_VALUE;
            mnPageRightMargin = SWPAGE_NORMAL_VALUE;
            mnPageTopMargin = SWPAGE_NORMAL_VALUE;
            mnPageBottomMargin = SWPAGE_NORMAL_VALUE;
            bMirrored = false;
            break;
        case 3:
            mnPageLeftMargin = SWPAGE_WIDE_VALUE2;
            mnPageRightMargin = SWPAGE_WIDE_VALUE2;
            mnPageTopMargin = SWPAGE_WIDE_VALUE1;
            mnPageBottomMargin = SWPAGE_WIDE_VALUE1;
            bMirrored = false;
            break;
        case 4:
            mnPageLeftMargin = SWPAGE_WIDE_VALUE3;
            mnPageRightMargin = SWPAGE_WIDE_VALUE1;
            mnPageTopMargin = SWPAGE_WIDE_VALUE1;
            mnPageBottomMargin = SWPAGE_WIDE_VALUE1;
            bMirrored = true;
            break;
        case 5:
            if ( mbUserCustomValuesAvailable )
            {
                mnPageLeftMargin = mnUserCustomPageLeftMargin;
                mnPageRightMargin = mnUserCustomPageRightMargin;
                mnPageTopMargin = mnUserCustomPageTopMargin;
                mnPageBottomMargin = mnUserCustomPageBottomMargin;
                bMirrored = mbUserCustomMirrored;
            }
            else
            {
                bApplyNewPageMargins = false;
            }
            break;
        }

        if ( bApplyNewPageMargins )
        {
            mrPagePropPanel.StartUndo();
            mpMarginValueSet->SetNoSelection();
            mrPagePropPanel.ExecuteMarginLRChange( mnPageLeftMargin, mnPageRightMargin );
            mrPagePropPanel.ExecuteMarginULChange( mnPageTopMargin, mnPageBottomMargin );
            if ( mbMirrored != bMirrored )
            {
                mbMirrored = bMirrored;
                mrPagePropPanel.ExecutePageLayoutChange( mbMirrored );
            }
            mrPagePropPanel.EndUndo();

            mbCustomValuesUsed = false;
            mrPagePropPanel.ClosePageMarginPopup();
        }
        else
        {
            // back to initial selection
            SelectValueSetItem();
        }
    }

    return 0;
}


IMPL_LINK( PageMarginControl, ModifyLRMarginHdl, MetricField *, EMPTYARG )
{
    mpMarginValueSet->SetNoSelection();
    mpMarginValueSet->SelectItem(0);
    mpMarginValueSet->Format();
    mpMarginValueSet->StartSelection();

    mnPageLeftMargin = GetCoreValue( maLeftMarginEdit, meUnit );
    mnPageRightMargin = GetCoreValue( maRightMarginEdit, meUnit );
    mrPagePropPanel.ExecuteMarginLRChange( mnPageLeftMargin, mnPageRightMargin );
    mbCustomValuesUsed = true;
    return 0;
}

IMPL_LINK( PageMarginControl, ModifyULMarginHdl, MetricField *, EMPTYARG )
{
    mpMarginValueSet->SetNoSelection();
    mpMarginValueSet->SelectItem(0);
    mpMarginValueSet->Format();
    mpMarginValueSet->StartSelection();

    mnPageTopMargin = GetCoreValue( maTopMarginEdit, meUnit );
    mnPageBottomMargin = GetCoreValue( maBottomMarginEdit, meUnit );
    mrPagePropPanel.ExecuteMarginULChange( mnPageTopMargin, mnPageBottomMargin );
    mbCustomValuesUsed = true;
    return 0;
}


bool PageMarginControl::GetUserCustomValues()
{
    bool bUserCustomValuesAvailable = false;

    SvtViewOptions aWinOpt( E_WINDOW, SWPAGE_LEFT_GVALUE );
    if ( aWinOpt.Exists() )
    {
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq = aWinOpt.GetUserData();
        ::rtl::OUString aTmp;
        if ( aSeq.getLength())
            aSeq[0].Value >>= aTmp;
        String aWinData( aTmp );
        mnUserCustomPageLeftMargin = aWinData.ToInt32();
        bUserCustomValuesAvailable = true;
    }

    SvtViewOptions aWinOpt2( E_WINDOW, SWPAGE_RIGHT_GVALUE );
    if ( aWinOpt2.Exists() )
    {
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq = aWinOpt2.GetUserData();
        ::rtl::OUString aTmp;
        if ( aSeq.getLength())
            aSeq[0].Value >>= aTmp;
        String aWinData( aTmp );
        mnUserCustomPageRightMargin = aWinData.ToInt32();
        bUserCustomValuesAvailable = true;
    }

    SvtViewOptions aWinOpt3( E_WINDOW, SWPAGE_TOP_GVALUE );
    if ( aWinOpt3.Exists() )
    {
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq = aWinOpt3.GetUserData();
        ::rtl::OUString aTmp;
        if ( aSeq.getLength())
            aSeq[0].Value >>= aTmp;
        String aWinData( aTmp );
        mnUserCustomPageTopMargin = aWinData.ToInt32();
        bUserCustomValuesAvailable = true;
    }

    SvtViewOptions aWinOpt4( E_WINDOW, SWPAGE_DOWN_GVALUE );
    if ( aWinOpt4.Exists() )
    {
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq = aWinOpt4.GetUserData();
        ::rtl::OUString aTmp;
        if ( aSeq.getLength())
            aSeq[0].Value >>= aTmp;
        String aWinData( aTmp );
        mnUserCustomPageBottomMargin = aWinData.ToInt32();
        bUserCustomValuesAvailable = true;
    }

    SvtViewOptions aWinOpt5( E_WINDOW, SWPAGE_MIRROR_GVALUE );
    if ( aWinOpt5.Exists() )
    {
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq = aWinOpt5.GetUserData();
        ::rtl::OUString aTmp;
        if ( aSeq.getLength())
            aSeq[0].Value >>= aTmp;
        String aWinData( aTmp );
        mbUserCustomMirrored = aWinData.ToInt32() == 0 ? false : true;
        bUserCustomValuesAvailable = true;
    }

    return bUserCustomValuesAvailable;
}

void PageMarginControl::StoreUserCustomValues()
{
    if ( !mbCustomValuesUsed )
    {
        return;
    }

    ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq(1);
    SvtViewOptions aWinOpt( E_WINDOW, SWPAGE_LEFT_GVALUE );

    aSeq[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("mnPageLeftMargin") );
    aSeq[0].Value <<= ::rtl::OUString::number( mnPageLeftMargin );
    aWinOpt.SetUserData( aSeq );

    SvtViewOptions aWinOpt2( E_WINDOW, SWPAGE_RIGHT_GVALUE );
    aSeq[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("mnPageRightMargin") );
    aSeq[0].Value <<= ::rtl::OUString::number( mnPageRightMargin );
    aWinOpt2.SetUserData( aSeq );

    SvtViewOptions aWinOpt3( E_WINDOW, SWPAGE_TOP_GVALUE );
    aSeq[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("mnPageTopMargin") );
    aSeq[0].Value <<= ::rtl::OUString::number( mnPageTopMargin );
    aWinOpt3.SetUserData( aSeq );

    SvtViewOptions aWinOpt4( E_WINDOW, SWPAGE_DOWN_GVALUE );
    aSeq[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("mnPageBottomMargin") );
    aSeq[0].Value <<= ::rtl::OUString::number( mnPageBottomMargin );
    aWinOpt4.SetUserData( aSeq );

    SvtViewOptions aWinOpt5( E_WINDOW, SWPAGE_MIRROR_GVALUE );
    aSeq[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("mbMirrored") );
    aSeq[0].Value <<= ::rtl::OUString::number( (mbMirrored ? 1 : 0) );
    aWinOpt5.SetUserData( aSeq );
}


} } // end of namespace sw::sidebar

