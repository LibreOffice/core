/*************************************************************************
 *
 *  $RCSfile: ww8glsy.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:58 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _WW8GLSY_HXX
#include <ww8glsy.hxx>
#endif
#ifndef _WW8PAR_HXX
#include <ww8par.hxx>
#endif
#ifndef _NDTXT
#include <ndtxt.hxx>
#endif
#include <pam.hxx>
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen wg. SwFmtAnchor
#include <fmtanchr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen wg. SwFrmFmt
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX //autogen wg. SwDoc
#include <docary.hxx>
#endif


WW8Glossary::WW8Glossary(SvStorageStreamRef &refStrm,BYTE nVersion,
    SvStorage *pStg) : rStrm(refStrm), xStg(pStg), nStrings( 0 )
{
    refStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    WW8Fib aWwFib( *refStrm, nVersion );

    String sTblNm( WW8_ASCII2STR( "1Table" ));
    if( 1 != aWwFib.fWhichTblStm )
        sTblNm.SetChar( 0, '0' );

    xTableStream = pStg->OpenStream( sTblNm, STREAM_STD_READ );
    xTableStream->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    pGlossary = new WW8GlossaryFib( *refStrm, nVersion,*xTableStream, aWwFib );
}

BOOL WW8Glossary::HasBareGraphicEnd(SwDoc *pDoc,SwNodeIndex &rIdx)
{
    BOOL bRet=FALSE;
    for( USHORT nCnt = pDoc->GetSpzFrmFmts()->Count(); nCnt; )
    {
        SwFrmFmt* pFrmFmt = (*pDoc->GetSpzFrmFmts())[ --nCnt ];
        if ( RES_FLYFRMFMT != pFrmFmt->Which() &&
            RES_DRAWFRMFMT != pFrmFmt->Which() )
                continue;
        const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
        const SwPosition* pAPos;
        if( ( FLY_AT_CNTNT == rAnchor.GetAnchorId() ||
            FLY_AUTO_CNTNT == rAnchor.GetAnchorId() ) &&
            0 != ( pAPos = rAnchor.GetCntntAnchor()) &&
            rIdx == pAPos->nNode.GetIndex() )
            {
                bRet=TRUE;
                break;
            }
    }
    return bRet;
}

BOOL WW8Glossary::MakeEntries( SwDoc *pD, SwTextBlocks &rBlocks,
                                BOOL bSaveRelFile, SvStrings& rStrings,
                                SvStrings& rExtra )
{
    // this code will be called after reading all text into the
    // empty sections
    const String aOldURL( INetURLObject::GetBaseURL() );
    BOOL bRet=FALSE;
    if( bSaveRelFile )
    {
        INetURLObject::SetBaseURL(
            URIHelper::SmartRelToAbs( rBlocks.GetFileName()));
    }
    else
        INetURLObject::SetBaseURL( aEmptyStr );

    SwNodeIndex aDocEnd( pD->GetNodes().GetEndOfContent() );
    SwNodeIndex aStart( *aDocEnd.GetNode().StartOfSectionNode(), 1 );

    // search the first NormalStartNode
    while( !( aStart.GetNode().IsStartNode() && SwNormalStartNode ==
           aStart.GetNode().GetStartNode()->GetStartNodeType()) &&
            aStart < aDocEnd )
        aStart++;

    if( aStart < aDocEnd )
    {
        SwTxtFmtColl* pColl = pD->GetTxtCollFromPool(
            RES_POOLCOLL_STANDARD );
        USHORT nGlosEntry = 0;
        SwCntntNode* pCNd = 0;
        do {
            SwPaM aPam( aStart );
            {
                SwNodeIndex& rIdx = aPam.GetPoint()->nNode;
                rIdx++;
                if( 0 == ( pCNd = rIdx.GetNode().GetTxtNode() ) )
                {
                    if( rIdx.GetNode().IsTableNode() )
                        pCNd = pD->GetNodes().GoNext( &rIdx );
                    else
                    {
                        pCNd = pD->GetNodes().MakeTxtNode( rIdx, pColl );
                        rIdx = *pCNd;
                    }
                }
            }
            aPam.GetPoint()->nContent.Assign( pCNd, 0 );
            aPam.SetMark();
            {
                SwNodeIndex& rIdx = aPam.GetPoint()->nNode;
                rIdx = aStart.GetNode().EndOfSectionIndex() - 1;
                if(( 0 == ( pCNd = rIdx.GetNode().GetCntntNode() ) )
                        || HasBareGraphicEnd(pD,rIdx))
                {
                    rIdx++;
                    pCNd = pD->GetNodes().MakeTxtNode( rIdx, pColl );
                    rIdx = *pCNd;
                }
            }
            aPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );

            // now we have the right selection for one entry.
            // Copy this to the definied TextBlock, but only if
            // it is not an autocorrection entry (== -1)
            // otherwise the group indicates the group in
            // the sttbfglsystyle list that this entry belongs
            // to. Unused at the moment
            INT16 group = rExtra[nGlosEntry]->ToInt32() + 2;
            if (group != -1)
            {
                rBlocks.ClearDoc();
                String sLNm( *rStrings[nGlosEntry] );
                if( rBlocks.BeginPutDoc( rBlocks.GetValidShortCut(sLNm,TRUE), sLNm ))
                {
                    SwDoc* pGlDoc = rBlocks.GetDoc();
                    SwNodeIndex aIdx( pGlDoc->GetNodes().GetEndOfContent(),
                        -1 );
                    pCNd = aIdx.GetNode().GetCntntNode();
                    SwPosition aPos( aIdx, SwIndex( pCNd, pCNd->Len() ));
                    pD->Copy( aPam, aPos );
                    rBlocks.PutDoc();
                }
            }
            aStart = aStart.GetNode().EndOfSectionIndex() + 1;
            ++nGlosEntry;
        } while( aStart.GetNode().IsStartNode() &&
                SwNormalStartNode == aStart.GetNode().
                    GetStartNode()->GetStartNodeType());
        bRet=TRUE;
    }

// this code will be called after reading all text into the empty sections

    INetURLObject::SetBaseURL( aOldURL );
    return bRet;
}


BOOL WW8Glossary::Load( SwTextBlocks &rBlocks, BOOL bSaveRelFile )
{
    BOOL bRet = FALSE;
    if( pGlossary->IsGlossaryFib() && rBlocks.StartPutMuchBlockEntries() )
    {
        //read the names of the autotext entries
        SvStrings aStrings( 0, 64 ), aExtra( 0, 64 );

        rtl_TextEncoding eStructCharSet = 0x0100 == pGlossary->chseTables
                                            ? RTL_TEXTENCODING_APPLE_ROMAN
                                            : rtl_getTextEncodingFromWindowsCharset(
                                                pGlossary->chseTables );

        WW8ReadSTTBF( TRUE, *xTableStream,
                        pGlossary->fcSttbfglsy, pGlossary->lcbSttbfglsy, 0,
                        eStructCharSet, aStrings, &aExtra );
        rStrm->Seek(0);

    //  SwDoc *pD = new SwDoc;
    //  pD->SetDocShell(new SwDocShell(SFX_CREATE_MODE_INTERNAL));
        SfxObjectShellRef xDocSh( new SwDocShell( SFX_CREATE_MODE_INTERNAL));
        if( xDocSh->DoInitNew( 0 ) )
        {
            SwDoc *pD =  ((SwDocShell*)(&xDocSh))->GetDoc();
            SwWW8ImplReader* pRdr = new
                SwWW8ImplReader( pGlossary->nVersion , xStg, &rStrm, *pD, TRUE );

            SwNodeIndex
                aIdx( *pD->GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
            if( !aIdx.GetNode().IsTxtNode() )
            {
                ASSERT( !this, "wo ist der TextNode?" );
                pD->GetNodes().GoNext( &aIdx );
            }
            SwPaM aPamo( aIdx );
            aPamo.GetPoint()->nContent.Assign( aIdx.GetNode().GetCntntNode(), 0 );
            pRdr->LoadDoc(aPamo,this);

            bRet = MakeEntries( pD, rBlocks, bSaveRelFile, aStrings, aExtra );

            delete pRdr;
        }
        xDocSh->DoClose();
        rBlocks.EndPutMuchBlockEntries();

        nStrings = aStrings.Count();
        aStrings.DeleteAndDestroy( 0, nStrings );
        aExtra.DeleteAndDestroy( 0, aExtra.Count() );
    }
    return bRet;
}


BOOL WW8GlossaryFib::IsGlossaryFib()
{
    if( !nFibError )
    {
        INT16 nFibMin;
        INT16 nFibMax;
        switch( nVersion )  // beachte: 6 steht fuer "6 ODER 7",
                            // 7 steht fuer "NUR 7"
            {
        case 6:
            nFibMin = 0x0065;   // von 101 WinWord 6.0
            //     102    "
            // und 103 WinWord 6.0 fuer Macintosh
            //     104    "
            nFibMax = 0x0069;   // bis 105 WinWord 95
            break;
        case 7:
            nFibMin = 0x0069;   // von 105 WinWord 95
            nFibMax = 0x0069;   // bis 105 WinWord 95
            break;
        case 8:
            nFibMin = 0x006A;   // von 106 WinWord 97
            nFibMax = 0x00c1;   // bis 193 WinWord 97 (?)
            break;
        default:
            nFibMin = 0;            // Programm-Fehler!
            nFibMax = 0;
            nFib    = 1;
        }
        if(    ( nFib < nFibMin )
            || ( nFib > nFibMax ) )
            nFibError = ERR_SWG_READ_ERROR; // Error melden
    }
    return(0 == nFibError);
}

UINT32 WW8GlossaryFib::FindGlossaryFibOffset(SvStream &rTableStrm,
        SvStream &rStrm, const WW8Fib &rFib)
{
    WW8PLCF aPlc( &rTableStrm, rFib.fcPlcfsed, rFib.lcbPlcfsed, 12 );
    long start,ende;
    void *pData;
    aPlc.Get(start,ende,pData);
    UINT32 nPo = SVBT32ToLong((BYTE *)pData+2);
    //*pOut << hex << "Offset of last SEPX is " << nPo << endl;

    UINT16 nLen;
    if (nPo != 0xFFFFFFFF)
    {
        rStrm.Seek(nPo);
        rStrm >> nLen;
    }
    else
    {
        nPo=0;
        nLen=0;
    }

//  *pOut << hex << "Ends at " << nPo+len << endl;
    nPo+=nLen;
    UINT32 nEndLastPage;
    if (nPo%512)
    {
        nEndLastPage = (nPo)/512;
        nEndLastPage = (nEndLastPage+1)*512;
    }
    else
        nEndLastPage = nPo;

    //*pOut << hex << "SECOND FIB SHOULD BE FOUND at " << k << endl;

    WW8PLCF xcPLCF( &rTableStrm, rFib.fcPlcfbteChpx,
            rFib.lcbPlcfbteChpx, (8 > rFib.nVersion) ? 2 : 4);

    xcPLCF.Get(start,ende,pData);

    nPo = SVBT32ToLong((BYTE *)pData);
    //*pOut << hex << "Offset of last CHPX is " << (nPo+1) *512<< endl;
    if (((nPo+1)*512) > nEndLastPage) nEndLastPage = (nPo+1)*512;

    WW8PLCF xpPLCF( &rTableStrm, rFib.fcPlcfbtePapx,
            rFib.lcbPlcfbtePapx, (8 > rFib.nVersion) ? 2 : 4);
    xpPLCF.Get(start,ende,pData);
    nPo = SVBT32ToLong((BYTE *)pData);
    //*pOut << hex << "Offset of last PAPX is " << nPo *512 << endl;
    if (((nPo+1)*512) > nEndLastPage) nEndLastPage = (nPo+1)*512;

    //*pOut << hex << "SECOND FIB SHOULD BE FOUND at " << nEndLastPage << endl;
    return nEndLastPage;
}

/*************************************************************************
      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/ww8glsy.cxx,v 1.1.1.1 2000-09-18 17:14:58 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.8  2000/09/18 16:04:59  willem.vandorp
      OpenOffice header added.

      Revision 1.7  2000/06/26 12:59:10  os
      INetURLObject::SmartRelToAbs removed

      Revision 1.6  2000/05/18 10:58:34  jp
      Changes for Unicode

      Revision 1.5  2000/05/16 11:08:03  khz
      Unicode code-conversion

      Revision 1.4  2000/03/13 10:22:14  jp
      Bug #72579#: little/big endian problem

      Revision 1.3  2000/03/03 14:03:39  os
      precompiled header

      Revision 1.2  2000/02/28 08:33:46  cmc
      #72579# Enable Glossary Import

      Revision 1.1  2000/02/14 09:05:56  cmc
      #72579# added ww8glsy.cxx


*************************************************************************/
