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

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/builder.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <unotools/localedatawrapper.hxx>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/mslangid.hxx>
#include <svx/dialogs.hrc>
#include <svtools/colorcfg.hxx>

#include <strings.hrc>

#include <svx/numinf.hxx>

#include <numfmt.hxx>
#include <svx/numfmtsh.hxx>
#include <dialmgr.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/flagsdef.hxx>
#include <vector>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <memory>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::uno::UNO_QUERY;

#define NUMKEY_UNDEFINED SAL_MAX_UINT32

// static ----------------------------------------------------------------

const sal_uInt16 SvxNumberFormatTabPage::pRanges[] =
{
    SID_ATTR_NUMBERFORMAT_VALUE,
    SID_ATTR_NUMBERFORMAT_INFO,
    SID_ATTR_NUMBERFORMAT_NOLANGUAGE,
    SID_ATTR_NUMBERFORMAT_NOLANGUAGE,
    SID_ATTR_NUMBERFORMAT_ONE_AREA,
    SID_ATTR_NUMBERFORMAT_ONE_AREA,
    SID_ATTR_NUMBERFORMAT_SOURCE,
    SID_ATTR_NUMBERFORMAT_SOURCE,
    0
};

/*************************************************************************
#*  Method:        SvxNumberPreview
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberPreview
#*  Function:   Constructor of the class SvxNumberPreview
#*  Input:      Window, Resource-ID
#*  Output:     ---
#*
#************************************************************************/

SvxNumberPreview::SvxNumberPreview()
    : mnPos(-1)
    , mnChar(0x0)
{
}

