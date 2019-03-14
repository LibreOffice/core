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

#include "swdocxreader.hxx"

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XElement.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <ndtxt.hxx>
#include <poolfmt.hxx>
#include <svl/urihelper.hxx>
#include <swerror.h>
#include <tools/ref.hxx>
#include <unotxdoc.hxx>
#include <unotools/streamwrap.hxx>
#include <unotextrange.hxx>
#include <sfx2/docfile.hxx>
#include <sal/log.hxx>
#define AUTOTEXT_GALLERY "autoTxt"

using namespace css;

extern "C" SAL_DLLPUBLIC_EXPORT Reader* ImportDOCX()
{
    return new SwDOCXReader;
}

ErrCode SwDOCXReader::Read(SwDoc& rDoc, const OUString& /* rBaseURL */, SwPaM& rPam, const OUString& /* FileName */ )
{
    if (!m_pMedium->GetInStream())
        return ERR_SWG_READ_ERROR;

    // We want to work in an empty paragraph.
    std::shared_ptr<SwNodeIndex> pSttNdIdx(new SwNodeIndex(rDoc.GetNodes()));
    const SwPosition* pPos = rPam.GetPoint();
    rDoc.getIDocumentContentOperations().SplitNode(*pPos, false);
    rDoc.SetTextFormatColl(rPam, rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD, false));

    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<uno::XInterface> xInterface(xMultiServiceFactory->createInstance("com.sun.star.comp.Writer.WriterFilter"), uno::UNO_QUERY_THROW);

    SwDocShell* pDocShell(rDoc.GetDocShell());
    uno::Reference<lang::XComponent> xDstDoc(pDocShell->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY_THROW);
    xImporter->setTargetDocument(xDstDoc);

    const uno::Reference<text::XTextRange> xInsertTextRange = SwXTextRange::CreateXTextRange(rDoc, *rPam.GetPoint(), nullptr);
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*m_pMedium->GetInStream()));

    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while its considered uninitialized,
    //setting a property will inform the document its modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    pDocShell->SetLoading(SfxLoadedFlags::NONE);

    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence(
    {
        { "InputStream", uno::Any(xStream) },
        { "InsertMode", uno::Any(true) },
        { "TextInsertModeRange", uno::Any(xInsertTextRange) }
    }));

    ErrCode ret = ERRCODE_NONE;
    uno::Reference<document::XFilter> xFilter(xInterface, uno::UNO_QUERY_THROW);
    try
    {
        xFilter->filter(aDescriptor);
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("sw.docx", "SwDOCXReader::Read(): " << e);
        ret = ERR_SWG_READ_ERROR;
    }
    pDocShell->SetLoading(SfxLoadedFlags::ALL);

    return ret;
}

SwReaderType SwDOCXReader::GetReaderType()
{
    return SwReaderType::Storage | SwReaderType::Stream;
}

bool SwDOCXReader::HasGlossaries() const
{
    // TODO
    return true;
}

bool SwDOCXReader::ReadGlossaries( SwTextBlocks& rBlocks, bool /* bSaveRelFiles */ ) const
{
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(
                comphelper::getProcessServiceFactory() );

    uno::Reference<uno::XInterface> xInterface(
                xMultiServiceFactory->createInstance( "com.sun.star.comp.Writer.WriterFilter" ),
                uno::UNO_QUERY_THROW );

    uno::Reference<document::XFilter> xFilter( xInterface, uno::UNO_QUERY_THROW );
    uno::Reference<document::XImporter> xImporter( xFilter, uno::UNO_QUERY_THROW );

    SfxObjectShellLock xDocSh( new SwDocShell( SfxObjectCreateMode::INTERNAL ) );
    if( xDocSh->DoInitNew() )
    {
        uno::Reference<lang::XComponent> xDstDoc( xDocSh->GetModel(), uno::UNO_QUERY_THROW );
        xImporter->setTargetDocument( xDstDoc );

        uno::Reference<io::XStream> xStream( new utl::OStreamWrapper( *m_pMedium->GetInStream() ) );

        uno::Sequence<beans::PropertyValue> aDescriptor( comphelper::InitPropertySequence({
                { "InputStream", uno::Any(xStream) },
                { "ReadGlossaries", uno::Any(true) }
            }));

        if( xFilter->filter( aDescriptor ) )
        {
            if (rBlocks.StartPutMuchBlockEntries())
            {
                bool bRet = MakeEntries(static_cast<SwDocShell*>(&xDocSh)->GetDoc(), rBlocks);
                rBlocks.EndPutMuchBlockEntries();
                return bRet;
            }
        }
    }

    return false;
}

