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

#include <limits.h>

#include <tools/solar.h>

#include <tools/color.hxx>

#include <vcl/mapunit.hxx>

#include <bf_xmloff/xmlement.hxx>

#include <bf_xmloff/xmltoken.hxx>

#include <com/sun/star/util/Date.hpp>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

// #110680#
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace rtl
{
class OUString;
class OUStringBuffer;
}

namespace com { namespace sun { namespace star {
    namespace util { struct DateTime; }
    namespace text { class XNumberingTypeInfo; }
}}}
namespace binfilter {
class Vector3D;

class SvXMLTokenEnumerator
{
private:
    const ::rtl::OUString&	maTokenString;
    sal_Int32				mnNextTokenPos;
    sal_Unicode				mcSeperator;

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

class SvXMLUnitConverter
{
private:
    MapUnit	meCoreMeasureUnit;
    MapUnit	meXMLMeasureUnit;
    ::com::sun::star::util::Date aNullDate;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XNumberingTypeInfo > xNumTypeInfo;

    // #110680#
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxServiceFactory;

    static ::rtl::OUString msXML_false;
    static ::rtl::OUString msXML_true;

    void createNumTypeInfo() const;

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
                         sal_Int32 nMax = SAL_MAX_INT32 ) const;

    /** convert measure to string */
    void convertMeasure( ::rtl::OUStringBuffer& rBuffer,
                         sal_Int32 nMeasure ) const;


    /** convert string to measure in given unit
        using optional min and max values */
    static sal_Bool convertMeasure( sal_Int32& rVal,
                                const ::rtl::OUString& rString,
                                MapUnit eDstUnit,
                                sal_Int32 nMin = SAL_MIN_INT32,
                                sal_Int32 nMax = SAL_MAX_INT32 );

    /** convert measure in given unit to string with given unit */
    static void convertMeasure( ::rtl::OUStringBuffer& rBuffer,
                                sal_Int32 nMeasure,
                                MapUnit eSrcUnit,
                                MapUnit eDstUnit );

    /** convert string to boolean */
    static sal_Bool convertBool( sal_Bool& rBool,
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
                                 USHORT nValue,
                                 const SvXMLEnumMapEntry *pMap,
                                 enum ::binfilter::xmloff::token::XMLTokenEnum eDefault =
                                         ::binfilter::xmloff::token::XML_TOKEN_INVALID );

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
                               sal_Int32 nMax = SAL_MAX_INT32 );

    /** convert double number to string (using ::rtl::math) and DO
        convert to export MapUnit */
    void convertDouble(::rtl::OUStringBuffer& rBuffer,
        double fNumber, BOOL bWriteUnits) const;

    /** convert double number to string (using ::rtl::math) and
        DO convert from eSrcUnit to export MapUnit */
    static void convertDouble( ::rtl::OUStringBuffer& rBuffer,
        double fNumber, BOOL bWriteUnits, MapUnit eCoreUnit, MapUnit eDstUnit);

    /** convert double number to string (using ::rtl::math) without unit conversion */
    static void convertDouble( ::rtl::OUStringBuffer& rBuffer, double fNumber);

    /** convert string to double number (using ::rtl::math) and DO convert. */
    sal_Bool convertDouble(double& rValue, const ::rtl::OUString& rString, BOOL bLookForUnits) const;

    /** convert string to double number (using ::rtl::math) and DO convert from
        SrcUnit to DstUnit. */
    static sal_Bool convertDouble(double& rValue,
        const ::rtl::OUString& rString, MapUnit eSrcUnit, MapUnit eCoreUnit);

    /** convert string to double number (using ::rtl::math) without unit conversion */
    static sal_Bool convertDouble(double& rValue, const ::rtl::OUString& rString);

    /** get the Null Date of the XModel and set it to the UnitConverter */
    sal_Bool setNullDate (
        const ::com::sun::star::uno::Reference < ::com::sun::star::frame::XModel>& xModel);

    /** Set the Null Date of the UnitConverter */
    void setNullDate ( const ::com::sun::star::util::Date& aTempNullDate ) { aNullDate = aTempNullDate; }

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
                                const double& fDateTime) { convertDateTime(rBuffer, fDateTime, aNullDate); }

    /** convert ISO Date Time String to double */
    sal_Bool convertDateTime( double& fDateTime,
                                const ::rtl::OUString& rString) { return convertDateTime(fDateTime, rString, aNullDate); }

    /** convert double to ISO Date Time String */
    static void convertDateTime( ::rtl::OUStringBuffer& rBuffer,
                                const double& fDateTime,
                                const ::com::sun::star::util::Date& aNullDate);
    /** convert ISO Date Time String to double */
    static sal_Bool convertDateTime( double& fDateTime,
                                const ::rtl::OUString& rString,
                                const ::com::sun::star::util::Date& aNullDate);

    /** convert string to vector3D */
    static sal_Bool convertVector3D( Vector3D& rVector,
                              const ::rtl::OUString& rValue );

    /** convert vector3D to string */
    static void convertVector3D( ::rtl::OUStringBuffer &rBuffer,
                              const Vector3D& rVector );

    /** convert util::DateTime to ISO Date String */
    static void convertDateTime( ::rtl::OUStringBuffer& rBuffer,
                                const ::com::sun::star::util::DateTime& rDateTime );

    /** convert ISO Date String to util::DateTime */
    static sal_Bool convertDateTime( ::com::sun::star::util::DateTime& rDateTime,
                                     const ::rtl::OUString& rString );

    /** gets the position of the first comma after npos in the string
        rStr. Commas inside '"' pairs are not matched */
    static sal_Int32 indexOfComma( const ::rtl::OUString& rStr,
                                   sal_Int32 nPos );

    static void encodeBase64( ::rtl::OUStringBuffer& aStrBuffer, const ::com::sun::star::uno::Sequence<sal_Int8>& aPass);

    // Decode a base 64 encoded string into a sequence of bytes. The first
    // version can be used for attribute values only, bacause it does not
    // return any chars left from conversion.
    // For text submitted throgh the SAX characters call, the later method
    // must be used!
    static void decodeBase64( ::com::sun::star::uno::Sequence<sal_Int8>& aPass, const ::rtl::OUString& sBuffer);

    static sal_Int32 decodeBase64SomeChars( ::com::sun::star::uno::Sequence<sal_Int8>& aPass, const ::rtl::OUString& sBuffer);

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

    static void convertPropertySet( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rProps,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& aProperties);
    static void convertPropertySet( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& rProperties,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aProps);
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

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLUCONV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
