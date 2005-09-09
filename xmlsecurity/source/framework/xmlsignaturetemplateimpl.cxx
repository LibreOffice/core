/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlsignaturetemplateimpl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:21:02 $
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

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef _XMLSIGNATURETEMPLATEIMPL_HXX_
#include "xmlsignaturetemplateimpl.hxx"
#endif

using namespace ::com::sun::star::uno ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;
using ::rtl::OUString ;

using ::com::sun::star::xml::wrapper::XXMLElementWrapper ;
using ::com::sun::star::xml::crypto::XXMLSignatureTemplate ;

XMLSignatureTemplateImpl :: XMLSignatureTemplateImpl( const Reference< XMultiServiceFactory >& aFactory )
    :m_xServiceManager( aFactory ),
     m_xTemplate( NULL ),
     m_nStatus ( ::com::sun::star::xml::crypto::SecurityOperationStatus_UNKNOWN )
{
}

XMLSignatureTemplateImpl :: ~XMLSignatureTemplateImpl() {
}

/* XXMLSignatureTemplate */
void SAL_CALL XMLSignatureTemplateImpl :: setTemplate( const Reference< XXMLElementWrapper >& aTemplate )
    throw( com::sun::star::uno::RuntimeException, com::sun::star::lang::IllegalArgumentException)
{
    m_xTemplate = aTemplate ;
}

/* XXMLSignatureTemplate */
Reference< XXMLElementWrapper > SAL_CALL XMLSignatureTemplateImpl :: getTemplate()
    throw (com::sun::star::uno::RuntimeException)
{
    return m_xTemplate ;
}

void SAL_CALL XMLSignatureTemplateImpl :: setTarget( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper >& aXmlElement )
    throw( com::sun::star::uno::RuntimeException, com::sun::star::lang::IllegalArgumentException)
{
    targets.push_back( aXmlElement );
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper > > SAL_CALL XMLSignatureTemplateImpl :: getTargets()
    throw (com::sun::star::uno::RuntimeException)
{
    sal_Int32 length = targets.size();
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper >
        > aTargets (length);

    sal_Int32 i;

    for (i=0; i<length; i++)
    {
        aTargets[i] = targets[i];
    }

    return aTargets;
}

void SAL_CALL XMLSignatureTemplateImpl::setBinding(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::crypto::XUriBinding >& aUriBinding )
    throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    m_xUriBinding = aUriBinding;
}

::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XUriBinding > SAL_CALL XMLSignatureTemplateImpl::getBinding()
    throw (::com::sun::star::uno::RuntimeException)
{
    return m_xUriBinding;
}

void SAL_CALL XMLSignatureTemplateImpl::setStatus(
    ::com::sun::star::xml::crypto::SecurityOperationStatus status )
    throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    m_nStatus = status;
}

::com::sun::star::xml::crypto::SecurityOperationStatus SAL_CALL XMLSignatureTemplateImpl::getStatus(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    return m_nStatus;
}

/* XInitialization */
void SAL_CALL XMLSignatureTemplateImpl :: initialize( const Sequence< Any >& aArguments ) throw( Exception, RuntimeException ) {
    // TBD
} ;

/* XServiceInfo */
OUString SAL_CALL XMLSignatureTemplateImpl :: getImplementationName() throw( RuntimeException ) {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSignatureTemplateImpl :: supportsService( const OUString& serviceName) throw( RuntimeException ) {
    Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
    const OUString* pArray = seqServiceNames.getConstArray() ;
    for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
        if( *( pArray + i ) == serviceName )
            return sal_True ;
    }
    return sal_False ;
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLSignatureTemplateImpl :: getSupportedServiceNames() throw( RuntimeException ) {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLSignatureTemplateImpl :: impl_getSupportedServiceNames() {
    ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
    Sequence< OUString > seqServiceNames( 1 ) ;
    seqServiceNames.getArray()[0] = OUString::createFromAscii( "com.sun.star.xml.crypto.XMLSignatureTemplate" ) ;
    return seqServiceNames ;
}

OUString XMLSignatureTemplateImpl :: impl_getImplementationName() throw( RuntimeException ) {
    return OUString::createFromAscii( "com.sun.star.xml.security.framework.XMLSignatureTemplateImpl" ) ;
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLSignatureTemplateImpl :: impl_createInstance( const Reference< XMultiServiceFactory >& aServiceManager ) throw( RuntimeException ) {
    return Reference< XInterface >( *new XMLSignatureTemplateImpl( aServiceManager ) ) ;
}

Reference< XSingleServiceFactory > XMLSignatureTemplateImpl :: impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    //Reference< XSingleServiceFactory > xFactory ;
    //xFactory = ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName , impl_createInstance , impl_getSupportedServiceNames ) ;
    //return xFactory ;
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

