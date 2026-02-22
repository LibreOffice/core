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

enum class VclPolicyType;

namespace weld
{
class VCL_DLLPUBLIC ScrolledWindow : virtual public Widget
{
    friend class ::LOKTrigger;

    Link<ScrolledWindow&, void> m_aVValueChangeHdl;
    Link<ScrolledWindow&, void> m_aHValueChangeHdl;

protected:
    void signal_vadjustment_value_changed() { m_aVValueChangeHdl.Call(*this); }
    void signal_hadjustment_value_changed() { m_aHValueChangeHdl.Call(*this); }

public:
    virtual void hadjustment_configure(int value, int upper, int step_increment, int page_increment,
                                       int page_size)
        = 0;
    virtual int hadjustment_get_value() const = 0;
    virtual void hadjustment_set_value(int value) = 0;
    virtual int hadjustment_get_upper() const = 0;
    virtual void hadjustment_set_upper(int upper) = 0;
    virtual int hadjustment_get_page_size() const = 0;
    virtual void hadjustment_set_page_size(int size) = 0;
    virtual void hadjustment_set_page_increment(int size) = 0;
    virtual void hadjustment_set_step_increment(int size) = 0;
    virtual void set_hpolicy(VclPolicyType eHPolicy) = 0;
    virtual VclPolicyType get_hpolicy() const = 0;
    void connect_hadjustment_value_changed(const Link<ScrolledWindow&, void>& rLink)
    {
        m_aHValueChangeHdl = rLink;
    }

    virtual void vadjustment_configure(int value, int upper, int step_increment, int page_increment,
                                       int page_size)
        = 0;
    virtual int vadjustment_get_value() const = 0;
    virtual void vadjustment_set_value(int value) = 0;
    virtual int vadjustment_get_upper() const = 0;
    virtual void vadjustment_set_upper(int upper) = 0;
    virtual int vadjustment_get_page_size() const = 0;
    virtual void vadjustment_set_page_size(int size) = 0;
    virtual void vadjustment_set_page_increment(int size) = 0;
    virtual void vadjustment_set_step_increment(int size) = 0;
    virtual void set_vpolicy(VclPolicyType eVPolicy) = 0;
    virtual VclPolicyType get_vpolicy() const = 0;
    void connect_vadjustment_value_changed(const Link<ScrolledWindow&, void>& rLink)
    {
        m_aVValueChangeHdl = rLink;
    }
    virtual int get_scroll_thickness() const = 0;
    virtual void set_scroll_thickness(int nThickness) = 0;

    //trying to use custom color for a scrollbar is generally a bad idea.
    virtual void customize_scrollbars(const Color& rBackgroundColor, const Color& rShadowColor,
                                      const Color& rFaceColor)
        = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
