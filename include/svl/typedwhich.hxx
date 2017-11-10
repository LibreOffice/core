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
#ifndef INCLUDED_SVL_TYPEDWHICH_HXX
#define INCLUDED_SVL_TYPEDWHICH_HXX

#include <sal/config.h>
#include <sal/types.h>

/**
 * A very thin wrapper around the sal_uInt16 WhichId whose purpose is mostly to carry type information,
 * so that we Put() and Get() the right subclasses of SfxPoolItem for each WhichId.
 */
template <class T> class TypedWhichId final
{
public:
    constexpr TypedWhichId(sal_uInt16 nWhich)
        : mnWhich(nWhich)
    {
    }
    constexpr sal_uInt16 Which() const { return mnWhich; }
    //constexpr operator sal_uInt16() const { return mnWhich; }
private:
    sal_uInt16 const mnWhich;
};

template <class T> constexpr bool operator==(sal_uInt16 lhs, TypedWhichId<T> const& rhs)
{
    return lhs == rhs.Which();
}
template <class T> constexpr bool operator!=(sal_uInt16 lhs, TypedWhichId<T> const& rhs)
{
    return lhs != rhs.Which();
}
template <class T> constexpr bool operator==(TypedWhichId<T> const& lhs, sal_uInt16 rhs)
{
    return lhs.Which() == rhs;
}
template <class T> constexpr bool operator!=(TypedWhichId<T> const& lhs, sal_uInt16 rhs)
{
    return lhs.Which() != rhs;
}

#endif // INCLUDED_SVL_TYPEDWHICH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
