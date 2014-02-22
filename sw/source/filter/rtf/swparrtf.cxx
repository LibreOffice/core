/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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


class SwRTFReader : public Reader
{
    virtual sal_uLong Read( SwDoc &, const OUString& rBaseURL, SwPaM &,const OUString &);
};

sal_uLong SwRTFReader::Read( SwDoc &rDoc, const OUString& /*rBaseURL*/, SwPaM& rPam, const OUString &)
{
    if (!pStrm)
        return ERR_SWG_READ_ERROR;

    
    
    
    const uno::Reference<text::XTextRange> xInsertPosition =
        SwXTextRange::CreateXTextRange(rDoc, *rPam.GetPoint(), 0);
    SwNodeIndex *pSttNdIdx = new SwNodeIndex(rDoc.GetNodes());
    const SwPosition* pPos = rPam.GetPoint();

    
    rDoc.SplitNode( *pPos, false );
    *pSttNdIdx = pPos->nNode.GetIndex()-1;

    
    rDoc.SplitNode( *pPos, false );

    
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
    aDescriptor[1].Name = "InsertMode";
    aDescriptor[1].Value <<= sal_True;
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

    
    SwUnoInternalPaM aPam(rDoc);
    ::sw::XTextRangeToSwPaM(aPam, xInsertPosition);
    if (pSttNdIdx->GetIndex())
    {
        
        
        
        SwTxtNode* pTxtNode = pSttNdIdx->GetNode().GetTxtNode();
        SwNodeIndex aNxtIdx( *pSttNdIdx );
        if( pTxtNode && pTxtNode->CanJoinNext( &aNxtIdx ) &&
                pSttNdIdx->GetIndex() + 1 == aNxtIdx.GetIndex() )
        {
            
            
            if( aPam.GetPoint()->nNode == aNxtIdx )
            {
                aPam.GetPoint()->nNode = *pSttNdIdx;
                aPam.GetPoint()->nContent.Assign( pTxtNode,
                        pTxtNode->GetTxt().getLength() );
            }
            
            
            
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
