/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SIGNATUREENGINE_HXX
#define _SIGNATUREENGINE_HXX

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

#include <cppuhelper/implbase2.hxx>

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
                              com::sun::star::xml::crypto::XXMLSignatureTemplate >&)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
