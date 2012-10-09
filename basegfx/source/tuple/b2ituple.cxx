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

#include <basegfx/tuple/b2ituple.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <rtl/instance.hxx>

namespace { struct EmptyTuple : public rtl::Static<basegfx::B2ITuple, EmptyTuple> {}; }

namespace basegfx
{
    // external operators
    //////////////////////////////////////////////////////////////////////////

    B2ITuple operator+(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aSum(rTupA);
        aSum += rTupB;
        return aSum;
    }

    B2ITuple operator-(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aSub(rTupA);
        aSub -= rTupB;
        return aSub;
    }

    B2ITuple operator/(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aDiv(rTupA);
        aDiv /= rTupB;
        return aDiv;
    }

    B2ITuple operator*(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aMul(rTupA);
        aMul *= rTupB;
        return aMul;
    }

    B2ITuple operator*(const B2ITuple& rTup, sal_Int32 t)
    {
        B2ITuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

    B2ITuple operator*(sal_Int32 t, const B2ITuple& rTup)
    {
        B2ITuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

    B2ITuple operator/(const B2ITuple& rTup, sal_Int32 t)
    {
        B2ITuple aNew(rTup);
        aNew /= t;
        return aNew;
    }

    B2ITuple operator/(sal_Int32 t, const B2ITuple& rTup)
    {
        B2ITuple aNew(t, t);
        B2ITuple aTmp(rTup);
        aNew /= aTmp;
        return aNew;
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
