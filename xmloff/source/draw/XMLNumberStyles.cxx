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
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>

#include <sal/log.hxx>

#include "sdxmlexp_impl.hxx"
#include "sdxmlimp_impl.hxx"

using namespace ::xmloff::token;

struct SdXMLDataStyleNumber
{
    enum XMLTokenEnum const meNumberStyle;
    bool const    mbLong;
    bool const    mbTextual;
    bool const    mbDecimal02;
    const char* mpText;
}
const aSdXMLDataStyleNumbers[] =
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

#define DATA_STYLE_NUMBER_DAY               1   // <number:day/>
#define DATA_STYLE_NUMBER_DAY_LONG          2   // <number:day number:style="long"/>
#define DATA_STYLE_NUMBER_MONTH_LONG        3   // <number:month number:style="long"/>
#define DATA_STYLE_NUMBER_MONTH_TEXT        4   // <number:month number:textual="true"/>
#define DATA_STYLE_NUMBER_MONTH_LONG_TEXT   5   // <number:month number:style="long" number:textual="true"/>
#define DATA_STYLE_NUMBER_YEAR              6   // <number:year/>
#define DATA_STYLE_NUMBER_YEAR_LONG         7   // <number:year number:style="long"/>
#define DATA_STYLE_NUMBER_DAYOFWEEK         8   // <number:day-of-week/>
#define DATA_STYLE_NUMBER_DAYOFWEEK_LONG    9   // <number:day-of-week number:style="long"/>
#define DATA_STYLE_NUMBER_TEXT_POINT        10  // <number:text>.</number:text>
#define DATA_STYLE_NUMBER_TEXT_SPACE        11  // <number:text> </number:text>
#define DATA_STYLE_NUMBER_TEXT_COMMASPACE   12  // <number:text>, </number:text>
#define DATA_STYLE_NUMBER_TEXT_POINTSPACE   13  // <number:text>. </number:text>
#define DATA_STYLE_NUMBER_HOURS             14  // <number:hours/>
#define DATA_STYLE_NUMBER_MINUTES           15  // <number:minutes/>
#define DATA_STYLE_NUMBER_TEXT_COLON        16  // <number:text>:</number:text>
#define DATA_STYLE_NUMBER_AMPM              17  // <number:am-pm/>
#define DATA_STYLE_NUMBER_SECONDS           18  // <number:seconds/>
#define DATA_STYLE_NUMBER_SECONDS_02        19  // <number:seconds number:/>

struct SdXMLFixedDataStyle
{
    const char* mpName;
    bool const  mbAutomatic;
    bool const  mbDateStyle;
    sal_uInt8   mpFormat[8];
};

const SdXMLFixedDataStyle aSdXML_Standard_Short =
{
    "D1", true, true,
    {
        DATA_STYLE_NUMBER_DAY_LONG,
        DATA_STYLE_NUMBER_TEXT_POINT,
        DATA_STYLE_NUMBER_MONTH_LONG,
        DATA_STYLE_NUMBER_TEXT_POINT,
        DATA_STYLE_NUMBER_YEAR_LONG,
        0, 0, 0
    }
};

const SdXMLFixedDataStyle aSdXML_Standard_Long =
{
    "D2", true, true,
    {
        DATA_STYLE_NUMBER_DAYOFWEEK_LONG,
        DATA_STYLE_NUMBER_TEXT_COMMASPACE,
        DATA_STYLE_NUMBER_DAY,
        DATA_STYLE_NUMBER_TEXT_POINTSPACE,
        DATA_STYLE_NUMBER_MONTH_LONG_TEXT,
        DATA_STYLE_NUMBER_TEXT_SPACE,
        DATA_STYLE_NUMBER_YEAR_LONG,
        0
    }
};

