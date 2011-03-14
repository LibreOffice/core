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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <hintids.hxx>
#include <i18npool/mslangid.hxx>
#include <unotools/localedatawrapper.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/emphitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <viewopt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <fmtfsize.hxx>
#include <poolfmt.hxx>
#include <paratr.hxx>
#include <pagedesc.hxx>
#include <frmtool.hxx>
#include <charfmt.hxx>
#include <docary.hxx>
#include <fmtcol.hxx>
#include <ndtxt.hxx>
#include <fmtline.hxx>
#include <poolfmt.hrc>
#include <GetMetricVal.hxx>
#include <numrule.hxx>


using namespace ::com::sun::star;

const sal_uInt16 PT_3   =  3 * 20;      //  3 pt
const sal_uInt16 PT_6   =  6 * 20;      //  6 pt
const sal_uInt16 PT_7   =  7 * 20;      //  6 pt
const sal_uInt16 PT_8   =  8 * 20;      //  8 pt
const sal_uInt16 PT_9   =  9 * 20;      //  9 pt
const sal_uInt16 PT_10  = 10 * 20;      // 10 pt
const sal_uInt16 PT_11  = 11 * 20;      // 11 pt
const sal_uInt16 PT_12  = 12 * 20;      // 12 pt
const sal_uInt16 PT_14  = 14 * 20;      // 14 pt
const sal_uInt16 PT_16  = 16 * 20;      // 16 pt
const sal_uInt16 PT_18  = 18 * 20;      // 18 pt
const sal_uInt16 PT_22  = 22 * 20;      // 22 pt
const sal_uInt16 PT_24  = 24 * 20;      // 22 pt


//const sal_uInt16 HTML_PARSPACE = ((CM_05 * 7) / 10);
#define HTML_PARSPACE   GetMetricVal( CM_05 )

static const sal_Char sKomma[] = ", ";

static const sal_uInt16 aHeadlineSizes[ 2 * MAXLEVEL ] = {
//  PT_16, PT_14, PT_14, PT_12, PT_12,          // normal
//JP 10.12.96: jetzt soll alles prozentual sein:
    115, 100, 100, 85, 85,
    75,   75,  75, 75, 75,  // normal
//  PT_22, PT_16, PT_12, PT_11, PT_9            // HTML-Mode
    PT_24, PT_18, PT_14, PT_12, PT_10,
    PT_7, PT_7, PT_7, PT_7, PT_7            // HTML-Mode
};

long lcl_GetRightMargin( SwDoc& rDoc )
{
    // sorge dafuer, dass die Druckereinstellungen in die Standard-
    // Seitenvorlage uebernommen wurden.
    const SwFrmFmt& rPgDscFmt =
            const_cast<const SwDoc *>(&rDoc)->GetPageDesc( 0 ).GetMaster();
    const SvxLRSpaceItem& rLR = rPgDscFmt.GetLRSpace();
    const long nLeft = rLR.GetLeft();
    const long nRight = rLR.GetRight();
    const long nWidth = rPgDscFmt.GetFrmSize().GetWidth();
    return nWidth - nLeft - nRight;
}

void SetAllScriptItem( SfxItemSet& rSet, const SfxPoolItem& rItem )
{
    rSet.Put( rItem );
    sal_uInt16 nWhCJK = 0, nWhCTL = 0;
    switch( rItem.Which() )
    {
    case RES_CHRATR_FONTSIZE:
        nWhCJK = RES_CHRATR_CJK_FONTSIZE, nWhCTL = RES_CHRATR_CTL_FONTSIZE;
        break;
    case RES_CHRATR_FONT:
        nWhCJK = RES_CHRATR_CJK_FONT, nWhCTL = RES_CHRATR_CTL_FONT;
        break;
    case RES_CHRATR_LANGUAGE:
        nWhCJK = RES_CHRATR_CJK_LANGUAGE, nWhCTL = RES_CHRATR_CTL_LANGUAGE;
        break;
    case RES_CHRATR_POSTURE:
        nWhCJK = RES_CHRATR_CJK_POSTURE, nWhCTL = RES_CHRATR_CTL_POSTURE;
        break;
    case RES_CHRATR_WEIGHT:
        nWhCJK = RES_CHRATR_CJK_WEIGHT, nWhCTL = RES_CHRATR_CTL_WEIGHT;
        break;
    }

    if( nWhCJK )
        rSet.Put( rItem, nWhCJK );
    if( nWhCTL )
        rSet.Put( rItem, nWhCTL );
}

void lcl_SetDfltFont( sal_uInt16 nFntType, SfxItemSet& rSet )
{
    static struct {
        sal_uInt16 nResLngId;
        sal_uInt16 nResFntId;
    } aArr[ 3 ] = {
        { RES_CHRATR_LANGUAGE, RES_CHRATR_FONT },
        { RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_FONT },
        { RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_FONT }
    };
    for( sal_uInt16 n = 0; n < 3; ++n )
    {
        sal_uInt16 nLng = ((SvxLanguageItem&)rSet.GetPool()->GetDefaultItem(
                            aArr[n].nResLngId )).GetLanguage();
        Font aFnt( OutputDevice::GetDefaultFont( nFntType,
                                nLng, DEFAULTFONT_FLAGS_ONLYONE ) );

        rSet.Put( SvxFontItem( aFnt.GetFamily(), aFnt.GetName(),
                            aEmptyStr, aFnt.GetPitch(),
                            aFnt.GetCharSet(), aArr[n].nResFntId ));
    }
}

void lcl_SetDfltFont( sal_uInt16 nLatinFntType, sal_uInt16 nCJKFntType,
                        sal_uInt16 nCTLFntType, SfxItemSet& rSet )
{
    static struct {
        sal_uInt16 nResLngId;
        sal_uInt16 nResFntId;
        sal_uInt16 nFntType;
    } aArr[ 3 ] = {
        { RES_CHRATR_LANGUAGE, RES_CHRATR_FONT, 0 },
        { RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_FONT, 0 },
        { RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_FONT, 0 }
    };
    aArr[0].nFntType = nLatinFntType;
    aArr[1].nFntType = nCJKFntType;
    aArr[2].nFntType = nCTLFntType;

    for( sal_uInt16 n = 0; n < 3; ++n )
    {
        sal_uInt16 nLng = ((SvxLanguageItem&)rSet.GetPool()->GetDefaultItem(
                            aArr[n].nResLngId )).GetLanguage();
        Font aFnt( OutputDevice::GetDefaultFont( aArr[n].nFntType,
                                nLng, DEFAULTFONT_FLAGS_ONLYONE ) );

        rSet.Put( SvxFontItem( aFnt.GetFamily(), aFnt.GetName(),
                            aEmptyStr, aFnt.GetPitch(),
                            aFnt.GetCharSet(), aArr[n].nResFntId ));
    }
}

