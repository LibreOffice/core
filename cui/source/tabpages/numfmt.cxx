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
#include <vcl/svapp.hxx>
#include <unotools/localedatawrapper.hxx>
#include <i18npool/lang.h>
#include <svx/dialogs.hrc>
#include <svtools/colorcfg.hxx>

#define _SVX_NUMFMT_CXX

#include <cuires.hrc>
#include "numfmt.hrc"

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

// static ----------------------------------------------------------------

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
#*  Method:        SvxNumberPreviewImpl
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberPreview
#*  Function:   Constructor of the class SvxNumberPreviewImpl
#*  Input:      Window, Resource-ID
#*  Output:     ---
#*
#************************************************************************/

SvxNumberPreviewImpl::SvxNumberPreviewImpl( Window* pParent, const ResId& rResId ) :

    Window( pParent, rResId ),
    mnPos( STRING_NOTFOUND ),
    mnChar( 0x0 )
{
    Font aFont( GetFont() );
    aFont.SetTransparent( sal_True );
    aFont.SetColor( Application::GetSettings().GetStyleSettings().GetFieldColor() );
    SetFont( aFont );
    InitSettings( sal_True, sal_True );
    SetBorderStyle( WINDOW_BORDER_MONO );
}

/*************************************************************************
#*  Method:        SvxNumberPreviewImpl
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberPreview
#*  Function:   Destructor of the class SvxNumberPreviewImpl
#*  Input:      ---
#*  Output:     ---
#*
#************************************************************************/

SvxNumberPreviewImpl::~SvxNumberPreviewImpl()
{
}

