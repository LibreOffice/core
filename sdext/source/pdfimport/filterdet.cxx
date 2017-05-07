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


#include "filterdet.hxx"
#include "inc/pdfihelper.hxx"
#include "inc/pdfparse.hxx"

#include <osl/file.h>
#include <osl/thread.h>
#include <rtl/digest.h>
#include <rtl/ref.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <comphelper/fileurl.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <memory>
#include <string.h>

using namespace com::sun::star;

namespace pdfi
{

// TODO(T3): locking/thread safety

class FileEmitContext : public pdfparse::EmitContext
{
private:
    oslFileHandle                        m_aReadHandle;
    unsigned int                         m_nReadLen;
    uno::Reference< io::XStream >        m_xContextStream;
    uno::Reference< io::XSeekable >      m_xSeek;
    uno::Reference< io::XOutputStream >  m_xOut;

public:
    FileEmitContext( const OUString&                            rOrigFile,
                     const uno::Reference< uno::XComponentContext >& xContext,
                     const pdfparse::PDFContainer*                   pTop );
    virtual ~FileEmitContext() override;

    virtual bool         write( const void* pBuf, unsigned int nLen ) override;
    virtual unsigned int getCurPos() override;
    virtual bool         copyOrigBytes( unsigned int nOrigOffset, unsigned int nLen ) override;
    virtual unsigned int readOrigBytes( unsigned int nOrigOffset, unsigned int nLen, void* pBuf ) override;

