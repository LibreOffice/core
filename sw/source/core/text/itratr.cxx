/*************************************************************************
 *
 *  $RCSfile: itratr.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-08 10:53:32 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SVX_CHARSCALEITEM_HXX
#include <svx/charscaleitem.hxx>
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>   // ViewShell
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>      // SwField
#endif
#ifndef _PAM_HXX
#include <pam.hxx>         // SwPosition        (lcl_MinMaxNode)
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _ITRATR_HXX
#include <itratr.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif
#ifndef _HTMLTBL_HXX
#include <htmltbl.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _REDLNITR_HXX
#include <redlnitr.hxx>
#endif
#ifndef _FMTSRND_HXX
#include <fmtsrnd.hxx>
#endif
#ifndef _ITRTXT_HXX
#include <itrtxt.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;

/*************************************************************************
 *                      SwAttrIter::Chg()
 *************************************************************************/

void SwAttrIter::Chg( SwTxtAttr *pHt )
{
#ifndef OLD_ATTR_HANDLING
    ASSERT( pHt && pFnt, "No attribute of font available for change");
    if( pRedln && pRedln->IsOn() )
        pRedln->ChangeTxtAttr( pFnt, *pHt, sal_True );
    else
        aAttrHandler.PushAndChg( *pHt, *pFnt );
    nChgCnt++;
#else
    if( pRedln && pRedln->IsOn() )
        pRedln->ChangeTxtAttr( pFnt, *pHt, sal_True );
    else
        pHt->ChgFnt( pFnt );
    nChgCnt++;
#endif
}

/*************************************************************************
 *                      SwAttrIter::Rst()
 *************************************************************************/

void SwAttrIter::Rst( SwTxtAttr *pHt )
{
#ifndef OLD_ATTR_HANDLING
    ASSERT( pHt && pFnt, "No attribute of font available for reset");
    // get top from stack after removing pHt
    if( pRedln && pRedln->IsOn() )
        pRedln->ChangeTxtAttr( pFnt, *pHt, sal_False );
    else
        aAttrHandler.PopAndChg( *pHt, *pFnt );
    nChgCnt--;
#else
    if( pRedln && pRedln->IsOn() )
        pRedln->ChangeTxtAttr( pFnt, *pHt, sal_False );
    else
        pHt->RstFnt( pFnt );
    nChgCnt--;
#endif
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

SwTxtAttr *SwAttrIter::GetAttr( const xub_StrLen nPos ) const
{
    if( pHints )
    {
        for( MSHORT i = 0; i < pHints->Count(); ++i )
        {
            SwTxtAttr *pPos = pHints->GetHt(i);
            xub_StrLen nStart = *pPos->GetStart();
            if( nPos < nStart )
                return 0;
            if( nPos == nStart && !pPos->GetEnd() )
                return pPos;
        }
    }
    return 0;
}

/*************************************************************************
 *                        SwAttrIter::SeekAndChg()
 *************************************************************************/

sal_Bool SwAttrIter::SeekAndChg( const xub_StrLen nNewPos, OutputDevice *pOut )
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
        pFnt->ChgPhysFnt( pShell, pOut );
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

sal_Bool SwAttrIter::SeekStartAndChg( OutputDevice *pOut, const sal_Bool bParaFont )
{
    if ( pRedln && pRedln->ExtOn() )
        pRedln->LeaveExtend( *pFnt, 0 );

    // Gehe zurueck auf Start ...
    pFnt->SetFnt( pAttrSet );
    pFnt->GetTox() = 0;
    pFnt->GetRef() = 0;
    nStartIndex = nEndIndex = nPos = nChgCnt = 0;
    if( nPropFont )
        pFnt->SetProportion( nPropFont );
    if( pRedln )
    {
        pRedln->Clear( pFnt );
        if( !bParaFont )
            nChgCnt += pRedln->Seek( *pFnt, 0, STRING_LEN );
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

    register sal_Bool bChg = pFnt->IsFntChg();
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
        pFnt->ChgPhysFnt( pShell, pOut );
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
        if( !nNewPos || nNewPos < nPos )
        {
            // Gehe zurueck auf Start ...
            if( pRedln )
                pRedln->Clear( NULL );
            pFnt->SetFnt( pAttrSet );
            pFnt->GetTox() = 0;
            pFnt->GetRef() = 0;
            if( nPropFont )
                pFnt->SetProportion( nPropFont );
            nStartIndex = nEndIndex = nPos = 0;
            nChgCnt = 0;
        }
        SeekFwd( nNewPos );
    }
    USHORT nScript = pScriptInfo->ScriptType( nNewPos );
    BYTE nScr = SW_LATIN;
    if( nScript == i18n::ScriptType::ASIAN )
        nScr = SW_CJK;
    else if( nScript == i18n::ScriptType::COMPLEX )
        nScr = SW_CTL;
    pFnt->SetActual( nScr );
    if( pRedln )
        nChgCnt += pRedln->Seek( *pFnt, nNewPos, nPos );
    nPos = nNewPos;
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
        if (pHints->GetStartCount() > nStartIndex) // Gibt es noch Starts?
           nNext = (*pHints->GetStart(nStartIndex)->GetStart());
        if (pHints->GetEndCount() > nEndIndex) // Gibt es noch Enden?
        {
            xub_StrLen nNextEnd = (*pHints->GetEnd(nEndIndex)->GetAnyEnd());
            if ( nNextEnd<nNext ) nNext = nNextEnd; // Wer ist naeher?
        }
    }
    if( pRedln )
        return pRedln->GetNextRedln( nNext );
    return nNext;
}

