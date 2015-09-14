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
    com::sun::star::xml::crypto::sax::XBlockerMonitor
>
/****** encryptionEngine.hxx/CLASS encryptionEngine ***************************
 *
 *   NAME
 *  EncryptionEngine -- Base class of Encryptor and Decryptor
 *
 *   FUNCTION
 *  Maintains common members and methods related with encryption.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
private:
    com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext > m_xContext;

protected:
    /*
     * the Encryption bridge component, which performs encrypt and decrypt
     * operation based on xmlsec library.
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLEncryption > m_xXMLEncryption;

    /*
     * the Id of template blocker.
     */
    sal_Int32 m_nIdOfBlocker;

protected:
    explicit EncryptionEngine( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > & xContext );
    virtual ~EncryptionEngine(){};

    virtual void tryToPerform( )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void clearUp( ) const SAL_OVERRIDE;
    virtual bool checkReady() const SAL_OVERRIDE;

    /*
     * starts the main function. This method will be implemented by any sub-class.
     * For a Encryptor, it performs encryption operation;
     * for a Decryptor, decryption operation is performed.
     */
    virtual void startEngine( const com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLEncryptionTemplate >&)
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
        {};

public:
    /* XBlockerMonitor */
    virtual void SAL_CALL setBlockerId( sal_Int32 id )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
