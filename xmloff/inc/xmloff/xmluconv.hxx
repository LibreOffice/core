/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _XMLOFF_XMLUCONV_HXX
#define _XMLOFF_XMLUCONV_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"

#include <limits.h>
#include <tools/solar.h>
#include <tools/color.hxx>
#include <tools/mapunit.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/Position3D.hpp>

// #110680#
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class Time;
namespace rtl
{
class OUString;
class OUStringBuffer;
}

namespace com { namespace sun { namespace star {
    namespace util { struct DateTime; }
    namespace text { class XNumberingTypeInfo; }
    namespace i18n { class XCharacterClassification; }
}}}

namespace basegfx
{
    class B3DVector;
} // end of namespace basegfx

class XMLOFF_DLLPUBLIC SvXMLTokenEnumerator
{
private:
    const ::rtl::OUString&  maTokenString;
    sal_Int32               mnNextTokenPos;
    sal_Unicode             mcSeperator;

public:
    SvXMLTokenEnumerator( const ::rtl::OUString& rString, sal_Unicode cSeperator = sal_Unicode(' ') );

    sal_Bool getNextToken( ::rtl::OUString& rToken );
};

/** the SvXMLTypeConverter converts values of various types from
    their internal represantation to the textual form used in xml
    and back.
    Most of the methods are static but the SvXMLTypeConverter can
    also store default units for both numerical and textual measures.
*/

class XMLOFF_DLLPUBLIC SvXMLUnitConverter
{
private:
    MapUnit meCoreMeasureUnit;
    MapUnit meXMLMeasureUnit;
    com::sun::star::util::Date aNullDate;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XNumberingTypeInfo > xNumTypeInfo;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::i18n::XCharacterClassification > xCharClass;
    // #110680#
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxServiceFactory;

    SAL_DLLPRIVATE static ::rtl::OUString msXML_false;
    SAL_DLLPRIVATE static ::rtl::OUString msXML_true;

    SAL_DLLPRIVATE static void initXMLStrings();

    SAL_DLLPRIVATE void createNumTypeInfo() const;

public:
    /** constructs a SvXMLUnitConverter. The core measure unit is the
        default unit for numerical measures, the XML measure unit is
        the default unit for textual measures */
    // #110680#
    // SvXMLUnitConverter( MapUnit eCoreMeasureUnit, MapUnit eXMLMeasureUnit );
    SvXMLUnitConverter(
        MapUnit eCoreMeasureUnit,
        MapUnit eXMLMeasureUnit,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory );

    virtual ~SvXMLUnitConverter();

    static MapUnit GetMapUnit(sal_Int16 nFieldUnit);

    /** sets the default unit for numerical measures */
    inline void setCoreMeasureUnit( MapUnit eCoreMeasureUnit );

    /** gets the default unit for numerical measures */
    inline MapUnit getCoreMeasureUnit() const;

    /** sets the default unit for textual measures */
    void setXMLMeasureUnit( MapUnit eXMLMeasureUnit );

    /** gets the default unit for textual measures */
    MapUnit getXMLMeasureUnit() const;

    /** gets XNumberingTypeInfo */
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XNumberingTypeInfo >& getNumTypeInfo() const
    {
        if( !xNumTypeInfo.is() )
            createNumTypeInfo();
        return xNumTypeInfo;
    }

    /** convert string to measure using optional min and max values*/
    sal_Bool convertMeasure( sal_Int32& rValue,
                         const ::rtl::OUString& rString,
                         sal_Int32 nMin = SAL_MIN_INT32,
                         sal_Int32 nMax = SAL_MAX_INT32) const;

    /** convert measure to string */
    void convertMeasure( ::rtl::OUStringBuffer& rBuffer,
                         sal_Int32 nMeasure ) const;

    /** convert measure with given unit to string */
    void convertMeasure( ::rtl::OUStringBuffer&,
                         sal_Int32 nMeasure,
                         MapUnit eSrcUnit ) const;

    /** convert string to measure in given unit
        using optional min and max values */
    static sal_Bool convertMeasure( sal_Int32& rVal,
                                const ::rtl::OUString& rString,
                                MapUnit eDstUnit,
                                sal_Int32 nMin = SAL_MIN_INT32,
                                sal_Int32 nMax = SAL_MAX_INT32);

