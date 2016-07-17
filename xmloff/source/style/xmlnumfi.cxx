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

#include <comphelper/string.hxx>
#include <unotools/syslocale.hxx>

#include <svl/zforlist.hxx>

#include <svl/zformat.hxx>
#include <svl/numuno.hxx>
#include <rtl/math.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/color.hxx>
#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmlnumfi.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/languagetagodf.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

struct SvXMLNumFmtEntry
{
    OUString   aName;
    sal_uInt32  nKey;
    bool        bRemoveAfterUse;

    SvXMLNumFmtEntry( const OUString& rN, sal_uInt32 nK, bool bR ) :
        aName(rN), nKey(nK), bRemoveAfterUse(bR) {}
};

class SvXMLNumImpData
{
    SvNumberFormatter*  pFormatter;
    SvXMLTokenMap*      pStylesElemTokenMap;
    SvXMLTokenMap*      pStyleElemTokenMap;
    SvXMLTokenMap*      pStyleAttrTokenMap;
    SvXMLTokenMap*      pStyleElemAttrTokenMap;
    LocaleDataWrapper*  pLocaleData;
    std::vector<SvXMLNumFmtEntry> m_NameEntries;

    uno::Reference< uno::XComponentContext > m_xContext;

public:
    SvXMLNumImpData(
        SvNumberFormatter* pFmt,
        const uno::Reference<uno::XComponentContext>& rxContext );
    ~SvXMLNumImpData();

    SvNumberFormatter*      GetNumberFormatter() const  { return pFormatter; }
    const SvXMLTokenMap&    GetStylesElemTokenMap();
    const SvXMLTokenMap&    GetStyleElemTokenMap();
    const SvXMLTokenMap&    GetStyleAttrTokenMap();
    const SvXMLTokenMap&    GetStyleElemAttrTokenMap();
    const LocaleDataWrapper&    GetLocaleData( LanguageType nLang );
    sal_uInt32              GetKeyForName( const OUString& rName );
    void                    AddKey( sal_uInt32 nKey, const OUString& rName, bool bRemoveAfterUse );
    void                    SetUsed( sal_uInt32 nKey );
    void                    RemoveVolatileFormats();
};

struct SvXMLNumberInfo
{
    sal_Int32   nDecimals;
    sal_Int32   nInteger;
    sal_Int32   nExpDigits;
    sal_Int32   nExpInterval;
    sal_Int32   nMinNumerDigits;
    sal_Int32   nMinDenomDigits;
    sal_Int32   nMaxNumerDigits;
    sal_Int32   nMaxDenomDigits;
    sal_Int32   nFracDenominator;
    sal_Int32   nMinDecimalDigits;
    sal_Int32   nZerosNumerDigits;
    sal_Int32   nZerosDenomDigits;
    bool        bGrouping;
    bool        bDecReplace;
    bool        bExpSign;
    bool        bDecAlign;
    double      fDisplayFactor;
    std::map<sal_Int32, OUString> m_EmbeddedElements;

    SvXMLNumberInfo()
    {
        nDecimals = nInteger = nExpDigits = nExpInterval = nMinNumerDigits = nMinDenomDigits = nMaxNumerDigits = nMaxDenomDigits =
            nFracDenominator = nMinDecimalDigits = nZerosNumerDigits = nZerosDenomDigits = -1;
        bGrouping = bDecReplace = bDecAlign = false;
        bExpSign = true;
        fDisplayFactor = 1.0;
    }
};

class SvXMLNumFmtElementContext : public SvXMLImportContext
{
    SvXMLNumFormatContext&  rParent;
    sal_uInt16              nType;
    OUStringBuffer          aContent;
    SvXMLNumberInfo         aNumInfo;
    LanguageType            nElementLang;
    bool                    bLong;
    bool                    bTextual;
    OUString                sCalendar;

public:
                SvXMLNumFmtElementContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext, sal_uInt16 nNewType,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList );
    virtual     ~SvXMLNumFmtElementContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void Characters( const OUString& rChars ) override;
    virtual void EndElement() override;

    void    AddEmbeddedElement( sal_Int32 nFormatPos, const OUString& rContent );
};

class SvXMLNumFmtEmbeddedTextContext : public SvXMLImportContext
{
    SvXMLNumFmtElementContext&  rParent;
    OUStringBuffer         aContent;
    sal_Int32                   nTextPosition;

public:
                SvXMLNumFmtEmbeddedTextContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    SvXMLNumFmtElementContext& rParentContext,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList );
    virtual     ~SvXMLNumFmtEmbeddedTextContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void Characters( const OUString& rChars ) override;
    virtual void EndElement() override;
};

class SvXMLNumFmtMapContext : public SvXMLImportContext
{
    SvXMLNumFormatContext&  rParent;
    OUString           sCondition;
    OUString           sName;

public:
                SvXMLNumFmtMapContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList );
    virtual     ~SvXMLNumFmtMapContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void Characters( const OUString& rChars ) override;
    virtual void EndElement() override;
};

class SvXMLNumFmtPropContext : public SvXMLImportContext
{
    SvXMLNumFormatContext&  rParent;
    sal_Int32               m_nColor;
    bool                    bColSet;

public:
                SvXMLNumFmtPropContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList );
    virtual     ~SvXMLNumFmtPropContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void Characters( const OUString& rChars ) override;
    virtual void EndElement() override;
};

enum SvXMLStyleTokens
{
    XML_TOK_STYLE_TEXT,
    XML_TOK_STYLE_FILL_CHARACTER,
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
    XML_TOK_STYLE_ATTR_RFC_LANGUAGE_TAG,
    XML_TOK_STYLE_ATTR_LANGUAGE,
    XML_TOK_STYLE_ATTR_SCRIPT,
    XML_TOK_STYLE_ATTR_COUNTRY,
    XML_TOK_STYLE_ATTR_TITLE,
    XML_TOK_STYLE_ATTR_AUTOMATIC_ORDER,
    XML_TOK_STYLE_ATTR_FORMAT_SOURCE,
    XML_TOK_STYLE_ATTR_TRUNCATE_ON_OVERFLOW,
    XML_TOK_STYLE_ATTR_VOLATILE,
    XML_TOK_STYLE_ATTR_TRANSL_FORMAT,
    XML_TOK_STYLE_ATTR_TRANSL_RFC_LANGUAGE_TAG,
    XML_TOK_STYLE_ATTR_TRANSL_LANGUAGE,
    XML_TOK_STYLE_ATTR_TRANSL_SCRIPT,
    XML_TOK_STYLE_ATTR_TRANSL_COUNTRY,
    XML_TOK_STYLE_ATTR_TRANSL_STYLE
};

enum SvXMLStyleElemAttrTokens
{
    XML_TOK_ELEM_ATTR_DECIMAL_PLACES,
    XML_TOK_ELEM_ATTR_MIN_DECIMAL_PLACES,
    XML_TOK_ELEM_ATTR_MIN_INTEGER_DIGITS,
    XML_TOK_ELEM_ATTR_GROUPING,
    XML_TOK_ELEM_ATTR_DISPLAY_FACTOR,
    XML_TOK_ELEM_ATTR_DECIMAL_REPLACEMENT,
    XML_TOK_ELEM_ATTR_DENOMINATOR_VALUE,
    XML_TOK_ELEM_ATTR_MIN_EXPONENT_DIGITS,
    XML_TOK_ELEM_ATTR_EXPONENT_INTERVAL,
    XML_TOK_ELEM_ATTR_FORCED_EXPONENT_SIGN,
    XML_TOK_ELEM_ATTR_MIN_NUMERATOR_DIGITS,
    XML_TOK_ELEM_ATTR_MIN_DENOMINATOR_DIGITS,
    XML_TOK_ELEM_ATTR_MAX_NUMERATOR_DIGITS,
    XML_TOK_ELEM_ATTR_MAX_DENOMINATOR_VALUE,
    XML_TOK_ELEM_ATTR_ZEROS_NUMERATOR_DIGITS,
    XML_TOK_ELEM_ATTR_ZEROS_DENOMINATOR_DIGITS,
    XML_TOK_ELEM_ATTR_RFC_LANGUAGE_TAG,
    XML_TOK_ELEM_ATTR_LANGUAGE,
    XML_TOK_ELEM_ATTR_SCRIPT,
    XML_TOK_ELEM_ATTR_COUNTRY,
    XML_TOK_ELEM_ATTR_STYLE,
    XML_TOK_ELEM_ATTR_TEXTUAL,
    XML_TOK_ELEM_ATTR_CALENDAR
};


//  standard colors


#define XML_NUMF_COLORCOUNT     10

static const ColorData aNumFmtStdColors[XML_NUMF_COLORCOUNT] =
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


//  token maps


// maps for SvXMLUnitConverter::convertEnum

static const SvXMLEnumMapEntry aStyleValueMap[] =
{
    { XML_SHORT,            sal_uInt16(false) },
    { XML_LONG,             sal_uInt16(true) },
    { XML_TOKEN_INVALID,    0 }
};

static const SvXMLEnumMapEntry aFormatSourceMap[] =
{
    { XML_FIXED,            sal_uInt16(false) },
    { XML_LANGUAGE,         sal_uInt16(true) },
    { XML_TOKEN_INVALID,    0 }
};

struct SvXMLDefaultDateFormat
{
    NfIndexTableOffset          eFormat;
    SvXMLDateElementAttributes  eDOW;
    SvXMLDateElementAttributes  eDay;
    SvXMLDateElementAttributes  eMonth;
    SvXMLDateElementAttributes  eYear;
    SvXMLDateElementAttributes  eHours;
    SvXMLDateElementAttributes  eMins;
    SvXMLDateElementAttributes  eSecs;
    bool                        bSystem;
};

static const SvXMLDefaultDateFormat aDefaultDateFormats[] =
{
    // format                           day-of-week     day             month               year            hours           minutes         seconds         format-source

    { NF_DATE_SYSTEM_SHORT,             XML_DEA_NONE,   XML_DEA_ANY,    XML_DEA_ANY,        XML_DEA_ANY,    XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   true },
    { NF_DATE_SYSTEM_LONG,              XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_ANY,        XML_DEA_ANY,    XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   true },
    { NF_DATE_SYS_MMYY,                 XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_LONG,       XML_DEA_SHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_DDMMM,                XML_DEA_NONE,   XML_DEA_LONG,   XML_DEA_TEXTSHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_DDMMYYYY,             XML_DEA_NONE,   XML_DEA_LONG,   XML_DEA_LONG,       XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_DDMMYY,               XML_DEA_NONE,   XML_DEA_LONG,   XML_DEA_LONG,       XML_DEA_SHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_DMMMYY,               XML_DEA_NONE,   XML_DEA_SHORT,  XML_DEA_TEXTSHORT,  XML_DEA_SHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_DMMMYYYY,             XML_DEA_NONE,   XML_DEA_SHORT,  XML_DEA_TEXTSHORT,  XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_DMMMMYYYY,            XML_DEA_NONE,   XML_DEA_SHORT,  XML_DEA_TEXTLONG,   XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_NNDMMMYY,             XML_DEA_SHORT,  XML_DEA_SHORT,  XML_DEA_TEXTSHORT,  XML_DEA_SHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_NNDMMMMYYYY,          XML_DEA_SHORT,  XML_DEA_SHORT,  XML_DEA_TEXTLONG,   XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_NNNNDMMMMYYYY,        XML_DEA_LONG,   XML_DEA_SHORT,  XML_DEA_TEXTLONG,   XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATETIME_SYSTEM_SHORT_HHMM,    XML_DEA_NONE,   XML_DEA_ANY,    XML_DEA_ANY,        XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_NONE,   true },
    { NF_DATETIME_SYS_DDMMYYYY_HHMMSS,  XML_DEA_NONE,   XML_DEA_ANY,    XML_DEA_ANY,        XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_ANY,    false }
};


