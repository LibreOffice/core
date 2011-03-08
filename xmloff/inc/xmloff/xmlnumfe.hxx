/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLNUMFE_HXX
#define _XMLOFF_XMLNUMFE_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <rtl/ustrbuf.hxx>

#define XML_WRITTENNUMBERSTYLES "WrittenNumberStyles"

class Color;
class LocaleDataWrapper;
class CharClass;
class SvXMLExport;
class SvXMLNamespaceMap;
class SvXMLAttributeList;
class SvNumberFormatter;
class SvNumberformat;
class SvXMLNumUsedList_Impl;
class SvXMLEmbeddedTextEntryArr;

class XMLOFF_DLLPUBLIC SvXMLNumFmtExport
{
private:
    SvXMLExport&                rExport;
    ::rtl::OUString             sPrefix;
    SvNumberFormatter*          pFormatter;
    ::rtl::OUStringBuffer       sTextContent;
    SvXMLNumUsedList_Impl*      pUsedList;
    CharClass*                  pCharClass;
    LocaleDataWrapper*          pLocaleData;

    SAL_DLLPRIVATE void AddCalendarAttr_Impl( const ::rtl::OUString& rCalendar );
    SAL_DLLPRIVATE void AddStyleAttr_Impl( sal_Bool bLong );
    SAL_DLLPRIVATE void AddTextualAttr_Impl( sal_Bool bText );
    SAL_DLLPRIVATE void AddLanguageAttr_Impl( sal_Int32 nLang );

    SAL_DLLPRIVATE void AddToTextElement_Impl( const ::rtl::OUString& rString );
    SAL_DLLPRIVATE void FinishTextElement_Impl();

    SAL_DLLPRIVATE void WriteColorElement_Impl( const Color& rColor );
    SAL_DLLPRIVATE void WriteNumberElement_Impl( sal_Int32 nDecimals, sal_Int32 nInteger,
                                        const ::rtl::OUString& rDashStr, sal_Bool bVarDecimals,
                                        sal_Bool bGrouping, sal_Int32 nTrailingThousands,
                                        const SvXMLEmbeddedTextEntryArr& rEmbeddedEntries );
    SAL_DLLPRIVATE void WriteScientificElement_Impl( sal_Int32 nDecimals, sal_Int32 nInteger,
                                        sal_Bool bGrouping, sal_Int32 nExp );
    SAL_DLLPRIVATE void WriteFractionElement_Impl( sal_Int32 nInteger, sal_Bool bGrouping,
                                        sal_Int32 nNumerator, sal_Int32 nDenominator );
    SAL_DLLPRIVATE void WriteCurrencyElement_Impl( const ::rtl::OUString& rString,
                                        const ::rtl::OUString& rExt );
    SAL_DLLPRIVATE void WriteBooleanElement_Impl();
    SAL_DLLPRIVATE void WriteTextContentElement_Impl();
    SAL_DLLPRIVATE void WriteDayElement_Impl( const ::rtl::OUString& rCalendar, sal_Bool bLong );
    SAL_DLLPRIVATE void WriteMonthElement_Impl( const ::rtl::OUString& rCalendar, sal_Bool bLong, sal_Bool bText );
    SAL_DLLPRIVATE void WriteYearElement_Impl( const ::rtl::OUString& rCalendar, sal_Bool bLong );
    SAL_DLLPRIVATE void WriteEraElement_Impl( const ::rtl::OUString& rCalendar, sal_Bool bLong );
    SAL_DLLPRIVATE void WriteDayOfWeekElement_Impl( const ::rtl::OUString& rCalendar, sal_Bool bLong );
    SAL_DLLPRIVATE void WriteWeekElement_Impl( const ::rtl::OUString& rCalendar );
    SAL_DLLPRIVATE void WriteQuarterElement_Impl( const ::rtl::OUString& rCalendar, sal_Bool bLong );
    SAL_DLLPRIVATE void WriteHoursElement_Impl( sal_Bool bLong );
    SAL_DLLPRIVATE void WriteMinutesElement_Impl( sal_Bool bLong );
    SAL_DLLPRIVATE void WriteSecondsElement_Impl( sal_Bool bLong, sal_uInt16 nDecimals );
    SAL_DLLPRIVATE void WriteAMPMElement_Impl();
    SAL_DLLPRIVATE void WriteMapElement_Impl( sal_Int32 nOp, double fLimit,
                                    sal_Int32 nKey, sal_Int32 nPart );

    SAL_DLLPRIVATE sal_Bool WriteTextWithCurrency_Impl( const ::rtl::OUString& rString,
                            const ::com::sun::star::lang::Locale& rLocale );
    SAL_DLLPRIVATE void ExportPart_Impl( const SvNumberformat& rFormat, sal_uInt32 nKey,
                                sal_uInt16 nPart, sal_Bool bDefPart );

    SAL_DLLPRIVATE void ExportFormat_Impl( const SvNumberformat& rFormat, sal_uInt32 nKey );

public:
    SvXMLNumFmtExport( SvXMLExport& rExport,
                       const ::com::sun::star::uno::Reference<
                        ::com::sun::star::util::XNumberFormatsSupplier >& rSupp );
    SvXMLNumFmtExport( SvXMLExport& rExport,
                       const ::com::sun::star::uno::Reference<
                        ::com::sun::star::util::XNumberFormatsSupplier >& rSupp,
                       const rtl::OUString& rPrefix );

    virtual ~SvXMLNumFmtExport();

    // core API
    void Export( sal_Bool bIsAutoStyle);

    // mark number format as used
    void SetUsed( sal_uInt32 nKey );

    // get the style name that was generated for a key
    ::rtl::OUString GetStyleName( sal_uInt32 nKey );

    void GetWasUsed(com::sun::star::uno::Sequence<sal_Int32>& rWasUsed);
    void SetWasUsed(const com::sun::star::uno::Sequence<sal_Int32>& rWasUsed);



    // two methods to allow the field import/export to treat system languages
    // properly:

    /// obtain number format with system languange for a given key
    sal_uInt32 ForceSystemLanguage( sal_uInt32 nKey );

    /// determine whether number format uses system language
    bool IsSystemLanguage( sal_uInt32 nKey );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