const SdXMLFixedDataStyle aSdXML_DateStyle_1 =
{
    "D3", false, true,
    {
        DATA_STYLE_NUMBER_DAY_LONG,
        DATA_STYLE_NUMBER_TEXT_POINT,
        DATA_STYLE_NUMBER_MONTH_LONG,
        DATA_STYLE_NUMBER_TEXT_POINT,
        DATA_STYLE_NUMBER_YEAR,
        0, 0, 0
    }
};

const SdXMLFixedDataStyle aSdXML_DateStyle_2 =
{
    "D4", false, true,
    {
        DATA_STYLE_NUMBER_DAY_LONG,
        DATA_STYLE_NUMBER_TEXT_POINT,
        DATA_STYLE_NUMBER_MONTH_LONG,
        DATA_STYLE_NUMBER_TEXT_POINT,
        DATA_STYLE_NUMBER_YEAR_LONG,
        0, 0, 0
    }
};

const SdXMLFixedDataStyle aSdXML_DateStyle_3 =
{
    "D5", false, true,
    {
        DATA_STYLE_NUMBER_DAY,
        DATA_STYLE_NUMBER_TEXT_POINTSPACE,
        DATA_STYLE_NUMBER_MONTH_TEXT,
        DATA_STYLE_NUMBER_TEXT_SPACE,
        DATA_STYLE_NUMBER_YEAR_LONG,
        0, 0, 0
    }
};

const SdXMLFixedDataStyle aSdXML_DateStyle_4 =
{
    "D6", false, true,
    {
        DATA_STYLE_NUMBER_DAY,
        DATA_STYLE_NUMBER_TEXT_POINTSPACE,
        DATA_STYLE_NUMBER_MONTH_LONG_TEXT,
        DATA_STYLE_NUMBER_TEXT_SPACE,
        DATA_STYLE_NUMBER_YEAR_LONG,
        0, 0, 0
    }
};

const SdXMLFixedDataStyle aSdXML_DateStyle_5 =
{
    "D7", false, true,
    {
        DATA_STYLE_NUMBER_DAYOFWEEK,
        DATA_STYLE_NUMBER_TEXT_COMMASPACE,
        DATA_STYLE_NUMBER_DAY,
        DATA_STYLE_NUMBER_TEXT_POINTSPACE,
        DATA_STYLE_NUMBER_MONTH_LONG_TEXT,
        DATA_STYLE_NUMBER_TEXT_SPACE,
        DATA_STYLE_NUMBER_YEAR_LONG,
        0
    }
};

const SdXMLFixedDataStyle aSdXML_DateStyle_6 =
{
    "D8", false, true,
    {
        DATA_STYLE_NUMBER_DAYOFWEEK_LONG,
        DATA_STYLE_NUMBER_TEXT_COMMASPACE,
        DATA_STYLE_NUMBER_DAY,
        DATA_STYLE_NUMBER_TEXT_POINTSPACE,
        DATA_STYLE_NUMBER_MONTH_LONG_TEXT,
        DATA_STYLE_NUMBER_TEXT_SPACE,
        DATA_STYLE_NUMBER_YEAR_LONG,
        0
    }
};

const SdXMLFixedDataStyle aSdXML_TimeStyle_1 =
{   "T1", true, false,
    {
        DATA_STYLE_NUMBER_HOURS,
        DATA_STYLE_NUMBER_TEXT_COLON,
        DATA_STYLE_NUMBER_MINUTES,
        DATA_STYLE_NUMBER_TEXT_COLON,
        DATA_STYLE_NUMBER_SECONDS,
        DATA_STYLE_NUMBER_AMPM,
        0, 0,
    }
};

const SdXMLFixedDataStyle aSdXML_TimeStyle_2 =
{   "T2", false, false,
    {
        DATA_STYLE_NUMBER_HOURS,
        DATA_STYLE_NUMBER_TEXT_COLON,
        DATA_STYLE_NUMBER_MINUTES,
        0, 0, 0, 0, 0
    }
};

