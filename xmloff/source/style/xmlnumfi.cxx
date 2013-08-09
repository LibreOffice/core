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
#include <rtl/ustrbuf.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmlnumfi.hxx>
#include <xmloff/xmltkmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/languagetagodf.hxx>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_set.hpp>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

struct SvXMLNumFmtEntry
{
    OUString   aName;
    sal_uInt32      nKey;
    sal_Bool        bRemoveAfterUse;

    SvXMLNumFmtEntry( const OUString& rN, sal_uInt32 nK, sal_Bool bR ) :
        aName(rN), nKey(nK), bRemoveAfterUse(bR) {}
};

typedef ::boost::ptr_vector<SvXMLNumFmtEntry> SvXMLNumFmtEntryArr;

struct SvXMLEmbeddedElement
{
    sal_Int32       nFormatPos;
    OUString   aText;

    SvXMLEmbeddedElement( sal_Int32 nFP, const OUString& rT ) :
        nFormatPos(nFP), aText(rT) {}

    //  comparison operators for PTRARR sorting - sorted by position
    sal_Bool operator ==( const SvXMLEmbeddedElement& r ) const { return nFormatPos == r.nFormatPos; }
    sal_Bool operator < ( const SvXMLEmbeddedElement& r ) const { return nFormatPos <  r.nFormatPos; }
};

typedef boost::ptr_set<SvXMLEmbeddedElement> SvXMLEmbeddedElementArr;

class SvXMLNumImpData
{
    SvNumberFormatter*  pFormatter;
    SvXMLTokenMap*      pStylesElemTokenMap;
    SvXMLTokenMap*      pStyleElemTokenMap;
    SvXMLTokenMap*      pStyleAttrTokenMap;
    SvXMLTokenMap*      pStyleElemAttrTokenMap;
    LocaleDataWrapper*  pLocaleData;
    SvXMLNumFmtEntryArr aNameEntries;

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
    void                    AddKey( sal_uInt32 nKey, const OUString& rName, sal_Bool bRemoveAfterUse );
    void                    SetUsed( sal_uInt32 nKey );
    void                    RemoveVolatileFormats();
};

struct SvXMLNumberInfo
{
    sal_Int32   nDecimals;
    sal_Int32   nInteger;
    sal_Int32   nExpDigits;
    sal_Int32   nNumerDigits;
    sal_Int32   nDenomDigits;
    sal_Int32   nFracDenominator;
    sal_Bool    bGrouping;
    sal_Bool    bDecReplace;
    sal_Bool    bVarDecimals;
    double      fDisplayFactor;
    SvXMLEmbeddedElementArr aEmbeddedElements;

    SvXMLNumberInfo()
    {
        nDecimals = nInteger = nExpDigits = nNumerDigits = nDenomDigits = nFracDenominator = -1;
        bGrouping = bDecReplace = bVarDecimals = sal_False;
        fDisplayFactor = 1.0;
    }
};

class SvXMLNumFmtElementContext : public SvXMLImportContext
{
    SvXMLNumFormatContext&  rParent;
    sal_uInt16              nType;
    OUStringBuffer     aContent;
    SvXMLNumberInfo         aNumInfo;
    LanguageType            nElementLang;
    sal_Bool                bLong;
    sal_Bool                bTextual;
    OUString           sCalendar;

public:
                SvXMLNumFmtElementContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext, sal_uInt16 nNewType,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual     ~SvXMLNumFmtElementContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void Characters( const OUString& rChars );
    virtual void EndElement();

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
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual     ~SvXMLNumFmtEmbeddedTextContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void Characters( const OUString& rChars );
    virtual void EndElement();
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
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual     ~SvXMLNumFmtMapContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void Characters( const OUString& rChars );
    virtual void EndElement();
};

