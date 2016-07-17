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

#include <cuires.hrc>

#include <svx/numinf.hxx>

#include "numfmt.hxx"
#include <svx/numfmtsh.hxx>
#include <dialmgr.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>
#include <sfx2/basedlgs.hxx>
#include "svx/flagsdef.hxx"
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

SvxNumberPreview::SvxNumberPreview(vcl::Window* pParent, WinBits nStyle)
    : Window(pParent, nStyle)
    , mnPos(-1)
    , mnChar(0x0)
{
    vcl::Font aFont( GetFont() );
    aFont.SetTransparent( true );
    aFont.SetColor( Application::GetSettings().GetStyleSettings().GetFieldColor() );
    SetFont( aFont );
    InitSettings( true, true );
    SetBorderStyle( WindowBorderStyle::MONO );
}

VCL_BUILDER_FACTORY(SvxNumberPreview)

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
    Update();
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

void SvxNumberPreview::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    vcl::Font aDrawFont = rRenderContext.GetFont();
    Size aSzWnd(GetOutputSizePixel());
    OUString aTmpStr( aPrevStr );
    long nLeadSpace = (aSzWnd.Width() - rRenderContext.GetTextWidth(aTmpStr)) / 2;

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
}

void SvxNumberPreview::InitSettings( bool bForeground, bool bBackground )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if ( bForeground )
    {
        svtools::ColorConfig aColorConfig;
        Color aTextColor( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );

        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( rStyleSettings.GetWindowColor() );
    }
    Invalidate();
}


void SvxNumberPreview::StateChanged( StateChangedType nType )
{
    if ( nType == StateChangedType::ControlForeground )
        InitSettings( true, false );
    else if ( nType == StateChangedType::ControlBackground )
        InitSettings( false, true );

    Window::StateChanged( nType );
}


void SvxNumberPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
        InitSettings( true, true );
}

// class SvxNumberFormatTabPage ------------------------------------------

#define REMOVE_DONTKNOW() \
    if ( !m_pFtLanguage->IsEnabled() )                                     \
    {                                                                   \
        m_pFtLanguage->Enable();                                          \
        m_pLbLanguage->Enable();                                          \
        m_pLbLanguage->SelectLanguage( pNumFmtShell->GetCurLanguage() );  \
    }

#define HDL(hdl) LINK( this, SvxNumberFormatTabPage, hdl )

SvxNumberFormatTabPage::SvxNumberFormatTabPage(vcl::Window* pParent,
    const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pParent, "NumberingFormatPage",
        "cui/ui/numberingformatpage.ui", &rCoreAttrs)
    , pNumItem(nullptr)
    , pNumFmtShell(nullptr)
    , nInitFormat(ULONG_MAX)
    , sAutomaticEntry(CUI_RES(RID_SVXSTR_AUTO_ENTRY))
    , pLastActivWindow(nullptr)
{
    get(m_pFtCategory, "categoryft");
    get(m_pLbCategory, "categorylb");
    get(m_pFtFormat, "formatft");
    get(m_pLbCurrency, "currencylb");
    get(m_pLbFormat, "formatlb");
    long nWidth = approximate_char_width() * 26;
    m_pLbFormat->set_width_request(nWidth);
    m_pLbCurrency->set_width_request(nWidth);
    get(m_pFtLanguage, "languageft");
    get(m_pLbLanguage, "languagelb");
    get(m_pCbSourceFormat, "sourceformat");
    get(m_pWndPreview, "preview");
    get(m_pFtOptions, "optionsft");
    get(m_pFtDecimals, "decimalsft");
    get(m_pEdDecimals, "decimalsed");
    get(m_pFtDenominator, "denominatorft");
    get(m_pEdDenominator, "denominatored");
    get(m_pBtnNegRed, "negnumred");
    get(m_pFtLeadZeroes, "leadzerosft");
    get(m_pEdLeadZeroes, "leadzerosed");
    get(m_pBtnThousand, "thousands");
    get(m_pBtnEngineering, "engineering");
    get(m_pFormatCodeFrame, "formatcode");
    get(m_pEdFormat, "formatted");
    get(m_pIbAdd, "add");
    get(m_pIbInfo, "edit");
    get(m_pIbRemove, "delete");
    get(m_pFtComment, "commentft");
    get(m_pEdComment, "commented");

    m_pLbCategory->SetDropDownLineCount(8);
    m_pWndPreview->set_height_request(GetTextHeight()*3);

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
    delete pNumFmtShell;
    pNumFmtShell = nullptr;
    delete pNumItem;
    pNumItem = nullptr;
    m_pFtCategory.clear();
    m_pLbCategory.clear();
    m_pFtFormat.clear();
    m_pLbCurrency.clear();
    m_pLbFormat.clear();
    m_pFtLanguage.clear();
    m_pLbLanguage.clear();
    m_pCbSourceFormat.clear();
    m_pWndPreview.clear();
    m_pFtOptions.clear();
    m_pFtDecimals.clear();
    m_pEdDecimals.clear();
    m_pFtDenominator.clear();
    m_pEdDenominator.clear();
    m_pBtnNegRed.clear();
    m_pFtLeadZeroes.clear();
    m_pEdLeadZeroes.clear();
    m_pBtnThousand.clear();
    m_pBtnEngineering.clear();
    m_pFormatCodeFrame.clear();
    m_pEdFormat.clear();
    m_pIbAdd.clear();
    m_pIbInfo.clear();
    m_pIbRemove.clear();
    m_pFtComment.clear();
    m_pEdComment.clear();
    pLastActivWindow.clear();
    SfxTabPage::dispose();
}

