/*************************************************************************
 *
 *  $RCSfile: ww8par.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: cmc $ $Date: 2001-04-02 08:58:16 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#define _SVSTDARR_STRINGSDTOR

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_CSCOITEM_HXX //autogen
#include <svx/cscoitem.hxx>
#endif

#ifndef _SVX_SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif

#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx> // SvxLRSpaceItem
#endif
#ifndef _MSDFFIMP_HXX
#include <svx/msdffimp.hxx>
#endif

#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif

#ifndef _SVXMSBAS_HXX
#include <svx/svxmsbas.hxx>
#endif

#ifndef _SVX_UNOAPI_HXX_
#include <svx/unoapi.hxx>
#endif

#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif

#ifdef DEBUG
#   ifndef _SOUND_HXX //autogen
#       include <vcl/sound.hxx>
#   endif
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _OFA_FLTRCFG_HXX
#include <offmgr/fltrcfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTCNCT_HXX
#include <fmtcnct.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _WW8PAR2_HXX
#include <ww8par2.hxx>          // class WW8RStyle, class WW8AnchorPara
#endif
#ifndef _WW8PAR_HXX
#include <ww8par.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>              // fuer SwPam
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>            // class SwTxtNode
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>         // class SwPageDesc
#endif
#ifndef _NUMRULE_HXX //autogen
#include <numrule.hxx>
#endif
#ifndef _PARATR_HXX //autogen
#include <paratr.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>           // ReadFilterFlags
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _DOCUFLD_HXX //autogen
#include <docufld.hxx>
#endif
#ifndef _SWFLTOPT_HXX
#include <swfltopt.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _VIEWSH_HXX     // for the pagedescname from the ShellRes
#include <viewsh.hxx>
#endif
#ifndef _SHELLRES_HXX   // for the pagedescname from the ShellRes
#include <shellres.hxx>
#endif

#ifdef DEBUG
#ifndef _DOCARY_HXX //autogen
#include <docary.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif

#endif

#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // Progress
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>          // ResId fuer Statusleiste
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>            // ERR_WW8_...
#endif

#ifndef _COM_SUN_STAR_I18N_FORBIDDENCHARACTERS_HPP_
#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#endif
using namespace ::com::sun::star;


//-----------------------------------------
//              diverses
//-----------------------------------------


#define MM_250 1417             // WW-Default fuer Hor. Seitenraender: 2.5 cm
#define MM_200 1134             // WW-Default fuer u.Seitenrand: 2.0 cm





SwMSDffManager::SwMSDffManager( SwWW8ImplReader& rRdr )
    : SvxMSDffManager( *rRdr.pTableStream,
                        rRdr.pWwFib->fcDggInfo,
                        rRdr.pDataStream, 0, 0,
                        COL_WHITE, 12 /* in Point */,
                        rRdr.pStrm ),
    rReader( rRdr )
{
    nSvxMSDffOLEConvFlags = SwMSDffManager::GetFilterFlags();
}

UINT32 SwMSDffManager::GetFilterFlags()
{
    UINT32 nFlags = 0;
    const OfaFilterOptions* pOpt = OFF_APP()->GetFilterOptions();
    if( pOpt->IsMathType2Math() )
        nFlags |= OLE_MATHTYPE_2_STARMATH;
/*
    // !! don't convert the OLE-Object into the own format
    if( pOpt->IsWinWord2Writer() )
        nFlags |= OLE_WINWORD_2_STARWRITER;
*/
    if( pOpt->IsExcel2Calc() )
        nFlags |= OLE_EXCEL_2_STARCALC;
    if( pOpt->IsPowerPoint2Impress() )
        nFlags |= OLE_POWERPOINT_2_STARIMPRESS;
    return nFlags;
}


/*
 * I would like to override the default OLE importing to add a test
 * and conversion of OCX controls from their native OLE type into our
 * native nonOLE Form Control Objects.
 *
 * cmc
 */
SdrObject* SwMSDffManager::ImportOLE( long nOLEId, const Graphic& rGrf,
                                        const Rectangle& rBoundRect ) const
{
    SdrObject* pRet = 0;
    String sStorageName;
    SvStorageRef xSrcStg, xDstStg;
    if( GetOLEStorageName( nOLEId, sStorageName, xSrcStg, xDstStg ))
    {
        SvStorageRef xSrc = xSrcStg->OpenStorage( sStorageName,
            STREAM_READWRITE| STREAM_SHARE_DENYALL );
        ASSERT(rReader.pFormImpl, "No Form Implementation!");
        if (rReader.pFormImpl->ReadOCXStream(xSrc,&xShape,TRUE))
            pRet = GetSdrObjectFromXShape(xShape);
        else
            pRet = CreateSdrOLEFromStorage( sStorageName, xSrcStg, xDstStg,
                rGrf, rBoundRect, pStData, nSvxMSDffOLEConvFlags );
    }
    return pRet;
}


/***************************************************************************
#  Spezial FastSave - Attribute
#**************************************************************************/

#if 0
typedef BYTE Bit256[32];

inline BOOL GetBit( Bit256& rBits, BYTE nBitNo )
{
    return ( rBits[nBitNo >> 3] >> ( nBitNo & 0x7 ) ) & 0x1;
}

inline void SetBit( Bit256& rBits, BYTE nBitNo, BOOL bInp )
{
    if( bInp )
        rBits[nBitNo >> 3] |= 1 << ( nBitNo & 0x7 );
    else
        rBits[nBitNo >> 3] &= ~( 1 << ( nBitNo & 0x7 ) );
}
#endif

void SwWW8ImplReader::Read_StyleCode( USHORT, BYTE* pData, short nLen )
{
    if( nLen < 0 )
    {
        bCpxStyle = FALSE;
        return;
    }
    INT16 nColl = SVBT16ToShort( pData );
    if( (USHORT)nColl < nColls )
    {
        SetTxtFmtCollAndListLevel( *pPaM, pCollA[ nColl ] );
        bCpxStyle = TRUE;
    }
}

// Read_Majority ist fuer Majority ( 103 ) und Majority50 ( 108 )
void SwWW8ImplReader::Read_Majority( USHORT, BYTE* pData, short nLen )
{
    if( nLen < 0 ){
        return;
    }
}

//-----------------------------------------
//           temp. Test
//-----------------------------------------

long SwWW8ImplReader::Read_Piece( WW8PLCFManResult* pRes, BOOL bStartAttr )
{
    if( !bStartAttr )
        return 0;
//  pStrm->Seek( WW8Cp2Fc( pRes->nCp2OrIdx, 0 ) );  // nAktCp   //!!!Mogel
    return 0;
}



//-----------------------------------------
//            Stack
//-----------------------------------------
BOOL SwWW8FltControlStack::IsFtnEdnBkmField(SwFmtFld& rFmtFld, USHORT& nBkmNo)
{
    const SwField* pFld = rFmtFld.GetFld();
    USHORT nSubType;
    return(    pFld
            && (RES_GETREFFLD == pFld->Which())
            && (   (REF_FOOTNOTE == (nSubType = pFld->GetSubType()))
                || (REF_ENDNOTE  == nSubType))
            && ((SwGetRefField*)pFld)->GetSetRefName().Len()
                // find Sequence No of corresponding Foot-/Endnote
            && (USHRT_MAX != (nBkmNo = pDoc->FindBookmark(
                                ((SwGetRefField*)pFld)->GetSetRefName() ))));
}

void SwWW8FltControlStack::NewAttr(const SwPosition& rPos, const SfxPoolItem& rAttr)
{
    USHORT nBkmNo;
    if(    (RES_TXTATR_FIELD == rAttr.Which())
        && IsFtnEdnBkmField((SwFmtFld&)rAttr, nBkmNo) )
    {
        SwFltStackEntry *pTmp = new SwFltStackEntry(rPos, rAttr.Clone());
        Insert(pTmp, Count());
    }
    else
        SwFltControlStack::NewAttr(rPos, rAttr);
}

void SwWW8FltControlStack::SetAttrInDoc(const SwPosition& rTmpPos, SwFltStackEntry* pEntry)
{
    switch( pEntry->pAttr->Which() )
    {
        case RES_LR_SPACE:
        {
            SwPaM aRegion( rTmpPos );
            if (pEntry->MakeRegion(pDoc, aRegion, FALSE))
            {
                SvxLRSpaceItem aLR( *(SvxLRSpaceItem*)pEntry->pAttr );
                BOOL bChange1stLine = 1 == aLR.GetTxtFirstLineOfst();
                ULONG nStart = aRegion.Start()->nNode.GetIndex();
                ULONG nEnd   = aRegion.End()->nNode.GetIndex();
                const SwNumRule* pRule;
                const SwNodeNum* pNum;
                for(; nStart <= nEnd; ++nStart)
                {
                    SwNode* pNode = pDoc->GetNodes()[ nStart ];
                    if( pNode->IsTxtNode() )
                    {
                        if( bChange1stLine )
                        {
                            if(    0 != (pNum = ((SwTxtNode*)pNode)->GetNum() )
                                && (MAXLEVEL > pNum->GetLevel())
                                && 0 != (pRule = ((SwTxtNode*)pNode)->GetNumRule() ) )
                            {
                                const SwNumFmt rNumFmt = pRule->Get( pNum->GetLevel() );
                                aLR.SetTxtFirstLineOfst( rNumFmt.GetFirstLineOffset() );
                            }
                            else
                                aLR.SetTxtFirstLineOfst( 0 );
                        }
                        ((SwCntntNode*)pNode)->SetAttr( aLR );

                        // wenn wir dies nicht tun, ueberschreibt die NumRule uns alle
                        // harten L-Randeinstellungen
                        pNode->SetNumLSpace( FALSE );
                    }
                }
            }
        }
        break;
        case RES_TXTATR_FIELD:
        {
            SwFmtFld& rFmtFld   = *(SwFmtFld*)pEntry->pAttr;
            const SwField* pFld = rFmtFld.GetFld();
            USHORT nBkmNo;
            if( IsFtnEdnBkmField(rFmtFld, nBkmNo) )
            {
                SwBookmark& rBkMrk = pDoc->GetBookmark( nBkmNo );

                const SwPosition& rBkMrkPos = rBkMrk.GetPos();

                SwTxtNode* pTxt = rBkMrkPos.nNode.GetNode().GetTxtNode();
                if( pTxt && rBkMrkPos.nContent.GetIndex() )
                {
                    SwTxtAttr* pFtn = pTxt->GetTxtAttr( rBkMrkPos.nContent.GetIndex()-1,
                                                        RES_TXTATR_FTN );
                    if( pFtn )
                    {
                        USHORT nRefNo = ((SwTxtFtn*)pFtn)->GetSeqRefNo();

                        ((SwGetRefField*)pFld)->SetSeqNo( nRefNo );

                        if( pFtn->GetFtn().IsEndNote() )
                            ((SwGetRefField*)pFld)->SetSubType( REF_ENDNOTE );
                    }
                }
                /*
                const SwStartNode* pSearchNode =
                  rBkMrkPos.nNode.GetNode().FindFootnoteStartNode();

                if( pSearchNode )
                {
                    const SwFtnIdxs& rFtnIdxs = pDoc->GetFtnIdxs();

                    const USHORT nFtnCnt = rFtnIdxs.Count();

                    for(USHORT n = 0; n < nFtnCnt; ++n )
                    {
                        SwTxtFtn* pFtn = rFtnIdxs[ n ];
                        const SwNodeIndex* pSttIdx =
                            ((SwTxtFtn*)pFtn)->GetStartNode();
                        if( pSttIdx &&
                            (pSearchNode ==
                                pSttIdx->GetNode().GetStartNode()) )
                        {
                            USHORT nRefNo = pFtn->SetSeqRefNo();

                            ((SwGetRefField*)pFld)->SetSeqNo( nRefNo );

                        }
                    }
                }
                */
            }
            SwNodeIndex aIdx( pEntry->nMkNode, +1 );
            SwPaM aPaM( aIdx, pEntry->nMkCntnt );
            pDoc->Insert(aPaM, *pEntry->pAttr);
        }
        break;
/*
        case RES_TXTATR_INETFMT:
        {
            if( rReader.pSBase )
            {
                if( rReader.pSBase->pFldPLCF )
                {
                    WW8PLCFx_FLD& rPLCF = *rReader.pSBase->pFldPLCF;
                    if( rPLCF.SeekPos( pEntry->nCPStart ) )
                    {
                        const ULONG nIdxPoint = rPLCF.GetIdx();
                        if( rPLCF.SeekPos( pEntry->nCPEnd ) )
                        {
                            const ULONG nIdxMark = rPLCF.GetIdx();
                            if( nIdxPoint == nIdxMark )
                            {
                                ;
                            }
                        }
                    }
                }
            }
        }
        break;
*/
        default: SwFltControlStack::SetAttrInDoc(rTmpPos, pEntry);
    }
}



//-----------------------------------------
//            Tabs
//-----------------------------------------
#define DEF_TAB_ANZ 13          // So viele Default-Tabs

#if 0           // Nach MA 5.8.96 nicht mehr noetig
void SwWW8ImplReader::SetImplicitTab()
{
    SvxTabStopItem aTabs( *(SvxTabStopItem*)GetFmtAttr( RES_PARATR_TABSTOP ));
    aTabs.Insert( SvxTabStop( 0, SVX_TAB_ADJUST_DEFAULT ) );
    NewAttr( aTabs );
}
#endif

