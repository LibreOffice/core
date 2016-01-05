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

#ifndef INCLUDED_XMLSECURITY_INC_XMLSECURITY_SIGSTRUCT_HXX
#define INCLUDED_XMLSECURITY_INC_XMLSECURITY_SIGSTRUCT_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/xml/crypto/SecurityOperationStatus.hpp>

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
    OUString   ouDigestValue;

    SignatureReferenceInformation( SignatureReferenceType type, const OUString& uri )
    {
        nType = type;
        ouURI = uri;
    }
};

typedef ::std::vector< SignatureReferenceInformation > SignatureReferenceInformations;

struct SignatureInformation
{
    sal_Int32 nSecurityId;
    sal_Int32 nSecurityEnvironmentIndex;
    ::com::sun::star::xml::crypto::SecurityOperationStatus nStatus;
    SignatureReferenceInformations  vSignatureReferenceInfors;
    OUString ouX509IssuerName;
    OUString ouX509SerialNumber;
    OUString ouX509Certificate;
    OUString ouSignatureValue;
    ::com::sun::star::util::DateTime stDateTime;

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

    SignatureInformation( sal_Int32 nId )
    {
        nSecurityId = nId;
        nStatus = ::com::sun::star::xml::crypto::SecurityOperationStatus_UNKNOWN;
        nSecurityEnvironmentIndex = -1;
    }
};

typedef ::std::vector< SignatureInformation > SignatureInformations;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
