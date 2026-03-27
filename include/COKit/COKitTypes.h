/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COKIT_COKIT_TYPES_H
#define INCLUDED_COKIT_COKIT_TYPES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @see kit::Office::registerCallback().
    @since LibreOffice 6.0
 */
typedef void (*COKitCallback)(int nType, const char* pPayload, void* pData);

/** @see kit::Office::runLoop().
    @since LibreOffice 6.3
 */
typedef int (*COKitPollCallback)(void* pData, int timeoutUs);
typedef void (*COKitWakeCallback)(void* pData);

/// @see kit::Office::registerAnyInputCallback()
typedef bool (*COKitAnyInputCallback)(void* pData, int nMostUrgentPriority);

/// @see kit::Office::registerFileSaveDialogCallback()
typedef void (*COKitFileSaveDialogCallback)(const char* pSuggestedUri, char* pResultUri,
                                            size_t nResultUri);

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_COKIT_COKIT_TYPES_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