    /** convert measure in given unit to string with given unit */
    static void convertMeasure( ::rtl::OUStringBuffer& rBuffer,
                                sal_Int32 nMeasure,
                                MapUnit eSrcUnit,
                                MapUnit eDstUnit );

    /** convert string to boolean */
    static sal_Bool convertBool( bool& rBool,
                             const ::rtl::OUString& rString );

    /** convert boolean to string */
    static void convertBool( ::rtl::OUStringBuffer& rBuffer,
                             sal_Bool bValue );

    /** convert string to percent */
    static sal_Bool convertPercent( sal_Int32& rValue,
                                const ::rtl::OUString& rString );

    /** convert percent to string */
    static void convertPercent( ::rtl::OUStringBuffer& rBuffer,
                                sal_Int32 nValue );

    /** convert string to pixel measure unite */
    static sal_Bool convertMeasurePx( sal_Int32& rValue,
                                const ::rtl::OUString& rString );

    /** convert pixel measure unit to string */
    static void convertMeasurePx( ::rtl::OUStringBuffer& rBuffer,
                                sal_Int32 nValue );

    /** convert string to enum using given enum map, if the enum is
        not found in the map, this method will return false */
    static sal_Bool convertEnum( sal_uInt16& rEnum,
                             const ::rtl::OUString& rValue,
                             const SvXMLEnumMapEntry *pMap );

    /** convert string to enum using given token map, if the enum is
        not found in the map, this method will return false */
    static sal_Bool convertEnum( sal_uInt16& rEnum,
                             const ::rtl::OUString& rValue,
                             const SvXMLEnumStringMapEntry *pMap );

    /** convert enum to string using given enum map with an optional
        default token. If the enum is not found in the map,
        this method will either use the given default or return
        false if not default is set */
    static sal_Bool convertEnum( ::rtl::OUStringBuffer& rBuffer,
                                 unsigned int nValue,
                                 const SvXMLEnumMapEntry *pMap,
                                 enum ::xmloff::token::XMLTokenEnum eDefault =
                                         ::xmloff::token::XML_TOKEN_INVALID );

    /** convert enum to string using given token map with an optional
        default token. If the enum is not found in the map,
        this method will either use the given default or return
        false if not default is set */
    static sal_Bool convertEnum( ::rtl::OUStringBuffer& rBuffer,
                                 sal_uInt16 nValue,
                                 const SvXMLEnumStringMapEntry *pMap,
                                 sal_Char* pDefault = NULL );

    /** convert string to color */
    static sal_Bool convertColor( Color& rColor,
                              const ::rtl::OUString&rValue );

    /** convert color to string */
    static void convertColor( ::rtl::OUStringBuffer &rBuffer,
                              const Color& rCol );

    /** convert number to string */
    static void convertNumber( ::rtl::OUStringBuffer& rBuffer,
                               sal_Int32 nNumber );

    /** convert string to number with optional min and max values */
    static sal_Bool convertNumber( sal_Int32& rValue,
                               const ::rtl::OUString& rString,
                               sal_Int32 nMin = SAL_MIN_INT32,
                               sal_Int32 nMax = SAL_MAX_INT32);

    /** convert number to string */
    static void convertNumber64( ::rtl::OUStringBuffer& rBuffer,
                               sal_Int64 nNumber );

    /** convert string to number with optional min and max values */
    static sal_Bool convertNumber64( sal_Int64& rValue,
                               const ::rtl::OUString& rString,
                               sal_Int64 nMin = SAL_MIN_INT64,
                               sal_Int64 nMax = SAL_MAX_INT64);

    /** convert double number to string (using ::rtl::math) and DO
        convert to export MapUnit */
    void convertDouble(::rtl::OUStringBuffer& rBuffer,
        double fNumber, sal_Bool bWriteUnits) const;

    /** convert double number to string (using ::rtl::math) and
        DO convert from eSrcUnit to export MapUnit */
    static void convertDouble( ::rtl::OUStringBuffer& rBuffer,
        double fNumber, sal_Bool bWriteUnits, MapUnit eCoreUnit, MapUnit eDstUnit);