//  SvXMLNumImpData


SvXMLNumImpData::SvXMLNumImpData(
    SvNumberFormatter* pFmt,
    const uno::Reference<uno::XComponentContext>& rxContext )
:   pFormatter(pFmt),
    pStylesElemTokenMap(nullptr),
    pStyleElemTokenMap(nullptr),
    pStyleAttrTokenMap(nullptr),
    pStyleElemAttrTokenMap(nullptr),
    pLocaleData(nullptr),
    m_xContext(rxContext)
{
    SAL_WARN_IF( !rxContext.is(), "xmloff", "got no service manager" );
}

SvXMLNumImpData::~SvXMLNumImpData()
{
    delete pStylesElemTokenMap;
    delete pStyleElemTokenMap;
    delete pStyleAttrTokenMap;
    delete pStyleElemAttrTokenMap;
    delete pLocaleData;
}

sal_uInt32 SvXMLNumImpData::GetKeyForName( const OUString& rName )
{
    sal_uInt16 nCount = m_NameEntries.size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        const SvXMLNumFmtEntry *const pObj = &m_NameEntries[i];
        if ( pObj->aName == rName )
            return pObj->nKey;              // found
    }
    return NUMBERFORMAT_ENTRY_NOT_FOUND;
}

void SvXMLNumImpData::AddKey( sal_uInt32 nKey, const OUString& rName, bool bRemoveAfterUse )
{
    if ( bRemoveAfterUse )
    {
        //  if there is already an entry for this key without the bRemoveAfterUse flag,
        //  clear the flag for this entry, too

        sal_uInt16 nCount = m_NameEntries.size();
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            SvXMLNumFmtEntry *const pObj = &m_NameEntries[i];
            if ( pObj->nKey == nKey && !pObj->bRemoveAfterUse )
            {
                bRemoveAfterUse = false;        // clear flag for new entry
                break;
            }
        }
    }
    else
    {
        //  call SetUsed to clear the bRemoveAfterUse flag for other entries for this key
        SetUsed( nKey );
    }

    m_NameEntries.push_back(SvXMLNumFmtEntry(rName, nKey, bRemoveAfterUse));
}

void SvXMLNumImpData::SetUsed( sal_uInt32 nKey )
{
    sal_uInt16 nCount = m_NameEntries.size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        SvXMLNumFmtEntry *const pObj = &m_NameEntries[i];
        if ( pObj->nKey == nKey )
        {
            pObj->bRemoveAfterUse = false;      // used -> don't remove

            //  continue searching - there may be several entries for the same key
            //  (with different names), the format must not be deleted if any one of
            //  them is used
        }
    }
}

void SvXMLNumImpData::RemoveVolatileFormats()
{
    //  remove temporary (volatile) formats from NumberFormatter
    //  called at the end of each import (styles and content), so volatile formats
    //  from styles can't be used in content

    if ( !pFormatter )
        return;

    sal_uInt16 nCount = m_NameEntries.size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        const SvXMLNumFmtEntry *const pObj = &m_NameEntries[i];
        if ( pObj->bRemoveAfterUse )
        {
            const SvNumberformat* pFormat = pFormatter->GetEntry(pObj->nKey);
            if (pFormat && (pFormat->GetType() & css::util::NumberFormat::DEFINED))
                pFormatter->DeleteEntry( pObj->nKey );
        }
    }
}

const SvXMLTokenMap& SvXMLNumImpData::GetStylesElemTokenMap()
{
    if( !pStylesElemTokenMap )
    {
        static const SvXMLTokenMapEntry aStylesElemMap[] =
        {
            //  style elements
            { XML_NAMESPACE_NUMBER, XML_NUMBER_STYLE,      XML_TOK_STYLES_NUMBER_STYLE      },
            { XML_NAMESPACE_NUMBER, XML_CURRENCY_STYLE,    XML_TOK_STYLES_CURRENCY_STYLE    },
            { XML_NAMESPACE_NUMBER, XML_PERCENTAGE_STYLE,  XML_TOK_STYLES_PERCENTAGE_STYLE  },
            { XML_NAMESPACE_NUMBER, XML_DATE_STYLE,        XML_TOK_STYLES_DATE_STYLE        },
            { XML_NAMESPACE_NUMBER, XML_TIME_STYLE,        XML_TOK_STYLES_TIME_STYLE        },
            { XML_NAMESPACE_NUMBER, XML_BOOLEAN_STYLE,     XML_TOK_STYLES_BOOLEAN_STYLE     },
            { XML_NAMESPACE_NUMBER, XML_TEXT_STYLE,        XML_TOK_STYLES_TEXT_STYLE        },
            XML_TOKEN_MAP_END
        };

        pStylesElemTokenMap = new SvXMLTokenMap( aStylesElemMap );
    }
    return *pStylesElemTokenMap;
}

const SvXMLTokenMap& SvXMLNumImpData::GetStyleElemTokenMap()
{
    if( !pStyleElemTokenMap )
    {
        static const SvXMLTokenMapEntry aStyleElemMap[] =
        {
            //  elements in a style
            { XML_NAMESPACE_LO_EXT, XML_TEXT,               XML_TOK_STYLE_TEXT              },
            { XML_NAMESPACE_NUMBER, XML_TEXT,               XML_TOK_STYLE_TEXT              },
            { XML_NAMESPACE_LO_EXT, XML_FILL_CHARACTER,     XML_TOK_STYLE_FILL_CHARACTER    },
            { XML_NAMESPACE_NUMBER, XML_FILL_CHARACTER,     XML_TOK_STYLE_FILL_CHARACTER    },
            { XML_NAMESPACE_NUMBER, XML_NUMBER,             XML_TOK_STYLE_NUMBER            },
            { XML_NAMESPACE_NUMBER, XML_SCIENTIFIC_NUMBER,  XML_TOK_STYLE_SCIENTIFIC_NUMBER },
            { XML_NAMESPACE_NUMBER, XML_FRACTION,           XML_TOK_STYLE_FRACTION          },
            { XML_NAMESPACE_NUMBER, XML_CURRENCY_SYMBOL,    XML_TOK_STYLE_CURRENCY_SYMBOL   },
            { XML_NAMESPACE_NUMBER, XML_DAY,                XML_TOK_STYLE_DAY               },
            { XML_NAMESPACE_NUMBER, XML_MONTH,              XML_TOK_STYLE_MONTH             },
            { XML_NAMESPACE_NUMBER, XML_YEAR,               XML_TOK_STYLE_YEAR              },
            { XML_NAMESPACE_NUMBER, XML_ERA,                XML_TOK_STYLE_ERA               },
            { XML_NAMESPACE_NUMBER, XML_DAY_OF_WEEK,        XML_TOK_STYLE_DAY_OF_WEEK       },
            { XML_NAMESPACE_NUMBER, XML_WEEK_OF_YEAR,       XML_TOK_STYLE_WEEK_OF_YEAR      },
            { XML_NAMESPACE_NUMBER, XML_QUARTER,            XML_TOK_STYLE_QUARTER           },
            { XML_NAMESPACE_NUMBER, XML_HOURS,              XML_TOK_STYLE_HOURS             },
            { XML_NAMESPACE_NUMBER, XML_AM_PM,              XML_TOK_STYLE_AM_PM             },
            { XML_NAMESPACE_NUMBER, XML_MINUTES,            XML_TOK_STYLE_MINUTES           },
            { XML_NAMESPACE_NUMBER, XML_SECONDS,            XML_TOK_STYLE_SECONDS           },
            { XML_NAMESPACE_NUMBER, XML_BOOLEAN,            XML_TOK_STYLE_BOOLEAN           },
            { XML_NAMESPACE_NUMBER, XML_TEXT_CONTENT,       XML_TOK_STYLE_TEXT_CONTENT      },
            { XML_NAMESPACE_STYLE,  XML_TEXT_PROPERTIES,    XML_TOK_STYLE_PROPERTIES        },
            { XML_NAMESPACE_STYLE,  XML_MAP,                XML_TOK_STYLE_MAP               },
            XML_TOKEN_MAP_END
        };

        pStyleElemTokenMap = new SvXMLTokenMap( aStyleElemMap );
    }
    return *pStyleElemTokenMap;
}

const SvXMLTokenMap& SvXMLNumImpData::GetStyleAttrTokenMap()
{
    if( !pStyleAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aStyleAttrMap[] =
        {
            //  attributes for a style
            { XML_NAMESPACE_STYLE,  XML_NAME,                  XML_TOK_STYLE_ATTR_NAME                  },
            { XML_NAMESPACE_NUMBER, XML_RFC_LANGUAGE_TAG,      XML_TOK_STYLE_ATTR_RFC_LANGUAGE_TAG,     },
            { XML_NAMESPACE_NUMBER, XML_LANGUAGE,              XML_TOK_STYLE_ATTR_LANGUAGE              },
            { XML_NAMESPACE_NUMBER, XML_SCRIPT,                XML_TOK_STYLE_ATTR_SCRIPT                },
            { XML_NAMESPACE_NUMBER, XML_COUNTRY,               XML_TOK_STYLE_ATTR_COUNTRY               },
            { XML_NAMESPACE_NUMBER, XML_TITLE,                 XML_TOK_STYLE_ATTR_TITLE                 },
            { XML_NAMESPACE_NUMBER, XML_AUTOMATIC_ORDER,       XML_TOK_STYLE_ATTR_AUTOMATIC_ORDER       },
            { XML_NAMESPACE_NUMBER, XML_FORMAT_SOURCE,         XML_TOK_STYLE_ATTR_FORMAT_SOURCE         },
            { XML_NAMESPACE_NUMBER, XML_TRUNCATE_ON_OVERFLOW,  XML_TOK_STYLE_ATTR_TRUNCATE_ON_OVERFLOW  },
            { XML_NAMESPACE_STYLE,  XML_VOLATILE,              XML_TOK_STYLE_ATTR_VOLATILE              },
            { XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_FORMAT,     XML_TOK_STYLE_ATTR_TRANSL_FORMAT    },
            // not defined in ODF { XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_RFC_LANGUAGE_TAG, XML_TOK_STYLE_ATTR_TRANSL_RFC_LANGUAGE_TAG   },
            { XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_LANGUAGE,   XML_TOK_STYLE_ATTR_TRANSL_LANGUAGE  },
            // not defined in ODF { XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_SCRIPT,     XML_TOK_STYLE_ATTR_TRANSL_SCRIPT    },
            { XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_COUNTRY,    XML_TOK_STYLE_ATTR_TRANSL_COUNTRY   },
            { XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_STYLE,      XML_TOK_STYLE_ATTR_TRANSL_STYLE     },
            XML_TOKEN_MAP_END
        };

        pStyleAttrTokenMap = new SvXMLTokenMap( aStyleAttrMap );
    }
    return *pStyleAttrTokenMap;
}

