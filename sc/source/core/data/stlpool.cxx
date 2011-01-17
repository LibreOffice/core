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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


//------------------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
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

__EXPORT ScStyleSheetPool::~ScStyleSheetPool()
{
}

//------------------------------------------------------------------------

void ScStyleSheetPool::SetDocument( ScDocument* pDocument )
{
    pDoc = pDocument;
}

//------------------------------------------------------------------------

//UNUSED2009-05 void ScStyleSheetPool::SetForceStdName( const String* pSet )
//UNUSED2009-05 {
//UNUSED2009-05     pForceStdName = pSet;
//UNUSED2009-05 }

//------------------------------------------------------------------------

SfxStyleSheetBase& ScStyleSheetPool::Make( const String& rName,
                            SfxStyleFamily eFam, sal_uInt16 mask, sal_uInt16 nPos )
{
    //  When updating styles from a template, Office 5.1 sometimes created
    //  files with multiple default styles.
    //  Create new styles in that case:

    //! only when loading?

    if ( rName.EqualsAscii(STRING_STANDARD) && Find( rName, eFam ) != NULL )
    {
        DBG_ERROR("renaming additional default style");
        sal_uInt32 nCount = aStyles.size();
        for ( sal_uInt32 nAdd = 1; nAdd <= nCount; nAdd++ )
        {
            String aNewName = ScGlobal::GetRscString(STR_STYLENAME_STANDARD);
            aNewName += String::CreateFromInt32( nAdd );
            if ( Find( aNewName, eFam ) == NULL )
                return SfxStyleSheetPool::Make( aNewName, eFam, mask, nPos );
        }
    }

    return SfxStyleSheetPool::Make( rName, eFam, mask, nPos );
}

//------------------------------------------------------------------------

SfxStyleSheetBase* __EXPORT ScStyleSheetPool::Create(
                                            const String&   rName,
                                            SfxStyleFamily  eFamily,
                                            sal_uInt16          nMaskP )
{
    ScStyleSheet* pSheet = new ScStyleSheet( rName, *this, eFamily, nMaskP );
    if ( eFamily == SFX_STYLE_FAMILY_PARA && ScGlobal::GetRscString(STR_STYLENAME_STANDARD) != rName )
        pSheet->SetParent( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );

    return pSheet;
}

//------------------------------------------------------------------------

SfxStyleSheetBase* __EXPORT ScStyleSheetPool::Create( const SfxStyleSheetBase& rStyle )
{
    DBG_ASSERT( rStyle.ISA(ScStyleSheet), "Invalid StyleSheet-class! :-/" );
    return new ScStyleSheet( (const ScStyleSheet&) rStyle );
}

//------------------------------------------------------------------------

void __EXPORT ScStyleSheetPool::Remove( SfxStyleSheetBase* pStyle )
{
    if ( pStyle )
    {
        DBG_ASSERT( IS_SET( SFXSTYLEBIT_USERDEF, pStyle->GetMask() ),
                    "SFXSTYLEBIT_USERDEF not set!" );

        ((ScDocumentPool&)rPool).StyleDeleted((ScStyleSheet*)pStyle);
        SfxStyleSheetPool::Remove(pStyle);
    }
}

//------------------------------------------------------------------------