class SvXMLNumFmtPropContext : public SvXMLImportContext
{
    SvXMLNumFormatContext&  rParent;
	sal_Int32				m_nColor;
    sal_Bool                bColSet;

public:
                SvXMLNumFmtPropContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual     ~SvXMLNumFmtPropContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void Characters( const OUString& rChars );
    virtual void EndElement();
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
    XML_TOK_ELEM_ATTR_MIN_INTEGER_DIGITS,
    XML_TOK_ELEM_ATTR_GROUPING,
    XML_TOK_ELEM_ATTR_DISPLAY_FACTOR,
    XML_TOK_ELEM_ATTR_DECIMAL_REPLACEMENT,
    XML_TOK_ELEM_ATTR_DENOMINATOR_VALUE,
    XML_TOK_ELEM_ATTR_MIN_EXPONENT_DIGITS,
    XML_TOK_ELEM_ATTR_MIN_NUMERATOR_DIGITS,
    XML_TOK_ELEM_ATTR_MIN_DENOMINATOR_DIGITS,
    XML_TOK_ELEM_ATTR_RFC_LANGUAGE_TAG,
    XML_TOK_ELEM_ATTR_LANGUAGE,
    XML_TOK_ELEM_ATTR_SCRIPT,
    XML_TOK_ELEM_ATTR_COUNTRY,
    XML_TOK_ELEM_ATTR_STYLE,
    XML_TOK_ELEM_ATTR_TEXTUAL,
    XML_TOK_ELEM_ATTR_CALENDAR
};

//
//  standard colors
//

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

//
//  token maps
//

// maps for SvXMLUnitConverter::convertEnum

static const SvXMLEnumMapEntry aStyleValueMap[] =
{
    { XML_SHORT,            sal_False   },
    { XML_LONG,             sal_True    },
    { XML_TOKEN_INVALID,    0 }
};

static const SvXMLEnumMapEntry aFormatSourceMap[] =
{
    { XML_FIXED,            sal_False },
    { XML_LANGUAGE,         sal_True  },
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
    sal_Bool                    bSystem;
};

static const SvXMLDefaultDateFormat aDefaultDateFormats[] =
{
    // format                           day-of-week     day             month               year            hours           minutes         seconds         format-source

    { NF_DATE_SYSTEM_SHORT,             XML_DEA_NONE,   XML_DEA_ANY,    XML_DEA_ANY,        XML_DEA_ANY,    XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   sal_True },
    { NF_DATE_SYSTEM_LONG,              XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_ANY,        XML_DEA_ANY,    XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   sal_True },
    { NF_DATE_SYS_MMYY,                 XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_LONG,       XML_DEA_SHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   sal_False },
    { NF_DATE_SYS_DDMMM,                XML_DEA_NONE,   XML_DEA_LONG,   XML_DEA_TEXTSHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   sal_False },
    { NF_DATE_SYS_DDMMYYYY,             XML_DEA_NONE,   XML_DEA_LONG,   XML_DEA_LONG,       XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   sal_False },
    { NF_DATE_SYS_DDMMYY,               XML_DEA_NONE,   XML_DEA_LONG,   XML_DEA_LONG,       XML_DEA_SHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   sal_False },
    { NF_DATE_SYS_DMMMYY,               XML_DEA_NONE,   XML_DEA_SHORT,  XML_DEA_TEXTSHORT,  XML_DEA_SHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   sal_False },
    { NF_DATE_SYS_DMMMYYYY,             XML_DEA_NONE,   XML_DEA_SHORT,  XML_DEA_TEXTSHORT,  XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   sal_False },
    { NF_DATE_SYS_DMMMMYYYY,            XML_DEA_NONE,   XML_DEA_SHORT,  XML_DEA_TEXTLONG,   XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   sal_False },
    { NF_DATE_SYS_NNDMMMYY,             XML_DEA_SHORT,  XML_DEA_SHORT,  XML_DEA_TEXTSHORT,  XML_DEA_SHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   sal_False },
    { NF_DATE_SYS_NNDMMMMYYYY,          XML_DEA_SHORT,  XML_DEA_SHORT,  XML_DEA_TEXTLONG,   XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   sal_False },
    { NF_DATE_SYS_NNNNDMMMMYYYY,        XML_DEA_LONG,   XML_DEA_SHORT,  XML_DEA_TEXTLONG,   XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   sal_False },
    { NF_DATETIME_SYSTEM_SHORT_HHMM,    XML_DEA_NONE,   XML_DEA_ANY,    XML_DEA_ANY,        XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_NONE,   sal_True },
    { NF_DATETIME_SYS_DDMMYYYY_HHMMSS,  XML_DEA_NONE,   XML_DEA_ANY,    XML_DEA_ANY,        XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_ANY,    sal_False }
};

