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

#ifndef _XMLSECURITY_XMLSIGNATUREHELPER_HXX
#define _XMLSECURITY_XMLSIGNATUREHELPER_HXX

#ifndef _STLP_VECTOR
#include <vector>
#endif

#include <tools/link.hxx>

#include <rtl/ustring.hxx>

#include <xmlsecurity/sigstruct.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XWriter.hpp>
#include <com/sun/star/xml/crypto/XUriBinding.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/xml/crypto/sax/XSecurityController.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultListener.hpp>

class XSecController;
class Date;
class Time;

namespace com {
namespace sun {
namespace star {
namespace io {
    class XOutputStream;
    class XInputStream; }
namespace embed {
    class XStorage; }
}}}

struct XMLSignatureCreationResult
{
    sal_Int32 nSecurityId;
    com::sun::star::xml::crypto::SecurityOperationStatus nSignatureCreationResult;

    XMLSignatureCreationResult( sal_Int32 nId, com::sun::star::xml::crypto::SecurityOperationStatus nResult )
    {
        nSecurityId = nId;
        nSignatureCreationResult = nResult;
    }
};

struct XMLSignatureVerifyResult
{
    sal_Int32 nSecurityId;
    com::sun::star::xml::crypto::SecurityOperationStatus nSignatureVerifyResult;

    XMLSignatureVerifyResult( sal_Int32 nId, com::sun::star::xml::crypto::SecurityOperationStatus nResult )
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
    ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxCtx;
    ::com::sun::star::uno::Reference< com::sun::star::xml::crypto::sax::XSecurityController > mxSecurityController;
    ::com::sun::star::uno::Reference< com::sun::star::xml::crypto::XUriBinding > mxUriBinding;

    ::com::sun::star::uno::Reference< com::sun::star::xml::crypto::XSEInitializer > mxSEInitializer;
    ::com::sun::star::uno::Reference< com::sun::star::xml::crypto::XXMLSecurityContext > mxSecurityContext;

    XMLSignatureCreationResults maCreationResults;
    XMLSignatureVerifyResults   maVerifyResults;
    XSecController*             mpXSecController;
    bool                        mbError;
    bool mbODFPre1_2;
    Link                        maStartVerifySignatureHdl;

private:
    void        ImplCreateSEInitializer();
    DECL_LINK(  SignatureCreationResultListener, XMLSignatureCreationResult*);
    DECL_LINK(  SignatureVerifyResultListener, XMLSignatureVerifyResult* );
    DECL_LINK(  StartVerifySignatureElement, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >* );

    // Not allowed:
    XMLSignatureHelper(const XMLSignatureHelper&);

public:
    XMLSignatureHelper(const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& mrCtx );
    ~XMLSignatureHelper();

    // Initialize the security context with default crypto token.
    // Returns true for success.
    bool        Init();

    // Set the storage which should be used by the default UriBinding
    // Must be set before StatrtMission().
    //sODFVersion indicates  the ODF version
    void        SetStorage( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rxStorage, ::rtl::OUString sODFVersion );

                // Argument for the Link is a uno::Reference< xml::sax::XAttributeList >*
                // Return 1 to verify, 0 to skip.
                // Default handler will verify all.
    void        SetStartVerifySignatureHdl( const Link& rLink );

                // Get the security environment
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > GetSecurityEnvironment();

                // After signing/veryfieng, get information about signatures
    SignatureInformation  GetSignatureInformation( sal_Int32 nSecurityId ) const;
    SignatureInformations GetSignatureInformations() const;

                // See XSecController for documentation
    void        StartMission();
    void        EndMission();
    sal_Int32   GetNewSecurityId();
    /** sets data that describes the certificate.

        It is absolutely necessary that the parameter ouX509IssuerName is set. It contains
        the base64 encoded certificate, which is DER encoded. The XMLSec needs it to find
        the private key. Although issuer name and certificate should be sufficient to identify
        the certificate the implementation in XMLSec is broken, both for Windows and mozilla.
        The reason is that they use functions to find the certificate which take as parameter
        the DER encoded ASN.1 issuer name. The issuer name is a DName, where most attributes
        are of type DirectoryName, which is a choice of 5 string types. This information is
        not contained in the issuer string and while it is converted to the ASN.1 name the
        conversion function must assume a particular type, which is often wrong. For example,
        the Windows function CertStrToName will use a T.61 string if the string does not contain
        special characters. So if the certificate uses simple characters but encodes the
        issuer attributes in Utf8, then CertStrToName will use T.61. The resulting DER encoded
        ASN.1 name now contains different bytes which indicate the string type. The functions
        for finding the certificate apparently use memcmp - hence they fail to find the
        certificate.
     */
    void SetX509Certificate(sal_Int32 nSecurityId, const rtl::OUString& ouX509IssuerName,
        const rtl::OUString& ouX509SerialNumber, const rtl::OUString& ouX509Cert);

    void        SetDateTime( sal_Int32 nSecurityId, const Date& rDate, const Time& rTime );

    void        AddForSigning( sal_Int32 securityId, const rtl::OUString& uri, const rtl::OUString& objectURL, sal_Bool bBinary );
    bool        CreateAndWriteSignature( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xDocumentHandler );
    bool        ReadAndVerifySignature( const com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& xInputStream );

    // MT: ??? I think only for adding/removing, not for new signatures...
    // MM: Yes, but if you want to insert a new signature into an existing signature file, those function
    //     will be very usefull, see Mission 3 in the new "multisigdemo" program   :-)
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XWriter> CreateDocumentHandlerWithHeader( const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xOutputStream );
    void CloseDocumentHandler( const ::com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler>& xDocumentHandler );
    void ExportSignature( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xDocumentHandler, const SignatureInformation& signatureInfo );
};

#endif // _XMLSECURITY_XMLSIGNATUREHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
