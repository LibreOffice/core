/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "filterdet.hxx"
#include "inc/pdfparse.hxx"

#include <osl/diagnose.h>
#include <osl/file.h>
#include <osl/thread.h>
#include <rtl/digest.h>
#include <rtl/ref.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XDialogEventHandler.hpp>
#include <com/sun/star/awt/XDialogProvider2.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/TempFile.hpp>

#include <boost/scoped_ptr.hpp>

using namespace com::sun::star;

namespace pdfi
{

// TODO(T3): locking/thread safety

namespace {
    typedef ::cppu::WeakComponentImplHelper1<
        com::sun::star::awt::XDialogEventHandler > ChooserDialogHandlerBase;
    class ChooserDialogHandler : private cppu::BaseMutex,
                                 public ChooserDialogHandlerBase
    {
        uno::Reference<awt::XListBox> m_xListbox;
        uno::Reference<awt::XWindow>  m_xWriterText;
        uno::Reference<awt::XWindow>  m_xDrawText;
        uno::Reference<awt::XWindow>  m_xImpressText;

        enum{ DRAW_INDEX=0, IMPRESS_INDEX=1, WRITER_INDEX=2 };
        void selectionChanged( sal_Int32 nIndex ) const
        {
            sal_Bool bWriterState(sal_False);
            sal_Bool bDrawState(sal_False);
            sal_Bool bImpressState(sal_False);
            switch(nIndex)
            {
            default:
                OSL_FAIL("Unexpected case!");
                break;
            case DRAW_INDEX:
                bDrawState=sal_True;
                break;
            case IMPRESS_INDEX:
                bImpressState=sal_True;
                break;
            case WRITER_INDEX:
                bWriterState=sal_True;
                break;
            }
            m_xWriterText->setVisible(bWriterState);
            m_xDrawText->setVisible(bDrawState);
            m_xImpressText->setVisible(bImpressState);
        }
    public:
        ChooserDialogHandler() :
            ChooserDialogHandlerBase(m_aMutex),
            m_xListbox(),
            m_xWriterText(),
            m_xDrawText(),
            m_xImpressText()
        {}

        void initControls( const uno::Reference<awt::XControlContainer>& xControls,
                           const rtl::OUString&                          rFilename )
        {
            m_xListbox.set(xControls->getControl(
                           rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ListBox" ))),
                           uno::UNO_QUERY_THROW );
            m_xWriterText.set(xControls->getControl(
                               rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InfoWriter" ))),
                               uno::UNO_QUERY_THROW );
            m_xImpressText.set(xControls->getControl(
                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InfoImpress" ))),
                                uno::UNO_QUERY_THROW );
            m_xDrawText.set(xControls->getControl(
                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InfoDraw" ))),
                                uno::UNO_QUERY_THROW );

            uno::Reference<awt::XWindow> xControl;
            xControl.set(xControls->getControl(
                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ListBoxWriter" ))),
                                uno::UNO_QUERY_THROW );
            xControl->setVisible(sal_False);
            xControl.set(xControls->getControl(
                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ListBoxImpress" ))),
                                uno::UNO_QUERY_THROW );
            xControl->setVisible(sal_False);
            xControl.set(xControls->getControl(
                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ListBoxDraw" ))),
                                uno::UNO_QUERY_THROW );
            xControl->setVisible(sal_False);
            uno::Reference<beans::XPropertySet> xPropSet(
                xControls->getControl(
                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ComboLabel" )))->getModel(),
                                uno::UNO_QUERY_THROW );
            rtl::OUString aFilename( rFilename.copy(rFilename.lastIndexOf('/')+1) );
            rtl::OUString aLabel;
            xPropSet->getPropertyValue(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Label" ))) >>= aLabel;
            const char pFileName[] = "%FILENAME";
            aLabel = aLabel.replaceAt(
                aLabel.indexOfAsciiL(pFileName,SAL_N_ELEMENTS(pFileName)-1),
                SAL_N_ELEMENTS(pFileName)-1,
                aFilename );
            xPropSet->setPropertyValue(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Label" )),
                                       uno::makeAny(aLabel));

            uno::Sequence<rtl::OUString> aListboxItems(3);
            aListboxItems[DRAW_INDEX]    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Drawing" ));
            aListboxItems[IMPRESS_INDEX] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Presentation" ));
            aListboxItems[WRITER_INDEX]  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Text Document" ));

            m_xListbox->addItems(aListboxItems,0);
            m_xListbox->selectItemPos(0,sal_True);
            selectionChanged(0);
        }

        sal_Int32 getSelectedItem() const
        {
            return m_xListbox->getSelectedItemPos();
        }

        virtual ::sal_Bool SAL_CALL callHandlerMethod( const uno::Reference< awt::XDialog >& /*xDialog*/,
                                                       const uno::Any& /*EventObject*/,
                                                       const ::rtl::OUString& MethodName ) throw (lang::WrappedTargetException, uno::RuntimeException)
        {
            (void)MethodName;
            OSL_ENSURE( MethodName.compareToAscii("SelectionChanged") == 0,
                        "Invalid event name" );
            selectionChanged(getSelectedItem());
            return sal_True;
        }

        virtual uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedMethodNames(  ) throw (uno::RuntimeException)
        {
            uno::Sequence< ::rtl::OUString > aMethods(1);
            aMethods[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SelectionChanged" ));
            return aMethods;
        }
    };
}

class FileEmitContext : public pdfparse::EmitContext
{
private:
    oslFileHandle                        m_aReadHandle;
    unsigned int                         m_nReadLen;
    uno::Reference< io::XStream >        m_xContextStream;
    uno::Reference< io::XSeekable >      m_xSeek;
    uno::Reference< io::XOutputStream >  m_xOut;

public:
    FileEmitContext( const rtl::OUString&                            rOrigFile,
                     const uno::Reference< uno::XComponentContext >& xContext,
                     const pdfparse::PDFContainer*                   pTop );
    virtual ~FileEmitContext();

    virtual bool         write( const void* pBuf, unsigned int nLen );
    virtual unsigned int getCurPos();
    virtual bool         copyOrigBytes( unsigned int nOrigOffset, unsigned int nLen );
    virtual unsigned int readOrigBytes( unsigned int nOrigOffset, unsigned int nLen, void* pBuf );

    const uno::Reference< io::XStream >& getContextStream() const { return m_xContextStream; }
};

FileEmitContext::FileEmitContext( const rtl::OUString&                            rOrigFile,
                                  const uno::Reference< uno::XComponentContext >& xContext,
                                  const pdfparse::PDFContainer*                   pTop ) :
    pdfparse::EmitContext( pTop ),
    m_aReadHandle(NULL),
    m_nReadLen(0),
    m_xContextStream(),
    m_xSeek(),
    m_xOut()
{
    m_xContextStream = uno::Reference< io::XStream >(
        io::TempFile::create(xContext), uno::UNO_QUERY_THROW );
    m_xOut = m_xContextStream->getOutputStream();
    m_xSeek = uno::Reference<io::XSeekable>(m_xOut, uno::UNO_QUERY_THROW );

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
            m_aReadHandle = NULL;
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
    rtl_copyMemory( aSeq.getArray(), pBuf, nLen );
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


////////////////////////////////////////////////////////////////////////////////


PDFDetector::PDFDetector( const uno::Reference< uno::XComponentContext >& xContext) :
    PDFDetectorBase( m_aMutex ),
    m_xContext( xContext )
{}

// XExtendedFilterDetection
rtl::OUString SAL_CALL PDFDetector::detect( uno::Sequence< beans::PropertyValue >& rFilterData ) throw( uno::RuntimeException )
{
    osl::MutexGuard const guard( m_aMutex );
    bool bSuccess = false;

    // get the InputStream carrying the PDF content
    uno::Reference< io::XInputStream > xInput;
    uno::Reference< io::XStream > xEmbedStream;
    rtl::OUString aOutFilterName, aOutTypeName;
    rtl::OUString aURL;
    rtl::OUString aPwd;
    const beans::PropertyValue* pAttribs = rFilterData.getConstArray();
    sal_Int32 nAttribs = rFilterData.getLength();
    sal_Int32 nFilterNamePos = -1;
    sal_Int32 nPwdPos = -1;
    for( sal_Int32 i = 0; i < nAttribs; i++ )
    {
#if OSL_DEBUG_LEVEL > 1
        rtl::OUString aVal( RTL_CONSTASCII_USTRINGPARAM( "<no string>" ) );
        pAttribs[i].Value >>= aVal;
        OSL_TRACE( "doDetection: Attrib: %s = %s\n",
                   rtl::OUStringToOString( pAttribs[i].Name, RTL_TEXTENCODING_UTF8 ).getStr(),
                   rtl::OUStringToOString( aVal, RTL_TEXTENCODING_UTF8 ).getStr() );
#endif
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
        oslFileHandle aFile = NULL;
        if( bSuccess &&
            ( aURL.isEmpty() || aURL.compareToAscii( "file:", 5 ) != 0 )
        )
        {
            sal_uInt64 nWritten = 0;
            if( osl_createTempFile( NULL, &aFile, &aURL.pData ) != osl_File_E_None )
            {
                bSuccess = false;
            }
            else
            {
#if OSL_DEBUG_LEVEL > 1
                OSL_TRACE( "created temp file %s\n",
                           rtl::OUStringToOString( aURL, RTL_TEXTENCODING_UTF8 ).getStr() );
#endif
                osl_writeFile( aFile, aBuf.getConstArray(), nBytes, &nWritten );

                OSL_ENSURE( nWritten == nBytes, "writing of header bytes failed" );

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
        rtl::OUString aEmbedMimetype;
        xEmbedStream = getAdditionalStream( aURL, aEmbedMimetype, aPwd, m_xContext, rFilterData, false );
        if( aFile )
            osl_removeFile( aURL.pData );
        if( !aEmbedMimetype.isEmpty() )
        {
            if( aEmbedMimetype.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "application/vnd.oasis.opendocument.text" ) )
                || aEmbedMimetype.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "application/vnd.oasis.opendocument.text-master" ) ) )
                aOutFilterName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "writer_pdf_addstream_import" ) );
            else if ( aEmbedMimetype == "application/vnd.oasis.opendocument.presentation" )
                aOutFilterName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "impress_pdf_addstream_import" ) );
            else if( aEmbedMimetype.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "application/vnd.oasis.opendocument.graphics" ) )
                     || aEmbedMimetype.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "application/vnd.oasis.opendocument.drawing" ) ) )
                aOutFilterName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "draw_pdf_addstream_import" ) );
            else if ( aEmbedMimetype == "application/vnd.oasis.opendocument.spreadsheet" )
                aOutFilterName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "calc_pdf_addstream_import" ) );
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
                rFilterData[ nFilterNamePos ].Name =
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ) );
            }
            aOutTypeName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("pdf_Portable_Document_Format") );

            OSL_TRACE( "setting filter name %s, input stream %s\n",
                       rtl::OUStringToOString( aOutFilterName, RTL_TEXTENCODING_UTF8 ).getStr(),
                       xEmbedStream.is() ? "present" : "not present" );

            rFilterData[nFilterNamePos].Value <<= aOutFilterName;
            if( xEmbedStream.is() )
            {
                rFilterData.realloc( ++nAttribs );
                rFilterData[nAttribs-1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "EmbeddedSubstream" ) );
                rFilterData[nAttribs-1].Value <<= xEmbedStream;
            }
            if( !aPwd.isEmpty() )
            {
                if( nPwdPos == -1 )
                {
                    nPwdPos = nAttribs;
                    rFilterData.realloc( ++nAttribs );
                    rFilterData[ nPwdPos ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Password" ) );
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
                rFilterData[ nFilterNamePos ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ) );
            }

            const sal_Int32 nDocumentType = 0; //const sal_Int32 nDocumentType = queryDocumentTypeDialog(m_xContext,aURL);
            if( nDocumentType < 0 )
            {
                return rtl::OUString();
            }
            else switch( nDocumentType )
            {
                case 0:
                    rFilterData[nFilterNamePos].Value <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "draw_pdf_import" ) );
                    break;

                case 1:
                    rFilterData[nFilterNamePos].Value <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "impress_pdf_import" ) );
                    break;

                case 2:
                    rFilterData[nFilterNamePos].Value <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "writer_pdf_import" ) );
                    break;

                default:
                    OSL_FAIL("Unexpected case");
            }

            aOutTypeName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("pdf_Portable_Document_Format") );
        }
    }

    return aOutTypeName;
}

