/*************************************************************************
 *
 *  $RCSfile: doctxm.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-20 10:55:55 $
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

#include <limits.h>

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _APP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#define _SVSTDARR_STRINGSSORT
#include <svtools/svstdarr.hxx>
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif
#ifndef SMDLL0_HXX
#include <starmath/smdll0.hxx>
#endif
#ifndef SC_SCDLL_HXX
#include <sc/scdll.hxx>
#endif
#ifndef _SCHDLL0_HXX
#include <sch/schdll0.hxx>
#endif
#ifndef _SDDLL_HXX
#include <sd/sddll.hxx>
#endif

#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _TXTTXMRK_HXX
#include <txttxmrk.hxx>
#endif
#ifndef _FMTINFMT_HXX
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTPDSC_HXX
#include <fmtpdsc.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FMTFSIZE_HXX
#include <fmtfsize.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>
#endif
#ifndef _TXMSRT_HXX
#include <txmsrt.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _FCHRFMT_HXX
#include <fchrfmt.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX
#include <txtfld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _CHPFLD_HXX
#include <chpfld.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _NODE2LAY_HXX
#include <node2lay.hxx>
#endif


const sal_Unicode cNumRepl      = '@';
const sal_Unicode cEndPageNum   = '~';
const sal_Char __FAR_DATA sPageDeli[] = ", ";

SV_IMPL_PTRARR(SwTOXSortTabBases, SwTOXSortTabBasePtr)

TYPEINIT2( SwTOXBaseSection, SwTOXBase, SwSection );    // fuers RTTI

struct LinkStruct
{
    SwFmtINetFmt    aINetFmt;
    xub_StrLen nStartTextPos, nEndTextPos;

    LinkStruct( const String& rURL, xub_StrLen nStart, xub_StrLen nEnd )
        : aINetFmt( rURL, aEmptyStr),
        nStartTextPos( nStart),
        nEndTextPos(nEnd) {}
};

typedef LinkStruct* LinkStructPtr;
SV_DECL_PTRARR(LinkStructArr, LinkStructPtr, 0, 5 );
SV_IMPL_PTRARR(LinkStructArr, LinkStructPtr)

USHORT SwDoc::GetTOIKeys( SwTOIKeyType eTyp, SvStringsSort& rArr ) const
{
    if( rArr.Count() )
        rArr.Remove( USHORT(0), rArr.Count() );

    // dann mal ueber den Pool und alle Primary oder Secondary heraussuchen
    const SwTxtTOXMark* pMark;
    const SfxPoolItem* pItem;
    USHORT i, nMaxItems = GetAttrPool().GetItemCount( RES_TXTATR_TOXMARK );
    for( i = 0; i < nMaxItems; ++i )
        if( 0 != (pItem = GetAttrPool().GetItem( RES_TXTATR_TOXMARK, i ) ) &&
            TOX_INDEX == ((SwTOXMark*)pItem)->GetTOXType()->GetType() &&
            0 != ( pMark = ((SwTOXMark*)pItem)->GetTxtTOXMark() ) &&
            pMark->GetpTxtNd() &&
            pMark->GetpTxtNd()->GetNodes().IsDocNodes() )
        {
            const String* pStr;
            if( TOI_PRIMARY == eTyp )
                pStr = &((SwTOXMark*)pItem)->GetPrimaryKey();
            else
                pStr = &((SwTOXMark*)pItem)->GetSecondaryKey();

            if( pStr->Len() )
                rArr.Insert( (StringPtr)pStr );
        }

    return rArr.Count();
}

/*--------------------------------------------------------------------
     Beschreibung: aktuelle Verzeichnismarkierungen ermitteln
 --------------------------------------------------------------------*/


USHORT SwDoc::GetCurTOXMark( const SwPosition& rPos,
                                SwTOXMarks& rArr ) const
{
    // suche an der Position rPos nach allen SwTOXMark's
    SwTxtNode* pTxtNd = GetNodes()[ rPos.nNode ]->GetTxtNode();
    // kein TextNode oder kein HintsArray vorhanden ??
    if( !pTxtNd || !pTxtNd->GetpSwpHints() )
        return 0;

    const SwpHints & rHts = *pTxtNd->GetpSwpHints();
    const SwTxtAttr* pHt;
    xub_StrLen nSttIdx;
    const xub_StrLen *pEndIdx;

    xub_StrLen nAktPos = rPos.nContent.GetIndex();

    for( USHORT n = 0; n < rHts.Count(); ++n )
    {
        if( RES_TXTATR_TOXMARK != (pHt = rHts[n])->Which() )
            continue;
        if( ( nSttIdx = *pHt->GetStart() ) < nAktPos )
        {
            // pruefe Ende mit ab
            if( 0 == ( pEndIdx = pHt->GetEnd() ) ||
                *pEndIdx <= nAktPos )
                continue;       // weiter suchen
        }
        else if( nSttIdx > nAktPos )
            // ist Start vom Hint groesser als rPos, dann abbrechen. Denn
            // die Attribute sind nach Start sortiert !
            break;

        const SwTOXMark* pTMark = &pHt->GetTOXMark();
        rArr.Insert( pTMark, rArr.Count() );
    }
    return rArr.Count();
}

/*--------------------------------------------------------------------
     Beschreibung: Marke loeschen
 --------------------------------------------------------------------*/

void SwDoc::Delete( SwTOXMark* pTOXMark )
{
    // hole den TextNode und
    SwTxtTOXMark* pTxtTOXMark = pTOXMark->GetTxtTOXMark();
    ASSERT( pTxtTOXMark, "Kein TxtTOXMark, kann nicht geloescht werden" );

    SwTxtNode& rTxtNd = (SwTxtNode&)pTxtTOXMark->GetTxtNode();
    ASSERT( rTxtNd.GetpSwpHints(), "kann nicht geloescht werden" );

    if( DoesUndo() )
    {
        // fuers Undo die Attribute sichern
        ClearRedo();
        SwUndoRstAttr* pUndo = new SwUndoRstAttr( *this, SwPosition( rTxtNd,
                            SwIndex( &rTxtNd, *pTxtTOXMark->GetStart() ) ),
                                    RES_TXTATR_TOXMARK );
        AppendUndo( pUndo );

        SwRegHistory aRHst( rTxtNd, pUndo->GetHistory() );
        rTxtNd.GetpSwpHints()->Register( &aRHst );
        rTxtNd.Delete( pTxtTOXMark, TRUE );
        if( rTxtNd.GetpSwpHints() )
            rTxtNd.GetpSwpHints()->DeRegister();
    }
    else
        rTxtNd.Delete( pTxtTOXMark, TRUE );
    SetModified();
}

/*--------------------------------------------------------------------
     Beschreibung: Traveln zwischen TOXMarks
 --------------------------------------------------------------------*/

class CompareNodeCntnt
{
    ULONG nNode;
    xub_StrLen nCntnt;
public:
    CompareNodeCntnt( ULONG nNd, xub_StrLen nCnt )
        : nNode( nNd ), nCntnt( nCnt ) {}

    int operator==( const CompareNodeCntnt& rCmp )
        { return nNode == rCmp.nNode && nCntnt == rCmp.nCntnt; }
    int operator!=( const CompareNodeCntnt& rCmp )
        { return nNode != rCmp.nNode || nCntnt != rCmp.nCntnt; }
    int operator< ( const CompareNodeCntnt& rCmp )
        { return nNode < rCmp.nNode ||
            ( nNode == rCmp.nNode && nCntnt < rCmp.nCntnt); }
    int operator<=( const CompareNodeCntnt& rCmp )
        { return nNode < rCmp.nNode ||
            ( nNode == rCmp.nNode && nCntnt <= rCmp.nCntnt); }
    int operator> ( const CompareNodeCntnt& rCmp )
        { return nNode > rCmp.nNode ||
            ( nNode == rCmp.nNode && nCntnt > rCmp.nCntnt); }
    int operator>=( const CompareNodeCntnt& rCmp )
        { return nNode > rCmp.nNode ||
            ( nNode == rCmp.nNode && nCntnt >= rCmp.nCntnt); }
};

const SwTOXMark& SwDoc::GotoTOXMark( const SwTOXMark& rCurTOXMark,
                                    SwTOXSearch eDir, BOOL bInReadOnly )
{
    const SwTxtTOXMark* pMark = rCurTOXMark.GetTxtTOXMark();
    ASSERT(pMark, "pMark==0 Ungueltige TxtTOXMark");

    const SwTxtNode *pTOXSrc = pMark->GetpTxtNd();

    CompareNodeCntnt aAbsIdx( pTOXSrc->GetIndex(), *pMark->GetStart() );
    CompareNodeCntnt aPrevPos( 0, 0 );
    CompareNodeCntnt aNextPos( ULONG_MAX, STRING_NOTFOUND );
    CompareNodeCntnt aMax( 0, 0 );
    CompareNodeCntnt aMin( ULONG_MAX, STRING_NOTFOUND );

    const SwTOXMark*    pNew    = 0;
    const SwTOXMark*    pMax    = &rCurTOXMark;
    const SwTOXMark*    pMin    = &rCurTOXMark;

    const SwModify* pType = rCurTOXMark.GetRegisteredIn();
    SwClientIter    aIter( *(SwModify*)pType );

    const SwTOXMark* pTOXMark;
    const SwCntntFrm* pCFrm;
    Point aPt;
    for( pTOXMark = (SwTOXMark*)aIter.First( TYPE( SwTOXMark )); pTOXMark;
         pTOXMark = (SwTOXMark*)aIter.Next() )
    {
        if( pTOXMark != &rCurTOXMark &&
            0 != ( pMark = pTOXMark->GetTxtTOXMark()) &&
            0 != ( pTOXSrc = pMark->GetpTxtNd() ) &&
            0 != ( pCFrm = pTOXSrc->GetFrm( &aPt, 0, FALSE )) &&
            ( bInReadOnly || !pCFrm->IsProtected() ))
        {
            CompareNodeCntnt aAbsNew( pTOXSrc->GetIndex(), *pMark->GetStart() );
            switch( eDir )
            {
                //Die untenstehenden etwas komplizierter ausgefallen Ausdruecke
                //dienen dazu auch ueber Eintraege auf der selben (!) Position
                //traveln zu koennen. Wenn einer Zeit hat mag er sie mal
                //optimieren.

            case TOX_SAME_PRV:
                if( pTOXMark->GetText() != rCurTOXMark.GetText() )
                    break;
                /* no break here */
            case TOX_PRV:
                if ( (aAbsNew < aAbsIdx && aAbsNew > aPrevPos &&
                      aPrevPos != aAbsIdx && aAbsNew != aAbsIdx ) ||
                     (aAbsIdx == aAbsNew &&
                      (ULONG(&rCurTOXMark) > ULONG(pTOXMark) &&
                       (!pNew ||
                        (pNew && (aPrevPos < aAbsIdx ||
                                  ULONG(pNew) < ULONG(pTOXMark)))))) ||
                     (aPrevPos == aAbsNew && aAbsIdx != aAbsNew &&
                      ULONG(pTOXMark) > ULONG(pNew)) )
                {
                    pNew = pTOXMark;
                    aPrevPos = aAbsNew;
                    if ( aAbsNew >= aMax )
                    {
                        aMax = aAbsNew;
                        pMax = pTOXMark;
                    }
                }
                break;

            case TOX_SAME_NXT:
                if( pTOXMark->GetText() != rCurTOXMark.GetText() )
                    break;
                /* no break here */
            case TOX_NXT:
                if ( (aAbsNew > aAbsIdx && aAbsNew < aNextPos &&
                      aNextPos != aAbsIdx && aAbsNew != aAbsIdx ) ||
                     (aAbsIdx == aAbsNew &&
                      (ULONG(&rCurTOXMark) < ULONG(pTOXMark) &&
                       (!pNew ||
                        (pNew && (aNextPos > aAbsIdx ||
                                  ULONG(pNew) > ULONG(pTOXMark)))))) ||
                     (aNextPos == aAbsNew && aAbsIdx != aAbsNew &&
                      ULONG(pTOXMark) < ULONG(pNew)) )
                {
                    pNew = pTOXMark;
                    aNextPos = aAbsNew;
                    if ( aAbsNew <= aMin )
                    {
                        aMin = aAbsNew;
                        pMin = pTOXMark;
                    }
                }
                break;
            }
        }
    }


    // kein Nachfolger wurde gefunden
    // Min oder Max benutzen
    if(!pNew)
    {
        switch(eDir)
        {
        case TOX_PRV:
        case TOX_SAME_PRV:
            pNew = pMax;
            break;
        case TOX_NXT:
        case TOX_SAME_NXT:
            pNew = pMin;
            break;
        default:
            pNew = &rCurTOXMark;
        }
    }
    return *pNew;
}

