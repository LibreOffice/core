/*************************************************************************
 *
 *  $RCSfile: xmlsignaturehelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mmi $ $Date: 2004-07-16 02:22:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <xmlsecurity/xmlsignaturehelper.hxx>
#include <xsecctl.hxx>

#include <xmlsignaturehelper2.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>

#include <xmloff/attrlist.hxx>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>

#include <tools/date.hxx>
#include <tools/time.hxx>

//MM : search for the default profile
#include <unotools/streamhelper.hxx>
//MM : end

/* SEInitializer component */
#define SEINITIALIZER_COMPONENT "com.sun.star.xml.crypto.SEInitializer"

#define SIGFILEROOTELEMENT  "all-signatures"

using namespace ::com::sun::star;

XMLSignatureHelper::XMLSignatureHelper( const uno::Reference< lang::XMultiServiceFactory>& rxMSF )
    : mxMSF(rxMSF)
{
    mpXSecController = new XSecController;
    mxSecurityController = mpXSecController;
    mpXSecController->setFactory( rxMSF );
    mbError = false;
}

XMLSignatureHelper::~XMLSignatureHelper()
{
    if (mxSEInitializer.is())
    {
        mxSEInitializer->freeSecurityContext( mxSecurityContext );
    }
}

bool XMLSignatureHelper::Init( const rtl::OUString& rTokenPath )
{
    DBG_ASSERT( !mxSEInitializer.is(), "XMLSignatureHelper::Init - mxSEInitializer already set!" );
    DBG_ASSERT( !mxSecurityContext.is(), "XMLSignatureHelper::Init - mxSecurityContext already set!" );

    ImplCreateSEInitializer();

    //MM : search for the default profile
    rtl::OUString tokenPath = rTokenPath;

    if( tokenPath.getLength() == 0 )
    {
        rtl::OUString aDefaultCryptokenFileName = rtl::OUString::createFromAscii("cryptoken.default");
        SvFileStream* pStream = new SvFileStream( aDefaultCryptokenFileName, STREAM_READ );
        if (pStream != NULL)
        {
            pStream->Seek( STREAM_SEEK_TO_END );
            ULONG nBytes = pStream->Tell();

            if (nBytes > 0)
            {
                pStream->Seek( STREAM_SEEK_TO_BEGIN );
                SvLockBytesRef xLockBytes = new SvLockBytes( pStream, TRUE );
                uno::Reference< io::XInputStream > xInputStream = new utl::OInputStreamHelper( xLockBytes, nBytes );

                if (xInputStream.is())
                {
                    uno::Sequence< sal_Int8 > tokenFileName( 1024 );
                    int numbers = xInputStream->readBytes( tokenFileName, 1024 );
                    const sal_Int8* readBytes = ( const sal_Int8* )tokenFileName.getArray();

                    sal_Char cToken[1024];

                    for (int i=0; i<numbers; i++)
                    {
                        cToken[i] = (sal_Char)(*(readBytes+i));
                    }

                    xInputStream->closeInput();

                    tokenPath = rtl::OStringToOUString(rtl::OString((const sal_Char*)cToken, numbers), RTL_TEXTENCODING_UTF8);
                }
            }
        }
    }

    mxSecurityContext = mxSEInitializer->createSecurityContext( tokenPath );
    //MM : end

    //MM : search for the default profile
    //mxSecurityContext = mxSEInitializer->createSecurityContext( rTokenPath );
    //MM : end

    return mxSecurityContext.is();
}

void XMLSignatureHelper::ImplCreateSEInitializer()
{
    rtl::OUString sSEInitializer;

    sSEInitializer = rtl::OUString::createFromAscii( SEINITIALIZER_COMPONENT );
    mxSEInitializer = uno::Reference< com::sun::star::xml::crypto::XSEInitializer > (
            mxMSF->createInstance( sSEInitializer ), uno::UNO_QUERY );
}

void XMLSignatureHelper::SetUriBinding( com::sun::star::uno::Reference< com::sun::star::xml::crypto::XUriBinding >& rxUriBinding )
{
    mxUriBinding = rxUriBinding;
}

void XMLSignatureHelper::SetStorage( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rxStorage )
{
    DBG_ASSERT( !mxUriBinding.is(), "SetStorage - UriBinding already set!" );
    mxUriBinding = new UriBindingHelper( rxStorage );
}


void XMLSignatureHelper::SetStartVerifySignatureHdl( const Link& rLink )
{
    maStartVerifySignatureHdl = rLink;
}


