/*************************************************************************
 *
 *  $RCSfile: defaultnumberingprovider.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 10:54:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _I18N_DEFAULT_NUMBERING_PROVIDER_HXX_
#define _I18N_DEFAULT_NUMBERING_PROVIDER_HXX_

#ifndef _COM_SUN_STAR_TEXT_XDEFAULTNUMBERINGPROVIDER_HPP_
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XNUMBERINGFORMATTER_HPP_
#include <com/sun/star/text/XNumberingFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XNUMBERINGTYPEINFO_HPP_
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_XTRANSLITERATION_HPP_
#include <com/sun/star/i18n/XTransliteration.hpp>
#endif
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