void lcl_SetHeadline( SwDoc* pDoc, SwTxtFmtColl* pColl,
                        SfxItemSet& rSet,
                        sal_uInt16 nOutLvlBits, sal_uInt8 nLevel, sal_Bool bItalic )
{
    SetAllScriptItem( rSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
    SvxFontHeightItem aHItem(240, 100, RES_CHRATR_FONTSIZE);
    const bool bHTMLMode = pDoc->get(IDocumentSettingAccess::HTML_MODE);
    if( bHTMLMode )
        aHItem.SetHeight( aHeadlineSizes[ MAXLEVEL + nLevel ] );
    else
        aHItem.SetHeight( PT_14, aHeadlineSizes[ nLevel ] );
    SetAllScriptItem( rSet, aHItem );

    if( bItalic && !bHTMLMode )
        SetAllScriptItem( rSet, SvxPostureItem( ITALIC_NORMAL, RES_CHRATR_POSTURE ) );

    if( bHTMLMode )
    {
        ::lcl_SetDfltFont( DEFAULTFONT_LATIN_TEXT, DEFAULTFONT_CJK_TEXT,
                            DEFAULTFONT_CTL_TEXT, rSet );
    }

    if( pColl )
    {
        if( !( nOutLvlBits & ( 1 << nLevel )) )
        {
            pColl->AssignToListLevelOfOutlineStyle(nLevel);//<-end,zhaojianwei
            if( !bHTMLMode )
            {
                SwNumRule * pOutlineRule = pDoc->GetOutlineNumRule();
                const SwNumFmt& rNFmt = pOutlineRule->Get( nLevel );

                if ( rNFmt.GetPositionAndSpaceMode() ==
                                    SvxNumberFormat::LABEL_WIDTH_AND_POSITION &&
                     ( rNFmt.GetAbsLSpace() || rNFmt.GetFirstLineOffset() ) )
                {
                    SvxLRSpaceItem aLR( (SvxLRSpaceItem&)pColl->GetFmtAttr( RES_LR_SPACE ) );
                    aLR.SetTxtFirstLineOfstValue( rNFmt.GetFirstLineOffset() );
                    aLR.SetTxtLeft( rNFmt.GetAbsLSpace() );
                    pColl->SetFmtAttr( aLR );
                }

                // #i71764#
                // Check on document setting OUTLINE_LEVEL_YIELDS_OUTLINE_RULE no longer needed.
                // All paragraph styles, which are assigned to a level of the
                // outline style has to have the outline style set as its list style.
                {
                    SwNumRuleItem aItem(pOutlineRule->GetName());

                    pColl->SetFmtAttr(aItem);
                }
                // <--
            }
        }
        pColl->SetNextTxtFmtColl( *pDoc->GetTxtCollFromPool(
                                        RES_POOLCOLL_TEXT ));
    }
}



void lcl_SetRegister( SwDoc* pDoc, SfxItemSet& rSet, sal_uInt16 nFact,
                        sal_Bool bHeader, sal_Bool bTab )
{
    SvxLRSpaceItem aLR( RES_LR_SPACE );
    sal_uInt16 nLeft = nFact ? GetMetricVal( CM_05 ) * nFact : 0;
    aLR.SetTxtLeft( nLeft );

    rSet.Put( aLR );
    if( bHeader )
    {
        SetAllScriptItem( rSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
        SetAllScriptItem( rSet, SvxFontHeightItem( PT_16, 100, RES_CHRATR_FONTSIZE ) );
    }
    if( bTab )
    {
        long nRightMargin = lcl_GetRightMargin( *pDoc );
        SvxTabStopItem aTStops( 0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
        aTStops.Insert( SvxTabStop( nRightMargin - nLeft,
                                    SVX_TAB_ADJUST_RIGHT,
                                    cDfltDecimalChar, '.' ));
        rSet.Put( aTStops );
    }
}



void lcl_SetNumBul( SwDoc* pDoc, SwTxtFmtColl* pColl,
                        SfxItemSet& rSet,
                        sal_uInt16 nNxt, SwTwips nEZ, SwTwips nLeft,
                        SwTwips nUpper, SwTwips nLower )
{

    SvxLRSpaceItem aLR( RES_LR_SPACE ); SvxULSpaceItem aUL( RES_UL_SPACE );
    aLR.SetTxtFirstLineOfst( sal_uInt16(nEZ) ); aLR.SetTxtLeft( sal_uInt16(nLeft) );
    aUL.SetUpper( sal_uInt16(nUpper) ); aUL.SetLower( sal_uInt16(nLower) );
    rSet.Put( aLR );
    rSet.Put( aUL );

    if( pColl )
        pColl->SetNextTxtFmtColl( *pDoc->GetTxtCollFromPool( nNxt ));
}



// Gebe die "Auto-Collection" mit der Id zurueck. Existiert
// sie noch nicht, dann erzeuge sie
// Ist der String-Pointer definiert, dann erfrage nur die
// Beschreibung der Attribute, !! es legt keine Vorlage an !!

SvxFrameDirection GetDefaultFrameDirection(sal_uLong nLanguage)
{
    SvxFrameDirection eResult = (MsLangId::isRightToLeft( static_cast<LanguageType>(nLanguage)) ?
            FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP);
    return eResult;
}

SwTxtFmtColl* SwDoc::GetTxtCollFromPool( sal_uInt16 nId, bool bRegardLanguage )
{
    OSL_ENSURE(
        (RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END) ||
        (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END) ||
        (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END) ||
        (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END) ||
        (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END) ||
        (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END),
            "Falsche AutoFormat-Id" );

    SwTxtFmtColl* pNewColl;
    sal_uInt16 nOutLvlBits = 0;
    for( sal_uInt16 n = 0; n < pTxtFmtCollTbl->Count(); ++n )
    {
        if( nId == ( pNewColl = (*pTxtFmtCollTbl)[ n ] )->GetPoolFmtId() )
        {
            return pNewColl;
        }

        if( pNewColl->IsAssignedToListLevelOfOutlineStyle())
            nOutLvlBits |= ( 1 << pNewColl->GetAssignedOutlineStyleLevel() );//<-end,zhaojianwei
    }

    // bis hierher nicht gefunden -> neu anlegen
    sal_uInt16 nResId = 0;
    if( RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END )
        nResId = RC_POOLCOLL_TEXT_BEGIN - RES_POOLCOLL_TEXT_BEGIN;
    else if (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END)
        nResId = RC_POOLCOLL_LISTS_BEGIN - RES_POOLCOLL_LISTS_BEGIN;
    else if (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END)
        nResId = RC_POOLCOLL_EXTRA_BEGIN - RES_POOLCOLL_EXTRA_BEGIN;
    else if (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END)
        nResId = RC_POOLCOLL_REGISTER_BEGIN - RES_POOLCOLL_REGISTER_BEGIN;
    else if (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END)
        nResId = RC_POOLCOLL_DOC_BEGIN - RES_POOLCOLL_DOC_BEGIN;
    else if (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END)
        nResId = RC_POOLCOLL_HTML_BEGIN - RES_POOLCOLL_HTML_BEGIN;

    OSL_ENSURE( nResId, "Ungueltige Pool-ID" );
    if( !nResId )
        return GetTxtCollFromPool( RES_POOLCOLL_STANDARD );

    ResId aResId( nResId + nId, *pSwResMgr );
    String aNm( aResId );

    // ein Set fuer alle zusetzenden Attribute
    SwAttrSet aSet( GetAttrPool(), aTxtFmtCollSetRange );
    sal_uInt16 nParent = GetPoolParent( nId );

    {

//FEATURE::CONDCOLL
        if(::IsConditionalByPoolId( nId ))
            pNewColl = new SwConditionTxtFmtColl( GetAttrPool(), aNm, !nParent
                                                ? pDfltTxtFmtColl
                                                : GetTxtCollFromPool( nParent ));
        else
//FEATURE::CONDCOLL
        pNewColl = new SwTxtFmtColl( GetAttrPool(), aNm, !nParent
                                            ? pDfltTxtFmtColl
                                            : GetTxtCollFromPool( nParent ));
        pNewColl->SetPoolFmtId( nId );
        pTxtFmtCollTbl->Insert( pNewColl, pTxtFmtCollTbl->Count() );
    }

    switch( nId )
    {
    // allgemeine Inhaltsformen
    case RES_POOLCOLL_STANDARD:
        /* koreans do not like SvxScriptItem(TRUE) */
        if (bRegardLanguage)
        {
            sal_uLong nAppLanguage = GetAppLanguage();
            if (GetDefaultFrameDirection(nAppLanguage) ==
                FRMDIR_HORI_RIGHT_TOP)
            {
                SvxAdjustItem aAdjust(SVX_ADJUST_RIGHT, RES_PARATR_ADJUST );
                aSet.Put(aAdjust);
            }
            if (nAppLanguage == LANGUAGE_KOREAN)
            {
                SvxScriptSpaceItem aScriptSpace(sal_False, RES_PARATR_SCRIPTSPACE);
                aSet.Put(aScriptSpace);
            }
        }
        break;

    case RES_POOLCOLL_TEXT:                 // Textkoerper
        {
            SvxULSpaceItem aUL( 0, PT_6, RES_UL_SPACE );
            if( get(IDocumentSettingAccess::HTML_MODE) ) aUL.SetLower( HTML_PARSPACE );
            aSet.Put( aUL );
        }
        break;
    case RES_POOLCOLL_TEXT_IDENT:           // Textkoerper Einzug
        {
            SvxLRSpaceItem aLR( RES_LR_SPACE );
            aLR.SetTxtFirstLineOfst( GetMetricVal( CM_05 ));
            aSet.Put( aLR );
        }
        break;
    case RES_POOLCOLL_TEXT_NEGIDENT:        // Textkoerper neg. Einzug
        {
            SvxLRSpaceItem aLR( RES_LR_SPACE );
            aLR.SetTxtFirstLineOfst( -(short)GetMetricVal( CM_05 ));
            aLR.SetTxtLeft( GetMetricVal( CM_1 ));
            SvxTabStopItem aTStops(RES_PARATR_TABSTOP);    aTStops.Insert( SvxTabStop( 0 ));

            aSet.Put( aLR );
            aSet.Put( aTStops );
        }
        break;
    case RES_POOLCOLL_TEXT_MOVE:            // Textkoerper Einrueckung
        {
            SvxLRSpaceItem aLR( RES_LR_SPACE );
            aLR.SetTxtLeft( GetMetricVal( CM_05 ));
            aSet.Put( aLR );
        }
        break;

    case RES_POOLCOLL_CONFRONTATION:    // Textkoerper Gegenueberstellung
        {
            SvxLRSpaceItem aLR( RES_LR_SPACE );
            aLR.SetTxtFirstLineOfst( - short( GetMetricVal( CM_1 ) * 4 +
                                              GetMetricVal( CM_05)) );
            aLR.SetTxtLeft( GetMetricVal( CM_1 ) * 5 );
            SvxTabStopItem aTStops( RES_PARATR_TABSTOP );    aTStops.Insert( SvxTabStop( 0 ));

            aSet.Put( aLR );
            aSet.Put( aTStops );
        }
        break;
    case RES_POOLCOLL_MARGINAL:         // Textkoerper maginalie
        {
            SvxLRSpaceItem aLR( RES_LR_SPACE );
            aLR.SetTxtLeft( GetMetricVal( CM_1 ) * 4 );
            aSet.Put( aLR );
        }
        break;

    case RES_POOLCOLL_HEADLINE_BASE:            // Basis Ueberschrift
        {
            static const sal_uInt16 aFntInit[] = {
                DEFAULTFONT_LATIN_HEADING,  RES_CHRATR_FONT,
                                RES_CHRATR_LANGUAGE, LANGUAGE_ENGLISH_US,
                DEFAULTFONT_CJK_HEADING,    RES_CHRATR_CJK_FONT,
                                RES_CHRATR_CJK_LANGUAGE, LANGUAGE_ENGLISH_US,
                DEFAULTFONT_CTL_HEADING,    RES_CHRATR_CTL_FONT,
                                RES_CHRATR_CTL_LANGUAGE, LANGUAGE_ARABIC_SAUDI_ARABIA,
                0
            };

            for( const sal_uInt16* pArr = aFntInit; *pArr; pArr += 4 )
            {
                sal_uInt16 nLng = ((SvxLanguageItem&)GetDefault( *(pArr+2) )).GetLanguage();
                if( LANGUAGE_DONTKNOW == nLng )
                    nLng = *(pArr+3);

                Font aFnt( OutputDevice::GetDefaultFont( *pArr,
                                        nLng, DEFAULTFONT_FLAGS_ONLYONE ) );

                aSet.Put( SvxFontItem( aFnt.GetFamily(), aFnt.GetName(),
                                        aEmptyStr, aFnt.GetPitch(),
                                        aFnt.GetCharSet(), *(pArr+1) ));
            }

            SvxFontHeightItem aFntSize( PT_14, 100, RES_CHRATR_FONTSIZE );
            SvxULSpaceItem aUL( PT_12, PT_6, RES_UL_SPACE );
            if( get(IDocumentSettingAccess::HTML_MODE) )
                aUL.SetLower( HTML_PARSPACE );
            aSet.Put( SvxFmtKeepItem( sal_True, RES_KEEP ));

            pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool( RES_POOLCOLL_TEXT ));

            aSet.Put( aUL );
            SetAllScriptItem( aSet, aFntSize );
        }
        break;

    case RES_POOLCOLL_NUMBUL_BASE:          // Basis Numerierung/Aufzaehlung
        break;

    case RES_POOLCOLL_GREETING:             // Grussformel
    case RES_POOLCOLL_REGISTER_BASE:        // Basis Verzeichnisse
    case RES_POOLCOLL_SIGNATURE:            // Unterschrift
    case RES_POOLCOLL_TABLE:                // Tabelle-Inhalt
        {
            SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
            aSet.Put( aLN );
        }
        break;

    case RES_POOLCOLL_HEADLINE1:        // Ueberschrift 1
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 0, sal_False );
        break;
    case RES_POOLCOLL_HEADLINE2:        // Ueberschrift 2
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 1, sal_True );
        break;
    case RES_POOLCOLL_HEADLINE3:        // Ueberschrift 3
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 2, sal_False );
        break;
    case RES_POOLCOLL_HEADLINE4:        // Ueberschrift 4
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 3, sal_True );
        break;
    case RES_POOLCOLL_HEADLINE5:        // Ueberschrift 5
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 4, sal_False );
        break;
    case RES_POOLCOLL_HEADLINE6:        // Ueberschrift 6
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 5, sal_False );
        break;
    case RES_POOLCOLL_HEADLINE7:        // Ueberschrift 7
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 6, sal_False );
        break;
    case RES_POOLCOLL_HEADLINE8:        // Ueberschrift 8
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 7, sal_False );
        break;
    case RES_POOLCOLL_HEADLINE9:        // Ueberschrift 9
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 8, sal_False );
        break;
    case RES_POOLCOLL_HEADLINE10:       // Ueberschrift 10
        lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 9, sal_False );
        break;


    // Sonderbereiche:
    // Kopfzeilen
    case RES_POOLCOLL_HEADER:
    case RES_POOLCOLL_HEADERL:
    case RES_POOLCOLL_HEADERR:
    // Fusszeilen
    case RES_POOLCOLL_FOOTER:
    case RES_POOLCOLL_FOOTERL:
    case RES_POOLCOLL_FOOTERR:
        {
            SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
            aSet.Put( aLN );

            long nRightMargin = lcl_GetRightMargin( *this );

            SvxTabStopItem aTStops( 0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
            aTStops.Insert( SvxTabStop( nRightMargin / 2, SVX_TAB_ADJUST_CENTER ) );
            aTStops.Insert( SvxTabStop( nRightMargin, SVX_TAB_ADJUST_RIGHT ) );

            aSet.Put( aTStops );
        }
        break;

    case RES_POOLCOLL_TABLE_HDLN:
        {
            SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
            aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER, RES_PARATR_ADJUST ) );
            SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
            aSet.Put( aLN );
        }
        break;

    case RES_POOLCOLL_FOOTNOTE:             // Fussnote
    case RES_POOLCOLL_ENDNOTE:
        {
            SvxLRSpaceItem aLR( RES_LR_SPACE );
            aLR.SetTxtFirstLineOfst( -(short)GetMetricVal( CM_05 ));
            aLR.SetTxtLeft( GetMetricVal( CM_05 ));
            SetAllScriptItem( aSet, SvxFontHeightItem( PT_10, 100, RES_CHRATR_FONTSIZE ) );
            aSet.Put( aLR );
            SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
            aSet.Put( aLN );
        }
        break;

    case RES_POOLCOLL_LABEL:                // Beschriftung-Basis
        {
            SvxULSpaceItem aUL( RES_UL_SPACE ); aUL.SetUpper( PT_6 ); aUL.SetLower( PT_6 );
            aSet.Put( aUL );
            SetAllScriptItem( aSet, SvxPostureItem( ITALIC_NORMAL, RES_CHRATR_POSTURE ) );
            SetAllScriptItem( aSet, SvxFontHeightItem( PT_10, 100, RES_CHRATR_FONTSIZE ) );
            SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
            aSet.Put( aLN );
        }
        break;

    case RES_POOLCOLL_FRAME:                // Rahmen Inhalt
    case RES_POOLCOLL_LABEL_ABB:            // Beschriftung-Abbildung
    case RES_POOLCOLL_LABEL_TABLE:          // Beschriftung-Tabelle
    case RES_POOLCOLL_LABEL_FRAME:          // Beschriftung-Rahmen
    case RES_POOLCOLL_LABEL_DRAWING:        // Beschriftung-Zeichnung
        break;

    case RES_POOLCOLL_JAKETADRESS:          // UmschlagAdresse
        {
            SvxULSpaceItem aUL( RES_UL_SPACE ); aUL.SetLower( PT_3 );
            aSet.Put( aUL );
            SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
            aSet.Put( aLN );
        }
        break;

    case RES_POOLCOLL_SENDADRESS:           // AbsenderAdresse
        {
            if( get(IDocumentSettingAccess::HTML_MODE) )
                SetAllScriptItem( aSet, SvxPostureItem(ITALIC_NORMAL, RES_CHRATR_POSTURE) );
            else
            {
                SvxULSpaceItem aUL( RES_UL_SPACE ); aUL.SetLower( PT_3 );
                aSet.Put( aUL );
            }
            SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
            aSet.Put( aLN );
        }
        break;

    // Benutzer-Verzeichnisse:
    case RES_POOLCOLL_TOX_USERH:            // Header
        lcl_SetRegister( this, aSet, 0, sal_True, sal_False );
        {
            SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
            aSet.Put( aLN );
        }
        break;
    case RES_POOLCOLL_TOX_USER1:            // 1. Ebene
        lcl_SetRegister( this, aSet, 0, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_USER2:            // 2. Ebene
        lcl_SetRegister( this, aSet, 1, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_USER3:            // 3. Ebene
        lcl_SetRegister( this, aSet, 2, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_USER4:            // 4. Ebene
        lcl_SetRegister( this, aSet, 3, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_USER5:            // 5. Ebene
        lcl_SetRegister( this, aSet, 4, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_USER6:            // 6. Ebene
        lcl_SetRegister( this, aSet, 5, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_USER7:            // 7. Ebene
        lcl_SetRegister( this, aSet, 6, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_USER8:            // 8. Ebene
        lcl_SetRegister( this, aSet, 7, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_USER9:            // 9. Ebene
        lcl_SetRegister( this, aSet, 8, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_USER10:           // 10. Ebene
        lcl_SetRegister( this, aSet, 9, sal_False, sal_True );
        break;

    // Index-Verzeichnisse
    case RES_POOLCOLL_TOX_IDXH:         // Header
        lcl_SetRegister( this, aSet, 0, sal_True, sal_False );
        {
            SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
            aSet.Put( aLN );
        }
        break;
    case RES_POOLCOLL_TOX_IDX1:         // 1. Ebene
        lcl_SetRegister( this, aSet, 0, sal_False, sal_False );
        break;
    case RES_POOLCOLL_TOX_IDX2:         // 2. Ebene
        lcl_SetRegister( this, aSet, 1, sal_False, sal_False );
        break;
    case RES_POOLCOLL_TOX_IDX3:         // 3. Ebene
        lcl_SetRegister( this, aSet, 2, sal_False, sal_False );
        break;
    case RES_POOLCOLL_TOX_IDXBREAK:     // Trenner
        lcl_SetRegister( this, aSet, 0, sal_False, sal_False );
        break;

    // Inhalts-Verzeichnisse
    case RES_POOLCOLL_TOX_CNTNTH:       // Header
        lcl_SetRegister( this, aSet, 0, sal_True, sal_False );
        {
            SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
            aSet.Put( aLN );
        }
        break;
    case RES_POOLCOLL_TOX_CNTNT1:       // 1. Ebene
        lcl_SetRegister( this, aSet, 0, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_CNTNT2:       // 2. Ebene
        lcl_SetRegister( this, aSet, 1, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_CNTNT3:       // 3. Ebene
        lcl_SetRegister( this, aSet, 2, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_CNTNT4:       // 4. Ebene
        lcl_SetRegister( this, aSet, 3, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_CNTNT5:       // 5. Ebene
        lcl_SetRegister( this, aSet, 4, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_CNTNT6:       // 6. Ebene
        lcl_SetRegister( this, aSet, 5, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_CNTNT7:       // 7. Ebene
        lcl_SetRegister( this, aSet, 6, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_CNTNT8:       // 8. Ebene
        lcl_SetRegister( this, aSet, 7, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_CNTNT9:       // 9. Ebene
        lcl_SetRegister( this, aSet, 8, sal_False, sal_True );
        break;
    case RES_POOLCOLL_TOX_CNTNT10:      // 10. Ebene
        lcl_SetRegister( this, aSet, 9, sal_False, sal_True );
        break;

    case RES_POOLCOLL_TOX_ILLUSH:
    case RES_POOLCOLL_TOX_OBJECTH:
    case RES_POOLCOLL_TOX_TABLESH:
    case RES_POOLCOLL_TOX_AUTHORITIESH:
        lcl_SetRegister( this, aSet, 0, sal_True, sal_False );
        {
            SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
            aSet.Put( aLN );
        }
        break;
    case RES_POOLCOLL_TOX_ILLUS1:
    case RES_POOLCOLL_TOX_OBJECT1:
    case RES_POOLCOLL_TOX_TABLES1:
    case RES_POOLCOLL_TOX_AUTHORITIES1:
        lcl_SetRegister( this, aSet, 0, sal_False, sal_True );
    break;



    case RES_POOLCOLL_NUM_LEVEL1S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL1:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL1E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_NUM_NONUM1:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM1,
                        0, SwNumRule::GetNumIndent( 0 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL2S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL2:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL2E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_NUM_NONUM2:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM2,
                        0, SwNumRule::GetNumIndent( 1 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL3S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL3:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL3E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_NUM_NONUM3:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM3,
                        0, SwNumRule::GetNumIndent( 2 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL4S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL4:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL4E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_NUM_NONUM4:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM4,
                        0, SwNumRule::GetNumIndent( 3 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL5S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL5:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_NUM_LEVEL5E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
                        lNumFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_NUM_NONUM5:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM5,
                        0, SwNumRule::GetNumIndent( 4 ), 0, PT_6 );
        break;

    case RES_POOLCOLL_BUL_LEVEL1S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL1,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL1:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL1,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL1E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL1,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_BUL_NONUM1:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM1,
                        0, SwNumRule::GetBullIndent( 0 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL2S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL2,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL2:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL2,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL2E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL2,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_BUL_NONUM2:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM2,
                        0, SwNumRule::GetBullIndent( 1 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL3S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL3,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL3:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL3,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL3E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL3,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_BUL_NONUM3:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM3,
                        0, SwNumRule::GetBullIndent( 2 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL4S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL4,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL4:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL4,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL4E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL4,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_BUL_NONUM4:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM4,
                        0, SwNumRule::GetBullIndent( 3 ), 0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL5S:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL5,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
                        PT_12, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL5:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL5,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
                        0, PT_6 );
        break;
    case RES_POOLCOLL_BUL_LEVEL5E:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL5,
                        lBullFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
                        0, PT_12 );
        break;
    case RES_POOLCOLL_BUL_NONUM5:
        lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM5,
                        0, SwNumRule::GetBullIndent( 4 ), 0, PT_6 );
        break;

    case RES_POOLCOLL_DOC_TITEL:            // Doc. Titel
        {
            SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
            SetAllScriptItem( aSet, SvxFontHeightItem( PT_18, 100, RES_CHRATR_FONTSIZE ) );

            aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER, RES_PARATR_ADJUST ) );

            pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
                                                RES_POOLCOLL_DOC_SUBTITEL ));
        }
        break;

    case RES_POOLCOLL_DOC_SUBTITEL:         // Doc. UnterTitel
        {
            SetAllScriptItem( aSet, SvxPostureItem( ITALIC_NORMAL, RES_CHRATR_POSTURE ));
            SetAllScriptItem( aSet, SvxFontHeightItem( PT_14, 100, RES_CHRATR_FONTSIZE ));

            aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER, RES_PARATR_ADJUST ));

            pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
                                                RES_POOLCOLL_TEXT ));
        }
        break;

    case RES_POOLCOLL_HTML_BLOCKQUOTE:
        {
            SvxLRSpaceItem aLR( RES_LR_SPACE );
            aLR.SetLeft( GetMetricVal( CM_1 ));
            aLR.SetRight( GetMetricVal( CM_1 ));
            aSet.Put( aLR );
            SvxULSpaceItem aUL( RES_UL_SPACE );
            aUL = pNewColl->GetULSpace();
            aUL.SetLower( HTML_PARSPACE );
            aSet.Put( aUL);
        }
        break;

    case RES_POOLCOLL_HTML_PRE:
        {
            ::lcl_SetDfltFont( DEFAULTFONT_FIXED, aSet );

// WORKAROUND: PRE auf 10pt setzten
            SetAllScriptItem( aSet, SvxFontHeightItem(PT_10, 100, RES_CHRATR_FONTSIZE) );
// WORKAROUND: PRE auf 10pt setzten

            // der untere Absatz-Abstand wird explizit gesetzt (macht
            // die harte Attributierung einfacher)
            SvxULSpaceItem aULSpaceItem( RES_UL_SPACE );
            aULSpaceItem = pNewColl->GetULSpace();
            aULSpaceItem.SetLower( 0 );
            aSet.Put( aULSpaceItem );
        }
        break;

    case RES_POOLCOLL_HTML_HR:
        {
            SvxBoxItem aBox( RES_BOX );
            Color aColor( COL_GRAY );
            SvxBorderLine aNew( &aColor, DEF_DOUBLE_LINE0_OUT,
                                         DEF_DOUBLE_LINE0_IN,
                                         DEF_DOUBLE_LINE0_DIST );
            aBox.SetLine( &aNew, BOX_LINE_BOTTOM );

            aSet.Put( aBox );
            aSet.Put( SwParaConnectBorderItem( sal_False ) );
            SetAllScriptItem( aSet, SvxFontHeightItem(120, 100, RES_CHRATR_FONTSIZE) );

            SvxULSpaceItem aUL( RES_UL_SPACE );
            {
                pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
                                                RES_POOLCOLL_TEXT ));
                aUL = pNewColl->GetULSpace();
            }
            aUL.SetLower( HTML_PARSPACE );
            aSet.Put( aUL);
            SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
            aSet.Put( aLN );
        }
        break;

    case RES_POOLCOLL_HTML_DD:
        {
            SvxLRSpaceItem aLR( RES_LR_SPACE );
            aLR = pNewColl->GetLRSpace();
            // es wird um 1cm eingerueckt. Die IDs liegen immer 2 auseinander!
            aLR.SetLeft( GetMetricVal( CM_1 ));
            aSet.Put( aLR );
        }
        break;
    case RES_POOLCOLL_HTML_DT:
        {
            SvxLRSpaceItem aLR( RES_LR_SPACE );
            {
                pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
                                                    RES_POOLCOLL_HTML_DD ));
                aLR = pNewColl->GetLRSpace();
            }
            // es wird um 0cm eingerueckt. Die IDs liegen immer 2 auseinander!
            aLR.SetLeft( 0 );
            aSet.Put( aLR );
        }
        break;
    }

    if( aSet.Count() )
    {
        {
            pNewColl->SetFmtAttr( aSet );
        }
    }
    return pNewColl;
}



    // pruefe, ob diese "Auto-Collection" in Dokument schon/noch
    // benutzt wird
bool SwDoc::IsPoolTxtCollUsed( sal_uInt16 nId ) const
{
    OSL_ENSURE(
        (RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END) ||
        (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END) ||
        (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END) ||
        (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END) ||
        (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END) ||
        (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END),
            "Falsche AutoFormat-Id" );

    SwTxtFmtColl* pNewColl = 0;
    sal_Bool bFnd = sal_False;
    for( sal_uInt16 n = 0; !bFnd && n < pTxtFmtCollTbl->Count(); ++n )
    {
        pNewColl = (*pTxtFmtCollTbl)[ n ];
        if( nId == pNewColl->GetPoolFmtId() )
            bFnd = sal_True;
    }

    if( !bFnd || !pNewColl->GetDepends() )
        return sal_False;

    SwAutoFmtGetDocNode aGetHt( &GetNodes() );
    return !pNewColl->GetInfo( aGetHt );
}

    // Gebe das "Auto[matische]-Format" mit der Id zurueck. Existiert
    // es noch nicht, dann erzeuge es

SwFmt* SwDoc::GetFmtFromPool( sal_uInt16 nId )
{
    SwFmt *pNewFmt = 0;
    SwFmt *pDeriveFmt = 0;

    SvPtrarr* pArray[ 2 ];
    sal_uInt16 nArrCnt = 1, nRCId = 0;
    sal_uInt16* pWhichRange = 0;

    switch( nId & (COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID) )
    {
    case POOLGRP_CHARFMT:
        {
            pArray[0] = pCharFmtTbl;
            pDeriveFmt = pDfltCharFmt;

            if( nId > RES_POOLCHR_NORMAL_END )
                nRCId = RC_POOLCHRFMT_HTML_BEGIN - RES_POOLCHR_HTML_BEGIN;
            else
                nRCId = RC_POOLCHRFMT_BEGIN - RES_POOLCHR_BEGIN;
            pWhichRange =  aCharFmtSetRange;

            // Fehlerfall: unbekanntes Format, aber CharFormat
            //          -> returne das erste
            if( RES_POOLCHR_BEGIN > nId || nId >= RES_POOLCHR_END )
            {
                OSL_ENSURE( !this, "ungueltige Id" );
                nId = RES_POOLCHR_BEGIN;
            }
        }
        break;
    case POOLGRP_FRAMEFMT:
        {
            pArray[0] = pFrmFmtTbl;
            pArray[1] = pSpzFrmFmtTbl;
            pDeriveFmt = pDfltFrmFmt;
            nArrCnt = 2;
            nRCId = RC_POOLFRMFMT_BEGIN - RES_POOLFRM_BEGIN;
            pWhichRange = aFrmFmtSetRange;

            // Fehlerfall: unbekanntes Format, aber FrameFormat
            //          -> returne das erste
            if( RES_POOLFRM_BEGIN > nId || nId >= RES_POOLFRM_END )
            {
                OSL_ENSURE( !this, "ungueltige Id" );
                nId = RES_POOLFRM_BEGIN;
            }
        }
        break;

    default:
        // Fehlerfall, unbekanntes Format
        OSL_ENSURE( nId, "ungueltige Id" );
        return 0;
    }
    OSL_ENSURE( nRCId, "ungueltige Id" );

    while( nArrCnt-- )
        for( sal_uInt16 n = 0; n < (*pArray[nArrCnt]).Count(); ++n )
            if( nId == ( pNewFmt = (SwFmt*)(*pArray[ nArrCnt ] )[ n ] )->
                    GetPoolFmtId() )
            {
                return pNewFmt;
            }

    ResId aResId( nRCId + nId, *pSwResMgr );
    String aNm( aResId );
    SwAttrSet aSet( GetAttrPool(), pWhichRange );

    {
        sal_Bool bIsModified = IsModified();

        {
            ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());
            switch (nId & (COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID) )
            {
                case POOLGRP_CHARFMT:
                    pNewFmt = _MakeCharFmt(aNm, pDeriveFmt, sal_False, sal_True);
                break;
                case POOLGRP_FRAMEFMT:
                    pNewFmt = _MakeFrmFmt(aNm, pDeriveFmt, sal_False, sal_True);
                break;
                default:
                break;
            }
        }

        if( !bIsModified )
            ResetModified();
        pNewFmt->SetPoolFmtId( nId );
        pNewFmt->SetAuto( sal_False );      // kein Auto-Format
    }

    switch( nId )
    {
    case RES_POOLCHR_FOOTNOTE:              // Fussnote
    case RES_POOLCHR_PAGENO:                // Seiten/Feld
    case RES_POOLCHR_LABEL:                 // Beschriftung
    case RES_POOLCHR_DROPCAPS:              // Initialien
    case RES_POOLCHR_NUM_LEVEL:             // Aufzaehlungszeichen
    case RES_POOLCHR_TOXJUMP:               // Verzeichnissprung
    case RES_POOLCHR_ENDNOTE:               // Endnote
    case RES_POOLCHR_LINENUM:               // Zeilennummerierung
        break;

    case RES_POOLCHR_ENDNOTE_ANCHOR:        // Endnotenanker
    case RES_POOLCHR_FOOTNOTE_ANCHOR:       // Fussnotenanker
        {
            aSet.Put( SvxEscapementItem( DFLT_ESC_AUTO_SUPER, 58, RES_CHRATR_ESCAPEMENT ) );
        }
        break;


    case RES_POOLCHR_BUL_LEVEL:             // Aufzaehlungszeichen
        {
            const Font& rBulletFont = numfunc::GetDefBulletFont();
            SetAllScriptItem( aSet, SvxFontItem( rBulletFont.GetFamily(),
                      rBulletFont.GetName(), rBulletFont.GetStyleName(),
                        rBulletFont.GetPitch(), rBulletFont.GetCharSet(), RES_CHRATR_FONT ));
        }
        break;

    case RES_POOLCHR_INET_NORMAL:
        {
            Color aCol( COL_BLUE );
            aSet.Put( SvxColorItem( aCol, RES_CHRATR_COLOR ) );
            aSet.Put( SvxUnderlineItem( UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE ) );
            // i40133: patch submitted by rail: set language to 'none' to prevent spell checking:
            aSet.Put( SvxLanguageItem( LANGUAGE_NONE, RES_CHRATR_LANGUAGE ) );
            aSet.Put( SvxLanguageItem( LANGUAGE_NONE, RES_CHRATR_CJK_LANGUAGE ) );
            aSet.Put( SvxLanguageItem( LANGUAGE_NONE, RES_CHRATR_CTL_LANGUAGE ) );
        }
        break;
    case RES_POOLCHR_INET_VISIT:
        {
            Color aCol( COL_RED );
            aSet.Put( SvxColorItem( aCol, RES_CHRATR_COLOR ) );
            aSet.Put( SvxUnderlineItem( UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE ) );
            aSet.Put( SvxLanguageItem( LANGUAGE_NONE, RES_CHRATR_LANGUAGE ) );
            aSet.Put( SvxLanguageItem( LANGUAGE_NONE, RES_CHRATR_CJK_LANGUAGE ) );
            aSet.Put( SvxLanguageItem( LANGUAGE_NONE, RES_CHRATR_CTL_LANGUAGE ) );
        }
        break;
    case RES_POOLCHR_JUMPEDIT:
        {
            Color aCol( COL_CYAN );
            aSet.Put( SvxColorItem( aCol, RES_CHRATR_COLOR ) );
            aSet.Put( SvxUnderlineItem( UNDERLINE_DOTTED, RES_CHRATR_UNDERLINE ) );
            aSet.Put( SvxCaseMapItem( SVX_CASEMAP_KAPITAELCHEN, RES_CHRATR_CASEMAP ) );
        }
        break;

    case RES_POOLCHR_RUBYTEXT:
        {
            long nH = ((SvxFontHeightItem*)GetDfltAttr(
                                RES_CHRATR_CJK_FONTSIZE ))->GetHeight() / 2;
            SetAllScriptItem( aSet, SvxFontHeightItem( nH, 100, RES_CHRATR_FONTSIZE));
            aSet.Put(SvxUnderlineItem( UNDERLINE_NONE, RES_CHRATR_UNDERLINE ));
            aSet.Put(SvxEmphasisMarkItem( EMPHASISMARK_NONE, RES_CHRATR_EMPHASIS_MARK) );
        }
        break;

    case RES_POOLCHR_HTML_EMPHASIS:
    case RES_POOLCHR_HTML_CITIATION:
    case RES_POOLCHR_HTML_VARIABLE:
        {
            SetAllScriptItem( aSet, SvxPostureItem( ITALIC_NORMAL, RES_CHRATR_POSTURE) );
        }
        break;

    case RES_POOLCHR_IDX_MAIN_ENTRY:
    case RES_POOLCHR_HTML_STRONG:
        {
            SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ));
        }
        break;

    case RES_POOLCHR_HTML_CODE:
    case RES_POOLCHR_HTML_SAMPLE:
    case RES_POOLCHR_HTML_KEYBOARD:
    case RES_POOLCHR_HTML_TELETYPE:
        {
            ::lcl_SetDfltFont( DEFAULTFONT_FIXED, aSet );
        }
        break;
   case RES_POOLCHR_VERT_NUM:
            aSet.Put( SvxCharRotateItem( 900, sal_False, RES_CHRATR_ROTATE ) );
    break;

    case RES_POOLFRM_FRAME:
        {
            if ( get(IDocumentSettingAccess::BROWSE_MODE) )
            {
                aSet.Put( SwFmtAnchor( FLY_AS_CHAR ));
                aSet.Put( SwFmtVertOrient( 0, text::VertOrientation::LINE_CENTER, text::RelOrientation::PRINT_AREA ) );
                aSet.Put( SwFmtSurround( SURROUND_NONE ) );
            }
            else
            {
                aSet.Put( SwFmtAnchor( FLY_AT_PARA ));
                aSet.Put( SwFmtSurround( SURROUND_PARALLEL ) );
                aSet.Put( SwFmtHoriOrient( 0, text::HoriOrientation::CENTER, text::RelOrientation::PRINT_AREA ) );
                aSet.Put( SwFmtVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::PRINT_AREA ) );
                Color aCol( COL_BLACK );
                SvxBorderLine aLine( &aCol, DEF_LINE_WIDTH_0 );
                SvxBoxItem aBox( RES_BOX );
                aBox.SetLine( &aLine, BOX_LINE_TOP );
                aBox.SetLine( &aLine, BOX_LINE_BOTTOM );
                aBox.SetLine( &aLine, BOX_LINE_LEFT );
                aBox.SetLine( &aLine, BOX_LINE_RIGHT );
                aBox.SetDistance( 85 );
                aSet.Put( aBox );
                aSet.Put( SvxLRSpaceItem( 114, 114, 0, 0, RES_LR_SPACE ) );
                aSet.Put( SvxULSpaceItem( 114, 114, RES_UL_SPACE ) );
            }
        }
        break;
    case RES_POOLFRM_GRAPHIC:
    case RES_POOLFRM_OLE:
        {
            aSet.Put( SwFmtAnchor( FLY_AT_PARA ));
            aSet.Put( SwFmtHoriOrient( 0, text::HoriOrientation::CENTER, text::RelOrientation::FRAME ));
            aSet.Put( SwFmtVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ));
            aSet.Put( SwFmtSurround( SURROUND_NONE ));
        }
        break;
    case RES_POOLFRM_FORMEL:
        {
            aSet.Put( SwFmtAnchor( FLY_AS_CHAR ) );
            aSet.Put( SwFmtVertOrient( 0, text::VertOrientation::CHAR_CENTER, text::RelOrientation::FRAME ) );
            aSet.Put( SvxLRSpaceItem( 114, 114, 0, 0, RES_LR_SPACE ) );
        }
        break;
    case RES_POOLFRM_MARGINAL:
        {
            aSet.Put( SwFmtAnchor( FLY_AT_PARA ));
            aSet.Put( SwFmtHoriOrient( 0, text::HoriOrientation::LEFT, text::RelOrientation::FRAME ));
            aSet.Put( SwFmtVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ));
            aSet.Put( SwFmtSurround( SURROUND_PARALLEL ));
            // Breite 3.5 centimeter vorgegeben, als Hoehe nur den
            // min. Wert benutzen
            aSet.Put( SwFmtFrmSize( ATT_MIN_SIZE,
                    GetMetricVal( CM_1 ) * 3 + GetMetricVal( CM_05 ),
                    MM50 ));
        }
        break;
    case RES_POOLFRM_WATERSIGN:
        {
            aSet.Put( SwFmtAnchor( FLY_AT_PAGE ));
            aSet.Put( SwFmtHoriOrient( 0, text::HoriOrientation::CENTER, text::RelOrientation::FRAME ));
            aSet.Put( SwFmtVertOrient( 0, text::VertOrientation::CENTER, text::RelOrientation::FRAME ));
            aSet.Put( SvxOpaqueItem( sal_False ));
            aSet.Put( SwFmtSurround( SURROUND_THROUGHT ));
        }
        break;

    case RES_POOLFRM_LABEL:
        {
            aSet.Put( SwFmtAnchor( FLY_AS_CHAR ) );
            aSet.Put( SwFmtVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ) );
            aSet.Put( SvxLRSpaceItem( 114, 114, 0, 0, RES_LR_SPACE ) );

            SvxProtectItem aProtect( RES_PROTECT );
            aProtect.SetSizeProtect( sal_True );
            aProtect.SetPosProtect( sal_True );
            aSet.Put( aProtect );

            pNewFmt->SetAutoUpdateFmt( sal_True );
        }
        break;
    }
    if( aSet.Count() )
    {
        {
            pNewFmt->SetFmtAttr( aSet );
        }
    }
    return pNewFmt;
}

