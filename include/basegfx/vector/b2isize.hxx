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

#include <basegfx/tuple/Size2D.hxx>

namespace basegfx
{
class B2ISize : public Size2D<sal_Int32>
{
public:
    B2ISize()
        : Size2D(0, 0)
    {
    }

    B2ISize(sal_Int32 nX, sal_Int32 nY)
        : Size2D(nX, nY)
    {
    }

    B2ISize(Size2D<sal_Int32> const& rSize)
        : Size2D(rSize)
    {
    }

    using Size2D<sal_Int32>::operator+=;
    using Size2D<sal_Int32>::operator-=;
    using Size2D<sal_Int32>::operator*=;
    using Size2D<sal_Int32>::operator/=;
    using Size2D<sal_Int32>::operator-;
};

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const B2ISize& size)
{
    return stream << "(" << size.getWidth() << "," << size.getHeight() << ")";
}

} // end basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
