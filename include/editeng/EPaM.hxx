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

#include <sal/config.h>

#include <sal/types.h>

inline constexpr sal_Int32 EE_PARA_MAX = SAL_MAX_INT32;
inline constexpr sal_Int32 EE_TEXTPOS_MAX = SAL_MAX_INT32;

struct EPaM
{
    static constexpr EPaM NotFound() { return { EE_PARA_MAX, EE_TEXTPOS_MAX }; }

    sal_Int32 nPara = 0;
    sal_Int32 nIndex = 0;

    constexpr EPaM() = default;

    constexpr EPaM(sal_Int32 _nParagraph, sal_Int32 _nIndex)
        : nPara(_nParagraph)
        , nIndex(_nIndex)
    {
    }

    bool operator==(const EPaM&) const = default;

    bool operator<(const EPaM& rInstance) const
    {
        return (nPara < rInstance.nPara) || (nPara == rInstance.nPara && nIndex < rInstance.nIndex);
    }
};

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     EPaM const& aPaM)
{
    return stream << "EPaM(" << aPaM.nPara << ',' << aPaM.nIndex << ")";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
