/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textconversionImpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:56:45 $
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
#ifndef _I18N_TEXTCONERSION_HXX_
#define _I18N_TEXTCONERSION_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/i18n/XTextConversion.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class TextConversion
//  ----------------------------------------------------
class TextConversionImpl : public cppu::WeakImplHelper2
<
    com::sun::star::i18n::XTextConversion,
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
    com::sun::star::uno::Reference < com::sun::star::i18n::XTextConversion > xTC;
    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xMSF;

    void SAL_CALL getLocaleSpecificTextConversion( const com::sun::star::lang::Locale& rLocale )
            throw( com::sun::star::lang::NoSupportException );
};

} // i18n
} // star
} // sun
} // com


#endif
