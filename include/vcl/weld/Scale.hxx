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
#include <vcl/weld/Widget.hxx>

namespace weld
{
class VCL_DLLPUBLIC Scale : virtual public Widget
{
    Link<Scale&, void> m_aValueChangedHdl;

protected:
    void signal_value_changed() { m_aValueChangedHdl.Call(*this); }

public:
    virtual void set_value(int value) = 0;
    virtual int get_value() const = 0;
    virtual void set_range(int min, int max) = 0;

    virtual void set_increments(int step, int page) = 0;
    virtual void get_increments(int& step, int& page) const = 0;

    void connect_value_changed(const Link<Scale&, void>& rLink) { m_aValueChangedHdl = rLink; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
