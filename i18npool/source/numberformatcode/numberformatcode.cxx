/*************************************************************************
 *
 *  $RCSfile: numberformatcode.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bustamam $ $Date: 2001-09-16 15:23:00 $
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

#include <numberformatcode.hxx>
#include <com/sun/star/i18n/KNumberFormatUsage.hpp>
#include <com/sun/star/i18n/KNumberFormatType.hpp>



NumberFormatCodeMapper::NumberFormatCodeMapper(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::lang::XMultiServiceFactory >& rxMSF )
        :
        xMSF( rxMSF ),
        bFormatsValid( sal_False )
{
}


NumberFormatCodeMapper::~NumberFormatCodeMapper()
{
}


::com::sun::star::i18n::NumberFormatCode SAL_CALL
NumberFormatCodeMapper::getDefault( sal_Int16 formatType, sal_Int16 formatUsage, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{

    ::rtl::OUString elementType = mapElementTypeShortToString(formatType);
    ::rtl::OUString elementUsage = mapElementUsageShortToString(formatUsage);

    getFormats( rLocale );

    for(sal_Int32 i = 0; i < aFormatSeq.getLength(); i++) {
        if(aFormatSeq[i].isDefault && aFormatSeq[i].formatType == elementType &&
            aFormatSeq[i].formatUsage == elementUsage) {
            com::sun::star::i18n::NumberFormatCode anumberFormatCode(formatType,
                                                                    formatUsage,
                                                                    aFormatSeq[i].formatCode,
                                                                    aFormatSeq[i].formatName,
                                                                    aFormatSeq[i].formatKey,
                                                                    aFormatSeq[i].formatIndex,
                                                                    sal_True);
            return anumberFormatCode;
        }
    }
    com::sun::star::i18n::NumberFormatCode defaultNumberFormatCode;
    return defaultNumberFormatCode;
}



::com::sun::star::i18n::NumberFormatCode SAL_CALL
NumberFormatCodeMapper::getFormatCode( sal_Int16 formatIndex, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{
    getFormats( rLocale );

    for(sal_Int32 i = 0; i < aFormatSeq.getLength(); i++) {
        if(aFormatSeq[i].formatIndex == formatIndex) {
            com::sun::star::i18n::NumberFormatCode anumberFormatCode(mapElementTypeStringToShort(aFormatSeq[i].formatType),
                                                                    mapElementUsageStringToShort(aFormatSeq[i].formatUsage),
                                                                    aFormatSeq[i].formatCode,
                                                                    aFormatSeq[i].formatName,
                                                                    aFormatSeq[i].formatKey,
                                                                    aFormatSeq[i].formatIndex,
                                                                    aFormatSeq[i].isDefault);
            return anumberFormatCode;
        }
    }
    com::sun::star::i18n::NumberFormatCode defaultNumberFormatCode;
    return defaultNumberFormatCode;

}



::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > SAL_CALL
NumberFormatCodeMapper::getAllFormatCode( sal_Int16 formatUsage, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{
    getFormats( rLocale );

    sal_Int32 i, count;
    count = 0;
    for(i = 0; i < aFormatSeq.getLength(); i++) {
        sal_Int16 elementUsage = mapElementUsageStringToShort(aFormatSeq[i].formatUsage);
        if( elementUsage == formatUsage)
            count++;
    }

    ::com::sun::star::uno::Sequence<com::sun::star::i18n::NumberFormatCode> seq(count);
    sal_Int32 j = 0;
    for(i = 0; i < aFormatSeq.getLength(); i++) {
        sal_Int16 elementUsage = mapElementUsageStringToShort(aFormatSeq[i].formatUsage);
        if( elementUsage == formatUsage) {
            seq[j] = com::sun::star::i18n::NumberFormatCode(mapElementTypeStringToShort(aFormatSeq[i].formatType),
                                                            formatUsage,
                                                            aFormatSeq[i].formatCode,
                                                            aFormatSeq[i].formatName,
                                                            aFormatSeq[i].formatKey,
                                                            aFormatSeq[i].formatIndex,
                                                            aFormatSeq[i].isDefault);
            j++;
        }
    }
    return seq;

}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > SAL_CALL
NumberFormatCodeMapper::getAllFormatCodes( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{
    getFormats( rLocale );

    ::com::sun::star::uno::Sequence<com::sun::star::i18n::NumberFormatCode> seq(aFormatSeq.getLength());
    for(sal_Int32 i = 0; i < aFormatSeq.getLength(); i++)
    {
        seq[i] = com::sun::star::i18n::NumberFormatCode(mapElementTypeStringToShort(aFormatSeq[i].formatType),
                                                        mapElementUsageStringToShort(aFormatSeq[i].formatUsage),
                                                        aFormatSeq[i].formatCode,
                                                        aFormatSeq[i].formatName,
                                                        aFormatSeq[i].formatKey,
                                                        aFormatSeq[i].formatIndex,
                                                        aFormatSeq[i].isDefault);
    }
    return seq;
}


// --- private implementation -----------------------------------------

void NumberFormatCodeMapper::setupLocale( const ::com::sun::star::lang::Locale& rLocale )
{
    if ( aLocale.Country    != rLocale.Country
      || aLocale.Language   != rLocale.Language
      || aLocale.Variant    != rLocale.Variant )
    {
        bFormatsValid = sal_False;
        aLocale = rLocale;
    }
}


void NumberFormatCodeMapper::getFormats( const ::com::sun::star::lang::Locale& rLocale )
{
    setupLocale( rLocale );
    if ( !bFormatsValid )
    {
        createLocaleDataObject();
        if( !xlocaleData.is() )
            aFormatSeq = ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > (0);
        else
            aFormatSeq = xlocaleData->getAllFormats( aLocale );
        bFormatsValid = sal_True;
    }
}


::rtl::OUString
NumberFormatCodeMapper::mapElementTypeShortToString(sal_Int16 formatType)
{

    switch ( formatType )
    {
        case com::sun::star::i18n::KNumberFormatType::SHORT :
            return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "short" ) );
        break;
        case com::sun::star::i18n::KNumberFormatType::MEDIUM :
            return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "medium" ) );
        break;
        case com::sun::star::i18n::KNumberFormatType::LONG :
            return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "long" ) );
        break;
    }
    return ::rtl::OUString();
}

sal_Int16
NumberFormatCodeMapper::mapElementTypeStringToShort(const ::rtl::OUString& formatType)
{
    if(formatType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "short" ) ))
        return com::sun::star::i18n::KNumberFormatType::SHORT;
    if(formatType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "medium" ) ))
        return com::sun::star::i18n::KNumberFormatType::MEDIUM;
    if(formatType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "long" ) ))
        return com::sun::star::i18n::KNumberFormatType::LONG;

    return com::sun::star::i18n::KNumberFormatType::SHORT;
}

::rtl::OUString
NumberFormatCodeMapper::mapElementUsageShortToString(sal_Int16 formatUsage)
{
    switch ( formatUsage )
    {
        case com::sun::star::i18n::KNumberFormatUsage::DATE :
            return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DATE" ) );
        break;
        case com::sun::star::i18n::KNumberFormatUsage::TIME :
            return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TIME" ) );
        break;
        case com::sun::star::i18n::KNumberFormatUsage::DATE_TIME :
            return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DATE_TIME" ) );
        break;
        case com::sun::star::i18n::KNumberFormatUsage::FIXED_NUMBER :
            return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FIXED_NUMBER" ) );
        break;
        case com::sun::star::i18n::KNumberFormatUsage::FRACTION_NUMBER :
            return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FRACTION_NUMBER" ) );
        break;
        case com::sun::star::i18n::KNumberFormatUsage::PERCENT_NUMBER :
            return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PERCENT_NUMBER" ) );
        break;
        case com::sun::star::i18n::KNumberFormatUsage::CURRENCY :
            return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CURRENCY" ) );
        break;
        case com::sun::star::i18n::KNumberFormatUsage::SCIENTIFIC_NUMBER :
            return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SCIENTIFIC_NUMBER" ) );
        break;
    }
    return ::rtl::OUString();
}


sal_Int16
NumberFormatCodeMapper::mapElementUsageStringToShort(const ::rtl::OUString& formatUsage)
{
    if(formatUsage.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "DATE" ) ))
        return com::sun::star::i18n::KNumberFormatUsage::DATE;
    if(formatUsage.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "TIME" ) ))
        return com::sun::star::i18n::KNumberFormatUsage::TIME;
    if(formatUsage.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "DATE_TIME" ) ))
        return com::sun::star::i18n::KNumberFormatUsage::DATE_TIME;
    if(formatUsage.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FIXED_NUMBER" ) ))
        return com::sun::star::i18n::KNumberFormatUsage::FIXED_NUMBER;
    if(formatUsage.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FRACTION_NUMBER" ) ))
        return com::sun::star::i18n::KNumberFormatUsage::FRACTION_NUMBER;
    if(formatUsage.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "PERCENT_NUMBER" ) ))
        return  com::sun::star::i18n::KNumberFormatUsage::PERCENT_NUMBER;
    if(formatUsage.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CURRENCY" ) ))
        return com::sun::star::i18n::KNumberFormatUsage::CURRENCY;
    if(formatUsage.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "SCIENTIFIC_NUMBER" ) ))
        return com::sun::star::i18n::KNumberFormatUsage::SCIENTIFIC_NUMBER;

    return 0;
}


void
NumberFormatCodeMapper::createLocaleDataObject() {

    if(xlocaleData.is())
        return;

    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >
        xI = xMSF->createInstance(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.i18n.LocaleData" ) ));

    if ( xI.is() ) {
        ::com::sun::star::uno::Any x = xI->queryInterface( ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XLocaleData >*)0) );
            x >>= xlocaleData;
    }
}
