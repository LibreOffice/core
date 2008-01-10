/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: converter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 12:50:44 $
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

#ifndef _SAX_CONVERTER_HXX
#define _SAX_CONVERTER_HXX

#include <sal/types.h>

#ifndef INCLUDED_SAX_DLLAPI_H
#include "sax/dllapi.h"
#endif

#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_MEASUREUNIT_HPP_
#include <com/sun/star/util/MeasureUnit.hpp>
#endif

namespace rtl
{
class OUString;
class OUStringBuffer;
}

namespace com { namespace sun { namespace star {
    namespace util { struct DateTime; }
}}}

namespace sax {

/** the Converter converts values of various types from
    their internal represantation to the textual form used in xml
    and back.

    All unit types are expressed as com::sun::star::util::MeasureUnit
*/


class SAX_DLLPUBLIC Converter
{
public:
    /** convert string to measure using optional min and max values*/
    static bool convertMeasure( sal_Int32& rValue,
                                const ::rtl::OUString& rString,
                                sal_Int16 nTargetUnit = ::com::sun::star::util::MeasureUnit::MM_100TH,
                                sal_Int32 nMin = SAL_MIN_INT32,
                                sal_Int32 nMax = SAL_MAX_INT32 );

    /** convert measure to string */
    static void convertMeasure( ::rtl::OUStringBuffer& rBuffer,
                                sal_Int32 nMeasure,
                                sal_Int16 SourceUnit = ::com::sun::star::util::MeasureUnit::MM_100TH,
                                sal_Int16 nTargetUnit = ::com::sun::star::util::MeasureUnit::INCH  );

    /** convert string to boolean */
    static bool convertBool( bool& rBool,
                             const ::rtl::OUString& rString );

    /** convert boolean to string */
    static void convertBool( ::rtl::OUStringBuffer& rBuffer,
                             bool bValue );

    /** convert string to percent */
    static bool convertPercent( sal_Int32& rValue,
                                const ::rtl::OUString& rString );

    /** convert percent to string */
    static void convertPercent( ::rtl::OUStringBuffer& rBuffer,
                                sal_Int32 nValue );

    /** convert string to pixel measure unite */
    static bool convertMeasurePx( sal_Int32& rValue,
                                  const ::rtl::OUString& rString );

    /** convert pixel measure unit to string */
    static void convertMeasurePx( ::rtl::OUStringBuffer& rBuffer,
                                  sal_Int32 nValue );

    /** convert string to color */
    static bool convertColor( sal_Int32& rColor,
                              const ::rtl::OUString&rValue );

    /** convert color to string */
    static void convertColor( ::rtl::OUStringBuffer &rBuffer,
                              sal_Int32 nColor );

    /** convert number to string */
    static void convertNumber( ::rtl::OUStringBuffer& rBuffer,
                               sal_Int32 nNumber );

    /** convert string to number with optional min and max values */
    static bool convertNumber( sal_Int32& rValue,
                               const ::rtl::OUString& rString,
                               sal_Int32 nMin = SAL_MIN_INT32,
                               sal_Int32 nMax = SAL_MAX_INT32 );

    /** convert double number to string (using ::rtl::math) and
        DO convert from source unit to target unit */
    static void convertDouble( ::rtl::OUStringBuffer& rBuffer,
                               double fNumber,
                               bool bWriteUnits,
                               sal_Int16 nSourceUnit,
                               sal_Int16 nTargetUnit );

    /** convert double number to string (using ::rtl::math) without unit conversion */
    static void convertDouble( ::rtl::OUStringBuffer& rBuffer, double fNumber);

    /** convert string to double number (using ::rtl::math) and DO convert from
        source unit to target unit. */
    static bool convertDouble(  double& rValue,
                                const ::rtl::OUString& rString,
                                sal_Int16 nSourceUnit,
                                sal_Int16 nTargetUnit );

    /** convert string to double number (using ::rtl::math) without unit conversion */
    static bool convertDouble(double& rValue, const ::rtl::OUString& rString);

    /** convert string to double number (using ::rtl::math) with unit conversion */
    static bool convertDouble(double& rValue, const ::rtl::OUString& rString, sal_Int16 nTargetUnit );

    /** convert double to ISO Time String */
    static void convertTime( ::rtl::OUStringBuffer& rBuffer,
                                const double& fTime);

    /** convert util::DateTime to ISO Time String */
    static void convertTime( ::rtl::OUStringBuffer& rBuffer,
                                const ::com::sun::star::util::DateTime& rDateTime );

    /** convert ISO Time String to double */
    static bool convertTime( double& fTime,
                                const ::rtl::OUString& rString);

    /** convert ISO Time String to util::DateTime */
    static bool convertTime( ::com::sun::star::util::DateTime& rDateTime,
                                 const ::rtl::OUString& rString );

    /** convert util::DateTime to ISO Date String */
    static void convertDateTime( ::rtl::OUStringBuffer& rBuffer,
                                const com::sun::star::util::DateTime& rDateTime,
                                   bool bAddTimeIf0AM = false );

    /** convert ISO Date String to util::DateTime */
    static bool convertDateTime( com::sun::star::util::DateTime& rDateTime,
                                 const ::rtl::OUString& rString );

    /** gets the position of the first comma after npos in the string
        rStr. Commas inside '"' pairs are not matched */
    static sal_Int32 indexOfComma( const ::rtl::OUString& rStr,
                                   sal_Int32 nPos );

    /** encodes the given byte sequence into Base64 */
    static void encodeBase64(rtl::OUStringBuffer& aStrBuffer, const com::sun::star::uno::Sequence<sal_Int8>& aPass);

    // Decode a base 64 encoded string into a sequence of bytes. The first
    // version can be used for attribute values only, bacause it does not
    // return any chars left from conversion.
    // For text submitted throgh the SAX characters call, the later method
    // must be used!
    static void decodeBase64(com::sun::star::uno::Sequence<sal_Int8>& aPass, const rtl::OUString& sBuffer);

    static sal_Int32 decodeBase64SomeChars(com::sun::star::uno::Sequence<sal_Int8>& aPass, const rtl::OUString& sBuffer);

    static void clearUndefinedChars(rtl::OUString& rTarget, const rtl::OUString& rSource);

    static double GetConversionFactor(::rtl::OUStringBuffer& rUnit, sal_Int16 nSourceUnit, sal_Int16 nTargetUnit);
    static sal_Int16 GetUnitFromString(const ::rtl::OUString& rString, sal_Int16 nDefaultUnit);

};

}

#endif  //  _SAX_CONVERTER_HXX
