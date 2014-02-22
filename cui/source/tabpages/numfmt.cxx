/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/builder.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unotools/localedatawrapper.hxx>
#include <i18nlangtag/lang.h>
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

using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::uno::UNO_QUERY;

#define NUMKEY_UNDEFINED SAL_MAX_UINT32



static sal_uInt16 pRanges[] =
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

SvxNumberPreview::SvxNumberPreview(Window* pParent, WinBits nStyle)
    : Window(pParent, nStyle)
    , mnPos(-1)
    , mnChar(0x0)
{
    Font aFont( GetFont() );
    aFont.SetTransparent( true );
    aFont.SetColor( Application::GetSettings().GetStyleSettings().GetFieldColor() );
    SetFont( aFont );
    InitSettings( sal_True, sal_True );
    SetBorderStyle( WINDOW_BORDER_MONO );
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxNumberPreview(Window *pParent, VclBuilder::stringmap &)
{
    return new SvxNumberPreview(pParent);
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
    
    aPrevStr = rPrevStr;
    mnPos = aPrevStr.indexOf( 0x1B );
    if ( mnPos != -1 )
    {
        mnChar = aPrevStr[ mnPos + 1 ];
        
        aPrevStr = aPrevStr.replaceAt( mnPos, 2, "" );
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

void SvxNumberPreview::Paint( const Rectangle& )
{
    Font    aDrawFont   = GetFont();
    Size    aSzWnd      = GetOutputSizePixel();
    OUString aTmpStr( aPrevStr );
    long    nLeadSpace = (aSzWnd.Width()  - GetTextWidth( aTmpStr )) /2;

    aDrawFont.SetColor( aPrevCol );
    SetFont( aDrawFont );

    if ( mnPos != -1 )
    {
        long nCharWidth = GetTextWidth( OUString( mnChar ) );

        int nNumCharsToInsert = 0;
        if (nCharWidth > 0) nNumCharsToInsert = nLeadSpace / nCharWidth;

        if ( nNumCharsToInsert > 0)
        {
            for ( int i = 0; i < nNumCharsToInsert; ++i )
                aTmpStr = aTmpStr.replaceAt( mnPos, 0, OUString(mnChar) );
        }
    }
    Point   aPosText    = Point( ( mnPos != -1 ) ? 0 : nLeadSpace,
                                 (aSzWnd.Height() - GetTextHeight())/2 );
    DrawText( aPosText, aTmpStr );
}



void SvxNumberPreview::InitSettings( sal_Bool bForeground, sal_Bool bBackground )
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
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
        InitSettings( sal_True, sal_False );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        InitSettings( sal_False, sal_True );

    Window::StateChanged( nType );
}



void SvxNumberPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings( sal_True, sal_True );
}



#define REMOVE_DONTKNOW() \
    if ( !m_pFtLanguage->IsEnabled() )                                     \
    {                                                                   \
        m_pFtLanguage->Enable();                                          \
        m_pLbLanguage->Enable();                                          \
        m_pLbLanguage->SelectLanguage( pNumFmtShell->GetCurLanguage() );  \
    }

#define HDL(hdl) LINK( this, SvxNumberFormatTabPage, hdl )

SvxNumberFormatTabPage::SvxNumberFormatTabPage(Window* pParent,
    const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pParent, "NumberingFormatPage",
        "cui/ui/numberingformatpage.ui", rCoreAttrs)
    , pNumItem(NULL)
    , pNumFmtShell(NULL)
    , nInitFormat(ULONG_MAX)
    , sAutomaticEntry(CUI_RES(RID_SVXSTR_AUTO_ENTRY))
    , pLastActivWindow(NULL)
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
    get(m_pBtnNegRed, "negnumred");
    get(m_pFtLeadZeroes, "leadzerosft");
    get(m_pEdLeadZeroes, "leadzerosed");
    get(m_pBtnThousand, "thousands");
    get(m_pFormatCodeFrame, "formatcode");
    get(m_pEdFormat, "formated");
    get(m_pIbAdd, "add");
    get(m_pIbInfo, "edit");
    get(m_pIbRemove, "delete");
    get(m_pFtComment, "commentft");
    get(m_pEdComment, "commented");

    m_pLbCategory->SetDropDownLineCount(8);
    m_pWndPreview->set_height_request(GetTextHeight()*3);

    Init_Impl();
    SetExchangeSupport(); 
    nFixedCategory=-1;
}

