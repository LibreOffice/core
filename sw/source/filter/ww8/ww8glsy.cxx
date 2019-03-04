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

#include <memory>
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
#include <IDocumentStylePoolAccess.hxx>
#include <docary.hxx>
#include "ww8glsy.hxx"
#include "ww8par.hxx"
#include "ww8par2.hxx"

WW8Glossary::WW8Glossary(tools::SvRef<SotStorageStream> &refStrm, sal_uInt8 nVersion, SotStorage *pStg)
    : rStrm(refStrm)
    , xStg(pStg)
    , nStrings(0)
{
    refStrm->SetEndian(SvStreamEndian::LITTLE);
    WW8Fib aWwFib(*refStrm, nVersion);

    if (aWwFib.m_nFibBack >= 0x6A)   //Word97
    {
        xTableStream = pStg->OpenSotStream(
            aWwFib.m_fWhichTableStm ? SL::a1Table : SL::a0Table, StreamMode::STD_READ);

        if (xTableStream.is() && ERRCODE_NONE == xTableStream->GetError())
        {
            xTableStream->SetEndian(SvStreamEndian::LITTLE);
            xGlossary.reset(new WW8GlossaryFib(*refStrm, nVersion, aWwFib));
        }
    }
}

bool WW8Glossary::HasBareGraphicEnd(SwDoc *pDoc,SwNodeIndex const &rIdx)
{
    bool bRet=false;
    for( sal_uInt16 nCnt = pDoc->GetSpzFrameFormats()->size(); nCnt; )
    {
        const SwFrameFormat* pFrameFormat = (*pDoc->GetSpzFrameFormats())[ --nCnt ];
        if ( RES_FLYFRMFMT != pFrameFormat->Which() &&
            RES_DRAWFRMFMT != pFrameFormat->Which() )
                continue;
        const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
        SwPosition const*const pAPos = rAnchor.GetContentAnchor();
        if (pAPos &&
            ((RndStdIds::FLY_AT_PARA == rAnchor.GetAnchorId()) ||
             (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId())) &&
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
    const OUString aOldURL( rBlocks.GetBaseURL() );
    bool bRet=false;
    if( bSaveRelFile )
    {
        rBlocks.SetBaseURL(
            URIHelper::SmartRel2Abs(
                INetURLObject(), rBlocks.GetFileName(),
                URIHelper::GetMaybeFileHdl()));
    }
    else
        rBlocks.SetBaseURL( OUString() );

    SwNodeIndex aDocEnd( pD->GetNodes().GetEndOfContent() );
    SwNodeIndex aStart( *aDocEnd.GetNode().StartOfSectionNode(), 1 );

    // search the first NormalStartNode
    while( !( aStart.GetNode().IsStartNode() && SwNormalStartNode ==
           aStart.GetNode().GetStartNode()->GetStartNodeType()) &&
            aStart < aDocEnd )
        ++aStart;

    if( aStart < aDocEnd )
    {
        SwTextFormatColl* pColl = pD->getIDocumentStylePoolAccess().GetTextCollFromPool
            (RES_POOLCOLL_STANDARD, false);
        sal_uInt16 nGlosEntry = 0;
        SwContentNode* pCNd = nullptr;
        do {
            SwPaM aPam( aStart );
            {
                SwNodeIndex& rIdx = aPam.GetPoint()->nNode;
                ++rIdx;
                if( nullptr == ( pCNd = rIdx.GetNode().GetTextNode() ) )
                {
                    pCNd = pD->GetNodes().MakeTextNode( rIdx, pColl );
                    rIdx = *pCNd;
                }
            }
            aPam.GetPoint()->nContent.Assign( pCNd, 0 );
            aPam.SetMark();
            {
                SwNodeIndex& rIdx = aPam.GetPoint()->nNode;
                rIdx = aStart.GetNode().EndOfSectionIndex() - 1;
                if(( nullptr == ( pCNd = rIdx.GetNode().GetContentNode() ) )
                        || HasBareGraphicEnd(pD,rIdx))
                {
                    ++rIdx;
                    pCNd = pD->GetNodes().MakeTextNode( rIdx, pColl );
                    rIdx = *pCNd;
                }
            }
            aPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );

            // now we have the right selection for one entry.  Copy this to
            // the defined TextBlock, but only if it is not an autocorrection
            // entry (== -1) otherwise the group indicates the group in the
            // sttbfglsystyle list that this entry belongs to. Unused at the
            // moment
            const ww::bytes &rData = rExtra[nGlosEntry];
            sal_uInt16 n = SVBT16ToUInt16( &(rData[2]) );
            if(n != 0xFFFF)
            {
                rBlocks.ClearDoc();
                const OUString &rLNm = rStrings[nGlosEntry];

                OUString sShortcut = rLNm;

                // Need to check make sure the shortcut is not already being used
                sal_Int32 nStart = 0;
                sal_uInt16 nCurPos = rBlocks.GetIndex( sShortcut );
                while( sal_uInt16(-1) != nCurPos )
                {
                    sShortcut = rLNm + OUString::number(++nStart);    // add an Number to it
                    nCurPos = rBlocks.GetIndex( sShortcut );
                }

                if( rBlocks.BeginPutDoc( sShortcut, sShortcut ))    // Make the shortcut and the name the same

                {
                    SwDoc* pGlDoc = rBlocks.GetDoc();
                    SwNodeIndex aIdx( pGlDoc->GetNodes().GetEndOfContent(),
                        -1 );
                    pCNd = aIdx.GetNode().GetContentNode();
                    SwPosition aPos(aIdx, SwIndex(pCNd, pCNd ? pCNd->Len() : 0));
                    pD->getIDocumentContentOperations().CopyRange( aPam, aPos, /*bCopyAll=*/false, /*bCheckPos=*/true );
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
    if (xGlossary && xGlossary->IsGlossaryFib() && rBlocks.StartPutMuchBlockEntries())
    {
        //read the names of the autotext entries
        std::vector<OUString> aStrings;
        std::vector<ww::bytes> aData;

        rtl_TextEncoding eStructCharSet =
            WW8Fib::GetFIBCharset(xGlossary->m_chseTables, xGlossary->m_lid);

        WW8ReadSTTBF(true, *xTableStream, xGlossary->m_fcSttbfglsy,
            xGlossary->m_lcbSttbfglsy, 0, eStructCharSet, aStrings, &aData );

        rStrm->Seek(0);

        if ( 0 != (nStrings = static_cast< sal_uInt16 >(aStrings.size())))
        {
            SfxObjectShellLock xDocSh(new SwDocShell(SfxObjectCreateMode::INTERNAL));
            if (xDocSh->DoInitNew())
            {
                SwDoc *pD =  static_cast<SwDocShell*>((&xDocSh))->GetDoc();

                SwNodeIndex aIdx(
                    *pD->GetNodes().GetEndOfContent().StartOfSectionNode(), 1);
                if( !aIdx.GetNode().IsTextNode() )
                {
                    OSL_ENSURE( false, "Where is the TextNode?" );
                    pD->GetNodes().GoNext( &aIdx );
                }
                SwPaM aPamo( aIdx );
                aPamo.GetPoint()->nContent.Assign(aIdx.GetNode().GetContentNode(),
                    0);
                std::unique_ptr<SwWW8ImplReader> xRdr(new SwWW8ImplReader(
                    xGlossary->m_nVersion, xStg.get(), rStrm.get(), *pD, rBlocks.GetBaseURL(),
                    true, false, *aPamo.GetPoint()));
                xRdr->LoadDoc(this);
                bRet = MakeEntries(pD, rBlocks, bSaveRelFile, aStrings, aData);
            }
            xDocSh->DoClose();
            rBlocks.EndPutMuchBlockEntries();
        }
    }
    return bRet;
}

sal_uInt32 WW8GlossaryFib::FindGlossaryFibOffset(const WW8Fib &rFib)
{
    sal_uInt32 nGlossaryFibOffset = 0;
    if ( rFib.m_fDot ) // it's a template
    {
        if ( rFib.m_pnNext  )
            nGlossaryFibOffset = ( rFib.m_pnNext * 512 );
    }
    return nGlossaryFibOffset;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