    /** convert double number to string (using ::rtl::math) without unit conversion */
    static void convertDouble( ::rtl::OUStringBuffer& rBuffer, double fNumber);

    /** convert string to double number (using ::rtl::math) and DO convert. */
    sal_Bool convertDouble(double& rValue, const ::rtl::OUString& rString, sal_Bool bLookForUnits) const;

    /** convert string to double number (using ::rtl::math) and DO convert from
        SrcUnit to DstUnit. */
    static sal_Bool convertDouble(double& rValue,
        const ::rtl::OUString& rString, MapUnit eSrcUnit, MapUnit eCoreUnit);

    /** convert string to double number (using ::rtl::math) without unit conversion */
    static sal_Bool convertDouble(double& rValue, const ::rtl::OUString& rString);

    /** get the Null Date of the XModel and set it to the UnitConverter */
    sal_Bool setNullDate (
        const com::sun::star::uno::Reference <com::sun::star::frame::XModel>& xModel);

    /** Set the Null Date of the UnitConverter */
    void setNullDate ( const com::sun::star::util::Date& aTempNullDate ) { aNullDate = aTempNullDate; }

    /** convert double to ISO Time String */
    static void convertTime( ::rtl::OUStringBuffer& rBuffer,
                                const double& fTime);

    /** convert util::DateTime to ISO Time String */
    static void convertTime( ::rtl::OUStringBuffer& rBuffer,
                                const ::com::sun::star::util::DateTime& rDateTime );

    /** convert ISO Time String to double */
    static sal_Bool convertTime( double& fTime,
                                const ::rtl::OUString& rString);

    /** convert ISO Time String to util::DateTime */
    static sal_Bool convertTime( ::com::sun::star::util::DateTime& rDateTime,
                                 const ::rtl::OUString& rString );

    /** convert double to ISO Date Time String */
    void convertDateTime( ::rtl::OUStringBuffer& rBuffer,
                                const double& fDateTime,
                                   sal_Bool bAddTimeIf0AM=sal_False) { convertDateTime(rBuffer, fDateTime, aNullDate, bAddTimeIf0AM); }

    /** convert ISO Date Time String to double */
    sal_Bool convertDateTime( double& fDateTime,
                                const ::rtl::OUString& rString) { return convertDateTime(fDateTime, rString, aNullDate); }

    /** convert double to ISO Date Time String */
    static void convertDateTime( ::rtl::OUStringBuffer& rBuffer,
                                const double& fDateTime,
                                const com::sun::star::util::Date& aNullDate,
                                   sal_Bool bAddTimeIf0AM=sal_False);
    /** convert ISO Date Time String to double */
    static sal_Bool convertDateTime( double& fDateTime,
                                const ::rtl::OUString& rString,
                                const com::sun::star::util::Date& aNullDate);

    /** converts the given time value into an ISO-conform duration string

        @param rTime
            the time value to convert. This parameter is evaluated only down to the seconds - in particular,
            "100th seconds" are ignored.
        @param nSecondsFraction
            Additional milleseconds to add to the time. Must be smaller than 1000.
            This parameter is necessary since neither <type>Time</type> nor <type scope="com::sun::star::util">Time</type>
            have a sufficient resolution to transport milliseconds.
        @see http://www.w3.org/TR/xmlschema-2/#duration
    */
    static ::rtl::OUString convertTimeDuration( const ::Time& rTime, sal_Int32 nSecondsFraction = 0 );

    /** converts the given ISO-conform duration string into a time value

        @param rTime
            the converted time value. Fractions of seconds of this object are not filled, even if present in
            the string. See <arg>nSecondsFraction</arg>

        @param pSecondsFraction
            recieves fractions of whole seconds, in milliseconds. May be <NULL/>
            This parameter is necessary since neither <type>Time</type> nor <type scope="com::sun::star::util">Time</type>
            have a sufficient resolution to transport milliseconds.
        @see http://www.w3.org/TR/xmlschema-2/#duration
    */
    static bool convertTimeDuration( const rtl::OUString& rString, ::Time& rTime, sal_Int32* pSecondsFraction = NULL );