const SvXMLTokenMap& SvXMLNumImpData::GetStyleElemAttrTokenMap()
{
    if( !pStyleElemAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aStyleElemAttrMap[] =
        {
            //  attributes for an element within a style
            { XML_NAMESPACE_NUMBER, XML_DECIMAL_PLACES,          XML_TOK_ELEM_ATTR_DECIMAL_PLACES       },
            { XML_NAMESPACE_LO_EXT, XML_MIN_DECIMAL_PLACES,      XML_TOK_ELEM_ATTR_MIN_DECIMAL_PLACES   },
            { XML_NAMESPACE_NUMBER, XML_MIN_DECIMAL_PLACES,      XML_TOK_ELEM_ATTR_MIN_DECIMAL_PLACES   },
            { XML_NAMESPACE_NUMBER, XML_MIN_INTEGER_DIGITS,      XML_TOK_ELEM_ATTR_MIN_INTEGER_DIGITS   },
            { XML_NAMESPACE_NUMBER, XML_GROUPING,                XML_TOK_ELEM_ATTR_GROUPING             },
            { XML_NAMESPACE_NUMBER, XML_DISPLAY_FACTOR,          XML_TOK_ELEM_ATTR_DISPLAY_FACTOR       },
            { XML_NAMESPACE_NUMBER, XML_DECIMAL_REPLACEMENT,     XML_TOK_ELEM_ATTR_DECIMAL_REPLACEMENT  },
            { XML_NAMESPACE_NUMBER, XML_DENOMINATOR_VALUE,       XML_TOK_ELEM_ATTR_DENOMINATOR_VALUE  },
            { XML_NAMESPACE_NUMBER, XML_MIN_EXPONENT_DIGITS,     XML_TOK_ELEM_ATTR_MIN_EXPONENT_DIGITS  },
            { XML_NAMESPACE_LO_EXT, XML_EXPONENT_INTERVAL,       XML_TOK_ELEM_ATTR_EXPONENT_INTERVAL    },
            { XML_NAMESPACE_NUMBER, XML_EXPONENT_INTERVAL,       XML_TOK_ELEM_ATTR_EXPONENT_INTERVAL    },
            { XML_NAMESPACE_LO_EXT, XML_FORCED_EXPONENT_SIGN,    XML_TOK_ELEM_ATTR_FORCED_EXPONENT_SIGN },
            { XML_NAMESPACE_NUMBER, XML_FORCED_EXPONENT_SIGN,    XML_TOK_ELEM_ATTR_FORCED_EXPONENT_SIGN },
            { XML_NAMESPACE_NUMBER, XML_MIN_NUMERATOR_DIGITS,    XML_TOK_ELEM_ATTR_MIN_NUMERATOR_DIGITS },
            { XML_NAMESPACE_NUMBER, XML_MIN_DENOMINATOR_DIGITS,  XML_TOK_ELEM_ATTR_MIN_DENOMINATOR_DIGITS },
            { XML_NAMESPACE_LO_EXT, XML_MAX_NUMERATOR_DIGITS,    XML_TOK_ELEM_ATTR_MAX_NUMERATOR_DIGITS },
            { XML_NAMESPACE_LO_EXT, XML_MAX_DENOMINATOR_VALUE,   XML_TOK_ELEM_ATTR_MAX_DENOMINATOR_VALUE },
            { XML_NAMESPACE_NUMBER, XML_MAX_DENOMINATOR_VALUE,   XML_TOK_ELEM_ATTR_MAX_DENOMINATOR_VALUE },
            { XML_NAMESPACE_LO_EXT, XML_ZEROS_NUMERATOR_DIGITS,  XML_TOK_ELEM_ATTR_ZEROS_NUMERATOR_DIGITS },
            { XML_NAMESPACE_NUMBER, XML_ZEROS_NUMERATOR_DIGITS,  XML_TOK_ELEM_ATTR_ZEROS_NUMERATOR_DIGITS },
            { XML_NAMESPACE_LO_EXT, XML_ZEROS_DENOMINATOR_DIGITS,XML_TOK_ELEM_ATTR_ZEROS_DENOMINATOR_DIGITS },
            { XML_NAMESPACE_NUMBER, XML_ZEROS_DENOMINATOR_DIGITS,XML_TOK_ELEM_ATTR_ZEROS_DENOMINATOR_DIGITS },
            { XML_NAMESPACE_NUMBER, XML_RFC_LANGUAGE_TAG,        XML_TOK_ELEM_ATTR_RFC_LANGUAGE_TAG     },
            { XML_NAMESPACE_NUMBER, XML_LANGUAGE,                XML_TOK_ELEM_ATTR_LANGUAGE             },
            { XML_NAMESPACE_NUMBER, XML_SCRIPT,                  XML_TOK_ELEM_ATTR_SCRIPT               },
            { XML_NAMESPACE_NUMBER, XML_COUNTRY,                 XML_TOK_ELEM_ATTR_COUNTRY              },
            { XML_NAMESPACE_NUMBER, XML_STYLE,                   XML_TOK_ELEM_ATTR_STYLE                },
            { XML_NAMESPACE_NUMBER, XML_TEXTUAL,                 XML_TOK_ELEM_ATTR_TEXTUAL              },
            { XML_NAMESPACE_NUMBER, XML_CALENDAR,                XML_TOK_ELEM_ATTR_CALENDAR             },
            XML_TOKEN_MAP_END
        };

        pStyleElemAttrTokenMap = new SvXMLTokenMap( aStyleElemAttrMap );
    }
    return *pStyleElemAttrTokenMap;
}

const LocaleDataWrapper& SvXMLNumImpData::GetLocaleData( LanguageType nLang )
{
    if ( !pLocaleData )
        pLocaleData = new LocaleDataWrapper(
               pFormatter ? pFormatter->GetComponentContext() : m_xContext,
            LanguageTag( nLang ) );
    else
        pLocaleData->setLanguageTag( LanguageTag( nLang ) );
    return *pLocaleData;
}


//  SvXMLNumFmtMapContext


SvXMLNumFmtMapContext::SvXMLNumFmtMapContext( SvXMLImport& rImport,
                                    sal_uInt16 nPrfx, const OUString& rLName,
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
            if ( IsXMLToken( aLocalName, XML_CONDITION) )
                sCondition = sValue;
            else if ( IsXMLToken( aLocalName, XML_APPLY_STYLE_NAME) )
                sName = sValue;
        }
    }
}

SvXMLNumFmtMapContext::~SvXMLNumFmtMapContext()
{
}

SvXMLImportContext* SvXMLNumFmtMapContext::CreateChildContext(
                                    sal_uInt16 nPrfx, const OUString& rLName,
                                    const uno::Reference<xml::sax::XAttributeList>& )
{
    // no elements supported - use default context
    return new SvXMLImportContext( GetImport(), nPrfx, rLName );
}

void SvXMLNumFmtMapContext::Characters( const OUString& )
{
}

void SvXMLNumFmtMapContext::EndElement()
{
    rParent.AddCondition( sCondition, sName );
}


//  SvXMLNumFmtPropContext


SvXMLNumFmtPropContext::SvXMLNumFmtPropContext( SvXMLImport& rImport,
                                    sal_uInt16 nPrfx, const OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rParent( rParentContext ),
    m_nColor( 0 ),
    bColSet( false )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString sValue = xAttrList->getValueByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        if ( nPrefix == XML_NAMESPACE_FO && IsXMLToken( aLocalName, XML_COLOR ) )
        {
            bColSet = ::sax::Converter::convertColor( m_nColor, sValue );
        }
    }
}

SvXMLNumFmtPropContext::~SvXMLNumFmtPropContext()
{
}

SvXMLImportContext* SvXMLNumFmtPropContext::CreateChildContext(
                                    sal_uInt16 nPrfx, const OUString& rLName,
                                    const uno::Reference<xml::sax::XAttributeList>& )
{
    // no elements supported - use default context
    return new SvXMLImportContext( GetImport(), nPrfx, rLName );
}

void SvXMLNumFmtPropContext::Characters( const OUString& )
{
}

void SvXMLNumFmtPropContext::EndElement()
{
    if (bColSet)
        rParent.AddColor( m_nColor );
}


//  SvXMLNumFmtEmbeddedTextContext


SvXMLNumFmtEmbeddedTextContext::SvXMLNumFmtEmbeddedTextContext( SvXMLImport& rImport,
                                    sal_uInt16 nPrfx, const OUString& rLName,
                                    SvXMLNumFmtElementContext& rParentContext,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rParent( rParentContext ),
    nTextPosition( 0 )
{
    sal_Int32 nAttrVal;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString sValue = xAttrList->getValueByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        if ( nPrefix == XML_NAMESPACE_NUMBER && IsXMLToken( aLocalName, XML_POSITION ) )
        {
            if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                nTextPosition = nAttrVal;
        }
    }
}

SvXMLNumFmtEmbeddedTextContext::~SvXMLNumFmtEmbeddedTextContext()
{
}

SvXMLImportContext* SvXMLNumFmtEmbeddedTextContext::CreateChildContext(
                                    sal_uInt16 nPrfx, const OUString& rLName,
                                    const uno::Reference<xml::sax::XAttributeList>& )
{
    // no elements supported - use default context
    return new SvXMLImportContext( GetImport(), nPrfx, rLName );
}

void SvXMLNumFmtEmbeddedTextContext::Characters( const OUString& rChars )
{
    aContent.append( rChars );
}

void SvXMLNumFmtEmbeddedTextContext::EndElement()
{
    rParent.AddEmbeddedElement( nTextPosition, aContent.makeStringAndClear() );
}