void SvxNumberFormatTabPage::Init_Impl()
{
    bNumItemFlag=true;
    bOneAreaFlag=false;

    m_pIbAdd->Enable(false );
    m_pIbRemove->Enable(false );
    m_pIbInfo->Enable(false );

    m_pEdComment->SetText(m_pLbCategory->GetEntry(1));    // string for user defined

    m_pEdComment->Hide();

    m_pCbSourceFormat->Check( false );
    m_pCbSourceFormat->Disable();
    m_pCbSourceFormat->Hide();

    Link<ListBox&,void> aLink2 = LINK( this, SvxNumberFormatTabPage, SelFormatListBoxHdl_Impl );

    m_pLbCategory->SetSelectHdl( aLink2 );
    m_pLbFormat->SetSelectHdl( LINK( this, SvxNumberFormatTabPage, SelFormatTreeListBoxHdl_Impl ) );
    m_pLbLanguage->SetSelectHdl( aLink2 );
    m_pLbCurrency->SetSelectHdl( aLink2 );
    m_pCbSourceFormat->SetClickHdl( LINK( this, SvxNumberFormatTabPage, SelFormatClickHdl_Impl ) );

    Link<Edit&,void> aLink = LINK( this, SvxNumberFormatTabPage, OptEditHdl_Impl );

    m_pEdDecimals->SetModifyHdl( aLink );
    m_pEdDenominator->SetModifyHdl( aLink );
    m_pEdLeadZeroes->SetModifyHdl( aLink );

    m_pBtnNegRed->SetClickHdl( LINK( this, SvxNumberFormatTabPage, OptClickHdl_Impl ) );
    m_pBtnThousand->SetClickHdl( LINK( this, SvxNumberFormatTabPage, OptClickHdl_Impl ) );
    m_pBtnEngineering->SetClickHdl( LINK( this, SvxNumberFormatTabPage, OptClickHdl_Impl ) );
    m_pLbFormat->SetDoubleClickHdl( HDL( DoubleClickHdl_Impl ) );
    m_pEdFormat->SetModifyHdl( HDL( EditModifyHdl_Impl ) );
    m_pIbAdd->SetClickHdl( HDL( ClickHdl_Impl ) );
    m_pIbRemove->SetClickHdl( HDL( ClickHdl_Impl ) );
    m_pIbInfo->SetClickHdl( HDL( ClickHdl_Impl ) );
    UpdateThousandEngineeringCheckBox();
    UpdateDecimalsDenominatorEditBox();

    m_pEdComment->SetLoseFocusHdl( LINK( this, SvxNumberFormatTabPage, LostFocusHdl_Impl) );
    aResetWinTimer.SetTimeoutHdl(LINK( this, SvxNumberFormatTabPage, TimeHdl_Impl));
    aResetWinTimer.SetTimeout( 10);

    // initialize language ListBox

    m_pLbLanguage->InsertLanguage( LANGUAGE_SYSTEM );
    /* TODO: any reason we're doing a manual init here instead of using
     * SvxLanguageBoxBase::SetLanguageList( SvxLanguageListFlags::ONLY_KNOWN, ...)? */
    // Don't list ambiguous locales where we won't be able to convert the
    // LanguageType back to an identical Language_Country name and therefore
    // couldn't load the i18n LocaleData. Show DebugMsg in non-PRODUCT version.
    css::uno::Sequence< sal_uInt16 > xLang = LocaleDataWrapper::getInstalledLanguageTypes();
    sal_Int32 nCount = xLang.getLength();
    for ( sal_Int32 i=0; i<nCount; i++ )
    {
        if (!MsLangId::isLegacy( xLang[i]))
            m_pLbLanguage->InsertLanguage( xLang[i] );
    }
}

VclPtr<SfxTabPage> SvxNumberFormatTabPage::Create( vcl::Window* pParent,
                                                   const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxNumberFormatTabPage>::Create( pParent, *rAttrSet );
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
    SvxNumberValueType          eValType        = SVX_VALUE_TYPE_UNDEFINED;
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
            pNumItem= static_cast<SvxNumberInfoItem *>(pItem->Clone());
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
            m_pCbSourceFormat->Check( pBoolItem->GetValue() );
        else
            m_pCbSourceFormat->Check( false );
        m_pCbSourceFormat->Enable();
        m_pCbSourceFormat->Show();
    }
    else
    {
        bool bInit = false;     // set to sal_True for debug test
        m_pCbSourceFormat->Check( bInit );
        m_pCbSourceFormat->Enable( bInit );
        m_pCbSourceFormat->Show( bInit );
    }

    // pNumItem must have been set from outside!
    DBG_ASSERT( pNumItem, "No NumberInfo, no NumberFormatter, good bye.CRASH. :-(" );

    eState = rSet->GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_VALUE ) );

    if ( SfxItemState::DONTCARE != eState )
        pValFmtAttr = static_cast<const SfxUInt32Item*>(
                      GetItem( *rSet, SID_ATTR_NUMBERFORMAT_VALUE ));

    eValType = pNumItem->GetValueType();

    switch ( eValType )
    {
        case SVX_VALUE_TYPE_STRING:
            aValString = pNumItem->GetValueString();
            break;
        case SVX_VALUE_TYPE_NUMBER:
            //  #50441# string may be set in addition to the value
            aValString = pNumItem->GetValueString();
            nValDouble = pNumItem->GetValueDouble();
            break;
        case SVX_VALUE_TYPE_UNDEFINED:
        default:
            break;
    }

    delete pNumFmtShell;   // delete old shell if applicable (== reset)

    nInitFormat = ( pValFmtAttr )               // memorize init key
                    ? pValFmtAttr->GetValue()   // (for FillItemSet())
                    : ULONG_MAX;                // == DONT_KNOW


    if ( eValType == SVX_VALUE_TYPE_STRING )
        pNumFmtShell =SvxNumberFormatShell::Create(
                                pNumItem->GetNumberFormatter(),
                                (pValFmtAttr) ? nInitFormat : 0L,
                                eValType,
                                aValString );
    else
        pNumFmtShell =SvxNumberFormatShell::Create(
                                pNumItem->GetNumberFormatter(),
                                (pValFmtAttr) ? nInitFormat : 0L,
                                eValType,
                                nValDouble,
                                &aValString );


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
            // Enable "Automatically" if currently used so it is selectable.
            m_pLbCurrency->SetEntryFlags( nPos, ListBoxEntryFlags::NONE );

        m_pLbCurrency->SelectEntryPos(nPos);
    }

    nFixedCategory=nCatLbSelPos;
    if(bOneAreaFlag)
    {
        OUString sFixedCategory = m_pLbCategory->GetEntry(nFixedCategory);
        m_pLbCategory->Clear();
        m_pLbCategory->InsertEntry(sFixedCategory);
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
        m_pLbLanguage->RemoveLanguage( eLangType );    // in case we're already called
        m_pLbLanguage->InsertLanguage( eLangType );
    }
    m_pLbLanguage->SelectLanguage( eLangType );
    if(pAutoEntryAttr)
        AddAutomaticLanguage_Impl(eLangType, pAutoEntryAttr->GetValue());
    UpdateFormatListBox_Impl(false,true);

