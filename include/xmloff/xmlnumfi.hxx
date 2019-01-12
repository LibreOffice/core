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

#ifndef INCLUDED_XMLOFF_XMLNUMFI_HXX
#define INCLUDED_XMLOFF_XMLNUMFI_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <xmloff/xmlstyle.hxx>
#include <rtl/ustrbuf.hxx>
#include <i18nlangtag/lang.h>
#include <memory>
#include <vector>

namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }
namespace com { namespace sun { namespace star { namespace util { class XNumberFormatsSupplier; } } } }
namespace com { namespace sun { namespace star { namespace xml { namespace sax { class XAttributeList; } } } } }

#define XML_NUMBERSTYLES "NumberStyles"

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

enum SvXMLDateElementAttributes
{
    XML_DEA_NONE,
    XML_DEA_ANY,
    XML_DEA_SHORT,
    XML_DEA_LONG,
    XML_DEA_TEXTSHORT,
    XML_DEA_TEXTLONG
};

class Color;
class SvXMLNumImpData;
class SvXMLImport;
struct SvXMLNumberInfo;
class SvNumberFormatter;
class LocaleDataWrapper;


//  use SvXMLNumFmtHelper in the context for <office:styles> to create
//  child contexts for data styles

class SvXMLNumFmtHelper
{
    std::unique_ptr<SvXMLNumImpData> pData;

public:
    SvXMLNumFmtHelper(
        const css::uno::Reference< css::util::XNumberFormatsSupplier >& rSupp,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    SvXMLNumFmtHelper(
        SvNumberFormatter* pNumberFormatter,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    ~SvXMLNumFmtHelper();

    SvXMLStyleContext*  CreateChildContext( SvXMLImport& rImport,
                sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
                SvXMLStylesContext& rStyles);

    SvXMLNumImpData* getData() { return pData.get(); }

    const SvXMLTokenMap&    GetStylesElemTokenMap();

//  sal_uInt32  GetKeyForName( const OUString& rName );
};

//  SvXMLNumFmtDefaults is used in import and export

class SvXMLNumFmtDefaults
{
public:
    // return value is NfIndexTableOffset
    static sal_uInt16 GetDefaultDateFormat( SvXMLDateElementAttributes eDOW,
                SvXMLDateElementAttributes eDay, SvXMLDateElementAttributes eMonth,
                SvXMLDateElementAttributes eYear, SvXMLDateElementAttributes eHours,
                SvXMLDateElementAttributes eMins, SvXMLDateElementAttributes eSecs,
                bool bSystem );
};

struct MyCondition
{
    OUString   sCondition;
    OUString   sMapName;
};

class XMLOFF_DLLPUBLIC SvXMLNumFormatContext : public SvXMLStyleContext
{
    SvXMLNumImpData*    pData;
    SvXMLStylesContext* const   pStyles;
    std::vector <MyCondition>   aMyConditions;
    sal_uInt16 const          nType;
    sal_Int32           nKey;
//  OUString       sFormatName;
    OUString       sFormatTitle;
//  OUString       sMapName;
    OUString       sCalendar;
    LanguageType   nFormatLang;
    bool            bAutoOrder;
    bool            bFromSystem;
    bool            bTruncate;
    bool            bAutoDec;       // set in AddNumber
    bool            bAutoInt;       // set in AddNumber
    bool            bHasExtraText;
    OUStringBuffer aFormatCode;
    OUStringBuffer aConditions;
    bool            bHasLongDoW;
    bool            bHasEra;
    bool            bHasDateTime;
    bool            bRemoveAfterUse;

    //  contained date elements, used to recognize default date formats
    SvXMLDateElementAttributes  eDateDOW;
    SvXMLDateElementAttributes  eDateDay;
    SvXMLDateElementAttributes  eDateMonth;
    SvXMLDateElementAttributes  eDateYear;
    SvXMLDateElementAttributes  eDateHours;
    SvXMLDateElementAttributes  eDateMins;
    SvXMLDateElementAttributes  eDateSecs;
    bool                        bDateNoDefault;

    SAL_DLLPRIVATE sal_Int32 PrivateGetKey();

public:
                SvXMLNumFormatContext( SvXMLImport& rImport,
                sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    SvXMLNumImpData* pNewData,
                                    sal_uInt16 nNewType,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
                                    SvXMLStylesContext& rStyles );
                SvXMLNumFormatContext( SvXMLImport& rImport,
                                    sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
                                    const sal_Int32 nKey,
                                    SvXMLStylesContext& rStyles );
    virtual     ~SvXMLNumFormatContext() override;

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void CreateAndInsert(bool bOverwrite) override;

    SvXMLNumImpData* GetData() const                { return pData; }
    sal_Int32 GetKey();
    sal_Int32 CreateAndInsert( SvNumberFormatter* pFormatter );
    sal_Int32 CreateAndInsert( css::uno::Reference< css::util::XNumberFormatsSupplier > const & xFormatsSupplier );
    sal_uInt16 GetType() const                      { return nType; }   // SvXMLStylesTokens

    bool HasLongDoW() const                     { return bHasLongDoW; }
    void SetHasLongDoW(bool bSet)               { bHasLongDoW = bSet; }
    bool HasEra() const                         { return bHasEra; }

    void UpdateCalendar( const OUString& rNewCalendar, bool bImplicitSecondaryCalendarEC = false );

    const LocaleDataWrapper& GetLocaleData() const;

    void AddToCode( const OUString& rString );
    void AddToCode( sal_Unicode c );
    void AddNumber( const SvXMLNumberInfo& rInfo );
    void AddCurrency( const OUString& rContent, LanguageType nLang );

    void AddNfKeyword( sal_uInt16 nIndex );
    bool ReplaceNfKeyword( sal_uInt16 nOld, sal_uInt16 nNew );
    void AddCondition( const sal_Int32 nIndex );
    void AddCondition( const OUString& rCondition, const OUString& rApplyName );
    void AddColor( Color nColor );

    /// determine whether number format uses the system language
    bool IsSystemLanguage();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
