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

#ifndef INCLUDED_XMLSECURITY_INC_SIGSTRUCT_HXX
#define INCLUDED_XMLSECURITY_INC_SIGSTRUCT_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/xml/crypto/SecurityOperationStatus.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <set>
#include <vector>

namespace com::sun::star::graphic { class XGraphic; }

/*
 * type of reference
 */
enum class SignatureReferenceType
{
    SAMEDOCUMENT = 1,
    BINARYSTREAM = 2,
    XMLSTREAM = 3
};

struct SignatureReferenceInformation
{
    SignatureReferenceType  nType;
    OUString   ouURI;
    // For ODF: XAdES digests (SHA256) or the old SHA1, from css::xml::crypto::DigestID
    sal_Int32  nDigestID;
    OUString   ouDigestValue;
    /// Type of the reference: a URI (newer idSignedProperties references) or empty.
    OUString ouType;

    SignatureReferenceInformation() :
        nType(SignatureReferenceType::SAMEDOCUMENT),
        ouURI(""),
        nDigestID(css::xml::crypto::DigestID::SHA1),
        ouDigestValue("")
    {
    }

    SignatureReferenceInformation( SignatureReferenceType type, sal_Int32 digestID, const OUString& uri, const OUString& rType ) :
        SignatureReferenceInformation()
    {
        nType = type;
        nDigestID = digestID;
        ouURI = uri;
        ouType = rType;
    }
};

typedef ::std::vector< SignatureReferenceInformation > SignatureReferenceInformations;

namespace svl::crypto
{
/// Specifies the algorithm used for signature generation and validation.
enum class SignatureMethodAlgorithm
{
    RSA,
    ECDSA
};
}


struct SignatureInformation
{
    sal_Int32 nSecurityId;
    css::xml::crypto::SecurityOperationStatus nStatus;
    SignatureReferenceInformations  vSignatureReferenceInfors;
    struct X509CertInfo
    {
        OUString X509IssuerName;
        OUString X509SerialNumber;
        OUString X509Certificate;
        /// OOXML certificate SHA-256 digest, empty for ODF except when doing XAdES signature.
        OUString CertDigest;
        /// The certificate owner (aka subject).
        OUString X509Subject;
    };
    typedef std::vector<X509CertInfo> X509Data;
    // note: at parse time, it's unkown which one is the signing certificate;
    // ImplVerifySignatures() figures it out and puts it at the back
    std::vector<X509Data> X509Datas;

    X509CertInfo const* GetSigningCertificate() const
    {
        if (X509Datas.empty())
        {
            return nullptr;
        }
        assert(!X509Datas.back().empty());
        return & X509Datas.back().back();
    }

    OUString ouGpgKeyID;
    OUString ouGpgCertificate;
    OUString ouGpgOwner;

    OUString ouSignatureValue;
    css::util::DateTime stDateTime;

    // XAdES EncapsulatedX509Certificate values
    std::set<OUString> maEncapsulatedX509Certificates;

    OUString ouSignatureId;
    // signature may contain multiple time stamps - check they're consistent
    bool hasInconsistentSigningTime = false;
    //We also keep the date and time as string. This is done when this
    //structure is created as a result of a XML signature being read.
    //When then a signature is added or another removed, then the original
    //XML signatures are written again (unless they have been removed).
    //If the date time string is converted into the DateTime structure
    //then information can be lost because it only holds a fractional
    //of a second with an accuracy of one hundredth of second.
    //If the string contains
    //milliseconds (because the document was created by an application other than OOo)
    //and the converted time is written back, then the string looks different
    //and the signature is broken.
    OUString ouDateTime;
    /// The Id attribute of the <SignatureProperty> element that contains the <dc:date>.
    OUString ouDateTimePropertyId;
    /// Characters of the <dc:description> element inside the signature.
    OUString ouDescription;
    /// The Id attribute of the <SignatureProperty> element that contains the <dc:description>.
    OUString ouDescriptionPropertyId;
    /// Valid and invalid signature line images
    css::uno::Reference<css::graphic::XGraphic> aValidSignatureImage;
    css::uno::Reference<css::graphic::XGraphic> aInvalidSignatureImage;
    /// Signature Line Id, used to map signatures to their respective signature line images.
    OUString ouSignatureLineId;
    /// A full OOXML signature for unchanged roundtrip, empty for ODF.
    css::uno::Sequence<sal_Int8> aSignatureBytes;
    /// For PDF: digest format, from css::xml::crypto::DigestID
    sal_Int32 nDigestID;
    /// For PDF: has id-aa-signingCertificateV2 as a signed attribute.
    bool bHasSigningCertificate;
    /// For PDF: the byte range doesn't cover the whole document.
    bool bPartialDocumentSignature;

    svl::crypto::SignatureMethodAlgorithm eAlgorithmID;

    SignatureInformation( sal_Int32 nId )
    {
        nSecurityId = nId;
        nStatus = css::xml::crypto::SecurityOperationStatus_UNKNOWN;
        nDigestID = 0;
        bHasSigningCertificate = false;
        bPartialDocumentSignature = false;
        eAlgorithmID = svl::crypto::SignatureMethodAlgorithm::RSA;
    }
};

typedef ::std::vector< SignatureInformation > SignatureInformations;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