SwFrmFmt* SwDoc::GetFrmFmtFromPool( sal_uInt16 nId )
{
    return (SwFrmFmt*)GetFmtFromPool( nId );
}

SwCharFmt* SwDoc::GetCharFmtFromPool( sal_uInt16 nId )
{
    return (SwCharFmt*)GetFmtFromPool( nId );
}

    // pruefe, ob diese "Auto-Collection" in Dokument schon/noch
    // benutzt wird
bool SwDoc::IsPoolFmtUsed( sal_uInt16 nId ) const
{
    SwFmt *pNewFmt = 0;
    const SvPtrarr* pArray[ 2 ];
    sal_uInt16 nArrCnt = 1;
    sal_Bool bFnd = sal_True;

    if( RES_POOLCHR_BEGIN <= nId && nId < RES_POOLCHR_END )
    {
        pArray[0] = pCharFmtTbl;
    }
    if( RES_POOLFRM_BEGIN <= nId && nId < RES_POOLFRM_END )
    {
        pArray[0] = pFrmFmtTbl;
        pArray[1] = pSpzFrmFmtTbl;
        nArrCnt = 2;
    }
    else
    {
        OSL_ENSURE( sal_False, "ungueltige Id" );
        bFnd = sal_False;
    }

    if( bFnd )
    {
        bFnd = sal_False;
        while( nArrCnt-- && !bFnd )
            for( sal_uInt16 n = 0; !bFnd && n < (*pArray[nArrCnt]).Count(); ++n )
                if( nId == ( pNewFmt = (SwFmt*)(*pArray[ nArrCnt ] )[ n ] )->
                        GetPoolFmtId() )
                    bFnd = sal_True;
    }

    // nicht gefunden oder keine Abhaengigen ?
    if( bFnd && pNewFmt->GetDepends() )
    {
        // dann teste mal, ob es abhaengige ContentNodes im Nodes Array gibt
        // (auch indirekte fuer Format-Ableitung! )
        SwAutoFmtGetDocNode aGetHt( &GetNodes() );
        bFnd = !pNewFmt->GetInfo( aGetHt );
    }
    else
        bFnd = sal_False;

    return bFnd;
}



