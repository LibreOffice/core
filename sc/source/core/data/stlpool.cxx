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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/fontitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/itemset.hxx>
#include <svl/zforlist.hxx>
#include <unotools/charclass.hxx>
#include <unotools/fontcvt.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>

#include "sc.hrc"
#include "attrib.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "document.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "rechead.hxx"
#include "editutil.hxx"
#include "patattr.hxx"


//========================================================================

ScStyleSheetPool::ScStyleSheetPool( SfxItemPool&    rPoolP,
                                    ScDocument*     pDocument )
    :   SfxStyleSheetPool( rPoolP ),
        pActualStyleSheet( NULL ),
        pDoc( pDocument ),
        pForceStdName( NULL )
{
}

//------------------------------------------------------------------------

ScStyleSheetPool::~ScStyleSheetPool()
{
}

//------------------------------------------------------------------------

void ScStyleSheetPool::SetDocument( ScDocument* pDocument )
{
    pDoc = pDocument;
}

//------------------------------------------------------------------------

SfxStyleSheetBase& ScStyleSheetPool::Make( const OUString& rName,
                                           SfxStyleFamily eFam, sal_uInt16 mask)
{
    //  When updating styles from a template, Office 5.1 sometimes created
    //  files with multiple default styles.
    //  Create new styles in that case:

    //! only when loading?

    if ( rName == STRING_STANDARD && Find( rName, eFam ) != NULL )
    {
        OSL_FAIL("renaming additional default style");
        sal_uInt32 nCount = aStyles.size();
        for ( sal_uInt32 nAdd = 1; nAdd <= nCount; nAdd++ )
        {
            OUString aNewName = ScGlobal::GetRscString(STR_STYLENAME_STANDARD);
            aNewName += OUString::number( nAdd );
            if ( Find( aNewName, eFam ) == NULL )
                return SfxStyleSheetPool::Make(aNewName, eFam, mask);
        }
    }

    return SfxStyleSheetPool::Make(rName, eFam, mask);
}

//------------------------------------------------------------------------

SfxStyleSheetBase* ScStyleSheetPool::Create( const OUString&   rName,
                                             SfxStyleFamily  eFamily,
                                             sal_uInt16          nMaskP )
{
    ScStyleSheet* pSheet = new ScStyleSheet( rName, *this, eFamily, nMaskP );
    if ( eFamily == SFX_STYLE_FAMILY_PARA && ScGlobal::GetRscString(STR_STYLENAME_STANDARD) != rName )
        pSheet->SetParent( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );

    return pSheet;
}

//------------------------------------------------------------------------

SfxStyleSheetBase* ScStyleSheetPool::Create( const SfxStyleSheetBase& rStyle )
{
    OSL_ENSURE( rStyle.ISA(ScStyleSheet), "Invalid StyleSheet-class! :-/" );
    return new ScStyleSheet( (const ScStyleSheet&) rStyle );
}

//------------------------------------------------------------------------

void ScStyleSheetPool::Remove( SfxStyleSheetBase* pStyle )
{
    if ( pStyle )
    {
        OSL_ENSURE( IS_SET( SFXSTYLEBIT_USERDEF, pStyle->GetMask() ),
                    "SFXSTYLEBIT_USERDEF not set!" );

        ((ScDocumentPool&)rPool).StyleDeleted((ScStyleSheet*)pStyle);
        SfxStyleSheetPool::Remove(pStyle);
    }
}

//------------------------------------------------------------------------