/*  */

const SwTOXBaseSection* SwDoc::InsertTableOf( const SwPosition& rPos,
                                                const SwTOXBase& rTOX,
                                                const SfxItemSet* pSet,
                                                BOOL bExpand )
{
    StartUndo( UNDO_INSTOX );

    SwTOXBaseSection* pNew = new SwTOXBaseSection( rTOX );
    String sSectNm( rTOX.GetTOXName() );
    sSectNm = GetUniqueTOXBaseName( *rTOX.GetTOXType(), &sSectNm );
    pNew->SetTOXName(sSectNm);
    pNew->SwSection::SetName(sSectNm);
    SwPaM aPam( rPos );
    SwSection* pSect = Insert( aPam, *pNew, pSet, FALSE );
    if( pSect )
    {
        SwSectionNode* pSectNd = pSect->GetFmt()->GetSectionNode();
        SwSection* pCl = pNew;
        pSect->GetFmt()->Add( pCl );
        pSectNd->SetNewSection( pNew );

        if( bExpand )
            pNew->Update();
    }
    else
        delete pNew, pNew = 0;

    EndUndo( UNDO_INSTOX );

    return pNew;
}



const SwTOXBaseSection* SwDoc::InsertTableOf( ULONG nSttNd, ULONG nEndNd,
                                                const SwTOXBase& rTOX,
                                                const SfxItemSet* pSet )
{
    // check for recursiv TOX
    SwNode* pNd = GetNodes()[ nSttNd ];
    SwSectionNode* pSectNd = pNd->FindSectionNode();
    while( pSectNd )
    {
        SectionType eT = pSectNd->GetSection().GetType();
        if( TOX_HEADER_SECTION == eT || TOX_CONTENT_SECTION == eT )
            return 0;
        pSectNd = pSectNd->FindStartNode()->FindSectionNode();
    }

    // create SectionNode around the Nodes
    SwTOXBaseSection* pNew = new SwTOXBaseSection( rTOX );

    String sSectNm( rTOX.GetTOXName() );
    sSectNm = GetUniqueTOXBaseName(*rTOX.GetTOXType(), &sSectNm);
    pNew->SetTOXName(sSectNm);
    pNew->SwSection::SetName(sSectNm);

    SwNodeIndex aStt( GetNodes(), nSttNd ), aEnd( GetNodes(), nEndNd );
    SwSectionFmt* pFmt = MakeSectionFmt( 0 );
    if(pSet)
        pFmt->SetAttr(*pSet);

//  --aEnd;     // im InsertSection ist Ende inclusive

    pSectNd = GetNodes().InsertSection( aStt, *pFmt, *pNew, &aEnd );
    if( pSectNd )
    {
        SwSection* pCl = pNew;
        pFmt->Add( pCl );
        pSectNd->SetNewSection( pNew );
    }
    else
    {
        delete pNew, pNew = 0;
        DelSectionFmt( pFmt );
    }

    return pNew;
}

/*--------------------------------------------------------------------
     Beschreibung: Aktuelles Verzeichnis ermitteln
 --------------------------------------------------------------------*/

const SwTOXBase* SwDoc::GetCurTOX( const SwPosition& rPos ) const
{
    const SwNode& rNd = rPos.nNode.GetNode();
    const SwSectionNode* pSectNd = rNd.FindSectionNode();
    while( pSectNd )
    {
        SectionType eT = pSectNd->GetSection().GetType();
        if( TOX_CONTENT_SECTION == eT )
        {
            ASSERT( pSectNd->GetSection().ISA( SwTOXBaseSection ),
                    "keine TOXBaseSection!" );
            SwTOXBaseSection& rTOXSect = (SwTOXBaseSection&)
                                                pSectNd->GetSection();
            return &rTOXSect;
        }
        pSectNd = pSectNd->FindStartNode()->FindSectionNode();
    }
    return 0;
}
/* -----------------01.09.99 16:01-------------------

 --------------------------------------------------*/
const SwAttrSet& SwDoc::GetTOXBaseAttrSet(const SwTOXBase& rTOXBase) const
{
    ASSERT( rTOXBase.ISA( SwTOXBaseSection ), "no TOXBaseSection!" );
    const SwTOXBaseSection& rTOXSect = (const SwTOXBaseSection&)rTOXBase;
    SwSectionFmt* pFmt = rTOXSect.GetFmt();
    ASSERT( pFmt, "invalid TOXBaseSection!" );
    return pFmt->GetAttrSet();
}
/* -----------------02.09.99 07:48-------------------

 --------------------------------------------------*/
const SwTOXBase* SwDoc::GetDefaultTOXBase( TOXTypes eTyp, BOOL bCreate )
{
    SwTOXBase** prBase;
    switch(eTyp)
    {
    case  TOX_CONTENT:          prBase = &pDefTOXBases->pContBase; break;
    case  TOX_INDEX:            prBase = &pDefTOXBases->pIdxBase;  break;
    case  TOX_USER:             prBase = &pDefTOXBases->pUserBase; break;
    case  TOX_TABLES:           prBase = &pDefTOXBases->pTblBase;  break;
    case  TOX_OBJECTS:          prBase = &pDefTOXBases->pObjBase;  break;
    case  TOX_ILLUSTRATIONS:    prBase = &pDefTOXBases->pIllBase;  break;
    case  TOX_AUTHORITIES:      prBase = &pDefTOXBases->pAuthBase; break;
    }
    if(!(*prBase) && bCreate)
    {
        SwForm aForm(eTyp);
        const SwTOXType* pType = GetTOXType(eTyp, 0);
        (*prBase) = new SwTOXBase(pType, aForm, 0, pType->GetTypeName());
    }
    return (*prBase);
}
/* -----------------02.09.99 08:06-------------------

 --------------------------------------------------*/
void    SwDoc::SetDefaultTOXBase(const SwTOXBase& rBase)
{
    SwTOXBase** prBase;
    switch(rBase.GetType())
    {
    case  TOX_CONTENT:          prBase = &pDefTOXBases->pContBase; break;
    case  TOX_INDEX:            prBase = &pDefTOXBases->pIdxBase;  break;
    case  TOX_USER:             prBase = &pDefTOXBases->pUserBase; break;
    case  TOX_TABLES:           prBase = &pDefTOXBases->pTblBase;  break;
    case  TOX_OBJECTS:          prBase = &pDefTOXBases->pObjBase;  break;
    case  TOX_ILLUSTRATIONS:    prBase = &pDefTOXBases->pIllBase;  break;
    case  TOX_AUTHORITIES:      prBase = &pDefTOXBases->pAuthBase; break;
    }
    if(*prBase)
        delete (*prBase);
    (*prBase) = new SwTOXBase(rBase);
}

/*--------------------------------------------------------------------
     Beschreibung: Verzeichnis loeschen
 --------------------------------------------------------------------*/


BOOL SwDoc::DeleteTOX( const SwTOXBase& rTOXBase, BOOL bDelNodes )
{
    // its only delete the TOX, not the nodes
    BOOL bRet = FALSE;
    ASSERT( rTOXBase.ISA( SwTOXBaseSection ), "keine TOXBaseSection!" );

    const SwTOXBaseSection& rTOXSect = (const SwTOXBaseSection&)rTOXBase;
    SwSectionFmt* pFmt = rTOXSect.GetFmt();
    if( pFmt )
    {
        StartUndo( UNDO_CLEARTOXRANGE );

        if( !bDelNodes )
        {
            SwSections aArr( 0, 4 );
            USHORT nCnt = pFmt->GetChildSections( aArr, SORTSECT_NOT, FALSE );
            for( USHORT n = 0; n < nCnt; ++n )
            {
                SwSection* pSect = aArr[ n ];
                if( TOX_HEADER_SECTION == pSect->GetType() )
                {
                    DelSectionFmt( pSect->GetFmt(), bDelNodes );
                }
            }
        }
        DelSectionFmt( pFmt, bDelNodes );

        EndUndo( UNDO_CLEARTOXRANGE );
        bRet = TRUE;
    }
    return bRet;
}

/*--------------------------------------------------------------------
     Beschreibung:  Verzeichnistypen verwalten
 --------------------------------------------------------------------*/

USHORT SwDoc::GetTOXTypeCount(TOXTypes eTyp) const
{
    const SwTOXTypePtr * ppTTypes = pTOXTypes->GetData();
    USHORT nCnt = 0;
    for( USHORT n = 0; n < pTOXTypes->Count(); ++n, ++ppTTypes )
        if( eTyp == (*ppTTypes)->GetType() )
            ++nCnt;
    return nCnt;
}
/*--------------------------------------------------------------------

 --------------------------------------------------------------------*/
const SwTOXType* SwDoc::GetTOXType( TOXTypes eTyp, USHORT nId ) const
{
    const SwTOXTypePtr * ppTTypes = pTOXTypes->GetData();
    USHORT nCnt = 0;
    for( USHORT n = 0; n < pTOXTypes->Count(); ++n, ++ppTTypes )
        if( eTyp == (*ppTTypes)->GetType() && nCnt++ == nId )
            return (*ppTTypes);
    return 0;
}