void lcl_GetStdPgSize( SwDoc* pDoc, SfxItemSet& rSet )
{
    SwPageDesc* pStdPgDsc = pDoc->GetPageDescFromPool( RES_POOLPAGE_STANDARD );
    SwFmtFrmSize aFrmSz( pStdPgDsc->GetMaster().GetFrmSize() );
    if( pStdPgDsc->GetLandscape() )
    {
        SwTwips nTmp = aFrmSz.GetHeight();
        aFrmSz.SetHeight( aFrmSz.GetWidth() );
        aFrmSz.SetWidth( nTmp );
    }
    rSet.Put( aFrmSz );
}

SwPageDesc* SwDoc::GetPageDescFromPool( sal_uInt16 nId, bool bRegardLanguage )
{
    OSL_ENSURE( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END,
            "Falsche AutoFormat-Id" );

    SwPageDesc *pNewPgDsc;
    sal_uInt16 n;

    for( n = 0; n < aPageDescs.Count(); ++n )
        if( nId == ( pNewPgDsc = aPageDescs[ n ] )->GetPoolFmtId() )
        {
            return pNewPgDsc;
        }

    // Fehlerfall: unbekannte Poolvorlage
    if( RES_POOLPAGE_BEGIN > nId ||  nId >= RES_POOLPAGE_END )
    {
        OSL_ENSURE( !this, "ungueltige Id" );
        nId = RES_POOLPAGE_BEGIN;
    }

    ResId aResId( sal_uInt32(RC_POOLPAGEDESC_BEGIN + nId - RES_POOLPAGE_BEGIN), *pSwResMgr );
    String aNm( aResId );
    {
        sal_Bool bIsModified = IsModified();

        {
            ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());
            n = MakePageDesc( aNm, 0, bRegardLanguage );
        }

        pNewPgDsc = aPageDescs[ n ];
        pNewPgDsc->SetPoolFmtId( nId );
        if( !bIsModified )
            ResetModified();
    }


    SvxLRSpaceItem aLR( RES_LR_SPACE );
    aLR.SetLeft( GetMetricVal( CM_1 ) * 2 );
    aLR.SetRight( aLR.GetLeft() );
    SvxULSpaceItem aUL( RES_UL_SPACE );
    aUL.SetUpper( (sal_uInt16)aLR.GetLeft() );
    aUL.SetLower( (sal_uInt16)aLR.GetLeft() );

    SwAttrSet aSet( GetAttrPool(), aPgFrmFmtSetRange );
    sal_Bool bSetLeft = sal_True;

    switch( nId )
    {
    case RES_POOLPAGE_STANDARD:             // Standard-Seite
        {
            aSet.Put( aLR );
            aSet.Put( aUL );
            if( pNewPgDsc )
                pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL );
        }
        break;

    case RES_POOLPAGE_FIRST:                // Erste Seite
    case RES_POOLPAGE_REGISTER:             // Verzeichnis
        {
            lcl_GetStdPgSize( this, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            if( pNewPgDsc )
            {
                pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL );
                if( RES_POOLPAGE_FIRST == nId )
                    pNewPgDsc->SetFollow( GetPageDescFromPool( RES_POOLPAGE_STANDARD ));
            }
        }
        break;

    case RES_POOLPAGE_LEFT:                 // Linke Seite
        {
            lcl_GetStdPgSize( this, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            bSetLeft = sal_False;
            if( pNewPgDsc )
                pNewPgDsc->SetUseOn( nsUseOnPage::PD_LEFT );
        }
        break;
    case RES_POOLPAGE_RIGHT:                // Rechte Seite
        {
            lcl_GetStdPgSize( this, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            bSetLeft = sal_False;
            if( pNewPgDsc )
                pNewPgDsc->SetUseOn( nsUseOnPage::PD_RIGHT );
        }
        break;

    case RES_POOLPAGE_JAKET:                // Umschlag
        {
            aLR.SetLeft( 0 ); aLR.SetRight( 0 );
            aUL.SetUpper( 0 ); aUL.SetLower( 0 );
            Size aPSize( SvxPaperInfo::GetPaperSize( PAPER_ENV_C65 ) );
            LandscapeSwap( aPSize );
            aSet.Put( SwFmtFrmSize( ATT_FIX_SIZE, aPSize.Width(), aPSize.Height() ));
            aSet.Put( aLR );
            aSet.Put( aUL );

            if( pNewPgDsc )
            {
                pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL );
                pNewPgDsc->SetLandscape( sal_True );
            }
        }
        break;

    case RES_POOLPAGE_HTML:             // HTML
        {
            lcl_GetStdPgSize( this, aSet );
            aLR.SetRight( GetMetricVal( CM_1 ));
            aUL.SetUpper( (sal_uInt16)aLR.GetRight() );
            aUL.SetLower( (sal_uInt16)aLR.GetRight() );
            aSet.Put( aLR );
            aSet.Put( aUL );

            if( pNewPgDsc )
                pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL );
        }
        break;
    case RES_POOLPAGE_FOOTNOTE:
    case RES_POOLPAGE_ENDNOTE:
        {
            lcl_GetStdPgSize( this, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            if( pNewPgDsc )
                pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL );
            SwPageFtnInfo aInf( pNewPgDsc->GetFtnInfo() );
            aInf.SetLineWidth( 0 );
            aInf.SetTopDist( 0 );
            aInf.SetBottomDist( 0 );
            pNewPgDsc->SetFtnInfo( aInf );
        }
        break;
        case RES_POOLPAGE_LANDSCAPE:
        {
            SwPageDesc* pStdPgDsc = this->GetPageDescFromPool( RES_POOLPAGE_STANDARD );
            SwFmtFrmSize aFrmSz( pStdPgDsc->GetMaster().GetFrmSize() );
            SwTwips nTmp = aFrmSz.GetHeight();
            aFrmSz.SetHeight( aFrmSz.GetWidth() );
            aFrmSz.SetWidth( nTmp );
            aSet.Put( aFrmSz );
            aSet.Put( aLR );
            aSet.Put( aUL );
            if( pNewPgDsc )
            {
                pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL );
                pNewPgDsc->SetLandscape( sal_True );
            }
       }
       break;

    }

    if( aSet.Count() )
    {
        {
            if( bSetLeft )
                pNewPgDsc->GetLeft().SetFmtAttr( aSet );
            pNewPgDsc->GetMaster().SetFmtAttr( aSet );
        }
    }
    return pNewPgDsc;
}

