/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pcrcommon.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 12:01:23 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_
#include "pcrcommon.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif
#ifndef EXTENSIONS_PROPRESID_HRC
#include "propresid.hrc"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UTIL_MEASUREUNIT_HPP_
#include <com/sun/star/util/MeasureUnit.hpp>
#endif
/** === end UNO includes === **/

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star::util;

    //====================================================================
    //= file-local helpers
    //====================================================================
    namespace
    {
        enum UnitConversionDirection
        {
            FieldUnitToMeasurementUnit,
            MeasurementUnitToFieldUnit
        };

        sal_Int16 convertMeasurementUnit( sal_Int16 _nUnit, UnitConversionDirection eDirection, sal_Int16& _rFieldToUNOValueFactor )
        {
            static struct _unit_table
            {
                FieldUnit eFieldUnit;
                sal_Int16 nMeasurementUnit;
                sal_Int16 nFieldToMeasureFactor;
            } aUnits[] = {
                { FUNIT_NONE,       -1 },
                { FUNIT_MM,         MeasureUnit::MM,            1 },    // must precede MM_10TH
                { FUNIT_MM,         MeasureUnit::MM_10TH,       10 },
                { FUNIT_100TH_MM,   MeasureUnit::MM_100TH,      1 },
                { FUNIT_CM,         MeasureUnit::CM,            1 },
                { FUNIT_M,          MeasureUnit::M,             1 },
                { FUNIT_KM,         MeasureUnit::KM,            1 },
                { FUNIT_TWIP,       MeasureUnit::TWIP,          1 },
                { FUNIT_POINT,      MeasureUnit::POINT,         1 },
                { FUNIT_PICA,       MeasureUnit::PICA,          1 },
                { FUNIT_INCH,       MeasureUnit::INCH,          1 },    // must precede INCH_*TH
                { FUNIT_INCH,       MeasureUnit::INCH_10TH,     10 },
                { FUNIT_INCH,       MeasureUnit::INCH_100TH,    100 },
                { FUNIT_INCH,       MeasureUnit::INCH_1000TH,   1000 },
                { FUNIT_FOOT,       MeasureUnit::FOOT,          1 },
                { FUNIT_MILE,       MeasureUnit::MILE,          1 },
            };
            for ( size_t i = 0; i < sizeof( aUnits ) / sizeof( aUnits[0] ); ++i )
            {
                if ( eDirection == FieldUnitToMeasurementUnit )
                {
                    if ( ( aUnits[ i ].eFieldUnit == (FieldUnit)_nUnit ) && ( aUnits[ i ].nFieldToMeasureFactor == _rFieldToUNOValueFactor ) )
                        return aUnits[ i ].nMeasurementUnit;
                }
                else
                {
                    if ( aUnits[ i ].nMeasurementUnit == _nUnit )
                    {
                        _rFieldToUNOValueFactor = aUnits[ i ].nFieldToMeasureFactor;
                        return (sal_Int16)aUnits[ i ].eFieldUnit;
                    }
                }
            }
            if ( eDirection == FieldUnitToMeasurementUnit )
                return -1;

            _rFieldToUNOValueFactor = 1;
            return (sal_Int16)FUNIT_NONE;
        }
    }

    //========================================================================
    //= HelpIdUrl
    //========================================================================
    //------------------------------------------------------------------------
    sal_uInt32 HelpIdUrl::getHelpId( const ::rtl::OUString& _rHelpURL )
    {
        sal_uInt32 nHelpId = 0;
        if ( 0 == _rHelpURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "HID:" ) ) )
            nHelpId = _rHelpURL.copy( sizeof( "HID:" ) - 1 ).toInt32();
        return nHelpId;
    }

    //------------------------------------------------------------------------
    ::rtl::OUString HelpIdUrl::getHelpURL( sal_uInt32 _nHelpId )
    {
        ::rtl::OUStringBuffer aBuffer;
        aBuffer.appendAscii( "HID:" );
        aBuffer.append( (sal_Int32)_nHelpId );
        return aBuffer.makeStringAndClear();
    }
    //========================================================================
    //= MeasurementUnitConversion
    //========================================================================
    //------------------------------------------------------------------------
    sal_Int16 MeasurementUnitConversion::convertToMeasurementUnit( FieldUnit _nFieldUnit, sal_Int16 _nUNOToFieldValueFactor )
    {
        return convertMeasurementUnit( (sal_Int16)_nFieldUnit, FieldUnitToMeasurementUnit, _nUNOToFieldValueFactor );
    }

    //------------------------------------------------------------------------
    FieldUnit MeasurementUnitConversion::convertToFieldUnit( sal_Int16 _nMeasurementUnit, sal_Int16& _rFieldToUNOValueFactor )
    {
        return (FieldUnit)convertMeasurementUnit( _nMeasurementUnit, MeasurementUnitToFieldUnit, _rFieldToUNOValueFactor );
    }

//............................................................................
} // namespace pcr
//............................................................................

