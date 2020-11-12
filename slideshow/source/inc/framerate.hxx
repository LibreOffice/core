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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_FRAMERATE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_FRAMERATE_HXX

#include <sal/types.h>

namespace slideshow::internal
{
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

} // end of namespace slideshow::internal

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
