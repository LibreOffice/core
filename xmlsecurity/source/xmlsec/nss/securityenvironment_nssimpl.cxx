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

#include "nssrenam.h"
#include <nss.h>
#include <cert.h>
#include <pk11pub.h>
#include <secerr.h>
#include <ocsp.h>

#include <sal/config.h>
#include <osl/diagnose.h>
#include "securityenvironment_nssimpl.hxx"
#include <cppuhelper/supportsservice.hxx>

#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/docpasswordrequest.hxx>
#include <sal/log.hxx>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <vector>
#include <memory>
#include <osl/thread.h>
#include <comphelper/sequence.hxx>

#include "x509certificate_nssimpl.hxx"
#include "secerror.hxx"
#include <prerror.h>
#include <keyhi.h>
#include <xmlsec/base64.h>
#include <xmlsec/keysmngr.h>
#include <xmlsec/nss/app.h>
#include <xmlsec/nss/pkikeys.h>

// added for password exception
#include <com/sun/star/security/NoPasswordException.hpp>
#include <com/sun/star/security/CertificateCharacters.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/xml/crypto/NSSInitializer.hpp>

namespace csss = ::com::sun::star::security;
using namespace ::com::sun::star::security;
using namespace com::sun::star;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;

using ::com::sun::star::security::XCertificate ;

namespace std
{
template <> struct default_delete<PRArenaPool>
{
    void operator()(PRArenaPool* ptr) { PORT_FreeArena(ptr, PR_FALSE); }
};
}

static rtl::Reference<X509Certificate_NssImpl> NssCertToXCert( CERTCertificate* cert ) ;
static rtl::Reference<X509Certificate_NssImpl> NssPrivKeyToXCert( SECKEYPrivateKey* ) ;

namespace {

struct UsageDescription
{
    SECCertificateUsage usage;
    char const* description;

    UsageDescription()
    : usage( certificateUsageCheckAllUsages )
    , description( nullptr )
    {}

    UsageDescription( SECCertificateUsage i_usage, char const* i_description )
    : usage( i_usage )
    , description( i_description )
    {}
};

}

static char* GetPasswordFunction( PK11SlotInfo* pSlot, PRBool bRetry, void* /*arg*/ )
{
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference < task::XInteractionHandler2 > xInteractionHandler(
        task::InteractionHandler::createWithParent(xContext, nullptr) );

    task::PasswordRequestMode eMode = bRetry ? task::PasswordRequestMode_PASSWORD_REENTER : task::PasswordRequestMode_PASSWORD_ENTER;
    OUString passwordLabel = xml::crypto::NSSInitializer::create(xContext)->getNSSPath();
    if (!passwordLabel.isEmpty())
        passwordLabel = ": " + passwordLabel;
    passwordLabel = OUString::createFromAscii(PK11_GetTokenName(pSlot)) + passwordLabel;
    rtl::Reference<::comphelper::DocPasswordRequest> pPasswordRequest = new ::comphelper::DocPasswordRequest(
        ::comphelper::DocPasswordRequestType::Standard, eMode, passwordLabel);

    xInteractionHandler->handle( pPasswordRequest );

    if ( pPasswordRequest->isPassword() )
    {
        OString aPassword(OUStringToOString(
            pPasswordRequest->getPassword(),
            osl_getThreadTextEncoding()));
        sal_Int32 nLen = aPassword.getLength();
        char* pPassword = static_cast<char*>(PORT_Alloc( nLen+1 ) );
        pPassword[nLen] = 0;
        memcpy( pPassword, aPassword.getStr(), nLen );
        return pPassword;
    }
    return nullptr;
}

SecurityEnvironment_NssImpl::SecurityEnvironment_NssImpl() :
m_pHandler( nullptr ) {
    PK11_SetPasswordFunc( GetPasswordFunction ) ;
}

SecurityEnvironment_NssImpl::~SecurityEnvironment_NssImpl() {

    PK11_SetPasswordFunc( nullptr ) ;

    for (auto& slot : m_Slots)
    {
        PK11_FreeSlot(slot);
    }

    for( auto& symKey : m_tSymKeyList )
        PK11_FreeSymKey( symKey ) ;
}

/* XServiceInfo */
OUString SAL_CALL SecurityEnvironment_NssImpl::getImplementationName() {
    return u"com.sun.star.xml.crypto.SecurityEnvironment"_ustr;
}