void XMLSignatureHelper::StartMission()
{
    if ( !mxUriBinding.is() )
        mxUriBinding = new UriBindingHelper();

    mpXSecController->startMission( mxUriBinding, mxSecurityContext );
}

void XMLSignatureHelper::EndMission()
{
    mpXSecController->endMission();
}

sal_Int32 XMLSignatureHelper::GetNewSecurityId()
{
    return mpXSecController->getNewSecurityId();
}

void XMLSignatureHelper::SetX509Certificate(
        sal_Int32 nSecurityId,
        const rtl::OUString& ouX509IssuerName,
        const rtl::OUString& ouX509SerialNumber)
{
    mpXSecController->setX509Certificate(
        nSecurityId,
        ouX509IssuerName,
        ouX509SerialNumber);
}

void XMLSignatureHelper::SetDateTime( sal_Int32 nSecurityId, const Date& rDate, const Time& rTime )
{
    rtl::OUString aDate = String::CreateFromInt32( rDate.GetDate() );
    rtl::OUString aTime = String::CreateFromInt32( rTime.GetTime() );
    mpXSecController->setDateTime( nSecurityId, aDate, aTime );
}

void XMLSignatureHelper::AddForSigning( sal_Int32 nSecurityId, const rtl::OUString& uri, const rtl::OUString& objectURL, sal_Bool bBinary )
{
    mpXSecController->signAStream( nSecurityId, uri, objectURL, bBinary );
}

uno::Reference<xml::sax::XDocumentHandler> XMLSignatureHelper::CreateDocumentHandlerWithHeader(
    const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xOutputStream )
{
    /*
     * get SAX writer component
     */
    uno::Reference< io::XActiveDataSource > xSaxWriter(
        mxMSF->createInstance(rtl::OUString::createFromAscii(
            "com.sun.star.xml.sax.Writer")),
        uno::UNO_QUERY );

    DBG_ASSERT( xSaxWriter.is(), "can't instantiate XML writer" );

    /*
     * connect XML writer to output stream
     */
    xSaxWriter->setOutputStream( xOutputStream );

    /*
     * prepare document handler
     */
    uno::Reference<xml::sax::XDocumentHandler>
        xDocHandler( xSaxWriter,uno::UNO_QUERY);

    /*
     * write the xml context for signatures
     */
    rtl::OUString tag_AllSignatures(RTL_CONSTASCII_USTRINGPARAM(SIGFILEROOTELEMENT));

    xDocHandler->startDocument();
    xDocHandler->startElement(
        tag_AllSignatures,
        uno::Reference< xml::sax::XAttributeList > (new SvXMLAttributeList()));

    return xDocHandler;
}

void XMLSignatureHelper::CloseDocumentHandler( const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler )
{
    rtl::OUString tag_AllSignatures(RTL_CONSTASCII_USTRINGPARAM(SIGFILEROOTELEMENT));
    xDocumentHandler->endElement( tag_AllSignatures );
    xDocumentHandler->endDocument();
}

void XMLSignatureHelper::ExportSignature(
    const uno::Reference< xml::sax::XDocumentHandler >& xDocumentHandler,
    const SignatureInformation& signatureInfo )
{
    mpXSecController->exportSignature(xDocumentHandler, signatureInfo);
}

bool XMLSignatureHelper::CreateAndWriteSignature( const uno::Reference< xml::sax::XDocumentHandler >& xDocumentHandler )
{
    mbError = false;

    /*
     * create a signature listener
     */
    ImplXMLSignatureListener* pSignatureListener = new ImplXMLSignatureListener(
                                                    LINK( this, XMLSignatureHelper, SignatureCreationResultListener ),
                                                    LINK( this, XMLSignatureHelper, SignatureVerifyResultListener ),
                                                    LINK( this, XMLSignatureHelper, StartVerifySignatureElement ) );

    /*
     * configure the signature creation listener
     */
    //mpXSecController->setSignatureCreationResultListener( pSignatureListener );

    /*
     * write signatures
     */
    if ( !mpXSecController->WriteSignature( xDocumentHandler ) )
    {
        mbError = true;
    }

    /*
     * clear up the signature creation listener
     */
    //mpXSecController->setSignatureCreationResultListener( NULL );

    return !mbError;
}

bool XMLSignatureHelper::CreateAndWriteSignature( const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xOutputStream )
{
    uno::Reference<xml::sax::XDocumentHandler> xDocHandler
        = CreateDocumentHandlerWithHeader(xOutputStream);

    bool rc = CreateAndWriteSignature( xDocHandler );

    CloseDocumentHandler(xDocHandler);

    return rc;
}