bool checkDocChecksum( const rtl::OUString& rInPDFFileURL,
                       sal_uInt32           nBytes,
                       const rtl::OUString& rChkSum )
{
    bool bRet = false;
    if( rChkSum.getLength() != 2* RTL_DIGEST_LENGTH_MD5 )
    {
        OSL_TRACE( "checksum of length %d, expected %d\n",
                   rChkSum.getLength(), 2*RTL_DIGEST_LENGTH_MD5 );
        return false;
    }

    // prepare checksum to test
    sal_uInt8 nTestChecksum[ RTL_DIGEST_LENGTH_MD5 ];
    const sal_Unicode* pChar = rChkSum.getStr();
    for( unsigned int i = 0; i < RTL_DIGEST_LENGTH_MD5; i++ )
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
        nTestChecksum[i] = nByte;
    }

    // open file and calculate actual checksum up to index nBytes
    sal_uInt8 nActualChecksum[ RTL_DIGEST_LENGTH_MD5 ];
    rtl_zeroMemory( nActualChecksum, sizeof(nActualChecksum) );
    rtlDigest aActualDigest = rtl_digest_createMD5();
    oslFileHandle aRead = NULL;
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
    bRet = (0 == rtl_compareMemory( nActualChecksum, nTestChecksum, sizeof( nActualChecksum ) ));
