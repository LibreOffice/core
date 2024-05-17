/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <unitconverter.hxx>

#include <com/sun/star/awt/DeviceInfo.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XFont.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <o3tl/unit_conversion.hxx>
#include <osl/diagnose.h>
#include <oox/core/filterbase.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>
#include <stylesbuffer.hxx>
#include <biffhelper.hxx>
#include <docuno.hxx>

namespace com::sun::star::awt { struct FontDescriptor; }

namespace oox::xls {

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;

namespace {

/** Returns true, if the passed year is a leap year. */
bool lclIsLeapYear( sal_Int32 nYear )
{
    return ((nYear % 4) == 0) && (((nYear % 100) != 0) || ((nYear % 400) == 0));
}

void lclSkipYearBlock( sal_Int32& ornDays, sal_Int16& ornYear, sal_Int32 nDaysInBlock, sal_Int32 nYearsPerBlock, sal_Int32 nMaxBlocks )
{
    sal_Int32 nBlocks = ::std::min< sal_Int32 >( ornDays / nDaysInBlock, nMaxBlocks );
    ornYear = static_cast< sal_Int16 >( ornYear + nYearsPerBlock * nBlocks );
    ornDays -= nBlocks * nDaysInBlock;
}

/** Returns the number of days before the passed date, starting from the null
    date 0000-Jan-01, using standard leap year conventions. */
sal_Int32 lclGetDays( const util::Date& rDate )
{
    // number of days in all full years before passed date including all leap days
    sal_Int32 nDays = rDate.Year * 365 + ((rDate.Year + 3) / 4) - ((rDate.Year + 99) / 100) + ((rDate.Year + 399) / 400);
    OSL_ENSURE( (1 <= rDate.Month) && (rDate.Month <= 12), "lclGetDays - invalid month" );
    OSL_ENSURE( (1 <= rDate.Day) && (rDate.Day <= 31), "lclGetDays - invalid day" );    // yes, this is weak...
    if( (1 <= rDate.Month) && (rDate.Month <= 12) )
    {
        // number of days at start of month   jan feb mar apr  may  jun  jul  aug  sep  oct  nov  dec
        static const sal_Int32 spnCumDays[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
        // add number of days in full months before passed date
        nDays += spnCumDays[ rDate.Month - 1 ];
        // add number of days from passed date (this adds one day too much)
        nDays += rDate.Day;
        /*  Remove the one day added too much if there is no leap day before
            the passed day in the passed year. This means: remove the day, if
            we are in january or february (leap day not reached if existing),
            or if the passed year is not a leap year. */
        if( (rDate.Month < 3) || !lclIsLeapYear( rDate.Year ) )
            --nDays;
    }
    return nDays;
}

} // namespace

UnitConverter::UnitConverter( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mnNullDate( lclGetDays( util::Date( 30, 12, 1899 ) ) )
{
    // initialize constant and default coefficients
    const DeviceInfo& rDeviceInfo = getBaseFilter().getGraphicHelper().getDeviceInfo();
    maCoeffs[Unit::Twip] = o3tl::convert(1.0, o3tl::Length::twip, o3tl::Length::emu);
    maCoeffs[Unit::Emu] = 1;
    maCoeffs[Unit::ScreenX] = o3tl::convert((rDeviceInfo.PixelPerMeterX > 0) ? (1000.0 / rDeviceInfo.PixelPerMeterX) : 0.5, o3tl::Length::mm, o3tl::Length::emu);
    maCoeffs[Unit::ScreenY] = o3tl::convert((rDeviceInfo.PixelPerMeterY > 0) ? (1000.0 / rDeviceInfo.PixelPerMeterY) : 0.5, o3tl::Length::mm, o3tl::Length::emu);
    maCoeffs[Unit::Digit]   = o3tl::convert(2.0, o3tl::Length::mm, o3tl::Length::emu); // default: 1 digit = 2 mm
    maCoeffs[Unit::Space]   = o3tl::convert(1.0, o3tl::Length::mm, o3tl::Length::emu); // default  1 space = 1 mm

    // error code maps
    addErrorCode( BIFF_ERR_NULL,  u"#NULL!"_ustr );
    addErrorCode( BIFF_ERR_DIV0,  u"#DIV/0!"_ustr );
    addErrorCode( BIFF_ERR_VALUE, u"#VALUE!"_ustr );
    addErrorCode( BIFF_ERR_REF,   u"#REF!"_ustr );
    addErrorCode( BIFF_ERR_NAME,  u"#NAME?"_ustr );
    addErrorCode( BIFF_ERR_NUM,   u"#NUM!"_ustr );
    addErrorCode( BIFF_ERR_NA,    u"#N/A"_ustr );
}

void UnitConverter::finalizeImport()
{
    PropertySet aDocProps(( Reference< css::beans::XPropertySet >(getDocument()) ));
    Reference< XDevice > xDevice( aDocProps.getAnyProperty( PROP_ReferenceDevice ), UNO_QUERY );
    if( !xDevice.is() )
        return;

    // get character widths from default font
    const oox::xls::Font* pDefFont = getStyles().getDefaultFont().get();
    if( !pDefFont )
        return;

    // XDevice expects pixels in font descriptor, but font contains twips
    const FontDescriptor& aDesc = pDefFont->getFontDescriptor();
    Reference< XFont > xFont = xDevice->getFont( aDesc );
    if( !xFont.is() )
        return;

    // get maximum width of all digits
    sal_Int64 nDigitWidth = 0;
    for( sal_Unicode cChar = '0'; cChar <= '9'; ++cChar )
        nDigitWidth = ::std::max(nDigitWidth, o3tl::convert(xFont->getCharWidth(cChar),
                                                            o3tl::Length::twip, o3tl::Length::emu));
    if( nDigitWidth > 0 )
        maCoeffs[ Unit::Digit ] = nDigitWidth;
    // get width of space character
    sal_Int64 nSpaceWidth
        = o3tl::convert(xFont->getCharWidth(' '), o3tl::Length::twip, o3tl::Length::emu);
    if( nSpaceWidth > 0 )
        maCoeffs[ Unit::Space ] = nSpaceWidth;
}

void UnitConverter::finalizeNullDate( const util::Date& rNullDate )
{
    // convert the nulldate to number of days since 0000-Jan-01
    mnNullDate = lclGetDays( rNullDate );
}

// conversion -----------------------------------------------------------------

double UnitConverter::scaleValue( double fValue, Unit eFromUnit, Unit eToUnit ) const
{
    return (eFromUnit == eToUnit) ? fValue : (fValue * getCoefficient( eFromUnit ) / getCoefficient( eToUnit ));
}

double UnitConverter::calcSerialFromDateTime( const util::DateTime& rDateTime ) const
{
    sal_Int32 nDays = lclGetDays( util::Date( rDateTime.Day, rDateTime.Month, rDateTime.Year ) ) - mnNullDate;
    OSL_ENSURE( nDays >= 0, "UnitConverter::calcDateTimeSerial - invalid date" );
    OSL_ENSURE( (rDateTime.Hours <= 23) && (rDateTime.Minutes <= 59) && (rDateTime.Seconds <= 59), "UnitConverter::calcDateTimeSerial - invalid time" );
    return nDays + rDateTime.Hours / 24.0 + rDateTime.Minutes / 1440.0 + rDateTime.Seconds / 86400.0;
}

util::DateTime UnitConverter::calcDateTimeFromSerial( double fSerial ) const
{
    util::DateTime aDateTime( 0, 0, 0, 0, 1, 1, 0, false );
    double fDays = 0.0;
    double fTime = modf( fSerial, &fDays );

    // calculate date from number of days with O(1) complexity
    sal_Int32 nDays = getLimitedValue< sal_Int32, double >( fDays + mnNullDate, 0, 3652424 );
    // skip year 0, assumed to be a leap year. By starting at year 1, leap years can be handled easily
    if( nDays >= 366 ) { ++aDateTime.Year; nDays -= 366; }
    // skip full blocks of 400, 100, 4 years, and remaining full years
    lclSkipYearBlock( nDays, aDateTime.Year, 400 * 365 + 97, 400, 24 );
    lclSkipYearBlock( nDays, aDateTime.Year, 100 * 365 + 24, 100, 3 );
    lclSkipYearBlock( nDays, aDateTime.Year, 4 * 365 + 1, 4, 24 );
    lclSkipYearBlock( nDays, aDateTime.Year, 365, 1, 3 );
    // skip full months of current year
    static const sal_Int32 spnDaysInMonth[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if( (nDays >= 59) && !lclIsLeapYear( aDateTime.Year ) ) ++nDays;
    const sal_Int32* pnDaysInMonth = spnDaysInMonth;
    while( *pnDaysInMonth >= nDays ) { ++aDateTime.Month; nDays -= *pnDaysInMonth; ++pnDaysInMonth; }
    aDateTime.Day = static_cast< sal_uInt16 >( nDays + 1 );

    // calculate time from fractional part of serial
    sal_Int32 nTime = getLimitedValue< sal_Int32, double >( fTime * 86400, 0, 86399 );
    aDateTime.Seconds = static_cast< sal_uInt16 >( nTime % 60 );
    nTime /= 60;
    aDateTime.Minutes = static_cast< sal_uInt16 >( nTime % 60 );
    aDateTime.Hours = static_cast< sal_uInt16 >( nTime / 60 );

    return aDateTime;
}

sal_uInt8 UnitConverter::calcBiffErrorCode( const OUString& rErrorCode ) const
{
    auto aIt = maOoxErrCodes.find( rErrorCode );
    return (aIt == maOoxErrCodes.end()) ? BIFF_ERR_NA : aIt->second;
}

OUString UnitConverter::calcErrorString( sal_uInt8 nErrorCode ) const
{
    auto iFail( maOoxErrCodes.cend());
    for (auto aIt( maOoxErrCodes.cbegin()); aIt != maOoxErrCodes.cend(); ++aIt)
    {
        if (aIt->second == nErrorCode)
            return aIt->first;
        if (aIt->second == BIFF_ERR_NA)
            iFail = aIt;
    }
    assert(iFail != maOoxErrCodes.end());   // BIFF_ERR_NA really should be in the map...
    return iFail != maOoxErrCodes.end() ? iFail->first : OUString();
}

void UnitConverter::addErrorCode( sal_uInt8 nErrorCode, const OUString& rErrorCode )
{
    maOoxErrCodes[ rErrorCode ]  = nErrorCode;
}

double UnitConverter::getCoefficient( Unit eUnit ) const
{
    return maCoeffs[ eUnit ];
}

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
