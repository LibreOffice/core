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
#include "precompiled_editeng.hxx"

#include <editeng/flstitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fwdtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/akrnitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/prszitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/cscoitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/nlbkitem.hxx>
#include <editeng/nhypitem.hxx>
#include <editeng/lcolitem.hxx>
#include <editeng/blnkitem.hxx>
#include <editeng/emphitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/bolnitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/pmdlitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/hyznitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/paravertalignitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/charhiddenitem.hxx>

#include <svtools/rtftoken.h>
#include <svl/itempool.hxx>
#include <svl/itemiter.hxx>

#include <editeng/svxrtf.hxx>
#include <editeng/editids.hrc>

#define BRACELEFT   '{'
#define BRACERIGHT  '}'


// Some helper functions
// char
inline const SvxEscapementItem& GetEscapement(const SfxItemSet& rSet,sal_uInt16 nId,sal_Bool bInP=sal_True)
    { return (const SvxEscapementItem&)rSet.Get( nId,bInP); }
inline const SvxLineSpacingItem& GetLineSpacing(const SfxItemSet& rSet,sal_uInt16 nId,sal_Bool bInP=sal_True)
    { return (const SvxLineSpacingItem&)rSet.Get( nId,bInP); }
// frm
inline const SvxLRSpaceItem& GetLRSpace(const SfxItemSet& rSet,sal_uInt16 nId,sal_Bool bInP=sal_True)
    { return (const SvxLRSpaceItem&)rSet.Get( nId,bInP); }
inline const SvxULSpaceItem& GetULSpace(const SfxItemSet& rSet,sal_uInt16 nId,sal_Bool bInP=sal_True)
    { return (const SvxULSpaceItem&)rSet.Get( nId,bInP); }

#define PARDID      ((RTFPardAttrMapIds*)aPardMap.GetData())
#define PLAINID     ((RTFPlainAttrMapIds*)aPlainMap.GetData())

void SvxRTFParser::SetScriptAttr( RTF_CharTypeDef eType, SfxItemSet& rSet,
                                    SfxPoolItem& rItem )
{
    const sal_uInt16 *pNormal = 0, *pCJK = 0, *pCTL = 0;
    const RTFPlainAttrMapIds* pIds = (RTFPlainAttrMapIds*)aPlainMap.GetData();
    switch( rItem.Which() )
    {
    case SID_ATTR_CHAR_FONT:
        pNormal = &pIds->nFont;
        pCJK = &pIds->nCJKFont;
        pCTL = &pIds->nCTLFont;
        break;

    case SID_ATTR_CHAR_FONTHEIGHT:
        pNormal = &pIds->nFontHeight;
        pCJK = &pIds->nCJKFontHeight;
        pCTL = &pIds->nCTLFontHeight;
        break;

    case SID_ATTR_CHAR_POSTURE:
        pNormal = &pIds->nPosture;
        pCJK = &pIds->nCJKPosture;
        pCTL = &pIds->nCTLPosture;
        break;

    case SID_ATTR_CHAR_WEIGHT:
        pNormal = &pIds->nWeight;
        pCJK = &pIds->nCJKWeight;
        pCTL = &pIds->nCTLWeight;
        break;

    case SID_ATTR_CHAR_LANGUAGE:
        pNormal = &pIds->nLanguage;
        pCJK = &pIds->nCJKLanguage;
        pCTL = &pIds->nCTLLanguage;
        break;

    case 0:
        // it exist no WhichId - don't set this item
        break;

    default:
       rSet.Put( rItem );
       break;
    }


    if( DOUBLEBYTE_CHARTYPE == eType )
    {
        if( bIsLeftToRightDef && *pCJK )
        {
            rItem.SetWhich( *pCJK );
            rSet.Put( rItem );
        }
    }
    else if( !bIsLeftToRightDef )
    {
        if( *pCTL )
        {
            rItem.SetWhich( *pCTL );
            rSet.Put( rItem );
        }
    }
    else
    {
        if( LOW_CHARTYPE == eType )
        {
            if( *pNormal )
            {
                rItem.SetWhich( *pNormal );
                rSet.Put( rItem );
            }
        }
        else if( HIGH_CHARTYPE == eType )
        {
            if( *pCTL )
            {
                rItem.SetWhich( *pCTL );
                rSet.Put( rItem );
            }
        }
        else
        {
            if( *pCJK )
            {
                rItem.SetWhich( *pCJK );
                rSet.Put( rItem );
            }
            if( *pCTL )
            {
                rItem.SetWhich( *pCTL );
                rSet.Put( rItem );
            }
            if( *pNormal )
            {
                rItem.SetWhich( *pNormal );
                rSet.Put( rItem );
            }
        }
    }
}

// --------------------

