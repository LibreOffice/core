/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unitconverter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:09:13 $
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

#ifndef OOX_XLS_UNITCONVERTER_HXX
#define OOX_XLS_UNITCONVERTER_HXX

#include <vector>
#include <map>
#include "oox/xls/workbookhelper.hxx"

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
    UNIT_PIXELX,        /// Horizontal screen pixels.
    UNIT_PIXELY,        /// Vertical screen pixels.
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

    /** Converts the passed value between the passed units. */
    double              scaleValue( double fValue, Unit eFromUnit, Unit eToUnit ) const;

    /** Converts the passed value to 1/100 millimeters. */
    sal_Int32           scaleToMm100( double fValue, Unit eUnit ) const;
    /** Converts the passed value from 1/100 millimeters to the passed unit. */
    double              scaleFromMm100( sal_Int32 nMm100, Unit eUnit ) const;

    /** Returns a BIFF error code from the passed error string. */
    sal_uInt8           calcBiffErrorCode( const ::rtl::OUString& rErrorCode ) const;

private:
    /** Returns the conversion coefficient for the passed unit. */
    double              getCoefficient( Unit eUnit ) const;

private:
    typedef ::std::vector< double >                     DoubleVector;
    typedef ::std::map< ::rtl::OUString, sal_uInt8 >    ErrorCodeMap;

    DoubleVector        maCoeffs;           /// Coefficients for unit conversion.
    ErrorCodeMap        maErrorCodes;       /// Maps error code strings to BIFF error constants.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

