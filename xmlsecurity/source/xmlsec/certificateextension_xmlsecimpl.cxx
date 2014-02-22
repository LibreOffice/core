/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sal/config.h>
#include <rtl/uuid.h>

#ifndef _certificateextension_nssimpl_hxx_
#include "certificateextension_xmlsecimpl.hxx"
#endif

using namespace ::com::sun::star::uno ;

using ::com::sun::star::security::XCertificateExtension ;

CertificateExtension_XmlSecImpl :: CertificateExtension_XmlSecImpl() :
    m_critical( sal_False ) ,
    m_xExtnId() ,
    m_xExtnValue()
{
}

CertificateExtension_XmlSecImpl :: ~CertificateExtension_XmlSecImpl() {
}



sal_Bool SAL_CALL CertificateExtension_XmlSecImpl :: isCritical() throw( ::com::sun::star::uno::RuntimeException ) {
    return m_critical ;
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL CertificateExtension_XmlSecImpl :: getExtensionId() throw( ::com::sun::star::uno::RuntimeException ) {
    return m_xExtnId ;
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL CertificateExtension_XmlSecImpl :: getExtensionValue() throw( ::com::sun::star::uno::RuntimeException ) {
    return m_xExtnValue ;
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
