/*************************************************************************
 *
 *  $RCSfile: xmlnumfi.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-14 17:30:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _ZFORLIST_DECLARE_TABLE

#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <svtools/numuno.hxx>
#include <tools/isolang.hxx>
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>

#include "xmlnumfi.hxx"
#include "xmltkmap.hxx"
#include "xmlkywd.hxx"
#include "xmlnmspe.hxx"
#include "xmlictxt.hxx"
#include "xmlimp.hxx"
#include "xmluconv.hxx"
#include "nmspmap.hxx"
#include "families.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

//-------------------------------------------------------------------------

struct SvXMLNumFmtEntry
{
    rtl::OUString   aName;
    sal_uInt32      nKey;
    sal_Bool        bVolatile;

    SvXMLNumFmtEntry( const rtl::OUString& rN, sal_uInt32 nK, sal_Bool bV ) :
        aName(rN), nKey(nK), bVolatile(bV) {}
};

typedef SvXMLNumFmtEntry* SvXMLNumFmtEntryPtr;
SV_DECL_PTRARR_DEL( SvXMLNumFmtEntryArr, SvXMLNumFmtEntryPtr, 4, 4 );

//-------------------------------------------------------------------------

class SvXMLNumImpData
{
    SvNumberFormatter*  pFormatter;
    SvXMLTokenMap*      pStylesElemTokenMap;
    SvXMLTokenMap*      pStyleElemTokenMap;
    SvXMLTokenMap*      pStyleAttrTokenMap;
    SvXMLTokenMap*      pStyleElemAttrTokenMap;
    LocaleDataWrapper*  pLocaleData;
    SvXMLNumFmtEntryArr aNameEntries;

public:
                SvXMLNumImpData( SvNumberFormatter* pFmt );
                ~SvXMLNumImpData();

    SvNumberFormatter*      GetNumberFormatter() const  { return pFormatter; }
    const SvXMLTokenMap&    GetStylesElemTokenMap();
    const SvXMLTokenMap&    GetStyleElemTokenMap();
    const SvXMLTokenMap&    GetStyleAttrTokenMap();
    const SvXMLTokenMap&    GetStyleElemAttrTokenMap();
    const LocaleDataWrapper&    GetLocaleData( LanguageType nLang );
    sal_uInt32              GetKeyForName( const rtl::OUString& rName );
    void                    AddKey( sal_uInt32 nKey, const rtl::OUString& rName );
};


struct SvXMLNumberInfo
{
    sal_Int32   nDecimals;
    sal_Int32   nInteger;
    sal_Int32   nExpDigits;
    sal_Int32   nNumerDigits;
    sal_Int32   nDenomDigits;
    sal_Bool    bGrouping;
    sal_Bool    bDecReplace;

    SvXMLNumberInfo()
    {
        nDecimals = nInteger = nExpDigits = nNumerDigits = nDenomDigits = -1;
        bGrouping = bDecReplace = sal_False;
    }
};

class SvXMLNumFmtElementContext : public SvXMLImportContext
{
    SvXMLNumFormatContext&  rParent;
    sal_uInt16              nType;
    rtl::OUStringBuffer     aContent;
    SvXMLNumberInfo         aNumInfo;
    LanguageType            nElementLang;
    sal_Bool                bLong;
    sal_Bool                bTextual;
    rtl::OUString           sCalendar;

public:
                SvXMLNumFmtElementContext( SvXMLImport& rImport, USHORT nPrfx,
                                    const rtl::OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext, sal_uInt16 nNewType,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual     ~SvXMLNumFmtElementContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                    const rtl::OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void Characters( const rtl::OUString& rChars );
    virtual void EndElement();
};


class SvXMLNumFmtMapContext : public SvXMLImportContext
{
    SvXMLNumFormatContext&  rParent;
    rtl::OUString           sCondition;
    rtl::OUString           sName;

public:
                SvXMLNumFmtMapContext( SvXMLImport& rImport, USHORT nPrfx,
                                    const rtl::OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual     ~SvXMLNumFmtMapContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                    const rtl::OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void Characters( const rtl::OUString& rChars );
    virtual void EndElement();
};


class SvXMLNumFmtPropContext : public SvXMLImportContext
{
    SvXMLNumFormatContext&  rParent;
    Color                   aColor;
    sal_Bool                bColSet;

public:
                SvXMLNumFmtPropContext( SvXMLImport& rImport, USHORT nPrfx,
                                    const rtl::OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual     ~SvXMLNumFmtPropContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                    const rtl::OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void Characters( const rtl::OUString& rChars );
    virtual void EndElement();
};


//-------------------------------------------------------------------------

enum SvXMLStylesTokens
{
    XML_TOK_STYLES_NUMBER_STYLE,
    XML_TOK_STYLES_CURRENCY_STYLE,
    XML_TOK_STYLES_PERCENTAGE_STYLE,
    XML_TOK_STYLES_DATE_STYLE,
    XML_TOK_STYLES_TIME_STYLE,
    XML_TOK_STYLES_BOOLEAN_STYLE,
    XML_TOK_STYLES_TEXT_STYLE
};

enum SvXMLStyleTokens
{
    XML_TOK_STYLE_TEXT,
    XML_TOK_STYLE_NUMBER,
    XML_TOK_STYLE_SCIENTIFIC_NUMBER,
    XML_TOK_STYLE_FRACTION,
    XML_TOK_STYLE_CURRENCY_SYMBOL,
    XML_TOK_STYLE_DAY,
    XML_TOK_STYLE_MONTH,
    XML_TOK_STYLE_YEAR,
    XML_TOK_STYLE_ERA,
    XML_TOK_STYLE_DAY_OF_WEEK,
    XML_TOK_STYLE_WEEK_OF_YEAR,
    XML_TOK_STYLE_QUARTER,
    XML_TOK_STYLE_HOURS,
    XML_TOK_STYLE_AM_PM,
    XML_TOK_STYLE_MINUTES,
    XML_TOK_STYLE_SECONDS,
    XML_TOK_STYLE_BOOLEAN,
    XML_TOK_STYLE_TEXT_CONTENT,
    XML_TOK_STYLE_PROPERTIES,
    XML_TOK_STYLE_MAP
};

enum SvXMLStyleAttrTokens
{
    XML_TOK_STYLE_ATTR_NAME,
    XML_TOK_STYLE_ATTR_LANGUAGE,
    XML_TOK_STYLE_ATTR_COUNTRY,
    XML_TOK_STYLE_ATTR_TITLE,
    XML_TOK_STYLE_ATTR_AUTOMATIC_ORDER,
    XML_TOK_STYLE_ATTR_FORMAT_SOURCE,
    XML_TOK_STYLE_ATTR_TRUNCATE_ON_OVERFLOW,
    XML_TOK_STYLE_ATTR_VOLATILE
};

enum SvXMLStyleElemAttrTokens
{
    XML_TOK_ELEM_ATTR_DECIMAL_PLACES,
    XML_TOK_ELEM_ATTR_MIN_INTEGER_DIGITS,
    XML_TOK_ELEM_ATTR_GROUPING,
    XML_TOK_ELEM_ATTR_DECIMAL_REPLACEMENT,
    XML_TOK_ELEM_ATTR_MIN_EXPONENT_DIGITS,
    XML_TOK_ELEM_ATTR_MIN_NUMERATOR_DIGITS,
    XML_TOK_ELEM_ATTR_MIN_DENOMINATOR_DIGITS,
    XML_TOK_ELEM_ATTR_LANGUAGE,
    XML_TOK_ELEM_ATTR_COUNTRY,
    XML_TOK_ELEM_ATTR_STYLE,
    XML_TOK_ELEM_ATTR_TEXTUAL,
    XML_TOK_ELEM_ATTR_CALENDAR
};

//-------------------------------------------------------------------------

//
//  standard colors
//

#define XML_NUMF_COLORCOUNT     10

static ColorData aNumFmtStdColors[XML_NUMF_COLORCOUNT] =
{
    COL_BLACK,
    COL_LIGHTBLUE,
    COL_LIGHTGREEN,
    COL_LIGHTCYAN,
    COL_LIGHTRED,
    COL_LIGHTMAGENTA,
    COL_BROWN,
    COL_GRAY,
    COL_YELLOW,
    COL_WHITE
};

//
//  token maps
//

static __FAR_DATA SvXMLTokenMapEntry aStylesElemMap[] =
{
    //  style elements
    { XML_NAMESPACE_NUMBER, sXML_number_style,     XML_TOK_STYLES_NUMBER_STYLE      },
    { XML_NAMESPACE_NUMBER, sXML_currency_style,   XML_TOK_STYLES_CURRENCY_STYLE    },
    { XML_NAMESPACE_NUMBER, sXML_percentage_style, XML_TOK_STYLES_PERCENTAGE_STYLE  },
    { XML_NAMESPACE_NUMBER, sXML_date_style,       XML_TOK_STYLES_DATE_STYLE        },
    { XML_NAMESPACE_NUMBER, sXML_time_style,       XML_TOK_STYLES_TIME_STYLE        },
    { XML_NAMESPACE_NUMBER, sXML_boolean_style,    XML_TOK_STYLES_BOOLEAN_STYLE     },
    { XML_NAMESPACE_NUMBER, sXML_text_style,       XML_TOK_STYLES_TEXT_STYLE        },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStyleElemMap[] =
{
    //  elements in a style
    { XML_NAMESPACE_NUMBER, sXML_text,              XML_TOK_STYLE_TEXT              },
    { XML_NAMESPACE_NUMBER, sXML_number,            XML_TOK_STYLE_NUMBER            },
    { XML_NAMESPACE_NUMBER, sXML_scientific_number, XML_TOK_STYLE_SCIENTIFIC_NUMBER },
    { XML_NAMESPACE_NUMBER, sXML_fraction,          XML_TOK_STYLE_FRACTION          },
    { XML_NAMESPACE_NUMBER, sXML_currency_symbol,   XML_TOK_STYLE_CURRENCY_SYMBOL   },
    { XML_NAMESPACE_NUMBER, sXML_day,               XML_TOK_STYLE_DAY               },
    { XML_NAMESPACE_NUMBER, sXML_month,             XML_TOK_STYLE_MONTH             },
    { XML_NAMESPACE_NUMBER, sXML_year,              XML_TOK_STYLE_YEAR              },
    { XML_NAMESPACE_NUMBER, sXML_era,               XML_TOK_STYLE_ERA               },
    { XML_NAMESPACE_NUMBER, sXML_day_of_week,       XML_TOK_STYLE_DAY_OF_WEEK       },
    { XML_NAMESPACE_NUMBER, sXML_week_of_year,      XML_TOK_STYLE_WEEK_OF_YEAR      },
    { XML_NAMESPACE_NUMBER, sXML_quarter,           XML_TOK_STYLE_QUARTER           },
    { XML_NAMESPACE_NUMBER, sXML_hours,             XML_TOK_STYLE_HOURS             },
    { XML_NAMESPACE_NUMBER, sXML_am_pm,             XML_TOK_STYLE_AM_PM             },
    { XML_NAMESPACE_NUMBER, sXML_minutes,           XML_TOK_STYLE_MINUTES           },
    { XML_NAMESPACE_NUMBER, sXML_seconds,           XML_TOK_STYLE_SECONDS           },
    { XML_NAMESPACE_NUMBER, sXML_boolean,           XML_TOK_STYLE_BOOLEAN           },
    { XML_NAMESPACE_NUMBER, sXML_text_content,      XML_TOK_STYLE_TEXT_CONTENT      },
    { XML_NAMESPACE_STYLE,  sXML_properties,        XML_TOK_STYLE_PROPERTIES        },
    { XML_NAMESPACE_STYLE,  sXML_map,               XML_TOK_STYLE_MAP               },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStyleAttrMap[] =
{
    //  attributes for a style
    { XML_NAMESPACE_STYLE,  sXML_name,                 XML_TOK_STYLE_ATTR_NAME                  },
    { XML_NAMESPACE_NUMBER, sXML_language,             XML_TOK_STYLE_ATTR_LANGUAGE              },
    { XML_NAMESPACE_NUMBER, sXML_country,              XML_TOK_STYLE_ATTR_COUNTRY               },
    { XML_NAMESPACE_NUMBER, sXML_title,                XML_TOK_STYLE_ATTR_TITLE                 },
    { XML_NAMESPACE_NUMBER, sXML_automatic_order,      XML_TOK_STYLE_ATTR_AUTOMATIC_ORDER       },
    { XML_NAMESPACE_NUMBER, sXML_format_source,        XML_TOK_STYLE_ATTR_FORMAT_SOURCE         },
    { XML_NAMESPACE_NUMBER, sXML_truncate_on_overflow, XML_TOK_STYLE_ATTR_TRUNCATE_ON_OVERFLOW  },
    { XML_NAMESPACE_STYLE,  sXML_volatile,             XML_TOK_STYLE_ATTR_VOLATILE              },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStyleElemAttrMap[] =
{
    //  attributes for an element within a style
    { XML_NAMESPACE_NUMBER, sXML_decimal_places,         XML_TOK_ELEM_ATTR_DECIMAL_PLACES       },
    { XML_NAMESPACE_NUMBER, sXML_min_integer_digits,     XML_TOK_ELEM_ATTR_MIN_INTEGER_DIGITS   },
    { XML_NAMESPACE_NUMBER, sXML_grouping,               XML_TOK_ELEM_ATTR_GROUPING             },
    { XML_NAMESPACE_NUMBER, sXML_decimal_replacement,    XML_TOK_ELEM_ATTR_DECIMAL_REPLACEMENT  },
    { XML_NAMESPACE_NUMBER, sXML_min_exponent_digits,    XML_TOK_ELEM_ATTR_MIN_EXPONENT_DIGITS  },
    { XML_NAMESPACE_NUMBER, sXML_min_numerator_digits,   XML_TOK_ELEM_ATTR_MIN_NUMERATOR_DIGITS },
    { XML_NAMESPACE_NUMBER, sXML_min_denominator_digits, XML_TOK_ELEM_ATTR_MIN_DENOMINATOR_DIGITS },
    { XML_NAMESPACE_NUMBER, sXML_language,               XML_TOK_ELEM_ATTR_LANGUAGE             },
    { XML_NAMESPACE_NUMBER, sXML_country,                XML_TOK_ELEM_ATTR_COUNTRY              },
    { XML_NAMESPACE_NUMBER, sXML_style,                  XML_TOK_ELEM_ATTR_STYLE                },
    { XML_NAMESPACE_NUMBER, sXML_textual,                XML_TOK_ELEM_ATTR_TEXTUAL              },
    { XML_NAMESPACE_NUMBER, sXML_calendar,               XML_TOK_ELEM_ATTR_CALENDAR             },
    XML_TOKEN_MAP_END
};

// maps for SvXMLUnitConverter::convertEnum

static __FAR_DATA SvXMLEnumMapEntry aStyleValueMap[] =
{
    { sXML_short, sal_False },
    { sXML_long,  sal_True  },
    { 0,          0 }
};

static __FAR_DATA SvXMLEnumMapEntry aFormatSourceMap[] =
{
    { sXML_fixed,    sal_False },
    { sXML_language, sal_True  },
    { 0,             0 }
};

//-------------------------------------------------------------------------

SV_IMPL_PTRARR( SvXMLNumFmtEntryArr, SvXMLNumFmtEntryPtr );

//-------------------------------------------------------------------------

//
//  SvXMLNumImpData
//

SvXMLNumImpData::SvXMLNumImpData( SvNumberFormatter* pFmt ) :
    pFormatter(pFmt),
    pStylesElemTokenMap(NULL),
    pStyleElemTokenMap(NULL),
    pStyleAttrTokenMap(NULL),
    pStyleElemAttrTokenMap(NULL),
    pLocaleData(NULL)
{
}

SvXMLNumImpData::~SvXMLNumImpData()
{
    delete pStylesElemTokenMap;
    delete pStyleElemTokenMap;
    delete pStyleAttrTokenMap;
    delete pStyleElemAttrTokenMap;
    delete pLocaleData;
}

sal_uInt32 SvXMLNumImpData::GetKeyForName( const rtl::OUString& rName )
{
    USHORT nCount = aNameEntries.Count();
    for (USHORT i=0; i<nCount; i++)
    {
        const SvXMLNumFmtEntry* pObj = aNameEntries[i];
        if ( pObj->aName == rName )
            return pObj->nKey;              // found
    }
    return NUMBERFORMAT_ENTRY_NOT_FOUND;
}

void SvXMLNumImpData::AddKey( sal_uInt32 nKey, const rtl::OUString& rName )
{
    sal_Bool bVolatile = sal_False;
    SvXMLNumFmtEntry* pObj = new SvXMLNumFmtEntry( rName, nKey, bVolatile );
    aNameEntries.Insert( pObj, aNameEntries.Count() );
}

const SvXMLTokenMap& SvXMLNumImpData::GetStylesElemTokenMap()
{
    if( !pStylesElemTokenMap )
        pStylesElemTokenMap = new SvXMLTokenMap( aStylesElemMap );
    return *pStylesElemTokenMap;
}

const SvXMLTokenMap& SvXMLNumImpData::GetStyleElemTokenMap()
{
    if( !pStyleElemTokenMap )
        pStyleElemTokenMap = new SvXMLTokenMap( aStyleElemMap );
    return *pStyleElemTokenMap;
}

const SvXMLTokenMap& SvXMLNumImpData::GetStyleAttrTokenMap()
{
    if( !pStyleAttrTokenMap )
        pStyleAttrTokenMap = new SvXMLTokenMap( aStyleAttrMap );
    return *pStyleAttrTokenMap;
}

const SvXMLTokenMap& SvXMLNumImpData::GetStyleElemAttrTokenMap()
{
    if( !pStyleElemAttrTokenMap )
        pStyleElemAttrTokenMap = new SvXMLTokenMap( aStyleElemAttrMap );
    return *pStyleElemAttrTokenMap;
}

const LocaleDataWrapper& SvXMLNumImpData::GetLocaleData( LanguageType nLang )
{
    if ( !pLocaleData )
        pLocaleData = new LocaleDataWrapper(
            (pFormatter ? pFormatter->GetServiceManager() :
            ::comphelper::getProcessServiceFactory()),
            SvNumberFormatter::ConvertLanguageToLocale( nLang ) );
    else
        pLocaleData->setLocale( SvNumberFormatter::ConvertLanguageToLocale( nLang ) );
    return *pLocaleData;
}

//-------------------------------------------------------------------------

//
//  SvXMLNumFmtMapContext
//

SvXMLNumFmtMapContext::SvXMLNumFmtMapContext( SvXMLImport& rImport,
                                    USHORT nPrfx, const rtl::OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rParent( rParentContext )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString sValue = xAttrList->getValueByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        if ( nPrefix == XML_NAMESPACE_STYLE )
        {
            if ( aLocalName.compareToAscii(sXML_condition) == 0 )
                sCondition = sValue;
            else if ( aLocalName.compareToAscii(sXML_apply_style_name) == 0 )
                sName = sValue;
        }
    }
}

SvXMLNumFmtMapContext::~SvXMLNumFmtMapContext()
{
}

SvXMLImportContext* SvXMLNumFmtMapContext::CreateChildContext(
                                    USHORT nPrfx, const rtl::OUString& rLName,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    // no elements supported - use default context
    return new SvXMLImportContext( GetImport(), nPrfx, rLName );
}

void SvXMLNumFmtMapContext::Characters( const rtl::OUString& rChars )
{
}

void SvXMLNumFmtMapContext::EndElement()
{
    rParent.AddCondition( sCondition, sName );
}

//-------------------------------------------------------------------------

//
//  SvXMLNumFmtPropContext
//

SvXMLNumFmtPropContext::SvXMLNumFmtPropContext( SvXMLImport& rImport,
                                    USHORT nPrfx, const rtl::OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rParent( rParentContext ),
    bColSet( sal_False )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString sValue = xAttrList->getValueByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        if ( nPrefix == XML_NAMESPACE_FO && aLocalName.compareToAscii(sXML_color) == 0 )
            bColSet = SvXMLUnitConverter::convertColor( aColor, sValue );
    }
}

SvXMLNumFmtPropContext::~SvXMLNumFmtPropContext()
{
}

SvXMLImportContext* SvXMLNumFmtPropContext::CreateChildContext(
                                    USHORT nPrfx, const rtl::OUString& rLName,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    // no elements supported - use default context
    return new SvXMLImportContext( GetImport(), nPrfx, rLName );
}

void SvXMLNumFmtPropContext::Characters( const rtl::OUString& rChars )
{
}

void SvXMLNumFmtPropContext::EndElement()
{
    if (bColSet)
        rParent.AddColor( aColor );
}

//-------------------------------------------------------------------------

sal_Bool lcl_ValidChar( sal_Unicode cChar, sal_uInt16 nFormatType )
{
    //  see ImpSvNumberformatScan::Next_Symbol
    if ( cChar == ' ' ||
         cChar == '-' ||
         cChar == '/' ||
         cChar == '.' ||
         cChar == ',' ||
         cChar == ':' ||
         cChar == '\'' )
        return sal_True;    // for all format types

    //  percent sign must be used without quotes for percentage styles only
    if ( nFormatType == XML_TOK_STYLES_PERCENTAGE_STYLE && cChar == '%' )
        return sal_True;

    return sal_False;
}

//
//  SvXMLNumFmtElementContext
//

SvXMLNumFmtElementContext::SvXMLNumFmtElementContext( SvXMLImport& rImport,
                                    USHORT nPrfx, const rtl::OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext, sal_uInt16 nNewType,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rParent( rParentContext ),
    nType( nNewType ),
    nElementLang( LANGUAGE_SYSTEM ),
    bLong( FALSE ),
    bTextual( FALSE )
{
    OUString sLanguage, sCountry;
    sal_Int32 nAttrVal;
    sal_Bool bAttrBool;
    sal_uInt16 nAttrEnum;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString sValue = xAttrList->getValueByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        const SvXMLTokenMap& rTokenMap = rParent.GetData()->GetStyleElemAttrTokenMap();
        sal_uInt16 nToken = rTokenMap.Get( nPrefix, aLocalName );

        switch (nToken)
        {
            case XML_TOK_ELEM_ATTR_DECIMAL_PLACES:
                if ( SvXMLUnitConverter::convertNumber( nAttrVal, sValue, 0 ) )
                    aNumInfo.nDecimals = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_MIN_INTEGER_DIGITS:
                if ( SvXMLUnitConverter::convertNumber( nAttrVal, sValue, 0 ) )
                    aNumInfo.nInteger = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_GROUPING:
                if ( SvXMLUnitConverter::convertBool( bAttrBool, sValue ) )
                    aNumInfo.bGrouping = bAttrBool;
                break;
            case XML_TOK_ELEM_ATTR_DECIMAL_REPLACEMENT:
                if ( sValue.getLength() > 0 )
                    aNumInfo.bDecReplace = sal_True;    // only a default string is supported
                break;
            case XML_TOK_ELEM_ATTR_MIN_EXPONENT_DIGITS:
                if ( SvXMLUnitConverter::convertNumber( nAttrVal, sValue, 0 ) )
                    aNumInfo.nExpDigits = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_MIN_NUMERATOR_DIGITS:
                if ( SvXMLUnitConverter::convertNumber( nAttrVal, sValue, 0 ) )
                    aNumInfo.nNumerDigits = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_MIN_DENOMINATOR_DIGITS:
                if ( SvXMLUnitConverter::convertNumber( nAttrVal, sValue, 0 ) )
                    aNumInfo.nDenomDigits = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_LANGUAGE:
                sLanguage = sValue;
                break;
            case XML_TOK_ELEM_ATTR_COUNTRY:
                sCountry = sValue;
                break;
            case XML_TOK_ELEM_ATTR_STYLE:
                if ( SvXMLUnitConverter::convertEnum( nAttrEnum, sValue, aStyleValueMap ) )
                    bLong = (sal_Bool) nAttrEnum;
                break;
            case XML_TOK_ELEM_ATTR_TEXTUAL:
                if ( SvXMLUnitConverter::convertBool( bAttrBool, sValue ) )
                    bTextual = bAttrBool;
                break;
            case XML_TOK_ELEM_ATTR_CALENDAR:
                sCalendar = sValue;
                break;
        }
    }

    if ( sLanguage.getLength() || sCountry.getLength() )
    {
        nElementLang = ConvertIsoNamesToLanguage( sLanguage, sCountry );
        if ( nElementLang == LANGUAGE_DONTKNOW )
            nElementLang = LANGUAGE_SYSTEM;         //! error handling for invalid locales?
    }
}

SvXMLNumFmtElementContext::~SvXMLNumFmtElementContext()
{
}

SvXMLImportContext* SvXMLNumFmtElementContext::CreateChildContext(
                                    USHORT nPrfx, const rtl::OUString& rLName,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    // no elements supported - use default context
    return new SvXMLImportContext( GetImport(), nPrfx, rLName );
}

void SvXMLNumFmtElementContext::Characters( const rtl::OUString& rChars )
{
    aContent.append( rChars );
}

void SvXMLNumFmtElementContext::EndElement()
{
    sal_Bool bEffLong = bLong;
    switch (nType)
    {
        case XML_TOK_STYLE_TEXT:
            if ( rParent.HasLongDoW() &&
                    rParent.GetLocaleData().getLongDateDayOfWeekSep() ==
                        String( aContent.getStr() ) )
            {
                //  skip separator constant after long day of week
                //  (NF_KEY_NNNN contains the separator)

                if ( rParent.ReplaceNfKeyword( NF_KEY_NNN, NF_KEY_NNNN ) )
                {
                    //!aContent.setLength(0);       //! doesn't work, #76293#
                    aContent = OUStringBuffer();
                }

                rParent.SetHasLongDoW( sal_False );     // only once
            }
            if ( aContent.getLength() )
            {
                if ( ( aContent.getLength() == 1 &&
                        lcl_ValidChar( aContent.charAt(0), rParent.GetType() ) ) ||
                     ( aContent.getLength() == 2 &&
                         lcl_ValidChar( aContent.charAt(0), rParent.GetType() ) &&
                         aContent.charAt(1) == ' ' ) )
                {
                    //  don't quote single separator characters like space or percent,
                    //  or separator characters followed by space (used in date formats)
                }
                else
                {
                    //  quote string literals
                    //! escape quotes in string
                    aContent.insert( 0, (sal_Unicode) '"' );
                    aContent.append( (sal_Unicode) '"' );
                }
                rParent.AddToCode( aContent.makeStringAndClear() );
            }
            break;

        case XML_TOK_STYLE_NUMBER:
            rParent.AddNumber( aNumInfo );
            break;

        case XML_TOK_STYLE_CURRENCY_SYMBOL:
            rParent.AddCurrency( aContent.makeStringAndClear(), nElementLang );
            break;

        case XML_TOK_STYLE_TEXT_CONTENT:
            rParent.AddToCode( OUString::valueOf((sal_Unicode)'@') );
            break;
        case XML_TOK_STYLE_BOOLEAN:
            // ignored - only default boolean format is supported
            break;

        case XML_TOK_STYLE_DAY:
            rParent.UpdateCalendar( sCalendar );
#if 0
//! I18N doesn't provide SYSTEM or extended date information yet
            if ( rParent.IsFromSystem() )
                bEffLong = SvXMLNumFmtDefaults::IsSystemLongDay( rParent.GetInternational(), bLong );
#endif
            rParent.AddNfKeyword( bEffLong ? NF_KEY_DD : NF_KEY_D );
            break;
        case XML_TOK_STYLE_MONTH:
            rParent.UpdateCalendar( sCalendar );
#if 0
//! I18N doesn't provide SYSTEM or extended date information yet
            if ( rParent.IsFromSystem() )
            {
                bEffLong = SvXMLNumFmtDefaults::IsSystemLongMonth( rParent.GetInternational(), bLong );
                bTextual = SvXMLNumFmtDefaults::IsSystemTextualMonth( rParent.GetInternational(), bLong );
            }
#endif
            rParent.AddNfKeyword( bTextual ? ( bEffLong ? NF_KEY_MMMM : NF_KEY_MMM ) :
                                             ( bEffLong ? NF_KEY_MM : NF_KEY_M ) );
            break;
        case XML_TOK_STYLE_YEAR:
            rParent.UpdateCalendar( sCalendar );
#if 0
//! I18N doesn't provide SYSTEM or extended date information yet
            if ( rParent.IsFromSystem() )
                bEffLong = SvXMLNumFmtDefaults::IsSystemLongYear( rParent.GetInternational(), bLong );
#endif
            // Y after G (era) is replaced by E
            if ( rParent.HasEra() )
                rParent.AddNfKeyword( bEffLong ? NF_KEY_EEC : NF_KEY_EC );
            else
                rParent.AddNfKeyword( bEffLong ? NF_KEY_YYYY : NF_KEY_YY );
            break;
        case XML_TOK_STYLE_ERA:
            rParent.UpdateCalendar( sCalendar );
#if 0
//! I18N doesn't provide SYSTEM or extended date information yet
            if ( rParent.IsFromSystem() )
                bEffLong = SvXMLNumFmtDefaults::IsSystemLongEra( rParent.GetInternational(), bLong );
#endif
            rParent.AddNfKeyword( bEffLong ? NF_KEY_GGG : NF_KEY_G );
            //  HasEra flag is set
            break;
        case XML_TOK_STYLE_DAY_OF_WEEK:
            rParent.UpdateCalendar( sCalendar );
#if 0
//! I18N doesn't provide SYSTEM or extended date information yet
            if ( rParent.IsFromSystem() )
                bEffLong = SvXMLNumFmtDefaults::IsSystemLongDayOfWeek( rParent.GetInternational(), bLong );
#endif
            rParent.AddNfKeyword( bEffLong ? NF_KEY_NNNN : NF_KEY_NN );
            break;
        case XML_TOK_STYLE_WEEK_OF_YEAR:
            rParent.UpdateCalendar( sCalendar );
            rParent.AddNfKeyword( NF_KEY_WW );
            break;
        case XML_TOK_STYLE_QUARTER:
            rParent.UpdateCalendar( sCalendar );
            rParent.AddNfKeyword( bEffLong ? NF_KEY_QQ : NF_KEY_Q );
            break;
        case XML_TOK_STYLE_HOURS:
            rParent.AddNfKeyword( bEffLong ? NF_KEY_HH : NF_KEY_H );
            break;
        case XML_TOK_STYLE_AM_PM:
            //! short/long?
            rParent.AddNfKeyword( NF_KEY_AMPM );
            break;
        case XML_TOK_STYLE_MINUTES:
            rParent.AddNfKeyword( bEffLong ? NF_KEY_MMI : NF_KEY_MI );
            break;
        case XML_TOK_STYLE_SECONDS:
            rParent.AddNfKeyword( bEffLong ? NF_KEY_SS : NF_KEY_S );
            if ( aNumInfo.nDecimals > 0 )
            {
                //  manually add the decimal places
                const String& rSep = rParent.GetLocaleData().getNumDecimalSep();
                for ( xub_StrLen j=0; j<rSep.Len(); j++ )
                {
                    rParent.AddToCode( OUString::valueOf( rSep.GetChar(j) ) );
                }
                for (sal_Int32 i=0; i<aNumInfo.nDecimals; i++)
                    rParent.AddToCode( OUString::valueOf((sal_Unicode)'0') );
            }
            break;

        case XML_TOK_STYLE_FRACTION:
            {
                aNumInfo.nDecimals = 0;
                rParent.AddNumber( aNumInfo );      // number without decimals

                //! build string and add at once

                sal_Int32 i;
                rParent.AddToCode( OUString::valueOf((sal_Unicode)' ') );
                for (i=0; i<aNumInfo.nNumerDigits; i++)
                    rParent.AddToCode( OUString::valueOf((sal_Unicode)'?') );
                rParent.AddToCode( OUString::valueOf((sal_Unicode)'/') );
                for (i=0; i<aNumInfo.nDenomDigits; i++)
                    rParent.AddToCode( OUString::valueOf((sal_Unicode)'?') );
            }
            break;

        case XML_TOK_STYLE_SCIENTIFIC_NUMBER:
            {
                rParent.AddNumber( aNumInfo );      // simple number

                rParent.AddToCode( OUString::createFromAscii( "E+" ) );
                for (sal_Int32 i=0; i<aNumInfo.nExpDigits; i++)
                    rParent.AddToCode( OUString::valueOf((sal_Unicode)'0') );
            }
            break;

        default:
            DBG_ERROR("invalid element ID");
    }
}

//-------------------------------------------------------------------------

sal_Bool SvXMLNumFmtDefaults::IsSystemLongDay( const International& rIntn, BOOL bLong )
{
    return bLong ? rIntn.IsLongDateDayLeadingZero() : rIntn.IsDateDayLeadingZero();
}

sal_Bool SvXMLNumFmtDefaults::IsSystemLongMonth( const International& rIntn, BOOL bLong )
{
    if (bLong)
    {
        MonthFormat eMonth = rIntn.GetLongDateMonthFormat();
        return ( eMonth == MONTH_ZERO || eMonth == MONTH_LONG );
    }
    else
        return rIntn.IsDateMonthLeadingZero();
}

sal_Bool SvXMLNumFmtDefaults::IsSystemTextualMonth( const International& rIntn, BOOL bLong )
{
    if (bLong)
    {
        MonthFormat eMonth = rIntn.GetLongDateMonthFormat();
        return ( eMonth == MONTH_SHORT || eMonth == MONTH_LONG );
    }
    else
        return sal_False;
}

sal_Bool SvXMLNumFmtDefaults::IsSystemLongYear( const International& rIntn, BOOL bLong )
{
    return bLong ? rIntn.IsLongDateCentury() : rIntn.IsDateCentury();
}

sal_Bool SvXMLNumFmtDefaults::IsSystemLongEra( const International& rIntn, BOOL bLong )
{
    return IsSystemLongYear( rIntn, bLong );        // no separate setting
}

sal_Bool SvXMLNumFmtDefaults::IsSystemLongDayOfWeek( const International& rIntn, BOOL bLong )
{
    return ( bLong && rIntn.GetLongDateDayOfWeekFormat() == DAYOFWEEK_LONG );
}

//-------------------------------------------------------------------------

//
//  SvXMLNumFormatContext
//

SvXMLNumFormatContext::SvXMLNumFormatContext( SvXMLImport& rImport,
                                    USHORT nPrfx, const rtl::OUString& rLName,
                                    SvXMLNumImpData* pNewData, sal_uInt16 nNewType,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                    SvXMLStylesContext& rStyles ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList ),
    pData( pNewData ),
    aMyConditions(),
    nType( nNewType ),
    nFormatLang( LANGUAGE_SYSTEM ),
    bAutoOrder( FALSE ),
    bFromSystem( FALSE ),
    bTruncate( TRUE ),
    bAutoDec( FALSE ),
    bAutoInt( FALSE ),
    bHasExtraText( FALSE ),
    bHasLongDoW( FALSE ),
    bHasEra( FALSE ),
    bHasDateTime( FALSE ),
    bHasMap( sal_False ),
    pStyles( &rStyles ),
    nKey(-1)
{
    OUString sLanguage, sCountry;
    sal_Bool bAttrBool;
    sal_uInt16 nAttrEnum;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString sValue = xAttrList->getValueByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        const SvXMLTokenMap& rTokenMap = pData->GetStyleAttrTokenMap();
        sal_uInt16 nToken = rTokenMap.Get( nPrefix, aLocalName );
        switch (nToken)
        {
            case XML_TOK_STYLE_ATTR_NAME:
//              aName = sValue;
                break;
            case XML_TOK_STYLE_ATTR_LANGUAGE:
                sLanguage = sValue;
                break;
            case XML_TOK_STYLE_ATTR_COUNTRY:
                sCountry = sValue;
                break;
            case XML_TOK_STYLE_ATTR_TITLE:
                sFormatTitle = sValue;
                break;
            case XML_TOK_STYLE_ATTR_AUTOMATIC_ORDER:
                if ( SvXMLUnitConverter::convertBool( bAttrBool, sValue ) )
                    bAutoOrder = bAttrBool;
                break;
            case XML_TOK_STYLE_ATTR_FORMAT_SOURCE:
                if ( SvXMLUnitConverter::convertEnum( nAttrEnum, sValue, aFormatSourceMap ) )
                    bFromSystem = (sal_Bool) nAttrEnum;
                break;
            case XML_TOK_STYLE_ATTR_TRUNCATE_ON_OVERFLOW:
                if ( SvXMLUnitConverter::convertBool( bAttrBool, sValue ) )
                    bTruncate = bAttrBool;
                break;
            case XML_TOK_STYLE_ATTR_VOLATILE:
                //! ...
                break;
        }
    }

    if ( sLanguage.getLength() || sCountry.getLength() )
    {
        nFormatLang = ConvertIsoNamesToLanguage( sLanguage, sCountry );
        if ( nFormatLang == LANGUAGE_DONTKNOW )
            nFormatLang = LANGUAGE_SYSTEM;          //! error handling for invalid locales?
    }
}

SvXMLNumFormatContext::~SvXMLNumFormatContext()
{
}

SvXMLImportContext* SvXMLNumFormatContext::CreateChildContext(
                                    USHORT nPrfx, const rtl::OUString& rLName,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    const SvXMLTokenMap& rTokenMap = pData->GetStyleElemTokenMap();
    sal_uInt16 nToken = rTokenMap.Get( nPrfx, rLName );
    switch (nToken)
    {
        case XML_TOK_STYLE_TEXT:
        case XML_TOK_STYLE_NUMBER:
        case XML_TOK_STYLE_SCIENTIFIC_NUMBER:
        case XML_TOK_STYLE_FRACTION:
        case XML_TOK_STYLE_CURRENCY_SYMBOL:
        case XML_TOK_STYLE_DAY:
        case XML_TOK_STYLE_MONTH:
        case XML_TOK_STYLE_YEAR:
        case XML_TOK_STYLE_ERA:
        case XML_TOK_STYLE_DAY_OF_WEEK:
        case XML_TOK_STYLE_WEEK_OF_YEAR:
        case XML_TOK_STYLE_QUARTER:
        case XML_TOK_STYLE_HOURS:
        case XML_TOK_STYLE_AM_PM:
        case XML_TOK_STYLE_MINUTES:
        case XML_TOK_STYLE_SECONDS:
        case XML_TOK_STYLE_BOOLEAN:
        case XML_TOK_STYLE_TEXT_CONTENT:
            pContext = new SvXMLNumFmtElementContext( GetImport(), nPrfx, rLName,
                                                        *this, nToken, xAttrList );
            break;

        case XML_TOK_STYLE_PROPERTIES:
            pContext = new SvXMLNumFmtPropContext( GetImport(), nPrfx, rLName,
                                                        *this, xAttrList );
            break;
        case XML_TOK_STYLE_MAP:
            {
                bHasMap = sal_True;
                pContext = new SvXMLNumFmtMapContext( GetImport(), nPrfx, rLName,
                                                            *this, xAttrList );
            }
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrfx, rLName );
    return pContext;
}

sal_Int32 SvXMLNumFormatContext::GetKey()
{
    if (nKey > -1)
        return nKey;
    else
    {
        CreateAndInsert(sal_True);
        return nKey;
    }
}

void SvXMLNumFormatContext::CreateAndInsert(sal_Bool bOverwrite)
{
    if (!(nKey > -1))
    {
        SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
        if (!pFormatter)
        {
            DBG_ERROR("no number formatter");
            return;
        }

        sal_uInt32 nIndex = NUMBERFORMAT_ENTRY_NOT_FOUND;

        if (bHasMap)
        {
            for (sal_Int32 i = 0; i < aMyConditions.size(); i++)
            {
                SvXMLNumFormatContext* pStyle = (SvXMLNumFormatContext *)pStyles->FindStyleChildContext(
                    XML_STYLE_FAMILY_DATA_STYLE, aMyConditions[i].sMapName, sal_False);
                if (pStyle)
                {
                    if ((pStyle->GetKey() > -1))
                        AddCondition(i);
                }
            }
        }
        aFormatCode.insert( 0, aConditions.makeStringAndClear() );
        OUString sFormat = aFormatCode.makeStringAndClear();

        //  test special cases

        if ( bAutoDec )         // automatic decimal places
        {
            if ( nType == XML_TOK_STYLES_NUMBER_STYLE && !bHasExtraText )
                nIndex = pFormatter->GetStandardIndex( nFormatLang );
        }
        if ( bAutoInt )         // automatic integer digits
        {
            //! only if two decimal places was set?

            if ( nType == XML_TOK_STYLES_NUMBER_STYLE && !bHasExtraText )
                nIndex = pFormatter->GetFormatIndex( NF_NUMBER_SYSTEM, nFormatLang );
        }

        //  boolean is always the builtin boolean format
        //  (no other boolean formats are implemented)
        if ( nType == XML_TOK_STYLES_BOOLEAN_STYLE )
            nIndex = pFormatter->GetFormatIndex( NF_BOOLEAN, nFormatLang );

        if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND && sFormat.getLength() )
        {
            //  insert by format string

            String aFormatStr( sFormat );
            nIndex = pFormatter->GetEntryKey( aFormatStr, nFormatLang );
            if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND )
            {
                xub_StrLen  nErrPos = 0;
                short       nType   = 0;
                sal_Bool bOk = pFormatter->PutEntry( aFormatStr, nErrPos, nType, nIndex, nFormatLang );
                if (!bOk)
                    nIndex = NUMBERFORMAT_ENTRY_NOT_FOUND;
            }
        }

#if 0
//! I18N doesn't provide SYSTEM or extended date information yet
        if ( nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND && !bFromSystem )
        {
            //  instead of automatic date format, use fixed formats if bFromSystem is not set
            //! prevent use of automatic formats in other cases, force user-defined format?

            sal_uInt32 nNewIndex = nIndex;

            NfIndexTableOffset eOffset = pFormatter->GetIndexTableOffset( nIndex );
            if ( eOffset == NF_DATE_SYSTEM_SHORT )
            {
                const International& rInt = pData->GetInternational( nFormatLang );
                if ( rInt.IsDateDayLeadingZero() && rInt.IsDateMonthLeadingZero() )
                {
                    if ( rInt.IsDateCentury() )
                        nNewIndex = pFormatter->GetFormatIndex( NF_DATE_SYS_DDMMYYYY, nFormatLang );
                    else
                        nNewIndex = pFormatter->GetFormatIndex( NF_DATE_SYS_DDMMYY, nFormatLang );
                }
            }
            else if ( eOffset == NF_DATE_SYSTEM_LONG )
            {
                const International& rInt = pData->GetInternational( nFormatLang );
                if ( !rInt.IsLongDateDayLeadingZero() )
                {
                    sal_Bool bCentury = rInt.IsLongDateCentury();
                    MonthFormat eMonth = rInt.GetLongDateMonthFormat();
                    if ( eMonth == MONTH_LONG && bCentury )
                    {
                        if ( rInt.GetLongDateDayOfWeekFormat() == DAYOFWEEK_LONG )
                            nNewIndex = pFormatter->GetFormatIndex( NF_DATE_SYS_NNNNDMMMMYYYY, nFormatLang );
                        else
                            nNewIndex = pFormatter->GetFormatIndex( NF_DATE_SYS_NNDMMMMYYYY, nFormatLang );
                    }
                    else if ( eMonth == MONTH_SHORT && !bCentury )
                        nNewIndex = pFormatter->GetFormatIndex( NF_DATE_SYS_NNDMMMYY, nFormatLang );
                }
            }

            if ( nNewIndex != nIndex )
            {
                //  verify the fixed format really matches the format string
                //  (not the case with some formats from locale data)

                const SvNumberformat* pFixedFormat = pFormatter->GetEntry( nNewIndex );
                if ( pFixedFormat && pFixedFormat->GetFormatstring() == String(sFormat) )
                    nIndex = nNewIndex;
            }
        }
#endif

        if ( nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND && !bAutoOrder )
        {
            //  use fixed-order formats instead of SYS... if bAutoOrder is false

            NfIndexTableOffset eOffset = pFormatter->GetIndexTableOffset( nIndex );
            if ( eOffset == NF_DATE_SYS_DMMMYYYY )
                nIndex = pFormatter->GetFormatIndex( NF_DATE_DIN_DMMMYYYY, nFormatLang );
            else if ( eOffset == NF_DATE_SYS_DMMMMYYYY )
                nIndex = pFormatter->GetFormatIndex( NF_DATE_DIN_DMMMMYYYY, nFormatLang );
        }

        if ((nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND) && sFormatTitle.getLength())
        {
            SvNumberformat* pFormat = const_cast<SvNumberformat*>(pFormatter->GetEntry( nIndex ));
            if (pFormat)
            {
                String sTitle (sFormatTitle);
                pFormat->SetComment(sTitle);
            }
        }

        pData->AddKey( nIndex, GetName() );
        nKey = nIndex;

    #if 0
        ByteString aByte( String(sFormatName), gsl_getSystemTextEncoding() );
        aByte.Append( " | " );
        aByte.Append(ByteString( String(sFormat), gsl_getSystemTextEncoding() ));
        aByte.Append( " | " );
        aByte.Append(ByteString::CreateFromInt32( nIndex ));

    //  DBG_ERROR(aByte.GetBuffer());
        int xxx=42;
    #endif
    }
}

void SvXMLNumFormatContext::Finish( sal_Bool bOverwrite )
{
    SvXMLStyleContext::Finish( bOverwrite );
//  AddCondition();
}

const LocaleDataWrapper& SvXMLNumFormatContext::GetLocaleData() const
{
    return pData->GetLocaleData( nFormatLang );
}

void SvXMLNumFormatContext::AddToCode( const rtl::OUString& rString )
{
    aFormatCode.append( rString );
    bHasExtraText = sal_True;
}

void SvXMLNumFormatContext::AddNumber( const SvXMLNumberInfo& rInfo )
{
    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    if (!pFormatter)
        return;

    //  store special conditions
    bAutoDec = ( rInfo.nDecimals < 0 );
    bAutoInt = ( rInfo.nInteger < 0 );

    sal_uInt16 nPrec = 0;
    sal_uInt16 nLeading = 0;
    if ( rInfo.nDecimals >= 0 )                     //  < 0 : Default
        nPrec = (sal_uInt16) rInfo.nDecimals;
    if ( rInfo.nInteger >= 0 )                      //  < 0 : Default
        nLeading = (sal_uInt16) rInfo.nInteger;

    if ( bAutoDec )
    {
        //  use language defaults for other than builtin formats

        const LocaleDataWrapper& rLoc = pData->GetLocaleData( nFormatLang );

        if ( nType == XML_TOK_STYLES_CURRENCY_STYLE )
            nPrec = rLoc.getCurrDigits();
        else
            nPrec = 2; //! was rInt.GetNumDigits(), how about LocaleData providing this?
    }
    if ( bAutoInt )
    {
        //!...
    }

    sal_uInt16 nGenPrec = nPrec;
    if ( rInfo.bDecReplace )
        nGenPrec = 0;               // generate format without decimals...

    String aNumStr;
    sal_uInt32 nStdIndex = pFormatter->GetStandardIndex( nFormatLang );
    pFormatter->GenerateFormat( aNumStr, nStdIndex, nFormatLang,
                                rInfo.bGrouping, sal_False, nGenPrec, nLeading );

    aFormatCode.append( aNumStr );

    if ( rInfo.bDecReplace && nPrec )       // add decimal replacement (dashes)
    {
        aFormatCode.append( pData->GetLocaleData( nFormatLang ).getNumDecimalSep() );
        for ( sal_uInt16 i=0; i<nPrec; i++)
            aFormatCode.append( (sal_Unicode)'-' );
    }
}

void SvXMLNumFormatContext::AddCurrency( const rtl::OUString& rContent, LanguageType nLang )
{
    sal_Bool bAutomatic = sal_False;
    OUString aSymbol = rContent;
    if ( aSymbol.getLength() == 0 )
    {
        //  get currency symbol for language
        aSymbol = pData->GetLocaleData( nFormatLang ).getCurrSymbol();
        bAutomatic = sal_True;
    }
    else if ( nLang == LANGUAGE_SYSTEM && aSymbol.compareToAscii("CCC") == 0 )
    {
        //  "CCC" is used for automatic long symbol
        bAutomatic = sal_True;
    }

    if (!bAutomatic)
        aFormatCode.appendAscii( "[$" );            // intro for "new" currency symbols

    aFormatCode.append( aSymbol );

    if (!bAutomatic)
    {
        if ( nLang != LANGUAGE_SYSTEM )
        {
            //  '-' sign and language code in hex:
            aFormatCode.append( (sal_Unicode) '-' );
            aFormatCode.append( String::CreateFromInt32( sal_Int32( nLang ), 16 ).ToUpperAscii() );
        }

        aFormatCode.append( (sal_Unicode) ']' );    // end of "new" currency symbol
    }
}

void SvXMLNumFormatContext::AddNfKeyword( sal_uInt16 nIndex )
{
    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    if (!pFormatter)
        return;

    if ( nIndex == NF_KEY_G || nIndex == NF_KEY_GG || nIndex == NF_KEY_GGG )
        bHasEra = sal_True;

    if ( nIndex == NF_KEY_NNNN )
    {
        nIndex = NF_KEY_NNN;
        bHasLongDoW = sal_True;         // to remove string constant with separator
    }

    String sKeyword = pFormatter->GetKeyword( nFormatLang, nIndex );

    if ( nIndex == NF_KEY_H  || nIndex == NF_KEY_HH  ||
         nIndex == NF_KEY_MI || nIndex == NF_KEY_MMI ||
         nIndex == NF_KEY_S  || nIndex == NF_KEY_SS )
    {
        if ( !bTruncate && !bHasDateTime )
        {
            //  with truncate-on-overflow = false, add "[]" to first time part

            sKeyword.Insert( (sal_Unicode) '[', 0 );
            sKeyword.Append( (sal_Unicode) ']' );
        }
        bHasDateTime = sal_True;
    }

    aFormatCode.append( sKeyword );
}

sal_Bool lcl_IsAtEnd( rtl::OUStringBuffer& rBuffer, const String& rToken )
{
    sal_Int32 nBufLen = rBuffer.getLength();
    xub_StrLen nTokLen = rToken.Len();

    if ( nTokLen > nBufLen )
        return sal_False;

    sal_Int32 nStartPos = nTokLen - nBufLen;
    for ( xub_StrLen nTokPos = 0; nTokPos < nTokLen; nTokPos++ )
        if ( rToken.GetChar( nTokPos ) != rBuffer.charAt( nStartPos + nTokPos ) )
            return sal_False;

    return sal_True;
}

sal_Bool SvXMLNumFormatContext::ReplaceNfKeyword( sal_uInt16 nOld, sal_uInt16 nNew )
{
    //  replaces one keyword with another if it is found at the end of the code

    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    if (!pFormatter)
        return sal_False;

    String sOldStr = pFormatter->GetKeyword( nFormatLang, nOld );
    if ( lcl_IsAtEnd( aFormatCode, sOldStr ) )
    {
        // remove old keyword
        aFormatCode.setLength( aFormatCode.getLength() - sOldStr.Len() );

        // add new keyword
        String sNewStr = pFormatter->GetKeyword( nFormatLang, nNew );
        aFormatCode.append( sNewStr );

        return sal_True;    // changed
    }
    return sal_False;       // not found
}

void SvXMLNumFormatContext::AddCondition( const sal_Int32 nIndex )
{
    rtl::OUString rApplyName = aMyConditions[nIndex].sMapName;
    rtl::OUString rCondition = aMyConditions[nIndex].sCondition;
    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    sal_uInt32 nKey = pData->GetKeyForName( rApplyName );
    OUString sValue = OUString::createFromAscii( "value()" );       //! define constant
    sal_Int32 nValLen = sValue.getLength();

    if ( pFormatter && nKey != NUMBERFORMAT_ENTRY_NOT_FOUND &&
            rCondition.copy( 0, nValLen ) == sValue )
    {
        //! test for valid conditions
        //! test for default conditions

        OUString sRealCond = rCondition.copy( nValLen, rCondition.getLength() - nValLen );
        sal_Bool bDefaultCond = sal_False;

        //! collect all conditions first and adjust default to >=0, >0 or <0 depending on count
        //! allow blanks in conditions
        sal_Bool bFirstCond = ( aConditions.getLength() == 0 );
        if ( bFirstCond && sRealCond.compareToAscii( ">=0" ) == 0 )
            bDefaultCond = sal_True;

        if (!bDefaultCond)
        {
            aConditions.append( (sal_Unicode) '[' );
            aConditions.append( sRealCond );
            aConditions.append( (sal_Unicode) ']' );
        }

        const SvNumberformat* pFormat = pFormatter->GetEntry(nKey);
        if ( pFormat )
            aConditions.append( OUString( pFormat->GetFormatstring() ) );

        aConditions.append( (sal_Unicode) ';' );
    }
}

void SvXMLNumFormatContext::AddCondition( const rtl::OUString& rCondition, const rtl::OUString& rApplyName )
{
    MyCondition aCondition;
    aCondition.sCondition = rCondition;
    aCondition.sMapName = rApplyName;
    aMyConditions.push_back(aCondition);
}

void SvXMLNumFormatContext::AddColor( const Color& rColor )
{
    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    if (!pFormatter)
        return;

    OUStringBuffer aColName;
    for ( sal_uInt16 i=0; i<XML_NUMF_COLORCOUNT; i++ )
        if ( rColor == aNumFmtStdColors[i] )
        {
            aColName = OUString( pFormatter->GetKeyword( nFormatLang, NF_KEY_FIRSTCOLOR + i ) );
            break;
        }

    if ( aColName.getLength() )
    {
        aColName.insert( 0, (sal_Unicode) '[' );
        aColName.append( (sal_Unicode) ']' );
        aFormatCode.insert( 0, aColName.makeStringAndClear() );
    }
}

void SvXMLNumFormatContext::UpdateCalendar( const rtl::OUString& rNewCalendar )
{
    if ( rNewCalendar != sCalendar )
    {
        sCalendar = rNewCalendar;
        if ( sCalendar.getLength() )
        {
            aFormatCode.appendAscii( "[~" );            // intro for calendar code
            aFormatCode.append( sCalendar );
            aFormatCode.append( (sal_Unicode) ']' );    // end of "new" currency symbolcalendar code
        }
    }
}

//-------------------------------------------------------------------------

//
//  SvXMLNumFmtHelper
//

SvXMLNumFmtHelper::SvXMLNumFmtHelper(
                        const uno::Reference<util::XNumberFormatsSupplier>& rSupp )
{
    SvNumberFormatter* pFormatter = NULL;
    SvNumberFormatsSupplierObj* pObj =
                    SvNumberFormatsSupplierObj::getImplementation( rSupp );
    if (pObj)
        pFormatter = pObj->GetNumberFormatter();

    pData = new SvXMLNumImpData( pFormatter );
}

SvXMLNumFmtHelper::~SvXMLNumFmtHelper()
{
    delete pData;
}

SvXMLStyleContext*  SvXMLNumFmtHelper::CreateChildContext( SvXMLImport& rImport,
                USHORT nPrefix, const OUString& rLocalName,
                const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                SvXMLStylesContext& rStyles )
{
    SvXMLStyleContext* pContext = NULL;

    const SvXMLTokenMap& rTokenMap = pData->GetStylesElemTokenMap();
    sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );
    switch (nToken)
    {
        case XML_TOK_STYLES_NUMBER_STYLE:
        case XML_TOK_STYLES_CURRENCY_STYLE:
        case XML_TOK_STYLES_PERCENTAGE_STYLE:
        case XML_TOK_STYLES_DATE_STYLE:
        case XML_TOK_STYLES_TIME_STYLE:
        case XML_TOK_STYLES_BOOLEAN_STYLE:
        case XML_TOK_STYLES_TEXT_STYLE:
            pContext = new SvXMLNumFormatContext( rImport, nPrefix, rLocalName,
                                                    pData, nToken, xAttrList, rStyles );
            break;
    }

    // return NULL if not a data style, caller must handle other elements
    return pContext;
}

/*sal_uInt32 SvXMLNumFmtHelper::GetKeyForName( const rtl::OUString& rName )
{
    return pData->GetKeyForName( rName );
}*/