bool SwDOCXReader::MakeEntries( SwDoc *pD, SwTextBlocks &rBlocks )
{
    const OUString aOldURL( rBlocks.GetBaseURL() );
    rBlocks.SetBaseURL( OUString() );

    bool bRet = false;

    SwNodeIndex aDocEnd( pD->GetNodes().GetEndOfContent() );
    SwNodeIndex aStart( *aDocEnd.GetNode().StartOfSectionNode(), 1 );
    bool bIsAutoText = false;

    if( aStart < aDocEnd && ( aDocEnd.GetIndex() - aStart.GetIndex() > 2 ) )
    {
        SwTextFormatColl* pColl = pD->getIDocumentStylePoolAccess().GetTextCollFromPool
            (RES_POOLCOLL_STANDARD, false);
        SwContentNode* pCNd = nullptr;
        bRet = true;
        do {
            // Get name - first paragraph
            OUString aLNm;
            {
                SwPaM aPam( aStart );
                SwNodeIndex& rIdx = aPam.GetPoint()->nNode;
                ++rIdx;
                aLNm = aPam.GetNode().GetTextNode()->GetText();

                // is AutoText?
                bIsAutoText = aLNm.startsWith(AUTOTEXT_GALLERY);
                aLNm = aLNm.copy(strlen(AUTOTEXT_GALLERY) + 1);
            }

            // Do not copy name
            aStart++;

            // Get content
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
                // don't add extra empty text node if exist (.dotx but not .dotm)
                if( rIdx.GetNode().GetTextNode() &&
                    rIdx.GetNode().GetTextNode()->GetText().isEmpty() )
                    rIdx = aStart.GetNode().EndOfSectionIndex() - 2;
                if( nullptr == ( pCNd = rIdx.GetNode().GetContentNode() ) )
                {
                    ++rIdx;
                    pCNd = pD->GetNodes().MakeTextNode( rIdx, pColl );
                    rIdx = *pCNd;
                }
            }
            aPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );

            if( bIsAutoText )
            {
                // Now we have the right selection for one entry
                rBlocks.ClearDoc();

                OUString sShortcut = aLNm;

                // Need to check make sure the shortcut is not already being used
                sal_Int32 nStart = 0;
                sal_uInt16 nCurPos = rBlocks.GetIndex( sShortcut );

                while( sal_uInt16(-1) != nCurPos )
                {
                    // add an Number to it
                    sShortcut = aLNm + OUString::number( ++nStart );
                    nCurPos = rBlocks.GetIndex( sShortcut );
                }

                if( rBlocks.BeginPutDoc( sShortcut, sShortcut ) )
                {
                    SwDoc* pGlDoc = rBlocks.GetDoc();
                    SwNodeIndex aIdx( pGlDoc->GetNodes().GetEndOfContent(), -1 );
                    pCNd = aIdx.GetNode().GetContentNode();
                    SwPosition aPos( aIdx, SwIndex( pCNd, pCNd ? pCNd->Len() : 0 ) );
                    pD->getIDocumentContentOperations().CopyRange( aPam, aPos, /*bCopyAll=*/false, /*bCheckPos=*/true );
                    rBlocks.PutDoc();
                }
                else
                {
                    bRet = false;
                }
            }

            aStart = aStart.GetNode().EndOfSectionIndex() + 1;
        } while( aStart < aDocEnd && aStart.GetNode().IsStartNode() );
    }

    rBlocks.SetBaseURL( aOldURL );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