/*************************************************************************
#*  Method:        NotifyChange
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberPreview
#*  Function:   Function for changing the preview string
#*  Input:      String, color
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberPreview::NotifyChange( const OUString& rPrevStr,
                                         const Color* pColor )
{
    // detect and strip out '*' related placeholders
    aPrevStr = rPrevStr;
    mnPos = aPrevStr.indexOf( 0x1B );
    if ( mnPos != -1 )
    {
        // Right during user input the star symbol is the very
        // last character before the user enters another one.
        if (mnPos < aPrevStr.getLength() - 1)
        {
            mnChar = aPrevStr[ mnPos + 1 ];
            // delete placeholder and char to repeat
            aPrevStr = aPrevStr.replaceAt( mnPos, 2, "" );
        }
        else
        {
            // delete placeholder
            aPrevStr = aPrevStr.replaceAt( mnPos, 1, "" );
            // do not attempt to draw a 0 fill character
            mnPos = -1;
        }
    }
    svtools::ColorConfig aColorConfig;
    Color aWindowTextColor( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
    aPrevCol = pColor ? *pColor : aWindowTextColor;
    Invalidate();
}

/*************************************************************************
#*  Method:        Paint
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberPreview
#*  Function:   Function for repainting the window.
#*  Input:      ---
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberPreview::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    rRenderContext.Push(PushFlags::ALL);

    svtools::ColorConfig aColorConfig;
    rRenderContext.SetTextColor(aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor);
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    rRenderContext.SetBackground(rStyleSettings.GetWindowColor());

    vcl::Font aDrawFont = rRenderContext.GetFont();
    Size aSzWnd(GetOutputSizePixel());
    OUString aTmpStr( aPrevStr );
    long nLeadSpace = (aSzWnd.Width() - rRenderContext.GetTextWidth(aTmpStr)) / 2;

    aDrawFont.SetTransparent(true);
    aDrawFont.SetColor(aPrevCol);
    rRenderContext.SetFont(aDrawFont);

    if (mnPos != -1)
    {
        long nCharWidth = rRenderContext.GetTextWidth(OUString(mnChar));

        int nNumCharsToInsert = 0;
        if (nCharWidth > 0)
            nNumCharsToInsert = nLeadSpace / nCharWidth;

        if (nNumCharsToInsert > 0)
        {
            for (int i = 0; i < nNumCharsToInsert; ++i)
                aTmpStr = aTmpStr.replaceAt(mnPos, 0, OUString(mnChar));
        }
    }
    Point aPosText = Point((mnPos != -1) ? 0 : nLeadSpace,
                           (aSzWnd.Height() - GetTextHeight()) / 2);
    rRenderContext.DrawText(aPosText, aTmpStr);
    rRenderContext.Pop();
}

// class SvxNumberFormatTabPage ------------------------------------------

#define REMOVE_DONTKNOW() \
    if (!m_xFtLanguage->get_sensitive())                                 \
    {                                                                    \
        m_xFtLanguage->set_sensitive(true);                              \
        m_xLbLanguage->set_sensitive(true);                              \
        m_xLbLanguage->SelectLanguage( pNumFmtShell->GetCurLanguage() ); \
    }

#define HDL(hdl) LINK( this, SvxNumberFormatTabPage, hdl )

SvxNumberFormatTabPage::SvxNumberFormatTabPage(TabPageParent pParent,
    const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pParent, "cui/ui/numberingformatpage.ui", "NumberingFormatPage", &rCoreAttrs)
    , nInitFormat(ULONG_MAX)
    , sAutomaticEntry(CuiResId(RID_SVXSTR_AUTO_ENTRY))
    , pLastActivWindow(nullptr)
    , m_xFtCategory(m_xBuilder->weld_label("categoryft"))
    , m_xLbCategory(m_xBuilder->weld_tree_view("categorylb"))
    , m_xFtFormat(m_xBuilder->weld_label("formatft"))
    , m_xLbCurrency(m_xBuilder->weld_combo_box("currencylb"))
    , m_xLbFormat(m_xBuilder->weld_tree_view("formatlb"))
    , m_xFtLanguage(m_xBuilder->weld_label("languageft"))
    , m_xCbSourceFormat(m_xBuilder->weld_check_button("sourceformat"))
    , m_xFtOptions(m_xBuilder->weld_label("optionsft"))
    , m_xFtDecimals(m_xBuilder->weld_label("decimalsft"))
    , m_xEdDecimals(m_xBuilder->weld_spin_button("decimalsed"))
    , m_xFtDenominator(m_xBuilder->weld_label("denominatorft"))
    , m_xEdDenominator(m_xBuilder->weld_spin_button("denominatored"))
    , m_xBtnNegRed(m_xBuilder->weld_check_button("negnumred"))
    , m_xFtLeadZeroes(m_xBuilder->weld_label("leadzerosft"))
    , m_xEdLeadZeroes(m_xBuilder->weld_spin_button("leadzerosed"))
    , m_xBtnThousand(m_xBuilder->weld_check_button("thousands"))
    , m_xBtnEngineering(m_xBuilder->weld_check_button("engineering"))
    , m_xFormatCodeFrame(m_xBuilder->weld_widget("formatcode"))
    , m_xEdFormat(m_xBuilder->weld_entry("formatted"))
    , m_xIbAdd(m_xBuilder->weld_button("add"))
    , m_xIbInfo(m_xBuilder->weld_button("edit"))
    , m_xIbRemove(m_xBuilder->weld_button("delete"))
    , m_xFtComment(m_xBuilder->weld_label("commentft"))
    , m_xEdComment(m_xBuilder->weld_entry("commented"))
    , m_xLbLanguage(new LanguageBox(m_xBuilder->weld_combo_box("languagelb")))
    , m_xWndPreview(new weld::CustomWeld(*m_xBuilder, "preview", m_aWndPreview))
{
    long nWidth = approximate_char_width() * 26;
    m_xLbFormat->set_size_request(nWidth, -1);
    m_xLbCurrency->set_size_request(nWidth, -1);
    m_xLbCategory->set_size_request(-1, m_xLbCategory->get_height_rows(8));
    m_xWndPreview->set_size_request(GetTextHeight()*3, -1);

    Init_Impl();
    SetExchangeSupport(); // this page needs ExchangeSupport
    nFixedCategory=-1;
}

SvxNumberFormatTabPage::~SvxNumberFormatTabPage()
{
    disposeOnce();
}

void SvxNumberFormatTabPage::dispose()
{
    pNumFmtShell.reset();
    pNumItem.reset();
    m_xWndPreview.reset();
    m_xLbLanguage.reset();
    pLastActivWindow.clear();
    SfxTabPage::dispose();
}

void SvxNumberFormatTabPage::Init_Impl()
{
    bNumItemFlag=true;
    bOneAreaFlag=false;

    m_xIbAdd->set_sensitive(false );
    m_xIbRemove->set_sensitive(false );
    m_xIbInfo->set_sensitive(false );

    m_xEdComment->set_text(m_xLbCategory->get_text(1));    // string for user defined

    m_xEdComment->hide();

    m_xCbSourceFormat->set_active( false );
    m_xCbSourceFormat->set_sensitive(false);
    m_xCbSourceFormat->hide();

    Link<weld::TreeView&,void> aLink2 = LINK(this, SvxNumberFormatTabPage, SelFormatTreeListBoxHdl_Impl);
    Link<weld::ComboBox&,void> aLink3 = LINK(this, SvxNumberFormatTabPage, SelFormatListBoxHdl_Impl);
    m_xLbCategory->connect_changed(aLink2);
    m_xLbFormat->connect_changed(aLink2);
    m_xLbLanguage->connect_changed(aLink3);
    m_xLbCurrency->connect_changed(aLink3);
    m_xCbSourceFormat->connect_toggled(LINK(this, SvxNumberFormatTabPage, SelFormatClickHdl_Impl));

    Link<weld::SpinButton&,void> aLink = LINK( this, SvxNumberFormatTabPage, OptEditHdl_Impl );

    m_xEdDecimals->connect_value_changed(aLink);
    m_xEdDenominator->connect_value_changed(aLink);
    m_xEdLeadZeroes->connect_value_changed(aLink);

    m_xBtnNegRed->connect_toggled(LINK(this, SvxNumberFormatTabPage, OptClickHdl_Impl));
    m_xBtnThousand->connect_toggled(LINK(this, SvxNumberFormatTabPage, OptClickHdl_Impl));
    m_xBtnEngineering->connect_toggled(LINK(this, SvxNumberFormatTabPage, OptClickHdl_Impl));
    m_xLbFormat->connect_row_activated(HDL(DoubleClickHdl_Impl));
    m_xEdFormat->connect_changed(HDL(EditModifyHdl_Impl));
    m_xIbAdd->connect_clicked(HDL(ClickHdl_Impl));
    m_xIbRemove->connect_clicked(HDL(ClickHdl_Impl));
    m_xIbInfo->connect_clicked(HDL(ClickHdl_Impl));
    UpdateThousandEngineeringCheckBox();
    UpdateDecimalsDenominatorEditBox();

    m_xEdComment->connect_focus_out(LINK(this, SvxNumberFormatTabPage, LostFocusHdl_Impl));
    aResetWinTimer.SetInvokeHandler(LINK( this, SvxNumberFormatTabPage, TimeHdl_Impl));
    aResetWinTimer.SetTimeout(10);

    // initialize language ListBox

    m_xLbLanguage->SetLanguageList( SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN, false);
    m_xLbLanguage->InsertLanguage( LANGUAGE_SYSTEM );
}

VclPtr<SfxTabPage> SvxNumberFormatTabPage::Create( TabPageParent pParent,
                                                   const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxNumberFormatTabPage>::Create(pParent, *rAttrSet);
}


/*************************************************************************
#*  Method:        Reset
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   The dialog's attributes are reset
#*              using the Itemset.
#*  Input:      SfxItemSet
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::Reset( const SfxItemSet* rSet )
{
    const SfxUInt32Item*        pValFmtAttr     = nullptr;
    const SfxPoolItem*          pItem           = nullptr;
    const SfxBoolItem*          pAutoEntryAttr = nullptr;

    sal_uInt16                  nCatLbSelPos    = 0;
    sal_uInt16                  nFmtLbSelPos    = 0;
    LanguageType                eLangType       = LANGUAGE_DONTKNOW;
    std::vector<OUString>       aFmtEntryList;
    SvxNumberValueType          eValType        = SvxNumberValueType::Undefined;
    double                      nValDouble      = 0;
    OUString                    aValString;

    SfxItemState eState = rSet->GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_NOLANGUAGE ),true,&pItem);

    if(eState==SfxItemState::SET)
    {
        const SfxBoolItem* pBoolLangItem = static_cast<const SfxBoolItem*>(
                      GetItem( *rSet, SID_ATTR_NUMBERFORMAT_NOLANGUAGE));

        if(pBoolLangItem!=nullptr && pBoolLangItem->GetValue())
        {
            HideLanguage();
        }
        else
        {
            HideLanguage(false);
        }

    }

    eState = rSet->GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_INFO ),true,&pItem);

    if(eState==SfxItemState::SET)
    {
        if(pNumItem==nullptr)
        {
            bNumItemFlag=true;
            pNumItem.reset( static_cast<SvxNumberInfoItem *>(pItem->Clone()) );
        }
        else
        {
            bNumItemFlag=false;
        }
    }
    else
    {
        bNumItemFlag=false;
    }


    eState = rSet->GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_ONE_AREA ));

    if(eState==SfxItemState::SET)
    {
        const SfxBoolItem* pBoolItem = static_cast<const SfxBoolItem*>(
                      GetItem( *rSet, SID_ATTR_NUMBERFORMAT_ONE_AREA));

        if(pBoolItem!=nullptr)
        {
            bOneAreaFlag= pBoolItem->GetValue();
        }
    }

    eState = rSet->GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_SOURCE ) );

    if ( eState == SfxItemState::SET )
    {
        const SfxBoolItem* pBoolItem = static_cast<const SfxBoolItem*>(
                      GetItem( *rSet, SID_ATTR_NUMBERFORMAT_SOURCE ));
        if ( pBoolItem )
            m_xCbSourceFormat->set_active(pBoolItem->GetValue());
        else
            m_xCbSourceFormat->set_active( false );
        m_xCbSourceFormat->set_sensitive(true);
        m_xCbSourceFormat->show();
    }
    else
    {
        bool bInit = false;     // set to sal_True for debug test
        m_xCbSourceFormat->set_active( bInit );
        m_xCbSourceFormat->set_sensitive( bInit );
        m_xCbSourceFormat->show( bInit );
    }

    // pNumItem must have been set from outside!
    DBG_ASSERT( pNumItem, "No NumberInfo, no NumberFormatter, good bye.CRASH. :-(" );

    eState = rSet->GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_VALUE ) );

    if ( SfxItemState::DONTCARE != eState )
        pValFmtAttr = GetItem( *rSet, SID_ATTR_NUMBERFORMAT_VALUE );

    eValType = pNumItem->GetValueType();

    switch ( eValType )
    {
        case SvxNumberValueType::String:
            aValString = pNumItem->GetValueString();
            break;
        case SvxNumberValueType::Number:
            //  #50441# string may be set in addition to the value
            aValString = pNumItem->GetValueString();
            nValDouble = pNumItem->GetValueDouble();
            break;
        case SvxNumberValueType::Undefined:
        default:
            break;
    }

    pNumFmtShell.reset();   // delete old shell if applicable (== reset)

    nInitFormat = pValFmtAttr                   // memorize init key
                    ? pValFmtAttr->GetValue()   // (for FillItemSet())
                    : ULONG_MAX;                // == DONT_KNOW


    if ( eValType == SvxNumberValueType::String )
        pNumFmtShell.reset( SvxNumberFormatShell::Create(
                                pNumItem->GetNumberFormatter(),
                                pValFmtAttr ? nInitFormat : 0,
                                eValType,
                                aValString ) );
    else
        pNumFmtShell.reset( SvxNumberFormatShell::Create(
                                pNumItem->GetNumberFormatter(),
                                pValFmtAttr ? nInitFormat : 0,
                                eValType,
                                nValDouble,
                                &aValString ) );


    bool bUseStarFormat = false;
    SfxObjectShell* pDocSh  = SfxObjectShell::Current();
    if ( pDocSh )
    {
        // is this a calc document
        Reference< XServiceInfo > xSI( pDocSh->GetModel(), UNO_QUERY );
        if ( xSI.is() )
            bUseStarFormat = xSI->supportsService("com.sun.star.sheet.SpreadsheetDocument");
    }
    pNumFmtShell->SetUseStarFormat( bUseStarFormat );

    FillCurrencyBox();

    OUString aPrevString;
    Color* pDummy = nullptr;
    pNumFmtShell->GetInitSettings( nCatLbSelPos, eLangType, nFmtLbSelPos,
                                   aFmtEntryList, aPrevString, pDummy );

    if (nCatLbSelPos==CAT_CURRENCY)
    {
        sal_Int32 nPos = pNumFmtShell->GetCurrencySymbol();
        if (nPos == 0)
        {
            // Enable "Automatically" if currently used so it is selectable.
//TODO            m_xLbCurrency->SetEntryFlags( nPos, ListBoxEntryFlags::NONE );
        }
        m_xLbCurrency->set_active(nPos);
    }

    nFixedCategory=nCatLbSelPos;
    if(bOneAreaFlag)
    {
        OUString sFixedCategory = m_xLbCategory->get_text(nFixedCategory);
        m_xLbCategory->clear();
        m_xLbCategory->append_text(sFixedCategory);
        SetCategory(0);
    }
    else
    {
        SetCategory(nCatLbSelPos );
    }
    eState = rSet->GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_ADD_AUTO ) );
    if(SfxItemState::SET == eState)
         pAutoEntryAttr = static_cast<const SfxBoolItem*>(
                      GetItem( *rSet, SID_ATTR_NUMBERFORMAT_ADD_AUTO ));
    // no_NO is an alias for nb_NO and normally isn't listed, we need it for
    // backwards compatibility, but only if the format passed is of
    // LanguageType no_NO.
    if ( eLangType == LANGUAGE_NORWEGIAN )
    {
        m_xLbLanguage->RemoveLanguage( eLangType );    // in case we're already called
        m_xLbLanguage->InsertLanguage( eLangType );
    }
    m_xLbLanguage->SelectLanguage( eLangType );
    if(pAutoEntryAttr)
        AddAutomaticLanguage_Impl(eLangType, pAutoEntryAttr->GetValue());
    UpdateFormatListBox_Impl(false,true);

//! This spoils everything because it rematches currency formats based on
//! the selected m_xLbCurrency entry instead of the current format.
//! Besides that everything seems to be initialized by now, so why call it?
//  SelFormatHdl_Impl( m_xLbCategory );

    if ( pValFmtAttr )
    {
        EditHdl_Impl(m_xEdFormat.get()); // UpdateOptions_Impl() as a side effect
    }
    else    // DONT_KNOW
    {
        // everything disabled except direct input or changing the category
        Obstructing();
    }

    if ( m_xCbSourceFormat->get_active() )
    {
        // everything disabled except SourceFormat checkbox
        EnableBySourceFormat_Impl();
    }
}

/*************************************************************************
#*  Method:        Obstructing
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Disable the controls except from changing the category
#*              and direct input.
#*  Input:      ---
#*  Output:     ---
#*
#************************************************************************/
void SvxNumberFormatTabPage::Obstructing()
{
    m_xLbFormat->select(-1);
    m_xLbLanguage->set_active(-1);
    m_xFtLanguage->set_sensitive(false);
    m_xLbLanguage->set_sensitive(false);

    m_xIbAdd->set_sensitive(false );
    m_xIbRemove->set_sensitive(false );
    m_xIbInfo->set_sensitive(false );

    m_xBtnNegRed->set_sensitive(false);
    m_xBtnThousand->set_sensitive(false);
    m_xBtnEngineering->set_sensitive(false);
    m_xFtLeadZeroes->set_sensitive(false);
    m_xFtDecimals->set_sensitive(false);
    m_xFtDenominator->set_sensitive(false);
    m_xEdLeadZeroes->set_sensitive(false);
    m_xEdDecimals->set_sensitive(false);
    m_xEdDenominator->set_sensitive(false);
    m_xFtOptions->set_sensitive(false);
    m_xEdDecimals->set_text( OUString() );
    m_xEdLeadZeroes->set_text( OUString() );
    m_xBtnNegRed->set_active( false );
    m_xBtnThousand->set_active( false );
    m_xBtnEngineering->set_active( false );
    m_aWndPreview.NotifyChange( OUString() );

    m_xLbCategory->select(0);
    m_xEdFormat->set_text( OUString() );
    m_xFtComment->set_label( OUString() );
    m_xEdComment->set_text(m_xLbCategory->get_text(1));  // string for user defined

    m_xEdFormat->grab_focus();
}


