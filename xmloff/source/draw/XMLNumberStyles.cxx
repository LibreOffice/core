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

#include "XMLNumberStylesExport.hxx"
#include <XMLNumberStylesImport.hxx>
#include <utility>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>

#include <o3tl/string_view.hxx>
#include <sal/log.hxx>

#include "sdxmlexp_impl.hxx"
#include "sdxmlimp_impl.hxx"

using namespace ::xmloff::token;

namespace {

struct SdXMLDataStyleNumber
{
    enum XMLTokenEnum meNumberStyle;
    bool    mbLong;
    bool    mbTextual;
    bool    mbDecimal02;
    const char* mpText;
};

}

SdXMLDataStyleNumber const aSdXMLDataStyleNumbers[] =
{
    { XML_DAY,          false,      false,      false,      nullptr },
    { XML_DAY,          true,       false,      false,      nullptr },
    { XML_MONTH,        true,       false,      false,      nullptr },
    { XML_MONTH,        false,      true,       false,      nullptr },
    { XML_MONTH,        true,       true,       false,      nullptr },
    { XML_YEAR,         false,      false,      false,      nullptr },
    { XML_YEAR,         true,       false,      false,      nullptr },
    { XML_DAY_OF_WEEK,  false,      false,      false,      nullptr },
    { XML_DAY_OF_WEEK,  true,       false,      false,      nullptr },
    { XML_TEXT,         false,      false,      false,      "."  },
    { XML_TEXT,         false,      false,      false,      " "  },
    { XML_TEXT,         false,      false,      false,      ", " },
    { XML_TEXT,         false,      false,      false,      ". " },
    { XML_HOURS,        false,      false,      false,      nullptr },
    { XML_MINUTES,      false,      false,      false,      nullptr },
    { XML_TEXT,         false,      false,      false,      ":"  },
    { XML_AM_PM,        false,      false,      false,      nullptr },
    { XML_SECONDS,      false,      false,      false,      nullptr },
    { XML_SECONDS,      false,      false,      true,       nullptr },
    { XML_TOKEN_INVALID,        false,              false,             false,       nullptr  }
};

// date
enum class DataStyleNumber : sal_uInt8
{
    NONE              = 0,
    Day               = 1,   // <number:day/>
    DayLong           = 2,   // <number:day number:style="long"/>
    MonthLong         = 3,   // <number:month number:style="long"/>
    MonthText         = 4,   // <number:month number:textual="true"/>
    MonthLongText     = 5,   // <number:month number:style="long" number:textual="true"/>
    Year              = 6,   // <number:year/>
    YearLong          = 7,   // <number:year number:style="long"/>
    DayOfWeek         = 8,   // <number:day-of-week/>
    DayOfWeekLong     = 9,   // <number:day-of-week number:style="long"/>
    TextPoint         = 10,  // <number:text>.</number:text>
    TextSpace         = 11,  // <number:text> </number:text>
    TextCommaSpace    = 12,  // <number:text>, </number:text>
    TextPointSpace    = 13,  // <number:text>. </number:text>
    Hours             = 14,  // <number:hours/>
    Minutes           = 15,  // <number:minutes/>
    TextColon         = 16,  // <number:text>:</number:text>
    AmPm              = 17,  // <number:am-pm/>
    Seconds           = 18,  // <number:seconds/>
    Seconds_02        = 19,  // <number:seconds number:/>
};

struct SdXMLFixedDataStyle
{
    OUString maName;
    bool    mbAutomatic;
    bool    mbDateStyle;
    DataStyleNumber mpFormat[8];
};