//! This spoils everything because it rematches currency formats based on
//! the selected m_pLbCurrency entry instead of the current format.
//! Besides that everything seems to be initialized by now, so why call it?
//  SelFormatHdl_Impl( m_pLbCategory );

    if ( pValFmtAttr )
    {
        EditHdl_Impl(m_pEdFormat); // UpdateOptions_Impl() als Seiteneffekt
    }
    else    // DONT_KNOW
    {
        // everything disabled except direct input or changing the category
        Obstructing();
    }

    if ( m_pCbSourceFormat->IsChecked() )
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
    m_pLbFormat->SetNoSelection();
    m_pLbLanguage->SetNoSelection();
    m_pFtLanguage->Disable();
    m_pLbLanguage->Disable();

    m_pIbAdd->Enable(false );
    m_pIbRemove->Enable(false );
    m_pIbInfo->Enable(false );

    m_pBtnNegRed->Disable();
    m_pBtnThousand->Disable();
    m_pBtnEngineering->Disable();
    m_pFtLeadZeroes->Disable();
    m_pFtDecimals->Disable();
    m_pFtDenominator->Disable();
    m_pEdLeadZeroes->Disable();
    m_pEdDecimals->Disable();
    m_pEdDenominator->Disable();
    m_pFtOptions->Disable();
    m_pEdDecimals->SetText( OUString() );
    m_pEdLeadZeroes->SetText( OUString() );
    m_pBtnNegRed->Check( false );
    m_pBtnThousand->Check( false );
    m_pBtnEngineering->Check( false );
    m_pWndPreview->NotifyChange( OUString() );

    m_pLbCategory->SelectEntryPos( 0 );
    m_pEdFormat->SetText( OUString() );
    m_pFtComment->SetText( OUString() );
    m_pEdComment->SetText(m_pLbCategory->GetEntry(1));  // string for user defined

    m_pEdFormat->GrabFocus();
}


/*************************************************************************
#* Enable/Disable dialog parts depending on the value of the SourceFormat
#* checkbox.
#************************************************************************/
void SvxNumberFormatTabPage::EnableBySourceFormat_Impl()
{
    bool bEnable = !m_pCbSourceFormat->IsChecked();
    if ( !bEnable )
        m_pCbSourceFormat->GrabFocus();
    m_pFtCategory->Enable( bEnable );
    m_pLbCategory->Enable( bEnable );
    m_pFtFormat->Enable( bEnable );
    m_pLbCurrency->Enable( bEnable );
    m_pLbFormat->Enable( bEnable );
    m_pFtLanguage->Enable( bEnable );
    m_pLbLanguage->Enable( bEnable );
    m_pFtDecimals->Enable( bEnable );
    m_pEdDecimals->Enable( bEnable );
    m_pFtDenominator->Enable( bEnable );
    m_pEdDenominator->Enable( bEnable );
    m_pFtLeadZeroes->Enable( bEnable );
    m_pEdLeadZeroes->Enable( bEnable );
    m_pBtnNegRed->Enable( bEnable );
    m_pBtnThousand->Enable( bEnable );
    m_pBtnEngineering->Enable( bEnable );
    m_pFtOptions->Enable( bEnable );
    m_pFormatCodeFrame->Enable( bEnable );
    m_pLbFormat->Invalidate(); // #i43322#
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
    m_pFtLanguage->Show(!bFlag);
    m_pLbLanguage->Show(!bFlag);
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
    bool bDataChanged   = m_pFtLanguage->IsEnabled() || m_pCbSourceFormat->IsEnabled();
    if ( bDataChanged )
    {
        const SfxItemSet& rMyItemSet = GetItemSet();
        sal_uInt16          nWhich       = GetWhich( SID_ATTR_NUMBERFORMAT_VALUE );
        SfxItemState    eItemState   = rMyItemSet.GetItemState( nWhich, false );

        // OK chosen - Is format code input entered already taken over?
        // If not, simulate Add. Upon syntax error ignore input and prevent Put.
        OUString    aFormat = m_pEdFormat->GetText();
        sal_uInt32 nCurKey = pNumFmtShell->GetCurNumFmtKey();

        if ( m_pIbAdd->IsEnabled() || pNumFmtShell->IsTmpCurrencyFormat(aFormat) )
        {   // #79599# It is not sufficient to just add the format code (or
            // delete it in case of bOneAreaFlag and resulting category change).
            // Upon switching tab pages we need all settings to be consistent
            // in case this page will be redisplayed later.
            bDataChanged = Click_Impl(m_pIbAdd);
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

        const size_t nDelCount = pNumFmtShell->GetUpdateDataCount();

        if ( nDelCount > 0 )
        {
            std::unique_ptr<sal_uInt32[]> pDelArr(new sal_uInt32[nDelCount]);

            pNumFmtShell->GetUpdateData( pDelArr.get(), nDelCount );
            pNumItem->SetDelFormatArray( pDelArr.get(), nDelCount );

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

        if ( m_pCbSourceFormat->IsEnabled() )
        {
            sal_uInt16 _nWhich = GetWhich( SID_ATTR_NUMBERFORMAT_SOURCE );
            SfxItemState _eItemState = rMyItemSet.GetItemState( _nWhich, false );
            const SfxBoolItem* pBoolItem = static_cast<const SfxBoolItem*>(
                        GetItem( rMyItemSet, SID_ATTR_NUMBERFORMAT_SOURCE ));
            bool bOld = pBoolItem && pBoolItem->GetValue();
            rCoreAttrs->Put( SfxBoolItem( _nWhich, m_pCbSourceFormat->IsChecked() ) );
            if ( !bDataChanged )
                bDataChanged = (bOld != m_pCbSourceFormat->IsChecked() ||
                    _eItemState != SfxItemState::SET);
        }

        // FillItemSet is only called on OK, here we can notify the
        // NumberFormatShell that all new user defined formats are valid.
        pNumFmtShell->ValidateNewEntries();
        if(m_pLbLanguage->IsVisible() &&
                LISTBOX_ENTRY_NOTFOUND != m_pLbLanguage->GetEntryPos(sAutomaticEntry))
                rCoreAttrs->Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ADD_AUTO,
                    m_pLbLanguage->GetSelectEntry() == sAutomaticEntry));
    }

    return bDataChanged;
}