#ifdef USED


BOOL SwDoc::DeleteTOXType(TOXTypes eTyp, USHORT nId)
{
    // was passiert mit den Abhaengigen ??
    //  - alle Marken und alle Verzeichnisse aus dem Text loeschen ??
    SwTOXType* pTOXTyp = (SwTOXType*)GetTOXType( eTyp, nId );

    ASSERT( !pTOXTyp->GetDepends(), "noch Marken/Verzeichnisse am Typ" );
    delete pTOXTyp;     // ???

    return TRUE;
}

#endif

/*--------------------------------------------------------------------

 --------------------------------------------------------------------*/
const SwTOXType* SwDoc::InsertTOXType( const SwTOXType& rTyp )
{
    SwTOXType * pNew = new SwTOXType( rTyp );
    pTOXTypes->Insert( pNew, pTOXTypes->Count() );
    return pNew;
}
/*--------------------------------------------------------------------

 --------------------------------------------------------------------*/
String SwDoc::GetUniqueTOXBaseName( const SwTOXType& rType,
                                    const String* pChkStr ) const
{
    if(pChkStr && !pChkStr->Len())
        pChkStr = 0;
    String aName( rType.GetTypeName() );
    xub_StrLen nNmLen = aName.Len();

    USHORT nNum, nTmp, nFlagSize = ( pSectionFmtTbl->Count() / 8 ) +2;
    BYTE* pSetFlags = new BYTE[ nFlagSize ];
    memset( pSetFlags, 0, nFlagSize );

    const SwSectionNode* pSectNd;
    const SwSection* pSect;
    for( USHORT n = 0; n < pSectionFmtTbl->Count(); ++n )
        if( 0 != ( pSectNd = (*pSectionFmtTbl)[ n ]->GetSectionNode( FALSE ) )&&
             TOX_CONTENT_SECTION == (pSect = &pSectNd->GetSection())->GetType())
        {
            const String& rNm = pSect->GetName();
            if( rNm.Match( aName ) == nNmLen )
            {
                // Nummer bestimmen und das Flag setzen
                nNum = rNm.Copy( nNmLen ).ToInt32();
                if( nNum-- && nNum < pSectionFmtTbl->Count() )
                    pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
            }
            if( pChkStr && pChkStr->Equals( rNm ) )
                pChkStr = 0;
        }

    if( !pChkStr )
    {
        // alle Nummern entsprechend geflag, also bestimme die richtige Nummer
        nNum = pSectionFmtTbl->Count();
        for( n = 0; n < nFlagSize; ++n )
            if( 0xff != ( nTmp = pSetFlags[ n ] ))
            {
                // also die Nummer bestimmen
                nNum = n * 8;
                while( nTmp & 1 )
                    ++nNum, nTmp >>= 1;
                break;
            }
    }
    __DELETE( nFlagSize ) pSetFlags;
    if( pChkStr )
        return *pChkStr;
    return aName += String::CreateFromInt32( ++nNum );
}

/*--------------------------------------------------------------------

 --------------------------------------------------------------------*/
BOOL SwDoc::SetTOXBaseName(const SwTOXBase& rTOXBase, const String& rName)
{
    ASSERT( rTOXBase.ISA( SwTOXBaseSection ),
                    "keine TOXBaseSection!" );
    SwTOXBaseSection* pTOX = (SwTOXBaseSection*)&rTOXBase;

    String sTmp = GetUniqueTOXBaseName(*rTOXBase.GetTOXType(), &rName);
    BOOL bRet = sTmp == rName;
    if(bRet)
    {
        pTOX->SetTOXName(rName);
        pTOX->SwTOXBaseSection::SetName(rName);
        SetModified();
    }
    return bRet;
}
/*--------------------------------------------------------------------

 --------------------------------------------------------------------*/
void SwDoc::SetTOXBaseProtection(const SwTOXBase& rTOXBase, BOOL bProtect)
{
    ASSERT( rTOXBase.ISA( SwTOXBaseSection ),
                    "keine TOXBaseSection!" );
    SwTOXBaseSection* pTOX = (SwTOXBaseSection*)&rTOXBase;
    if(bProtect != pTOX->IsProtected())
    {
        pTOX->SetProtect(bProtect);
        pTOX->SetProtected(bProtect);
        SetModified();
    }
}


/*  */

const SwTxtNode* lcl_FindChapterNode( const SwNode& rNd, BYTE nLvl = 0 )
{
    const SwNode* pNd = &rNd;
    if( pNd->GetNodes().GetEndOfExtras().GetIndex() > pNd->GetIndex() )
    {
        // then find the "Anchor" (Body) position
        Point aPt;
        SwNode2Layout aNode2Layout( *pNd, pNd->GetIndex() );
        const SwFrm* pFrm = aNode2Layout.GetFrm( &aPt, 0, FALSE );

        if( pFrm )
        {
            SwPosition aPos( *pNd );
            pNd = GetBodyTxtNode( *pNd->GetDoc(), aPos, *pFrm );
            ASSERT( pNd,    "wo steht der Absatz" );
        }
    }
    return pNd ? pNd->FindOutlineNodeOfLevel( nLvl ) : 0;
}


/*--------------------------------------------------------------------
     Beschreibung: Verzeichnis-Klasse
 --------------------------------------------------------------------*/

SwTOXBaseSection::SwTOXBaseSection( const SwTOXBase& rBase )
    : SwTOXBase( rBase ), SwSection( TOX_CONTENT_SECTION, aEmptyStr )
{
    SetProtect( rBase.IsProtected() );
    SwSection::SetName( GetTOXName() );
}


SwTOXBaseSection::~SwTOXBaseSection()
{
}


BOOL SwTOXBaseSection::SetPosAtStartEnd( SwPosition& rPos, BOOL bAtStart ) const
{
    BOOL bRet = FALSE;
    const SwSectionNode* pSectNd = GetFmt()->GetSectionNode();
    if( pSectNd )
    {
        SwCntntNode* pCNd;
        xub_StrLen nC = 0;
        if( bAtStart )
        {
            rPos.nNode = *pSectNd;
            pCNd = pSectNd->GetDoc()->GetNodes().GoNext( &rPos.nNode );
        }
        else
        {
            rPos.nNode = *pSectNd->EndOfSectionNode();
            pCNd = pSectNd->GetDoc()->GetNodes().GoPrevious( &rPos.nNode );
            if( pCNd ) nC = pCNd->Len();
        }
        rPos.nContent.Assign( pCNd, nC );
        bRet = TRUE;
    }
    return bRet;
}

/*--------------------------------------------------------------------
     Beschreibung: Verzeichnisinhalt zusammensammeln
 --------------------------------------------------------------------*/

