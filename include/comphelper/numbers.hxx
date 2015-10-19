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

#ifndef INCLUDED_COMPHELPER_NUMBERS_HXX
#define INCLUDED_COMPHELPER_NUMBERS_HXX

#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <comphelper/comphelperdllapi.h>


namespace comphelper
{

    /// returns the css::util::NumberFormat of the given key under the given formats
    COMPHELPER_DLLPUBLIC sal_Int16 getNumberFormatType(const css::uno::Reference<css::util::XNumberFormats>& xFormats, sal_Int32 nKey);

    /// returns the css::util::NumberFormat of the given key under the given formatter
    COMPHELPER_DLLPUBLIC sal_Int16 getNumberFormatType(const css::uno::Reference<css::util::XNumberFormatter>& xFormatter, sal_Int32 nKey);

    /// returns the decimals of the given numeric number formatunder the given formats
    COMPHELPER_DLLPUBLIC css::uno::Any getNumberFormatDecimals(const css::uno::Reference<css::util::XNumberFormats>& xFormats, sal_Int32 nKey);

    /** returns the standard format for the given type and the given _rLocale
    */
    sal_Int32 getStandardFormat(
            const css::uno::Reference<css::util::XNumberFormatter>& xFormatter,
            sal_Int16 nType,
            const css::lang::Locale& _rLocale);

    /** retrieves a the value of a given property for a given format key, relating to a given formatter
    */
    COMPHELPER_DLLPUBLIC css::uno::Any getNumberFormatProperty(
        const css::uno::Reference< css::util::XNumberFormatter >& _rxFormatter,
        sal_Int32 _nKey,
        const OUString& _rPropertyName
    );


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_NUMBERS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
