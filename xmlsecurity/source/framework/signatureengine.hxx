/*************************************************************************
 *
 *  $RCSfile: signatureengine.hxx,v $
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

#ifndef _SIGNATUREENGINE_HXX
#define _SIGNATUREENGINE_HXX

#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XREFERENCERESOLVEDLISTENER_HPP_
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedListener.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XREFERENCERESOLVEDBROADCASTER_HPP_
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XSIGNATURECOLLECTOR_HPP_
#include <com/sun/star/xml/crypto/sax/XReferenceCollector.hpp>
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
#ifndef _COM_SUN_STAR_XML_CRYPTO_XURIBINDING_HPP_
#include <com/sun/star/xml/crypto/XUriBinding.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#include "securityengine.hxx"

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

class SignatureEngine : public cppu::ImplInheritanceHelper2
<
    SecurityEngine,
    com::sun::star::xml::crypto::sax::XReferenceCollector,
    com::sun::star::xml::crypto::XUriBinding
>
/****** signatureengine.hxx/CLASS SignatureEngine *****************************
 *
 *   NAME
 *  SignatureEngine -- Base class of SignatureCreator and SignatureVerifier
 *
 *   FUNCTION
 *  Maintains common members and methods related with signature operation.
 *
 *   HISTORY
 *  05.01.2004 -    Interface supported: XReferenceCollector
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
protected:

    /*
     * the Signature bridge component, which performs signature generation
     * and verification based on xmlsec library.
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSignature > m_xXMLSignature;

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
    std::vector< rtl::OUString > m_vUris;
    std::vector< com::sun::star::uno::Reference<
        com::sun::star::io::XInputStream > > m_vXInputStreams;

protected:
    SignatureEngine( );
    virtual ~SignatureEngine() {};

    virtual void tryToPerform( )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
    virtual void clearUp( ) const;
    virtual bool checkReady() const;

    /*
     * starts the main function. This method will be implemented by any sub-class.
     * For a SignatureCreator, it performs signing operation;
     * for a SignatureVerifier, verification operation is performed.
     */
    virtual void startEngine( const com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSignatureTemplate >&
        xSignatureTemplate)
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
        {};

public:
    /* XReferenceCollector */
    virtual void SAL_CALL setReferenceCount( sal_Int32 count )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setReferenceId( sal_Int32 id )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    /* XUriBinding */
    virtual void SAL_CALL setUriBinding(
        const rtl::OUString& uri,
        const com::sun::star::uno::Reference<
            com::sun::star::io::XInputStream >& aInputStream )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
        SAL_CALL getUriBinding( const rtl::OUString& uri )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
};

#endif