SwNumRule* SwDoc::GetNumRuleFromPool( sal_uInt16 nId )
{
    OSL_ENSURE( RES_POOLNUMRULE_BEGIN <= nId && nId < RES_POOLNUMRULE_END,
            "Falsche AutoFormat-Id" );

    SwNumRule* pNewRule;
    sal_uInt16 n;

    for( n = 0; n < GetNumRuleTbl().Count(); ++n )
        if( nId == ( pNewRule = GetNumRuleTbl()[ n ] )->GetPoolFmtId() )
        {
            return pNewRule;
        }

    // Fehlerfall: unbekannte Poolvorlage
    if( RES_POOLNUMRULE_BEGIN > nId ||  nId >= RES_POOLNUMRULE_END )
    {
        OSL_ENSURE( !this, "ungueltige Id" );
        nId = RES_POOLNUMRULE_BEGIN;
    }

    ResId aResId( sal_uInt32(RC_POOLNUMRULE_BEGIN + nId - RES_POOLNUMRULE_BEGIN), *pSwResMgr );
    String aNm( aResId );

    SwCharFmt *pNumCFmt = 0, *pBullCFmt = 0;

    const SvxNumberFormat::SvxNumPositionAndSpaceMode eNumberFormatPositionAndSpaceMode
                                  = numfunc::GetDefaultPositionAndSpaceMode(); //#i89178#
    // <--
    {
        sal_Bool bIsModified = IsModified();

        n = MakeNumRule( aNm, 0, sal_False, eNumberFormatPositionAndSpaceMode );

        pNewRule = GetNumRuleTbl()[ n ];
        pNewRule->SetPoolFmtId( nId );
        pNewRule->SetAutoRule( sal_False );

        if( RES_POOLNUMRULE_NUM1 <= nId && nId <= RES_POOLNUMRULE_NUM5 )
            pNumCFmt = GetCharFmtFromPool( RES_POOLCHR_NUM_LEVEL );

        if( ( RES_POOLNUMRULE_BUL1 <= nId && nId <= RES_POOLNUMRULE_BUL5 ) ||
            RES_POOLNUMRULE_NUM5 == nId )
            pBullCFmt = GetCharFmtFromPool( RES_POOLCHR_NUM_LEVEL );

        if( !bIsModified )
            ResetModified();
    }

    switch( nId )
    {
    case RES_POOLNUMRULE_NUM1:
        {
            SwNumFmt aFmt;
            aFmt.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFmt.SetNumberingType(SVX_NUM_ARABIC);
            aFmt.SetCharFmt( pNumCFmt );
            aFmt.SetStart( 1 );
            aFmt.SetIncludeUpperLevels( 1 );
            aFmt.SetSuffix( aDotStr );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
                    283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
                };
#ifdef USE_MEASUREMENT
            static const sal_uInt16 aAbsSpaceInch[ MAXLEVEL ] =
                {
                    283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
                };
            const sal_uInt16* pArr = MEASURE_METRIC ==
                                SvtSysLocale().GetLocaleData().getMeasurementSystemEnum()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const sal_uInt16* pArr = aAbsSpace;
#endif

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFmt.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFmt.SetFirstLineIndent( - (*pArr) );
            }

            for( n = 0; n < MAXLEVEL; ++n, ++pArr )
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFmt.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFmt.SetListtabPos( *pArr );
                    aFmt.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFmt );
            }
        }
        break;

    case RES_POOLNUMRULE_NUM2:
        {
            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
                    283,  283,  567,  709,      // 0.50, 0.50, 1.00, 1.25
                    850, 1021, 1304, 1474,      // 1.50, 1.80, 2.30, 2.60
                   1588, 1758                   // 2.80, 3.10
                };