void SvxRTFParser::ReadAttr( int nToken, SfxItemSet* pSet )
{
    DBG_ASSERT( pSet, "A SfxItemSet has to be provided as argument!" );
    int bFirstToken = sal_True, bWeiter = sal_True;
    sal_uInt16 nStyleNo = 0;        // default
    FontUnderline eUnderline;
    FontUnderline eOverline;
    FontEmphasisMark eEmphasis;
    bPardTokenRead = sal_False;
    RTF_CharTypeDef eCharType = NOTDEF_CHARTYPE;
    sal_uInt16 nFontAlign;

    int bChkStkPos = !bNewGroup && !aAttrStack.empty();

    while( bWeiter && IsParserWorking() )  // as long as known Attribute are recognized
    {
        switch( nToken )
        {
        case RTF_PARD:
            RTFPardPlain( sal_True, &pSet );
            ResetPard();
            nStyleNo = 0;
            bPardTokenRead = sal_True;
            break;

        case RTF_PLAIN:
            RTFPardPlain( sal_False, &pSet );
            break;

        default:
            do {        // middle checked loop
                if( !bChkStkPos )
                    break;

                SvxRTFItemStackType* pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();
                if( !pAkt || (pAkt->pSttNd->GetIdx() == pInsPos->GetNodeIdx() &&
                    pAkt->nSttCnt == pInsPos->GetCntIdx() ))
                    break;

                int nLastToken = GetStackPtr(-1)->nTokenId;
                if( RTF_PARD == nLastToken || RTF_PLAIN == nLastToken )
                    break;

                if( pAkt->aAttrSet.Count() || pAkt->pChildList ||
                    pAkt->nStyleNo )
                {
                    // Open a new Group
                    SvxRTFItemStackType* pNew = new SvxRTFItemStackType(
                                                *pAkt, *pInsPos, sal_True );
                    pNew->SetRTFDefaults( GetRTFDefaults() );

                    // "Set" all valid attributes up until this point
                    AttrGroupEnd();
                    pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();  // can be changed after AttrGroupEnd!
                    pNew->aAttrSet.SetParent( pAkt ? &pAkt->aAttrSet : 0 );

                    aAttrStack.push_back( pNew );
                    pAkt = pNew;
                }
                else
                    // continue to use this entry as a new one
                    pAkt->SetStartPos( *pInsPos );

                pSet = &pAkt->aAttrSet;
            } while( sal_False );

            switch( nToken )
            {
            case RTF_INTBL:
            case RTF_PAGEBB:
            case RTF_SBYS:
            case RTF_CS:
            case RTF_LS:
            case RTF_ILVL:
                    UnknownAttrToken( nToken, pSet );
                    break;

            case RTF_S:
                if( bIsInReadStyleTab )
                {
                    if( !bFirstToken )
                        SkipToken( -1 );
                    bWeiter = sal_False;
                }
                else
                {
                    nStyleNo = -1 == nTokenValue ? 0 : sal_uInt16(nTokenValue);
                    // setze am akt. auf dem AttrStack stehenden Style die
                    // StyleNummer
                    SvxRTFItemStackType* pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();
                    if( !pAkt )
                        break;

                    pAkt->nStyleNo = sal_uInt16( nStyleNo );

                }
                break;

            case RTF_KEEP:
                if( PARDID->nSplit )
                {
                    pSet->Put( SvxFmtSplitItem( sal_False, PARDID->nSplit ));
                }
                break;

            case RTF_KEEPN:
                if( PARDID->nKeep )
                {
                    pSet->Put( SvxFmtKeepItem( sal_True, PARDID->nKeep ));
                }
                break;

            case RTF_LEVEL:
                if( PARDID->nOutlineLvl )
                {
                    pSet->Put( SfxUInt16Item( PARDID->nOutlineLvl,
                                                (sal_uInt16)nTokenValue ));
                }
                break;

            case RTF_QL:
                if( PARDID->nAdjust )
                {
                    pSet->Put( SvxAdjustItem( SVX_ADJUST_LEFT, PARDID->nAdjust ));
                }
                break;
            case RTF_QR:
                if( PARDID->nAdjust )
                {
                    pSet->Put( SvxAdjustItem( SVX_ADJUST_RIGHT, PARDID->nAdjust ));
                }
                break;
            case RTF_QJ:
                if( PARDID->nAdjust )
                {
                    pSet->Put( SvxAdjustItem( SVX_ADJUST_BLOCK, PARDID->nAdjust ));
                }
                break;
            case RTF_QC:
                if( PARDID->nAdjust )
                {
                    pSet->Put( SvxAdjustItem( SVX_ADJUST_CENTER, PARDID->nAdjust ));
                }
                break;

            case RTF_FI:
                if( PARDID->nLRSpace )
                {
                    SvxLRSpaceItem aLR( GetLRSpace(*pSet, PARDID->nLRSpace ));
                    sal_uInt16 nSz = 0;
                    if( -1 != nTokenValue )
                    {
                        if( IsCalcValue() )
                            CalcValue();
                        nSz = sal_uInt16(nTokenValue);
                    }
                    aLR.SetTxtFirstLineOfst( nSz );
                    pSet->Put( aLR );
                }
                break;

            case RTF_LI:
            case RTF_LIN:
                if( PARDID->nLRSpace )
                {
                    SvxLRSpaceItem aLR( GetLRSpace(*pSet, PARDID->nLRSpace ));
                    sal_uInt16 nSz = 0;
                    if( 0 < nTokenValue )
                    {
                        if( IsCalcValue() )
                            CalcValue();
                        nSz = sal_uInt16(nTokenValue);
                    }
                    aLR.SetTxtLeft( nSz );
                    pSet->Put( aLR );
                }
                break;

            case RTF_RI:
            case RTF_RIN:
                if( PARDID->nLRSpace )
                {
                    SvxLRSpaceItem aLR( GetLRSpace(*pSet, PARDID->nLRSpace ));
                    sal_uInt16 nSz = 0;
                    if( 0 < nTokenValue )
                    {
                        if( IsCalcValue() )
                            CalcValue();
                        nSz = sal_uInt16(nTokenValue);
                    }
                    aLR.SetRight( nSz );
                    pSet->Put( aLR );
                }
                break;

            case RTF_SB:
                if( PARDID->nULSpace )
                {
                    SvxULSpaceItem aUL( GetULSpace(*pSet, PARDID->nULSpace ));
                    sal_uInt16 nSz = 0;
                    if( 0 < nTokenValue )
                    {
                        if( IsCalcValue() )
                            CalcValue();
                        nSz = sal_uInt16(nTokenValue);
                    }
                    aUL.SetUpper( nSz );
                    pSet->Put( aUL );
                }
                break;

            case RTF_SA:
                if( PARDID->nULSpace )
                {
                    SvxULSpaceItem aUL( GetULSpace(*pSet, PARDID->nULSpace ));
                    sal_uInt16 nSz = 0;
                    if( 0 < nTokenValue )
                    {
                        if( IsCalcValue() )
                            CalcValue();
                        nSz = sal_uInt16(nTokenValue);
                    }
                    aUL.SetLower( nSz );
                    pSet->Put( aUL );
                }
                break;

            case RTF_SLMULT:
                if( PARDID->nLinespacing && 1 == nTokenValue )
                {
                    // then switches to multi-line!
                    SvxLineSpacingItem aLSpace( GetLineSpacing( *pSet,
                                                PARDID->nLinespacing, sal_False ));

                    // how much do you get from the line height value?

                    // Proportional-Size:
                    // Ie, the ratio is (n / 240) twips

                    nTokenValue = 240;
                    if( IsCalcValue() )
                        CalcValue();

                    nTokenValue = short( 100L * aLSpace.GetLineHeight()
                                            / long( nTokenValue ) );

                    if( nTokenValue > 200 )     // Data value for PropLnSp
                        nTokenValue = 200;      // is one BYTE !!!

                    aLSpace.SetPropLineSpace( (const sal_uInt8)nTokenValue );
                    aLSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;

                    pSet->Put( aLSpace );
                }
                break;

            case RTF_SL:
                if( PARDID->nLinespacing )
                {
                    // Calculate the ratio between the default font and the
                    // specified size. The distance consists of the line height
                    // (100%) and the space above the line (20%).
                    SvxLineSpacingItem aLSpace(0, PARDID->nLinespacing);

                    nTokenValue = !bTokenHasValue ? 0 : nTokenValue;
                    if (1000 == nTokenValue )
                        nTokenValue = 240;

                    SvxLineSpace eLnSpc;
                    if (nTokenValue < 0)
                    {
                        eLnSpc = SVX_LINE_SPACE_FIX;
                        nTokenValue = -nTokenValue;
                    }
                    else if (nTokenValue == 0)
                    {
                        //if \sl0 is used, the line spacing is automatically
                        //determined
                        eLnSpc = SVX_LINE_SPACE_AUTO;
                    }
                    else
                        eLnSpc = SVX_LINE_SPACE_MIN;

                    if (IsCalcValue())
                        CalcValue();

                    if (eLnSpc != SVX_LINE_SPACE_AUTO)
                        aLSpace.SetLineHeight( (const sal_uInt16)nTokenValue );

                    aLSpace.GetLineSpaceRule() = eLnSpc;
                    pSet->Put(aLSpace);
                }
                break;

            case RTF_NOCWRAP:
                if( PARDID->nForbRule )
                {
                    pSet->Put( SvxForbiddenRuleItem( sal_False,
                                                    PARDID->nForbRule ));
                }
                break;
            case RTF_NOOVERFLOW:
                if( PARDID->nHangPunct )
                {
                    pSet->Put( SvxHangingPunctuationItem( sal_False,
                                                    PARDID->nHangPunct ));
                }
                break;

            case RTF_ASPALPHA:
                if( PARDID->nScriptSpace )
                {
                    pSet->Put( SvxScriptSpaceItem( sal_True,
                                                PARDID->nScriptSpace ));
                }
                break;

            case RTF_FAFIXED:
            case RTF_FAAUTO:    nFontAlign = SvxParaVertAlignItem::AUTOMATIC;
                                goto SET_FONTALIGNMENT;
            case RTF_FAHANG:    nFontAlign = SvxParaVertAlignItem::TOP;
                                goto SET_FONTALIGNMENT;
            case RTF_FAVAR:     nFontAlign = SvxParaVertAlignItem::BOTTOM;
                                goto SET_FONTALIGNMENT;
            case RTF_FACENTER:  nFontAlign = SvxParaVertAlignItem::CENTER;
                                goto SET_FONTALIGNMENT;
            case RTF_FAROMAN:   nFontAlign = SvxParaVertAlignItem::BASELINE;
                                goto SET_FONTALIGNMENT;
SET_FONTALIGNMENT:
            if( PARDID->nFontAlign )
            {
                pSet->Put( SvxParaVertAlignItem( nFontAlign,
                                                PARDID->nFontAlign ));
            }
            break;

            case RTF_B:
            case RTF_AB:
                if( IsAttrSttPos() )    // not in the text flow?
                {

                    SvxWeightItem aTmpItem(
                                    nTokenValue ? WEIGHT_BOLD : WEIGHT_NORMAL,
                                    SID_ATTR_CHAR_WEIGHT );
                    SetScriptAttr( eCharType, *pSet, aTmpItem);
                }
                break;

            case RTF_CAPS:
            case RTF_SCAPS:
                if( PLAINID->nCaseMap &&
                    IsAttrSttPos() )        // not in the text flow?
                {
                    SvxCaseMap eCaseMap;
                    if( !nTokenValue )
                        eCaseMap = SVX_CASEMAP_NOT_MAPPED;
                    else if( RTF_CAPS == nToken )
                        eCaseMap = SVX_CASEMAP_VERSALIEN;
                    else
                        eCaseMap = SVX_CASEMAP_KAPITAELCHEN;

                    pSet->Put( SvxCaseMapItem( eCaseMap, PLAINID->nCaseMap ));
                }
                break;

            case RTF_DN:
            case RTF_SUB:
                if( PLAINID->nEscapement )
                {
                    const sal_uInt16 nEsc = PLAINID->nEscapement;
                    if( -1 == nTokenValue || RTF_SUB == nToken )
                        nTokenValue = 6;
                    if( IsCalcValue() )
                        CalcValue();
                    const SvxEscapementItem& rOld = GetEscapement( *pSet, nEsc, sal_False );
                    short nEs;
                    sal_uInt8 nProp;
                    if( DFLT_ESC_AUTO_SUPER == rOld.GetEsc() )
                    {
                        nEs = DFLT_ESC_AUTO_SUB;
                        nProp = rOld.GetProp();
                    }
                    else
                    {
                        nEs = (short)-nTokenValue;
                        nProp = (nToken == RTF_SUB) ? DFLT_ESC_PROP : 100;
                    }
                    pSet->Put( SvxEscapementItem( nEs, nProp, nEsc ));
                }
                break;

            case RTF_NOSUPERSUB:
                if( PLAINID->nEscapement )
                {
                    const sal_uInt16 nEsc = PLAINID->nEscapement;
                    pSet->Put( SvxEscapementItem( nEsc ));
                }
                break;

            case RTF_EXPND:
                if( PLAINID->nKering )
                {
                    if( -1 == nTokenValue )
                        nTokenValue = 0;
                    else
                        nTokenValue *= 5;
                    if( IsCalcValue() )
                        CalcValue();
                    pSet->Put( SvxKerningItem( (short)nTokenValue, PLAINID->nKering ));
                }
                break;

            case RTF_KERNING:
                if( PLAINID->nAutoKerning )
                {
                    if( -1 == nTokenValue )
                        nTokenValue = 0;
                    else
                        nTokenValue *= 10;
                    if( IsCalcValue() )
                        CalcValue();
                    pSet->Put( SvxAutoKernItem( 0 != nTokenValue,
                                                PLAINID->nAutoKerning ));
                }
                break;

            case RTF_EXPNDTW:
                if( PLAINID->nKering )
                {
                    if( -1 == nTokenValue )
                        nTokenValue = 0;
                    if( IsCalcValue() )
                        CalcValue();
                    pSet->Put( SvxKerningItem( (short)nTokenValue, PLAINID->nKering ));
                }
                break;

            case RTF_F:
            case RTF_AF:
                {
                    const Font& rSVFont = GetFont( sal_uInt16(nTokenValue) );
                    SvxFontItem aTmpItem( rSVFont.GetFamily(),
                                    rSVFont.GetName(), rSVFont.GetStyleName(),
                                    rSVFont.GetPitch(), rSVFont.GetCharSet(),
                                    SID_ATTR_CHAR_FONT );
                    SetScriptAttr( eCharType, *pSet, aTmpItem );
                    if( RTF_F == nToken )
                    {
                        SetEncoding( rSVFont.GetCharSet() );
                        RereadLookahead();
                    }
                }
                break;

            case RTF_FS:
            case RTF_AFS:
                {
                    if( -1 == nTokenValue )
                        nTokenValue = 240;
                    else
                        nTokenValue *= 10;
// #i66167#
// for the SwRTFParser 'IsCalcValue' will be false and for the EditRTFParser
// the converiosn takes now place in EditRTFParser since for other reasons
// the wrong MapUnit might still be use there
//                   if( IsCalcValue() )
//                       CalcValue();
                    SvxFontHeightItem aTmpItem(
                            (const sal_uInt16)nTokenValue, 100,
                            SID_ATTR_CHAR_FONTHEIGHT );
                    SetScriptAttr( eCharType, *pSet, aTmpItem );
                }
                break;

            case RTF_I:
            case RTF_AI:
                if( IsAttrSttPos() )        // not in the text flow?
                {
                    SvxPostureItem aTmpItem(
                                    nTokenValue ? ITALIC_NORMAL : ITALIC_NONE,
                                    SID_ATTR_CHAR_POSTURE );
                    SetScriptAttr( eCharType, *pSet, aTmpItem );
                }
                break;

            case RTF_OUTL:
                if( PLAINID->nContour &&
                    IsAttrSttPos() )        // not in the text flow?
                {
                    pSet->Put( SvxContourItem( nTokenValue ? sal_True : sal_False,
                                PLAINID->nContour ));
                }
                break;

            case RTF_SHAD:
                if( PLAINID->nShadowed &&
                    IsAttrSttPos() )        // not in the text flow?
                {
                    pSet->Put( SvxShadowedItem( nTokenValue ? sal_True : sal_False,
                                PLAINID->nShadowed ));
                }
                break;

            case RTF_STRIKE:
                if( PLAINID->nCrossedOut &&
                    IsAttrSttPos() )        // not in the text flow?
                {
                    pSet->Put( SvxCrossedOutItem(
                        nTokenValue ? STRIKEOUT_SINGLE : STRIKEOUT_NONE,
                        PLAINID->nCrossedOut ));
                }
                break;

            case RTF_STRIKED:
                if( PLAINID->nCrossedOut )      // not in the text flow?
                {
                    pSet->Put( SvxCrossedOutItem(
                        nTokenValue ? STRIKEOUT_DOUBLE : STRIKEOUT_NONE,
                        PLAINID->nCrossedOut ));
                }
                break;

            case RTF_UL:
                if( !IsAttrSttPos() )
                    break;
                eUnderline = nTokenValue ? UNDERLINE_SINGLE : UNDERLINE_NONE;
                goto ATTR_SETUNDERLINE;

            case RTF_ULD:
                eUnderline = UNDERLINE_DOTTED;
                goto ATTR_SETUNDERLINE;
            case RTF_ULDASH:
                eUnderline = UNDERLINE_DASH;
                goto ATTR_SETUNDERLINE;
            case RTF_ULDASHD:
                eUnderline = UNDERLINE_DASHDOT;
                goto ATTR_SETUNDERLINE;
            case RTF_ULDASHDD:
                eUnderline = UNDERLINE_DASHDOTDOT;
                goto ATTR_SETUNDERLINE;
            case RTF_ULDB:
                eUnderline = UNDERLINE_DOUBLE;
                goto ATTR_SETUNDERLINE;
            case RTF_ULNONE:
                eUnderline = UNDERLINE_NONE;
                goto ATTR_SETUNDERLINE;
            case RTF_ULTH:
                eUnderline = UNDERLINE_BOLD;
                goto ATTR_SETUNDERLINE;
            case RTF_ULWAVE:
                eUnderline = UNDERLINE_WAVE;
                goto ATTR_SETUNDERLINE;
            case RTF_ULTHD:
                eUnderline = UNDERLINE_BOLDDOTTED;
                goto ATTR_SETUNDERLINE;
            case RTF_ULTHDASH:
                eUnderline = UNDERLINE_BOLDDASH;
                goto ATTR_SETUNDERLINE;
            case RTF_ULLDASH:
                eUnderline = UNDERLINE_LONGDASH;
                goto ATTR_SETUNDERLINE;
            case RTF_ULTHLDASH:
                eUnderline = UNDERLINE_BOLDLONGDASH;
                goto ATTR_SETUNDERLINE;
            case RTF_ULTHDASHD:
                eUnderline = UNDERLINE_BOLDDASHDOT;
                goto ATTR_SETUNDERLINE;
            case RTF_ULTHDASHDD:
                eUnderline = UNDERLINE_BOLDDASHDOTDOT;
                goto ATTR_SETUNDERLINE;
            case RTF_ULHWAVE:
                eUnderline = UNDERLINE_BOLDWAVE;
                goto ATTR_SETUNDERLINE;
            case RTF_ULULDBWAVE:
                eUnderline = UNDERLINE_DOUBLEWAVE;
                goto ATTR_SETUNDERLINE;

            case RTF_ULW:
                eUnderline = UNDERLINE_SINGLE;

                if( PLAINID->nWordlineMode )
                {
                    pSet->Put( SvxWordLineModeItem( sal_True, PLAINID->nWordlineMode ));
                }
                goto ATTR_SETUNDERLINE;

ATTR_SETUNDERLINE:
                if( PLAINID->nUnderline )
                {
                    pSet->Put( SvxUnderlineItem( eUnderline, PLAINID->nUnderline ));
                }
                break;

            case RTF_ULC:
                if( PLAINID->nUnderline )
                {
                    SvxUnderlineItem aUL( UNDERLINE_SINGLE, PLAINID->nUnderline );
                    const SfxPoolItem* pItem;
                    if( SFX_ITEM_SET == pSet->GetItemState(
                        PLAINID->nUnderline, sal_False, &pItem ) )
                    {
                        // is switched off ?
                        if( UNDERLINE_NONE ==
                            ((SvxUnderlineItem*)pItem)->GetLineStyle() )
                            break;
                        aUL = *(SvxUnderlineItem*)pItem;
                    }
                    else
                        aUL = (const SvxUnderlineItem&)pSet->Get( PLAINID->nUnderline, sal_False );

                    if( UNDERLINE_NONE == aUL.GetLineStyle() )
                        aUL.SetLineStyle( UNDERLINE_SINGLE );
                    aUL.SetColor( GetColor( sal_uInt16(nTokenValue) ));
                    pSet->Put( aUL );
                }
                break;

            case RTF_OL:
                if( !IsAttrSttPos() )
                    break;
                eOverline = nTokenValue ? UNDERLINE_SINGLE : UNDERLINE_NONE;
                goto ATTR_SETOVERLINE;

            case RTF_OLD:
                eOverline = UNDERLINE_DOTTED;
                goto ATTR_SETOVERLINE;
            case RTF_OLDASH:
                eOverline = UNDERLINE_DASH;
                goto ATTR_SETOVERLINE;
            case RTF_OLDASHD:
                eOverline = UNDERLINE_DASHDOT;
                goto ATTR_SETOVERLINE;
            case RTF_OLDASHDD:
                eOverline = UNDERLINE_DASHDOTDOT;
                goto ATTR_SETOVERLINE;
            case RTF_OLDB:
                eOverline = UNDERLINE_DOUBLE;
                goto ATTR_SETOVERLINE;
            case RTF_OLNONE:
                eOverline = UNDERLINE_NONE;
                goto ATTR_SETOVERLINE;
            case RTF_OLTH:
                eOverline = UNDERLINE_BOLD;
                goto ATTR_SETOVERLINE;
            case RTF_OLWAVE:
                eOverline = UNDERLINE_WAVE;
                goto ATTR_SETOVERLINE;
            case RTF_OLTHD:
                eOverline = UNDERLINE_BOLDDOTTED;
                goto ATTR_SETOVERLINE;
            case RTF_OLTHDASH:
                eOverline = UNDERLINE_BOLDDASH;
                goto ATTR_SETOVERLINE;
            case RTF_OLLDASH:
                eOverline = UNDERLINE_LONGDASH;
                goto ATTR_SETOVERLINE;
            case RTF_OLTHLDASH:
                eOverline = UNDERLINE_BOLDLONGDASH;
                goto ATTR_SETOVERLINE;
            case RTF_OLTHDASHD:
                eOverline = UNDERLINE_BOLDDASHDOT;
                goto ATTR_SETOVERLINE;
            case RTF_OLTHDASHDD:
                eOverline = UNDERLINE_BOLDDASHDOTDOT;
                goto ATTR_SETOVERLINE;
            case RTF_OLHWAVE:
                eOverline = UNDERLINE_BOLDWAVE;
                goto ATTR_SETOVERLINE;
            case RTF_OLOLDBWAVE:
                eOverline = UNDERLINE_DOUBLEWAVE;
                goto ATTR_SETOVERLINE;

            case RTF_OLW:
                eOverline = UNDERLINE_SINGLE;

                if( PLAINID->nWordlineMode )
                {
                    pSet->Put( SvxWordLineModeItem( sal_True, PLAINID->nWordlineMode ));
                }
                goto ATTR_SETOVERLINE;

ATTR_SETOVERLINE:
                if( PLAINID->nUnderline )
                {
                    pSet->Put( SvxOverlineItem( eOverline, PLAINID->nOverline ));
                }
                break;

            case RTF_OLC:
                if( PLAINID->nOverline )
                {
                    SvxOverlineItem aOL( UNDERLINE_SINGLE, PLAINID->nOverline );
                    const SfxPoolItem* pItem;
                    if( SFX_ITEM_SET == pSet->GetItemState(
                        PLAINID->nOverline, sal_False, &pItem ) )
                    {
                        // is switched off ?
                        if( UNDERLINE_NONE ==
                            ((SvxOverlineItem*)pItem)->GetLineStyle() )
                            break;
                        aOL = *(SvxOverlineItem*)pItem;
                    }
                    else
                        aOL = (const SvxOverlineItem&)pSet->Get( PLAINID->nUnderline, sal_False );

                    if( UNDERLINE_NONE == aOL.GetLineStyle() )
                        aOL.SetLineStyle( UNDERLINE_SINGLE );
                    aOL.SetColor( GetColor( sal_uInt16(nTokenValue) ));
                    pSet->Put( aOL );
                }
                break;

            case RTF_UP:
            case RTF_SUPER:
                if( PLAINID->nEscapement )
                {
                    const sal_uInt16 nEsc = PLAINID->nEscapement;
                    if( -1 == nTokenValue || RTF_SUPER == nToken )
                        nTokenValue = 6;
                    if( IsCalcValue() )
                        CalcValue();
                    const SvxEscapementItem& rOld = GetEscapement( *pSet, nEsc, sal_False );
                    short nEs;
                    sal_uInt8 nProp;
                    if( DFLT_ESC_AUTO_SUB == rOld.GetEsc() )
                    {
                        nEs = DFLT_ESC_AUTO_SUPER;
                        nProp = rOld.GetProp();
                    }
                    else
                    {
                        nEs = (short)nTokenValue;
                        nProp = (nToken == RTF_SUPER) ? DFLT_ESC_PROP : 100;
                    }
                    pSet->Put( SvxEscapementItem( nEs, nProp, nEsc ));
                }
                break;

            case RTF_CF:
                if( PLAINID->nColor )
                {
                    pSet->Put( SvxColorItem( GetColor( sal_uInt16(nTokenValue) ),
                                PLAINID->nColor ));
                }
                break;
            //#i12501# While cb is clearly documented in the rtf spec, word
            //doesn't accept it at all
#if 0
            case RTF_CB:
                if( PLAINID->nBgColor )
                {
                    pSet->Put( SvxBrushItem( GetColor( sal_uInt16(nTokenValue) ),
                                PLAINID->nBgColor ));
                }
                break;
#endif

            case RTF_LANG:
                if( PLAINID->nLanguage )
                {
                    pSet->Put( SvxLanguageItem( (LanguageType)nTokenValue,
                                PLAINID->nLanguage ));
                }
                break;

            case RTF_LANGFE:
                if( PLAINID->nCJKLanguage )
                {
                    pSet->Put( SvxLanguageItem( (LanguageType)nTokenValue,
                                                PLAINID->nCJKLanguage ));
                }
                break;
            case RTF_ALANG:
                {
                    SvxLanguageItem aTmpItem( (LanguageType)nTokenValue,
                                    SID_ATTR_CHAR_LANGUAGE );
                    SetScriptAttr( eCharType, *pSet, aTmpItem );
                }
                break;

            case RTF_RTLCH:
                bIsLeftToRightDef = sal_False;
                break;
            case RTF_LTRCH:
                bIsLeftToRightDef = sal_True;
                break;
            case RTF_RTLPAR:
                if (PARDID->nDirection)
                {
                    pSet->Put(SvxFrameDirectionItem(FRMDIR_HORI_RIGHT_TOP,
                        PARDID->nDirection));
                }
                break;
            case RTF_LTRPAR:
                if (PARDID->nDirection)
                {
                    pSet->Put(SvxFrameDirectionItem(FRMDIR_HORI_LEFT_TOP,
                        PARDID->nDirection));
                }
                break;
            case RTF_LOCH:      eCharType = LOW_CHARTYPE;           break;
            case RTF_HICH:      eCharType = HIGH_CHARTYPE;          break;
            case RTF_DBCH:      eCharType = DOUBLEBYTE_CHARTYPE;    break;


            case RTF_ACCNONE:
                eEmphasis = EMPHASISMARK_NONE;
                goto ATTR_SETEMPHASIS;
            case RTF_ACCDOT:
                eEmphasis = EMPHASISMARK_DOTS_ABOVE;
                goto ATTR_SETEMPHASIS;

            case RTF_ACCCOMMA:
                eEmphasis = EMPHASISMARK_SIDE_DOTS;
ATTR_SETEMPHASIS:
                if( PLAINID->nEmphasis )
                {
                    pSet->Put( SvxEmphasisMarkItem( eEmphasis,
                                                       PLAINID->nEmphasis ));
                }
                break;

            case RTF_TWOINONE:
                if( PLAINID->nTwoLines )
                {
                    sal_Unicode cStt, cEnd;
                    switch ( nTokenValue )
                    {
                    case 1: cStt = '(', cEnd = ')'; break;
                    case 2: cStt = '[', cEnd = ']'; break;
                    case 3: cStt = '<', cEnd = '>'; break;
                    case 4: cStt = '{', cEnd = '}'; break;
                    default: cStt = 0, cEnd = 0; break;
                    }

                    pSet->Put( SvxTwoLinesItem( sal_True, cStt, cEnd,
                                                       PLAINID->nTwoLines ));
                }
                break;

            case RTF_CHARSCALEX :
                if (PLAINID->nCharScaleX)
                {
                    //i21372
                    if (nTokenValue < 1 || nTokenValue > 600)
                        nTokenValue = 100;
                    pSet->Put( SvxCharScaleWidthItem( sal_uInt16(nTokenValue),
                                                       PLAINID->nCharScaleX ));
                }
                break;

            case RTF_HORZVERT:
                if( PLAINID->nHorzVert )
                {
                    // RTF knows only 90deg
                    pSet->Put( SvxCharRotateItem( 900, 1 == nTokenValue,
                                                       PLAINID->nHorzVert ));
                }
                break;

            case RTF_EMBO:
                if (PLAINID->nRelief)
                {
                    pSet->Put(SvxCharReliefItem(RELIEF_EMBOSSED,
                        PLAINID->nRelief));
                }
                break;
            case RTF_IMPR:
                if (PLAINID->nRelief)
                {
                    pSet->Put(SvxCharReliefItem(RELIEF_ENGRAVED,
                        PLAINID->nRelief));
                }
                break;
            case RTF_V:
                if (PLAINID->nHidden)
                {
                    pSet->Put(SvxCharHiddenItem(nTokenValue != 0,
                        PLAINID->nHidden));
                }
                break;
            case RTF_CHBGFDIAG:
            case RTF_CHBGDKVERT:
            case RTF_CHBGDKHORIZ:
            case RTF_CHBGVERT:
            case RTF_CHBGHORIZ:
            case RTF_CHBGDKFDIAG:
            case RTF_CHBGDCROSS:
            case RTF_CHBGCROSS:
            case RTF_CHBGBDIAG:
            case RTF_CHBGDKDCROSS:
            case RTF_CHBGDKCROSS:
            case RTF_CHBGDKBDIAG:
            case RTF_CHCBPAT:
            case RTF_CHCFPAT:
            case RTF_CHSHDNG:
                if( PLAINID->nBgColor )
                    ReadBackgroundAttr( nToken, *pSet );
                break;

            case BRACELEFT:
                {
                    // tests on Swg internal tokens
                    bool bHandled = false;
                    short nSkip = 0;
                    if( RTF_IGNOREFLAG != GetNextToken())
                        nSkip = -1;
                    else if( (nToken = GetNextToken() ) & RTF_SWGDEFS )
                    {
                        bHandled = true;
                        switch( nToken )
                        {
                        case RTF_PGDSCNO:
                        case RTF_PGBRK:
                        case RTF_SOUTLVL:
                            UnknownAttrToken( nToken, pSet );
                            // overwrite the closing parenthesis
                            break;

                        case RTF_SWG_ESCPROP:
                            {
                                // Store percentage change!
                                sal_uInt8 nProp = sal_uInt8( nTokenValue / 100 );
                                short nEsc = 0;
                                if( 1 == ( nTokenValue % 100 ))
                                    // Recognize own auto-flags!
                                    nEsc = DFLT_ESC_AUTO_SUPER;

                                if( PLAINID->nEscapement )
                                    pSet->Put( SvxEscapementItem( nEsc, nProp,
                                                       PLAINID->nEscapement ));
                            }
                            break;

                        case RTF_HYPHEN:
                            {
                                SvxHyphenZoneItem aHypenZone(
                                            (nTokenValue & 1) ? sal_True : sal_False,
                                                PARDID->nHyphenzone );
                                aHypenZone.SetPageEnd(
                                            (nTokenValue & 2) ? sal_True : sal_False );

                                if( PARDID->nHyphenzone &&
                                    RTF_HYPHLEAD == GetNextToken() &&
                                    RTF_HYPHTRAIL == GetNextToken() &&
                                    RTF_HYPHMAX == GetNextToken() )
                                {
                                    aHypenZone.GetMinLead() =
                                        sal_uInt8(GetStackPtr( -2 )->nTokenValue);
                                    aHypenZone.GetMinTrail() =
                                            sal_uInt8(GetStackPtr( -1 )->nTokenValue);
                                    aHypenZone.GetMaxHyphens() =
                                            sal_uInt8(nTokenValue);

                                    pSet->Put( aHypenZone );
                                }
                                else
                                    SkipGroup();  // at the end of the group
                            }
                            break;

                        case RTF_SHADOW:
                            {
                                int bSkip = sal_True;
                                do {    // middle check loop
                                    SvxShadowLocation eSL = SvxShadowLocation( nTokenValue );
                                    if( RTF_SHDW_DIST != GetNextToken() )
                                        break;
                                    sal_uInt16 nDist = sal_uInt16( nTokenValue );

                                    if( RTF_SHDW_STYLE != GetNextToken() )
                                        break;

                                    if( RTF_SHDW_COL != GetNextToken() )
                                        break;
                                    sal_uInt16 nCol = sal_uInt16( nTokenValue );

                                    if( RTF_SHDW_FCOL != GetNextToken() )
                                        break;

                                    Color aColor = GetColor( nCol );

                                    if( PARDID->nShadow )
                                        pSet->Put( SvxShadowItem( PARDID->nShadow,
                                                                  &aColor, nDist, eSL ) );

                                    bSkip = sal_False;
                                } while( sal_False );

                                if( bSkip )
                                    SkipGroup();  // at the end of the group
                            }
                            break;

                        default:
                            bHandled = false;
                            if( (nToken & ~(0xff | RTF_SWGDEFS)) == RTF_TABSTOPDEF )
                            {
                                nToken = SkipToken( -2 );
                                ReadTabAttr( nToken, *pSet );

                                /*
                                cmc: #i76140, he who consumed the { must consume the }
                                We rewound to a state of { being the current
                                token so it is our responsibility to consume the }
                                token if we consumed the {. We will not have consumed
                                the { if it belonged to our caller, i.e. if the { we
                                are handling is the "firsttoken" passed to us then
                                the *caller* must consume it, not us. Otherwise *we*
                                should consume it.
                                */
                                if (nToken == BRACELEFT && !bFirstToken)
                                {
                                    nToken = GetNextToken();
                                    DBG_ASSERT( nToken == BRACERIGHT,
                                        "} did not follow { as expected\n");
                                }
                            }
                            else if( (nToken & ~(0xff| RTF_SWGDEFS)) == RTF_BRDRDEF)
                            {
                                nToken = SkipToken( -2 );
                                ReadBorderAttr( nToken, *pSet );
                            }
                            else        // so no more attribute
                                nSkip = -2;
                            break;
                        }

#if 1
                        /*
                        cmc: #i4727# / #i12713# Who owns this closing bracket?
                        If we read the opening one, we must read this one, if
                        other is counting the brackets so as to push/pop off
                        the correct environment then we will have pushed a new
                        environment for the start { of this, but will not see
                        the } and so is out of sync for the rest of the
                        document.
                        */
                        if (bHandled && !bFirstToken)
                            GetNextToken();
#endif
                    }
                    else
                        nSkip = -2;

                    if( nSkip )             // all completely unknown
                    {
                        if (!bFirstToken)
                            --nSkip;    // BRACELEFT: is the next token
                        SkipToken( nSkip );
                        bWeiter = sal_False;
                    }
                }
                break;
            default:
                if( (nToken & ~0xff ) == RTF_TABSTOPDEF )
                    ReadTabAttr( nToken, *pSet );
                else if( (nToken & ~0xff ) == RTF_BRDRDEF )
                    ReadBorderAttr( nToken, *pSet );
                else if( (nToken & ~0xff ) == RTF_SHADINGDEF )
                    ReadBackgroundAttr( nToken, *pSet );
                else
                {
                    // unknown token, so token "returned in Parser"
                    if( !bFirstToken )
                        SkipToken( -1 );
                    bWeiter = sal_False;
                }
            }
        }
        if( bWeiter )
        {
            nToken = GetNextToken();
        }
        bFirstToken = sal_False;
    }
}

