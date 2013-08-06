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
#include <editeng/charscaleitem.hxx>
#include <txtatr.hxx>
#include <sfx2/printer.hxx>
#include <svx/svdobj.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtflcnt.hxx>
#include <fmtcntnt.hxx>
#include <fmtftn.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <fmtfld.hxx>
#include <doc.hxx>
#include <viewsh.hxx>   // ViewShell
#include <rootfrm.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <dcontact.hxx>
#include <fldbas.hxx>      // SwField
#include <pam.hxx>         // SwPosition        (lcl_MinMaxNode)
#include <itratr.hxx>
#include <htmltbl.hxx>
#include <swtable.hxx>
#include <redlnitr.hxx>
#include <fmtsrnd.hxx>
#include <itrtxt.hxx>
#include <breakit.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <editeng/lrspitem.hxx>
#include <switerator.hxx>
#include <boost/foreach.hpp>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;

/*************************************************************************
 *                      SwAttrIter::Chg()
 *************************************************************************/

void SwAttrIter::Chg( SwTxtAttr *pHt )
{
    OSL_ENSURE( pHt && pFnt, "No attribute of font available for change");
    if( pRedln && pRedln->IsOn() )
        pRedln->ChangeTxtAttr( pFnt, *pHt, sal_True );
    else
        aAttrHandler.PushAndChg( *pHt, *pFnt );
    nChgCnt++;
}

/*************************************************************************
 *                      SwAttrIter::Rst()
 *************************************************************************/

void SwAttrIter::Rst( SwTxtAttr *pHt )
{
    OSL_ENSURE( pHt && pFnt, "No attribute of font available for reset");
    // get top from stack after removing pHt
    if( pRedln && pRedln->IsOn() )
        pRedln->ChangeTxtAttr( pFnt, *pHt, sal_False );
    else
        aAttrHandler.PopAndChg( *pHt, *pFnt );
    nChgCnt--;
}

/*************************************************************************
 *              virtual SwAttrIter::~SwAttrIter()
 *************************************************************************/

SwAttrIter::~SwAttrIter()
{
    delete pRedln;
    delete pFnt;
}

/*************************************************************************
 *                      SwAttrIter::GetAttr()
 *
 * Liefert fuer eine Position das Attribut, wenn das Attribut genau auf
 * der Position nPos liegt und kein EndIndex besitzt.
 * GetAttr() wird fuer Attribute benoetigt, die die Formatierung beeinflussen
 * sollen, ohne dabei den Inhalt des Strings zu veraendern. Solche "entarteten"
 * Attribute sind z.B. Felder (die expandierten Text bereit halten) und
 * zeilengebundene Frames. Um Mehrdeutigkeiten zwischen verschiedenen
 * solcher Attribute zu vermeiden, werden beim Anlegen eines Attributs
 * an der Startposition ein Sonderzeichen in den String einfuegt.
 * Der Formatierer stoesst auf das Sonderzeichen und holt sich per
 * GetAttr() das entartete Attribut.
 *************************************************************************/

SwTxtAttr *SwAttrIter::GetAttr( const xub_StrLen nPosition ) const
{
    return (m_pTxtNode) ? m_pTxtNode->GetTxtAttrForCharAt(nPosition) : 0;
}

/*************************************************************************
 *                        SwAttrIter::SeekAndChg()
 *************************************************************************/

sal_Bool SwAttrIter::SeekAndChgAttrIter( const xub_StrLen nNewPos, OutputDevice* pOut )
{
    sal_Bool bRet = ImplSeekAndChgAttrIter(nNewPos, pOut);
    return MergeCharBorder(false) || bRet;
}

sal_Bool SwAttrIter::ImplSeekAndChgAttrIter( const xub_StrLen nNewPos, OutputDevice* pOut )
{
    sal_Bool bChg = nStartIndex && nNewPos == nPos ? pFnt->IsFntChg() : Seek( nNewPos );
    if ( pLastOut != pOut )
    {
        pLastOut = pOut;
        pFnt->SetFntChg( sal_True );
        bChg = sal_True;
    }
    if( bChg )
    {
        // wenn der Aenderungszaehler auf Null ist, kennen wir die MagicNo
        // des gewuenschten Fonts ...
        if ( !nChgCnt && !nPropFont )
            pFnt->SetMagic( aMagicNo[ pFnt->GetActual() ],
                aFntIdx[ pFnt->GetActual() ], pFnt->GetActual() );
        pFnt->ChgPhysFnt( pShell, *pOut );
    }

    return bChg;
}

sal_Bool SwAttrIter::IsSymbol( const xub_StrLen nNewPos )
{
    Seek( nNewPos );
    if ( !nChgCnt && !nPropFont )
        pFnt->SetMagic( aMagicNo[ pFnt->GetActual() ],
            aFntIdx[ pFnt->GetActual() ], pFnt->GetActual() );
    return pFnt->IsSymbol( pShell );
}

/*************************************************************************
 *                        SwAttrIter::SeekStartAndChg()
 *************************************************************************/
sal_Bool SwAttrIter::SeekStartAndChgAttrIter( OutputDevice* pOut, const sal_Bool bParaFont )
{
    sal_Bool bRet = ImplSeekStartAndChgAttrIter( pOut, bParaFont );
    return bParaFont ? bRet : MergeCharBorder(true) || bRet;
}

