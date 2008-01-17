/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unitconverter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:49 $
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

#include <map>
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

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

    /** Converts teh passed value in inches to 1/100 millimeters. */
    sal_Int32           calcMm100FromInches( double fInches ) const;
    /** Converts the passed value from points to 1/100 millimeters. */
    sal_Int32           calcMm100FromPoints( double fPoints ) const;
    /** Converts the passed value from twips to 1/100 millimeters. */
    sal_Int32           calcMm100FromTwips( double fTwips ) const;
    /** Converts the passed value from pixels in X direction to 1/100 millimeters. */
    sal_Int32           calcMm100FromPixelsX( double fPixels ) const;
    /** Converts the passed value from pixels in Y direction to 1/100 millimeters. */
    sal_Int32           calcMm100FromPixelsY( double fPixels ) const;
    /** Converts the passed value from number of digits to 1/100 millimeters. */
    sal_Int32           calcMm100FromDigits( double fChars ) const;
    /** Converts the passed value from number of spaces to 1/100 millimeters. */
    sal_Int32           calcMm100FromSpaces( double fSpaces ) const;

    /** Converts the passed value from 1/100 millimeters to inches. */
    double              calcInchesFromMm100( sal_Int32 nMm100 ) const;
    /** Converts the passed value from 1/100 millimeters to points. */
    double              calcPointsFromMm100( sal_Int32 nMm100 ) const;
    /** Converts the passed value from 1/100 millimeters to twips. */
    double              calcTwipsFromMm100( sal_Int32 nMm100 ) const;
    /** Converts the passed value from 1/100 millimeters to pixels in X direction. */
    double              calcPixelsXFromMm100( sal_Int32 nMm100 ) const;
    /** Converts the passed value from 1/100 millimeters to pixels in Y direction. */
    double              calcPixelsYFromMm100( sal_Int32 nMm100 ) const;
    /** Converts the passed value from 1/100 millimeters to number of digits. */
    double              calcDigitsFromMm100( sal_Int32 nMm100 ) const;
    /** Converts the passed value from 1/100 millimeters to number of spaces. */
    double              calcSpacesFromMm100( sal_Int32 nMm100 ) const;

    /** Returns a BIFF error code from the passed error string. */
    sal_uInt8           calcBiffErrorCode( const ::rtl::OUString& rErrorCode ) const;

private:
    typedef ::std::map< ::rtl::OUString, sal_uInt8 > ErrorCodeMap;

    ErrorCodeMap        maErrorCodes;       /// Maps error code strings to BIFF error constants.
    double              mfPixelPerMm100X;   /// Number of pixels per 1/100 mm in X direction in reference device.
    double              mfPixelPerMm100Y;   /// Number of pixels per 1/100 mm in Y direction in reference device.
    sal_Int32           mnDigitWidth;       /// Width of a digit using default font in 1/100 mm.
    sal_Int32           mnSpaceWidth;       /// Width of a space using default font in 1/100 mm.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

