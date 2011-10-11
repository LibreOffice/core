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

    @attention:
        a lot of the methods here have been moved to <sax/tools/converter.hxx>!
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


    /** convert double number to string (using ::rtl::math) and DO
		convert to export MapUnit using meCoreMeasureUnit/meXMLMeasureUnit */
    void convertDouble(::rtl::OUStringBuffer& rBuffer,
        double fNumber, sal_Bool bWriteUnits) const;

    /** convert string to double number (using ::rtl::math) and DO convert. */
    sal_Bool convertDouble(double& rValue, const ::rtl::OUString& rString, sal_Bool bLookForUnits) const;

    /** get the Null Date of the XModel and set it to the UnitConverter */
    sal_Bool setNullDate (
        const com::sun::star::uno::Reference <com::sun::star::frame::XModel>& xModel);

    /** Set the Null Date of the UnitConverter */
    void setNullDate ( const com::sun::star::util::Date& aTempNullDate ) { aNullDate = aTempNullDate; }

    /** convert double to ISO Date Time String */
    void convertDateTime( ::rtl::OUStringBuffer& rBuffer,
                                const double& fDateTime,
                                   sal_Bool bAddTimeIf0AM=sal_False) { convertDateTime(rBuffer, fDateTime, aNullDate, bAddTimeIf0AM); }

    /** convert ISO Date Time String to double */
    sal_Bool convertDateTime( double& fDateTime,
                                const ::rtl::OUString& rString) { return convertDateTime(fDateTime, rString, aNullDate); }

    /// these 2 functions use tools Date, so they're not yet moved to sax

    /** convert double to ISO Date Time String */
    static void convertDateTime( ::rtl::OUStringBuffer& rBuffer,
                                const double& fDateTime,
                                const com::sun::star::util::Date& aNullDate,
                                   sal_Bool bAddTimeIf0AM=sal_False);
    /** convert ISO Date Time String to double */
    static sal_Bool convertDateTime( double& fDateTime,
                                const ::rtl::OUString& rString,
                                const com::sun::star::util::Date& aNullDate);


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

    ::rtl::OUString encodeStyleName( const ::rtl::OUString& rName,
                                     sal_Bool *pEncoded=0 ) const;

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
