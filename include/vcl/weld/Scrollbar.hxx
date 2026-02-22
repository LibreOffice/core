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

enum class ScrollType;

namespace weld
{
class VCL_DLLPUBLIC Scrollbar : virtual public Widget
{
    Link<Scrollbar&, void> m_aValueChangeHdl;

protected:
    void signal_adjustment_value_changed() { m_aValueChangeHdl.Call(*this); }

public:
    virtual void adjustment_configure(int value, int lower, int upper, int step_increment,
                                      int page_increment, int page_size)
        = 0;
    virtual int adjustment_get_value() const = 0;
    virtual void adjustment_set_value(int value) = 0;
    virtual int adjustment_get_upper() const = 0;
    virtual void adjustment_set_upper(int upper) = 0;
    virtual int adjustment_get_page_size() const = 0;
    virtual void adjustment_set_page_size(int size) = 0;
    virtual int adjustment_get_page_increment() const = 0;
    virtual void adjustment_set_page_increment(int size) = 0;
    virtual int adjustment_get_step_increment() const = 0;
    virtual void adjustment_set_step_increment(int size) = 0;
    virtual int adjustment_get_lower() const = 0;
    virtual void adjustment_set_lower(int lower) = 0;

    virtual int get_scroll_thickness() const = 0;
    virtual void set_scroll_thickness(int nThickness) = 0;
    virtual void set_scroll_swap_arrows(bool bSwap) = 0;

    virtual ScrollType get_scroll_type() const = 0;

    void connect_adjustment_value_changed(const Link<Scrollbar&, void>& rLink)
    {
        m_aValueChangeHdl = rLink;
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