void ScStyleSheetPool::CopyStyleFrom( ScStyleSheetPool* pSrcPool,
                                        const String& rName, SfxStyleFamily eFamily )
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

            if ( rSourceSet.GetItemState( ATTR_PAGE_HEADERSET, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                const SfxItemSet& rSrcSub = ((const SvxSetItem*) pItem)->GetItemSet();
                SfxItemSet aDestSub( *rDestSet.GetPool(), rSrcSub.GetRanges() );
                aDestSub.PutExtended( rSrcSub, SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT );
                rDestSet.Put( SvxSetItem( ATTR_PAGE_HEADERSET, aDestSub ) );
            }
            if ( rSourceSet.GetItemState( ATTR_PAGE_FOOTERSET, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                const SfxItemSet& rSrcSub = ((const SvxSetItem*) pItem)->GetItemSet();
                SfxItemSet aDestSub( *rDestSet.GetPool(), rSrcSub.GetRanges() );
                aDestSub.PutExtended( rSrcSub, SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT );
                rDestSet.Put( SvxSetItem( ATTR_PAGE_FOOTERSET, aDestSub ) );
            }
        }
        else    // cell styles
        {
            // #b5017505# number format exchange list has to be handled here, too

            if ( pDoc && pDoc->GetFormatExchangeList() &&
                 rSourceSet.GetItemState( ATTR_VALUE_FORMAT, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                sal_uLong nOldFormat = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
                sal_uInt32* pNewFormat = static_cast<sal_uInt32*>(pDoc->GetFormatExchangeList()->Get( nOldFormat ));
                if (pNewFormat)
                    rDestSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, *pNewFormat ) );
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

void lcl_CheckFont( SfxItemSet& rSet, LanguageType eLang, sal_uInt16 nFontType, sal_uInt16 nItemId )
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
    String          aStr;
    xub_StrLen      nStrLen;
    String          aHelpFile;//XXX JN welcher Text???
    //sal_uLong         nNumFmt         = 0L;
    SfxItemSet*     pSet            = NULL;
    SfxItemSet*     pHFSet          = NULL;
    SvxSetItem*     pHFSetItem      = NULL;
    ScEditEngineDefaulter*  pEdEngine   = new ScEditEngineDefaulter( EditEngine::CreatePool(), sal_True );
    pEdEngine->SetUpdateMode( sal_False );
    EditTextObject* pEmptyTxtObj    = pEdEngine->CreateTextObject();
    EditTextObject* pTxtObj         = NULL;
    ScPageHFItem*   pHeaderItem     = new ScPageHFItem( ATTR_PAGE_HEADERRIGHT );
    ScPageHFItem*   pFooterItem     = new ScPageHFItem( ATTR_PAGE_FOOTERRIGHT );
    ScStyleSheet*   pSheet          = NULL;
    SvxBorderLine   aBorderLine     ( &aColBlack, DEF_LINE_WIDTH_2 );
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

    //  #108374# / #107782#: If the UI language is Korean, the default Latin font has to
    //  be queried for Korean, too (the Latin language from the document can't be Korean).
    //  This is the same logic as in SwDocShell::InitNew.
    LanguageType eUiLanguage = Application::GetSettings().GetUILanguage();
    switch( eUiLanguage )
    {
        case LANGUAGE_KOREAN:
        case LANGUAGE_KOREAN_JOHAB:
            eLatin = eUiLanguage;
        break;
    }

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
    // will now be done in GetItemSet();
    // pSet = &pSheet->GetItemSet();
    // nNumFmt = pDoc->GetFormatTable()->GetStandardFormat( NUMBERFORMAT_CURRENCY,
            //                                          ScGlobal::eLnge );
    // pSet->Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumFmt ) );

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
    aStr = SCSTR( STR_PAGE ); aStr += ' ';
    pEdEngine->SetText( aStr );
    nStrLen = aStr.Len();
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
    aBoxInfoItem.SetTable( sal_False );
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
    aStr = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(" ()"));
    pEdEngine->SetText( aStr );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxFileField(), EE_FEATURE_FIELD), ESelection(0,2,0,2) );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD), ESelection() );
    pTxtObj = pEdEngine->CreateTextObject();
    pHeaderItem->SetLeftArea( *pTxtObj );
    pHeaderItem->SetCenterArea( *pEmptyTxtObj );
    DELETEZ( pTxtObj );
    aStr = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    pEdEngine->SetText( aStr );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxTimeField(), EE_FEATURE_FIELD), ESelection(0,2,0,2) );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxDateField(Date(),SVXDATETYPE_VAR), EE_FEATURE_FIELD),
                                    ESelection() );
    pTxtObj = pEdEngine->CreateTextObject();
    pHeaderItem->SetRightArea( *pTxtObj );
    DELETEZ( pTxtObj );
    pSet->Put( *pHeaderItem );

    //----------------------------------------
    // Fusszeile:
    // [leer][Seite: \SEITE\ / \SEITEN\][leer]
    //----------------------------------------
    aStr = SCSTR( STR_PAGE ); aStr += ' ';
    nStrLen = aStr.Len();
    aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM(" / "));
    xub_StrLen nStrLen2 = aStr.Len();
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