sal_Bool SwAttrIter::ImplSeekStartAndChgAttrIter( OutputDevice* pOut, const sal_Bool bParaFont )
{
    if ( pRedln && pRedln->ExtOn() )
        pRedln->LeaveExtend( *pFnt, 0 );

    // reset font to its original state
    aAttrHandler.Reset();
    aAttrHandler.ResetFont( *pFnt );

    nStartIndex = nEndIndex = nPos = nChgCnt = 0;
    if( nPropFont )
        pFnt->SetProportion( nPropFont );
    if( pRedln )
    {
        pRedln->Clear( pFnt );
        if( !bParaFont )
            nChgCnt = nChgCnt + pRedln->Seek( *pFnt, 0, STRING_LEN );
        else
            pRedln->Reset();
    }

    if ( pHints && !bParaFont )
    {
        SwTxtAttr *pTxtAttr;
        // Solange wir noch nicht am Ende des StartArrays angekommen sind &&
        // das TextAttribut an Position 0 beginnt ...
        while ( ( nStartIndex < pHints->GetStartCount() ) &&
                !(*(pTxtAttr=pHints->GetStart(nStartIndex))->GetStart()) )
        {
            // oeffne die TextAttribute
            Chg( pTxtAttr );
            nStartIndex++;
        }
    }

    sal_Bool bChg = pFnt->IsFntChg();
    if ( pLastOut != pOut )
    {
        pLastOut = pOut;
        pFnt->SetFntChg( sal_True );
        bChg = sal_True;
    }
    if( bChg )
    {
        // wenn der Aenderungszaehler auf Null ist, kennen wir die MagicNo
        // des gewuenschten Fonts ...
        if ( !nChgCnt && !nPropFont )
            pFnt->SetMagic( aMagicNo[ pFnt->GetActual() ],
                aFntIdx[ pFnt->GetActual() ], pFnt->GetActual() );
        pFnt->ChgPhysFnt( pShell, *pOut );
    }
    return bChg;
}

/*************************************************************************
 *                       SwAttrIter::SeekFwd()
 *************************************************************************/

// AMA: Neuer AttrIter Nov 94

void SwAttrIter::SeekFwd( const xub_StrLen nNewPos )
{
    SwTxtAttr *pTxtAttr;

    if ( nStartIndex ) // wenn ueberhaupt schon Attribute geoeffnet wurden...
    {
        // Schliesse Attr, die z. Z. geoeffnet sind, vor nNewPos+1 aber enden.

        // Solange wir noch nicht am Ende des EndArrays angekommen sind &&
        // das TextAttribut vor oder an der neuen Position endet ...
        while ( ( nEndIndex < pHints->GetEndCount() ) &&
                (*(pTxtAttr=pHints->GetEnd(nEndIndex))->GetAnyEnd()<=nNewPos))
        {
            // schliesse die TextAttribute, deren StartPos vor
            // oder an der alten nPos lag, die z.Z. geoeffnet sind.
            if (*pTxtAttr->GetStart() <= nPos)  Rst( pTxtAttr );
            nEndIndex++;
        }
    }
    else // ueberlies die nicht geoeffneten Enden
    {
        while ( ( nEndIndex < pHints->GetEndCount() ) &&
                (*(pTxtAttr=pHints->GetEnd(nEndIndex))->GetAnyEnd()<=nNewPos))
        {
            nEndIndex++;
        }
    }
    // Solange wir noch nicht am Ende des StartArrays angekommen sind &&
    // das TextAttribut vor oder an der neuen Position beginnt ...
    while ( ( nStartIndex < pHints->GetStartCount() ) &&
           (*(pTxtAttr=pHints->GetStart(nStartIndex))->GetStart()<=nNewPos))
    {

        // oeffne die TextAttribute, deren Ende hinter der neuen Position liegt
        if ( *pTxtAttr->GetAnyEnd() > nNewPos )  Chg( pTxtAttr );
        nStartIndex++;
    }

}

/*************************************************************************
 *                       SwAttrIter::Seek()
 *************************************************************************/

sal_Bool SwAttrIter::Seek( const xub_StrLen nNewPos )
{
    if ( pRedln && pRedln->ExtOn() )
        pRedln->LeaveExtend( *pFnt, nNewPos );

    if( pHints )
    {
        if( !nNewPos || nNewPos < nPos || m_bPrevSeekRemBorder )
        {
            if( pRedln )
                pRedln->Clear( NULL );

            // reset font to its original state
            aAttrHandler.Reset();
            aAttrHandler.ResetFont( *pFnt );

            if( nPropFont )
                pFnt->SetProportion( nPropFont );
            nStartIndex = nEndIndex = nPos = 0;
            nChgCnt = 0;

            // Achtung!
            // resetting the font here makes it necessary to apply any
            // changes for extended input directly to the font
            if ( pRedln && pRedln->ExtOn() )
            {
                pRedln->UpdateExtFont( *pFnt );
                ++nChgCnt;
            }
        }
        SeekFwd( nNewPos );
    }

    pFnt->SetActual( SwScriptInfo::WhichFont( nNewPos, 0, pScriptInfo ) );

    if( pRedln )
        nChgCnt = nChgCnt + pRedln->Seek( *pFnt, nNewPos, nPos );
    nPos = nNewPos;

    if( nPropFont )
        pFnt->SetProportion( nPropFont );

    return pFnt->IsFntChg();
}

/*************************************************************************
 *                      SwAttrIter::GetNextAttr()
 *************************************************************************/