void SwWW8ImplReader::Read_Tab( USHORT nId, BYTE* pData, short nLen )
{
    if( nLen < 0 ){
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_TABSTOP );
        return;
    }

    // Para L-Space
    short nLeftPMgn    = ( pAktColl ) ? pCollA[nAktColl].nLeftParaMgn
                                      : nLeftParaMgn;

    // Para L-Space OR Left Margin of Para's first line respectively
    short nLeftMostPos = ( pAktColl ) ? pCollA[nAktColl].nTxtFirstLineOfst
                                      : nTxtFirstLineOfst;
    if( 0 < nLeftMostPos )
        nLeftMostPos = 0;

    short i;
    BYTE* pDel = pData + 1;                     // Del - Array
    BYTE nDel = pData[0];
    BYTE* pIns = pData + 2*nDel + 2;            // Ins - Array
    BYTE nIns = pData[nDel*2+1];
    WW8_TBD* pTyp = (WW8_TBD*)(pData + 2*nDel + 2*nIns + 2);// Typ - Array

    SvxTabStopItem aAttr( 0, 0, SVX_TAB_ADJUST_DEFAULT );

    SwTxtFmtColl* pSty = 0;
    USHORT nTabBase;
    if( pAktColl ){                             // StyleDef
        nTabBase = pCollA[nAktColl].nBase;
        if( nTabBase < nColls ){                // Based On
            pSty = (SwTxtFmtColl*)pCollA[nTabBase].pFmt;
        }
    }else{                                      // Text
        nTabBase = nAktColl;
        pSty = (SwTxtFmtColl*)pCollA[nAktColl].pFmt;
    }

    BOOL bFound = FALSE;
    while( pSty && !bFound ){
        const SfxPoolItem* pTabs;
        bFound = pSty->GetAttrSet().GetItemState( RES_PARATR_TABSTOP, FALSE, &pTabs )
                   == SFX_ITEM_SET;
        if( bFound ){
            aAttr = *((const SvxTabStopItem*)pTabs);
        }else{
            if( nTabBase < nColls                   // Based On
                &&  (nTabBase = pCollA[nTabBase].nBase) < nColls )                  // Based On
                pSty = (SwTxtFmtColl*)pCollA[nTabBase].pFmt;
            else
                pSty = 0;                           // gib die Suche auf
        }
    }

    SvxTabStop aTabStop;

    for( i=0; i<nDel; i++ ){
        USHORT nPos = aAttr.GetPos( SVBT16ToShort( pDel + i*2 ) - nLeftPMgn );
        if( nPos != SVX_TAB_NOTFOUND )
            aAttr.Remove( nPos, 1 );
    }
    for( i=0; i<nIns; i++ )
    {
        short nPos = SVBT16ToShort( pIns + i*2 ) - nLeftPMgn;
        if( nPos < nLeftMostPos )
            continue;
        aTabStop.GetTabPos() = nPos;
        switch( SVBT8ToByte( pTyp[i].aBits1 ) & 0x7 ){      // pTyp[i].jc
        case 0: aTabStop.GetAdjustment() = SVX_TAB_ADJUST_LEFT; break;
        case 1: aTabStop.GetAdjustment() = SVX_TAB_ADJUST_CENTER; break;
        case 2: aTabStop.GetAdjustment() = SVX_TAB_ADJUST_RIGHT; break;
        case 3: aTabStop.GetAdjustment() = SVX_TAB_ADJUST_DECIMAL; break;
        case 4: continue;                   // ignoriere Bar
        }

        switch( SVBT8ToByte( pTyp[i].aBits1 ) >> 3 & 0x7 ){ // pTyp[i].tlc
        case 0: aTabStop.GetFill() = ' '; break;
        case 1: aTabStop.GetFill() = '.'; break;
        case 2: aTabStop.GetFill() = '-'; break;
        case 3:
        case 4: aTabStop.GetFill() = '_'; break;
        }

        USHORT nPos2 = aAttr.GetPos( nPos );
        if( nPos2 != SVX_TAB_NOTFOUND )
            aAttr.Remove( nPos2, 1 );       // sonst weigert sich das Insert()

        aAttr.Insert( aTabStop );
    }
    NewAttr( aAttr );
}

//-----------------------------------------
//              DOP
//-----------------------------------------

void SwWW8ImplReader::ImportDop( BOOL bNewDoc )
{
    if( bNewDoc )
    {
        // correct the LastPrinted date in DocumentInfo
        if( rDoc.GetpInfo() )
        {
            DateTime aLastPrinted( WW8ScannerBase::WW8DTTM2DateTime( pWDop->dttmLastPrint ));
            SfxDocumentInfo* pNeuDocInf = new SfxDocumentInfo( *rDoc.GetpInfo() );
            SfxStamp aPrinted( pNeuDocInf->GetPrinted() );
            if( aPrinted.GetTime() != aLastPrinted)
            {
                // check if WW8 date was set
                if( aLastPrinted == DateTime(Date( 0 ), Time( 0 )))
                    // create "invalid" value for SfxStamp
                    // (as seen in sfx2/DOSINF.HXX)
                    aPrinted.SetTime(DateTime(Date( 1, 1, 1601 ), Time( 0, 0, 0 )));
                else
                    aPrinted.SetTime( aLastPrinted );
                pNeuDocInf->SetPrinted( aPrinted );
                rDoc.SetInfo( *pNeuDocInf );
                delete( pNeuDocInf );
            }
        }

        // Import Default-Tabs
        long nDefTabSiz = pWDop->dxaTab;
        if( nDefTabSiz < 56 )
            nDefTabSiz = 709;

        // wir wollen genau einen DefaultTab
        SvxTabStopItem aNewTab( 1, USHORT(nDefTabSiz), SVX_TAB_ADJUST_DEFAULT );
        ((SvxTabStop&)aNewTab[0]).GetAdjustment() = SVX_TAB_ADJUST_DEFAULT;

        rDoc.GetAttrPool().SetPoolDefaultItem( aNewTab );

        // set default language (from FIB)
        rDoc.GetAttrPool().SetPoolDefaultItem(
            SvxLanguageItem( (const LanguageType)pWwFib->lid )  );

        //import magic doptypography information, if its there
        if (pWwFib->nFib > 105)
            ImportDopTypography(pWDop->doptypography);
    }
}

void SwWW8ImplReader::ImportDopTypography(const WW8DopTypography &rTypo)
{
    switch (rTypo.iLevelOfKinsoku)
    {
#if 0   /*
        Do the defaults differ between Microsoft versions ?, do we do
        something about it if so ?
        */
        case 0:
            if (pWwFib->nFib < 2000)
                //use old defaults ??
            else
                //use new defaults ??
            break;
        case 1:
            if (pWwFib->nFib < 2000)
                //use old defaults ??
            else
                //use new defaults ??
            break;
#endif
        case 2: //custom
            {
            i18n::ForbiddenCharacters aForbidden(rTypo.rgxchFPunct,
                rTypo.rgxchLPunct);
            rDoc.SetForbiddenCharacters(rTypo.GetConvertedLang(),aForbidden);
            //Obviously cannot set the standard level 1 for japanese, so bail out
            //now while we can.
            if (rTypo.GetConvertedLang() == LANGUAGE_JAPANESE)
                return;
            }
            break;
        default:
            break;
    }

    /*
    This MS hack means that level 2 of japanese is not in operation, so we put
    in what we know are the MS defaults, there is a complementary reverse
    hack in the writer. Its our default as well, but we can set it anyway
    as a flag for later.
    */
    if (!rTypo.reserved2)
    {
        i18n::ForbiddenCharacters aForbidden(
            WW8DopTypography::aJapanNotBeginLevel1,
            WW8DopTypography::aJapanNotEndLevel1);
        rDoc.SetForbiddenCharacters(LANGUAGE_JAPANESE,aForbidden);
    }
}


//-----------------------------------------
//      Fuss- und Endnoten
//-----------------------------------------

WW8ReaderSave::WW8ReaderSave( SwWW8ImplReader* pRdr ,WW8_CP nStartCp)
: aTmpPos(*pRdr->pPaM->GetPoint())
{
    pWFlyPara       = pRdr->pWFlyPara;
    pSFlyPara       = pRdr->pSFlyPara;
    pTableDesc      = pRdr->pTableDesc;
    cSymbol         = pRdr->cSymbol;
    bSymbol         = pRdr->bSymbol;
    bIgnoreText     = pRdr->bIgnoreText;
    bDontCreateSep  = pRdr->bDontCreateSep;
    bHdFtFtnEdn     = pRdr->bHdFtFtnEdn;
    bApo            = pRdr->bApo;
    bTxbxFlySection = pRdr->bTxbxFlySection;
    bTable          = pRdr->bTable ;
    bTableInApo     = pRdr->bTableInApo;
    bAnl            = pRdr->bAnl;
    bNeverCallProcessSpecial = pRdr->bNeverCallProcessSpecial;
    nAktColl        = pRdr->nAktColl;
    nNoAttrScan     = pRdr->pSBase->GetNoAttrScan();

                                    // Tracking beginnt neu
    pRdr->bHdFtFtnEdn = TRUE;
    pRdr->bApo = pRdr->bTxbxFlySection
               = pRdr->bTable = bTableInApo = pRdr->bAnl = FALSE;
    pRdr->pWFlyPara = 0;
    pRdr->pSFlyPara = 0;
    pRdr->pTableDesc = 0;

    // schliesse Attribute auf dem End-Stack
    pRdr->pEndStck->SetAttr( *pRdr->pPaM->GetPoint(), 0, FALSE );

    pOldStck = pRdr->pCtrlStck;
    pRdr->pCtrlStck = new SwWW8FltControlStack(&pRdr->rDoc, pRdr->nFieldFlags, *pRdr);

    // rette die Attributverwaltung: dies ist noetig, da der neu anzulegende
    // PLCFx Manager natuerlich auf die gleichen FKPs zugreift, wie der alte
    // und deren Start-End-Positionen veraendert...
    pRdr->pPlcxMan->SaveAllPLCFx( aPLCFxSave );

    pOldPlcxMan = pRdr->pPlcxMan;

    if (nStartCp != -1)
        pRdr->pPlcxMan = new WW8PLCFMan( pRdr->pSBase,
            pOldPlcxMan->GetManType(), nStartCp );
    pRdr->pSBase->SetNoAttrScan( 0 );
}

void WW8ReaderSave::Restore( SwWW8ImplReader* pRdr )
{
    pRdr->pWFlyPara     = pWFlyPara;
    pRdr->pSFlyPara     = pSFlyPara;
    pRdr->pTableDesc    = pTableDesc;
    pRdr->cSymbol       = cSymbol;
    pRdr->bSymbol       = bSymbol;
    pRdr->bIgnoreText   = bIgnoreText;
    pRdr->bDontCreateSep= bDontCreateSep;
    pRdr->bHdFtFtnEdn   = bHdFtFtnEdn;
    pRdr->bApo          = bApo;
    pRdr->bTxbxFlySection=bTxbxFlySection;
    pRdr->bTable        = bTable ;
    pRdr->bTableInApo   = bTableInApo;
    pRdr->bAnl          = bAnl;
    pRdr->bNeverCallProcessSpecial = bNeverCallProcessSpecial;
    pRdr->nAktColl      = nAktColl;
    pRdr->pSBase->SetNoAttrScan( nNoAttrScan );

    // schliesse alle Attribute, da sonst Attribute
    // entstehen koennen, die aus dem Fly rausragen
    pRdr->pCtrlStck->SetAttr( *pRdr->pPaM->GetPoint(), 0, FALSE );
    pRdr->pEndStck->SetAttr(  *pRdr->pPaM->GetPoint(), 0, FALSE );

    pRdr->DeleteCtrlStk();
    pRdr->pCtrlStck = pOldStck;

    *pRdr->pPaM->GetPoint() = aTmpPos;

    DELETEZ( pRdr->pPlcxMan );

    // restauriere die Attributverwaltung
    pRdr->pPlcxMan = pOldPlcxMan;
    pRdr->pPlcxMan->RestoreAllPLCFx( aPLCFxSave );
}

void SwWW8ImplReader::Read_HdFtFtnText( const SwNodeIndex* pSttIdx, long nStartCp,
                                long nLen, short nType )
{
    WW8ReaderSave aSave( this );        // rettet Flags u.ae. u. setzt sie zurueck

    pPaM->GetPoint()->nNode = pSttIdx->GetIndex() + 1;      //
    pPaM->GetPoint()->nContent.Assign( pPaM->GetCntntNode(), 0 );

    // dann Text fuer Header, Footer o. Footnote einlesen

    ReadText( nStartCp, nLen, nType );              // Sepx dabei ignorieren
    aSave.Restore( this );
}

long SwWW8ImplReader::Read_Ftn( WW8PLCFManResult* pRes, BOOL )
{
    BOOL bFtEdOk = FALSE;

    if( nIniFlags & WW8FL_NO_FTN )
        return 0;

#if 0
    if ( pPaM->GetPoint()->nNode < rDoc.GetNodes().GetEndOfExtras().GetIndex() )
#else
    /*
    #84095#
    Ignoring Footnote outside of the normal Text. People will put footnotes
    into field results and field commands.
    */
    if (bIgnoreText ||
        pPaM->GetPoint()->nNode < rDoc.GetNodes().GetEndOfExtras().GetIndex())
#endif
        return 0;

    USHORT nType;
    BOOL bAutoNum = TRUE;
    if( 257 == pRes->nSprmId )
    {
        nType = MAN_EDN;
        if( pPlcxMan->GetEdn() )
            bAutoNum = 0 != *(short*)pPlcxMan->GetEdn()->GetData();
    }
    else
    {
        nType = MAN_FTN;
        if( pPlcxMan->GetFtn() )
            bAutoNum = 0 != *(short*)pPlcxMan->GetFtn()->GetData();
    }

    WW8PLCFxSaveAll aSave;
    pPlcxMan->SaveAllPLCFx( aSave );
    WW8PLCFMan* pOldPlcxMan = pPlcxMan;

    SwFmtFtn aFtn( 257 == pRes->nSprmId ) ;         // erzeuge Fussnote
    rDoc.Insert( *pPaM, aFtn );

    SwPosition aTmpPos( *pPaM->GetPoint() );    // merke alte Cursorposition

    pPaM->Move( fnMoveBackward, fnGoCntnt );    // hole Index auf Fussnoteninhalt
    SwTxtNode* pTxt = pPaM->GetNode()->GetTxtNode();
    SwTxtAttr* pFN = pTxt->GetTxtAttr( pPaM->GetPoint()->nContent.GetIndex(),
                                        RES_TXTATR_FTN );
    ASSERT(pFN, "Probleme beim Anlegen des Fussnoten-Textes");
    if( pFN )
    {
        const SwNodeIndex* pSttIdx = ((SwTxtFtn*)pFN)->GetStartNode();
        ASSERT(pSttIdx, "Probleme beim Anlegen des Fussnoten-Textes");

        ((SwTxtFtn*)pFN)->SetSeqNo( rDoc.GetFtnIdxs().Count() );

        BOOL bOld = bFtnEdn;
        bFtnEdn = TRUE;

        // read content of Ft-/End-Note
        Read_HdFtFtnText( pSttIdx, pRes->nCp2OrIdx, pRes->nMemLen, nType );
        bFtEdOk = TRUE;
        bFtnEdn = bOld;

        // falls keine automatische Numerierung eingestellt ist, so hole
        // das 1. Zeichen aus der Fuss-/End-Note und setze das als Zeichen
        if( !bAutoNum )
        {
            SwNodeIndex& rNIdx = pPaM->GetPoint()->nNode;
            rNIdx = pSttIdx->GetIndex() + 1;
            SwTxtNode* pTNd = rNIdx.GetNode().GetTxtNode();
            if( pTNd )
            {
                String sNo( pTNd->GetTxt().GetChar( 0 ));
                ((SwTxtFtn*)pFN)->SetNumber( 0, &sNo );
                pPaM->GetPoint()->nContent.Assign( pTNd, 0 );
                pPaM->SetMark();
                pPaM->GetMark()->nContent++;
                rDoc.Delete( *pPaM );
                pPaM->DeleteMark();
            }
        }
    }
    *pPaM->GetPoint() = aTmpPos;                // restore Cursor

    pPlcxMan = pOldPlcxMan;             // Attributverwaltung restoren
    pPlcxMan->RestoreAllPLCFx( aSave );

    if( bSymbol )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_FONT );
        bSymbol = FALSE;
    }

    // insert Section to get this Ft-/End-Note at the end of the section,
    // when there is no open section at the moment
       if( bFtEdOk && pLastPgDeskIdx && !pBehindSection )
    {
        const SwNodeIndex aOrgLastPgDeskIdx( *pLastPgDeskIdx );

        (*pLastPgDeskIdx)++;
        SwPosition aSectStart( *pLastPgDeskIdx );
        aSectStart.nContent.Assign( pLastPgDeskIdx->GetNode().GetCntntNode(), 0 );

        SwPaM aSectPaM( aSectStart, *pPaM->GetPoint() );
        InsertSectionWithWithoutCols( aSectPaM, 0 );
        pPaM->Move( fnMoveBackward );
        DELETEZ( pLastPgDeskIdx );
        // set attributes to correct position
        pCtrlStck->MoveAttrsToNextNode( aOrgLastPgDeskIdx );
    }

    return 1;       // das Fussnotenzeichen ueberlesen!
}

