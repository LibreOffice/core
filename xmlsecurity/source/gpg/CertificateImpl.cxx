/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "CertificateImpl.hxx"

#include <comphelper/servicehelper.hxx>

#include <com/sun/star/security/KeyUsage.hpp>

using namespace css;
using namespace css::uno;
using namespace css::security;
using namespace css::util;

CertificateImpl::CertificateImpl() :
    m_pKey(nullptr)
{
}

CertificateImpl::~CertificateImpl()
{
}

//Methods from XCertificateImpl
sal_Int16 SAL_CALL CertificateImpl::getVersion()
{
    return 0;
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getSerialNumber()
{
    // Empty for gpg
    return Sequence< sal_Int8 > ();
}

OUString SAL_CALL CertificateImpl::getIssuerName()
{
    // Empty for gpg
    return OUString("");
}

OUString SAL_CALL CertificateImpl::getSubjectName()
{
    // Empty for gpg
    return OUString("");
}

namespace {
    DateTime convertUnixTimeToDateTime(time_t time)
    {
        DateTime dateTime;
        struct tm *timeStruct = gmtime(&time);
        dateTime.Year = timeStruct->tm_year + 1900;
        dateTime.Month = timeStruct->tm_mon + 1;
        dateTime.Day = timeStruct->tm_mday;
        dateTime.Hours = timeStruct->tm_hour;
        dateTime.Minutes = timeStruct->tm_min;
        dateTime.Seconds = timeStruct->tm_sec;
        return dateTime;
    }
}

DateTime SAL_CALL CertificateImpl::getNotValidBefore()
{
    const GpgME::Subkey subkey = m_pKey.subkey(0);
    if (subkey.isNull())
        return DateTime();

    return convertUnixTimeToDateTime(m_pKey.subkey(0).creationTime());
}

DateTime SAL_CALL CertificateImpl::getNotValidAfter()
{
    const GpgME::Subkey subkey = m_pKey.subkey(0);
    if (subkey.isNull() || subkey.neverExpires())
        return DateTime();

    return convertUnixTimeToDateTime(m_pKey.subkey(0).expirationTime());
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getIssuerUniqueID()
{
    // Empty for gpg
    return Sequence< sal_Int8 > ();
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getSubjectUniqueID()
{
    // Empty for gpg
    return Sequence< sal_Int8 > ();
}

Sequence< Reference< XCertificateExtension > > SAL_CALL CertificateImpl::getExtensions()
{
    // Empty for gpg
    return Sequence< Reference< XCertificateExtension > > ();
}

Reference< XCertificateExtension > SAL_CALL CertificateImpl::findCertificateExtension( const Sequence< sal_Int8 >& /*oid*/ )
{
    // Empty for gpg
    return Reference< XCertificateExtension > ();
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getEncoded()
{
    // Empty for gpg
    return Sequence< sal_Int8 > ();
}

OUString SAL_CALL CertificateImpl::getSubjectPublicKeyAlgorithm()
{
    const GpgME::Subkey subkey = m_pKey.subkey(0);
    if (subkey.isNull())
        return OUString();

    return OStringToOUString(subkey.publicKeyAlgorithmAsString(), RTL_TEXTENCODING_UTF8);
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getSubjectPublicKeyValue()
{
    return Sequence< sal_Int8 > ();
}

OUString SAL_CALL CertificateImpl::getSignatureAlgorithm()
{
    const GpgME::UserID userId = m_pKey.userID(0);
    if (userId.isNull())
        return OUString();

    const GpgME::UserID::Signature signature = userId.signature(0);
    if (signature.isNull())
        return OUString();

    return OStringToOUString(signature.algorithmAsString(), RTL_TEXTENCODING_UTF8);
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getSHA1Thumbprint()
{
    // Empty for gpg
    return Sequence< sal_Int8 > ();
}

uno::Sequence<sal_Int8> CertificateImpl::getSHA256Thumbprint()
{
    // Empty for gpg
    return Sequence< sal_Int8 > ();
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getMD5Thumbprint()
{
    // Empty for gpg
    return Sequence< sal_Int8 > ();
}

sal_Int32 SAL_CALL CertificateImpl::getCertificateUsage()
{
    return KeyUsage::DIGITAL_SIGNATURE | KeyUsage::NON_REPUDIATION  | KeyUsage::KEY_ENCIPHERMENT | KeyUsage::DATA_ENCIPHERMENT;
}

/* XUnoTunnel */
sal_Int64 SAL_CALL CertificateImpl::getSomething(const Sequence< sal_Int8 >& aIdentifier)
{
    if( aIdentifier.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0 ;
}

/* XUnoTunnel extension */

namespace
{
    class CertificateImplUnoTunnelId : public rtl::Static< UnoTunnelIdInit, CertificateImplUnoTunnelId > {};
}

const Sequence< sal_Int8>& CertificateImpl::getUnoTunnelId() {
    return CertificateImplUnoTunnelId::get().getSeq();
}

void CertificateImpl::setCertificate(GpgME::Key key)
{
    m_pKey = key;
}

const GpgME::Key* CertificateImpl::getCertificate() const
{
    return &m_pKey;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