SvxNumberFormatTabPage::~SvxNumberFormatTabPage()
{
    delete pNumFmtShell;
    delete pNumItem;
}

void SvxNumberFormatTabPage::Init_Impl()
{
    bNumItemFlag=sal_True;
    bOneAreaFlag=sal_False;

    m_pIbAdd->Enable(false );
    m_pIbRemove->Enable(false );
    m_pIbInfo->Enable(false );

    m_pEdComment->SetText(m_pLbCategory->GetEntry(1));    

    m_pEdComment->Hide();

    m_pCbSourceFormat->Check( false );
    m_pCbSourceFormat->Disable();
    m_pCbSourceFormat->Hide();

    Link aLink = LINK( this, SvxNumberFormatTabPage, SelFormatHdl_Impl );

    m_pLbCategory->SetSelectHdl( aLink );
    m_pLbFormat->SetSelectHdl( aLink );
    m_pLbLanguage->SetSelectHdl( aLink );
    m_pLbCurrency->SetSelectHdl( aLink );
    m_pCbSourceFormat->SetClickHdl( aLink );

    aLink = LINK( this, SvxNumberFormatTabPage, OptHdl_Impl );

    m_pEdDecimals->SetModifyHdl( aLink );
    m_pEdLeadZeroes->SetModifyHdl( aLink );
    m_pBtnNegRed->SetClickHdl( aLink );
    m_pBtnThousand->SetClickHdl( aLink );
    m_pLbFormat->SetDoubleClickHdl( HDL( DoubleClickHdl_Impl ) );
    m_pEdFormat->SetModifyHdl( HDL( EditHdl_Impl ) );
    m_pIbAdd->SetClickHdl( HDL( ClickHdl_Impl ) );
    m_pIbRemove->SetClickHdl( HDL( ClickHdl_Impl ) );
    m_pIbInfo->SetClickHdl( HDL( ClickHdl_Impl ) );

    aLink = LINK( this, SvxNumberFormatTabPage, LostFocusHdl_Impl);

    m_pEdComment->SetLoseFocusHdl( aLink);
    aResetWinTimer.SetTimeoutHdl(LINK( this, SvxNumberFormatTabPage, TimeHdl_Impl));
    aResetWinTimer.SetTimeout( 10);

    

    m_pLbLanguage->InsertLanguage( LANGUAGE_SYSTEM );
    
    
    
    ::com::sun::star::uno::Sequence< sal_uInt16 > xLang =
        LocaleDataWrapper::getInstalledLanguageTypes();
    sal_Int32 nCount = xLang.getLength();
    for ( sal_Int32 i=0; i<nCount; i++ )
    {
        m_pLbLanguage->InsertLanguage( xLang[i] );
    }
}

/*************************************************************************
#*  Method:        GetRanges
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Returns area information.
#*  Input:      ---
#*  Output:     area
#*
#************************************************************************/

sal_uInt16* SvxNumberFormatTabPage::GetRanges()
{
    return pRanges;
}


/*************************************************************************
#*  Method:        Create
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Creates a new number format page.
#*  Input:      Window, SfxItemSet
#*  Output:     new TabPage
#*
#************************************************************************/

