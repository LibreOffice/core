/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textconversion.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:56:29 $
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
#ifndef _I18N_TEXTCONVERSION_KO_HXX_
#define _I18N_TEXTCONVERSION_KO_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/i18n/XTextConversion.hpp>
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/linguistic2/XConversionDictionaryList.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class TextConversion
//  ----------------------------------------------------
class TextConversion: public cppu::WeakImplHelper2
<
    com::sun::star::i18n::XTextConversion,
    com::sun::star::lang::XServiceInfo
>
{
public:
        // Methods
        virtual com::sun::star::i18n::TextConversionResult SAL_CALL
        getConversions( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException ) = 0;
        virtual rtl::OUString SAL_CALL
        getConversion( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException ) = 0;
        virtual sal_Bool SAL_CALL
        interactiveConversion(const ::com::sun::star::lang::Locale& aLocale,
            sal_Int16 nTextConversionType, sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException ) = 0;

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
protected :
    const sal_Char* implementationName;
};

// for Hangul2Hanja conversion
typedef struct {
    sal_Unicode code;
    sal_Int16 address;
    sal_Int16 count;
} Hangul_Index;

//  ----------------------------------------------------
//  class TextConversion_ko
//  ----------------------------------------------------
class TextConversion_ko : public TextConversion
{
public:
    TextConversion_ko( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );

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
        interactiveConversion(const ::com::sun::star::lang::Locale& aLocale,
            sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );

private :
        // Hangul/Hanja system dictionary
        com::sun::star::uno::Reference < com::sun::star::linguistic2::XConversionDictionary > xCD;
        // Hangul/Hanja user defined dictionary list
        com::sun::star::uno::Reference < com::sun::star::linguistic2::XConversionDictionaryList > xCDL;
        sal_Int32 maxLeftLength;
        sal_Int32 maxRightLength;
};

//  ----------------------------------------------------
//  class TextConversion_zh
//  ----------------------------------------------------

// for SChines/TChinese word conversion
typedef struct {
    sal_uInt16 start;
    sal_Int16 count;
} STC_WordIndex;

class TextConversion_zh : public TextConversion
{
public:
    TextConversion_zh( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );

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
        interactiveConversion(const ::com::sun::star::lang::Locale& aLocale,
            sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
private :
        // user defined dictionary list
        com::sun::star::uno::Reference < com::sun::star::linguistic2::XConversionDictionaryList > xCDL;
        rtl::OUString SAL_CALL getWordConversion(const ::rtl::OUString& aText,
            sal_Int32 nStartPos, sal_Int32 nLength, sal_Bool toSChinese, sal_Int32 nConversionOptions);
        com::sun::star::lang::Locale aLocale;
};

} // i18n
} // star
} // sun
} // com

#endif
