/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKIT_TYPES_H
#define INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKIT_TYPES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY
typedef void (*LibreOfficeKitCallback)(int nType, const char* pPayload, void* pData);
#endif // defined LOK_USE_UNSTABLE_API || defined LIBO_INTERNAL_ONLY

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKIT_TYPES_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
