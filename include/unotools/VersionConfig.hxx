/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#pragma once

#include <sal/config.h>

#include <unotools/unotoolsdllapi.h>

namespace utl
{
/** This method is called when there's a need to determine if the
 * current version of LibreOffice has been upgraded to a newer one.
 */
bool UNOTOOLS_DLLPUBLIC isProductVersionUpgraded();
}
