/*************************************************************************
 *
 *  $RCSfile: rtfitem.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-16 17:55:17 $
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

#pragma hdrstop

#define ITEMID_FONTLIST     0
#define ITEMID_FONT         0
#define ITEMID_POSTURE      0
#define ITEMID_WEIGHT       0
#define ITEMID_FONTHEIGHT   0
#define ITEMID_UNDERLINE    0
#define ITEMID_CROSSEDOUT   0
#define ITEMID_SHADOWED     0
#define ITEMID_AUTOKERN     0
#define ITEMID_WORDLINEMODE 0
#define ITEMID_CONTOUR      0
#define ITEMID_PROPSIZE     0
#define ITEMID_COLOR        0
#define ITEMID_CHARSETCOLOR 0
#define ITEMID_KERNING      0
#define ITEMID_CASEMAP      0
#define ITEMID_ESCAPEMENT   0
#define ITEMID_LANGUAGE     0
#define ITEMID_NOLINEBREAK  0
#define ITEMID_NOHYPHENHERE 0
#define ITEMID_BLINK        0
#define ITEMID_PAPERBIN     0
#define ITEMID_SIZE         0
#define ITEMID_LRSPACE      0
#define ITEMID_ULSPACE      0
#define ITEMID_PRINT        0
#define ITEMID_OPAQUE       0
#define ITEMID_PROTECT      0
#define ITEMID_SHADOW       0
#define ITEMID_BOX          0
#define ITEMID_BOXINFO      0
#define TEMID_FMTBREAK      0
#define ITEMID_FMTKEEP      0
#define ITEMID_LINE         0
#define ITEMID_BRUSH        0
#define ITEMID_LINESPACING  0
#define TEMID_ADJUST        0
#define ITEMID_ORPHANS      0
#define ITEMID_WIDOWS       0
#define ITEMID_TABSTOP      0
#define ITEMID_PAGEMODEL    0
#define ITEMID_FMTSPLIT     0
#define ITEMID_HYPHENZONE   0
#define ITEMID_FMTBREAK     0
#define ITEMID_ADJUST       0
#define ITEMID_EMPHASISMARK 0
#define ITEMID_TWOLINES     0

#include "flstitem.hxx"
#include "fontitem.hxx"
#include "postitem.hxx"
#include "wghtitem.hxx"
#include "fhgtitem.hxx"
#include "fwdtitem.hxx"
#include "udlnitem.hxx"
#include "crsditem.hxx"
#include "shdditem.hxx"
#include "akrnitem.hxx"
#include "wrlmitem.hxx"
#include "cntritem.hxx"
#include "prszitem.hxx"
#include "colritem.hxx"
#include "cscoitem.hxx"
#include "kernitem.hxx"
#include "cmapitem.hxx"
#include "escpitem.hxx"
#include "langitem.hxx"
#include "nlbkitem.hxx"
#include "nhypitem.hxx"
#include "lcolitem.hxx"
#include "blnkitem.hxx"
#include "emphitem.hxx"
#include "twolinesitem.hxx"

#include "pbinitem.hxx"
#include "sizeitem.hxx"
#include "lrspitem.hxx"
#include "ulspitem.hxx"
#include "prntitem.hxx"
#include "opaqitem.hxx"
#include "protitem.hxx"
#include "shaditem.hxx"
#include "boxitem.hxx"
#include "brkitem.hxx"
#include "keepitem.hxx"
#include "bolnitem.hxx"
#include "brshitem.hxx"

#include "lspcitem.hxx"
#include "adjitem.hxx"
#include "orphitem.hxx"
#include "widwitem.hxx"
#include "tstpitem.hxx"
#include "pmdlitem.hxx"
#include "spltitem.hxx"
#include "hyznitem.hxx"

#ifndef _RTFTOKEN_H
#include <svtools/rtftoken.h>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#include "svxrtf.hxx"


#define BRACELEFT   '{'
#define BRACERIGHT  '}'


// einige Hilfs-Funktionen
// char
inline const SvxFontHeightItem& GetSize(const SfxItemSet& rSet,USHORT nId,BOOL bInP=TRUE)
    { return (const SvxFontHeightItem&)rSet.Get( nId,bInP); }
inline const SvxEscapementItem& GetEscapement(const SfxItemSet& rSet,USHORT nId,BOOL bInP=TRUE)
    { return (const SvxEscapementItem&)rSet.Get( nId,bInP); }
inline const SvxLineSpacingItem& GetLineSpacing(const SfxItemSet& rSet,USHORT nId,BOOL bInP=TRUE)
    { return (const SvxLineSpacingItem&)rSet.Get( nId,bInP); }
inline const SvxUnderlineItem& GetUnderline(const SfxItemSet& rSet,USHORT nId,BOOL bInP=TRUE)
    { return (const SvxUnderlineItem&)rSet.Get( nId,bInP); }
// frm
inline const SvxLRSpaceItem& GetLRSpace(const SfxItemSet& rSet,USHORT nId,BOOL bInP=TRUE)
    { return (const SvxLRSpaceItem&)rSet.Get( nId,bInP); }
inline const SvxULSpaceItem& GetULSpace(const SfxItemSet& rSet,USHORT nId,BOOL bInP=TRUE)
    { return (const SvxULSpaceItem&)rSet.Get( nId,bInP); }

#define PARDID      ((RTFPardAttrMapIds*)aPardMap.GetData())
#define PLAINID     ((RTFPlainAttrMapIds*)aPlainMap.GetData())


enum RTF_CharTypeDef
{
    NOTDEF_CHARTYPE,
    LOW_CHARTYPE,
    HIGH_CHARTYPE,
    DOUBLEBYTE_CHARTYPE
};


USHORT SvxRTFParser::GetAttrWhichID( USHORT eType, USHORT nSlotId )
{
    const USHORT *pNormal = 0, *pCJK = 0, *pCTL = 0;
    const RTFPlainAttrMapIds* pIds = (RTFPlainAttrMapIds*)aPlainMap.GetData();
    switch( nSlotId )
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

    default:
       DBG_ASSERT( FALSE, "wrong call to GetAttrWhichId" );
       return 0;
    }

    nSlotId = 0;

    if( DOUBLEBYTE_CHARTYPE == eType )
    {
        if( bIsLeftToRightDef )
            nSlotId = *pCJK;
    }
    else if( !bIsLeftToRightDef )
        nSlotId = *pCTL;
    else
        nSlotId = *pNormal;
    return nSlotId;
}

// --------------------

void SvxRTFParser::ReadAttr( int nToken, SfxItemSet* pSet )
{
    DBG_ASSERT( pSet, "Es muss ein SfxItemSet uebergeben werden!" );
    int bFirstToken = TRUE, bWeiter = TRUE;
    USHORT nStyleNo = 0;        // default
    FontUnderline eUnderline;
    FontEmphasisMark eEmphasis;
    bPardTokenRead = FALSE;
    RTF_CharTypeDef eCharType = NOTDEF_CHARTYPE;
    USHORT nWhichId;

    int bChkStkPos = !bNewGroup && aAttrStack.Top();

    while( bWeiter && IsParserWorking() )           // solange bekannte Attribute erkannt werden
    {
        switch( nToken )
        {
        case RTF_PARD:
            RTFPardPlain( TRUE, &pSet );
            nStyleNo = 0;
            bPardTokenRead = TRUE;
            break;

        case RTF_PLAIN:
            RTFPardPlain( FALSE, &pSet );
            break;

        default:
            do {        // middle checked loop
                if( !bChkStkPos )
                    break;

                SvxRTFItemStackType* pAkt = aAttrStack.Top();
                if( !pAkt || (pAkt->pSttNd->GetIdx() == pInsPos->GetNodeIdx() &&
                    pAkt->nSttCnt == pInsPos->GetCntIdx() ))
                    break;

                int nLastToken = GetStackPtr(-1)->nTokenId;
                if( RTF_PARD == nLastToken || RTF_PLAIN == nLastToken )
                    break;

                if( pAkt->aAttrSet.Count() || pAkt->pChildList ||
                    pAkt->nStyleNo )
                {
                    // eine neue Gruppe aufmachen
                    SvxRTFItemStackType* pNew = new SvxRTFItemStackType(
                                                *pAkt, *pInsPos, TRUE );
                    pNew->aAttrSet.SetParent( pAkt->aAttrSet.GetParent() );
                    // alle bis hierher gueltigen Attribute "setzen"
                    AttrGroupEnd();
                    aAttrStack.Push( pNew );
                    pAkt = pNew;
                }
                else
                    // diesen Eintrag als neuen weiterbenutzen
                    pAkt->SetStartPos( *pInsPos );

                pSet = &pAkt->aAttrSet;
            } while( FALSE );

            switch( nToken )
            {
            case RTF_INTBL:
            case RTF_PAGEBB:
            case RTF_SBYS:
            case RTF_V:
            case RTF_CS:
            case RTF_LS:
            case RTF_ILVL:
                    UnknownAttrToken( nToken, pSet );
                    break;

            case RTF_S:
                {
                    nStyleNo = -1 == nTokenValue ? 0 : USHORT(nTokenValue);
                    // setze am akt. auf dem AttrStack stehenden Style die
                    // StyleNummer
                    SvxRTFItemStackType* pAkt = aAttrStack.Top();
                    if( !pAkt )
                        break;

                    pAkt->nStyleNo = USHORT( nStyleNo );

#if 0
// JP 05.09.95: zuruecksetzen der Style-Attribute fuehrt nur zu Problemen.
//              Es muss reichen, wenn das ueber pard/plain erfolgt
//  ansonsten Bugdoc 15304.rtf - nach nur "\pard" falscher Font !!

                    SvxRTFStyleType* pStyle = aStyleTbl.Get( pAkt->nStyleNo );
                    if( pStyle && pStyle->aAttrSet.Count() )
                    {
                        //JP 07.07.95:
                        // alle Attribute, die in der Vorlage gesetzt werden
                        // auf defaults setzen. In RTF werden die Attribute
                        // der Vorlage danach ja wiederholt.
                        // WICHTIG: Attribute die in der Vorlage definiert
                        //          sind, werden zurueckgesetzt !!!!
                        // pAkt->aAttrSet.Put( pStyle->aAttrSet );

                        SfxItemIter aIter( pStyle->aAttrSet );
                        SfxItemPool* pPool = pStyle->aAttrSet.GetPool();
                        USHORT nWh = aIter.GetCurItem()->Which();
                        while( TRUE )
                        {
                            pAkt->aAttrSet.Put( pPool->GetDefaultItem( nWh ));
                            if( aIter.IsAtEnd() )
                                break;
                            nWh = aIter.NextItem()->Which();
                        }
                    }
#endif
                }
                break;

            case RTF_KEEP:
                if( PARDID->nSplit )
                {
                    pSet->Put( SvxFmtSplitItem( FALSE, PARDID->nSplit ));
                }
                break;

            case RTF_KEEPN:
                if( PARDID->nKeep )
                {
                    pSet->Put( SvxFmtKeepItem( TRUE, PARDID->nKeep ));
                }
                break;

            case RTF_LEVEL:
                if( PARDID->nOutlineLvl )
                {
                    pSet->Put( SfxUInt16Item( PARDID->nOutlineLvl,
                                                (UINT16)nTokenValue ));
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
                    USHORT nSz = 0;
                    if( -1 != nTokenValue )
                    {
                        if( IsCalcValue() )
                            CalcValue();
                        nSz = USHORT(nTokenValue);
                    }
                    aLR.SetTxtFirstLineOfst( nSz );
                    pSet->Put( aLR );
                }
                break;

            case RTF_LI:
                if( PARDID->nLRSpace )
                {
                    SvxLRSpaceItem aLR( GetLRSpace(*pSet, PARDID->nLRSpace ));
                    USHORT nSz = 0;
                    if( 0 < nTokenValue )
                    {
                        if( IsCalcValue() )
                            CalcValue();
                        nSz = USHORT(nTokenValue);
                    }
                    aLR.SetTxtLeft( nSz );
                    pSet->Put( aLR );
                }
                break;

            case RTF_RI:
                if( PARDID->nLRSpace )
                {
                    SvxLRSpaceItem aLR( GetLRSpace(*pSet, PARDID->nLRSpace ));
                    USHORT nSz = 0;
                    if( 0 < nTokenValue )
                    {
                        if( IsCalcValue() )
                            CalcValue();
                        nSz = USHORT(nTokenValue);
                    }
                    aLR.SetRight( nSz );
                    pSet->Put( aLR );
                }
                break;

            case RTF_SB:
                if( PARDID->nULSpace )
                {
                    SvxULSpaceItem aUL( GetULSpace(*pSet, PARDID->nULSpace ));
                    USHORT nSz = 0;
                    if( 0 < nTokenValue )
                    {
                        if( IsCalcValue() )
                            CalcValue();
                        nSz = USHORT(nTokenValue);
                    }
                    aUL.SetUpper( nSz );
                    pSet->Put( aUL );
                }
                break;

            case RTF_SA:
                if( PARDID->nULSpace )
                {
                    SvxULSpaceItem aUL( GetULSpace(*pSet, PARDID->nULSpace ));
                    USHORT nSz = 0;
                    if( 0 < nTokenValue )
                    {
                        if( IsCalcValue() )
                            CalcValue();
                        nSz = USHORT(nTokenValue);
                    }
                    aUL.SetLower( nSz );
                    pSet->Put( aUL );
                }
                break;

            case RTF_SLMULT:
                if( PARDID->nLinespacing && 1 == nTokenValue )
                {
                    // dann wird auf mehrzeilig umgeschaltet!
                    SvxLineSpacingItem aLSpace( GetLineSpacing( *pSet,
                                                PARDID->nLinespacing, FALSE ));

                    // wieviel bekommt man aus dem LineHeight Wert heraus

                    // Proportionale-Groesse:
                    // D.H. das Verhaeltnis ergibt sich aus ( n / 240 ) Twips

                    nTokenValue = 240;
                    if( IsCalcValue() )
                        CalcValue();

                    nTokenValue = short( 100L * aLSpace.GetLineHeight()
                                            / long( nTokenValue ) );

                    if( nTokenValue > 200 )     // Datenwert fuer PropLnSp
                        nTokenValue = 200;      // ist ein BYTE !!!

                    aLSpace.SetPropLineSpace( (const BYTE)nTokenValue );
                    aLSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;

                    pSet->Put( aLSpace );
                }
                break;

            case RTF_SL:
                if( PARDID->nLinespacing )
                {
                    // errechne das Verhaeltnis aus dem default Font zu der
                    // Size Angabe. Der Abstand besteht aus der Zeilenhoehe
                    // (100%) und dem Leerraum ueber der Zeile (20%).
                    SvxLineSpacingItem aLSpace( 0, PARDID->nLinespacing );

                    if( !nTokenValue || -1 == nTokenValue || 1000 == nTokenValue )
                        nTokenValue = 240;

                    SvxLineSpace eLnSpc;
                    if( 0 > nTokenValue )
                    {
                        eLnSpc = SVX_LINE_SPACE_FIX;
                        nTokenValue = -nTokenValue;
                    }
                    else
                        eLnSpc = SVX_LINE_SPACE_MIN;

                    if( IsCalcValue() )
                        CalcValue();
                    aLSpace.SetLineHeight( (const USHORT)nTokenValue );
                    aLSpace.GetLineSpaceRule() = eLnSpc;

                    pSet->Put( aLSpace );
                }
                break;


/*  */
            case RTF_B:
                if( 0 != ( nWhichId =
                        GetAttrWhichID( eCharType, SID_ATTR_CHAR_WEIGHT )) &&
                    IsAttrSttPos() )    // nicht im Textfluss ?
                {
                    pSet->Put( SvxWeightItem(
                                nTokenValue ? WEIGHT_BOLD : WEIGHT_NORMAL,
                                nWhichId ));
                }
                break;

            case RTF_CAPS:
            case RTF_SCAPS:
                if( PLAINID->nCaseMap &&
                    IsAttrSttPos() )        // nicht im Textfluss ?
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
                    const USHORT nEsc = PLAINID->nEscapement;
                    if( -1 == nTokenValue || RTF_SUB == nToken )
                        nTokenValue = 6;
                    if( IsCalcValue() )
                        CalcValue();
                    const SvxEscapementItem& rOld = GetEscapement( *pSet, nEsc, FALSE );
                    short nEs;
                    BYTE nProp;
                    if( DFLT_ESC_AUTO_SUPER == rOld.GetEsc() )
                    {
                        nEs = DFLT_ESC_AUTO_SUB;
                        nProp = rOld.GetProp();
                    }
                    else
                    {
                        nEs = (short)-nTokenValue;
                        nProp = DFLT_ESC_PROP;
                    }
                    pSet->Put( SvxEscapementItem( nEs, nProp, nEsc ));
                }
                break;

            case RTF_NOSUPERSUB:
                if( PLAINID->nEscapement )
                {
                    const USHORT nEsc = PLAINID->nEscapement;
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
                if( 0 != ( nWhichId =
                            GetAttrWhichID( eCharType, SID_ATTR_CHAR_FONT )) )
                {
                    const Font& rSVFont = GetFont( USHORT(nTokenValue) );
                    pSet->Put( SvxFontItem( rSVFont.GetFamily(),
                        rSVFont.GetName(), rSVFont.GetStyleName(),
                        rSVFont.GetPitch(), rSVFont.GetCharSet(),
                        nWhichId ));
                }
                break;

            case RTF_FS:
            case RTF_AFS:
                if( 0 != ( nWhichId =
                    GetAttrWhichID( eCharType, SID_ATTR_CHAR_FONTHEIGHT )) )
                {
                    if( -1 == nTokenValue )
                        nTokenValue = 240;
                    else
                        nTokenValue *= 10;
                    if( IsCalcValue() )
                        CalcValue();
                    SvxFontHeightItem aFH( (const USHORT)nTokenValue, 100,
                                            nWhichId );
                    pSet->Put( aFH );
                }
                break;

            case RTF_I:
                if( 0 != ( nWhichId =
                        GetAttrWhichID( eCharType, SID_ATTR_CHAR_POSTURE )) &&
                    IsAttrSttPos() )        // nicht im Textfluss ?
                {
                    pSet->Put( SvxPostureItem(
                                nTokenValue ? ITALIC_NORMAL : ITALIC_NONE,
                                nWhichId ));
                }
                break;

            case RTF_OUTL:
                if( PLAINID->nContour &&
                    IsAttrSttPos() )        // nicht im Textfluss ?
                {
                    pSet->Put( SvxContourItem( nTokenValue ? TRUE : FALSE,
                                PLAINID->nContour ));
                }
                break;

            case RTF_SHAD:
                if( PLAINID->nShadowed &&
                    IsAttrSttPos() )        // nicht im Textfluss ?
                {
                    pSet->Put( SvxShadowedItem( nTokenValue ? TRUE : FALSE,
                                PLAINID->nShadowed ));
                }
                break;

            case RTF_STRIKE:
                if( PLAINID->nCrossedOut &&
                    IsAttrSttPos() )        // nicht im Textfluss ?
                {
                    pSet->Put( SvxCrossedOutItem(
                        nTokenValue ? STRIKEOUT_SINGLE : STRIKEOUT_NONE,
                        PLAINID->nCrossedOut ));
                }
                break;

            case RTF_STRIKED:
                if( PLAINID->nCrossedOut )      // nicht im Textfluss ?
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
                    pSet->Put( SvxWordLineModeItem( TRUE, PLAINID->nWordlineMode ));
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
                    SvxUnderlineItem aUL( UNDERLINE_SINGLE,
                                            PLAINID->nUnderline );
                    const SfxPoolItem* pItem;
                    if( SFX_ITEM_SET == pSet->GetItemState(
                        PLAINID->nUnderline, FALSE, &pItem ) )
                    {
                        // is switched off ?
                        if( UNDERLINE_NONE ==
                            ((SvxUnderlineItem*)pItem)->GetUnderline() )
                            break;
                        aUL = *(SvxUnderlineItem*)pItem;
                    }
                    else
                        aUL = GetUnderline( *pSet, PLAINID->nUnderline, FALSE );

                    if( UNDERLINE_NONE == aUL.GetUnderline() )
                        aUL.SetUnderline( UNDERLINE_SINGLE );
                    aUL.SetColor( GetColor( USHORT(nTokenValue) ));
                    pSet->Put( aUL );
                }
                break;

            case RTF_UP:
            case RTF_SUPER:
                if( PLAINID->nEscapement )
                {
                    const USHORT nEsc = PLAINID->nEscapement;
                    if( -1 == nTokenValue || RTF_SUPER == nToken )
                        nTokenValue = 6;
                    if( IsCalcValue() )
                        CalcValue();
                    const SvxEscapementItem& rOld = GetEscapement( *pSet, nEsc, FALSE );
                    short nEs;
                    BYTE nProp;
                    if( DFLT_ESC_AUTO_SUB == rOld.GetEsc() )
                    {
                        nEs = DFLT_ESC_AUTO_SUPER;
                        nProp = rOld.GetProp();
                    }
                    else
                    {
                        nEs = (short)nTokenValue;
                        nProp = DFLT_ESC_PROP;
                    }
                    pSet->Put( SvxEscapementItem( nEs, nProp, nEsc ));
                }
                break;

            case RTF_CF:
                if( PLAINID->nColor )
                {
                    pSet->Put( SvxColorItem( GetColor( USHORT(nTokenValue) ),
                                PLAINID->nColor ));
                }
                break;
            case RTF_CB:
                if( PLAINID->nBgColor )
                {
                    pSet->Put( SvxColorItem( GetColor( USHORT(nTokenValue) ),
                                PLAINID->nBgColor ));
                }
                break;

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
                if( 0 != ( nWhichId =
                        GetAttrWhichID( eCharType, SID_ATTR_CHAR_LANGUAGE )) )
                {
                    pSet->Put( SvxLanguageItem( (LanguageType)nTokenValue,
                                                    nWhichId ));
                }
                break;

            case RTF_RTLCH:     bIsLeftToRightDef = FALSE;          break;
            case RTF_LTRCH:     bIsLeftToRightDef = TRUE;           break;

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
                    sal_Unicode cStt = 0, cEnd = 0;
                    pSet->Put( SvxTwoLinesItem( TRUE, cStt, cEnd,
                                                       PLAINID->nTwoLines ));
                }
                break;