static bool lcl_ValidChar( sal_Unicode cChar, const SvXMLNumFormatContext& rParent )
{
    sal_uInt16 nFormatType = rParent.GetType();

    // Treat space equal to non-breaking space separator.
    const sal_Unicode cNBSP = 0x00A0;
    sal_Unicode cTS;
    if ( ( nFormatType == XML_TOK_STYLES_NUMBER_STYLE ||
           nFormatType == XML_TOK_STYLES_CURRENCY_STYLE ||
           nFormatType == XML_TOK_STYLES_PERCENTAGE_STYLE ) &&
            (cChar == (cTS = rParent.GetLocaleData().getNumThousandSep()[0]) ||
             (cChar == ' ' && cTS == cNBSP)) )
    {
        //  #i22394# Extra occurrences of thousands separator must be quoted, so they
        //  aren't mis-interpreted as display-factor.
        //  This must be limited to the format types that can contain a number element,
        //  because the same character can be a date separator that should not be quoted
        //  in date formats.

        return false;   // force quotes
    }

    //  see ImpSvNumberformatScan::Next_Symbol
    if ( cChar == '-' )
        return true;   // all format types may content minus sign or delimiter
    if ( ( cChar == ' ' ||
           cChar == '/' ||
           cChar == '.' ||
           cChar == ',' ||
           cChar == ':' ||
           cChar == '\''   ) &&
         ( nFormatType == XML_TOK_STYLES_CURRENCY_STYLE ||
           nFormatType == XML_TOK_STYLES_DATE_STYLE ||
           nFormatType == XML_TOK_STYLES_TIME_STYLE ) ) // other formats do not require delimiter tdf#97837
        return true;

    //  percent sign must be used without quotes for percentage styles only
    if ( nFormatType == XML_TOK_STYLES_PERCENTAGE_STYLE && cChar == '%' )
        return true;

    //  don't put quotes around single parentheses (often used for negative numbers)
    if ( ( nFormatType == XML_TOK_STYLES_NUMBER_STYLE ||
           nFormatType == XML_TOK_STYLES_CURRENCY_STYLE ||
           nFormatType == XML_TOK_STYLES_PERCENTAGE_STYLE ) &&
         ( cChar == '(' || cChar == ')' ) )
        return true;

    return false;
}

static void lcl_EnquoteIfNecessary( OUStringBuffer& rContent, const SvXMLNumFormatContext& rParent )
{
    bool bQuote = true;
    sal_Int32 nLength = rContent.getLength();

    if ( ( nLength == 1 &&
            lcl_ValidChar( rContent[0], rParent ) ) ||
         ( nLength == 2 &&
             lcl_ValidChar( rContent[0], rParent ) &&
             rContent[1] == ' ' ) )
    {
        //  don't quote single separator characters like space or percent,
        //  or separator characters followed by space (used in date formats)
        bQuote = false;
    }
    else if ( rParent.GetType() == XML_TOK_STYLES_PERCENTAGE_STYLE && nLength > 1 )
    {
        //  the percent character in percentage styles must be left out of quoting
        //  (one occurrence is enough even if there are several percent characters in the string)

        OUString aString( rContent.getStr() );
        sal_Int32 nPos = aString.indexOf( '%' );
        if ( nPos >= 0 )
        {
            if ( nPos + 1 < nLength )
            {
                if ( nPos + 2 == nLength && lcl_ValidChar( rContent[nPos + 1], rParent ) )
                {
                    //  single character that doesn't need quoting
                }
                else
                {
                    //  quote text behind percent character
                    rContent.insert( nPos + 1, '"' );
                    rContent.append( '"' );
                }
            }
            if ( nPos > 0 )
            {
                if ( nPos == 1 && lcl_ValidChar( rContent[0], rParent ) )
                {
                    //  single character that doesn't need quoting
                }
                else
                {
                    //  quote text before percent character
                    rContent.insert( nPos, '"' );
                    rContent.insert( 0, '"' );
                }
            }
            bQuote = false;
        }
        // else: normal quoting (below)
    }

    if ( bQuote )
    {
        // #i55469# quotes in the string itself have to be escaped
        bool bEscape = ( rContent.indexOf( '"' ) >= 0 );
        if ( bEscape )
        {
            // A quote is turned into "\"" - a quote to end quoted text, an escaped quote,
            // and a quote to resume quoting.
            OUString aInsert(  "\"\\\""  );

            sal_Int32 nPos = 0;
            while ( nPos < rContent.getLength() )
            {
                if ( rContent[nPos] == '"' )
                {
                    rContent.insert( nPos, aInsert );
                    nPos += aInsert.getLength();
                }
                ++nPos;
            }
        }

        //  quote string literals
        rContent.insert( 0, '"' );
        rContent.append( '"' );

        // remove redundant double quotes at start or end
        if ( bEscape )
        {
            if ( rContent.getLength() > 2 &&
                 rContent[0] == '"' &&
                 rContent[1] == '"' )
            {
                rContent.remove(0, 2);
            }

            sal_Int32 nLen = rContent.getLength();
            if ( nLen > 2 &&
                 rContent[nLen - 1] == '"' &&
                 rContent[nLen - 2] == '"' )
            {
                rContent.truncate(nLen - 2);
            }
        }
    }
}


//  SvXMLNumFmtElementContext


const sal_Int32 MAX_SECOND_DIGITS = 20; // fdo#58539 & gnome#627420: limit number of digits during import

SvXMLNumFmtElementContext::SvXMLNumFmtElementContext( SvXMLImport& rImport,
                                    sal_uInt16 nPrfx, const OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext, sal_uInt16 nNewType,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rParent( rParentContext ),
    nType( nNewType ),
    nElementLang( LANGUAGE_SYSTEM ),
    bLong( false ),
    bTextual( false )
{
    LanguageTagODF aLanguageTagODF;
    sal_Int32 nAttrVal;
    bool bAttrBool(false);
    bool bVarDecimals = false;
    bool bIsMaxDenominator = false;
    sal_uInt16 nAttrEnum;
    double fAttrDouble;

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
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nDecimals = std::min<sal_Int32>(nAttrVal, MAX_SECOND_DIGITS);
                break;
            case XML_TOK_ELEM_ATTR_MIN_DECIMAL_PLACES:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nMinDecimalDigits = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_MIN_INTEGER_DIGITS:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nInteger = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_GROUPING:
                if (::sax::Converter::convertBool( bAttrBool, sValue ))
                    aNumInfo.bGrouping = bAttrBool;
                break;
            case XML_TOK_ELEM_ATTR_DISPLAY_FACTOR:
                if (::sax::Converter::convertDouble( fAttrDouble, sValue ))
                    aNumInfo.fDisplayFactor = fAttrDouble;
                break;
            case XML_TOK_ELEM_ATTR_DECIMAL_REPLACEMENT:
                if ( sValue == " " )
                {
                    aNumInfo.bDecAlign = true; // space replacement for "?"
                    bVarDecimals = true;
                }
                else
                    if ( sValue.isEmpty() )
                        bVarDecimals = true;   // empty replacement string: variable decimals
                    else                                // all other strings
                        aNumInfo.bDecReplace = true;    // decimal replacement with dashes
                break;
            case XML_TOK_ELEM_ATTR_MIN_EXPONENT_DIGITS:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nExpDigits = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_EXPONENT_INTERVAL:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nExpInterval = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_FORCED_EXPONENT_SIGN:
                if (::sax::Converter::convertBool( bAttrBool, sValue ))
                    aNumInfo.bExpSign = bAttrBool;
                break;
            case XML_TOK_ELEM_ATTR_MIN_NUMERATOR_DIGITS:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nMinNumerDigits = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_MIN_DENOMINATOR_DIGITS:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nMinDenomDigits = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_MAX_NUMERATOR_DIGITS:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 1 ))  // at least one '#'
                    aNumInfo.nMaxNumerDigits = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_DENOMINATOR_VALUE:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 1 )) // 0 is not valid
                {
                    aNumInfo.nFracDenominator = nAttrVal;
                    bIsMaxDenominator = false;
                }
                break;
            case XML_TOK_ELEM_ATTR_MAX_DENOMINATOR_VALUE:  // part of ODF 1.3
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 1 ) && aNumInfo.nFracDenominator <= 0)
                {   // if denominator value not yet defined
                    aNumInfo.nFracDenominator = nAttrVal;
                    bIsMaxDenominator = true;
                }
                break;
            case XML_TOK_ELEM_ATTR_ZEROS_NUMERATOR_DIGITS:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nZerosNumerDigits = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_ZEROS_DENOMINATOR_DIGITS:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nZerosDenomDigits = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_RFC_LANGUAGE_TAG:
                aLanguageTagODF.maRfcLanguageTag = sValue;
                break;
            case XML_TOK_ELEM_ATTR_LANGUAGE:
                aLanguageTagODF.maLanguage = sValue;
                break;
            case XML_TOK_ELEM_ATTR_SCRIPT:
                aLanguageTagODF.maScript = sValue;
                break;
            case XML_TOK_ELEM_ATTR_COUNTRY:
                aLanguageTagODF.maCountry = sValue;
                break;
            case XML_TOK_ELEM_ATTR_STYLE:
                if ( SvXMLUnitConverter::convertEnum( nAttrEnum, sValue, aStyleValueMap ) )
                    bLong = (bool) nAttrEnum;
                break;
            case XML_TOK_ELEM_ATTR_TEXTUAL:
                if (::sax::Converter::convertBool( bAttrBool, sValue ))
                    bTextual = bAttrBool;
                break;
            case XML_TOK_ELEM_ATTR_CALENDAR:
                sCalendar = sValue;
                break;
        }
    }
    if ( aNumInfo.nMinDecimalDigits == -1)
    {
        if ( bVarDecimals || aNumInfo.bDecReplace )
            aNumInfo.nMinDecimalDigits = 0;
        else
            aNumInfo.nMinDecimalDigits = aNumInfo.nDecimals;
    }
    if ( aNumInfo.nZerosDenomDigits > 0 )
    {   // nMin = count of '0' and '?'
        if ( aNumInfo.nMinDenomDigits < aNumInfo.nZerosDenomDigits )
            aNumInfo.nMinDenomDigits = aNumInfo.nZerosDenomDigits;
    }
    else
        aNumInfo.nZerosDenomDigits = 0;
    if ( aNumInfo.nMinDenomDigits >= 0 )
        if ( aNumInfo.nMaxDenomDigits < aNumInfo.nMinDenomDigits )
            aNumInfo.nMaxDenomDigits = ( aNumInfo.nMinDenomDigits ? aNumInfo.nMinDenomDigits : 1 );
    if ( aNumInfo.nZerosNumerDigits > 0 )
    {
        if ( aNumInfo.nMinNumerDigits < aNumInfo.nZerosNumerDigits )
            aNumInfo.nMinNumerDigits = aNumInfo.nZerosNumerDigits;
    }
    else
        aNumInfo.nZerosNumerDigits = 0;
    if ( aNumInfo.nMinNumerDigits >= 0 )
        if ( aNumInfo.nMaxNumerDigits < aNumInfo.nMinNumerDigits )
            aNumInfo.nMaxNumerDigits = ( aNumInfo.nMinNumerDigits ? aNumInfo.nMinNumerDigits : 1 );
    if ( bIsMaxDenominator && aNumInfo.nFracDenominator > 0 )
    {
        aNumInfo.nMaxDenomDigits = floor( log10( aNumInfo.nFracDenominator ) ) + 1;
        aNumInfo.nFracDenominator = -1;  // Max denominator value only gives number of digits at denominator
    }
    if ( aNumInfo.nMaxDenomDigits > 0 )
    {
        if ( aNumInfo.nMinDenomDigits < 0 )
            aNumInfo.nMinDenomDigits = 0;
        else if ( aNumInfo.nMinDenomDigits > aNumInfo.nMaxDenomDigits )
            aNumInfo.nMinDenomDigits = aNumInfo.nMaxDenomDigits;
    }

    if ( !aLanguageTagODF.isEmpty() )
    {
        nElementLang = aLanguageTagODF.getLanguageTag().getLanguageType( false);
        if ( nElementLang == LANGUAGE_DONTKNOW )
            nElementLang = LANGUAGE_SYSTEM;         //! error handling for unknown locales?
    }
}

