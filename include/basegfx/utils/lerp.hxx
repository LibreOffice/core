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

#ifndef INCLUDED_BASEGFX_UTILS_LERP_HXX
#define INCLUDED_BASEGFX_UTILS_LERP_HXX

#include <sal/types.h>

namespace basegfx
{
    namespace utils
    {
        /** Generic linear interpolator

            @tpl ValueType
            Must have operator+ and operator* defined, and should
            have value semantics.

            @param t
            As usual, t must be in the [0,1] range
        */
        template< typename ValueType > ValueType lerp( const ValueType&     rFrom,
                                                       const ValueType&     rTo,
                                                       double               t )
        {
            // This is only to suppress a double->int warning. All other
            // types should be okay here.
            return static_cast<ValueType>( (1.0-t)*rFrom + t*rTo );
        }
    }
}

#endif // INCLUDED_BASEGFX_UTILS_LERP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