#ifdef USE_MEASUREMENT
            static const sal_uInt16 aAbsSpaceInch[ MAXLEVEL ] =
                {
                    385,  385,  770,  963,
                   1155, 1386, 1771, 2002,
                   2156, 2387
                };

            const sal_uInt16* pArr = MEASURE_METRIC ==
                                SvtSysLocale().GetLocaleData().getMeasurementSystemEnum()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const sal_uInt16* pArr = aAbsSpace;
#endif
            SwNumFmt aFmt;

            aFmt.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFmt.SetNumberingType(SVX_NUM_ARABIC);
            aFmt.SetCharFmt( pNumCFmt );
            aFmt.SetIncludeUpperLevels( 1 );

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
            }

            sal_uInt16 nSpace = 0;
            for( n = 0; n < MAXLEVEL; ++n )
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFmt.SetAbsLSpace( nSpace = nSpace + pArr[ n ] );
                    aFmt.SetFirstLineOffset( - pArr[ n ] );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFmt.SetListtabPos( nSpace = nSpace + pArr[ n ] );
                    aFmt.SetIndentAt( nSpace );
                    aFmt.SetFirstLineIndent( - pArr[ n ] );
                }

                aFmt.SetStart( n+1 );
                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_NUM3:
        {
            SwNumFmt aFmt;

            aFmt.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFmt.SetNumberingType(SVX_NUM_ARABIC);
            aFmt.SetCharFmt( pNumCFmt );
            aFmt.SetIncludeUpperLevels( 1 );

            sal_uInt16 nOffs = GetMetricVal( CM_1 ) * 3;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFmt.SetFirstLineOffset( - nOffs );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFmt.SetFirstLineIndent( - nOffs );
            }

            for( n = 0; n < MAXLEVEL; ++n )
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFmt.SetAbsLSpace( (n+1) * nOffs );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFmt.SetListtabPos( (n+1) * nOffs );
                    aFmt.SetIndentAt( (n+1) * nOffs );
                }

                aFmt.SetStart( n+1 );
                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_NUM4:
        {
            SwNumFmt aFmt;

            aFmt.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFmt.SetNumberingType(SVX_NUM_ROMAN_UPPER);
            aFmt.SetCharFmt( pNumCFmt );
            aFmt.SetIncludeUpperLevels( 1 );
            aFmt.SetSuffix( aDotStr );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
                    283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
                };
