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

#ifndef INCLUDED_XMLOFF_XMLNUMFE_HXX
#define INCLUDED_XMLOFF_XMLNUMFE_HXX

#include <config_options.h>
#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/uno/Sequence.h>
#include <rtl/ustrbuf.hxx>
#include <i18nlangtag/lang.h>
#include <memory>

namespace com::sun::star::lang { struct Locale; }
namespace com::sun::star::uno { template <typename > class Reference; }
namespace com::sun::star::util { class XNumberFormatsSupplier; }

inline constexpr OUString XML_WRITTENNUMBERSTYLES = u"WrittenNumberStyles"_ustr;

class Color;
class LocaleDataWrapper;
class SvXMLExport;
class SvNumberFormatter;
class SvNumberformat;
class SvXMLNumUsedList_Impl;

class SvXMLEmbeddedTextEntryArr;

class UNLESS_MERGELIBS_MORE(XMLOFF_DLLPUBLIC) SvXMLNumFmtExport final
{
private:
    SvXMLExport&                m_rExport;
    OUString                    m_sPrefix;
    SvNumberFormatter*          m_pFormatter;
    OUStringBuffer              m_sTextContent;
    OUStringBuffer              m_sBlankWidthString;
    bool                        m_bHasText;
    std::unique_ptr<SvXMLNumUsedList_Impl>      m_pUsedList;
    const LocaleDataWrapper*    m_pLocaleData { nullptr };

    SAL_DLLPRIVATE void AddCalendarAttr_Impl( const OUString& rCalendar );
    SAL_DLLPRIVATE void AddStyleAttr_Impl( bool bLong );
    SAL_DLLPRIVATE void AddLanguageAttr_Impl( LanguageType nLang );

    SAL_DLLPRIVATE void AddToTextElement_Impl( std::u16string_view rString );
    SAL_DLLPRIVATE void FinishTextElement_Impl(bool bUseExtensionNS = false);

    SAL_DLLPRIVATE void WriteColorElement_Impl( const Color& rColor );
    SAL_DLLPRIVATE void WriteIntegerElement_Impl( sal_Int32 nInteger, sal_Int32 nBlankInteger, bool bGrouping );
    SAL_DLLPRIVATE void WriteEmbeddedEntries_Impl( const SvXMLEmbeddedTextEntryArr& rEmbeddedEntries );
    SAL_DLLPRIVATE void WriteNumberElement_Impl( sal_Int32 nDecimals, sal_Int32 nMinDecimals,
                                        sal_Int32 nInteger, sal_Int32 nBlankInteger, const OUString& rDashStr,
                                        bool bGrouping, sal_Int32 nTrailingThousands,
                                        const SvXMLEmbeddedTextEntryArr& rEmbeddedEntries );
    SAL_DLLPRIVATE void WriteScientificElement_Impl( sal_Int32 nDecimals, sal_Int32 nMinDecimals, sal_Int32 nInteger, sal_Int32 nBlankInteger,
                                        bool bGrouping, sal_Int32 nExp, sal_Int32 nExpInterval, bool bExpSign, bool bExponentLowercase, sal_Int32 nBlankExp,
                                        const SvXMLEmbeddedTextEntryArr& rEmbeddedEntries );
    SAL_DLLPRIVATE void WriteFractionElement_Impl( sal_Int32 nInteger, sal_Int32 nBlankInteger, bool bGrouping,
                                                   const SvNumberformat& rFormat, sal_uInt16 nPart );
    SAL_DLLPRIVATE void WriteCurrencyElement_Impl( const OUString& rString,
                                        std::u16string_view rExt );
    SAL_DLLPRIVATE void WriteBooleanElement_Impl();
    SAL_DLLPRIVATE void WriteTextContentElement_Impl();
    SAL_DLLPRIVATE void WriteDayElement_Impl( const OUString& rCalendar, bool bLong );
    SAL_DLLPRIVATE void WriteMonthElement_Impl( const OUString& rCalendar, bool bLong, bool bText );
    SAL_DLLPRIVATE void WriteYearElement_Impl( const OUString& rCalendar, bool bLong );
    SAL_DLLPRIVATE void WriteEraElement_Impl( const OUString& rCalendar, bool bLong );
    SAL_DLLPRIVATE void WriteDayOfWeekElement_Impl( const OUString& rCalendar, bool bLong );
    SAL_DLLPRIVATE void WriteWeekElement_Impl( const OUString& rCalendar );
    SAL_DLLPRIVATE void WriteQuarterElement_Impl( const OUString& rCalendar, bool bLong );
    SAL_DLLPRIVATE void WriteHoursElement_Impl( bool bLong );
    SAL_DLLPRIVATE void WriteMinutesElement_Impl( bool bLong );
    SAL_DLLPRIVATE void WriteSecondsElement_Impl( bool bLong, sal_uInt16 nDecimals );
    SAL_DLLPRIVATE void WriteAMPMElement_Impl();
    SAL_DLLPRIVATE void WriteMapElement_Impl( sal_Int32 nOp, double fLimit,
                                    sal_Int32 nKey, sal_Int32 nPart );

    SAL_DLLPRIVATE void WriteRepeatedElement_Impl( sal_Unicode ch );
    SAL_DLLPRIVATE bool WriteTextWithCurrency_Impl( const OUString& rString,
                            const css::lang::Locale& rLocale );
    SAL_DLLPRIVATE void ExportPart_Impl( const SvNumberformat& rFormat, sal_uInt32 nKey, sal_uInt32 nRealKey,
                                sal_uInt16 nPart, bool bDefPart );

    SAL_DLLPRIVATE void ExportFormat_Impl( const SvNumberformat& rFormat, sal_uInt32 nKey, sal_uInt32 nRealKey );

public:
    SvXMLNumFmtExport( SvXMLExport& rExport,
                       const css::uno::Reference< css::util::XNumberFormatsSupplier >& rSupp );
    SvXMLNumFmtExport( SvXMLExport& rExport,
                       const css::uno::Reference< css::util::XNumberFormatsSupplier >& rSupp,
                       OUString sPrefix );

    ~SvXMLNumFmtExport();

    // core API
    void Export( bool bIsAutoStyle);

    // mark number format as used
    void SetUsed( sal_uInt32 nKey );

    // get the style name that was generated for a key
    OUString GetStyleName( sal_uInt32 nKey );

    css::uno::Sequence<sal_Int32> GetWasUsed() const;
    void SetWasUsed(const css::uno::Sequence<sal_Int32>& rWasUsed);


    // two methods to allow the field import/export to treat system languages
    // properly:

    /// obtain number format with system language for a given key
    sal_uInt32 ForceSystemLanguage( sal_uInt32 nKey );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