    const uno::Reference< io::XStream >& getContextStream() const { return m_xContextStream; }
};

FileEmitContext::FileEmitContext( const OUString&                            rOrigFile,
                                  const uno::Reference< uno::XComponentContext >& xContext,
                                  const pdfparse::PDFContainer*                   pTop ) :
    pdfparse::EmitContext( pTop ),
    m_aReadHandle(nullptr),
    m_nReadLen(0),
    m_xContextStream(),
    m_xSeek(),
    m_xOut()
{
    m_xContextStream.set( io::TempFile::create(xContext), uno::UNO_QUERY_THROW );
    m_xOut = m_xContextStream->getOutputStream();
    m_xSeek.set(m_xOut, uno::UNO_QUERY_THROW );

    oslFileError aErr = osl_File_E_None;
    if( (aErr=osl_openFile( rOrigFile.pData,
                            &m_aReadHandle,
                            osl_File_OpenFlag_Read )) == osl_File_E_None )
    {
        if( (aErr=osl_setFilePos( m_aReadHandle,
                                  osl_Pos_End,
                                  0 )) == osl_File_E_None )
        {
            sal_uInt64 nFileSize = 0;
            if( (aErr=osl_getFilePos( m_aReadHandle,
                                      &nFileSize )) == osl_File_E_None )
            {
                m_nReadLen = static_cast<unsigned int>(nFileSize);
            }
        }
        if( aErr != osl_File_E_None )
        {
            osl_closeFile( m_aReadHandle );
            m_aReadHandle = nullptr;
        }
    }
    m_bDeflate = true;
}

FileEmitContext::~FileEmitContext()
{
    if( m_aReadHandle )
        osl_closeFile( m_aReadHandle );
}

bool FileEmitContext::write( const void* pBuf, unsigned int nLen )
{
    if( ! m_xOut.is() )
        return false;

    uno::Sequence< sal_Int8 > aSeq( nLen );
    memcpy( aSeq.getArray(), pBuf, nLen );
    m_xOut->writeBytes( aSeq );
    return true;
}

unsigned int FileEmitContext::getCurPos()
{
    unsigned int nPos = 0;
    if( m_xSeek.is() )
    {
        nPos = static_cast<unsigned int>( m_xSeek->getPosition() );
    }
    return nPos;
}

bool FileEmitContext::copyOrigBytes( unsigned int nOrigOffset, unsigned int nLen )
{
    if( nOrigOffset + nLen > m_nReadLen )
        return false;

    if( osl_setFilePos( m_aReadHandle, osl_Pos_Absolut, nOrigOffset ) != osl_File_E_None )
        return false;

    uno::Sequence< sal_Int8 > aSeq( nLen );

    sal_uInt64 nBytesRead = 0;
    if( osl_readFile( m_aReadHandle,
                      aSeq.getArray(),
                      nLen,
                      &nBytesRead ) != osl_File_E_None
        || nBytesRead != static_cast<sal_uInt64>(nLen) )
    {
        return false;
    }

    m_xOut->writeBytes( aSeq );
    return true;
}

unsigned int FileEmitContext::readOrigBytes( unsigned int nOrigOffset, unsigned int nLen, void* pBuf )
{
    if( nOrigOffset + nLen > m_nReadLen )
        return 0;

    if( osl_setFilePos( m_aReadHandle,
                        osl_Pos_Absolut,
                        nOrigOffset ) != osl_File_E_None )
    {
        return 0;
    }

    sal_uInt64 nBytesRead = 0;
    if( osl_readFile( m_aReadHandle,
                      pBuf,
                      nLen,
                      &nBytesRead ) != osl_File_E_None )
    {
        return 0;
    }
    return static_cast<unsigned int>(nBytesRead);
}


PDFDetector::PDFDetector( const uno::Reference< uno::XComponentContext >& xContext) :
    PDFDetectorBase( m_aMutex ),
    m_xContext( xContext )
{}

// XExtendedFilterDetection
OUString SAL_CALL PDFDetector::detect( uno::Sequence< beans::PropertyValue >& rFilterData )
{
    osl::MutexGuard const guard( m_aMutex );
    bool bSuccess = false;

    // get the InputStream carrying the PDF content
    uno::Reference< io::XInputStream > xInput;
    uno::Reference< io::XStream > xEmbedStream;
    OUString aOutFilterName, aOutTypeName;
    OUString aURL;
    OUString aPwd;
    const beans::PropertyValue* pAttribs = rFilterData.getConstArray();
    sal_Int32 nAttribs = rFilterData.getLength();
    sal_Int32 nFilterNamePos = -1;
    sal_Int32 nPwdPos = -1;
    for( sal_Int32 i = 0; i < nAttribs; i++ )
    {
        OUString aVal( "<no string>" );
        pAttribs[i].Value >>= aVal;
        SAL_INFO( "sdext.pdfimport", "doDetection: Attrib: " + pAttribs[i].Name + " = " + aVal);

        if ( pAttribs[i].Name == "InputStream" )
            pAttribs[i].Value >>= xInput;
        else if ( pAttribs[i].Name == "URL" )
            pAttribs[i].Value >>= aURL;
        else if ( pAttribs[i].Name == "FilterName" )
            nFilterNamePos = i;
        else if ( pAttribs[i].Name == "Password" )
        {
            nPwdPos = i;
            pAttribs[i].Value >>= aPwd;
        }
    }
    if( xInput.is() )
    {
        oslFileHandle aFile = nullptr;
        try {
            uno::Reference< io::XSeekable > xSeek( xInput, uno::UNO_QUERY );
            if( xSeek.is() )
                xSeek->seek( 0 );
            // read the first 1024 byte (see PDF reference implementation note 12)
            const sal_Int32 nHeaderSize = 1024;
            uno::Sequence< sal_Int8 > aBuf( nHeaderSize );
            sal_uInt64 nBytes = 0;
            nBytes = xInput->readBytes( aBuf, nHeaderSize );
            if( nBytes > 5 )
            {
                const sal_Int8* pBytes = aBuf.getConstArray();
                for( unsigned int i = 0; i < nBytes-5; i++ )
                {
                    if( pBytes[i]   == '%' &&
                        pBytes[i+1] == 'P' &&
                        pBytes[i+2] == 'D' &&
                        pBytes[i+3] == 'F' &&
                        pBytes[i+4] == '-' )
                    {
                        bSuccess = true;
                        break;
                    }
                }
            }

            // check for hybrid PDF
            if( bSuccess &&
                ( aURL.isEmpty() || !comphelper::isFileUrl(aURL) )
            )
            {
                sal_uInt64 nWritten = 0;
                if( osl_createTempFile( nullptr, &aFile, &aURL.pData ) != osl_File_E_None )
                {
                    bSuccess = false;
                }
                else
                {
                    SAL_INFO( "sdext.pdfimport", "created temp file " + aURL );

                    osl_writeFile( aFile, aBuf.getConstArray(), nBytes, &nWritten );

                    SAL_WARN_IF( nWritten != nBytes, "sdext.pdfimport", "writing of header bytes failed" );

                    if( nWritten == nBytes )
                    {
                        const sal_uInt32 nBufSize = 4096;
                        aBuf = uno::Sequence<sal_Int8>(nBufSize);
                        // copy the bytes
                        do
                        {
                            nBytes = xInput->readBytes( aBuf, nBufSize );
                            if( nBytes > 0 )
                            {
                                osl_writeFile( aFile, aBuf.getConstArray(), nBytes, &nWritten );
                                if( nWritten != nBytes )
                                {
                                    bSuccess = false;
                                    break;
                                }
                            }
                        } while( nBytes == nBufSize );
                    }
                }
                osl_closeFile( aFile );
            }
        } catch (css::io::IOException & e) {
            SAL_WARN("sdext.pdfimport", "caught IOException " + e.Message);
            return OUString();
        }
        OUString aEmbedMimetype;
        xEmbedStream = getAdditionalStream( aURL, aEmbedMimetype, aPwd, m_xContext, rFilterData, false );
        if( aFile )
            osl_removeFile( aURL.pData );
        if( !aEmbedMimetype.isEmpty() )
        {
            if( aEmbedMimetype == "application/vnd.oasis.opendocument.text"
                || aEmbedMimetype == "application/vnd.oasis.opendocument.text-master" )
                aOutFilterName = "writer_pdf_addstream_import";
            else if ( aEmbedMimetype == "application/vnd.oasis.opendocument.presentation" )
                aOutFilterName = "impress_pdf_addstream_import";
            else if( aEmbedMimetype == "application/vnd.oasis.opendocument.graphics"
                     || aEmbedMimetype == "application/vnd.oasis.opendocument.drawing" )
                aOutFilterName = "draw_pdf_addstream_import";
            else if ( aEmbedMimetype == "application/vnd.oasis.opendocument.spreadsheet" )
                aOutFilterName = "calc_pdf_addstream_import";
        }
    }

    if( bSuccess )
    {
        if( !aOutFilterName.isEmpty() )
        {
            if( nFilterNamePos == -1 )
            {
                nFilterNamePos = nAttribs;
                rFilterData.realloc( ++nAttribs );
                rFilterData[ nFilterNamePos ].Name = "FilterName";
            }
            aOutTypeName = "pdf_Portable_Document_Format";

            rFilterData[nFilterNamePos].Value <<= aOutFilterName;
            if( xEmbedStream.is() )
            {
                rFilterData.realloc( ++nAttribs );
                rFilterData[nAttribs-1].Name = "EmbeddedSubstream";
                rFilterData[nAttribs-1].Value <<= xEmbedStream;
            }
            if( !aPwd.isEmpty() )
            {
                if( nPwdPos == -1 )
                {
                    nPwdPos = nAttribs;
                    rFilterData.realloc( ++nAttribs );
                    rFilterData[ nPwdPos ].Name = "Password";
                }
                rFilterData[ nPwdPos ].Value <<= aPwd;
            }
        }
        else
        {
            if( nFilterNamePos == -1 )
            {
                nFilterNamePos = nAttribs;
                rFilterData.realloc( ++nAttribs );
                rFilterData[ nFilterNamePos ].Name = "FilterName";
            }

            const sal_Int32 nDocumentType = 0; //const sal_Int32 nDocumentType = queryDocumentTypeDialog(m_xContext,aURL);
            if( nDocumentType < 0 )
            {
                return OUString();
            }
            else switch( nDocumentType )
            {
                case 0:
                    rFilterData[nFilterNamePos].Value <<= OUString( "draw_pdf_import" );
                    break;

                case 1:
                    rFilterData[nFilterNamePos].Value <<= OUString( "impress_pdf_import" );
                    break;

                case 2:
                    rFilterData[nFilterNamePos].Value <<= OUString( "writer_pdf_import" );
                    break;

                default:
                    assert(!"Unexpected case");
            }

            aOutTypeName = "pdf_Portable_Document_Format";
        }
    }

    return aOutTypeName;
}

OUString PDFDetector::getImplementationName()
{
    return OUString("org.libreoffice.comp.documents.PDFDetector");
}

sal_Bool PDFDetector::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> PDFDetector::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{"com.sun.star.document.ImportFilter"};
}

