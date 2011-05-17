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

#ifndef SD_SLIDESORTER_CACHE_REQUEST_PRIORITY_CLASS_HXX
#define SD_SLIDESORTER_CACHE_REQUEST_PRIORITY_CLASS_HXX

namespace sd { namespace slidesorter { namespace cache {


/** Each request for a preview creation has a priority.  This enum defines
    the available priorities.  The special values MIN__CLASS and MAX__CLASS
    are/can be used for validation and have to be kept up-to-date.
*/
enum RequestPriorityClass
{
    MIN__CLASS = 0,

    // The slide is visible.  A preview does not yet exist.
    VISIBLE_NO_PREVIEW = MIN__CLASS,
    // The slide is visible.  A preview exists but is not up-to-date anymore.
    VISIBLE_OUTDATED_PREVIEW,
    // The slide is not visible.
    NOT_VISIBLE,

    MAX__CLASS = NOT_VISIBLE
};


} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
