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


#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
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
#include <sal/log.hxx>
#include <vcl/font.hxx>

#include <editeng/svxrtf.hxx>
#include <editeng/editids.hrc>

#include <limits.h>

#define BRACELEFT   '{'
#define BRACERIGHT  '}'

using namespace ::com::sun::star;
using namespace editeng;

void SvxRTFParser::SetScriptAttr( RTF_CharTypeDef eType, SfxItemSet& rSet,
                                    SfxPoolItem& rItem )
{
    const sal_uInt16 *pNormal = nullptr, *pCJK = nullptr, *pCTL = nullptr;
    switch( rItem.Which() )
    {
    case SID_ATTR_CHAR_FONT:
        pNormal = &aPlainMap.nFont;
        pCJK = &aPlainMap.nCJKFont;
        pCTL = &aPlainMap.nCTLFont;
        break;

    case SID_ATTR_CHAR_FONTHEIGHT:
        pNormal = &aPlainMap.nFontHeight;
        pCJK = &aPlainMap.nCJKFontHeight;
        pCTL = &aPlainMap.nCTLFontHeight;
        break;

    case SID_ATTR_CHAR_POSTURE:
        pNormal = &aPlainMap.nPosture;
        pCJK = &aPlainMap.nCJKPosture;
        pCTL = &aPlainMap.nCTLPosture;
        break;

    case SID_ATTR_CHAR_WEIGHT:
        pNormal = &aPlainMap.nWeight;
        pCJK = &aPlainMap.nCJKWeight;
        pCTL = &aPlainMap.nCTLWeight;
        break;

    case SID_ATTR_CHAR_LANGUAGE:
        pNormal = &aPlainMap.nLanguage;
        pCJK = &aPlainMap.nCJKLanguage;
        pCTL = &aPlainMap.nCTLLanguage;
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
        if( bIsLeftToRightDef && pCJK )
        {
            rItem.SetWhich( *pCJK );
            rSet.Put( rItem );
        }
    }
    else if( !bIsLeftToRightDef )
    {
        if( pCTL )
        {
            rItem.SetWhich( *pCTL );
            rSet.Put( rItem );
        }
    }
    else
    {
        if( LOW_CHARTYPE == eType )
        {
            if( pNormal )
            {
                rItem.SetWhich( *pNormal );
                rSet.Put( rItem );
            }
        }
        else if( HIGH_CHARTYPE == eType )
        {
            if( pCTL )
            {
                rItem.SetWhich( *pCTL );
                rSet.Put( rItem );
            }
        }
        else
        {
            if( pCJK )
            {
                rItem.SetWhich( *pCJK );
                rSet.Put( rItem );
            }
            if( pCTL )
            {
                rItem.SetWhich( *pCTL );
                rSet.Put( rItem );
            }
            if( pNormal )
            {
                rItem.SetWhich( *pNormal );
                rSet.Put( rItem );
            }
        }
    }
}