bool checkDocChecksum( const OUString& rInPDFFileURL,
                       sal_uInt32           nBytes,
                       const OUString& rChkSum )
{
    if( rChkSum.getLength() != 2* RTL_DIGEST_LENGTH_MD5 )
    {
        SAL_INFO(
            "sdext.pdfimport",
            "checksum of length " << rChkSum.getLength() << ", expected "
                << 2*RTL_DIGEST_LENGTH_MD5);
        return false;
    }

    // prepare checksum to test
    sal_uInt8 nTestChecksum[ RTL_DIGEST_LENGTH_MD5 ];
    const sal_Unicode* pChar = rChkSum.getStr();
    for(sal_uInt8 & rn : nTestChecksum)
    {
        sal_uInt8 nByte = sal_uInt8( ( (*pChar >= '0' && *pChar <= '9') ? *pChar - '0' :
                          ( (*pChar >= 'A' && *pChar <= 'F') ? *pChar - 'A' + 10 :
                          ( (*pChar >= 'a' && *pChar <= 'f') ? *pChar - 'a' + 10 :
                          0 ) ) ) );
        nByte <<= 4;
        pChar++;
        nByte |= ( (*pChar >= '0' && *pChar <= '9') ? *pChar - '0' :
                 ( (*pChar >= 'A' && *pChar <= 'F') ? *pChar - 'A' + 10 :
                 ( (*pChar >= 'a' && *pChar <= 'f') ? *pChar - 'a' + 10 :
                 0 ) ) );
        pChar++;
        rn = nByte;
    }

    // open file and calculate actual checksum up to index nBytes
    sal_uInt8 nActualChecksum[ RTL_DIGEST_LENGTH_MD5 ];
    memset( nActualChecksum, 0, sizeof(nActualChecksum) );
    rtlDigest aActualDigest = rtl_digest_createMD5();
    oslFileHandle aRead = nullptr;
    oslFileError aErr = osl_File_E_None;
    if( (aErr = osl_openFile(rInPDFFileURL.pData,
                             &aRead,
                             osl_File_OpenFlag_Read )) == osl_File_E_None )
    {
        sal_Int8 aBuf[4096];
        sal_uInt32 nCur = 0;
        sal_uInt64 nBytesRead = 0;
        while( nCur < nBytes )
        {
            sal_uInt32 nPass = (nBytes - nCur) > sizeof( aBuf ) ? sizeof( aBuf ) : nBytes - nCur;
            if( (aErr = osl_readFile( aRead, aBuf, nPass, &nBytesRead)) != osl_File_E_None
                || nBytesRead == 0 )
            {
                break;
            }
            nPass = static_cast<sal_uInt32>(nBytesRead);
            nCur += nPass;
            rtl_digest_updateMD5( aActualDigest, aBuf, nPass );
        }
        rtl_digest_getMD5( aActualDigest, nActualChecksum, sizeof(nActualChecksum) );
        osl_closeFile( aRead );
    }
    rtl_digest_destroyMD5( aActualDigest );

    // compare the contents
    return (0 == memcmp( nActualChecksum, nTestChecksum, sizeof( nActualChecksum ) ));
}

