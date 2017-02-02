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
using namespace css::security ;

CertificateImpl::CertificateImpl()
{
}

CertificateImpl::~CertificateImpl() {
}

//Methods from XCertificateImpl
sal_Int16 SAL_CALL CertificateImpl::getVersion()
    throw ( css::uno::RuntimeException, std::exception) {
}

css::uno::Sequence< sal_Int8 > SAL_CALL CertificateImpl::getSerialNumber()
    throw ( css::uno::RuntimeException, std::exception) {
}

OUString SAL_CALL CertificateImpl::getIssuerName()
    throw ( css::uno::RuntimeException, std::exception) {
}

OUString SAL_CALL CertificateImpl::getSubjectName()
    throw ( css::uno::RuntimeException, std::exception) {
}

css::util::DateTime SAL_CALL CertificateImpl::getNotValidBefore()
    throw ( css::uno::RuntimeException, std::exception) {
}

css::util::DateTime SAL_CALL CertificateImpl::getNotValidAfter()
    throw ( css::uno::RuntimeException, std::exception) {
}

css::uno::Sequence< sal_Int8 > SAL_CALL CertificateImpl::getIssuerUniqueID()
    throw ( css::uno::RuntimeException, std::exception) {
}

css::uno::Sequence< sal_Int8 > SAL_CALL CertificateImpl::getSubjectUniqueID()
    throw ( css::uno::RuntimeException, std::exception) {
}

css::uno::Sequence< css::uno::Reference< css::security::XCertificateExtension > > SAL_CALL CertificateImpl::getExtensions()
    throw ( css::uno::RuntimeException, std::exception) {
}

css::uno::Reference< css::security::XCertificateExtension > SAL_CALL CertificateImpl::findCertificateExtension( const css::uno::Sequence< sal_Int8 >& /*oid*/ )
    throw (css::uno::RuntimeException, std::exception)
{
}

css::uno::Sequence< sal_Int8 > SAL_CALL CertificateImpl::getEncoded()
    throw ( css::uno::RuntimeException, std::exception)
{
}

/* XUnoTunnel */
sal_Int64 SAL_CALL CertificateImpl::getSomething( const Sequence< sal_Int8 >& /*aIdentifier*/ )
    throw( RuntimeException, std::exception ) {
}

OUString SAL_CALL CertificateImpl::getSubjectPublicKeyAlgorithm()
    throw ( css::uno::RuntimeException, std::exception)
{
}

css::uno::Sequence< sal_Int8 > SAL_CALL CertificateImpl::getSubjectPublicKeyValue()
    throw ( css::uno::RuntimeException, std::exception)
{
}

OUString SAL_CALL CertificateImpl::getSignatureAlgorithm()
    throw ( css::uno::RuntimeException, std::exception)
{
}

css::uno::Sequence< sal_Int8 > SAL_CALL CertificateImpl::getSHA1Thumbprint()
    throw ( css::uno::RuntimeException, std::exception)
{
}

uno::Sequence<sal_Int8> CertificateImpl::getSHA256Thumbprint()
    throw (uno::RuntimeException, std::exception)
{
}

css::uno::Sequence< sal_Int8 > SAL_CALL CertificateImpl::getMD5Thumbprint()
    throw ( css::uno::RuntimeException, std::exception)
{
}

sal_Int32 SAL_CALL CertificateImpl::getCertificateUsage()
    throw ( css::uno::RuntimeException, std::exception)
{

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