//UNUSED2008-05  void ScStyleSheetPool::UpdateStdNames()
//UNUSED2008-05  {
//UNUSED2008-05      //  Standard-Styles den richtigen Namen in der Programm-Sprache geben
//UNUSED2008-05
//UNUSED2008-05      String aHelpFile;
//UNUSED2008-05      sal_uInt32 nCount = aStyles.size();
//UNUSED2008-05      for (sal_uInt32 n=0; n<nCount; n++)
//UNUSED2008-05      {
//UNUSED2008-05          SfxStyleSheetBase* pStyle = aStyles[n].get();
//UNUSED2008-05          if (!pStyle->IsUserDefined())
//UNUSED2008-05          {
//UNUSED2008-05              String aOldName     = pStyle->GetName();
//UNUSED2008-05              sal_uLong nHelpId       = pStyle->GetHelpId( aHelpFile );
//UNUSED2008-05              SfxStyleFamily eFam = pStyle->GetFamily();
//UNUSED2008-05
//UNUSED2008-05              sal_Bool bHelpKnown = sal_True;
//UNUSED2008-05              String aNewName;
//UNUSED2008-05              sal_uInt16 nNameId = 0;
//UNUSED2008-05              switch( nHelpId )
//UNUSED2008-05              {
//UNUSED2008-05                  case HID_SC_SHEET_CELL_STD:
//UNUSED2008-05                  case HID_SC_SHEET_PAGE_STD:  nNameId = STR_STYLENAME_STANDARD;  break;
//UNUSED2008-05                  case HID_SC_SHEET_CELL_ERG:  nNameId = STR_STYLENAME_RESULT;    break;
//UNUSED2008-05                  case HID_SC_SHEET_CELL_ERG1: nNameId = STR_STYLENAME_RESULT1;   break;
//UNUSED2008-05                  case HID_SC_SHEET_CELL_UEB:  nNameId = STR_STYLENAME_HEADLINE;  break;
//UNUSED2008-05                  case HID_SC_SHEET_CELL_UEB1: nNameId = STR_STYLENAME_HEADLINE1; break;
//UNUSED2008-05                  case HID_SC_SHEET_PAGE_REP:  nNameId = STR_STYLENAME_REPORT;    break;
//UNUSED2008-05                  default:
//UNUSED2008-05                      // 0 oder falsche (alte) HelpId
//UNUSED2008-05                      bHelpKnown = sal_False;
//UNUSED2008-05              }
//UNUSED2008-05              if (bHelpKnown)
//UNUSED2008-05              {
//UNUSED2008-05                  if ( nNameId )
//UNUSED2008-05                      aNewName = SCSTR( nNameId );
//UNUSED2008-05
//UNUSED2008-05                  if ( aNewName.Len() && aNewName != aOldName && !Find( aNewName, eFam ) )
//UNUSED2008-05                  {
//UNUSED2008-05                      DBG_TRACE( "Renaming style..." );
//UNUSED2008-05
//UNUSED2008-05                      pStyle->SetName( aNewName );    // setzt auch Parents um
//UNUSED2008-05
//UNUSED2008-05                      //  Styles in Patterns sind schon auf Pointer umgesetzt
//UNUSED2008-05                      if (eFam == SFX_STYLE_FAMILY_PAGE)
//UNUSED2008-05                      {
//UNUSED2008-05                          //  Page-Styles umsetzen
//UNUSED2008-05                          //  TableCount am Doc ist noch nicht initialisiert
//UNUSED2008-05                          for (SCTAB nTab=0; nTab<=MAXTAB && pDoc->HasTable(nTab); nTab++)
//UNUSED2008-05                              if (pDoc->GetPageStyle(nTab) == aOldName)
//UNUSED2008-05                                  pDoc->SetPageStyle(nTab, aNewName);
//UNUSED2008-05                      }
//UNUSED2008-05                  }
//UNUSED2008-05              }
//UNUSED2008-05              else
//UNUSED2008-05              {
//UNUSED2008-05                  //  wrong or no HelpId -> set new HelpId
//UNUSED2008-05
//UNUSED2008-05                  //  no assertion for wrong HelpIds because this happens
//UNUSED2008-05                  //  with old files (#67218#) or with old files that were
//UNUSED2008-05                  //  saved again with a new version in a different language
//UNUSED2008-05                  //  (so SrcVersion doesn't help)
//UNUSED2008-05
//UNUSED2008-05                  sal_uInt16 nNewId = 0;
//UNUSED2008-05                  if ( eFam == SFX_STYLE_FAMILY_PARA )
//UNUSED2008-05                  {
//UNUSED2008-05                      if ( aOldName == SCSTR( STR_STYLENAME_STANDARD ) )
//UNUSED2008-05                          nNewId = HID_SC_SHEET_CELL_STD;
//UNUSED2008-05                      else if ( aOldName == SCSTR( STR_STYLENAME_RESULT ) )
//UNUSED2008-05                          nNewId = HID_SC_SHEET_CELL_ERG;
//UNUSED2008-05                      else if ( aOldName == SCSTR( STR_STYLENAME_RESULT1 ) )
//UNUSED2008-05                          nNewId = HID_SC_SHEET_CELL_ERG1;
//UNUSED2008-05                      else if ( aOldName == SCSTR( STR_STYLENAME_HEADLINE ) )
//UNUSED2008-05                          nNewId = HID_SC_SHEET_CELL_UEB;
//UNUSED2008-05                      else if ( aOldName == SCSTR( STR_STYLENAME_HEADLINE1 ) )
//UNUSED2008-05                          nNewId = HID_SC_SHEET_CELL_UEB1;
//UNUSED2008-05                  }
//UNUSED2008-05                  else        // PAGE
//UNUSED2008-05                  {
//UNUSED2008-05                      if ( aOldName == SCSTR( STR_STYLENAME_STANDARD ) )
//UNUSED2008-05                          nNewId = HID_SC_SHEET_PAGE_STD;
//UNUSED2008-05                      else if ( aOldName == SCSTR( STR_STYLENAME_REPORT ) )
//UNUSED2008-05                          nNewId = HID_SC_SHEET_PAGE_REP;
//UNUSED2008-05                  }
//UNUSED2008-05
//UNUSED2008-05                  if ( nNewId )               // new ID found from name -> set ID
//UNUSED2008-05                  {
//UNUSED2008-05                      pStyle->SetHelpId( aHelpFile, nNewId );
//UNUSED2008-05                  }
//UNUSED2008-05                  else if ( nHelpId == 0 )    // no old and no new ID
//UNUSED2008-05                  {
//UNUSED2008-05                      //  #71471# probably user defined style without SFXSTYLEBIT_USERDEF set
//UNUSED2008-05                      //  (from StarCalc 1.0 import), fixed in src563 and above
//UNUSED2008-05                      //! may also be default style from a different language
//UNUSED2008-05                      //! test if name was generated from StarCalc 1.0 import?
//UNUSED2008-05                      DBG_ASSERT(pDoc->GetSrcVersion() <= SC_SUBTOTAL_BUGFIX,
//UNUSED2008-05                                  "user defined style without SFXSTYLEBIT_USERDEF");
//UNUSED2008-05                      pStyle->SetMask( pStyle->GetMask() | SFXSTYLEBIT_USERDEF );
//UNUSED2008-05                  }
//UNUSED2008-05                  // else: wrong old ID and no new ID found:
//UNUSED2008-05                  //  probably default style from a different language
//UNUSED2008-05                  //  -> leave unchanged (HelpId will be set if loaded with matching
//UNUSED2008-05                  //  language version later)
//UNUSED2008-05              }
//UNUSED2008-05          }
//UNUSED2008-05      }
//UNUSED2008-05  }

//------------------------------------------------------------------------

ScStyleSheet* ScStyleSheetPool::FindCaseIns( const String& rName, SfxStyleFamily eFam )
{
    String aUpSearch = rName;
    ScGlobal::pCharClass->toUpper(aUpSearch);

    sal_uInt32 nCount = aStyles.size();
    for (sal_uInt32 n=0; n<nCount; n++)
    {
        SfxStyleSheetBase* pStyle = aStyles[n].get();
        if ( pStyle->GetFamily() == eFam )
        {
            String aUpName = pStyle->GetName();
            ScGlobal::pCharClass->toUpper(aUpName);
            if (aUpName == aUpSearch)
                return (ScStyleSheet*)pStyle;
        }
    }

    return NULL;
}