void ScStyleSheetPool::CopyStyleFrom( ScStyleSheetPool* pSrcPool,
                                      const OUString& rName, SfxStyleFamily eFamily )
{
    //  this ist Dest-Pool

    SfxStyleSheetBase* pStyleSheet = pSrcPool->Find( rName, eFamily );
    if (pStyleSheet)
    {
        const SfxItemSet& rSourceSet = pStyleSheet->GetItemSet();
        SfxStyleSheetBase* pDestSheet = Find( rName, eFamily );
        if (!pDestSheet)
            pDestSheet = &Make( rName, eFamily );
        SfxItemSet& rDestSet = pDestSheet->GetItemSet();
        rDestSet.PutExtended( rSourceSet, SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT );

        const SfxPoolItem* pItem;
        if ( eFamily == SFX_STYLE_FAMILY_PAGE )
        {
            //  Set-Items

            if ( rSourceSet.GetItemState( ATTR_PAGE_HEADERSET, false, &pItem ) == SFX_ITEM_SET )
            {
                const SfxItemSet& rSrcSub = ((const SvxSetItem*) pItem)->GetItemSet();
                SfxItemSet aDestSub( *rDestSet.GetPool(), rSrcSub.GetRanges() );
                aDestSub.PutExtended( rSrcSub, SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT );
                rDestSet.Put( SvxSetItem( ATTR_PAGE_HEADERSET, aDestSub ) );
            }
            if ( rSourceSet.GetItemState( ATTR_PAGE_FOOTERSET, false, &pItem ) == SFX_ITEM_SET )
            {
                const SfxItemSet& rSrcSub = ((const SvxSetItem*) pItem)->GetItemSet();
                SfxItemSet aDestSub( *rDestSet.GetPool(), rSrcSub.GetRanges() );
                aDestSub.PutExtended( rSrcSub, SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT );
                rDestSet.Put( SvxSetItem( ATTR_PAGE_FOOTERSET, aDestSub ) );
            }
        }
        else    // cell styles
        {
            // number format exchange list has to be handled here, too

            if ( pDoc && pDoc->GetFormatExchangeList() &&
                 rSourceSet.GetItemState( ATTR_VALUE_FORMAT, false, &pItem ) == SFX_ITEM_SET )
            {
                sal_uLong nOldFormat = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
                SvNumberFormatterIndexTable::const_iterator it = pDoc->GetFormatExchangeList()->find(nOldFormat);
                if (it != pDoc->GetFormatExchangeList()->end())
                {
                    sal_uInt32 nNewFormat = it->second;
                    rDestSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNewFormat ) );
                }
            }
        }
    }
}

//------------------------------------------------------------------------
//
//                      Standard-Vorlagen
//
//------------------------------------------------------------------------

#define SCSTR(id)   ScGlobal::GetRscString(id)

void ScStyleSheetPool::CopyStdStylesFrom( ScStyleSheetPool* pSrcPool )
{
    //  Default-Styles kopieren

    CopyStyleFrom( pSrcPool, SCSTR(STR_STYLENAME_STANDARD),     SFX_STYLE_FAMILY_PARA );
    CopyStyleFrom( pSrcPool, SCSTR(STR_STYLENAME_RESULT),       SFX_STYLE_FAMILY_PARA );
    CopyStyleFrom( pSrcPool, SCSTR(STR_STYLENAME_RESULT1),      SFX_STYLE_FAMILY_PARA );
    CopyStyleFrom( pSrcPool, SCSTR(STR_STYLENAME_HEADLINE),     SFX_STYLE_FAMILY_PARA );
    CopyStyleFrom( pSrcPool, SCSTR(STR_STYLENAME_HEADLINE1),    SFX_STYLE_FAMILY_PARA );
    CopyStyleFrom( pSrcPool, SCSTR(STR_STYLENAME_STANDARD),     SFX_STYLE_FAMILY_PAGE );
    CopyStyleFrom( pSrcPool, SCSTR(STR_STYLENAME_REPORT),       SFX_STYLE_FAMILY_PAGE );
}

//------------------------------------------------------------------------

static void lcl_CheckFont( SfxItemSet& rSet, LanguageType eLang, sal_uInt16 nFontType, sal_uInt16 nItemId )
{
    if ( eLang != LANGUAGE_NONE && eLang != LANGUAGE_DONTKNOW && eLang != LANGUAGE_SYSTEM )
    {
        Font aDefFont = OutputDevice::GetDefaultFont( nFontType, eLang, DEFAULTFONT_FLAGS_ONLYONE );
        SvxFontItem aNewItem( aDefFont.GetFamily(), aDefFont.GetName(), aDefFont.GetStyleName(),
                              aDefFont.GetPitch(), aDefFont.GetCharSet(), nItemId );
        if ( aNewItem != rSet.Get( nItemId ) )
        {
            // put item into style's ItemSet only if different from (static) default
            rSet.Put( aNewItem );
        }
    }
}