/*************************************************************************
#*  Method:        NotifyChange
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberPreviewImpl
#*  Function:   Function for changing the preview string
#*  Input:      String, color
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberPreviewImpl::NotifyChange( const String& rPrevStr,
                                         const Color* pColor )
{
    // detect and strip out '*' related placeholders
    aPrevStr = rPrevStr;
    mnPos = aPrevStr.Search( 0x1B );
    if ( mnPos != STRING_NOTFOUND )
    {
        mnChar = aPrevStr.GetChar( mnPos + 1 );
        // delete placeholder and char to repeat
        aPrevStr.Erase( mnPos, 2 );
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
#*  Class:      SvxNumberPreviewImpl
#*  Function:   Function for repainting the window.
#*  Input:      ---
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberPreviewImpl::Paint( const Rectangle& )
{
    Font    aDrawFont   = GetFont();
    Size    aSzWnd      = GetOutputSizePixel();
    String aTmpStr( aPrevStr );
    long    nLeadSpace = (aSzWnd.Width()  - GetTextWidth( aTmpStr )) /2;

    aDrawFont.SetColor( aPrevCol );
    SetFont( aDrawFont );

    if ( mnPos != STRING_NOTFOUND )
    {
        long nCharWidth = GetTextWidth( rtl::OUString::valueOf( mnChar ) );

        int nNumCharsToInsert = 0;
        if (nCharWidth > 0) nNumCharsToInsert = nLeadSpace / nCharWidth;

        if ( nNumCharsToInsert > 0)
        {
            for ( int i = 0; i < nNumCharsToInsert; ++i )
                aTmpStr.Insert( mnChar, mnPos );
        }
    }
    Point   aPosText    = Point( ( mnPos != STRING_NOTFOUND ) ? 0 : nLeadSpace,
                                 (aSzWnd.Height() - GetTextHeight())/2 );
    DrawText( aPosText, aTmpStr );
}

// -----------------------------------------------------------------------

void SvxNumberPreviewImpl::InitSettings( sal_Bool bForeground, sal_Bool bBackground )
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

// -----------------------------------------------------------------------

void SvxNumberPreviewImpl::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
        InitSettings( sal_True, sal_False );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        InitSettings( sal_False, sal_True );

    Window::StateChanged( nType );
}

// -----------------------------------------------------------------------

void SvxNumberPreviewImpl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings( sal_True, sal_True );
}

// class SvxNumberFormatTabPage ------------------------------------------

#define REMOVE_DONTKNOW() \
    if ( !aFtLanguage.IsEnabled() )                                     \
    {                                                                   \
        aFtLanguage .Enable();                                          \
        aLbLanguage .Enable();                                          \
        aLbLanguage .SelectLanguage( pNumFmtShell->GetCurLanguage() );  \
    }

#define HDL(hdl) LINK( this, SvxNumberFormatTabPage, hdl )

SvxNumberFormatTabPage::SvxNumberFormatTabPage( Window*             pParent,
                                                const SfxItemSet&   rCoreAttrs )

    :   SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_NUMBERFORMAT ), rCoreAttrs ),

        aFtCategory     ( this, CUI_RES( FT_CATEGORY ) ),
        aLbCategory     ( this, CUI_RES( LB_CATEGORY ) ),
        aFtFormat       ( this, CUI_RES( FT_FORMAT ) ),
        aLbCurrency     ( this, CUI_RES( LB_CURRENCY) ),
        aLbFormat       ( this, CUI_RES( LB_FORMAT ) ),
        aFtLanguage     ( this, CUI_RES( FT_LANGUAGE ) ),
        aLbLanguage     ( this, CUI_RES( LB_LANGUAGE ), sal_False ),
        aCbSourceFormat ( this, CUI_RES( CB_SOURCEFORMAT ) ),
        aWndPreview     ( this, CUI_RES( WND_NUMBER_PREVIEW ) ),
        aFlOptions      ( this, CUI_RES( FL_OPTIONS ) ),
        aFtDecimals     ( this, CUI_RES( FT_DECIMALS ) ),
        aEdDecimals     ( this, CUI_RES( ED_DECIMALS ) ),
        aBtnNegRed      ( this, CUI_RES( BTN_NEGRED ) ),
        aFtLeadZeroes   ( this, CUI_RES( FT_LEADZEROES ) ),
        aEdLeadZeroes   ( this, CUI_RES( ED_LEADZEROES ) ),
        aBtnThousand    ( this, CUI_RES( BTN_THOUSAND ) ),

        aFtEdFormat     ( this, CUI_RES( FT_EDFORMAT ) ),
        aEdFormat       ( this, CUI_RES( ED_FORMAT ) ),
        aIbAdd          ( this, CUI_RES( IB_ADD       ) ),
        aIbInfo         ( this, CUI_RES( IB_INFO      ) ),
        aIbRemove       ( this, CUI_RES( IB_REMOVE    ) ),
        aFtComment      ( this, CUI_RES( FT_COMMENT ) ),
        aEdComment      ( this, CUI_RES( ED_COMMENT ) ),

        pNumItem        ( NULL ),
        pNumFmtShell    ( NULL ),
        nInitFormat     ( ULONG_MAX ),

        sAutomaticEntry ( CUI_RES( STR_AUTO_ENTRY)),
        pLastActivWindow( NULL )
{
    Init_Impl();
    SetExchangeSupport(); // this page needs ExchangeSupport
    FreeResource();
    nFixedCategory=-1;
}

SvxNumberFormatTabPage::~SvxNumberFormatTabPage()
{
    delete pNumFmtShell;
    delete pNumItem;
}

void SvxNumberFormatTabPage::Init_Impl()
{
    ImageList               aIconList( CUI_RES ( IL_ICON ) );

    bNumItemFlag=sal_True;
    bOneAreaFlag=sal_False;

    nCatHeight=aLbCategory.GetSizePixel().Height();

    nCurFormatY     =aLbFormat.GetPosPixel().Y();
    nCurFormatHeight=aLbFormat.GetSizePixel().Height();
    nStdFormatY     =aLbCurrency.GetPosPixel().Y();
    nStdFormatHeight=nCurFormatY-nStdFormatY+nCurFormatHeight;

    aIbAdd.     SetModeImage( aIconList.GetImage( IID_ADD ) );
    aIbRemove.  SetModeImage( aIconList.GetImage( IID_REMOVE ) );
    aIbInfo.    SetModeImage( aIconList.GetImage( IID_INFO ) );

    aIbAdd.Enable(sal_False );
    aIbRemove.Enable(sal_False );
    aIbInfo.Enable(sal_False );

    aEdComment.SetText(aLbCategory.GetEntry(1));    // string for user defined

    aEdComment.Hide();

    aCbSourceFormat.Check( sal_False );
    aCbSourceFormat.Disable();
    aCbSourceFormat.Hide();

    Link aLink = LINK( this, SvxNumberFormatTabPage, SelFormatHdl_Impl );

    aLbCategory     .SetSelectHdl( aLink );
    aLbFormat       .SetSelectHdl( aLink );
    aLbLanguage     .SetSelectHdl( aLink );
    aLbCurrency     .SetSelectHdl( aLink );
    aCbSourceFormat .SetClickHdl( aLink );

    aLink = LINK( this, SvxNumberFormatTabPage, OptHdl_Impl );

    aEdDecimals     .SetModifyHdl( aLink );
    aEdLeadZeroes   .SetModifyHdl( aLink );
    aBtnNegRed      .SetClickHdl( aLink );
    aBtnThousand    .SetClickHdl( aLink );
    aLbFormat       .SetDoubleClickHdl( HDL( DoubleClickHdl_Impl ) );
    aEdFormat       .SetModifyHdl( HDL( EditHdl_Impl ) );
    aIbAdd.SetClickHdl( HDL( ClickHdl_Impl ) );
    aIbRemove.SetClickHdl( HDL( ClickHdl_Impl ) );
    aIbInfo.SetClickHdl( HDL( ClickHdl_Impl ) );

    aLink = LINK( this, SvxNumberFormatTabPage, LostFocusHdl_Impl);

    aEdComment      .SetLoseFocusHdl( aLink);
    aResetWinTimer  .SetTimeoutHdl(LINK( this, SvxNumberFormatTabPage, TimeHdl_Impl));
    aResetWinTimer  .SetTimeout( 10);

    // initialize language ListBox

    aLbLanguage.InsertLanguage( LANGUAGE_SYSTEM );
    // Don't list ambiguous locales where we won't be able to convert the
    // LanguageType back to an identical Language_Country name and therefore
    // couldn't load the i18n LocaleData. Show DebugMsg in non-PRODUCT version.
    ::com::sun::star::uno::Sequence< sal_uInt16 > xLang =
        LocaleDataWrapper::getInstalledLanguageTypes();
    sal_Int32 nCount = xLang.getLength();
    for ( sal_Int32 i=0; i<nCount; i++ )
    {
        aLbLanguage.InsertLanguage( xLang[i] );
    }

    aIbAdd.SetAccessibleRelationLabeledBy( &aFtEdFormat );
    aIbInfo.SetAccessibleRelationLabeledBy( &aFtEdFormat );
    aIbRemove.SetAccessibleRelationLabeledBy( &aFtEdFormat );
    aIbAdd.SetAccessibleRelationMemberOf( &aIbAdd );
    aIbInfo.SetAccessibleRelationMemberOf( &aIbInfo );
    aIbRemove.SetAccessibleRelationMemberOf( &aIbRemove );
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
    std::vector<String*>        aFmtEntryList;
    SvxNumberValueType          eValType        = SVX_VALUE_TYPE_UNDEFINED;
    double                      nValDouble      = 0;
    String                      aValString;
    SfxItemState                eState          = SFX_ITEM_DONTCARE;


    eState = rSet.GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_NOLANGUAGE ),sal_True,&pItem);

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

    eState = rSet.GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_INFO ),sal_True,&pItem);

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
            aCbSourceFormat.Check( pBoolItem->GetValue() );
        else
            aCbSourceFormat.Check( sal_False );
        aCbSourceFormat.Enable();
        aCbSourceFormat.Show();
    }
    else
    {
        sal_Bool bInit = sal_False;     // set to sal_True for debug test
        aCbSourceFormat.Check( bInit );
        aCbSourceFormat.Enable( bInit );
        aCbSourceFormat.Show( bInit );
    }

    // pNumItem must have been set from outside!
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
            //  #50441# string may be set in addition to the value
            aValString = pNumItem->GetValueString();
            nValDouble = pNumItem->GetValueDouble();
            break;
        case SVX_VALUE_TYPE_UNDEFINED:
        default:
            break;
    }

    if ( pNumFmtShell )
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
            bUseStarFormat = xSI->supportsService( rtl::OUString( "com.sun.star.sheet.SpreadsheetDocument" ) );
    }
    pNumFmtShell->SetUseStarFormat( bUseStarFormat );

    FillCurrencyBox();

    String aPrevString;
    Color* pDummy = NULL;
    pNumFmtShell->GetInitSettings( nCatLbSelPos, eLangType, nFmtLbSelPos,
                                   aFmtEntryList, aPrevString, pDummy );

    aLbCurrency.SelectEntryPos((sal_uInt16)pNumFmtShell->GetCurrencySymbol());

    nFixedCategory=nCatLbSelPos;
    if(bOneAreaFlag)
    {
        String sFixedCategory=aLbCategory.GetEntry(nFixedCategory);
        aLbCategory.Clear();
        aLbCategory.InsertEntry(sFixedCategory);
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
    // no_NO is an alias for nb_NO and normally isn't listed, we need it for
    // backwards compatibility, but only if the format passed is of
    // LanguageType no_NO.
    if ( eLangType == LANGUAGE_NORWEGIAN )
    {
        aLbLanguage.RemoveLanguage( eLangType );    // in case we're already called
        aLbLanguage.InsertLanguage( eLangType );
    }
    aLbLanguage.SelectLanguage( eLangType );
    if(pAutoEntryAttr)
        AddAutomaticLanguage_Impl(eLangType, pAutoEntryAttr->GetValue());
    UpdateFormatListBox_Impl(sal_False,sal_True);

//! This spoils everything because it rematches currency formats based on
//! the selected aLbCurrency entry instead of the current format.
//! Besides that everything seems to be initialized by now, so why call it?
//  SelFormatHdl_Impl( &aLbCategory );

    if ( pValFmtAttr )
    {
        EditHdl_Impl( &aEdFormat ); // UpdateOptions_Impl() als Seiteneffekt
    }
    else    // DONT_KNOW
    {
        // everything disabled except direct input or changing the category
        Obstructing();
    }

    if ( aCbSourceFormat.IsChecked() )
    {
        // everything disabled except SourceFormat checkbox
        EnableBySourceFormat_Impl();
    }

    DeleteEntryList_Impl(aFmtEntryList);
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
    aLbFormat       .SetNoSelection();
    aLbLanguage     .SetNoSelection();
    aFtLanguage     .Disable();
    aLbLanguage     .Disable();

    aIbAdd.Enable(sal_False );
    aIbRemove.Enable(sal_False );
    aIbInfo.Enable(sal_False );

    aBtnNegRed      .Disable();
    aBtnThousand    .Disable();
    aFtLeadZeroes   .Disable();
    aFtDecimals     .Disable();
    aEdLeadZeroes   .Disable();
    aEdDecimals     .Disable();
    aFlOptions      .Disable();
    aEdDecimals     .SetText( String() );
    aEdLeadZeroes   .SetText( String() );
    aBtnNegRed      .Check( sal_False );
    aBtnThousand    .Check( sal_False );
    aWndPreview     .NotifyChange( String() );

    aLbCategory     .SelectEntryPos( 0 );
    aEdFormat       .SetText( String() );
    aFtComment      .SetText( String() );
    aEdComment      .SetText(aLbCategory.GetEntry(1));  // string for user defined

    aEdFormat       .GrabFocus();
}


/*************************************************************************
#* Enable/Disable dialog parts depending on the value of the SourceFormat
#* checkbox.
#************************************************************************/
void SvxNumberFormatTabPage::EnableBySourceFormat_Impl()
{
    sal_Bool bEnable = !aCbSourceFormat.IsChecked();
    if ( !bEnable )
        aCbSourceFormat.GrabFocus();
    aFtCategory     .Enable( bEnable );
    aLbCategory     .Enable( bEnable );
    aFtFormat       .Enable( bEnable );
    aLbCurrency     .Enable( bEnable );
    aLbFormat       .Enable( bEnable );
    aFtLanguage     .Enable( bEnable );
    aLbLanguage     .Enable( bEnable );
    aFtDecimals     .Enable( bEnable );
    aEdDecimals     .Enable( bEnable );
    aFtLeadZeroes   .Enable( bEnable );
    aEdLeadZeroes   .Enable( bEnable );
    aBtnNegRed      .Enable( bEnable );
    aBtnThousand    .Enable( bEnable );
    aFlOptions      .Enable( bEnable );
    aFtEdFormat     .Enable( bEnable );
    aEdFormat       .Enable( bEnable );
    aIbAdd          .Enable( bEnable );
    aIbRemove       .Enable( bEnable );
    aIbInfo         .Enable( bEnable );
    aFtComment      .Enable( bEnable );
    aEdComment      .Enable( bEnable );
    aLbFormat.Invalidate(); // #i43322#
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
    Size aSize=aLbCategory.GetSizePixel();

    if(nFlag)
    {
        aSize.Height()=aLbFormat.GetSizePixel().Height();
    }
    else
    {
        aSize.Height()=nCatHeight;
    }

    aLbCategory.SetSizePixel(aSize);

    aFtLanguage.Show(!nFlag);
    aLbLanguage.Show(!nFlag);
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
    sal_Bool bDataChanged   = aFtLanguage.IsEnabled() || aCbSourceFormat.IsEnabled();
    if ( bDataChanged )
    {
        const SfxItemSet& rMyItemSet = GetItemSet();
        sal_uInt16          nWhich       = GetWhich( SID_ATTR_NUMBERFORMAT_VALUE );
        SfxItemState    eItemState   = rMyItemSet.GetItemState( nWhich, sal_False );

        // OK chosen - Is format code input entered already taken over?
        // If not, simulate Add. Upon syntax error ignore input and prevent Put.
        String      aFormat = aEdFormat.GetText();
        sal_uInt32 nCurKey = pNumFmtShell->GetCurNumFmtKey();

        if ( aIbAdd.IsEnabled() || pNumFmtShell->IsTmpCurrencyFormat(aFormat) )
        {   // #79599# It is not sufficient to just add the format code (or
            // delete it in case of bOneAreaFlag and resulting category change).
            // Upon switching tab pages we need all settings to be consistent
            // in case this page will be redisplayed later.
            bDataChanged = (ClickHdl_Impl( &aIbAdd ) != 0);
            nCurKey = pNumFmtShell->GetCurNumFmtKey();
        }
        else if(nCurKey == NUMKEY_UNDEFINED)
        {   // something went wrong, e.g. in Writer #70281#
            pNumFmtShell->FindEntry(aFormat, &nCurKey);
        }

        //---------------------------------------------------------------
        // Chosen format:
        // --------------
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

        // --------------------------------------------------------------
        // List of changed user defined formats:
        // -------------------------------------
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

        //---------------------------------------------------------------
        // Whether source format is to be taken or not:
        // --------------------------------------------
        if ( aCbSourceFormat.IsEnabled() )
        {
            sal_uInt16 _nWhich = GetWhich( SID_ATTR_NUMBERFORMAT_SOURCE );
            SfxItemState _eItemState = rMyItemSet.GetItemState( _nWhich, sal_False );
            const SfxBoolItem* pBoolItem = (const SfxBoolItem*)
                        GetItem( rMyItemSet, SID_ATTR_NUMBERFORMAT_SOURCE );
            sal_Bool bOld = (pBoolItem ? pBoolItem->GetValue() : sal_False);
            rCoreAttrs.Put( SfxBoolItem( _nWhich, aCbSourceFormat.IsChecked() ) );
            if ( !bDataChanged )
                bDataChanged = (bOld != (sal_Bool) aCbSourceFormat.IsChecked() ||
                    _eItemState != SFX_ITEM_SET);
        }

        // FillItemSet is only called on OK, here we can notify the
        // NumberFormatShell that all new user defined formats are valid.
        pNumFmtShell->ValidateNewEntries();
        if(aLbLanguage.IsVisible() &&
                LISTBOX_ENTRY_NOTFOUND != aLbLanguage.GetEntryPos(sAutomaticEntry))
                rCoreAttrs.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ADD_AUTO,
                    aLbLanguage.GetSelectEntry() == sAutomaticEntry));
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

