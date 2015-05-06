/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_LOK_HXX
#define INCLUDED_COMPHELPER_LOK_HXX

#include <comphelper/comphelperdllapi.h>

namespace comphelper
{

namespace LibreOfficeKit
{

COMPHELPER_DLLPUBLIC void setActive();

COMPHELPER_DLLPUBLIC bool isActive();

enum class statusIndicatorCallbackType { Start, SetValue, Finish };

COMPHELPER_DLLPUBLIC void setStatusIndicatorCallback(void (*callback)(void *data, statusIndicatorCallbackType type, int percent), void *data);

COMPHELPER_DLLPUBLIC void statusIndicatorStart();

COMPHELPER_DLLPUBLIC void statusIndicatorSetValue(int percent);

COMPHELPER_DLLPUBLIC void statusIndicatorFinish();

}
}

#endif // INCLUDED_COMPHELPER_LOK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