// JP 03.12.98 - Anmerkungen einlesen
// - Als default wird erstmal das Initial als Author benutzt. Im Writer wird
//  auch immer das Initial benutzt!
#undef AUTHOR_AS_AUTHOR
#define INITIAL_AS_AUTHOR

long SwWW8ImplReader::Read_And( WW8PLCFManResult* pRes, BOOL )
{
    WW8PLCFx_SubDoc* pSD = pPlcxMan->GetAtn();
    if( !pSD )
        return 0;

    String sAuthor;
    if( bVer67 )
    {
        const WW67_ATRD* pDescri = (WW67_ATRD*)pSD->GetData();
#ifdef AUTHOR_AS_AUTHOR
        const String* pA = GetAnnotationAuthor( SVBT16ToShort( pDescri->ibst ) );
        if( pA )
            sAuthor = *pA;
#endif

#ifdef INITIAL_AS_AUTHOR
        sAuthor = String( pDescri->xstUsrInitl + 1,
                        (USHORT)pDescri->xstUsrInitl[0] );
#endif
    }
    else
    {
        const WW8_ATRD* pDescri = (const WW8_ATRD*)pSD->GetData();

#ifdef AUTHOR_AS_AUTHOR
        const String* pA = GetAnnotationAuthor( SVBT16ToShort( pDescri->ibst ) );
        if( pA )
            sAuthor = *pA;
#endif

#ifdef INITIAL_AS_AUTHOR
        BYTE nLen = (BYTE)SVBT16ToShort( pDescri->xstUsrInitl[0] );
        String aTmp;
        sal_Unicode* pData = aTmp.AllocBuffer( nLen );
        sal_Unicode* pWork = pData;

        for( BYTE nIdx = 1; nIdx <= nLen; ++nIdx, ++pWork )
            *pWork = SVBT16ToShort( pDescri->xstUsrInitl[ nIdx ] );
//      {
//          UINT16 nChar = SVBT16ToShort( pDescri->xstUsrInitl[ nIdx ] );
//          if( 0xF000 == (nChar & 0xFF00))
//              nChar &= 0x00FF;
//          *pWStr = nChar;
//      }
//      sAuthor = String( aTmp, CHARSET_ANSI );     // ANSI???

// 2000/03/30 KHZ UNICODE
// still missing: unicode-back conversion AND 0xF0.. specials

#endif
    }

    WW8PLCFxSaveAll aSave;
    pPlcxMan->SaveAllPLCFx( aSave );
    WW8PLCFMan* pOldPlcxMan = pPlcxMan;

    SwNodeIndex aNdIdx( rDoc.GetNodes().GetEndOfExtras() );
    aNdIdx = *rDoc.GetNodes().MakeTextSection( aNdIdx, SwNormalStartNode,
                            rDoc.GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));

    SwPosition aTmpPos( *pPaM->GetPoint() );    // merke alte Cursorposition
    Read_HdFtFtnText( &aNdIdx, pRes->nCp2OrIdx, pRes->nMemLen, MAN_AND );
                                                // lese Inhalt ein
    *pPaM->GetPoint() = aTmpPos;                // restore Cursor

    // erzeuge das PostIt
    Date aDate;
    String sTxt;


    {   // Text aus den Nodes in den String uebertragen
        SwNodeIndex aIdx( aNdIdx, 1 ),
                    aEnd( *aNdIdx.GetNode().EndOfSectionNode() );
        SwTxtNode* pTxtNd;
        while( aIdx != aEnd )
        {
            if( 0 != ( pTxtNd = aIdx.GetNode().GetTxtNode() ))
            {
                if( sTxt.Len() )
                    sTxt += '\x0a';     // Zeilenumbruch
                sTxt += pTxtNd->GetExpandTxt();
            }
            aIdx++;
        }
        rDoc.DeleteSection( &aNdIdx.GetNode() );
    }

    rDoc.Insert( *pPaM, SwFmtFld( SwPostItField(
                    (SwPostItFieldType*)rDoc.GetSysFldType( RES_POSTITFLD ),
                    sAuthor, sTxt, aDate )));

    pPlcxMan = pOldPlcxMan;             // Attributverwaltung restoren
    pPlcxMan->RestoreAllPLCFx( aSave );
    return 0;
}

//-----------------------------------------
//      Header und Footer
//-----------------------------------------

void SwWW8ImplReader::Read_HdFtText( long nStartCp, long nLen, SwPageDesc* pPD,
                             BOOL bUseLeft, BOOL bFooter )
{
    SwFrmFmt* pFmt = ( bUseLeft ) ? &pPD->GetLeft() : &pPD->GetMaster();
    SwFrmFmt* pHdFtFmt;

    if( bFooter ){
        bIsFooter = TRUE;
        pFmt->SetAttr( SwFmtFooter( TRUE ) );
        pHdFtFmt = (SwFrmFmt*)pFmt->GetFooter().GetFooterFmt();
    }else{
        bIsHeader = TRUE;
        pFmt->SetAttr( SwFmtHeader( TRUE ) );
        pHdFtFmt = (SwFrmFmt*)pFmt->GetHeader().GetHeaderFmt();
    }

    const SwNodeIndex* pSttIdx = pHdFtFmt->GetCntnt().GetCntntIdx();
    if( !pSttIdx ) return;

    SwPosition aTmpPos( *pPaM->GetPoint() );    // merke alte Cursorposition

    Read_HdFtFtnText( pSttIdx, nStartCp, nLen - 1, MAN_HDFT );

    *pPaM->GetPoint() = aTmpPos;
    bIsHeader = bIsFooter = FALSE;
}

BYTE SwWW8ImplReader::HdFtCorrectPara( BYTE nPara )
{
    WW8_CP start;
    long nLen;
    BYTE nNewPara = nPara;

    for( BYTE nI = 0x20; nI; nI >>= 1 )
        if(     ( nI & nPara )
            &&  pHdFt
            &&  ( !pHdFt->GetTextPos( nPara, nI, start, nLen ) || nLen <= 2 ) )

        {
            nNewPara &= ~nI;        // leere KF-Texte nicht anlegen
        }

// Wenn im Doc keine Facing Pages vorhanden sind, sollen
// die geraden Header/Footer ignoriert werden.
// Die Facing Pages lassen sich nur Doc-weit,
// nicht aber abschnittsweise umschalten !
    if( pWDop->fFacingPages )
    {       // Wenn pDoc->FacingPages
//      if( nNewPara & WW8_HEADER_ODD )     // und ungerader Header vorhanden
//          nNewPara |= WW8_HEADER_EVEN;        // ... dann muss auch ein gerader
                                            // Header erzeugt werden
//      if( nNewPara & WW8_FOOTER_ODD )     // Footer ebenso.
//          nNewPara |= WW8_FOOTER_EVEN;
    }
    else
    {
        nNewPara &= ~( WW8_HEADER_EVEN |  WW8_FOOTER_EVEN ); // ergibt bEven = 0
    }
    return nNewPara;
}

void SwWW8ImplReader::Read_HdFt1( BYTE nPara, BYTE nWhichItems, SwPageDesc* pPD )
{
    if( pHdFt )
    {
        WW8_CP start;
        long nLen;
        BYTE nNumber = 5;

        for( BYTE nI = 0x20; nI; nI >>= 1, nNumber-- )
            if( nI & nWhichItems )
            {
                BOOL bOk = TRUE;
                if( bVer67 )
                    bOk = ( pHdFt->GetTextPos( nPara, nI, start, nLen ) && nLen > 2 );
                else
                {
                    pHdFt->GetTextPosExact(nNumber+ (nActSectionNo+1)*6, start, nLen);
                    bOk = ( 2 < nLen );
                }
                if( bOk )
                {
                    BOOL bUseLeft
                        = (nI & ( WW8_HEADER_EVEN | WW8_FOOTER_EVEN )) ? TRUE : FALSE;
                    BOOL bFooter
                        = (nI & ( WW8_FOOTER_EVEN | WW8_FOOTER_ODD | WW8_FOOTER_FIRST )) ? TRUE : FALSE;
                    Read_HdFtText( start, nLen, pPD, bUseLeft, bFooter );
                }
            }
    }
}

static BYTE ReadBSprm( const WW8PLCFx_SEPX* pSep, USHORT nId, BYTE nDefaultVal )
{
    BYTE* pS = pSep->HasSprm( nId );            // sprm da ?
    BYTE nVal = ( pS ) ? SVBT8ToByte( pS ) : nDefaultVal;
    return nVal;
}

void SwWW8ImplReader::SetHdFt( SwPageDesc* pPageDesc0, SwPageDesc* pPageDesc1,
                               const WW8PLCFx_SEPX* pSep, BYTE nIPara )
{
    if( !nCorrIhdt || ( pAktColl != 0 ) )   // Header / Footer nicht da
                                            // oder StyleDef
        return;

    long nOldDoc = pStrm->Tell(     );      // WW-Scanner: alles retten
    long nOldTbl = pTableStream->Tell();    // WW-Scanner: alles retten
    WW8PLCFxSaveAll aSave;
    pPlcxMan->SaveAllPLCFx( aSave );        // Attributverwaltung sichern
    WW8PLCFMan* pOldPlcxMan = pPlcxMan;

    if( !pPageDesc1 ){          // 1 Pagedesc reicht

        Read_HdFt1( nIPara,
                    nCorrIhdt & ~( WW8_HEADER_FIRST | WW8_FOOTER_FIRST ),
                    pPageDesc0 );

    }else{                      // 2 Pagedescs noetig: 1.Seite und folgende


                                // 1. Seite einlesen
        Read_HdFt1( nIPara,
                    nCorrIhdt & ( WW8_HEADER_FIRST | WW8_FOOTER_FIRST ),
                    pPageDesc0 );

                                // Folgeseiten einlesen
        Read_HdFt1( nIPara,
                    nCorrIhdt & ~( WW8_HEADER_FIRST | WW8_FOOTER_FIRST ),
                    pPageDesc1 );
    }
                                // dann WW-Scanner-Kram restoren
    pPlcxMan = pOldPlcxMan;             // Attributverwaltung restoren
    pPlcxMan->RestoreAllPLCFx( aSave );
    pStrm->Seek(      nOldDoc );                // FilePos dito (Sprms)
    pTableStream->Seek( nOldTbl );              // FilePos dito (Sprms)
}

//-----------------------------------------
//          PageDescs
//-----------------------------------------


void SwWW8ImplReader::SetLastPgDeskIdx()
{
    if(    0 == pWDop->epc      // store NodeIndex for adding section when FT-/End-Note
        || 2 == pWDop->fpc )    // found that shall be on end of section.
    {
        if(  pLastPgDeskIdx )
            *pLastPgDeskIdx = pPaM->GetPoint()->nNode;
        else
            pLastPgDeskIdx = new SwNodeIndex( pPaM->GetPoint()->nNode );
        (*pLastPgDeskIdx)--;
    }
}


SwPageDesc* SwWW8ImplReader::CreatePageDesc( SwPageDesc* pFirstPageDesc,
                                             SwPaM** ppPaMWanted )
{
    ASSERT( pFirstPageDesc || ppPaMWanted, "!pFirstPageDesc but NO ppPaMWanted" );


    BOOL bFollow = ( pFirstPageDesc != 0 );
    SwPageDesc* pNewPD;
    USHORT nPos;

    if(    bFollow
        && pFirstPageDesc->GetFollow() != pFirstPageDesc )
        return pFirstPageDesc;      // Fehler: hat schon Follow

    // compose name of PageDescriptor
    USHORT nPageDescCount = rDoc.GetPageDescCnt();
    nPos = rDoc.MakePageDesc( ViewShell::GetShellRes()->GetPageDescName(
                                        nPageDescCount, FALSE, bFollow ),
                        bFollow // && (pFirstPageDesc != &rDoc._GetPageDesc( 0 ))
                        ? pFirstPageDesc
                        : 0 );
    pNewPD = &rDoc._GetPageDesc( nPos );

    if ( bFollow ){             // Dieser ist der folgende von pPageDesc
        pFirstPageDesc->SetFollow( pNewPD );
        pNewPD->SetFollow( pNewPD );
    }
    else
    {                       // setze PgDesc-Attr ins Doc
        if( bApo || bTxbxFlySection )
        {               // PageDesc *muss* ausserhalb des Apo stehen
            if( pSFlyPara && pSFlyPara->pMainTextPos )
            {
                SwPaM aMyPaM( *pSFlyPara->GetMainTextPos() );
                if( 1 < nPageDescCount )
                    rDoc.AppendTxtNode( *aMyPaM.GetPoint() );
                SwFmtCol* pCol;
                RemoveCols(  *pNewPD, pCol );
                rDoc.Insert( aMyPaM, SwFmtPageDesc( pNewPD ) );
                if( ppPaMWanted )
                    *ppPaMWanted = new SwPaM( aMyPaM );
                if( pCol )
                {
                    InsertSectionWithWithoutCols( aMyPaM, pCol );
                    delete pCol;
                }
                else
                    SetLastPgDeskIdx();
            }
        }
        else
        {
            if( 0 < pPaM->GetPoint()->nContent.GetIndex() )
                rDoc.AppendTxtNode( *pPaM->GetPoint() );
            SwFmtCol* pCol;
            RemoveCols(  *pNewPD, pCol );
            if( ppPaMWanted )
                *ppPaMWanted = new SwPaM( *pPaM );

            rDoc.Insert( *pPaM, SwFmtPageDesc( pNewPD ) );
            if( pCol )
            {
                InsertSectionWithWithoutCols( *pPaM, pCol );
                delete pCol;
            }
            else
                SetLastPgDeskIdx();
        }
    }
    return pNewPD;
}

// UpdatePageDescs muss am Ende des Einlesevorganges aufgerufen werden, damit
// der Writer den Inhalt der Pagedescs wirklich akzeptiert
void SwWW8ImplReader::UpdatePageDescs( USHORT nPageDescOffset )
{
    USHORT i;

    // Pagedescriptoren am Dokument updaten (nur so werden auch die
    // linken Seiten usw. eingestellt).

    // PageDesc "Standard"
    rDoc.ChgPageDesc( 0, rDoc.GetPageDesc( 0 ));

    // PageDescs "Konvert..."
    for ( i=nPageDescOffset; i < rDoc.GetPageDescCnt(); i++ )
    {
        const SwPageDesc* pPD = &rDoc.GetPageDesc( i );
        rDoc.ChgPageDesc( i, *pPD );
    }
}

//-----------------------------------------
//              Text
//-----------------------------------------

