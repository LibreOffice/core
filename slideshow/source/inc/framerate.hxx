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

#ifndef INCLUDED_SLIDESHOW_FRAMERATE_HXX
#define INCLUDED_SLIDESHOW_FRAMERATE_HXX

#include <sal/types.h>

namespace slideshow { namespace internal {

/** Some frame rate related data.
*/
class FrameRate
{
public:
    /** The minimum number of frames per second is used to calculate the
        minimum number of frames that is to be shown for active activities.
    */
    static const sal_Int32 MinimumFramesPerSecond = 10;

    /** Aim high with the number of preferred number of frames per second.
        This number is the maximum as well and the true number will be lower.
    */
    static const sal_Int32 PreferredFramesPerSecond = 50;
};

} } // end of namespace slideshow::internal

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