void SvxRTFParser::ReadTabAttr( int nToken, SfxItemSet& rSet )
{
    bool bMethodOwnsToken = false; // #i52542# patch from cmc.
// then read all the TabStops
    SvxTabStop aTabStop;
    SvxTabStopItem aAttr( 0, 0, SVX_TAB_ADJUST_DEFAULT, PARDID->nTabStop );
    int bWeiter = sal_True;
    do {
        switch( nToken )
        {
        case RTF_TB:        // BarTab ???
        case RTF_TX:
            {
                if( IsCalcValue() )
                    CalcValue();
                aTabStop.GetTabPos() = nTokenValue;
                aAttr.Insert( aTabStop );
                aTabStop = SvxTabStop();    // all values default
            }
            break;

        case RTF_TQL:
            aTabStop.GetAdjustment() = SVX_TAB_ADJUST_LEFT;
            break;
        case RTF_TQR:
            aTabStop.GetAdjustment() = SVX_TAB_ADJUST_RIGHT;
            break;
        case RTF_TQC:
            aTabStop.GetAdjustment() = SVX_TAB_ADJUST_CENTER;
            break;
        case RTF_TQDEC:
            aTabStop.GetAdjustment() = SVX_TAB_ADJUST_DECIMAL;
            break;

        case RTF_TLDOT:     aTabStop.GetFill() = '.';   break;
        case RTF_TLHYPH:    aTabStop.GetFill() = ' ';   break;
        case RTF_TLUL:      aTabStop.GetFill() = '_';   break;
        case RTF_TLTH:      aTabStop.GetFill() = '-';   break;
        case RTF_TLEQ:      aTabStop.GetFill() = '=';   break;

        case BRACELEFT:
            {
                // Swg - control BRACELEFT RTF_IGNOREFLAG RTF_TLSWG BRACERIGHT
                short nSkip = 0;
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nSkip = -1;
                else if( RTF_TLSWG != ( nToken = GetNextToken() ))
                    nSkip = -2;
                else
                {
                    aTabStop.GetDecimal() = sal_uInt8(nTokenValue & 0xff);
                    aTabStop.GetFill() = sal_uInt8((nTokenValue >> 8) & 0xff);
                    // overwrite the closing parenthesis
                    if (bMethodOwnsToken)
                        GetNextToken();
                }
                if( nSkip )
                {
                    SkipToken( nSkip );     // Ignore back again
                    bWeiter = sal_False;
                }
            }
            break;

        default:
            bWeiter = sal_False;
        }
        if( bWeiter )
        {
            nToken = GetNextToken();
            bMethodOwnsToken = true;
        }
    } while( bWeiter );

    // Fill with defaults is still missing!
    rSet.Put( aAttr );
    SkipToken( -1 );
}