void SwTOXBaseSection::Update(const SfxItemSet* pAttr)
{
    const SwSectionNode* pSectNd;
    if( !SwTOXBase::GetRegisteredIn()->GetDepends() ||
        !GetFmt() || 0 == (pSectNd = GetFmt()->GetSectionNode() ) ||
        !pSectNd->GetNodes().IsDocNodes() ||
        IsHiddenFlag() )
        return;

    if(pAttr)
        GetFmt()->SetAttr(*pAttr);

    SwDoc* pDoc = (SwDoc*)pSectNd->GetDoc();
    pDoc->SetModified();

    // get current Language
    SwTOXInternational aIntl(  ((const SvxLanguageItem&)pDoc->GetAttrPool().
                        GetDefaultItem(RES_CHRATR_LANGUAGE )).GetLanguage() );

    aSortArr.DeleteAndDestroy( 0, aSortArr.Count() );

    // find the first layout node for this TOX, if it only find the content
    // in his own chapter
    const SwTxtNode* pOwnChapterNode = IsFromChapter()
            ? ::lcl_FindChapterNode( *pSectNd, 0 )
            : 0;

    SwNode2Layout aN2L( *pSectNd );
    ((SwSectionNode*)pSectNd)->DelFrms();

    // remove old content an insert one empty textnode (to hold the layout!)
    SwTxtNode* pFirstEmptyNd;
    {
        pDoc->DeleteRedline( *pSectNd );

        SwNodeIndex aSttIdx( *pSectNd, +1 );
        SwNodeIndex aEndIdx( *pSectNd->EndOfSectionNode() );
        pFirstEmptyNd = pDoc->GetNodes().MakeTxtNode( aEndIdx,
                        pDoc->GetTxtCollFromPool( RES_POOLCOLL_TEXT ) );

        {
            // Task 70995 - save and restore PageDesc and Break Attributes
            SwNodeIndex aNxtIdx( aSttIdx );
            const SwCntntNode* pCNd = aNxtIdx.GetNode().GetCntntNode();
            if( !pCNd )
                pCNd = pDoc->GetNodes().GoNext( &aNxtIdx );
            if( pCNd->GetpSwAttrSet() )
            {
                SfxItemSet aBrkSet( pDoc->GetAttrPool(), aBreakSetRange );
                aBrkSet.Put( *pCNd->GetpSwAttrSet() );
                if( aBrkSet.Count() )
                    pFirstEmptyNd->SwCntntNode::SetAttr( aBrkSet );
            }
        }
        aEndIdx--;
        SwPosition aPos( aEndIdx, SwIndex( pFirstEmptyNd, 0 ));
        pDoc->CorrAbs( aSttIdx, aEndIdx, aPos, TRUE );

        // delete all before
        DelFlyInRange( aSttIdx, aEndIdx );
        _DelBookmarks( aSttIdx, aEndIdx );

        pDoc->GetNodes().Delete( aSttIdx, aEndIdx.GetIndex() - aSttIdx.GetIndex() );

    }

    //
    // insert title of TOX
    if( GetTitle().Len() )
    {
        // then insert the headline section
        SwNodeIndex aIdx( *pSectNd, +1 );

        SwTxtNode* pHeadNd = pDoc->GetNodes().MakeTxtNode( aIdx,
                                GetTxtFmtColl( FORM_TITLE ) );
        pHeadNd->Insert( GetTitle(), SwIndex( pHeadNd ));

        String sNm( GetTOXName() );
// ??Resource
sNm.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "_Head" ));

        SwSection aSect( TOX_HEADER_SECTION, sNm );

        SwNodeIndex aStt( *pHeadNd ); aIdx--;
        SwSectionFmt* pSectFmt = pDoc->MakeSectionFmt( 0 );
        pDoc->GetNodes().InsertSection( aStt, *pSectFmt, aSect, &aIdx,
                                        TRUE, FALSE );
    }

    // jetzt waere ein prima Zeitpunkt, um die Numerierung zu updaten
    pDoc->UpdateNumRule();

    if( GetCreateType() & TOX_MARK )
        UpdateMarks( aIntl, pOwnChapterNode );

    if( GetCreateType() & TOX_OUTLINELEVEL )
        UpdateOutline( pOwnChapterNode );

    if( GetCreateType() & TOX_TEMPLATE )
        UpdateTemplate( pOwnChapterNode );

    if( GetCreateType() & TOX_OLE ||
            TOX_OBJECTS == SwTOXBase::GetType())
        UpdateCntnt( TOX_OLE, pOwnChapterNode );

    if( GetCreateType() & TOX_TABLE ||
            (TOX_TABLES == SwTOXBase::GetType() && IsFromObjectNames()) )
        UpdateTable( pOwnChapterNode );

    if( GetCreateType() & TOX_GRAPHIC ||
        (TOX_ILLUSTRATIONS == SwTOXBase::GetType() && IsFromObjectNames()))
        UpdateCntnt( TOX_GRAPHIC, pOwnChapterNode );

    if( GetSequenceName().Len() && !IsFromObjectNames() &&
        (TOX_TABLES == SwTOXBase::GetType() ||
         TOX_ILLUSTRATIONS == SwTOXBase::GetType() ) )
        UpdateSequence( pOwnChapterNode );

    if( GetCreateType() & TOX_FRAME )
        UpdateCntnt( TOX_FRAME, pOwnChapterNode );

    if(TOX_AUTHORITIES == SwTOXBase::GetType())
        UpdateAuthorities( pOwnChapterNode, aIntl );

    // Bei Bedarf Alphadelimitter einfuegen (nur bei Stichwoertern)
    //
    if( TOX_INDEX == SwTOXBase::GetType() &&
        ( GetOptions() & TOI_ALPHA_DELIMITTER ) )
        InsertAlphaDelimitter( aIntl );

    // sortierte Liste aller Verzeichnismarken und Verzeichnisbereiche
    void* p = 0;
    String* pStr = 0;
    USHORT nCnt = 0, nFormMax = GetTOXForm().GetFormMax();
    SvStringsDtor aStrArr( (BYTE)nFormMax );
    SvPtrarr aCollArr( (BYTE)nFormMax );
    for( ; nCnt < nFormMax; ++nCnt )
    {
        aCollArr.Insert( p, nCnt );
        aStrArr.Insert( pStr, nCnt );
    }

    SwNodeIndex aInsPos( *pFirstEmptyNd, 1 );
    for( nCnt = 0; nCnt < aSortArr.Count(); ++nCnt )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );

        // setze den Text in das Verzeichniss
        USHORT nLvl = aSortArr[ nCnt ]->GetLevel();
        SwTxtFmtColl* pColl = (SwTxtFmtColl*)aCollArr[ nLvl ];
        if( !pColl )
        {
            pColl = GetTxtFmtColl( nLvl );
            aCollArr.Remove( nLvl );
            p = pColl;
            aCollArr.Insert( p , nLvl );
        }

        // Generierung: dynamische TabStops setzen
        SwTxtNode* pTOXNd = pDoc->GetNodes().MakeTxtNode( aInsPos , pColl );
        aSortArr[ nCnt ]->pTOXNd = pTOXNd;

        // Generierung: Form auswerten und Platzhalter
        //              fuer die Seitennummer eintragen
        //if it is a TOX_INDEX and the SwForm IsCommaSeparated()
        // then a range of entries must be generated into one paragraph
        USHORT nRange = 1;
        if(TOX_INDEX == SwTOXBase::GetType() &&
                GetTOXForm().IsCommaSeparated() &&
                aSortArr[nCnt]->GetType() == TOX_SORT_INDEX)
        {
            const SwTOXMark& rMark = aSortArr[nCnt]->pTxtMark->GetTOXMark();
            const String sPrimKey = rMark.GetPrimaryKey();
            const String sSecKey = rMark.GetSecondaryKey();
            const SwTOXMark* pNextMark = 0;
            while(aSortArr.Count() > (nCnt + nRange)&&
                    aSortArr[nCnt + nRange]->GetType() == TOX_SORT_INDEX &&
                    0 != (pNextMark = &(aSortArr[nCnt + nRange]->pTxtMark->GetTOXMark())) &&
                    pNextMark->GetPrimaryKey() == sPrimKey &&
                    pNextMark->GetSecondaryKey() == sSecKey)
                nRange++;
        }
        GenerateText( nCnt, nRange, aStrArr );
        nCnt += nRange - 1;
    }

    // delete the first dummy node and remove all Cursor into the prev node
    aInsPos = *pFirstEmptyNd;
    {
        SwPaM aCorPam( *pFirstEmptyNd );
        aCorPam.GetPoint()->nContent.Assign( pFirstEmptyNd, 0 );
        if( !aCorPam.Move( fnMoveForward ) )
            aCorPam.Move( fnMoveBackward );
        SwNodeIndex aEndIdx( aInsPos, 1 );
        pDoc->CorrAbs( aInsPos, aEndIdx, *aCorPam.GetPoint(), TRUE );

        // Task 70995 - save and restore PageDesc and Break Attributes
        if( pFirstEmptyNd->GetpSwAttrSet() )
        {
            if( GetTitle().Len() )
                aEndIdx = *pSectNd;
            else
                aEndIdx = *pFirstEmptyNd;
            SwCntntNode* pCNd = pDoc->GetNodes().GoNext( &aEndIdx );
            pCNd->SetAttr( *pFirstEmptyNd->GetpSwAttrSet() );
        }
    }

    // now create the new Frames
    ULONG nIdx = pSectNd->GetIndex();
    // don't delete if index is empty
    if(nIdx + 2 < pSectNd->EndOfSectionIndex())
        pDoc->GetNodes().Delete( aInsPos, 1 );

    aN2L.RestoreUpperFrms( pDoc->GetNodes(), nIdx, nIdx + 1 );
    SwFrm::CheckPageDescs( (SwPageFrm*)pDoc->GetRootFrm()->Lower() );

    SetProtect( SwTOXBase::IsProtected() );
}

/*--------------------------------------------------------------------
     Beschreibung: AlphaDelimitter einfuegen
 --------------------------------------------------------------------*/


void SwTOXBaseSection::InsertAlphaDelimitter( const SwTOXInternational& rIntl )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    sal_Unicode nDeli, nLastDeli = 0;
    USHORT  i = 0;
    while( i < aSortArr.Count() )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );

        USHORT nLevel = aSortArr[i]->GetLevel();

        // Alpha-Delimitter ueberlesen
        if( nLevel == FORM_ALPHA_DELIMITTER )
            continue;

        nDeli = rIntl.GetIndexChar( aSortArr[i]->GetTxt() );

        // Delimitter schon vorhanden ??
        if( nDeli && nLastDeli != nDeli )
        {
            // alle kleiner Blank wollen wir nicht haben -> sind Sonderzeichen
            if( ' ' <= nDeli )
            {
                SwTOXCustom* pCst = new SwTOXCustom(nDeli, FORM_ALPHA_DELIMITTER,
                                                rIntl );
                aSortArr.Insert(pCst, i++);
            }
            nLastDeli = nDeli;
        }

        // Skippen bis gleibhes oder kleineres Level erreicht ist
        do {
            i++;
        } while (i < aSortArr.Count() && aSortArr[i]->GetLevel() > nLevel);
    }
}

/*--------------------------------------------------------------------
     Beschreibung: Template  auswerten
 --------------------------------------------------------------------*/

SwTxtFmtColl* SwTOXBaseSection::GetTxtFmtColl( USHORT nLevel )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    const String& rName = GetTOXForm().GetTemplate( nLevel );
    SwTxtFmtColl* pColl = rName.Len() ? pDoc->FindTxtFmtCollByName(rName) :0;
    if( !pColl )
    {
        USHORT nPoolFmt;
        const TOXTypes eType = SwTOXBase::GetType();
        switch( eType )
        {
        case TOX_INDEX:         nPoolFmt = RES_POOLCOLL_TOX_IDXH;       break;
        case TOX_USER:
            if( nLevel < 6 )
                nPoolFmt = RES_POOLCOLL_TOX_USERH;
            else
                nPoolFmt = RES_POOLCOLL_TOX_USER6 - 6;
            break;
        break;
        case TOX_ILLUSTRATIONS: nPoolFmt = RES_POOLCOLL_TOX_ILLUSH;     break;
        case TOX_OBJECTS:       nPoolFmt = RES_POOLCOLL_TOX_OBJECTH;    break;
        case TOX_TABLES:        nPoolFmt = RES_POOLCOLL_TOX_TABLESH;    break;
        case TOX_AUTHORITIES:   nPoolFmt = RES_POOLCOLL_TOX_AUTHORITIESH; break;

        case TOX_CONTENT:
            // im Content Bereich gibt es einen Sprung!
            if( nLevel < 6 )
                nPoolFmt = RES_POOLCOLL_TOX_CNTNTH;
            else
                nPoolFmt = RES_POOLCOLL_TOX_CNTNT6 - 6;
            break;
        }

        if(eType == TOX_AUTHORITIES && nLevel)
            nPoolFmt = nPoolFmt + 1;
        else if(eType == TOX_INDEX && nLevel)
        {
            //pool: Level 1,2,3, Delimiter
            //SwForm: Delimiter, Level 1,2,3
            nPoolFmt += 1 == nLevel ? nLevel + 3 : nLevel - 1;
        }
        else
            nPoolFmt += nLevel;
        pColl = pDoc->GetTxtCollFromPool( nPoolFmt );
    }
    return pColl;
}


/*--------------------------------------------------------------------
     Beschreibung: Aus Markierungen erzeugen
 --------------------------------------------------------------------*/

void SwTOXBaseSection::UpdateMarks( const SwTOXInternational& rIntl,
                                    const SwTxtNode* pOwnChapterNode )
{
    const SwModify* pType = SwTOXBase::GetRegisteredIn();
    if( !pType->GetDepends() )
        return;

    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    TOXTypes eTOXTyp = GetTOXType()->GetType();
    SwClientIter aIter( *(SwModify*)pType );

    SwTxtTOXMark* pTxtMark;
    SwTOXMark* pMark;
    for( pMark = (SwTOXMark*)aIter.First( TYPE( SwTOXMark )); pMark;
        pMark = (SwTOXMark*)aIter.Next() )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );

        if( pMark->GetTOXType()->GetType() == eTOXTyp &&
            0 != ( pTxtMark = pMark->GetTxtTOXMark() ) )
        {
            const SwTxtNode* pTOXSrc = pTxtMark->GetpTxtNd();
            // nur TOXMarks einfuegen die im Doc stehen
            // nicht die, die im UNDO stehen
            //
            // if selected use marks from the same chapter only
            if( pTOXSrc->GetNodes().IsDocNodes() &&
                pTOXSrc->GetTxt().Len() && pTOXSrc->GetDepends() &&
                pTOXSrc->GetFrm() &&
                (!IsFromChapter() ||
                    ::lcl_FindChapterNode( *pTOXSrc, 0 ) == pOwnChapterNode ))
            {
                SwTOXSortTabBase* pBase = 0;
                if(TOX_INDEX == eTOXTyp)
                {
                    // Stichwortverzeichnismarkierung
                    pBase = new SwTOXIndex( *pTOXSrc, pTxtMark,
                                            GetOptions(), FORM_ENTRY, rIntl );
                    InsertSorted(pBase);
                    if(GetOptions() & TOI_KEY_AS_ENTRY &&
                        pTxtMark->GetTOXMark().GetPrimaryKey().Len())
                    {
                        pBase = new SwTOXIndex( *pTOXSrc, pTxtMark,
                                                GetOptions(), FORM_PRIMARY_KEY, rIntl );
                        InsertSorted(pBase);
                        if(pTxtMark->GetTOXMark().GetSecondaryKey().Len())
                        {
                            pBase = new SwTOXIndex( *pTOXSrc, pTxtMark,
                                                    GetOptions(), FORM_SECONDARY_KEY, rIntl );
                            InsertSorted(pBase);
                        }
                    }
                }
                else if( TOX_USER == eTOXTyp ||
                    pMark->GetLevel() <= GetLevel())
                {   // Inhaltsberzeichnismarkierung
                    // also used for user marks
                    pBase = new SwTOXContent( *pTOXSrc, pTxtMark, rIntl );
                    InsertSorted(pBase);
                }
            }
        }
    }
}


