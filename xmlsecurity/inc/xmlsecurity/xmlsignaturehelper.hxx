/*************************************************************************
 *
 *  $RCSfile: xmlsignaturehelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-15 07:16:08 $
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

#ifndef _XMLSECURITY_XMLSIGNATUREHELPER_HXX
#define _XMLSECURITY_XMLSIGNATUREHELPER_HXX

#ifndef _STLP_VECTOR
#include <vector>
#endif

#include <tools/link.hxx>

#include <rtl/ustring.hxx>

#include <xmlsecurity/sigstruct.hxx>

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/crypto/XUriBinding.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/xml/crypto/sax/XSecurityController.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultListener.hpp>

extern rtl::OUString bigIntegerToNumericString( ::com::sun::star::uno::Sequence< sal_Int8 > serial );
extern ::com::sun::star::uno::Sequence< sal_Int8 > numericStringToBigInteger (  rtl::OUString serialNumber );

class XSecController;
class Date;
class Time;

namespace com {
namespace sun {
namespace star {
namespace lang {
    class XMultiServiceFactory; }
namespace io {
    class XOutputStream;
    class XInputStream; }
namespace embed {
    class XStorage; }
}}}

struct XMLSignatureCreationResult
{
    sal_Int32 nSecurityId;
    com::sun::star::xml::crypto::sax::SignatureCreationResult nSignatureCreationResult;

    XMLSignatureCreationResult( sal_Int32 nId, com::sun::star::xml::crypto::sax::SignatureCreationResult nResult )
    {
        nSecurityId = nId;
        nSignatureCreationResult = nResult;
    }
};

struct XMLSignatureVerifyResult
{
    sal_Int32 nSecurityId;
    com::sun::star::xml::crypto::sax::SignatureVerifyResult nSignatureVerifyResult;

    XMLSignatureVerifyResult( sal_Int32 nId, com::sun::star::xml::crypto::sax::SignatureVerifyResult nResult )
    {
        nSecurityId = nId;
        nSignatureVerifyResult = nResult;
    }
};

typedef ::std::vector<XMLSignatureCreationResult> XMLSignatureCreationResults;
typedef ::std::vector<XMLSignatureVerifyResult> XMLSignatureVerifyResults;



/**********************************************************
 XMLSignatureHelper

 Helper class for the XML Security framework

 Functions:
 1. help to create a security context;
 2. help to listen signature creation result;
 3. help to listen signature verify result;
 4. help to indicate which signature to verify.

 **********************************************************/

class XMLSignatureHelper
{
private:
    ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory> mxMSF;
    ::com::sun::star::uno::Reference< com::sun::star::xml::crypto::sax::XSecurityController > mxSecurityController;
    ::com::sun::star::uno::Reference< com::sun::star::xml::crypto::XUriBinding > mxUriBinding;

    ::com::sun::star::uno::Reference< com::sun::star::xml::crypto::XSEInitializer > mxSEInitializer;
    ::com::sun::star::uno::Reference< com::sun::star::xml::crypto::XXMLSecurityContext > mxSecurityContext;

    XMLSignatureCreationResults maCreationResults;
    XMLSignatureVerifyResults   maVerifyResults;
    XSecController*             mpXSecController;
    bool                        mbError;
    Link                        maStartVerifySignatureHdl;

private:
    void        ImplCreateSEInitializer();
    DECL_LINK(  SignatureCreationResultListener, XMLSignatureCreationResult*);
    DECL_LINK(  SignatureVerifyResultListener, XMLSignatureVerifyResult* );
    DECL_LINK(  StartVerifySignatureElement, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >* );

    // Not allowed:
    XMLSignatureHelper(const XMLSignatureHelper&);

public:
    XMLSignatureHelper(const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory>& rxMSF );
    ~XMLSignatureHelper();

                // Initialize the security context with given crypto token.
                // Empty string means default crypto token.
                // Returns true for success.
    bool        Init( const rtl::OUString& rTokenPath );

                // Set UriBinding to create input streams to open files.
                // Default implementation is capable to open files from disk.
    void        SetUriBinding( com::sun::star::uno::Reference< com::sun::star::xml::crypto::XUriBinding >& rxUriBinding );

                // Set the storage which should be used by the default UriBinding
                // Must be set before StatrtMission().
    void        SetStorage( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rxStorage );

                // Argument for the Link is a uno::Reference< xml::sax::XAttributeList >*
                // Return 1 to verify, 0 to skip.
                // Default handler will verify all.
    void        SetStartVerifySignatureHdl( const Link& rLink );

                // Get the security environment
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > GetSecurityEnvironment();

                // After signing/veryfieng, get information about signatures
    SignatureInformations GetSignatureInformations() const;

                // See XSecController for documentation
    void        StartMission();
    void        EndMission();
    sal_Int32   GetNewSecurityId();
    void        SetX509Certificate( sal_Int32 nSecurityId, const rtl::OUString& ouX509IssuerName, const rtl::OUString& ouX509SerialNumber);
    void        SetDateTime( sal_Int32 nSecurityId, const Date& rDate, const Time& rTime );

    void        AddForSigning( sal_Int32 securityId, const rtl::OUString& uri, const rtl::OUString& objectURL, sal_Bool bBinary );
    bool        CreateAndWriteSignature( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xDocumentHandler );
    bool        CreateAndWriteSignature( const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xOutputStream );
    bool        ReadAndVerifySignature( const com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& xInputStream );

    // MT: ??? I think only for adding/removing, not for new signatures...
    // MM: Yes, but if you want to insert a new signature into an existing signature file, those function
    //     will be very usefull, see Mission 3 in the new "multisigdemo" program   :-)
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler> CreateDocumentHandlerWithHeader( const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xOutputStream );
    void CloseDocumentHandler( const ::com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler>& xDocumentHandler );
    void ExportSignature( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xDocumentHandler, const SignatureInformation& signatureInfo );
};

#endif // _XMLSECURITY_XMLSIGNATUREHELPER_HXX
