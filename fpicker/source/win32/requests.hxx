/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <comphelper/sequenceashashmap.hxx>

namespace fpicker
{
namespace win32
{
namespace vista
{
/** @todo document me
 */
class Request
{
    // interface

public:
    explicit Request()
        : m_nRequest(-1)
        , m_lArguments()
    {
    }

    virtual ~Request(){};

    void setRequest(::sal_Int32 nRequest) { m_nRequest = nRequest; }

    ::sal_Int32 getRequest() { return m_nRequest; }

    void clearArguments() { m_lArguments.clear(); }

    template <class TArgumentType>
    void setArgument(const OUString& sName, const TArgumentType& aValue)
    {
        m_lArguments[sName] = css::uno::toAny(aValue);
    }

    template <class TArgumentType>
    TArgumentType getArgumentOrDefault(const OUString& sName, const TArgumentType& aDefault)
    {
        return m_lArguments.getUnpackedValueOrDefault(sName, aDefault);
    }

    css::uno::Any getValue(OUString const& key) const { return m_lArguments.getValue(key); }

    // member

private:
    ::sal_Int32 m_nRequest;
    ::comphelper::SequenceAsHashMap m_lArguments;
};

} // namespace vista
} // namespace win32
} // namespace fpicker

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
