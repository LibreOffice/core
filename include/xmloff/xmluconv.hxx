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

namespace com::sun::star {
    namespace text { class XNumberingTypeInfo; }
}

namespace basegfx
{
    class B3DVector;
}

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::drawing { struct Position3D; }
namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::uno { template <class E> class Sequence; }
namespace com::sun::star::util { struct Date; }
template <typename EnumT> struct SvXMLEnumMapEntry;
template <typename EnumT> struct SvXMLEnumStringMapEntry;

class XMLOFF_DLLPUBLIC SvXMLTokenEnumerator
{
private:
    std::u16string_view     maTokenString;
    size_t                  mnNextTokenPos;
    sal_Unicode             mcSeparator;

public:
    SvXMLTokenEnumerator( std::u16string_view rString, sal_Unicode cSeparator = u' ' );
    /** just so no-one accidentally passes a temporary to this, and ends up with this class
     * accessing the temporary after the temporary has been deleted. */
    SvXMLTokenEnumerator( OUString&& , sal_Unicode cSeparator = u' ' ) = delete;

    bool getNextToken( std::u16string_view& rToken );
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

    struct SAL_DLLPRIVATE Impl;
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
                         std::u16string_view rString,
                         sal_Int32 nMin = SAL_MIN_INT32,
                         sal_Int32 nMax = SAL_MAX_INT32) const;

    /** convert string to measure with meCoreMeasureUnit,
        using optional min and max values*/
    bool convertMeasureToCore( sal_Int32& rValue,
                         std::string_view rString,
                         sal_Int32 nMin = SAL_MIN_INT32,
                         sal_Int32 nMax = SAL_MAX_INT32) const;

    /** convert measure to string: from meCoreMeasureUnit to meXMLMeasureUnit */
    void convertMeasureToXML( OUStringBuffer& rBuffer,
                         sal_Int32 nMeasure ) const;

    /** convert measure to string: from meCoreMeasureUnit to meXMLMeasureUnit */
    OUString convertMeasureToXML( sal_Int32 nMeasure ) const;

    /** convert string to enum using given enum map, if the enum is
        not found in the map, this method will return false */
    template<typename EnumT>
    static bool convertEnum( EnumT& rEnum,
                             std::u16string_view rValue,
                             const SvXMLEnumMapEntry<EnumT> *pMap )
    {
        sal_uInt16 nTmp;
        bool bRet = convertEnumImpl(nTmp, rValue,
                        reinterpret_cast<const SvXMLEnumMapEntry<sal_uInt16>*>(pMap));
        if (bRet)
            rEnum = static_cast<EnumT>(nTmp);
        return bRet;
    }

    /** convert string to enum using given enum map, if the enum is
        not found in the map, this method will return false */
    template<typename EnumT>
    static bool convertEnum( EnumT& rEnum,
                             std::string_view rValue,
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
                             std::u16string_view rValue,
                             const SvXMLEnumStringMapEntry<EnumT> *pMap )
    {
        sal_uInt16 nTmp;
        bool bRet = convertEnumImpl(nTmp, rValue,
                        reinterpret_cast<const SvXMLEnumStringMapEntry<sal_uInt16>*>(pMap));
        if (bRet)
            rEnum = static_cast<EnumT>(nTmp);
        return bRet;
    }

    /** convert string to enum using given token map, if the enum is
        not found in the map, this method will return false */
    template<typename EnumT>
    static bool convertEnum( EnumT& rEnum,
                             std::string_view rValue,
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
    bool convertDouble(double& rValue, std::u16string_view rString) const;

    /** get the Null Date of the XModel and set it to the UnitConverter */
    bool setNullDate (
        const css::uno::Reference <css::frame::XModel>& xModel);

    /** convert double to ISO Date Time String */
    void convertDateTime( OUStringBuffer& rBuffer,
                                const double& fDateTime,
                                bool const bAddTimeIf0AM = false);

    /** convert ISO Date Time String to double */
    bool convertDateTime(double& fDateTime,
                         std::u16string_view rString);

    /// these 2 functions use tools Date, so they're not yet moved to sax

    /** convert double to ISO Date Time String */
    static void convertDateTime( OUStringBuffer& rBuffer,
                                const double& fDateTime,
                                const css::util::Date& aNullDate,
                                bool bAddTimeIf0AM = false);
    /** convert ISO Date Time String to double */
    static bool convertDateTime( double& fDateTime,
                                std::u16string_view rString,
                                const css::util::Date& aNullDate);


    /** convert string to ::basegfx::B3DVector */
    static bool convertB3DVector( ::basegfx::B3DVector& rVector,
                              std::u16string_view rValue );

    /** convert string to ::basegfx::B3DVector */
    static bool convertB3DVector( ::basegfx::B3DVector& rVector,
                              std::string_view rValue );

    /** convert B3DVector to string */
    static void convertB3DVector( OUStringBuffer &rBuffer,
        const ::basegfx::B3DVector& rVector );

    /** convert string to Position3D */
    bool convertPosition3D( css::drawing::Position3D& rPosition,
                              std::string_view rValue );

    /** convert Position3D to string */
    void convertPosition3D( OUStringBuffer &rBuffer,
                              const css::drawing::Position3D& rVector );


    /** convert num-format and num-letter-sync values to NumberingType */
    bool convertNumFormat( sal_Int16& rType,
                           const OUString& rNumFormat,
                           std::u16string_view rNumLetterSync,
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
                              std::u16string_view rValue );

    /** convert number (sal_uInt32) to string (hex) */
    static void convertHex( OUStringBuffer& rBuffer,
                               sal_uInt32 nVal );

private:
    static bool convertEnumImpl( sal_uInt16& rEnum,
                             std::u16string_view rValue,
                             const SvXMLEnumMapEntry<sal_uInt16> *pMap );

    static bool convertEnumImpl( sal_uInt16& rEnum,
                             std::u16string_view rValue,
                             const SvXMLEnumStringMapEntry<sal_uInt16> *pMap );

    static bool convertEnumImpl( sal_uInt16& rEnum,
                             std::string_view rValue,
                             const SvXMLEnumMapEntry<sal_uInt16> *pMap );

    static bool convertEnumImpl( sal_uInt16& rEnum,
                             std::string_view rValue,
                             const SvXMLEnumStringMapEntry<sal_uInt16> *pMap );

    static bool convertEnumImpl( OUStringBuffer& rBuffer,
                             sal_uInt16 nValue,
                             const SvXMLEnumMapEntry<sal_uInt16> *pMap,
                             enum ::xmloff::token::XMLTokenEnum eDefault );
};

#endif // INCLUDED_XMLOFF_XMLUCONV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
