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

    SignatureReferenceInformation() :
        nType(SignatureReferenceType::SAMEDOCUMENT),
        ouURI(""),
        nDigestID(css::xml::crypto::DigestID::SHA1),
        ouDigestValue("")
    {
    }

    SignatureReferenceInformation( SignatureReferenceType type, sal_Int32 digestID, const OUString& uri ) :
        SignatureReferenceInformation()
    {
        nType = type;
        nDigestID = digestID;
        ouURI = uri;
    }
};

typedef ::std::vector< SignatureReferenceInformation > SignatureReferenceInformations;

struct SignatureInformation
{
    sal_Int32 nSecurityId;
    css::xml::crypto::SecurityOperationStatus nStatus;
    SignatureReferenceInformations  vSignatureReferenceInfors;
    OUString ouX509IssuerName;
    OUString ouX509SerialNumber;
    OUString ouX509Certificate;

    OUString ouGpgKeyID;
    OUString ouGpgCertificate;
    OUString ouGpgOwner;

    OUString ouSignatureValue;
    css::util::DateTime stDateTime;

    // XAdES EncapsulatedX509Certificate values
    std::set<OUString> maEncapsulatedX509Certificates;

    //We also keep the date and time as string. This is done when this
    //structure is created as a result of a XML signature being read.
    //When then a signature is added or another removed, then the original
    //XML signatures are written again (unless they have been removed).
    //If the date time string is converted into the DateTime structure
    //then information can be lost because it only holds a fractional
    //of a second with a accuracy of one hundredth of second.
    //If the string contains
    //milli seconds (because the document was created by an application other than OOo)
    //and the converted time is written back, then the string looks different
    //and the signature is broken.
    OUString ouDateTime;
    OUString ouSignatureId;
    OUString ouPropertyId;
    /// Characters of the <dc:description> element inside the signature.
    OUString ouDescription;
    /// The Id attribute of the <SignatureProperty> element that contains the <dc:description>.
    OUString ouDescriptionPropertyId;
    /// OOXML certificate SHA-256 digest, empty for ODF except when doing XAdES signature.
    OUString ouCertDigest;
    /// A full OOXML signature for unchanged roundtrip, empty for ODF.
    css::uno::Sequence<sal_Int8> aSignatureBytes;
    /// For PDF: digest format, from css::xml::crypto::DigestID
    sal_Int32 nDigestID;
    /// For PDF: has id-aa-signingCertificateV2 as a signed attribute.
    bool bHasSigningCertificate;
    /// For PDF: the byte range doesn't cover the whole document.
    bool bPartialDocumentSignature;

    SignatureInformation( sal_Int32 nId )
    {
        nSecurityId = nId;
        nStatus = css::xml::crypto::SecurityOperationStatus_UNKNOWN;
        nDigestID = 0;
        bHasSigningCertificate = false;
        bPartialDocumentSignature = false;
    }
};

typedef ::std::vector< SignatureInformation > SignatureInformations;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