static void SetBorderLine( int nBorderTyp, SvxBoxItem& rItem,
                            const SvxBorderLine& rBorder )
{
    switch( nBorderTyp )
    {
    case RTF_BOX:           // run through all levels

    case RTF_BRDRT:
        rItem.SetLine( &rBorder, BOX_LINE_TOP );
        if( RTF_BOX != nBorderTyp )
            return;

    case RTF_BRDRB:
        rItem.SetLine( &rBorder, BOX_LINE_BOTTOM );
        if( RTF_BOX != nBorderTyp )
            return;

    case RTF_BRDRL:
        rItem.SetLine( &rBorder, BOX_LINE_LEFT );
        if( RTF_BOX != nBorderTyp )
            return;

    case RTF_BRDRR:
        rItem.SetLine( &rBorder, BOX_LINE_RIGHT );
        if( RTF_BOX != nBorderTyp )
            return;
    }
}

void SvxRTFParser::ReadBorderAttr( int nToken, SfxItemSet& rSet,
                                    int bTableDef )
{
    // then read the border attribute
    SvxBoxItem aAttr( PARDID->nBox );
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( PARDID->nBox, sal_False, &pItem ) )
        aAttr = *(SvxBoxItem*)pItem;

    SvxBorderLine aBrd( 0, DEF_LINE_WIDTH_0, 0, 0 );    // simple lines
    int bWeiter = sal_True, nBorderTyp = 0;

    do {
        switch( nToken )
        {
        case RTF_BOX:
        case RTF_BRDRT:
        case RTF_BRDRB:
        case RTF_BRDRL:
        case RTF_BRDRR:
            nBorderTyp = nToken;
            goto SETBORDER;

        case RTF_CLBRDRT:
            if( !bTableDef )
                break;
            nBorderTyp = RTF_BRDRT;
            goto SETBORDER;
        case RTF_CLBRDRB:
            if( !bTableDef )
                break;
            nBorderTyp = RTF_BRDRB;
            goto SETBORDER;
        case RTF_CLBRDRL:
            if( !bTableDef )
                break;
            nBorderTyp = RTF_BRDRL;
            goto SETBORDER;
        case RTF_CLBRDRR:
            if( !bTableDef )
                break;
            nBorderTyp = RTF_BRDRR;
            goto SETBORDER;

SETBORDER:
            {
                // set to defaults
                aBrd.SetOutWidth( DEF_LINE_WIDTH_0 );
                aBrd.SetInWidth( 0 );
                aBrd.SetDistance( 0 );
                aBrd.SetColor( Color( COL_BLACK ) );
            }
            break;


// yet to be evaluated
        case RTF_BRSP:
            {
                switch( nBorderTyp )
                {
                case RTF_BRDRB:
                    aAttr.SetDistance( (sal_uInt16)nTokenValue, BOX_LINE_BOTTOM );
                    break;

                case RTF_BRDRT:
                    aAttr.SetDistance( (sal_uInt16)nTokenValue, BOX_LINE_TOP );
                    break;

                case RTF_BRDRL:
                    aAttr.SetDistance( (sal_uInt16)nTokenValue, BOX_LINE_LEFT );
                    break;

                case RTF_BRDRR:
                    aAttr.SetDistance( (sal_uInt16)nTokenValue, BOX_LINE_RIGHT );
                    break;

                case RTF_BOX:
                    aAttr.SetDistance( (sal_uInt16)nTokenValue );
                    break;
                }
            }
            break;

case RTF_BRDRBTW:
case RTF_BRDRBAR:           break;


        case RTF_BRDRCF:
            {
                aBrd.SetColor( GetColor( sal_uInt16(nTokenValue) ) );
                SetBorderLine( nBorderTyp, aAttr, aBrd );
            }
            break;

        case RTF_BRDRTH:
            aBrd.SetOutWidth( DEF_LINE_WIDTH_1 );
            aBrd.SetInWidth( 0 );
            aBrd.SetDistance( 0 );
            goto SETBORDERLINE;

        case RTF_BRDRDB:
            aBrd.SetOutWidth( DEF_DOUBLE_LINE0_OUT );
            aBrd.SetInWidth( DEF_DOUBLE_LINE0_IN );
            aBrd.SetDistance( DEF_DOUBLE_LINE0_DIST );
            goto SETBORDERLINE;

        case RTF_BRDRSH:
            // shaded box
            {
                rSet.Put( SvxShadowItem( PARDID->nShadow, (Color*) 0, 60 /*3pt*/,
                                        SVX_SHADOW_BOTTOMRIGHT ) );
            }
            break;

        case RTF_BRDRW:
            if( -1 != nTokenValue )
            {
                // It should be a "thick" line?
                if( DEF_LINE_WIDTH_0 != aBrd.GetOutWidth() )
                    nTokenValue *= 2;

                // a double line?
                if( aBrd.GetInWidth() )
                {
                    // WinWord - adapt values to StarOffice
                    if( nTokenValue < DEF_LINE_WIDTH_1 - (DEF_LINE_WIDTH_1/10))
                    {
                        aBrd.SetOutWidth( DEF_DOUBLE_LINE0_OUT );
                        aBrd.SetInWidth( DEF_DOUBLE_LINE0_IN );
                        aBrd.SetDistance( DEF_DOUBLE_LINE0_DIST );
                    }
                    else
                    if( nTokenValue < DEF_LINE_WIDTH_2 - (DEF_LINE_WIDTH_2/10))
                    {
                        aBrd.SetOutWidth( DEF_DOUBLE_LINE1_OUT );
                        aBrd.SetInWidth( DEF_DOUBLE_LINE1_IN );
                        aBrd.SetDistance( DEF_DOUBLE_LINE1_DIST );
                    }
                    else
                    {
                        aBrd.SetOutWidth( DEF_DOUBLE_LINE2_OUT );
                        aBrd.SetInWidth( DEF_DOUBLE_LINE2_IN );
                        aBrd.SetDistance( DEF_DOUBLE_LINE2_DIST );
                    }
                }
                else
                {
                    // WinWord - adapt values to StarOffice
                    if( nTokenValue < DEF_LINE_WIDTH_1 - (DEF_LINE_WIDTH_1/10))
                        aBrd.SetOutWidth( DEF_LINE_WIDTH_0 );
                    else
                    if( nTokenValue < DEF_LINE_WIDTH_2 - (DEF_LINE_WIDTH_2/10))
                        aBrd.SetOutWidth( DEF_LINE_WIDTH_1 );
                    else
                    if( nTokenValue < DEF_LINE_WIDTH_3 - (DEF_LINE_WIDTH_3/10))
                        aBrd.SetOutWidth( DEF_LINE_WIDTH_2 );
                    else
                    if( nTokenValue < DEF_LINE_WIDTH_4 )
                        aBrd.SetOutWidth( DEF_LINE_WIDTH_3 );
                    else
                        aBrd.SetOutWidth( DEF_LINE_WIDTH_4 );
                }
            }
            goto SETBORDERLINE;

        case RTF_BRDRDOT:
            aBrd.SetStyle( DOTTED );
            break;
        case RTF_BRDRDASH:
            aBrd.SetStyle( DASHED );
            break;
        case RTF_BRDRS:
        case RTF_BRDRHAIR:
            aBrd.SetStyle( SOLID );
SETBORDERLINE:
            SetBorderLine( nBorderTyp, aAttr, aBrd );
            break;

        case BRACELEFT:
            {
                short nSkip = 0;
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nSkip = -1;
                else
                {
                    int bSwgControl = sal_True, bFirstToken = sal_True;
                    nToken = GetNextToken();
                    do {
                        switch( nToken )
                        {
                        case RTF_BRDBOX:
                            aAttr.SetDistance( sal_uInt16(nTokenValue) );
                            break;

                        case RTF_BRDRT:
                        case RTF_BRDRB:
                        case RTF_BRDRR:
                        case RTF_BRDRL:
                            nBorderTyp = nToken;
                            bFirstToken = sal_False;
                            if( RTF_BRDLINE_COL != GetNextToken() )
                            {
                                bSwgControl = sal_False;
                                break;
                            }
                            aBrd.SetColor( GetColor( sal_uInt16(nTokenValue) ));

                            if( RTF_BRDLINE_IN != GetNextToken() )
                            {
                                bSwgControl = sal_False;
                                break;
                            }
                            aBrd.SetInWidth( sal_uInt16(nTokenValue));

                            if( RTF_BRDLINE_OUT != GetNextToken() )
                            {
                                bSwgControl = sal_False;
                                break;
                            }
                            aBrd.SetOutWidth( sal_uInt16(nTokenValue));

                            if( RTF_BRDLINE_DIST != GetNextToken() )
                            {
                                bSwgControl = sal_False;
                                break;
                            }
                            aBrd.SetDistance( sal_uInt16(nTokenValue));
                            SetBorderLine( nBorderTyp, aAttr, aBrd );
                            break;

                        default:
                            bSwgControl = sal_False;
                            break;
                        }

                        if( bSwgControl )
                        {
                            nToken = GetNextToken();
                            bFirstToken = sal_False;
                        }
                    } while( bSwgControl );

                    // End of the Swg-Group
                    // -> still read the closing parenthesis
                    if( BRACERIGHT == nToken )
                        ;
                    else if( !bFirstToken )
                    {
                        // It is a parser error, jump to the end of the group
                        SkipGroup();
                        // Disregard closing BRACERIGHT
                        GetNextToken();
                    }
                    else
                        nSkip = -2;
                }

                if( nSkip )
                {
                    SkipToken( nSkip );     // Ignore back again
                    bWeiter = sal_False;
                }
            }
            break;

        default:
            bWeiter = (nToken & ~(0xff| RTF_SWGDEFS)) == RTF_BRDRDEF;
        }
        if( bWeiter )
            nToken = GetNextToken();
    } while( bWeiter );
    rSet.Put( aAttr );
    SkipToken( -1 );
}

