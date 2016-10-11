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
    static bool putFlag();
    static bool hasFlag();
    static bool removeFlag();
    static OUString getFileName();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
