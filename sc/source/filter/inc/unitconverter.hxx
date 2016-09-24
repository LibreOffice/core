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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_UNITCONVERTER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_UNITCONVERTER_HXX

#include <map>
#include <vector>
#include "workbookhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace util { struct Date; struct DateTime; }
} } }

namespace oox {
namespace xls {

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
    unformatted cells in the document. To be able to calculate the character
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
    void                finalizeNullDate( const css::util::Date& rNullDate );

    /** Converts the passed value between the passed units. */
    double              scaleValue( double fValue, Unit eFromUnit, Unit eToUnit ) const;

    /** Converts the passed value to 1/100 millimeters. */
    sal_Int32           scaleToMm100( double fValue, Unit eUnit ) const;
    /** Converts the passed value from 1/100 millimeters to the passed unit. */
    double              scaleFromMm100( sal_Int32 nMm100, Unit eUnit ) const;

    /** Returns the serial value of the passed datetime, based on current nulldate. */
    double              calcSerialFromDateTime( const css::util::DateTime& rDateTime ) const;
    /** Returns the datetime of the passed serial value, based on current nulldate. */
    css::util::DateTime calcDateTimeFromSerial( double fSerial ) const;

    /** Returns a BIFF error code from the passed error string. */
    sal_uInt8           calcBiffErrorCode( const OUString& rErrorCode ) const;

    /** Returns an error string from the passed BIFF error code. */
    OUString            calcErrorString( sal_uInt8 nErrorCode ) const;

    /** Returns the conversion coefficient for the passed unit. */
    double              getCoefficient( Unit eUnit ) const;

private:
    /** Adds an error code to the internal maps. */
    void                addErrorCode( sal_uInt8 nErrorCode, const OUString& rErrorCode );

private:
    typedef ::std::vector< double >                     DoubleVector;
    typedef ::std::map< OUString, sal_uInt8 >    OoxErrorCodeMap;
    typedef ::std::map< sal_uInt8, OUString >    BiffErrorCodeMap;

    DoubleVector        maCoeffs;           /// Coefficients for unit conversion.
    OoxErrorCodeMap     maOoxErrCodes;      /// Maps error code strings to BIFF error constants.
    sal_Int32           mnNullDate;         /// Nulldate of this workbook (number of days since 0000-01-01).
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
