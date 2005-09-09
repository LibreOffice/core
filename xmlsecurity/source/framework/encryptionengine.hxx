/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: encryptionengine.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:16:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _ENCRYPTIONENGINE_HXX
#define _ENCRYPTIONENGINE_HXX

#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XREFERENCERESOLVEDLISTENER_HPP_
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedListener.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XREFERENCERESOLVEDBROADCASTER_HPP_
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XBLOCKERMONITOR_HPP_
#include <com/sun/star/xml/crypto/sax/XBlockerMonitor.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XKEYCOLLECTOR_HPP_
#include <com/sun/star/xml/crypto/sax/XKeyCollector.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XMISSIONTAKER_HPP_
#include <com/sun/star/xml/crypto/sax/XMissionTaker.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XSAXEVENTKEEPER_HPP_
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeper.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_XXMLSECURITYCONTEXT_HPP_
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_XXMLENCRYPTION_HPP_
#include <com/sun/star/xml/crypto/XXMLEncryption.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

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
        com::sun::star::xml::crypto::XXMLEncryptionTemplate >&
        xEncryptionTemplate)
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
        {};

public:
    /* XBlockerMonitor */
    virtual void SAL_CALL setBlockerId( sal_Int32 id )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
};

#endif