//
//  SvXMLNumImpData
//

SvXMLNumImpData::SvXMLNumImpData(
    SvNumberFormatter* pFmt,
    const uno::Reference<uno::XComponentContext>& rxContext )
:   pFormatter(pFmt),
    pStylesElemTokenMap(NULL),
    pStyleElemTokenMap(NULL),
    pStyleAttrTokenMap(NULL),
    pStyleElemAttrTokenMap(NULL),
    pLocaleData(NULL),
    m_xContext(rxContext)
{
    DBG_ASSERT( rxContext.is(), "got no service manager" );
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
    sal_uInt16 nCount = aNameEntries.size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        const SvXMLNumFmtEntry* pObj = &aNameEntries[i];
        if ( pObj->aName == rName )
            return pObj->nKey;              // found
    }
    return NUMBERFORMAT_ENTRY_NOT_FOUND;
}

void SvXMLNumImpData::AddKey( sal_uInt32 nKey, const OUString& rName, sal_Bool bRemoveAfterUse )
{
    if ( bRemoveAfterUse )
    {
        //  if there is already an entry for this key without the bRemoveAfterUse flag,
        //  clear the flag for this entry, too

        sal_uInt16 nCount = aNameEntries.size();
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            SvXMLNumFmtEntry* pObj = &aNameEntries[i];
            if ( pObj->nKey == nKey && !pObj->bRemoveAfterUse )
            {
                bRemoveAfterUse = sal_False;        // clear flag for new entry
                break;
            }
        }
    }
    else
    {
        //  call SetUsed to clear the bRemoveAfterUse flag for other entries for this key
        SetUsed( nKey );
    }

    SvXMLNumFmtEntry* pObj = new SvXMLNumFmtEntry( rName, nKey, bRemoveAfterUse );
    aNameEntries.push_back( pObj );
}

