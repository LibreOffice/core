/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: certificateextension_xmlsecimpl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:25:06 $
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

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef _certificateextension_nssimpl_hxx_
#include "certificateextension_xmlsecimpl.hxx"
#endif

using namespace ::com::sun::star::uno ;
using ::rtl::OUString ;

using ::com::sun::star::security::XCertificateExtension ;

CertificateExtension_XmlSecImpl :: CertificateExtension_XmlSecImpl() :
    m_critical( sal_False ) ,
    m_xExtnId() ,
    m_xExtnValue()
{
}

CertificateExtension_XmlSecImpl :: ~CertificateExtension_XmlSecImpl() {
}


//Methods from XCertificateExtension
sal_Bool SAL_CALL CertificateExtension_XmlSecImpl :: isCritical() throw( ::com::sun::star::uno::RuntimeException ) {
    return m_critical ;
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL CertificateExtension_XmlSecImpl :: getExtensionId() throw( ::com::sun::star::uno::RuntimeException ) {
    return m_xExtnId ;
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL CertificateExtension_XmlSecImpl :: getExtensionValue() throw( ::com::sun::star::uno::RuntimeException ) {
    return m_xExtnValue ;
}

//Helper method
void CertificateExtension_XmlSecImpl :: setCertExtn( ::com::sun::star::uno::Sequence< sal_Int8 > extnId, ::com::sun::star::uno::Sequence< sal_Int8 > extnValue, sal_Bool critical ) {
    m_critical = critical ;
    m_xExtnId = extnId ;
    m_xExtnValue = extnValue ;
}

void CertificateExtension_XmlSecImpl :: setCertExtn( unsigned char* value, unsigned int vlen, unsigned char* id, unsigned int idlen, sal_Bool critical ) {
    unsigned int i ;
    if( value != NULL && vlen != 0 ) {
        Sequence< sal_Int8 > extnv( vlen ) ;
        for( i = 0; i < vlen ; i ++ )
            extnv[i] = *( value + i ) ;

        m_xExtnValue = extnv ;
    } else {
        m_xExtnValue = NULL ;
    }

    if( id != NULL && idlen != 0 ) {
        Sequence< sal_Int8 > extnId( idlen ) ;
        for( i = 0; i < idlen ; i ++ )
            extnId[i] = *( id + i ) ;

        m_xExtnId = extnId ;
    } else {
        m_xExtnId = NULL ;
    }

    m_critical = critical ;
}

