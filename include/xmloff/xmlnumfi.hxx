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
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <xmloff/xmlstyle.hxx>
#include <rtl/ustrbuf.hxx>
#include <i18nlangtag/lang.h>
#include <vector>
#include <unotools/localedatawrapper.hxx>

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
class SvXMLStyleContext;
class SvXMLStylesContext;
struct SvXMLNumberInfo;
class SvNumberFormatter;
class SvtSysLocale;
namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
}}}}


//  use SvXMLNumFmtHelper in the context for <office:styles> to create
//  child contexts for data styles

class SvXMLNumFmtHelper
{
    SvXMLNumImpData* pData;

public:
    SvXMLNumFmtHelper(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& rSupp,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

    SvXMLNumFmtHelper(
        SvNumberFormatter* pNumberFormatter,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

    ~SvXMLNumFmtHelper();

    SvXMLStyleContext*  CreateChildContext( SvXMLImport& rImport,
                sal_uInt16 nPrefix, const OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                    SvXMLStylesContext& rStyles);

    SvXMLNumImpData* getData() { return pData; }

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
                sal_Bool bSystem );
};

struct MyCondition
{
    OUString   sCondition;
    OUString   sMapName;
};

class XMLOFF_DLLPUBLIC SvXMLNumFormatContext : public SvXMLStyleContext
{
    SvXMLNumImpData*    pData;
    SvXMLStylesContext*             pStyles;
    std::vector <MyCondition>   aMyConditions;
    sal_uInt16          nType;
    sal_Int32           nKey;
//  OUString       sFormatName;
    OUString       sFormatTitle;
//  OUString       sMapName;
    OUString       sCalendar;
    OUString       sFormatString;
    LanguageType        nFormatLang;
    com::sun::star::lang::Locale    aLocale;
    sal_Bool            bAutoOrder;
    sal_Bool            bFromSystem;
    sal_Bool            bTruncate;
    sal_Bool            bAutoDec;       // set in AddNumber
    sal_Bool            bAutoInt;       // set in AddNumber
    sal_Bool            bHasExtraText;
    OUStringBuffer aFormatCode;
    OUStringBuffer aConditions;
    sal_Bool            bHasLongDoW;
    sal_Bool            bHasEra;
    sal_Bool            bHasDateTime;
    sal_Bool            bRemoveAfterUse;

    //  contained date elements, used to recognize default date formats
    SvXMLDateElementAttributes  eDateDOW;
    SvXMLDateElementAttributes  eDateDay;
    SvXMLDateElementAttributes  eDateMonth;
    SvXMLDateElementAttributes  eDateYear;
    SvXMLDateElementAttributes  eDateHours;
    SvXMLDateElementAttributes  eDateMins;
    SvXMLDateElementAttributes  eDateSecs;
    sal_Bool                    bDateNoDefault;

    SAL_DLLPRIVATE sal_Int32 PrivateGetKey();

public:
                SvXMLNumFormatContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    SvXMLNumImpData* pNewData, sal_uInt16 nNewType,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                    SvXMLStylesContext& rStyles );
                SvXMLNumFormatContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                    const sal_Int32 nKey,
                                    SvXMLStylesContext& rStyles );
    virtual     ~SvXMLNumFormatContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;
    virtual void CreateAndInsert(bool bOverwrite) SAL_OVERRIDE;
    virtual void Finish(bool bOverwrite) SAL_OVERRIDE;

    SvXMLNumImpData* GetData() const                { return pData; }
    sal_Int32 GetKey();
    sal_Int32 CreateAndInsert( SvNumberFormatter* pFormatter );
    sal_Int32 CreateAndInsert( com::sun::star::uno::Reference< com::sun::star::util::XNumberFormatsSupplier >& xFormatsSupplier );
    sal_uInt16 GetType() const                      { return nType; }   // SvXMLStylesTokens

    sal_Bool IsFromSystem() const                   { return bFromSystem; }
    sal_Bool HasLongDoW() const                     { return bHasLongDoW; }
    void SetHasLongDoW(sal_Bool bSet)               { bHasLongDoW = bSet; }
    sal_Bool HasEra() const                         { return bHasEra; }
    void SetHasEra(sal_Bool bSet)                   { bHasEra = bSet; }

    void UpdateCalendar( const OUString& rNewCalendar );

    const LocaleDataWrapper& GetLocaleData() const;

    void AddToCode( const OUString& rString );
    void AddToCode( sal_Unicode c );
    void AddNumber( const SvXMLNumberInfo& rInfo );
    void AddCurrency( const OUString& rContent, LanguageType nLang );

    void AddNfKeyword( sal_uInt16 nIndex );
    sal_Bool ReplaceNfKeyword( sal_uInt16 nOld, sal_uInt16 nNew );
    void AddCondition( const sal_Int32 nIndex );
    void AddCondition( const OUString& rCondition, const OUString& rApplyName );
    void AddColor( sal_uInt32 const nColor );

    /// determine whether number format uses the system language
    sal_Bool IsSystemLanguage();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
