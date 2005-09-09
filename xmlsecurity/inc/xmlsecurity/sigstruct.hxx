/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sigstruct.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:07:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SIGSTRUCT_HXX
#define _SIGSTRUCT_HXX

#include <rtl/ustring.hxx>

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

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
    sal_Int32                       nSecurityId;
    sal_Int32           nSecurityEnvironmentIndex;

    //sal_Int32         nStatus;
    ::com::sun::star::xml::crypto::SecurityOperationStatus nStatus;

    SignatureReferenceInformations  vSignatureReferenceInfors;
    rtl::OUString                   ouX509IssuerName;
    rtl::OUString                   ouX509SerialNumber;
    rtl::OUString                   ouX509Certificate;
    rtl::OUString                   ouSignatureValue;

    //rtl::OUString                   ouDate;
    //rtl::OUString                   ouTime;
    ::com::sun::star::util::DateTime stDateTime;

    rtl::OUString                   ouSignatureId;
    rtl::OUString                   ouPropertyId;

    SignatureInformation( sal_Int32 nId )
    {
        nSecurityId = nId;
        nStatus = ::com::sun::star::xml::crypto::SecurityOperationStatus_UNKNOWN;
        nSecurityEnvironmentIndex = -1;
    }
};

typedef ::std::vector< SignatureInformation > SignatureInformations;

#endif