SfxTabPage* SvxNumberFormatTabPage::Create( Window* pParent,
                                            const SfxItemSet& rAttrSet )
{
    return ( new SvxNumberFormatTabPage( pParent, rAttrSet ) );
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

void SvxNumberFormatTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxUInt32Item*        pValFmtAttr     = NULL;
    const SfxPoolItem*          pItem           = NULL;
    const SfxBoolItem*          pAutoEntryAttr = NULL;

    sal_uInt16                      nCatLbSelPos    = 0;
    sal_uInt16                      nFmtLbSelPos    = 0;
    LanguageType                eLangType       = LANGUAGE_DONTKNOW;
    std::vector<OUString>       aFmtEntryList;
    SvxNumberValueType          eValType        = SVX_VALUE_TYPE_UNDEFINED;
    double                      nValDouble      = 0;
    OUString                    aValString;

    SfxItemState eState = rSet.GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_NOLANGUAGE ),true,&pItem);

    if(eState==SFX_ITEM_SET)
    {
        const SfxBoolItem* pBoolLangItem = (const SfxBoolItem*)
                      GetItem( rSet, SID_ATTR_NUMBERFORMAT_NOLANGUAGE);

        if(pBoolLangItem!=NULL && pBoolLangItem->GetValue())
        {
            HideLanguage();
        }
        else
        {
            HideLanguage(sal_False);
        }

    }

    eState = rSet.GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_INFO ),true,&pItem);

    if(eState==SFX_ITEM_SET)
    {
        if(pNumItem==NULL)
        {
            bNumItemFlag=sal_True;
            pNumItem= (SvxNumberInfoItem *) pItem->Clone();
        }
        else
        {
            bNumItemFlag=sal_False;
        }
    }
    else
    {
        bNumItemFlag=sal_False;
    }


    eState = rSet.GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_ONE_AREA ));

    if(eState==SFX_ITEM_SET)
    {
        const SfxBoolItem* pBoolItem = (const SfxBoolItem*)
                      GetItem( rSet, SID_ATTR_NUMBERFORMAT_ONE_AREA);

        if(pBoolItem!=NULL)
        {
            bOneAreaFlag= pBoolItem->GetValue();
        }
    }

    eState = rSet.GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_SOURCE ) );

    if ( eState == SFX_ITEM_SET )
    {
        const SfxBoolItem* pBoolItem = (const SfxBoolItem*)
                      GetItem( rSet, SID_ATTR_NUMBERFORMAT_SOURCE );
        if ( pBoolItem )
            m_pCbSourceFormat->Check( pBoolItem->GetValue() );
        else
            m_pCbSourceFormat->Check( false );
        m_pCbSourceFormat->Enable();
        m_pCbSourceFormat->Show();
    }
    else
    {
        sal_Bool bInit = sal_False;     
        m_pCbSourceFormat->Check( bInit );
        m_pCbSourceFormat->Enable( bInit );
        m_pCbSourceFormat->Show( bInit );
    }

    
    DBG_ASSERT( pNumItem, "No NumberInfo, no NumberFormatter, good bye.CRASH. :-(" );

    eState = rSet.GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_VALUE ) );

    if ( SFX_ITEM_DONTCARE != eState )
        pValFmtAttr = (const SfxUInt32Item*)
                      GetItem( rSet, SID_ATTR_NUMBERFORMAT_VALUE );

    eValType = pNumItem->GetValueType();

    switch ( eValType )
    {
        case SVX_VALUE_TYPE_STRING:
            aValString = pNumItem->GetValueString();
            break;
        case SVX_VALUE_TYPE_NUMBER:
            
            aValString = pNumItem->GetValueString();
            nValDouble = pNumItem->GetValueDouble();
            break;
        case SVX_VALUE_TYPE_UNDEFINED:
        default:
            break;
    }

    if ( pNumFmtShell )
         delete pNumFmtShell;   

    nInitFormat = ( pValFmtAttr )               
                    ? pValFmtAttr->GetValue()   
                    : ULONG_MAX;                


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
        
        Reference< XServiceInfo > xSI( pDocSh->GetModel(), UNO_QUERY );
        if ( xSI.is() )
            bUseStarFormat = xSI->supportsService("com.sun.star.sheet.SpreadsheetDocument");
    }
    pNumFmtShell->SetUseStarFormat( bUseStarFormat );

    FillCurrencyBox();

    OUString aPrevString;
    Color* pDummy = NULL;
    pNumFmtShell->GetInitSettings( nCatLbSelPos, eLangType, nFmtLbSelPos,
                                   aFmtEntryList, aPrevString, pDummy );

    m_pLbCurrency->SelectEntryPos((sal_uInt16)pNumFmtShell->GetCurrencySymbol());

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
    eState = rSet.GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_ADD_AUTO ) );
    if(SFX_ITEM_SET == eState)
         pAutoEntryAttr = (const SfxBoolItem*)
                      GetItem( rSet, SID_ATTR_NUMBERFORMAT_ADD_AUTO );
    
    
    
    if ( eLangType == LANGUAGE_NORWEGIAN )
    {
        m_pLbLanguage->RemoveLanguage( eLangType );    
        m_pLbLanguage->InsertLanguage( eLangType );
    }
    m_pLbLanguage->SelectLanguage( eLangType );
    if(pAutoEntryAttr)
        AddAutomaticLanguage_Impl(eLangType, pAutoEntryAttr->GetValue());
    UpdateFormatListBox_Impl(sal_False,sal_True);






    if ( pValFmtAttr )
    {
        EditHdl_Impl(m_pEdFormat); 
    }
    else    
    {
        
        Obstructing();
    }

    if ( m_pCbSourceFormat->IsChecked() )
    {
        
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
    m_pFtLeadZeroes->Disable();
    m_pFtDecimals->Disable();
    m_pEdLeadZeroes->Disable();
    m_pEdDecimals->Disable();
    m_pFtOptions->Disable();
    m_pEdDecimals->SetText( OUString() );
    m_pEdLeadZeroes->SetText( OUString() );
    m_pBtnNegRed->Check( false );
    m_pBtnThousand->Check( false );
    m_pWndPreview->NotifyChange( OUString() );

    m_pLbCategory->SelectEntryPos( 0 );
    m_pEdFormat->SetText( OUString() );
    m_pFtComment->SetText( OUString() );
    m_pEdComment->SetText(m_pLbCategory->GetEntry(1));  

    m_pEdFormat->GrabFocus();
}


