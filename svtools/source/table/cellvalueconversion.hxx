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

#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <unordered_map>
#include <memory>

namespace svt
{
class StandardFormatNormalizer
{
public:
    /** converts the given <code>Any</code> into a <code>double</code> value to be fed into a number formatter
        */
    virtual double convertToDouble(css::uno::Any const& i_value) const = 0;

    /** returns the format key to be used for formatting values
        */
    sal_Int32 getFormatKey() const { return m_nFormatKey; }

protected:
    StandardFormatNormalizer(css::uno::Reference<css::util::XNumberFormatter> const& i_formatter,
                             ::sal_Int32 const i_numberFormatType);

    virtual ~StandardFormatNormalizer() {}

private:
    ::sal_Int32 m_nFormatKey;
};

class CellValueConversion
{
public:
    CellValueConversion();
    ~CellValueConversion();

    OUString convertToString(const css::uno::Any& i_cellValue);

private:
    bool ensureNumberFormatter();
    bool getValueNormalizer(css::uno::Type const& i_valueType,
                            std::shared_ptr<StandardFormatNormalizer>& o_formatter);

    typedef std::unordered_map<OUString, std::shared_ptr<StandardFormatNormalizer>> NormalizerCache;

    css::uno::Reference<css::util::XNumberFormatter> xNumberFormatter;
    bool bAttemptedFormatterCreation;
    NormalizerCache aNormalizers;
};

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
