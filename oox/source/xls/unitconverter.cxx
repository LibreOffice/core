/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unitconverter.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:07:33 $
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
const double MM100_PER_POINT        = MM100_PER_INCH / 72.0;
const double MM100_PER_TWIP         = MM100_PER_POINT / 20.0;
const double MM100_PER_EMU          = 1.0 / 360.0;

} // namespace

// ----------------------------------------------------------------------------

UnitConverter::UnitConverter( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maCoeffs( UNIT_ENUM_SIZE, 1.0 )
{
    // initialize constant and default coefficients
    maCoeffs[ UNIT_INCH ]   = MM100_PER_INCH;
    maCoeffs[ UNIT_POINT ]  = MM100_PER_POINT;
    maCoeffs[ UNIT_TWIP ]   = MM100_PER_TWIP;
    maCoeffs[ UNIT_EMU ]    = MM100_PER_EMU;
    maCoeffs[ UNIT_PIXELX ] = 12.5;                 // default: 1 px = 0.125 mm
    maCoeffs[ UNIT_PIXELY ] = 12.5;                 // default: 1 px = 0.125 mm
    maCoeffs[ UNIT_DIGIT ]  = 200.0;                // default: 1 digit = 2 mm
    maCoeffs[ UNIT_SPACE ]  = 100.0;                // default  1 space = 1 mm

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
        maCoeffs[ UNIT_PIXELX ] = 100000.0 / aInfo.PixelPerMeterX;
        maCoeffs[ UNIT_PIXELY ] = 100000.0 / aInfo.PixelPerMeterY;

        // get character widths from default font
        if( const Font* pDefFont = getStyles().getDefaultFont().get() )
        {
            // XDevice expects pixels in font descriptor, but font contains twips
            FontDescriptor aDesc = pDefFont->getFontDescriptor();
            aDesc.Height = static_cast< sal_Int16 >( scaleValue( aDesc.Height, UNIT_TWIP, UNIT_PIXELX ) + 0.5 );
            Reference< XFont > xFont = xDevice->getFont( aDesc );
            if( xFont.is() )
            {
                // get maximum width of all digits
                sal_Int32 nDigitWidth = 0;
                for( sal_Unicode cChar = '0'; cChar <= '9'; ++cChar )
                    nDigitWidth = ::std::max( nDigitWidth, scaleToMm100( xFont->getCharWidth( cChar ), UNIT_PIXELX ) );
                if( nDigitWidth > 0 )
                    maCoeffs[ UNIT_DIGIT ] = nDigitWidth;
                // get width of space character
                sal_Int32 nSpaceWidth = scaleToMm100( xFont->getCharWidth( ' ' ), UNIT_PIXELX );
                if( nSpaceWidth > 0 )
                    maCoeffs[ UNIT_SPACE ] = nSpaceWidth;
            }
        }
    }
}

// conversion -----------------------------------------------------------------

double UnitConverter::scaleValue( double fValue, Unit eFromUnit, Unit eToUnit ) const
{
    return fValue * getCoefficient( eFromUnit ) / getCoefficient( eToUnit );
}

sal_Int32 UnitConverter::scaleToMm100( double fValue, Unit eUnit ) const
{
    return static_cast< sal_Int32 >( fValue * getCoefficient( eUnit ) + 0.5 );
}

double UnitConverter::scaleFromMm100( sal_Int32 nMm100, Unit eUnit ) const
{
    return static_cast< double >( nMm100 ) / getCoefficient( eUnit );
}

sal_uInt8 UnitConverter::calcBiffErrorCode( const OUString& rErrorCode ) const
{
    ErrorCodeMap::const_iterator aIt = maErrorCodes.find( rErrorCode );
    return (aIt == maErrorCodes.end()) ? BIFF_ERR_NA : aIt->second;
}

double UnitConverter::getCoefficient( Unit eUnit ) const
{
    OSL_ENSURE( static_cast< size_t >( eUnit ) < UNIT_ENUM_SIZE, "UnitConverter::getCoefficient - invalid unit" );
    return maCoeffs[ static_cast< size_t >( eUnit ) ];
}

// ============================================================================

} // namespace xls
} // namespace oox

