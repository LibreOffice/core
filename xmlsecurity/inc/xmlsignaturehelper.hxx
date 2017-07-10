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

#ifndef INCLUDED_XMLSECURITY_INC_XMLSIGNATUREHELPER_HXX
#define INCLUDED_XMLSECURITY_INC_XMLSIGNATUREHELPER_HXX

#include <vector>

#include <tools/link.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <sigstruct.hxx>
#include <xsecctl.hxx>
#include <xmlsecurity/xmlsecuritydllapi.h>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XWriter.hpp>
#include <com/sun/star/xml/crypto/XUriBinding.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultListener.hpp>

class Date;
namespace tools { class Time; }

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
    css::xml::crypto::SecurityOperationStatus nSignatureCreationResult;

    XMLSignatureCreationResult( css::xml::crypto::SecurityOperationStatus nResult )
    {
        nSignatureCreationResult = nResult;
    }
};

struct XMLSignatureVerifyResult
{
    css::xml::crypto::SecurityOperationStatus nSignatureVerifyResult;

    XMLSignatureVerifyResult( css::xml::crypto::SecurityOperationStatus nResult )
    {
        nSignatureVerifyResult = nResult;
    }
};


/**********************************************************
 XMLSignatureHelper

 Helper class for the XML Security framework

 Functions:
 1. help to create a security context;
 2. help to listen signature creation result;
 3. help to listen signature verify result;
 4. help to indicate which signature to verify.

 **********************************************************/

class XMLSECURITY_DLLPUBLIC XMLSignatureHelper
{
private:
    css::uno::Reference< css::uno::XComponentContext > mxCtx;
    css::uno::Reference< css::xml::crypto::XUriBinding > mxUriBinding;

    std::vector<XMLSignatureCreationResult>
                                maCreationResults;
    std::vector<XMLSignatureVerifyResult>
                                maVerifyResults;
    rtl::Reference<XSecController> mpXSecController;
    bool                        mbError;
    bool mbODFPre1_2;
    Link<LinkParamNone*,bool>   maStartVerifySignatureHdl;

private:
    XMLSignatureHelper(const XMLSignatureHelper&) = delete;

public:
    XMLSignatureHelper(const css::uno::Reference< css::uno::XComponentContext >& mrCtx );
    ~XMLSignatureHelper();

    void SignatureCreationResultListener(XMLSignatureCreationResult& rResult);
    void SignatureVerifyResultListener(XMLSignatureVerifyResult& rResult);
    void StartVerifySignatureElement();

    // Set the storage which should be used by the default UriBinding
    // Must be set before StartMission().
    //sODFVersion indicates  the ODF version
    void        SetStorage( const css::uno::Reference < css::embed::XStorage >& rxStorage, const OUString& sODFVersion );

                // Argument for the Link is a uno::Reference< xml::sax::XAttributeList >*
                // Return 1 to verify, 0 to skip.
                // Default handler will verify all.
    void        SetStartVerifySignatureHdl( const Link<LinkParamNone*,bool>& rLink );

                // After signing/verifying, get information about signatures
    SignatureInformation  GetSignatureInformation( sal_Int32 nSecurityId ) const;
    SignatureInformations GetSignatureInformations() const;

                // See XSecController for documentation
    void        StartMission(const css::uno::Reference<css::xml::crypto::XXMLSecurityContext>& xSecurityContext);
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
    void SetX509Certificate(sal_Int32 nSecurityId, const OUString& ouX509IssuerName,
        const OUString& ouX509SerialNumber, const OUString& ouX509Cert, const OUString& ouX509CertDigest);

    void AddEncapsulatedX509Certificate(const OUString& ouEncapsulatedX509Certificate);

    void SetGpgCertificate(sal_Int32 nSecurityId, const OUString& ouGpgCertDigest,
        const OUString& ouGpgCert, const OUString& ouGpgOwner);

    void        SetDateTime( sal_Int32 nSecurityId, const Date& rDate, const tools::Time& rTime );
    void SetDescription(sal_Int32 nSecurityId, const OUString& rDescription);

    void        AddForSigning( sal_Int32 securityId, const OUString& uri, bool bBinary, bool bXAdESCompliantIfODF );
    bool        CreateAndWriteSignature( const css::uno::Reference< css::xml::sax::XDocumentHandler >& xDocumentHandler, bool bXAdESCompliantIfODF );
    bool        ReadAndVerifySignature( const css::uno::Reference< css::io::XInputStream >& xInputStream );

    // MT: ??? I think only for adding/removing, not for new signatures...
    // MM: Yes, but if you want to insert a new signature into an existing signature file, those function
    //     will be very useful, see Mission 3 in the new "multisigdemo" program   :-)
    css::uno::Reference< css::xml::sax::XWriter> CreateDocumentHandlerWithHeader( const css::uno::Reference< css::io::XOutputStream >& xOutputStream );
    static void CloseDocumentHandler( const css::uno::Reference< css::xml::sax::XDocumentHandler>& xDocumentHandler );
    static void ExportSignature(
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xDocumentHandler,
        const SignatureInformation& signatureInfo,
        bool bXAdESCompliantIfODF );

    /// Read and verify OOXML signatures.
    bool ReadAndVerifySignatureStorage(const css::uno::Reference<css::embed::XStorage>& xStorage, bool bCacheLastSignature = true);
    /// Read and verify a single OOXML signature.
    bool ReadAndVerifySignatureStorageStream(const css::uno::Reference<css::io::XInputStream>& xInputStream);
    /// Adds or removes an OOXML digital signature relation to _rels/.rels if there wasn't any before.
    void EnsureSignaturesRelation(const css::uno::Reference<css::embed::XStorage>& xStorage, bool bAdd);
    /// Given that xStorage is an OOXML _xmlsignatures storage, create origin.sigs and its relations.
    void ExportSignatureRelations(const css::uno::Reference<css::embed::XStorage>& xStorage, int nSignatureCount);
    /// Given that xSignatureStorage is an OOXML _xmlsignatures storage, create and write a new signature.
    bool CreateAndWriteOOXMLSignature(const css::uno::Reference<css::embed::XStorage>& xRootStorage, const css::uno::Reference<css::embed::XStorage>& xSignatureStorage, int nSignatureIndex);
    /// Similar to CreateAndWriteOOXMLSignature(), but used to write the signature to the persistent storage, not the temporary one.
    void ExportOOXMLSignature(const css::uno::Reference<css::embed::XStorage>& xRootStorage, const css::uno::Reference<css::embed::XStorage>& xSignatureStorage, const SignatureInformation& rInformation, int nSignatureIndex);
    /// Given that xStorage is an OOXML root storage, advertise signatures in its [Content_Types].xml stream.
    void ExportSignatureContentTypes(const css::uno::Reference<css::embed::XStorage>& xStorage, int nSignatureCount);
};

#endif // INCLUDED_XMLSECURITY_INC_XMLSIGNATUREHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
