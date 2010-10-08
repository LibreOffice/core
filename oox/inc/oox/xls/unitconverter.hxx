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

#ifndef OOX_XLS_UNITCONVERTER_HXX
#define OOX_XLS_UNITCONVERTER_HXX

#include <vector>
#include <map>
#include "oox/xls/workbookhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace util { struct Date; struct DateTime; }
} } }

namespace oox {
namespace xls {

// ============================================================================

/** Units supported by the UnitConverter class. */
enum Unit
{
    UNIT_INCH,          /// Inches.
    UNIT_POINT,         /// Points.
    UNIT_TWIP,          /// Twips (1/20 point).
    UNIT_EMU,           /// English Metric Unit (1/360,000 cm).
    UNIT_SCREENX,       /// Horizontal screen pixels.
    UNIT_SCREENY,       /// Vertical screen pixels.
    UNIT_REFDEVX,       /// Horizontal pixels in Calc reference device.
    UNIT_REFDEVY,       /// Vertical pixels in Calc reference device.
    UNIT_DIGIT,         /// Digit width of document default font.
    UNIT_SPACE,         /// Space character width of document default font.

    UNIT_ENUM_SIZE
};

/** Helper class that provides functions to convert values from and to
    different units.

    Provides functions to calculate the width of certain characters of the
    default font of the imported/exported document. The default font is always
    the first font in the styles font list, and is always referenced by the
    default cell style ("Normal" style in Excel) which is used by all empty
    unformatted cells in the document. To be able to calculate the charcter
    width correctly, the default font must be known, which is the case after
    the finalizeImport() or finalizeExport() functions have been called. Caller
    must make sure to not call the character width conversion functions before.
 */
class UnitConverter : public WorkbookHelper
{
public:
    explicit            UnitConverter( const WorkbookHelper& rHelper );

    /** Final processing after import of all style settings. */
    void                finalizeImport();
    /** Updates internal nulldate for date/serial conversion. */
    void                finalizeNullDate( const ::com::sun::star::util::Date& rNullDate );

    /** Converts the passed value between the passed units. */
    double              scaleValue( double fValue, Unit eFromUnit, Unit eToUnit ) const;

    /** Converts the passed value to 1/100 millimeters. */
    sal_Int32           scaleToMm100( double fValue, Unit eUnit ) const;
    /** Converts the passed value from 1/100 millimeters to the passed unit. */
    double              scaleFromMm100( sal_Int32 nMm100, Unit eUnit ) const;

    /** Returns the serial value of the passed datetime, based on current nulldate. */
    double              calcSerialFromDateTime( const ::com::sun::star::util::DateTime& rDateTime ) const;
    /** Returns the datetime of the passed serial value, based on current nulldate. */
    ::com::sun::star::util::DateTime calcDateTimeFromSerial( double fSerial ) const;

    /** Returns an error string from the passed BIFF error code. */
    ::rtl::OUString     calcOoxErrorCode( sal_uInt8 nErrorCode ) const;
    /** Returns a BIFF error code from the passed error string. */
    sal_uInt8           calcBiffErrorCode( const ::rtl::OUString& rErrorCode ) const;

private:
    /** Adds an error code to the internal maps. */
    void                addErrorCode( sal_uInt8 nErrorCode, const ::rtl::OUString& rErrorCode );
    /** Returns the conversion coefficient for the passed unit. */
    double              getCoefficient( Unit eUnit ) const;

private:
    typedef ::std::vector< double >                     DoubleVector;
    typedef ::std::map< ::rtl::OUString, sal_uInt8 >    OoxErrorCodeMap;
    typedef ::std::map< sal_uInt8, ::rtl::OUString >    BiffErrorCodeMap;

    DoubleVector        maCoeffs;           /// Coefficients for unit conversion.
    OoxErrorCodeMap     maOoxErrCodes;      /// Maps error code strings to BIFF error constants.
    BiffErrorCodeMap    maBiffErrCodes;     /// Maps BIFF error constants to error code strings.
    sal_Int32           mnNullDate;         /// Nulldate of this workbook (number of days since 0000-01-01).
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