xub_StrLen SwAttrIter::GetNextAttr( ) const
{
    xub_StrLen nNext = STRING_LEN;
    if( pHints )
    {
        // are there attribute starts left?
        for (sal_uInt16 i = nStartIndex; i < pHints->GetStartCount(); ++i)
        {
            SwTxtAttr *const pAttr(pHints->GetStart(i));
            if (!pAttr->IsFormatIgnoreStart())
            {
                nNext = *pAttr->GetStart();
                break;
            }
        }
        // are there attribute ends left?
        for (sal_uInt16 i = nEndIndex; i < pHints->GetEndCount(); ++i)
        {
            SwTxtAttr *const pAttr(pHints->GetEnd(i));
            if (!pAttr->IsFormatIgnoreEnd())
            {
                xub_StrLen const nNextEnd = *pAttr->GetAnyEnd();
                nNext = std::min(nNext, nNextEnd); // pick nearest one
                break;
            }
        }
    }
    if (m_pTxtNode!=NULL) {
        //TODO maybe use hints like FieldHints for this instead of looking at the text...
        int l=(nNext<m_pTxtNode->Len()?nNext:m_pTxtNode->Len());
        sal_uInt16 p=nPos;
        while (p<l && m_pTxtNode->GetTxt()[p] != CH_TXT_ATR_FIELDSTART
                   && m_pTxtNode->GetTxt()[p] != CH_TXT_ATR_FIELDEND
                   && m_pTxtNode->GetTxt()[p] != CH_TXT_ATR_FORMELEMENT)
        {
            ++p;
        }
        if ((p<l && p>nPos) || nNext<=p)
        nNext=p;
        else
        nNext=p+1;
    }
    if( pRedln )
        return pRedln->GetNextRedln( nNext );
    return nNext;
}

static bool lcl_HasMergeableBorder(const SwFont& rFirst, const SwFont& rSecond)
{
    return
        rFirst.GetOrientation() == rSecond.GetOrientation() &&
        rFirst.GetTopBorder() == rSecond.GetTopBorder() &&
        rFirst.GetBottomBorder() == rSecond.GetBottomBorder() &&
        rFirst.GetLeftBorder() == rSecond.GetLeftBorder() &&
        rFirst.GetRightBorder() == rSecond.GetRightBorder();
}

bool SwAttrIter::MergeCharBorder( const bool bStart )
{
    const xub_StrLen nActPos = nPos;
    bool bRemoveLeft = false;
    bool bRemoveRight = false;
    SwFont aTmpFont = *pFnt;
    const sal_Int32 nTmpStart = nStartIndex;

    // Check whether next neightbour has same border and height
    if( aTmpFont.GetRightBorder() && pHints && nEndIndex < pHints->GetEndCount() )
    {
        ImplSeekAndChgAttrIter(GetNextAttr(), pLastOut);
        if( aTmpFont.GetHeight(pShell, *pLastOut) == pFnt->GetHeight(pShell, *pLastOut) &&
            lcl_HasMergeableBorder(aTmpFont, *pFnt) )
        {
            bRemoveRight = true;
        }
    }

    // Check whether previous neightbour has same border and height
    if( aTmpFont.GetLeftBorder() && nTmpStart > 0)
    {
        ImplSeekAndChgAttrIter(nActPos-1, pLastOut);
        if( aTmpFont.GetHeight(pShell, *pLastOut) == pFnt->GetHeight(pShell, *pLastOut) &&
            lcl_HasMergeableBorder(aTmpFont, *pFnt) )
        {
            bRemoveLeft = true;
        }
    }

    // If the iterator changed its position, than we have to reset it.
    if( nPos != nActPos )
    {
        if( bStart )
            ImplSeekStartAndChgAttrIter(pLastOut, false);
        else
            ImplSeekAndChgAttrIter(nActPos, pLastOut);
    }

    if( bRemoveRight )
        pFnt->SetRightBorder(0);
    if( bRemoveLeft )
        pFnt->SetLeftBorder(0);

    return (m_bPrevSeekRemBorder = bRemoveLeft || bRemoveRight);
}

class SwMinMaxArgs
{
public:
    OutputDevice* pOut;
    ViewShell* pSh;
    sal_uLong &rMin;
    sal_uLong &rMax;
    sal_uLong &rAbsMin;
    long nRowWidth;
    long nWordWidth;
    long nWordAdd;
    xub_StrLen nNoLineBreak;
    SwMinMaxArgs( OutputDevice* pOutI, ViewShell* pShI, sal_uLong& rMinI, sal_uLong &rMaxI, sal_uLong &rAbsI )
        : pOut( pOutI ), pSh( pShI ), rMin( rMinI ), rMax( rMaxI ), rAbsMin( rAbsI )
        { nRowWidth = nWordWidth = nWordAdd = 0; nNoLineBreak = STRING_LEN; }
    void Minimum( long nNew ) const { if( (long)rMin < nNew ) rMin = nNew; }
    void NewWord() { nWordAdd = nWordWidth = 0; }
};

