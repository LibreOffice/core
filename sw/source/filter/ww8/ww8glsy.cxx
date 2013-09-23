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


#include <tools/urlobj.hxx>
#include <svl/urihelper.hxx>
#include <rtl/tencinfo.h>
#include <swerror.h>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <shellio.hxx>
#include <docsh.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include "ww8glsy.hxx"
#include "ww8par.hxx"


WW8Glossary::WW8Glossary(SvStorageStreamRef &refStrm, sal_uInt8 nVersion,
    SvStorage *pStg)
    : pGlossary(0), rStrm(refStrm), xStg(pStg), nStrings(0)
{
    refStrm->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
    WW8Fib aWwFib(*refStrm, nVersion);

    if (aWwFib.nFibBack >= 0x6A)   //Word97
    {
        xTableStream = pStg->OpenSotStream(OUString::createFromAscii(
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
    for( sal_uInt16 nCnt = pDoc->GetSpzFrmFmts()->size(); nCnt; )
    {
        SwFrmFmt* pFrmFmt = (*pDoc->GetSpzFrmFmts())[ --nCnt ];
        if ( RES_FLYFRMFMT != pFrmFmt->Which() &&
            RES_DRAWFRMFMT != pFrmFmt->Which() )
                continue;
        const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
        SwPosition const*const pAPos = rAnchor.GetCntntAnchor();
        if (pAPos &&
            ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
             (FLY_AT_CHAR == rAnchor.GetAnchorId())) &&
            rIdx == pAPos->nNode.GetIndex() )
            {
                bRet=true;
                break;
            }
    }
    return bRet;
}

bool WW8Glossary::MakeEntries(SwDoc *pD, SwTextBlocks &rBlocks,
    bool bSaveRelFile, const std::vector<OUString>& rStrings,
    const std::vector<ww::bytes>& rExtra)
{
    // this code will be called after reading all text into the
    // empty sections
    const String aOldURL( rBlocks.GetBaseURL() );
    bool bRet=false;
    if( bSaveRelFile )
    {
        rBlocks.SetBaseURL(
            URIHelper::SmartRel2Abs(
                INetURLObject(), rBlocks.GetFileName(),
                URIHelper::GetMaybeFileHdl()));
    }
    else
        rBlocks.SetBaseURL( aEmptyStr );

    SwNodeIndex aDocEnd( pD->GetNodes().GetEndOfContent() );
    SwNodeIndex aStart( *aDocEnd.GetNode().StartOfSectionNode(), 1 );

    // search the first NormalStartNode
    while( !( aStart.GetNode().IsStartNode() && SwNormalStartNode ==
           aStart.GetNode().GetStartNode()->GetStartNodeType()) &&
            aStart < aDocEnd )
        ++aStart;

    if( aStart < aDocEnd )
    {
        SwTxtFmtColl* pColl = pD->GetTxtCollFromPool
            (RES_POOLCOLL_STANDARD, false);
        sal_uInt16 nGlosEntry = 0;
        SwCntntNode* pCNd = 0;
        do {
            SwPaM aPam( aStart );
            {
                SwNodeIndex& rIdx = aPam.GetPoint()->nNode;
                ++rIdx;
                if( 0 == ( pCNd = rIdx.GetNode().GetTxtNode() ) )
                {
                    pCNd = pD->GetNodes().MakeTxtNode( rIdx, pColl );
                    rIdx = *pCNd;
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
                    ++rIdx;
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
            const ww::bytes &rData = rExtra[nGlosEntry];
            sal_uInt16 n = SVBT16ToShort( &(rData[2]) );
            if(n != 0xFFFF)
            {
                rBlocks.ClearDoc();
                const OUString &rLNm = rStrings[nGlosEntry];

                String sShortcut = rLNm;

                // Need to check make sure the shortcut is not already being used
                sal_Int32 nStart = 0;
                sal_uInt16 nCurPos = rBlocks.GetIndex( sShortcut );
                xub_StrLen nLen = sShortcut.Len();
                while( (sal_uInt16)-1 != nCurPos )
                {
                    sShortcut.Erase( nLen ) +=
                        OUString::number(++nStart);    // add an Number to it
                    nCurPos = rBlocks.GetIndex( sShortcut );
                }

                if( rBlocks.BeginPutDoc( sShortcut, sShortcut ))    // Make the shortcut and the name the same

                {
                    SwDoc* pGlDoc = rBlocks.GetDoc();
                    SwNodeIndex aIdx( pGlDoc->GetNodes().GetEndOfContent(),
                        -1 );
                    pCNd = aIdx.GetNode().GetCntntNode();
                    SwPosition aPos( aIdx, SwIndex( pCNd, pCNd->Len() ));
                    pD->CopyRange( aPam, aPos, false );
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

    rBlocks.SetBaseURL( aOldURL );
    return bRet;
}


bool WW8Glossary::Load( SwTextBlocks &rBlocks, bool bSaveRelFile )
{
    bool bRet=false;
    if (pGlossary && pGlossary->IsGlossaryFib() && rBlocks.StartPutMuchBlockEntries())
    {
        //read the names of the autotext entries
        std::vector<OUString> aStrings;
        std::vector<ww::bytes> aData;

        rtl_TextEncoding eStructCharSet =
            WW8Fib::GetFIBCharset(pGlossary->chseTables);

        WW8ReadSTTBF(true, *xTableStream, pGlossary->fcSttbfglsy,
            pGlossary->lcbSttbfglsy, 0, eStructCharSet, aStrings, &aData );

        rStrm->Seek(0);

        if ( 0 != (nStrings = static_cast< sal_uInt16 >(aStrings.size())))
        {
            SfxObjectShellLock xDocSh(new SwDocShell(SFX_CREATE_MODE_INTERNAL));
            if (xDocSh->DoInitNew(0))
            {
                SwDoc *pD =  ((SwDocShell*)(&xDocSh))->GetDoc();
                SwWW8ImplReader* pRdr = new SwWW8ImplReader(pGlossary->nVersion,
                    xStg, &rStrm, *pD, rBlocks.GetBaseURL(), true);

                SwNodeIndex aIdx(
                    *pD->GetNodes().GetEndOfContent().StartOfSectionNode(), 1);
                if( !aIdx.GetNode().IsTxtNode() )
                {
                    OSL_ENSURE( !this, "wo ist der TextNode?" );
                    pD->GetNodes().GoNext( &aIdx );
                }
                SwPaM aPamo( aIdx );
                aPamo.GetPoint()->nContent.Assign(aIdx.GetNode().GetCntntNode(),
                    0);
                pRdr->LoadDoc(aPamo,this);

                bRet = MakeEntries(pD, rBlocks, bSaveRelFile, aStrings, aData);

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
    // fGlsy will indicate whether this has AutoText or not
    return fGlsy;
}

sal_uInt32 WW8GlossaryFib::FindGlossaryFibOffset(SvStream & /* rTableStrm */,
                                             SvStream & /* rStrm */,
                                             const WW8Fib &rFib)
{
    sal_uInt32 nGlossaryFibOffset = 0;
    if ( rFib.fDot ) // its a template
    {
        if ( rFib.pnNext  )
            nGlossaryFibOffset = ( rFib.pnNext * 512 );
    }
    return nGlossaryFibOffset;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