    /** convert string to ::basegfx::B3DVector */
    static sal_Bool convertB3DVector( ::basegfx::B3DVector& rVector,
                              const ::rtl::OUString& rValue );

    /** convert B3DVector to string */
    static void convertB3DVector( ::rtl::OUStringBuffer &rBuffer,
        const ::basegfx::B3DVector& rVector );

    /** convert string to Position3D */
    sal_Bool convertPosition3D( com::sun::star::drawing::Position3D& rPosition,
                              const ::rtl::OUString& rValue );

    /** convert Position3D to string */
    void convertPosition3D( ::rtl::OUStringBuffer &rBuffer,
                              const com::sun::star::drawing::Position3D& rVector );

    /** convert util::DateTime to ISO Date String */
    static void convertDateTime( ::rtl::OUStringBuffer& rBuffer,
                                const com::sun::star::util::DateTime& rDateTime,
                                   sal_Bool bAddTimeIf0AM=sal_False );

    /** convert ISO Date String to util::DateTime */
    static sal_Bool convertDateTime( com::sun::star::util::DateTime& rDateTime,
                                     const ::rtl::OUString& rString );

    /** gets the position of the first comma after npos in the string
        rStr. Commas inside '"' pairs are not matched */
    static sal_Int32 indexOfComma( const ::rtl::OUString& rStr,
                                   sal_Int32 nPos );

    static void encodeBase64(rtl::OUStringBuffer& aStrBuffer, const com::sun::star::uno::Sequence<sal_Int8>& aPass);

    // Decode a base 64 encoded string into a sequence of bytes. The first
    // version can be used for attribute values only, bacause it does not
    // return any chars left from conversion.
    // For text submitted throgh the SAX characters call, the later method
    // must be used!
    static void decodeBase64(com::sun::star::uno::Sequence<sal_Int8>& aPass, const rtl::OUString& sBuffer);

    static sal_Int32 decodeBase64SomeChars(com::sun::star::uno::Sequence<sal_Int8>& aPass, const rtl::OUString& sBuffer);

    /** convert num-forat and num-letter-sync values to NumberingType */
    sal_Bool convertNumFormat( sal_Int16& rType,
                                const ::rtl::OUString& rNumFormat,
                               const ::rtl::OUString& rNumLetterSync,
                               sal_Bool bNumberNone = sal_False ) const;

    /** convert NumberingType to num-forat and num-letter-sync values */
    void convertNumFormat( ::rtl::OUStringBuffer& rBuffer,
                           sal_Int16 nType ) const;
    void convertNumLetterSync( ::rtl::OUStringBuffer& rBuffer,
                                  sal_Int16 nType ) const;

    static void convertPropertySet(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProps,
                        const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& aProperties);
    static void convertPropertySet(com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& rProperties,
                        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);

    static void clearUndefinedChars(rtl::OUString& rTarget, const rtl::OUString& rSource);

    ::rtl::OUString encodeStyleName( const ::rtl::OUString& rName,
                                     sal_Bool *pEncoded=0 ) const;

    /** convert an Any to string (typesafe) */
    static sal_Bool convertAny(      ::rtl::OUStringBuffer&    sValue,
                                     ::rtl::OUStringBuffer&    sType ,
                               const com::sun::star::uno::Any& aValue);

    /** convert a string to Any (typesafe) */
    static sal_Bool convertAny(      com::sun::star::uno::Any& aValue,
                               const ::rtl::OUString&          sType ,
                               const ::rtl::OUString&          sValue);
};

inline void SvXMLUnitConverter::setCoreMeasureUnit( MapUnit eCoreMeasureUnit )
{
    meCoreMeasureUnit = eCoreMeasureUnit;
}

inline MapUnit SvXMLUnitConverter::getCoreMeasureUnit() const
{
    return meCoreMeasureUnit;
}

inline void SvXMLUnitConverter::setXMLMeasureUnit( MapUnit eXMLMeasureUnit )
{
    meXMLMeasureUnit = eXMLMeasureUnit;
}

inline MapUnit SvXMLUnitConverter::getXMLMeasureUnit() const
{
    return meXMLMeasureUnit;
}

#endif  //  _XMLOFF_XMLUCONV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