/*************************************************************************
#* Enable/Disable dialog parts depending on the value of the SourceFormat
#* checkbox.
#************************************************************************/
void SvxNumberFormatTabPage::EnableBySourceFormat_Impl()
{
    bool bEnable = !m_xCbSourceFormat->get_active();
    if ( !bEnable )
        m_xCbSourceFormat->grab_focus();
    m_xFtCategory->set_sensitive( bEnable );
    m_xLbCategory->set_sensitive( bEnable );
    m_xFtFormat->set_sensitive( bEnable );
    m_xLbCurrency->set_sensitive( bEnable );
    m_xLbFormat->set_sensitive( bEnable );
    m_xFtLanguage->set_sensitive( bEnable );
    m_xLbLanguage->set_sensitive( bEnable );
    m_xFtDecimals->set_sensitive( bEnable );
    m_xEdDecimals->set_sensitive( bEnable );
    m_xFtDenominator->set_sensitive( bEnable );
    m_xEdDenominator->set_sensitive( bEnable );
    m_xFtLeadZeroes->set_sensitive( bEnable );
    m_xEdLeadZeroes->set_sensitive( bEnable );
    m_xBtnNegRed->set_sensitive( bEnable );
    m_xBtnThousand->set_sensitive( bEnable );
    m_xBtnEngineering->set_sensitive( bEnable );
    m_xFtOptions->set_sensitive( bEnable );
    m_xFormatCodeFrame->set_sensitive( bEnable );
    m_xLbFormat->Invalidate(); // #i43322#
}


/*************************************************************************
#*  Method:    HideLanguage
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Hides the language settings:
#*  Input:      sal_Bool nFlag
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::HideLanguage(bool bFlag)
{
    m_xFtLanguage->show(!bFlag);
    m_xLbLanguage->show(!bFlag);
}

/*************************************************************************
#*  Method:        FillItemSet
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Adjusts the attributes in the ItemSet,
#*              and - if bNumItemFlag is not set - the
#*              numItem in the DocShell.
#*  Input:      SfxItemSet
#*  Output:     ---
#*
#************************************************************************/

