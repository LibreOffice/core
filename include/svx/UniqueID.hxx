/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <atomic>

/** Unique ID for an object.
 *
 * Generates an unique ID and stores it in a member variable, so the ID returned
 * by getId() is the same as long as the object is alive.
 *
 * ID 0 means the ID is not yet created, so 0 is not a valid ID
 *
 */
class UniqueID final
{
private:
    sal_uInt64 mnID = 0;

public:
    sal_uInt64 getID() const
    {
        if (!mnID)
        {
            static std::atomic<sal_uInt64> staticCounter(1);
            const_cast<UniqueID*>(this)->mnID = staticCounter.fetch_add(1);
        }
        return mnID;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