inline sal_uInt32 CalcShading( sal_uInt32 nColor, sal_uInt32 nFillColor, sal_uInt8 nShading )
{
    nColor = (nColor * nShading) / 100;
    nFillColor = (nFillColor * ( 100 - nShading )) / 100;
    return nColor + nFillColor;
}

void SvxRTFParser::ReadBackgroundAttr( int nToken, SfxItemSet& rSet,
                                        int bTableDef )
{
    // then read the border attribute
    int bWeiter = sal_True;
    sal_uInt16 nColor = USHRT_MAX, nFillColor = USHRT_MAX;
    sal_uInt8 nFillValue = 0;

    sal_uInt16 nWh = ( nToken & ~0xff ) == RTF_CHRFMT
                    ? PLAINID->nBgColor
                    : PARDID->nBrush;

    do {
        switch( nToken )
        {
        case RTF_CLCBPAT:
        case RTF_CHCBPAT:
        case RTF_CBPAT:
            nFillColor = sal_uInt16( nTokenValue );
            break;

        case RTF_CLCFPAT:
        case RTF_CHCFPAT:
        case RTF_CFPAT:
            nColor = sal_uInt16( nTokenValue );
            break;

        case RTF_CLSHDNG:
        case RTF_CHSHDNG:
        case RTF_SHADING:
            nFillValue = (sal_uInt8)( nTokenValue / 100 );
            break;

        case RTF_CLBGDKHOR:
        case RTF_CHBGDKHORIZ:
        case RTF_BGDKHORIZ:
        case RTF_CLBGDKVERT:
        case RTF_CHBGDKVERT:
        case RTF_BGDKVERT:
        case RTF_CLBGDKBDIAG:
        case RTF_CHBGDKBDIAG:
        case RTF_BGDKBDIAG:
        case RTF_CLBGDKFDIAG:
        case RTF_CHBGDKFDIAG:
        case RTF_BGDKFDIAG:
        case RTF_CLBGDKCROSS:
        case RTF_CHBGDKCROSS:
        case RTF_BGDKCROSS:
        case RTF_CLBGDKDCROSS:
        case RTF_CHBGDKDCROSS:
        case RTF_BGDKDCROSS:
            // dark -> 60%
            nFillValue = 60;
            break;

        case RTF_CLBGHORIZ:
        case RTF_CHBGHORIZ:
        case RTF_BGHORIZ:
        case RTF_CLBGVERT:
        case RTF_CHBGVERT:
        case RTF_BGVERT:
        case RTF_CLBGBDIAG:
        case RTF_CHBGBDIAG:
        case RTF_BGBDIAG:
        case RTF_CLBGFDIAG:
        case RTF_CHBGFDIAG:
        case RTF_BGFDIAG:
        case RTF_CLBGCROSS:
        case RTF_CHBGCROSS:
        case RTF_BGCROSS:
        case RTF_CLBGDCROSS:
        case RTF_CHBGDCROSS:
        case RTF_BGDCROSS:
            // light -> 20%
            nFillValue = 20;
            break;

        default:
            if( bTableDef )
                bWeiter = (nToken & ~(0xff | RTF_TABLEDEF) ) == RTF_SHADINGDEF;
            else
                bWeiter = (nToken & ~0xff) == RTF_SHADINGDEF;
        }
        if( bWeiter )
            nToken = GetNextToken();
    } while( bWeiter );

    Color aCol( COL_WHITE ), aFCol;
    if( !nFillValue )
    {
        // there was only one of two colors specified or no BrushTyp
        if( USHRT_MAX != nFillColor )
        {
            nFillValue = 100;
            aCol = GetColor( nFillColor );
        }
        else if( USHRT_MAX != nColor )
            aFCol = GetColor( nColor );
    }
    else
    {
        if( USHRT_MAX != nColor )
            aCol = GetColor( nColor );
        else
            aCol = Color( COL_BLACK );

        if( USHRT_MAX != nFillColor )
            aFCol = GetColor( nFillColor );
        else
            aFCol = Color( COL_WHITE );
    }

    Color aColor;
    if( 0 == nFillValue || 100 == nFillValue )
        aColor = aCol;
    else
        aColor = Color(
            (sal_uInt8)CalcShading( aCol.GetRed(), aFCol.GetRed(), nFillValue ),
            (sal_uInt8)CalcShading( aCol.GetGreen(), aFCol.GetGreen(), nFillValue ),
            (sal_uInt8)CalcShading( aCol.GetBlue(), aFCol.GetBlue(), nFillValue ) );

    rSet.Put( SvxBrushItem( aColor, nWh ) );
    SkipToken( -1 );
}


