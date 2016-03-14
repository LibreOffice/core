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

#ifndef INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_SIGNATUREENGINE_HXX
#define INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_SIGNATUREENGINE_HXX

#include <com/sun/star/xml/crypto/sax/XReferenceResolvedListener.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XKeyCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XMissionTaker.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeper.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>
#include <com/sun/star/xml/crypto/XUriBinding.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>

#include "securityengine.hxx"

#include <vector>

class SignatureEngine : public cppu::ImplInheritanceHelper
<
    SecurityEngine,
    css::xml::crypto::sax::XReferenceCollector,
    css::xml::crypto::XUriBinding
>
/****** signatureengine.hxx/CLASS SignatureEngine *****************************
 *
 *   NAME
 *  SignatureEngine -- Base class of SignatureCreator and SignatureVerifier
 *
 *   FUNCTION
 *  Maintains common members and methods related with signature operation.
 ******************************************************************************/
{
private:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

protected:

    /*
     * the Signature bridge component, which performs signature generation
     * and verification based on xmlsec library.
     */
    css::uno::Reference< css::xml::crypto::XXMLSignature > m_xXMLSignature;

    /*
     * a collection of ElementCollector's ids. Each ElementCollector
     * represents one element signed by this signature.
     */
    std::vector< sal_Int32 > m_vReferenceIds;

    /*
     * remembers how many references this signature has.
     */
    sal_Int32 m_nTotalReferenceNumber;

    /*
     * a collection of Uri binding.
     *
     * the m_vUris is used to hold the Uri strings, and the m_vXInputStreams is used
     * to hold corresponding binded XInputStream interface.
     */
    std::vector< OUString > m_vUris;
    std::vector< css::uno::Reference< css::io::XInputStream > > m_vXInputStreams;

protected:
    explicit SignatureEngine( const css::uno::Reference< css::uno::XComponentContext > & xContext);
    virtual ~SignatureEngine() {};

    virtual void tryToPerform( )
        throw (css::uno::Exception, css::uno::RuntimeException) override;
    virtual void clearUp( ) const override;
    virtual bool checkReady() const override;

    /*
     * starts the main function. This method will be implemented by any sub-class.
     * For a SignatureCreator, it performs signing operation;
     * for a SignatureVerifier, verification operation is performed.
     */
    virtual void startEngine( const css::uno::Reference<
                              css::xml::crypto::XXMLSignatureTemplate >&)
        throw (css::uno::Exception, css::uno::RuntimeException)
        {};

public:
    /* XReferenceCollector */
    virtual void SAL_CALL setReferenceCount( sal_Int32 count )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setReferenceId( sal_Int32 id )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    /* XUriBinding */
    virtual void SAL_CALL setUriBinding(
        const OUString& uri,
        const css::uno::Reference< css::io::XInputStream >& aInputStream )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::io::XInputStream >
        SAL_CALL getUriBinding( const OUString& uri )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