static sal_Bool lcl_MinMaxString( SwMinMaxArgs& rArg, SwFont* pFnt, const XubString &rTxt,
    xub_StrLen nIdx, xub_StrLen nEnd )
{
    sal_Bool bRet = sal_False;
    while( nIdx < nEnd )
    {
        xub_StrLen nStop = nIdx;
        sal_Bool bClear;
        LanguageType eLang = pFnt->GetLanguage();
        if( g_pBreakIt->GetBreakIter().is() )
        {
            bClear = CH_BLANK == rTxt.GetChar( nStop );
            Boundary aBndry( g_pBreakIt->GetBreakIter()->getWordBoundary( rTxt, nIdx,
                             g_pBreakIt->GetLocale( eLang ),
                             WordType::DICTIONARY_WORD, sal_True ) );
            nStop = (xub_StrLen)aBndry.endPos;
            if( nIdx <= aBndry.startPos && nIdx && nIdx-1 != rArg.nNoLineBreak )
                rArg.NewWord();
            if( nStop == nIdx )
                ++nStop;
            if( nStop > nEnd )
                nStop = nEnd;
        }
        else
        {
            while( nStop < nEnd && CH_BLANK != rTxt.GetChar( nStop ) )
                ++nStop;
            bClear = nStop == nIdx;
            if ( bClear )
            {
                rArg.NewWord();
                while( nStop < nEnd && CH_BLANK == rTxt.GetChar( nStop ) )
                    ++nStop;
            }
        }

        SwDrawTextInfo aDrawInf( rArg.pSh, *rArg.pOut, 0, rTxt, nIdx, nStop - nIdx );
        long nAktWidth = pFnt->_GetTxtSize( aDrawInf ).Width();
        rArg.nRowWidth += nAktWidth;
        if( bClear )
            rArg.NewWord();
        else
        {
            rArg.nWordWidth += nAktWidth;
            if( (long)rArg.rAbsMin < rArg.nWordWidth )
                rArg.rAbsMin = rArg.nWordWidth;
            rArg.Minimum( rArg.nWordWidth + rArg.nWordAdd );
            bRet = sal_True;
        }
        nIdx = nStop;
    }
    return bRet;
}

sal_Bool SwTxtNode::IsSymbol( const xub_StrLen nBegin ) const//swmodtest 080307
{
    SwScriptInfo aScriptInfo;
    SwAttrIter aIter( *(SwTxtNode*)this, aScriptInfo );
    aIter.Seek( nBegin );
    return aIter.GetFnt()->IsSymbol(
        const_cast<ViewShell *>(getIDocumentLayoutAccess()->GetCurrentViewShell()) );//swmod 080311
}

class SwMinMaxNodeArgs
{
public:
    sal_uLong nMaxWidth;    // Summe aller Rahmenbreite
    long nMinWidth;     // Breitester Rahmen
    long nLeftRest;     // noch nicht von Rahmen ueberdeckter Platz im l. Rand
    long nRightRest;    // noch nicht von Rahmen ueberdeckter Platz im r. Rand
    long nLeftDiff;     // Min/Max-Differenz des Rahmens im linken Rand
    long nRightDiff;    // Min/Max-Differenz des Rahmens im rechten Rand
    sal_uLong nIndx;        // Indexnummer des Nodes
    void Minimum( long nNew ) { if( nNew > nMinWidth ) nMinWidth = nNew; }
};

static void lcl_MinMaxNode( SwFrmFmt* pNd, SwMinMaxNodeArgs* pIn )
{
    const SwFmtAnchor& rFmtA = pNd->GetAnchor();

    bool bCalculate = false;
    if ((FLY_AT_PARA == rFmtA.GetAnchorId()) ||
        (FLY_AT_CHAR == rFmtA.GetAnchorId()))
    {
        bCalculate = true;
    }

    if (bCalculate)
    {
        const SwPosition *pPos = rFmtA.GetCntntAnchor();
        OSL_ENSURE(pPos && pIn, "Unexpected NULL arguments");
        if (!pPos || !pIn || pIn->nIndx != pPos->nNode.GetIndex())
            bCalculate = false;
    }

    if (bCalculate)
    {
        long nMin, nMax;
        SwHTMLTableLayout *pLayout = 0;
        MSHORT nWhich = pNd->Which();
        if( RES_DRAWFRMFMT != nWhich )
        {
            // Enthaelt der Rahmen zu Beginn oder am Ende eine Tabelle?
            const SwNodes& rNodes = pNd->GetDoc()->GetNodes();
            const SwFmtCntnt& rFlyCntnt = pNd->GetCntnt();
            sal_uLong nStt = rFlyCntnt.GetCntntIdx()->GetIndex();
            SwTableNode* pTblNd = rNodes[nStt+1]->GetTableNode();
            if( !pTblNd )
            {
                SwNode *pNd2 = rNodes[nStt];
                pNd2 = rNodes[pNd2->EndOfSectionIndex()-1];
                if( pNd2->IsEndNode() )
                    pTblNd = pNd2->StartOfSectionNode()->GetTableNode();
            }

            if( pTblNd )
                pLayout = pTblNd->GetTable().GetHTMLTableLayout();
        }

        const SwFmtHoriOrient& rOrient = pNd->GetHoriOrient();
        sal_Int16 eHoriOri = rOrient.GetHoriOrient();

        long nDiff;
        if( pLayout )
        {
            nMin = pLayout->GetMin();
            nMax = pLayout->GetMax();
            nDiff = nMax - nMin;
        }
        else
        {
            if( RES_DRAWFRMFMT == nWhich )
            {
                const SdrObject* pSObj = pNd->FindSdrObject();
                if( pSObj )
                    nMin = pSObj->GetCurrentBoundRect().GetWidth();
                else
                nMin = 0;

            }
            else
            {
                const SwFmtFrmSize &rSz = pNd->GetFrmSize();
                nMin = rSz.GetWidth();
            }
            nMax = nMin;
            nDiff = 0;
        }

        const SvxLRSpaceItem &rLR = pNd->GetLRSpace();
        nMin += rLR.GetLeft();
        nMin += rLR.GetRight();
        nMax += rLR.GetLeft();
        nMax += rLR.GetRight();

        if( SURROUND_THROUGHT == pNd->GetSurround().GetSurround() )
        {
            pIn->Minimum( nMin );
            return;
        }

        // Rahmen, die recht bzw. links ausgerichtet sind, gehen nur
        // teilweise in die Max-Berechnung ein, da der Rand schon berueck-
        // sichtigt wird. Nur wenn die Rahmen in den Textkoerper ragen,
        // wird dieser Teil hinzuaddiert.
        switch( eHoriOri )
        {
            case text::HoriOrientation::RIGHT:
            {
                if( nDiff )
                {
                    pIn->nRightRest -= pIn->nRightDiff;
                    pIn->nRightDiff = nDiff;
                }
                if( text::RelOrientation::FRAME != rOrient.GetRelationOrient() )
                {
                    if( pIn->nRightRest > 0 )
                        pIn->nRightRest = 0;
                }
                pIn->nRightRest -= nMin;
                break;
            }
            case text::HoriOrientation::LEFT:
            {
                if( nDiff )
                {
                    pIn->nLeftRest -= pIn->nLeftDiff;
                    pIn->nLeftDiff = nDiff;
                }
                if( text::RelOrientation::FRAME != rOrient.GetRelationOrient() &&
                    pIn->nLeftRest < 0 )
                    pIn->nLeftRest = 0;
                pIn->nLeftRest -= nMin;
                break;
            }
            default:
            {
                pIn->nMaxWidth += nMax;
                pIn->Minimum( nMin );
            }
        }
    }
}