uno::Reference< io::XStream > getAdditionalStream( const OUString&                          rInPDFFileURL,
                                                   OUString&                                rOutMimetype,
                                                   OUString&                                io_rPwd,
                                                   const uno::Reference<uno::XComponentContext>& xContext,
                                                   const uno::Sequence<beans::PropertyValue>&    rFilterData,
                                                   bool                                          bMayUseUI )
{
    uno::Reference< io::XStream > xEmbed;
    OString aPDFFile;
    OUString aSysUPath;
    if( osl_getSystemPathFromFileURL( rInPDFFileURL.pData, &aSysUPath.pData ) != osl_File_E_None )
        return xEmbed;
    aPDFFile = OUStringToOString( aSysUPath, osl_getThreadTextEncoding() );

    pdfparse::PDFReader aParser;
    std::unique_ptr<pdfparse::PDFEntry> pEntry( pdfparse::PDFReader::read( aPDFFile.getStr() ));
    if( pEntry )
    {
        pdfparse::PDFFile* pPDFFile = dynamic_cast<pdfparse::PDFFile*>(pEntry.get());
        if( pPDFFile )
        {
            unsigned int nElements = pPDFFile->m_aSubElements.size();
            while( nElements-- > 0 )
            {
                pdfparse::PDFTrailer* pTrailer = dynamic_cast<pdfparse::PDFTrailer*>(pPDFFile->m_aSubElements[nElements]);
                if( pTrailer && pTrailer->m_pDict )
                {
                    // search document checksum entry
                    std::unordered_map< OString,
                                   pdfparse::PDFEntry*,
                                   OStringHash >::iterator chk;
                    chk = pTrailer->m_pDict->m_aMap.find( "DocChecksum" );
                    if( chk == pTrailer->m_pDict->m_aMap.end() )
                    {
                        SAL_INFO( "sdext.pdfimport", "no DocChecksum entry" );
                        continue;
                    }
                    pdfparse::PDFName* pChkSumName = dynamic_cast<pdfparse::PDFName*>(chk->second);
                    if( pChkSumName == nullptr )
                    {
                        SAL_INFO( "sdext.pdfimport", "no name for DocChecksum entry" );
                        continue;
                    }

                    // search for AdditionalStreams entry
                    std::unordered_map< OString,
                                   pdfparse::PDFEntry*,
                                   OStringHash >::iterator add_stream;
                    add_stream = pTrailer->m_pDict->m_aMap.find( "AdditionalStreams" );
                    if( add_stream == pTrailer->m_pDict->m_aMap.end() )
                    {
                        SAL_INFO( "sdext.pdfimport", "no AdditionalStreams entry" );
                        continue;
                    }
                    pdfparse::PDFArray* pStreams = dynamic_cast<pdfparse::PDFArray*>(add_stream->second);
                    if( ! pStreams || pStreams->m_aSubElements.size() < 2 )
                    {
                        SAL_INFO( "sdext.pdfimport", "AdditionalStreams array too small" );
                        continue;
                    }

                    // check checksum
                    OUString aChkSum = pChkSumName->getFilteredName();
                    if( ! checkDocChecksum( rInPDFFileURL, pTrailer->m_nOffset, aChkSum ) )
                        continue;

                    // extract addstream and mimetype
                    pdfparse::PDFName* pMimeType = dynamic_cast<pdfparse::PDFName*>(pStreams->m_aSubElements[0]);
                    pdfparse::PDFObjectRef* pStreamRef = dynamic_cast<pdfparse::PDFObjectRef*>(pStreams->m_aSubElements[1]);

                    SAL_WARN_IF( !pMimeType, "sdext.pdfimport", "error: no mimetype element" );
                    SAL_WARN_IF( !pStreamRef, "sdext.pdfimport", "error: no stream ref element" );

                    if( pMimeType && pStreamRef )
                    {
                        pdfparse::PDFObject* pObject = pPDFFile->findObject( pStreamRef->m_nNumber, pStreamRef->m_nGeneration );
                        SAL_WARN_IF( !pObject, "sdext.pdfimport", "object not found" );
                        if( pObject )
                        {
                            if( pPDFFile->isEncrypted() )
                            {
                                bool bAuthenticated = false;
                                if( !io_rPwd.isEmpty() )
                                {
                                    OString aIsoPwd = OUStringToOString( io_rPwd,
                                                                                   RTL_TEXTENCODING_ISO_8859_1 );
                                    bAuthenticated = pPDFFile->setupDecryptionData( aIsoPwd.getStr() );
                                }
                                if( ! bAuthenticated )
                                {
                                    const beans::PropertyValue* pAttribs = rFilterData.getConstArray();
                                    sal_Int32 nAttribs = rFilterData.getLength();
                                    uno::Reference< task::XInteractionHandler > xIntHdl;
                                    for( sal_Int32 i = 0; i < nAttribs; i++ )
                                    {
                                        if ( pAttribs[i].Name == "InteractionHandler" )
                                            pAttribs[i].Value >>= xIntHdl;
                                    }
                                    if( ! bMayUseUI || ! xIntHdl.is() )
                                    {
                                        rOutMimetype = pMimeType->getFilteredName();
                                        xEmbed.clear();
                                        break;
                                    }

                                    OUString aDocName( rInPDFFileURL.copy( rInPDFFileURL.lastIndexOf( '/' )+1 ) );

                                    bool bEntered = false;
                                    do
                                    {
                                        bEntered = getPassword( xIntHdl, io_rPwd, ! bEntered, aDocName );
                                        OString aIsoPwd = OUStringToOString( io_rPwd,
                                                                                       RTL_TEXTENCODING_ISO_8859_1 );
                                        bAuthenticated = pPDFFile->setupDecryptionData( aIsoPwd.getStr() );
                                    } while( bEntered && ! bAuthenticated );
                                }

                                if( ! bAuthenticated )
                                    continue;
                            }
                            rOutMimetype = pMimeType->getFilteredName();
                            FileEmitContext aContext( rInPDFFileURL,
                                                      xContext,
                                                      pPDFFile );
                            aContext.m_bDecrypt = pPDFFile->isEncrypted();
                            pObject->writeStream( aContext, pPDFFile );
                            xEmbed = aContext.getContextStream();
                            break; // success
                        }
                    }
                }
            }
        }
    }

    return xEmbed;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