bool SvxNumberFormatTabPage::FillItemSet( SfxItemSet* rCoreAttrs )
{
    bool bDataChanged   = m_xFtLanguage->get_sensitive() || m_xCbSourceFormat->get_sensitive();
    if ( bDataChanged )
    {
        const SfxItemSet& rMyItemSet = GetItemSet();
        sal_uInt16          nWhich       = GetWhich( SID_ATTR_NUMBERFORMAT_VALUE );
        SfxItemState    eItemState   = rMyItemSet.GetItemState( nWhich, false );

        // OK chosen - Is format code input entered already taken over?
        // If not, simulate Add. Upon syntax error ignore input and prevent Put.
        OUString    aFormat = m_xEdFormat->get_text();
        sal_uInt32 nCurKey = pNumFmtShell->GetCurNumFmtKey();

        if ( m_xIbAdd->get_sensitive() || pNumFmtShell->IsTmpCurrencyFormat(aFormat) )
        {   // #79599# It is not sufficient to just add the format code (or
            // delete it in case of bOneAreaFlag and resulting category change).
            // Upon switching tab pages we need all settings to be consistent
            // in case this page will be redisplayed later.
            bDataChanged = Click_Impl(*m_xIbAdd);
            nCurKey = pNumFmtShell->GetCurNumFmtKey();
        }
        else if(nCurKey == NUMKEY_UNDEFINED)
        {   // something went wrong, e.g. in Writer #70281#
            pNumFmtShell->FindEntry(aFormat, &nCurKey);
        }


        // Chosen format:

        if ( bDataChanged )
        {
            bDataChanged = ( nInitFormat != nCurKey );

            if (bDataChanged)
            {
                rCoreAttrs->Put( SfxUInt32Item( nWhich, nCurKey ) );
            }
            else if(SfxItemState::DEFAULT == eItemState)
            {
                rCoreAttrs->ClearItem( nWhich );
            }
        }


        // List of changed user defined formats:

        std::vector<sal_uInt32> const & aDelFormats = pNumFmtShell->GetUpdateData();

        if ( !aDelFormats.empty() )
        {

            pNumItem->SetDelFormats( aDelFormats );

            if(bNumItemFlag)
            {
                rCoreAttrs->Put( *pNumItem );
            }
            else
            {
                SfxObjectShell* pDocSh  = SfxObjectShell::Current();

                DBG_ASSERT( pDocSh, "DocShell not found!" );


                if ( pDocSh )
                    pDocSh->PutItem( *pNumItem );
            }
        }


        // Whether source format is to be taken or not:

        if ( m_xCbSourceFormat->get_sensitive() )
        {
            sal_uInt16 _nWhich = GetWhich( SID_ATTR_NUMBERFORMAT_SOURCE );
            SfxItemState _eItemState = rMyItemSet.GetItemState( _nWhich, false );
            const SfxBoolItem* pBoolItem = static_cast<const SfxBoolItem*>(
                        GetItem( rMyItemSet, SID_ATTR_NUMBERFORMAT_SOURCE ));
            bool bOld = pBoolItem && pBoolItem->GetValue();
            rCoreAttrs->Put( SfxBoolItem( _nWhich, m_xCbSourceFormat->get_active() ) );
            if ( !bDataChanged )
                bDataChanged = (bOld != m_xCbSourceFormat->get_active() ||
                    _eItemState != SfxItemState::SET);
        }

        // FillItemSet is only called on OK, here we can notify the
        // NumberFormatShell that all new user defined formats are valid.
        pNumFmtShell->ValidateNewEntries();
        if(m_xLbLanguage->get_visible() &&
                m_xLbLanguage->find_text(sAutomaticEntry) != -1)
                rCoreAttrs->Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ADD_AUTO,
                    m_xLbLanguage->GetSelectedEntry() == sAutomaticEntry));
    }

    return bDataChanged;
}


DeactivateRC SvxNumberFormatTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

void SvxNumberFormatTabPage::FillFormatListBox_Impl( std::vector<OUString>& rEntries )
{
    OUString    aEntry;
    OUString    aTmpString;
    vcl::Font   aFont=m_xLbCategory->GetFont();
    size_t      i = 0;
    short       nTmpCatPos;

    m_xLbFormat->Clear();
    m_xLbFormat->SetUpdateMode( false );

    if( rEntries.empty() )
        return;

    if(bOneAreaFlag)
    {
        nTmpCatPos=nFixedCategory;
    }
    else
    {
        nTmpCatPos=m_xLbCategory->GetSelectedEntryPos();
    }

    switch (nTmpCatPos)
    {
        case CAT_ALL:
        case CAT_TEXT:
        case CAT_NUMBER:        i=1;
                                aEntry=rEntries[0];
                                if (nTmpCatPos == CAT_TEXT)
                                    aTmpString=aEntry;
                                else
                                    aTmpString = pNumFmtShell->GetStandardName();
                                m_xLbFormat->InsertFontEntry( aTmpString, aFont );
                                break;

        default:                break;
    }

    if(pNumFmtShell!=nullptr)
    {
        for ( ; i < rEntries.size(); ++i )
        {
            aEntry = rEntries[i];
            short aPrivCat = pNumFmtShell->GetCategory4Entry( static_cast<short>(i) );
            if(aPrivCat!=CAT_TEXT)
            {
                Color* pPreviewColor = nullptr;
                OUString aPreviewString( GetExpColorString( pPreviewColor, aEntry, aPrivCat ) );
                vcl::Font aEntryFont( m_xLbFormat->GetFont() );
                m_xLbFormat->InsertFontEntry( aPreviewString, aEntryFont, pPreviewColor );
            }
            else
            {
                m_xLbFormat->InsertFontEntry(aEntry,aFont);
            }
        }
    }
    m_xLbFormat->SetUpdateMode( true );
    rEntries.clear();
}