#define FLYINCNT_MIN_WIDTH 284

// changing this method very likely requires changing of
// "GetScalingOfSelectedText"
void SwTxtNode::GetMinMaxSize( sal_uLong nIndex, sal_uLong& rMin, sal_uLong &rMax,
                               sal_uLong& rAbsMin, OutputDevice* pOut ) const
{
    ViewShell* pSh = 0;
    GetDoc()->GetEditShell( &pSh );
    if( !pOut )
    {
        if( pSh )
            pOut = pSh->GetWin();
        if( !pOut )
            pOut = GetpApp()->GetDefaultDevice();
    }

    MapMode aOldMap( pOut->GetMapMode() );
    pOut->SetMapMode( MapMode( MAP_TWIP ) );

    rMin = 0;
    rMax = 0;
    rAbsMin = 0;

    const SvxLRSpaceItem &rSpace = GetSwAttrSet().GetLRSpace();
    long nLROffset = rSpace.GetTxtLeft() + GetLeftMarginWithNum( sal_True );
    short nFLOffs;
    // Bei Numerierung ist ein neg. Erstzeileneinzug vermutlich
    // bereits gefuellt...
    if( !GetFirstLineOfsWithNum( nFLOffs ) || nFLOffs > nLROffset )
        nLROffset = nFLOffs;

    SwMinMaxNodeArgs aNodeArgs;
    aNodeArgs.nMinWidth = 0;
    aNodeArgs.nMaxWidth = 0;
    aNodeArgs.nLeftRest = nLROffset;
    aNodeArgs.nRightRest = rSpace.GetRight();
    aNodeArgs.nLeftDiff = 0;
    aNodeArgs.nRightDiff = 0;
    if( nIndex )
    {
        SwFrmFmts* pTmp = (SwFrmFmts*)GetDoc()->GetSpzFrmFmts();
        if( pTmp )
        {
            aNodeArgs.nIndx = nIndex;
            BOOST_FOREACH( SwFrmFmt *pFmt, *pTmp )
                lcl_MinMaxNode( pFmt, &aNodeArgs );
        }
    }
    if( aNodeArgs.nLeftRest < 0 )
        aNodeArgs.Minimum( nLROffset - aNodeArgs.nLeftRest );
    aNodeArgs.nLeftRest -= aNodeArgs.nLeftDiff;
    if( aNodeArgs.nLeftRest < 0 )
        aNodeArgs.nMaxWidth -= aNodeArgs.nLeftRest;

    if( aNodeArgs.nRightRest < 0 )
        aNodeArgs.Minimum( rSpace.GetRight() - aNodeArgs.nRightRest );
    aNodeArgs.nRightRest -= aNodeArgs.nRightDiff;
    if( aNodeArgs.nRightRest < 0 )
        aNodeArgs.nMaxWidth -= aNodeArgs.nRightRest;

    SwScriptInfo aScriptInfo;
    SwAttrIter aIter( *(SwTxtNode*)this, aScriptInfo );
    xub_StrLen nIdx = 0;
    aIter.SeekAndChgAttrIter( nIdx, pOut );
    xub_StrLen nLen = m_Text.getLength();
    long nAktWidth = 0;
    MSHORT nAdd = 0;
    SwMinMaxArgs aArg( pOut, pSh, rMin, rMax, rAbsMin );
    while( nIdx < nLen )
    {
        xub_StrLen nNextChg = aIter.GetNextAttr();
        xub_StrLen nStop = aScriptInfo.NextScriptChg( nIdx );
        if( nNextChg > nStop )
            nNextChg = nStop;
        SwTxtAttr *pHint = NULL;
        sal_Unicode cChar = CH_BLANK;
        nStop = nIdx;
        while( nStop < nLen && nStop < nNextChg &&
               CH_TAB != ( cChar = m_Text[nStop] ) &&
               CH_BREAK != cChar && CHAR_HARDBLANK != cChar &&
               CHAR_HARDHYPHEN != cChar && CHAR_SOFTHYPHEN != cChar &&
               !pHint )
        {
            if( ( CH_TXTATR_BREAKWORD != cChar && CH_TXTATR_INWORD != cChar )
                || ( 0 == ( pHint = aIter.GetAttr( nStop ) ) ) )
                ++nStop;
        }
        if ( lcl_MinMaxString( aArg, aIter.GetFnt(), m_Text, nIdx, nStop ) )
        {
            nAdd = 20;
        }
        nIdx = nStop;
        aIter.SeekAndChgAttrIter( nIdx, pOut );
        switch( cChar )
        {
            case CH_BREAK  :
            {
                if( (long)rMax < aArg.nRowWidth )
                    rMax = aArg.nRowWidth;
                aArg.nRowWidth = 0;
                aArg.NewWord();
                aIter.SeekAndChgAttrIter( ++nIdx, pOut );
            }
            break;
            case CH_TAB    :
            {
                aArg.NewWord();
                aIter.SeekAndChgAttrIter( ++nIdx, pOut );
            }
            break;
            case CHAR_SOFTHYPHEN:
                ++nIdx;
            break;
            case CHAR_HARDBLANK:
            case CHAR_HARDHYPHEN:
            {
                OUString sTmp( cChar );
                SwDrawTextInfo aDrawInf( const_cast<ViewShell *>(getIDocumentLayoutAccess()->GetCurrentViewShell()),
                    *pOut, 0, sTmp, 0, 1, 0, sal_False );//swmod 080311
                nAktWidth = aIter.GetFnt()->_GetTxtSize( aDrawInf ).Width();
                aArg.nWordWidth += nAktWidth;
                aArg.nRowWidth += nAktWidth;
                if( (long)rAbsMin < aArg.nWordWidth )
                    rAbsMin = aArg.nWordWidth;
                aArg.Minimum( aArg.nWordWidth + aArg.nWordAdd );
                aArg.nNoLineBreak = nIdx++;
            }
            break;
            case CH_TXTATR_BREAKWORD:
            case CH_TXTATR_INWORD:
            {
                if( !pHint )
                    break;
                long nOldWidth = aArg.nWordWidth;
                long nOldAdd = aArg.nWordAdd;
                aArg.NewWord();

                switch( pHint->Which() )
                {
                    case RES_TXTATR_FLYCNT :
                    {
                        SwFrmFmt *pFrmFmt = pHint->GetFlyCnt().GetFrmFmt();
                        const SvxLRSpaceItem &rLR = pFrmFmt->GetLRSpace();
                        if( RES_DRAWFRMFMT == pFrmFmt->Which() )
                        {
                            const SdrObject* pSObj = pFrmFmt->FindSdrObject();
                            if( pSObj )
                                nAktWidth = pSObj->GetCurrentBoundRect().GetWidth();
                            else
                                nAktWidth = 0;
                        }
                        else
                        {
                            const SwFmtFrmSize& rTmpSize = pFrmFmt->GetFrmSize();
                            if( RES_FLYFRMFMT == pFrmFmt->Which()
                                && rTmpSize.GetWidthPercent() )
                            {
/*-----------------------------------------------------------------------------
 * Hier ein HACK fuer folgende Situation: In dem Absatz befindet sich
 * ein Textrahmen mit relativer Groesse. Dann nehmen wir mal als minimale
 * Breite 0,5 cm und als maximale KSHRT_MAX.
 * Sauberer und vielleicht spaeter notwendig waere es, ueber den Inhalt
 * des Textrahmens zu iterieren und GetMinMaxSize rekursiv zu rufen.
 * --------------------------------------------------------------------------*/
                                nAktWidth = FLYINCNT_MIN_WIDTH; // 0,5 cm
                                if( (long)rMax < KSHRT_MAX )
                                    rMax = KSHRT_MAX;
                            }
                            else
                                nAktWidth = pFrmFmt->GetFrmSize().GetWidth();
                        }
                        nAktWidth += rLR.GetLeft();
                        nAktWidth += rLR.GetRight();
                        aArg.nWordAdd = nOldWidth + nOldAdd;
                        aArg.nWordWidth = nAktWidth;
                        aArg.nRowWidth += nAktWidth;
                        if( (long)rAbsMin < aArg.nWordWidth )
                            rAbsMin = aArg.nWordWidth;
                        aArg.Minimum( aArg.nWordWidth + aArg.nWordAdd );
                        break;
                    }
                    case RES_TXTATR_FTN :
                    {
                        const XubString aTxt = pHint->GetFtn().GetNumStr();
                        if( lcl_MinMaxString( aArg, aIter.GetFnt(), aTxt, 0,
                            aTxt.Len() ) )
                            nAdd = 20;
                        break;
                    }
                    case RES_TXTATR_FIELD :
                    {
                        SwField *pFld = (SwField*)pHint->GetFld().GetFld();
                        const String aTxt = pFld->ExpandField(true);
                        if( lcl_MinMaxString( aArg, aIter.GetFnt(), aTxt, 0,
                            aTxt.Len() ) )
                            nAdd = 20;
                        break;
                    }
                    default: aArg.nWordWidth = nOldWidth;
                             aArg.nWordAdd = nOldAdd;

                }
                aIter.SeekAndChgAttrIter( ++nIdx, pOut );
            }
            break;
        }
    }
    if( (long)rMax < aArg.nRowWidth )
        rMax = aArg.nRowWidth;

    nLROffset += rSpace.GetRight();

    rAbsMin += nLROffset;
    rAbsMin += nAdd;
    rMin += nLROffset;
    rMin += nAdd;
    if( (long)rMin < aNodeArgs.nMinWidth )
        rMin = aNodeArgs.nMinWidth;
    if( (long)rAbsMin < aNodeArgs.nMinWidth )
        rAbsMin = aNodeArgs.nMinWidth;
    rMax += aNodeArgs.nMaxWidth;
    rMax += nLROffset;
    rMax += nAdd;
    if( rMax < rMin ) // z.B. Rahmen mit Durchlauf gehen zunaechst nur
        rMax = rMin;  // in das Minimum ein
    pOut->SetMapMode( aOldMap );
}