// TestApo() ist die aus ProcessSpecial() herausgeloeste Apo-Abfrage.
// sie wird auch beim Aufbau der Tabellen-Struktur (ww8par6.cxx)
// verwendet.
// Die Parameter rbStartApo, rbStopApo und rbNowStyleApo sind reine
// Rueckgabeparameter
BYTE* SwWW8ImplReader::TestApo( BOOL& rbStartApo, BOOL& rbStopApo,
                                BOOL& rbNowStyleApo,
                                BOOL  bInTable,   BOOL bTableRowEnd,
                                BOOL  bStillInTable )
{
    BYTE* pSprm37;
    BYTE* pSprm29;
    rbNowStyleApo = (0 != pCollA[nAktColl].pWWFly); // Apo in StyleDef

    if( bInTable && rbNowStyleApo )
    {
        pSprm37       = 0;
        pSprm29       = 0;
        rbNowStyleApo = FALSE;
    }
    else
    {
        pSprm37 = pPlcxMan->HasParaSprm( bVer67 ? 37 : 0x2423 );
        pSprm29 = pPlcxMan->HasParaSprm( bVer67 ? 29 : 0x261B );
    }

    // here Apo
    BOOL bNowApo = rbNowStyleApo || pSprm29 || pSprm37;
    BOOL bApoContinuedInTabCell2ndParagraph
            = (bApo && bTableInApo && bStillInTable) && !bNowApo;
    bNowApo |= bApoContinuedInTabCell2ndParagraph;

    rbStartApo = bNowApo && !bApo && !bTableRowEnd; // normal APO-start
    rbStopApo  = bApo && !bNowApo && !bTableRowEnd; // normal APO-end

    if( bApo && bNowApo && !bTableRowEnd
        && !bApoContinuedInTabCell2ndParagraph
        && !TestSameApo( pSprm29, rbNowStyleApo ) )
    {
        rbStopApo = rbStartApo = TRUE;              // aneinandergrenzende APOs
    };
    return pSprm29;
}

BOOL SwWW8ImplReader::ProcessSpecial( BOOL bAllEnd, BOOL* pbReSync )    // Apo / Table / Anl
{
    if( bNeverCallProcessSpecial )
        return FALSE;

    *pbReSync = FALSE;
    if( bAllEnd ){
        if( bAnl )
            StopAnl();                  // -> bAnl = FALSE
        if( bTable && !bFtnEdn )        // Tabelle in FtnEdn nicht erlaubt
            StopTable();
        if( bApo )
            StopApo();
        bTable = bApo = FALSE;
        return FALSE;
    }

    BOOL bTableRowEnd = ( pPlcxMan->HasParaSprm( ( bVer67 ? 25 : 0x2417 )) != 0 );  // TabRowEnd

// es muss leider fuer jeden Absatz zuerst nachgesehen werden,
// ob sich unter den sprms
// das sprm 29 (bzw. 0x261B) befindet, das ein APO einleitet.
// Alle weiteren sprms  beziehen sich dann naemlich auf das APO und nicht
// auf den normalen Text drumrum.
// Dasselbe gilt fuer eine Tabelle ( sprm 24 (bzw. 0x2416) )
// und Anls ( sprm 13 ).
// WW: Tabelle in APO geht ( Beide Anfaende treten gleichzeitig auf )
// WW: APO in Tabelle geht nicht
// d.h. Wenn eine Tabelle Inhalt eines Apo ist, dann muss der
// Apo-Anfang zuerst bearbeitet werden, damit die Tabelle im Apo steht
// und nicht umgekehrt. Am Ende muss dagegen zuerst das Tabellenende
// bearbeitet werden, da die Apo erst nach der Tabelle abgeschlossen
// werden darf ( sonst wird das Apo-Ende nie gefunden ).
// Dasselbe gilt fuer Fly / Anl, Tab / Anl, Fly / Tab / Anl.
//
// Wenn die Tabelle in einem Apo steht, fehlen im TabRowEnd-Bereich
// die Apo-Angaben. Damit hier die Apo nicht beendet wird, wird
// ProcessApo dann nicht aufgerufen.

// KHZ: When there is a table inside the Apo the Apo-flags are also
//      missing for the 2nd, 3rd... paragraphs of each cell.


//  1st look for in-table flag
    BYTE* pSprm24 = pPlcxMan->HasParaSprm(  bVer67
                                          ? 24
                                          : 0x2416 ); // Flag: Absatz in(!) Tabelle

//  then look if we are in an Apo

    BOOL bStartApo, bStopApo, bNowStyleApo;
    BYTE* pSprm29 = TestApo( bStartApo, bStopApo, bNowStyleApo,
                                bTable, (bTableRowEnd && bTableInApo),
                                bTable && (pSprm24 != 0) );

//  look if we are in a Tabelle
    BOOL bStartTab = pSprm24 && !bTable && !bFtnEdn; // Table in FtnEdn nicht erlaubt

    BOOL bStopTab = bTable && (bWasTabRowEnd && !pSprm24) && !bFtnEdn;

    bWasTabRowEnd = FALSE;  // must be deactivated right here to prevent next
                            // WW8TabDesc::TableCellEnd() from making nonsense

    if( bTable && !bStopTab && ( bStartApo || bStopApo ) )
    {                                   // Wenn Apowechsel in Tabelle
        bStopTab = bStartTab = TRUE;    // ... dann auch neue Tabelle
    }

//          Dann auf Anl (Nummerierung) testen
//          und dann alle Ereignisse in der richtigen Reihenfolge bearbeiten

    if( bAnl && !bTableRowEnd )
    {
        BYTE* pSprm13 = pPlcxMan->HasParaSprm( 13 );
        if( pSprm13 )
        {                                   // Noch Anl ?
            BYTE nT = GetNumType( *pSprm13 );
            if( ( nT != WW8_Pause && nT != nWwNumType ) // Anl-Wechsel
                || bStopApo  || bStopTab                // erzwungenes Anl-Ende
                || bStartApo || bStartTab )
            {
                StopAnl();          // Anl-Restart ( = Wechsel ) ueber sprms
            }
            else
            {
                NextAnlLine( pSprm13 );                 // naechste Anl-Zeile
            }
        }
        else
        {                                           // Anl normal zuende
            StopAnl();                                  // Wirkliches Ende
        }
    }
    if( bStopTab )
    {
        StopTable();
        bTable = FALSE;
    }
    if( bStopApo )
    {
        StopApo();
        bApo = FALSE;
    }

    if( bStartApo && !( nIniFlags & WW8FL_NO_APO ) )
    {
        bApo = StartApo( pSprm29, bNowStyleApo );
        *pbReSync = TRUE;                   // nach StartApo ist ein ReSync
                                            // noetig ( eigentlich nur, falls
                                            // die Apo ueber eine FKP-Grenze
                                            // geht
    }
    if( bStartTab && !( nIniFlags & WW8FL_NO_TABLE ) ){
        if( bAnl )                          // Nummerierung ueber Zellengrenzen
            StopAnl();                      // fuehrt zu Absturz -> keine Anls
                                            // in Tabellen
        bTable = StartTable();
        *pbReSync = TRUE;                   // nach StartTable ist ein ReSync
                                            // noetig ( eigentlich nur, falls
                                            // die Tabelle ueber eine
                                            // FKP-Grenze geht
        bTableInApo = bTable && bApo;
    }
    return bTableRowEnd;
}


#if defined OS2
// eigentlich besser inline, aber das kann der BLC nicht
static UCHAR ConvOs2( UCHAR ch, CharSet eDst )
{
    switch( ch ){
    case 132:
    case 148: return ( eDst == CHARSET_IBMPC_865 ) ? '"' : 175;
                            // typographische "(links) gegen aehnliche
                            // im OS/2-Charset
    case 147: return ( eDst == CHARSET_IBMPC_865 ) ? '"' : 174;
    case 173:               // kurze, mittellange und lange Striche gegen Minus
    case 150:
    case 151: return  '-';
    case 130: return ',';
    case 145:
    case 146: return '\'';  // typographische ' gegen normale
    case 139: return '<';
    case 155: return '>';
    case 152: return '~';
    }                       // ansonsten macht noch TM AErger.

    return 0;               // kenn ick nich
}
#endif

// Returnwert: TRUE fuer keine Sonderzeichen
BOOL SwWW8ImplReader::ReadPlainChars( long& rPos, long nEnd, long nCpOfs )
{
    // Unicode-Flag neu setzen und notfalls File-Pos korrigieren
    // merke: Seek kostet nicht viel, da inline geprueft wird,
    //        ob die korrekte FilePos nicht schon erreicht ist.
    WW8_FC nStreamPos = pSBase->WW8Cp2Fc(nCpOfs+rPos, &bIsUnicode);
    pStrm->Seek( nStreamPos );

    // ammount of characters to read == length to next attribut
    ULONG nLen = nEnd - rPos;

    CharSet eSrcCharSet = ( eHardCharSet != RTL_TEXTENCODING_DONTKNOW )
                        ? eHardCharSet
                        : eFontSrcCharSet;

    // (re)alloc UniString data
    String sPlainCharsBuf;

    sal_Unicode* pWork = sPlainCharsBuf.AllocBuffer( nLen );

    // read the stream data
    BYTE   nBCode;
    UINT16 nUCode;
    for( ULONG nL2 = 0; nL2 < nLen; ++nL2, ++pWork )
    {
        if( bIsUnicode )
            *pStrm >> nUCode;   // unicode  --> read 2 bytes
        else
        {
            *pStrm >> nBCode;   // old code --> read 1 byte
            nUCode = nBCode;
        }

        if( 0 != pStrm->GetError() )
        {
            rPos = LONG_MAX-10;     // -> eof or other error
            sPlainCharsBuf.ReleaseBufferAccess( 0 );
            return TRUE;
        }

        if( (32 > nUCode) || (0xa0 == nUCode) )
        {
            pStrm->SeekRel( bIsUnicode ? -2 : -1 );
            sPlainCharsBuf.ReleaseBufferAccess( nL2 );
            break;              // Sonderzeichen < 32, == 0xa0 gefunden
        }

        if( bIsUnicode )
            *pWork = nUCode;
        else
            *pWork = ByteString::ConvertToUnicode( nBCode, eSrcCharSet );
    }
    if( sPlainCharsBuf.Len() )
        rDoc.Insert( *pPaM, sPlainCharsBuf );

    rPos += nL2;
    return nL2 >= nLen;
}


// Returnwert: TRUE fuer Zeilenende
BOOL SwWW8ImplReader::ReadChars( long& rPos, long nNextAttr, long nTextEnd, long nCpOfs )
{
    long nEnd = ( nNextAttr < nTextEnd ) ? nNextAttr : nTextEnd;

    if( bSymbol || bIgnoreText )
    {
        if( bSymbol )   // Spezialzeichen einfuegen
        {
            for(USHORT nCh = 0; nCh < nEnd - rPos; ++nCh)
                rDoc.Insert( *pPaM, cSymbol );
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_FONT );
        }
        pStrm->SeekRel( nEnd- rPos );
        rPos = nEnd;    // ignoriere bis Attributende
        return FALSE;
    }
    while( TRUE )
    {
        if( ReadPlainChars( rPos, nEnd, nCpOfs ) )
            return FALSE;                   // Fertig, kein CR

        BOOL bStartLine = ReadChar( rPos, nCpOfs );
        rPos++;
        if( bPgSecBreak || bStartLine || rPos == nEnd ) // CR oder Fertig
            return bStartLine;
    }
}

BOOL SwWW8ImplReader::ReadChar( long nPosCp, long nCpOfs )
{
    // Unicode-Flag neu setzen und notfalls File-Pos korrigieren
    // merke: Seek kostet nicht viel, da inline geprueft wird,
    //        ob die korrekte FilePos nicht schon erreicht ist.
    pStrm->Seek( pSBase->WW8Cp2Fc(nCpOfs+nPosCp, &bIsUnicode) );

    BYTE   nBCode;
    UINT16 nWCharVal;
    if( bIsUnicode )
        *pStrm >> nWCharVal;    // unicode  --> read 2 bytes
    else
    {
        *pStrm  >>  nBCode;     // old code --> read 1 byte
        nWCharVal = nBCode;
    }

    sal_Char cInsert = '\x0';
    BOOL bRet = FALSE;
    switch( nWCharVal )
    {
    case 0xe:  {
                    SwTxtNode* pNd = pPaM->GetCntntNode()->GetTxtNode();
                    if ( pNd )
                    {
                        const SfxItemSet* pSet = pNd->GetpSwAttrSet();
                        if ( pSet && ( SFX_ITEM_ON == pSet->GetItemState(
                                                    RES_PAGEDESC, FALSE ) ) )
                        {
                            rDoc.AppendTxtNode( *pPaM->GetPoint() );
                        }
                        else
                        {
                            const SwPosition* pPos  = pPaM->GetPoint();
                            const SwTxtNode* pSttNd = rDoc.GetNodes()[ pPos->nNode ]->GetTxtNode();
                            USHORT nCntPos          = pPos->nContent.GetIndex();
                            if( nCntPos && pSttNd->GetTxt().Len() )
                                rDoc.SplitNode( *pPos );
                        }
                    }
                    rDoc.Insert( *pPaM,     // column break
                        SvxFmtBreakItem( SVX_BREAK_COLUMN_BEFORE ) );
                }
                break;
    case 0x7:   TabCellEnd();       // table cell end (Flags abfragen!)
                if( bWasTabRowEnd )
                    pSBase->SetNoAttrScan( 0 );
                break;

    case 0xf:   if( !bSpec )        // "Satellit"
                    cInsert = '\xa4';
                break;

    case 0x14:  if( !bSpec )        // "Para-Ende"-Zeichen
                    cInsert = '\xb5';
                break;

    case 0x15:  if( !bSpec )        // Juristenparagraph
                    cInsert = '\xa7';
                break;

    case 0x9:   cInsert = '\x9';    // Tab
                break;

    case 0xb:   cInsert = '\xa';    // Hard NewLine
                break;

    case 0xc:   bPgSecBreak = TRUE;
                // new behavior: insert additional node only WHEN the Pagebreak
                // ( #74468# )   is in a NODE that is NOT EMPTY
                if( 0 < pPaM->GetPoint()->nContent.GetIndex() )
                    bRet = TRUE;
                pCtrlStck->KillUnlockedAttrs( *pPaM->GetPoint() );
                break;

    case 0x1e:
                rDoc.Insert( *pPaM, CHAR_HARDHYPHEN);   // Non-breaking hyphen
                break;
    case 0x1f:
                rDoc.Insert( *pPaM, CHAR_SOFTHYPHEN);   // Non-required hyphens
                break;
    case 0xa0:
                rDoc.Insert( *pPaM, CHAR_HARDBLANK);    // Non-breaking spaces
                break;

    case 0x1:
                if( bObj )
                    pFmtOfJustInsertedGraphicOrOLE = ImportOle();
                else if( bEmbeddObj )
                {
                    // wenn der OLE-Import nicht klappt, dann versuche
                    // zumindest die Grafik zu importieren
                    if( !ImportOle() )
                        pFmtOfJustInsertedGraphicOrOLE = ImportGraf();
                }
                else
                    pFmtOfJustInsertedGraphicOrOLE = ImportGraf();
                // das Flag auf immer zurueck setzen
                bObj = bEmbeddObj = FALSE;
                nObjLocFc = 0;
                //##515## set nLastFlyNode so we can determine if a section
                //has ended with this paragraph unclosed
                nLastFlyNode = (*pPaM->GetPoint()).nNode.GetIndex();
                break;
    case 0x8:
                if( !bObj )
                {
                    Read_GrafLayer( nPosCp );
                    //##515##. Set nLastFlyNode so we can determine if a
                    //section has ended with this paragraph unclosed
                    nLastFlyNode = (*pPaM->GetPoint()).nNode.GetIndex();
                }
                break;

    case 0xd:   bRet = TRUE;    break;              // line end

    case 0x5:                           // Annotation reference
    case 0x13:
    case 0x2:   break;                  // Auto-Fussnoten-Nummer

#ifdef DEBUG
    default:
        {
            String sUnknown( '<' );
            sUnknown += String::CreateFromInt32( nWCharVal );
            sUnknown += '>';
            rDoc.Insert( *pPaM, sUnknown );
        }
#endif
               break;
    }
    if( '\x0' != cInsert )
        rDoc.Insert( *pPaM, ByteString::ConvertToUnicode(
                                        cInsert, RTL_TEXTENCODING_MS_1252 ) );
    return bRet;
}


