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

#ifndef XMLOFF_NUMEHELP_HXX
#define XMLOFF_NUMEHELP_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif

#ifndef __SGI_STL_SET
#include <set>
#endif

namespace rtl
{
    class OUString;
}
namespace binfilter {
class SvXMLExport;

struct XMLNumberFormat
{
    ::rtl::OUString	sCurrency;
    sal_Int32		nNumberFormat;
    sal_Int16		nType;
    sal_Bool		bIsStandard : 1;
    XMLNumberFormat() : nNumberFormat(0), nType(0) {}
    XMLNumberFormat(const ::rtl::OUString& sTempCurrency, sal_Int32 nTempFormat,
        sal_Int16 nTempType) : sCurrency(sTempCurrency), nNumberFormat(nTempFormat),
            nType(nTempType) {}
};

struct LessNumberFormat
{
    sal_Bool operator() (const XMLNumberFormat& rValue1, const XMLNumberFormat& rValue2) const
    {
        return rValue1.nNumberFormat < rValue2.nNumberFormat;
    }
};

typedef ::std::set<XMLNumberFormat, LessNumberFormat>	XMLNumberFormatSet;

class XMLNumberFormatAttributesExportHelper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats > xNumberFormats;
    SvXMLExport*		pExport;
    const ::rtl::OUString     sEmpty;
    const ::rtl::OUString sStandardFormat;
    const ::rtl::OUString sType;
    const ::rtl::OUString sAttrValueType;
    const ::rtl::OUString sAttrValue;
    const ::rtl::OUString sAttrDateValue;
    const ::rtl::OUString sAttrTimeValue;
    const ::rtl::OUString sAttrBooleanValue;
    const ::rtl::OUString sAttrStringValue;
    const ::rtl::OUString sAttrCurrency;
    XMLNumberFormatSet	aNumberFormats;
    sal_uInt16          nNamespace;
public :
    XMLNumberFormatAttributesExportHelper(::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier);
    XMLNumberFormatAttributesExportHelper(::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier,
                                            SvXMLExport& rExport, sal_uInt16 nNamespace);
    ~XMLNumberFormatAttributesExportHelper();
    void SetExport(SvXMLExport* pExport) { this->pExport = pExport; }

    sal_Int16 GetCellType(const sal_Int32 nNumberFormat, ::rtl::OUString& sCurrency, sal_Bool& bIsStandard);

    static void WriteAttributes(SvXMLExport& rXMLExport,
                                const sal_Int16 nTypeKey,
                                const double& rValue,
                                const ::rtl::OUString& rCurrencySymbol,
                                sal_uInt16 nNamespace,
                                sal_Bool bExportValue = sal_True);
    static sal_Bool GetCurrencySymbol(const sal_Int32 nNumberFormat, ::rtl::OUString& rCurrencySymbol,
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & xNumberFormatsSupplier);
    static sal_Int16 GetCellType(const sal_Int32 nNumberFormat, sal_Bool& bIsStandard,
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & xNumberFormatsSupplier);
    static void SetNumberFormatAttributes(SvXMLExport& rXMLExport,
                                          const sal_Int32 nNumberFormat,
                                          const double& rValue,
                                          sal_uInt16 nNamespace,
                                          sal_Bool bExportValue = sal_True);
    static void SetNumberFormatAttributes(SvXMLExport& rXMLExport,
                                          const ::rtl::OUString& rValue,
                                          const ::rtl::OUString& rCharacters,
                                          sal_uInt16 nNamespace,
                                          sal_Bool bExportValue = sal_True,
                                          sal_Bool bExportTypeAttribute = sal_True);

    sal_Bool GetCurrencySymbol(const sal_Int32 nNumberFormat, ::rtl::OUString& rCurrencySymbol);
    sal_Int16 GetCellType(const sal_Int32 nNumberFormat, sal_Bool& bIsStandard);
    void WriteAttributes(const sal_Int16 nTypeKey,
                                          const double& rValue,
                                          const ::rtl::OUString& rCurrencySymbol,
                                          sal_Bool bExportValue = sal_True);
    void SetNumberFormatAttributes(const sal_Int32 nNumberFormat,
                                          const double& rValue,
                                          sal_Bool bExportValue = sal_True);
    void SetNumberFormatAttributes(const ::rtl::OUString& rValue,
                                          const ::rtl::OUString& rCharacters,
                                          sal_Bool bExportValue = sal_True,
                                          sal_Bool bExportTypeAttribute = sal_True);
};

}//end of namespace binfilter
#endif

