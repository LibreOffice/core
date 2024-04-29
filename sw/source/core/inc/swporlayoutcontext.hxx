/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>

class SwLinePortionLayoutContext
{
public:
    sal_Int32 m_nBegin = -1;
    sal_Int32 m_nEnd = -1;

    SwLinePortionLayoutContext() = default;
    SwLinePortionLayoutContext(sal_Int32 nBegin, sal_Int32 nEnd)
        : m_nBegin(nBegin)
        , m_nEnd(nEnd)
    {
    }
};
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