void SwWW8ImplReader::ProcessAktCollChange( WW8PLCFManResult& rRes,
                                            BOOL* pStartAttr,
                                            BOOL bCallProcessSpecial )
{
    USHORT nOldColl = nAktColl;
    nAktColl = pPlcxMan->GetColl();

    if( bIsHeader )             // Fuer Kopfzeilenabstaende
        nHdTextHeight += pPlcxMan->GetPapPLCF()->GetParaHeight();
    else
    if( bIsFooter )             // Fuer Kopfzeilenabstaende
        nFtTextHeight += pPlcxMan->GetPapPLCF()->GetParaHeight();

    if( nAktColl >= nColls || !pCollA[nAktColl].pFmt
        || !pCollA[nAktColl].bColl )
    {
        nAktColl = 0;           // Unguelige Style-Id
    }
    else
    {
        nLeftParaMgn      = pCollA[nAktColl].nLeftParaMgn;
        nTxtFirstLineOfst = pCollA[nAktColl].nTxtFirstLineOfst;
    }
    BOOL bTabRowEnd = FALSE;
    if( pStartAttr && bCallProcessSpecial && !bNeverCallProcessSpecial )
    {
        BOOL bReSync;
        bTabRowEnd = ProcessSpecial( FALSE, &bReSync );// Apo / Table / Anl
        if( bReSync )
            *pStartAttr = pPlcxMan->Get( &rRes ); // hole Attribut-Pos neu
    }


/*
SwWW8ImplReader::ProcessAktCollChange(WW8PLCFManResult & {...}, unsigned char * 0x0012d6d8, unsigned char 0x01) line 1643
SwWW8ImplReader::ReadTextAttr(long & 0x00000000, unsigned char & 0x00) line 1679
SwWW8ImplReader::ReadAttrs(long & 0x00000000, long & 0x00000000, unsigned char & 0x00) line 1762 + 16 bytes
SwWW8ImplReader::ReadText(long 0x00000000, long 0x00000267, short 0x0000) line 1860
SwWW8ImplReader::LoadDoc1(SwPaM & {...}, WW8Glossary * 0x00000000) line 2270
SwWW8ImplReader::LoadDoc(SwPaM & {...}, WW8Glossary * 0x00000000) line 2609 + 16 bytes
WW8Reader::Read(SwDoc & {...}, SwPaM & {...}, const String & {???}) line 2675 + 14 bytes
SW612MI! SwReader::Read(class Reader const &) + 779 bytes
SW612MI! SwDocShell::ConvertFrom(class SfxMedium &) + 245 bytes
SFX612MI! SfxObjectShell::DoLoad(class SfxMedium *) + 3576 bytes
*/


    if( !bTabRowEnd )
    {
        SetTxtFmtCollAndListLevel( *pPaM, pCollA[ nAktColl ]);

        ChkToggleAttr( pCollA[ nOldColl ].n81Flags,
                       pCollA[ nAktColl ].n81Flags );
    }

    eFontSrcCharSet = pCollA[nAktColl].eFontSrcCharSet; // aus P-Style
}


long SwWW8ImplReader::ReadTextAttr( long& rTxtPos, BOOL& rbStartLine )
{
    long nOld = pStrm->Tell();
    long nSkipChars = 0;
    WW8PLCFManResult aRes;

    BOOL bStartAttr = pPlcxMan->Get( &aRes ); // hole Attribut-Pos

    if( aRes.nFlags & MAN_MASK_NEW_SEP ){   // neue Section
        CreateSep( rTxtPos );               // PageDesc erzeugen und fuellen
        bPgSecBreak = FALSE;                // -> 0xc war ein Sectionbreak, aber
    }                                       // kein Pagebreak;

    if(    ( aRes.nFlags & MAN_MASK_NEW_PAP )   // neuer Absatz ueber Plcx.Fkp.papx
            || rbStartLine )
    {                   // oder ueber 0x0d o.ae. im Text
        ProcessAktCollChange(
                        aRes,
                        &bStartAttr,
                            MAN_MASK_NEW_PAP == (aRes.nFlags & MAN_MASK_NEW_PAP)
                        &&  !bIgnoreText );
        rbStartLine = FALSE;
    }

    // position of last CP that's to be ignored
    long nSkipPos = -1;
    BOOL bOldDontCreateSep = bDontCreateSep;

    if( 0 < aRes.nSprmId )                      // leere Attrs ignorieren
    {
        if( ( 256 > aRes.nSprmId ) || ( 0x0800 <= aRes.nSprmId ) )
        {
            if( bStartAttr )                            // WW-Attribute
            {
                if( aRes.nMemLen >= 0 )
                {   // Attr anschalten
                    ImportSprm( aRes.pMemPos, (short)aRes.nMemLen, aRes.nSprmId );
                }
            }
            else
                EndSprm( aRes.nSprmId );        // Attr ausschalten
        }
        else if( aRes.nSprmId < 0x800 ) // eigene Hilfs-Attribute
        {
            aRes.nAktCp = rTxtPos;              // Akt. Cp-Pos
            nSkipChars = ImportExtSprm( &aRes, bStartAttr );
            if( 256 <= aRes.nSprmId && 258 >= aRes.nSprmId )
            {
                rTxtPos += nSkipChars;          // Felder/Ftn-/End-Note hier ueberlesen
                nSkipPos = rTxtPos-1;
            }
        }
    }

    if( bVer8 || nSkipChars || aRes.nSprmId == 260 )
        // Feld oder Piece
        // um nSkipChars bewegen bei Feldern und Pieces
        pStrm->Seek( pSBase->WW8Cp2Fc( pPlcxMan->GetCpOfs() + rTxtPos, &bIsUnicode));
    else
    {
        // sonst alte Pos wiederherstellen und Unicode-Flag ermitteln
        pStrm->Seek( nOld );
    }

    // Find next Attr position (and Skip attributes of field contents if needed)
    if( nSkipChars && !bIgnoreText )
        pCtrlStck->MarkAllAttrsOld();
    BOOL  bOldIgnoreText = bIgnoreText;
    bIgnoreText          = TRUE;
    USHORT nOldColl = nAktColl;
    BOOL bDoPlcxManPlusPLus = TRUE;
    long nNext;
    do
    {
        if( bDoPlcxManPlusPLus )
            (*pPlcxMan)++;
        nNext = pPlcxMan->Where();
        if( (0 <= nNext) && (nSkipPos >= nNext) )
        {
            nNext = ReadTextAttr( rTxtPos, rbStartLine );
            bDoPlcxManPlusPLus = FALSE;
            bIgnoreText = TRUE;
        }
    }
    while( nSkipPos >= nNext );
    bIgnoreText    = bOldIgnoreText;
    bDontCreateSep = bOldDontCreateSep;
    if( nSkipChars )
    {
        pCtrlStck->KillUnlockedAttrs( *pPaM->GetPoint() );
        if( nOldColl != pPlcxMan->GetColl() )
            ProcessAktCollChange(aRes, 0, FALSE);
    }

    return nNext;
}

void SwWW8ImplReader::ReadAttrs( long& rNext, long& rTxtPos, BOOL& rbStartLine )
{
    if( rTxtPos >= rNext )
    {           // Stehen Attribute an ?

        do
        {
            rNext = ReadTextAttr( rTxtPos, rbStartLine );
        }while( rTxtPos >= rNext );

    }
    else if ( rbStartLine )
    {
// keine Attribute, aber trotzdem neue Zeile
// wenn eine Zeile mit einem Seitenumbruch aufhoert und sich keine
// Absatzattribute / Absatzvorlagen aendern, ist das Zeilenende
// nicht im Plcx.Fkp.papx eingetragen, d.h. ( nFlags & MAN_MASK_NEW_PAP )
// ist FALSE. Deshalb muss als Sonderbehandlung hier die Vorlage gesetzt
// werden.
        if( !bCpxStyle )
        {
            SetTxtFmtCollAndListLevel( *pPaM, pCollA[ nAktColl ] );
        }
        rbStartLine = FALSE;
    }
}

// ReadAttrEnds zum Lesen nur der Attributenden am Ende eines Textes oder
// Textbereiches ( Kopfzeile, Fussnote, ...). Attributanfaenge, Felder
// werden ignoriert.
void SwWW8ImplReader::ReadAttrEnds( long& rNext, long& rTxtPos )
{

    while( rTxtPos >= rNext )
    {
        WW8PLCFManResult aRes;

        BOOL b = pPlcxMan->Get( &aRes );     // hole Attribut-Pos

        if(    !b
            && (aRes.nSprmId >=  0)     // nur Attributenden noch bearbeiten,
            && (    (aRes.nSprmId <     256)
                 || (aRes.nSprmId >= 0x0800) )
            )
        {                                                           // Anfaenge gehoeren zum naechsten Spezialtext
            EndSprm( aRes.nSprmId );                                // Fussnoten und Felder ignorieren
        }
        (*pPlcxMan)++;
        rNext = pPlcxMan->Where();
    }
    BOOL bDummyReSync;
    ProcessSpecial( TRUE, &bDummyReSync );
}

void SwWW8ImplReader::ReadText( long nStartCp, long nTextLen, short nType )
{
    if( nIniFlags & WW8FL_NO_TEXT )
        return;

    BOOL bStartLine = TRUE;
    short nCrCount = 0;

    nAktColl    =  0;
    pAktItemSet =  0;
    nCharFmt    = -1;
    bSpec = FALSE;
    nHdTextHeight = nFtTextHeight = 0;

    pPlcxMan = new WW8PLCFMan( pSBase, nType, nStartCp );
    long nCpOfs = pPlcxMan->GetCpOfs(); // Offset fuer Header/Footer, Footnote

    WW8_CP nNext = pPlcxMan->Where();

    pStrm->Seek( pSBase->WW8Cp2Fc( nStartCp + nCpOfs, &bIsUnicode ) );

    if(    (0 == nStartCp+nCpOfs)
        && SetCols( 0, pPlcxMan->GetSepPLCF(), 0, TRUE   ) )
    {
        // Start of text:
        //
        // look for cols and insert a section at the very beginning...
        //
        if( !pPageDesc )
            pPageDesc = &rDoc._GetPageDesc( 0 );
        SwFrmFmt &rFmt = pPageDesc->GetMaster();
        USHORT nLIdx = ( ( pWwFib->lid & 0xff ) == 0x9 ) ? 1 : 0;
        SetPage1( pPageDesc, rFmt, pPlcxMan->GetSepPLCF(), nLIdx, FALSE );
        const SwFmtCol& rCol = rFmt.GetCol();
        // if PageDesc has been inserted and has cols
        // insert a *section* with cols instead
        if( rCol.GetNumCols() )
        {
            InsertSectionWithWithoutCols( *pPaM, &rCol );
            // remove columns from PageDesc
            SwFmtCol aCol;
            rFmt.SetAttr( aCol );
        }
    }

    WW8_CP l = nStartCp;
    while ( l<nStartCp+nTextLen )
    {

        ReadAttrs( nNext, l, bStartLine );// behandelt auch Section-Breaks

        if( l>= nStartCp + nTextLen )
            break;

        bStartLine = ReadChars( l, nNext, nStartCp+nTextLen, nCpOfs );

        if( bStartLine ) // Zeilenende
        {
            rDoc.AppendTxtNode( *pPaM->GetPoint() );

            if( ( nCrCount++ & 0x40 ) == 0          // alle 64 CRs aufrufen
                && nType == MAN_MAINTEXT ){         // nicht fuer Header u. ae.
                nProgress = (USHORT)( l * 100 / nTextLen );
                ::SetProgressState( nProgress, rDoc.GetDocShell() ); // Update
            }
        }

        // If we have encountered a 0x0c which indicates either section of
        // pagebreak then look it up to see if it is a section break, and
        // if it is not then insert a page break. If it is a section break
        // it will be handled as such in the ReadAttrs of the next loop
        if( bPgSecBreak)
        {
            // We need only to see if a section is ending at this cp,
            // the plcf will already be sitting on the correct location
            // if it is there.
            WW8PLCFxDesc aTemp;
            pPlcxMan->GetSepPLCF()->GetSprms(&aTemp);
            if ((aTemp.nStartPos != l) && (aTemp.nEndPos != l))
            {
                /*
                #74468#, ##515##
                Insert additional node only WHEN the Pagebreak is contained in
                a NODE that is NOT EMPTY. Word can have empty paragraphs with
                numbering information before a section break that are not
                closed before the section break. In this case they are ignored
                and not numbered, this is 74468's problem. But word can have
                open paragraphs that only contain a node that in word can be
                anchored, i.e. a single character indicating that a graphic or
                an ole node or a text box is anchored here. In this case then
                we should close the paragraph to ensure that it is is anchored
                to the current page, and not pushed to the next page, this is
                515's problem. nLastFlyNode is set on insertion of 0x01 and
                0x08 graphics as well as on insertion of old ww6 textboxes.
                */
                SwPosition&  rPt = *pPaM->GetPoint();
                if ( (nLastFlyNode == rPt.nNode.GetIndex())
                    || (0 < rPt.nContent.GetIndex()) )
                {
                    rDoc.AppendTxtNode( rPt );
                }

                rDoc.Insert( *pPaM, SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE ) );
                bPgSecBreak = FALSE;
            }
        }
    }
    ReadAttrEnds( nNext, l );
    JoinNode( pPaM );
    if( nType == MAN_MAINTEXT )
        UpdatePageDescs( nPageDescOffset ); // muss passieren, solange es den
                                            // PlcxMan noch gibt
    DELETEZ( pPlcxMan );
}

/***************************************************************************
#           class SwWW8ImplReader
#**************************************************************************/