#ifdef USE_MEASUREMENT
            static const sal_uInt16 aAbsSpaceInch[ MAXLEVEL ] =
                {
                    283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
                };
            const sal_uInt16* pArr = MEASURE_METRIC ==
                                SvtSysLocale().GetLocaleData().getMeasurementSystemEnum()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const sal_uInt16* pArr = aAbsSpace;
#endif

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFmt.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetLabelFollowedBy( SvxNumberFormat::SPACE );
                aFmt.SetFirstLineIndent( - (*pArr) );
            }

            for( n = 0; n < MAXLEVEL; ++n, ++pArr )
            {
                aFmt.SetStart( n + 1 );

                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFmt.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFmt.SetListtabPos( *pArr );
                    aFmt.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_NUM5:
        {
            // [ First, LSpace ]
            static const sal_uInt16 aAbsSpace0to2[] =
                {
                    227,  227,      // 0.40, 0.40,
                    369,  624,      // 0.65, 1.10,
                    255,  879       // 0.45, 1.55
                };

#ifdef USE_MEASUREMENT
            static const sal_uInt16 aAbsSpaceInch0to2[] =
                {
                    308,  308,
                    501,  847,
                    347, 1194
                };
            const sal_uInt16* pArr0to2 = MEASURE_METRIC ==
                            SvtSysLocale().GetLocaleData().getMeasurementSystemEnum()
                                ? aAbsSpace0to2
                                : aAbsSpaceInch0to2;
#else
            const sal_uInt16* pArr0to2 = aAbsSpace0to2;
#endif
            SwNumFmt aFmt;

            aFmt.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFmt.SetNumberingType(SVX_NUM_ARABIC);
            aFmt.SetStart( 1 );
            aFmt.SetIncludeUpperLevels( 1 );
            aFmt.SetSuffix( aDotStr );

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
            }

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFmt.SetFirstLineOffset( -pArr0to2[0] );    // == 0.40 cm
                aFmt.SetAbsLSpace( pArr0to2[1] );           // == 0.40 cm
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetFirstLineIndent( -pArr0to2[0] );
                aFmt.SetListtabPos( pArr0to2[1] );
                aFmt.SetIndentAt( pArr0to2[1] );
            }

            aFmt.SetCharFmt( pNumCFmt );
            pNewRule->Set( 0, aFmt );

            aFmt.SetIncludeUpperLevels( 2 );
            aFmt.SetStart( 2 );

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFmt.SetFirstLineOffset( -pArr0to2[2] );    // == 0.65 cm
                aFmt.SetAbsLSpace( pArr0to2[3] );           // == 1.10 cm
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetFirstLineIndent( -pArr0to2[2] );
                aFmt.SetListtabPos( pArr0to2[3] );
                aFmt.SetIndentAt( pArr0to2[3] );
            }

            pNewRule->Set( 1, aFmt );

            aFmt.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER);
            aFmt.SetSuffix( ')');
            aFmt.SetIncludeUpperLevels( 1 );
            aFmt.SetStart( 3 );

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFmt.SetFirstLineOffset( - pArr0to2[4] );   // == 0.45cm
                aFmt.SetAbsLSpace( pArr0to2[5] );           // == 1.55 cm
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetFirstLineIndent( -pArr0to2[4] );
                aFmt.SetListtabPos( pArr0to2[5] );
                aFmt.SetIndentAt( pArr0to2[5] );
            }

            pNewRule->Set( 2, aFmt );


            aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFmt.SetCharFmt( pBullCFmt );
            aFmt.SetBulletFont(  &numfunc::GetDefBulletFont() );
            aFmt.SetBulletChar( cBulletChar );
            sal_uInt16 nOffs = GetMetricVal( CM_01 ) * 4,
                   nOffs2 = GetMetricVal( CM_1 ) * 2;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFmt.SetFirstLineOffset( - nOffs );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetFirstLineIndent( - nOffs );
            }

            aFmt.SetSuffix( aEmptyStr );
            for( n = 3; n < MAXLEVEL; ++n )
            {
                aFmt.SetStart( n+1 );

                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFmt.SetAbsLSpace( nOffs2 + ((n-3) * nOffs) );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFmt.SetListtabPos( nOffs2 + ((n-3) * nOffs) );
                    aFmt.SetIndentAt( nOffs2 + ((n-3) * nOffs) );
                }

                pNewRule->Set( n, aFmt );
            }
        }
        break;

    case RES_POOLNUMRULE_BUL1:
        {
            SwNumFmt aFmt;

            aFmt.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFmt.SetCharFmt( pBullCFmt );
            aFmt.SetStart( 1 );
            aFmt.SetIncludeUpperLevels( 1 );
            aFmt.SetBulletFont( &numfunc::GetDefBulletFont() );
            aFmt.SetBulletChar( cBulletChar );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
#ifdef USE_MEASUREMENT
            static const sal_uInt16 aAbsSpaceInch[ MAXLEVEL ] =
                {
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
            const sal_uInt16* pArr = MEASURE_METRIC ==
                                SvtSysLocale().GetLocaleData().getMeasurementSystemEnum()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const sal_uInt16* pArr = aAbsSpace;
#endif

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFmt.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFmt.SetFirstLineIndent( - (*pArr) );
            }

            for( n = 0; n < MAXLEVEL; ++n, ++pArr )
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFmt.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFmt.SetListtabPos( *pArr );
                    aFmt.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL2:
        {
            SwNumFmt aFmt;

            aFmt.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFmt.SetCharFmt( pBullCFmt );
            aFmt.SetStart( 1 );
            aFmt.SetIncludeUpperLevels( 1 );
            aFmt.SetBulletFont(  &numfunc::GetDefBulletFont() );
            aFmt.SetBulletChar( 0x2013 );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,3  0,6  0,9  1,2  1,5  1,8   2,1   2,4   2,7   3,0
                    170, 340, 510, 680, 850, 1020, 1191, 1361, 1531, 1701
                };
#ifdef USE_MEASUREMENT
            static const sal_uInt16 aAbsSpaceInch[ MAXLEVEL ] =
                {
                    170, 340, 510, 680, 850, 1020, 1191, 1361, 1531, 1701
                };
            const sal_uInt16* pArr = MEASURE_METRIC ==
                                SvtSysLocale().GetLocaleData().getMeasurementSystemEnum()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const sal_uInt16* pArr = aAbsSpace;
#endif

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFmt.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFmt.SetFirstLineIndent( - (*pArr) );
            }

            for( n = 0; n < MAXLEVEL; ++n, ++pArr )
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFmt.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFmt.SetListtabPos( *pArr );
                    aFmt.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL3:
        {
            SwNumFmt aFmt;

            aFmt.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );

            aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFmt.SetCharFmt( pBullCFmt );
            aFmt.SetStart( 1 );
            aFmt.SetIncludeUpperLevels( 1 );
            aFmt.SetBulletFont(  &numfunc::GetDefBulletFont() );

            sal_uInt16 nOffs = GetMetricVal( CM_01 ) * 4;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFmt.SetFirstLineOffset( - nOffs );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFmt.SetFirstLineIndent( - nOffs );
            }

            for( n = 0; n < MAXLEVEL; ++n )
            {
                aFmt.SetBulletChar( ( n & 1 ? 0x25a1 : 0x2611 ) );

                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFmt.SetAbsLSpace( ((n & 1) +1) * nOffs );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFmt.SetListtabPos( ((n & 1) +1) * nOffs );
                    aFmt.SetIndentAt( ((n & 1) +1) * nOffs );
                }

                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL4:
        {
            SwNumFmt aFmt;

            aFmt.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFmt.SetCharFmt( pBullCFmt );
            aFmt.SetStart( 1 );
            aFmt.SetIncludeUpperLevels( 1 );
            aFmt.SetBulletFont(  &numfunc::GetDefBulletFont() );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
#ifdef USE_MEASUREMENT
            static const sal_uInt16 aAbsSpaceInch[ MAXLEVEL ] =
                {
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
            const sal_uInt16* pArr = MEASURE_METRIC ==
                                SvtSysLocale().GetLocaleData().getMeasurementSystemEnum()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const sal_uInt16* pArr = aAbsSpace;
#endif

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFmt.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetLabelFollowedBy( SvxNumberFormat::SPACE );
                aFmt.SetFirstLineIndent( - (*pArr) );
            }

            for( n = 0; n < MAXLEVEL; ++n, ++pArr )
            {
                switch( n )
                {
                case 0:     aFmt.SetBulletChar( 0x27a2 );   break;
                case 1:     aFmt.SetBulletChar( 0xE006 );   break;
                default:    aFmt.SetBulletChar( 0xE004 );   break;
                }

                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFmt.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFmt.SetListtabPos( *pArr );
                    aFmt.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFmt );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL5:
        {
            SwNumFmt aFmt;

            aFmt.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFmt.SetCharFmt( pBullCFmt );
            aFmt.SetStart( 1 );
            aFmt.SetIncludeUpperLevels( 1 );
            aFmt.SetBulletChar( 0x2717 );
            aFmt.SetBulletFont( &numfunc::GetDefBulletFont() );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
#ifdef USE_MEASUREMENT
            static const sal_uInt16 aAbsSpaceInch[ MAXLEVEL ] =
                {
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
            const sal_uInt16* pArr = MEASURE_METRIC ==
                                SvtSysLocale().GetLocaleData().getMeasurementSystemEnum()
                                    ? aAbsSpace
                                    : aAbsSpaceInch;
#else
            const sal_uInt16* pArr = aAbsSpace;
#endif

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFmt.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFmt.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFmt.SetFirstLineIndent( - (*pArr) );
            }

            for( n = 0; n < MAXLEVEL; ++n, ++pArr )
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFmt.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFmt.SetListtabPos( *pArr );
                    aFmt.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFmt );
            }
        }
        break;
    }

    return pNewRule;
}



    // pruefe, ob diese "Auto-Collection" in Dokument schon/noch
    // benutzt wird