void SvXMLNumImpData::SetUsed( sal_uInt32 nKey )
{
    sal_uInt16 nCount = aNameEntries.size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        SvXMLNumFmtEntry* pObj = &aNameEntries[i];
        if ( pObj->nKey == nKey )
        {
            pObj->bRemoveAfterUse = sal_False;      // used -> don't remove

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

    sal_uInt16 nCount = aNameEntries.size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        const SvXMLNumFmtEntry* pObj = &aNameEntries[i];
        if ( pObj->bRemoveAfterUse )
        {
            const SvNumberformat* pFormat = pFormatter->GetEntry(pObj->nKey);
            if (pFormat && (pFormat->GetType() & NUMBERFORMAT_DEFINED))
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
            { XML_NAMESPACE_NUMBER, XML_TEXT,               XML_TOK_STYLE_TEXT              },
            { XML_NAMESPACE_NUMBER, XML_FILL_CHARACTER,           XML_TOK_STYLE_FILL_CHARACTER   },
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
            { XML_NAMESPACE_NUMBER, XML_MIN_INTEGER_DIGITS,      XML_TOK_ELEM_ATTR_MIN_INTEGER_DIGITS   },
            { XML_NAMESPACE_NUMBER, XML_GROUPING,                XML_TOK_ELEM_ATTR_GROUPING             },
            { XML_NAMESPACE_NUMBER, XML_DISPLAY_FACTOR,          XML_TOK_ELEM_ATTR_DISPLAY_FACTOR       },
            { XML_NAMESPACE_NUMBER, XML_DECIMAL_REPLACEMENT,     XML_TOK_ELEM_ATTR_DECIMAL_REPLACEMENT  },
            { XML_NAMESPACE_NUMBER, XML_DENOMINATOR_VALUE,       XML_TOK_ELEM_ATTR_DENOMINATOR_VALUE  },
            { XML_NAMESPACE_NUMBER, XML_MIN_EXPONENT_DIGITS,     XML_TOK_ELEM_ATTR_MIN_EXPONENT_DIGITS  },
            { XML_NAMESPACE_NUMBER, XML_MIN_NUMERATOR_DIGITS,    XML_TOK_ELEM_ATTR_MIN_NUMERATOR_DIGITS },
            { XML_NAMESPACE_NUMBER, XML_MIN_DENOMINATOR_DIGITS,  XML_TOK_ELEM_ATTR_MIN_DENOMINATOR_DIGITS },
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

//
//  SvXMLNumFmtMapContext
//

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

//
//  SvXMLNumFmtPropContext
//

SvXMLNumFmtPropContext::SvXMLNumFmtPropContext( SvXMLImport& rImport,
                                    sal_uInt16 nPrfx, const OUString& rLName,
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

//
//  SvXMLNumFmtEmbeddedTextContext
//

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

static sal_Bool lcl_ValidChar( sal_Unicode cChar, const SvXMLNumFormatContext& rParent )
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

        return sal_False;   // force quotes
    }

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

    //  don't put quotes around single parentheses (often used for negative numbers)
    if ( ( nFormatType == XML_TOK_STYLES_NUMBER_STYLE ||
           nFormatType == XML_TOK_STYLES_CURRENCY_STYLE ||
           nFormatType == XML_TOK_STYLES_PERCENTAGE_STYLE ) &&
         ( cChar == '(' || cChar == ')' ) )
        return sal_True;

    return sal_False;
}

static void lcl_EnquoteIfNecessary( OUStringBuffer& rContent, const SvXMLNumFormatContext& rParent )
{
    sal_Bool bQuote = sal_True;
    sal_Int32 nLength = rContent.getLength();

    if ( ( nLength == 1 &&
            lcl_ValidChar( rContent[0], rParent ) ) ||
         ( nLength == 2 &&
             lcl_ValidChar( rContent[0], rParent ) &&
             rContent[1] == ' ' ) )
    {
        //  don't quote single separator characters like space or percent,
        //  or separator characters followed by space (used in date formats)
        bQuote = sal_False;
    }
    else if ( rParent.GetType() == XML_TOK_STYLES_PERCENTAGE_STYLE && nLength > 1 )
    {
        //  the percent character in percentage styles must be left out of quoting
        //  (one occurrence is enough even if there are several percent characters in the string)

        OUString aString( rContent.getStr() );
        sal_Int32 nPos = aString.indexOf( (sal_Unicode) '%' );
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
                    rContent.insert( nPos + 1, (sal_Unicode) '"' );
                    rContent.append( (sal_Unicode) '"' );
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
                    rContent.insert( nPos, (sal_Unicode) '"' );
                    rContent.insert( 0, (sal_Unicode) '"' );
                }
            }
            bQuote = sal_False;
        }
        // else: normal quoting (below)
    }

    if ( bQuote )
    {
        // #i55469# quotes in the string itself have to be escaped
        bool bEscape = ( rContent.indexOf( (sal_Unicode) '"' ) >= 0 );
        if ( bEscape )
        {
            // A quote is turned into "\"" - a quote to end quoted text, an escaped quote,
            // and a quote to resume quoting.
            OUString aInsert(  "\"\\\""  );

            sal_Int32 nPos = 0;
            while ( nPos < rContent.getLength() )
            {
                if ( rContent[nPos] == (sal_Unicode) '"' )
                {
                    rContent.insert( nPos, aInsert );
                    nPos += aInsert.getLength();
                }
                ++nPos;
            }
        }

        //  quote string literals
        rContent.insert( 0, (sal_Unicode) '"' );
        rContent.append( (sal_Unicode) '"' );

        // remove redundant double quotes at start or end
        if ( bEscape )
        {
            if ( rContent.getLength() > 2 &&
                 rContent[0] == (sal_Unicode) '"' &&
                 rContent[1] == (sal_Unicode) '"' )
            {
                rContent.remove(0, 2);
            }

            sal_Int32 nLen = rContent.getLength();
            if ( nLen > 2 &&
                 rContent[nLen - 1] == (sal_Unicode) '"' &&
                 rContent[nLen - 2] == (sal_Unicode) '"' )
            {
                rContent.truncate(nLen - 2);
            }
        }
    }
}

//
//  SvXMLNumFmtElementContext
//

const sal_Int32 MAX_SECOND_DIGITS = 20; // fdo#58539 & gnome#627420: limit number of digits during import

