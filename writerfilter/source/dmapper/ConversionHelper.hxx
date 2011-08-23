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
#ifndef INCLUDED_DMAPPER_CONVERSIONHELPER_HXX
#define INCLUDED_DMAPPER_CONVERSIONHELPER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/util/DateTime.hpp>
namespace com{ namespace sun{ namespace star{
    namespace lang{
        struct Locale;
    }
    namespace table{
        struct BorderLine;
}}}}

namespace writerfilter {
namespace dmapper{
namespace ConversionHelper{

    // create a border line and return the distance value
    sal_Int32 MakeBorderLine( sal_Int32 nSprmValue, ::com::sun::star::table::BorderLine& rToFill );
    void MakeBorderLine( sal_Int32 nLineThickness,   sal_Int32 nLineType,
                              sal_Int32 nLineColor,
                              ::com::sun::star::table::BorderLine& rToFill, bool bIsOOXML );
    //convert the number format string form MS format to SO format
    ::rtl::OUString ConvertMSFormatStringToSO(
            const ::rtl::OUString& rFormat, ::com::sun::star::lang::Locale& rLocale, bool bHijri);
    sal_Int32 convertTwipToMM100(sal_Int32 _t);
    // probably the most useless unit in the world - English Metric Units (EMU) 360 000 EMU == 1cm
    sal_Int32 convertEMUToMM100(sal_Int32 _t);
    sal_Int32 ConvertColor(sal_Int32 nWordColor );
    sal_Int16 convertTableJustification( sal_Int32 nIntValue );
    ::com::sun::star::util::DateTime convertDateTime( const ::rtl::OUString& rDateTimeString );
    sal_Int16 ConvertNumberingType(sal_Int32 nNFC);

} // namespace ConversionHelper
} //namespace dmapper
} // namespace writerfilter
#endif