bool SwDoc::IsPoolPageDescUsed( sal_uInt16 nId ) const
{
    OSL_ENSURE( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END,
            "Falsche AutoFormat-Id" );
    SwPageDesc *pNewPgDsc = 0;
    sal_Bool bFnd = sal_False;
    for( sal_uInt16 n = 0; !bFnd && n < aPageDescs.Count(); ++n )
    {
        pNewPgDsc = aPageDescs[ n ];
        if( nId == pNewPgDsc->GetPoolFmtId() )
            bFnd = sal_True;
    }

    // nicht gefunden oder keine Abhaengigen ?
    if( !bFnd || !pNewPgDsc->GetDepends() )     // ??????
        return sal_False;

    // dann teste mal, ob es abhaengige ContentNodes im Nodes Array gibt
    // (auch indirekte fuer Format-Ableitung! )
    SwAutoFmtGetDocNode aGetHt( &GetNodes() );
    return !pNewPgDsc->GetInfo( aGetHt );
}

// erfrage ob die Absatz-/Zeichen-/Rahmen-/Seiten - Vorlage benutzt wird
sal_Bool SwDoc::IsUsed( const SwModify& rModify ) const
{
    // dann teste mal, ob es abhaengige ContentNodes im Nodes Array gibt
    // (auch indirekte fuer Format-Ableitung! )
    SwAutoFmtGetDocNode aGetHt( &GetNodes() );
    return !rModify.GetInfo( aGetHt );
}

// erfrage ob die NumRule benutzt wird
sal_Bool SwDoc::IsUsed( const SwNumRule& rRule ) const
{
    sal_Bool bUsed = rRule.GetTxtNodeListSize() > 0 ||
                     rRule.GetParagraphStyleListSize() > 0;

    return bUsed;
    // <--
}

    // Suche die Position vom Vorlagen-Namen. Ist nicht vorhanden
    // dann fuege neu ein
sal_uInt16 SwDoc::SetDocPattern( const String& rPatternName )
{
    OSL_ENSURE( rPatternName.Len(), "kein Dokument-Vorlagenname" );

    sal_uInt16 nNewPos = aPatternNms.Count();
    for( sal_uInt16 n = 0; n < aPatternNms.Count(); ++n )
        if( !aPatternNms[n] )
        {
            if( nNewPos == aPatternNms.Count() )
                nNewPos = n;
        }
        else if( rPatternName == *aPatternNms[n] )
            return n;

    if( nNewPos < aPatternNms.Count() )
        aPatternNms.Remove( nNewPos );      // Platz wieder frei machen

    String* pNewNm = new String( rPatternName );
    aPatternNms.Insert( pNewNm, nNewPos );
    SetModified();
    return nNewPos;
}

sal_uInt16 GetPoolParent( sal_uInt16 nId )
{
    sal_uInt16 nRet = USHRT_MAX;
    if( POOLGRP_NOCOLLID & nId )        // 1 == Formate / 0 == Collections
    {
        switch( ( COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID ) & nId )
        {
        case POOLGRP_CHARFMT:
        case POOLGRP_FRAMEFMT:
            nRet = 0;           // vom default abgeleitet
            break;
        case POOLGRP_PAGEDESC:
        case POOLGRP_NUMRULE:
            break;              // es gibt keine Ableitung
        }
    }
    else
    {
        switch( COLL_GET_RANGE_BITS & nId )
        {
        case COLL_TEXT_BITS:
            switch( nId )
            {
            case RES_POOLCOLL_STANDARD:
                    nRet = 0;                                   break;
            case RES_POOLCOLL_TEXT_IDENT:
            case RES_POOLCOLL_TEXT_NEGIDENT:
            case RES_POOLCOLL_TEXT_MOVE:
            case RES_POOLCOLL_CONFRONTATION:
            case RES_POOLCOLL_MARGINAL:
                    nRet = RES_POOLCOLL_TEXT;                   break;

            case RES_POOLCOLL_TEXT:
            case RES_POOLCOLL_GREETING:
            case RES_POOLCOLL_SIGNATURE:
            case RES_POOLCOLL_HEADLINE_BASE:
                    nRet = RES_POOLCOLL_STANDARD;               break;

            case RES_POOLCOLL_HEADLINE1:
            case RES_POOLCOLL_HEADLINE2:
            case RES_POOLCOLL_HEADLINE3:
            case RES_POOLCOLL_HEADLINE4:
            case RES_POOLCOLL_HEADLINE5:
            case RES_POOLCOLL_HEADLINE6:
            case RES_POOLCOLL_HEADLINE7:
            case RES_POOLCOLL_HEADLINE8:
            case RES_POOLCOLL_HEADLINE9:
            case RES_POOLCOLL_HEADLINE10:
                    nRet = RES_POOLCOLL_HEADLINE_BASE;          break;
            }
            break;

        case COLL_LISTS_BITS:
            switch( nId )
            {
            case RES_POOLCOLL_NUMBUL_BASE:
                    nRet = RES_POOLCOLL_TEXT;                   break;

            default:
                nRet = RES_POOLCOLL_NUMBUL_BASE;                break;
            }
            break;

        case COLL_EXTRA_BITS:
            switch( nId )
            {
            case RES_POOLCOLL_FRAME:
                    nRet = RES_POOLCOLL_TEXT;                   break;

            case RES_POOLCOLL_TABLE_HDLN:
                    nRet = RES_POOLCOLL_TABLE;                  break;

            case RES_POOLCOLL_TABLE:
            case RES_POOLCOLL_FOOTNOTE:
            case RES_POOLCOLL_ENDNOTE:
            case RES_POOLCOLL_JAKETADRESS:
            case RES_POOLCOLL_SENDADRESS:
            case RES_POOLCOLL_HEADER:
            case RES_POOLCOLL_HEADERL:
            case RES_POOLCOLL_HEADERR:
            case RES_POOLCOLL_FOOTER:
            case RES_POOLCOLL_FOOTERL:
            case RES_POOLCOLL_FOOTERR:
            case RES_POOLCOLL_LABEL:
                    nRet = RES_POOLCOLL_STANDARD;               break;

            case RES_POOLCOLL_LABEL_ABB:
            case RES_POOLCOLL_LABEL_TABLE:
            case RES_POOLCOLL_LABEL_FRAME:
            case RES_POOLCOLL_LABEL_DRAWING:
                    nRet = RES_POOLCOLL_LABEL;                  break;
            }
            break;

        case COLL_REGISTER_BITS:
            switch( nId )
            {
            case RES_POOLCOLL_REGISTER_BASE:
                    nRet = RES_POOLCOLL_STANDARD;               break;

            case RES_POOLCOLL_TOX_USERH:
            case RES_POOLCOLL_TOX_CNTNTH:
            case RES_POOLCOLL_TOX_IDXH:
            case RES_POOLCOLL_TOX_ILLUSH:
            case RES_POOLCOLL_TOX_OBJECTH:
            case RES_POOLCOLL_TOX_TABLESH:
            case RES_POOLCOLL_TOX_AUTHORITIESH:
                    nRet = RES_POOLCOLL_HEADLINE_BASE;          break;

            default:
                    nRet = RES_POOLCOLL_REGISTER_BASE;          break;
            }
            break;

        case COLL_DOC_BITS:
            nRet = RES_POOLCOLL_HEADLINE_BASE;
            break;

        case COLL_HTML_BITS:
            nRet = RES_POOLCOLL_STANDARD;
            break;
        }
    }

    return nRet;
}

void SwDoc::RemoveAllFmtLanguageDependencies()
{
    /* Restore the language independ pool defaults and styles. */
    GetAttrPool().ResetPoolDefaultItem( RES_PARATR_ADJUST );

    SwTxtFmtColl * pTxtFmtColl = GetTxtCollFromPool( RES_POOLCOLL_STANDARD );

    pTxtFmtColl->ResetFmtAttr( RES_PARATR_ADJUST );
    /* koreans do not like SvxScriptItem(TRUE) */
    pTxtFmtColl->ResetFmtAttr( RES_PARATR_SCRIPTSPACE );

    SvxFrameDirectionItem aFrameDir( FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR );

    sal_uInt16 nCount = GetPageDescCnt();
    for( sal_uInt16 i=0; i<nCount; ++i )
    {
        SwPageDesc& rDesc = _GetPageDesc( i );
        rDesc.GetMaster().SetFmtAttr( aFrameDir );
        rDesc.GetLeft().SetFmtAttr( aFrameDir );
    }

    // #i18732# - restore static pool default for item
    // RES_FOLLOW_TEXT_FLOW.
    GetAttrPool().ResetPoolDefaultItem( RES_FOLLOW_TEXT_FLOW );

    //#i16874# AutoKerning as default for new documents
    GetAttrPool().ResetPoolDefaultItem( RES_CHRATR_AUTOKERN );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
