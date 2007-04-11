/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlnumfe.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:34:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLNUMFE_HXX
#define _XMLOFF_XMLNUMFE_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

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

