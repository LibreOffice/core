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

#include <memory>
#include <limits.h>
#include <boost/utility.hpp>

#include <tools/solar.h>
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
    : private ::boost::noncopyable
{
private:
    struct Impl;
    ::std::auto_ptr<Impl> m_pImpl;

public:
    /** constructs a SvXMLUnitConverter. The core measure unit is the
        default unit for numerical measures, the XML measure unit is
        the default unit for textual measures */
    SvXMLUnitConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
        sal_Int16 eCoreMeasureUnit,
        sal_Int16 eXMLMeasureUnit);

    virtual ~SvXMLUnitConverter();

    static sal_Int16 GetMeasureUnit(sal_Int16 const nFieldUnit);

    /** sets the default unit for numerical measures */
    void SetCoreMeasureUnit( sal_Int16 const eCoreMeasureUnit );

    /** sets the default unit for textual measures */
    void SetXMLMeasureUnit( sal_Int16 const eXMLMeasureUnit );

    /** gets the default unit for textual measures */
    sal_Int16 GetXMLMeasureUnit() const;

    /** gets XNumberingTypeInfo */
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XNumberingTypeInfo >& getNumTypeInfo() const;

    /** convert string to measure with meCoreMeasureUnit,
        using optional min and max values*/
    bool convertMeasureToCore( sal_Int32& rValue,
                         const ::rtl::OUString& rString,
                         sal_Int32 nMin = SAL_MIN_INT32,
                         sal_Int32 nMax = SAL_MAX_INT32) const;

    /** convert measure to string: from meCoreMeasureUnit to meXMLMeasureUnit */
    void convertMeasureToXML( ::rtl::OUStringBuffer& rBuffer,
                         sal_Int32 nMeasure ) const;

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

    /** convert double number to string (using ::rtl::math) and DO
		convert to export MapUnit using meCoreMeasureUnit/meXMLMeasureUnit */
    void convertDouble(::rtl::OUStringBuffer& rBuffer,
        double fNumber, sal_Bool bWriteUnits) const;

    /** convert string to double number (using ::rtl::math) and DO convert. */
    sal_Bool convertDouble(double& rValue, const ::rtl::OUString& rString, sal_Bool bLookForUnits) const;

    /** get the Null Date of the XModel and set it to the UnitConverter */
    sal_Bool setNullDate (
        const com::sun::star::uno::Reference <com::sun::star::frame::XModel>& xModel);

    /** convert double to ISO Date Time String */
    void convertDateTime( ::rtl::OUStringBuffer& rBuffer,
                                const double& fDateTime,
                                bool const bAddTimeIf0AM = false);

    /** convert ISO Date Time String to double */
    bool convertDateTime(double& fDateTime,
                         const ::rtl::OUString& rString);

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
    /** convert string (hex) to number (sal_uInt32) */
    static sal_Bool convertHex( sal_uInt32& nVal,
                              const ::rtl::OUString& rValue );

    /** convert number (sal_uInt32) to string (hex) */
    static void convertHex( ::rtl::OUStringBuffer& rBuffer,
                               sal_uInt32 nVal );

};

#endif  //  _XMLOFF_XMLUCONV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