SwWW8ImplReader::SwWW8ImplReader( BYTE nVersionPara,
                                    SvStorage* pStorage, SvStream* pSt,
                                    SwDoc& rD, BOOL bNewDoc )
    : pStg( pStorage ), rDoc( rD ), pStrm( pSt ), bNew( 0 != bNewDoc ),
    pMSDffManager( 0 ), pAtnNames( 0 ), pAuthorInfos( 0 ), pLastPgDeskIdx( 0 ),
    pDataStream( 0 ),   pTableStream( 0 )
{
    pStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    nWantedVersion = nVersionPara;
    pCtrlStck   = 0;
    pEndStck    = 0;
    pRefFldStck = 0;
    pFonts          = 0;
    pSBase          = 0;
    pPlcxMan        = 0;
    pStyles         = 0;
    pAktColl        = 0;
    pLstManager = 0;
    pAktItemSet = 0;
    pCollA          = 0;
    pHdFt           = 0;
    pWFlyPara       = 0;
    pSFlyPara       = 0;
    pFlyFmtOfJustInsertedGraphic   = 0;
    pFmtOfJustInsertedGraphicOrOLE = 0;
    nColls = nAktColl = 0;
    nObjLocFc = nPicLocFc = 0;
    bReadNoTbl = bPgSecBreak = bSpec = bObj = bApo = bTxbxFlySection
               = bHasBorder = bSymbol = bIgnoreText = bDontCreateSep = bTable
               = bTableInApo = bWasTabRowEnd = bTxtCol = FALSE;
    bShdTxtCol = bCharShdTxtCol = bAnl = bHdFtFtnEdn = bFtnEdn
               = bIsHeader = bIsFooter = bSectionHasATitlePage
               = bIsUnicode = bCpxStyle = bStyNormal = bWWBugNormal  = FALSE;
    bNoAttrImport = bPgChpLevel = bEmbeddObj = bFloatingCtrl = FALSE;
    bAktAND_fNumberAcross = FALSE;
    bNoLnNumYet = TRUE;
    bRestartLnNumPerSection = FALSE;
    bNeverCallProcessSpecial= FALSE;
    nProgress = 0;
    nHdTextHeight = nFtTextHeight = 0;
    nPgWidth = lA4Width;
    nPgLeft = nPgRight = nPgTop = MM_250;
    nCorrIhdt = 0;
    nSwNumLevel = nWwNumType = 0xff;
    pTableDesc = 0;
    pNumRule = 0;
    pNumOlst = 0;
    pBehindSection = 0;
    pNewSection    = 0;
    pNode_FLY_AT_CNTNT = 0;
    pDrawFmt = 0;
    pDrawModel = 0;
    pDrawPg = 0;
    pDrawGroup = 0;
    pDrawHeight = 0;
    nDrawTxbx = 0;
    pDrawEditEngine = 0;
    pFormImpl = 0;
    nLeftParaMgn = 0;
    nTxtFirstLineOfst = 0;
    pNumFldType = 0;
    nFldNum = 0;

    nLastFlyNode = ULONG_MAX;
    nImportedGraphicsCount = 0;

    nLFOPosition = USHRT_MAX;
    nListLevel   = nWW8MaxListLevel;

    eFontSrcCharSet = RTL_TEXTENCODING_DONTKNOW;
    eFontDstCharSet = RTL_TEXTENCODING_DONTKNOW;
    eHardCharSet = RTL_TEXTENCODING_DONTKNOW;
    pPageDesc = 0;

    nNfcPgn = nPgChpDelim = nPgChpLevel = 0;

#ifdef DEBUG
    Sound::Beep();
#endif
}

void SwWW8ImplReader::DeleteStk(SwFltControlStack* pStck)
{
    if( pStck )
    {
        pStck->SetAttr( *pPaM->GetPoint(), 0, FALSE );
        pStck->SetAttr( *pPaM->GetPoint(), 0, FALSE );
        delete pStck;
    }
    else
    {
        ASSERT( !this, "WW-Stack bereits geloescht" );
    }
}

ULONG SwWW8ImplReader::LoadDoc1( SwPaM& rPaM ,WW8Glossary *pGloss)
{
    ULONG nErrRet = 0;

    if( bNew && pStg && !pGloss)
        ReadDocInfo();

    pPaM = new SwPaM( *rPaM.GetPoint() );

    pCtrlStck = new SwWW8FltControlStack( &rDoc, nFieldFlags, *this );

    /*
        Endestack: haelt z.B. Bookmarks und Variablen solange vor,
        bis er den Befehl zum inserten bekommt.
    */
    pEndStck    = new SwFltEndStack(        &rDoc, nFieldFlags );
    /*
        fieldstack holds Reference Fields until the very end of file import
    */
    pRefFldStck = new SwWW8FltControlStack( &rDoc, nFieldFlags, *this );

    nPageDescOffset = rDoc.GetPageDescCnt();

    SwNodeIndex aSttNdIdx( rDoc.GetNodes() );
    SwRelNumRuleSpaces aRelNumRule( rDoc, bNew );

    USHORT eMode = REDLINE_SHOW_INSERT;

    // enum-para mitgeben, welche Versions-Nummern erlaubt sind: ww6, ww8, ...

    if (pGloss)
        pWwFib = pGloss->GetFib();
    else
        pWwFib = new WW8Fib( *pStrm, nWantedVersion );  // Fib einlesen

    if ( pWwFib->nFibError )
    {           // ERR_NO_WW8_FILE
        nErrRet = ERR_SWG_READ_ERROR;               // oder ERR_SW6_READ_ERROR
    }
    else if( pWwFib->fEncrypted )
    {
        nErrRet = ERR_SW6_PASSWD;                   // gecryptet geht nicht



        // Annotation:  Crypted files  C O U L D  be imported if we used the methods
        //              provided by Caolan Mc Nammara.
        //              Written permission (my e-mail) was given by him
        //              but no special document has been signed...
        //              27. Sept. 1999, K.-H. Zimmer


    }
    else if( ( nIniFlags & WW8FL_NO_COMPLEX ) && pWwFib->fComplex )
    {
        nErrRet = WARN_WW6_FASTSAVE_ERR;            // Warning melden
    }
    else
    {
        // praktische Hilfsvariablen besetzen:
        bVer67 = (     (6 == pWwFib->nVersion)
                                || (7 == pWwFib->nVersion) );   // z.B.: altes Sprm-Id-Format!
        bVer6  =  (6 == pWwFib->nVersion);
        bVer7  =  (7 == pWwFib->nVersion);
        bVer8  =  (8 == pWwFib->nVersion);

        SvStorageStreamRef xTableStream, xDataStream;

        // Nachdem wir nun den FIB eingelesen haben, wissen wir ja,
        // welcher Table-Stream gueltig ist.
        // Diesen oeffnen wir nun.
        switch( pWwFib->nVersion )  // 6 steht fuer "6 ODER 7",  7 steht fuer "NUR 7"
        {
        case 6:
        case 7: pTableStream = pStrm;
                pDataStream = pStrm;
                break;

        case 8:
                if( !pStg )
                {
                    ASSERT( pStg, "Version 8 muss immer einen Storage haben!" );
                    nErrRet = ERR_SWG_READ_ERROR;
                    break;
                }
                xTableStream = pStg->OpenStream(
                        String(   (1 == pWwFib->fWhichTblStm)
                                ? "1Table"
                                : "0Table",
                                RTL_TEXTENCODING_MS_1252 ),
                        STREAM_STD_READ );
                pTableStream = &xTableStream;
                pTableStream->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

                xDataStream = pStg->OpenStream(
                                    String( "Data", RTL_TEXTENCODING_MS_1252 ),
                                    STREAM_STD_READ | STREAM_NOCREATE );
                if( xDataStream.Is() && SVSTREAM_OK == xDataStream->GetError() )
                {
                    pDataStream = &xDataStream;
                    pDataStream->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
                }
                else
                    pDataStream = pStrm;

                if( pWwFib->lcbPlcfspaHdr || pWwFib->lcbPlcfspaMom )
                {
                    pMSDffManager = new SwMSDffManager( *this );
                    //#79055# Now the dff manager always needs a controls
                    //converter as well, but a control converter may still
                    //exist without a dffmanager. cmc
                    pFormImpl = new SwMSConvertControls(rDoc.GetDocShell(),
                        pPaM);
                }
                break;

        default:// Programm-Fehler!
                        ASSERT( !this, "Es wurde vergessen, nVersion zu kodieren!" );
                        nErrRet = ERR_SWG_READ_ERROR;
        }

        if( ERR_SWG_READ_ERROR != nErrRet )
        {
            if( 0x0100 == pWwFib->chse )
                eTextCharSet   = RTL_TEXTENCODING_APPLE_ROMAN;
            else
                eTextCharSet   = rtl_getTextEncodingFromWindowsCharset( pWwFib->chse );
            if( 0x0100 == pWwFib->chseTables )
                eStructCharSet = RTL_TEXTENCODING_APPLE_ROMAN;
            else
                eStructCharSet = rtl_getTextEncodingFromWindowsCharset( pWwFib->chseTables );

            bWWBugNormal = pWwFib->nProduct == 0xc03d;

            if( !bNew )
                aSttNdIdx = pPaM->GetPoint()->nNode;

            ::StartProgress( STR_STATSTR_W4WREAD, 0, 100, rDoc.GetDocShell() );


            rDoc.SetParaSpaceMax( TRUE, TRUE ); // Abstand zwischen zwei Absaetzen ist
            // die SUMME von unterem Abst. des ersten und oberem Abst. des zweiten


            // read Font Table
            pFonts = new WW8Fonts( *pTableStream, *pWwFib );


            pWDop = new WW8Dop( *pTableStream, pWwFib->nFib, pWwFib->fcDop, pWwFib->lcbDop  );      // Document Properties
            ImportDop( bNew != 0 );


            /*
                Import revisioning data: author names
            */
            if( pWwFib->lcbSttbfRMark )
                ReadRevMarkAuthorStrTabl( *pTableStream,
                                            pWwFib->fcSttbfRMark,
                                            pWwFib->lcbSttbfRMark, rDoc );

            /*
                zuerst(!) alle Styles importieren   (siehe WW8PAR2.CXX)
                    VOR dem Import der Listen !!
            */
            ::SetProgressState( nProgress, rDoc.GetDocShell() );    // Update
            pStyles = new WW8RStyle( *pWwFib, this );   // Styles
            pStyles->Import();
            ::SetProgressState( nProgress, rDoc.GetDocShell() );    // Update



            /*
                jetzt erst alle Listen importieren  (siehe WW8PAR3.CXX)
                    NACH dem Import der Styles !!
            */
            pLstManager = new WW8ListManager( *pTableStream, *this );
            ::SetProgressState( nProgress, rDoc.GetDocShell() );    // Update



            /*
                zu guter Letzt:                     (siehe ebenfalls WW8PAR3.CXX)
                ===============
                alle Styles durchgehen und ggfs. zugehoeriges Listen-Format anhaengen
                    NACH dem Import der Styles und NACH dem Import der Listen !!
            */
            pStyles->RegisterNumFmts();


            pSBase = new WW8ScannerBase( pStrm, pTableStream, pDataStream,
                                         pWwFib );
            static SvxExtNumType __READONLY_DATA eNumTA[16] = {
                SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER, SVX_NUM_CHARS_UPPER_LETTER_N,
                SVX_NUM_CHARS_LOWER_LETTER_N, SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
                SVX_NUM_ARABIC, SVX_NUM_ARABIC,
                SVX_NUM_ARABIC, SVX_NUM_ARABIC,
                SVX_NUM_ARABIC, SVX_NUM_ARABIC,
                SVX_NUM_ARABIC, SVX_NUM_ARABIC };

            if( pSBase->AreThereFootnotes() ){
                static SwFtnNum __READONLY_DATA eNumA[4] = {
                    FTNNUM_DOC, FTNNUM_CHAPTER, FTNNUM_PAGE, FTNNUM_DOC };

                SwFtnInfo aInfo;
                aInfo = rDoc.GetFtnInfo();      // Copy-Ctor privat

                aInfo.ePos = FTNPOS_PAGE;
                aInfo.eNum = eNumA[pWDop->rncFtn];
                aInfo.aFmt.SetNumberingType(eNumTA[pWDop->nfcFtnRef]);
                if( pWDop->nFtn )
                    aInfo.nFtnOffset = pWDop->nFtn - 1;
                rDoc.SetFtnInfo( aInfo );
            }
            if( pSBase->AreThereEndnotes() ){
                SwEndNoteInfo aInfo;
                aInfo = rDoc.GetEndNoteInfo();  // parallel zu Ftn

                // Ich kann nicht setzen, wann neu nummerieren...
                //  aInfo.eNum = eNumA[pWDop->pDop->rncEdn];
                aInfo.aFmt.SetNumberingType(eNumTA[pWDop->nfcEdnRef]);
                if( pWDop->nEdn )
                    aInfo.nFtnOffset = pWDop->nEdn - 1;
                rDoc.SetEndNoteInfo( aInfo );
            }

            if( pWwFib->lcbPlcfhdd )
                pHdFt = new WW8PLCF_HdFt( pTableStream, *pWwFib, *pWDop );

            if( !bNew ){        // in ein Dokument einfuegen ?
                                // Da immer ganze Zeile eingelesen werden, muessen
                                // evtl. Zeilen eingefuegt / aufgebrochen werden
                const SwPosition* pPos = pPaM->GetPoint();
                const SwTxtNode* pSttNd = rDoc.GetNodes()[ pPos->nNode ]->GetTxtNode();
                USHORT nCntPos = pPos->nContent.GetIndex();

                if( nCntPos && pSttNd->GetTxt().Len() ) // EinfuegePos nicht in leerer Zeile
                    rDoc.SplitNode( *pPos );            // neue Zeile erzeugen

                if( pSttNd->GetTxt().Len() )
                {   // EinfuegePos nicht am Ende der Zeile
                    rDoc.SplitNode( *pPos );    // neue Zeile
                    pPaM->Move( fnMoveBackward );   // gehe in leere Zeile
                }

                // verhinder das Einlesen von Tabellen in Fussnoten / Tabellen
                ULONG nNd = pPos->nNode.GetIndex();
                bReadNoTbl = 0 != pSttNd->FindTableNode() ||
                                        ( nNd < rDoc.GetNodes().GetEndOfInserts().GetIndex() &&
                    rDoc.GetNodes().GetEndOfInserts().StartOfSectionIndex() < nNd );

            }

            ::SetProgressState( nProgress, rDoc.GetDocShell() );    // Update

            if (pGloss) // loop for each glossary entry and add dummy section node
            {
                WW8PLCF aPlc(&xTableStream,pWwFib->fcPlcfglsy,
                                pWwFib->lcbPlcfglsy,0 );

                WW8_CP nStart, nEnd;
                void* pDummy;

                for (int i=0;i<pGloss->GetNoStrings();i++,aPlc++)
                //for (int i=0;i<1;i++,aPlc++)
                {
                    SwNodeIndex aIdx( rDoc.GetNodes().GetEndOfContent());
                    SwTxtFmtColl* pColl =
                        rDoc.GetTxtCollFromPool( RES_POOLCOLL_STANDARD);
                    SwStartNode *pNode =
                        rDoc.GetNodes().MakeTextSection(aIdx,
                        SwNormalStartNode,pColl);
                    pPaM->GetPoint()->nNode = pNode->GetIndex()+1;
                    pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(),0);
                    aPlc.Get( nStart, nEnd, pDummy );
                    ReadText(nStart,nEnd-nStart-1,MAN_MAINTEXT);
                    //ReadText( 0, pWwFib->ccpText, MAN_MAINTEXT );
                }
            }
            else //ordinary case
            {
                ReadText( 0, pWwFib->ccpText, MAN_MAINTEXT );
            }

            ::SetProgressState( nProgress, rDoc.GetDocShell() );    // Update

            if( pCollA )
            {
                SwNumRule aOutlineRule( *rDoc.GetOutlineNumRule() );

                USHORT nI;
                // Bitfeld, welche Outline-Level bereits an einem
                // Style gesetzt wurden (vermeiden von Doppeltvergabe)
                USHORT nFlagsStyleOutlLevel = 0;
                if( !bNew )
                {
                    const SwTxtFmtColls& rColls = *rDoc.GetTxtFmtColls();
                    for( nI = 0; nI < rColls.Count(); nI++ )
                    {
                        const SwTxtFmtColl& rColl = *rColls[ nI ];
                        if( MAXLEVEL > rColl.GetOutlineLevel() )
                        {
                            nFlagsStyleOutlLevel |= 1 << rColl.GetOutlineLevel();
                        }
                    }
                }
                USHORT nIa=10;
                USHORT nIz=nColls;
                for( USHORT nJ = 0; nJ < 2; nJ++ )
                {
                    for( nI = nIa; nI < nIz; nI++ )
                    {
                        SwWW8StyInf& rSI = pCollA[ nI ];

                        if(    ( MAXLEVEL > rSI.nOutlineLevel )
                            && rSI.pOutlineNumrule
                            && rSI.pFmt )
                        {
                            USHORT nAktFlags = 1 << rSI.nOutlineLevel;
                            if( nAktFlags & nFlagsStyleOutlLevel )
                            {
                                rSI.pFmt->SetAttr(
                                    SwNumRuleItem( rSI.pOutlineNumrule->GetName() ) );
                                ((SwTxtFmtColl*)rSI.pFmt)->SetOutlineLevel( NO_NUMBERING );
                            }
                            else
                            // die Default-Styles im ZWEITEN Durchgang auf jeden Fall
                            // nehmen, die User-definierten nur, wenn auch verwendet.
                            if( nJ || rSI.pFmt->GetDepends() )
                            {
                                // Numformat aus der NumRule nehmen
                                // und bei der OutlineRule setzen.
                                aOutlineRule.Set(
                                    rSI.nOutlineLevel,
                                    rSI.pOutlineNumrule->Get( rSI.nOutlineLevel ) );
                                // am Style die Outlinenummer eintragen
                                ((SwTxtFmtColl*)rSI.pFmt)->SetOutlineLevel( rSI.nOutlineLevel );
                                // Flag verodern, um Doppeltvergabe zu vermeiden
                                nFlagsStyleOutlLevel |= nAktFlags;
                            }
                        }
                    }
                    nIa = 1;
                    nIz = 10;
                }
                if( nFlagsStyleOutlLevel )
                    rDoc.SetOutlineNumRule( aOutlineRule );

                /*  // wird nun ueber D'tor erledigt!
                for( nI = 0; nI < nColls; nI++ ){
                    if( pCollA[nI].pWWFly )
                        delete( pCollA[nI].pWWFly );
                }
                */
                delete[] pCollA;
            }

            if( pDrawPg && pMSDffManager && pMSDffManager->GetShapeOrders() )
            {
                // Hilfsarray zum Verketten der (statt SdrTxtObj) eingefuegten Rahmen
                SvxMSDffShapeTxBxSort aTxBxSort;

                // korrekte Z-Order der eingelesen Escher-Objekte sicherstellen
                USHORT nShapeCount = pMSDffManager->GetShapeOrders()->Count();
                USHORT nNewObjNum = 0;
                for (USHORT nShapeNum=0; nShapeNum < nShapeCount; nShapeNum++)
                {
                    SvxMSDffShapeOrder& rOrder = *(SvxMSDffShapeOrder*)
                        (pMSDffManager->GetShapeOrders()->GetObject( nShapeNum ));
                    if( rOrder.pObj )
                    {
                        ULONG nOldObjNum = rOrder.pObj->GetOrdNum();
                        if(    (nOldObjNum != nNewObjNum )
                            && (pDrawPg->GetObj( nOldObjNum )) )
                            pDrawPg->NbcSetObjectOrdNum( nOldObjNum, nNewObjNum );
                        ++nNewObjNum;
                    }
                    // Pointer in neues Sort-Array einfuegen
                    if( rOrder.nTxBxComp && rOrder.pFly )
                        aTxBxSort.Insert( &rOrder );
                }
                // zu verkettende Rahmen jetzt verketten
                USHORT nTxBxCount = aTxBxSort.Count();
                if( nTxBxCount )
                {
                    SwFmtChain aChain;
                    for (USHORT nTxBxNum=0; nTxBxNum < nTxBxCount; nTxBxNum++)
                    {
                        SvxMSDffShapeOrder& rOrder =
                            *(SvxMSDffShapeOrder*)(aTxBxSort.GetObject( nTxBxNum ));

                        // Fly-Frame-Formate initialisieren
                        SwFlyFrmFmt* pFlyFmt     = rOrder.pFly;
                        SwFlyFrmFmt* pNextFlyFmt = 0;
                        SwFlyFrmFmt* pPrevFlyFmt = 0;
                        // ggfs. Nachfolger ermitteln
                        if( 1+nTxBxNum < nTxBxCount )
                        {
                            SvxMSDffShapeOrder& rNextOrder =
                                *(SvxMSDffShapeOrder*)(aTxBxSort.GetObject( nTxBxNum+1 ));
                            if(       (0xFFFF0000 &     rOrder.nTxBxComp)
                                   == (0xFFFF0000 & rNextOrder.nTxBxComp)
                                &&    rOrder.nHdFtSection
                                   == rNextOrder.nHdFtSection  )
                                pNextFlyFmt = rNextOrder.pFly;
                        }
                        // ggfs. Vorgaenger ermitteln
                        if( nTxBxNum )
                        {
                            SvxMSDffShapeOrder& rPrevOrder =
                                *(SvxMSDffShapeOrder*)(aTxBxSort.GetObject( nTxBxNum-1 ));
                            if(       (0xFFFF0000 &     rOrder.nTxBxComp)
                                   == (0xFFFF0000 & rPrevOrder.nTxBxComp)
                                &&    rOrder.nHdFtSection
                                   == rPrevOrder.nHdFtSection  )
                                pPrevFlyFmt = rPrevOrder.pFly;
                        }
                        // Falls Nachfolger oder Vorgaenger vorhanden,
                        // die Verkettung am Fly-Frame-Format eintragen
                        if( pNextFlyFmt || pPrevFlyFmt )
                        {
                            aChain.SetNext( pNextFlyFmt );
                            aChain.SetPrev( pPrevFlyFmt );
                            pFlyFmt->SetAttr( aChain );
                        }
                    }

                }

            }

            if( bNew )
            {
                if( pWDop->fRevMarking )
                    eMode |= REDLINE_ON;
                if( pWDop->fRMView )
                    eMode |= REDLINE_SHOW_DELETE;
                if(pStg && !pGloss) /*meaningless for a glossary, cmc*/
                {
                    const OfaFilterOptions* pVBAFlags = OFF_APP()->GetFilterOptions();
                    SvxImportMSVBasic aVBasic(*rDoc.GetDocShell(),*pStg,
                                    pVBAFlags->IsLoadWordBasicCode(),
                                    pVBAFlags->IsLoadWordBasicStorage() );
                    String s1( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "Macros" )));
                    String s2( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "VBA" )));
                    int nRet = aVBasic.Import( s1, s2 );
                    if( 2 & nRet )
                        rDoc.SetContainsMSVBasic( TRUE );
                  }
            }

            DELETEZ( pStyles );

            if( pFormImpl )
                DeleteFormImpl();
            GrafikDtor();
            DELETEZ( pMSDffManager );
            DELETEZ( pHdFt );
            DELETEZ( pLstManager );
            DELETEZ( pSBase );
            DELETEZ( pWDop );
            DELETEZ( pFonts );
            DELETEZ( pAtnNames );
            DELETEZ( pAuthorInfos );
            DELETEZ( pLastPgDeskIdx );
            ::EndProgress( rDoc.GetDocShell() );
        }
        pDataStream = 0;
        pTableStream = 0;
    }
    DELETEZ( pBehindSection );
    if (!pGloss)
        DELETEZ( pWwFib );
    DeleteCtrlStk();
    DeleteEndStk();
    DeleteRefFldStk();

    // set NoBallanced flag on last inserted section
    if( pNewSection )
    {
        pNewSection->GetFmt()->SetAttr( SwFmtNoBalancedColumns( TRUE ) );
    }

    // NumRules koennen erst nach dem setzen aller Attribute korrgiert werden
