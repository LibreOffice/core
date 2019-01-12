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

#ifndef INCLUDED_XMLOFF_XMLUCONV_HXX
#define INCLUDED_XMLOFF_XMLUCONV_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>

#include <memory>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmltoken.hxx>

#include <tools/fldunit.hxx>

namespace com { namespace sun { namespace star {
    namespace text { class XNumberingTypeInfo; }
}}}

namespace basegfx
{
    class B3DVector;
}

namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace beans { struct PropertyValue; } } } }
namespace com { namespace sun { namespace star { namespace drawing { struct Position3D; } } } }
namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }
namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <class E> class Sequence; } } } }
namespace com { namespace sun { namespace star { namespace util { struct Date; } } } }
template <typename EnumT> struct SvXMLEnumMapEntry;
template <typename EnumT> struct SvXMLEnumStringMapEntry;

class XMLOFF_DLLPUBLIC SvXMLTokenEnumerator
{
private:
    const OUString&  maTokenString;
    sal_Int32               mnNextTokenPos;
    sal_Unicode const       mcSeparator;

public:
    SvXMLTokenEnumerator( const OUString& rString, sal_Unicode cSeparator = u' ' );

    bool getNextToken( OUString& rToken );
};

/** the SvXMLTypeConverter converts values of various types from
    their internal representation to the textual form used in xml
    and back.
    Most of the methods are static but the SvXMLTypeConverter can
    also store default units for both numerical and textual measures.

    @attention:
        a lot of the methods here have been moved to <sax/tools/converter.hxx>!
*/

class XMLOFF_DLLPUBLIC SvXMLUnitConverter final
{
private:
    SvXMLUnitConverter(const SvXMLUnitConverter&) = delete;
    SvXMLUnitConverter& operator=(const SvXMLUnitConverter&) = delete;

    struct Impl;
    ::std::unique_ptr<Impl> m_pImpl;

public:
    /** constructs a SvXMLUnitConverter. The core measure unit is the
        default unit for numerical measures, the XML measure unit is
        the default unit for textual measures */
    SvXMLUnitConverter(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        sal_Int16 eCoreMeasureUnit /*css::util::MeasureUnit*/,
        sal_Int16 eXMLMeasureUnit /*css::util::MeasureUnit*/);

    ~SvXMLUnitConverter();

    static sal_Int16 GetMeasureUnit(FieldUnit const nFieldUnit);

    /** sets the default unit for numerical measures */
    void SetCoreMeasureUnit( sal_Int16 const eCoreMeasureUnit /*css::util::MeasureUnit*/);

    /** sets the default unit for textual measures */
    void SetXMLMeasureUnit( sal_Int16 const eXMLMeasureUnit /*css::util::MeasureUnit*/);

    /** gets the default unit for textual measures */
    sal_Int16 GetXMLMeasureUnit() const;

    /** gets XNumberingTypeInfo */
    const css::uno::Reference< css::text::XNumberingTypeInfo >& getNumTypeInfo() const;

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
    template<typename EnumT>
    static bool convertEnum( EnumT& rEnum,
                             const OUString& rValue,
                             const SvXMLEnumMapEntry<EnumT> *pMap )
    {
        sal_uInt16 nTmp;
        bool bRet = convertEnumImpl(nTmp, rValue,
                        reinterpret_cast<const SvXMLEnumMapEntry<sal_uInt16>*>(pMap));
        if (bRet)
            rEnum = static_cast<EnumT>(nTmp);
        return bRet;
    }

    /** convert string to enum using given token map, if the enum is
        not found in the map, this method will return false */
    template<typename EnumT>
    static bool convertEnum( EnumT& rEnum,
                             const OUString& rValue,
                             const SvXMLEnumStringMapEntry<EnumT> *pMap )
    {
        sal_uInt16 nTmp;
        bool bRet = convertEnumImpl(nTmp, rValue,
                        reinterpret_cast<const SvXMLEnumStringMapEntry<sal_uInt16>*>(pMap));
        if (bRet)
            rEnum = static_cast<EnumT>(nTmp);
        return bRet;
    }

