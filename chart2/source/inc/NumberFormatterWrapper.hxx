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

#include "charttoolsdllapi.hxx"
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <tools/solar.h>
#include <tools/date.hxx>

class SvNumberFormatter;
class Color;

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS NumberFormatterWrapper final
{
public:
    NumberFormatterWrapper( const css::uno::Reference< css::util::XNumberFormatsSupplier >& xSupplier );
    ~NumberFormatterWrapper();

    SvNumberFormatter* getSvNumberFormatter() const { return m_pNumberFormatter;}
    const css::uno::Reference< css::util::XNumberFormatsSupplier >&
                getNumberFormatsSupplier() const { return m_xNumberFormatsSupplier; };

    OUString getFormattedString( sal_Int32 nNumberFormatKey, double fValue, Color& rLabelColor, bool& rbColorChanged ) const;
    Date    getNullDate() const;

private: //private member
    css::uno::Reference< css::util::XNumberFormatsSupplier >
                        m_xNumberFormatsSupplier;

    SvNumberFormatter* m_pNumberFormatter;
    css::uno::Any m_aNullDate;
};

class FixedNumberFormatter final
{
public:
    FixedNumberFormatter( const css::uno::Reference< css::util::XNumberFormatsSupplier >& xSupplier
        , sal_Int32 nNumberFormatKey );
    ~FixedNumberFormatter();

    OUString getFormattedString( double fValue, Color& rLabelColor, bool& rbColorChanged ) const;

private:
    NumberFormatterWrapper      m_aNumberFormatterWrapper;
    sal_uLong                       m_nNumberFormatKey;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