/*  */

            case BRACELEFT:
                {
                    // teste auf Swg-Interne Tokens
                    short nSkip = 0;
                    if( RTF_IGNOREFLAG != GetNextToken() )
                        nSkip = -1;
                    else if( (nToken = GetNextToken() ) & RTF_SWGDEFS )
                    {
                        switch( nToken )
                        {
                        case RTF_PGDSCNO:
                        case RTF_PGBRK:
                        case RTF_SOUTLVL:
                            UnknownAttrToken( nToken, pSet );
                            // ueberlese die schliessende Klammer
                            GetNextToken();
                            break;

                        case RTF_SWG_ESCPROP:
                            {
                                // prozentuale Veraenderung speichern !
                                BYTE nProp = BYTE( nTokenValue / 100 );
                                short nEsc = 0;
                                if( 1 == ( nTokenValue % 100 ))
                                    // Erkennung unseres AutoFlags!
                                    nEsc = DFLT_ESC_AUTO_SUPER;

                                if( PLAINID->nEscapement )
                                    pSet->Put( SvxEscapementItem( nEsc, nProp,
                                                       PLAINID->nEscapement ));
                                // ueberlese die schliessende Klammer
                                GetNextToken();
                            }
                            break;

                        case RTF_HYPHEN:
                            {
                                SvxHyphenZoneItem aHypenZone(
                                            (nTokenValue & 1) ? TRUE : FALSE,
                                                PARDID->nHyphenzone );
                                aHypenZone.SetPageEnd(
                                            (nTokenValue & 2) ? TRUE : FALSE );

                                if( PARDID->nHyphenzone &&
                                    RTF_HYPHLEAD == GetNextToken() &&
                                    RTF_HYPHTRAIL == GetNextToken() &&
                                    RTF_HYPHMAX == GetNextToken() )
                                {
                                    aHypenZone.GetMinLead() =
                                        BYTE(GetStackPtr( -2 )->nTokenValue);
                                    aHypenZone.GetMinTrail() =
                                            BYTE(GetStackPtr( -1 )->nTokenValue);
                                    aHypenZone.GetMaxHyphens() =
                                            BYTE(nTokenValue);

                                    pSet->Put( aHypenZone );
                                }
                                else
                                    SkipGroup();        // ans Ende der Gruppe
                                GetNextToken();         // Klammer ueberlesen
                            }
                            break;

                        case RTF_SHADOW:
                            {
                                int bSkip = TRUE;
                                do {    // middle check loop
                                    SvxShadowLocation eSL = SvxShadowLocation( nTokenValue );
                                    if( RTF_SHDW_DIST != GetNextToken() )
                                        break;
                                    USHORT nDist = USHORT( nTokenValue );

                                    if( RTF_SHDW_STYLE != GetNextToken() )
                                        break;
                                    //! (pb) class Brush removed -> obsolete
                                    //! BrushStyle eStyle = BrushStyle( nTokenValue );

                                    if( RTF_SHDW_COL != GetNextToken() )
                                        break;
                                    USHORT nCol = USHORT( nTokenValue );

                                    if( RTF_SHDW_FCOL != GetNextToken() )
                                        break;
                                    USHORT nFillCol = USHORT( nTokenValue );

                                    Color aColor = GetColor( nCol );

                                    if( PARDID->nShadow )
                                        pSet->Put( SvxShadowItem( PARDID->nShadow,
                                                                  &aColor, nDist, eSL ) );

                                    bSkip = FALSE;
                                } while( FALSE );

                                if( bSkip )
                                    SkipGroup();        // ans Ende der Gruppe
                                GetNextToken();         // Klammer ueberlesen
                            }
                            break;

                        default:
                            if( (nToken & ~(0xff | RTF_SWGDEFS)) == RTF_TABSTOPDEF )
                            {
                                nToken = SkipToken( -2 );
                                ReadTabAttr( nToken, *pSet );
                            }
                            else if( (nToken & ~(0xff| RTF_SWGDEFS)) == RTF_BRDRDEF)
                            {
                                nToken = SkipToken( -2 );
                                ReadBorderAttr( nToken, *pSet );
                            }
                            else        // also kein Attribut mehr
                                nSkip = -2;
                            break;
                        }
                    }
                    else
                        nSkip = -2;

                    if( nSkip )             // alles voellig unbekannt
                    {
                        --nSkip;            // BRACELEFT: ist das naechste Token
                        SkipToken( nSkip );
                        bWeiter = FALSE;
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
                    // kenne das Token nicht also das Token "in den Parser zurueck"
                    if( !bFirstToken )
                        SkipToken( -1 );
                    bWeiter = FALSE;
                }
            }
        }
        if( bWeiter )
        {
            nToken = GetNextToken();
        }
        bFirstToken = FALSE;
    }

