/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SAFEMODE_HXX
#define INCLUDED_SFX2_SAFEMODE_HXX

#include <sfx2/dllapi.h>

#include <rtl/ustring.hxx>

namespace sfx2 {

class SFX2_DLLPUBLIC SafeMode
{
public:
    /**
     * Write a flag file to the user profile indicating that the next launch should be in safe mode.
     *
     * @return Whether the file could be written successfully
     */
    static bool putFlag();

    /**
     * Check the existence of the safe mode flag file.
     *
     * @return Whether the flag file for the safe mode exists
     */
    static bool hasFlag();

    /**
     * Remove the flag file for the safe mode.
     *
     * @return Whether the flag file could be removed successfully
     */
    static bool removeFlag();

private:
    /** Returns the path of the safe mode flag file.*/
    static OUString getFileName();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
