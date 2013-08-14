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

#include <tools/stream.hxx>
#include <poolfmt.hxx>
#include <shellio.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <pam.hxx>
#include <swerror.h>

#include <unotextrange.hxx>

#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/text/XTextRange.hpp>

using namespace ::com::sun::star;

/// Glue class to call RtfImport as an internal filter, needed by copy&paste support.
class SwRTFReader : public Reader
{
    virtual sal_uLong Read( SwDoc &, const OUString& rBaseURL, SwPaM &,const OUString &);
};

sal_uLong SwRTFReader::Read( SwDoc &rDoc, const OUString& /*rBaseURL*/, SwPaM& rPam, const OUString &)
{
    if (!pStrm)
        return ERR_SWG_READ_ERROR;

    // We want to work in an empty paragraph.
    // Step 1: XTextRange will be updated when content is inserted, so we know
    // the end position.
    const uno::Reference<text::XTextRange> xInsertPosition =
        SwXTextRange::CreateXTextRange(rDoc, *rPam.GetPoint(), 0);
    SwNodeIndex *pSttNdIdx = new SwNodeIndex(rDoc.GetNodes());
    const SwPosition* pPos = rPam.GetPoint();

    // Step 2: Split once and remember the node that has been split.
    rDoc.SplitNode( *pPos, false );
    *pSttNdIdx = pPos->nNode.GetIndex()-1;

    // Step 3: Split again.
    rDoc.SplitNode( *pPos, false );

    // Step 4: Insert all content into the new node
    rPam.Move( fnMoveBackward );
    rDoc.SetTxtFmtColl
        ( rPam, rDoc.GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ) );

    SwDocShell *pDocShell(rDoc.GetDocShell());
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<uno::XInterface> xInterface(xMultiServiceFactory->createInstance(
        "com.sun.star.comp.Writer.RtfFilter"), uno::UNO_QUERY_THROW);

    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY_THROW);
    uno::Reference<lang::XComponent> xDstDoc(pDocShell->GetModel(), uno::UNO_QUERY_THROW);
    xImporter->setTargetDocument(xDstDoc);

    const uno::Reference<text::XTextRange> xInsertTextRange =
        SwXTextRange::CreateXTextRange(rDoc, *rPam.GetPoint(), 0);

    uno::Reference<document::XFilter> xFilter(xInterface, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aDescriptor(3);
    aDescriptor[0].Name = "InputStream";
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStrm));
    aDescriptor[0].Value <<= xStream;
    aDescriptor[1].Name = "IsNewDoc";
    aDescriptor[1].Value <<= sal_False;
    aDescriptor[2].Name = "TextInsertModeRange";
    aDescriptor[2].Value <<= xInsertTextRange;
    sal_uLong ret(0);
    try {
        xFilter->filter(aDescriptor);
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("sw.rtf", "SwRTFReader::Read(): exception: " << e.Message);
        ret = ERR_SWG_READ_ERROR;
    }

    // Clean up the fake paragraphs.
    SwUnoInternalPaM aPam(rDoc);
    ::sw::XTextRangeToSwPaM(aPam, xInsertPosition);
    if (pSttNdIdx->GetIndex())
    {
        // If we are in insert mode, join the split node that is in front
        // of the new content with the first new node. Or in other words:
        // Revert the first split node.
        SwTxtNode* pTxtNode = pSttNdIdx->GetNode().GetTxtNode();
        SwNodeIndex aNxtIdx( *pSttNdIdx );
        if( pTxtNode && pTxtNode->CanJoinNext( &aNxtIdx ) &&
                pSttNdIdx->GetIndex() + 1 == aNxtIdx.GetIndex() )
        {
            // If the PaM points to the first new node, move the PaM to the
            // end of the previous node.
            if( aPam.GetPoint()->nNode == aNxtIdx )
            {
                aPam.GetPoint()->nNode = *pSttNdIdx;
                aPam.GetPoint()->nContent.Assign( pTxtNode,
                        pTxtNode->GetTxt().getLength() );
            }
            // If the first new node isn't empty, convert  the node's text
            // attributes into hints. Otherwise, set the new node's
            // paragraph style at the previous (empty) node.
            SwTxtNode* pDelNd = aNxtIdx.GetNode().GetTxtNode();
            if (pTxtNode->GetTxt().getLength())
                pDelNd->FmtToTxtAttr( pTxtNode );
            else
                pTxtNode->ChgFmtColl( pDelNd->GetTxtColl() );
            pTxtNode->JoinNext();
        }
    }

    return ret;
}

extern "C" SAL_DLLPUBLIC_EXPORT Reader* SAL_CALL ImportRTF()
{
    return new SwRTFReader();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