/*
    // teste Attribute gegen ihre Styles
    if( IsChkStyleAttr() && pSet->Count() && !pInsPos->GetCntIdx() )
    {
        SvxRTFStyleType* pStyle = aStyleTbl.Get( nStyleNo );
        if( pStyle && pStyle->aAttrSet.Count() )
        {
            // alle Attribute, die schon vom Style definiert sind, aus dem
            // akt. Set entfernen
            const SfxPoolItem* pItem;
            SfxItemIter aIter( *pSet );
            USHORT nWhich = aIter.GetCurItem()->Which();
            while( TRUE )
            {
                if( SFX_ITEM_SET == pStyle->aAttrSet.GetItemState(
                    nWhich, FALSE, &pItem ) && *pItem == *aIter.GetCurItem())
                    pSet->ClearItem( nWhich );      // loeschen

                if( aIter.IsAtEnd() )
                    break;
                nWhich = aIter.NextItem()->Which();
            }
        }
    }
*/
}

void SvxRTFParser::ReadTabAttr( int nToken, SfxItemSet& rSet )
{
    // dann lese doch mal alle TabStops ein
    SvxTabStop aTabStop;
    SvxTabStopItem aAttr( 0, 0, SVX_TAB_ADJUST_DEFAULT, PARDID->nTabStop );
    int bWeiter = TRUE;
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
                aTabStop = SvxTabStop();    // alle Werte default
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
                // Swg - Kontrol BRACELEFT RTF_IGNOREFLAG RTF_TLSWG BRACERIGHT
                short nSkip = 0;
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nSkip = -1;
                else if( RTF_TLSWG != ( nToken = GetNextToken() ))
                    nSkip = -2;
                else
                {
                    aTabStop.GetDecimal() = BYTE(nTokenValue & 0xff);
                    aTabStop.GetFill() = BYTE((nTokenValue >> 8) & 0xff);
                    // ueberlese noch die schliessende Klammer
                    GetNextToken();
                }
                if( nSkip )
                {
                    SkipToken( nSkip );     // Ignore wieder zurueck
                    bWeiter = FALSE;
                }
            }
            break;

        default:
            bWeiter = FALSE;
        }
        if( bWeiter )
            nToken = GetNextToken();
    } while( bWeiter );

    // mit Defaults aufuellen fehlt noch !!!
    rSet.Put( aAttr );
    SkipToken( -1 );
}