void SvxNumberFormatTabPage::FillFormatListBox_Impl( std::vector<String*>& rEntries )
{
    String*     pEntry;
    String      aTmpString;
    String      aTmpCatString;
    Font        aFont=aLbCategory.GetFont();
    size_t      i = 0;
    short       nTmpCatPos;
    short       aPrivCat;

    aLbFormat.Clear();
    aLbFormat.SetUpdateMode( sal_False );

    if( rEntries.empty() )
        return;

    if(bOneAreaFlag)
    {
        nTmpCatPos=nFixedCategory;
    }
    else
    {
        nTmpCatPos=aLbCategory.GetSelectEntryPos();
    }

    switch (nTmpCatPos)
    {
        case CAT_ALL:
        case CAT_TEXT:
        case CAT_NUMBER:        i=1;
                                pEntry=rEntries[0];
                                if(pEntry!=NULL)
                                {
                                    if (nTmpCatPos == CAT_TEXT)
                                        aTmpString=*pEntry;
                                    else
                                        aTmpString = pNumFmtShell->GetStandardName();
                                    aPrivCat=pNumFmtShell->GetCategory4Entry(0);
                                    aLbFormat.InsertFontEntry( aTmpString, aFont );
                                }
                                break;

        default:                break;
    }

    if(pNumFmtShell!=NULL)
    {
        for ( ; i < rEntries.size(); ++i )
        {
            pEntry = rEntries[i];
            aPrivCat=pNumFmtShell->GetCategory4Entry( static_cast<sal_uInt16>(i) );
            if(aPrivCat!=CAT_TEXT)
            {
                Color* pPreviewColor = NULL;
                String aPreviewString( GetExpColorString( pPreviewColor, *pEntry, aPrivCat ) );
                Font aEntryFont( aLbFormat.GetFont() );
                aLbFormat.InsertFontEntry( aPreviewString, aEntryFont, pPreviewColor );
            }
            else
            {
                aLbFormat.InsertFontEntry(*pEntry,aFont);
            }
        }
    }
    aLbFormat.SetUpdateMode( sal_True );
    DeleteEntryList_Impl(rEntries);
}


