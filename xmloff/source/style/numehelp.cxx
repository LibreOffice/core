/*************************************************************************
 *
 *  $RCSfile: numehelp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2000-09-25 14:44:26 $
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

#include "numehelp.hxx"

#include "nmspmap.hxx"
#include "xmlnmspe.hxx"
#include "xmlkywd.hxx"
#include "xmluconv.hxx"

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _TOOLS_SOLMATH_HXX
#include <tools/solmath.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

using namespace com::sun::star;

#define XML_TYPE "Type"
#define XML_CURRENCYSYMBOL "CurrencySymbol"
#define XML_CURRENCYEXTENSION "CurrencyExtension"
#define XML_STANDARDFORMAT "StandardFormat"

sal_Bool XMLNumberFormatAttributesExportHelper::GetCurrencySymbol(const sal_Int32 nNumberFormat, rtl::OUString& sCurrencySymbol,
    uno::Reference <util::XNumberFormatsSupplier>& xNumberFormatsSupplier)
{
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xNumberFormats = xNumberFormatsSupplier->getNumberFormats();
        if (xNumberFormats.is())
        {
            uno::Reference <beans::XPropertySet> xNumberPropertySet = xNumberFormats->getByKey(nNumberFormat);
            uno::Any aCurrencySymbol = xNumberPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_CURRENCYSYMBOL)));
            uno::Any aCurrencyExtension = xNumberPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_CURRENCYEXTENSION)));
            if ( aCurrencySymbol >>= sCurrencySymbol)
            {
                rtl::OUString sCurrencyExtension;
                if ( aCurrencyExtension >>= sCurrencyExtension)
                {
                    if ( sCurrencyExtension.getLength() != 0 )
                    {
                        International IntTest((sCurrencyExtension.toInt32(16) * -1));
                        sCurrencySymbol = rtl::OUString(IntTest.GetCurrBankSymbol());
                        return sal_True;
                    }
                    else
                    {
                        if ( sCurrencySymbol.getLength() == 1 && sCurrencySymbol.toChar() == NfCurrencyEntry::GetEuroSymbol() )
                            sCurrencySymbol = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EUR"));
                        return sal_True;
                    }
                }
                else
                    return sal_True;
            }
        }
    }
    return sal_False;
}


sal_Int16 XMLNumberFormatAttributesExportHelper::GetCellType(const sal_Int32 nNumberFormat, sal_Bool& bIsStandard,
    uno::Reference <util::XNumberFormatsSupplier>& xNumberFormatsSupplier)
{
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xNumberFormats = xNumberFormatsSupplier->getNumberFormats();
        if (xNumberFormats.is())
        {
            uno::Reference <beans::XPropertySet> xNumberPropertySet = xNumberFormats->getByKey(nNumberFormat);
            uno::Any aIsStandardFormat = xNumberPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STANDARDFORMAT)));
            aIsStandardFormat >>= bIsStandard;
            uno::Any aNumberFormat = xNumberPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_TYPE)));
            sal_Int16 nNumberFormat;
            if ( aNumberFormat >>= nNumberFormat )
            {
                return nNumberFormat;
            }
        }
    }
    return 0;
}

void XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(SvXMLExport& rXMLExport,
    const sal_Int32 nNumberFormat, const double& fValue, sal_uInt16 nNamespace, sal_Bool bExportValue)
{
    sal_Bool bIsStandard;
    sal_Int16 nTypeKey = GetCellType(nNumberFormat, bIsStandard, rXMLExport.GetNumberFormatsSupplier());
    sal_Bool bWasSetTypeAttribute = sal_False;
    switch(nTypeKey & ~util::NumberFormat::DEFINED)
    {
    case util::NumberFormat::NUMBER:
    case util::NumberFormat::SCIENTIFIC:
    case util::NumberFormat::FRACTION:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttributeASCII(nNamespace, sXML_value_type, sXML_float);
                bWasSetTypeAttribute = sal_True;
            }
        }       // No Break
    case util::NumberFormat::PERCENT:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttributeASCII(nNamespace, sXML_value_type, sXML_percentage);
                bWasSetTypeAttribute = sal_True;
            }
        }       // No Break
    case util::NumberFormat::CURRENCY:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttributeASCII(nNamespace, sXML_value_type, sXML_currency);
                rtl::OUString sCurrencySymbol;
                if ( GetCurrencySymbol(nNumberFormat, sCurrencySymbol, rXMLExport.GetNumberFormatsSupplier()))
                {
                    if (sCurrencySymbol.getLength() > 0)
                        rXMLExport.AddAttribute(nNamespace, sXML_currency, sCurrencySymbol);
                }
                bWasSetTypeAttribute = sal_True;
            }

            if (bExportValue)
            {
                String sValue;
                SolarMath::DoubleToString(sValue, fValue, 'A', INT_MAX, '.', sal_True);
                rXMLExport.AddAttribute(nNamespace, sXML_value, sValue);
            }
        }
        break;
    case util::NumberFormat::DATE:
    case util::NumberFormat::DATETIME:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttributeASCII(nNamespace, sXML_value_type, sXML_date);
                bWasSetTypeAttribute = sal_True;
            }
            if (bExportValue)
            {
                if ( rXMLExport.GetMM100UnitConverter().setNullDate(rXMLExport.GetModel()) )
                {
                    rtl::OUStringBuffer sBuffer;
                    rXMLExport.GetMM100UnitConverter().convertDateTime(sBuffer, fValue);
                    rtl::OUString sOUValue = sBuffer.makeStringAndClear();
                    rXMLExport.AddAttribute(nNamespace, sXML_date_value, sOUValue);
                }
            }
        }
        break;
    case util::NumberFormat::TIME:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttributeASCII(nNamespace, sXML_value_type, sXML_time);
                bWasSetTypeAttribute = sal_True;
            }
            if (bExportValue)
            {
                rtl::OUStringBuffer sBuffer;
                rXMLExport.GetMM100UnitConverter().convertTime(sBuffer, fValue);
                rtl::OUString sOUValue = sBuffer.makeStringAndClear();
                rXMLExport.AddAttribute(nNamespace, sXML_time_value, sOUValue);
            }
        }
        break;
    case util::NumberFormat::LOGICAL:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttributeASCII(nNamespace, sXML_value_type, sXML_boolean);
                bWasSetTypeAttribute = sal_True;
            }
            if (bExportValue)
            {
                rtl::OUString sOUValue;
                double fTempValue = fValue;
                if (SolarMath::ApproxEqual( fTempValue, 1.0 ))
                {
                    sOUValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true));
                }
                else
                {
                    if (SolarMath::ApproxEqual( fValue, 0.0 ))
                    {
                        sOUValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_false));
                    }
                    else
                    {
                        String sValue;
                        SolarMath::DoubleToString(sValue, fTempValue, 'A', INT_MAX, '.', sal_True);
                        sOUValue = sValue;
                    }
                }
                rXMLExport.AddAttribute(nNamespace, sXML_boolean_value, sOUValue);
            }
        }
        break;
    case util::NumberFormat::TEXT:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttributeASCII(nNamespace, sXML_value_type, sXML_float);
                bWasSetTypeAttribute = sal_True;
                if (bExportValue)
                {
                    String sValue;
                    SolarMath::DoubleToString(sValue, fValue, 'A', INT_MAX, '.', sal_True);
                    rXMLExport.AddAttribute(nNamespace, sXML_value, sValue);
                }
            }
        }
        break;
    }

}

void XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(SvXMLExport& rXMLExport,
    const rtl::OUString& sValue, const rtl::OUString& sCharacters, sal_uInt16 nNamespace, sal_Bool bExportValue)
{
    rXMLExport.AddAttributeASCII(nNamespace, sXML_value_type, sXML_string);
    if (bExportValue)
    {
        if (sValue != sCharacters)
        {
            if (!(sValue[0] == '\'' && (sValue.getLength() == sCharacters.getLength() + 1)))
                rXMLExport.AddAttribute(nNamespace, sXML_string_value, sValue);
        }
    }
}