static void SetBorderLine( int nBorderTyp, SvxBoxItem& rItem,
                            const SvxBorderLine& rBorder )
{
    switch( nBorderTyp )
    {
    case RTF_BOX:           // alle Stufen durchlaufen

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
    // dann lese doch mal das BoderAttribut ein
    SvxBoxItem aAttr( PARDID->nBox );
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( PARDID->nBox, FALSE, &pItem ) )
        aAttr = *(SvxBoxItem*)pItem;

    SvxBorderLine aBrd( 0, DEF_LINE_WIDTH_0, 0, 0 );    // einfache Linien
    int bWeiter = TRUE, nBorderTyp = 0;

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
                // auf defaults setzen
                aBrd.SetOutWidth( DEF_LINE_WIDTH_0 );
                aBrd.SetInWidth( 0 );
                aBrd.SetDistance( 0 );
                aBrd.SetColor( Color( COL_BLACK ) );
            }
            break;


// werden noch nicht ausgewertet
        case RTF_BRSP:
            {
                switch( nBorderTyp )
                {
                case RTF_BRDRB:
                    aAttr.SetDistance( (USHORT)nTokenValue, BOX_LINE_BOTTOM );
                    break;

                case RTF_BRDRT:
                    aAttr.SetDistance( (USHORT)nTokenValue, BOX_LINE_TOP );
                    break;

                case RTF_BRDRL:
                    aAttr.SetDistance( (USHORT)nTokenValue, BOX_LINE_LEFT );
                    break;

                case RTF_BRDRR:
                    aAttr.SetDistance( (USHORT)nTokenValue, BOX_LINE_RIGHT );
                    break;

                case RTF_BOX:
                    aAttr.SetDistance( (USHORT)nTokenValue );
                    break;
                }
            }
            break;

