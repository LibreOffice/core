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

namespace weld
{
class VCL_DLLPUBLIC Button : virtual public Widget
{
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
