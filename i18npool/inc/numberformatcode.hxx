/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numberformatcode.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:54:35 $
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

#ifndef _I18N_NUMBERFORMATCODE_HXX_
#define _I18N_NUMBERFORMATCODE_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase2.hxx> // helper for implementations

#include <com/sun/star/i18n/XNumberFormatCode.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

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