    /** convert enum to string using given enum map with an optional
        default token. If the enum is not found in the map,
        this method will either use the given default or return
        false if not default is set */
    template<typename EnumT>
    static bool convertEnum( OUStringBuffer& rBuffer,
                             EnumT nValue,
                             const SvXMLEnumMapEntry<EnumT> *pMap,
                             enum ::xmloff::token::XMLTokenEnum eDefault =
                                         ::xmloff::token::XML_TOKEN_INVALID )
    {
        return convertEnumImpl(rBuffer, static_cast<sal_uInt16>(nValue),
                   reinterpret_cast<const SvXMLEnumMapEntry<sal_uInt16>*>(pMap), eDefault);
    }

    /** convert double number to string (using ::rtl::math) and DO
        convert to export MapUnit using meCoreMeasureUnit/meXMLMeasureUnit */
    void convertDouble(OUStringBuffer& rBuffer,
        double fNumber) const;

    /** convert string to double number (using ::rtl::math) and DO convert. */
    bool convertDouble(double& rValue, const OUString& rString) const;

    /** get the Null Date of the XModel and set it to the UnitConverter */
    bool setNullDate (
        const css::uno::Reference <css::frame::XModel>& xModel);

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
                                const css::util::Date& aNullDate,
                                bool bAddTimeIf0AM = false);
    /** convert ISO Date Time String to double */
    static bool convertDateTime( double& fDateTime,
                                const OUString& rString,
                                const css::util::Date& aNullDate);


    /** convert string to ::basegfx::B3DVector */
    static bool convertB3DVector( ::basegfx::B3DVector& rVector,
                              const OUString& rValue );

    /** convert B3DVector to string */
    static void convertB3DVector( OUStringBuffer &rBuffer,
        const ::basegfx::B3DVector& rVector );

    /** convert string to Position3D */
    bool convertPosition3D( css::drawing::Position3D& rPosition,
                              const OUString& rValue );

    /** convert Position3D to string */
    void convertPosition3D( OUStringBuffer &rBuffer,
                              const css::drawing::Position3D& rVector );


    /** convert num-format and num-letter-sync values to NumberingType */
    bool convertNumFormat( sal_Int16& rType,
                           const OUString& rNumFormat,
                           const OUString& rNumLetterSync,
                           bool bNumberNone = false ) const;

    /** convert NumberingType to num-format and num-letter-sync values */
    void convertNumFormat( OUStringBuffer& rBuffer,
                           sal_Int16 nType ) const;
    static void convertNumLetterSync( OUStringBuffer& rBuffer,
                                  sal_Int16 nType );

    static void convertPropertySet(css::uno::Sequence<css::beans::PropertyValue>& rProps,
                        const css::uno::Reference<css::beans::XPropertySet>& aProperties);
    static void convertPropertySet(css::uno::Reference<css::beans::XPropertySet> const & rProperties,
                        const css::uno::Sequence<css::beans::PropertyValue>& aProps);

    OUString encodeStyleName( const OUString& rName,
                                     bool *pEncoded=nullptr ) const;
    /** convert string (hex) to number (sal_uInt32) */
    static bool convertHex( sal_uInt32& nVal,
                              const OUString& rValue );

    /** convert number (sal_uInt32) to string (hex) */
    static void convertHex( OUStringBuffer& rBuffer,
                               sal_uInt32 nVal );

private:
    static bool convertEnumImpl( sal_uInt16& rEnum,
                             const OUString& rValue,
                             const SvXMLEnumMapEntry<sal_uInt16> *pMap );

    static bool convertEnumImpl( sal_uInt16& rEnum,
                             const OUString& rValue,
                             const SvXMLEnumStringMapEntry<sal_uInt16> *pMap );

    static bool convertEnumImpl( OUStringBuffer& rBuffer,
                             sal_uInt16 nValue,
                             const SvXMLEnumMapEntry<sal_uInt16> *pMap,
                             enum ::xmloff::token::XMLTokenEnum eDefault );
};

#endif // INCLUDED_XMLOFF_XMLUCONV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