// pard / plain abarbeiten
void SvxRTFParser::RTFPardPlain( int bPard, SfxItemSet** ppSet )
{
    if( !bNewGroup && !aAttrStack.empty() ) // not at the beginning of a new group
    {
        SvxRTFItemStackType* pAkt = aAttrStack.back();

        int nLastToken = GetStackPtr(-1)->nTokenId;
        int bNewStkEntry = sal_True;
        if( RTF_PARD != nLastToken &&
            RTF_PLAIN != nLastToken &&
            BRACELEFT != nLastToken )
        {
            if( pAkt->aAttrSet.Count() || pAkt->pChildList || pAkt->nStyleNo )
            {
                // open a new group
                SvxRTFItemStackType* pNew = new SvxRTFItemStackType( *pAkt, *pInsPos, sal_True );
                pNew->SetRTFDefaults( GetRTFDefaults() );

                // Set all until here valid attributes
                AttrGroupEnd();
                pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();  // can be changed after AttrGroupEnd!
                pNew->aAttrSet.SetParent( pAkt ? &pAkt->aAttrSet : 0 );
                aAttrStack.push_back( pNew );
                pAkt = pNew;
            }
            else
            {
                // continue to use this entry as new
                pAkt->SetStartPos( *pInsPos );
                bNewStkEntry = sal_False;
            }
        }

        // now reset all to default
        if( bNewStkEntry &&
            ( pAkt->aAttrSet.GetParent() || pAkt->aAttrSet.Count() ))
        {
            const SfxPoolItem *pItem, *pDef;
            const sal_uInt16* pPtr;
            sal_uInt16 nCnt;
            const SfxItemSet* pDfltSet = &GetRTFDefaults();
            if( bPard )
            {
                pAkt->nStyleNo = 0;
                pPtr = aPardMap.GetData();
                nCnt = aPardMap.Count();
            }
            else
            {
                pPtr = aPlainMap.GetData();
                nCnt = aPlainMap.Count();
            }

            for( sal_uInt16 n = 0; n < nCnt; ++n, ++pPtr )
            {
                // Item set and different -> Set the Default Pool
                if( !*pPtr )
                    ;
                else if( SFX_WHICH_MAX < *pPtr )
                    pAkt->aAttrSet.ClearItem( *pPtr );
                else if( IsChkStyleAttr() )
                    pAkt->aAttrSet.Put( pDfltSet->Get( *pPtr ) );
                else if( !pAkt->aAttrSet.GetParent() )
                {
                    if( SFX_ITEM_SET ==
                        pDfltSet->GetItemState( *pPtr, sal_False, &pDef ))
                        pAkt->aAttrSet.Put( *pDef );
                    else
                        pAkt->aAttrSet.ClearItem( *pPtr );
                }
                else if( SFX_ITEM_SET == pAkt->aAttrSet.GetParent()->
                            GetItemState( *pPtr, sal_True, &pItem ) &&
                        *( pDef = &pDfltSet->Get( *pPtr )) != *pItem )
                    pAkt->aAttrSet.Put( *pDef );
                else
                {
                    if( SFX_ITEM_SET ==
                        pDfltSet->GetItemState( *pPtr, sal_False, &pDef ))
                        pAkt->aAttrSet.Put( *pDef );
                    else
                        pAkt->aAttrSet.ClearItem( *pPtr );
                }
            }
        }
        else if( bPard )
            pAkt->nStyleNo = 0;     // reset Style number

        *ppSet = &pAkt->aAttrSet;

        if (!bPard)
        {
            //Once we have a default font, then any text without a font specifier is
            //in the default font, and thus has the default font charset, otherwise
            //we can fall back to the ansicpg set codeset
            if (nDfltFont != -1)
            {
                const Font& rSVFont = GetFont(sal_uInt16(nDfltFont));
                SetEncoding(rSVFont.GetCharSet());
            }
            else
                SetEncoding(GetCodeSet());
        }
    }
}