case RTF_BRDRBTW:
case RTF_BRDRBAR:           break;


        case RTF_BRDRCF:
            {
                aBrd.SetColor( GetColor( USHORT(nTokenValue) ) );
                SetBorderLine( nBorderTyp, aAttr, aBrd );
            }
            break;

        case RTF_BRDRTH:
            aBrd.SetOutWidth( DEF_LINE_WIDTH_1 );
            aBrd.SetInWidth( 0 );
            aBrd.SetDistance( 0 );
            goto SETBORDERLINE;
            break;

        case RTF_BRDRDB:
            aBrd.SetOutWidth( DEF_DOUBLE_LINE0_OUT );
            aBrd.SetInWidth( DEF_DOUBLE_LINE0_IN );
            aBrd.SetDistance( DEF_DOUBLE_LINE0_DIST );
            goto SETBORDERLINE;
            break;

        case RTF_BRDRSH:
            // schattierte Box
            {
                rSet.Put( SvxShadowItem( PARDID->nShadow, (Color*) 0, 60 /*3pt*/,
                                        SVX_SHADOW_BOTTOMRIGHT ) );
            }
            break;

        case RTF_BRDRW:
            if( -1 != nTokenValue )
            {
                // sollte es eine "dicke" Linie sein ?
                if( DEF_LINE_WIDTH_0 != aBrd.GetOutWidth() )
                    nTokenValue *= 2;

                // eine Doppelline?
                if( aBrd.GetInWidth() )
                {
                    // WinWord - Werte an StarOffice anpassen
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
                    // WinWord - Werte an StarOffice anpassen
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

        case RTF_BRDRS:
        case RTF_BRDRDOT:
        case RTF_BRDRHAIR:
        case RTF_BRDRDASH:
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
                    int bSwgControl = TRUE, bFirstToken = TRUE;
                    nToken = GetNextToken();
                    do {
                        switch( nToken )
                        {
                        case RTF_BRDBOX:
                            aAttr.SetDistance( USHORT(nTokenValue) );
                            break;

                        case RTF_BRDRT:
                        case RTF_BRDRB:
                        case RTF_BRDRR:
                        case RTF_BRDRL:
                            nBorderTyp = nToken;
                            bFirstToken = FALSE;
                            if( RTF_BRDLINE_COL != GetNextToken() )
                            {
                                bSwgControl = FALSE;
                                break;
                            }
                            aBrd.SetColor( GetColor( USHORT(nTokenValue) ));

                            if( RTF_BRDLINE_IN != GetNextToken() )
                            {
                                bSwgControl = FALSE;
                                break;
                            }
                            aBrd.SetInWidth( USHORT(nTokenValue));

                            if( RTF_BRDLINE_OUT != GetNextToken() )
                            {
                                bSwgControl = FALSE;
                                break;
                            }
                            aBrd.SetOutWidth( USHORT(nTokenValue));

                            if( RTF_BRDLINE_DIST != GetNextToken() )
                            {
                                bSwgControl = FALSE;
                                break;
                            }
                            aBrd.SetDistance( USHORT(nTokenValue));
                            SetBorderLine( nBorderTyp, aAttr, aBrd );
                            break;

                        default:
                            bSwgControl = FALSE;
                            break;
                        }

                        if( bSwgControl )
                        {
                            nToken = GetNextToken();
                            bFirstToken = FALSE;
                        }
                    } while( bSwgControl );

                    // Ende der Swg-Gruppe
                    // -> lese noch die schliessende Klammer
                    if( BRACERIGHT == nToken )
                        ;
                    else if( !bFirstToken )
                    {
                        // es ist ein Parser-Fehler, springe zum
                        // Ende der Gruppe
                        SkipGroup();
                        // schliessende BRACERIGHT ueberspringen
                        GetNextToken();
                    }
                    else
                        nSkip = -2;
                }

                if( nSkip )
                {
                    SkipToken( nSkip );     // Ignore wieder zurueck
                    bWeiter = FALSE;
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

inline ULONG CalcShading( ULONG nColor, ULONG nFillColor, BYTE nShading )
{
    nColor = (nColor * nShading) / 100;
    nFillColor = (nFillColor * ( 100 - nShading )) / 100;
    return nColor + nFillColor;
}

void SvxRTFParser::ReadBackgroundAttr( int nToken, SfxItemSet& rSet, int bTableDef )
{
    // dann lese doch mal das BoderAttribut ein
    int bWeiter = TRUE;
    USHORT nColor = USHRT_MAX, nFillColor = USHRT_MAX;
    BYTE nFillValue = 0;

    do {
        switch( nToken )
        {
        case RTF_CLCBPAT:
        case RTF_CBPAT:
            nFillColor = USHORT( nTokenValue );
            break;

        case RTF_CLCFPAT:
        case RTF_CFPAT:
            nColor = USHORT( nTokenValue );
            break;

        case RTF_CLSHDNG:
        case RTF_SHADING:
            nFillValue = (BYTE)( nTokenValue / 100 );
            break;

        case RTF_CLBGDKHOR:
        case RTF_BGDKHORIZ:
        case RTF_CLBGDKVERT:
        case RTF_BGDKVERT:
        case RTF_CLBGDKBDIAG:
        case RTF_BGDKBDIAG:
        case RTF_CLBGDKFDIAG:
        case RTF_BGDKFDIAG:
        case RTF_CLBGDKCROSS:
        case RTF_BGDKCROSS:
        case RTF_CLBGDKDCROSS:
        case RTF_BGDKDCROSS:
            // dark -> 60%
            nFillValue = 60;
            break;

        case RTF_CLBGHORIZ:
        case RTF_BGHORIZ:
        case RTF_CLBGVERT:
        case RTF_BGVERT:
        case RTF_CLBGBDIAG:
        case RTF_BGBDIAG:
        case RTF_CLBGFDIAG:
        case RTF_BGFDIAG:
        case RTF_CLBGCROSS:
        case RTF_BGCROSS:
        case RTF_CLBGDCROSS:
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
        // es wurde nur eine von beiden Farben angegeben oder kein BrushTyp
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
            (BYTE)CalcShading( aCol.GetRed(), aFCol.GetRed(), nFillValue ),
            (BYTE)CalcShading( aCol.GetGreen(), aFCol.GetGreen(), nFillValue ),
            (BYTE)CalcShading( aCol.GetBlue(), aFCol.GetBlue(), nFillValue ) );

    rSet.Put( SvxBrushItem( aColor, PARDID->nBrush ) );

    SkipToken( -1 );
}


// pard / plain abarbeiten
void SvxRTFParser::RTFPardPlain( int bPard, SfxItemSet** ppSet )
{
    if( !bNewGroup && aAttrStack.Top() )    // nicht am Anfang einer neuen Gruppe
    {
        SvxRTFItemStackType* pAkt = aAttrStack.Top();

        int nLastToken = GetStackPtr(-1)->nTokenId;
        int bNewStkEntry = TRUE;
        if( RTF_PARD != nLastToken &&
            RTF_PLAIN != nLastToken &&
            BRACELEFT != nLastToken )
        {
            if( pAkt->aAttrSet.Count() || pAkt->pChildList || pAkt->nStyleNo )
            {
                // eine neue Gruppe aufmachen
                SvxRTFItemStackType* pNew = new SvxRTFItemStackType( *pAkt, *pInsPos, TRUE );
                pNew->aAttrSet.SetParent( pAkt->aAttrSet.GetParent() );
                // alle bis hierher gueltigen Attribute "setzen"
                AttrGroupEnd();
                aAttrStack.Push( pNew );
                pAkt = pNew;
            }
            else
            {
                // diesen Eintrag als neuen weiterbenutzen
                pAkt->SetStartPos( *pInsPos );
                bNewStkEntry = FALSE;
            }
        }

        // jetzt noch alle auf default zuruecksetzen
        if( bNewStkEntry &&
            ( pAkt->aAttrSet.GetParent() || pAkt->aAttrSet.Count() ))
        {
            const SfxPoolItem *pItem, *pDef;
            const USHORT* pPtr;
            USHORT nCnt;
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

            for( USHORT n = 0; n < nCnt; ++n, ++pPtr )
            {
                // Item gesetzt und unterschiedlich -> das Pooldefault setzen
                //JP 06.04.98: bei Items die nur SlotItems sind, darf nicht
                //              auf das Default zugefriffen werden. Diese
                //              werden gecleart
                if( !*pPtr )
                    ;
                else if( SFX_WHICH_MAX < *pPtr )
                    pAkt->aAttrSet.ClearItem( *pPtr );
                else if( IsChkStyleAttr() )
                    pAkt->aAttrSet.Put( pAttrPool->GetDefaultItem( *pPtr ) );
                else if( !pAkt->aAttrSet.GetParent() )
                    pAkt->aAttrSet.ClearItem( *pPtr );
                else if( SFX_ITEM_SET == pAkt->aAttrSet.GetParent()->
                            GetItemState( *pPtr, TRUE, &pItem ) &&
                        *( pDef = &pAttrPool->GetDefaultItem( *pPtr ))
                                    != *pItem )
                    pAkt->aAttrSet.Put( *pDef );
                else
                    pAkt->aAttrSet.ClearItem( *pPtr );
            }
        }
        else if( bPard )
            pAkt->nStyleNo = 0;     // Style-Nummer zuruecksetzen

        *ppSet = &pAkt->aAttrSet;
    }
}

void SvxRTFParser::SetDefault( int nToken, short nValue )
{
    if( !bNewDoc )
        return;

    USHORT nWhichId;
    BOOL bLefToRight = TRUE;
    switch( nToken )
    {
    case RTF_ADEFF: bLefToRight = FALSE;  // no break!
    case RTF_DEFF:

        if( 0 != ( nWhichId = GetAttrWhichID( 0, SID_ATTR_CHAR_FONT )))
        {
            if( -1 == nValue )
                nValue = 0;
            const Font& rSVFont = GetFont( USHORT(nValue) );
            pAttrPool->SetPoolDefaultItem( SvxFontItem(
                    rSVFont.GetFamily(), rSVFont.GetName(),
                    rSVFont.GetStyleName(), rSVFont.GetPitch(),
                    rSVFont.GetCharSet(),
                    nWhichId ));
        }
        break;

    case RTF_ADEFLANG:  bLefToRight = FALSE;  // no break!
    case RTF_DEFLANG:
        // default Language merken
        if( -1 != nValue &&
            0 != ( nWhichId = GetAttrWhichID( 0, SID_ATTR_CHAR_LANGUAGE )))
        {
            pAttrPool->SetPoolDefaultItem(
                SvxLanguageItem( (const LanguageType)nValue, nWhichId ));
        }
        break;

    case RTF_DEFTAB:
        if( PARDID->nTabStop )
        {
            // RTF definiert 720 twips als default
            bIsSetDfltTab = TRUE;
            if( -1 == nValue || !nValue )
                nValue = 720;

            // wer keine Twips haben moechte ...
            if( IsCalcValue() )
            {
                nTokenValue = nValue;
                CalcValue();
                nValue = (short)nTokenValue;
            }

            // Verhaeltnis der def. TabWidth / Tabs errechnen und
            // enstsprechend die neue Anzahl errechnen.
/*-----------------14.12.94 19:32-------------------
 ?? wie kommt man auf die 13 ??
--------------------------------------------------*/
            USHORT nAnzTabs = (SVX_TAB_DEFDIST * 13 ) / USHORT(nValue);

            // wir wollen Defaulttabs
            SvxTabStopItem aNewTab( nAnzTabs, USHORT(nValue),
                                SVX_TAB_ADJUST_DEFAULT, PARDID->nTabStop );
            while( nAnzTabs )
                ((SvxTabStop&)aNewTab[ --nAnzTabs ]).GetAdjustment() = SVX_TAB_ADJUST_DEFAULT;

            pAttrPool->SetPoolDefaultItem( aNewTab );
        }
        break;
    }
}

// default: keine Umrechnung, alles bei Twips lassen.
void SvxRTFParser::CalcValue()
{
}

    // fuer Tokens, die im ReadAttr nicht ausgewertet werden
void SvxRTFParser::UnknownAttrToken( int nToken, SfxItemSet* pSet )
{
}