#ifdef DEBUG
    {
    ULONG nN = rDoc.GetNodes().Count();
    for( ULONG iN = 0; iN < nN; ++iN )
    {
        SwTxtNode* pN = rDoc.GetNodes()[ iN ]->GetTxtNode();
        if( pN && pN->GetNum() )
        {
            const SwNumRuleItem& rItem
                    = (SwNumRuleItem&)(pN->SwCntntNode::GetAttr( RES_PARATR_NUMRULE ) );
            if( !rItem.GetValue().Len() )
                ASSERT( !this, "NdNum gesetzt, aber KEIN NumRuleItem" );
        }
    }
    }
#endif

#ifdef DEBUG
    {
    const SwSpzFrmFmts& rFmts = *rDoc.GetSpzFrmFmts();
    for( ULONG iN = 0, nN = rFmts.Count(); iN < nN; ++iN )
    {
        const SwFmtAnchor& rA = rFmts[ iN ]->GetAnchor();
        if( FLY_IN_CNTNT == rA.GetAnchorId() &&
            !rA.GetCntntAnchor())
        {
            int x = 0;
        }
    }
    }
#endif

    aRelNumRule.SetNumRelSpaces( rDoc );
    if( !bNew && !nErrRet && aSttNdIdx.GetIndex() )
    {
        aSttNdIdx++;
        aRelNumRule.SetOultineRelSpaces( aSttNdIdx,
                                    pPaM->GetPoint()->nNode );
    }

    UpdateFields();

    DELETEZ( pPaM );
    // delete the pam before the call for hide all redlines (Bug 73683)
    if( bNew )
        rDoc.SetRedlineMode( eMode );

    return nErrRet;
}

const String* SwWW8ImplReader::GetAnnotationAuthor( short nIdx )
{
    if( !pAtnNames && pWwFib->lcbGrpStAtnOwners )
    {
        // Authoren bestimmen: steht im TableStream
        pAtnNames = new SvStringsDtor( 4, 4 );
        SvStream& rStrm = *pTableStream;

        long nOldPos = rStrm.Tell();
        rStrm.Seek( pWwFib->fcGrpStAtnOwners );

        long nRead = 0, nCount = pWwFib->lcbGrpStAtnOwners;
        String* pAutName;
        while( nRead < nCount )
        {
            if( bVer67 )
            {
                pAutName = new String( WW8ReadPString( rStrm, FALSE ) );
                nRead += pAutName->Len() + 1;   // Laenge + BYTE Count
            }
            else
            {
                pAutName = new String( WW8Read_xstz( rStrm, 0, FALSE ) );
                nRead += pAutName->Len() * 2 + 2;// UNICode: doppelte Laenge + USHORT Count
            }
            pAtnNames->Insert( pAutName, pAtnNames->Count() );
        }
        rStrm.Seek( nOldPos );
    }

    return pAtnNames && nIdx < pAtnNames->Count() ? (*pAtnNames)[ nIdx ] : 0;
}


#pragma optimize( "", off )

ULONG SwWW8ImplReader::LoadDoc( SwPaM& rPaM,WW8Glossary *pGloss)
{
    ULONG nErrRet = 0;

    pAktColl    = 0;
    pAktItemSet = 0;

    {
        static const sal_Char* aNames[ 12 ] = {
            "WinWord/WW", "WinWord/WW8", "WinWord/WWFT",
            "WinWord/WWFLX", "WinWord/WWFLY",
            "WinWord/WWF",
            "WinWord/WWFA0", "WinWord/WWFA1", "WinWord/WWFA2",
            "WinWord/WWFB0", "WinWord/WWFB1", "WinWord/WWFB2"
        };
        sal_uInt32 aVal[ 12 ];
#if SUPD>612
        SwFilterOptions aOpt( 12, aNames, aVal );
#else
        memset( &aVal, 0, sizeof( aVal ) );
#endif

        nIniFlags = aVal[ 0 ];
        nIniFlags1= aVal[ 1 ];
//      nIniHdSiz = ReadFilterFlags( "WWHD" );
        nIniFtSiz = aVal[ 2 ];
        // schiebt Flys um x twips nach rechts o. links
        nIniFlyDx = aVal[ 3 ];
        nIniFlyDy = aVal[ 4 ];

        nFieldFlags = aVal[ 5 ];
        if ( SwFltGetFlag( nFieldFlags, SwFltControlStack::HYPO ) )
        {
            SwFltSetFlag( nFieldFlags, SwFltControlStack::BOOK_TO_VAR_REF );
            SwFltSetFlag( nFieldFlags, SwFltControlStack::TAGS_DO_ID );
            SwFltSetFlag( nFieldFlags, SwFltControlStack::TAGS_IN_TEXT );
            SwFltSetFlag( nFieldFlags, SwFltControlStack::ALLOW_FLD_CR );
            for( USHORT i = 0; i < 3; i++ )
                nFieldTagAlways[i] = 0;         // Hypo-Default: bekannte Felder
                                                // nicht taggen
            nFieldTagBad[0] = 0xffffffff;       // unbekannte Felder taggen
            nFieldTagBad[1] = 0xffffffff;       //
            nFieldTagBad[2] = 0xffffffef;       // "EinfuegenText" nicht taggen
        }
        else
        {
            nFieldTagAlways[0] = aVal[ 6 ];
            nFieldTagAlways[1] = aVal[ 7 ];
            nFieldTagAlways[2] = aVal[ 8 ];
            nFieldTagBad[0] = aVal[ 9 ];
            nFieldTagBad[1] = aVal[ 10 ];
            nFieldTagBad[2] = aVal[ 11 ];
        }
    }

    UINT16 nMagic;
    *pStrm >> nMagic;

    switch( nWantedVersion )    // beachte: 6 steht fuer "6 ODER 7",  7 steht fuer "NUR 7"
    {
    case 6:
    case 7:
            if( 0xa5dc != nMagic )
            {
                //JP 06.05.99: teste auf eigenen 97-Fake!
                if( pStg && 0xa5ec == nMagic )
                {
                    ULONG nCurPos = pStrm->Tell();
                    UINT32 nfcMin;
                    if( pStrm->Seek( nCurPos + 22 ) )
                    {
                        *pStrm >> nfcMin;
                        if( 0x300 != nfcMin )
                            nErrRet = ERR_WW6_NO_WW6_FILE_ERR;
                    }
                    pStrm->Seek( nCurPos );
                }
                else
                    nErrRet = ERR_WW6_NO_WW6_FILE_ERR;
            }
            break;
    case 8: if( 0xa5ec != nMagic )
                nErrRet = ERR_WW8_NO_WW8_FILE_ERR;
            break;

    default:
        nErrRet = ERR_WW8_NO_WW8_FILE_ERR;
        ASSERT( !this, "Es wurde vergessen, nVersion zu kodieren!" );
    }

    if( !nErrRet )
        nErrRet = LoadDoc1( rPaM ,pGloss);

#ifdef DEBUG
    Sound::Beep();
#endif

    return nErrRet;                 // return Errorcode
}