/*************************************************************************
 *                      SwTxtNode::GetScalingOfSelectedText()
 *
 * Calculates the width of the text part specified by nStt and nEnd,
 * the height of the line containing nStt is devided by this width,
 * indicating the scaling factor, if the text part is rotated.
 * Having CH_BREAKs in the text part, this method returns the scaling
 * factor for the longest of the text parts separated by the CH_BREAKs.
 *
 * changing this method very likely requires changing of "GetMinMaxSize"
 *************************************************************************/

sal_uInt16 SwTxtNode::GetScalingOfSelectedText( xub_StrLen nStt, xub_StrLen nEnd )
    const
{
    ViewShell* pSh = NULL;
    OutputDevice* pOut = NULL;
    GetDoc()->GetEditShell( &pSh );

    if ( pSh )
        pOut = &pSh->GetRefDev();
    else
    {
        //Zugriff ueber StarONE, es muss keine Shell existieren oder aktiv sein.
        if ( getIDocumentSettingAccess()->get(IDocumentSettingAccess::HTML_MODE) )
            pOut = GetpApp()->GetDefaultDevice();
        else
            pOut = getIDocumentDeviceAccess()->getReferenceDevice( true );
    }

    OSL_ENSURE( pOut, "GetScalingOfSelectedText without outdev" );

    MapMode aOldMap( pOut->GetMapMode() );
    pOut->SetMapMode( MapMode( MAP_TWIP ) );

    if ( nStt == nEnd )
    {
        if ( !g_pBreakIt->GetBreakIter().is() )
            return 100;

        SwScriptInfo aScriptInfo;
        SwAttrIter aIter( *(SwTxtNode*)this, aScriptInfo );
        aIter.SeekAndChgAttrIter( nStt, pOut );

        Boundary aBound =
            g_pBreakIt->GetBreakIter()->getWordBoundary( GetTxt(), nStt,
            g_pBreakIt->GetLocale( aIter.GetFnt()->GetLanguage() ),
            WordType::DICTIONARY_WORD, sal_True );

        if ( nStt == aBound.startPos )
        {
            // cursor is at left or right border of word
            pOut->SetMapMode( aOldMap );
            return 100;
        }

        nStt = (xub_StrLen)aBound.startPos;
        nEnd = (xub_StrLen)aBound.endPos;

        if ( nStt == nEnd )
        {
            pOut->SetMapMode( aOldMap );
            return 100;
        }
    }

    SwScriptInfo aScriptInfo;
    SwAttrIter aIter( *(SwTxtNode*)this, aScriptInfo );

    // We do not want scaling attributes to be considered during this
    // calculation. For this, we push a temporary scaling attribute with
    // scaling value 100 and priority flag on top of the scaling stack
    SwAttrHandler& rAH = aIter.GetAttrHandler();
    SvxCharScaleWidthItem aItem(100, RES_CHRATR_SCALEW);
    SwTxtAttrEnd aAttr( aItem, nStt, nEnd );
    aAttr.SetPriorityAttr( sal_True );
    rAH.PushAndChg( aAttr, *(aIter.GetFnt()) );

    xub_StrLen nIdx = nStt;

    sal_uLong nWidth = 0;
    sal_uLong nProWidth = 0;

    while( nIdx < nEnd )
    {
        aIter.SeekAndChgAttrIter( nIdx, pOut );

        // scan for end of portion
        xub_StrLen nNextChg = aIter.GetNextAttr();
        xub_StrLen nStop = aScriptInfo.NextScriptChg( nIdx );
        if( nNextChg > nStop )
            nNextChg = nStop;

        nStop = nIdx;
        sal_Unicode cChar = CH_BLANK;
        SwTxtAttr* pHint = NULL;

        // stop at special characters in [ nIdx, nNextChg ]
        while( nStop < nEnd && nStop < nNextChg )
        {
            cChar = m_Text[nStop];
            if (
                CH_TAB == cChar ||
                CH_BREAK == cChar ||
                CHAR_HARDBLANK == cChar ||
                CHAR_HARDHYPHEN == cChar ||
                CHAR_SOFTHYPHEN == cChar ||
                (
                  (CH_TXTATR_BREAKWORD == cChar || CH_TXTATR_INWORD == cChar) &&
                  (0 == (pHint = aIter.GetAttr(nStop)))
                )
               )
            {
                break;
            }
            else
                ++nStop;
        }

        // calculate text widths up to cChar
        if ( nStop > nIdx )
        {
            SwDrawTextInfo aDrawInf( pSh, *pOut, 0, GetTxt(), nIdx, nStop - nIdx );
            nProWidth += aIter.GetFnt()->_GetTxtSize( aDrawInf ).Width();
        }

        nIdx = nStop;
        aIter.SeekAndChgAttrIter( nIdx, pOut );

        if ( cChar == CH_BREAK )
        {
            nWidth = std::max( nWidth, nProWidth );
            nProWidth = 0;
            nIdx++;
        }
        else if ( cChar == CH_TAB )
        {
            // tab receives width of one space
            OUString sTmp( CH_BLANK );
            SwDrawTextInfo aDrawInf( pSh, *pOut, 0, sTmp, 0, 1 );
            nProWidth += aIter.GetFnt()->_GetTxtSize( aDrawInf ).Width();
            nIdx++;
        }
        else if ( cChar == CHAR_SOFTHYPHEN )
            ++nIdx;
        else if ( cChar == CHAR_HARDBLANK || cChar == CHAR_HARDHYPHEN )
        {
            OUString sTmp( cChar );
            SwDrawTextInfo aDrawInf( pSh, *pOut, 0, sTmp, 0, 1 );
            nProWidth += aIter.GetFnt()->_GetTxtSize( aDrawInf ).Width();
            nIdx++;
        }
        else if ( pHint && ( cChar == CH_TXTATR_BREAKWORD || CH_TXTATR_INWORD ) )
        {
            switch( pHint->Which() )
            {
                case RES_TXTATR_FTN :
                {
                    const XubString aTxt = pHint->GetFtn().GetNumStr();
                    SwDrawTextInfo aDrawInf( pSh, *pOut, 0, aTxt, 0, aTxt.Len() );

                    nProWidth += aIter.GetFnt()->_GetTxtSize( aDrawInf ).Width();
                    break;
                }
                case RES_TXTATR_FIELD :
                {
                    SwField *pFld = (SwField*)pHint->GetFld().GetFld();
                    String const aTxt = pFld->ExpandField(true);
                    SwDrawTextInfo aDrawInf( pSh, *pOut, 0, aTxt, 0, aTxt.Len() );

                    nProWidth += aIter.GetFnt()->_GetTxtSize( aDrawInf ).Width();
                    break;
                }
                default:
                {
                // any suggestions for a default action?
                }
            } // end of switch
            nIdx++;
        } // end of while
    }

    nWidth = std::max( nWidth, nProWidth );

    // search for a text frame this node belongs to
    SwIterator<SwTxtFrm,SwTxtNode> aFrmIter( *this );
    SwTxtFrm* pFrm = 0;
    for( SwTxtFrm* pTmpFrm = aFrmIter.First(); pTmpFrm; pTmpFrm = aFrmIter.Next() )
    {
            if ( pTmpFrm->GetOfst() <= nStt &&
                ( !pTmpFrm->GetFollow() ||
                   pTmpFrm->GetFollow()->GetOfst() > nStt ) )
            {
                pFrm = pTmpFrm;
                break;
            }
        }

    // search for the line containing nStt
    if ( pFrm && pFrm->HasPara() )
    {
        SwTxtInfo aInf( pFrm );
        SwTxtIter aLine( pFrm, &aInf );
        aLine.CharToLine( nStt );
        pOut->SetMapMode( aOldMap );
        return (sal_uInt16)( nWidth ?
            ( ( 100 * aLine.GetCurr()->Height() ) / nWidth ) : 0 );
    }
    // no frame or no paragraph, we take the height of the character
    // at nStt as line height

    aIter.SeekAndChgAttrIter( nStt, pOut );
    pOut->SetMapMode( aOldMap );

    SwDrawTextInfo aDrawInf( pSh, *pOut, 0, GetTxt(), nStt, 1 );
    return (sal_uInt16)
           ( nWidth ? ((100 * aIter.GetFnt()->_GetTxtSize( aDrawInf ).Height()) / nWidth ) : 0 );
}