SvXMLNumFmtElementContext::SvXMLNumFmtElementContext( SvXMLImport& rImport,
                                    sal_uInt16 nPrfx, const OUString& rLName,
                                    SvXMLNumFormatContext& rParentContext, sal_uInt16 nNewType,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rParent( rParentContext ),
    nType( nNewType ),
    nElementLang( LANGUAGE_SYSTEM ),
    bLong( sal_False ),
    bTextual( sal_False )
{
    LanguageTagODF aLanguageTagODF;
    sal_Int32 nAttrVal;
    bool bAttrBool(false);
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
                if ( !sValue.isEmpty() )
                    aNumInfo.bDecReplace = sal_True;    // only a default string is supported
                else
                    aNumInfo.bVarDecimals = sal_True;   // empty replacement string: variable decimals
                break;
            case XML_TOK_ELEM_ATTR_MIN_EXPONENT_DIGITS:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nExpDigits = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_MIN_NUMERATOR_DIGITS:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nNumerDigits = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_MIN_DENOMINATOR_DIGITS:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nDenomDigits = nAttrVal;
                break;
            case XML_TOK_ELEM_ATTR_DENOMINATOR_VALUE:
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nFracDenominator = nAttrVal;
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
                    bLong = (sal_Bool) nAttrEnum;
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
    if ( !rContent.isEmpty() )
    {
        SvXMLEmbeddedElement* pObj = new SvXMLEmbeddedElement( nFormatPos, rContent );
        if ( !aNumInfo.aEmbeddedElements.insert( pObj ).second )
        {
            //  there's already an element at this position - append text to existing element

            delete pObj;
            for (SvXMLEmbeddedElementArr::iterator it = aNumInfo.aEmbeddedElements.begin();
                 it != aNumInfo.aEmbeddedElements.end(); ++it)
            {
                pObj = &*it;
                if ( pObj->nFormatPos == nFormatPos )
                {
                    pObj->aText += rContent;
                    break;
                }
            }
        }
    }
}

void SvXMLNumFmtElementContext::EndElement()
{
    sal_Bool bEffLong = bLong;
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

                rParent.SetHasLongDoW( sal_False );     // only once
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
            rParent.AddToCode( (sal_Unicode)'@');
            break;
        case XML_TOK_STYLE_FILL_CHARACTER:
            if ( !aContent.isEmpty() )
            {
                rParent.AddToCode( (sal_Unicode)'*' );
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
                    rParent.AddToCode( (sal_Unicode)'0');
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
                    rParent.AddToCode( (sal_Unicode)' ' );
                }

                //! build string and add at once

                sal_Int32 i;
                for (i=0; i<aNumInfo.nNumerDigits; i++)
                {
                    rParent.AddToCode( (sal_Unicode)'?' );
                }
                rParent.AddToCode( (sal_Unicode)'/' );
                if ( aNumInfo.nFracDenominator > 0 )
                {
                    rParent.AddToCode(  OUString::valueOf( aNumInfo.nFracDenominator ) );
                }
                else
                {
                    for (i=0; i<aNumInfo.nDenomDigits; i++)
                    {
                        rParent.AddToCode( (sal_Unicode)'?');
                    }
                }
            }
            break;

        case XML_TOK_STYLE_SCIENTIFIC_NUMBER:
            {
                rParent.AddNumber( aNumInfo );      // simple number

                rParent.AddToCode( OUString("E+") );
                for (sal_Int32 i=0; i<aNumInfo.nExpDigits; i++)
                {
                    rParent.AddToCode( (sal_Unicode)'0' );
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
                sal_Bool bSystem )
{
    static const sal_uInt16 nCount = sizeof(aDefaultDateFormats) / sizeof(SvXMLDefaultDateFormat);
    for (sal_uInt16 nPos=0; nPos<nCount; nPos++)
    {
        const SvXMLDefaultDateFormat& rEntry = aDefaultDateFormats[nPos];
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

//
//  SvXMLNumFormatContext
//

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
    bAutoOrder( sal_False ),
    bFromSystem( sal_False ),
    bTruncate( sal_True ),
    bAutoDec( sal_False ),
    bAutoInt( sal_False ),
    bHasExtraText( sal_False ),
    bHasLongDoW( sal_False ),
    bHasEra( sal_False ),
    bHasDateTime( sal_False ),
    bRemoveAfterUse( sal_False ),
    eDateDOW( XML_DEA_NONE ),
    eDateDay( XML_DEA_NONE ),
    eDateMonth( XML_DEA_NONE ),
    eDateYear( XML_DEA_NONE ),
    eDateHours( XML_DEA_NONE ),
    eDateMins( XML_DEA_NONE ),
    eDateSecs( XML_DEA_NONE ),
    bDateNoDefault( sal_False )
{
    LanguageTagODF aLanguageTagODF;
    OUString sNatNumAttrScript, sNatNumAttrRfcLanguageTag;
    ::com::sun::star::i18n::NativeNumberXmlAttributes aNatNumAttr;
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
                    bFromSystem = (sal_Bool) nAttrEnum;
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
            aFormatCode.append( nNatNum, 10 );

            LanguageType eLang = aLanguageTag.getLanguageType( false);
            if ( eLang == LANGUAGE_DONTKNOW )
                eLang = LANGUAGE_SYSTEM;            //! error handling for unknown locales?
            if ( eLang != nFormatLang && eLang != LANGUAGE_SYSTEM )
            {
                aFormatCode.append( "][$-" );
                // language code in upper hex:
                aFormatCode.append(OUString::valueOf(sal_Int32(eLang), 16).toAsciiUpperCase());
            }
            aFormatCode.append( sal_Unicode(']') );
        }
    }
}

SvXMLNumFormatContext::SvXMLNumFormatContext( SvXMLImport& rImport,
                                    sal_uInt16 nPrfx, const OUString& rLName,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                    const sal_Int32 nTempKey,
                                    SvXMLStylesContext& rStyles ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_DATA_STYLE ),
    pData( NULL ),
    pStyles( &rStyles ),
    aMyConditions(),
    nType( 0 ),
    nKey(nTempKey),
    nFormatLang( LANGUAGE_SYSTEM ),
    bAutoOrder( sal_False ),
    bFromSystem( sal_False ),
    bTruncate( sal_True ),
    bAutoDec( sal_False ),
    bAutoInt( sal_False ),
    bHasExtraText( sal_False ),
    bHasLongDoW( sal_False ),
    bHasEra( sal_False ),
    bHasDateTime( sal_False ),
    bRemoveAfterUse( sal_False ),
    eDateDOW( XML_DEA_NONE ),
    eDateDay( XML_DEA_NONE ),
    eDateMonth( XML_DEA_NONE ),
    eDateYear( XML_DEA_NONE ),
    eDateHours( XML_DEA_NONE ),
    eDateMins( XML_DEA_NONE ),
    eDateSecs( XML_DEA_NONE ),
    bDateNoDefault( sal_False )
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
    SvXMLImportContext* pContext = NULL;

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
            bRemoveAfterUse = sal_False;
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
        bRemoveAfterUse = sal_False;
        CreateAndInsert(sal_True);
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
        CreateAndInsert(sal_True);
        return nKey;
    }
}