void ScStyleSheetPool::CreateStandardStyles()
{
    //  neue Eintraege auch bei CopyStdStylesFrom eintragen

    Color           aColBlack   ( COL_BLACK );
    Color           aColGrey    ( COL_LIGHTGRAY );
    OUString        aStr;
    sal_Int32       nStrLen;
    String          aHelpFile;//XXX JN welcher Text???
    SfxItemSet*     pSet            = NULL;
    SfxItemSet*     pHFSet          = NULL;
    SvxSetItem*     pHFSetItem      = NULL;
    ScEditEngineDefaulter*  pEdEngine   = new ScEditEngineDefaulter( EditEngine::CreatePool(), sal_True );
    pEdEngine->SetUpdateMode( false );
    EditTextObject* pEmptyTxtObj    = pEdEngine->CreateTextObject();
    EditTextObject* pTxtObj         = NULL;
    ScPageHFItem*   pHeaderItem     = new ScPageHFItem( ATTR_PAGE_HEADERRIGHT );
    ScPageHFItem*   pFooterItem     = new ScPageHFItem( ATTR_PAGE_FOOTERRIGHT );
    ScStyleSheet*   pSheet          = NULL;
    ::editeng::SvxBorderLine    aBorderLine     ( &aColBlack, DEF_LINE_WIDTH_2 );
    SvxBoxItem      aBoxItem        ( ATTR_BORDER );
    SvxBoxInfoItem  aBoxInfoItem    ( ATTR_BORDER_INNER );

    String aStrStandard = ScGlobal::GetRscString(STR_STYLENAME_STANDARD);

    //==========================================================
    // Zellformatvorlagen:
    //==========================================================

    //------------
    // 1. Standard
    //------------
    pSheet = (ScStyleSheet*) &Make( aStrStandard, SFX_STYLE_FAMILY_PARA, SCSTYLEBIT_STANDARD );
    pSheet->SetHelpId( aHelpFile, HID_SC_SHEET_CELL_STD );

    //  if default fonts for the document's languages are different from the pool default,
    //  put them into the default style
    //  (not as pool defaults, because pool defaults can't be changed by the user)
    //  the document languages must be set before creating the default styles!

    pSet = &pSheet->GetItemSet();
    LanguageType eLatin, eCjk, eCtl;
    pDoc->GetLanguage( eLatin, eCjk, eCtl );

    //  If the UI language is Korean, the default Latin font has to
    //  be queried for Korean, too (the Latin language from the document can't be Korean).
    //  This is the same logic as in SwDocShell::InitNew.
    LanguageType eUiLanguage = Application::GetSettings().GetUILanguageTag().getLanguageType();
    if (MsLangId::isKorean(eUiLanguage))
        eLatin = eUiLanguage;

    lcl_CheckFont( *pSet, eLatin, DEFAULTFONT_LATIN_SPREADSHEET, ATTR_FONT );
    lcl_CheckFont( *pSet, eCjk, DEFAULTFONT_CJK_SPREADSHEET, ATTR_CJK_FONT );
    lcl_CheckFont( *pSet, eCtl, DEFAULTFONT_CTL_SPREADSHEET, ATTR_CTL_FONT );

    // #i55300# default CTL font size for Thai has to be larger
    // #i59408# The 15 point size causes problems with row heights, so no different
    // size is used for Thai in Calc for now.
//    if ( eCtl == LANGUAGE_THAI )
//        pSet->Put( SvxFontHeightItem( 300, 100, ATTR_CTL_FONT_HEIGHT ) );   // 15 pt

    //------------
    // 2. Ergebnis
    //------------

    pSheet = (ScStyleSheet*) &Make( SCSTR( STR_STYLENAME_RESULT ),
                                    SFX_STYLE_FAMILY_PARA,
                                    SCSTYLEBIT_STANDARD );
    pSheet->SetParent( aStrStandard );
    pSheet->SetHelpId( aHelpFile, HID_SC_SHEET_CELL_ERG );
    pSet = &pSheet->GetItemSet();
    pSet->Put( SvxWeightItem( WEIGHT_BOLD, ATTR_FONT_WEIGHT ) );
    pSet->Put( SvxPostureItem( ITALIC_NORMAL, ATTR_FONT_POSTURE ) );
    pSet->Put( SvxUnderlineItem( UNDERLINE_SINGLE, ATTR_FONT_UNDERLINE ) );

    //-------------
    // 3. Ergebnis1
    //-------------

    pSheet = (ScStyleSheet*) &Make( SCSTR( STR_STYLENAME_RESULT1 ),
                                    SFX_STYLE_FAMILY_PARA,
                                    SCSTYLEBIT_STANDARD );

    pSheet->SetParent( SCSTR( STR_STYLENAME_RESULT ) );
    pSheet->SetHelpId( aHelpFile, HID_SC_SHEET_CELL_ERG1 );

    //----------------
    // 4. Ueberschrift
    //----------------

    pSheet = (ScStyleSheet*) &Make( SCSTR( STR_STYLENAME_HEADLINE ),
                                    SFX_STYLE_FAMILY_PARA,
                                    SCSTYLEBIT_STANDARD );

    pSheet->SetParent( aStrStandard );
    pSheet->SetHelpId( aHelpFile, HID_SC_SHEET_CELL_UEB );
    pSet = &pSheet->GetItemSet();
    pSet->Put( SvxFontHeightItem( 320, 100, ATTR_FONT_HEIGHT ) ); // 16pt
    pSet->Put( SvxWeightItem( WEIGHT_BOLD, ATTR_FONT_WEIGHT ) );
    pSet->Put( SvxPostureItem( ITALIC_NORMAL, ATTR_FONT_POSTURE ) );
    pSet->Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_CENTER, ATTR_HOR_JUSTIFY ) );

    //-----------------
    // 5. Ueberschrift1
    //-----------------

    pSheet = (ScStyleSheet*) &Make( SCSTR( STR_STYLENAME_HEADLINE1 ),
                                    SFX_STYLE_FAMILY_PARA,
                                    SCSTYLEBIT_STANDARD );

    pSheet->SetParent( SCSTR( STR_STYLENAME_HEADLINE ) );
    pSheet->SetHelpId( aHelpFile, HID_SC_SHEET_CELL_UEB1 );
    pSet = &pSheet->GetItemSet();
    pSet->Put( SfxInt32Item( ATTR_ROTATE_VALUE, 9000 ) );

    //==========================================================
    // Seitenformat-Vorlagen:
    //==========================================================

    //------------
    // 1. Standard
    //------------

    pSheet = (ScStyleSheet*) &Make( aStrStandard,
                                    SFX_STYLE_FAMILY_PAGE,
                                    SCSTYLEBIT_STANDARD );

    pSet = &pSheet->GetItemSet();
    pSheet->SetHelpId( aHelpFile, HID_SC_SHEET_PAGE_STD );

    // Abstand der Kopf-/Fusszeilen von der Tabelle
    pHFSetItem = new SvxSetItem( ((SvxSetItem&)pSet->Get( ATTR_PAGE_HEADERSET ) ) );
    pSet->Put( *pHFSetItem, ATTR_PAGE_HEADERSET );
    pSet->Put( *pHFSetItem, ATTR_PAGE_FOOTERSET );
    DELETEZ( pHFSetItem );

    //----------------------------------------
    // Kopfzeile:
    // [leer][\TABELLE\][leer]
    //----------------------------------------
    pEdEngine->SetText(EMPTY_STRING);
    pEdEngine->QuickInsertField( SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD), ESelection() );
    pTxtObj = pEdEngine->CreateTextObject();
    pHeaderItem->SetLeftArea  ( *pEmptyTxtObj );
    pHeaderItem->SetCenterArea( *pTxtObj );
    pHeaderItem->SetRightArea ( *pEmptyTxtObj );
    pSet->Put( *pHeaderItem );
    DELETEZ( pTxtObj );

    //----------------------------------------
    // Fusszeile:
    // [leer][Seite \SEITE\][leer]
    //----------------------------------------
    aStr = SCSTR( STR_PAGE ) + " ";
    pEdEngine->SetText( aStr );
    nStrLen = aStr.getLength();
    pEdEngine->QuickInsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(0,nStrLen,0,nStrLen) );
    pTxtObj = pEdEngine->CreateTextObject();
    pFooterItem->SetLeftArea  ( *pEmptyTxtObj );
    pFooterItem->SetCenterArea( *pTxtObj );
    pFooterItem->SetRightArea ( *pEmptyTxtObj );
    pSet->Put( *pFooterItem );
    DELETEZ( pTxtObj );

    //----------
    // 2. Report
    //----------

    pSheet = (ScStyleSheet*) &Make( SCSTR( STR_STYLENAME_REPORT ),
                                    SFX_STYLE_FAMILY_PAGE,
                                    SCSTYLEBIT_STANDARD );
    pSet = &pSheet->GetItemSet();
    pSheet->SetHelpId( aHelpFile, HID_SC_SHEET_PAGE_REP );

    // Hintergrund und Umrandung
    aBoxItem.SetLine( &aBorderLine, BOX_LINE_TOP );
    aBoxItem.SetLine( &aBorderLine, BOX_LINE_BOTTOM );
    aBoxItem.SetLine( &aBorderLine, BOX_LINE_LEFT );
    aBoxItem.SetLine( &aBorderLine, BOX_LINE_RIGHT );
    aBoxItem.SetDistance( 10 ); // 0.2mm
    aBoxInfoItem.SetValid( VALID_TOP, sal_True );
    aBoxInfoItem.SetValid( VALID_BOTTOM, sal_True );
    aBoxInfoItem.SetValid( VALID_LEFT, sal_True );
    aBoxInfoItem.SetValid( VALID_RIGHT, sal_True );
    aBoxInfoItem.SetValid( VALID_DISTANCE, sal_True );
    aBoxInfoItem.SetTable( false );
    aBoxInfoItem.SetDist ( sal_True );

    pHFSetItem = new SvxSetItem( ((SvxSetItem&)pSet->Get( ATTR_PAGE_HEADERSET ) ) );
    pHFSet = &(pHFSetItem->GetItemSet());

    pHFSet->Put( SvxBrushItem( aColGrey, ATTR_BACKGROUND ) );
    pHFSet->Put( aBoxItem );
    pHFSet->Put( aBoxInfoItem );
    pSet->Put( *pHFSetItem, ATTR_PAGE_HEADERSET );
    pSet->Put( *pHFSetItem, ATTR_PAGE_FOOTERSET );
    DELETEZ( pHFSetItem );

    //----------------------------------------
    // Kopfzeile:
    // [\TABELLE\ (\DATEI\)][leer][\DATUM\, \ZEIT\]
    //----------------------------------------
    aStr = " ()";
    pEdEngine->SetText( aStr );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxFileField(), EE_FEATURE_FIELD), ESelection(0,2,0,2) );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD), ESelection() );
    pTxtObj = pEdEngine->CreateTextObject();
    pHeaderItem->SetLeftArea( *pTxtObj );
    pHeaderItem->SetCenterArea( *pEmptyTxtObj );
    DELETEZ( pTxtObj );
    aStr = ", ";
    pEdEngine->SetText( aStr );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxTimeField(), EE_FEATURE_FIELD), ESelection(0,2,0,2) );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxDateField(Date( Date::SYSTEM ),SVXDATETYPE_VAR), EE_FEATURE_FIELD),
                                    ESelection() );
    pTxtObj = pEdEngine->CreateTextObject();
    pHeaderItem->SetRightArea( *pTxtObj );
    DELETEZ( pTxtObj );
    pSet->Put( *pHeaderItem );

    //----------------------------------------
    // Fusszeile:
    // [leer][Seite: \SEITE\ / \SEITEN\][leer]
    //----------------------------------------
    aStr = SCSTR( STR_PAGE ) + " ";
    nStrLen = aStr.getLength();
    aStr += " / ";
    sal_Int32 nStrLen2 = aStr.getLength();
    pEdEngine->SetText( aStr );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxPagesField(), EE_FEATURE_FIELD), ESelection(0,nStrLen2,0,nStrLen2) );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), ESelection(0,nStrLen,0,nStrLen) );
    pTxtObj = pEdEngine->CreateTextObject();
    pFooterItem->SetLeftArea  ( *pEmptyTxtObj );
    pFooterItem->SetCenterArea( *pTxtObj );
    pFooterItem->SetRightArea ( *pEmptyTxtObj );
    pSet->Put( *pFooterItem );
    DELETEZ( pTxtObj );

    //----------------------------------------------------
    DELETEZ( pEmptyTxtObj );
    DELETEZ( pHeaderItem );
    DELETEZ( pFooterItem );
    DELETEZ( pEdEngine );
}

//------------------------------------------------------------------------


ScStyleSheet* ScStyleSheetPool::FindCaseIns( const OUString& rName, SfxStyleFamily eFam )
{
    OUString aUpSearch = ScGlobal::pCharClass->uppercase(rName);

    sal_uInt32 nCount = aStyles.size();
    for (sal_uInt32 n=0; n<nCount; n++)
    {
        SfxStyleSheetBase* pStyle = aStyles[n].get();
        if ( pStyle->GetFamily() == eFam )
        {
            String aUpName = ScGlobal::pCharClass->uppercase(pStyle->GetName());
            if (aUpName == aUpSearch)
                return (ScStyleSheet*)pStyle;
        }
    }

    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
