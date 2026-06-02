/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>
#include <vcl/weld/Toggleable.hxx>
#include <vcl/weld/Widget.hxx>

namespace com::sun::star::graphic
{
class XGraphic;
}
class Color;
class VirtualDevice;

namespace vcl
{
class Font;
}

class LOKTrigger;

namespace weld
{
inline OUString toId(const void* pValue)
{
    return OUString::number(reinterpret_cast<sal_uIntPtr>(pValue));
}

template <typename T> T fromId(const OUString& rValue)
{
    return reinterpret_cast<T>(rValue.toUInt64());
}

enum class EntryMessageType
{
    Normal,
    Warning,
    Error,
};

class VCL_DLLPUBLIC Button : virtual public Widget
{
    friend class ::LOKTrigger;

protected:
    Link<Button&, void> m_aClickHdl;

    void signal_clicked() { m_aClickHdl.Call(*this); }

public:
    virtual void set_label(const OUString& rText) = 0;
    // pDevice, the image for the button, or nullptr to unset
    virtual void set_image(VirtualDevice* pDevice) = 0;
    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) = 0;
    virtual void set_from_icon_name(const OUString& rIconName) = 0;
    virtual OUString get_label() const = 0;
    void clicked() { signal_clicked(); }
    bool is_custom_handler_set() { return m_aClickHdl.IsSet(); }

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_font(const vcl::Font& rFont) = 0;

    /* Sometimes, a widget should behave like a button (activate on click,
       accept keyboard focus, etc), but look entirely different.

       pDevice, the custom look to use, or nullptr to unset.

       Typically doing this is ill advised. Consider using
       set_accessible_name if you do. */
    virtual void set_custom_button(VirtualDevice* pDevice) = 0;

    virtual void connect_clicked(const Link<Button&, void>& rLink) { m_aClickHdl = rLink; }
};

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

enum class Placement
{
    Under,
    End
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
