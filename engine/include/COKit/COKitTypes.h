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

/** @see kit::Office::registerCallback(). */
typedef void (*COKitCallback)(int nType, const char* pPayload, void* pData);

/** @see kit::Office::runLoop(). */
typedef int (*COKitPollCallback)(void* pData, int timeoutUs);
typedef void (*COKitWakeCallback)(void* pData);

/// @see kit::Office::registerAnyInputCallback()
typedef bool (*COKitAnyInputCallback)(void* pData, int nMostUrgentPriority);

/// @see kit::Office::registerFileSaveDialogCallback()
typedef void (*COKitFileSaveDialogCallback)(const char* pSuggestedUri, char* pResultUri,
                                            size_t nResultUri);

/// @see kit::Office::registerRevealInFileManagerCallback()
typedef void (*COKitRevealInFileManagerCallback)(const char* pUri);

typedef struct COKitClipboardProviderStruct COKitClipboardProvider;

/**
 * A clipboard backend the app registers per document to do the raw platform
 * clipboard input and output, in both directions. The engine drives the format
 * decision: on a paste it asks for the list of available types and then for the
 * bytes of the single format it chose; on a copy it hands over the list of
 * formats it offers so the app can advertise them to the platform clipboard.
 *
 * Every callback runs synchronously on the thread that runs the document's UNO
 * commands. In the in-process native apps that is the app's main thread, so the
 * callbacks may touch the platform clipboard directly. A callback must not
 * re-enter the engine.
 *
 * @see kit::Document::installClipboardProvider().
 */
struct COKitClipboardProviderStruct
{
    /**
     * sizeof(COKitClipboardProvider), so the struct can grow later.
     */
    size_t nSize;

    /**
     * Opaque app data passed to every callback.
     */
    void* pUserData;

    /**
     * Copy: advertise the given nullptr-terminated list of mime types on the
     * platform clipboard without serializing any bytes. The app serves the
     * bytes later by pulling a single format from the engine through
     * getClipboard(). May be null.
     */
    void (*advertiseToPlatform)(void* pUserData, const char** pMimeTypes);

    /**
     * Return 1 if the platform clipboard still holds the content this document
     * last advertised, 0 if some other source now owns it. When it still holds
     * ours, the engine pastes from its own in-memory copy (full fidelity); when
     * it does not, the engine reads the platform through the calls below.
     */
    int (*ownsClipboard)(void* pUserData);

    /**
     * Paste: return a nullptr-terminated, malloc'd array of malloc'd mime-type
     * strings the platform clipboard currently offers. No bytes are read. The
     * engine takes ownership and frees each string and the array.
     */
    char** (*getMimeTypes)(void* pUserData);

    /**
     * Paste: fetch the bytes for one mime type. On success set *pOutData to a
     * malloc'd buffer and *pOutSize to its length and return 1; on failure
     * return 0. The engine frees *pOutData.
     */
    int (*getDataForMimeType)(void* pUserData, const char* pMimeType, char** pOutData,
                              size_t* pOutSize);

    /**
     * The provider is no longer referenced; release pUserData and any platform
     * snapshot the app held for it. May be null.
     */
    void (*release)(void* pUserData);
};

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_COKIT_COKIT_TYPES_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
