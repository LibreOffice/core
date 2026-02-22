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
class VCL_DLLPUBLIC Menu
{
    friend class ::LOKTrigger;

    Link<const OUString&, void> m_aActivateHdl;

protected:
    void signal_activate(const OUString& rIdent) { m_aActivateHdl.Call(rIdent); }

public:
    virtual OUString popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect,
                                   Placement ePlace = Placement::Under)
        = 0;

    void connect_activate(const Link<const OUString&, void>& rLink) { m_aActivateHdl = rLink; }

    virtual void set_sensitive(const OUString& rIdent, bool bSensitive) = 0;
    virtual bool get_sensitive(const OUString& rIdent) const = 0;
    virtual void set_label(const OUString& rIdent, const OUString& rLabel) = 0;
    virtual OUString get_label(const OUString& rIdent) const = 0;
    virtual void set_tooltip_text(const OUString& rIdent, const OUString& rTip) = 0;
    virtual void set_active(const OUString& rIdent, bool bActive) = 0;
    virtual bool get_active(const OUString& rIdent) const = 0;
    virtual void set_visible(const OUString& rIdent, bool bVisible) = 0;

    virtual void insert(int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSurface,
                        const css::uno::Reference<css::graphic::XGraphic>& rImage,
                        TriState eCheckRadioFalse)
        = 0;

    virtual void set_item_help_id(const OUString& rIdent, const OUString& rHelpId) = 0;
    virtual void remove(const OUString& rId) = 0;

    virtual void clear() = 0;

    virtual void insert_separator(int pos, const OUString& rId) = 0;
    void append_separator(const OUString& rId) { insert_separator(-1, rId); }

    void append(const OUString& rId, const OUString& rStr)
    {
        insert(-1, rId, rStr, nullptr, nullptr, nullptr, TRISTATE_INDET);
    }
    void append_check(const OUString& rId, const OUString& rStr)
    {
        insert(-1, rId, rStr, nullptr, nullptr, nullptr, TRISTATE_TRUE);
    }
    void append_radio(const OUString& rId, const OUString& rStr)
    {
        insert(-1, rId, rStr, nullptr, nullptr, nullptr, TRISTATE_FALSE);
    }
    void append(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert(-1, rId, rStr, &rImage, nullptr, nullptr, TRISTATE_INDET);
    }
    void append(const OUString& rId, const OUString& rStr, VirtualDevice& rImage)
    {
        insert(-1, rId, rStr, nullptr, &rImage, nullptr, TRISTATE_INDET);
    }

    // return the number of toplevel nodes
    virtual int n_children() const = 0;

    virtual OUString get_id(int pos) const = 0;

    virtual ~Menu() {}
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