/*************************************************************************
#*  Method:        UpdateOptions_Impl
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Adjusts the options attributes
#*              depending on the selected format.
#*  Input:      Flag, whether the category has changed.
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::UpdateOptions_Impl( bool bCheckCatChange /*= sal_False*/ )
{
    OUString    theFormat           = m_xEdFormat->get_text();
    sal_Int32   nCurCategory        = m_xLbCategory->GetSelectedEntryPos();
    sal_uInt16  nCategory           = static_cast<sal_uInt16>(nCurCategory);
    sal_uInt16  nDecimals           = 0;
    sal_uInt16  nZeroes             = 0;
    bool        bNegRed             = false;
    bool        bThousand           = false;
    sal_Int32   nCurrencyPos        =m_xLbCurrency->GetSelectedEntryPos();

    if(bOneAreaFlag)
        nCurCategory=nFixedCategory;


    pNumFmtShell->GetOptions( theFormat,
                              bThousand, bNegRed,
                              nDecimals, nZeroes,
                              nCategory );
    bool bDoIt=false;
    if(nCategory==CAT_CURRENCY)
    {
        sal_uInt16 nTstPos=pNumFmtShell->FindCurrencyFormat(theFormat);
        if(nCurrencyPos!=static_cast<sal_Int32>(nTstPos) && nTstPos!=sal_uInt16(-1))
        {
            m_xLbCurrency->set_active(nTstPos);
            pNumFmtShell->SetCurrencySymbol(nTstPos);
            bDoIt=true;
        }
    }


    if ( nCategory != nCurCategory || bDoIt)
    {
        if ( bCheckCatChange )
        {
            if(bOneAreaFlag)
                SetCategory(0);
            else
                SetCategory(nCategory );

            UpdateFormatListBox_Impl( true, false );
        }
    }
    else if ( m_xLbFormat->n_children() > 0 )
    {
        sal_uInt32 nCurEntryKey=NUMKEY_UNDEFINED;
        if(!pNumFmtShell->FindEntry( m_xEdFormat->get_text(),&nCurEntryKey))
        {
            m_xLbFormat->SetNoSelection();
        }
    }
    if(bOneAreaFlag)
    {
        nCategory=nFixedCategory;
    }

    UpdateThousandEngineeringCheckBox();
    UpdateDecimalsDenominatorEditBox();
    switch ( nCategory )
    {
        case CAT_SCIENTIFIC: // bThousand is for Engineering notation
            {
                sal_uInt16 nIntDigits = pNumFmtShell->GetFormatIntegerDigits(theFormat);
                bThousand = (nIntDigits > 0) && (nIntDigits % 3 == 0);
                m_xBtnEngineering->set_sensitive(true);
                m_xBtnEngineering->set_active( bThousand );
            }
            SAL_FALLTHROUGH;
        case CAT_NUMBER:
        case CAT_PERCENT:
        case CAT_CURRENCY:
        case CAT_FRACTION:
            m_xFtOptions->set_sensitive(true);
            if ( nCategory == CAT_FRACTION )
            {
                m_xFtDenominator->set_sensitive(true);
                m_xEdDenominator->set_sensitive(true);
            }
            else
            {
                m_xFtDecimals->set_sensitive(true);
                m_xEdDecimals->set_sensitive(true);
            }
            m_xFtLeadZeroes->set_sensitive(true);
            m_xEdLeadZeroes->set_sensitive(true);
            m_xBtnNegRed->set_sensitive(true);
            if ( nCategory == CAT_NUMBER && m_xLbFormat->GetSelectedEntryPos() == 0 )
                m_xEdDecimals->set_text( "" ); //General format tdf#44399
            else
                if ( nCategory == CAT_FRACTION )
                    m_xEdDenominator->set_text( OUString::number( nDecimals ) );
                else
                    m_xEdDecimals->set_text( OUString::number( nDecimals ) );
            m_xEdLeadZeroes->set_text( OUString::number( nZeroes ) );
            m_xBtnNegRed->set_active( bNegRed );
            if ( nCategory != CAT_SCIENTIFIC )
            {
                m_xBtnThousand->set_sensitive(true);
                m_xBtnThousand->set_active( bThousand );
            }
            break;

        case CAT_ALL:
        case CAT_USERDEFINED:
        case CAT_TEXT:
        case CAT_DATE:
        case CAT_TIME:
        case CAT_BOOLEAN:
        default:
            m_xFtOptions->set_sensitive(false);
            m_xFtDecimals->set_sensitive(false);
            m_xEdDecimals->set_sensitive(false);
            m_xFtDenominator->set_sensitive(false);
            m_xEdDenominator->set_sensitive(false);
            m_xFtLeadZeroes->set_sensitive(false);
            m_xEdLeadZeroes->set_sensitive(false);
            m_xBtnNegRed->set_sensitive(false);
            m_xBtnThousand->set_sensitive(false);
            m_xBtnEngineering->set_sensitive(false);
            m_xEdDecimals->set_text( OUString::number( 0 ) );
            m_xEdLeadZeroes->set_text( OUString::number( 0 ) );
            m_xBtnNegRed->set_active( false );
            m_xBtnThousand->set_active( false );
            m_xBtnEngineering->set_active( false );
    }
}


/*************************************************************************
#*  Method:        UpdateFormatListBox_Impl
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Updates the format listbox and additionally the
#*              string in the editbox is changed depending on
#*              the bUpdateEdit flag.
#*  Input:      Flags for category and editbox.
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::UpdateFormatListBox_Impl
    (
        bool bCat,        // Category or country/language ListBox?
        bool   bUpdateEdit
    )
{
    std::vector<OUString> aEntryList;
    short                 nFmtLbSelPos = 0;
    short                 nTmpCatPos;

    if(bOneAreaFlag)
    {
        nTmpCatPos=nFixedCategory;
    }
    else
    {
        nTmpCatPos=m_xLbCategory->GetSelectedEntryPos();
    }


    if ( bCat )
    {
        if(nTmpCatPos!=CAT_CURRENCY)
            m_xLbCurrency->hide();
        else
            m_xLbCurrency->show();

        pNumFmtShell->CategoryChanged( nTmpCatPos,nFmtLbSelPos, aEntryList );
    }
    else
        pNumFmtShell->LanguageChanged( m_xLbLanguage->GetSelectedLanguage(),
                                       nFmtLbSelPos,aEntryList );

    REMOVE_DONTKNOW() // possibly UI-Enable


    if ( (!aEntryList.empty()) && (nFmtLbSelPos != SELPOS_NONE) )
    {
        if(bUpdateEdit)
        {
            OUString aFormat=aEntryList[nFmtLbSelPos];
            m_xEdFormat->set_text(aFormat);
            m_xFtComment->set_label(pNumFmtShell->GetComment4Entry(nFmtLbSelPos));
        }

        if(!bOneAreaFlag || !bCat)
        {
            FillFormatListBox_Impl( aEntryList );
            m_xLbFormat->SelectEntryPos( nFmtLbSelPos );

            m_xFtComment->set_label(pNumFmtShell->GetComment4Entry(nFmtLbSelPos));
            if(pNumFmtShell->GetUserDefined4Entry(nFmtLbSelPos))
            {
                if(pNumFmtShell->GetComment4Entry(nFmtLbSelPos).isEmpty())
                {
                    m_xFtComment->set_label(m_xLbCategory->get_text(1));
                }
            }
            ChangePreviewText( static_cast<sal_uInt16>(nFmtLbSelPos) );
        }

    }
    else
    {
        FillFormatListBox_Impl( aEntryList );
        if(nFmtLbSelPos != SELPOS_NONE)
        {
            m_xLbFormat->SelectEntryPos( static_cast<sal_uInt16>(nFmtLbSelPos) );

            m_xFtComment->set_label(pNumFmtShell->GetComment4Entry(nFmtLbSelPos));
            if(pNumFmtShell->GetUserDefined4Entry(nFmtLbSelPos))
            {
                if(pNumFmtShell->GetComment4Entry(nFmtLbSelPos).isEmpty())
                {
                    m_xFtComment->set_label(m_xLbCategory->get_text(1));
                }
            }
        }
        else
        {
            m_xLbFormat->SetNoSelection();
        }

        if ( bUpdateEdit )
        {
            m_xEdFormat->set_text( OUString() );
            m_aWndPreview.NotifyChange( OUString() );
        }
    }

    aEntryList.clear();
}


/**
 * Change visible checkbox according to category format
 * if scientific format "Engineering notation"
 * else "Thousands separator"
 */

void SvxNumberFormatTabPage::UpdateThousandEngineeringCheckBox()
{
    bool bIsScientific = m_xLbCategory->GetSelectedEntryPos() == CAT_SCIENTIFIC;
    m_xBtnThousand->show( !bIsScientific );
    m_xBtnEngineering->show( bIsScientific );
}


/**
 * Change visible Edit box and Fixed text according to category format
 * if fraction format "Denominator places"
 * else "Decimal places"
 */

void SvxNumberFormatTabPage::UpdateDecimalsDenominatorEditBox()
{
    bool bIsFraction = m_xLbCategory->GetSelectedEntryPos() == CAT_FRACTION;
    m_xFtDecimals->show( !bIsFraction );
    m_xEdDecimals->show( !bIsFraction );
    m_xFtDenominator->show( bIsFraction );
    m_xEdDenominator->show( bIsFraction );
}


