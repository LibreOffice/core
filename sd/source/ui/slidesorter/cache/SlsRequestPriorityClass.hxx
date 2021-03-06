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

#pragma once

namespace sd::slidesorter::cache
{
/** Each request for a preview creation has a priority.  This enum defines
    the available priorities.  The special values MIN_CLASS and MAX_CLASS
    are/can be used for validation and have to be kept up-to-date.
*/
enum RequestPriorityClass
{
    MIN_CLASS = 0,

    // The slide is visible.  A preview does not yet exist.
    VISIBLE_NO_PREVIEW = MIN_CLASS,
    // The slide is visible.  A preview exists but is not up-to-date anymore.
    VISIBLE_OUTDATED_PREVIEW,
    // The slide is not visible.
    NOT_VISIBLE,

    MAX_CLASS = NOT_VISIBLE
};

} // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