DeactivateRC SvxNumberFormatTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

void SvxNumberFormatTabPage::SetInfoItem( const SvxNumberInfoItem& rItem )
{
    if(pNumItem==nullptr)
    {
        pNumItem = static_cast<SvxNumberInfoItem*>(rItem.Clone());
    }
}

void SvxNumberFormatTabPage::FillFormatListBox_Impl( std::vector<OUString>& rEntries )
{
    OUString    aEntry;
    OUString    aTmpString;
    vcl::Font   aFont=m_pLbCategory->GetFont();
    size_t      i = 0;
    short       nTmpCatPos;

    m_pLbFormat->Clear();
    m_pLbFormat->SetUpdateMode( false );

    if( rEntries.empty() )
        return;

    if(bOneAreaFlag)
    {
        nTmpCatPos=nFixedCategory;
    }
    else
    {
        nTmpCatPos=m_pLbCategory->GetSelectEntryPos();
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
                                m_pLbFormat->InsertFontEntry( aTmpString, aFont );
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
                vcl::Font aEntryFont( m_pLbFormat->GetFont() );
                m_pLbFormat->InsertFontEntry( aPreviewString, aEntryFont, pPreviewColor );
            }
            else
            {
                m_pLbFormat->InsertFontEntry(aEntry,aFont);
            }
        }
    }
    m_pLbFormat->SetUpdateMode( true );
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
    OUString    theFormat           = m_pEdFormat->GetText();
    sal_Int32   nCurCategory        = m_pLbCategory->GetSelectEntryPos();
    sal_uInt16  nCategory           = static_cast<sal_uInt16>(nCurCategory);
    sal_uInt16  nDecimals           = 0;
    sal_uInt16  nZeroes             = 0;
    bool        bNegRed             = false;
    bool        bThousand           = false;
    sal_Int32   nCurrencyPos        =m_pLbCurrency->GetSelectEntryPos();

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
        if(nCurrencyPos!=static_cast<sal_Int32>(nTstPos) && nTstPos!=(sal_uInt16)-1)
        {
            m_pLbCurrency->SelectEntryPos(nTstPos);
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
    else if ( m_pLbFormat->GetEntryCount() > 0 )
    {
        sal_uInt32 nCurEntryKey=NUMKEY_UNDEFINED;
        if(!pNumFmtShell->FindEntry( m_pEdFormat->GetText(),&nCurEntryKey))
        {
            m_pLbFormat->SetNoSelection();
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
                if ( (nIntDigits > 0) && (nIntDigits % 3 == 0) )
                    bThousand = true;
                else
                    bThousand = false;
                m_pBtnEngineering->Enable();
                m_pBtnEngineering->Check( bThousand );
            }
            SAL_FALLTHROUGH;
        case CAT_NUMBER:
        case CAT_PERCENT:
        case CAT_CURRENCY:
        case CAT_FRACTION:
            m_pFtOptions->Enable();
            if ( nCategory == CAT_FRACTION )
            {
                m_pFtDenominator->Enable();
                m_pEdDenominator->Enable();
            }
            else
            {
                m_pFtDecimals->Enable();
                m_pEdDecimals->Enable();
            }
            m_pFtLeadZeroes->Enable();
            m_pEdLeadZeroes->Enable();
            m_pBtnNegRed->Enable();
            if ( nCategory == CAT_NUMBER && m_pLbFormat->GetSelectEntryPos() == 0 )
                m_pEdDecimals->SetText( "" ); //General format tdf#44399
            else
                if ( nCategory == CAT_FRACTION )
                    m_pEdDenominator->SetText( OUString::number( nDecimals ) );
                else
                    m_pEdDecimals->SetText( OUString::number( nDecimals ) );
            m_pEdLeadZeroes->SetText( OUString::number( nZeroes ) );
            m_pBtnNegRed->Check( bNegRed );
            if ( nCategory != CAT_SCIENTIFIC && nCategory != CAT_FRACTION )
            {
                m_pBtnThousand->Enable();
                m_pBtnThousand->Check( bThousand );
            }
            break;

        case CAT_ALL:
        case CAT_USERDEFINED:
        case CAT_TEXT:
        case CAT_DATE:
        case CAT_TIME:
        case CAT_BOOLEAN:
        default:
            m_pFtOptions->Disable();
            m_pFtDecimals->Disable();
            m_pEdDecimals->Disable();
            m_pFtDenominator->Disable();
            m_pEdDenominator->Disable();
            m_pFtLeadZeroes->Disable();
            m_pEdLeadZeroes->Disable();
            m_pBtnNegRed->Disable();
            m_pBtnThousand->Disable();
            m_pBtnEngineering->Disable();
            m_pEdDecimals->SetText( OUString::number( 0 ) );
            m_pEdLeadZeroes->SetText( OUString::number( 0 ) );
            m_pBtnNegRed->Check( false );
            m_pBtnThousand->Check( false );
            m_pBtnEngineering->Check( false );
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
        nTmpCatPos=m_pLbCategory->GetSelectEntryPos();
    }


    if ( bCat )
    {
        if(nTmpCatPos!=CAT_CURRENCY)
            m_pLbCurrency->Hide();
        else
            m_pLbCurrency->Show();

        pNumFmtShell->CategoryChanged( nTmpCatPos,nFmtLbSelPos, aEntryList );
    }
    else
        pNumFmtShell->LanguageChanged( m_pLbLanguage->GetSelectLanguage(),
                                       nFmtLbSelPos,aEntryList );

    REMOVE_DONTKNOW() // possibly UI-Enable


    if ( (!aEntryList.empty()) && (nFmtLbSelPos != SELPOS_NONE) )
    {
        if(bUpdateEdit)
        {
            OUString aFormat=aEntryList[nFmtLbSelPos];
            m_pEdFormat->SetText(aFormat);
            m_pFtComment->SetText(pNumFmtShell->GetComment4Entry(nFmtLbSelPos));
        }

        if(!bOneAreaFlag || !bCat)
        {
            FillFormatListBox_Impl( aEntryList );
            m_pLbFormat->SelectEntryPos( nFmtLbSelPos );

            m_pFtComment->SetText(pNumFmtShell->GetComment4Entry(nFmtLbSelPos));
            if(pNumFmtShell->GetUserDefined4Entry(nFmtLbSelPos))
            {
                if(pNumFmtShell->GetComment4Entry(nFmtLbSelPos).isEmpty())
                {
                    m_pFtComment->SetText(m_pLbCategory->GetEntry(1));
                }
            }
            ChangePreviewText( (sal_uInt16)nFmtLbSelPos );
        }

    }
    else
    {
        FillFormatListBox_Impl( aEntryList );
        if(nFmtLbSelPos != SELPOS_NONE)
        {
            m_pLbFormat->SelectEntryPos( (sal_uInt16)nFmtLbSelPos );

            m_pFtComment->SetText(pNumFmtShell->GetComment4Entry(nFmtLbSelPos));
            if(pNumFmtShell->GetUserDefined4Entry(nFmtLbSelPos))
            {
                if(pNumFmtShell->GetComment4Entry(nFmtLbSelPos).isEmpty())
                {
                    m_pFtComment->SetText(m_pLbCategory->GetEntry(1));
                }
            }
        }
        else
        {
            m_pLbFormat->SetNoSelection();
        }

        if ( bUpdateEdit )
        {
            m_pEdFormat->SetText( OUString() );
            m_pWndPreview->NotifyChange( OUString() );
        }
    }

    aEntryList.clear();
}


