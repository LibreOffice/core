/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
