/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: signaturecreatorimpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:19:18 $
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

#ifndef _SIGNATURECREATORIMPL_HXX
#define _SIGNATURECREATORIMPL_HXX

#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XBLOCKERMONITOR_HPP_
#include <com/sun/star/xml/crypto/sax/XBlockerMonitor.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XSIGNATURECREATIONRESULTLISTENER_HPP_
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultListener.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XSIGNATURECREATIONRESULTBROADCASTER_HPP_
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_XSECURITYENVIRONMENT_HPP_
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif

#include "signatureengine.hxx"

class SignatureCreatorImpl : public cppu::ImplInheritanceHelper4
<
    SignatureEngine,
    com::sun::star::xml::crypto::sax::XBlockerMonitor,
    com::sun::star::xml::crypto::sax::XSignatureCreationResultBroadcaster,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
/****** SignatureCreatorImpl.hxx/CLASS SignatureCreatorImpl *******************
 *
 *   NAME
 *  SignatureCreatorImpl -- generates a signature
 *
 *   FUNCTION
 *  Collects all resources for a signature generation, then generates the
 *  signature by invoking a xmlsec-based signature bridge component.
 *
 *   HISTORY
 *  05.01.2004 -    Interface supported: XBlockerMonitor,
 *          XInitialization, XServiceInfo
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
private:
    /*
     * the Id of the signature, which is used for the result listener to
     * identify the signature.
     */
    sal_Int32 m_nSignatureId;

    /*
     * the Id of template blocker.
     */
    sal_Int32 m_nIdOfBlocker;

    /*
     * the signature creation result
     */
    bool      m_bCreationSucceed;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XSecurityEnvironment > m_xSecurityEnvironment;

    virtual void notifyResultListener() const
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
    virtual void clearUp( ) const;
    virtual bool checkReady() const;
    virtual void startEngine( const com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSignatureTemplate >&
        xSignatureTemplate)
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

public:
    explicit SignatureCreatorImpl( const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >& rxMSF);
    virtual ~SignatureCreatorImpl();

    /* XBlockerMonitor */
    virtual void SAL_CALL setBlockerId( sal_Int32 id )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    /* XSignatureCreationResultBroadcaster */
    void SAL_CALL addSignatureCreationResultListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XSignatureCreationResultListener >& listener )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    void SAL_CALL removeSignatureCreationResultListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XSignatureCreationResultListener >& listener )
        throw (com::sun::star::uno::RuntimeException);

    /* XInitialization */
    virtual void SAL_CALL initialize(
        const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    /* XServiceInfo */
    virtual rtl::OUString SAL_CALL getImplementationName(  )
        throw (com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName )
        throw (com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (com::sun::star::uno::RuntimeException);
};

rtl::OUString SignatureCreatorImpl_getImplementationName()
    throw ( com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL SignatureCreatorImpl_supportsService( const rtl::OUString& ServiceName )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL SignatureCreatorImpl_getSupportedServiceNames(  )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
SAL_CALL SignatureCreatorImpl_createInstance(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw ( com::sun::star::uno::Exception );

#endif



