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

#include <memory>

#include <rtl/uuid.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/security/ExtAltNameType.hpp>
#include <com/sun/star/security/CertAltNameEntry.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <comphelper/sequence.hxx>

#include "sanextension_mscryptimpl.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::security ;

using ::com::sun::star::security::XCertificateExtension ;


SanExtensionImpl::SanExtensionImpl() :
    m_critical( false )
{
}

SanExtensionImpl::~SanExtensionImpl() {
}


//Methods from XCertificateExtension
sal_Bool SAL_CALL SanExtensionImpl::isCritical() {
    return m_critical ;
}

css::uno::Sequence< sal_Int8 > SAL_CALL SanExtensionImpl::getExtensionId() {
    return m_xExtnId ;
}

css::uno::Sequence< sal_Int8 > SAL_CALL SanExtensionImpl::getExtensionValue() {
    return m_xExtnValue ;
}

//Methods from XSanExtension
css::uno::Sequence< css::security::CertAltNameEntry > SAL_CALL SanExtensionImpl::getAlternativeNames(){

    if (!m_Entries.hasElements())
    {
        CERT_ALT_NAME_INFO *subjectName;
        DWORD size;
        CryptDecodeObjectEx(X509_ASN_ENCODING, X509_ALTERNATE_NAME, reinterpret_cast<unsigned char*>(m_xExtnValue.getArray()), m_xExtnValue.getLength(), CRYPT_DECODE_ALLOC_FLAG | CRYPT_DECODE_NOCOPY_FLAG, nullptr,&subjectName, &size);

        auto arrCertAltNameEntry = std::unique_ptr<CertAltNameEntry[]>(new CertAltNameEntry[subjectName->cAltEntry]);

        for (unsigned int i = 0; i < (unsigned int)subjectName->cAltEntry; i++){
          PCERT_ALT_NAME_ENTRY pEntry = &subjectName->rgAltEntry[i];

          switch(pEntry->dwAltNameChoice) {
            case CERT_ALT_NAME_OTHER_NAME :
                {
                    arrCertAltNameEntry[i].Type = ExtAltNameType_OTHER_NAME;
                    PCERT_OTHER_NAME pOtherName = pEntry->pOtherName;

                    css::beans::NamedValue otherNameProp;
                    otherNameProp.Name = OUString::createFromAscii(pOtherName->pszObjId);

                    Sequence< sal_Int8 > otherName( pOtherName->Value.cbData ) ;
                    for( unsigned int n = 0; n < (unsigned int) pOtherName->Value.cbData ; n ++ )
                        otherName[n] = *( pOtherName->Value.pbData + n ) ;

                    otherNameProp.Value <<= otherName;

                    arrCertAltNameEntry[i].Value <<= otherNameProp;
                    break;
                }
            case CERT_ALT_NAME_RFC822_NAME :
                arrCertAltNameEntry[i].Type = ExtAltNameType_RFC822_NAME;
                arrCertAltNameEntry[i].Value <<= OUString(pEntry->pwszRfc822Name);
                break;
            case CERT_ALT_NAME_DNS_NAME :
                arrCertAltNameEntry[i].Type = ExtAltNameType_DNS_NAME;
                arrCertAltNameEntry[i].Value <<= OUString(pEntry->pwszDNSName);
                break;
            case CERT_ALT_NAME_DIRECTORY_NAME :
                {
                    arrCertAltNameEntry[i].Type = ExtAltNameType_DIRECTORY_NAME;
                    break;
                }
            case CERT_ALT_NAME_URL :
                arrCertAltNameEntry[i].Type = ExtAltNameType_URL;
                arrCertAltNameEntry[i].Value <<= OUString(pEntry->pwszURL);
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
                arrCertAltNameEntry[i].Value <<= OUString::createFromAscii(pEntry->pszRegisteredID);
                break;
          }
        }
        m_Entries = ::comphelper::arrayToSequence< css::security::CertAltNameEntry >(arrCertAltNameEntry.get(), subjectName->cAltEntry);
    }

    return m_Entries;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
