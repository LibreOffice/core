/*************************************************************************
 *
 *  $RCSfile: sigstruct.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mmi $ $Date: 2004-07-15 08:12:08 $
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

#ifndef _SIGSTRUCT_HXX
#define _SIGSTRUCT_HXX

#include <rtl/ustring.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

/*
 * signature status
 */
#define STATUS_INIT         0
#define STATUS_CREATION_SUCCEED     1
#define STATUS_CREATION_FAIL        2
#define STATUS_VERIFY_SUCCEED       3
#define STATUS_VERIFY_FAIL      4

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
    sal_Int32           nStatus;
    SignatureReferenceInformations  vSignatureReferenceInfors;
    rtl::OUString                   ouX509IssuerName;
    rtl::OUString                   ouX509SerialNumber;
    rtl::OUString                   ouX509Certificate;
    rtl::OUString                   ouSignatureValue;
    rtl::OUString                   ouDate;
    rtl::OUString                   ouTime;
    rtl::OUString                   ouSignatureId;
    rtl::OUString                   ouPropertyId;

    SignatureInformation( sal_Int32 nId )
    {
        nSecurityId = nId;
        nStatus = STATUS_INIT;
    }
};

typedef ::std::vector< SignatureInformation > SignatureInformations;

#endif