SvXMLNumFmtElementContext::~SvXMLNumFmtElementContext()
{
}

SvXMLImportContext* SvXMLNumFmtElementContext::CreateChildContext(
                                    sal_uInt16 nPrfx, const OUString& rLName,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    //  only number:number supports number:embedded-text child element

    if ( nType == XML_TOK_STYLE_NUMBER &&
         nPrfx == XML_NAMESPACE_NUMBER && IsXMLToken( rLName, XML_EMBEDDED_TEXT ) )
    {
        return new SvXMLNumFmtEmbeddedTextContext( GetImport(), nPrfx, rLName, *this, xAttrList );
    }
    else
        return new SvXMLImportContext( GetImport(), nPrfx, rLName );
}

void SvXMLNumFmtElementContext::Characters( const OUString& rChars )
{
    aContent.append( rChars );
}

void SvXMLNumFmtElementContext::AddEmbeddedElement( sal_Int32 nFormatPos, const OUString& rContent )
{
    if (rContent.isEmpty())
        return;

    auto const iter(aNumInfo.m_EmbeddedElements.find(nFormatPos));
    if (iter == aNumInfo.m_EmbeddedElements.end())
    {
        aNumInfo.m_EmbeddedElements.insert(std::make_pair(nFormatPos, rContent));
    }
    else
    {
        // there's already an element at this position - append text to existing element
        iter->second += rContent;
    }
}

void SvXMLNumFmtElementContext::EndElement()
{
    bool bEffLong = bLong;
    switch (nType)
    {
        case XML_TOK_STYLE_TEXT:
            if ( rParent.HasLongDoW() &&
                 aContent.toString().equals(rParent.GetLocaleData().getLongDateDayOfWeekSep()) )
            {
                //  skip separator constant after long day of week
                //  (NF_KEY_NNNN contains the separator)

                if ( rParent.ReplaceNfKeyword( NF_KEY_NNN, NF_KEY_NNNN ) )
                {
                    aContent = OUStringBuffer();
                }

                rParent.SetHasLongDoW( false );     // only once
            }
            if ( !aContent.isEmpty() )
            {
                lcl_EnquoteIfNecessary( aContent, rParent );
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
            rParent.AddToCode( '@');
            break;
        case XML_TOK_STYLE_FILL_CHARACTER:
            if ( !aContent.isEmpty() )
            {
                rParent.AddToCode( '*' );
                rParent.AddToCode( aContent[0] );
            }
            break;
        case XML_TOK_STYLE_BOOLEAN:
            // ignored - only default boolean format is supported
            break;

        case XML_TOK_STYLE_DAY:
            rParent.UpdateCalendar( sCalendar );
//! I18N doesn't provide SYSTEM or extended date information yet

            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_DD : NF_KEY_D ) );
            break;
        case XML_TOK_STYLE_MONTH:
            rParent.UpdateCalendar( sCalendar );
//! I18N doesn't provide SYSTEM or extended date information yet

            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bTextual
                    ? ( bEffLong ? NF_KEY_MMMM : NF_KEY_MMM )
                    : ( bEffLong ? NF_KEY_MM : NF_KEY_M ) ) );
            break;
        case XML_TOK_STYLE_YEAR:
            rParent.UpdateCalendar( sCalendar );
//! I18N doesn't provide SYSTEM or extended date information yet
            // Y after G (era) is replaced by E
            if ( rParent.HasEra() )
                rParent.AddNfKeyword(
                    sal::static_int_cast< sal_uInt16 >(
                        bEffLong ? NF_KEY_EEC : NF_KEY_EC ) );
            else
                rParent.AddNfKeyword(
                    sal::static_int_cast< sal_uInt16 >(
                        bEffLong ? NF_KEY_YYYY : NF_KEY_YY ) );
            break;
        case XML_TOK_STYLE_ERA:
            rParent.UpdateCalendar( sCalendar );
//! I18N doesn't provide SYSTEM or extended date information yet
            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_GGG : NF_KEY_G ) );
            //  HasEra flag is set
            break;
        case XML_TOK_STYLE_DAY_OF_WEEK:
            rParent.UpdateCalendar( sCalendar );
//! I18N doesn't provide SYSTEM or extended date information yet
            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_NNNN : NF_KEY_NN ) );
            break;
        case XML_TOK_STYLE_WEEK_OF_YEAR:
            rParent.UpdateCalendar( sCalendar );
            rParent.AddNfKeyword( NF_KEY_WW );
            break;
        case XML_TOK_STYLE_QUARTER:
            rParent.UpdateCalendar( sCalendar );
            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_QQ : NF_KEY_Q ) );
            break;
        case XML_TOK_STYLE_HOURS:
            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_HH : NF_KEY_H ) );
            break;
        case XML_TOK_STYLE_AM_PM:
            //! short/long?
            rParent.AddNfKeyword( NF_KEY_AMPM );
            break;
        case XML_TOK_STYLE_MINUTES:
            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_MMI : NF_KEY_MI ) );
            break;
        case XML_TOK_STYLE_SECONDS:
            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_SS : NF_KEY_S ) );
            if ( aNumInfo.nDecimals > 0 )
            {
                //  manually add the decimal places
                rParent.AddToCode(rParent.GetLocaleData().getNumDecimalSep());
                for (sal_Int32 i=0; i<aNumInfo.nDecimals; i++)
                {
                    rParent.AddToCode( '0');
                }
            }
            break;

        case XML_TOK_STYLE_FRACTION:
            {
                if ( aNumInfo.nInteger >= 0 )
                {
                    // add integer part only if min-integer-digits attribute is there
                    aNumInfo.nDecimals = 0;
                    rParent.AddNumber( aNumInfo );      // number without decimals
                    rParent.AddToCode( ' ' );
                }

                //! build string and add at once

                sal_Int32 i;
                for (i=aNumInfo.nMaxNumerDigits; i > 0; i--)
                {
                    if ( i > aNumInfo.nMinNumerDigits )
                        rParent.AddToCode( '#' );
                    else if ( i > aNumInfo.nZerosNumerDigits )
                        rParent.AddToCode( '?' );
                    else
                        rParent.AddToCode( '0' );
                }
                rParent.AddToCode( '/' );
                if ( aNumInfo.nFracDenominator > 0 )
                {
                    rParent.AddToCode(  OUString::number( aNumInfo.nFracDenominator ) );
                }
                else
                {
                    for (i=aNumInfo.nMaxDenomDigits; i > 0 ; i--)
                    {
                        if ( i > aNumInfo.nMinDenomDigits )
                            rParent.AddToCode( '#' );
                        else if ( i > aNumInfo.nZerosDenomDigits )
                            rParent.AddToCode( '?' );
                        else
                            rParent.AddToCode( '0' );
                    }
                }
            }
            break;

        case XML_TOK_STYLE_SCIENTIFIC_NUMBER:
            {
                // exponential interval for engineering notation
                if( !aNumInfo.bGrouping && aNumInfo.nExpInterval > aNumInfo.nInteger )
                {
                    for (sal_Int32 i=aNumInfo.nInteger; i<aNumInfo.nExpInterval; i++)
                    {
                        rParent.AddToCode( '#' );
                    }
                }
                rParent.AddNumber( aNumInfo );      // simple number

                if ( aNumInfo.bExpSign )
                    rParent.AddToCode( OUString("E+") );
                else
                    rParent.AddToCode( OUString("E") );
                for (sal_Int32 i=0; i<aNumInfo.nExpDigits; i++)
                {
                    rParent.AddToCode( '0' );
                }
            }
            break;

        default:
            OSL_FAIL("invalid element ID");
    }
}

sal_uInt16 SvXMLNumFmtDefaults::GetDefaultDateFormat( SvXMLDateElementAttributes eDOW,
                SvXMLDateElementAttributes eDay, SvXMLDateElementAttributes eMonth,
                SvXMLDateElementAttributes eYear, SvXMLDateElementAttributes eHours,
                SvXMLDateElementAttributes eMins, SvXMLDateElementAttributes eSecs,
                bool bSystem )
{
    for (const auto & rEntry : aDefaultDateFormats)
    {
        if ( bSystem == rEntry.bSystem &&
            ( eDOW   == rEntry.eDOW   || ( rEntry.eDOW   == XML_DEA_ANY && eDOW   != XML_DEA_NONE ) ) &&
            ( eDay   == rEntry.eDay   || ( rEntry.eDay   == XML_DEA_ANY && eDay   != XML_DEA_NONE ) ) &&
            ( eMonth == rEntry.eMonth || ( rEntry.eMonth == XML_DEA_ANY && eMonth != XML_DEA_NONE ) ) &&
            ( eYear  == rEntry.eYear  || ( rEntry.eYear  == XML_DEA_ANY && eYear  != XML_DEA_NONE ) ) &&
            ( eHours == rEntry.eHours || ( rEntry.eHours == XML_DEA_ANY && eHours != XML_DEA_NONE ) ) &&
            ( eMins  == rEntry.eMins  || ( rEntry.eMins  == XML_DEA_ANY && eMins  != XML_DEA_NONE ) ) &&
            ( eSecs  == rEntry.eSecs  || ( rEntry.eSecs  == XML_DEA_ANY && eSecs  != XML_DEA_NONE ) ) )
        {
            return sal::static_int_cast< sal_uInt16 >(rEntry.eFormat);
        }
    }

    return NF_INDEX_TABLE_ENTRIES;  // invalid
}


//  SvXMLNumFormatContext