sal_Int32 SvXMLNumFormatContext::CreateAndInsert( com::sun::star::uno::Reference< com::sun::star::util::XNumberFormatsSupplier >& xFormatsSupplier )
{
    if (nKey <= -1)
    {
        SvNumberFormatter* pFormatter = NULL;
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

void SvXMLNumFormatContext::CreateAndInsert(sal_Bool /*bOverwrite*/)
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

    for (sal_uInt32 i = 0; i < aMyConditions.size(); i++)
    {
        SvXMLNumFormatContext* pStyle = (SvXMLNumFormatContext *)pStyles->FindStyleChildContext(
            XML_STYLE_FAMILY_DATA_STYLE, aMyConditions[i].sMapName, sal_False);
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
        aFormatCode.appendAscii("\"\"");    // ""
    }

    aFormatCode.insert( 0, aConditions.makeStringAndClear() );
    OUString sFormat = aFormatCode.makeStringAndClear();

    //  test special cases

    if ( bAutoDec )         // automatic decimal places
    {
        //  #99391# adjust only if the format contains no text elements, no conditions
        //  and no color definition (detected by the '[' at the start)

        if ( nType == XML_TOK_STYLES_NUMBER_STYLE && !bHasExtraText &&
                aMyConditions.empty() && sFormat.toChar() != (sal_Unicode)'[' )
            nIndex = pFormatter->GetStandardIndex( nFormatLang );
    }
    if ( bAutoInt )         // automatic integer digits
    {
        //! only if two decimal places was set?

        if ( nType == XML_TOK_STYLES_NUMBER_STYLE && !bHasExtraText &&
                aMyConditions.empty() && sFormat.toChar() != (sal_Unicode)'[' )
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
            sal_Bool bOk = pFormatter->PutEntry( aFormatStr, nErrPos, l_nType, nIndex, nFormatLang );
            if ( !bOk && nErrPos == 0 && aFormatStr != sFormat )
            {
                //  if the string was modified by PutEntry, look for an existing format
                //  with the modified string
                nIndex = pFormatter->GetEntryKey( aFormatStr, nFormatLang );
                if ( nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND )
                    bOk = sal_True;
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

void SvXMLNumFormatContext::Finish( sal_Bool bOverwrite )
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
    bHasExtraText = sal_True;
}

void SvXMLNumFormatContext::AddToCode( const OUString& rString )
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
    if ( rInfo.bDecReplace || rInfo.bVarDecimals )
        nGenPrec = 0;               // generate format without decimals...

    sal_Bool bGrouping = rInfo.bGrouping;
    sal_uInt16 nEmbeddedCount = rInfo.aEmbeddedElements.size();
    if ( nEmbeddedCount )
        bGrouping = sal_False;      // grouping and embedded characters can't be used together

    sal_uInt32 nStdIndex = pFormatter->GetStandardIndex( nFormatLang );
    OUStringBuffer aNumStr = pFormatter->GenerateFormat( nStdIndex, nFormatLang,
                                                         bGrouping, sal_False, nGenPrec, nLeading );

    if ( rInfo.nExpDigits >= 0 && nLeading == 0 && !bGrouping && nEmbeddedCount == 0 )
    {
        // #i43959# For scientific numbers, "#" in the integer part forces a digit,
        // so it has to be removed if nLeading is 0 (".00E+0", not "#.00E+0").

        aNumStr.stripStart((sal_Unicode)'#');
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

        //  aEmbeddedElements is sorted - last entry has the largest position (leftmost)
        const SvXMLEmbeddedElement* pLastObj = &*rInfo.aEmbeddedElements.rbegin();
        sal_Int32 nLastFormatPos = pLastObj->nFormatPos;
        if ( nLastFormatPos >= nZeroPos )
        {
            //  add '#' characters so all embedded texts are really embedded in digits
            //  (there always has to be a digit before the leftmost embedded text)

            sal_Int32 nAddCount = nLastFormatPos + 1 - nZeroPos;
            for(sal_Int32 index = 0; index < nAddCount; ++index)
            {
                aNumStr.insert(0, (sal_Unicode)'#');
            }
            nZeroPos = nZeroPos + nAddCount;
        }

        //  aEmbeddedElements is sorted with ascending positions - loop is from right to left
        for (SvXMLEmbeddedElementArr::const_iterator it = rInfo.aEmbeddedElements.begin();
             it != rInfo.aEmbeddedElements.end(); ++it)
        {
            const SvXMLEmbeddedElement* pObj = &*it;
            sal_Int32 nFormatPos = pObj->nFormatPos;
            sal_Int32 nInsertPos = nZeroPos - nFormatPos;
            if ( nFormatPos >= 0 && nInsertPos >= 0 )
            {
                //  #107805# always quote embedded strings - even space would otherwise
                //  be recognized as thousands separator in French.

                aNumStr.insert(nInsertPos, '"');
                aNumStr.insert(nInsertPos, pObj->aText);
                aNumStr.insert(nInsertPos, '"');
            }
        }
    }

    aFormatCode.append( aNumStr.makeStringAndClear() );

    if ( ( rInfo.bDecReplace || rInfo.bVarDecimals ) && nPrec )     // add decimal replacement (dashes)
    {
        //  add dashes for explicit decimal replacement, # for variable decimals
        sal_Unicode cAdd = rInfo.bDecReplace ? '-' : '#';

        aFormatCode.append( pData->GetLocaleData( nFormatLang ).getNumDecimalSep() );
        for ( sal_uInt16 i=0; i<nPrec; i++)
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
    sal_Bool bAutomatic = sal_False;
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

            bAutomatic = sal_True;
        }
    }
    else if ( nLang == LANGUAGE_SYSTEM && aSymbol.compareToAscii("CCC") == 0 )
    {
        //  "CCC" is used for automatic long symbol
        bAutomatic = sal_True;
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
        aFormatCode.appendAscii( "[$" );            // intro for "new" currency symbols

    aFormatCode.append( aSymbol );

    if (!bAutomatic)
    {
        if ( nLang != LANGUAGE_SYSTEM )
        {
            //  '-' sign and language code in hex:
            aFormatCode.append("-" + OUString::number(sal_Int32(nLang), 16).toAsciiUpperCase());
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
        bHasDateTime = sal_True;
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
            bDateNoDefault = sal_True;      // any other element -> no default format
    }
}

static sal_Bool lcl_IsAtEnd( OUStringBuffer& rBuffer, const OUString& rToken )
{
    sal_Int32 nBufLen = rBuffer.getLength();
    sal_Int32 nTokLen = rToken.getLength();

    if ( nTokLen > nBufLen )
        return sal_False;

    sal_Int32 nStartPos = nBufLen - nTokLen;
    for ( sal_Int32 nTokPos = 0; nTokPos < nTokLen; nTokPos++ )
        if ( rToken[ nTokPos ] != rBuffer[nStartPos + nTokPos] )
            return sal_False;

    return sal_True;
}

sal_Bool SvXMLNumFormatContext::ReplaceNfKeyword( sal_uInt16 nOld, sal_uInt16 nNew )
{
    //  replaces one keyword with another if it is found at the end of the code

    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    if (!pFormatter)
        return sal_False;

    OUString sOldStr = pFormatter->GetKeyword( nFormatLang, nOld );
    if ( lcl_IsAtEnd( aFormatCode, sOldStr ) )
    {
        // remove old keyword
        aFormatCode.setLength( aFormatCode.getLength() - sOldStr.getLength() );

        // add new keyword
        OUString sNewStr = pFormatter->GetKeyword( nFormatLang, nNew );
        aFormatCode.append( sNewStr );

        return sal_True;    // changed
    }
    return sal_False;       // not found
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
        sal_Bool bDefaultCond = sal_False;

        //! collect all conditions first and adjust default to >=0, >0 or <0 depending on count
        //! allow blanks in conditions
        if ( aConditions.isEmpty() && aMyConditions.size() == 1 && sRealCond.compareToAscii( ">=0" ) == 0 )
            bDefaultCond = sal_True;

        if ( nType == XML_TOK_STYLES_TEXT_STYLE && nIndex == 2 )
        {
            //  The third condition in a number format with a text part can only be
            //  "all other numbers", the condition string must be empty.
            bDefaultCond = sal_True;
        }

        if (!bDefaultCond)
        {
            sal_Int32 nPos = sRealCond.indexOf( '.' );
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

        aConditions.append( (sal_Unicode) ';' );
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
            aColName = OUString( pFormatter->GetKeyword( nFormatLang, sal::static_int_cast< sal_uInt16 >(NF_KEY_FIRSTCOLOR + i) ) );
            break;
        }

    if ( !aColName.isEmpty() )
    {
        aColName.insert( 0, (sal_Unicode) '[' );
        aColName.append( (sal_Unicode) ']' );
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
            aFormatCode.appendAscii( "[~" );            // intro for calendar code
            aFormatCode.append( sCalendar );
            aFormatCode.append( (sal_Unicode) ']' );    // end of "new" currency symbolcalendar code
        }
    }
}

sal_Bool SvXMLNumFormatContext::IsSystemLanguage()
{
    return nFormatLang == LANGUAGE_SYSTEM;
}

//
//  SvXMLNumFmtHelper
//

SvXMLNumFmtHelper::SvXMLNumFmtHelper(
    const uno::Reference<util::XNumberFormatsSupplier>& rSupp,
    const uno::Reference<uno::XComponentContext>& rxContext )
{
    DBG_ASSERT( rxContext.is(), "got no service manager" );

    SvNumberFormatter* pFormatter = NULL;
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
    DBG_ASSERT( rxContext.is(), "got no service manager" );

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

const SvXMLTokenMap& SvXMLNumFmtHelper::GetStylesElemTokenMap()
{
    return pData->GetStylesElemTokenMap();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