/*--------------------------------------------------------------------
     Beschreibung:  Verzeichnisinhalt aus Gliederungsebene generieren
 --------------------------------------------------------------------*/


void SwTOXBaseSection::UpdateOutline( const SwTxtNode* pOwnChapterNode )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    SwNodes& rNds = pDoc->GetNodes();

    const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
    for( USHORT n = 0; n < rOutlNds.Count(); ++n )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );
        SwTxtNode* pTxtNd = rOutlNds[ n ]->GetTxtNode();
        if( pTxtNd && pTxtNd->Len() && pTxtNd->GetDepends() &&
            USHORT(pTxtNd->GetTxtColl()->GetOutlineLevel()+1) <= GetLevel() &&
            pTxtNd->GetFrm() &&
            ( !IsFromChapter() ||
               ::lcl_FindChapterNode( *pTxtNd, 0 ) == pOwnChapterNode ))
        {
            SwTOXPara * pNew = new SwTOXPara( *pTxtNd, TOX_TEMPLATE );
            InsertSorted( pNew );
        }
    }
}

/*--------------------------------------------------------------------
     Beschreibung: Verzeichnisinhalt aus Vorlagenbereichen generieren
 --------------------------------------------------------------------*/

void SwTOXBaseSection::UpdateTemplate( const SwTxtNode* pOwnChapterNode )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    for(USHORT i = 0; i < MAXLEVEL; i++)
    {
        String sTmpStyleNames = GetStyleNames(i);
        USHORT nTokenCount = sTmpStyleNames.GetTokenCount(TOX_STYLE_DELIMITER);
        for( USHORT nStyle = 0; nStyle < nTokenCount; ++nStyle )
        {
            SwTxtFmtColl* pColl = pDoc->FindTxtFmtCollByName(
                                    sTmpStyleNames.GetToken( nStyle,
                                                    TOX_STYLE_DELIMITER ));
            //TODO: no outline Collections in content indexes if OutlineLevels are already included
            if( !pColl ||
                ( TOX_CONTENT == SwTOXBase::GetType() &&
                  GetCreateType() & TOX_OUTLINELEVEL &&
                  NO_NUMBERING != pColl->GetOutlineLevel() ) )
                continue;

            SwClientIter aIter( *pColl );
            SwTxtNode* pTxtNd = (SwTxtNode*)aIter.First( TYPE( SwTxtNode ));
            for( ; pTxtNd; pTxtNd = (SwTxtNode*)aIter.Next() )
            {
                ::SetProgressState( 0, pDoc->GetDocShell() );

                if( pTxtNd->GetTxt().Len() && pTxtNd->GetFrm() &&
                    pTxtNd->GetNodes().IsDocNodes() &&
                    ( !IsFromChapter() || pOwnChapterNode ==
                        ::lcl_FindChapterNode( *pTxtNd, 0 ) ) )
                {
                    SwTOXPara * pNew = new SwTOXPara( *pTxtNd, TOX_TEMPLATE, i + 1 );
                    InsertSorted(pNew);
                }
            }
        }
    }
}

/* -----------------14.07.99 09:59-------------------
    Description: generate content from sequence fields
 --------------------------------------------------*/
void SwTOXBaseSection::UpdateSequence( const SwTxtNode* pOwnChapterNode )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    SwFieldType* pSeqFld = pDoc->GetFldType(RES_SETEXPFLD, GetSequenceName());
    if(!pSeqFld)
        return;

    SwClientIter aIter( *pSeqFld );
    SwFmtFld* pFmtFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
    for( ; pFmtFld; pFmtFld = (SwFmtFld*)aIter.Next() )
    {
        const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
        ::SetProgressState( 0, pDoc->GetDocShell() );

        if( rTxtNode.GetTxt().Len() && rTxtNode.GetFrm() &&
            rTxtNode.GetNodes().IsDocNodes() &&
            ( !IsFromChapter() ||
                ::lcl_FindChapterNode( rTxtNode, 0 ) == pOwnChapterNode ) )
        {
            SwTOXPara * pNew = new SwTOXPara( rTxtNode, TOX_SEQUENCE, 1 );
            //set indexes if the number or the reference text are to be displayed
            if( GetCaptionDisplay() == CAPTION_TEXT )
            {
                pNew->SetStartIndex(
                    SwGetExpField::GetReferenceTextPos( *pFmtFld, *pDoc ));
            }
            else if(GetCaptionDisplay() == CAPTION_NUMBER)
            {
                pNew->SetEndIndex(*pTxtFld->GetStart() + 1);
            }
            InsertSorted(pNew);
        }
    }
}
/* -----------------15.09.99 14:18-------------------

 --------------------------------------------------*/
void SwTOXBaseSection::UpdateAuthorities( const SwTxtNode* pOwnChapterNode,
                                            const SwTOXInternational& rIntl )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    SwFieldType* pAuthFld = pDoc->GetFldType(RES_AUTHORITY, aEmptyStr);
    if(!pAuthFld)
        return;

    SwClientIter aIter( *pAuthFld );
    SwFmtFld* pFmtFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
    for( ; pFmtFld; pFmtFld = (SwFmtFld*)aIter.Next() )
    {
        const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        //undo
        if(!pTxtFld)
            continue;
        const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
        ::SetProgressState( 0, pDoc->GetDocShell() );

//      const SwTxtNode* pChapterCompareNode = 0;

        if( rTxtNode.GetTxt().Len() && rTxtNode.GetFrm() &&
            rTxtNode.GetNodes().IsDocNodes() /*&&
            (!IsFromChapter() || pChapterCompareNode == pOwnChapterNode) */)
        {
            SwTOXAuthority* pNew = new SwTOXAuthority( rTxtNode, *pFmtFld, rIntl );

            InsertSorted(pNew);
        }
    }
}

/*--------------------------------------------------------------------
     Beschreibung:  Verzeichnisinhalt aus Inhaltsformen generieren
                    OLE, Grafik, Frame
                    Achtung: Spezielle Section !
 --------------------------------------------------------------------*/
/*
            nPos = pNd->GetIndex();
            if( nPos < pNd->GetNodes().GetEndOfExtras().GetIndex() )
            {
                // dann die "Anker" (Body) Position holen.
                Point aPt;
                const SwCntntFrm* pFrm = pNd->GetFrm( &aPt, 0, FALSE );
                if( pFrm )
                {
                    SwPosition aPos( *pNd );
                    SwDoc* pDoc = (SwDoc*)pNd->GetDoc();
#ifndef PRODUCT
                    ASSERT( GetBodyTxtNode( pDoc, aPos, pFrm ),
                            "wo steht der Absatz" );
#else
                    GetBodyTxtNode( pDoc, aPos, pFrm );
#endif
                    nPos = aPos.nNode.GetIndex();
                    nCntPos = aPos.nContent.GetIndex();
                }
            }



 */
void SwTOXBaseSection::UpdateCntnt( SwTOXElement eType,
                                    const SwTxtNode* pOwnChapterNode )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    SwNodes& rNds = pDoc->GetNodes();
    // auf den 1. Node der 1. Section
    ULONG nIdx = rNds.GetEndOfAutotext().StartOfSectionIndex() + 2,
         nEndIdx = rNds.GetEndOfAutotext().GetIndex();

    while( nIdx < nEndIdx )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );

        SwNode* pNd = rNds[ nIdx ];
        SwCntntNode* pCNd = 0;
        switch( eType )
        {
        case TOX_FRAME:
            if( !pNd->IsNoTxtNode() )
            {
                pCNd = pNd->GetCntntNode();
                if( !pCNd )
                {
                    SwNodeIndex aTmp( *pNd );
                    pCNd = rNds.GoNext( &aTmp );
                }
            }
            break;
        case TOX_GRAPHIC:
            if( pNd->IsGrfNode() )
                pCNd = (SwCntntNode*)pNd;
            break;
        case TOX_OLE:
            if( pNd->IsOLENode() )
            {
                BOOL bInclude = FALSE;
                if(TOX_OBJECTS == SwTOXBase::GetType())
                {
                    SwOLENode* pOLENode = pNd->GetOLENode();
                    long nOLEOptions = GetOLEOptions();
                    //
                    const SwOLEObj& rOLEObj = pOLENode->GetOLEObj();

                    if ( pOLENode->GetOLEObj().IsOleRef() ) //Noch nicht geladen
                    {
                        const SotFactory* pFact = pOLENode->GetOLEObj().
                                                GetOleRef()->GetSvFactory();
                        BOOL bMath = SmModuleDummy::HasID( *pFact );
                        BOOL bChart = SchModuleDummy::HasID( *pFact );
                        BOOL bCalc = ScModuleDummy::HasID( *pFact );
                        BOOL bDrawImage = SdModuleDummy::HasID( *pFact );
                        if(
                            ((nOLEOptions & TOO_MATH) && bMath ) ||
                            ((nOLEOptions & TOO_CHART)&& bChart ) ||
                            ((nOLEOptions & TOO_CALC) && bCalc ) ||
                            ((nOLEOptions & TOO_DRAW_IMPRESS) && bDrawImage ) ||
                            ((nOLEOptions & TOO_OTHER) &&
                                !bMath && !bChart && !bCalc && !bDrawImage))
                            bInclude = TRUE;
                    }
                    else
                    {
                        DBG_ERROR("OLE-object nicht geladen?")
                    }
                }
                else
                    bInclude = TRUE;
                if(bInclude)
                    pCNd = (SwCntntNode*)pNd;
            }
            break;
        }

        if( pCNd )
        {
            //find node in body text
            Point aPt;
            const SwCntntFrm* pFrm = pCNd->GetFrm( &aPt, 0, FALSE );
            USHORT nSetLevel = USHRT_MAX;

            if( IsLevelFromChapter() )
            {
                const SwTxtNode* pOutlNd = ::lcl_FindChapterNode( *pCNd,
                                                        MAXLEVEL - 1 );
                if( pOutlNd )
                {
                    USHORT nTmp = pOutlNd->GetTxtColl()->GetOutlineLevel();
                    if( nTmp < NO_NUMBERING )
                        nSetLevel = nTmp + 1;
                }
            }

            if( pCNd->GetFrm() && ( !IsFromChapter() ||
                    ::lcl_FindChapterNode( *pCNd, 0 ) == pOwnChapterNode ))
            {
                SwTOXPara * pNew = new SwTOXPara( *pCNd, eType,
                            USHRT_MAX != nSetLevel ? nSetLevel : FORM_ALPHA_DELIMITTER );
                InsertSorted( pNew );
            }
        }

        nIdx = pNd->FindStartNode()->EndOfSectionIndex() + 2;   // 2 == End-/StartNode
    }
}

