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

#include <hintids.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <unotools/localedatawrapper.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/emphasismarkitem.hxx>
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

using namespace ::editeng;
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

#define HTML_PARSPACE   GetMetricVal( CM_05 )

static const sal_uInt16 aHeadlineSizes[ 2 * MAXLEVEL ] = {
//  PT_16, PT_14, PT_14, PT_12, PT_12,          // normal
// we do everything procentual now:
    115, 100, 100, 85, 85,
    75,   75,  75, 75, 75,  // normal
//  PT_22, PT_16, PT_12, PT_11, PT_9            // HTML mode
    PT_24, PT_18, PT_14, PT_12, PT_10,
    PT_7, PT_7, PT_7, PT_7, PT_7            // HTML mode
};

static long lcl_GetRightMargin( SwDoc& rDoc )
{
    // Make sure that the printer settings are taken over to the standard
    // page style
    const SwFrmFmt& rPgDscFmt = rDoc.GetPageDesc( 0 ).GetMaster();
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

static void lcl_SetDfltFont( sal_uInt16 nFntType, SfxItemSet& rSet )
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

static void lcl_SetDfltFont( sal_uInt16 nLatinFntType, sal_uInt16 nCJKFntType,
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

static void lcl_SetHeadline( SwDoc* pDoc, SwTxtFmtColl* pColl,
                        SfxItemSet& rSet,
                        sal_uInt16 nOutLvlBits, sal_uInt8 nLevel, bool bItalic )
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
            }
        }
        pColl->SetNextTxtFmtColl( *pDoc->GetTxtCollFromPool(
                                        RES_POOLCOLL_TEXT ));
    }
}

static void lcl_SetRegister( SwDoc* pDoc, SfxItemSet& rSet, sal_uInt16 nFact,
                        bool bHeader, bool bTab )
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