/*************************************************************************
#* Enable/Disable dialog parts depending on the value of the SourceFormat
#* checkbox.
#************************************************************************/
void SvxNumberFormatTabPage::EnableBySourceFormat_Impl()
{
    sal_Bool bEnable = !m_pCbSourceFormat->IsChecked();
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
    m_pFtLeadZeroes->Enable( bEnable );
    m_pEdLeadZeroes->Enable( bEnable );
    m_pBtnNegRed->Enable( bEnable );
    m_pBtnThousand->Enable( bEnable );
    m_pFtOptions->Enable( bEnable );
    m_pFormatCodeFrame->Enable( bEnable );
    m_pLbFormat->Invalidate(); 
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

void SvxNumberFormatTabPage::HideLanguage(sal_Bool nFlag)
{
    m_pFtLanguage->Show(!nFlag);
    m_pLbLanguage->Show(!nFlag);
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

sal_Bool SvxNumberFormatTabPage::FillItemSet( SfxItemSet& rCoreAttrs )
{
    sal_Bool bDataChanged   = m_pFtLanguage->IsEnabled() || m_pCbSourceFormat->IsEnabled();
    if ( bDataChanged )
    {
        const SfxItemSet& rMyItemSet = GetItemSet();
        sal_uInt16          nWhich       = GetWhich( SID_ATTR_NUMBERFORMAT_VALUE );
        SfxItemState    eItemState   = rMyItemSet.GetItemState( nWhich, false );

        
        
        OUString    aFormat = m_pEdFormat->GetText();
        sal_uInt32 nCurKey = pNumFmtShell->GetCurNumFmtKey();

        if ( m_pIbAdd->IsEnabled() || pNumFmtShell->IsTmpCurrencyFormat(aFormat) )
        {   
            
            
            
            bDataChanged = (ClickHdl_Impl(m_pIbAdd) != 0);
            nCurKey = pNumFmtShell->GetCurNumFmtKey();
        }
        else if(nCurKey == NUMKEY_UNDEFINED)
        {   
            pNumFmtShell->FindEntry(aFormat, &nCurKey);
        }

        
        
        
        if ( bDataChanged )
        {
            bDataChanged = ( nInitFormat != nCurKey );

            if (bDataChanged)
            {
                rCoreAttrs.Put( SfxUInt32Item( nWhich, nCurKey ) );
            }
            else if(SFX_ITEM_DEFAULT == eItemState)
            {
                rCoreAttrs.ClearItem( nWhich );
            }
        }

        
        
        
        const size_t nDelCount = pNumFmtShell->GetUpdateDataCount();

        if ( nDelCount > 0 )
        {
            sal_uInt32*         pDelArr = new sal_uInt32[nDelCount];

            pNumFmtShell->GetUpdateData( pDelArr, nDelCount );
            pNumItem->SetDelFormatArray( pDelArr, nDelCount );

            if(bNumItemFlag==sal_True)
            {
                rCoreAttrs.Put( *pNumItem );
            }
            else
            {
                SfxObjectShell* pDocSh  = SfxObjectShell::Current();

                DBG_ASSERT( pDocSh, "DocShell not found!" );


                if ( pDocSh )
                    pDocSh->PutItem( *pNumItem );
            }
            delete [] pDelArr;
        }

        
        
        
        if ( m_pCbSourceFormat->IsEnabled() )
        {
            sal_uInt16 _nWhich = GetWhich( SID_ATTR_NUMBERFORMAT_SOURCE );
            SfxItemState _eItemState = rMyItemSet.GetItemState( _nWhich, false );
            const SfxBoolItem* pBoolItem = (const SfxBoolItem*)
                        GetItem( rMyItemSet, SID_ATTR_NUMBERFORMAT_SOURCE );
            sal_Bool bOld = (pBoolItem ? pBoolItem->GetValue() : sal_False);
            rCoreAttrs.Put( SfxBoolItem( _nWhich, m_pCbSourceFormat->IsChecked() ) );
            if ( !bDataChanged )
                bDataChanged = (bOld != (sal_Bool) m_pCbSourceFormat->IsChecked() ||
                    _eItemState != SFX_ITEM_SET);
        }

        
        
        pNumFmtShell->ValidateNewEntries();
        if(m_pLbLanguage->IsVisible() &&
                LISTBOX_ENTRY_NOTFOUND != m_pLbLanguage->GetEntryPos(sAutomaticEntry))
                rCoreAttrs.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ADD_AUTO,
                    m_pLbLanguage->GetSelectEntry() == sAutomaticEntry));
    }

    return bDataChanged;
}


int SvxNumberFormatTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

void SvxNumberFormatTabPage::SetInfoItem( const SvxNumberInfoItem& rItem )
{
    if(pNumItem==NULL)
    {
        pNumItem = (SvxNumberInfoItem*)rItem.Clone();
    }
}

void SvxNumberFormatTabPage::FillFormatListBox_Impl( std::vector<OUString>& rEntries )
{
    OUString    aEntry;
    OUString    aTmpString;
    Font        aFont=m_pLbCategory->GetFont();
    size_t      i = 0;
    short       nTmpCatPos;
    short       aPrivCat;

    m_pLbFormat->Clear();
    m_pLbFormat->SetUpdateMode( sal_False );

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
                                aPrivCat=pNumFmtShell->GetCategory4Entry(0);
                                m_pLbFormat->InsertFontEntry( aTmpString, aFont );
                                break;

        default:                break;
    }

    if(pNumFmtShell!=NULL)
    {
        for ( ; i < rEntries.size(); ++i )
        {
            aEntry = rEntries[i];
            aPrivCat=pNumFmtShell->GetCategory4Entry( static_cast<sal_uInt16>(i) );
            if(aPrivCat!=CAT_TEXT)
            {
                Color* pPreviewColor = NULL;
                OUString aPreviewString( GetExpColorString( pPreviewColor, aEntry, aPrivCat ) );
                Font aEntryFont( m_pLbFormat->GetFont() );
                m_pLbFormat->InsertFontEntry( aPreviewString, aEntryFont, pPreviewColor );
            }
            else
            {
                m_pLbFormat->InsertFontEntry(aEntry,aFont);
            }
        }
    }
    m_pLbFormat->SetUpdateMode( sal_True );
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

