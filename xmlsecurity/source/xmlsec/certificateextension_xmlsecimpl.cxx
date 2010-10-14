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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"
#include <sal/config.h>
#include <rtl/uuid.h>

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
        m_xExtnValue = Sequence<sal_Int8>();
    }

    if( id != NULL && idlen != 0 ) {
        Sequence< sal_Int8 > extnId( idlen ) ;
        for( i = 0; i < idlen ; i ++ )
            extnId[i] = *( id + i ) ;

        m_xExtnId = extnId ;
    } else {
        m_xExtnId =  Sequence<sal_Int8>();
    }

    m_critical = critical ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
