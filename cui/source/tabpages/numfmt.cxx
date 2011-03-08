/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// include ---------------------------------------------------------------
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>
#include <unotools/localedatawrapper.hxx>
#include <i18npool/lang.h>
#include <svx/dialogs.hrc>
#define _SVSTDARR_STRINGS
#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>
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

#define NUMKEY_UNDEFINED SAL_MAX_UINT32

// static ----------------------------------------------------------------

static USHORT pRanges[] =
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
#*  Methode:        SvxNumberPreviewImpl
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberPreview
#*  Funktion:   Konstruktor der Klasse SvxNumberPreviewImpl
#*  Input:      Fenster, Resource-ID
#*  Output:     ---
#*
#************************************************************************/

SvxNumberPreviewImpl::SvxNumberPreviewImpl( Window* pParent, const ResId& rResId ) :

    Window( pParent, rResId )

{
    Font aFont( GetFont() );
    aFont.SetTransparent( TRUE );
    aFont.SetColor( Application::GetSettings().GetStyleSettings().GetFieldColor() );
    SetFont( aFont );
    InitSettings( TRUE, TRUE );
    SetBorderStyle( WINDOW_BORDER_MONO );
}

/*************************************************************************
#*  Methode:        SvxNumberPreviewImpl
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberPreview
#*  Funktion:   Destruktor der Klasse SvxNumberPreviewImpl
#*  Input:      ---
#*  Output:     ---
#*
#************************************************************************/

SvxNumberPreviewImpl::~SvxNumberPreviewImpl()
{
}

/*************************************************************************
#*  Methode:        NotifyChange
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberPreviewImpl
#*  Funktion:   Funktion fuer das Aendern des Preview- Strings
#*  Input:      String, Farbe
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberPreviewImpl::NotifyChange( const String& rPrevStr,
                                         const Color* pColor )
{
    aPrevStr = rPrevStr;
    svtools::ColorConfig aColorConfig;
    Color aWindowTextColor( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
    aPrevCol = pColor ? *pColor : aWindowTextColor;
    Invalidate();
    Update();
}

/*************************************************************************
#*  Methode:        Paint
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberPreviewImpl
#*  Funktion:   Funktion fuer das neu zeichnen des Fensters.
#*  Input:      ---
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberPreviewImpl::Paint( const Rectangle& )
{
    Font    aDrawFont   = GetFont();
    Size    aSzWnd      = GetOutputSizePixel();
    Point   aPosText    = Point( (aSzWnd.Width()  - GetTextWidth( aPrevStr )) /2,
                                 (aSzWnd.Height() - GetTextHeight())/2 );

    aDrawFont.SetColor( aPrevCol );
    SetFont( aDrawFont );
    DrawText( aPosText, aPrevStr );
}

// -----------------------------------------------------------------------

void SvxNumberPreviewImpl::InitSettings( BOOL bForeground, BOOL bBackground )
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
        InitSettings( TRUE, FALSE );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        InitSettings( FALSE, TRUE );

    Window::StateChanged( nType );
}

// -----------------------------------------------------------------------

void SvxNumberPreviewImpl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings( TRUE, TRUE );
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

#include <layout/layout-pre.hxx>

#if ENABLE_LAYOUT
#undef CUI_RES
#define CUI_RES(x) #x
#define CUI_RES_PLAIN(x) ResId (x, CUI_MGR ())
#define THIS_CUI_RES(x) this, #x
#undef SfxTabPage
#define SfxTabPage( parent, id, args ) SfxTabPage( parent, "number-format.xml", id, &args )
#else /* !ENABLE_LAYOUT */
#define CUI_RES_PLAIN CUI_RES
#define THIS_CUI_RES CUI_RES
#endif /* !ENABLE_LAYOUT */

