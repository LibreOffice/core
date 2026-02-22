/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/weld/weld.hxx>

namespace weld
{
class VCL_DLLPUBLIC Dialog : virtual public Window
{
private:
    friend DialogController;
    virtual bool runAsync(std::shared_ptr<DialogController> const& rxOwner,
                          const std::function<void(sal_Int32)>& func)
        = 0;

    DECL_LINK(CommandHdl, const CommandEvent&, bool);

protected:
    virtual bool signal_command(const CommandEvent& rCEvt);

public:
    virtual void set_modal(bool bModal) = 0;
    virtual bool get_modal() const = 0;

    // center dialog on its parent
    //
    // bTrackGeometryRequests set to true tries to ensure the dialog will end
    // up still centered on its parent windows final size, taking into account
    // that there may currently be pending geometry requests for the parent not
    // yet processed by the underlying toolkit
    //
    // for e.g gtk this will means the dialog is always centered even when
    // resized, calling set_centered_on_parent with false will turn this
    // off again.
    virtual void set_centered_on_parent(bool bTrackGeometryRequests) = 0;

    virtual int run() = 0;
    // Run async without a controller
    // @param self - must point to this, to enforce that the dialog was created/held by a shared_ptr
    virtual bool runAsync(std::shared_ptr<Dialog> const& rxSelf,
                          const std::function<void(sal_Int32)>& func)
        = 0;
    virtual void response(int response) = 0;
    virtual void add_button(const OUString& rText, int response, const OUString& rHelpId = {}) = 0;
    virtual std::unique_ptr<Button> weld_button_for_response(int response) = 0;
    virtual std::unique_ptr<weld::Container> weld_content_area() = 0;

    // with pOld of null, automatically find the old default widget and unset
    // it, otherwise use as hint to the old default
    virtual void change_default_button(weld::Button* pOld, weld::Button* pNew) = 0;
    virtual bool is_default_button(const weld::Button* pCandidate) const = 0;

    virtual void set_default_response(int nResponse);

    // shrink the dialog down to shown just these widgets
    virtual void collapse(weld::Widget& rEdit, weld::Widget* pButton) = 0;
    // undo previous dialog collapse
    virtual void undo_collapse() = 0;

    virtual void SetInstallLOKNotifierHdl(const Link<void*, vcl::ILibreOfficeKitNotifier*>& rLink)
        = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
