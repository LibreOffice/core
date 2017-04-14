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
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <docsh.hxx>
#include <swerror.h>
#include <tools/ref.hxx>
#include <unotxdoc.hxx>
#include <unotools/streamwrap.hxx>

using namespace css;

extern "C" SAL_DLLPUBLIC_EXPORT Reader* SAL_CALL ImportDOCX()
{
    return new SwDOCXReader;
}

sal_uLong SwDOCXReader::Read( SwDoc& /* rDoc */, const OUString& /* rBaseURL */, SwPaM& /* rPaM */, const OUString& /* FileName */ )
{
    return ERR_SWG_READ_ERROR;
}

int SwDOCXReader::GetReaderType()
{
    return SW_STORAGE_READER | SW_STREAM_READER;
}

bool SwDOCXReader::HasGlossaries() const
{
    // TODO
    return true;
}

bool SwDOCXReader::ReadGlossaries( SwTextBlocks& rBlocks, bool /* bSaveRelFiles */ ) const
{
    bool bRet = false;

    uno::Reference<xml::dom::XDocument> xDoc = OpenDocument();

    if( xDoc.is() )
    {
        uno::Reference<xml::dom::XNodeList> xList = xDoc->getElementsByTagName( "docPartBody" );
        for( int i = 0; i < xList->getLength(); i++ )
        {
            uno::Reference<xml::dom::XNode> xBody = xList->item( i );
            uno::Reference<xml::dom::XNode> xP = xBody->getFirstChild();
            uno::Reference<xml::dom::XNode> xR = xP->getFirstChild();
            uno::Reference<xml::dom::XNode> xT = xR->getFirstChild();
            uno::Reference<xml::dom::XNode> xText = xT->getFirstChild();
            OUString aText = xText->getNodeValue();
            if( !aText.isEmpty() )
            {
                rBlocks.PutText( aText, aText, aText );
                bRet = true;
            }
        }
    }

    return bRet;
}

uno::Reference<xml::dom::XDocument> SwDOCXReader::OpenDocument() const
{
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(
                comphelper::getProcessServiceFactory() );

    uno::Reference<uno::XInterface> xInterface(
                xMultiServiceFactory->createInstance( "com.sun.star.comp.Writer.WriterFilter" ),
                uno::UNO_QUERY_THROW );

    uno::Reference<document::XFilter> xFilter( xInterface, uno::UNO_QUERY_THROW );
    uno::Reference<document::XImporter> xImporter( xFilter, uno::UNO_QUERY_THROW );

    SfxObjectShellLock xDocSh( new SwDocShell( SfxObjectCreateMode::INTERNAL ) );
    xDocSh->DoInitNew();

    uno::Reference<lang::XComponent> xDstDoc( xDocSh->GetModel(), uno::UNO_QUERY_THROW );
    xImporter->setTargetDocument( xDstDoc );

    uno::Sequence<beans::PropertyValue> aDescriptor( 1 );
    aDescriptor[0].Name = "InputStream";
    uno::Reference<io::XStream> xStream( new utl::OStreamWrapper( *pMedium->GetInStream() ) );
    aDescriptor[0].Value <<= xStream;

    uno::Reference<xml::dom::XDocument> xDoc;

    try
    {
        xFilter->filter( aDescriptor );

        comphelper::SequenceAsHashMap aGrabBag = GetGrabBag( xDstDoc );
        aGrabBag["OOXGlossary"] >>= xDoc;
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("sw.docx", "SwDOCXReader::OpenDocument(): exception: " << e.Message);
    }

    return xDoc;
}

comphelper::SequenceAsHashMap SwDOCXReader::GetGrabBag( const uno::Reference<lang::XComponent>& xDocument )
{
    if( xDocument.is() )
    {
        // get glossar document from the GrabBag
        uno::Reference<beans::XPropertySet> xDocProps( xDocument, uno::UNO_QUERY );
        if( xDocProps.is() )
        {
            uno::Reference<beans::XPropertySetInfo> xPropsInfo = xDocProps->getPropertySetInfo();

            const OUString aGrabBagPropName = "InteropGrabBag";
            if( xPropsInfo.is() && xPropsInfo->hasPropertyByName( aGrabBagPropName ) )
            {
                // get existing grab bag
                comphelper::SequenceAsHashMap aGrabBag( xDocProps->getPropertyValue( aGrabBagPropName ) );
                return aGrabBag;
            }
        }
    }

    return comphelper::SequenceAsHashMap();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