SvXMLNumFormatContext::SvXMLNumFormatContext( SvXMLImport& rImport,
                                    sal_uInt16 nPrfx, const OUString& rLName,
                                    SvXMLNumImpData* pNewData, sal_uInt16 nNewType,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                    SvXMLStylesContext& rStyles ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList ),
    pData( pNewData ),
    pStyles( &rStyles ),
    aMyConditions(),
    nType( nNewType ),
    nKey(-1),
    nFormatLang( LANGUAGE_SYSTEM ),
    bAutoOrder( false ),
    bFromSystem( false ),
    bTruncate( true ),
    bAutoDec( false ),
    bAutoInt( false ),
    bHasExtraText( false ),
    bHasLongDoW( false ),
    bHasEra( false ),
    bHasDateTime( false ),
    bRemoveAfterUse( false ),
    eDateDOW( XML_DEA_NONE ),
    eDateDay( XML_DEA_NONE ),
    eDateMonth( XML_DEA_NONE ),
    eDateYear( XML_DEA_NONE ),
    eDateHours( XML_DEA_NONE ),
    eDateMins( XML_DEA_NONE ),
    eDateSecs( XML_DEA_NONE ),
    bDateNoDefault( false )
{
    LanguageTagODF aLanguageTagODF;
    OUString sNatNumAttrScript, sNatNumAttrRfcLanguageTag;
    css::i18n::NativeNumberXmlAttributes aNatNumAttr;
    bool bAttrBool(false);
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
                break;
            case XML_TOK_STYLE_ATTR_RFC_LANGUAGE_TAG:
                aLanguageTagODF.maRfcLanguageTag = sValue;
                break;
            case XML_TOK_STYLE_ATTR_LANGUAGE:
                aLanguageTagODF.maLanguage = sValue;
                break;
            case XML_TOK_STYLE_ATTR_SCRIPT:
                aLanguageTagODF.maScript = sValue;
                break;
            case XML_TOK_STYLE_ATTR_COUNTRY:
                aLanguageTagODF.maCountry = sValue;
                break;
            case XML_TOK_STYLE_ATTR_TITLE:
                sFormatTitle = sValue;
                break;
            case XML_TOK_STYLE_ATTR_AUTOMATIC_ORDER:
                if (::sax::Converter::convertBool( bAttrBool, sValue ))
                    bAutoOrder = bAttrBool;
                break;
            case XML_TOK_STYLE_ATTR_FORMAT_SOURCE:
                if ( SvXMLUnitConverter::convertEnum( nAttrEnum, sValue, aFormatSourceMap ) )
                    bFromSystem = (bool) nAttrEnum;
                break;
            case XML_TOK_STYLE_ATTR_TRUNCATE_ON_OVERFLOW:
                if (::sax::Converter::convertBool( bAttrBool, sValue ))
                    bTruncate = bAttrBool;
                break;
            case XML_TOK_STYLE_ATTR_VOLATILE:
                //  volatile formats can be removed after importing
                //  if not used in other styles
                if (::sax::Converter::convertBool( bAttrBool, sValue ))
                    bRemoveAfterUse = bAttrBool;
                break;
            case XML_TOK_STYLE_ATTR_TRANSL_FORMAT:
                aNatNumAttr.Format = sValue;
                break;
            case XML_TOK_STYLE_ATTR_TRANSL_RFC_LANGUAGE_TAG:
                sNatNumAttrRfcLanguageTag = sValue;
                break;
            case XML_TOK_STYLE_ATTR_TRANSL_LANGUAGE:
                aNatNumAttr.Locale.Language = sValue;
                break;
            case XML_TOK_STYLE_ATTR_TRANSL_SCRIPT:
                sNatNumAttrScript = sValue;
                break;
            case XML_TOK_STYLE_ATTR_TRANSL_COUNTRY:
                aNatNumAttr.Locale.Country = sValue;
                break;
            case XML_TOK_STYLE_ATTR_TRANSL_STYLE:
                aNatNumAttr.Style = sValue;
                break;
        }
    }

    if (!aLanguageTagODF.isEmpty())
    {
        nFormatLang = aLanguageTagODF.getLanguageTag().getLanguageType( false);
        if ( nFormatLang == LANGUAGE_DONTKNOW )
            nFormatLang = LANGUAGE_SYSTEM;          //! error handling for unknown locales?
    }

    if ( !aNatNumAttr.Format.isEmpty() )
    {
        SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
        if ( pFormatter )
        {
            LanguageTag aLanguageTag( sNatNumAttrRfcLanguageTag, aNatNumAttr.Locale.Language,
                    sNatNumAttrScript, aNatNumAttr.Locale.Country);
            aNatNumAttr.Locale = aLanguageTag.getLocale( false);

            sal_Int32 nNatNum = pFormatter->GetNatNum()->convertFromXmlAttributes( aNatNumAttr );
            aFormatCode.append( "[NatNum" );
            aFormatCode.append( nNatNum );

            LanguageType eLang = aLanguageTag.getLanguageType( false);
            if ( eLang == LANGUAGE_DONTKNOW )
                eLang = LANGUAGE_SYSTEM;            //! error handling for unknown locales?
            if ( eLang != nFormatLang && eLang != LANGUAGE_SYSTEM )
            {
                aFormatCode.append( "][$-" );
                // language code in upper hex:
                aFormatCode.append(OUString::number(eLang, 16).toAsciiUpperCase());
            }
            aFormatCode.append( ']' );
        }
    }
}

SvXMLNumFormatContext::SvXMLNumFormatContext( SvXMLImport& rImport,
                                    sal_uInt16 nPrfx, const OUString& rLName,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                    const sal_Int32 nTempKey,
                                    SvXMLStylesContext& rStyles ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_DATA_STYLE ),
    pData( nullptr ),
    pStyles( &rStyles ),
    aMyConditions(),
    nType( 0 ),
    nKey(nTempKey),
    nFormatLang( LANGUAGE_SYSTEM ),
    bAutoOrder( false ),
    bFromSystem( false ),
    bTruncate( true ),
    bAutoDec( false ),
    bAutoInt( false ),
    bHasExtraText( false ),
    bHasLongDoW( false ),
    bHasEra( false ),
    bHasDateTime( false ),
    bRemoveAfterUse( false ),
    eDateDOW( XML_DEA_NONE ),
    eDateDay( XML_DEA_NONE ),
    eDateMonth( XML_DEA_NONE ),
    eDateYear( XML_DEA_NONE ),
    eDateHours( XML_DEA_NONE ),
    eDateMins( XML_DEA_NONE ),
    eDateSecs( XML_DEA_NONE ),
    bDateNoDefault( false )
{
    SetAttribute(XML_NAMESPACE_STYLE, GetXMLToken(XML_NAME), rLName);
}

SvXMLNumFormatContext::~SvXMLNumFormatContext()
{
}

SvXMLImportContext* SvXMLNumFormatContext::CreateChildContext(
                                    sal_uInt16 nPrfx, const OUString& rLName,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = pData->GetStyleElemTokenMap();
    sal_uInt16 nToken = rTokenMap.Get( nPrfx, rLName );
    switch (nToken)
    {
        case XML_TOK_STYLE_TEXT:
        case XML_TOK_STYLE_FILL_CHARACTER:
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
                //  SvXMLNumFmtMapContext::EndElement adds to aMyConditions,
                //  so there's no need for an extra flag
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
    {
        if (bRemoveAfterUse)
        {
            //  format is used -> don't remove
            bRemoveAfterUse = false;
            if (pData)
                pData->SetUsed(nKey);

            //  Add to import's list of keys now - CreateAndInsert didn't add
            //  the style if bRemoveAfterUse was set.
            GetImport().AddNumberStyle( nKey, GetName() );
        }
        return nKey;
    }
    else
    {
        // reset bRemoveAfterUse before CreateAndInsert, so AddKey is called without bRemoveAfterUse set
        bRemoveAfterUse = false;
        CreateAndInsert(true);
        return nKey;
    }
}

sal_Int32 SvXMLNumFormatContext::PrivateGetKey()
{
    //  used for map elements in CreateAndInsert - don't reset bRemoveAfterUse flag

    if (nKey > -1)
        return nKey;
    else
    {
        CreateAndInsert(true);
        return nKey;
    }
}

sal_Int32 SvXMLNumFormatContext::CreateAndInsert( css::uno::Reference< css::util::XNumberFormatsSupplier >& xFormatsSupplier )
{
    if (nKey <= -1)
    {
        SvNumberFormatter* pFormatter = nullptr;
        SvNumberFormatsSupplierObj* pObj =
                        SvNumberFormatsSupplierObj::getImplementation( xFormatsSupplier );
        if (pObj)
            pFormatter = pObj->GetNumberFormatter();

        if ( pFormatter )
            return CreateAndInsert( pFormatter );
        else
            return -1;
    }
    else
        return nKey;
}

void SvXMLNumFormatContext::CreateAndInsert(bool /*bOverwrite*/)
{
    if (!(nKey > -1))
        CreateAndInsert(pData->GetNumberFormatter());
}

sal_Int32 SvXMLNumFormatContext::CreateAndInsert(SvNumberFormatter* pFormatter)
{
    if (!pFormatter)
    {
        OSL_FAIL("no number formatter");
        return -1;
    }

    sal_uInt32 nIndex = NUMBERFORMAT_ENTRY_NOT_FOUND;

    for (size_t i = 0; i < aMyConditions.size(); i++)
    {
        SvXMLNumFormatContext* pStyle = const_cast<SvXMLNumFormatContext*>( static_cast<const SvXMLNumFormatContext *>(pStyles->FindStyleChildContext(
            XML_STYLE_FAMILY_DATA_STYLE, aMyConditions[i].sMapName)));
        if (pStyle)
        {
            if ((pStyle->PrivateGetKey() > -1))     // don't reset pStyle's bRemoveAfterUse flag
                AddCondition(i);
        }
    }

    if ( aFormatCode.isEmpty() )
    {
        //  insert empty format as empty string (with quotes)
        //  #93901# this check has to be done before inserting the conditions
        aFormatCode.append("\"\"");    // ""
    }

    aFormatCode.insert( 0, aConditions.makeStringAndClear() );
    OUString sFormat = aFormatCode.makeStringAndClear();

    //  test special cases

    if ( bAutoDec )         // automatic decimal places
    {
        //  #99391# adjust only if the format contains no text elements, no conditions
        //  and no color definition (detected by the '[' at the start)

        if ( nType == XML_TOK_STYLES_NUMBER_STYLE && !bHasExtraText &&
                aMyConditions.empty() && sFormat.toChar() != '[' )
            nIndex = pFormatter->GetStandardIndex( nFormatLang );
    }
    if ( bAutoInt )         // automatic integer digits
    {
        //! only if two decimal places was set?

        if ( nType == XML_TOK_STYLES_NUMBER_STYLE && !bHasExtraText &&
                aMyConditions.empty() && sFormat.toChar() != '[' )
            nIndex = pFormatter->GetFormatIndex( NF_NUMBER_SYSTEM, nFormatLang );
    }

    //  boolean is always the builtin boolean format
    //  (no other boolean formats are implemented)
    if ( nType == XML_TOK_STYLES_BOOLEAN_STYLE )
        nIndex = pFormatter->GetFormatIndex( NF_BOOLEAN, nFormatLang );

    //  check for default date formats
    if ( nType == XML_TOK_STYLES_DATE_STYLE && bAutoOrder && !bDateNoDefault )
    {
        NfIndexTableOffset eFormat = (NfIndexTableOffset) SvXMLNumFmtDefaults::GetDefaultDateFormat(
            eDateDOW, eDateDay, eDateMonth, eDateYear,
            eDateHours, eDateMins, eDateSecs, bFromSystem );
        if ( eFormat < NF_INDEX_TABLE_LOCALE_DATA_DEFAULTS )
        {
            //  #109651# if a date format has the automatic-order attribute and
            //  contains exactly the elements of one of the default date formats,
            //  use that default format, with the element order and separators
            //  from the current locale settings

            nIndex = pFormatter->GetFormatIndex( eFormat, nFormatLang );
        }
    }

    if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND && !sFormat.isEmpty() )
    {
        //  insert by format string

        OUString aFormatStr( sFormat );
        nIndex = pFormatter->GetEntryKey( aFormatStr, nFormatLang );
        if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            sal_Int32  nErrPos = 0;
            short       l_nType = 0;
            bool bOk = pFormatter->PutEntry( aFormatStr, nErrPos, l_nType, nIndex, nFormatLang );
            if ( !bOk && nErrPos == 0 && aFormatStr != sFormat )
            {
                //  if the string was modified by PutEntry, look for an existing format
                //  with the modified string
                nIndex = pFormatter->GetEntryKey( aFormatStr, nFormatLang );
                if ( nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND )
                    bOk = true;
            }
            if (!bOk)
                nIndex = NUMBERFORMAT_ENTRY_NOT_FOUND;
        }
    }