/*--------------------------------------------------------------------
     Beschreibung:  Tabelleneintraege zusammensuchen
 --------------------------------------------------------------------*/

void SwTOXBaseSection::UpdateTable( const SwTxtNode* pOwnChapterNode )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    SwNodes& rNds = pDoc->GetNodes();
    const SwFrmFmts& rArr = *pDoc->GetTblFrmFmts();

    for( USHORT n = 0; n < rArr.Count(); ++n )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );

        SwTable* pTmpTbl = SwTable::FindTable( rArr[ n ] );
        SwTableBox* pFBox;
        if( pTmpTbl && 0 != (pFBox = pTmpTbl->GetTabSortBoxes()[0] ) &&
            pFBox->GetSttNd() && pFBox->GetSttNd()->GetNodes().IsDocNodes() )
        {
            const SwTableNode* pTblNd = pFBox->GetSttNd()->FindTableNode();
            SwNodeIndex aCntntIdx( *pTblNd, 1 );

            SwCntntNode* pCNd;
            while( 0 != ( pCNd = rNds.GoNext( &aCntntIdx ) ) &&
                aCntntIdx.GetIndex() < pTblNd->EndOfSectionIndex() )
            {
                if( pCNd->GetFrm() && (!IsFromChapter() ||
                    ::lcl_FindChapterNode( *pCNd, 0 ) == pOwnChapterNode ))
                {
                    SwTOXTable * pNew = new SwTOXTable( *pCNd );
                    if( IsLevelFromChapter() )
                    {
                        const SwTxtNode* pOutlNd =
                            ::lcl_FindChapterNode( *pCNd, MAXLEVEL - 1 );
                        if( pOutlNd )
                        {
                            USHORT nTmp = pOutlNd->GetTxtColl()->GetOutlineLevel();
                            if( nTmp < NO_NUMBERING )
                                pNew->SetLevel( nTmp + 1 );
                        }
                    }
                    InsertSorted(pNew);
                    break;
                }
            }
        }
    }
}

/*--------------------------------------------------------------------
     Beschreibung:  String generieren anhand der Form
                    SonderZeichen 0-31 und 255 entfernen
 --------------------------------------------------------------------*/

String lcl_GetNumString( const SwTOXSortTabBase& rBase )
{
    String sRet;

    if( !rBase.pTxtMark && rBase.aTOXSources.Count() > 0 )
    {   // nur wenn es keine Marke ist
        const SwTxtNode* pNd = rBase.aTOXSources[0].pNd->GetTxtNode();
        if( pNd )
        {
            const SwNodeNum* pNum;
            const SwNumRule* pRule;

            if( (( 0 != ( pNum = pNd->GetNum() ) &&
                    0 != ( pRule = pNd->GetNumRule() )) ||
                    ( 0 != ( pNum = pNd->GetOutlineNum() ) &&
                    0 != ( pRule = pNd->GetDoc()->GetOutlineNumRule() ) ) ) &&
                pNum->GetLevel() < MAXLEVEL )
                sRet = pRule->MakeNumString( *pNum );
        }
    }
    return sRet;
}

void SwTOXBaseSection::GenerateText( USHORT nArrayIdx, USHORT nCount,
                                    SvStringsDtor& rTabForms )
{
    LinkStructArr   aLinkArr;
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    ::SetProgressState( 0, pDoc->GetDocShell() );

    //pTOXNd is only set at the first mark
    SwTxtNode* pTOXNd = (SwTxtNode*)aSortArr[nArrayIdx]->pTOXNd;
    String& rTxt = (String&)pTOXNd->GetTxt();
    rTxt.Erase();
    for(USHORT nIndex = nArrayIdx; nIndex < nArrayIdx + nCount; nIndex++)
    {
        if(nIndex > nArrayIdx)
            rTxt.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ", " )); // comma separation
        // String mit dem Pattern aus der Form initialisieren
        const SwTOXSortTabBase& rBase = *aSortArr[nIndex];
        USHORT nLvl = rBase.GetLevel();
        ASSERT( nLvl < GetTOXForm().GetFormMax(), "ungueltiges FORM_LEVEL");

        SvxTabStopItem aTStops( 0, 0 );
        xub_StrLen nLinkStartPosition = STRING_NOTFOUND;
        String sURL;
        // create an enumerator
        SwFormTokenEnumerator aTokenEnum = GetTOXForm().CreateTokenEnumerator(nLvl);
        // remove text from node
        while(aTokenEnum.HasNextToken())
        {
            SwFormToken aToken = aTokenEnum.GetNextToken();
            xub_StrLen nStartCharStyle = rTxt.Len();
            switch( aToken.eTokenType )
            {
            case TOKEN_ENTRY_NO:
                // fuer Inhaltsverzeichnis Numerierung
                rTxt.Insert( lcl_GetNumString( rBase ));
                break;

            case TOKEN_ENTRY_TEXT:
                {
                    SwIndex aIdx( pTOXNd, rTxt.Len() );
                    rBase.FillText( *pTOXNd, aIdx );
                }
                break;

            case TOKEN_ENTRY:
                {
                    // fuer Inhaltsverzeichnis Numerierung
                    rTxt.Insert( lcl_GetNumString( rBase ));

                    SwIndex aIdx( pTOXNd, rTxt.Len() );
                    rBase.FillText( *pTOXNd, aIdx );
                }
                break;

            case TOKEN_TAB_STOP:
                rTxt.Append('\t');
                //
                if(SVX_TAB_ADJUST_END > aToken.eTabAlign)
                {
                    const SvxLRSpaceItem& rLR = (SvxLRSpaceItem&)pTOXNd->
                                        SwCntntNode::GetAttr( RES_LR_SPACE );

                    long nTabPosition = aToken.nTabStopPosition;
                    if( !GetTOXForm().IsRelTabPos() && rLR.GetTxtLeft() )
                        nTabPosition -= rLR.GetTxtLeft();
                    aTStops.Insert( SvxTabStop( nTabPosition,
                                                SVX_TAB_ADJUST_LEFT,
                                                cDfltDecimalChar,
                                                aToken.cTabFillChar ));
                }
                else
                {
                    const SwPageDesc* pPageDesc = ((SwFmtPageDesc&)pTOXNd->
                                SwCntntNode::GetAttr( RES_PAGEDESC )).GetPageDesc();

                    BOOL bCallFindRect = TRUE;
                    long nRightMargin;
                    if( pPageDesc )
                    {
                        const SwFrm* pFrm = pTOXNd->GetFrm( 0, 0, TRUE );
                        if( !pFrm || 0 == ( pFrm = pFrm->FindPageFrm() ) ||
                            pPageDesc != ((SwPageFrm*)pFrm)->GetPageDesc() )
                            // dann muss man ueber den PageDesc gehen
                            bCallFindRect = FALSE;
                    }

                    SwRect aNdRect;
                    if( bCallFindRect )
                        aNdRect = pTOXNd->FindLayoutRect( TRUE );

                    if( aNdRect.IsEmpty() )
                    {
                        // dann hilft alles nichts, wir muessen ueber die Seiten-
                        // vorlage gehen.
                        if( !pPageDesc &&
                            0 == (pPageDesc = pTOXNd->FindPageDesc( FALSE ) ) )
                            pPageDesc = &pDoc->GetPageDesc( 0 );

                        const SwFrmFmt& rPgDscFmt = pPageDesc->GetMaster();
                        nRightMargin = rPgDscFmt.GetFrmSize().GetWidth() -
                                         rPgDscFmt.GetLRSpace().GetLeft() -
                                         rPgDscFmt.GetLRSpace().GetRight();
                    }
                    else
                        nRightMargin = aNdRect.Width();
                    aTStops.Insert( SvxTabStop( nRightMargin,SVX_TAB_ADJUST_RIGHT,
                                                cDfltDecimalChar,
                                                aToken.cTabFillChar ));
                }
                break;

            case TOKEN_TEXT:
                rTxt.Append( aToken.sText );
                break;

            case TOKEN_PAGE_NUMS:
                    // Platzhalter fuer Seitennummer(n) es wird nur der erste beachtet
                    //
                {
                    // Die Anzahl der gleichen Eintrage bestimmt die Seitennummern-Pattern
                    //
                    USHORT nSize = rBase.aTOXSources.Count();
                    if( nSize > 0 )
                    {
                        String aInsStr( cNumRepl );
                        for(USHORT i=1; i < nSize; ++i)
                        {
                            aInsStr.AppendAscii( sPageDeli );
                            aInsStr += cNumRepl;
                        }
                        aInsStr += cEndPageNum;
                        rTxt.Append( aInsStr );
                    }
//                      // Tab entfernen, wenn keine Seitennummer
//                  else if( rTxt.Len() && '\t' == rTxt.GetChar( rTxt.Len() - 1 ))
//                      rTxt.Erase( rTxt.Len()-1, 1 );
                }
                break;

            case TOKEN_CHAPTER_INFO:
                {
                    // ein bischen trickreich: suche irgend einen Frame
                    const SwTOXSource* pTOXSource = 0;
                    if(rBase.aTOXSources.Count())
                        pTOXSource = &rBase.aTOXSources[0];
                    if( pTOXSource && pTOXSource->pNd && pTOXSource->pNd->IsTxtNode() )
                    {
                        const SwCntntFrm* pFrm = pTOXSource->pNd->GetFrm();
                        if( pFrm )
                        {
                            SwChapterFieldType aFldTyp;
                            SwChapterField aFld( &aFldTyp, aToken.nChapterFormat );
                            aFld.SetLevel( MAXLEVEL - 1 );
                            aFld.ChangeExpansion( pFrm, (SwTxtNode*)pTOXSource->pNd, TRUE );

                            if(CF_NUMBER == aToken.nChapterFormat)
                                rTxt.Insert(aFld.GetNumber());
                            else if(CF_NUM_TITLE == aToken.nChapterFormat)
                            {
                                rTxt += aFld.GetNumber();
                                rTxt += ' ';
                                rTxt += aFld.GetTitle();
                            }
                            else if(CF_TITLE == aToken.nChapterFormat)
                                rTxt += aFld.GetTitle();
                        }
                    }
                }
                break;

            case TOKEN_LINK_START:
                nLinkStartPosition = rTxt.Len();
                break;

            case TOKEN_LINK_END:
                    //TODO: only paired start/end tokens are valid
                if( STRING_NOTFOUND != nLinkStartPosition)
                {
                    SwIndex aIdx( pTOXNd, nLinkStartPosition );
                    //pTOXNd->Erase( aIdx, SwForm::nFormLinkSttLen );
                    xub_StrLen nEnd = rTxt.Len();

                    if( !sURL.Len() )
                    {
                        sURL = rBase.GetURL();
                        if( !sURL.Len() )
                            break;
                    }
                    aLinkArr.Insert( new LinkStruct(sURL, nLinkStartPosition,
                                                    nEnd), aLinkArr.Count() );
                    nLinkStartPosition = STRING_NOTFOUND;
                }
                break;

            case TOKEN_AUTHORITY:
                {
                    ToxAuthorityField eField = (ToxAuthorityField)aToken.nAuthorityField;
                    SwIndex aIdx( pTOXNd, rTxt.Len() );
                    rBase.FillText( *pTOXNd, aIdx, eField );
                }
                break;
            }

            if( aToken.sCharStyleName.Len() )
            {
                SwCharFmt* pCharFmt;
                if( USHRT_MAX != aToken.nPoolId )
                    pCharFmt = pDoc->GetCharFmtFromPool( aToken.nPoolId );
                else
                    pCharFmt = pDoc->FindCharFmtByName( aToken.sCharStyleName);

                if(pCharFmt)
                    pTOXNd->Insert( SwFmtCharFmt( pCharFmt ), nStartCharStyle,
                                    rTxt.Len(), SETATTR_DONTEXPAND );
            }
        }
        pTOXNd->SwCntntNode::SetAttr( aTStops );
    }

    if(aLinkArr.Count())
        for(USHORT i = 0; i < aLinkArr.Count(); ++i )
        {
            LinkStruct* pTmp = aLinkArr.GetObject(i);
            pTOXNd->Insert( pTmp->aINetFmt, pTmp->nStartTextPos,
                            pTmp->nEndTextPos);
        }
}