#ifdef DEBUG
/*************************************************************************
 *                      SwAttrIter::Dump()
 *************************************************************************/

void SwAttrIter::Dump( SvStream &rOS ) const
{
// Noch nicht an den neuen Attributiterator angepasst ...
}

#endif

class SwMinMaxArgs
{
public:
    OutputDevice *pOut;
    ULONG &rMin;
    ULONG &rMax;
    ULONG &rAbsMin;
    long nRowWidth;
    long nWordWidth;
    long nWordAdd;
    SwMinMaxArgs( OutputDevice *pOutI, ULONG& rMinI, ULONG &rMaxI, ULONG &rAbsI )
        : pOut( pOutI ), rMin( rMinI ), rMax( rMaxI ), rAbsMin( rAbsI )
        { nRowWidth = nWordWidth = nWordAdd = 0; }
    void Minimum( long nNew ) { if( (long)rMin < nNew ) rMin = nNew; }
    void NewWord() { nWordAdd = nWordWidth = 0; }
};

sal_Bool lcl_MinMaxString( SwMinMaxArgs& rArg, SwFont* pFnt, const XubString &rTxt,
    xub_StrLen nIdx, xub_StrLen nEnd )
{
    sal_Bool bRet = sal_False;
    while( nIdx < nEnd )
    {
        xub_StrLen nStop = nIdx;
        while( nStop < nEnd && CH_BLANK != rTxt.GetChar( nStop ) )
            ++nStop;
        sal_Bool bClear = nStop == nIdx;
        if ( bClear )
        {
            rArg.NewWord();
            while( nStop < nEnd && CH_BLANK == rTxt.GetChar( nStop ) )
                ++nStop;
        }
        long nAktWidth = pFnt->_GetTxtSize( 0, rArg.pOut, rTxt, nIdx,
                                        nStop - nIdx ).Width();
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

sal_Bool SwTxtNode::IsSymbol( const xub_StrLen nBegin ) const
{
    sal_Bool bRet = sal_False;
    OutputDevice *pOut = GetDoc()->GetPrt();
    if( !pOut )
        pOut = GetpApp()->GetDefaultDevice();
    if( pOut )
    {
        SwScriptInfo aScriptInfo;
        SwAttrIter aIter( *(SwTxtNode*)this, aScriptInfo );
        aIter.SeekAndChg( nBegin, pOut );
        bRet = aIter.GetFnt()->IsSymbol( GetDoc()->GetRootFrm() ?
                GetDoc()->GetRootFrm()->GetCurrShell() : 0 );
    }
    return bRet;
}

class SwMinMaxNodeArgs
{
public:
    ULONG nMaxWidth;    // Summe aller Rahmenbreite
    long nMinWidth;     // Breitester Rahmen
    long nLeftRest;     // noch nicht von Rahmen ueberdeckter Platz im l. Rand
    long nRightRest;    // noch nicht von Rahmen ueberdeckter Platz im r. Rand
    long nLeftDiff;     // Min/Max-Differenz des Rahmens im linken Rand
    long nRightDiff;    // Min/Max-Differenz des Rahmens im rechten Rand
    ULONG nIndx;        // Indexnummer des Nodes
    void Minimum( long nNew ) { if( nNew > nMinWidth ) nMinWidth = nNew; }
};

sal_Bool lcl_MinMaxNode( const SwFrmFmtPtr& rpNd, void* pArgs )
{
    const SwFmtAnchor& rFmtA = ((SwFrmFmt*)rpNd)->GetAnchor();
    if ( (FLY_AT_CNTNT == rFmtA.GetAnchorId() ||
          FLY_AUTO_CNTNT == rFmtA.GetAnchorId()) &&
        ((SwMinMaxNodeArgs*)pArgs)->nIndx ==
            rFmtA.GetCntntAnchor()->nNode.GetIndex() )
    {
        long nMin, nMax;
        SwHTMLTableLayout *pLayout = 0;
        MSHORT nWhich = ((SwFrmFmt*)rpNd)->Which();
        if( RES_DRAWFRMFMT != nWhich )
        {
            // Enthaelt der Rahmen zu Beginn oder am Ende eine Tabelle?
            SwDoc *pDoc = ((SwFrmFmt*)rpNd)->GetDoc();

            const SwFmtCntnt& rFlyCntnt = ((SwFrmFmt*)rpNd)->GetCntnt();
            ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex();
            SwTableNode* pTblNd = pDoc->GetNodes()[nStt+1]->GetTableNode();
            if( !pTblNd )
            {
                SwNode *pNd = pDoc->GetNodes()[nStt];
                pNd = pDoc->GetNodes()[pNd->EndOfSectionIndex()-1];
                if( pNd->IsEndNode() )
                    pTblNd = pNd->StartOfSectionNode()->GetTableNode();
            }

            if( pTblNd )
                pLayout = pTblNd->GetTable().GetHTMLTableLayout();
        }

        const SwFmtHoriOrient& rOrient = ((SwFrmFmt*)rpNd)->GetHoriOrient();
        SwHoriOrient eHoriOri = rOrient.GetHoriOrient();

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
                const SdrObject* pSObj = rpNd->FindSdrObject();
                if( pSObj )
                    nMin = pSObj->GetBoundRect().GetWidth();
                else
                nMin = 0;

            }
            else
            {
                const SwFmtFrmSize &rSz = ( (SwFrmFmt*)rpNd )->GetFrmSize();
                nMin = rSz.GetWidth();
            }
            nMax = nMin;
            nDiff = 0;
        }

        const SvxLRSpaceItem &rLR = ( (SwFrmFmt*)rpNd )->GetLRSpace();
        nMin += rLR.GetLeft();
        nMin += rLR.GetRight();
        nMax += rLR.GetLeft();
        nMax += rLR.GetRight();

        if( SURROUND_THROUGHT == ((SwFrmFmt*)rpNd)->GetSurround().GetSurround() )
        {
            ( (SwMinMaxNodeArgs*)pArgs )->Minimum( nMin );
            return sal_True;
        }

        // Rahmen, die recht bzw. links ausgerichtet sind, gehen nur
        // teilweise in die Max-Berechnung ein, da der Rand schon berueck-
        // sichtigt wird. Nur wenn die Rahmen in den Textkoerper ragen,
        // wird dieser Teil hinzuaddiert.
        switch( eHoriOri )
        {
            case HORI_RIGHT:
            {
                if( nDiff )
                {
                    ((SwMinMaxNodeArgs*)pArgs)->nRightRest -=
                        ((SwMinMaxNodeArgs*)pArgs)->nRightDiff;
                    ((SwMinMaxNodeArgs*)pArgs)->nRightDiff = nDiff;
                }
                if( FRAME!=rOrient.GetRelationOrient() )
                {
                    if( ((SwMinMaxNodeArgs*)pArgs)->nRightRest > 0 )
                        ((SwMinMaxNodeArgs*)pArgs)->nRightRest = 0;
                }
                ((SwMinMaxNodeArgs*)pArgs)->nRightRest -= nMin;
                break;
            }
            case HORI_LEFT:
            {
                if( nDiff )
                {
                    ((SwMinMaxNodeArgs*)pArgs)->nLeftRest -=
                        ((SwMinMaxNodeArgs*)pArgs)->nLeftDiff;
                    ((SwMinMaxNodeArgs*)pArgs)->nLeftDiff = nDiff;
                }
                if( FRAME!=rOrient.GetRelationOrient() &&
                    ((SwMinMaxNodeArgs*)pArgs)->nLeftRest < 0 )
                    ((SwMinMaxNodeArgs*)pArgs)->nLeftRest = 0;
                ((SwMinMaxNodeArgs*)pArgs)->nLeftRest -= nMin;
                break;
            }
            default:
            {
                ( (SwMinMaxNodeArgs*)pArgs )->nMaxWidth += nMax;
                ( (SwMinMaxNodeArgs*)pArgs )->Minimum( nMin );
            }
        }
    }
    return sal_True;
}

