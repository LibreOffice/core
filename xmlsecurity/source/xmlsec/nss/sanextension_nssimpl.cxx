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
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/sequence.hxx>
#include <seccomon.h>
#include <cert.h>
#include <certt.h>
#include <secitem.h>
#include <secport.h>

#include "sanextension_nssimpl.hxx"

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

namespace {
    // Helper functions from nss/lib/certdb/genname.c
    static int GetNamesLength(CERTGeneralName *names)
    {
        int              length = 0;
        CERTGeneralName  *first;

        first = names;
        if (names != NULL) {
            do {
                length++;
                names = CERT_GetNextGeneralName(names);
            } while (names != first);
        }
        return length;
    }

}

//Methods from XSanExtension
::com::sun::star::uno::Sequence< com::sun::star::security::CertAltNameEntry > SAL_CALL SanExtensionImpl :: getAlternativeNames() throw( ::com::sun::star::uno::RuntimeException ){

    if (!m_Entries.hasElements())
    {
        SECItem item;

        item.type = siDERCertBuffer;
        item.data = (unsigned char*) m_xExtnValue.getArray();
        item.len = m_xExtnValue.getLength();

        PRArenaPool *arena;
        CERTGeneralName *nameList;
        arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);

        if (!arena)
            return m_Entries;

        nameList = CERT_DecodeAltNameExtension(arena, &item);

        CERTGeneralName* current = nameList;

        int size = GetNamesLength(nameList);
        CertAltNameEntry* arrCertAltNameEntry = new CertAltNameEntry[size];
        for(int i = 0; i < size ; i++){
            switch (current->type) {
                case certOtherName: {
                    arrCertAltNameEntry[i].Type = ExtAltNameType_OTHER_NAME;
                    ::com::sun::star::beans::PropertyValue otherNameProp;
                    otherNameProp.Name = ::rtl::OUString::createFromAscii(CERT_GetOidString(&current->name.OthName.oid));

                    Sequence< sal_Int8 > otherName( current->name.OthName.name.len ) ;
                    for( unsigned int r = 0; r < current->name.OthName.name.len ; r ++ )
                        otherName[r] = *( current->name.OthName.name.data + r ) ;

                    otherNameProp.Value <<= otherName;

                    arrCertAltNameEntry[i].Value <<= otherNameProp;
                    break;
                                    }
                case certRFC822Name:
                    arrCertAltNameEntry[i].Type = ExtAltNameType_RFC822_NAME;
                    arrCertAltNameEntry[i].Value <<= ::rtl::OUString((const sal_Char*)current->name.other.data, current->name.other.len, RTL_TEXTENCODING_ASCII_US);
                    break;
                case certDNSName:
                    arrCertAltNameEntry[i].Type = ExtAltNameType_DNS_NAME;
                    arrCertAltNameEntry[i].Value <<= ::rtl::OUString((const sal_Char*)current->name.other.data, current->name.other.len, RTL_TEXTENCODING_ASCII_US);
                    break;
                case certX400Address: {
                    // unsupported
                    arrCertAltNameEntry[i].Type = ExtAltNameType_X400_ADDRESS;
                    break;
                                      }
                case certDirectoryName: {
                    // unsupported
                    arrCertAltNameEntry[i].Type = ExtAltNameType_DIRECTORY_NAME;
                    break;
                                        }
                case certEDIPartyName:  {
                    // unsupported
                    arrCertAltNameEntry[i].Type = ExtAltNameType_EDI_PARTY_NAME;
                    break;
                                        }
                case certURI:
                    arrCertAltNameEntry[i].Type = ExtAltNameType_URL;
                    arrCertAltNameEntry[i].Value <<= ::rtl::OUString((const sal_Char*)current->name.other.data, current->name.other.len, RTL_TEXTENCODING_ASCII_US);
                    break;
                case certIPAddress: {
                    arrCertAltNameEntry[i].Type = ExtAltNameType_IP_ADDRESS;

                    Sequence< sal_Int8 > ipAddress( current->name.other.len ) ;
                    for( unsigned int r = 0; r < current->name.other.len ; r ++ )
                        ipAddress[r] = *( current->name.other.data + r ) ;

                    arrCertAltNameEntry[i].Value <<= ipAddress;
                    break;
                                    }
                case certRegisterID:
                    arrCertAltNameEntry[i].Type = ExtAltNameType_REGISTERED_ID;


                    rtl::OString nssOid = ::rtl::OString(CERT_GetOidString(&current->name.other));
                    rtl::OString unoOid = removeOIDFromString(nssOid);
                    arrCertAltNameEntry[i].Value <<= rtl::OStringToOUString( unoOid, RTL_TEXTENCODING_ASCII_US );
                    break;
            }
            current = CERT_GetNextGeneralName(current);
        }

        m_Entries = ::comphelper::arrayToSequence< com::sun::star::security::CertAltNameEntry >(arrCertAltNameEntry, size);

        delete [] arrCertAltNameEntry;

        PORT_FreeArena(arena, PR_FALSE);


    }

    return m_Entries;
}

::rtl::OString SanExtensionImpl :: removeOIDFromString( const ::rtl::OString &oidString)
{
    ::rtl::OString objID;
    ::rtl::OString oid("OID.");
    if (oidString.match(oid))
        objID = oidString.copy(oid.getLength());
    else
        objID = oidString;
    return objID;

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