const SdXMLFixedDataStyle aSdXML_TimeStyle_3 =
{   "T3", false, false,
    {
        DATA_STYLE_NUMBER_HOURS,
        DATA_STYLE_NUMBER_TEXT_COLON,
        DATA_STYLE_NUMBER_MINUTES,
        DATA_STYLE_NUMBER_TEXT_COLON,
        DATA_STYLE_NUMBER_SECONDS,
        0, 0, 0
    }
};

const SdXMLFixedDataStyle aSdXML_TimeStyle_4 =
{   "T4", false, false,
    {
        DATA_STYLE_NUMBER_HOURS,
        DATA_STYLE_NUMBER_TEXT_COLON,
        DATA_STYLE_NUMBER_MINUTES,
        DATA_STYLE_NUMBER_TEXT_COLON,
        DATA_STYLE_NUMBER_SECONDS_02,
        0, 0, 0
    }
};

const SdXMLFixedDataStyle aSdXML_TimeStyle_5 =
{   "T5", false, false,
    {
        DATA_STYLE_NUMBER_HOURS,
        DATA_STYLE_NUMBER_TEXT_COLON,
        DATA_STYLE_NUMBER_MINUTES,
        DATA_STYLE_NUMBER_AMPM,
        0, 0, 0, 0
    }
};

const SdXMLFixedDataStyle aSdXML_TimeStyle_6 =
{   "T6", false, false,
    {
        DATA_STYLE_NUMBER_HOURS,
        DATA_STYLE_NUMBER_TEXT_COLON,
        DATA_STYLE_NUMBER_MINUTES,
        DATA_STYLE_NUMBER_TEXT_COLON,
        DATA_STYLE_NUMBER_SECONDS,
        DATA_STYLE_NUMBER_AMPM,
        0, 0
    }
};

const SdXMLFixedDataStyle aSdXML_TimeStyle_7 =
{   "T7", false, false,
    {
        DATA_STYLE_NUMBER_HOURS,
        DATA_STYLE_NUMBER_TEXT_COLON,
        DATA_STYLE_NUMBER_MINUTES,
        DATA_STYLE_NUMBER_TEXT_COLON,
        DATA_STYLE_NUMBER_SECONDS_02,
        DATA_STYLE_NUMBER_AMPM,
        0, 0
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
    OUString sAttrValue;

    // name
    sAttrValue = OUString::createFromAscii( pStyle->mpName );
    if( pStyle2 )
        sAttrValue += OUString::createFromAscii( pStyle2->mpName );

    rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_NAME, sAttrValue );

    if( pStyle->mbAutomatic )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_AUTOMATIC_ORDER, XML_TRUE );
    }

    SvXMLElementExport aElement( rExport, XML_NAMESPACE_NUMBER, pStyle->mbDateStyle ? XML_DATE_STYLE : XML_TIME_STYLE, true, true );

    do
    {

        const sal_uInt8* pElements = &pStyle->mpFormat[0];

        while( *pElements )
        {
            SdXMLDataStyleNumber const & rElement = aSdXMLDataStyleNumbers[ (*pElements++) - 1 ];
            SdXMLExportDataStyleNumber( rExport, rElement );
        }

        if( pStyle2 )
        {
            SdXMLDataStyleNumber const & rElement = aSdXMLDataStyleNumbers[ DATA_STYLE_NUMBER_TEXT_SPACE - 1 ];
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
        return OUString::createFromAscii(aSdXMLFixedTimeFormats[nFormat]->mpName );
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
        return OUString::createFromAscii(aSdXMLFixedDateFormats[nFormat]->mpName );
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

    OUString const maNumberStyle;
    bool mbLong;
    bool mbTextual;
    bool mbDecimal02;
    OUString maText;
    SvXMLImportContextRef mxSlaveContext;

public:

    SdXMLNumberFormatMemberImportContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        SdXMLNumberFormatImportContext* pParent,
        const SvXMLImportContextRef& rSlaveContext );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    virtual void EndElement() override;

    virtual void Characters( const OUString& rChars ) override;
};


