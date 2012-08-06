/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
