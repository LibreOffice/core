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

#ifndef _SIGSTRUCT_HXX
#define _SIGSTRUCT_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/util/DateTime.hpp>

#include <com/sun/star/xml/crypto/SecurityOperationStatus.hpp>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

/*
 * signature status
 */
/*
#define STATUS_INIT         0
#define STATUS_CREATION_SUCCEED     1
#define STATUS_CREATION_FAIL        2
#define STATUS_VERIFY_SUCCEED       3
#define STATUS_VERIFY_FAIL      4
*/

/*
 * type of reference
 */
#define TYPE_SAMEDOCUMENT_REFERENCE 1
#define TYPE_BINARYSTREAM_REFERENCE 2
#define TYPE_XMLSTREAM_REFERENCE    3

struct SignatureReferenceInformation
{
    sal_Int32       nType;
    rtl::OUString   ouURI;
    rtl::OUString   ouDigestValue;

    SignatureReferenceInformation( sal_Int32 type, rtl::OUString uri )
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
    rtl::OUString ouX509IssuerName;
    rtl::OUString ouX509SerialNumber;
    rtl::OUString ouX509Certificate;
    rtl::OUString ouSignatureValue;
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
    rtl::OUString ouDateTime;
    rtl::OUString ouSignatureId;
    rtl::OUString ouPropertyId;

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