sal_uInt16 SwTxtNode::GetWidthOfLeadingTabs() const
{
    sal_uInt16 nRet = 0;

    xub_StrLen nIdx = 0;
    sal_Unicode cCh;

    while ( nIdx < GetTxt().getLength() &&
             ( '\t' == ( cCh = GetTxt()[nIdx] ) ||
                ' ' == cCh ) )
        ++nIdx;

    if ( nIdx > 0 )
    {
        SwPosition aPos( *this );
        aPos.nContent += nIdx;

        // Find the non-follow text frame:
        SwIterator<SwTxtFrm,SwTxtNode> aIter( *this );
        for( SwTxtFrm* pFrm = aIter.First(); pFrm; pFrm = aIter.Next() )
        {
            // Only consider master frames:
            if ( !pFrm->IsFollow() )
            {
                SWRECTFN( pFrm )
                SwRect aRect;
                pFrm->GetCharRect( aRect, aPos );
                nRet = (sal_uInt16)
                       ( pFrm->IsRightToLeft() ?
                            (pFrm->*fnRect->fnGetPrtRight)() - (aRect.*fnRect->fnGetRight)() :
                            (aRect.*fnRect->fnGetLeft)() - (pFrm->*fnRect->fnGetPrtLeft)() );
                break;
            }
        }
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