SdXMLNumberFormatMemberImportContext::SdXMLNumberFormatMemberImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList, SdXMLNumberFormatImportContext* pParent, const SvXMLImportContextRef& rSlaveContext )
:   SvXMLImportContext(rImport, nPrfx, rLocalName),
    mpParent( pParent ),
    maNumberStyle( rLocalName ),
    mxSlaveContext( rSlaveContext )
{
    mbLong = false;
    mbTextual = false;
    mbDecimal02 = false;

    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );

        if( nPrefix == XML_NAMESPACE_NUMBER )
        {
            if( IsXMLToken( aLocalName, XML_DECIMAL_PLACES ) )
            {
                mbDecimal02 =  IsXMLToken( sValue, XML_2 );
            }
            else if( IsXMLToken( aLocalName, XML_STYLE ) )
            {
                mbLong = IsXMLToken( sValue, XML_LONG );
            }
            else if( IsXMLToken( aLocalName, XML_TEXTUAL ) )
            {
                mbTextual = IsXMLToken( sValue, XML_TRUE );
            }
        }
    }

}

SvXMLImportContextRef SdXMLNumberFormatMemberImportContext::CreateChildContext( sal_uInt16 nPrefix,
                           const OUString& rLocalName,
                           const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList )
{
    return mxSlaveContext->CreateChildContext( nPrefix, rLocalName, xAttrList );
}

void SdXMLNumberFormatMemberImportContext::StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList )
{
    mxSlaveContext->StartElement( xAttrList );
}

void SdXMLNumberFormatMemberImportContext::EndElement()
{
    mxSlaveContext->EndElement();

    if( mpParent )
        mpParent->add( maNumberStyle, mbLong, mbTextual, mbDecimal02, maText );
}

void SdXMLNumberFormatMemberImportContext::Characters( const OUString& rChars )
{
    mxSlaveContext->Characters( rChars );
    maText += rChars;
}


SdXMLNumberFormatImportContext::SdXMLNumberFormatImportContext( SdXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, SvXMLNumImpData* pNewData, sal_uInt16 nNewType, const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList, SvXMLStylesContext& rStyles)
:   SvXMLNumFormatContext(rImport, nPrfx, rLocalName, pNewData, nNewType, xAttrList, rStyles),
    mbAutomatic( false ),
    mnIndex(0),
    mnKey( -1 )
{
    mbTimeStyle = IsXMLToken( rLocalName, XML_TIME_STYLE );

    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );

        if( nPrefix == XML_NAMESPACE_NUMBER )
        {
            if( IsXMLToken( aLocalName, XML_AUTOMATIC_ORDER ) )
            {
                mbAutomatic = IsXMLToken( sValue, XML_TRUE );
            }
        }
    }
}

SdXMLNumberFormatImportContext::~SdXMLNumberFormatImportContext()
{
}

void SdXMLNumberFormatImportContext::add( OUString const & rNumberStyle, bool bLong, bool bTextual, bool bDecimal02, OUString const & rText )
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
            ( ( (pStyleMember->mpText == nullptr) && (rText.isEmpty()) ) ||
              ( pStyleMember->mpText && (rText.equalsAscii( pStyleMember->mpText ) ) ) ) )
        {
            mnElements[mnIndex++] = nIndex + 1;
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

void SdXMLNumberFormatImportContext::EndElement()
{
    SvXMLNumFormatContext::EndElement();

    for( ; mnIndex < 16; mnIndex++ )
    {
        mnElements[mnIndex] = 0;
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
            else if( mnElements[nIndex] == DATA_STYLE_NUMBER_TEXT_SPACE )
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

SvXMLImportContextRef SdXMLNumberFormatImportContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList )
{
    return new SdXMLNumberFormatMemberImportContext( GetImport(), nPrefix, rLocalName, xAttrList, this, SvXMLNumFormatContext::CreateChildContext( nPrefix, rLocalName, xAttrList ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
