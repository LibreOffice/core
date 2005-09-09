/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: seinitializer_nssimpl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:34:28 $
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

#ifndef _SEINITIALIZERIMPL_HXX
#define _SEINITIALIZERIMPL_HXX

#ifndef _COM_SUN_STAR_XML_CRYPTO_XXMLSECURITYCONTEXT_HPP_
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SEINITIALIZER_HPP_
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#include <libxml/tree.h>

class SEInitializer_NssImpl : public cppu::WeakImplHelper2
<
    com::sun::star::xml::crypto::XSEInitializer,
    com::sun::star::lang::XServiceInfo
>
/****** SEInitializer_NssImpl.hxx/CLASS SEInitializer_NssImpl ***********
 *
 *   NAME
 *  SEInitializer_NssImpl -- Class to initialize a Security Context
 *  instance
 *
 *   FUNCTION
 *  Use this class to initialize a XmlSec based Security Context
 *  instance. After this instance is used up, use this class to free this
 *  instance.
 *
 *   HISTORY
 *  05.01.2004 -    Interface supported: XSEInitializer, XSEInitializer
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;

public:
    SEInitializer_NssImpl(const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > &rxMSF);
    virtual ~SEInitializer_NssImpl();

    /* XSEInitializer */
    virtual com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSecurityContext >
        SAL_CALL createSecurityContext( const rtl::OUString& certDB )
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL freeSecurityContext( const com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSecurityContext >& securityContext )
        throw (com::sun::star::uno::RuntimeException);

    /* XServiceInfo */
    virtual rtl::OUString SAL_CALL getImplementationName(  )
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName )
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (com::sun::star::uno::RuntimeException);
};

rtl::OUString SEInitializer_NssImpl_getImplementationName()
    throw ( com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL SEInitializer_NssImpl_supportsService( const rtl::OUString& ServiceName )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL SEInitializer_NssImpl_getSupportedServiceNames(  )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
SAL_CALL SEInitializer_NssImpl_createInstance( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw ( com::sun::star::uno::Exception );

#endif