/*************************************************************************
#*  Handle:     DoubleClickHdl_Impl
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   On a double click in the format listbox the
#*              value is adopted and the OK button pushed.
#*  Input:      Pointer on the Listbox
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK(SvxNumberFormatTabPage, DoubleClickHdl_Impl, weld::TreeView&, rLb, void)
{
    SelFormatHdl_Impl(&rLb);

    if ( fnOkHdl.IsSet() )
    {   // temporary solution, should be offered by SfxTabPage
        fnOkHdl.Call( nullptr );
    }
    else
    {
        SfxSingleTabDialog* pParent = dynamic_cast<SfxSingleTabDialog*>(GetParentDialog());
        OKButton* pOKButton = pParent ? pParent->GetOKButton() : nullptr;
        if ( pOKButton )
            pOKButton->Click();
    }
}


/*************************************************************************
#*  Method:    SelFormatHdl_Impl
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Is called when the language, the category or the format
#*              is changed. Accordingly the settings are adjusted.
#*  Input:      Pointer on the Listbox
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK(SvxNumberFormatTabPage, SelFormatClickHdl_Impl, weld::ToggleButton&, rLb, void)
{
    SelFormatHdl_Impl(&rLb);
}

IMPL_LINK(SvxNumberFormatTabPage, SelFormatTreeListBoxHdl_Impl, weld::TreeView&, rLb, void)
{
    SelFormatHdl_Impl(&rLb);
}

IMPL_LINK(SvxNumberFormatTabPage, SelFormatListBoxHdl_Impl, weld::ComboBox&, rLb, void)
{
    SelFormatHdl_Impl(&rLb);
}

void SvxNumberFormatTabPage::SelFormatHdl_Impl(void * pLb )
{
    if (pLb == m_xCbSourceFormat.get())
    {
        EnableBySourceFormat_Impl();    // enable/disable everything else
        if ( m_xCbSourceFormat->get_active() )
            return;   // just disabled everything else

        // Reinit options enable/disable for current selection.

        // Current category may be UserDefined with no format entries defined.
        // And yes, m_xLbFormat is a SvxFontListBox with sal_uLong list positions,
        // implementation returns a TREELIST_ENTRY_NOTFOUND if empty,
        // comparison with sal_Int32 LISTBOX_ENTRY_NOTFOUND wouldn't match.
        if ( m_xLbFormat->GetSelectedEntryPos() == TREELIST_ENTRY_NOTFOUND )
            pLb = m_xLbCategory; // continue with the current category selected
        else
            pLb = m_xLbFormat;   // continue with the current format selected
    }

    sal_Int32 nTmpCatPos;

    if(bOneAreaFlag)
    {
        nTmpCatPos=nFixedCategory;
    }
    else
    {
        nTmpCatPos=m_xLbCategory->GetSelectedEntryPos();
    }

    if (nTmpCatPos==CAT_CURRENCY && pLb == m_xLbCurrency )
    {
        sal_Int32 nCurrencyPos = m_xLbCurrency->GetSelectedEntryPos();
        pNumFmtShell->SetCurrencySymbol(static_cast<sal_uInt32>(nCurrencyPos));
    }


    // Format-ListBox ----------------------------------------------------
    if (pLb == m_xLbFormat)
    {
        sal_uLong nSelPos = m_xLbFormat->GetSelectedEntryPos();
        short nFmtLbSelPos = static_cast<short>(nSelPos);

        OUString aFormat = pNumFmtShell->GetFormat4Entry(nFmtLbSelPos);
        OUString aComment = pNumFmtShell->GetComment4Entry(nFmtLbSelPos);

        if(pNumFmtShell->GetUserDefined4Entry(nFmtLbSelPos))
        {
            if(aComment.isEmpty())
            {
                aComment = m_xLbCategory->get_text(1);
            }
        }

        if ( !aFormat.isEmpty() )
        {
            if(!m_xEdFormat->HasFocus()) m_xEdFormat->set_text( aFormat );
            m_xFtComment->set_label(aComment);
            ChangePreviewText( static_cast<sal_uInt16>(nSelPos) );
        }

        REMOVE_DONTKNOW() // possibly UI-Enable

        if ( pNumFmtShell->FindEntry( aFormat) )
        {
            m_xIbAdd->set_sensitive(false );
            bool bIsUserDef=pNumFmtShell->IsUserDefined( aFormat );
            m_xIbRemove->set_sensitive(bIsUserDef);
            m_xIbInfo->set_sensitive(bIsUserDef);

        }
        else
        {
            m_xIbAdd->set_sensitive(true);
            m_xIbInfo->set_sensitive(true);
            m_xIbRemove->set_sensitive(false );
            m_xFtComment->set_label(m_xEdComment->get_text());

        }
        UpdateOptions_Impl( false );

        return;
    }


    // category-ListBox -------------------------------------------------
    if (pLb == m_xLbCategory || pLb == m_xLbCurrency)
    {
        UpdateFormatListBox_Impl( true, true );
        EditHdl_Impl( nullptr );
        UpdateOptions_Impl( false );

        return;
    }


    // language/country-ListBox ----------------------------------------------
    if (pLb == m_xLbLanguage)
    {
        UpdateFormatListBox_Impl( false, true );
        EditHdl_Impl(m_xEdFormat.get());

        return;
    }
}


/*************************************************************************
#*  Method:    ClickHdl_Impl, weld::Button& rIB
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Called when the add or delete button is pushed,
#*              adjusts the number format list.
#*  Input:      Toolbox- Button
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( SvxNumberFormatTabPage, ClickHdl_Impl, weld::Button&, rIB, void)
{
    Click_Impl(rIB);
}

bool SvxNumberFormatTabPage::Click_Impl(weld::Button& rIB)
{
    sal_uLong       nReturn = 0;
    const sal_uLong nReturnChanged  = 0x1;  // THE boolean return value
    const sal_uLong nReturnAdded    = 0x2;  // temp: format added
    const sal_uLong nReturnOneArea  = 0x4;  // temp: one area but category changed => ignored

    if (&rIB == m_xIbAdd.get())
    {   // Also called from FillItemSet() if a temporary currency format has
        // to be added, not only if the Add button is enabled.
        OUString               aFormat = m_xEdFormat->get_text();
        std::vector<OUString> aEntryList;
        std::vector<OUString> a2EntryList;
        sal_uInt16           nCatLbSelPos = 0;
        short                nFmtLbSelPos = SELPOS_NONE;
        sal_Int32            nErrPos=0;

        pNumFmtShell->SetCurCurrencyEntry(nullptr);
        bool bAdded = pNumFmtShell->AddFormat( aFormat, nErrPos,
                                          nCatLbSelPos, nFmtLbSelPos,
                                          aEntryList);
        if ( bAdded )
            nReturn |= nReturnChanged | nReturnAdded;

        if (pLastActivWindow == m_xEdComment)
        {
            m_xEdFormat->grab_focus();
            m_xEdComment->hide();
            m_xFtComment->show();
            m_xFtComment->set_label(m_xEdComment->get_text());
        }

        if ( !nErrPos ) // Syntax ok?
        {
            // May be sorted under a different locale if LCID was parsed.
            if (bAdded)
                m_xLbLanguage->SelectLanguage( pNumFmtShell->GetCurLanguage() );

            if(nCatLbSelPos==CAT_CURRENCY)
            {
                m_xLbCurrency->SelectEntryPos(static_cast<sal_uInt16>(pNumFmtShell->GetCurrencySymbol()));
            }

            if(bOneAreaFlag && (nFixedCategory!=nCatLbSelPos))
            {
                if(bAdded) aEntryList.clear();
                pNumFmtShell->RemoveFormat( aFormat,
                                            nCatLbSelPos,
                                            nFmtLbSelPos,
                                            a2EntryList);
                a2EntryList.clear();
                m_xEdFormat->grab_focus();
                m_xEdFormat->SetSelection( Selection( 0, SELECTION_MAX ) );
                nReturn |= nReturnOneArea;
            }
            else
            {
                if ( bAdded && (nFmtLbSelPos != SELPOS_NONE) )
                {
                    // everything alright
                    if(bOneAreaFlag)                  //@@ ???
                        SetCategory(0);
                    else
                        SetCategory(nCatLbSelPos );

                    FillFormatListBox_Impl( aEntryList );
                    if (m_xEdComment->get_text()!=m_xLbCategory->get_text(1))
                    {
                        pNumFmtShell->SetComment4Entry(nFmtLbSelPos,
                                                    m_xEdComment->get_text());
                    }
                    else
                    {
                        pNumFmtShell->SetComment4Entry(nFmtLbSelPos,
                                                        OUString());
                    }
                    m_xLbFormat->SelectEntryPos( static_cast<sal_uInt16>(nFmtLbSelPos) );
                    m_xEdFormat->set_text( aFormat );

                    m_xEdComment->set_text(m_xLbCategory->get_text(1));    // String for user defined

                    ChangePreviewText( static_cast<sal_uInt16>(nFmtLbSelPos) );
                }
            }
        }
        else // syntax error
        {
            m_xEdFormat->grab_focus();
            m_xEdFormat->SetSelection( Selection( nErrPos == -1 ? SELECTION_MAX : nErrPos, SELECTION_MAX ) );
        }
        EditHdl_Impl(m_xEdFormat.get());
        nReturn = ((nReturn & nReturnOneArea) ? 0 : (nReturn & nReturnChanged));

        aEntryList.clear();
        a2EntryList.clear();
    }
    else if (&rIB == m_xIbRemove.get())
    {
        OUString              aFormat = m_xEdFormat->get_text();
        std::vector<OUString> aEntryList;
        sal_uInt16           nCatLbSelPos = 0;
        short                nFmtLbSelPos = SELPOS_NONE;

        pNumFmtShell->RemoveFormat( aFormat,
                                    nCatLbSelPos,
                                    nFmtLbSelPos,
                                    aEntryList );

        m_xEdComment->set_text(m_xLbCategory->get_text(1));

        if( nFmtLbSelPos>=0 && static_cast<size_t>(nFmtLbSelPos)<aEntryList.size() )
        {
            aFormat = aEntryList[nFmtLbSelPos];
        }

        FillFormatListBox_Impl( aEntryList );

        if ( nFmtLbSelPos != SELPOS_NONE )
        {
            if(bOneAreaFlag)                  //@@ ???
                    SetCategory(0);
                else
                    SetCategory(nCatLbSelPos );

            m_xLbFormat->SelectEntryPos( static_cast<sal_uInt16>(nFmtLbSelPos) );
            m_xEdFormat->set_text( aFormat );
            ChangePreviewText( static_cast<sal_uInt16>(nFmtLbSelPos) );
        }
        else
        {
            // set to "all/standard"
            SetCategory(0);
            SelFormatHdl_Impl(m_xLbCategory);
        }

        EditHdl_Impl(m_xEdFormat.get());

        aEntryList.clear();
    }
    else if (&rIB == m_xIbInfo.get())
    {
        if(!(pLastActivWindow == m_xEdComment))
        {
            m_xEdComment->set_text(m_xFtComment->get_text());
            m_xEdComment->show();
            m_xFtComment->hide();
            m_xEdComment->grab_focus();
        }
        else
        {
            m_xEdFormat->grab_focus();
            m_xEdComment->hide();
            m_xFtComment->show();
        }
    }

    return nReturn;
}


/*************************************************************************
#*  Method:    EditHdl_Impl
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   When the entry in the edit field is changed
#*              the preview is updated and
#*  Input:      Pointer on Editbox
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK(SvxNumberFormatTabPage, EditModifyHdl_Impl, weld::Entry&, rEdit, void)
{
    EditHdl_Impl(&rEdit);
}

void SvxNumberFormatTabPage::EditHdl_Impl(const weld::Entry* pEdFormat)
{
    sal_uInt32 nCurKey = NUMKEY_UNDEFINED;

    if ( m_xEdFormat->get_text().isEmpty() )
    {
        m_xIbAdd->set_sensitive(false );
        m_xIbRemove->set_sensitive(false );
        m_xIbInfo->set_sensitive(false );
        m_xFtComment->set_label(OUString());
    }
    else
    {
        OUString aFormat = m_xEdFormat->get_text();
        MakePreviewText( aFormat );

        if ( pNumFmtShell->FindEntry( aFormat, &nCurKey ) )
        {
            m_xIbAdd->set_sensitive(false );
            bool bUserDef=pNumFmtShell->IsUserDefined( aFormat );

            m_xIbRemove->set_sensitive(bUserDef);
            m_xIbInfo->set_sensitive(bUserDef);

            if(bUserDef)
            {
                sal_uInt16 nTmpCurPos=pNumFmtShell->FindCurrencyFormat(aFormat );

                if(nTmpCurPos!=sal_uInt16(-1))
                    m_xLbCurrency->SelectEntryPos(nTmpCurPos);
            }
            short nPosi=pNumFmtShell->GetListPos4Entry(aFormat);
            if(nPosi>=0)
                m_xLbFormat->SelectEntryPos( static_cast<sal_uInt16>(nPosi));

        }
        else
        {

            m_xIbAdd->set_sensitive(true);
            m_xIbInfo->set_sensitive(true);
            m_xIbRemove->set_sensitive(false );

            m_xFtComment->set_label(m_xEdComment->get_text());

        }
    }

    if (pEdFormat)
    {
        pNumFmtShell->SetCurNumFmtKey( nCurKey );
        UpdateOptions_Impl( true );
    }
}


/*************************************************************************
#*  Method:        NotifyChange
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Does changes in the number attributes.
#*  Input:      Options- Controls
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK(SvxNumberFormatTabPage, OptClickHdl_Impl, weld::ToggleButton&, rOptCtrl, void)
{
    OptHdl_Impl(&rOptCtrl);
}

IMPL_LINK( SvxNumberFormatTabPage, OptEditHdl_Impl, Edit&, rEdit, void )
{
    OptHdl_Impl(&rEdit);
}

void SvxNumberFormatTabPage::OptHdl_Impl( void const * pOptCtrl )
{
    if (   (pOptCtrl == m_xEdLeadZeroes.get())
        || (pOptCtrl == m_xEdDecimals.get())
        || (pOptCtrl == m_xEdDenominator.get())
        || (pOptCtrl == m_xBtnNegRed.get())
        || (pOptCtrl == m_xBtnThousand.get())
        || (pOptCtrl == m_xBtnEngineering.get()) )
    {
        OUString          aFormat;
        bool          bThousand  = ( m_xBtnThousand->get_visible() && m_xBtnThousand->get_sensitive() && m_xBtnThousand->get_active() )
                                || ( m_xBtnEngineering->get_visible() && m_xBtnEngineering->get_sensitive() && m_xBtnEngineering->get_active() );
        bool          bNegRed    =   m_xBtnNegRed->get_sensitive() && m_xBtnNegRed->get_active();
        sal_uInt16    nPrecision = (m_xEdDecimals->get_sensitive() && m_xEdDecimals->get_visible())
                                 ? static_cast<sal_uInt16>(m_xEdDecimals->get_value())
                                 : ( (m_xEdDenominator->get_sensitive() && m_xEdDenominator->get_visible())
                                   ? static_cast<sal_uInt16>(m_xEdDenominator->get_value())
                                   : sal_uInt16(0) );
        sal_uInt16    nLeadZeroes = (m_xEdLeadZeroes->get_sensitive())
                                 ? static_cast<sal_uInt16>(m_xEdLeadZeroes->get_value())
                                 : sal_uInt16(0);
        if ( pNumFmtShell->GetStandardName() == m_xEdFormat->get_text() )
        {
            m_xEdDecimals->set_value(nPrecision);
        }

        pNumFmtShell->MakeFormat( aFormat,
                                  bThousand, bNegRed,
                                  nPrecision, nLeadZeroes,
                                  static_cast<sal_uInt16>(m_xLbFormat->get_selected_index()) );

        m_xEdFormat->set_text( aFormat );
        MakePreviewText( aFormat );

        if ( pNumFmtShell->FindEntry( aFormat ) )
        {
            m_xIbAdd->set_sensitive(false );
            bool bUserDef=pNumFmtShell->IsUserDefined( aFormat );
            m_xIbRemove->set_sensitive(bUserDef);
            m_xIbInfo->set_sensitive(bUserDef);
            EditHdl_Impl(m_xEdFormat.get());

        }
        else
        {
            EditHdl_Impl( nullptr );
            m_xLbFormat->select(-1);
        }
    }
}

IMPL_LINK_NOARG(SvxNumberFormatTabPage, TimeHdl_Impl, Timer *, void)
{
    pLastActivWindow=nullptr;
}


/*************************************************************************
#*  Method:    LostFocusHdl_Impl
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Does changes in the number attributes.
#*  Input:      Options- Controls
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK_NOARG( SvxNumberFormatTabPage, LostFocusHdl_Impl, weld::Widget&, void)
{
    aResetWinTimer.Start();
    m_xFtComment->set_label(m_xEdComment->get_text());
    m_xEdComment->hide();
    m_xFtComment->show();
    if(!m_xIbAdd->get_sensitive())
    {
        sal_uInt16 nSelPos = m_xLbFormat->get_selected_index();
        pNumFmtShell->SetComment4Entry(nSelPos, m_xEdComment->get_text());
        m_xEdComment->set_text(m_xLbCategory->get_text(1));    // String for user defined
    }
}

/*************************************************************************
#*  Method:        NotifyChange
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Does changes in the number attributes.
#*  Input:      Options- Controls
#*  Output:     ---
#*
#************************************************************************/

