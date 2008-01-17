/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unitconverter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:09 $
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

#include "oox/xls/unitconverter.hxx"
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/DeviceInfo.hpp>
#include <com/sun/star/awt/XFont.hpp>
#include "oox/xls/stylesbuffer.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::awt::FontDescriptor;
using ::com::sun::star::awt::XDevice;
using ::com::sun::star::awt::DeviceInfo;
using ::com::sun::star::awt::XFont;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const double MM100_PER_INCH         = 2540.0;
const double INCH_PER_MM100         = 1.0 / MM100_PER_INCH;

const double POINT_PER_INCH         = 72.0;
const double INCH_PER_POINT         = 1.0 / POINT_PER_INCH;

const double MM100_PER_POINT        = MM100_PER_INCH * INCH_PER_POINT;
const double POINT_PER_MM100        = 1.0 / MM100_PER_POINT;

const double TWIP_PER_POINT         = 20.0;
const double POINT_PER_TWIP         = 1.0 / TWIP_PER_POINT;

const double MM100_PER_TWIP         = MM100_PER_POINT * POINT_PER_TWIP;
const double TWIP_PER_MM100         = 1.0 / MM100_PER_TWIP;

} // namespace

// ----------------------------------------------------------------------------

UnitConverter::UnitConverter( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mfPixelPerMm100X( 0.08 ),
    mfPixelPerMm100Y( 0.08 ),
    mnDigitWidth( 200 ),
    mnSpaceWidth( 100 )
{
    // map error code names to BIFF error codes
    maErrorCodes[ CREATE_OUSTRING( "#NULL!" ) ]  = BIFF_ERR_NULL;
    maErrorCodes[ CREATE_OUSTRING( "#DIV/0!" ) ] = BIFF_ERR_DIV0;
    maErrorCodes[ CREATE_OUSTRING( "#VALUE!" ) ] = BIFF_ERR_VALUE;
    maErrorCodes[ CREATE_OUSTRING( "#REF!" ) ]   = BIFF_ERR_REF;
    maErrorCodes[ CREATE_OUSTRING( "#NAME?" ) ]  = BIFF_ERR_NAME;
    maErrorCodes[ CREATE_OUSTRING( "#NUM!" ) ]   = BIFF_ERR_NUM;
    maErrorCodes[ CREATE_OUSTRING( "#NA" ) ]     = BIFF_ERR_NA;
}

void UnitConverter::finalizeImport()
{
    Reference< XDevice > xDevice = getReferenceDevice();
    if( xDevice.is() )
    {
        // get pixel metric first, needed to get character widths below
        DeviceInfo aInfo = xDevice->getInfo();
        mfPixelPerMm100X = aInfo.PixelPerMeterX / 100000.0;
        mfPixelPerMm100Y = aInfo.PixelPerMeterY / 100000.0;

        // get character widths from default font
        if( const Font* pDefFont = getStyles().getDefaultFont().get() )
        {
            // XDevice expects pixels in font descriptor, but font contains twips
            FontDescriptor aDesc = pDefFont->getFontDescriptor();
            aDesc.Height = static_cast< sal_Int16 >( calcPixelsXFromMm100( calcMm100FromTwips( aDesc.Height ) ) );
            Reference< XFont > xFont = xDevice->getFont( aDesc );
            if( xFont.is() )
            {
                // get maximum width of all digits
                sal_Int32 nDigitWidth = 0;
                for( sal_Unicode cChar = '0'; cChar <= '9'; ++cChar )
                    nDigitWidth = ::std::max( nDigitWidth, calcMm100FromPixelsX( xFont->getCharWidth( cChar ) ) );
                if( nDigitWidth > 0 )
                    mnDigitWidth = nDigitWidth;
                // get width of space character
                sal_Int32 nSpaceWidth = calcMm100FromPixelsX( xFont->getCharWidth( ' ' ) );
                if( nSpaceWidth > 0 )
                    mnSpaceWidth = nSpaceWidth;
            }
        }
    }
}

// conversion -----------------------------------------------------------------

sal_Int32 UnitConverter::calcMm100FromInches( double fInches ) const
{
    return static_cast< sal_Int32 >( fInches * MM100_PER_INCH );
}

sal_Int32 UnitConverter::calcMm100FromPoints( double fPoints ) const
{
    return static_cast< sal_Int32 >( fPoints * MM100_PER_POINT + 0.5 );
}

sal_Int32 UnitConverter::calcMm100FromTwips( double fTwips ) const
{
    return static_cast< sal_Int32 >( fTwips * MM100_PER_TWIP + 0.5 );
}

sal_Int32 UnitConverter::calcMm100FromPixelsX( double fPixels ) const
{
    return static_cast< sal_Int32 >( fPixels / mfPixelPerMm100X + 0.5 );
}

sal_Int32 UnitConverter::calcMm100FromPixelsY( double fPixels ) const
{
    return static_cast< sal_Int32 >( fPixels / mfPixelPerMm100Y + 0.5 );
}

sal_Int32 UnitConverter::calcMm100FromDigits( double fChars ) const
{
    return static_cast< sal_Int32 >( fChars * mnDigitWidth + 0.5 );
}

sal_Int32 UnitConverter::calcMm100FromSpaces( double fSpaces ) const
{
    return static_cast< sal_Int32 >( fSpaces * mnSpaceWidth + 0.5 );
}

double UnitConverter::calcInchesFromMm100( sal_Int32 nMm100 ) const
{
    return nMm100 * INCH_PER_MM100;
}

double UnitConverter::calcPointsFromMm100( sal_Int32 nMm100 ) const
{
    return nMm100 * POINT_PER_MM100;
}

double UnitConverter::calcTwipsFromMm100( sal_Int32 nMm100 ) const
{
    return nMm100 * TWIP_PER_MM100;
}

double UnitConverter::calcPixelsXFromMm100( sal_Int32 nMm100 ) const
{
    return nMm100 * mfPixelPerMm100X;
}

double UnitConverter::calcPixelsYFromMm100( sal_Int32 nMm100 ) const
{
    return nMm100 * mfPixelPerMm100Y;
}

double UnitConverter::calcDigitsFromMm100( sal_Int32 nMm100 ) const
{
    return static_cast< double >( nMm100 ) / mnDigitWidth;
}

double UnitConverter::calcSpacesFromMm100( sal_Int32 nMm100 ) const
{
    return static_cast< double >( nMm100 ) / mnSpaceWidth;
}

sal_uInt8 UnitConverter::calcBiffErrorCode( const OUString& rErrorCode ) const
{
    ErrorCodeMap::const_iterator aIt = maErrorCodes.find( rErrorCode );
    return (aIt == maErrorCodes.end()) ? BIFF_ERR_NA : aIt->second;
}

// ============================================================================

} // namespace xls
} // namespace oox

