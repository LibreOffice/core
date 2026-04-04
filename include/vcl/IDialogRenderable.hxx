/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_IDIALOGRENDERABLE_HXX
#define INCLUDED_VCL_IDIALOGRENDERABLE_HXX

#include <vcl/dllapi.h>
#include <rtl/ustring.hxx>

#include <vector>

namespace tools { class Rectangle; }

namespace vcl
{

typedef std::pair<const OString, const OString> KitPayloadItem;

typedef sal_uInt32 KitWindowId;

class VCL_DLLPUBLIC ICOKitNotifier
{
public:
    virtual ~ICOKitNotifier();

    /// Callbacks
    virtual void notifyWindow(vcl::KitWindowId nKitWindowId,
                              const OUString& rAction,
                              const std::vector<KitPayloadItem>& rPayload = std::vector<KitPayloadItem>()) const = 0;

    virtual void viewCallback(int nType, const OString& pPayload) const = 0;

    /// Emits a KIT_CALLBACK_INVALIDATE_TILES.
    virtual void notifyInvalidation(tools::Rectangle const *) const = 0;

    /// Emits a KIT_CALLBACK_INVALIDATE_VISIBLE_CURSOR.
    virtual void notifyCursorInvalidation(tools::Rectangle const *, bool bControlEvent, int windowID) const = 0;

    /// Debugging
    virtual OString dumpNotifyState() const = 0;
};

} // namespace vcl

#endif // INCLUDED_VCL_IDIALOGRENDERABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
