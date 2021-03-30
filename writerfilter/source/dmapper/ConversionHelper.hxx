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
#pragma once

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/RubyAdjust.hpp>

namespace com::sun::star{
    namespace lang{
        struct Locale;
    }
    namespace table{
        struct BorderLine2;
    }
}

namespace writerfilter::dmapper::ConversionHelper{

    // create a border line and return the distance value
    void MakeBorderLine(sal_Int32 nLineThickness,
                        sal_Int32 nLineType,
                        sal_Int32 nLineColor,
                        css::table::BorderLine2& rToFill,
                        bool bIsOOXML);
    //convert the number format string form MS format to SO format
    OUString ConvertMSFormatStringToSO(const OUString& rFormat, css::lang::Locale& rLocale, bool bHijri);
    // export just for test
    SAL_DLLPUBLIC_EXPORT sal_Int32 convertTwipToMM100(sal_Int32 _t);
    SAL_DLLPUBLIC_EXPORT double convertTwipToMM100Double(sal_Int32 _t);
    SAL_DLLPUBLIC_EXPORT sal_uInt32 convertTwipToMM100Unsigned(sal_Int32 _t);
    sal_Int16 convertTableJustification( sal_Int32 nIntValue );
    css::text::RubyAdjust convertRubyAlign( sal_Int32 nIntValue );
    sal_Int16 ConvertNumberingType(const sal_Int32 nFmt, const sal_Int16 nDefault = css::style::NumberingType::ARABIC);
    sal_Int16 ConvertCustomNumberFormat(std::u16string_view rFormat);

    css::util::DateTime ConvertDateStringToDateTime(const OUString& rDateTime);
} // namespace writerfilter::dmapper::ConversionHelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
