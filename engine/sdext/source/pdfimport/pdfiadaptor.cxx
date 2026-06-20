/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


#include "pdfiadaptor.hxx"
#include "filterdet.hxx"

#include <osl/file.h>
#include <sal/log.hxx>
#include <rtl/ref.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <memory>

using namespace com::sun::star;


namespace pdfi
{

PDFIHybridAdaptor::PDFIHybridAdaptor( const uno::Reference< uno::XComponentContext >& xContext ) :
    m_xContext( xContext )
{
}

// XFilter
bool SAL_CALL PDFIHybridAdaptor::filter( const uno::Sequence< beans::PropertyValue >& rFilterData )
{
    bool bRet = false;
    if( m_xModel.is() )
    {
        uno::Reference< io::XStream > xSubStream;
        OUString aPwd;
        sal_Int32 nPwPos = -1;
        for( sal_Int32 i = 0; i < rFilterData.getLength(); i++ )
        {
            SAL_INFO("sdext.pdfimport", "filter: Attrib: " << rFilterData[i].Name
                    << " = " << (rFilterData[i].Value.has<OUString>()
                            ? rFilterData[i].Value.get<OUString>()
                            : u"<no string>"_ustr)
                    << "\n");
            if (rFilterData[i].Name == "EmbeddedSubstream")
                rFilterData[i].Value >>= xSubStream;
            else if (rFilterData[i].Name == "Password")
            {
                nPwPos = i;
                rFilterData[i].Value >>= aPwd;
            }
        }
        bool bAddPwdProp = false;
        if( ! xSubStream.is() )
        {
            uno::Reference< io::XInputStream > xInput;
            auto pAttr = std::find_if(rFilterData.begin(), rFilterData.end(),
                [](const beans::PropertyValue& rAttr) { return rAttr.Name == "InputStream"; });
            if (pAttr != rFilterData.end())
                pAttr->Value >>= xInput;
            if( xInput.is() )
            {
                // TODO(P2): extracting hybrid substream twice - once during detection, second time here
                uno::Reference< io::XSeekable > xSeek( xInput, uno::UNO_QUERY );
                if( xSeek.is() )
                    xSeek->seek( 0 );
                oslFileHandle aFile = nullptr;
                sal_uInt64 nWritten = 0;
                OUString aURL;
                if( osl_createTempFile( nullptr, &aFile, &aURL.pData ) == osl_File_E_None )
                {
                    SAL_INFO("sdext.pdfimport", "created temp file " << aURL);
                    const sal_Int32 nBufSize = 4096;
                    uno::Sequence<sal_Int8> aBuf(nBufSize);
                    // copy the bytes
                    sal_Int32 nBytes;
                    do
                    {
                        nBytes = xInput->readBytes( aBuf, nBufSize );
                        if( nBytes > 0 )
                        {
                            osl_writeFile( aFile, aBuf.getConstArray(), nBytes, &nWritten );
                            if( static_cast<sal_Int32>(nWritten) != nBytes )
                            {
                                xInput.clear();
                                break;
                            }
                        }
                    } while( nBytes == nBufSize );
                    osl_closeFile( aFile );
                    if( xInput.is() )
                    {
                        OUString aEmbedMimetype;
                        OUString aOrgPwd( aPwd );

                        xSubStream = getEmbeddedFile(aURL, aEmbedMimetype, aPwd, m_xContext,
                                                     rFilterData, true);
                        if (aEmbedMimetype.isEmpty()) {
                            xSubStream = getAdditionalStream(aURL, aEmbedMimetype, aPwd, m_xContext,
                                                             rFilterData, true);
                        }
                        if( aOrgPwd != aPwd )
                            bAddPwdProp = true;
                    }
                    osl_removeFile( aURL.pData );
                }
                else
                    xSubStream.clear();
            }
        }
        if( xSubStream.is() )
        {
            uno::Sequence< cpo::uno::Any > aArgs{ cpo::uno::Any(m_xModel), cpo::uno::Any(xSubStream) };

            SAL_INFO("sdext.pdfimport", "try to instantiate subfilter" );
            uno::Reference< document::XFilter > xSubFilter;
            try {
                xSubFilter.set(
                    m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                        u"com.sun.star.document.OwnSubFilter"_ustr,
                        aArgs,
                        m_xContext ),
                    uno::UNO_QUERY );
            }
            catch(const uno::Exception&)
            {
                TOOLS_INFO_EXCEPTION("sdext.pdfimport", "subfilter");
            }

            SAL_INFO("sdext.pdfimport", "subfilter: " << xSubFilter.get() );
            if( xSubFilter.is() )
            {
                if( bAddPwdProp )
                {
                    uno::Sequence<beans::PropertyValue> aFilterData( rFilterData );
                    if( nPwPos == -1 )
                    {
                        nPwPos = aFilterData.getLength();
                        aFilterData.realloc( nPwPos+1 );
                        aFilterData.getArray()[nPwPos].Name = "Password";
                    }
                    aFilterData.getArray()[nPwPos].Value <<= aPwd;
                    bRet = xSubFilter->filter( aFilterData );
                }
                else
                    bRet = xSubFilter->filter( rFilterData );
            }
        }
        else
            SAL_INFO("sdext.pdfimport", "PDFIAdaptor::filter: no embedded substream set" );
    }
    else
        SAL_INFO("sdext.pdfimport", "PDFIAdaptor::filter: no model set" );

    return bRet;
}

void SAL_CALL PDFIHybridAdaptor::cancel()
{
}

//XImporter
void SAL_CALL PDFIHybridAdaptor::setTargetDocument( const uno::Reference< lang::XComponent >& xDocument )
{
    SAL_INFO("sdext.pdfimport", "PDFIAdaptor::setTargetDocument" );
    m_xModel.set( xDocument, uno::UNO_QUERY );
    if( xDocument.is() && ! m_xModel.is() )
        throw lang::IllegalArgumentException();
}

OUString PDFIHybridAdaptor::getImplementationName()
{
    return u"org.libreoffice.comp.documents.HybridPDFImport"_ustr;
}

bool PDFIHybridAdaptor::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> PDFIHybridAdaptor::getSupportedServiceNames()
{
    return {u"com.sun.star.document.ImportFilter"_ustr};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
sdext_PDFIHybridAdaptor_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new pdfi::PDFIHybridAdaptor( context ));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
