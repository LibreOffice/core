/*************************************************************************
 *
 *  $RCSfile: securityengine.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SECURITYENGINE_HXX
#define _SECURITYENGINE_HXX

#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XREFERENCERESOLVEDLISTENER_HPP_
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedListener.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XREFERENCERESOLVEDBROADCASTER_HPP_
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
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
#ifndef _COM_SUN_STAR_XML_CRYPTO_XXMLSIGNATURE_HPP_
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

class SecurityEngine : public cppu::WeakImplHelper3
<
    com::sun::star::xml::crypto::sax::XReferenceResolvedListener,
    com::sun::star::xml::crypto::sax::XKeyCollector,
    com::sun::star::xml::crypto::sax::XMissionTaker
>
/****** securityengine.hxx/CLASS SecurityEngine *******************************
 *
 *   NAME
 *  SecurityEngine -- Base class of SignatureEngine and EncryptionEngine
 *
 *   FUNCTION
 *  Maintains common members and methods related with security engine
 *  operation.
 *
 *   HISTORY
 *  05.01.2004 -    Interface supported: XReferenceResolvedListener,
 *          XKeyCollector, and XMissionTaker
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
protected:
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > mxMSF;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSecurityContext > m_xXMLSecurityContext;

    /*
     * A SAXEventKeeper internally maintians all resources that a security
     * operation needs. The m_xSAXEventKeeper member is used to release
     * those resources when the security operation finishes.
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XSAXEventKeeper > m_xSAXEventKeeper;

    /*
     * the id of ElementCollector of the template element.
     * For a signature, the template element is the Signature element,
     * for a encryption, the EncryptedData/EncryptedKey element is.
     */
    sal_Int32 m_nIdOfTemplateEC;

    /*
     * remembers how many referenced elements have been bufferred completely,
     * including the key element, template element, and referenced element of
     * signature.
     */
    sal_Int32 m_nNumOfResolvedReferences;

    /*
     * the id of ElementCollector of the key element.
     * If a Signature element or EncryptedData/EncryptedKey element has
     * an internal key sub-element, then this member should be -1
     */
    sal_Int32 m_nIdOfKeyEC;

    /*
     * remembers whether the current opertion has finished.
     */
    bool      m_bMissionDone;

    /*
     * the Id of the security entity, a signature or encryption, which is used for
     * the result listener to identify the entity.
     */
    sal_Int32 m_nSecurityId;

    /*
     * the result of the operation
     */
    bool      m_bOperationSucceed;

    /*
     * the result listener, which will receives the security operation result.
     */
    com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface >
        m_xResultListener;

protected:
    explicit SecurityEngine( const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >& rxMSF = NULL );
    virtual ~SecurityEngine() {};

    /*
     * perform the security operation.
     * Any derived class will implement this method respectively.
     */
    virtual void tryToPerform( )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException){};

    /*
     * clear up all resources used by this operation.
     * This method is called after the operation finishes, or a End-Your-Mission
     * message is received.
     * Any derived class will implement this method respectively.
     */
    virtual void clearUp( ) const {};

        /*
         * notifies any possible result listener.
         * When verify a signature or conduct a decryption, the operation result will
         * be transferred to a listener by this method.
     * Any derived class will implement this method respectively.
         */
    virtual void notifyResultListener() const
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
        {};

    /*
     * checks whether everything is ready.
     * Any derived class will implement this method respectively.
     */
    virtual bool checkReady() const { return true; };

public:
    /* XReferenceResolvedListener */
    virtual void SAL_CALL referenceResolved( sal_Int32 referenceId )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    /* XKeyCollector */
    virtual void SAL_CALL setKeyId( sal_Int32 id )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

        /* XMissionTaker */
        virtual sal_Bool SAL_CALL endMission(  )
            throw (com::sun::star::uno::RuntimeException);
};

#endif

