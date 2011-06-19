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

#ifndef _I18N_NUMBERFORMATCODE_HXX_
#define _I18N_NUMBERFORMATCODE_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase2.hxx> // helper for implementations

#include <com/sun/star/i18n/XNumberFormatCode.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

class NumberFormatCodeMapper : public cppu::WeakImplHelper2
<
    ::com::sun::star::i18n::XNumberFormatCode,
    ::com::sun::star::lang::XServiceInfo
>
{
public:
    NumberFormatCodeMapper( const ::com::sun::star::uno::Reference <
                    ::com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    ~NumberFormatCodeMapper();

    virtual ::com::sun::star::i18n::NumberFormatCode SAL_CALL getDefault( sal_Int16 nFormatType, sal_Int16 nFormatUsage, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::i18n::NumberFormatCode SAL_CALL getFormatCode( sal_Int16 nFormatIndex, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > SAL_CALL getAllFormatCode( sal_Int16 nFormatUsage, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > SAL_CALL getAllFormatCodes( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
                throw( ::com::sun::star::uno::RuntimeException );

private:
    ::com::sun::star::lang::Locale aLocale;
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > xMSF;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > aFormatSeq;
    ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XLocaleData > xlocaleData;
    sal_Bool bFormatsValid;

    void setupLocale( const ::com::sun::star::lang::Locale& rLocale );
    void getFormats( const ::com::sun::star::lang::Locale& rLocale );
    ::rtl::OUString mapElementTypeShortToString(sal_Int16 formatType);
    sal_Int16 mapElementTypeStringToShort(const ::rtl::OUString& formatType);
    ::rtl::OUString mapElementUsageShortToString(sal_Int16 formatUsage);
    sal_Int16 mapElementUsageStringToShort(const ::rtl::OUString& formatUsage);
    void createLocaleDataObject();
};


#endif // _I18N_NUMBERFORMATCODE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