static void lcl_SetNumBul( SwDoc* pDoc, SwTxtFmtColl* pColl,
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

/// Return the AutoCollection by it's Id. If it doesn't
/// exist yet, create it.
/// If the String pointer is defined, then only query for
/// the Attribute descriptions. It doesn't create a style!
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
            "Wrong  AutoFormat Id" );

    SwTxtFmtColl* pNewColl;
    sal_uInt16 nOutLvlBits = 0;
    for( sal_uInt16 n = 0; n < mpTxtFmtCollTbl->size(); ++n )
    {
        if( nId == ( pNewColl = (*mpTxtFmtCollTbl)[ n ] )->GetPoolFmtId() )
        {
            return pNewColl;
        }

        if( pNewColl->IsAssignedToListLevelOfOutlineStyle())
            nOutLvlBits |= ( 1 << pNewColl->GetAssignedOutlineStyleLevel() );//<-end,zhaojianwei
    }

    // Didn't find it until here -> create anew
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

    OSL_ENSURE( nResId, "Invalid Pool ID" );
    if( !nResId )
        return GetTxtCollFromPool( RES_POOLCOLL_STANDARD );

    ResId aResId( nResId + nId, *pSwResMgr );
    String aNm( aResId );

    // A Set for all to-be-set Attributes
    SwAttrSet aSet( GetAttrPool(), aTxtFmtCollSetRange );
    sal_uInt16 nParent = GetPoolParent( nId );

    {

//FEATURE::CONDCOLL
        if(::IsConditionalByPoolId( nId ))
            pNewColl = new SwConditionTxtFmtColl( GetAttrPool(), aNm, !nParent
                                                ? mpDfltTxtFmtColl
                                                : GetTxtCollFromPool( nParent ));
        else
//FEATURE::CONDCOLL
            pNewColl = new SwTxtFmtColl( GetAttrPool(), aNm, !nParent
                                            ? mpDfltTxtFmtColl
                                            : GetTxtCollFromPool( nParent ));
        pNewColl->SetPoolFmtId( nId );
        mpTxtFmtCollTbl->push_back( pNewColl );
    }

    bool bNoDefault = get( IDocumentSettingAccess::STYLES_NODEFAULT );
    if ( !bNoDefault )
    {
        switch( nId )
        {
        // General content forms
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

        case RES_POOLCOLL_TEXT:                 // Text body
            {
                SvxULSpaceItem aUL( 0, PT_6, RES_UL_SPACE );
                if( get(IDocumentSettingAccess::HTML_MODE) ) aUL.SetLower( HTML_PARSPACE );
                aSet.Put( aUL );
            }
            break;
        case RES_POOLCOLL_TEXT_IDENT:           // Text body indentation
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetTxtFirstLineOfst( GetMetricVal( CM_05 ));
                aSet.Put( aLR );
            }
            break;
        case RES_POOLCOLL_TEXT_NEGIDENT:        // Text body neg. indentation
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetTxtFirstLineOfst( -(short)GetMetricVal( CM_05 ));
                aLR.SetTxtLeft( GetMetricVal( CM_1 ));
                SvxTabStopItem aTStops(RES_PARATR_TABSTOP);    aTStops.Insert( SvxTabStop( 0 ));

                aSet.Put( aLR );
                aSet.Put( aTStops );
            }
            break;
        case RES_POOLCOLL_TEXT_MOVE:            // Text body move
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetTxtLeft( GetMetricVal( CM_05 ));
                aSet.Put( aLR );
            }
            break;

        case RES_POOLCOLL_CONFRONTATION:    // Text body confrontation
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
        case RES_POOLCOLL_MARGINAL:         // Text body marginal
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetTxtLeft( GetMetricVal( CM_1 ) * 4 );
                aSet.Put( aLR );
            }
            break;

        case RES_POOLCOLL_HEADLINE_BASE:            // Base headline
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

        case RES_POOLCOLL_NUMBUL_BASE:          // Base Numbering
            break;

        case RES_POOLCOLL_GREETING:             // Greeting
        case RES_POOLCOLL_REGISTER_BASE:        // Base indexes
        case RES_POOLCOLL_SIGNATURE:            // Signatures
        case RES_POOLCOLL_TABLE:                // Tabele content
            {
                SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_HEADLINE1:        // Headinline 1
            lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 0, false );
            break;
        case RES_POOLCOLL_HEADLINE2:        // Headinline 2
            lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 1, true );
            break;
        case RES_POOLCOLL_HEADLINE3:        // Headinline 3
            lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 2, false );
            break;
        case RES_POOLCOLL_HEADLINE4:        // Headinline 4
            lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 3, true );
            break;
        case RES_POOLCOLL_HEADLINE5:        // Headinline 5
            lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 4, false );
            break;
        case RES_POOLCOLL_HEADLINE6:        // Headinline 6
            lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 5, false );
            break;
        case RES_POOLCOLL_HEADLINE7:        // Headinline 7
            lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 6, false );
            break;
        case RES_POOLCOLL_HEADLINE8:        // Headinline 8
            lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 7, false );
            break;
        case RES_POOLCOLL_HEADLINE9:        // Headinline 9
            lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 8, false );
            break;
        case RES_POOLCOLL_HEADLINE10:       // Headinline 10
            lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 9, false );
            break;

        // Special sections:
        // Header
        case RES_POOLCOLL_HEADER:
        case RES_POOLCOLL_HEADERL:
        case RES_POOLCOLL_HEADERR:
        // Footer
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

        case RES_POOLCOLL_FOOTNOTE:             // paragraph style Footnote
        case RES_POOLCOLL_ENDNOTE:              // paragraph style Endnote
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetTxtFirstLineOfst( -(short)( GetMetricVal( CM_05 ) + GetMetricVal( CM_01 ) ) );
                aLR.SetTxtLeft( GetMetricVal( CM_05 ) + GetMetricVal( CM_01 ) );
                SetAllScriptItem( aSet, SvxFontHeightItem( PT_10, 100, RES_CHRATR_FONTSIZE ) );
                aSet.Put( aLR );
                SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_LABEL:                // basic caption
            {
                SvxULSpaceItem aUL( RES_UL_SPACE ); aUL.SetUpper( PT_6 ); aUL.SetLower( PT_6 );
                aSet.Put( aUL );
                SetAllScriptItem( aSet, SvxPostureItem( ITALIC_NORMAL, RES_CHRATR_POSTURE ) );
                SetAllScriptItem( aSet, SvxFontHeightItem( PT_10, 100, RES_CHRATR_FONTSIZE ) );
                SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_FRAME:                // Frame content
        case RES_POOLCOLL_LABEL_ABB:            // caption image
        case RES_POOLCOLL_LABEL_TABLE:          // caption table
        case RES_POOLCOLL_LABEL_FRAME:          // caption frame
        case RES_POOLCOLL_LABEL_DRAWING:        // caption drawing
            break;

        case RES_POOLCOLL_JAKETADRESS:          // envelope address
            {
                SvxULSpaceItem aUL( RES_UL_SPACE ); aUL.SetLower( PT_3 );
                aSet.Put( aUL );
                SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_SENDADRESS:           // Sender address
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

        // User defined indexes:
        case RES_POOLCOLL_TOX_USERH:            // Header
            lcl_SetRegister( this, aSet, 0, true, false );
            {
                SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
                aSet.Put( aLN );
            }
            break;
        case RES_POOLCOLL_TOX_USER1:            // 1. Level
            lcl_SetRegister( this, aSet, 0, false, true );
            break;
        case RES_POOLCOLL_TOX_USER2:            // 2. Level
            lcl_SetRegister( this, aSet, 1, false, true );
            break;
        case RES_POOLCOLL_TOX_USER3:            // 3. Level
            lcl_SetRegister( this, aSet, 2, false, true );
            break;
        case RES_POOLCOLL_TOX_USER4:            // 4. Level
            lcl_SetRegister( this, aSet, 3, false, true );
            break;
        case RES_POOLCOLL_TOX_USER5:            // 5. Level
            lcl_SetRegister( this, aSet, 4, false, true );
            break;
        case RES_POOLCOLL_TOX_USER6:            // 6. Level
            lcl_SetRegister( this, aSet, 5, false, true );
            break;
        case RES_POOLCOLL_TOX_USER7:            // 7. Level
            lcl_SetRegister( this, aSet, 6, false, true );
            break;
        case RES_POOLCOLL_TOX_USER8:            // 8. Level
            lcl_SetRegister( this, aSet, 7, false, true );
            break;
        case RES_POOLCOLL_TOX_USER9:            // 9. Level
            lcl_SetRegister( this, aSet, 8, false, true );
            break;
        case RES_POOLCOLL_TOX_USER10:           // 10. Level
            lcl_SetRegister( this, aSet, 9, false, true );
            break;

        // Index
        case RES_POOLCOLL_TOX_IDXH:         // Header
            lcl_SetRegister( this, aSet, 0, true, false );
            {
                SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
                aSet.Put( aLN );
            }
            break;
        case RES_POOLCOLL_TOX_IDX1:         // 1. Level
            lcl_SetRegister( this, aSet, 0, false, false );
            break;
        case RES_POOLCOLL_TOX_IDX2:         // 2. Level
            lcl_SetRegister( this, aSet, 1, false, false );
            break;
        case RES_POOLCOLL_TOX_IDX3:         // 3. Level
            lcl_SetRegister( this, aSet, 2, false, false );
            break;
        case RES_POOLCOLL_TOX_IDXBREAK:     // Trenner
            lcl_SetRegister( this, aSet, 0, false, false );
            break;

        // Table of Content
        case RES_POOLCOLL_TOX_CNTNTH:       // Header
            lcl_SetRegister( this, aSet, 0, true, false );
            {
                SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
                aSet.Put( aLN );
            }
            break;
        case RES_POOLCOLL_TOX_CNTNT1:       // 1. Level
            lcl_SetRegister( this, aSet, 0, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT2:       // 2. Level
            lcl_SetRegister( this, aSet, 1, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT3:       // 3. Level
            lcl_SetRegister( this, aSet, 2, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT4:       // 4. Level
            lcl_SetRegister( this, aSet, 3, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT5:       // 5. Level
            lcl_SetRegister( this, aSet, 4, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT6:       // 6. Level
            lcl_SetRegister( this, aSet, 5, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT7:       // 7. Level
            lcl_SetRegister( this, aSet, 6, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT8:       // 8. Level
            lcl_SetRegister( this, aSet, 7, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT9:       // 9. Level
            lcl_SetRegister( this, aSet, 8, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT10:      // 10. Level
            lcl_SetRegister( this, aSet, 9, false, true );
            break;

        case RES_POOLCOLL_TOX_ILLUSH:
        case RES_POOLCOLL_TOX_OBJECTH:
        case RES_POOLCOLL_TOX_TABLESH:
        case RES_POOLCOLL_TOX_AUTHORITIESH:
            lcl_SetRegister( this, aSet, 0, true, false );
            {
                SwFmtLineNumber aLN; aLN.SetCountLines( sal_False );
                aSet.Put( aLN );
            }
            break;
        case RES_POOLCOLL_TOX_ILLUS1:
        case RES_POOLCOLL_TOX_OBJECT1:
        case RES_POOLCOLL_TOX_TABLES1:
        case RES_POOLCOLL_TOX_AUTHORITIES1:
            lcl_SetRegister( this, aSet, 0, false, true );
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

        case RES_POOLCOLL_DOC_TITEL:            // Document Title
            {
                SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
                SetAllScriptItem( aSet, SvxFontHeightItem( PT_18, 100, RES_CHRATR_FONTSIZE ) );

                aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER, RES_PARATR_ADJUST ) );

                pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool( RES_POOLCOLL_TEXT ));
            }
            break;

        case RES_POOLCOLL_DOC_SUBTITEL:         // Document subtitle
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

                // WORKAROUND: Set PRE to 10pt
                SetAllScriptItem( aSet, SvxFontHeightItem(PT_10, 100, RES_CHRATR_FONTSIZE) );

                // The lower paragraph distance is set explicitly (makes
                // assigning hard attributes easier)
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
                SvxBorderLine aNew(&aColor, 1, table::BorderLineStyle::DOUBLE);
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
                // We indent by 1 cm. The IDs are always 2 away from each other!
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
                // We indent by 0 cm. The IDs are always 2 away from each other!
                aLR.SetLeft( 0 );
                aSet.Put( aLR );
            }
            break;
        }
    }

    if( aSet.Count() )
    {
        {
            pNewColl->SetFmtAttr( aSet );
        }
    }
    return pNewColl;
}

/// Check if this AutoCollection is already/still in use in this Document
bool SwDoc::IsPoolTxtCollUsed( sal_uInt16 nId ) const
{
    OSL_ENSURE(
        (RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END) ||
        (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END) ||
        (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END) ||
        (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END) ||
        (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END) ||
        (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END),
            "Wrong AutoFormat Id" );

    SwTxtFmtColl* pNewColl = 0;
    bool bFnd = false;
    for( sal_uInt16 n = 0; !bFnd && n < mpTxtFmtCollTbl->size(); ++n )
    {
        pNewColl = (*mpTxtFmtCollTbl)[ n ];
        if( nId == pNewColl->GetPoolFmtId() )
            bFnd = true;
    }

    if( !bFnd || !pNewColl->GetDepends() )
        return false;

    SwAutoFmtGetDocNode aGetHt( &GetNodes() );
    return !pNewColl->GetInfo( aGetHt );
}

/// Return the AutomaticFormat with the supplied Id. If it doesn't
/// exist, create it.
SwFmt* SwDoc::GetFmtFromPool( sal_uInt16 nId )
{
    SwFmt *pNewFmt = 0;
    SwFmt *pDeriveFmt = 0;

    SwFmtsBase* pArray[ 2 ];
    sal_uInt16 nArrCnt = 1, nRCId = 0;
    sal_uInt16* pWhichRange = 0;

    switch( nId & (COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID) )
    {
    case POOLGRP_CHARFMT:
        {
            pArray[0] = mpCharFmtTbl;
            pDeriveFmt = mpDfltCharFmt;

            if( nId > RES_POOLCHR_NORMAL_END )
                nRCId = RC_POOLCHRFMT_HTML_BEGIN - RES_POOLCHR_HTML_BEGIN;
            else
                nRCId = RC_POOLCHRFMT_BEGIN - RES_POOLCHR_BEGIN;
            pWhichRange =  aCharFmtSetRange;

            // Fault: unknown Format, but a CharFormat
            //             -> return the first one
            if( RES_POOLCHR_BEGIN > nId || nId >= RES_POOLCHR_END )
            {
                OSL_ENSURE( !this, "invalid Id" );
                nId = RES_POOLCHR_BEGIN;
            }
        }
        break;
    case POOLGRP_FRAMEFMT:
        {
            pArray[0] = mpFrmFmtTbl;
            pArray[1] = mpSpzFrmFmtTbl;
            pDeriveFmt = mpDfltFrmFmt;
            nArrCnt = 2;
            nRCId = RC_POOLFRMFMT_BEGIN - RES_POOLFRM_BEGIN;
            pWhichRange = aFrmFmtSetRange;

            // Fault: unknown Format, but a FrameFormat
            //             -> return the first one
            if( RES_POOLFRM_BEGIN > nId || nId >= RES_POOLFRM_END )
            {
                OSL_ENSURE( !this, "invalid Id" );
                nId = RES_POOLFRM_BEGIN;
            }
        }
        break;

    default:
        // Fault, unknown Format
        OSL_ENSURE( nId, "invalid Id" );
        return 0;
    }
    OSL_ENSURE( nRCId, "invalid Id" );

    while( nArrCnt-- )
        for( sal_uInt16 n = 0; n < (*pArray[nArrCnt]).GetFmtCount(); ++n )
            if( nId == ( pNewFmt = (*pArray[ nArrCnt ] ).GetFmt( n ) )->
                    GetPoolFmtId() )
            {
                return pNewFmt;
            }

    ResId aResId( nRCId + nId, *pSwResMgr );
    String aNm( aResId );
    SwAttrSet aSet( GetAttrPool(), pWhichRange );

    {
        bool bIsModified = IsModified();

        {
            ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());
            switch (nId & (COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID) )
            {
                case POOLGRP_CHARFMT:
                    pNewFmt = _MakeCharFmt(aNm, pDeriveFmt, false, true);
                    break;
                case POOLGRP_FRAMEFMT:
                    pNewFmt = _MakeFrmFmt(aNm, pDeriveFmt, false, true);
                    break;
                default:
                    break;
            }
        }

        if( !bIsModified )
            ResetModified();
        pNewFmt->SetPoolFmtId( nId );
        pNewFmt->SetAuto( false );      // no AutoFormat
    }

    switch( nId )
    {
    case RES_POOLCHR_FOOTNOTE:              // Footnote
    case RES_POOLCHR_PAGENO:                // Page/Field
    case RES_POOLCHR_LABEL:                 // Label
    case RES_POOLCHR_DROPCAPS:              // Dropcaps
    case RES_POOLCHR_NUM_LEVEL:             // Numbering level
    case RES_POOLCHR_TOXJUMP:               // Table of contents jump
    case RES_POOLCHR_ENDNOTE:               // Endnote
    case RES_POOLCHR_LINENUM:               // Line numbering
        break;

    case RES_POOLCHR_ENDNOTE_ANCHOR:        // Endnote anchor
    case RES_POOLCHR_FOOTNOTE_ANCHOR:       // Footnote anchor
        {
            aSet.Put( SvxEscapementItem( DFLT_ESC_AUTO_SUPER, 58, RES_CHRATR_ESCAPEMENT ) );
        }
        break;

    case RES_POOLCHR_BUL_LEVEL:             // Bullet character
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
            if ( get(IDocumentSettingAccess::HTML_MODE) )
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
            aSet.Put( SwFmtSurround( SURROUND_IDEAL ));
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
            // Set the default width to 3.5 cm, use the minimum value for the height
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

/// Check if this AutoCollection is already/still in use
bool SwDoc::IsPoolFmtUsed( sal_uInt16 nId ) const
{
    SwFmt *pNewFmt = 0;
    const SwFmtsBase* pArray[ 2 ];
    sal_uInt16 nArrCnt = 1;
    bool bFnd = true;

    if (RES_POOLCHR_BEGIN <= nId && nId < RES_POOLCHR_END)
    {
        pArray[0] = mpCharFmtTbl;
    }
    else if (RES_POOLFRM_BEGIN <= nId && nId < RES_POOLFRM_END)
    {
        pArray[0] = mpFrmFmtTbl;
        pArray[1] = mpSpzFrmFmtTbl;
        nArrCnt = 2;
    }
    else
    {
        SAL_WARN("sw.core", "Invalid Pool Id: " << nId << " should be within " <<
            "[" << int(RES_POOLCHR_BEGIN) << "," << int(RES_POOLCHR_END) << ") or " <<
            "[" << int(RES_POOLFRM_BEGIN) << "," << int(RES_POOLFRM_END) << ")");
        bFnd = false;
    }

    if( bFnd )
    {
        bFnd = false;
        while( nArrCnt-- && !bFnd )
            for( sal_uInt16 n = 0; !bFnd && n < (*pArray[nArrCnt]).GetFmtCount(); ++n )
                if( nId == ( pNewFmt = (*pArray[ nArrCnt ] ).GetFmt( n ) )->
                        GetPoolFmtId() )
                    bFnd = true;
    }

    // Not found or no dependecies?
    if( bFnd && pNewFmt->GetDepends() )
    {
        // Check if we have dependent ContentNodes in the Nodes array
        // (also indirect ones for derived Formats)
        SwAutoFmtGetDocNode aGetHt( &GetNodes() );
        bFnd = !pNewFmt->GetInfo( aGetHt );
    }
    else
        bFnd = false;

    return bFnd;
}

static void lcl_PutStdPageSizeIntoItemSet( SwDoc* pDoc, SfxItemSet& rSet )
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
            "Wrong AutoFormat Id" );

    for( sal_uInt16 n = 0; n < maPageDescs.size(); ++n )
    {
        if ( nId == maPageDescs[ n ]->GetPoolFmtId() )
        {
            return maPageDescs[ n ];
        }
    }

    if( RES_POOLPAGE_BEGIN > nId ||  nId >= RES_POOLPAGE_END )
    {
        // unknown page pool ID
        OSL_ENSURE( !this, "<SwDoc::GetPageDescFromPool(..)> - unknown page pool ID" );
        nId = RES_POOLPAGE_BEGIN;
    }

    SwPageDesc* pNewPgDsc = 0;
    {
        const ResId aResId( sal_uInt32(RC_POOLPAGEDESC_BEGIN + nId - RES_POOLPAGE_BEGIN), *pSwResMgr );
        const String aNm( aResId );
        const bool bIsModified = IsModified();

        sal_uInt16 nPageDescIdx = 0;
        {
            ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());
            nPageDescIdx = MakePageDesc( aNm, 0, bRegardLanguage );
        }

        pNewPgDsc = maPageDescs[ nPageDescIdx ];
        pNewPgDsc->SetPoolFmtId( nId );
        if ( !bIsModified )
        {
            ResetModified();
        }
    }

    SvxLRSpaceItem aLR( RES_LR_SPACE );
    {
        aLR.SetLeft( GetMetricVal( CM_1 ) * 2 );
        aLR.SetRight( aLR.GetLeft() );
    }
    SvxULSpaceItem aUL( RES_UL_SPACE );
    {
        aUL.SetUpper( (sal_uInt16)aLR.GetLeft() );
        aUL.SetLower( (sal_uInt16)aLR.GetLeft() );
    }

    SwAttrSet aSet( GetAttrPool(), aPgFrmFmtSetRange );
    bool bSetLeft = true;

    switch( nId )
    {
    case RES_POOLPAGE_STANDARD:     // "Default"
        {
            aSet.Put( aLR );
            aSet.Put( aUL );
            if( pNewPgDsc )
                pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL | nsUseOnPage::PD_FIRSTSHARE );
        }
        break;

    case RES_POOLPAGE_FIRST:        // "First Page"
    case RES_POOLPAGE_REGISTER:     // "Index"
        {
            lcl_PutStdPageSizeIntoItemSet( this, aSet );
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

    case RES_POOLPAGE_LEFT:         // "Left Page"
        {
            lcl_PutStdPageSizeIntoItemSet( this, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            bSetLeft = false;
            if( pNewPgDsc )
            {
                pNewPgDsc->SetUseOn( nsUseOnPage::PD_LEFT );
                // this relies on GetPageDescFromPool() not going into infinite recursion
                // (by this point RES_POOLPAGE_LEFT will not reach this place again)
                pNewPgDsc->SetFollow( GetPageDescFromPool( RES_POOLPAGE_RIGHT ));
            }
        }
        break;
    case RES_POOLPAGE_RIGHT:        // "Right Page"
        {
            lcl_PutStdPageSizeIntoItemSet( this, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            bSetLeft = false;
            if( pNewPgDsc )
            {
                pNewPgDsc->SetUseOn( nsUseOnPage::PD_RIGHT );
                pNewPgDsc->SetFollow( GetPageDescFromPool( RES_POOLPAGE_LEFT ));
            }
        }
        break;

    case RES_POOLPAGE_JAKET:        // "Envelope"
        {
            Size aPSize( SvxPaperInfo::GetPaperSize( PAPER_ENV_C65 ) );
            LandscapeSwap( aPSize );
            aSet.Put( SwFmtFrmSize( ATT_FIX_SIZE, aPSize.Width(), aPSize.Height() ));
            aLR.SetLeft( 0 ); aLR.SetRight( 0 );
            aUL.SetUpper( 0 ); aUL.SetLower( 0 );
            aSet.Put( aLR );
            aSet.Put( aUL );

            if( pNewPgDsc )
            {
                pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL );
                pNewPgDsc->SetLandscape( sal_True );
            }
        }
        break;

    case RES_POOLPAGE_HTML:         // "HTML"
        {
            lcl_PutStdPageSizeIntoItemSet( this, aSet );
            aLR.SetRight( GetMetricVal( CM_1 ));
            aUL.SetUpper( (sal_uInt16)aLR.GetRight() );
            aUL.SetLower( (sal_uInt16)aLR.GetRight() );
            aSet.Put( aLR );
            aSet.Put( aUL );

            if( pNewPgDsc )
                pNewPgDsc->SetUseOn( nsUseOnPage::PD_ALL );
        }
        break;

    case RES_POOLPAGE_FOOTNOTE:     // "Footnote"
    case RES_POOLPAGE_ENDNOTE:      // "Endnote"
        {
            lcl_PutStdPageSizeIntoItemSet( this, aSet );
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

    case RES_POOLPAGE_LANDSCAPE:    // "Landscape"
        {
            SwPageDesc* pStdPgDsc = this->GetPageDescFromPool( RES_POOLPAGE_STANDARD );
            SwFmtFrmSize aFrmSz( pStdPgDsc->GetMaster().GetFrmSize() );
            if ( !pStdPgDsc->GetLandscape() )
            {
                const SwTwips nTmp = aFrmSz.GetHeight();
                aFrmSz.SetHeight( aFrmSz.GetWidth() );
                aFrmSz.SetWidth( nTmp );
            }
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
        if( bSetLeft )
            pNewPgDsc->GetLeft().SetFmtAttr( aSet );
        pNewPgDsc->GetMaster().SetFmtAttr( aSet );
        pNewPgDsc->GetFirst().SetFmtAttr( aSet );
    }
    return pNewPgDsc;
}

SwNumRule* SwDoc::GetNumRuleFromPool( sal_uInt16 nId )
{
    OSL_ENSURE( RES_POOLNUMRULE_BEGIN <= nId && nId < RES_POOLNUMRULE_END,
            "Wrong AutoFormat Id" );

    SwNumRule* pNewRule;
    sal_uInt16 n;

    for( n = 0; n < GetNumRuleTbl().size(); ++n )
        if( nId == ( pNewRule = GetNumRuleTbl()[ n ] )->GetPoolFmtId() )
        {
            return pNewRule;
        }

    // error: unknown Pool style
    if( RES_POOLNUMRULE_BEGIN > nId ||  nId >= RES_POOLNUMRULE_END )
    {
        OSL_ENSURE( !this, "invalid Id" );
        nId = RES_POOLNUMRULE_BEGIN;
    }

    ResId aResId( sal_uInt32(RC_POOLNUMRULE_BEGIN + nId - RES_POOLNUMRULE_BEGIN), *pSwResMgr );
    String aNm( aResId );

    SwCharFmt *pNumCFmt = 0, *pBullCFmt = 0;

    const SvxNumberFormat::SvxNumPositionAndSpaceMode eNumberFormatPositionAndSpaceMode
                                  = numfunc::GetDefaultPositionAndSpaceMode(); //#i89178#
    {
        bool bIsModified = IsModified();

        n = MakeNumRule( aNm, 0, false, eNumberFormatPositionAndSpaceMode );

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
            aFmt.SetSuffix(OUString(static_cast<sal_Unicode>(')')));
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

/// Check if this AutoCollection is already/still in use in this Document
bool SwDoc::IsPoolPageDescUsed( sal_uInt16 nId ) const
{
    OSL_ENSURE( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END,
            "Wrong AutoFormat Id" );
    SwPageDesc *pNewPgDsc = 0;
    bool bFnd = false;
    for( sal_uInt16 n = 0; !bFnd && n < maPageDescs.size(); ++n )
    {
        pNewPgDsc = maPageDescs[ n ];
        if( nId == pNewPgDsc->GetPoolFmtId() )
            bFnd = true;
    }

    // Not found or no dependencies?
    if( !bFnd || !pNewPgDsc->GetDepends() )     // ??????
        return false;

    // Check if we have dependent ContentNodes in the Nodes array
    // (also indirect ones for derived Formats)
    SwAutoFmtGetDocNode aGetHt( &GetNodes() );
    return !pNewPgDsc->GetInfo( aGetHt );
}

// See if the Paragraph/Character/Frame/Page style is in use
bool SwDoc::IsUsed( const SwModify& rModify ) const
{
    // Check if we have dependent ContentNodes in the Nodes array
    // (also indirect ones for derived Formats)
    SwAutoFmtGetDocNode aGetHt( &GetNodes() );
    return !rModify.GetInfo( aGetHt );
}

// See if the NumRule is used
bool SwDoc::IsUsed( const SwNumRule& rRule ) const
{
    bool bUsed = rRule.GetTxtNodeListSize() > 0 ||
                     rRule.GetParagraphStyleListSize() > 0;

    return bUsed;
}

// Look for the style name's position. If it doesn't exist,
// insert a anew
sal_uInt16 SwDoc::SetDocPattern( const String& rPatternName )
{
    OSL_ENSURE( rPatternName.Len(), "no Document style name" );

    size_t nNewPos = maPatternNms.size();
    for(size_t n = 0; n < maPatternNms.size(); ++n)
        if( boost::is_null(maPatternNms.begin() + n) )
        {
            if( nNewPos == maPatternNms.size() )
                nNewPos = n;
        }
        else if( rPatternName == maPatternNms[n] )
            return n;

    if( nNewPos < maPatternNms.size() )
        maPatternNms.erase(maPatternNms.begin() + nNewPos);   // Free space again

    maPatternNms.insert(maPatternNms.begin() + nNewPos, new String(rPatternName));
    SetModified();
    return nNewPos;
}

sal_uInt16 GetPoolParent( sal_uInt16 nId )
{
    sal_uInt16 nRet = USHRT_MAX;
    if( POOLGRP_NOCOLLID & nId )        // 1 == Formats / 0 == Collections
    {
        switch( ( COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID ) & nId )
        {
        case POOLGRP_CHARFMT:
        case POOLGRP_FRAMEFMT:
            nRet = 0;           // derived from the default
            break;
        case POOLGRP_PAGEDESC:
        case POOLGRP_NUMRULE:
            break;              // there are no derivations
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
            case RES_POOLCOLL_TABLE_HDLN:
                    nRet = RES_POOLCOLL_TABLE;                  break;

            case RES_POOLCOLL_FRAME:
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
        SwPageDesc& rDesc = GetPageDesc( i );
        rDesc.GetMaster().SetFmtAttr( aFrameDir );
        rDesc.GetLeft().SetFmtAttr( aFrameDir );
    }

    //#i16874# AutoKerning as default for new documents
    GetAttrPool().ResetPoolDefaultItem( RES_CHRATR_AUTOKERN );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