OUString SvxNumberFormatTabPage::GetExpColorString(
        Color*& rpPreviewColor, const OUString& rFormatStr, short nTmpCatPos)
{
    SvxNumValCategory i;
    switch (nTmpCatPos)
    {
        case CAT_ALL:           i=SvxNumValCategory::Standard; break;

        case CAT_NUMBER:        i=SvxNumValCategory::Standard; break;

        case CAT_PERCENT:       i=SvxNumValCategory::Percent; break;

        case CAT_CURRENCY:      i=SvxNumValCategory::Currency; break;

        case CAT_DATE:          i=SvxNumValCategory::Date; break;

        case CAT_TIME:          i=SvxNumValCategory::Time; break;

        case CAT_SCIENTIFIC:    i=SvxNumValCategory::Scientific; break;

        case CAT_FRACTION:      i=SvxNumValCategory::Fraction; break;

        case CAT_BOOLEAN:       i=SvxNumValCategory::Boolean; break;

        case CAT_USERDEFINED:   i=SvxNumValCategory::Standard; break;

        case CAT_TEXT:
        default:                i=SvxNumValCategory::NoValue;break;
    }
    double fVal = fSvxNumValConst[i];

    OUString aPreviewString;
    pNumFmtShell->MakePrevStringFromVal( rFormatStr, aPreviewString, rpPreviewColor, fVal );
    return aPreviewString;
}