SvxNumberFormatTabPage::SvxNumberFormatTabPage( Window*             pParent,
                                                const SfxItemSet&   rCoreAttrs )

    :   SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_NUMBERFORMAT ), rCoreAttrs ),

        aFtCategory     ( this, CUI_RES( FT_CATEGORY ) ),
        aLbCategory     ( this, CUI_RES( LB_CATEGORY ) ),
        aFtFormat       ( this, CUI_RES( FT_FORMAT ) ),
        aLbCurrency     ( this, CUI_RES( LB_CURRENCY) ),
        aLbFormat       ( this, CUI_RES( LB_FORMAT ) ),
        aFtLanguage     ( this, CUI_RES( FT_LANGUAGE ) ),
        aLbLanguage     ( this, CUI_RES( LB_LANGUAGE ), FALSE ),
        aCbSourceFormat ( this, CUI_RES( CB_SOURCEFORMAT ) ),
        aFtDecimals     ( this, CUI_RES( FT_DECIMALS ) ),
        aEdDecimals     ( this, CUI_RES( ED_DECIMALS ) ),
        aFtLeadZeroes   ( this, CUI_RES( FT_LEADZEROES ) ),
        aEdLeadZeroes   ( this, CUI_RES( ED_LEADZEROES ) ),
        aBtnNegRed      ( this, CUI_RES( BTN_NEGRED ) ),
        aBtnThousand    ( this, CUI_RES( BTN_THOUSAND ) ),
        aFlOptions      ( this, CUI_RES( FL_OPTIONS ) ),

        aFtEdFormat     ( this, CUI_RES( FT_EDFORMAT ) ),
        aEdFormat       ( this, CUI_RES( ED_FORMAT ) ),
        aIbAdd          ( this, CUI_RES( IB_ADD       ) ),
        aIbInfo         ( this, CUI_RES( IB_INFO      ) ),
        aIbRemove       ( this, CUI_RES( IB_REMOVE    ) ),
        aFtComment      ( this, CUI_RES( FT_COMMENT ) ),
        aEdComment      ( this, CUI_RES( ED_COMMENT ) ),

#if ENABLE_LAYOUT
        aWndPreview     ( LAYOUT_THIS_WINDOW(this), CUI_RES_PLAIN( WND_NUMBER_PREVIEW ) ),
#else
        aWndPreview     ( this, CUI_RES_PLAIN( WND_NUMBER_PREVIEW ) ),
#endif
        pNumItem        ( NULL ),
        pNumFmtShell    ( NULL ),
        nInitFormat     ( ULONG_MAX ),

        sAutomaticEntry ( THIS_CUI_RES( STR_AUTO_ENTRY)),
        pLastActivWindow( NULL )
{
#if ENABLE_LAYOUT
    aLbFormat.Clear ();
#endif /* ENABLE_LAYOUT */

    Init_Impl();
    SetExchangeSupport(); // diese Page braucht ExchangeSupport
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
    ImageList               aIconList( CUI_RES_PLAIN ( IL_ICON ) );

    bNumItemFlag=TRUE;
    bOneAreaFlag=FALSE;

    nCatHeight=aLbCategory.GetSizePixel().Height();

    nCurFormatY     =aLbFormat.GetPosPixel().Y();
    nCurFormatHeight=aLbFormat.GetSizePixel().Height();
    nStdFormatY     =aLbCurrency.GetPosPixel().Y();
    nStdFormatHeight=nCurFormatY-nStdFormatY+nCurFormatHeight;

    aIbAdd.     SetModeImage( aIconList.GetImage( IID_ADD ) );
    aIbRemove.  SetModeImage( aIconList.GetImage( IID_REMOVE ) );
    aIbInfo.    SetModeImage( aIconList.GetImage( IID_INFO ) );

    aIbAdd.Enable(FALSE );
    aIbRemove.Enable(FALSE );
    aIbInfo.Enable(FALSE );

    aEdComment.SetText(aLbCategory.GetEntry(1));    //String fuer Benutzerdefiniert
                                                        //holen
    aEdComment.Hide();

    aCbSourceFormat.Check( FALSE );
    aCbSourceFormat.Disable();
    aCbSourceFormat.Hide();

// Handler verbinden
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

    // Sprachen-ListBox initialisieren

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
}

/*************************************************************************
#*  Methode:        GetRanges
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Liefert Bereichsangaben zurueck.
#*  Input:      ---
#*  Output:     Bereich
#*
#************************************************************************/

