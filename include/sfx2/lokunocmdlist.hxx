/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/dllapi.h>
#include <map>

enum class PayloadType
{
    None,
    IsActivePayload,
    FontNamePayload,
    FontHeightPayload,
    StyleApplyPayload,
    ColorPayload,
    UndoRedoPayload,
    EnabledPayload,
    ParaDirectionPayload,
    Int32Payload,
    TransformPayload,
    StringPayload,
    RowColSelCountPayload,
    StateTableCellPayload,
    BooleanPayload,
    BooleanOrDisabledPayload,
    PointPayload,
    SizePayload,
    StringOrStrSeqPayload,
    StrSeqPayload,
    TableSizePayload,
};

struct KitUnoCommand
{
    // (if != None) --> function used by libreOfficeKitViewCallback in
    // unoctitm.cxx.
    PayloadType payloadType;
    // (if == true) --> command is listened to for status updates. This is used
    // by doc_iniUnoCommands in init.cxx.
    bool initializeForStatusUpdates;
};

// Get list of uno commands that are safe enough to be allowed to use in some cases
// Parameters:
SFX2_DLLPUBLIC const std::map<std::u16string_view, KitUnoCommand>& GetKitUnoCommandList();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