void SvxRTFParser::ReadAttr( int nToken, SfxItemSet* pSet )
{
    DBG_ASSERT( pSet, "A SfxItemSet has to be provided as argument!" );
    bool bFirstToken = true;
    bool bContinue = true;
    FontLineStyle eUnderline;
    FontLineStyle eOverline;
    FontEmphasisMark eEmphasis;
    RTF_CharTypeDef eCharType = NOTDEF_CHARTYPE;
    SvxParaVertAlignItem::Align nFontAlign;

    bool bChkStkPos = !bNewGroup && !aAttrStack.empty();

    while( bContinue && IsParserWorking() )  // as long as known Attribute are recognized
    {
        switch( nToken )
        {
        case RTF_PARD:
            RTFPardPlain( true, &pSet );
            break;

        case RTF_PLAIN:
            RTFPardPlain( false, &pSet );
            break;

        default:
            do {        // middle checked loop
                if( !bChkStkPos )
                    break;

                SvxRTFItemStackType* pCurrent = aAttrStack.empty() ? nullptr : aAttrStack.back().get();
                if( !pCurrent || (pCurrent->pSttNd->GetIdx() == pInsPos->GetNodeIdx() &&
                    pCurrent->nSttCnt == pInsPos->GetCntIdx() ))
                    break;

                int nLastToken = GetStackPtr(-1)->nTokenId;
                if( RTF_PARD == nLastToken || RTF_PLAIN == nLastToken )
                    break;

                if (pCurrent->aAttrSet.Count() || pCurrent->m_pChildList ||
                    pCurrent->nStyleNo )
                {
                    // Open a new Group
                    std::unique_ptr<SvxRTFItemStackType> pNew(new SvxRTFItemStackType(
                                                *pCurrent, *pInsPos, true ));
                    pNew->SetRTFDefaults( GetRTFDefaults() );

                    // "Set" all valid attributes up until this point
                    AttrGroupEnd();
                    pCurrent = aAttrStack.empty() ? nullptr : aAttrStack.back().get();  // can be changed after AttrGroupEnd!
                    pNew->aAttrSet.SetParent( pCurrent ? &pCurrent->aAttrSet : nullptr );

                    aAttrStack.push_back( std::move(pNew) );
                    pCurrent = aAttrStack.back().get();
                }
                else
                    // continue to use this entry as a new one
                    pCurrent->SetStartPos( *pInsPos );

                pSet = &pCurrent->aAttrSet;
            } while( false );

            switch( nToken )
            {
            case RTF_INTBL:
            case RTF_PAGEBB:
            case RTF_SBYS:
            case RTF_CS:
            case RTF_LS:
            case RTF_ILVL:
                    UnknownAttrToken( nToken );
                    break;

            case RTF_S:
                if( bIsInReadStyleTab )
                {
                    if( !bFirstToken )
                        SkipToken();
                    bContinue = false;
                }
                else
                {
                    sal_uInt16 nStyleNo = -1 == nTokenValue ? 0 : sal_uInt16(nTokenValue);
                    // set StyleNo to the current style on the AttrStack
                    SvxRTFItemStackType* pCurrent = aAttrStack.empty() ? nullptr : aAttrStack.back().get();
                    if( !pCurrent )
                        break;

                    pCurrent->nStyleNo = nStyleNo;
                }
                break;

            case RTF_KEEP:
                if( aPardMap.nSplit )
                {
                    pSet->Put( SvxFormatSplitItem( false, aPardMap.nSplit ));
                }
                break;

            case RTF_KEEPN:
                if( aPardMap.nKeep )
                {
                    pSet->Put( SvxFormatKeepItem( true, aPardMap.nKeep ));
                }
                break;

            case RTF_LEVEL:
                if( aPardMap.nOutlineLvl )
                {
                    pSet->Put( SfxInt16Item( aPardMap.nOutlineLvl,
                                                static_cast<sal_uInt16>(nTokenValue) ));
                }
                break;

            case RTF_QL:
                if( aPardMap.nAdjust )
                {
                    pSet->Put( SvxAdjustItem( SvxAdjust::Left, aPardMap.nAdjust ));
                }
                break;
            case RTF_QR:
                if( aPardMap.nAdjust )
                {
                    pSet->Put( SvxAdjustItem( SvxAdjust::Right, aPardMap.nAdjust ));
                }
                break;
            case RTF_QJ:
                if( aPardMap.nAdjust )
                {
                    pSet->Put( SvxAdjustItem( SvxAdjust::Block, aPardMap.nAdjust ));
                }
                break;
            case RTF_QC:
                if( aPardMap.nAdjust )
                {
                    pSet->Put( SvxAdjustItem( SvxAdjust::Center, aPardMap.nAdjust ));
                }
                break;

            case RTF_FI:
                if( aPardMap.nLRSpace )
                {
                    SvxLRSpaceItem aLR(
                        static_cast<const SvxLRSpaceItem&>(pSet->Get(aPardMap.nLRSpace)));
                    sal_uInt16 nSz = 0;
                    if( -1 != nTokenValue )
                    {
                        if( IsCalcValue() )
                            CalcValue();
                        nSz = sal_uInt16(nTokenValue);
                    }
                    aLR.SetTextFirstLineOffset( nSz );
                    pSet->Put( aLR );
                }
                break;

            case RTF_LI:
            case RTF_LIN:
                if( aPardMap.nLRSpace )
                {
                    SvxLRSpaceItem aLR(
                        static_cast<const SvxLRSpaceItem&>(pSet->Get(aPardMap.nLRSpace)));
                    sal_uInt16 nSz = 0;
                    if( 0 < nTokenValue )
                    {
                        if( IsCalcValue() )
                            CalcValue();
                        nSz = sal_uInt16(nTokenValue);
                    }
                    aLR.SetTextLeft( nSz );
                    pSet->Put( aLR );
                }
                break;

            case RTF_RI:
            case RTF_RIN:
                if( aPardMap.nLRSpace )
                {
                    SvxLRSpaceItem aLR(
                        static_cast<const SvxLRSpaceItem&>(pSet->Get(aPardMap.nLRSpace)));
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
                if( aPardMap.nULSpace )
                {
                    SvxULSpaceItem aUL(
                        static_cast<const SvxULSpaceItem&>(pSet->Get(aPardMap.nULSpace)));
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
                if( aPardMap.nULSpace )
                {
                    SvxULSpaceItem aUL(
                        static_cast<const SvxULSpaceItem&>(pSet->Get(aPardMap.nULSpace)));
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
                if( aPardMap.nLinespacing && 1 == nTokenValue )
                {
                    // then switches to multi-line!
                    SvxLineSpacingItem aLSpace(
                            static_cast<const SvxLineSpacingItem&>(pSet->Get( aPardMap.nLinespacing,false)));

                    // how much do you get from the line height value?

                    // Proportional-Size:
                    // Ie, the ratio is (n / 240) twips

                    nTokenValue = 240;
                    if( IsCalcValue() )
                        CalcValue();

                    nTokenValue = short( 100 * aLSpace.GetLineHeight() / nTokenValue );

                    aLSpace.SetPropLineSpace( static_cast<sal_uInt16>(nTokenValue) );
                    aLSpace.SetLineSpaceRule( SvxLineSpaceRule::Auto );

                    pSet->Put( aLSpace );
                }
                break;

            case RTF_SL:
                if( aPardMap.nLinespacing )
                {
                    // Calculate the ratio between the default font and the
                    // specified size. The distance consists of the line height
                    // (100%) and the space above the line (20%).
                    SvxLineSpacingItem aLSpace(0, aPardMap.nLinespacing);

                    nTokenValue = !bTokenHasValue ? 0 : nTokenValue;
                    if (1000 == nTokenValue )
                        nTokenValue = 240;

                    SvxLineSpaceRule eLnSpc;
                    if (nTokenValue < 0)
                    {
                        eLnSpc = SvxLineSpaceRule::Fix;
                        nTokenValue = -nTokenValue;
                    }
                    else if (nTokenValue == 0)
                    {
                        //if \sl0 is used, the line spacing is automatically
                        //determined
                        eLnSpc = SvxLineSpaceRule::Auto;
                    }
                    else
                        eLnSpc = SvxLineSpaceRule::Min;

                    if (IsCalcValue())
                        CalcValue();

                    if (eLnSpc != SvxLineSpaceRule::Auto)
                        aLSpace.SetLineHeight( static_cast<sal_uInt16>(nTokenValue) );

                    aLSpace.SetLineSpaceRule(eLnSpc);
                    pSet->Put(aLSpace);
                }
                break;

            case RTF_NOCWRAP:
                if( aPardMap.nForbRule )
                {
                    pSet->Put( SvxForbiddenRuleItem( false,
                                                    aPardMap.nForbRule ));
                }
                break;
            case RTF_NOOVERFLOW:
                if( aPardMap.nHangPunct )
                {
                    pSet->Put( SvxHangingPunctuationItem( false,
                                                    aPardMap.nHangPunct ));
                }
                break;

            case RTF_ASPALPHA:
                if( aPardMap.nScriptSpace )
                {
                    pSet->Put( SvxScriptSpaceItem( true,
                                                aPardMap.nScriptSpace ));
                }
                break;

            case RTF_FAFIXED:
            case RTF_FAAUTO:    nFontAlign = SvxParaVertAlignItem::Align::Automatic;
                                goto SET_FONTALIGNMENT;
            case RTF_FAHANG:    nFontAlign = SvxParaVertAlignItem::Align::Top;
                                goto SET_FONTALIGNMENT;
            case RTF_FAVAR:     nFontAlign = SvxParaVertAlignItem::Align::Bottom;
                                goto SET_FONTALIGNMENT;
            case RTF_FACENTER:  nFontAlign = SvxParaVertAlignItem::Align::Center;
                                goto SET_FONTALIGNMENT;
            case RTF_FAROMAN:   nFontAlign = SvxParaVertAlignItem::Align::Baseline;
                                goto SET_FONTALIGNMENT;
SET_FONTALIGNMENT:
            if( aPardMap.nFontAlign )
            {
                pSet->Put( SvxParaVertAlignItem( nFontAlign,
                                                aPardMap.nFontAlign ));
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
                if( aPlainMap.nCaseMap &&
                    IsAttrSttPos() )        // not in the text flow?
                {
                    SvxCaseMap eCaseMap;
                    if( !nTokenValue )
                        eCaseMap = SvxCaseMap::NotMapped;
                    else if( RTF_CAPS == nToken )
                        eCaseMap = SvxCaseMap::Uppercase;
                    else
                        eCaseMap = SvxCaseMap::SmallCaps;

                    pSet->Put( SvxCaseMapItem( eCaseMap, aPlainMap.nCaseMap ));
                }
                break;

            case RTF_DN:
            case RTF_SUB:
                if( aPlainMap.nEscapement )
                {
                    const sal_uInt16 nEsc = aPlainMap.nEscapement;
                    if( -1 == nTokenValue )
                         nTokenValue = 6;  //RTF default \dn value in half-points
                    if( IsCalcValue() )
                        CalcValue();
                    const SvxEscapementItem& rOld =
                        static_cast<const SvxEscapementItem&>(pSet->Get( nEsc,false));
                    sal_Int16 nEs;
                    sal_uInt8 nProp;
                    if( DFLT_ESC_AUTO_SUPER == rOld.GetEsc() )
                    {
                        nEs = DFLT_ESC_AUTO_SUB;
                        nProp = rOld.GetProportionalHeight();
                    }
                    else
                    {
                        nEs = (nToken == RTF_SUB) ? DFLT_ESC_AUTO_SUB : -nTokenValue;
                        nProp = (nToken == RTF_SUB) ? DFLT_ESC_PROP : 100;
                    }
                    pSet->Put( SvxEscapementItem( nEs, nProp, nEsc ));
                }
                break;

            case RTF_NOSUPERSUB:
                if( aPlainMap.nEscapement )
                {
                    const sal_uInt16 nEsc = aPlainMap.nEscapement;
                    pSet->Put( SvxEscapementItem( nEsc ));
                }
                break;

            case RTF_EXPND:
                if( aPlainMap.nKering )
                {
                    if( -1 == nTokenValue )
                        nTokenValue = 0;
                    else
                        nTokenValue *= 5;
                    if( IsCalcValue() )
                        CalcValue();
                    pSet->Put( SvxKerningItem( static_cast<short>(nTokenValue), aPlainMap.nKering ));
                }
                break;

            case RTF_KERNING:
                if( aPlainMap.nAutoKerning )
                {
                    if( -1 == nTokenValue )
                        nTokenValue = 0;
                    else
                        nTokenValue *= 10;
                    if( IsCalcValue() )
                        CalcValue();
                    pSet->Put( SvxAutoKernItem( 0 != nTokenValue,
                                                aPlainMap.nAutoKerning ));
                }
                break;

            case RTF_EXPNDTW:
                if( aPlainMap.nKering )
                {
                    if( -1 == nTokenValue )
                        nTokenValue = 0;
                    if( IsCalcValue() )
                        CalcValue();
                    pSet->Put( SvxKerningItem( static_cast<short>(nTokenValue), aPlainMap.nKering ));
                }
                break;

            case RTF_F:
            case RTF_AF:
                {
                    const vcl::Font& rSVFont = GetFont( sal_uInt16(nTokenValue) );
                    SvxFontItem aTmpItem( rSVFont.GetFamilyType(),
                                    rSVFont.GetFamilyName(), rSVFont.GetStyleName(),
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
// the conversion takes now place in EditRTFParser since for other reasons
// the wrong MapUnit might still be use there
//                   if( IsCalcValue() )
//                       CalcValue();
                    SvxFontHeightItem aTmpItem(
                            static_cast<sal_uInt16>(nTokenValue), 100,
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
                if( aPlainMap.nContour &&
                    IsAttrSttPos() )        // not in the text flow?
                {
                    pSet->Put( SvxContourItem(nTokenValue != 0,
                                aPlainMap.nContour ));
                }
                break;

            case RTF_SHAD:
                if( aPlainMap.nShadowed &&
                    IsAttrSttPos() )        // not in the text flow?
                {
                    pSet->Put( SvxShadowedItem(nTokenValue != 0,
                                aPlainMap.nShadowed ));
                }
                break;

            case RTF_STRIKE:
                if( aPlainMap.nCrossedOut &&
                    IsAttrSttPos() )        // not in the text flow?
                {
                    pSet->Put( SvxCrossedOutItem(
                        nTokenValue ? STRIKEOUT_SINGLE : STRIKEOUT_NONE,
                        aPlainMap.nCrossedOut ));
                }
                break;

            case RTF_STRIKED:
                if( aPlainMap.nCrossedOut )      // not in the text flow?
                {
                    pSet->Put( SvxCrossedOutItem(
                        nTokenValue ? STRIKEOUT_DOUBLE : STRIKEOUT_NONE,
                        aPlainMap.nCrossedOut ));
                }
                break;

            case RTF_UL:
                if( !IsAttrSttPos() )
                    break;
                eUnderline = nTokenValue ? LINESTYLE_SINGLE : LINESTYLE_NONE;
                goto ATTR_SETUNDERLINE;

            case RTF_ULD:
                eUnderline = LINESTYLE_DOTTED;
                goto ATTR_SETUNDERLINE;
            case RTF_ULDASH:
                eUnderline = LINESTYLE_DASH;
                goto ATTR_SETUNDERLINE;
            case RTF_ULDASHD:
                eUnderline = LINESTYLE_DASHDOT;
                goto ATTR_SETUNDERLINE;
            case RTF_ULDASHDD:
                eUnderline = LINESTYLE_DASHDOTDOT;
                goto ATTR_SETUNDERLINE;
            case RTF_ULDB:
                eUnderline = LINESTYLE_DOUBLE;
                goto ATTR_SETUNDERLINE;
            case RTF_ULNONE:
                eUnderline = LINESTYLE_NONE;
                goto ATTR_SETUNDERLINE;
            case RTF_ULTH:
                eUnderline = LINESTYLE_BOLD;
                goto ATTR_SETUNDERLINE;
            case RTF_ULWAVE:
                eUnderline = LINESTYLE_WAVE;
                goto ATTR_SETUNDERLINE;
            case RTF_ULTHD:
                eUnderline = LINESTYLE_BOLDDOTTED;
                goto ATTR_SETUNDERLINE;
            case RTF_ULTHDASH:
                eUnderline = LINESTYLE_BOLDDASH;
                goto ATTR_SETUNDERLINE;
            case RTF_ULLDASH:
                eUnderline = LINESTYLE_LONGDASH;
                goto ATTR_SETUNDERLINE;
            case RTF_ULTHLDASH:
                eUnderline = LINESTYLE_BOLDLONGDASH;
                goto ATTR_SETUNDERLINE;
            case RTF_ULTHDASHD:
                eUnderline = LINESTYLE_BOLDDASHDOT;
                goto ATTR_SETUNDERLINE;
            case RTF_ULTHDASHDD:
                eUnderline = LINESTYLE_BOLDDASHDOTDOT;
                goto ATTR_SETUNDERLINE;
            case RTF_ULHWAVE:
                eUnderline = LINESTYLE_BOLDWAVE;
                goto ATTR_SETUNDERLINE;
            case RTF_ULULDBWAVE:
                eUnderline = LINESTYLE_DOUBLEWAVE;
                goto ATTR_SETUNDERLINE;

            case RTF_ULW:
                eUnderline = LINESTYLE_SINGLE;

                if( aPlainMap.nWordlineMode )
                {
                    pSet->Put( SvxWordLineModeItem( true, aPlainMap.nWordlineMode ));
                }
                goto ATTR_SETUNDERLINE;

ATTR_SETUNDERLINE:
                if( aPlainMap.nUnderline )
                {
                    pSet->Put( SvxUnderlineItem( eUnderline, aPlainMap.nUnderline ));
                }
                break;

            case RTF_ULC:
                if( aPlainMap.nUnderline )
                {
                    std::unique_ptr<SvxUnderlineItem> aUL(std::make_unique<SvxUnderlineItem>(LINESTYLE_SINGLE, aPlainMap.nUnderline));
                    const SfxPoolItem* pItem(nullptr);

                    if( SfxItemState::SET == pSet->GetItemState(aPlainMap.nUnderline, false, &pItem ) )
                    {
                        // is switched off ?
                        if( LINESTYLE_NONE == static_cast<const SvxUnderlineItem*>(pItem)->GetLineStyle() )
                            break;

                        aUL.reset(static_cast<SvxUnderlineItem*>(pItem->Clone()));
                    }
                    else
                    {
                        aUL.reset(static_cast<SvxUnderlineItem*>(pSet->Get( aPlainMap.nUnderline, false).Clone()));
                    }

                    if(LINESTYLE_NONE == aUL->GetLineStyle())
                    {
                        aUL->SetLineStyle(LINESTYLE_SINGLE);
                    }

                    aUL->SetColor(GetColor(sal_uInt16(nTokenValue)));

                    pSet->Put(*aUL);
                }
                break;

            case RTF_OL:
                if( !IsAttrSttPos() )
                    break;
                eOverline = nTokenValue ? LINESTYLE_SINGLE : LINESTYLE_NONE;
                goto ATTR_SETOVERLINE;

            case RTF_OLD:
                eOverline = LINESTYLE_DOTTED;
                goto ATTR_SETOVERLINE;
            case RTF_OLDASH:
                eOverline = LINESTYLE_DASH;
                goto ATTR_SETOVERLINE;
            case RTF_OLDASHD:
                eOverline = LINESTYLE_DASHDOT;
                goto ATTR_SETOVERLINE;
            case RTF_OLDASHDD:
                eOverline = LINESTYLE_DASHDOTDOT;
                goto ATTR_SETOVERLINE;
            case RTF_OLDB:
                eOverline = LINESTYLE_DOUBLE;
                goto ATTR_SETOVERLINE;
            case RTF_OLNONE:
                eOverline = LINESTYLE_NONE;
                goto ATTR_SETOVERLINE;
            case RTF_OLTH:
                eOverline = LINESTYLE_BOLD;
                goto ATTR_SETOVERLINE;
            case RTF_OLWAVE:
                eOverline = LINESTYLE_WAVE;
                goto ATTR_SETOVERLINE;
            case RTF_OLTHD:
                eOverline = LINESTYLE_BOLDDOTTED;
                goto ATTR_SETOVERLINE;
            case RTF_OLTHDASH:
                eOverline = LINESTYLE_BOLDDASH;
                goto ATTR_SETOVERLINE;
            case RTF_OLLDASH:
                eOverline = LINESTYLE_LONGDASH;
                goto ATTR_SETOVERLINE;
            case RTF_OLTHLDASH:
                eOverline = LINESTYLE_BOLDLONGDASH;
                goto ATTR_SETOVERLINE;
            case RTF_OLTHDASHD:
                eOverline = LINESTYLE_BOLDDASHDOT;
                goto ATTR_SETOVERLINE;
            case RTF_OLTHDASHDD:
                eOverline = LINESTYLE_BOLDDASHDOTDOT;
                goto ATTR_SETOVERLINE;
            case RTF_OLHWAVE:
                eOverline = LINESTYLE_BOLDWAVE;
                goto ATTR_SETOVERLINE;
            case RTF_OLOLDBWAVE:
                eOverline = LINESTYLE_DOUBLEWAVE;
                goto ATTR_SETOVERLINE;

            case RTF_OLW:
                eOverline = LINESTYLE_SINGLE;

                if( aPlainMap.nWordlineMode )
                {
                    pSet->Put( SvxWordLineModeItem( true, aPlainMap.nWordlineMode ));
                }
                goto ATTR_SETOVERLINE;

ATTR_SETOVERLINE:
                if( aPlainMap.nUnderline )
                {
                    pSet->Put( SvxOverlineItem( eOverline, aPlainMap.nOverline ));
                }
                break;

            case RTF_OLC:
                if( aPlainMap.nOverline )
                {
                    std::unique_ptr<SvxOverlineItem> aOL(std::make_unique<SvxOverlineItem>(LINESTYLE_SINGLE, aPlainMap.nOverline));
                    const SfxPoolItem* pItem(nullptr);

                    if( SfxItemState::SET == pSet->GetItemState(aPlainMap.nOverline, false, &pItem ) )
                    {
                        // is switched off ?
                        if( LINESTYLE_NONE == static_cast<const SvxOverlineItem*>(pItem)->GetLineStyle() )
                            break;

                        aOL.reset(static_cast<SvxOverlineItem*>(pItem->Clone()));
                    }
                    else
                    {
                        aOL.reset(static_cast<SvxOverlineItem*>(pSet->Get( aPlainMap.nOverline, false).Clone()));
                    }

                    if(LINESTYLE_NONE == aOL->GetLineStyle())
                    {
                        aOL->SetLineStyle(LINESTYLE_SINGLE);
                    }

                    aOL->SetColor(GetColor(sal_uInt16(nTokenValue)));

                    pSet->Put(*aOL);
                }
                break;

            case RTF_UP:
            case RTF_SUPER:
                if( aPlainMap.nEscapement )
                {
                    const sal_uInt16 nEsc = aPlainMap.nEscapement;
                    if( -1 == nTokenValue )
                        nTokenValue = 6;  //RTF default \up value in half-points
                    if( IsCalcValue() )
                        CalcValue();
                    const SvxEscapementItem& rOld =
                        static_cast<const SvxEscapementItem&>(pSet->Get( nEsc,false));
                    sal_Int16 nEs;
                    sal_uInt8 nProp;
                    if( DFLT_ESC_AUTO_SUB == rOld.GetEsc() )
                    {
                        nEs = DFLT_ESC_AUTO_SUPER;
                        nProp = rOld.GetProportionalHeight();
                    }
                    else
                    {
                        nEs =  (nToken == RTF_SUPER) ? DFLT_ESC_AUTO_SUPER : nTokenValue;
                        nProp = (nToken == RTF_SUPER) ? DFLT_ESC_PROP : 100;
                    }
                    pSet->Put( SvxEscapementItem( nEs, nProp, nEsc ));
                }
                break;

            case RTF_CF:
                if( aPlainMap.nColor )
                {
                    pSet->Put( SvxColorItem( GetColor( sal_uInt16(nTokenValue) ),
                                aPlainMap.nColor ));
                }
                break;
            //#i12501# While cb is clearly documented in the rtf spec, word
            //doesn't accept it at all
#if 0
            case RTF_CB:
                if( aPlainMap.nBgColor )
                {
                    pSet->Put( SvxBrushItem( GetColor( sal_uInt16(nTokenValue) ),
                                aPlainMap.nBgColor ));
                }
                break;
#endif

            case RTF_LANG:
                if( aPlainMap.nLanguage )
                {
                    pSet->Put( SvxLanguageItem( LanguageType(nTokenValue),
                                aPlainMap.nLanguage ));
                }
                break;

            case RTF_LANGFE:
                if( aPlainMap.nCJKLanguage )
                {
                    pSet->Put( SvxLanguageItem( LanguageType(nTokenValue),
                                                aPlainMap.nCJKLanguage ));
                }
                break;
            case RTF_ALANG:
                {
                    SvxLanguageItem aTmpItem( LanguageType(nTokenValue),
                                    SID_ATTR_CHAR_LANGUAGE );
                    SetScriptAttr( eCharType, *pSet, aTmpItem );
                }
                break;

            case RTF_RTLCH:
                bIsLeftToRightDef = false;
                break;
            case RTF_LTRCH:
                bIsLeftToRightDef = true;
                break;
            case RTF_RTLPAR:
                if (aPardMap.nDirection)
                {
                    pSet->Put(SvxFrameDirectionItem(SvxFrameDirection::Horizontal_RL_TB,
                        aPardMap.nDirection));
                }
                break;
            case RTF_LTRPAR:
                if (aPardMap.nDirection)
                {
                    pSet->Put(SvxFrameDirectionItem(SvxFrameDirection::Horizontal_LR_TB,
                        aPardMap.nDirection));
                }
                break;
            case RTF_LOCH:      eCharType = LOW_CHARTYPE;           break;
            case RTF_HICH:      eCharType = HIGH_CHARTYPE;          break;
            case RTF_DBCH:      eCharType = DOUBLEBYTE_CHARTYPE;    break;


            case RTF_ACCNONE:
                eEmphasis = FontEmphasisMark::NONE;
                goto ATTR_SETEMPHASIS;
            case RTF_ACCDOT:
                eEmphasis = (FontEmphasisMark::Dot | FontEmphasisMark::PosAbove);
                goto ATTR_SETEMPHASIS;

            case RTF_ACCCOMMA:
                eEmphasis = (FontEmphasisMark::Accent | FontEmphasisMark::PosAbove);
ATTR_SETEMPHASIS:
                if( aPlainMap.nEmphasis )
                {
                    pSet->Put( SvxEmphasisMarkItem( eEmphasis,
                                                       aPlainMap.nEmphasis ));
                }
                break;

            case RTF_TWOINONE:
                if( aPlainMap.nTwoLines )
                {
                    sal_Unicode cStt, cEnd;
                    switch ( nTokenValue )
                    {
                    case 1: cStt = '('; cEnd = ')'; break;
                    case 2: cStt = '['; cEnd = ']'; break;
                    case 3: cStt = '<'; cEnd = '>'; break;
                    case 4: cStt = '{'; cEnd = '}'; break;
                    default: cStt = 0; cEnd = 0; break;
                    }

                    pSet->Put( SvxTwoLinesItem( true, cStt, cEnd,
                                                       aPlainMap.nTwoLines ));
                }
                break;

            case RTF_CHARSCALEX :
                if (aPlainMap.nCharScaleX)
                {
                    //i21372
                    if (nTokenValue < 1 || nTokenValue > 600)
                        nTokenValue = 100;
                    pSet->Put( SvxCharScaleWidthItem( sal_uInt16(nTokenValue),
                                                       aPlainMap.nCharScaleX ));
                }
                break;

            case RTF_HORZVERT:
                if( aPlainMap.nHorzVert )
                {
                    // RTF knows only 90deg
                    pSet->Put( SvxCharRotateItem( 900_deg10, 1 == nTokenValue,
                                                       aPlainMap.nHorzVert ));
                }
                break;

            case RTF_EMBO:
                if (aPlainMap.nRelief)
                {
                    pSet->Put(SvxCharReliefItem(FontRelief::Embossed,
                        aPlainMap.nRelief));
                }
                break;
            case RTF_IMPR:
                if (aPlainMap.nRelief)
                {
                    pSet->Put(SvxCharReliefItem(FontRelief::Engraved,
                        aPlainMap.nRelief));
                }
                break;
            case RTF_V:
                if (aPlainMap.nHidden)
                {
                    pSet->Put(SvxCharHiddenItem(nTokenValue != 0,
                        aPlainMap.nHidden));
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
                if( aPlainMap.nBgColor )
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
                            UnknownAttrToken( nToken );
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

                                if( aPlainMap.nEscapement )
                                    pSet->Put( SvxEscapementItem( nEsc, nProp,
                                                       aPlainMap.nEscapement ));
                            }
                            break;

                        case RTF_HYPHEN:
                            {
                                SvxHyphenZoneItem aHypenZone(
                                            (nTokenValue & 1) != 0,
                                                aPardMap.nHyphenzone );
                                aHypenZone.SetPageEnd((nTokenValue & 2) != 0);

                                if( aPardMap.nHyphenzone &&
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
                                bool bSkip = true;
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

                                    if( aPardMap.nShadow )
                                        pSet->Put( SvxShadowItem( aPardMap.nShadow,
                                                                  &aColor, nDist, eSL ) );

                                    bSkip = false;
                                } while( false );

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
                                    SAL_WARN_IF( nToken != BRACERIGHT,
                                        "editeng",
                                        "} did not follow { as expected");
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
                        bContinue = false;
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
                        SkipToken();
                    bContinue = false;
                }
            }
        }
        if( bContinue )
        {
            nToken = GetNextToken();
        }
        bFirstToken = false;
    }
}

void SvxRTFParser::ReadTabAttr( int nToken, SfxItemSet& rSet )
{
    bool bMethodOwnsToken = false; // #i52542# patch from cmc.
// then read all the TabStops
    SvxTabStop aTabStop;
    SvxTabStopItem aAttr( 0, 0, SvxTabAdjust::Default, aPardMap.nTabStop );
    bool bContinue = true;
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
            aTabStop.GetAdjustment() = SvxTabAdjust::Left;
            break;
        case RTF_TQR:
            aTabStop.GetAdjustment() = SvxTabAdjust::Right;
            break;
        case RTF_TQC:
            aTabStop.GetAdjustment() = SvxTabAdjust::Center;
            break;
        case RTF_TQDEC:
            aTabStop.GetAdjustment() = SvxTabAdjust::Decimal;
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
                    bContinue = false;
                }
            }
            break;

        default:
            bContinue = false;
        }
        if( bContinue )
        {
            nToken = GetNextToken();
            bMethodOwnsToken = true;
        }
    } while( bContinue );

    // Fill with defaults is still missing!
    rSet.Put( aAttr );
    SkipToken();
}

static void SetBorderLine( int nBorderTyp, SvxBoxItem& rItem,
                            const SvxBorderLine& rBorder )
{
    switch( nBorderTyp )
    {
    case RTF_BOX:           // run through all levels
    case RTF_BRDRT:
        rItem.SetLine( &rBorder, SvxBoxItemLine::TOP );
        if( RTF_BOX != nBorderTyp )
            return;
        [[fallthrough]];
    case RTF_BRDRB:
        rItem.SetLine( &rBorder, SvxBoxItemLine::BOTTOM );
        if( RTF_BOX != nBorderTyp )
            return;
        [[fallthrough]];
    case RTF_BRDRL:
        rItem.SetLine( &rBorder, SvxBoxItemLine::LEFT );
        if( RTF_BOX != nBorderTyp )
            return;
        [[fallthrough]];
    case RTF_BRDRR:
        rItem.SetLine( &rBorder, SvxBoxItemLine::RIGHT );
        if( RTF_BOX != nBorderTyp )
            return;
    }
}

void SvxRTFParser::ReadBorderAttr( int nToken, SfxItemSet& rSet,
                                   bool bTableDef )
{
    // then read the border attribute
    std::unique_ptr<SvxBoxItem> aAttr(std::make_unique<SvxBoxItem>(aPardMap.nBox));
    const SfxPoolItem* pItem(nullptr);

    if( SfxItemState::SET == rSet.GetItemState( aPardMap.nBox, false, &pItem ) )
    {
        aAttr.reset(static_cast<SvxBoxItem*>(pItem->Clone()));
    }

    SvxBorderLine aBrd( nullptr, DEF_LINE_WIDTH_0 );  // Simple plain line
    bool bContinue = true;
    int nBorderTyp = 0;

    tools::Long nWidth = 1;
    bool bDoubleWidth = false;

    do {
        switch( nToken )
        {
        case RTF_BOX:
        case RTF_BRDRT:
        case RTF_BRDRB:
        case RTF_BRDRL:
        case RTF_BRDRR:
            nBorderTyp = nToken;
            break;

        case RTF_CLBRDRT:       // Cell top border
            {
                if( bTableDef )
                {
                    if (nBorderTyp != 0)
                        SetBorderLine( nBorderTyp, *aAttr, aBrd );
                    nBorderTyp = RTF_BRDRT;
                }
                break;
            }
        case RTF_CLBRDRB:       // Cell bottom border
            {
                if( bTableDef )
                {
                    if (nBorderTyp != 0)
                        SetBorderLine( nBorderTyp, *aAttr, aBrd );
                    nBorderTyp = RTF_BRDRB;
                }
                break;
            }
        case RTF_CLBRDRL:       // Cell left border
            {
                if( bTableDef )
                {
                    if (nBorderTyp != 0)
                        SetBorderLine( nBorderTyp, *aAttr, aBrd );
                    nBorderTyp = RTF_BRDRL;
                }
                break;
            }
        case RTF_CLBRDRR:       // Cell right border
            {
                if( bTableDef )
                {
                    if (nBorderTyp != 0)
                        SetBorderLine( nBorderTyp, *aAttr, aBrd );
                    nBorderTyp = RTF_BRDRR;
                }
                break;
            }

        case RTF_BRDRDOT:       // dotted border
            aBrd.SetBorderLineStyle(SvxBorderLineStyle::DOTTED);
            break;
        case RTF_BRDRDASH:      // dashed border
            aBrd.SetBorderLineStyle(SvxBorderLineStyle::DASHED);
            break;
        case RTF_BRDRHAIR:      // hairline border
            {
                aBrd.SetBorderLineStyle( SvxBorderLineStyle::SOLID);
                aBrd.SetWidth( DEF_LINE_WIDTH_0 );
            }
            break;
        case RTF_BRDRDB:        // Double border
            aBrd.SetBorderLineStyle(SvxBorderLineStyle::DOUBLE);
            break;
        case RTF_BRDRINSET:     // inset border
            aBrd.SetBorderLineStyle(SvxBorderLineStyle::INSET);
            break;
        case RTF_BRDROUTSET:    // outset border
            aBrd.SetBorderLineStyle(SvxBorderLineStyle::OUTSET);
            break;
        case RTF_BRDRTNTHSG:    // ThinThick Small gap
            aBrd.SetBorderLineStyle(SvxBorderLineStyle::THINTHICK_SMALLGAP);
            break;
        case RTF_BRDRTNTHMG:    // ThinThick Medium gap
            aBrd.SetBorderLineStyle(SvxBorderLineStyle::THINTHICK_MEDIUMGAP);
            break;
        case RTF_BRDRTNTHLG:    // ThinThick Large gap
            aBrd.SetBorderLineStyle(SvxBorderLineStyle::THINTHICK_LARGEGAP);
            break;
        case RTF_BRDRTHTNSG:    // ThickThin Small gap
            aBrd.SetBorderLineStyle(SvxBorderLineStyle::THICKTHIN_SMALLGAP);
            break;
        case RTF_BRDRTHTNMG:    // ThickThin Medium gap
            aBrd.SetBorderLineStyle(SvxBorderLineStyle::THICKTHIN_MEDIUMGAP);
            break;
        case RTF_BRDRTHTNLG:    // ThickThin Large gap
            aBrd.SetBorderLineStyle(SvxBorderLineStyle::THICKTHIN_LARGEGAP);
            break;
        case RTF_BRDREMBOSS:    // Embossed border
            aBrd.SetBorderLineStyle(SvxBorderLineStyle::EMBOSSED);
            break;
        case RTF_BRDRENGRAVE:   // Engraved border
            aBrd.SetBorderLineStyle(SvxBorderLineStyle::ENGRAVED);
            break;

        case RTF_BRDRS:         // single thickness border
            bDoubleWidth = false;
            break;
        case RTF_BRDRTH:        // double thickness border width*2
            bDoubleWidth = true;
            break;
        case RTF_BRDRW:         // border width <255
            nWidth = nTokenValue;
           break;

        case RTF_BRDRCF:        // Border color
                aBrd.SetColor( GetColor( sal_uInt16(nTokenValue) ) );
                break;

        case RTF_BRDRSH:        // Shadowed border
                rSet.Put( SvxShadowItem( aPardMap.nShadow, nullptr, 60 /*3pt*/,
                                        SvxShadowLocation::BottomRight ) );
                break;

        case RTF_BRSP:          // Spacing to content in twip
            {
                switch( nBorderTyp )
                {
                case RTF_BRDRB:
                    aAttr->SetDistance( static_cast<sal_uInt16>(nTokenValue), SvxBoxItemLine::BOTTOM );
                    break;

                case RTF_BRDRT:
                    aAttr->SetDistance( static_cast<sal_uInt16>(nTokenValue), SvxBoxItemLine::TOP );
                    break;

                case RTF_BRDRL:
                    aAttr->SetDistance( static_cast<sal_uInt16>(nTokenValue), SvxBoxItemLine::LEFT );
                    break;

                case RTF_BRDRR:
                    aAttr->SetDistance( static_cast<sal_uInt16>(nTokenValue), SvxBoxItemLine::RIGHT );
                    break;

                case RTF_BOX:
                    aAttr->SetAllDistances( static_cast<sal_uInt16>(nTokenValue) );
                    break;
                }
            }
            break;

        case RTF_BRDRBTW:       // Border formatting group
        case RTF_BRDRBAR:       // Border outside
            // TODO unhandled ATM
            break;

        default:
            bContinue = (nToken & ~(0xff| RTF_SWGDEFS)) == RTF_BRDRDEF;
        }
        if( bContinue )
            nToken = GetNextToken();
    } while( bContinue );

    // Finally compute the border width
    if ( bDoubleWidth ) nWidth *= 2;
    aBrd.SetWidth( nWidth );

    SetBorderLine( nBorderTyp, *aAttr, aBrd );

    rSet.Put( *aAttr );
    SkipToken();
}

static sal_uInt32 CalcShading( sal_uInt32 nColor, sal_uInt32 nFillColor, sal_uInt8 nShading )
{
    nColor = (nColor * nShading) / 100;
    nFillColor = (nFillColor * ( 100 - nShading )) / 100;
    return nColor + nFillColor;
}

void SvxRTFParser::ReadBackgroundAttr( int nToken, SfxItemSet& rSet,
                                       bool bTableDef )
{
    // then read the border attribute
    bool bContinue = true;
    sal_uInt16 nColor = USHRT_MAX, nFillColor = USHRT_MAX;
    sal_uInt8 nFillValue = 0;

    sal_uInt16 nWh = ( nToken & ~0xff ) == RTF_CHRFMT
                    ? aPlainMap.nBgColor
                    : aPardMap.nBrush;

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
            nFillValue = static_cast<sal_uInt8>( nTokenValue / 100 );
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
                bContinue = (nToken & ~(0xff | RTF_TABLEDEF) ) == RTF_SHADINGDEF;
            else
                bContinue = (nToken & ~0xff) == RTF_SHADINGDEF;
        }
        if( bContinue )
            nToken = GetNextToken();
    } while( bContinue );

    Color aCol( COL_WHITE ), aFCol;
    if( !nFillValue )
    {
        // there was only one of two colors specified or no BrushType
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
            aCol = COL_BLACK;

        if( USHRT_MAX != nFillColor )
            aFCol = GetColor( nFillColor );
        else
            aFCol = COL_WHITE;
    }

    Color aColor;
    if( 0 == nFillValue || 100 == nFillValue )
        aColor = aCol;
    else
        aColor = Color(
            static_cast<sal_uInt8>(CalcShading( aCol.GetRed(), aFCol.GetRed(), nFillValue )),
            static_cast<sal_uInt8>(CalcShading( aCol.GetGreen(), aFCol.GetGreen(), nFillValue )),
            static_cast<sal_uInt8>(CalcShading( aCol.GetBlue(), aFCol.GetBlue(), nFillValue )) );

    rSet.Put( SvxBrushItem( aColor, nWh ) );
    SkipToken();
}


// pard / plain handling
void SvxRTFParser::RTFPardPlain( bool const bPard, SfxItemSet** ppSet )
{
    if( bNewGroup || aAttrStack.empty() ) // not at the beginning of a new group
        return;

    SvxRTFItemStackType* pCurrent = aAttrStack.back().get();

    int nLastToken = GetStackPtr(-1)->nTokenId;
    bool bNewStkEntry = true;
    if( RTF_PARD != nLastToken &&
        RTF_PLAIN != nLastToken &&
        BRACELEFT != nLastToken )
    {
        if (pCurrent->aAttrSet.Count() || pCurrent->m_pChildList || pCurrent->nStyleNo)
        {
            // open a new group
            std::unique_ptr<SvxRTFItemStackType> pNew(new SvxRTFItemStackType( *pCurrent, *pInsPos, true ));
            pNew->SetRTFDefaults( GetRTFDefaults() );

            // Set all until here valid attributes
            AttrGroupEnd();
            pCurrent = aAttrStack.empty() ? nullptr : aAttrStack.back().get();  // can be changed after AttrGroupEnd!
            pNew->aAttrSet.SetParent( pCurrent ? &pCurrent->aAttrSet : nullptr );
            aAttrStack.push_back( std::move(pNew) );
            pCurrent = aAttrStack.back().get();
        }
        else
        {
            // continue to use this entry as new
            pCurrent->SetStartPos( *pInsPos );
            bNewStkEntry = false;
        }
    }

    // now reset all to default
    if( bNewStkEntry &&
        ( pCurrent->aAttrSet.GetParent() || pCurrent->aAttrSet.Count() ))
    {
        const SfxPoolItem *pItem, *pDef;
        const sal_uInt16* pPtr;
        sal_uInt16 nCnt;
        const SfxItemSet* pDfltSet = &GetRTFDefaults();
        if( bPard )
        {
            pCurrent->nStyleNo = 0;
            pPtr = reinterpret_cast<sal_uInt16*>(&aPardMap);
            nCnt = sizeof(aPardMap) / sizeof(sal_uInt16);
        }
        else
        {
            pPtr = reinterpret_cast<sal_uInt16*>(&aPlainMap);
            nCnt = sizeof(aPlainMap) / sizeof(sal_uInt16);
        }

        for( sal_uInt16 n = 0; n < nCnt; ++n, ++pPtr )
        {
            // Item set and different -> Set the Default Pool
            if( !*pPtr )
                ;
            else if (SfxItemPool::IsSlot(*pPtr))
                pCurrent->aAttrSet.ClearItem( *pPtr );
            else if( IsChkStyleAttr() )
                pCurrent->aAttrSet.Put( pDfltSet->Get( *pPtr ) );
            else if( !pCurrent->aAttrSet.GetParent() )
            {
                if( SfxItemState::SET ==
                    pDfltSet->GetItemState( *pPtr, false, &pDef ))
                    pCurrent->aAttrSet.Put( *pDef );
                else
                    pCurrent->aAttrSet.ClearItem( *pPtr );
            }
            else if( SfxItemState::SET == pCurrent->aAttrSet.GetParent()->
                        GetItemState( *pPtr, true, &pItem ) &&
                    *( pDef = &pDfltSet->Get( *pPtr )) != *pItem )
                pCurrent->aAttrSet.Put( *pDef );
            else
            {
                if( SfxItemState::SET ==
                    pDfltSet->GetItemState( *pPtr, false, &pDef ))
                    pCurrent->aAttrSet.Put( *pDef );
                else
                    pCurrent->aAttrSet.ClearItem( *pPtr );
            }
        }
    }
    else if( bPard )
        pCurrent->nStyleNo = 0;     // reset Style number

    *ppSet = &pCurrent->aAttrSet;

    if (bPard)
        return;

    //Once we have a default font, then any text without a font specifier is
    //in the default font, and thus has the default font charset, otherwise
    //we can fall back to the ansicpg set codeset
    if (nDfltFont != -1)
    {
        const vcl::Font& rSVFont = GetFont(sal_uInt16(nDfltFont));
        SetEncoding(rSVFont.GetCharSet());
    }
    else
        SetEncoding(GetCodeSet());
}

void SvxRTFParser::SetDefault( int nToken, int nValue )
{
    if( !bNewDoc )
        return;

    SfxItemSet aTmp( *pAttrPool, aWhichMap.data() );
    bool bOldFlag = bIsLeftToRightDef;
    bIsLeftToRightDef = true;
    switch( nToken )
    {
    case RTF_ADEFF:
        bIsLeftToRightDef = false;
        [[fallthrough]];
    case RTF_DEFF:
        {
            if( -1 == nValue )
                nValue = 0;
            const vcl::Font& rSVFont = GetFont( sal_uInt16(nValue) );
            SvxFontItem aTmpItem(
                                rSVFont.GetFamilyType(), rSVFont.GetFamilyName(),
                                rSVFont.GetStyleName(), rSVFont.GetPitch(),
                                rSVFont.GetCharSet(), SID_ATTR_CHAR_FONT );
            SetScriptAttr( NOTDEF_CHARTYPE, aTmp, aTmpItem );
        }
        break;

    case RTF_ADEFLANG:
        bIsLeftToRightDef = false;
        [[fallthrough]];
    case RTF_DEFLANG:
        // store default Language
        if( -1 != nValue )
        {
            SvxLanguageItem aTmpItem( LanguageType(nValue), SID_ATTR_CHAR_LANGUAGE );
            SetScriptAttr( NOTDEF_CHARTYPE, aTmp, aTmpItem );
        }
        break;

    case RTF_DEFTAB:
        if( aPardMap.nTabStop )
        {
            // RTF defines 720 twips as default
            bIsSetDfltTab = true;
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
            sal_uInt16 nTabCount = (SVX_TAB_DEFDIST * 13 ) / sal_uInt16(nValue);
            /*
             cmc, make sure we have at least one, or all hell breaks loose in
             everybody exporters, #i8247#
            */
            if (nTabCount < 1)
                nTabCount = 1;

            // we want Defaulttabs
            SvxTabStopItem aNewTab( nTabCount, sal_uInt16(nValue),
                                SvxTabAdjust::Default, aPardMap.nTabStop );
            while( nTabCount )
                const_cast<SvxTabStop&>(aNewTab[ --nTabCount ]).GetAdjustment() = SvxTabAdjust::Default;

            pAttrPool->SetPoolDefaultItem( aNewTab );
        }
        break;
    }
    bIsLeftToRightDef = bOldFlag;

    if( aTmp.Count() )
    {
        SfxItemIter aIter( aTmp );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        do
        {
            pAttrPool->SetPoolDefaultItem( *pItem );
            pItem = aIter.NextItem();
        } while (pItem);
    }
}

// default: no conversion, leaving everything in twips.
void SvxRTFParser::CalcValue()
{
}

// for tokens that are not evaluated in ReadAttr
void SvxRTFParser::UnknownAttrToken( int )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