void SvxNumberFormatTabPage::UpdateOptions_Impl( sal_Bool bCheckCatChange /*= sal_False*/ )
{
    OUString  theFormat           = m_pEdFormat->GetText();
    sal_uInt16  nCurCategory        = m_pLbCategory->GetSelectEntryPos();
    sal_uInt16  nCategory           = nCurCategory;
    sal_uInt16  nDecimals           = 0;
    sal_uInt16  nZeroes             = 0;
    bool        bNegRed             = false;
    bool        bThousand           = false;
    sal_uInt16  nCurrencyPos        =m_pLbCurrency->GetSelectEntryPos();

    if(bOneAreaFlag)
        nCurCategory=nFixedCategory;


    pNumFmtShell->GetOptions( theFormat,
                              bThousand, bNegRed,
                              nDecimals, nZeroes,
                              nCategory );
    sal_Bool bDoIt=sal_False;
    if(nCategory==CAT_CURRENCY)
    {
        sal_uInt16 nTstPos=pNumFmtShell->FindCurrencyFormat(theFormat);
        if(nCurrencyPos!=nTstPos && nTstPos!=(sal_uInt16)-1)
        {
            m_pLbCurrency->SelectEntryPos(nTstPos);
            pNumFmtShell->SetCurrencySymbol(nTstPos);
            bDoIt=sal_True;
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

            UpdateFormatListBox_Impl( sal_True, sal_False );
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

    switch ( nCategory )
    {
        case CAT_NUMBER:
        case CAT_PERCENT:
        case CAT_CURRENCY:
            m_pFtOptions->Enable();
            m_pFtDecimals->Enable();
            m_pEdDecimals->Enable();
            m_pFtLeadZeroes->Enable();
            m_pEdLeadZeroes->Enable();
            m_pBtnNegRed->Enable();
            m_pBtnThousand->Enable();
            m_pEdDecimals->SetText( OUString::number( nDecimals ) );
            m_pEdLeadZeroes->SetText( OUString::number( nZeroes ) );
            m_pBtnNegRed->Check( bNegRed );
            m_pBtnThousand->Check( bThousand );
            break;

        case CAT_ALL:
        case CAT_USERDEFINED:
        case CAT_TEXT:
        case CAT_DATE:
        case CAT_TIME:
        case CAT_BOOLEAN:
        case CAT_SCIENTIFIC:
        case CAT_FRACTION:
        default:
            m_pFtOptions->Disable();
            m_pFtDecimals->Disable();
            m_pEdDecimals->Disable();
            m_pFtLeadZeroes->Disable();
            m_pEdLeadZeroes->Disable();
            m_pBtnNegRed->Disable();
            m_pBtnThousand->Disable();
            m_pEdDecimals->SetText( OUString::number( 0 ) );
            m_pEdLeadZeroes->SetText( OUString::number( 0 ) );
            m_pBtnNegRed->Check( false );
            m_pBtnThousand->Check( false );
    }
}


/*************************************************************************
#*  Method:        UpdateFormatListBox_Impl
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Updates the format lisbox and additionally the
#*              string in the editbox is changed depending on
#*              the bUpdateEdit flag.
#*  Input:      Flags for category and editbox.
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::UpdateFormatListBox_Impl
    (
        sal_uInt16 bCat,        
        sal_Bool   bUpdateEdit
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

    REMOVE_DONTKNOW() 


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
#*  Handle:     DoubleClickHdl_Impl
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   On a double click in the format lisbox the
#*              value is adopted and the OK button pushed.
#*  Input:      Pointer on the Listbox
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( SvxNumberFormatTabPage, DoubleClickHdl_Impl, SvxFontListBox*, pLb )
{
    if (pLb == m_pLbFormat)
    {
        SelFormatHdl_Impl( pLb );

        if ( fnOkHdl.IsSet() )
        {   
            fnOkHdl.Call( NULL );
        }
        else
        {
            SfxSingleTabDialog* pParent = dynamic_cast< SfxSingleTabDialog* >( GetParentDialog() );
            OKButton* pOKButton = pParent ? pParent->GetOKButton() : NULL;
            if ( pOKButton )
                pOKButton->Click();
        }
    }
    return 0;
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

IMPL_LINK( SvxNumberFormatTabPage, SelFormatHdl_Impl, void *, pLb )
{
    if (pLb == m_pCbSourceFormat)
    {
        EnableBySourceFormat_Impl();    
        if ( m_pCbSourceFormat->IsChecked() )
            return 0;   

        

        
        
        
        
        if ( m_pLbFormat->GetSelectEntryPos() == LIST_APPEND )
            pLb = m_pLbCategory; 
        else
            pLb = m_pLbFormat;   
    }

    short       nTmpCatPos;

    if(bOneAreaFlag)
    {
        nTmpCatPos=nFixedCategory;
    }
    else
    {
        nTmpCatPos=m_pLbCategory->GetSelectEntryPos();
    }

    sal_uInt16 nCurrencyPos=LISTBOX_ENTRY_NOTFOUND ;

    if (nTmpCatPos==CAT_CURRENCY && pLb == m_pLbCurrency )
    {
        nCurrencyPos = m_pLbCurrency->GetSelectEntryPos();
        pNumFmtShell->SetCurrencySymbol(nCurrencyPos);
    }

    
    
    if (pLb == m_pLbFormat)
    {
        sal_uInt16  nSelPos = (sal_uInt16) m_pLbFormat->GetSelectEntryPos();
        OUString  aFormat = m_pLbFormat->GetSelectEntry();
        OUString  aComment;

        short       nFmtLbSelPos = nSelPos;

        aFormat=pNumFmtShell->GetFormat4Entry(nSelPos);
        aComment=pNumFmtShell->GetComment4Entry(nSelPos);
        if(pNumFmtShell->GetUserDefined4Entry(nFmtLbSelPos))
        {
            if(pNumFmtShell->GetComment4Entry(nFmtLbSelPos).isEmpty())
            {
                aComment = m_pLbCategory->GetEntry(1);
            }
        }

        if ( !aFormat.isEmpty() )
        {
            if(!m_pEdFormat->HasFocus()) m_pEdFormat->SetText( aFormat );
            m_pFtComment->SetText(aComment);
            ChangePreviewText( nSelPos );
        }

        REMOVE_DONTKNOW() 

        if ( pNumFmtShell->FindEntry( aFormat) )
        {
            m_pIbAdd->Enable(false );
            sal_Bool bIsUserDef=pNumFmtShell->IsUserDefined( aFormat );
            m_pIbRemove->Enable(bIsUserDef);
            m_pIbInfo->Enable(bIsUserDef);

        }
        else
        {
            m_pIbAdd->Enable(true );
            m_pIbInfo->Enable(true );
            m_pIbRemove->Enable(false );
            m_pFtComment->SetText(m_pEdComment->GetText());

        }
        UpdateOptions_Impl( sal_False );

        return 0;
    }

    
    
    if (pLb == m_pLbCategory || pLb == m_pLbCurrency)
    {
        UpdateFormatListBox_Impl( sal_True, sal_True );
        EditHdl_Impl( NULL );
        UpdateOptions_Impl( sal_False );

        return 0;
    }

    
    
    if (pLb == m_pLbLanguage)
    {
        UpdateFormatListBox_Impl( sal_False, sal_True );
        EditHdl_Impl(m_pEdFormat);

        return 0;
    }
    return 0;
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

IMPL_LINK( SvxNumberFormatTabPage, ClickHdl_Impl, PushButton*, pIB)
{
    sal_Bool        bAdded = sal_False;
    sal_Bool        bDeleted = sal_False;
    sal_uLong       nReturn = 0;
    const sal_uLong nReturnChanged  = 0x1;  
    const sal_uLong nReturnAdded    = 0x2;  
    const sal_uLong nReturnOneArea  = 0x4;  

    if (pIB == m_pIbAdd)
    {   
        
        OUString               aFormat = m_pEdFormat->GetText();
        std::vector<OUString> aEntryList;
        std::vector<OUString> a2EntryList;
        sal_uInt16           nCatLbSelPos = 0;
        short                nFmtLbSelPos = SELPOS_NONE;
        sal_Int32            nErrPos=0;

        pNumFmtShell->SetCurCurrencyEntry(NULL);
        bAdded = pNumFmtShell->AddFormat( aFormat, nErrPos,
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

        if ( !nErrPos ) 
        {
            
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
                    
                    if(bOneAreaFlag)                  
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

                    m_pEdComment->SetText(m_pLbCategory->GetEntry(1));    

                    ChangePreviewText( (sal_uInt16)nFmtLbSelPos );
                }
            }
        }
        else 
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
                if(bOneAreaFlag)                  
                        SetCategory(0);
                    else
                        SetCategory(nCatLbSelPos );

                m_pLbFormat->SelectEntryPos( (sal_uInt16)nFmtLbSelPos );
                m_pEdFormat->SetText( aFormat );
                ChangePreviewText( (sal_uInt16)nFmtLbSelPos );
            }
            else
            {
                
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

IMPL_LINK( SvxNumberFormatTabPage, EditHdl_Impl, Edit*, pEdFormat )
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
            sal_Bool bUserDef=pNumFmtShell->IsUserDefined( aFormat );

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

            m_pIbAdd->Enable(true );
            m_pIbInfo->Enable(true);
            m_pIbRemove->Enable(false );

            m_pFtComment->SetText(m_pEdComment->GetText());

        }
    }

    if ( pEdFormat )
    {
        pNumFmtShell->SetCurNumFmtKey( nCurKey );
        UpdateOptions_Impl( sal_True );
    }

    return 0;
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

IMPL_LINK( SvxNumberFormatTabPage, OptHdl_Impl, void *, pOptCtrl )
{
    if (   (pOptCtrl == m_pEdLeadZeroes)
        || (pOptCtrl == m_pEdDecimals)
        || (pOptCtrl == m_pBtnNegRed)
        || (pOptCtrl == m_pBtnThousand) )
    {
        OUString          aFormat;
        sal_Bool          bThousand     =    m_pBtnThousand->IsEnabled()
                                      && m_pBtnThousand->IsChecked();
        sal_Bool          bNegRed       =    m_pBtnNegRed->IsEnabled()
                                      && m_pBtnNegRed->IsChecked();
        sal_uInt16        nPrecision    = (m_pEdDecimals->IsEnabled())
                                        ? (sal_uInt16)m_pEdDecimals->GetValue()
                                        : (sal_uInt16)0;
        sal_uInt16        nLeadZeroes   = (m_pEdLeadZeroes->IsEnabled())
                                        ? (sal_uInt16)m_pEdLeadZeroes->GetValue()
                                        : (sal_uInt16)0;

        pNumFmtShell->MakeFormat( aFormat,
                                  bThousand, bNegRed,
                                  nPrecision, nLeadZeroes,
                                  (sal_uInt16)m_pLbFormat->GetSelectEntryPos() );

        m_pEdFormat->SetText( aFormat );
        MakePreviewText( aFormat );

        if ( pNumFmtShell->FindEntry( aFormat ) )
        {
            m_pIbAdd->Enable(false );
            sal_Bool bUserDef=pNumFmtShell->IsUserDefined( aFormat );
            m_pIbRemove->Enable(bUserDef);
            m_pIbInfo->Enable(bUserDef);
            EditHdl_Impl(m_pEdFormat);

        }
        else
        {
            EditHdl_Impl( NULL );
            m_pLbFormat->SetNoSelection();
        }
    }
    return 0;
}

IMPL_LINK_NOARG(SvxNumberFormatTabPage, TimeHdl_Impl)
{
    pLastActivWindow=NULL;
    return 0;
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

IMPL_LINK( SvxNumberFormatTabPage, LostFocusHdl_Impl, Edit *, pEd)
{
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
            m_pEdComment->SetText(m_pLbCategory->GetEntry(1));    
        }
    }
    return 0;
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
    double nVal = 0;
    switch (nTmpCatPos)
    {
        case CAT_CURRENCY:      nVal=SVX_NUMVAL_CURRENCY; break;

        case CAT_SCIENTIFIC:
        case CAT_FRACTION:
        case CAT_NUMBER:        nVal=SVX_NUMVAL_STANDARD; break;

        case CAT_PERCENT:       nVal=SVX_NUMVAL_PERCENT; break;

        case CAT_ALL:           nVal=SVX_NUMVAL_STANDARD; break;

        case CAT_TIME:          nVal=SVX_NUMVAL_TIME; break;
        case CAT_DATE:          nVal=SVX_NUMVAL_DATE; break;

        case CAT_BOOLEAN:       nVal=SVX_NUMVAL_BOOLEAN; break;

        case CAT_USERDEFINED:
        case CAT_TEXT:
        default:                nVal=0;break;
    }

    OUString aPreviewString;
    pNumFmtShell->MakePrevStringFromVal( rFormatStr, aPreviewString, rpPreviewColor, nVal );
    return aPreviewString;
}

void SvxNumberFormatTabPage::MakePreviewText( const OUString& rFormat )
{
    OUString aPreviewString;
    Color* pPreviewColor = NULL;
    pNumFmtShell->MakePreviewString( rFormat, aPreviewString, pPreviewColor );
    m_pWndPreview->NotifyChange( aPreviewString, pPreviewColor );
}

void SvxNumberFormatTabPage::ChangePreviewText( sal_uInt16 nPos )
{
    OUString aPreviewString;
    Color* pPreviewColor = NULL;
    pNumFmtShell->FormatChanged( nPos, aPreviewString, pPreviewColor );
    m_pWndPreview->NotifyChange( aPreviewString, pPreviewColor );
}

bool SvxNumberFormatTabPage::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType()==EVENT_LOSEFOCUS)
    {
        if ( rNEvt.GetWindow() == dynamic_cast< Window* >( m_pEdComment ) && !m_pEdComment->IsVisible() )
        {
            pLastActivWindow = NULL;
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

void SvxNumberFormatTabPage::SetOkHdl( const Link& rOkHandler )
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
void SvxNumberFormatTabPage::AddAutomaticLanguage_Impl(LanguageType eAutoLang, sal_Bool bSelect)
{
    m_pLbLanguage->RemoveLanguage(LANGUAGE_SYSTEM);
    sal_uInt16 nPos = m_pLbLanguage->InsertEntry(sAutomaticEntry);
    m_pLbLanguage->SetEntryData(nPos, (void*)(sal_uLong)eAutoLang);
    if(bSelect)
        m_pLbLanguage->SelectEntryPos(nPos);
}

void SvxNumberFormatTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pNumberInfoItem,SvxNumberInfoItem,SID_ATTR_NUMBERFORMAT_INFO,false);
    SFX_ITEMSET_ARG (&aSet,pLinkItem,SfxLinkItem,SID_LINK_TYPE,false);
    if (pNumberInfoItem)
        SetNumberFormatList(*pNumberInfoItem);
    if (pLinkItem)
        SetOkHdl(pLinkItem->GetValue());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
