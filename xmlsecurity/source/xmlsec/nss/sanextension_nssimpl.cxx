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
#include <rtl/ustring.hxx>
#include <com/sun/star/security/ExtAltNameType.hpp>
#include <com/sun/star/security/CertAltNameEntry.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/sequence.hxx>
#include <seccomon.h>
#include <cert.h>
#include <certt.h>

#include "sanextension_nssimpl.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::security ;

using ::com::sun::star::security::XCertificateExtension ;

namespace {
    // Helper functions from nss/lib/certdb/genname.c
    int GetNamesLength(CERTGeneralName *names)
    {
        int              length = 0;
        CERTGeneralName  *first;

        first = names;
        if (names != nullptr) {
            do {
                length++;
                names = CERT_GetNextGeneralName(names);
            } while (names != first);
        }
        return length;
    }

}

//Methods from XSanExtension
css::uno::Sequence< css::security::CertAltNameEntry > SAL_CALL SanExtensionImpl::getAlternativeNames()
{
    if (m_Entries.empty())
    {
        SECItem item;

        item.type = siDERCertBuffer;
        item.data = reinterpret_cast<unsigned char*>(m_Extn.m_xExtnValue.getArray());
        item.len = m_Extn.m_xExtnValue.getLength();

        PRArenaPool *arena;
        CERTGeneralName *nameList;
        arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);

        if (!arena)
            return css::uno::Sequence<css::security::CertAltNameEntry>();

        nameList = CERT_DecodeAltNameExtension(arena, &item);

        CERTGeneralName* current = nameList;

        int size = GetNamesLength(nameList);
        m_Entries.resize(size);
        for(int i = 0; i < size; ++i){
            switch (current->type) {
                case certOtherName: {
                    m_Entries[i].Type = ExtAltNameType_OTHER_NAME;
                    css::beans::PropertyValue otherNameProp;
                    otherNameProp.Name = OUString::createFromAscii(CERT_GetOidString(&current->name.OthName.oid));

                    Sequence< sal_Int8 > otherName( current->name.OthName.name.len ) ;
                    for( unsigned int r = 0; r < current->name.OthName.name.len ; r ++ )
                        otherName[r] = *( current->name.OthName.name.data + r ) ;

                    otherNameProp.Value <<= otherName;

                    m_Entries[i].Value <<= otherNameProp;
                    break;
                                    }
                case certRFC822Name:
                    m_Entries[i].Type = ExtAltNameType_RFC822_NAME;
                    m_Entries[i].Value <<= OUString(reinterpret_cast<char*>(current->name.other.data), current->name.other.len, RTL_TEXTENCODING_ASCII_US);
                    break;
                case certDNSName:
                    m_Entries[i].Type = ExtAltNameType_DNS_NAME;
                    m_Entries[i].Value <<= OUString(reinterpret_cast<char*>(current->name.other.data), current->name.other.len, RTL_TEXTENCODING_ASCII_US);
                    break;
                case certX400Address: {
                    // unsupported
                    m_Entries[i].Type = ExtAltNameType_X400_ADDRESS;
                    break;
                                      }
                case certDirectoryName: {
                    // unsupported
                    m_Entries[i].Type = ExtAltNameType_DIRECTORY_NAME;
                    break;
                                        }
                case certEDIPartyName:  {
                    // unsupported
                    m_Entries[i].Type = ExtAltNameType_EDI_PARTY_NAME;
                    break;
                                        }
                case certURI:
                    m_Entries[i].Type = ExtAltNameType_URL;
                    m_Entries[i].Value <<= OUString(reinterpret_cast<char*>(current->name.other.data), current->name.other.len, RTL_TEXTENCODING_ASCII_US);
                    break;
                case certIPAddress: {
                    m_Entries[i].Type = ExtAltNameType_IP_ADDRESS;

                    Sequence< sal_Int8 > ipAddress( current->name.other.len ) ;
                    for( unsigned int r = 0; r < current->name.other.len ; r ++ )
                        ipAddress[r] = *( current->name.other.data + r ) ;

                    m_Entries[i].Value <<= ipAddress;
                    break;
                                    }
                case certRegisterID:
                    m_Entries[i].Type = ExtAltNameType_REGISTERED_ID;


                    OString nssOid(CERT_GetOidString(&current->name.other));
                    OString unoOid = removeOIDFromString(nssOid);
                    m_Entries[i].Value <<= OStringToOUString( unoOid, RTL_TEXTENCODING_ASCII_US );
                    break;
            }
            current = CERT_GetNextGeneralName(current);
        }

        PORT_FreeArena(arena, PR_FALSE);
    }

    return comphelper::containerToSequence<css::security::CertAltNameEntry>(m_Entries);
}

OString SanExtensionImpl::removeOIDFromString( const OString &oidString)
{
    OString objID;
    OString oid("OID.");
    if (oidString.match(oid))
        objID = oidString.copy(oid.getLength());
    else
        objID = oidString;
    return objID;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