//! I18N doesn't provide SYSTEM or extended date information yet
    if ( nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND && !bAutoOrder )
    {
        //  use fixed-order formats instead of SYS... if bAutoOrder is false
        //  (only if the format strings are equal for the locale)

        NfIndexTableOffset eOffset = pFormatter->GetIndexTableOffset( nIndex );
        if ( eOffset == NF_DATE_SYS_DMMMYYYY )
        {
            sal_uInt32 nNewIndex = pFormatter->GetFormatIndex( NF_DATE_DIN_DMMMYYYY, nFormatLang );
            const SvNumberformat* pOldEntry = pFormatter->GetEntry( nIndex );
            const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewIndex );
            if ( pOldEntry && pNewEntry && pOldEntry->GetFormatstring() == pNewEntry->GetFormatstring() )
                nIndex = nNewIndex;
        }
        else if ( eOffset == NF_DATE_SYS_DMMMMYYYY )
        {
            sal_uInt32 nNewIndex = pFormatter->GetFormatIndex( NF_DATE_DIN_DMMMMYYYY, nFormatLang );
            const SvNumberformat* pOldEntry = pFormatter->GetEntry( nIndex );
            const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewIndex );
            if ( pOldEntry && pNewEntry && pOldEntry->GetFormatstring() == pNewEntry->GetFormatstring() )
                nIndex = nNewIndex;
        }
    }

    if ((nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND) && !sFormatTitle.isEmpty())
    {
        SvNumberformat* pFormat = const_cast<SvNumberformat*>(pFormatter->GetEntry( nIndex ));
        if (pFormat)
        {
            pFormat->SetComment(sFormatTitle);
        }
    }

    if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        OSL_FAIL("invalid number format");
        nIndex = pFormatter->GetStandardIndex( nFormatLang );
    }

    pData->AddKey( nIndex, GetName(), bRemoveAfterUse );
    nKey = nIndex;

    //  Add to import's list of keys (shared between styles and content import)
    //  only if not volatile - formats are removed from NumberFormatter at the
    //  end of each import (in SvXMLNumFmtHelper dtor).
    //  If bRemoveAfterUse is reset later in GetKey, AddNumberStyle is called there.

    if (!bRemoveAfterUse)
        GetImport().AddNumberStyle( nKey, GetName() );

    return nKey;
}

void SvXMLNumFormatContext::Finish( bool bOverwrite )
{
    SvXMLStyleContext::Finish( bOverwrite );
}

const LocaleDataWrapper& SvXMLNumFormatContext::GetLocaleData() const
{
    return pData->GetLocaleData( nFormatLang );
}

void SvXMLNumFormatContext::AddToCode( sal_Unicode c )
{
    aFormatCode.append( c );
    bHasExtraText = true;
}

void SvXMLNumFormatContext::AddToCode( const OUString& rString )
{
    aFormatCode.append( rString );
    bHasExtraText = true;
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
        if ( nType == XML_TOK_STYLES_CURRENCY_STYLE )
        {
            //  for currency formats, "automatic decimals" is used for the automatic
            //  currency format with (fixed) decimals from the locale settings

            const LocaleDataWrapper& rLoc = pData->GetLocaleData( nFormatLang );
            nPrec = rLoc.getCurrDigits();
        }
        else
        {
            //  for other types, "automatic decimals" means dynamic determination of
            //  decimals, as achieved with the "general" keyword

            aFormatCode.append( pFormatter->GetStandardName( nFormatLang ) );
            return;
        }
    }
    if ( bAutoInt )
    {
        //!...
    }

    sal_uInt16 nGenPrec = nPrec;
    if ( rInfo.nMinDecimalDigits >= 0 )
        nGenPrec = rInfo.nMinDecimalDigits;
    if ( rInfo.bDecReplace )
        nGenPrec = 0;               // generate format without decimals...

    bool bGrouping = rInfo.bGrouping;
    size_t const nEmbeddedCount = rInfo.m_EmbeddedElements.size();
    if ( nEmbeddedCount )
        bGrouping = false;      // grouping and embedded characters can't be used together

    sal_uInt32 nStdIndex = pFormatter->GetStandardIndex( nFormatLang );
    OUStringBuffer aNumStr = pFormatter->GenerateFormat( nStdIndex, nFormatLang,
                                                         bGrouping, false, nGenPrec, nLeading );

    if ( rInfo.nExpDigits >= 0 && nLeading == 0 && !bGrouping && nEmbeddedCount == 0 )
    {
        // #i43959# For scientific numbers, "#" in the integer part forces a digit,
        // so it has to be removed if nLeading is 0 (".00E+0", not "#.00E+0").

        aNumStr.stripStart('#');
    }

    if ( bGrouping && rInfo.nExpInterval > rInfo.nInteger )
    {
        sal_Int32 nIndex = 0;
        sal_Int32 nDigits = rInfo.nInteger;
        sal_Int32 nIntegerEnd = aNumStr.indexOf( pFormatter->GetNumDecimalSep() );
        if ( nIntegerEnd < 0 )
            nIntegerEnd = aNumStr.getLength();
        while ( nIndex >= 0 && nIndex < nIntegerEnd )
        {
            if ( ( nIndex = aNumStr.indexOf( '#', nIndex ) ) >= 0 )
            {
                nDigits ++;
                nIndex ++;
            }
            else
                nIndex = -1;
        }
        while ( rInfo.nExpInterval > nDigits )
        {
            nDigits++;
            aNumStr.insert( 0, '#' );
        }
    }

    if ( nEmbeddedCount )
    {
        //  insert embedded strings into number string
        //  only the integer part is supported
        //  nZeroPos is the string position where format position 0 is inserted

        sal_Int32 nZeroPos = aNumStr.indexOf( pData->GetLocaleData( nFormatLang ).getNumDecimalSep() );
        if ( nZeroPos < 0 )
        {
            nZeroPos = aNumStr.getLength();
        }

        // m_EmbeddedElements is sorted - last entry has the largest position (leftmost)
        sal_Int32 const nLastFormatPos = rInfo.m_EmbeddedElements.rbegin()->first;
        if ( nLastFormatPos >= nZeroPos )
        {
            //  add '#' characters so all embedded texts are really embedded in digits
            //  (there always has to be a digit before the leftmost embedded text)

            sal_Int32 nAddCount = nLastFormatPos + 1 - nZeroPos;
            for(sal_Int32 index = 0; index < nAddCount; ++index)
            {
                aNumStr.insert(0, '#');
            }
            nZeroPos = nZeroPos + nAddCount;
        }

        // m_EmbeddedElements is sorted with ascending positions - loop is from right to left
        for (auto const& it : rInfo.m_EmbeddedElements)
        {
            sal_Int32 const nFormatPos = it.first;
            sal_Int32 nInsertPos = nZeroPos - nFormatPos;
            if ( nFormatPos >= 0 && nInsertPos >= 0 )
            {
                //  #107805# always quote embedded strings - even space would otherwise
                //  be recognized as thousands separator in French.

                aNumStr.insert(nInsertPos, '"');
                aNumStr.insert(nInsertPos, it.second);
                aNumStr.insert(nInsertPos, '"');
            }
        }
    }

    aFormatCode.append( aNumStr.makeStringAndClear() );

    if ( ( rInfo.bDecReplace || rInfo.nMinDecimalDigits < rInfo.nDecimals ) && nPrec )     // add decimal replacement (dashes)
    {
        //  add dashes for explicit decimal replacement, # or ? for variable decimals
        sal_Unicode cAdd = rInfo.bDecReplace ? '-' : ( rInfo.bDecAlign ? '?': '#' );

        if ( rInfo.nMinDecimalDigits == 0 )
            aFormatCode.append( pData->GetLocaleData( nFormatLang ).getNumDecimalSep() );
        for ( sal_uInt16 i=rInfo.nMinDecimalDigits; i<nPrec; i++)
            aFormatCode.append( cAdd );
    }

    //  add extra thousands separators for display factor

    if ( rInfo.fDisplayFactor != 1.0 && rInfo.fDisplayFactor > 0.0 )
    {
        //  test for 1.0 is just for optimization - nSepCount would be 0

        //  one separator for each factor of 1000
        sal_Int32 nSepCount = (sal_Int32) ::rtl::math::round( log10(rInfo.fDisplayFactor) / 3.0 );
        if ( nSepCount > 0 )
        {
            OUString aSep = pData->GetLocaleData( nFormatLang ).getNumThousandSep();
            for ( sal_Int32 i=0; i<nSepCount; i++ )
                aFormatCode.append( aSep );
        }
    }
}