bool XMLSignatureHelper::ReadAndVerifySignature( const com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& xInputStream )
{
    mbError = false;

    DBG_ASSERT(xInputStream.is(), "input stream missing");

    /*
     * prepare ParserInputSrouce
     */
    xml::sax::InputSource aParserInput;
    // aParserInput.sSystemId = ouName;
    aParserInput.aInputStream = xInputStream;

    /*
     * get SAX parser component
     */
    uno::Reference< xml::sax::XParser > xParser(
        mxMSF->createInstance(
            rtl::OUString::createFromAscii("com.sun.star.xml.sax.Parser") ),
        uno::UNO_QUERY );
    DBG_ASSERT( xParser.is(), "Can't create parser" );

    /*
     * create a signature reader
     */
    uno::Reference< xml::sax::XDocumentHandler > xHandler
        = mpXSecController->createSignatureReader( );

    /*
     * create a signature listener
     */
    ImplXMLSignatureListener* pSignatureListener = new ImplXMLSignatureListener(
                                                    LINK( this, XMLSignatureHelper, SignatureCreationResultListener ),
                                                    LINK( this, XMLSignatureHelper, SignatureVerifyResultListener ),
                                                    LINK( this, XMLSignatureHelper, StartVerifySignatureElement ) );

    /*
     * configure the signature verify listener
     */
    //mpXSecController->setSignatureVerifyResultListener( pSignatureListener );

    /*
     * setup the connection:
     * Parser -> SignatureListener -> SignatureReader
     */
    pSignatureListener->setNextHandler(xHandler);
    xParser->setDocumentHandler( pSignatureListener );

    /*
     * parser the stream
     */
    try
    {
        xParser->parseStream( aParserInput );
    }
    catch( xml::sax::SAXParseException& )
    {
        mbError = true;
    }
    catch( xml::sax::SAXException& )
    {
        mbError = true;
    }
    catch( com::sun::star::io::IOException& )
    {
        mbError = true;
    }
    catch( uno::Exception& )
    {
        mbError = true;
    }

    /*
     * clear up the connection
     */
    pSignatureListener->setNextHandler( NULL );

    /*
     * clear up the signature verify listener
     */
    //mpXSecController->setSignatureVerifyResultListener( NULL );

    /*
     * release the signature reader
     */
    mpXSecController->releaseSignatureReader( );

    return !mbError;
}

SignatureInformations XMLSignatureHelper::GetSignatureInformations() const
{
    return mpXSecController->getSignatureInformations();
}

uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > XMLSignatureHelper::GetSecurityEnvironment()
{
    return (mxSecurityContext.is()?(mxSecurityContext->getSecurityEnvironment()):NULL);
}

/*
void XMLSignatureHelper::createSecurityContext( rtl::OUString tokenPath )
{
    if ( !mxSEInitializer.is() )
        ImplCreateSEInitializer();

    mxSecurityContext = mxSEInitializer->createSecurityContext(tokenPath);
}

void XMLSignatureHelper::freeSecurityContext()
{
    if ( !mxSEInitializer.is() )
        ImplCreateSEInitializer();

    mxSEInitializer->freeSecurityContext( mxSecurityContext );
}
*/

IMPL_LINK( XMLSignatureHelper, SignatureCreationResultListener, XMLSignatureCreationResult*, pResult )
{
    maCreationResults.insert( maCreationResults.begin() + maCreationResults.size(), *pResult );
    if ( pResult->nSignatureCreationResult != com::sun::star::xml::crypto::sax::SignatureCreationResult_CREATIONSUCCEED )
        mbError = true;
    return 0;
}

IMPL_LINK( XMLSignatureHelper, SignatureVerifyResultListener, XMLSignatureVerifyResult*, pResult )
{
    maVerifyResults.insert( maVerifyResults.begin() + maVerifyResults.size(), *pResult );
    if ( pResult->nSignatureVerifyResult != com::sun::star::xml::crypto::sax::SignatureVerifyResult_VERIFYSUCCEED )
        mbError = true;
    return 0;
}

IMPL_LINK( XMLSignatureHelper, StartVerifySignatureElement, const uno::Reference< com::sun::star::xml::sax::XAttributeList >*, pAttrs )
{
    if ( !maStartVerifySignatureHdl.IsSet() || maStartVerifySignatureHdl.Call( (void*)pAttrs ) )
    {
        sal_Int32 nSignatureId = mpXSecController->getNewSecurityId();
        mpXSecController->addSignature( nSignatureId );
    }

    return 0;
}



