/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_IDIALOGRENDERABLE_HXX
#define INCLUDED_VCL_IDIALOGRENDERABLE_HXX

#include <config_options.h>
#include <vcl/dllapi.h>
#include <rtl/ustring.hxx>

#include <vector>

namespace tools { class Rectangle; }

class SfxItemSet;

namespace vcl
{

typedef std::pair<const OString, const OString> LOKPayloadItem;

typedef sal_uInt32 LOKWindowId;

class VCL_DLLPUBLIC ILibreOfficeKitNotifier
{
public:
    virtual ~ILibreOfficeKitNotifier();

    /// Callbacks
    virtual void notifyWindow(vcl::LOKWindowId nLOKWindowId,
                              const OUString& rAction,
                              const std::vector<LOKPayloadItem>& rPayload = std::vector<LOKPayloadItem>()) const = 0;

    virtual void libreOfficeKitViewCallback(int nType, const OString& pPayload) const = 0;

    /// Emits a LOK_CALLBACK_INVALIDATE_TILES.
    virtual void notifyInvalidation(tools::Rectangle const *) const = 0;

    /// Debugging
    virtual OString dumpNotifyState() const = 0;
};

} // namespace vcl

#endif // INCLUDED_VCL_IDIALOGRENDERABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