/*************************************************************************
#*  Method:        UpdateThousandEngineeringCheckBox
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Change visible checkbox according to category format
#*              if scientific format "Engineering notation"
#*              else "Thousands separator"
#*  Input:      ---
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::UpdateThousandEngineeringCheckBox()
{
    bool bIsScientific = m_pLbCategory->GetSelectEntryPos() == CAT_SCIENTIFIC;
    m_pBtnThousand->Show( !bIsScientific );
    m_pBtnEngineering->Show( bIsScientific );
}


/*************************************************************************
#*  Method:        UpdateDecimalsDenominatorEditBox
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Change visible Edit box and Fixed text according to category format
#*              if fraction format "Denominator places"
#*              else "Decimal places"
#*  Input:      ---
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::UpdateDecimalsDenominatorEditBox()
{
    bool bIsFraction = m_pLbCategory->GetSelectEntryPos() == CAT_FRACTION;
    m_pFtDecimals->Show( !bIsFraction );
    m_pEdDecimals->Show( !bIsFraction );
    m_pFtDenominator->Show( bIsFraction );
    m_pEdDenominator->Show( bIsFraction );
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

IMPL_LINK_TYPED( SvxNumberFormatTabPage, DoubleClickHdl_Impl, SvTreeListBox*, pLb, bool )
{
    if (pLb == m_pLbFormat)
    {
        SelFormatHdl_Impl( pLb );

        if ( fnOkHdl.IsSet() )
        {   // temporary solution, should be offered by SfxTabPage
            fnOkHdl.Call( nullptr );
        }
        else
        {
            SfxSingleTabDialog* pParent = dynamic_cast< SfxSingleTabDialog* >( GetParentDialog() );
            OKButton* pOKButton = pParent ? pParent->GetOKButton() : nullptr;
            if ( pOKButton )
                pOKButton->Click();
        }
    }
    return false;
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

IMPL_LINK_TYPED( SvxNumberFormatTabPage, SelFormatClickHdl_Impl, Button*, pLb, void )
{
    SelFormatHdl_Impl(pLb);
}
IMPL_LINK_TYPED( SvxNumberFormatTabPage, SelFormatTreeListBoxHdl_Impl, SvTreeListBox*, pLb, void )
{
    SelFormatHdl_Impl(pLb);
}
IMPL_LINK_TYPED( SvxNumberFormatTabPage, SelFormatListBoxHdl_Impl, ListBox&, rLb, void )
{
    SelFormatHdl_Impl(&rLb);
}
void SvxNumberFormatTabPage::SelFormatHdl_Impl(void * pLb )
{
    if (pLb == m_pCbSourceFormat)
    {
        EnableBySourceFormat_Impl();    // enable/disable everything else
        if ( m_pCbSourceFormat->IsChecked() )
            return;   // just disabled everything else

        // Reinit options enable/disable for current selection.

        // Current category may be UserDefined with no format entries defined.
        // And yes, m_pLbFormat is a SvxFontListBox with sal_uLong list positions,
        // implementation returns a TREELIST_ENTRY_NOTFOUND if empty,
        // comparison with sal_Int32 LISTBOX_ENTRY_NOTFOUND wouldn't match.
        if ( m_pLbFormat->GetSelectEntryPos() == TREELIST_ENTRY_NOTFOUND )
            pLb = m_pLbCategory; // continue with the current category selected
        else
            pLb = m_pLbFormat;   // continue with the current format selected
    }

    sal_Int32 nTmpCatPos;

    if(bOneAreaFlag)
    {
        nTmpCatPos=nFixedCategory;
    }
    else
    {
        nTmpCatPos=m_pLbCategory->GetSelectEntryPos();
    }

    if (nTmpCatPos==CAT_CURRENCY && pLb == m_pLbCurrency )
    {
        sal_Int32 nCurrencyPos = m_pLbCurrency->GetSelectEntryPos();
        pNumFmtShell->SetCurrencySymbol(static_cast<sal_uInt32>(nCurrencyPos));
    }


    // Format-ListBox ----------------------------------------------------
    if (pLb == m_pLbFormat)
    {
        sal_uLong nSelPos = m_pLbFormat->GetSelectEntryPos();
        short nFmtLbSelPos = static_cast<short>(nSelPos);

        OUString aFormat = pNumFmtShell->GetFormat4Entry(nFmtLbSelPos);
        OUString aComment = pNumFmtShell->GetComment4Entry(nFmtLbSelPos);

        if(pNumFmtShell->GetUserDefined4Entry(nFmtLbSelPos))
        {
            if(aComment.isEmpty())
            {
                aComment = m_pLbCategory->GetEntry(1);
            }
        }

        if ( !aFormat.isEmpty() )
        {
            if(!m_pEdFormat->HasFocus()) m_pEdFormat->SetText( aFormat );
            m_pFtComment->SetText(aComment);
            ChangePreviewText( static_cast<sal_uInt16>(nSelPos) );
        }

        REMOVE_DONTKNOW() // possibly UI-Enable

        if ( pNumFmtShell->FindEntry( aFormat) )
        {
            m_pIbAdd->Enable(false );
            bool bIsUserDef=pNumFmtShell->IsUserDefined( aFormat );
            m_pIbRemove->Enable(bIsUserDef);
            m_pIbInfo->Enable(bIsUserDef);

        }
        else
        {
            m_pIbAdd->Enable();
            m_pIbInfo->Enable();
            m_pIbRemove->Enable(false );
            m_pFtComment->SetText(m_pEdComment->GetText());

        }
        UpdateOptions_Impl( false );

        return;
    }


    // category-ListBox -------------------------------------------------
    if (pLb == m_pLbCategory || pLb == m_pLbCurrency)
    {
        UpdateFormatListBox_Impl( true, true );
        EditHdl_Impl( nullptr );
        UpdateOptions_Impl( false );

        return;
    }


    // language/country-ListBox ----------------------------------------------
    if (pLb == m_pLbLanguage)
    {
        UpdateFormatListBox_Impl( false, true );
        EditHdl_Impl(m_pEdFormat);

        return;
    }
}


/*************************************************************************
#*  Method:    ClickHdl_Impl, PushButton* pIB
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Called when the add or delete button is pushed,
#*              adjusts the number format list.
#*  Input:      Toolbox- Button
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK_TYPED( SvxNumberFormatTabPage, ClickHdl_Impl, Button*, pIB, void)
{
    Click_Impl(static_cast<PushButton*>(pIB));
}
bool SvxNumberFormatTabPage::Click_Impl(PushButton* pIB)
{
    bool            bDeleted = false;
    sal_uLong       nReturn = 0;
    const sal_uLong nReturnChanged  = 0x1;  // THE boolean return value
    const sal_uLong nReturnAdded    = 0x2;  // temp: format added
    const sal_uLong nReturnOneArea  = 0x4;  // temp: one area but category changed => ignored

    if (pIB == m_pIbAdd)
    {   // Also called from FillItemSet() if a temporary currency format has
        // to be added, not only if the Add button is enabled.
        OUString               aFormat = m_pEdFormat->GetText();
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

        if (pLastActivWindow == m_pEdComment)
        {
            m_pEdFormat->GrabFocus();
            m_pEdComment->Hide();
            m_pFtComment->Show();
            m_pFtComment->SetText(m_pEdComment->GetText());
        }

        if ( !nErrPos ) // Syntax ok?
        {
            // May be sorted under a different locale if LCID was parsed.
            if (bAdded)
                m_pLbLanguage->SelectLanguage( pNumFmtShell->GetCurLanguage() );

            if(nCatLbSelPos==CAT_CURRENCY)
            {
                m_pLbCurrency->SelectEntryPos((sal_uInt16)pNumFmtShell->GetCurrencySymbol());
            }

            if(bOneAreaFlag && (nFixedCategory!=nCatLbSelPos))
            {
                if(bAdded) aEntryList.clear();
                bDeleted = pNumFmtShell->RemoveFormat( aFormat,
                                               nCatLbSelPos,
                                               nFmtLbSelPos,
                                               a2EntryList);
                if(bDeleted) a2EntryList.clear();
                m_pEdFormat->GrabFocus();
                m_pEdFormat->SetSelection( Selection( 0, SELECTION_MAX ) );
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
                    if(m_pEdComment->GetText()!=OUString(m_pLbCategory->GetEntry(1)))
                    {
                        pNumFmtShell->SetComment4Entry(nFmtLbSelPos,
                                                    m_pEdComment->GetText());
                    }
                    else
                    {
                        pNumFmtShell->SetComment4Entry(nFmtLbSelPos,
                                                        OUString());
                    }
                    m_pLbFormat->SelectEntryPos( (sal_uInt16)nFmtLbSelPos );
                    m_pEdFormat->SetText( aFormat );

                    m_pEdComment->SetText(m_pLbCategory->GetEntry(1));    // String for user defined

                    ChangePreviewText( (sal_uInt16)nFmtLbSelPos );
                }
            }
        }
        else // syntax error
        {
            m_pEdFormat->GrabFocus();
            m_pEdFormat->SetSelection( Selection( nErrPos == -1 ? SELECTION_MAX : nErrPos, SELECTION_MAX ) );
        }
        EditHdl_Impl(m_pEdFormat);
        nReturn = ((nReturn & nReturnOneArea) ? 0 : (nReturn & nReturnChanged));

        aEntryList.clear();
        a2EntryList.clear();
    }
    else if (pIB == m_pIbRemove)
    {
        OUString              aFormat = m_pEdFormat->GetText();
        std::vector<OUString> aEntryList;
        sal_uInt16           nCatLbSelPos = 0;
        short                nFmtLbSelPos = SELPOS_NONE;

        bDeleted = pNumFmtShell->RemoveFormat( aFormat,
                                               nCatLbSelPos,
                                               nFmtLbSelPos,
                                               aEntryList );

        m_pEdComment->SetText(m_pLbCategory->GetEntry(1));
        if ( bDeleted )
        {
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

                m_pLbFormat->SelectEntryPos( (sal_uInt16)nFmtLbSelPos );
                m_pEdFormat->SetText( aFormat );
                ChangePreviewText( (sal_uInt16)nFmtLbSelPos );
            }
            else
            {
                // set to "all/standard"
                SetCategory(0);
                SelFormatHdl_Impl(m_pLbCategory);
            }
        }
        EditHdl_Impl(m_pEdFormat);

        aEntryList.clear();
    }
    else if (pIB == m_pIbInfo)
    {
        if(!(pLastActivWindow == m_pEdComment))
        {
            m_pEdComment->SetText(m_pFtComment->GetText());
            m_pEdComment->Show();
            m_pFtComment->Hide();
            m_pEdComment->GrabFocus();
        }
        else
        {
            m_pEdFormat->GrabFocus();
            m_pEdComment->Hide();
            m_pFtComment->Show();
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

IMPL_LINK_TYPED( SvxNumberFormatTabPage, EditModifyHdl_Impl, Edit&, rEdit, void )
{
    EditHdl_Impl(&rEdit);
}
void SvxNumberFormatTabPage::EditHdl_Impl( Edit* pEdFormat )
{
    sal_uInt32 nCurKey = NUMKEY_UNDEFINED;

    if ( m_pEdFormat->GetText().isEmpty() )
    {
        m_pIbAdd->Enable(false );
        m_pIbRemove->Enable(false );
        m_pIbInfo->Enable(false );
        m_pFtComment->SetText(OUString());
    }
    else
    {
        OUString aFormat = m_pEdFormat->GetText();
        MakePreviewText( aFormat );

        if ( pNumFmtShell->FindEntry( aFormat, &nCurKey ) )
        {
            m_pIbAdd->Enable(false );
            bool bUserDef=pNumFmtShell->IsUserDefined( aFormat );

            m_pIbRemove->Enable(bUserDef);
            m_pIbInfo->Enable(bUserDef);

            if(bUserDef)
            {
                sal_uInt16 nTmpCurPos=pNumFmtShell->FindCurrencyFormat(aFormat );

                if(nTmpCurPos!=(sal_uInt16)-1)
                    m_pLbCurrency->SelectEntryPos(nTmpCurPos);
            }
            short nPosi=pNumFmtShell->GetListPos4Entry(aFormat);
            if(nPosi>=0)
                m_pLbFormat->SelectEntryPos( (sal_uInt16)nPosi);

        }
        else
        {

            m_pIbAdd->Enable();
            m_pIbInfo->Enable();
            m_pIbRemove->Enable(false );

            m_pFtComment->SetText(m_pEdComment->GetText());

        }
    }

    if ( pEdFormat )
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

IMPL_LINK_TYPED( SvxNumberFormatTabPage, OptClickHdl_Impl, Button*, pOptCtrl, void )
{
    OptHdl_Impl(pOptCtrl);
}
IMPL_LINK_TYPED( SvxNumberFormatTabPage, OptEditHdl_Impl, Edit&, rEdit, void )
{
    OptHdl_Impl(&rEdit);
}
void SvxNumberFormatTabPage::OptHdl_Impl( void* pOptCtrl )
{
    if (   (pOptCtrl == m_pEdLeadZeroes)
        || (pOptCtrl == m_pEdDecimals)
        || (pOptCtrl == m_pEdDenominator)
        || (pOptCtrl == m_pBtnNegRed)
        || (pOptCtrl == m_pBtnThousand)
        || (pOptCtrl == m_pBtnEngineering) )
    {
        OUString          aFormat;
        bool          bThousand  = ( m_pBtnThousand->IsVisible() && m_pBtnThousand->IsEnabled() && m_pBtnThousand->IsChecked() )
                                || ( m_pBtnEngineering->IsVisible() && m_pBtnEngineering->IsEnabled() && m_pBtnEngineering->IsChecked() );
        bool          bNegRed    =   m_pBtnNegRed->IsEnabled() && m_pBtnNegRed->IsChecked();
        sal_uInt16    nPrecision = (m_pEdDecimals->IsEnabled() && m_pEdDecimals->IsVisible())
                                 ? (sal_uInt16)m_pEdDecimals->GetValue()
                                 : ( (m_pEdDenominator->IsEnabled() && m_pEdDenominator->IsVisible())
                                   ? (sal_uInt16)m_pEdDenominator->GetValue()
                                   : (sal_uInt16)0 );
        sal_uInt16    nLeadZeroes = (m_pEdLeadZeroes->IsEnabled())
                                 ? (sal_uInt16)m_pEdLeadZeroes->GetValue()
                                 : (sal_uInt16)0;
        if ( pNumFmtShell->GetStandardName() == m_pEdFormat->GetText() )
        {
            m_pEdDecimals->SetValue( nPrecision );
        }

        pNumFmtShell->MakeFormat( aFormat,
                                  bThousand, bNegRed,
                                  nPrecision, nLeadZeroes,
                                  (sal_uInt16)m_pLbFormat->GetSelectEntryPos() );

        m_pEdFormat->SetText( aFormat );
        MakePreviewText( aFormat );

        if ( pNumFmtShell->FindEntry( aFormat ) )
        {
            m_pIbAdd->Enable(false );
            bool bUserDef=pNumFmtShell->IsUserDefined( aFormat );
            m_pIbRemove->Enable(bUserDef);
            m_pIbInfo->Enable(bUserDef);
            EditHdl_Impl(m_pEdFormat);

        }
        else
        {
            EditHdl_Impl( nullptr );
            m_pLbFormat->SetNoSelection();
        }
    }
}

IMPL_LINK_NOARG_TYPED(SvxNumberFormatTabPage, TimeHdl_Impl, Timer *, void)
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

IMPL_LINK_TYPED( SvxNumberFormatTabPage, LostFocusHdl_Impl, Control&, rControl, void)
{
    Edit* pEd = static_cast<Edit*>(&rControl);
    if (pEd == m_pEdComment)
    {
        aResetWinTimer.Start();
        m_pFtComment->SetText(m_pEdComment->GetText());
        m_pEdComment->Hide();
        m_pFtComment->Show();
        if(!m_pIbAdd->IsEnabled())
        {
            sal_uInt16  nSelPos = (sal_uInt16) m_pLbFormat->GetSelectEntryPos();
            pNumFmtShell->SetComment4Entry(nSelPos,
                                        m_pEdComment->GetText());
            m_pEdComment->SetText(m_pLbCategory->GetEntry(1));    // String for user defined
        }
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
        case CAT_ALL:           i=SVX_NUMVAL_STANDARD; break;

        case CAT_NUMBER:        i=SVX_NUMVAL_STANDARD; break;

        case CAT_PERCENT:       i=SVX_NUMVAL_PERCENT; break;

        case CAT_CURRENCY:      i=SVX_NUMVAL_CURRENCY; break;

        case CAT_DATE:          i=SVX_NUMVAL_DATE; break;

        case CAT_TIME:          i=SVX_NUMVAL_TIME; break;

        case CAT_SCIENTIFIC:    i=SVX_NUMVAL_SCIENTIFIC; break;

        case CAT_FRACTION:      i=SVX_NUMVAL_FRACTION; break;

        case CAT_BOOLEAN:       i=SVX_NUMVAL_BOOLEAN; break;

        case CAT_USERDEFINED:   i=SVX_NUMVAL_STANDARD; break;

        case CAT_TEXT:
        default:                i=SVX_NUMVAL_NOVALUE;break;
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
    m_pWndPreview->NotifyChange( aPreviewString, pPreviewColor );
}

void SvxNumberFormatTabPage::ChangePreviewText( sal_uInt16 nPos )
{
    OUString aPreviewString;
    Color* pPreviewColor = nullptr;
    pNumFmtShell->FormatChanged( nPos, aPreviewString, pPreviewColor );
    m_pWndPreview->NotifyChange( aPreviewString, pPreviewColor );
}

bool SvxNumberFormatTabPage::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType()==MouseNotifyEvent::LOSEFOCUS)
    {
        if ( rNEvt.GetWindow() == dynamic_cast<const  vcl::Window* >( m_pEdComment.get() ) && !m_pEdComment->IsVisible() )
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
/*************************************************************************
#*  Method:    SetOkHdl
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Resets the OkHandler.
#*  Input:      new OkHandler
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::SetOkHdl( const Link<SfxPoolItem*,void>& rOkHandler )
{
    fnOkHdl = rOkHandler;
}

void SvxNumberFormatTabPage::FillCurrencyBox()
{
    std::vector<OUString> aList;

    sal_uInt16  nSelPos=0;
    pNumFmtShell->GetCurrencySymbols(aList, &nSelPos);

    for(std::vector<OUString>::iterator i = aList.begin() + 1;i != aList.end(); ++i)
        m_pLbCurrency->InsertEntry(*i);

    // Initially disable the "Automatically" entry. First ensure that nothing
    // is selected, else if the to be disabled (first) entry was selected it
    // would be sticky when disabled and could not be deselected!
    m_pLbCurrency->SetNoSelection();
    m_pLbCurrency->SetEntryFlags( 0, ListBoxEntryFlags::DisableSelection | ListBoxEntryFlags::DrawDisabled);
    m_pLbCurrency->SelectEntryPos(nSelPos);
}

void SvxNumberFormatTabPage::SetCategory(sal_uInt16 nPos)
{
    sal_uInt16  nCurCategory = m_pLbCategory->GetSelectEntryPos();
    sal_uInt16 nTmpCatPos;

    if(bOneAreaFlag)
    {
        nTmpCatPos=nFixedCategory;
    }
    else
    {
        nTmpCatPos=nPos;
    }

    if(m_pLbCategory->GetEntryCount()==1 || nCurCategory!=nPos)
    {
        if(nTmpCatPos!=CAT_CURRENCY)
            m_pLbCurrency->Hide();
        else
            m_pLbCurrency->Show();
    }
    m_pLbCategory->SelectEntryPos(nPos);
}
/* to support Writer text field language handling an
   additional entry needs to be inserted into the ListBox
   which marks a certain language as automatically detected
   Additionally the "Default" language is removed
*/
void SvxNumberFormatTabPage::AddAutomaticLanguage_Impl(LanguageType eAutoLang, bool bSelect)
{
    m_pLbLanguage->RemoveLanguage(LANGUAGE_SYSTEM);
    const sal_Int32 nPos = m_pLbLanguage->InsertEntry(sAutomaticEntry);
    m_pLbLanguage->SetEntryData(nPos, reinterpret_cast<void*>((sal_uLong)eAutoLang));
    if(bSelect)
        m_pLbLanguage->SelectEntryPos(nPos);
}

void SvxNumberFormatTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SvxNumberInfoItem* pNumberInfoItem = aSet.GetItem<SvxNumberInfoItem>(SID_ATTR_NUMBERFORMAT_INFO, false);
    const SfxLinkItem* pLinkItem = aSet.GetItem<SfxLinkItem>(SID_LINK_TYPE, false);
    if (pNumberInfoItem)
        SetNumberFormatList(*pNumberInfoItem);
    if (pLinkItem)
        SetOkHdl(pLinkItem->GetValue());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
