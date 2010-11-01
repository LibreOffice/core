/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lerp.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _BGFX_TOOLS_LERP_HXX
#define _BGFX_TOOLS_LERP_HXX

#include <sal/types.h>

namespace basegfx
{
    namespace tools
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

#endif /* _BGFX_TOOLS_LERP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