#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "test checksum: " );
    for( unsigned int i = 0; i < sizeof(nTestChecksum); i++ )
        OSL_TRACE( "%.2X", int(nTestChecksum[i]) );
    OSL_TRACE( "\n" );
    OSL_TRACE( "file checksum: " );
    for( unsigned int i = 0; i < sizeof(nActualChecksum); i++ )
        OSL_TRACE( "%.2X", int(nActualChecksum[i]) );
    OSL_TRACE( "\n" );
#endif
    return bRet;
}

uno::Reference< io::XStream > getAdditionalStream( const rtl::OUString&                          rInPDFFileURL,
                                                   rtl::OUString&                                rOutMimetype,
                                                   rtl::OUString&                                io_rPwd,
                                                   const uno::Reference<uno::XComponentContext>& xContext,
                                                   const uno::Sequence<beans::PropertyValue>&    rFilterData,
                                                   bool                                          bMayUseUI )
{
    uno::Reference< io::XStream > xEmbed;
    rtl::OString aPDFFile;
    rtl::OUString aSysUPath;
    if( osl_getSystemPathFromFileURL( rInPDFFileURL.pData, &aSysUPath.pData ) != osl_File_E_None )
        return xEmbed;
    aPDFFile = rtl::OUStringToOString( aSysUPath, osl_getThreadTextEncoding() );

    pdfparse::PDFReader aParser;
    boost::scoped_ptr<pdfparse::PDFEntry> pEntry( aParser.read( aPDFFile.getStr() ));
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
                    boost::unordered_map< rtl::OString,
                                   pdfparse::PDFEntry*,
                                   rtl::OStringHash >::iterator chk;
                    chk = pTrailer->m_pDict->m_aMap.find( "DocChecksum" );
                    if( chk == pTrailer->m_pDict->m_aMap.end() )
                    {
                        OSL_TRACE( "no DocChecksum entry" );
                        continue;
                    }
                    pdfparse::PDFName* pChkSumName = dynamic_cast<pdfparse::PDFName*>(chk->second);
                    if( pChkSumName == NULL )
                    {
                        OSL_TRACE( "no name for DocChecksum entry" );
                        continue;
                    }

                    // search for AdditionalStreams entry
                    boost::unordered_map< rtl::OString,
                                   pdfparse::PDFEntry*,
                                   rtl::OStringHash >::iterator add_stream;
                    add_stream = pTrailer->m_pDict->m_aMap.find( "AdditionalStreams" );
                    if( add_stream == pTrailer->m_pDict->m_aMap.end() )
                    {
                        OSL_TRACE( "no AdditionalStreams entry" );
                        continue;
                    }
                    pdfparse::PDFArray* pStreams = dynamic_cast<pdfparse::PDFArray*>(add_stream->second);
                    if( ! pStreams || pStreams->m_aSubElements.size() < 2 )
                    {
                        OSL_TRACE( "AdditionalStreams array too small" );
                        continue;
                    }

                    // check checksum
                    rtl::OUString aChkSum = pChkSumName->getFilteredName();
                    if( ! checkDocChecksum( rInPDFFileURL, pTrailer->m_nOffset, aChkSum ) )
                        continue;

                    // extract addstream and mimetype
                    pdfparse::PDFName* pMimeType = dynamic_cast<pdfparse::PDFName*>(pStreams->m_aSubElements[0]);
                    pdfparse::PDFObjectRef* pStreamRef = dynamic_cast<pdfparse::PDFObjectRef*>(pStreams->m_aSubElements[1]);

                    OSL_ENSURE( pMimeType, "error: no mimetype element\n" );
                    OSL_ENSURE( pStreamRef, "error: no stream ref element\n" );

                    if( pMimeType && pStreamRef )
                    {
                        pdfparse::PDFObject* pObject = pPDFFile->findObject( pStreamRef->m_nNumber, pStreamRef->m_nGeneration );
                        OSL_ENSURE( pObject, "object not found\n" );
                        if( pObject )
                        {
                            if( pPDFFile->isEncrypted() )
                            {
                                bool bAuthenticated = false;
                                if( !io_rPwd.isEmpty() )
                                {
                                    rtl::OString aIsoPwd = rtl::OUStringToOString( io_rPwd,
                                                                                   RTL_TEXTENCODING_ISO_8859_1 );
                                    bAuthenticated = pPDFFile->setupDecryptionData( aIsoPwd.getStr() );
                                    // trash password string on heap
                                    rtl_zeroMemory( (void*)aIsoPwd.getStr(), aIsoPwd.getLength() );
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

                                    rtl::OUString aDocName( rInPDFFileURL.copy( rInPDFFileURL.lastIndexOf( sal_Unicode('/') )+1 ) );

                                    bool bEntered = false;
                                    do
                                    {
                                        bEntered = getPassword( xIntHdl, io_rPwd, ! bEntered, aDocName );
                                        rtl::OString aIsoPwd = rtl::OUStringToOString( io_rPwd,
                                                                                       RTL_TEXTENCODING_ISO_8859_1 );
                                        bAuthenticated = pPDFFile->setupDecryptionData( aIsoPwd.getStr() );
                                        // trash password string on heap
                                        rtl_zeroMemory( (void*)aIsoPwd.getStr(), aIsoPwd.getLength() );
                                    } while( bEntered && ! bAuthenticated );
                                }

                                OSL_TRACE( "password: %s", bAuthenticated ? "matches" : "does not match" );
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

    OSL_TRACE( "extracted add stream: mimetype %s\n",
               rtl::OUStringToOString( rOutMimetype,
                                       RTL_TEXTENCODING_UTF8 ).getStr());
    return xEmbed;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
