/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: certificateextension_xmlsecimpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:25:20 $
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

#ifndef _CERTIFICATEEXTENSION_XMLSECIMPL_HXX_
#define _CERTIFICATEEXTENSION_XMLSECIMPL_HXX_

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SECURITYEXCEPTION_HPP_
#include "com/sun/star/uno/SecurityException.hpp"
#endif

#ifndef _COM_SUN_STAR_SECURITY_XCERTIFICATEEXTENSION_HPP_
#include <com/sun/star/security/XCertificateExtension.hpp>
#endif

class CertificateExtension_XmlSecImpl : public ::cppu::WeakImplHelper1<
    ::com::sun::star::security::XCertificateExtension >
{
    private :
        sal_Bool m_critical ;
        ::com::sun::star::uno::Sequence< sal_Int8 > m_xExtnId ;
        ::com::sun::star::uno::Sequence< sal_Int8 > m_xExtnValue ;

    public :
        CertificateExtension_XmlSecImpl() ;
        virtual ~CertificateExtension_XmlSecImpl() ;

        //Methods from XCertificateExtension
        virtual sal_Bool SAL_CALL isCritical() throw( ::com::sun::star::uno::RuntimeException ) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getExtensionId() throw( ::com::sun::star::uno::RuntimeException ) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getExtensionValue() throw( ::com::sun::star::uno::RuntimeException ) ;

        //Helper method
        void setCertExtn( ::com::sun::star::uno::Sequence< sal_Int8 > extnId, ::com::sun::star::uno::Sequence< sal_Int8 > extnValue, sal_Bool critical ) ;

        void setCertExtn( unsigned char* value, unsigned int vlen, unsigned char* id, unsigned int idlen, sal_Bool critical ) ;
} ;

#endif  // _CERTIFICATEEXTENSION_XMLSECIMPL_HXX_