void SvXMLNumFormatContext::AddCurrency( const OUString& rContent, LanguageType nLang )
{
    bool bAutomatic = false;
    OUString aSymbol = rContent;
    if ( aSymbol.isEmpty())
    {
        SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
        if ( pFormatter )
        {
            pFormatter->ChangeIntl( nFormatLang );
            OUString sCurString, sDummy;
            pFormatter->GetCompatibilityCurrency( sCurString, sDummy );
            aSymbol = sCurString;

            bAutomatic = true;
        }
    }
    else if ( nLang == LANGUAGE_SYSTEM && aSymbol == "CCC" )
    {
        //  "CCC" is used for automatic long symbol
        bAutomatic = true;
    }

    if ( bAutomatic )
    {
        //  remove unnecessary quotes before automatic symbol (formats like "-(0DM)")
        //  otherwise the currency symbol isn't recognized (#94048#)

        sal_Int32 nLength = aFormatCode.getLength();
        if ( nLength > 1 && aFormatCode[nLength - 1] == '"' )
        {
            //  find start of quoted string
            //  When SvXMLNumFmtElementContext::EndElement creates escaped quotes,
            //  they must be handled here, too.

            sal_Int32 nFirst = nLength - 2;
            while ( nFirst >= 0 && aFormatCode[nFirst] != '"' )
                --nFirst;
            if ( nFirst >= 0 )
            {
                //  remove both quotes from aFormatCode
                OUString aOld = aFormatCode.makeStringAndClear();
                if ( nFirst > 0 )
                    aFormatCode.append( aOld.copy( 0, nFirst ) );
                if ( nLength > nFirst + 2 )
                    aFormatCode.append( aOld.copy( nFirst + 1, nLength - nFirst - 2 ) );
            }
        }
    }

    if (!bAutomatic)
        aFormatCode.append( "[$" );            // intro for "new" currency symbols

    aFormatCode.append( aSymbol );

    if (!bAutomatic)
    {
        if ( nLang != LANGUAGE_SYSTEM )
        {
            //  '-' sign and language code in hex:
            aFormatCode.append("-" + OUString::number(sal_Int32(nLang), 16).toAsciiUpperCase());
        }

        aFormatCode.append( ']' );    // end of "new" currency symbol
    }
}

void SvXMLNumFormatContext::AddNfKeyword( sal_uInt16 nIndex )
{
    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    if (!pFormatter)
        return;

    if ( nIndex == NF_KEY_G || nIndex == NF_KEY_GG || nIndex == NF_KEY_GGG )
        bHasEra = true;

    if ( nIndex == NF_KEY_NNNN )
    {
        nIndex = NF_KEY_NNN;
        bHasLongDoW = true;         // to remove string constant with separator
    }

    OUString sKeyword = pFormatter->GetKeyword( nFormatLang, nIndex );

    if ( nIndex == NF_KEY_H  || nIndex == NF_KEY_HH  ||
         nIndex == NF_KEY_MI || nIndex == NF_KEY_MMI ||
         nIndex == NF_KEY_S  || nIndex == NF_KEY_SS )
    {
        if ( !bTruncate && !bHasDateTime )
        {
            //  with truncate-on-overflow = false, add "[]" to first time part
            aFormatCode.append("[" + sKeyword + "]");
        }
        else
        {
            aFormatCode.append( sKeyword );
        }
        bHasDateTime = true;
    }
    else
    {
        aFormatCode.append( sKeyword );
    }
    //  collect the date elements that the format contains, to recognize default date formats
    switch ( nIndex )
    {
        case NF_KEY_NN:     eDateDOW = XML_DEA_SHORT;       break;
        case NF_KEY_NNN:
        case NF_KEY_NNNN:   eDateDOW = XML_DEA_LONG;        break;
        case NF_KEY_D:      eDateDay = XML_DEA_SHORT;       break;
        case NF_KEY_DD:     eDateDay = XML_DEA_LONG;        break;
        case NF_KEY_M:      eDateMonth = XML_DEA_SHORT;     break;
        case NF_KEY_MM:     eDateMonth = XML_DEA_LONG;      break;
        case NF_KEY_MMM:    eDateMonth = XML_DEA_TEXTSHORT; break;
        case NF_KEY_MMMM:   eDateMonth = XML_DEA_TEXTLONG;  break;
        case NF_KEY_YY:     eDateYear = XML_DEA_SHORT;      break;
        case NF_KEY_YYYY:   eDateYear = XML_DEA_LONG;       break;
        case NF_KEY_H:      eDateHours = XML_DEA_SHORT;     break;
        case NF_KEY_HH:     eDateHours = XML_DEA_LONG;      break;
        case NF_KEY_MI:     eDateMins = XML_DEA_SHORT;      break;
        case NF_KEY_MMI:    eDateMins = XML_DEA_LONG;       break;
        case NF_KEY_S:      eDateSecs = XML_DEA_SHORT;      break;
        case NF_KEY_SS:     eDateSecs = XML_DEA_LONG;       break;
        case NF_KEY_AP:
        case NF_KEY_AMPM:   break;          // AM/PM may or may not be in date/time formats -> ignore by itself
        default:
            bDateNoDefault = true;      // any other element -> no default format
    }
}

static bool lcl_IsAtEnd( OUStringBuffer& rBuffer, const OUString& rToken )
{
    sal_Int32 nBufLen = rBuffer.getLength();
    sal_Int32 nTokLen = rToken.getLength();

    if ( nTokLen > nBufLen )
        return false;

    sal_Int32 nStartPos = nBufLen - nTokLen;
    for ( sal_Int32 nTokPos = 0; nTokPos < nTokLen; nTokPos++ )
        if ( rToken[ nTokPos ] != rBuffer[nStartPos + nTokPos] )
            return false;

    return true;
}

bool SvXMLNumFormatContext::ReplaceNfKeyword( sal_uInt16 nOld, sal_uInt16 nNew )
{
    //  replaces one keyword with another if it is found at the end of the code

    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    if (!pFormatter)
        return false;

    OUString sOldStr = pFormatter->GetKeyword( nFormatLang, nOld );
    if ( lcl_IsAtEnd( aFormatCode, sOldStr ) )
    {
        // remove old keyword
        aFormatCode.setLength( aFormatCode.getLength() - sOldStr.getLength() );

        // add new keyword
        OUString sNewStr = pFormatter->GetKeyword( nFormatLang, nNew );
        aFormatCode.append( sNewStr );

        return true;    // changed
    }
    return false;       // not found
}

void SvXMLNumFormatContext::AddCondition( const sal_Int32 nIndex )
{
    OUString rApplyName = aMyConditions[nIndex].sMapName;
    OUString rCondition = aMyConditions[nIndex].sCondition;
    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    sal_uInt32 l_nKey = pData->GetKeyForName( rApplyName );
    OUString sValue("value()");        //! define constant
    sal_Int32 nValLen = sValue.getLength();

    if ( pFormatter && l_nKey != NUMBERFORMAT_ENTRY_NOT_FOUND &&
            rCondition.copy( 0, nValLen ) == sValue )
    {
        //! test for valid conditions
        //! test for default conditions

        OUString sRealCond = rCondition.copy( nValLen, rCondition.getLength() - nValLen );
        bool bDefaultCond = false;

        //! collect all conditions first and adjust default to >=0, >0 or <0 depending on count
        //! allow blanks in conditions
        if ( aConditions.isEmpty() && aMyConditions.size() == 1 && sRealCond == ">=0" )
            bDefaultCond = true;

        if ( nType == XML_TOK_STYLES_TEXT_STYLE && static_cast<size_t>(nIndex) == aMyConditions.size() - 1 )
        {
            //  The last condition in a number format with a text part can only
            //  be "all other numbers", the condition string must be empty.
            bDefaultCond = true;
        }

        if (!bDefaultCond)
        {
            // Convert != to <>
            sal_Int32 nPos = sRealCond.indexOf( "!=" );
            if ( nPos >= 0 )
            {
                sRealCond = sRealCond.replaceAt( nPos, 2, "<>" );
            }

            nPos = sRealCond.indexOf( '.' );
            if ( nPos >= 0 )
            {
                // #i8026# #103991# localize decimal separator
                const OUString& rDecSep = GetLocaleData().getNumDecimalSep();
                if ( rDecSep.getLength() > 1 || rDecSep[0] != '.' )
                {
                    sRealCond = sRealCond.replaceAt( nPos, 1, rDecSep );
                }
            }
            aConditions.append("[" + sRealCond + "]");
        }

        const SvNumberformat* pFormat = pFormatter->GetEntry(l_nKey);
        if ( pFormat )
            aConditions.append( OUString( pFormat->GetFormatstring() ) );

        aConditions.append( ';' );
    }
}

void SvXMLNumFormatContext::AddCondition( const OUString& rCondition, const OUString& rApplyName )
{
    MyCondition aCondition;
    aCondition.sCondition = rCondition;
    aCondition.sMapName = rApplyName;
    aMyConditions.push_back(aCondition);
}

void SvXMLNumFormatContext::AddColor( sal_uInt32 const nColor )
{
    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    if (!pFormatter)
        return;

    OUStringBuffer aColName;
    for ( sal_uInt16 i=0; i<XML_NUMF_COLORCOUNT; i++ )
        if (nColor == aNumFmtStdColors[i])
        {
            aColName = pFormatter->GetKeyword( nFormatLang, sal::static_int_cast< sal_uInt16 >(NF_KEY_FIRSTCOLOR + i) );
            break;
        }

    if ( !aColName.isEmpty() )
    {
        aColName.insert( 0, '[' );
        aColName.append( ']' );
        aFormatCode.insert( 0, aColName.makeStringAndClear() );
    }
}

void SvXMLNumFormatContext::UpdateCalendar( const OUString& rNewCalendar )
{
    if ( rNewCalendar != sCalendar )
    {
        sCalendar = rNewCalendar;
        if ( !sCalendar.isEmpty() )
        {
            aFormatCode.append( "[~" );            // intro for calendar code
            aFormatCode.append( sCalendar );
            aFormatCode.append( ']' );    // end of "new" currency symbolcalendar code
        }
    }
}

bool SvXMLNumFormatContext::IsSystemLanguage()
{
    return nFormatLang == LANGUAGE_SYSTEM;
}


//  SvXMLNumFmtHelper


SvXMLNumFmtHelper::SvXMLNumFmtHelper(
    const uno::Reference<util::XNumberFormatsSupplier>& rSupp,
    const uno::Reference<uno::XComponentContext>& rxContext )
{
    SAL_WARN_IF( !rxContext.is(), "xmloff", "got no service manager" );

    SvNumberFormatter* pFormatter = nullptr;
    SvNumberFormatsSupplierObj* pObj =
                    SvNumberFormatsSupplierObj::getImplementation( rSupp );
    if (pObj)
        pFormatter = pObj->GetNumberFormatter();

    pData = new SvXMLNumImpData( pFormatter, rxContext );
}

SvXMLNumFmtHelper::SvXMLNumFmtHelper(
    SvNumberFormatter* pNumberFormatter,
    const uno::Reference<uno::XComponentContext>& rxContext )
{
    SAL_WARN_IF( !rxContext.is(), "xmloff", "got no service manager" );

    pData = new SvXMLNumImpData( pNumberFormatter, rxContext );
}

SvXMLNumFmtHelper::~SvXMLNumFmtHelper()
{
    //  remove temporary (volatile) formats from NumberFormatter
    pData->RemoveVolatileFormats();

    delete pData;
}

SvXMLStyleContext*  SvXMLNumFmtHelper::CreateChildContext( SvXMLImport& rImport,
                sal_uInt16 nPrefix, const OUString& rLocalName,
                const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                SvXMLStylesContext& rStyles )
{
    SvXMLStyleContext* pContext = nullptr;

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

const SvXMLTokenMap& SvXMLNumFmtHelper::GetStylesElemTokenMap()
{
    return pData->GetStylesElemTokenMap();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