/* XServiceInfo */
sal_Bool SAL_CALL SecurityEnvironment_NssImpl::supportsService( const OUString& serviceName) {
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL SecurityEnvironment_NssImpl::getSupportedServiceNames() {
    Sequence<OUString> seqServiceNames{ u"com.sun.star.xml.crypto.SecurityEnvironment"_ustr };
    return seqServiceNames;
}

OUString SecurityEnvironment_NssImpl::getSecurityEnvironmentInformation()
{
    OUStringBuffer buff;
    for (auto& slot : m_Slots)
    {
        buff.appendAscii(PK11_GetTokenName(slot));
        buff.append("\n");
    }
    return buff.makeStringAndClear();
}

void SecurityEnvironment_NssImpl::addCryptoSlot( PK11SlotInfo* aSlot)
{
    PK11_ReferenceSlot(aSlot);
    m_Slots.push_back(aSlot);
}

//Could we have multiple cert dbs?
void SecurityEnvironment_NssImpl::setCertDb( CERTCertDBHandle* aCertDb ) {
    m_pHandler = aCertDb ;
}

void SecurityEnvironment_NssImpl::adoptSymKey( PK11SymKey* aSymKey ) {
    if( aSymKey == nullptr )        return;

    //First try to find the key in the list
    if (std::find(m_tSymKeyList.begin(), m_tSymKeyList.end(), aSymKey) != m_tSymKeyList.end())
        return;

    //If we do not find the key in the list, add a new node
    PK11SymKey* symkey = PK11_ReferenceSymKey( aSymKey ) ;
    if( symkey == nullptr )
        throw RuntimeException() ;

    try {
        m_tSymKeyList.push_back( symkey ) ;
    } catch ( Exception& ) {
        PK11_FreeSymKey( symkey ) ;
    }
}

void SecurityEnvironment_NssImpl::updateSlots()
{
    //In case new tokens are present then we can obtain the corresponding slot
    std::scoped_lock guard(m_mutex);

    m_Slots.clear();
    m_tSymKeyList.clear();

    PK11SlotList * slotList = PK11_GetAllTokens( CKM_INVALID_MECHANISM, PR_FALSE, PR_FALSE, nullptr ) ;
    if( slotList == nullptr )
        return;

    for (PK11SlotListElement* slotEle = slotList->head ; slotEle != nullptr; slotEle = slotEle->next)
    {
        PK11SlotInfo * pSlot = slotEle->slot ;

        if(pSlot != nullptr)
        {
            SAL_INFO(
                "xmlsecurity.xmlsec",
                "Found a slot: SlotName=" << PK11_GetSlotName(pSlot)
                    << ", TokenName=" << PK11_GetTokenName(pSlot));

//The following code which is commented out checks if a slot, that is a smart card for example, is
//              able to generate a symmetric key of type CKM_DES3_CBC. If this fails then this token
//              will not be used. This key is possibly used for the encryption service. However, all
//              interfaces and services used for public key signature and encryption are not published
//              and the encryption is not used in OOo. Therefore it does not do any harm to remove
//              this code, hence allowing smart cards which cannot generate this type of key.
//
//              By doing this, the encryption may fail if a smart card is being used which does not
//              support this key generation.
//
            PK11SymKey * pSymKey = PK11_KeyGen( pSlot , CKM_DES3_CBC, nullptr, 128, nullptr ) ;
//              if( pSymKey == NULL )
//              {
//                  PK11_FreeSlot( pSlot ) ;
//                  SAL_INFO( "xmlsecurity", "XMLSEC: Error - pSymKey is NULL" );
//                  continue;
//              }
            addCryptoSlot(pSlot);

            if (pSymKey != nullptr)
            {
                adoptSymKey( pSymKey ) ;
                PK11_FreeSymKey( pSymKey ) ;
                pSymKey = nullptr;
            }

        }// end of if(pSlot != NULL)
    }// end of for

    PK11_FreeSlotList(slotList);
}

Sequence< Reference < XCertificate > >
SecurityEnvironment_NssImpl::getPersonalCertificates()
{
    std::vector< rtl::Reference<X509Certificate_NssImpl> > certsList ;

    updateSlots();
    //firstly, we try to find private keys in slot
    for (auto& slot : m_Slots)
    {
        SECKEYPrivateKeyList* priKeyList ;

        if( PK11_NeedLogin(slot ) ) {
            SECStatus nRet = PK11_Authenticate(slot, PR_TRUE, nullptr);
            //PK11_Authenticate may fail in case the a slot has not been initialized.
            //this is the case if the user has a new profile, so that they have never
            //added a personal certificate.
            if( nRet != SECSuccess && PORT_GetError() != SEC_ERROR_IO) {
                throw NoPasswordException();
            }
        }

        priKeyList = PK11_ListPrivateKeysInSlot(slot) ;
        if( priKeyList != nullptr )
        {
            for (SECKEYPrivateKeyListNode* curPri = PRIVKEY_LIST_HEAD(priKeyList);
                !PRIVKEY_LIST_END( curPri, priKeyList ) && curPri != nullptr;
                curPri = PRIVKEY_LIST_NEXT(curPri))
            {
                rtl::Reference<X509Certificate_NssImpl> xcert = NssPrivKeyToXCert( curPri->key ) ;
                if( xcert != nullptr )
                    certsList.push_back( xcert ) ;
            }
            SECKEY_DestroyPrivateKeyList( priKeyList ) ;
        }


    }

    if( !certsList.empty() ) {
        return comphelper::containerToSequence<Reference< XCertificate >>(certsList) ;
    }

    return Sequence< Reference < XCertificate > > ();
}

Reference< XCertificate > SecurityEnvironment_NssImpl::getCertificate( const OUString& issuerName, const Sequence< sal_Int8 >& serialNumber )
{
    if( !m_pHandler )
        return nullptr;

    rtl::Reference<X509Certificate_NssImpl> xcert;
    CERTIssuerAndSN issuerAndSN ;
    CERTCertificate* cert ;
    CERTName* nmIssuer ;
    char* chIssuer ;
    SECItem* derIssuer ;
    std::unique_ptr<PRArenaPool> arena(PORT_NewArena(DER_DEFAULT_CHUNKSIZE));
    if( arena == nullptr )
        throw RuntimeException() ;

    // Create cert info from issue and serial
    OString ostr = OUStringToOString( issuerName , RTL_TEXTENCODING_UTF8 ) ;
    chIssuer = PL_strndup( ostr.getStr(), static_cast<int>(ostr.getLength()) ) ;
    nmIssuer = CERT_AsciiToName( chIssuer ) ;
    if( nmIssuer == nullptr ) {
        PL_strfree( chIssuer ) ;
        return nullptr; // no need for exception cf. i40394
    }

    derIssuer = SEC_ASN1EncodeItem( arena.get(), nullptr, static_cast<void*>(nmIssuer), SEC_ASN1_GET( CERT_NameTemplate ) ) ;
    if( derIssuer == nullptr ) {
        PL_strfree( chIssuer ) ;
        CERT_DestroyName( nmIssuer ) ;
        throw RuntimeException() ;
    }

    memset( &issuerAndSN, 0, sizeof( issuerAndSN ) ) ;

    issuerAndSN.derIssuer.data = derIssuer->data ;
    issuerAndSN.derIssuer.len = derIssuer->len ;

    issuerAndSN.serialNumber.data = reinterpret_cast<unsigned char *>(const_cast<sal_Int8 *>(serialNumber.getConstArray()));
    issuerAndSN.serialNumber.len = serialNumber.getLength() ;

    cert = CERT_FindCertByIssuerAndSN( m_pHandler, &issuerAndSN ) ;
    if( cert != nullptr ) {
        xcert = NssCertToXCert( cert ) ;
    }

    PL_strfree( chIssuer ) ;
    CERT_DestroyName( nmIssuer ) ;
    //SECITEM_FreeItem( derIssuer, PR_FALSE ) ;
    CERT_DestroyCertificate( cert ) ;

    return xcert ;
}

Sequence< Reference < XCertificate > > SecurityEnvironment_NssImpl::buildCertificatePath( const Reference< XCertificate >& begin ) {

    X509Certificate_NssImpl* xcert = dynamic_cast<X509Certificate_NssImpl*>(begin.get());
    if( xcert == nullptr ) {
        throw RuntimeException() ;
    }

    // Remember the signing certificate.
    m_xSigningCertificate = xcert;

    const CERTCertificate* cert = xcert->getNssCert() ;
    if (!cert)
        return {};

    //Get the system clock time
    int64 timeboundary = PR_Now() ;
    CERTCertList* certChain = CERT_GetCertChainFromCert( const_cast<CERTCertificate*>(cert), timeboundary, certUsageAnyCA ) ;

    if( !certChain )
        return {};

    std::vector<uno::Reference<security::XCertificate>> aCertChain;

    for (CERTCertListNode* node = CERT_LIST_HEAD(certChain); !CERT_LIST_END(node, certChain); node = CERT_LIST_NEXT(node)) {
        rtl::Reference<X509Certificate_NssImpl> pCert = new X509Certificate_NssImpl();
        if( pCert == nullptr ) {
            CERT_DestroyCertList( certChain ) ;
            throw RuntimeException() ;
        }

        pCert->setCert( node->cert ) ;

        aCertChain.push_back(pCert);
    }

    CERT_DestroyCertList( certChain ) ;

    return comphelper::containerToSequence(aCertChain);
}

rtl::Reference<X509Certificate_NssImpl> SecurityEnvironment_NssImpl::createAndAddCertificateFromPackage(
                                                        const css::uno::Sequence<sal_Int8>& raDERCertificate,
                                                        std::u16string_view raString)
{
    auto pCertificateBytes = reinterpret_cast<char *>(const_cast<sal_Int8 *>(raDERCertificate.getConstArray()));
    CERTCertificate* pCERTCertificate = CERT_DecodeCertFromPackage(pCertificateBytes, raDERCertificate.getLength());

    if (!pCERTCertificate)
        return nullptr;

    SECStatus aStatus;

    OString aTrustString = OUStringToOString(raString, RTL_TEXTENCODING_ASCII_US);
    CERTCertTrust aTrust;

    aStatus = CERT_DecodeTrustString(&aTrust, aTrustString.getStr());

    if (aStatus != SECSuccess)
    {
        PRIntn err = PR_GetError();
        SAL_WARN("xmlsecurity.xmlsec", "Error: " <<  err << ": " << getCertError(err));
        return nullptr;
    }

    PK11SlotInfo* pSlot = PK11_GetInternalKeySlot();

    if (!pSlot)
        return nullptr;

    aStatus = PK11_ImportCert(pSlot, pCERTCertificate, CK_INVALID_HANDLE, nullptr, PR_FALSE);

    if (aStatus != SECSuccess)
    {
        PRIntn err = PR_GetError();
        SAL_WARN("xmlsecurity.xmlsec", "Error: " <<  err << ": " << getCertError(err));
        return nullptr;
    }

    aStatus = CERT_ChangeCertTrust(CERT_GetDefaultCertDB(), pCERTCertificate, &aTrust);

    if (aStatus != SECSuccess)
    {
        PRIntn err = PR_GetError();
        SAL_WARN("xmlsecurity.xmlsec", "Error: " <<  err << ": " << getCertError(err));
        return nullptr;
    }

    PK11_FreeSlot(pSlot);

    rtl::Reference<X509Certificate_NssImpl> pX509Certificate = new X509Certificate_NssImpl();
    pX509Certificate->setCert(pCERTCertificate);
    return pX509Certificate;
}

rtl::Reference<X509Certificate_NssImpl> SecurityEnvironment_NssImpl::createX509CertificateFromDER(const css::uno::Sequence<sal_Int8>& aDerCertificate)
{
    rtl::Reference<X509Certificate_NssImpl> pX509Certificate;

    if (aDerCertificate.hasElements())
    {
        pX509Certificate = new X509Certificate_NssImpl();
        if (pX509Certificate == nullptr)
            throw RuntimeException();
        pX509Certificate->setRawCert(aDerCertificate);
    }
    return pX509Certificate;
}

Reference<XCertificate> SecurityEnvironment_NssImpl::createCertificateFromRaw(const Sequence< sal_Int8 >& rawCertificate)
{
    return createX509CertificateFromDER(rawCertificate);
}

Reference< XCertificate > SecurityEnvironment_NssImpl::createCertificateFromAscii( const OUString& asciiCertificate )
{
    OString oscert = OUStringToOString( asciiCertificate , RTL_TEXTENCODING_ASCII_US ) ;
    xmlChar* chCert = xmlStrndup( reinterpret_cast<const xmlChar*>(oscert.getStr()), static_cast<int>(oscert.getLength()) ) ;
    xmlSecSize certSize;
    int nRet = xmlSecBase64Decode_ex( chCert, reinterpret_cast<xmlSecByte*>(chCert), xmlStrlen( chCert ), &certSize ) ;
    if (nRet < 0 || certSize == 0)
    {
        xmlFree(chCert);
        return nullptr;
    }

    Sequence< sal_Int8 > rawCert(comphelper::arrayToSequence<sal_Int8>(chCert, certSize)) ;

    xmlFree( chCert ) ;

    return createCertificateFromRaw( rawCert ) ;
}

sal_Int32 SecurityEnvironment_NssImpl ::
verifyCertificate( const Reference< csss::XCertificate >& aCert,
                   const Sequence< Reference< csss::XCertificate > >&  intermediateCerts )
{
    sal_Int32 validity = csss::CertificateValidity::INVALID;
    const CERTCertificate* cert ;

    SAL_INFO("xmlsecurity.xmlsec", "Start verification of certificate: " << aCert->getSubjectName());

    const X509Certificate_NssImpl* xcert = dynamic_cast<X509Certificate_NssImpl*>(aCert.get());
    if( xcert == nullptr ) {
        throw RuntimeException() ;
    }

    //CERT_PKIXVerifyCert does not take a db as argument. It will therefore
    //internally use CERT_GetDefaultCertDB
    //Make sure m_pHandler is the default DB
    OSL_ASSERT(m_pHandler == CERT_GetDefaultCertDB());
    CERTCertDBHandle * certDb = m_pHandler != nullptr ? m_pHandler : CERT_GetDefaultCertDB();
    cert = xcert->getNssCert() ;
    if( !cert )
        return css::security::CertificateValidity::INVALID;


    ::std::vector<CERTCertificate*> vecTmpNSSCertificates;

    //prepare the intermediate certificates
    for (const auto& rIntermediateCert : intermediateCerts)
    {
        Sequence<sal_Int8> der = rIntermediateCert->getEncoded();
        SECItem item;
        item.type = siBuffer;
        item.data = reinterpret_cast<unsigned char*>(der.getArray());
        item.len = der.getLength();

        CERTCertificate* certTmp = CERT_NewTempCertificate(certDb, &item,
                                       nullptr     /* nickname */,
                                       PR_FALSE /* isPerm */,
                                       PR_TRUE  /* copyDER */);
        if (!certTmp)
        {
             SAL_INFO("xmlsecurity.xmlsec", "Failed to add a temporary certificate: " << rIntermediateCert->getIssuerName());

        }
        else
        {
             SAL_INFO("xmlsecurity.xmlsec", "Added temporary certificate: " <<
                      (certTmp->subjectName ? certTmp->subjectName : ""));
             vecTmpNSSCertificates.push_back(certTmp);
        }
    }


    SECStatus status ;

    CERTVerifyLog log;
    log.arena = PORT_NewArena(512);
    log.head = log.tail = nullptr;
    log.count = 0;

    CERT_EnableOCSPChecking(certDb);
    CERT_DisableOCSPDefaultResponder(certDb);
    CERTValOutParam cvout[5];
    CERTValInParam cvin[3];
    int ncvinCount=0;

#if ( NSS_VMAJOR > 3 ) || ( NSS_VMAJOR == 3 && NSS_VMINOR > 12 ) || ( NSS_VMAJOR == 3 && NSS_VMINOR == 12 && NSS_VPATCH > 0 )
    cvin[ncvinCount].type = cert_pi_useAIACertFetch;
    cvin[ncvinCount].value.scalar.b = PR_TRUE;
    ncvinCount++;
#endif

    PRUint64 revFlagsLeaf[2];
    PRUint64 revFlagsChain[2];
    CERTRevocationFlags rev;
    rev.leafTests.number_of_defined_methods = 2;
    rev.leafTests.cert_rev_flags_per_method = revFlagsLeaf;
    //the flags are defined in cert.h
    //We check both leaf and chain.
    //It is enough if one revocation method has fresh info,
    //but at least one must have some. Otherwise validation fails.
    //!!! using leaf test and CERT_REV_MI_REQUIRE_SOME_FRESH_INFO_AVAILABLE
    // when validating a root certificate will result in "revoked". Usually
    //there is no revocation information available for the root cert because
    //it must be trusted anyway and it does itself issue revocation information.
    //When we use the flag here and OOo shows the certification path then the root
    //cert is invalid while all other can be valid. It would probably best if
    //this interface method returned the whole chain.
    //Otherwise we need to check if the certificate is self-signed and if it is
    //then not use the flag when doing the leaf-test.
    rev.leafTests.cert_rev_flags_per_method[cert_revocation_method_crl] =
        CERT_REV_M_TEST_USING_THIS_METHOD
        | CERT_REV_M_IGNORE_IMPLICIT_DEFAULT_SOURCE;
    rev.leafTests.cert_rev_flags_per_method[cert_revocation_method_ocsp] =
        CERT_REV_M_TEST_USING_THIS_METHOD
        | CERT_REV_M_IGNORE_IMPLICIT_DEFAULT_SOURCE;
    rev.leafTests.number_of_preferred_methods = 0;
    rev.leafTests.preferred_methods = nullptr;
    rev.leafTests.cert_rev_method_independent_flags =
        CERT_REV_MI_TEST_ALL_LOCAL_INFORMATION_FIRST;

    rev.chainTests.number_of_defined_methods = 2;
    rev.chainTests.cert_rev_flags_per_method = revFlagsChain;
    rev.chainTests.cert_rev_flags_per_method[cert_revocation_method_crl] =
        CERT_REV_M_TEST_USING_THIS_METHOD
        | CERT_REV_M_IGNORE_IMPLICIT_DEFAULT_SOURCE;
    rev.chainTests.cert_rev_flags_per_method[cert_revocation_method_ocsp] =
        CERT_REV_M_TEST_USING_THIS_METHOD
        | CERT_REV_M_IGNORE_IMPLICIT_DEFAULT_SOURCE;
    rev.chainTests.number_of_preferred_methods = 0;
    rev.chainTests.preferred_methods = nullptr;
    rev.chainTests.cert_rev_method_independent_flags =
        CERT_REV_MI_TEST_ALL_LOCAL_INFORMATION_FIRST;


    cvin[ncvinCount].type = cert_pi_revocationFlags;
    cvin[ncvinCount].value.pointer.revocation = &rev;
    ncvinCount++;
    // does not work, not implemented yet in 3.12.4
//         cvin[ncvinCount].type = cert_pi_keyusage;
//         cvin[ncvinCount].value.scalar.ui = KU_DIGITAL_SIGNATURE;
//         ncvinCount++;
    cvin[ncvinCount].type = cert_pi_end;

    cvout[0].type = cert_po_trustAnchor;
    cvout[0].value.pointer.cert = nullptr;
    cvout[1].type = cert_po_errorLog;
    cvout[1].value.pointer.log = &log;
    cvout[2].type = cert_po_end;

    // We check SSL server certificates, CA certificates and signing certificates.
    //
    // ToDo check keyusage, looking at CERT_KeyUsageAndTypeForCertUsage (
    // mozilla/security/nss/lib/certdb/certdb.c indicates that
    // certificateUsageSSLClient, certificateUsageSSLServer and certificateUsageSSLCA
    // are sufficient. They cover the key usages for digital signature, key agreement
    // and encipherment and certificate signature

    //never use the following usages because they are not checked properly
    // certificateUsageUserCertImport
    // certificateUsageVerifyCA
    // certificateUsageAnyCA
    // certificateUsageProtectedObjectSigner

    UsageDescription arUsages[5];
    arUsages[0] = UsageDescription( certificateUsageSSLClient, "certificateUsageSSLClient"  );
    arUsages[1] = UsageDescription( certificateUsageSSLServer, "certificateUsageSSLServer"  );
    arUsages[2] = UsageDescription( certificateUsageSSLCA, "certificateUsageSSLCA"  );
    arUsages[3] = UsageDescription( certificateUsageEmailSigner, "certificateUsageEmailSigner" );
    arUsages[4] = UsageDescription( certificateUsageEmailRecipient, "certificateUsageEmailRecipient" );

    int numUsages = std::size(arUsages);
    for (int i = 0; i < numUsages; i++)
    {
        SAL_INFO("xmlsecurity.xmlsec", "Testing usage " << i+1 <<
                 " of " << numUsages << ": " <<
                 arUsages[i].description <<
                 " (0x" << std::hex << static_cast<int>(arUsages[i].usage) << ")" << std::dec);

        status = CERT_PKIXVerifyCert(const_cast<CERTCertificate *>(cert), arUsages[i].usage,
                                     cvin, cvout, nullptr);
        if( status == SECSuccess )
        {
            SAL_INFO("xmlsecurity.xmlsec", "CERT_PKIXVerifyCert returned SECSuccess.");
            //When an intermediate or root certificate is checked then we expect the usage
            //certificateUsageSSLCA. This, however, will be only set when in the trust settings dialog
            //the button "This certificate can identify websites" is checked. If for example only
            //"This certificate can identify mail users" is set then the end certificate can
            //be validated and the returned usage will contain certificateUsageEmailRecipient.
            //But checking directly the root or intermediate certificate will fail. In the
            //certificate path view the end certificate will be shown as valid but the others
            //will be displayed as invalid.

            validity = csss::CertificateValidity::VALID;
            SAL_INFO("xmlsecurity.xmlsec", "Certificate is valid.");
            CERTCertificate * issuerCert = cvout[0].value.pointer.cert;
            if (issuerCert)
            {
                SAL_INFO("xmlsecurity.xmlsec", "Root certificate: " << issuerCert->subjectName);
                CERT_DestroyCertificate(issuerCert);
            };

            break;
        }
        else
        {
            PRIntn err = PR_GetError();
            SAL_INFO("xmlsecurity.xmlsec", "Error: " <<  err << ": " << getCertError(err));

            /* Display validation results */
            if ( log.count > 0)
            {
                CERTVerifyLogNode *node = nullptr;
                printChainFailure(&log);

                for (node = log.head; node; node = node->next) {
                    if (node->cert)
                        CERT_DestroyCertificate(node->cert);
                }
                log.head = log.tail = nullptr;
                log.count = 0;
            }
            SAL_INFO("xmlsecurity.xmlsec", "Certificate is invalid.");
        }
    }

    //Destroying the temporary certificates
    for (auto& tmpCert : vecTmpNSSCertificates)
    {
        SAL_INFO("xmlsecurity.xmlsec", "Destroying temporary certificate");
        CERT_DestroyCertificate(tmpCert);
    }
    PORT_FreeArena(log.arena, true);
    return validity ;
}

sal_Int32 SecurityEnvironment_NssImpl::getCertificateCharacters(
    const css::uno::Reference< css::security::XCertificate >& aCert ) {
    sal_Int32 characters ;
    const CERTCertificate* cert ;

    const X509Certificate_NssImpl* xcert = dynamic_cast<X509Certificate_NssImpl*>(aCert.get());
    if( xcert == nullptr ) {
        throw RuntimeException() ;
    }

    cert = xcert->getNssCert() ;

    characters = 0x00000000 ;

    //Firstly, find out whether or not the cert is self-signed.
    if( SECITEM_CompareItem( &(cert->derIssuer), &(cert->derSubject) ) == SECEqual ) {
        characters |= css::security::CertificateCharacters::SELF_SIGNED ;
    } else {
        characters &= ~ css::security::CertificateCharacters::SELF_SIGNED ;
    }

    //Secondly, find out whether or not the cert has a private key.

    /*
     * i40394
     *
     * mmi : need to check whether the cert's slot is valid first
     */
    SECKEYPrivateKey* priKey = nullptr;

    if (cert->slot != nullptr)
    {
        priKey = PK11_FindPrivateKeyFromCert( cert->slot, const_cast<CERTCertificate*>(cert), nullptr ) ;
    }
    if(priKey == nullptr)
    {
        for (auto& slot : m_Slots)
        {
            priKey = PK11_FindPrivateKeyFromCert(slot, const_cast<CERTCertificate*>(cert), nullptr);
            if (priKey)
                break;
        }
    }
    if( priKey != nullptr ) {
        characters |=  css::security::CertificateCharacters::HAS_PRIVATE_KEY ;

        SECKEY_DestroyPrivateKey( priKey ) ;
    } else {
        characters &= ~ css::security::CertificateCharacters::HAS_PRIVATE_KEY ;
    }

    return characters ;
}

rtl::Reference<X509Certificate_NssImpl> NssCertToXCert( CERTCertificate* cert )
{
    if( cert != nullptr ) {
        rtl::Reference<X509Certificate_NssImpl> xcert = new X509Certificate_NssImpl() ;
        xcert->setCert( cert ) ;
        return xcert;
    }

    return nullptr;
}

rtl::Reference<X509Certificate_NssImpl> NssPrivKeyToXCert( SECKEYPrivateKey* priKey )
{
    if( priKey != nullptr ) {
        rtl::Reference<X509Certificate_NssImpl> xcert;
        CERTCertificate* cert = PK11_GetCertFromPrivateKey( priKey ) ;

        if( cert != nullptr ) {
            xcert = NssCertToXCert( cert ) ;
        }

        CERT_DestroyCertificate( cert ) ;
        return xcert;
    }

    return nullptr;
}

xmlSecKeysMngrPtr SecurityEnvironment_NssImpl::createKeysManager() {

    /*-
     * The following lines is based on the private version of xmlSec-NSS
     * crypto engine
     */
    int cSlots = m_Slots.size();
    std::unique_ptr<PK11SlotInfo*[]> sarSlots(new PK11SlotInfo*[cSlots]);
    PK11SlotInfo**  slots = sarSlots.get();
    int count = 0;
    for (const auto& slot : m_Slots)
    {
        slots[count] = slot;
        ++count;
    }

    xmlSecKeysMngrPtr pKeysMngr = xmlSecKeysMngrCreate();
    if (!pKeysMngr)
        throw RuntimeException();

    if (xmlSecNssAppDefaultKeysMngrInit(pKeysMngr) < 0)
        throw RuntimeException();

    // Adopt the private key of the signing certificate, if it has any.
    if (m_xSigningCertificate)
    {
        SECKEYPrivateKey* pPrivateKey = SECKEY_CopyPrivateKey(m_xSigningCertificate->getPrivateKey());
        if (pPrivateKey)
        {
            xmlSecKeyDataPtr pKeyData = xmlSecNssPKIAdoptKey(pPrivateKey, nullptr);
            xmlSecKeyPtr pKey = xmlSecKeyCreate();
            xmlSecKeySetValue(pKey, pKeyData);
            xmlSecNssAppDefaultKeysMngrAdoptKey(pKeysMngr, pKey);
        }
        else
        {
            SAL_WARN("xmlsecurity.xmlsec", "Can't get the private key from the certificate.");
        }
    }

    return pKeysMngr;
}

void SecurityEnvironment_NssImpl::destroyKeysManager(xmlSecKeysMngrPtr pKeysMngr) {
    if( pKeysMngr != nullptr ) {
        xmlSecKeysMngrDestroy( pKeysMngr ) ;
    }
}

SECKEYPrivateKey* SecurityEnvironment_NssImpl::insertPrivateKey(css::uno::Sequence<sal_Int8> const & raPrivateKey)
{
    PK11SlotInfo* pSlot = PK11_GetInternalKeySlot();

    if (!pSlot)
        return nullptr;

    SECItem aDerPrivateKeyInfo;
    aDerPrivateKeyInfo.data = reinterpret_cast<unsigned char *>(const_cast<sal_Int8 *>(raPrivateKey.getConstArray()));
    aDerPrivateKeyInfo.len = raPrivateKey.getLength();

    const unsigned int aKeyUsage = KU_ALL;
    SECKEYPrivateKey* pPrivateKey = nullptr;

    bool bPermanent = PR_FALSE;
    bool bPrivate = PR_TRUE;

    SECStatus nStatus = PK11_ImportDERPrivateKeyInfoAndReturnKey(
          pSlot, &aDerPrivateKeyInfo, nullptr, nullptr, bPermanent, bPrivate,
          aKeyUsage, &pPrivateKey, nullptr);

    if (nStatus != SECSuccess)
        return nullptr;

    PK11_FreeSlot(pSlot);

    return pPrivateKey;
}

uno::Reference<security::XCertificate> SecurityEnvironment_NssImpl::createDERCertificateWithPrivateKey(
        Sequence<sal_Int8> const & raDERCertificate, Sequence<sal_Int8> const & raPrivateKey)
{
    SECKEYPrivateKey* pPrivateKey = insertPrivateKey(raPrivateKey);

    if (!pPrivateKey)
        return uno::Reference<security::XCertificate>();

    return createAndAddCertificateFromPackage(raDERCertificate, u"TCu,TCu,TCu");
}

uno::Reference<security::XCertificate> SecurityEnvironment_NssImpl::addDERCertificateToTheDatabase(
        uno::Sequence<sal_Int8> const & raDERCertificate, OUString const & raTrustString)
{
    return createAndAddCertificateFromPackage(raDERCertificate, raTrustString);
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_xml_crypto_SecurityEnvironment_get_implementation(
    uno::XComponentContext* /*pCtx*/, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SecurityEnvironment_NssImpl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
