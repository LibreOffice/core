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
#include <vcl/weld/Toggleable.hxx>

namespace weld
{
class VCL_DLLPUBLIC CheckButton : virtual public Toggleable
{
protected:
    virtual void do_set_state(TriState eState) = 0;

public:
    // must override Toggleable::get_state to support TRISTATE_INDET
    virtual TriState get_state() const override = 0;

    void set_state(TriState eState)
    {
        disable_notify_events();
        do_set_state(eState);
        enable_notify_events();
    }

    virtual void do_set_active(bool bActive) override final
    {
        do_set_state(bActive ? TRISTATE_TRUE : TRISTATE_FALSE);
    }

    virtual bool get_active() const override final { return get_state() == TRISTATE_TRUE; }

    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual void set_label_wrap(bool wrap) = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
