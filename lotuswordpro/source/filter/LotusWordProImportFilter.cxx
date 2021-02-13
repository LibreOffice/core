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

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <sal/macros.h>
#include <tools/stream.hxx>
#include <ucbhelper/content.hxx>

#include "LotusWordProImportFilter.hxx"
#include "lwpfilter.hxx"

#if OSL_DEBUG_LEVEL > 0
#include <memory>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

#include <comphelper/oslfile2streamwrap.hxx>

#include <cppuhelper/weak.hxx>

#include <osl/file.hxx>
#endif

using namespace com::sun::star;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Exception;
using com::sun::star::io::XInputStream;
using com::sun::star::beans::PropertyValue;
using com::sun::star::ucb::XCommandEnvironment;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::XDocumentHandler;

#if OSL_DEBUG_LEVEL > 0
namespace
{

class DebugDocumentHandler final : public cppu::WeakImplHelper< XDocumentHandler >
{
public:
    DebugDocumentHandler(const uno::Reference< XDocumentHandler >& handler, const uno::Reference< XDocumentHandler >& debug)
        : m_handler(handler)
        , m_debug(debug)
    {
    }

    // XDocumentHandler

    virtual void SAL_CALL
    startDocument() override
    {
        m_handler->startDocument();
        m_debug->startDocument();
    }

    virtual void SAL_CALL
    endDocument() override
    {
        m_handler->endDocument();
        m_debug->endDocument();
    }

    virtual void SAL_CALL
    startElement(
            const OUString& aName,
            const uno::Reference< xml::sax::XAttributeList > & xAttribs) override
    {
        m_handler->startElement(aName, xAttribs);
        m_debug->ignorableWhitespace(" "); // NOTE: needed for pretty-printing
        m_debug->startElement(aName, xAttribs);
    }

    virtual void SAL_CALL
    endElement(const OUString& aName) override
    {
        m_handler->endElement(aName);
        m_debug->ignorableWhitespace(" "); // NOTE: needed for pretty-printing
        m_debug->endElement(aName);
    }

    virtual void SAL_CALL
    characters(const OUString& aChars) override
    {
        m_handler->characters(aChars);
        m_debug->characters(aChars);
    }

    virtual void SAL_CALL
    ignorableWhitespace(const OUString& aWhitespaces) override
    {
        m_handler->ignorableWhitespace(aWhitespaces);
        m_debug->ignorableWhitespace(aWhitespaces);
    }

    virtual void SAL_CALL
    processingInstruction(const OUString& aTarget, const OUString& aData) override
    {
        m_handler->processingInstruction(aTarget, aData);
        m_debug->processingInstruction(aTarget, aData);
    }

    virtual void SAL_CALL
    setDocumentLocator(const uno::Reference< xml::sax::XLocator >& xLocator) override
    {
        m_handler->setDocumentLocator(xLocator);
        m_debug->setDocumentLocator(xLocator);
    }

    // XInterface

    virtual uno::Any SAL_CALL queryInterface(const uno::Type & rType) override
    {
        uno::Any aIface = cppu::WeakImplHelper< XDocumentHandler >::queryInterface(rType);
        // delegate all queries we cannot satisfy ourselves to the real handler
        if (!aIface.has< uno::Reference< uno::XInterface > >())
            aIface = m_handler->queryInterface(rType);
        return aIface;
    }

private:
    uno::Reference< XDocumentHandler > m_handler;
    uno::Reference< XDocumentHandler > m_debug;
};

}
#endif

//                                 W     o     r     d     P     r     o
const sal_Int8 header[] = { 0x57, 0x6f, 0x72, 0x64, 0x50, 0x72, 0x6f };