/*--------------------------------------------------------------------
     Beschreibung: Seitennummer errechnen und nach dem Formatieren
                   eintragen
 --------------------------------------------------------------------*/

void SwTOXBaseSection::UpdatePageNum()
{
    if( !aSortArr.Count() )
        return ;

    // die aktuellen Seitennummern ins Verzeichnis eintragen
    SwPageFrm*  pAktPage    = 0;
    USHORT      nPage       = 0;
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();

    SwTOXInternational aIntl(  ((const SvxLanguageItem&)pDoc->GetAttrPool().
                        GetDefaultItem(RES_CHRATR_LANGUAGE )).GetLanguage() );

    for( USHORT nCnt = 0; nCnt < aSortArr.Count(); ++nCnt )
    {
        // Schleife ueber alle SourceNodes
        SvUShorts aNums;        //Die Seitennummern
        SvPtrarr  aDescs;       //Die PageDescriptoren passend zu den Seitennummern.
        SvUShorts* pMainNums = 0; // contains page numbers of main entries

        // process run in lines
        USHORT nRange = 0;
        if(GetTOXForm().IsCommaSeparated() &&
                aSortArr[nCnt]->GetType() == TOX_SORT_INDEX)
        {
            const SwTOXMark& rMark = aSortArr[nCnt]->pTxtMark->GetTOXMark();
            const String sPrimKey = rMark.GetPrimaryKey();
            const String sSecKey = rMark.GetSecondaryKey();
            const SwTOXMark* pNextMark = 0;
            while(aSortArr.Count() > (nCnt + nRange)&&
                    aSortArr[nCnt + nRange]->GetType() == TOX_SORT_INDEX &&
                    0 != (pNextMark = &(aSortArr[nCnt + nRange]->pTxtMark->GetTOXMark())) &&
                    pNextMark->GetPrimaryKey() == sPrimKey &&
                    pNextMark->GetSecondaryKey() == sSecKey)
                nRange++;
        }
        else
            nRange = 1;

        for(USHORT nRunInEntry = nCnt; nRunInEntry < nCnt + nRange; nRunInEntry++)
        {
            SwTOXSortTabBase* pSortBase = aSortArr[nRunInEntry];
            USHORT nSize = pSortBase->aTOXSources.Count();
            for( USHORT j = 0; j < nSize; ++j )
            {
                ::SetProgressState( 0, pDoc->GetDocShell() );

                SwTOXSource& rTOXSource = pSortBase->aTOXSources[j];
                if( rTOXSource.pNd )
                {
                    SwCntntFrm* pFrm = rTOXSource.pNd->GetFrm();
                    ASSERT( pFrm, "TOX, no Frame found" );
                    if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->HasFollow() )
                    {
                        // dann suche den richtigen heraus
                        SwTxtFrm* pNext = (SwTxtFrm*)pFrm;
                        while( 0 != ( pNext = (SwTxtFrm*)pFrm->GetFollow() )
                                && rTOXSource.nPos >= pNext->GetOfst() )
                            pFrm = pNext;
                    }

                    SwPageFrm*  pTmpPage = pFrm->FindPageFrm();
                    if( pTmpPage != pAktPage )
                    {
                        nPage       = pTmpPage->GetVirtPageNum();
                        pAktPage    = pTmpPage;
                    }

                    // sortiert einfuegen
                    for( USHORT i = 0; i < aNums.Count() && aNums[i] < nPage; ++i )
                        ;

                    if( i >= aNums.Count() || aNums[ i ] != nPage )
                    {
                        aNums.Insert( nPage, i );
                        aDescs.Insert( (void*)pAktPage->GetPageDesc(), i );
                    }
                    // is it a main entry?
                    if(TOX_SORT_INDEX == pSortBase->GetType() &&
                        rTOXSource.bMainEntry)
                    {
                        if(!pMainNums)
                            pMainNums = new SvUShorts;
                        pMainNums->Insert(nPage, pMainNums->Count());
                    }
                }
            }
            // einfuegen der Seitennummer in den Verzeichnis-Text-Node
            const SwTOXSortTabBase* pBase = aSortArr[ nCnt ];
            if(pBase->pTOXNd)
            {
                const SwTxtNode* pTxtNd = pBase->pTOXNd->GetTxtNode();
                ASSERT( pTxtNd, "kein TextNode, falsches Verzeichnis" );

                _UpdatePageNum( (SwTxtNode*)pTxtNd, aNums, aDescs, pMainNums,
                                aIntl );
            }
            DELETEZ(pMainNums);
            aNums.Remove(0, aNums.Count());
        }
    }
    // nach dem Setzen der richtigen Seitennummer, das Mapping-Array
    // wieder loeschen !!
    aSortArr.DeleteAndDestroy( 0, aSortArr.Count() );
}


/*--------------------------------------------------------------------
     Beschreibung: Austausch der Seitennummer-Platzhalter
 --------------------------------------------------------------------*/

// search for the page no in the array of main entry page numbers
BOOL lcl_HasMainEntry( const SvUShorts* pMainEntryNums, USHORT nToFind )
{
    for(USHORT i = 0; pMainEntryNums && i < pMainEntryNums->Count(); ++i)
        if(nToFind == (*pMainEntryNums)[i])
            return TRUE;
    return FALSE;
}