void SvxNumberFormatTabPage::DeleteEntryList_Impl( std::vector<String*>& rEntries )
{
    for( std::vector<String*>::const_iterator it(rEntries.begin()); it != rEntries.end(); ++it )
        delete *it;
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
    String  theFormat           = aEdFormat.GetText();
    sal_uInt16  nCurCategory        = aLbCategory.GetSelectEntryPos();
    sal_uInt16  nCategory           = nCurCategory;
    sal_uInt16  nDecimals           = 0;
    sal_uInt16  nZeroes             = 0;
    bool        bNegRed             = false;
    bool        bThousand           = false;
    sal_uInt16  nCurrencyPos        =aLbCurrency.GetSelectEntryPos();

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
            aLbCurrency.SelectEntryPos(nTstPos);
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
    else if ( aLbFormat.GetEntryCount() > 0 )
    {
        sal_uInt32 nCurEntryKey=NUMKEY_UNDEFINED;
        if(!pNumFmtShell->FindEntry( aEdFormat.GetText(),&nCurEntryKey))
        {
            aLbFormat.SetNoSelection();
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
            aFlOptions.Enable();
            aFtDecimals.Enable();
            aEdDecimals.Enable();
            aFtLeadZeroes.Enable();
            aEdLeadZeroes.Enable();
            aBtnNegRed.Enable();
            aBtnThousand.Enable();
            aEdDecimals  .SetText( UniString::CreateFromInt32( nDecimals ) );
            aEdLeadZeroes.SetText( UniString::CreateFromInt32( nZeroes ) );
            aBtnNegRed   .Check( bNegRed );
            aBtnThousand .Check( bThousand );
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
            aFlOptions      .Disable();
            aFtDecimals     .Disable();
            aEdDecimals     .Disable();
            aFtLeadZeroes   .Disable();
            aEdLeadZeroes   .Disable();
            aBtnNegRed      .Disable();
            aBtnThousand    .Disable();
            aEdDecimals     .SetText( UniString::CreateFromInt32( 0 ) );
            aEdLeadZeroes   .SetText( UniString::CreateFromInt32( 0 ) );
            aBtnNegRed      .Check( sal_False );
            aBtnThousand    .Check( sal_False );
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
        sal_uInt16 bCat,        // Category or country/language ListBox?
        sal_Bool   bUpdateEdit
    )
{
    std::vector<String*> aEntryList;
    short                nFmtLbSelPos = 0;
    short                nTmpCatPos;

    if(bOneAreaFlag)
    {
        nTmpCatPos=nFixedCategory;
    }
    else
    {
        nTmpCatPos=aLbCategory.GetSelectEntryPos();
    }


    if ( bCat )
    {
        Point aPos=aLbFormat.GetPosPixel();
        Size  aSize=aLbFormat.GetSizePixel();

        if(nTmpCatPos!=CAT_CURRENCY)
        {
            aPos.Y()=nStdFormatY;
            aSize.Height()=nStdFormatHeight;
            aLbFormat.SetPosSizePixel(aPos,aSize);
            aLbCurrency.Hide();
        }
        else
        {
            aPos.Y()=nCurFormatY;
            aSize.Height()=nCurFormatHeight;
            aLbFormat.SetPosSizePixel(aPos,aSize);
            aLbCurrency.Show();
        }

        pNumFmtShell->CategoryChanged( nTmpCatPos,nFmtLbSelPos, aEntryList );
    }
    else
        pNumFmtShell->LanguageChanged( aLbLanguage.GetSelectLanguage(),
                                       nFmtLbSelPos,aEntryList );

    REMOVE_DONTKNOW() // possibly UI-Enable


    if ( (!aEntryList.empty()) && (nFmtLbSelPos != SELPOS_NONE) )
    {
        if(bUpdateEdit)
        {
            String aFormat=*aEntryList[nFmtLbSelPos];
            aEdFormat.SetText(aFormat);
            aFtComment.SetText(pNumFmtShell->GetComment4Entry(nFmtLbSelPos));
        }

        if(!bOneAreaFlag || !bCat)
        {
            FillFormatListBox_Impl( aEntryList );
            aLbFormat.SelectEntryPos( nFmtLbSelPos );

            aFtComment.SetText(pNumFmtShell->GetComment4Entry(nFmtLbSelPos));
            if(pNumFmtShell->GetUserDefined4Entry(nFmtLbSelPos))
            {
                if(pNumFmtShell->GetComment4Entry(nFmtLbSelPos).Len()==0)
                {
                    aFtComment.SetText(aLbCategory.GetEntry(1));
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
            aLbFormat.SelectEntryPos( (sal_uInt16)nFmtLbSelPos );

            aFtComment.SetText(pNumFmtShell->GetComment4Entry(nFmtLbSelPos));
            if(pNumFmtShell->GetUserDefined4Entry(nFmtLbSelPos))
            {
                if(pNumFmtShell->GetComment4Entry(nFmtLbSelPos).Len()==0)
                {
                    aFtComment.SetText(aLbCategory.GetEntry(1));
                }
            }
        }
        else
        {
            aLbFormat.SetNoSelection();
        }

        if ( bUpdateEdit )
        {
            aEdFormat.SetText( String() );
            aWndPreview.NotifyChange( String() );
        }
    }

    DeleteEntryList_Impl( aEntryList );
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
    if ( pLb == &aLbFormat )
    {
        SelFormatHdl_Impl( pLb );

        if ( fnOkHdl.IsSet() )
        {   // temporary solution, should be offered by SfxTabPage
            fnOkHdl.Call( NULL );
        }
        else
        {
            SfxSingleTabDialog* pParent = dynamic_cast< SfxSingleTabDialog* >( GetParent() );
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
    if ( (CheckBox*)pLb == &aCbSourceFormat )
    {
        EnableBySourceFormat_Impl();    // enable/disable everything else
        if ( aCbSourceFormat.IsChecked() )
            return 0;   // just disabled everything else

        // Reinit options enable/disable for current selection.

        // Current category may be UserDefined with no format entries defined.
        // And yes, aLbFormat is a SvxFontListBox with sal_uLong list positions,
        // implementation returns a LIST_APPEND if empty, comparison with
        // sal_uInt16 LISTBOX_ENTRY_NOTFOUND wouldn't match.
        if ( aLbFormat.GetSelectEntryPos() == LIST_APPEND )
            pLb = &aLbCategory; // continue with the current category selected
        else
            pLb = &aLbFormat;   // continue with the current format selected
    }

    short       nTmpCatPos;

    if(bOneAreaFlag)
    {
        nTmpCatPos=nFixedCategory;
    }
    else
    {
        nTmpCatPos=aLbCategory.GetSelectEntryPos();
    }

    sal_uInt16 nCurrencyPos=LISTBOX_ENTRY_NOTFOUND ;

    if(nTmpCatPos==CAT_CURRENCY && (ListBox *)pLb == &aLbCurrency )
    {
        nCurrencyPos=aLbCurrency.GetSelectEntryPos();
        pNumFmtShell->SetCurrencySymbol(nCurrencyPos);
    }

    //--------------------------------------------------------------------
    // Format-ListBox ----------------------------------------------------
    if ( (SvxFontListBox *)pLb == &aLbFormat )
    {
        sal_uInt16  nSelPos = (sal_uInt16) aLbFormat.GetSelectEntryPos();
        String  aFormat = aLbFormat.GetSelectEntry();
        String  aComment;

        short       nFmtLbSelPos = nSelPos;

        aFormat=pNumFmtShell->GetFormat4Entry(nSelPos);
        aComment=pNumFmtShell->GetComment4Entry(nSelPos);
        if(pNumFmtShell->GetUserDefined4Entry(nFmtLbSelPos))
        {
            if(pNumFmtShell->GetComment4Entry(nFmtLbSelPos).Len()==0)
            {
                aComment=aLbCategory.GetEntry(1);
            }
        }

        if ( aFormat.Len() > 0 )
        {
            if(!aEdFormat.HasFocus()) aEdFormat.SetText( aFormat );
            aFtComment.SetText(aComment);
            ChangePreviewText( nSelPos );
        }

        REMOVE_DONTKNOW() // possibly UI-Enable

        if ( pNumFmtShell->FindEntry( aFormat) )
        {
            aIbAdd.Enable(sal_False );
            sal_Bool bIsUserDef=pNumFmtShell->IsUserDefined( aFormat );
            aIbRemove.Enable(bIsUserDef);
            aIbInfo.Enable(bIsUserDef);

        }
        else
        {
            aIbAdd.Enable(sal_True );
            aIbInfo.Enable(sal_True );
            aIbRemove.Enable(sal_False );
            aFtComment.SetText(aEdComment.GetText());

        }
        UpdateOptions_Impl( sal_False );

        return 0;
    }

    //--------------------------------------------------------------------
    // category-ListBox -------------------------------------------------
    if ( pLb == &aLbCategory || pLb == &aLbCurrency)
    {
        UpdateFormatListBox_Impl( sal_True, sal_True );
        EditHdl_Impl( NULL );
        UpdateOptions_Impl( sal_False );

        return 0;
    }

    //--------------------------------------------------------------------
    // language/country-ListBox ----------------------------------------------
    if ( pLb == &aLbLanguage )
    {
        UpdateFormatListBox_Impl( sal_False, sal_True );
        EditHdl_Impl( &aEdFormat );

        return 0;
    }
    return 0;
}


/*************************************************************************
#*  Method:    ClickHdl_Impl, ImageButton* pIB
#*------------------------------------------------------------------------
#*
#*  Class:      SvxNumberFormatTabPage
#*  Function:   Called when the add or delete button is pushed,
#*              adjusts the number format list.
#*  Input:      Toolbox- Button
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( SvxNumberFormatTabPage, ClickHdl_Impl, ImageButton*, pIB)
{
    sal_Bool        bAdded = sal_False;
    sal_Bool        bDeleted = sal_False;
    sal_uLong       nReturn = 0;
    const sal_uLong nReturnChanged  = 0x1;  // THE boolean return value
    const sal_uLong nReturnAdded    = 0x2;  // temp: format added
    const sal_uLong nReturnOneArea  = 0x4;  // temp: one area but category changed => ignored

    if(pIB==&aIbAdd)
    {   // Also called from FillItemSet() if a temporary currency format has
        // to be added, not only if the Add button is enabled.
        String               aFormat = aEdFormat.GetText();
        std::vector<String*> aEntryList;
        std::vector<String*> a2EntryList;
        sal_uInt16           nCatLbSelPos = 0;
        short                nFmtLbSelPos = SELPOS_NONE;
        xub_StrLen           nErrPos=0;

        pNumFmtShell->SetCurCurrencyEntry(NULL);
        bAdded = pNumFmtShell->AddFormat( aFormat, nErrPos,
                                          nCatLbSelPos, nFmtLbSelPos,
                                          aEntryList);
        if ( bAdded )
            nReturn |= nReturnChanged | nReturnAdded;

        if(pLastActivWindow== (Window *) &aEdComment)
        {
            aEdFormat.GrabFocus();
            aEdComment.Hide();
            aFtComment.Show();
            aFtComment.SetText(aEdComment.GetText());
        }

        if ( !nErrPos ) // Syntax ok?
        {
            // May be sorted under a different locale if LCID was parsed.
            if (bAdded)
                aLbLanguage.SelectLanguage( pNumFmtShell->GetCurLanguage() );

            if(nCatLbSelPos==CAT_CURRENCY)
            {
                aLbCurrency.SelectEntryPos((sal_uInt16)pNumFmtShell->GetCurrencySymbol());
            }

            if(bOneAreaFlag && (nFixedCategory!=nCatLbSelPos))
            {
                if(bAdded) DeleteEntryList_Impl(aEntryList);
                bDeleted = pNumFmtShell->RemoveFormat( aFormat,
                                               nCatLbSelPos,
                                               nFmtLbSelPos,
                                               a2EntryList);
                if(bDeleted) DeleteEntryList_Impl(a2EntryList);
                aEdFormat.GrabFocus();
                aEdFormat.SetSelection( Selection( (short)nErrPos, SELECTION_MAX ) );
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
                    if(aEdComment.GetText()!=aLbCategory.GetEntry(1))
                    {
                        pNumFmtShell->SetComment4Entry(nFmtLbSelPos,
                                                    aEdComment.GetText());
                    }
                    else
                    {
                        pNumFmtShell->SetComment4Entry(nFmtLbSelPos,
                                                        String());
                    }
                    aLbFormat.SelectEntryPos( (sal_uInt16)nFmtLbSelPos );
                    aEdFormat.SetText( aFormat );

                    aEdComment.SetText(aLbCategory.GetEntry(1));    // String for user defined

                    ChangePreviewText( (sal_uInt16)nFmtLbSelPos );
                }
            }
        }
        else // syntax error
        {
            aEdFormat.GrabFocus();
            aEdFormat.SetSelection( Selection( (short)nErrPos, SELECTION_MAX ) );
        }
        EditHdl_Impl( &aEdFormat );
        nReturn = ((nReturn & nReturnOneArea) ? 0 : (nReturn & nReturnChanged));

        DeleteEntryList_Impl( aEntryList );
        DeleteEntryList_Impl( a2EntryList );
    }
    else if(pIB==&aIbRemove)
    {
        String               aFormat = aEdFormat.GetText();
        std::vector<String*> aEntryList;
        sal_uInt16           nCatLbSelPos = 0;
        short                nFmtLbSelPos = SELPOS_NONE;

        bDeleted = pNumFmtShell->RemoveFormat( aFormat,
                                               nCatLbSelPos,
                                               nFmtLbSelPos,
                                               aEntryList );

        aEdComment.SetText(aLbCategory.GetEntry(1));
        if ( bDeleted )
        {
            if( nFmtLbSelPos>=0 && static_cast<size_t>(nFmtLbSelPos)<aEntryList.size() )
            {
                aFormat = *aEntryList[nFmtLbSelPos];
            }

            FillFormatListBox_Impl( aEntryList );

            if ( nFmtLbSelPos != SELPOS_NONE )
            {
                if(bOneAreaFlag)                  //@@ ???
                        SetCategory(0);
                    else
                        SetCategory(nCatLbSelPos );

                aLbFormat.SelectEntryPos( (sal_uInt16)nFmtLbSelPos );
                aEdFormat.SetText( aFormat );
                ChangePreviewText( (sal_uInt16)nFmtLbSelPos );
            }
            else
            {
                // set to "all/standard"
                SetCategory(0 );
                SelFormatHdl_Impl( &aLbCategory );
            }
        }
        EditHdl_Impl( &aEdFormat );

        DeleteEntryList_Impl( aEntryList );
    }
    else if(pIB==&aIbInfo)
    {
        if(!(pLastActivWindow== (Window *) &aEdComment))
        {
            aEdComment.SetText(aFtComment.GetText());
            aEdComment.Show();
            aFtComment.Hide();
            aEdComment.GrabFocus();
        }
        else
        {
            aEdFormat.GrabFocus();
            aEdComment.Hide();
            aFtComment.Show();
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

    if ( aEdFormat.GetText().Len() == 0 )
    {
        aIbAdd.Enable(sal_False );
        aIbRemove.Enable(sal_False );
        aIbInfo.Enable(sal_False );
        aFtComment.SetText(String());
    }
    else
    {
        String aFormat = aEdFormat.GetText();
        MakePreviewText( aFormat );

        if ( pNumFmtShell->FindEntry( aFormat, &nCurKey ) )
        {
            aIbAdd.Enable(sal_False );
            sal_Bool bUserDef=pNumFmtShell->IsUserDefined( aFormat );

            aIbRemove.Enable(bUserDef);
            aIbInfo.Enable(bUserDef);

            if(bUserDef)
            {
                sal_uInt16 nTmpCurPos=pNumFmtShell->FindCurrencyFormat(aFormat );

                if(nTmpCurPos!=(sal_uInt16)-1)
                    aLbCurrency.SelectEntryPos(nTmpCurPos);
            }
            short nPosi=pNumFmtShell->GetListPos4Entry(aFormat);
            if(nPosi>=0)
                aLbFormat.SelectEntryPos( (sal_uInt16)nPosi);

        }
        else
        {

            aIbAdd.Enable(sal_True );
            aIbInfo.Enable(sal_True);
            aIbRemove.Enable(sal_False );

            aFtComment.SetText(aEdComment.GetText());

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
    if (   ((NumericField*)pOptCtrl == &aEdLeadZeroes)
        || ((NumericField*)pOptCtrl == &aEdDecimals)
        || ((CheckBox*)    pOptCtrl == &aBtnNegRed)
        || ((CheckBox*)    pOptCtrl == &aBtnThousand) )
    {
        String        aFormat;
        sal_Bool          bThousand     =    aBtnThousand.IsEnabled()
                                      && aBtnThousand.IsChecked();
        sal_Bool          bNegRed       =    aBtnNegRed.IsEnabled()
                                      && aBtnNegRed.IsChecked();
        sal_uInt16        nPrecision    = (aEdDecimals.IsEnabled())
                                        ? (sal_uInt16)aEdDecimals.GetValue()
                                        : (sal_uInt16)0;
        sal_uInt16        nLeadZeroes   = (aEdLeadZeroes.IsEnabled())
                                        ? (sal_uInt16)aEdLeadZeroes.GetValue()
                                        : (sal_uInt16)0;

        pNumFmtShell->MakeFormat( aFormat,
                                  bThousand, bNegRed,
                                  nPrecision, nLeadZeroes,
                                  (sal_uInt16)aLbFormat.GetSelectEntryPos() );

        aEdFormat.SetText( aFormat );
        MakePreviewText( aFormat );

        if ( pNumFmtShell->FindEntry( aFormat ) )
        {
            aIbAdd.Enable(sal_False );
            sal_Bool bUserDef=pNumFmtShell->IsUserDefined( aFormat );
            aIbRemove.Enable(bUserDef);
            aIbInfo.Enable(bUserDef);
            EditHdl_Impl( &aEdFormat);

        }
        else
        {
            EditHdl_Impl( NULL );
            aLbFormat.SetNoSelection();
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
    if (pEd==&aEdComment)
    {
        aResetWinTimer.Start();
        aFtComment.SetText(aEdComment.GetText());
        aEdComment.Hide();
        aFtComment.Show();
        if(!aIbAdd.IsEnabled())
        {
            sal_uInt16  nSelPos = (sal_uInt16) aLbFormat.GetSelectEntryPos();
            pNumFmtShell->SetComment4Entry(nSelPos,
                                        aEdComment.GetText());
            aEdComment.SetText(aLbCategory.GetEntry(1));    // String for user defined
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

String SvxNumberFormatTabPage::GetExpColorString(
        Color*& rpPreviewColor, const String& rFormatStr, short nTmpCatPos)
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

    String aPreviewString;
    pNumFmtShell->MakePrevStringFromVal( rFormatStr, aPreviewString, rpPreviewColor, nVal );
    return aPreviewString;
}

void SvxNumberFormatTabPage::MakePreviewText( const String& rFormat )
{
    String aPreviewString;
    Color* pPreviewColor = NULL;
    pNumFmtShell->MakePreviewString( rFormat, aPreviewString, pPreviewColor );
    aWndPreview.NotifyChange( aPreviewString, pPreviewColor );
}

void SvxNumberFormatTabPage::ChangePreviewText( sal_uInt16 nPos )
{
    String aPreviewString;
    Color* pPreviewColor = NULL;
    pNumFmtShell->FormatChanged( nPos, aPreviewString, pPreviewColor );
    aWndPreview.NotifyChange( aPreviewString, pPreviewColor );
}

long SvxNumberFormatTabPage::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType()==EVENT_LOSEFOCUS)
    {
        if ( rNEvt.GetWindow() == dynamic_cast< Window* >( &aEdComment ) && !aEdComment.IsVisible() )
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
    std::vector<rtl::OUString> aList;

    sal_uInt16  nSelPos=0;
    pNumFmtShell->GetCurrencySymbols(aList, &nSelPos);

    for(std::vector<rtl::OUString>::iterator i = aList.begin() + 1;i != aList.end(); ++i)
        aLbCurrency.InsertEntry(*i);

    aLbCurrency.SelectEntryPos(nSelPos);
}

void SvxNumberFormatTabPage::SetCategory(sal_uInt16 nPos)
{
    sal_uInt16  nCurCategory = aLbCategory.GetSelectEntryPos();
    Point aPos=aLbFormat.GetPosPixel();
    Size  aSize=aLbFormat.GetSizePixel();
    sal_uInt16 nTmpCatPos;

    if(bOneAreaFlag)
    {
        nTmpCatPos=nFixedCategory;
    }
    else
    {
        nTmpCatPos=nPos;
    }

    if(aLbCategory.GetEntryCount()==1 || nCurCategory!=nPos)
    {
        if(nTmpCatPos!=CAT_CURRENCY)
        {
            aPos.Y()=nStdFormatY;
            aSize.Height()=nStdFormatHeight;
            aLbFormat.SetPosSizePixel(aPos,aSize);
            aLbCurrency.Hide();
        }
        else
        {
            aPos.Y()=nCurFormatY;
            aSize.Height()=nCurFormatHeight;
            aLbFormat.SetPosSizePixel(aPos,aSize);
            aLbCurrency.Show();
        }
    }
    aLbCategory.SelectEntryPos(nPos);
}
/* to support Writer text field language handling an
   additional entry needs to be inserted into the ListBox
   which marks a certain language as automatically detected
   Additionally the "Default" language is removed
*/
void SvxNumberFormatTabPage::AddAutomaticLanguage_Impl(LanguageType eAutoLang, sal_Bool bSelect)
{
    aLbLanguage.RemoveLanguage(LANGUAGE_SYSTEM);
    sal_uInt16 nPos = aLbLanguage.InsertEntry(sAutomaticEntry);
    aLbLanguage.SetEntryData(nPos, (void*)(sal_uLong)eAutoLang);
    if(bSelect)
        aLbLanguage.SelectEntryPos(nPos);
}

void SvxNumberFormatTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pNumberInfoItem,SvxNumberInfoItem,SID_ATTR_NUMBERFORMAT_INFO,sal_False);
    SFX_ITEMSET_ARG (&aSet,pLinkItem,SfxLinkItem,SID_LINK_TYPE,sal_False);
    if (pNumberInfoItem)
        SetNumberFormatList(*pNumberInfoItem);
    if (pLinkItem)
        SetOkHdl(pLinkItem->GetValue());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
