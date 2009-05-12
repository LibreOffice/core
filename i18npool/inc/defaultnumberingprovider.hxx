/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: defaultnumberingprovider.hxx,v $
 * $Revision: 1.8 $
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
#ifndef _I18N_DEFAULT_NUMBERING_PROVIDER_HXX_
#define _I18N_DEFAULT_NUMBERING_PROVIDER_HXX_

#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/i18n/XTransliteration.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

#include <transliterationImpl.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class DefaultNumberingProvider : public cppu::WeakImplHelper4
<
    com::sun::star::text::XDefaultNumberingProvider,
    com::sun::star::text::XNumberingFormatter,
    com::sun::star::text::XNumberingTypeInfo,
    com::sun::star::lang::XServiceInfo
>
{
    void GetCharStrN( sal_Int32 nValue, sal_Int16 nType, rtl::OUString& rStr ) const;
    void GetCharStr( sal_Int32 nValue, sal_Int16 nType, rtl::OUString& rStr ) const;
    void GetRomanString( sal_Int32 nValue, sal_Int16 nType, rtl::OUString& rStr ) const;
public:
    DefaultNumberingProvider(
        const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& xMSF );
    ~DefaultNumberingProvider();

    //XDefaultNumberingProvider
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
        com::sun::star::container::XIndexAccess > > SAL_CALL
        getDefaultOutlineNumberings( const com::sun::star::lang::Locale& aLocale )
        throw(com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Sequence<
        com::sun::star::beans::PropertyValue > > SAL_CALL
        getDefaultContinuousNumberingLevels( const com::sun::star::lang::Locale& aLocale )
        throw(com::sun::star::uno::RuntimeException);

    //XNumberingFormatter
    virtual rtl::OUString SAL_CALL makeNumberingString(
        const com::sun::star::uno::Sequence<
        com::sun::star::beans::PropertyValue >& aProperties,
        const com::sun::star::lang::Locale& aLocale )
        throw(com::sun::star::lang::IllegalArgumentException,
        com::sun::star::uno::RuntimeException);

    //XNumberingTypeInfo
    virtual com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedNumberingTypes(  )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getNumberingType( const rtl::OUString& NumberingIdentifier )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasNumberingType( const rtl::OUString& NumberingIdentifier )
        throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getNumberingIdentifier( sal_Int16 NumberingType )
        throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
                throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
                throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
                throw( com::sun::star::uno::RuntimeException );
private:
    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xSMgr;
    com::sun::star::uno::Reference < com::sun::star::container::XHierarchicalNameAccess > xHierarchicalNameAccess;
    TransliterationImpl* translit;
    rtl::OUString SAL_CALL makeNumberingIdentifier( sal_Int16 index )
        throw(com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isScriptFlagEnabled(const rtl::OUString& aName )
        throw(com::sun::star::uno::RuntimeException);
};
} } } }

#endif
