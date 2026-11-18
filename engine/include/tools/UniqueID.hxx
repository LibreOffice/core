/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <tools/toolsdllapi.h>

/** Unique ID for an object.
 *
 * Generates a unique ID and stores it in a member variable, so the ID returned
 * by getId() is the same as long as the object is alive.
 *
 * ID numbers start with 1.
 *
 */
class TOOLS_DLLPUBLIC UniqueID final
{
private:
    sal_uInt64 mnID;

public:
    UniqueID();

    sal_uInt64 getID() const { return mnID; }

    /// Takes the id of rOther, for an object that takes the place of another one.
    void takeTheIdOf(const UniqueID& rOther) { mnID = rOther.mnID; }

    /// Reserves that many ids, the first answered and the rest following it, which no id made
    /// afterwards in this process takes unless it is given that one.
    static sal_uInt64 reserve(sal_uInt64 nCount);

    /// Gives the next id made in this thread that one, which was reserved, instead of the next
    /// of the counter; nought gives none.
    static void giveNext(sal_uInt64 nId);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
