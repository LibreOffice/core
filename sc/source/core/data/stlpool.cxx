/*************************************************************************
 *
 *  $RCSfile: stlpool.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/flditem.hxx>
#include <svx/pageitem.hxx>
#include <svx/postitem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/wghtitem.hxx>
#include <svtools/itemset.hxx>
#include <svtools/zforlist.hxx>
#include <tools/intn.hxx>
#include <unotools/charclass.hxx>

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


//========================================================================

ScStyleSheetPool::ScStyleSheetPool( SfxItemPool&    rPool,
                                    ScDocument*     pDocument )
    :   SfxStyleSheetPool( rPool ),
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

void ScStyleSheetPool::SetForceStdName( const String* pSet )
{
    pForceStdName = pSet;
}

//------------------------------------------------------------------------

SfxStyleSheetBase& ScStyleSheetPool::Make( const String& rName,
                            SfxStyleFamily eFam, USHORT mask, USHORT nPos )
{
    //  When updating styles from a template, Office 5.1 sometimes created
    //  files with multiple default styles.
    //  Create new styles in that case:

    //! only when loading?

    if ( rName.EqualsAscii(STRING_STANDARD) && Find( rName, eFam ) != NULL )
    {
        DBG_ERROR("renaming additional default style");
        long nCount = aStyles.Count();
        for ( long nAdd = 1; nAdd <= nCount; nAdd++ )
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
                                            USHORT          nMask )
{
    ScStyleSheet* pSheet = new ScStyleSheet( rName, *this, eFamily, nMask );
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

void __EXPORT ScStyleSheetPool::Erase( SfxStyleSheetBase* pStyle )
{
    if ( pStyle )
    {
        DBG_ASSERT( IS_SET( SFXSTYLEBIT_USERDEF, pStyle->GetMask() ),
                    "SFXSTYLEBIT_USERDEF not set!" );

        ((ScDocumentPool&)rPool).StyleDeleted((ScStyleSheet*)pStyle);
        SfxStyleSheetPool::Erase(pStyle);
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

        if ( eFamily == SFX_STYLE_FAMILY_PAGE )
        {
            //  Set-Items

            const SfxPoolItem* pItem;

            if ( rSourceSet.GetItemState( ATTR_PAGE_HEADERSET, FALSE, &pItem ) == SFX_ITEM_SET )
            {
                const SfxItemSet& rSrcSub = ((const SvxSetItem*) pItem)->GetItemSet();
                SfxItemSet aDestSub( *rDestSet.GetPool(), rSrcSub.GetRanges() );
                aDestSub.PutExtended( rSrcSub, SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT );
                rDestSet.Put( SvxSetItem( ATTR_PAGE_HEADERSET, aDestSub ) );
            }
            if ( rSourceSet.GetItemState( ATTR_PAGE_FOOTERSET, FALSE, &pItem ) == SFX_ITEM_SET )
            {
                const SfxItemSet& rSrcSub = ((const SvxSetItem*) pItem)->GetItemSet();
                SfxItemSet aDestSub( *rDestSet.GetPool(), rSrcSub.GetRanges() );
                aDestSub.PutExtended( rSrcSub, SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT );
                rDestSet.Put( SvxSetItem( ATTR_PAGE_FOOTERSET, aDestSub ) );
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

void ScStyleSheetPool::CreateStandardStyles()
{
    //  neue Eintraege auch bei CopyStdStylesFrom eintragen

    Color           aColBlack   ( COL_BLACK );
    Color           aColGrey    ( COL_LIGHTGRAY );
    String          aStr;
    xub_StrLen      nStrLen;
    String          aHelpFile;//XXX JN welcher Text???
    ULONG           nNumFmt         = 0L;
    SfxItemSet*     pSet            = NULL;
    SfxItemSet*     pHFSet          = NULL;
    SvxSetItem*     pHFSetItem      = NULL;
    ScEditEngineDefaulter*  pEdEngine   = new ScEditEngineDefaulter( EditEngine::CreatePool(), TRUE );
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
    pSet = &pSheet->GetItemSet();
    nNumFmt = pDoc->GetFormatTable()->GetStandardFormat( NUMBERFORMAT_CURRENCY,
                                                        ScGlobal::eLnge );
    pSet->Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumFmt ) );

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
    pSet->Put( SvxOrientationItem( SVX_ORIENTATION_BOTTOMTOP, ATTR_ORIENTATION ) );

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
    pEdEngine->QuickInsertField( SvxFieldItem(SvxTableField()), ESelection() );
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
    pEdEngine->QuickInsertField( SvxFieldItem(SvxPageField()), ESelection(0,nStrLen,0,nStrLen) );
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
    aBoxInfoItem.SetValid( VALID_TOP, TRUE );
    aBoxInfoItem.SetValid( VALID_BOTTOM, TRUE );
    aBoxInfoItem.SetValid( VALID_LEFT, TRUE );
    aBoxInfoItem.SetValid( VALID_RIGHT, TRUE );
    aBoxInfoItem.SetValid( VALID_DISTANCE, TRUE );
    aBoxInfoItem.SetTable( FALSE );
    aBoxInfoItem.SetDist ( TRUE );

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
    pEdEngine->QuickInsertField( SvxFieldItem(SvxFileField()), ESelection(0,2,0,2) );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxTableField()), ESelection() );
    pTxtObj = pEdEngine->CreateTextObject();
    pHeaderItem->SetLeftArea( *pTxtObj );
    pHeaderItem->SetCenterArea( *pEmptyTxtObj );
    DELETEZ( pTxtObj );
    aStr = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    pEdEngine->SetText( aStr );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxTimeField()), ESelection(0,2,0,2) );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxDateField(Date(),SVXDATETYPE_VAR)),
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
    pEdEngine->QuickInsertField( SvxFieldItem(SvxPagesField()), ESelection(0,nStrLen2,0,nStrLen2) );
    pEdEngine->QuickInsertField( SvxFieldItem(SvxPageField()), ESelection(0,nStrLen,0,nStrLen) );
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

void ScStyleSheetPool::UpdateStdNames()
{
    //  Standard-Styles den richtigen Namen in der Programm-Sprache geben

    String aHelpFile;
    ULONG nCount = aStyles.Count();
    for (ULONG n=0; n<nCount; n++)
    {
        SfxStyleSheetBase* pStyle = aStyles.GetObject(n);
        if (!pStyle->IsUserDefined())
        {
            String aOldName     = pStyle->GetName();
            ULONG nHelpId       = pStyle->GetHelpId( aHelpFile );
            SfxStyleFamily eFam = pStyle->GetFamily();

            BOOL bHelpKnown = TRUE;
            String aNewName;
            USHORT nNameId = 0;
            switch( nHelpId )
            {
                case HID_SC_SHEET_CELL_STD:
                case HID_SC_SHEET_PAGE_STD:  nNameId = STR_STYLENAME_STANDARD;  break;
                case HID_SC_SHEET_CELL_ERG:  nNameId = STR_STYLENAME_RESULT;    break;
                case HID_SC_SHEET_CELL_ERG1: nNameId = STR_STYLENAME_RESULT1;   break;
                case HID_SC_SHEET_CELL_UEB:  nNameId = STR_STYLENAME_HEADLINE;  break;
                case HID_SC_SHEET_CELL_UEB1: nNameId = STR_STYLENAME_HEADLINE1; break;
                case HID_SC_SHEET_PAGE_REP:  nNameId = STR_STYLENAME_REPORT;    break;
                default:
                    // 0 oder falsche (alte) HelpId
                    bHelpKnown = FALSE;
            }
            if (bHelpKnown)
            {
                if ( nNameId )
                    aNewName = SCSTR( nNameId );

                if ( aNewName.Len() && aNewName != aOldName && !Find( aNewName, eFam ) )
                {
                    DBG_TRACE( "Renaming style..." );

                    pStyle->SetName( aNewName );    // setzt auch Parents um

                    //  Styles in Patterns sind schon auf Pointer umgesetzt
                    if (eFam == SFX_STYLE_FAMILY_PAGE)
                    {
                        //  Page-Styles umsetzen
                        //  TableCount am Doc ist noch nicht initialisiert
                        for (USHORT nTab=0; nTab<=MAXTAB && pDoc->HasTable(nTab); nTab++)
                            if (pDoc->GetPageStyle(nTab) == aOldName)
                                pDoc->SetPageStyle(nTab, aNewName);
                    }
                }
            }
            else
            {
                //  wrong or no HelpId -> set new HelpId

                //  no assertion for wrong HelpIds because this happens
                //  with old files (#67218#) or with old files that were
                //  saved again with a new version in a different language
                //  (so SrcVersion doesn't help)

                USHORT nNewId = 0;
                if ( eFam == SFX_STYLE_FAMILY_PARA )
                {
                    if ( aOldName == SCSTR( STR_STYLENAME_STANDARD ) )
                        nNewId = HID_SC_SHEET_CELL_STD;
                    else if ( aOldName == SCSTR( STR_STYLENAME_RESULT ) )
                        nNewId = HID_SC_SHEET_CELL_ERG;
                    else if ( aOldName == SCSTR( STR_STYLENAME_RESULT1 ) )
                        nNewId = HID_SC_SHEET_CELL_ERG1;
                    else if ( aOldName == SCSTR( STR_STYLENAME_HEADLINE ) )
                        nNewId = HID_SC_SHEET_CELL_UEB;
                    else if ( aOldName == SCSTR( STR_STYLENAME_HEADLINE1 ) )
                        nNewId = HID_SC_SHEET_CELL_UEB1;
                }
                else        // PAGE
                {
                    if ( aOldName == SCSTR( STR_STYLENAME_STANDARD ) )
                        nNewId = HID_SC_SHEET_PAGE_STD;
                    else if ( aOldName == SCSTR( STR_STYLENAME_REPORT ) )
                        nNewId = HID_SC_SHEET_PAGE_REP;
                }

                if ( nNewId )               // new ID found from name -> set ID
                {
                    pStyle->SetHelpId( aHelpFile, nNewId );
                }
                else if ( nHelpId == 0 )    // no old and no new ID
                {
                    //  #71471# probably user defined style without SFXSTYLEBIT_USERDEF set
                    //  (from StarCalc 1.0 import), fixed in src563 and above
                    //! may also be default style from a different language
                    //! test if name was generated from StarCalc 1.0 import?
                    DBG_ASSERT(pDoc->GetSrcVersion() <= SC_SUBTOTAL_BUGFIX,
                                "user defined style without SFXSTYLEBIT_USERDEF");
                    pStyle->SetMask( pStyle->GetMask() | SFXSTYLEBIT_USERDEF );
                }
                // else: wrong old ID and no new ID found:
                //  probably default style from a different language
                //  -> leave unchanged (HelpId will be set if loaded with matching
                //  language version later)
            }
        }
    }
}

//------------------------------------------------------------------------

ScStyleSheet* ScStyleSheetPool::FindCaseIns( const String& rName, SfxStyleFamily eFam )
{
    String aUpSearch = rName;
    ScGlobal::pCharClass->toUpper(aUpSearch);

    ULONG nCount = aStyles.Count();
    for (ULONG n=0; n<nCount; n++)
    {
        SfxStyleSheetBase* pStyle = aStyles.GetObject(n);
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




