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

#ifndef WW_NEEDED_CAST_HXX
#define WW_NEEDED_CAST_HXX

#include <tools/debug.hxx>
#include "staticassert.hxx"

namespace ww
{
    template<typename Ret, typename Param> Ret checking_cast(Param in, Ret)
    {
        return static_cast<Ret>(in);
    }

    template<typename Ret> Ret checking_cast(Ret in, Ret)
    {
        DBG_ASSERT( false, "UnnecessaryCast" );
        return in;
    }

    /*
     needed_cast is the same as static_cast except that there will be a compile
     time assert when NDEBUG is not defined and the in and out types are the
     same.  i.e. needed_cast catches unneccessary casts
    */
    template<typename Ret, typename Param> Ret needed_cast(Param in)
    {
        /*
         Massage a single argument and a ret value into two arguments to allow
         a determination if the dest type is the same as the sourct type
        */
        return checking_cast(in, Ret());
    }
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