constexpr SdXMLFixedDataStyle aSdXML_Standard_Short =
{
    u"D1"_ustr, true, true,
    {
        DataStyleNumber::DayLong,
        DataStyleNumber::TextPoint,
        DataStyleNumber::MonthLong,
        DataStyleNumber::TextPoint,
        DataStyleNumber::YearLong,
        DataStyleNumber::NONE, DataStyleNumber::NONE, DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_Standard_Long =
{
    u"D2"_ustr, true, true,
    {
        DataStyleNumber::DayOfWeekLong,
        DataStyleNumber::TextCommaSpace,
        DataStyleNumber::Day,
        DataStyleNumber::TextPointSpace,
        DataStyleNumber::MonthLongText,
        DataStyleNumber::TextSpace,
        DataStyleNumber::YearLong,
        DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_DateStyle_1 =
{
    u"D3"_ustr, false, true,
    {
        DataStyleNumber::DayLong,
        DataStyleNumber::TextPoint,
        DataStyleNumber::MonthLong,
        DataStyleNumber::TextPoint,
        DataStyleNumber::Year,
        DataStyleNumber::NONE, DataStyleNumber::NONE, DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_DateStyle_2 =
{
    u"D4"_ustr, false, true,
    {
        DataStyleNumber::DayLong,
        DataStyleNumber::TextPoint,
        DataStyleNumber::MonthLong,
        DataStyleNumber::TextPoint,
        DataStyleNumber::YearLong,
        DataStyleNumber::NONE, DataStyleNumber::NONE, DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_DateStyle_3 =
{
    u"D5"_ustr, false, true,
    {
        DataStyleNumber::Day,
        DataStyleNumber::TextPointSpace,
        DataStyleNumber::MonthText,
        DataStyleNumber::TextSpace,
        DataStyleNumber::YearLong,
        DataStyleNumber::NONE, DataStyleNumber::NONE, DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_DateStyle_4 =
{
    u"D6"_ustr, false, true,
    {
        DataStyleNumber::Day,
        DataStyleNumber::TextPointSpace,
        DataStyleNumber::MonthLongText,
        DataStyleNumber::TextSpace,
        DataStyleNumber::YearLong,
        DataStyleNumber::NONE, DataStyleNumber::NONE, DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_DateStyle_5 =
{
    u"D7"_ustr, false, true,
    {
        DataStyleNumber::DayOfWeek,
        DataStyleNumber::TextCommaSpace,
        DataStyleNumber::Day,
        DataStyleNumber::TextPointSpace,
        DataStyleNumber::MonthLongText,
        DataStyleNumber::TextSpace,
        DataStyleNumber::YearLong,
        DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_DateStyle_6 =
{
    u"D8"_ustr, false, true,
    {
        DataStyleNumber::DayOfWeekLong,
        DataStyleNumber::TextCommaSpace,
        DataStyleNumber::Day,
        DataStyleNumber::TextPointSpace,
        DataStyleNumber::MonthLongText,
        DataStyleNumber::TextSpace,
        DataStyleNumber::YearLong,
        DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_TimeStyle_1 =
{   u"T1"_ustr, true, false,
    {
        DataStyleNumber::Hours,
        DataStyleNumber::TextColon,
        DataStyleNumber::Minutes,
        DataStyleNumber::TextColon,
        DataStyleNumber::Seconds,
        DataStyleNumber::AmPm,
        DataStyleNumber::NONE, DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_TimeStyle_2 =
{   u"T2"_ustr, false, false,
    {
        DataStyleNumber::Hours,
        DataStyleNumber::TextColon,
        DataStyleNumber::Minutes,
        DataStyleNumber::NONE, DataStyleNumber::NONE, DataStyleNumber::NONE, DataStyleNumber::NONE, DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_TimeStyle_3 =
{   u"T3"_ustr, false, false,
    {
        DataStyleNumber::Hours,
        DataStyleNumber::TextColon,
        DataStyleNumber::Minutes,
        DataStyleNumber::TextColon,
        DataStyleNumber::Seconds,
        DataStyleNumber::NONE, DataStyleNumber::NONE, DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_TimeStyle_4 =
{   u"T4"_ustr, false, false,
    {
        DataStyleNumber::Hours,
        DataStyleNumber::TextColon,
        DataStyleNumber::Minutes,
        DataStyleNumber::TextColon,
        DataStyleNumber::Seconds_02,
        DataStyleNumber::NONE, DataStyleNumber::NONE, DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_TimeStyle_5 =
{   u"T5"_ustr, false, false,
    {
        DataStyleNumber::Hours,
        DataStyleNumber::TextColon,
        DataStyleNumber::Minutes,
        DataStyleNumber::AmPm,
        DataStyleNumber::NONE, DataStyleNumber::NONE, DataStyleNumber::NONE, DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_TimeStyle_6 =
{   u"T6"_ustr, false, false,
    {
        DataStyleNumber::Hours,
        DataStyleNumber::TextColon,
        DataStyleNumber::Minutes,
        DataStyleNumber::TextColon,
        DataStyleNumber::Seconds,
        DataStyleNumber::AmPm,
        DataStyleNumber::NONE, DataStyleNumber::NONE
    }
};

constexpr SdXMLFixedDataStyle aSdXML_TimeStyle_7 =
{   u"T7"_ustr, false, false,
    {
        DataStyleNumber::Hours,
        DataStyleNumber::TextColon,
        DataStyleNumber::Minutes,
        DataStyleNumber::TextColon,
        DataStyleNumber::Seconds_02,
        DataStyleNumber::AmPm,
        DataStyleNumber::NONE, DataStyleNumber::NONE
    }
};

const SdXMLFixedDataStyle* const aSdXMLFixedDateFormats[SdXMLDateFormatCount] =
{
    &aSdXML_Standard_Short,
    &aSdXML_Standard_Long,
    &aSdXML_DateStyle_1,
    &aSdXML_DateStyle_2,
    &aSdXML_DateStyle_3,
    &aSdXML_DateStyle_4,
    &aSdXML_DateStyle_5,
    &aSdXML_DateStyle_6,
};

const SdXMLFixedDataStyle* const aSdXMLFixedTimeFormats[SdXMLTimeFormatCount] =
{
    &aSdXML_TimeStyle_1,
    &aSdXML_TimeStyle_2,
    &aSdXML_TimeStyle_3,
    &aSdXML_TimeStyle_4,
    &aSdXML_TimeStyle_5,
    &aSdXML_TimeStyle_6,
    &aSdXML_TimeStyle_7
};

// export

static void SdXMLExportDataStyleNumber( SdXMLExport& rExport, SdXMLDataStyleNumber const & rElement )
{
    if( rElement.mbDecimal02 )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_DECIMAL_PLACES, XML_2 );
    }

    if( rElement.mbLong )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_STYLE, XML_LONG );
    }

    if( rElement.mbTextual )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TEXTUAL, XML_TRUE );
    }

    SvXMLElementExport aNumberStyle( rExport, XML_NAMESPACE_NUMBER, rElement.meNumberStyle, true, false );
    if( rElement.mpText )
    {
        OUString sAttrValue( OUString::createFromAscii( rElement.mpText ) );
        rExport.GetDocHandler()->characters( sAttrValue );
    }
}

static void SdXMLExportStyle( SdXMLExport& rExport, const SdXMLFixedDataStyle* pStyle, const SdXMLFixedDataStyle* pStyle2 = nullptr )
{
    // name
    OUString sAttrValue = pStyle->maName;
    if( pStyle2 )
        sAttrValue += pStyle2->maName;

    rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_NAME, sAttrValue );

    if( pStyle->mbAutomatic )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_AUTOMATIC_ORDER, XML_TRUE );
    }

    SvXMLElementExport aElement( rExport, XML_NAMESPACE_NUMBER, pStyle->mbDateStyle ? XML_DATE_STYLE : XML_TIME_STYLE, true, true );

    do
    {

        const DataStyleNumber* pElements = &pStyle->mpFormat[0];

        while( *pElements != DataStyleNumber::NONE )
        {
            SdXMLDataStyleNumber const & rElement = aSdXMLDataStyleNumbers[ static_cast<int>(*pElements++) - 1 ];
            SdXMLExportDataStyleNumber( rExport, rElement );
        }

        if( pStyle2 )
        {
            SdXMLDataStyleNumber const & rElement = aSdXMLDataStyleNumbers[ static_cast<int>(DataStyleNumber::TextSpace) - 1 ];
            SdXMLExportDataStyleNumber( rExport, rElement );
        }

        pStyle = pStyle2;
        pStyle2 = nullptr;
    }
    while( pStyle );
}

void SdXMLNumberStylesExporter::exportTimeStyle( SdXMLExport& rExport, sal_Int32 nStyle )
{
    SAL_WARN_IF( (nStyle < 0) || (nStyle >= SdXMLTimeFormatCount), "xmloff", "Unknown time style!" );
    if( (nStyle >= 0) && (nStyle < SdXMLTimeFormatCount) )
        SdXMLExportStyle( rExport, aSdXMLFixedTimeFormats[ nStyle ] );
}

void SdXMLNumberStylesExporter::exportDateStyle( SdXMLExport& rExport, sal_Int32 nStyle )
{
    if( nStyle > 0x0f )
    {
        int nDateStyle = nStyle & 0x0f;
        bool bHasDate = nDateStyle != 0;

        if( nDateStyle > 1 )
            nDateStyle -= 2;

        SAL_WARN_IF(nDateStyle >= SdXMLDateFormatCount, "xmloff", "unknown date style!");

        int nTimeStyle = (nStyle >> 4) & 0x0f;
        bool bHasTime = nTimeStyle != 0;

        if( nTimeStyle > 1 )
            nTimeStyle -= 2;

        SAL_WARN_IF(nTimeStyle >= SdXMLTimeFormatCount, "xmloff", "Unknown time style!");

        if ((nDateStyle < SdXMLDateFormatCount) && (nTimeStyle < SdXMLTimeFormatCount))
        {
            if( bHasDate )
            {
                if( bHasTime )
                {
                    SdXMLExportStyle( rExport, aSdXMLFixedDateFormats[ nDateStyle ], aSdXMLFixedTimeFormats[ nTimeStyle ] );
                }
                else
                {
                    SdXMLExportStyle( rExport, aSdXMLFixedDateFormats[ nDateStyle ] );
                }
            }
            else if( bHasTime )
            {
                SdXMLExportStyle( rExport, aSdXMLFixedTimeFormats[ nTimeStyle ] );
            }
        }
    }
    else
    {
        SAL_WARN_IF( (nStyle < 0) || (nStyle >= SdXMLDateFormatCount), "xmloff", "unknown date style!" );
        if( (nStyle >= 0) && (nStyle < SdXMLDateFormatCount) )
            SdXMLExportStyle( rExport, aSdXMLFixedDateFormats[ nStyle ] );
    }
}

OUString SdXMLNumberStylesExporter::getTimeStyleName(const sal_Int32 nTimeFormat )
{
    sal_Int32 nFormat = nTimeFormat;
    if( nFormat > 1 )
        nFormat -= 2;

    if( (nFormat >= 0) && (nFormat < SdXMLTimeFormatCount) )
    {
        return aSdXMLFixedTimeFormats[nFormat]->maName;
    }
    else
    {
        return OUString();
    }
}

OUString SdXMLNumberStylesExporter::getDateStyleName(const sal_Int32 nDateFormat )
{
    sal_Int32 nFormat = nDateFormat;

    if( nFormat > 0x0f )
    {
        OUString aStr;
        if( nFormat & 0x0f )
            aStr = getDateStyleName( nFormat & 0x0f );
        aStr += getTimeStyleName( (nFormat >> 4) & 0x0f );
        return aStr;
    }

    if( nFormat > 1 )
        nFormat -= 2;

    if( (nFormat >= 0) && (nFormat < SdXMLDateFormatCount) )
    {
        return aSdXMLFixedDateFormats[nFormat]->maName;
    }
    else
    {
        return OUString();
    }
}

// import

class SdXMLNumberFormatMemberImportContext : public SvXMLImportContext
{
private:
    SdXMLNumberFormatImportContext* mpParent;

    OUString maNumberStyle;
    bool mbLong;
    bool mbTextual;
    bool mbDecimal02;
    OUString maText;
    SvXMLImportContextRef mxSlaveContext;

public:

    SdXMLNumberFormatMemberImportContext( SvXMLImport& rImport,
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
        SdXMLNumberFormatImportContext* pParent,
        SvXMLImportContextRef xSlaveContext );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual void SAL_CALL characters( const OUString& rChars ) override;
};


SdXMLNumberFormatMemberImportContext::SdXMLNumberFormatMemberImportContext(
    SvXMLImport& rImport,
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    SdXMLNumberFormatImportContext* pParent,
    SvXMLImportContextRef xSlaveContext )
:   SvXMLImportContext(rImport),
    mpParent( pParent ),
    maNumberStyle( SvXMLImport::getNameFromToken(nElement) ),
    mxSlaveContext(std::move( xSlaveContext ))
{
    mbLong = false;
    mbTextual = false;
    mbDecimal02 = false;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(NUMBER, XML_DECIMAL_PLACES):
                mbDecimal02 =  IsXMLToken( aIter, XML_2 );
                break;
            case XML_ELEMENT(NUMBER, XML_STYLE):
                mbLong = IsXMLToken( aIter, XML_LONG );
                break;
            case XML_ELEMENT(NUMBER, XML_TEXTUAL):
                mbTextual = IsXMLToken( aIter, XML_TRUE );
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLNumberFormatMemberImportContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    return mxSlaveContext->createFastChildContext( nElement, xAttrList );
}

void SdXMLNumberFormatMemberImportContext::startFastElement(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    mxSlaveContext->startFastElement( nElement, xAttrList );
}

void SdXMLNumberFormatMemberImportContext::endFastElement(sal_Int32 nElement)
{
    mxSlaveContext->endFastElement(nElement);

    if( mpParent )
        mpParent->add( maNumberStyle, mbLong, mbTextual, mbDecimal02, maText );
}

void SdXMLNumberFormatMemberImportContext::characters( const OUString& rChars )
{
    mxSlaveContext->characters( rChars );
    maText += rChars;
}


SdXMLNumberFormatImportContext::SdXMLNumberFormatImportContext( SdXMLImport& rImport, sal_Int32 nElement, SvXMLNumImpData* pNewData, SvXMLStylesTokens nNewType, const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList, SvXMLStylesContext& rStyles)
:   SvXMLNumFormatContext(rImport, nElement, pNewData, nNewType, xAttrList, rStyles),
    mbAutomatic( false ),
    mnElements{},
    mnIndex(0),
    mnKey( -1 )
{
    mbTimeStyle = (nElement & TOKEN_MASK) == XML_TIME_STYLE;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        if( aIter.getToken() == XML_ELEMENT(NUMBER, XML_AUTOMATIC_ORDER) )
                mbAutomatic = IsXMLToken( aIter, XML_TRUE );
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }
}

SdXMLNumberFormatImportContext::~SdXMLNumberFormatImportContext()
{
}

void SdXMLNumberFormatImportContext::add( std::u16string_view rNumberStyle, bool bLong, bool bTextual, bool bDecimal02, std::u16string_view rText )
{
    if (mnIndex == 16)
        return;

    const SdXMLDataStyleNumber* pStyleMember = aSdXMLDataStyleNumbers;
    for( sal_uInt8 nIndex = 0; pStyleMember->meNumberStyle != XML_TOKEN_INVALID; nIndex++, pStyleMember++ )
    {
        if( IsXMLToken(rNumberStyle, pStyleMember->meNumberStyle) &&
            (pStyleMember->mbLong == bLong) &&
            (pStyleMember->mbTextual == bTextual) &&
            (pStyleMember->mbDecimal02 == bDecimal02) &&
            ( ( (pStyleMember->mpText == nullptr) && (rText.empty()) ) ||
              ( pStyleMember->mpText && (o3tl::equalsAscii( rText, pStyleMember->mpText ) ) ) ) )
        {
            mnElements[mnIndex++] = static_cast<DataStyleNumber>(nIndex + 1);
            return;
        }
    }
}

bool SdXMLNumberFormatImportContext::compareStyle( const SdXMLFixedDataStyle* pStyle, sal_Int16& nIndex ) const
{
    if( (pStyle->mbAutomatic != mbAutomatic) && (nIndex == 0))
        return false;

    sal_Int16 nCompareIndex;
    for( nCompareIndex = 0; nCompareIndex < 8; nIndex++, nCompareIndex++ )
    {
        if( pStyle->mpFormat[nCompareIndex] != mnElements[nIndex] )
            return false;
    }

    return true;
}

void SdXMLNumberFormatImportContext::endFastElement(sal_Int32 )
{
    for( ; mnIndex < 16; mnIndex++ )
    {
        mnElements[mnIndex] = DataStyleNumber::NONE;
    }

    if( mbTimeStyle )
    {
        // compare import with all time styles
        for( sal_Int16 nFormat = 0; nFormat < SdXMLTimeFormatCount; nFormat++ )
        {
            sal_Int16 nIndex = 0;
            if( compareStyle( aSdXMLFixedTimeFormats[nFormat], nIndex ) )
            {
                mnKey = nFormat + 2;
                break;
            }
        }
    }
    else
    {
        // compare import with all date styles
        for( sal_Int16 nFormat = 0; nFormat < SdXMLDateFormatCount; nFormat++ )
        {
            sal_Int16 nIndex = 0;
            if( compareStyle( aSdXMLFixedDateFormats[nFormat], nIndex ) )
            {
                mnKey = nFormat + 2;
                break;
            }
            else if( mnElements[nIndex] == DataStyleNumber::TextSpace )
            {
                // if it's a valid date ending with a space, see if a time style follows
                for( sal_Int16 nTimeFormat = 0; nTimeFormat < SdXMLTimeFormatCount; nTimeFormat++ )
                {
                    sal_Int16 nIndex2 = nIndex + 1;
                    if( compareStyle( aSdXMLFixedTimeFormats[nTimeFormat], nIndex2 ) )
                    {
                        mnKey = (nFormat + 2) | ((nTimeFormat + 2) << 4);
                        break;
                    }
                }
            }
        }

        // no date style found? maybe it's an extended time style
        if( mnKey == -1 )
        {
            // compare import with all time styles
            for( sal_Int16 nFormat = 0; nFormat < SdXMLTimeFormatCount; nFormat++ )
            {
                sal_Int16 nIndex = 0;
                if( compareStyle( aSdXMLFixedTimeFormats[nFormat], nIndex ) )
                {
                    mnKey = (nFormat + 2) << 4;
                    break;
                }
            }
        }
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLNumberFormatImportContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    return new SdXMLNumberFormatMemberImportContext( GetImport(), nElement, xAttrList,
        this, static_cast<SvXMLImportContext*>(SvXMLNumFormatContext::createFastChildContext( nElement, xAttrList ).get()) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
