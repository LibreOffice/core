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

namespace rtl
{
class OString;
class OStringBuffer;
}
namespace tools
{
class Rectangle;
}

// An extended callback type that allows passing in also some binary data,
// so that post-processing the messages does not require conversions
// from and to strings.

// TODO: It might possibly make sense to drop the generic type/payload function
// and have only a dedicated function for each message type?

class SAL_NO_VTABLE SAL_DLLPUBLIC_RTTI CoKitCallbackInterface
{
public:
    virtual ~CoKitCallbackInterface() {}
    // COKitCallback equivalent.
    virtual void viewCallback(int nType, const rtl::OString& pPayload) = 0;
    // Callback that explicitly provides view id (which is also included in the payload).
    virtual void viewCallbackWithViewId(int nType, const rtl::OString& pPayload, int nViewId) = 0;
    // LOK_CALLBACK_INVALIDATE_TILES
    // nPart is either part, -1 for all-parts, or INT_MIN if
    // comphelper::COKit::isPartInInvalidation() is not set
    virtual void viewInvalidateTilesCallback(const tools::Rectangle* pRect, int nPart, int nMode)
        = 0;
    // A message of the given type should be sent, for performance purpose only a notification
    // is given here, details about the message should be queried from SfxViewShell when necessary.
    // This is used for messages that are generated often but only the last one is needed.
    virtual void viewUpdatedCallback(int nType) = 0;
    // Like viewUpdatedCallback(), but a last message is needed for each nViewId value.
    // SfxViewShell:getLOKPayload() will be called on nSourceViewId view.
    virtual void viewUpdatedCallbackPerViewId(int nType, int nViewId, int nSourceViewId) = 0;
    // There are pending invalidate tiles calls that need to be processed.
    // A call to SfxViewShell::flushPendingLOKInvalidateTiles() should be scheduled.
    virtual void viewAddPendingInvalidateTiles() = 0;
    virtual void dumpState(rtl::OStringBuffer& rState) = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
