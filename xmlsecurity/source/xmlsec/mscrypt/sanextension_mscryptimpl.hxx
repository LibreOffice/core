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

#ifndef _SANEXTENSION_MSCRYPTIMPL_HXX_
#define _SANEXTENSION_MSCRYPTIMPL_HXX_

#ifdef _MSC_VER
#pragma warning(push,1)
#endif
#include "Windows.h"
#include "WinCrypt.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include "com/sun/star/uno/SecurityException.hpp"
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/security/XCertificateExtension.hpp>
#include <com/sun/star/security/XSanExtension.hpp>
#include <com/sun/star/security/CertAltNameEntry.hpp>

class SanExtensionImpl : public ::cppu::WeakImplHelper1<
    ::com::sun::star::security::XSanExtension >
{
    private :
        sal_Bool m_critical ;
        ::com::sun::star::uno::Sequence< sal_Int8 > m_xExtnId ;
        ::com::sun::star::uno::Sequence< sal_Int8 > m_xExtnValue ;

        ::com::sun::star::uno::Sequence< com::sun::star::security::CertAltNameEntry > m_Entries;

    public :
        SanExtensionImpl() ;
        virtual ~SanExtensionImpl() ;

        //Methods from XCertificateExtension
        virtual sal_Bool SAL_CALL isCritical() throw( ::com::sun::star::uno::RuntimeException ) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getExtensionId() throw( ::com::sun::star::uno::RuntimeException ) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getExtensionValue() throw( ::com::sun::star::uno::RuntimeException ) ;

        //Methods from XSanExtension

        virtual ::com::sun::star::uno::Sequence< com::sun::star::security::CertAltNameEntry > SAL_CALL getAlternativeNames() throw( ::com::sun::star::uno::RuntimeException ) ;

        //Helper method
        void setCertExtn( ::com::sun::star::uno::Sequence< sal_Int8 > extnId, ::com::sun::star::uno::Sequence< sal_Int8 > extnValue, sal_Bool critical ) ;

        void setCertExtn( unsigned char* value, unsigned int vlen, unsigned char* id, unsigned int idlen, sal_Bool critical ) ;

        void extractCertExt() ;
} ;

#endif  // _CERTIFICATEEXTENSION_XMLSECIMPL_HXX_

