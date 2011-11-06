/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