void SwTOXBaseSection::_UpdatePageNum( SwTxtNode* pNd,
                                    const SvUShorts& rNums,
                                    const SvPtrarr & rDescs,
                                    const SvUShorts* pMainEntryNums,
                                    const SwTOXInternational& rIntl )
{
    //collect starts end ends of main entry character style
    SvUShorts* pCharStyleIdx = pMainEntryNums ? new SvUShorts : 0;

    String sSrchStr( cNumRepl ); sSrchStr.AppendAscii( sPageDeli ) += cNumRepl;
    xub_StrLen nStartPos = pNd->GetTxt().Search( sSrchStr );
    ( sSrchStr = cNumRepl ) += cEndPageNum;
    xub_StrLen nEndPos = pNd->GetTxt().Search( sSrchStr );

    if( STRING_NOTFOUND == nEndPos || !rNums.Count() )
        return;

    if( STRING_NOTFOUND == nStartPos || nStartPos > nEndPos)
        nStartPos = nEndPos;

    USHORT nOld = rNums[0],
           nBeg = nOld,
           nCount  = 0;
    String aNumStr( SwNumType( ((SwPageDesc*)rDescs[0])->GetNumType() ).
                    GetNumStr( nBeg ) );
    if( pCharStyleIdx && lcl_HasMainEntry( pMainEntryNums, nBeg ))
    {
        USHORT nTemp = 0;
        pCharStyleIdx->Insert( nTemp, pCharStyleIdx->Count());
    }

    // Platzhalter loeschen
    SwIndex aPos(pNd, nStartPos);
    const SwFmtCharFmt* pPageNoCharFmt = 0;
    SwpHints* pHints = pNd->GetpSwpHints();
    if(pHints)
        for(USHORT nHintIdx = 0; nHintIdx < pHints->GetStartCount(); nHintIdx++)
        {
            SwTxtAttr* pAttr = pHints->GetStart(nHintIdx);
            xub_StrLen nTmpEnd = pAttr->GetEnd() ? *pAttr->GetEnd() : 0;
            if( nStartPos >= *pAttr->GetStart() &&
                (nStartPos + 2) <= nTmpEnd &&
                pAttr->Which() == RES_TXTATR_CHARFMT)
            {
                pPageNoCharFmt = &pAttr->GetCharFmt();
                break;
            }
        }
    pNd->Erase(aPos, nEndPos - nStartPos + 2);

    for(USHORT i = 1; i < rNums.Count(); ++i)
    {
        SwNumType aType( ((SwPageDesc*)rDescs[i])->GetNumType() );
        if( TOX_INDEX == SwTOXBase::GetType() )
        {   // Zusammenfassen f. ff.
            // Alle folgenden aufaddieren
            // break up if main entry starts or ends and
            // insert a char style index
            BOOL bMainEntryChanges = lcl_HasMainEntry(pMainEntryNums, nOld)
                    != lcl_HasMainEntry(pMainEntryNums, rNums[i]);

            if(nOld == rNums[i]-1 && !bMainEntryChanges &&
                0 != (GetOptions() & (TOI_FF|TOI_DASH)))
                nCount++;
            else
            {
                // ff. f. alten Wert flushen
                if(GetOptions() & TOI_FF)
                {
                    if ( nCount >= 1 )
                    {
                        USHORT eText = nCount > 1 ? FOLLOWTEXT_PAGES
                                                     : FOLLOWTEXT_PAGE;
                        aNumStr += rIntl.GetFollowingText( eText );
                    }
                }
                else
                {
                    if(nCount >= 2 )
                        aNumStr += '-';
                    else if(nCount == 1 )
                        aNumStr.AppendAscii( sPageDeli );
//#58127# Wenn nCount == 0, dann steht die einzige Seitenzahl schon im aNumStr!
                    if(nCount)
                        aNumStr += aType.GetNumStr( nBeg + nCount );
                }

                // neuen String anlegen
                nBeg     = rNums[i];
                aNumStr.AppendAscii( sPageDeli );
                //the change of the character style must apply after sPageDeli is appended
                if(pCharStyleIdx && bMainEntryChanges)
                    pCharStyleIdx->Insert(aNumStr.Len(),
                                                    pCharStyleIdx->Count());
                aNumStr += aType.GetNumStr( nBeg );
                nCount   = 0;
            }
            nOld = rNums[i];
        }
        else
        {   // Alle Nummern eintragen
            aNumStr += aType.GetNumStr( USHORT(rNums[i]) );
            if(i != (rNums.Count()-1))
                aNumStr.AppendAscii( sPageDeli );
        }
    }
    // Bei Ende und ff. alten Wert flushen
    if( TOX_INDEX == SwTOXBase::GetType() )
    {
        if(GetOptions() & TOI_FF)
        {
            if( nCount >= 1 )
            {
                USHORT eText = nCount > 1 ? FOLLOWTEXT_PAGES
                                                    : FOLLOWTEXT_PAGE;
                aNumStr += rIntl.GetFollowingText( eText );
            }
        }
        else
        {
            if(nCount >= 2)
                aNumStr +='-';
            else if(nCount == 1)
                aNumStr.AppendAscii( sPageDeli );
//#58127# Wenn nCount == 0, dann steht die einzige Seitenzahl schon im aNumStr!
            if(nCount)
                aNumStr += SwNumType( ((SwPageDesc*)rDescs[i-1])->
                                GetNumType() ).GetNumStr( nBeg+nCount );
        }
    }
    pNd->Insert( aNumStr, aPos, INS_EMPTYEXPAND );
    if(pPageNoCharFmt)
    {
        SwDoc* pDoc = pNd->GetDoc();
        SwFmtCharFmt aCharFmt(pPageNoCharFmt->GetCharFmt());
        pNd->Insert(aCharFmt, nStartPos, nStartPos + aNumStr.Len(), SETATTR_DONTEXPAND);
    }

    //now the main entries should get there character style
    if(pCharStyleIdx && pCharStyleIdx->Count() && GetMainEntryCharStyle().Len())
    {
        // eventually the last index must me appended
        if(pCharStyleIdx->Count()&0x01)
            pCharStyleIdx->Insert(aNumStr.Len(), pCharStyleIdx->Count());

        //search by name
        SwDoc* pDoc = pNd->GetDoc();
        USHORT nPoolId = pDoc->GetPoolId( GetMainEntryCharStyle(), GET_POOLID_CHRFMT );
        SwCharFmt* pCharFmt = 0;
        if(USHRT_MAX != nPoolId)
            pCharFmt = pDoc->GetCharFmtFromPool(nPoolId);
        else
            pCharFmt = pDoc->FindCharFmtByName( GetMainEntryCharStyle() );
        if(!pCharFmt)
            pCharFmt = pDoc->MakeCharFmt(GetMainEntryCharStyle(), 0);

        //find the page numbers in aNumStr and set the character style
        xub_StrLen nOffset = pNd->GetTxt().Len() - aNumStr.Len();
        SwFmtCharFmt aCharFmt(pCharFmt);
        for(USHORT i = 0; i < pCharStyleIdx->Count(); i += 2)
        {
            xub_StrLen nStartIdx = (*pCharStyleIdx)[i] + nOffset;
            xub_StrLen nEndIdx = (*pCharStyleIdx)[i + 1]  + nOffset;
            pNd->Insert(aCharFmt, nStartIdx, nEndIdx, SETATTR_DONTEXPAND);
        }

    }
    delete pCharStyleIdx;
}


/*--------------------------------------------------------------------
     Beschreibung: Sortiert einfuegen in das SortArr
 --------------------------------------------------------------------*/

void SwTOXBaseSection::InsertSorted(SwTOXSortTabBase* pNew)
{
    Range aRange(0, aSortArr.Count());
    if( TOX_INDEX == SwTOXBase::GetType() && pNew->pTxtMark )
    {
        const SwTOXMark& rMark = pNew->pTxtMark->GetTOXMark();
        // Schluessel auswerten
        // Den Bereich ermitteln, in dem einzufuegen ist
        if( 0 == (GetOptions() & TOI_KEY_AS_ENTRY) &&
            rMark.GetPrimaryKey().Len() )
        {
            aRange = GetKeyRange( rMark.GetPrimaryKey(), FORM_PRIMARY_KEY,
                                    aRange, *pNew->pTOXIntl );
            if( rMark.GetSecondaryKey().Len() )
                aRange = GetKeyRange( rMark.GetSecondaryKey(), FORM_SECONDARY_KEY,
                                      aRange, *pNew->pTOXIntl );
        }
    }
    // Pos suchen und einfuegen
    //
    for(short i = (short)aRange.Min(); i < (short)aRange.Max(); ++i)
    {   // nur auf gleicher Ebene pruefen
        //
        SwTOXSortTabBase* pOld = aSortArr[i];
        if(*pOld == *pNew)
        {
            if(TOX_AUTHORITIES == SwTOXBase::GetType())
            {
                //only the first occurence in the document
                //has to be in the array
                if(*pOld < *pNew)
                {
                    delete pNew;
                }
                else
                {
                    // remove the old content
                    aSortArr.DeleteAndDestroy( i, 1 );
                    aSortArr.Insert(pNew, i );
                }
                return;
            }
            else
            {
                // Eigener Eintrag fuer Doppelte oder Keywords
                //
                if( pOld->GetType() == TOX_SORT_CUSTOM &&
                       pNew->GetOptions() & TOI_KEY_AS_ENTRY)
                    continue;

                if(!(pNew->GetOptions() & TOI_SAME_ENTRY))
                {   // Eigener Eintrag
                    aSortArr.Insert(pNew, i );
                    return;
                }
                // Eintrag schon vorhanden in Referenzliste aufnehmen
                pOld->aTOXSources.Insert( pNew->aTOXSources[0],
                                            pOld->aTOXSources.Count() );

                delete pNew;
                return;
            }
        }
        if(*pNew < *pOld)
            break;
    }
    // SubLevel Skippen
    while( TOX_INDEX == SwTOXBase::GetType() && i < aRange.Max() &&
          aSortArr[i]->GetLevel() > pNew->GetLevel() )
        i++;

    // An Position i wird eingefuegt
    aSortArr.Insert(pNew, i );
}

/*--------------------------------------------------------------------
     Beschreibung: Schluessel-Bereich suchen und evtl einfuegen
 --------------------------------------------------------------------*/

Range SwTOXBaseSection::GetKeyRange(const   String& rStr,
                                     USHORT nLevel,
                                     const  Range& rRange,
                                     const SwTOXInternational& rIntl )
{
    String sToCompare(rStr);
    if( 0 != (TOI_INITIAL_CAPS & GetOptions()) )
    {
        String sUpper( rIntl.ToUpper( sToCompare, 0 ));
        sToCompare.Erase( 0, 1 ).Insert( sUpper, 0 );
    }

    ASSERT(rRange.Min() >= 0 && rRange.Max() >= 0, "Min Max < 0");

    const USHORT nMin = (USHORT)rRange.Min();
    const USHORT nMax = (USHORT)rRange.Max();

    USHORT nOptions = GetOptions();
    USHORT nCmpFlags = (nOptions & TOI_SAME_ENTRY) && 0 == (nOptions & TOI_CASE_SENSITIVE)
                            ? INTN_COMPARE_IGNORECASE : 0;
    for(USHORT i = nMin; i < nMax; ++i)
    {
        SwTOXSortTabBase* pBase = aSortArr[i];
        String aTmp = pBase->GetTxt();
        if( rIntl.IsEqual( aTmp, sToCompare, nCmpFlags )  &&
                pBase->GetLevel() == nLevel &&
                    pBase->GetType() == TOX_SORT_CUSTOM)
            break;
    }
    if(i == nMax)
    {   // Falls nicht vorhanden erzeugen und einfuegen
        //
        SwTOXCustom* pKey = new SwTOXCustom( sToCompare, nLevel, rIntl );
        for(i = nMin; i < nMax; ++i)
        {
            if(nLevel == aSortArr[i]->GetLevel() &&  *pKey < *(aSortArr[i]))
                break;
        }
        aSortArr.Insert(pKey, i );
    }
    USHORT nStart = i+1;
    USHORT nEnd   = aSortArr.Count();

    // Ende des Bereiches suchen
    for(i = nStart; i < aSortArr.Count(); ++i)
    {
        if(aSortArr[i]->GetLevel() <= nLevel)
        {   nEnd = i;
            break;
        }
    }
    return Range(nStart, nEnd);
}


BOOL SwTOXBase::IsTOXBaseInReadonly() const
{
    const SwTOXBaseSection *pSect = PTR_CAST(SwTOXBaseSection, this);
    BOOL bRet = FALSE;
    const SwSectionNode* pSectNode;
    if(pSect && pSect->GetFmt() &&
            0 != (pSectNode = pSect->GetFmt()->GetSectionNode()))
    {
        const SwDocShell* pDocSh;
        bRet = (0 != (pDocSh = pSectNode->GetDoc()->GetDocShell()) &&
                                                    pDocSh->IsReadOnly()) ||
            (0 != (pSectNode = pSectNode->FindStartNode()->FindSectionNode())&&
                    pSectNode->GetSection().IsProtectFlag());

    }
    return bRet;
}
/* -----------------17.08.99 13:29-------------------

 --------------------------------------------------*/
const SfxItemSet* SwTOXBase::GetAttrSet() const
{
    const SwTOXBaseSection *pSect = PTR_CAST(SwTOXBaseSection, this);
    if(pSect && pSect->GetFmt())
        return &pSect->GetFmt()->GetAttrSet();
    return 0;
}

void SwTOXBase::SetAttrSet( const SfxItemSet& rSet )
{
    SwTOXBaseSection *pSect = PTR_CAST(SwTOXBaseSection, this);
    if( pSect && pSect->GetFmt() )
        pSect->GetFmt()->SetAttr( rSet );
}

BOOL SwTOXBase::GetInfo( SfxPoolItem& rInfo ) const
{
    switch( rInfo.Which() )
    {
    case RES_CONTENT_VISIBLE:
        {
            SwTOXBaseSection *pSect = PTR_CAST(SwTOXBaseSection, this);
            if( pSect && pSect->GetFmt() )
                pSect->GetFmt()->GetInfo( rInfo );
        }
        return FALSE;
    }
    return TRUE;
}

/*  */