ULONG WW8Reader::Read( SwDoc &rDoc, SwPaM &rPam,
                    const String & /* FileName, falls benoetigt wird */ )
{
    USHORT nOldBuffSize = 32768;
    BOOL bNew = !bInsertMode;               // Neues Doc ( kein Einfuegen )


    SvStorageStreamRef refStrm;         // damit uns keiner den Stream klaut
    SvStream* pIn = pStrm;

    ULONG nRet = 0;
    BYTE nVersion = 8;

    String sFltName = GetFltName();
    if( sFltName.EqualsAscii( "WW6" ) )
    {
        if( pStrm )
            nVersion = 6;
        else
        {
            ASSERT( FALSE, "WinWord 95 Reader-Read ohne Stream" );
            nRet = ERR_SWG_READ_ERROR;
        }
    }
    else
    {
        if( sFltName.EqualsAscii( "CWW6" ) )
            nVersion = 6;
        else if( sFltName.EqualsAscii( "CWW7" ) )
            nVersion = 7;

        if( pStg )
        {
            nRet = OpenMainStream( refStrm, nOldBuffSize );
            pIn = &refStrm;
        }
        else
        {
            ASSERT( FALSE, "WinWord 95/97 Reader-Read ohne Storage" );
            nRet = ERR_SWG_READ_ERROR;
        }
    }

    if( !nRet )
    {
        //JP 18.01.96: Alle Ueberschriften sind normalerweise ohne
        //              Kapitelnummer. Darum hier explizit abschalten
        //              weil das Default jetzt wieder auf AN ist.
        if( bNew )
        {
            Reader::SetNoOutlineNum( rDoc );
            // MIB 27.09.96: Umrandung uns Abstaende aus Frm-Vorlagen entf.
            Reader::ResetFrmFmts( rDoc );
        }
        SwWW8ImplReader* pRdr = new SwWW8ImplReader( nVersion, pStg,
                                                        pIn, rDoc, bNew );
        nRet = pRdr->LoadDoc( rPam );
        delete pRdr;

        if( refStrm.Is() )
        {
            refStrm->SetBufferSize( nOldBuffSize );
            refStrm.Clear();
        }
    }
    return nRet;
}

int WW8Reader::GetReaderType()
{
    return SW_STORAGE_READER | SW_STREAM_READER;
}

BOOL WW8Reader::HasGlossaries() const
{
    return TRUE;
}


BOOL WW8Reader::ReadGlossaries( SwTextBlocks& rBlocks, BOOL bSaveRelFiles ) const
{
    USHORT nOldBuffSize = 32768;
    SvStorageStreamRef refStrm;

    WW8Reader *pThis = (WW8Reader *)this;
    ULONG nRet = pThis->OpenMainStream( refStrm, nOldBuffSize );

    WW8Glossary aGloss( refStrm, 8, pStg );
    return aGloss.Load(rBlocks, bSaveRelFiles );
}


#pragma optimize( "", off )


BOOL SwMSDffManager::GetOLEStorageName( long nOLEId, String& rStorageName,
                                        SvStorageRef& rSrcStorage,
                                        SvStorageRef& rDestStorage ) const
{
    BOOL bRet = FALSE;

    long nPictureId = 0;
    if( !( rReader.nIniFlags & WW8FL_NO_OLE ) && rReader.pStg )
    {
        // dann holen wir uns mal ueber den TextBox-PLCF die richtigen
        // Char Start-/End-Positionen. In dem Bereich sollte dann
        // das EinbettenFeld und die entsprechenden Sprms zu finden
        // sein. Wir brauchen hier aber nur das Sprm fuer die Picture Id
        long nOldPos = rReader.pStrm->Tell();
        {
            long nStartCp, nEndCp;
            rReader.GetTxbxTextSttEndCp( nStartCp, nEndCp,
                                        ( nOLEId >> 16 ) & 0xFFFF,
                                        nOLEId & 0xFFFF );

            WW8PLCFxSaveAll aSave;
            memset( &aSave, 0, sizeof( aSave ) );
            rReader.pPlcxMan->SaveAllPLCFx( aSave );

            nStartCp += rReader.nDrawCpO;
            nEndCp   += rReader.nDrawCpO;
            WW8PLCFx_Cp_FKP* pChp = rReader.pPlcxMan->GetChpPLCF();
            pChp->SeekPos( nStartCp );

            WW8_CP nStart = pChp->Where();
            while( nStart <= nEndCp && !nPictureId )
            {
                WW8PLCFxDesc aDesc;
                pChp->GetSprms( &aDesc );
                (*pChp)++;
                WW8_CP nNextEnd = pChp->Where();
                WW8_CP nEnd = ( nNextEnd < nEndCp ) ? nNextEnd : nEndCp;

                if( aDesc.nSprmsLen && aDesc.pMemPos )  // Attribut(e) vorhanden
                {
                    long nLen = aDesc.nSprmsLen;
                    BYTE* pSprm = aDesc.pMemPos;

                    while( nLen >= 2 && !nPictureId )
                    {
                        BYTE   nDelta;
                        USHORT nId = WW8GetSprmId( rReader.pWwFib->nVersion,
                                                    pSprm, &nDelta );
                        short nSL = WW8GetSprmSizeBrutto(
                                    rReader.pWwFib->nVersion, pSprm, &nId );

                        if( nLen < nSL )
                            break;              // nicht mehr genug Bytes uebrig

                        if( 0x6A03 == nId && 0 < nLen )
                        {
                            nPictureId = SVBT32ToLong( pSprm + 1 +
                                    nDelta + WW8SprmDataOfs( nId ) );
                            bRet = TRUE;
                        }
                        pSprm += nSL;
                        nLen -= nSL;
                    }
                }
                nStart = nNextEnd;
            }

            rReader.pPlcxMan->RestoreAllPLCFx( aSave );
        }
        rReader.pStrm->Seek( nOldPos );
    }

    if( bRet )
    {
        ( rStorageName = '_' ) += String::CreateFromInt32( nPictureId );
        rSrcStorage = rReader.pStg->OpenStorage( WW8_ASCII2STR( "ObjectPool" ) );
        SwDocShell *pDocShell = rReader.rDoc.GetDocShell();
        if (pDocShell == 0)
            bRet=FALSE;
        else
            rDestStorage = pDocShell->GetStorage();
    }
    return bRet;
}

BOOL SwMSDffManager::ShapeHasText( ULONG nShapeId, ULONG nFilePos ) const
{
    // Zur Zeit des Einlesens einer einzelnen Box, die womoeglich Teil einer
    // Gruppe ist, liegen noch nicht genuegend Informationen vor, um
    // entscheiden zu koennen, ob wir sie nicht doch als Textfeld benoetigen.
    // Also vorsichtshalber mal alle umwandeln:
    return TRUE;
}
/*
void SwMSDffManager::ProcessClientAnchor2( SvStream& rSt, DffRecordHeader& rHd, void* pData, DffObjData& rObj )
{
    if( rHd.nRecLen && rObj.nShapeId )
    {
        SvxMSDffImportData& rData = *(SvxMSDffImportData*)pData;

        SvxMSDffImportRec* pImpRec = 0;
        // find out if this shape did get a Record already
        USHORT nRecCnt = rData.GetRecCount();
        if( nRecCnt )
        {
            for(USHORT i = nRecCnt; i; )
            {
                i--;
                SvxMSDffImportRec* pRec = rData.GetRecord( i );
                if( pRec && (rObj.nShapeId == pRec->nShapeId) )
                {
                    // Nun hab ich Dich, Du Wicht!
                    pImpRec = pRec;
                    break;
                }
            }
        }
        if( !pImpRec )
        {
            SvxMSDffImportRec* pTmpRec = new SvxMSDffImportRec;
            pTmpRec->nShapeId = rObj.nShapeId;
            rData.aRecords.Insert( pTmpRec );
            pImpRec = rData.GetRecord( nRecCnt );
            if( !pImpRec )
                delete pTmpRec;
        }
        if( pImpRec )
        {
            // new
            pImpRec->pClientAnchorBuffer    =   new char[ rHd.nRecLen ];
            pImpRec->nClientAnchorLen       =   rHd.nRecLen;
            rSt.Read( pImpRec->pClientAnchorBuffer, rHd.nRecLen );
        }
    }
}
*/
/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/ww8par.cxx,v 1.16 2001-04-02 08:58:16 cmc Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.15  2001/03/30 15:20:23  cmc
      ##540## rework pagebreak insertion, to avoid bas pagebreak localtion due to handling fields before pagebreaks

      Revision 1.14  2001/03/16 14:15:34  cmc
      reformat code

      Revision 1.13  2001/02/27 15:03:08  os
      Complete use of DefaultNumbering component

      Revision 1.12  2001/02/20 15:24:20  cmc
      #84095# Footnotes in field results that are being ignored shouldn't therefore be inserted

      Revision 1.11  2001/02/16 10:08:12  cmc
      Normalize japanese doptypography variable names

      Revision 1.10  2001/02/01 16:11:30  cmc
      #83362# Missing i18n header include

      Revision 1.9  2001/01/30 20:11:06  cmc
      #83362# CJK Forbidden Character {Im|Ex}port

      Revision 1.8  2001/01/26 15:43:22  jp
      Bug #77951#: MakePageDesc - create I18N names

      Revision 1.7  2001/01/22 09:05:58  os
      update of filter configuration

      Revision 1.6  2000/12/15 15:33:06  cmc
      #79055# OCX FormControls changes

      Revision 1.5  2000/12/04 14:08:08  khz
      #78930# Pictures in Hyperlinks will be imported as Graphics with Hyperlink

      Revision 1.4  2000/11/23 13:37:53  khz
      #79474# Save/restore PLCF state before/after reading header or footer data

      Revision 1.3  2000/11/20 14:11:52  jp
      Read_FieldIniFlags removed

      Revision 1.2  2000/11/15 14:31:46  jp
      GetFilterFlags: don't insert SW-Objects

      Revision 1.1.1.1  2000/09/18 17:14:58  hr
      initial import

      Revision 1.143  2000/09/18 16:04:59  willem.vandorp
      OpenOffice header added.

      Revision 1.142  2000/08/28 14:54:44  khz
      #64941# compare Hd./Ft./Section info to prevent prohibited TxBx linkage

      Revision 1.141  2000/08/22 17:08:54  cmc
      #77743# OLE Import, bad seek & bad FilterOptions

      Revision 1.140  2000/08/18 09:48:24  khz
      Import Line Numbering (restart on new section)

      Revision 1.139  2000/08/18 06:47:22  khz
      Import Line Numbering

      Revision 1.138  2000/08/04 10:56:49  jp
      Soft-/HardHyphens & HardBlanks changed from attribute to unicode character

      Revision 1.137  2000/07/28 15:37:14  khz
      #73796# don't delete NumRule from Attr but set it into pDoc

      Revision 1.136  2000/07/27 10:21:30  khz
      #73796# stop ANList when opening next cell in a row and !pAktANLD->fNumberAcross

      Revision 1.135  2000/07/17 13:47:00  khz
      #73987# check if sprmSNfcPgn should cause section change or not

      Revision 1.134  2000/07/12 12:20:30  khz
      #76503# use SwFltStackEntry.bOld to mark attributes before skipping field

      Revision 1.133  2000/07/10 12:52:37  jp
      new Errormessage

      Revision 1.132  2000/06/29 20:59:03  jp
      new MS Filteroptions - change OLE-Objects to StarOffice Objects

      Revision 1.131  2000/06/28 08:07:44  khz
      #70915# Insert Section if end-note with flag 'on end of section' found

      Revision 1.130  2000/06/23 10:30:01  khz
      #71707# Make sure pNdNum is set to zero when no numbering on node

      Revision 1.129  2000/06/21 12:19:15  khz
      Task #74876 teilbehoben.

      Revision 1.128  2000/05/31 12:22:50  khz
      Changes for Unicode

      Revision 1.127  2000/05/25 08:06:41  khz
      Piece Table optimization, Unicode changes, Bugfixes

      Revision 1.126  2000/05/18 10:58:55  jp
      Changes for Unicode

      Revision 1.125  2000/05/16 12:13:01  jp
      ASS_FALSE define removed

      Revision 1.124  2000/05/16 11:21:54  khz
      Unicode code-conversion

      Revision 1.123  2000/05/05 16:26:00  cmc
      #75358# WW8 97Controls Import Fix

      Revision 1.122  2000/05/05 15:59:24  khz
      Task #74474# don't create Sections while skipping result of multi-column index-field

      Revision 1.121  2000/05/04 07:43:51  khz
      Task #75379# use new Save() and Restore()

      Revision 1.120  2000/04/17 12:01:57  khz
      Task #74837# prevent from breaking table (in Apo) when Apo-flag is missing

      Revision 1.119  2000/04/12 11:08:31  khz
      Task #74260# import FLY_PAGE-frames in Header/Footer as FLY_AT_CNTNT

      Revision 1.118  2000/04/07 09:38:20  khz
      Task #74468# insert new node when Pagebreak is in NOT EMPTY node

      Revision 1.117  2000/03/01 13:32:20  jp
      Bug #73683#: LoadDoc1 - call setredlinemode after the destroying of the pam

      Revision 1.116  2000/02/25 15:03:33  jp
      Task #72781#: new filter options for MS-filters

      Revision 1.115  2000/02/22 16:23:23  khz
      Task #72987# Ignore Sprms covering ONLY a Tab-Row-End Char #7

      Revision 1.114  2000/02/21 13:08:23  jp
      #70473# changes for unicode

      Revision 1.113  2000/02/18 09:35:14  cmc
      #69372# Improved Hyperlink Importing for WW97

      Revision 1.107  2000/02/09 11:24:20  khz
      Task #70473# char_t --> sal_Unicode

      Revision 1.106  2000/02/02 18:12:04  jp
      Task #72579#: WW8Reader can import glossaries

      Revision 1.105  2000/02/02 18:07:35  khz
      Task #69885# Read SPRMs that are stored in piece table grpprls

      Revision 1.104  2000/01/26 18:55:59  jp
      Task #72268#: flag for stored M$-VBasic storage

      Revision 1.103  2000/01/26 10:27:09  cmc
      #72268# Visual Basic Decompression

      Revision 1.102  2000/01/14 11:27:56  khz
      Task #68832# -- CORRECTION: Task Number of rev below should have been: 68832 !

      Revision 1.101  2000/01/14 11:11:31  khz
      Task #71343# look for Sprm 37 (not 5) to recognize APO

      Revision 1.100  2000/01/10 13:38:52  jp
      Task #64574#: LoadDoc1 - calculate the correct Z-Order for the SDR-Objects

      Revision 1.99  2000/01/10 12:01:58  jp
      Task #69283#: LoadDoc1 - set redline mode after the last content/attribut is inserted

      Revision 1.98  2000/01/06 15:23:41  khz
      Task #71411# Let last Section be unbalanced

      Revision 1.97  1999/12/23 16:08:47  khz
      Task #70788# Set bStartLine on each char 0x0c

      Revision 1.96  1999/12/23 14:02:58  khz
      Task #68143# avoid attributes atached solely to Cell-End marks

      Revision 1.95  1999/12/22 18:03:09  khz
      Task #70919# look if ParaStyle is different behind filed than it was before

      Revision 1.94  1999/12/21 09:25:51  khz
      Task #71111# Prevent from reading CONTENT when skiping field attributes

*************************************************************************/
