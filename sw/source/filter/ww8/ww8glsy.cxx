/*************************************************************************
 *
 *  $RCSfile: ww8glsy.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 14:17:20 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */


#pragma hdrstop

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _NDTXT
#include <ndtxt.hxx>
#endif
#include <pam.hxx>
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
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

#ifndef _WW8GLSY_HXX
#include "ww8glsy.hxx"
#endif
#ifndef _WW8PAR_HXX
#include "ww8par.hxx"
#endif

WW8Glossary::WW8Glossary(SvStorageStreamRef &refStrm, BYTE nVersion,
    SvStorage *pStg)
    : pGlossary(0), rStrm(refStrm), xStg(pStg), nStrings(0)
{
    refStrm->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
    WW8Fib aWwFib(*refStrm, nVersion);

    if (aWwFib.nFibBack >= 0x6A)   //Word97
    {
        xTableStream = pStg->OpenStream(String::CreateFromAscii(
            aWwFib.fWhichTblStm ? SL::a1Table : SL::a0Table), STREAM_STD_READ);

        if (xTableStream.Is() && SVSTREAM_OK == xTableStream->GetError())
        {
            xTableStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
            pGlossary =
                new WW8GlossaryFib(*refStrm, nVersion, *xTableStream, aWwFib);
        }
    }
}

bool WW8Glossary::HasBareGraphicEnd(SwDoc *pDoc,SwNodeIndex &rIdx)
{
    bool bRet=false;
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
                bRet=true;
                break;
            }
    }
    return bRet;
}

bool WW8Glossary::MakeEntries(SwDoc *pD, SwTextBlocks &rBlocks,
    bool bSaveRelFile, const std::vector<String>& rStrings,
    const std::vector<String>& rExtra)
{
    // this code will be called after reading all text into the
    // empty sections
    const String aOldURL( INetURLObject::GetBaseURL() );
    bool bRet=false;
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
        SwTxtFmtColl* pColl = pD->GetTxtCollFromPoolSimple
            (RES_POOLCOLL_STANDARD, false);
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

            // now we have the right selection for one entry.  Copy this to
            // the definied TextBlock, but only if it is not an autocorrection
            // entry (== -1) otherwise the group indicates the group in the
            // sttbfglsystyle list that this entry belongs to. Unused at the
            // moment
            if ((rExtra[nGlosEntry].ToInt32() + 2) != -1)
            {
                rBlocks.ClearDoc();
                const String &rLNm = rStrings[nGlosEntry];

                String sShortcut = rLNm;

                // Need to check make sure the shortcut is not already being used
                xub_StrLen nStart = 0;
                USHORT nCurPos = rBlocks.GetIndex( sShortcut );
                xub_StrLen nLen = sShortcut.Len();
                while( (USHORT)-1 != nCurPos )
                {
                    sShortcut.Erase( nLen ) +=
                        String::CreateFromInt32( ++nStart );    // add an Number to it
                    nCurPos = rBlocks.GetIndex( sShortcut );
                }

                if( rBlocks.BeginPutDoc( sShortcut, sShortcut ))    // Make the shortcut and the name the same

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
        bRet=true;
    }

// this code will be called after reading all text into the empty sections

    INetURLObject::SetBaseURL( aOldURL );
    return bRet;
}


bool WW8Glossary::Load( SwTextBlocks &rBlocks, bool bSaveRelFile )
{
    bool bRet=false;
    if (pGlossary && pGlossary->IsGlossaryFib() && rBlocks.StartPutMuchBlockEntries())
    {
        //read the names of the autotext entries
        std::vector<String> aStrings;
        std::vector<String> aExtra;

        rtl_TextEncoding eStructCharSet =
            WW8Fib::GetFIBCharset(pGlossary->chseTables);

        WW8ReadSTTBF(true, *xTableStream, pGlossary->fcSttbfglsy,
            pGlossary->lcbSttbfglsy, 0, eStructCharSet, aStrings, &aExtra );

        rStrm->Seek(0);

        if ((nStrings = aStrings.size()))
        {
            SfxObjectShellRef xDocSh(new SwDocShell(SFX_CREATE_MODE_INTERNAL));
            if (xDocSh->DoInitNew(0))
            {
                SwDoc *pD =  ((SwDocShell*)(&xDocSh))->GetDoc();
                SwWW8ImplReader* pRdr = new SwWW8ImplReader(pGlossary->nVersion,
                    xStg, &rStrm, *pD, true);

                SwNodeIndex aIdx(
                    *pD->GetNodes().GetEndOfContent().StartOfSectionNode(), 1);
                if( !aIdx.GetNode().IsTxtNode() )
                {
                    ASSERT( !this, "wo ist der TextNode?" );
                    pD->GetNodes().GoNext( &aIdx );
                }
                SwPaM aPamo( aIdx );
                aPamo.GetPoint()->nContent.Assign(aIdx.GetNode().GetCntntNode(),
                    0);
                pRdr->LoadDoc(aPamo,this);

                bRet = MakeEntries(pD, rBlocks, bSaveRelFile, aStrings, aExtra);

                delete pRdr;
            }
            xDocSh->DoClose();
            rBlocks.EndPutMuchBlockEntries();
        }
    }
    return bRet;
}


bool WW8GlossaryFib::IsGlossaryFib()
{
    if (!nFibError)
    {
        INT16 nFibMin;
        INT16 nFibMax;
        switch(nVersion)
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
                nFibMax = 0x00c2;   // bis 194 WinWord 2000
                break;
            default:
                nFibMin = 0;            // Programm-Fehler!
                nFibMax = 0;
                nFib = nFibBack = 1;
                break;
        }
        if ( (nFibBack < nFibMin) || (nFibBack > nFibMax) )
            nFibError = ERR_SWG_READ_ERROR; // Error melden
    }
    return !nFibError;
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

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
