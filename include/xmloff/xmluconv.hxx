/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _XMLOFF_XMLUCONV_HXX
#define _XMLOFF_XMLUCONV_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"

#include <memory>
#include <limits.h>
#include <boost/utility.hpp>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/solar.h>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/Position3D.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class Time;

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
    const OUString&  maTokenString;
    sal_Int32               mnNextTokenPos;
    sal_Unicode             mcSeperator;

public:
    SvXMLTokenEnumerator( const OUString& rString, sal_Unicode cSeperator = sal_Unicode(' ') );

    sal_Bool getNextToken( OUString& rToken );
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
            ::com::sun::star::uno::XComponentContext >& xContext,
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
                         const OUString& rString,
                         sal_Int32 nMin = SAL_MIN_INT32,
                         sal_Int32 nMax = SAL_MAX_INT32) const;

    /** convert measure to string: from meCoreMeasureUnit to meXMLMeasureUnit */
    void convertMeasureToXML( OUStringBuffer& rBuffer,
                         sal_Int32 nMeasure ) const;

    /** convert string to enum using given enum map, if the enum is
        not found in the map, this method will return false */
    static sal_Bool convertEnum( sal_uInt16& rEnum,
                             const OUString& rValue,
                             const SvXMLEnumMapEntry *pMap );

    /** convert string to enum using given token map, if the enum is
        not found in the map, this method will return false */
    static sal_Bool convertEnum( sal_uInt16& rEnum,
                             const OUString& rValue,
                             const SvXMLEnumStringMapEntry *pMap );

    /** convert enum to string using given enum map with an optional
        default token. If the enum is not found in the map,
        this method will either use the given default or return
        false if not default is set */
    static sal_Bool convertEnum( OUStringBuffer& rBuffer,
                                 unsigned int nValue,
                                 const SvXMLEnumMapEntry *pMap,
                                 enum ::xmloff::token::XMLTokenEnum eDefault =
                                         ::xmloff::token::XML_TOKEN_INVALID );

    /** convert double number to string (using ::rtl::math) and DO
		convert to export MapUnit using meCoreMeasureUnit/meXMLMeasureUnit */
    void convertDouble(OUStringBuffer& rBuffer,
        double fNumber, sal_Bool bWriteUnits) const;

    /** convert string to double number (using ::rtl::math) and DO convert. */
    sal_Bool convertDouble(double& rValue, const OUString& rString, sal_Bool bLookForUnits) const;

    /** get the Null Date of the XModel and set it to the UnitConverter */
    sal_Bool setNullDate (
        const com::sun::star::uno::Reference <com::sun::star::frame::XModel>& xModel);

    /** convert double to ISO Date Time String */
    void convertDateTime( OUStringBuffer& rBuffer,
                                const double& fDateTime,
                                bool const bAddTimeIf0AM = false);

    /** convert ISO Date Time String to double */
    bool convertDateTime(double& fDateTime,
                         const OUString& rString);

    /// these 2 functions use tools Date, so they're not yet moved to sax

    /** convert double to ISO Date Time String */
    static void convertDateTime( OUStringBuffer& rBuffer,
                                const double& fDateTime,
                                const com::sun::star::util::Date& aNullDate,
                                   sal_Bool bAddTimeIf0AM=sal_False);
    /** convert ISO Date Time String to double */
    static sal_Bool convertDateTime( double& fDateTime,
                                const OUString& rString,
                                const com::sun::star::util::Date& aNullDate);


    /** convert string to ::basegfx::B3DVector */
    static sal_Bool convertB3DVector( ::basegfx::B3DVector& rVector,
                              const OUString& rValue );

    /** convert B3DVector to string */
    static void convertB3DVector( OUStringBuffer &rBuffer,
        const ::basegfx::B3DVector& rVector );

    /** convert string to Position3D */
    sal_Bool convertPosition3D( com::sun::star::drawing::Position3D& rPosition,
                              const OUString& rValue );

    /** convert Position3D to string */
    void convertPosition3D( OUStringBuffer &rBuffer,
                              const com::sun::star::drawing::Position3D& rVector );


    /** convert num-forat and num-letter-sync values to NumberingType */
    sal_Bool convertNumFormat( sal_Int16& rType,
                                const OUString& rNumFormat,
                               const OUString& rNumLetterSync,
                               sal_Bool bNumberNone = sal_False ) const;

    /** convert NumberingType to num-forat and num-letter-sync values */
    void convertNumFormat( OUStringBuffer& rBuffer,
                           sal_Int16 nType ) const;
    void convertNumLetterSync( OUStringBuffer& rBuffer,
                                  sal_Int16 nType ) const;

    static void convertPropertySet(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProps,
                        const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& aProperties);
    static void convertPropertySet(com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& rProperties,
                        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);

    OUString encodeStyleName( const OUString& rName,
                                     sal_Bool *pEncoded=0 ) const;
    /** convert string (hex) to number (sal_uInt32) */
    static sal_Bool convertHex( sal_uInt32& nVal,
                              const OUString& rValue );

    /** convert number (sal_uInt32) to string (hex) */
    static void convertHex( OUStringBuffer& rBuffer,
                               sal_uInt32 nVal );

};

#endif  //  _XMLOFF_XMLUCONV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