void SvxNumberFormatTabPage::MakePreviewText( const OUString& rFormat )
{
    OUString aPreviewString;
    Color* pPreviewColor = nullptr;
    pNumFmtShell->MakePreviewString( rFormat, aPreviewString, pPreviewColor );
    m_aWndPreview.NotifyChange( aPreviewString, pPreviewColor );
}

void SvxNumberFormatTabPage::ChangePreviewText( sal_uInt16 nPos )
{
    OUString aPreviewString;
    Color* pPreviewColor = nullptr;
    pNumFmtShell->FormatChanged( nPos, aPreviewString, pPreviewColor );
    m_aWndPreview.NotifyChange( aPreviewString, pPreviewColor );
}

bool SvxNumberFormatTabPage::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType()==MouseNotifyEvent::LOSEFOCUS)
    {
        if ( rNEvt.GetWindow() == dynamic_cast<const  vcl::Window* >( m_xEdComment.get() ) && !m_xEdComment->get_visible() )
        {
            pLastActivWindow = nullptr;
        }
        else
        {
            pLastActivWindow = rNEvt.GetWindow();
        }
    }

    return SfxTabPage::PreNotify( rNEvt );
}

void SvxNumberFormatTabPage::FillCurrencyBox()
{
    std::vector<OUString> aList;

    sal_uInt16  nSelPos=0;
    pNumFmtShell->GetCurrencySymbols(aList, &nSelPos);

    for (std::vector<OUString>::iterator i = aList.begin() + 1;i != aList.end(); ++i)
        m_xLbCurrency->append_text(*i);

    // Initially disable the "Automatically" entry. First ensure that nothing
    // is selected, else if the to be disabled (first) entry was selected it
    // would be sticky when disabled and could not be deselected!
    m_xLbCurrency->set_active(-1);
//TODO    m_xLbCurrency->SetEntryFlags( 0, ListBoxEntryFlags::DisableSelection | ListBoxEntryFlags::DrawDisabled);
    m_xLbCurrency->set_active(nSelPos);
}

void SvxNumberFormatTabPage::SetCategory(sal_uInt16 nPos)
{
    sal_uInt16 nCurCategory = m_xLbCategory->get_selected_index();
    sal_uInt16 nTmpCatPos;

    if (bOneAreaFlag)
    {
        nTmpCatPos=nFixedCategory;
    }
    else
    {
        nTmpCatPos=nPos;
    }

    if(m_xLbCategory->n_children()==1 || nCurCategory!=nPos)
    {
        if(nTmpCatPos!=CAT_CURRENCY)
            m_xLbCurrency->hide();
        else
            m_xLbCurrency->show();
    }
    m_xLbCategory->select(nPos);
}

/* to support Writer text field language handling an
   additional entry needs to be inserted into the ListBox
   which marks a certain language as automatically detected
   Additionally the "Default" language is removed
*/
void SvxNumberFormatTabPage::AddAutomaticLanguage_Impl(LanguageType eAutoLang, bool bSelect)
{
    m_xLbLanguage->RemoveLanguage(LANGUAGE_SYSTEM);
    const sal_Int32 nPos = m_xLbLanguage->InsertEntry(sAutomaticEntry);
    m_xLbLanguage->SetEntryData(nPos, reinterpret_cast<void*>(static_cast<sal_uInt16>(eAutoLang)));
    if(bSelect)
        m_xLbLanguage->SelectEntryPos(nPos);
}

void SvxNumberFormatTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SvxNumberInfoItem* pNumberInfoItem = aSet.GetItem<SvxNumberInfoItem>(SID_ATTR_NUMBERFORMAT_INFO, false);
    const SfxLinkItem* pLinkItem = aSet.GetItem<SfxLinkItem>(SID_LINK_TYPE, false);
    if (pNumberInfoItem && !pNumItem)
        pNumItem.reset( static_cast<SvxNumberInfoItem*>(pNumberInfoItem->Clone()) );
    if (pLinkItem)
        fnOkHdl = pLinkItem->GetValue();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
