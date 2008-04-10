/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: textconversionImpl.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _I18N_TEXTCONERSION_HXX_
#define _I18N_TEXTCONERSION_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/i18n/XExtendedTextConversion.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class TextConversion
//  ----------------------------------------------------
class TextConversionImpl : public cppu::WeakImplHelper2
<
    com::sun::star::i18n::XExtendedTextConversion,
    com::sun::star::lang::XServiceInfo
>
{
public:
    TextConversionImpl( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF ) : xMSF(rxMSF) {};

        // Methods
        com::sun::star::i18n::TextConversionResult SAL_CALL
        getConversions( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        rtl::OUString SAL_CALL
        getConversion( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        rtl::OUString SAL_CALL
        getConversionWithOffset( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions, com::sun::star::uno::Sequence< sal_Int32 >& offset )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        sal_Bool SAL_CALL
        interactiveConversion( const ::com::sun::star::lang::Locale& aLocale,
            sal_Int16 nTextConversionType, sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );

    //XServiceInfo
    rtl::OUString SAL_CALL
        getImplementationName()
            throw( com::sun::star::uno::RuntimeException );
    sal_Bool SAL_CALL
        supportsService(const rtl::OUString& ServiceName)
            throw( com::sun::star::uno::RuntimeException );
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        getSupportedServiceNames()
            throw( com::sun::star::uno::RuntimeException );
private :
    com::sun::star::lang::Locale aLocale;
    com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedTextConversion > xTC;
    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xMSF;

    void SAL_CALL getLocaleSpecificTextConversion( const com::sun::star::lang::Locale& rLocale )
            throw( com::sun::star::lang::NoSupportException );
};

} // i18n
} // star
} // sun
} // com


#endif
