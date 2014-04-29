/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
#ifndef WW_NEEDED_CAST_HXX
#define WW_NEEDED_CAST_HXX

#include <tools/debug.hxx>
#   include "staticassert.hxx"

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
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
