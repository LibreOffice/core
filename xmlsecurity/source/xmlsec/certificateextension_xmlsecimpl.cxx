/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>
#include <rtl/uuid.h>

#include "certificateextension_xmlsecimpl.hxx"

using namespace ::com::sun::star::uno ;

using ::com::sun::star::security::XCertificateExtension ;

CertificateExtension_XmlSecImpl::CertificateExtension_XmlSecImpl() :
    m_critical( false ) ,
    m_xExtnId() ,
    m_xExtnValue()
{
}

CertificateExtension_XmlSecImpl::~CertificateExtension_XmlSecImpl() {
}


//Methods from XCertificateExtension
sal_Bool SAL_CALL CertificateExtension_XmlSecImpl::isCritical() throw( ::com::sun::star::uno::RuntimeException, std::exception ) {
    return m_critical ;
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL CertificateExtension_XmlSecImpl::getExtensionId() throw( ::com::sun::star::uno::RuntimeException, std::exception ) {
    return m_xExtnId ;
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL CertificateExtension_XmlSecImpl::getExtensionValue() throw( ::com::sun::star::uno::RuntimeException, std::exception ) {
    return m_xExtnValue ;
}

void CertificateExtension_XmlSecImpl::setCertExtn( unsigned char* value, unsigned int vlen, unsigned char* id, unsigned int idlen, bool critical ) {
    unsigned int i ;
    if( value != nullptr && vlen != 0 ) {
        Sequence< sal_Int8 > extnv( vlen ) ;
        for( i = 0; i < vlen ; i ++ )
            extnv[i] = *( value + i ) ;

        m_xExtnValue = extnv ;
    } else {
        m_xExtnValue = Sequence<sal_Int8>();
    }

    if( id != nullptr && idlen != 0 ) {
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
