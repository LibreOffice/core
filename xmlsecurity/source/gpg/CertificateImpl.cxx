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
    throw ( RuntimeException, std::exception)
{
    return 0;
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getSerialNumber()
    throw ( RuntimeException, std::exception)
{
    return Sequence< sal_Int8 > ();
}

OUString SAL_CALL CertificateImpl::getIssuerName()
    throw ( RuntimeException, std::exception)
{
    return OStringToOUString(m_pKey.userID(0).name(), RTL_TEXTENCODING_UTF8);
}

OUString SAL_CALL CertificateImpl::getSubjectName()
    throw ( RuntimeException, std::exception)
{
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
    throw ( RuntimeException, std::exception)
{
    const GpgME::Subkey subkey = m_pKey.subkey(0);
    if (subkey.isNull())
        return DateTime();

    return convertUnixTimeToDateTime(m_pKey.subkey(0).creationTime());
}

DateTime SAL_CALL CertificateImpl::getNotValidAfter()
    throw ( RuntimeException, std::exception)
{
    const GpgME::Subkey subkey = m_pKey.subkey(0);
    if (subkey.isNull() || subkey.neverExpires())
        return DateTime();

    return convertUnixTimeToDateTime(m_pKey.subkey(0).expirationTime());
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getIssuerUniqueID()
    throw ( RuntimeException, std::exception)
{
    return Sequence< sal_Int8 > ();
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getSubjectUniqueID()
    throw ( RuntimeException, std::exception)
{
    return Sequence< sal_Int8 > ();
}

Sequence< Reference< XCertificateExtension > > SAL_CALL CertificateImpl::getExtensions()
    throw ( RuntimeException, std::exception)
{
    return Sequence< Reference< XCertificateExtension > > ();
}

Reference< XCertificateExtension > SAL_CALL CertificateImpl::findCertificateExtension( const Sequence< sal_Int8 >& /*oid*/ )
    throw (RuntimeException, std::exception)
{
    return Reference< XCertificateExtension > ();
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getEncoded()
    throw ( RuntimeException, std::exception)
{
    return Sequence< sal_Int8 > ();
}

OUString SAL_CALL CertificateImpl::getSubjectPublicKeyAlgorithm()
    throw ( RuntimeException, std::exception)
{
    return OUString();
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getSubjectPublicKeyValue()
    throw ( RuntimeException, std::exception)
{
    return Sequence< sal_Int8 > ();
}

OUString SAL_CALL CertificateImpl::getSignatureAlgorithm()
    throw ( RuntimeException, std::exception)
{
    return OUString();
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getSHA1Thumbprint()
    throw ( RuntimeException, std::exception)
{
    return Sequence< sal_Int8 > ();
}

uno::Sequence<sal_Int8> CertificateImpl::getSHA256Thumbprint()
    throw (uno::RuntimeException, std::exception)
{
    return Sequence< sal_Int8 > ();
}

Sequence< sal_Int8 > SAL_CALL CertificateImpl::getMD5Thumbprint()
    throw ( RuntimeException, std::exception)
{
    return Sequence< sal_Int8 > ();
}

sal_Int32 SAL_CALL CertificateImpl::getCertificateUsage()
    throw ( RuntimeException, std::exception)
{
    return 0;
}

/* XUnoTunnel */
sal_Int64 SAL_CALL CertificateImpl::getSomething(const Sequence< sal_Int8 >& aIdentifier)
    throw( RuntimeException, std::exception )
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
