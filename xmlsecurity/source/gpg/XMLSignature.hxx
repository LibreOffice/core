/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_GPG_XMLSIGNATURE_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_GPG_XMLSIGNATURE_HXX

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Exception.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>
#include <com/sun/star/xml/crypto/XXMLSignatureTemplate.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/xml/crypto/XMLSignatureException.hpp>

class XMLSignatureGpg : public cppu::WeakImplHelper< css::xml::crypto::XXMLSignature >
{
public:
    explicit XMLSignatureGpg();
    virtual ~XMLSignatureGpg() override;

    // XXMLSignature
    virtual css::uno::Reference< css::xml::crypto::XXMLSignatureTemplate > SAL_CALL generate(
        const css::uno::Reference< css::xml::crypto::XXMLSignatureTemplate >& aTemplate ,
        const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& aEnvironment
    ) override;

    virtual css::uno::Reference< css::xml::crypto::XXMLSignatureTemplate > SAL_CALL validate(
        const css::uno::Reference< css::xml::crypto::XXMLSignatureTemplate >& aTemplate ,
        const css::uno::Reference< css::xml::crypto::XXMLSecurityContext >& aContext
    ) override;
};

#endif // INCLUDED_XMLSECURITY_SOURCE_XMLSEC_GPG_XMLSIGNATURE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
