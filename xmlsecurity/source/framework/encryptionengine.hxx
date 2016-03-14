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

#ifndef INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_ENCRYPTIONENGINE_HXX
#define INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_ENCRYPTIONENGINE_HXX

#include <com/sun/star/xml/crypto/sax/XReferenceResolvedListener.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XBlockerMonitor.hpp>
#include <com/sun/star/xml/crypto/sax/XKeyCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XMissionTaker.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeper.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/xml/crypto/XXMLEncryption.hpp>
#include <cppuhelper/implbase.hxx>

#include "securityengine.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

class EncryptionEngine : public cppu::ImplInheritanceHelper
<
    SecurityEngine,
    css::xml::crypto::sax::XBlockerMonitor
>
/****** encryptionEngine.hxx/CLASS encryptionEngine ***************************
 *
 *   NAME
 *  EncryptionEngine -- Base class of Encryptor and Decryptor
 *
 *   FUNCTION
 *  Maintains common members and methods related with encryption.
 ******************************************************************************/
{
private:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

protected:
    /*
     * the Encryption bridge component, which performs encrypt and decrypt
     * operation based on xmlsec library.
     */
    css::uno::Reference< css::xml::crypto::XXMLEncryption > m_xXMLEncryption;

    /*
     * the Id of template blocker.
     */
    sal_Int32 m_nIdOfBlocker;

protected:
    explicit EncryptionEngine( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    virtual ~EncryptionEngine(){};

    virtual void tryToPerform( )
        throw (css::uno::Exception, css::uno::RuntimeException) override;
    virtual void clearUp( ) const override;
    virtual bool checkReady() const override;

    /*
     * starts the main function. This method will be implemented by any sub-class.
     * For a Encryptor, it performs encryption operation;
     * for a Decryptor, decryption operation is performed.
     */
    virtual void startEngine( const css::uno::Reference< css::xml::crypto::XXMLEncryptionTemplate >&)
        throw (css::uno::Exception, css::uno::RuntimeException)
        {};

public:
    /* XBlockerMonitor */
    virtual void SAL_CALL setBlockerId( sal_Int32 id )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
