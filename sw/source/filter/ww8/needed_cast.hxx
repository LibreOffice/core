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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_NEEDED_CAST_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_NEEDED_CAST_HXX

#include "staticassert.hxx"

namespace ww
{
    template<typename Ret, typename Param> Ret checking_cast(Param in, Ret)
    {
        return static_cast<Ret>(in);
    }

    template<typename Ret> Ret checking_cast(Ret in, Ret)
    {
        OSL_ENSURE( false, "UnnecessaryCast" );
        return in;
    }

    /*
     needed_cast is the same as static_cast except that there will be a compile
     time assert when NDEBUG is not defined and the in and out types are the
     same.  i.e. needed_cast catches unnecessary casts
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