#define FLYINCNT_MIN_WIDTH 284

// changing this method very likely requires changing of
// "GetScalingOfSelectedText"
void SwTxtNode::GetMinMaxSize( ULONG nIndex, ULONG& rMin, ULONG &rMax,
                               ULONG& rAbsMin, OutputDevice* pOut ) const
{
    if( !pOut )
    {
        ViewShell* pSh;
        GetDoc()->GetEditShell( &pSh );
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
        SwSpzFrmFmts* pTmp = (SwSpzFrmFmts*)GetDoc()->GetSpzFrmFmts();
        if( pTmp )
        {
            aNodeArgs.nIndx = nIndex;
            pTmp->ForEach( &lcl_MinMaxNode, &aNodeArgs );
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
    aIter.SeekAndChg( nIdx, pOut );
    xub_StrLen nLen = aText.Len();
    long nAktWidth = 0;
    MSHORT nAdd = 0;
    SwMinMaxArgs aArg( pOut, rMin, rMax, rAbsMin );
    while( nIdx < nLen )
    {
        xub_StrLen nNextChg = aIter.GetNextAttr();
        xub_StrLen nStop = aScriptInfo.NextScriptChg( nIdx );
        if( nNextChg > nStop )
            nNextChg = nStop;
        SwTxtAttr *pHint = NULL;
        xub_Unicode cChar = CH_BLANK;
        nStop = nIdx;
        while( nStop < nLen && nStop < nNextChg &&
               CH_TAB != ( cChar = aText.GetChar( nStop ) ) &&
               CH_BREAK != cChar && !pHint )
        {
            if( ( CH_TXTATR_BREAKWORD != cChar && CH_TXTATR_INWORD != cChar )
                || ( 0 == ( pHint = aIter.GetAttr( nStop ) ) ) )
                ++nStop;
        }
        if( lcl_MinMaxString( aArg, aIter.GetFnt(), aText, nIdx, nStop ) )
            nAdd = 20;
        nIdx = nStop;
        aIter.SeekAndChg( nIdx, pOut );
        switch( cChar )
        {
            case CH_BREAK  :
            {
                if( (long)rMax < aArg.nRowWidth )
                    rMax = aArg.nRowWidth;
                aArg.nRowWidth = 0;
                aArg.NewWord();
                aIter.SeekAndChg( ++nIdx, pOut );
            }
            break;
            case CH_TAB    :
            {
                aArg.NewWord();
                aIter.SeekAndChg( ++nIdx, pOut );
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
                                nAktWidth = pSObj->GetBoundRect().GetWidth();
                            else
                                nAktWidth = 0;
                        }
                        else
                        {
                            const SwFmtFrmSize& rTmpSize = pFrmFmt->GetFrmSize();
                            if( RES_FLYFRMFMT == pFrmFmt->Which()
                                && rTmpSize.GetWidthPercent() )
                            {
/*-----------------24.01.97 14:09----------------------------------------------
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
                        const String aTxt = pFld->GetCntnt( FALSE );
                        if( lcl_MinMaxString( aArg, aIter.GetFnt(), aTxt, 0,
                            aTxt.Len() ) )
                            nAdd = 20;
                        break;
                    }
                    case RES_TXTATR_HARDBLANK :
                    {
                        XubString sTmp( pHint->GetHardBlank().GetChar() );
                        nAktWidth = aIter.GetFnt()->_GetTxtSize(
                                GetDoc()->GetRootFrm() ? GetDoc()->GetRootFrm()->GetCurrShell() : 0,
                                pOut, sTmp, 0, 1 ).Width();
                        aArg.nWordWidth = nOldWidth + nAktWidth;
                        aArg.nWordAdd = nOldAdd;
                        aArg.nRowWidth += nAktWidth;
                        if( (long)rAbsMin < aArg.nWordWidth )
                            rAbsMin = aArg.nWordWidth;
                        aArg.Minimum( aArg.nWordWidth + aArg.nWordAdd );
                        break;
                    }
                    default: aArg.nWordWidth = nOldWidth;
                             aArg.nWordAdd = nOldAdd;

                }
                aIter.SeekAndChg( ++nIdx, pOut );
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

USHORT SwTxtNode::GetScalingOfSelectedText( xub_StrLen nStt, xub_StrLen nEnd )
    const
{
    ViewShell* pSh;
    OutputDevice* pOut;
    GetDoc()->GetEditShell( &pSh );
    pOut = GetDoc()->GetPrt();
    if( !pOut || !((Printer*)pOut)->IsValid() )
    {
        if( pSh )
        {
            if( 0 == ( pOut = pSh->GetReferenzDevice() ) )
                pOut = pSh->GetWin();
        }
        if( !pOut )
            pOut = GetpApp()->GetDefaultDevice();
    }

    MapMode aOldMap( pOut->GetMapMode() );
    pOut->SetMapMode( MapMode( MAP_TWIP ) );

    if ( nStt == nEnd )
    {
        if ( !pBreakIt->xBreak.is() )
            return 100;

        SwScriptInfo aScriptInfo;
        SwAttrIter aIter( *(SwTxtNode*)this, aScriptInfo );
        aIter.SeekAndChg( nStt, pOut );

        Boundary aBound =
            pBreakIt->xBreak->getWordBoundary( GetTxt(), nStt,
            pBreakIt->GetLocale( aIter.GetFnt()->GetLanguage() ),
            WordType::DICTIONARY_WORD, sal_True );

        if ( nStt == aBound.startPos )
            // cursor is at left or right border of word
            return 100;

        nStt = aBound.startPos;
        nEnd = aBound.endPos;

        if ( nStt == nEnd )
            return 100;
    }

    SwScriptInfo aScriptInfo;
    SwAttrIter aIter( *(SwTxtNode*)this, aScriptInfo );

#ifndef OLD_ATTR_HANDLING
    // We do not want scaling attributes to be considered during this
    // calculation. For this, we push a temporary scaling attribute with
    // scaling value 100 and priority flag on top of the scaling stack
    SwAttrHandler& rAH = aIter.GetAttrHandler();
    SvxCharScaleWidthItem aItem;
    SwTxtCharScaleWidth aAttr( aItem, nStt, nEnd );
    aAttr.SetPriorityAttr( sal_True );
    rAH.PushAndChg( aAttr, *(aIter.GetFnt()) );
#endif

    xub_StrLen nIdx = nStt;

    ULONG nWidth = 0;
    ULONG nProWidth = 0;

    while( nIdx < nEnd )
    {
        aIter.SeekAndChg( nIdx, pOut );

        // scan for end of portion
        xub_StrLen nNextChg = aIter.GetNextAttr();
        xub_StrLen nStop = aScriptInfo.NextScriptChg( nIdx );
        if( nNextChg > nStop )
            nNextChg = nStop;

        nStop = nIdx;
        xub_Unicode cChar = CH_BLANK;
        SwTxtAttr* pHint = NULL;

        // stop at special characters in [ nIdx, nNextChg ]
        while( nStop < nEnd && nStop < nNextChg )
        {
            cChar = aText.GetChar( nStop );
            if( CH_TAB == cChar || CH_BREAK == cChar ||
               ( CH_TXTATR_BREAKWORD == cChar ||  CH_TXTATR_INWORD == cChar ) &&
               ( 0 == ( pHint = aIter.GetAttr( nStop ) ) ) )
                break;
            else
                ++nStop;
        }

        // calculate text widths up to cChar
        if ( nStop > nIdx )
            nProWidth += aIter.GetFnt()->_GetTxtSize( 0, pOut, GetTxt(), nIdx,
                                                        nStop - nIdx ).Width();

        nIdx = nStop;
        aIter.SeekAndChg( nIdx, pOut );

        if ( cChar == CH_BREAK )
        {
            nWidth = Max( nWidth, nProWidth );
            nProWidth = 0;
            nIdx++;
        }
        else if ( cChar == CH_TAB )
        {
            // tab receives width of one space
            nProWidth += aIter.GetFnt()->_GetTxtSize( 0, pOut, ' ', 0, 1 ).Width();
            nIdx++;
        }
        else if ( pHint && ( cChar == CH_TXTATR_BREAKWORD || CH_TXTATR_INWORD ) )
        {
            switch( pHint->Which() )
            {
                case RES_TXTATR_FTN :
                {
                    const XubString aTxt = pHint->GetFtn().GetNumStr();
                    nProWidth += aIter.GetFnt()->_GetTxtSize( 0, pOut, aTxt, 0,
                                        aTxt.Len() ).Width();
                    break;
                }
                case RES_TXTATR_FIELD :
                {
                    SwField *pFld = (SwField*)pHint->GetFld().GetFld();
                    const String aTxt = pFld->GetCntnt( FALSE );
                    nProWidth += aIter.GetFnt()->_GetTxtSize( 0, pOut, aTxt, 0,
                                        aTxt.Len() ).Width();
                    break;
                }
                case RES_TXTATR_HARDBLANK :
                {
                    XubString aTxt( pHint->GetHardBlank().GetChar() );
                    nProWidth += aIter.GetFnt()->_GetTxtSize(
                            GetDoc()->GetRootFrm() ? GetDoc()->GetRootFrm()->GetCurrShell() : 0,
                            pOut, aTxt, 0, 1 ).Width();
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

    nWidth = Max( nWidth, nProWidth );

    // search for a text frame this node belongs to
    SwClientIter aClientIter( *(SwTxtNode*)this );
    SwClient* pLast = aClientIter.GoStart();
    SwTxtFrm* pFrm;

    while( pLast )
    {
        if ( pLast->ISA( SwTxtFrm ) )
        {
            SwTxtFrm* pTmpFrm = ( SwTxtFrm* )pLast;
            if ( pTmpFrm->GetOfst() <= nStt &&
                ( !pTmpFrm->GetFollow() ||
                   pTmpFrm->GetFollow()->GetOfst() > nStt ) )
            {
                pFrm = pTmpFrm;
                break;
            }
        }
        pLast = ++aClientIter;
    }

    // search for the line containing nStt
    if ( pFrm && pFrm->HasPara() )
    {
        SwTxtInfo aInf( pFrm );
        SwTxtIter aLine( pFrm, &aInf );
        aLine.CharToLine( nStt );
        pOut->SetMapMode( aOldMap );
        return (USHORT)( nWidth ?
            ( ( 100 * aLine.GetCurr()->Height() ) / nWidth ) : 0 );
    }
    // no frame or no paragraph, we take the height of the character
    // at nStt as line height

    aIter.SeekAndChg( nStt, pOut );
    pOut->SetMapMode( aOldMap );
    return (USHORT)( nWidth ? ( ( 100 * aIter.GetFnt()->_GetTxtSize( 0, pOut,
            GetTxt(), nStt, 1 ).Height() ) / nWidth ) : 0 );
}

