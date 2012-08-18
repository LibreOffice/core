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

#include <sal/config.h>
#include <rtl/uuid.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/security/ExtAltNameType.hpp>
#include <com/sun/star/security/CertAltNameEntry.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/sequence.hxx>

#include "sanextension_mscryptimpl.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::security ;
using ::rtl::OUString ;

using ::com::sun::star::security::XCertificateExtension ;


SanExtensionImpl :: SanExtensionImpl() :
    m_critical( sal_False )
{
}

SanExtensionImpl :: ~SanExtensionImpl() {
}


//Methods from XCertificateExtension
sal_Bool SAL_CALL SanExtensionImpl :: isCritical() throw( ::com::sun::star::uno::RuntimeException ) {
    return m_critical ;
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL SanExtensionImpl :: getExtensionId() throw( ::com::sun::star::uno::RuntimeException ) {
    return m_xExtnId ;
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL SanExtensionImpl :: getExtensionValue() throw( ::com::sun::star::uno::RuntimeException ) {
    return m_xExtnValue ;
}

//Methods from XSanExtension
::com::sun::star::uno::Sequence< com::sun::star::security::CertAltNameEntry > SAL_CALL SanExtensionImpl :: getAlternativeNames() throw( ::com::sun::star::uno::RuntimeException ){

    if (!m_Entries.hasElements())
    {
        CERT_ALT_NAME_INFO *subjectName;
        DWORD size;
        CryptDecodeObjectEx(X509_ASN_ENCODING, X509_ALTERNATE_NAME, (unsigned char*) m_xExtnValue.getArray(), m_xExtnValue.getLength(), CRYPT_DECODE_ALLOC_FLAG | CRYPT_DECODE_NOCOPY_FLAG, NULL,&subjectName, &size);

        CertAltNameEntry* arrCertAltNameEntry = new CertAltNameEntry[subjectName->cAltEntry];

        for (unsigned int i = 0; i < (unsigned int)subjectName->cAltEntry; i++){
          PCERT_ALT_NAME_ENTRY pEntry = &subjectName->rgAltEntry[i];

          switch(pEntry->dwAltNameChoice) {
            case CERT_ALT_NAME_OTHER_NAME :
                {
                    arrCertAltNameEntry[i].Type = ExtAltNameType_OTHER_NAME;
                    PCERT_OTHER_NAME pOtherName = pEntry->pOtherName;

                    ::com::sun::star::beans::NamedValue otherNameProp;
                    otherNameProp.Name = ::rtl::OUString::createFromAscii(pOtherName->pszObjId);

                    Sequence< sal_Int8 > otherName( pOtherName->Value.cbData ) ;
                    for( unsigned int n = 0; n < (unsigned int) pOtherName->Value.cbData ; n ++ )
                        otherName[n] = *( pOtherName->Value.pbData + n ) ;

                    otherNameProp.Value <<= otherName;

                    arrCertAltNameEntry[i].Value <<= otherNameProp;
                    break;
                }
            case CERT_ALT_NAME_RFC822_NAME :
                arrCertAltNameEntry[i].Type = ExtAltNameType_RFC822_NAME;
                arrCertAltNameEntry[i].Value <<= ::rtl::OUString((const sal_Unicode*)pEntry->pwszRfc822Name);
                break;
            case CERT_ALT_NAME_DNS_NAME :
                arrCertAltNameEntry[i].Type = ExtAltNameType_DNS_NAME;
                arrCertAltNameEntry[i].Value <<= ::rtl::OUString((const sal_Unicode*)pEntry->pwszDNSName);
                break;
            case CERT_ALT_NAME_DIRECTORY_NAME :
                {
                    arrCertAltNameEntry[i].Type = ExtAltNameType_DIRECTORY_NAME;
                    break;
                }
            case CERT_ALT_NAME_URL :
                arrCertAltNameEntry[i].Type = ExtAltNameType_URL;
                arrCertAltNameEntry[i].Value <<= ::rtl::OUString((const sal_Unicode*)pEntry->pwszURL);
                break;
            case CERT_ALT_NAME_IP_ADDRESS :
                {
                    arrCertAltNameEntry[i].Type = ExtAltNameType_IP_ADDRESS;

                    Sequence< sal_Int8 > ipAddress( pEntry->IPAddress.cbData ) ;
                    for( unsigned int n = 0; n < pEntry->IPAddress.cbData ; n ++ )
                        ipAddress[n] = *( pEntry->IPAddress.pbData + n ) ;

                    arrCertAltNameEntry[i].Value <<= ipAddress;
                    break;
                }
            case CERT_ALT_NAME_REGISTERED_ID :
                arrCertAltNameEntry[i].Type = ExtAltNameType_REGISTERED_ID;
                arrCertAltNameEntry[i].Value <<= ::rtl::OUString::createFromAscii(pEntry->pszRegisteredID);
                break;
          }
        }
        m_Entries = ::comphelper::arrayToSequence< com::sun::star::security::CertAltNameEntry >(arrCertAltNameEntry, subjectName->cAltEntry);

        delete [] arrCertAltNameEntry;
    }

    return m_Entries;
}

//Helper method
void SanExtensionImpl :: setCertExtn( unsigned char* value, unsigned int vlen, unsigned char* id, unsigned int idlen, sal_Bool critical ) {
    unsigned int i ;
    if( value != NULL && vlen != 0 ) {
        Sequence< sal_Int8 > extnv( vlen ) ;
        for( i = 0; i < vlen ; i ++ )
            extnv[i] = *( value + i ) ;

        m_xExtnValue = extnv ;
    } else {
        m_xExtnValue = Sequence<sal_Int8>();
    }

    if( id != NULL && idlen != 0 ) {
        Sequence< sal_Int8 > extnId( idlen ) ;
        for( i = 0; i < idlen ; i ++ )
            extnId[i] = *( id + i ) ;

        m_xExtnId = extnId ;
    } else {
        m_xExtnId =  Sequence<sal_Int8>();
    }

    m_critical = critical ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