void SvxRTFParser::SetDefault( int nToken, int nValue )
{
    if( !bNewDoc )
        return;

    SfxItemSet aTmp( *pAttrPool, aWhichMap.GetData() );
    sal_Bool bOldFlag = bIsLeftToRightDef;
    bIsLeftToRightDef = sal_True;
    switch( nToken )
    {
    case RTF_ADEFF: bIsLeftToRightDef = sal_False;  // no break!
    case RTF_DEFF:
        {
            if( -1 == nValue )
                nValue = 0;
            const Font& rSVFont = GetFont( sal_uInt16(nValue) );
            SvxFontItem aTmpItem(
                                rSVFont.GetFamily(), rSVFont.GetName(),
                                rSVFont.GetStyleName(), rSVFont.GetPitch(),
                                rSVFont.GetCharSet(), SID_ATTR_CHAR_FONT );
            SetScriptAttr( NOTDEF_CHARTYPE, aTmp, aTmpItem );
        }
        break;

    case RTF_ADEFLANG:  bIsLeftToRightDef = sal_False;  // no break!
    case RTF_DEFLANG:
        // store default Language
        if( -1 != nValue )
        {
            SvxLanguageItem aTmpItem( (const LanguageType)nValue,
                                        SID_ATTR_CHAR_LANGUAGE );
            SetScriptAttr( NOTDEF_CHARTYPE, aTmp, aTmpItem );
        }
        break;

    case RTF_DEFTAB:
        if( PARDID->nTabStop )
        {
            // RTF defines 720 twips as default
            bIsSetDfltTab = sal_True;
            if( -1 == nValue || !nValue )
                nValue = 720;

            // who would like to have no twips  ...
            if( IsCalcValue() )
            {
                nTokenValue = nValue;
                CalcValue();
                nValue = nTokenValue;
            }

            // Calculate the ratio of default TabWidth / Tabs and
            // calculate the corresponding new number.
            // ?? how did one come up with 13 ??
            sal_uInt16 nAnzTabs = (SVX_TAB_DEFDIST * 13 ) / sal_uInt16(nValue);
            /*
             cmc, make sure we have at least one, or all hell breaks loose in
             everybodies exporters, #i8247#
            */
            if (nAnzTabs < 1)
                nAnzTabs = 1;

            // we want Defaulttabs
            SvxTabStopItem aNewTab( nAnzTabs, sal_uInt16(nValue),
                                SVX_TAB_ADJUST_DEFAULT, PARDID->nTabStop );
            while( nAnzTabs )
                ((SvxTabStop&)aNewTab[ --nAnzTabs ]).GetAdjustment() = SVX_TAB_ADJUST_DEFAULT;

            pAttrPool->SetPoolDefaultItem( aNewTab );
        }
        break;
    }
    bIsLeftToRightDef = bOldFlag;

    if( aTmp.Count() )
    {
        SfxItemIter aIter( aTmp );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( sal_True )
        {
            pAttrPool->SetPoolDefaultItem( *pItem );
            if( aIter.IsAtEnd() )
                break;
            pItem = aIter.NextItem();
        }
    }
}

// default: no conversion, leaving everything in twips.
void SvxRTFParser::CalcValue()
{
}

// for tokens that are not evaluated in ReadAttr
void SvxRTFParser::UnknownAttrToken( int, SfxItemSet* )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
