/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>

namespace tools
{
class Rectangle;
}

// An extended callback type that allows passing in also some binary data,
// so that post-processing the messages does not require conversions
// from and to strings.

// TODO: It might possibly make sense to drop the generic type/payload function
// and have only a dedicated function for each message type?

class SAL_NO_VTABLE SAL_DLLPUBLIC_RTTI SfxLokCallbackInterface
{
public:
    virtual ~SfxLokCallbackInterface() {}
    // LibreOfficeKitCallback equivalent.
    virtual void libreOfficeKitViewCallback(int nType, const char* pPayload) = 0;
    // Callback that explicitly provides view id (which is also included in the payload).
    virtual void libreOfficeKitViewCallbackWithViewId(int nType, const char* pPayload, int nViewId)
        = 0;
    // LOK_CALLBACK_INVALIDATE_TILES
    // nPart is either part, -1 for all-parts, or INT_MIN if
    // comphelper::LibreOfficeKit::isPartInInvalidation() is not set
    virtual void libreOfficeKitViewInvalidateTilesCallback(const tools::Rectangle* pRect, int nPart)
        = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
