/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _ENCRYPTIONENGINE_HXX
#define _ENCRYPTIONENGINE_HXX

#include <com/sun/star/xml/crypto/sax/XReferenceResolvedListener.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XBlockerMonitor.hpp>
#include <com/sun/star/xml/crypto/sax/XKeyCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XMissionTaker.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeper.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/xml/crypto/XXMLEncryption.hpp>
#include <cppuhelper/implbase1.hxx>

#include "securityengine.hxx"

class EncryptionEngine : public cppu::ImplInheritanceHelper1
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
 *   HISTORY
 *  05.01.2004 -    Interface supported: XBlockerMonitor
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
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
    EncryptionEngine( );
    virtual ~EncryptionEngine(){};

    virtual void tryToPerform( )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
    virtual void clearUp( ) const;
    virtual bool checkReady() const;

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
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
};

#endif

