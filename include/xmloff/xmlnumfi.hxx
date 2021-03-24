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

namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::util { class XNumberFormatsSupplier; }
namespace com::sun::star::xml::sax { class XAttributeList; }

#define XML_NUMBERSTYLES "NumberStyles"

enum class SvXMLStylesTokens
{
    NUMBER_STYLE,
    CURRENCY_STYLE,
    PERCENTAGE_STYLE,
    DATE_STYLE,
    TIME_STYLE,
    BOOLEAN_STYLE,
    TEXT_STYLE
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
                sal_Int32 nElement,
                const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
                SvXMLStylesContext& rStyles);

    SvXMLNumImpData* getData() { return pData.get(); }

    LanguageType            GetLanguageForKey(sal_Int32 nKey);

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
public:
    enum ImplicitCalendar
    {
        DEFAULT,
        SECONDARY,
        OTHER,
        DEFAULT_FROM_OTHER,
        SECONDARY_FROM_OTHER
    };

private:

    SvXMLNumImpData*    pData;
    SvXMLStylesContext*             pStyles;
    std::vector <MyCondition>   aMyConditions;
    SvXMLStylesTokens nType;
    sal_Int32           nKey;
//  OUString       sFormatName;
    OUString       sFormatTitle;
//  OUString       sMapName;
    OUString       sCalendar;
    OUString       aImplicitCalendar[2];
    ImplicitCalendar eImplicitCalendar;
    LanguageType   nFormatLang;
    bool            bAutoOrder;
    bool            bFromSystem;
    bool            bTruncate;
    bool            bAutoDec;       // set in AddNumber
    bool            bAutoInt;       // set in AddNumber
    bool            bHasExtraText;
    OUStringBuffer aFormatCode{64};
    OUStringBuffer aConditions{32};
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
                                    sal_Int32 nElement,
                                    SvXMLNumImpData* pNewData,
                                    SvXMLStylesTokens nNewType,
                                    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
                                    SvXMLStylesContext& rStyles );
                SvXMLNumFormatContext( SvXMLImport& rImport,
                                    const OUString& rName,
                                    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
                                    const sal_Int32 nKey,
                                    LanguageType nLang,
                                    SvXMLStylesContext& rStyles );
    virtual     ~SvXMLNumFormatContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual void CreateAndInsert(bool bOverwrite) override;

    sal_Int32 GetKey();
    sal_Int32 CreateAndInsert( SvNumberFormatter* pFormatter );
    sal_Int32 CreateAndInsert( css::uno::Reference< css::util::XNumberFormatsSupplier > const & xFormatsSupplier );
    SvXMLStylesTokens GetType() const           { return nType; }   // SvXMLStylesTokens

    bool HasLongDoW() const                     { return bHasLongDoW; }
    void SetHasLongDoW(bool bSet)               { bHasLongDoW = bSet; }

    void UpdateCalendar( const OUString& rNewCalendar );
    ImplicitCalendar GetImplicitCalendarState() const { return eImplicitCalendar; }

    const LocaleDataWrapper& GetLocaleData() const;

    void AddToCode( std::u16string_view rString );
    void AddToCode( sal_Unicode c );
    void AddNumber( const SvXMLNumberInfo& rInfo );
    void AddCurrency( const OUString& rContent, LanguageType nLang );

    void AddNfKeyword( sal_uInt16 nIndex );
    bool ReplaceNfKeyword( sal_uInt16 nOld, sal_uInt16 nNew );
    void AddCondition( const sal_Int32 nIndex );
    void AddCondition( const OUString& rCondition, const OUString& rApplyName );
    void AddColor( Color nColor );

    /// determine whether number format uses the system language
    bool IsSystemLanguage() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
