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

#include "unitconverter.hxx"

#include <com/sun/star/awt/DeviceInfo.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XFont.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <rtl/math.hxx>
#include "oox/core/filterbase.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/token/properties.hxx"
#include "stylesbuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;


// ============================================================================

namespace {

const double MM100_PER_INCH         = 2540.0;
const double MM100_PER_POINT        = MM100_PER_INCH / 72.0;
const double MM100_PER_TWIP         = MM100_PER_POINT / 20.0;
const double MM100_PER_EMU          = 1.0 / 360.0;

// ----------------------------------------------------------------------------

/** Returns true, if the passed year is a leap year. */
inline sal_Int32 lclIsLeapYear( sal_Int32 nYear )
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

// ----------------------------------------------------------------------------

UnitConverter::UnitConverter( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maCoeffs( UNIT_ENUM_SIZE, 1.0 ),
    mnNullDate( lclGetDays( util::Date( 30, 12, 1899 ) ) )
{
    // initialize constant and default coefficients
    const DeviceInfo& rDeviceInfo = getBaseFilter().getGraphicHelper().getDeviceInfo();
    maCoeffs[ UNIT_INCH ]    = MM100_PER_INCH;
    maCoeffs[ UNIT_POINT ]   = MM100_PER_POINT;
    maCoeffs[ UNIT_TWIP ]    = MM100_PER_TWIP;
    maCoeffs[ UNIT_EMU ]     = MM100_PER_EMU;
    maCoeffs[ UNIT_SCREENX ] = (rDeviceInfo.PixelPerMeterX > 0) ? (100000.0 / rDeviceInfo.PixelPerMeterX) : 50.0;
    maCoeffs[ UNIT_SCREENY ] = (rDeviceInfo.PixelPerMeterY > 0) ? (100000.0 / rDeviceInfo.PixelPerMeterY) : 50.0;
    maCoeffs[ UNIT_REFDEVX ] = 12.5;                 // default: 1 px = 0.125 mm
    maCoeffs[ UNIT_REFDEVY ] = 12.5;                 // default: 1 px = 0.125 mm
    maCoeffs[ UNIT_DIGIT ]   = 200.0;                // default: 1 digit = 2 mm
    maCoeffs[ UNIT_SPACE ]   = 100.0;                // default  1 space = 1 mm

    // error code maps
    addErrorCode( BIFF_ERR_NULL,  "#NULL!" );
    addErrorCode( BIFF_ERR_DIV0,  "#DIV/0!" );
    addErrorCode( BIFF_ERR_VALUE, "#VALUE!" );
    addErrorCode( BIFF_ERR_REF,   "#REF!" );
    addErrorCode( BIFF_ERR_NAME,  "#NAME?" );
    addErrorCode( BIFF_ERR_NUM,   "#NUM!" );
    addErrorCode( BIFF_ERR_NA,    "#NA" );
}

void UnitConverter::finalizeImport()
{
    PropertySet aDocProps( getDocument() );
    Reference< XDevice > xDevice( aDocProps.getAnyProperty( PROP_ReferenceDevice ), UNO_QUERY );
    if( xDevice.is() )
    {
        // get reference device metric first, needed to get character widths below
        DeviceInfo aInfo = xDevice->getInfo();
        maCoeffs[ UNIT_REFDEVX ] = 100000.0 / aInfo.PixelPerMeterX;
        maCoeffs[ UNIT_REFDEVY ] = 100000.0 / aInfo.PixelPerMeterY;

        // get character widths from default font
        if( const Font* pDefFont = getStyles().getDefaultFont().get() )
        {
            // XDevice expects pixels in font descriptor, but font contains twips
            FontDescriptor aDesc = pDefFont->getFontDescriptor();
            Reference< XFont > xFont = xDevice->getFont( aDesc );
            if( xFont.is() )
            {
                // get maximum width of all digits
                sal_Int32 nDigitWidth = 0;
                for( sal_Unicode cChar = '0'; cChar <= '9'; ++cChar )
                    nDigitWidth = ::std::max( nDigitWidth, scaleToMm100( xFont->getCharWidth( cChar ), UNIT_TWIP ) );
                if( nDigitWidth > 0 )
                    maCoeffs[ UNIT_DIGIT ] = nDigitWidth;
                // get width of space character
                sal_Int32 nSpaceWidth = scaleToMm100( xFont->getCharWidth( ' ' ), UNIT_TWIP );
                if( nSpaceWidth > 0 )
                    maCoeffs[ UNIT_SPACE ] = nSpaceWidth;
            }
        }
    }
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

sal_Int32 UnitConverter::scaleToMm100( double fValue, Unit eUnit ) const
{
    return static_cast< sal_Int32 >( fValue * getCoefficient( eUnit ) + 0.5 );
}

double UnitConverter::scaleFromMm100( sal_Int32 nMm100, Unit eUnit ) const
{
    return static_cast< double >( nMm100 ) / getCoefficient( eUnit );
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
    OoxErrorCodeMap::const_iterator aIt = maOoxErrCodes.find( rErrorCode );
    return (aIt == maOoxErrCodes.end()) ? BIFF_ERR_NA : aIt->second;
}

void UnitConverter::addErrorCode( sal_uInt8 nErrorCode, const OUString& rErrorCode )
{
    maOoxErrCodes[ rErrorCode ]  = nErrorCode;
}

double UnitConverter::getCoefficient( Unit eUnit ) const
{
    OSL_ENSURE( static_cast< size_t >( eUnit ) < UNIT_ENUM_SIZE, "UnitConverter::getCoefficient - invalid unit" );
    return maCoeffs[ static_cast< size_t >( eUnit ) ];
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