bool LotusWordProImportFilter::importImpl( const Sequence< css::beans::PropertyValue >& aDescriptor )
{
    OUString sURL;
    for (const PropertyValue& rValue : aDescriptor)
    {
        //Note, we should attempt to use InputStream here first!
        if ( rValue.Name == "URL" )
            rValue.Value >>= sURL;
    }

    SvFileStream inputStream( sURL, StreamMode::READ );
    if (!inputStream.good())
         return false;

    // An XML import service: what we push sax messages to...
    uno::Reference< XDocumentHandler > xInternalHandler(
        mxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.comp.Writer.XMLImporter", mxContext ), UNO_QUERY );

#if OSL_DEBUG_LEVEL > 0
    std::unique_ptr<osl::File> pDebugFile;
    const char* pDir = getenv("DBG_LWPIMPORT_DIR");
    if (pDir)
    {
        OUString aStr(OStringToOUString(pDir, RTL_TEXTENCODING_UTF8));
        OUString aFileURL;
        osl_getFileURLFromSystemPath(aStr.pData, &aFileURL.pData);
        pDebugFile = std::make_unique<osl::File>(aFileURL + "/lwpimport.xml");
        if (pDebugFile->open(osl_File_OpenFlag_Write | osl_File_OpenFlag_Create) != osl::File::E_None)
        {
            pDebugFile->open(osl_File_OpenFlag_Write);
            pDebugFile->setSize(0);
        }

        uno::Reference< xml::sax::XWriter > xDebugWriter = xml::sax::Writer::create(mxContext);
        uno::Reference< io::XOutputStream > xOutputStream(new comphelper::OSLOutputStreamWrapper(*pDebugFile));
        xDebugWriter->setOutputStream(xOutputStream);

        xInternalHandler.set(new DebugDocumentHandler(xInternalHandler, xDebugWriter));
    }
#endif

    uno::Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    if (xImporter.is())
        xImporter->setTargetDocument(mxDoc);

    return ( ReadWordproFile( inputStream, xInternalHandler) == 0 );

}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportLWP(SvStream &rStream)
{
    uno::Reference< XDocumentHandler > xHandler;
    return ReadWordproFile(rStream, xHandler) == 0;
}

sal_Bool SAL_CALL LotusWordProImportFilter::filter( const Sequence< css::beans::PropertyValue >& aDescriptor )
{
    return importImpl ( aDescriptor );
}
void SAL_CALL LotusWordProImportFilter::cancel(  )
{
}

// XImporter
void SAL_CALL LotusWordProImportFilter::setTargetDocument( const uno::Reference< css::lang::XComponent >& xDoc )
{
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL LotusWordProImportFilter::detect( css::uno::Sequence< PropertyValue >& Descriptor )
{
    OUString sTypeName( "writer_LotusWordPro_Document" );
    OUString sURL;
    uno::Reference < XInputStream > xInputStream;
    for (const PropertyValue& rValue : std::as_const(Descriptor))
    {
        if ( rValue.Name == "TypeName" )
            rValue.Value >>= sTypeName;
        else if ( rValue.Name == "InputStream" )
            rValue.Value >>= xInputStream;
        else if ( rValue.Name == "URL" )
            rValue.Value >>= sURL;
    }

    uno::Reference< css::ucb::XCommandEnvironment > xEnv;
    if (!xInputStream.is())
    {
        try
        {
            ::ucbhelper::Content aContent(sURL, xEnv, mxContext);
            xInputStream = aContent.openStream();
        }
        catch ( Exception& )
        {
            return OUString();
        }

        if (!xInputStream.is())
            return OUString();
    }

    Sequence< ::sal_Int8 > aData;
    sal_Int32 nLen = SAL_N_ELEMENTS( header );
    if ( ( nLen != xInputStream->readBytes( aData, nLen ) )
            || ( memcmp( static_cast<void const *>(header), static_cast<void const *>(aData.getConstArray()), nLen ) != 0 ) )
        sTypeName.clear();

    return sTypeName;
}

// XInitialization
void SAL_CALL LotusWordProImportFilter::initialize( const Sequence< Any >& /*aArguments*/ )
{
}

// XServiceInfo
OUString SAL_CALL LotusWordProImportFilter::getImplementationName()
{
    return "com.sun.star.comp.Writer.LotusWordProImportFilter";
}

sal_Bool SAL_CALL LotusWordProImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SAL_CALL LotusWordProImportFilter::getSupportedServiceNames()
{
    return { "com.sun.star.document.ImportFilter", "com.sun.star.document.ExtendedTypeDetection" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
LotusWordProImportFilter_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new LotusWordProImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