USHORT* SvxNumberFormatTabPage::GetRanges()
{
    return pRanges;
}


/*************************************************************************
#*  Methode:        Create
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Erzeugt eine neue Zahlenformat- Seite.
#*  Input:      Fenster, SfxItemSet
#*  Output:     neue TabPage
#*
#************************************************************************/

SfxTabPage* SvxNumberFormatTabPage::Create( Window* pParent,
                                            const SfxItemSet& rAttrSet )
{
    return ( new SvxNumberFormatTabPage( pParent, rAttrSet ) );
}


/*************************************************************************
#*  Methode:        Reset
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Die Attribute des Dialogs werden mit Hilfe
#*              des Itemsets neu eingestellt.
#*  Input:      SfxItemSet
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxUInt32Item*        pValFmtAttr     = NULL;
    const SfxPoolItem*          pItem           = NULL;
    const SfxBoolItem*          pAutoEntryAttr = NULL;

    USHORT                      nCatLbSelPos    = 0;
    USHORT                      nFmtLbSelPos    = 0;
    LanguageType                eLangType       = LANGUAGE_DONTKNOW;
    SvxDelStrgs                 aFmtEntryList;
    SvxNumberValueType          eValType        = SVX_VALUE_TYPE_UNDEFINED;
    double                      nValDouble      = 0;
    String                      aValString;
    SfxItemState                eState          = SFX_ITEM_DONTCARE;


    eState = rSet.GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_NOLANGUAGE ),TRUE,&pItem);

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
            HideLanguage(FALSE);
        }

    }

    eState = rSet.GetItemState( GetWhich( SID_ATTR_NUMBERFORMAT_INFO ),TRUE,&pItem);

    if(eState==SFX_ITEM_SET)
    {
        if(pNumItem==NULL)
        {
            bNumItemFlag=TRUE;
            pNumItem= (SvxNumberInfoItem *) pItem->Clone();
        }
        else
        {
            bNumItemFlag=FALSE;
        }
    }
    else
    {
        bNumItemFlag=FALSE;
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
            aCbSourceFormat.Check( FALSE );
        aCbSourceFormat.Enable();
        aCbSourceFormat.Show();
    }
    else
    {
        BOOL bInit = FALSE;     // set to TRUE for debug test
        aCbSourceFormat.Check( bInit );
        aCbSourceFormat.Enable( bInit );
        aCbSourceFormat.Show( bInit );
    }

    // pNumItem muss von aussen gesetzt worden sein!
    DBG_ASSERT( pNumItem, "No NumberInfo, no NumberFormatter, good bye.CRASH. :-(" );

    // aktuellen Zahlenformat-Tabellenindex holen
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

    // nun sind alle Informationen fuer die Formatierer-Shell beisammen:

    if ( pNumFmtShell )
         delete pNumFmtShell;   // ggF. alte Shell loeschen (==Reset)

    nInitFormat = ( pValFmtAttr )               // Init-Key merken
                    ? pValFmtAttr->GetValue()   // (fuer FillItemSet())
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

    FillCurrencyBox();

    String aPrevString;
    Color* pDummy = NULL;
    pNumFmtShell->GetInitSettings( nCatLbSelPos, eLangType, nFmtLbSelPos,
                                   aFmtEntryList, aPrevString, pDummy );

    aLbCurrency.SelectEntryPos((USHORT)pNumFmtShell->GetCurrencySymbol());

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
    UpdateFormatListBox_Impl(FALSE,TRUE);

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
        // Kategoriewechsel und direkte Eingabe sind moeglich, sonst nix:
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
#*  Methode:        Obstructing
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Sperren der Controls mit Ausnahme von Kategoriewechsel
#*              und direkter Eingabe.
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

    aIbAdd.Enable(FALSE );
    aIbRemove.Enable(FALSE );
    aIbInfo.Enable(FALSE );

    aBtnNegRed      .Disable();
    aBtnThousand    .Disable();
    aFtLeadZeroes   .Disable();
    aFtDecimals     .Disable();
    aEdLeadZeroes   .Disable();
    aEdDecimals     .Disable();
    aFlOptions      .Disable();
    aEdDecimals     .SetText( String() );
    aEdLeadZeroes   .SetText( String() );
    aBtnNegRed      .Check( FALSE );
    aBtnThousand    .Check( FALSE );
    aWndPreview     .NotifyChange( String() );

    aLbCategory     .SelectEntryPos( 0 );
    aEdFormat       .SetText( String() );
    aFtComment      .SetText( String() );
    aEdComment      .SetText(aLbCategory.GetEntry(1));  //String fuer Benutzerdefiniert
                                                        //holen

    aEdFormat       .GrabFocus();
}


/*************************************************************************
#* Enable/Disable dialog parts depending on the value of the SourceFormat
#* checkbox.
#************************************************************************/
void SvxNumberFormatTabPage::EnableBySourceFormat_Impl()
{
    BOOL bEnable = !aCbSourceFormat.IsChecked();
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
#*  Methode:    HideLanguage
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Versteckt die Spracheinstellung:
#*  Input:      BOOL nFlag
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::HideLanguage(BOOL nFlag)
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
#*  Methode:        FillItemSet
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Stellt die Attribute im ItemSet ein,
#*              sowie in der DocShell den numItem, wenn
#*              bNumItemFlag nicht gesetzt ist.
#*  Input:      SfxItemSet
#*  Output:     ---
#*
#************************************************************************/

BOOL SvxNumberFormatTabPage::FillItemSet( SfxItemSet& rCoreAttrs )
{
    BOOL bDataChanged   = aFtLanguage.IsEnabled() || aCbSourceFormat.IsEnabled();
    if ( bDataChanged )
    {
        const SfxItemSet& rMyItemSet = GetItemSet();
        USHORT          nWhich       = GetWhich( SID_ATTR_NUMBERFORMAT_VALUE );
        SfxItemState    eItemState   = rMyItemSet.GetItemState( nWhich, FALSE );

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
        const sal_uInt32 nDelCount = pNumFmtShell->GetUpdateDataCount();

        if ( nDelCount > 0 )
        {
            sal_uInt32*         pDelArr = new sal_uInt32[nDelCount];

            pNumFmtShell->GetUpdateData( pDelArr, nDelCount );
            pNumItem->SetDelFormatArray( pDelArr, nDelCount );

            if(bNumItemFlag==TRUE)
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
            USHORT _nWhich = GetWhich( SID_ATTR_NUMBERFORMAT_SOURCE );
            SfxItemState _eItemState = rMyItemSet.GetItemState( _nWhich, FALSE );
            const SfxBoolItem* pBoolItem = (const SfxBoolItem*)
                        GetItem( rMyItemSet, SID_ATTR_NUMBERFORMAT_SOURCE );
            BOOL bOld = (pBoolItem ? pBoolItem->GetValue() : FALSE);
            rCoreAttrs.Put( SfxBoolItem( _nWhich, aCbSourceFormat.IsChecked() ) );
            if ( !bDataChanged )
                bDataChanged = (bOld != (BOOL) aCbSourceFormat.IsChecked() ||
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

void SvxNumberFormatTabPage::FillFormatListBox_Impl( SvxDelStrgs& rEntries )
{
    String*     pEntry;
    String      aTmpString;
    String      aTmpCatString;
    Font        aFont=aLbCategory.GetFont();
    USHORT      i = 0;
    short       nTmpCatPos;
    short       aPrivCat;

    aLbFormat.Clear();
    aLbFormat.SetUpdateMode( FALSE );

    USHORT  nCount = rEntries.Count();

    if(nCount<1) return;

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
        for ( ; i < nCount; ++i )
        {
            pEntry = rEntries[i];
            aPrivCat=pNumFmtShell->GetCategory4Entry(i);
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
    aLbFormat.SetUpdateMode( TRUE );
    DeleteEntryList_Impl(rEntries);
}


/*************************************************************************
#*  Methode:        DeleteEntryList_Impl
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Loescht eine SvStrings- Liste
#*  Input:      String-liste
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::DeleteEntryList_Impl( SvxDelStrgs& rEntries )
{
    USHORT  nCount = rEntries.Count();
    rEntries.DeleteAndDestroy(0,nCount);
}


/*************************************************************************
#*  Methode:        UpdateOptions_Impl
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Stellt je nach eingestelltem Format die Options-
#*              attribute neu ein.
#*  Input:      Flag, ob sich die Kategorie geaendert hat.
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::UpdateOptions_Impl( BOOL bCheckCatChange /*= FALSE*/ )
{
    SvxDelStrgs aEntryList;
    String  theFormat           = aEdFormat.GetText();
    USHORT  nCurCategory        = aLbCategory.GetSelectEntryPos();
    USHORT  nCategory           = nCurCategory;
    USHORT  nDecimals           = 0;
    USHORT  nZeroes             = 0;
    BOOL    bNegRed             = FALSE;
    BOOL    bThousand           = FALSE;
    USHORT  nCurrencyPos        =aLbCurrency.GetSelectEntryPos();

    if(bOneAreaFlag)
        nCurCategory=nFixedCategory;


    pNumFmtShell->GetOptions( theFormat,
                              bThousand, bNegRed,
                              nDecimals, nZeroes,
                              nCategory );
    BOOL bDoIt=FALSE;
    if(nCategory==CAT_CURRENCY)
    {
        USHORT nTstPos=pNumFmtShell->FindCurrencyFormat(theFormat);
        if(nCurrencyPos!=nTstPos && nTstPos!=(USHORT)-1)
        {
            aLbCurrency.SelectEntryPos(nTstPos);
            pNumFmtShell->SetCurrencySymbol(nTstPos);
            bDoIt=TRUE;
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

            UpdateFormatListBox_Impl( TRUE, FALSE );
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
            aBtnNegRed      .Check( FALSE );
            aBtnThousand    .Check( FALSE );
    }
}


/*************************************************************************
#*  Methode:        UpdateFormatListBox_Impl
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Aktualisiert die Format- Listbox und zusaetzlich
#*              wird abhaengig vom bUpdateEdit- Flag der String
#*              in der Editbox geaendert.
#*  Input:      Flags fuer Kategorie und Editbox
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::UpdateFormatListBox_Impl
    (
        USHORT bCat,        // Category oder Land/Sprache ListBox?
        BOOL   bUpdateEdit  // Format-Edit aktualisieren?
    )
{
    SvxDelStrgs aEntryList;
    short       nFmtLbSelPos = 0;
    short       nTmpCatPos;

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
#if ENABLE_LAYOUT
            aLbCurrency.Disable();
#else /* !ENABLE_LAYOUT */
            aLbCurrency.Hide();
#endif /* !ENABLE_LAYOUT */
        }
        else
        {
            aPos.Y()=nCurFormatY;
            aSize.Height()=nCurFormatHeight;
            aLbFormat.SetPosSizePixel(aPos,aSize);
#if ENABLE_LAYOUT
            aLbCurrency.Enable();
#else /* !ENABLE_LAYOUT */
            aLbCurrency.Show();
#endif /* !ENABLE_LAYOUT */
        }

        pNumFmtShell->CategoryChanged( nTmpCatPos,nFmtLbSelPos, aEntryList );
    }
    else
        pNumFmtShell->LanguageChanged( aLbLanguage.GetSelectLanguage(),
                                       nFmtLbSelPos,aEntryList );

    REMOVE_DONTKNOW() // ggF. UI-Enable


    if ( (aEntryList.Count() > 0) && (nFmtLbSelPos != SELPOS_NONE) )
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
            ChangePreviewText( (USHORT)nFmtLbSelPos );
        }

    }
    else
    {
        FillFormatListBox_Impl( aEntryList );
        if(nFmtLbSelPos != SELPOS_NONE)
        {
            aLbFormat.SelectEntryPos( (USHORT)nFmtLbSelPos );

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
}


/*************************************************************************
#*  Handle:     DoubleClickHdl_Impl
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Bei einem Doppelklick in die Format- Listbox
#*              wird der Wert uebernommen und der OK-Button
#*              ausgeloest
#*  Input:      Pointer auf Listbox
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( SvxNumberFormatTabPage, DoubleClickHdl_Impl, SvxFontListBox*, pLb )
{
    if ( pLb == &aLbFormat )
    {
        SelFormatHdl_Impl( pLb );

        if ( fnOkHdl.IsSet() )
        {   // Uebergangsloesung, sollte von SfxTabPage angeboten werden
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
#*  Methode:    SelFormatHdl_Impl
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Wird aufgerufen, wenn sich die Sprache, die Kategorie
#*              oder das Format aendert. Dem entsprechend werden die
#*              Einstellungen geaendert.
#*  Input:      Pointer auf Listbox
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

#if ENABLE_LAYOUT
        if (aLbFormat.GetSelectEntryPos () == LISTBOX_ENTRY_NOTFOUND)
#else /* !ENABLE_LAYOUT */
        // Current category may be UserDefined with no format entries defined.
        // And yes, aLbFormat is a SvxFontListBox with ULONG list positions,
        // implementation returns a LIST_APPEND if empty, comparison with
        // USHORT LISTBOX_ENTRY_NOTFOUND wouldn't match.
        if ( aLbFormat.GetSelectEntryPos() == LIST_APPEND )
#endif /* !ENABLE_LAYOUT */
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

    USHORT nCurrencyPos=LISTBOX_ENTRY_NOTFOUND ;

    if(nTmpCatPos==CAT_CURRENCY && (ListBox *)pLb == &aLbCurrency )
    {
        nCurrencyPos=aLbCurrency.GetSelectEntryPos();
        pNumFmtShell->SetCurrencySymbol(nCurrencyPos);
    }

    //--------------------------------------------------------------------
    // Format-ListBox ----------------------------------------------------
    if ( (SvxFontListBox *)pLb == &aLbFormat )
    {
        USHORT  nSelPos = (USHORT) aLbFormat.GetSelectEntryPos();
        String  aFormat = aLbFormat.GetSelectEntry();
        String  aComment;
        SvxDelStrgs aEntryList;

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

        REMOVE_DONTKNOW() // ggF. UI-Enable

        if ( pNumFmtShell->FindEntry( aFormat) )
        {
            aIbAdd.Enable(FALSE );
            BOOL bIsUserDef=pNumFmtShell->IsUserDefined( aFormat );
            aIbRemove.Enable(bIsUserDef);
            aIbInfo.Enable(bIsUserDef);

        }
        else
        {
            aIbAdd.Enable(TRUE );
            aIbInfo.Enable(TRUE );
            aIbRemove.Enable(FALSE );
            aFtComment.SetText(aEdComment.GetText());

        }
        UpdateOptions_Impl( FALSE );

        return 0;
    }

    //--------------------------------------------------------------------
    // Kategorie-ListBox -------------------------------------------------
    if ( pLb == &aLbCategory || pLb == &aLbCurrency)
    {
        UpdateFormatListBox_Impl( TRUE, TRUE );
        EditHdl_Impl( NULL );
        UpdateOptions_Impl( FALSE );

        return 0;
    }

    //--------------------------------------------------------------------
    // Sprache/Land-ListBox ----------------------------------------------
    if ( pLb == &aLbLanguage )
    {
        UpdateFormatListBox_Impl( FALSE, TRUE );
        EditHdl_Impl( &aEdFormat );

        return 0;
    }
    return 0;
}


/*************************************************************************
#*  Methode:    ClickHdl_Impl, ImageButton* pIB
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Wenn, der Hinzufuegen- oder Entfernen- Button
#*              wird diese Funktion aufgerufen und die Zahlenformat-
#*              Liste den entsprechend geaendert.
#*  Input:      Toolbox- Button
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( SvxNumberFormatTabPage, ClickHdl_Impl, ImageButton*, pIB)
{
    BOOL        bAdded = FALSE;
    BOOL        bDeleted = FALSE;
    ULONG       nReturn = 0;
    const ULONG nReturnChanged  = 0x1;  // THE boolean return value
    const ULONG nReturnAdded    = 0x2;  // temp: format added
    const ULONG nReturnOneArea  = 0x4;  // temp: one area but category changed => ignored

    if(pIB==&aIbAdd)
    {   // Also called from FillItemSet() if a temporary currency format has
        // to be added, not only if the Add button is enabled.
        String      aFormat = aEdFormat.GetText();
        SvxDelStrgs aEntryList;
        SvxDelStrgs a2EntryList;
        USHORT      nCatLbSelPos = 0;
        short       nFmtLbSelPos = SELPOS_NONE;
        xub_StrLen  nErrPos=0;

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
            if(nCatLbSelPos==CAT_CURRENCY)
            {
                aLbCurrency.SelectEntryPos((USHORT)pNumFmtShell->GetCurrencySymbol());
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
                    // Alles klar
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
                    aLbFormat.SelectEntryPos( (USHORT)nFmtLbSelPos );
                    aEdFormat.SetText( aFormat );

                    aEdComment.SetText(aLbCategory.GetEntry(1));    //String fuer Benutzerdefiniert
                                                                    //holen
                    ChangePreviewText( (USHORT)nFmtLbSelPos );
                }
            }
        }
        else // Syntaxfehler
        {
            aEdFormat.GrabFocus();
            aEdFormat.SetSelection( Selection( (short)nErrPos, SELECTION_MAX ) );
        }
        EditHdl_Impl( &aEdFormat );
        nReturn = ((nReturn & nReturnOneArea) ? 0 : (nReturn & nReturnChanged));
    }
    else if(pIB==&aIbRemove)
    {
        String      aFormat = aEdFormat.GetText();
        SvxDelStrgs aEntryList;
        USHORT      nCatLbSelPos = 0;
        short       nFmtLbSelPos = SELPOS_NONE;

        bDeleted = pNumFmtShell->RemoveFormat( aFormat,
                                               nCatLbSelPos,
                                               nFmtLbSelPos,
                                               aEntryList );

        aEdComment.SetText(aLbCategory.GetEntry(1));
        if ( bDeleted )
        {
            if(nFmtLbSelPos>=0 &&  nFmtLbSelPos<aEntryList.Count())
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

                aLbFormat.SelectEntryPos( (USHORT)nFmtLbSelPos );
                aEdFormat.SetText( aFormat );
                ChangePreviewText( (USHORT)nFmtLbSelPos );
            }
            else
            {
                // auf "Alle/Standard" setzen
                SetCategory(0 );
                SelFormatHdl_Impl( &aLbCategory );
            }
        }
        EditHdl_Impl( &aEdFormat );
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
#*  Methode:    EditHdl_Impl
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Wenn der Eintrag im Eingabefeld geaendert wird,
#*              so wird die Vorschau aktualisiert und
#*  Input:      Pointer auf Editbox
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( SvxNumberFormatTabPage, EditHdl_Impl, Edit*, pEdFormat )
{
    sal_uInt32 nCurKey = NUMKEY_UNDEFINED;

    if ( aEdFormat.GetText().Len() == 0 )
    {
        aIbAdd.Enable(FALSE );
        aIbRemove.Enable(FALSE );
        aIbInfo.Enable(FALSE );
        aFtComment.SetText(String());
    }
    else
    {
        String aFormat = aEdFormat.GetText();
        MakePreviewText( aFormat );

        if ( pNumFmtShell->FindEntry( aFormat, &nCurKey ) )
        {
            aIbAdd.Enable(FALSE );
            BOOL bUserDef=pNumFmtShell->IsUserDefined( aFormat );

            aIbRemove.Enable(bUserDef);
            aIbInfo.Enable(bUserDef);

            if(bUserDef)
            {
                USHORT nTmpCurPos=pNumFmtShell->FindCurrencyFormat(aFormat );

                if(nTmpCurPos!=(USHORT)-1)
                    aLbCurrency.SelectEntryPos(nTmpCurPos);
            }
            short nPosi=pNumFmtShell->GetListPos4Entry(aFormat);
            if(nPosi>=0)
                aLbFormat.SelectEntryPos( (USHORT)nPosi);

        }
        else
        {

            aIbAdd.Enable(TRUE );
            aIbInfo.Enable(TRUE);
            aIbRemove.Enable(FALSE );

            aFtComment.SetText(aEdComment.GetText());

        }
    }

    if ( pEdFormat )
    {
        pNumFmtShell->SetCurNumFmtKey( nCurKey );
        UpdateOptions_Impl( TRUE );
    }

    return 0;
}


/*************************************************************************
#*  Methode:        NotifyChange
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Fuehrt Aenderungen in den Zahlen- Attributen durch.
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
        BOOL          bThousand     =    aBtnThousand.IsEnabled()
                                      && aBtnThousand.IsChecked();
        BOOL          bNegRed       =    aBtnNegRed.IsEnabled()
                                      && aBtnNegRed.IsChecked();
        USHORT        nPrecision    = (aEdDecimals.IsEnabled())
                                        ? (USHORT)aEdDecimals.GetValue()
                                        : (USHORT)0;
        USHORT        nLeadZeroes   = (aEdLeadZeroes.IsEnabled())
                                        ? (USHORT)aEdLeadZeroes.GetValue()
                                        : (USHORT)0;

        pNumFmtShell->MakeFormat( aFormat,
                                  bThousand, bNegRed,
                                  nPrecision, nLeadZeroes,
                                  (USHORT)aLbFormat.GetSelectEntryPos() );

        aEdFormat.SetText( aFormat );
        MakePreviewText( aFormat );

        if ( pNumFmtShell->FindEntry( aFormat ) )
        {
            aIbAdd.Enable(FALSE );
            BOOL bUserDef=pNumFmtShell->IsUserDefined( aFormat );
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

IMPL_LINK( SvxNumberFormatTabPage, TimeHdl_Impl, Timer*, EMPTYARG)
{
    pLastActivWindow=NULL;
    return 0;
}


/*************************************************************************
#*  Methode:    LostFocusHdl_Impl
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Fuehrt Aenderungen in den Zahlen- Attributen durch.
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
            USHORT  nSelPos = (USHORT) aLbFormat.GetSelectEntryPos();
            pNumFmtShell->SetComment4Entry(nSelPos,
                                        aEdComment.GetText());
            aEdComment.SetText(aLbCategory.GetEntry(1));    //String fuer Benutzerdefiniert
                                                            //holen
        }
    }
    return 0;
}

/*************************************************************************
#*  Methode:        NotifyChange
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Fuehrt Aenderungen in den Zahlen- Attributen durch.
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

void SvxNumberFormatTabPage::ChangePreviewText( USHORT nPos )
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
#*  Methode:    SetOkHdl
#*------------------------------------------------------------------------
#*
#*  Klasse:     SvxNumberFormatTabPage
#*  Funktion:   Setzt den OkHandler neu.
#*  Input:      Neuer OkHandler
#*  Output:     ---
#*
#************************************************************************/

void SvxNumberFormatTabPage::SetOkHdl( const Link& rOkHandler )
{
    fnOkHdl = rOkHandler;
}

void SvxNumberFormatTabPage::FillCurrencyBox()
{
    SvStringsDtor   aList;
    NfShCurrencyEntries rEntries;
    XubString*      pEntry = NULL;
    USHORT  nSelPos=0;

    pNumFmtShell->GetCurrencySymbols( aList, &nSelPos);

    for(USHORT i=1;i<aList.Count();i++)
    {
        pEntry=aList[i];
        aLbCurrency.InsertEntry( *pEntry);
    }
    aLbCurrency.SelectEntryPos(nSelPos);
}

void SvxNumberFormatTabPage::SetCategory(USHORT nPos)
{
    USHORT  nCurCategory = aLbCategory.GetSelectEntryPos();
    Point aPos=aLbFormat.GetPosPixel();
    Size  aSize=aLbFormat.GetSizePixel();
    USHORT nTmpCatPos;

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
void SvxNumberFormatTabPage::AddAutomaticLanguage_Impl(LanguageType eAutoLang, BOOL bSelect)
{
    aLbLanguage.RemoveLanguage(LANGUAGE_SYSTEM);
    USHORT nPos = aLbLanguage.InsertEntry(sAutomaticEntry);
    aLbLanguage.SetEntryData(nPos, (void*)(ULONG)eAutoLang);
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
