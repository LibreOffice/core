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
class VCL_DLLPUBLIC MenuButton : virtual public ToggleButton
{
    friend class ::LOKTrigger;

    Link<const OUString&, void> m_aSelectHdl;

protected:
    void signal_selected(const OUString& rIdent) { m_aSelectHdl.Call(rIdent); }

public:
    void connect_selected(const Link<const OUString&, void>& rLink) { m_aSelectHdl = rLink; }

    virtual void insert_item(int pos, const OUString& rId, const OUString& rStr,
                             const OUString* pIconName, VirtualDevice* pImageSurface,
                             TriState eCheckRadioFalse)
        = 0;
    void append_item(const OUString& rId, const OUString& rStr)
    {
        insert_item(-1, rId, rStr, nullptr, nullptr, TRISTATE_INDET);
    }
    void append_item_check(const OUString& rId, const OUString& rStr)
    {
        insert_item(-1, rId, rStr, nullptr, nullptr, TRISTATE_TRUE);
    }
    void append_item_radio(const OUString& rId, const OUString& rStr)
    {
        insert_item(-1, rId, rStr, nullptr, nullptr, TRISTATE_FALSE);
    }
    void append_item(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert_item(-1, rId, rStr, &rImage, nullptr, TRISTATE_INDET);
    }
    void append_item(const OUString& rId, const OUString& rStr, VirtualDevice& rImage)
    {
        insert_item(-1, rId, rStr, nullptr, &rImage, TRISTATE_INDET);
    }
    virtual void insert_separator(int pos, const OUString& rId) = 0;
    void append_separator(const OUString& rId) { insert_separator(-1, rId); }
    virtual void remove_item(const OUString& rId) = 0;
    virtual void clear() = 0;
    virtual void set_item_sensitive(const OUString& rIdent, bool bSensitive) = 0;
    virtual void set_item_active(const OUString& rIdent, bool bActive) = 0;
    virtual void set_item_label(const OUString& rIdent, const OUString& rLabel) = 0;
    virtual OUString get_item_label(const OUString& rIdent) const = 0;
    virtual void set_item_visible(const OUString& rIdent, bool bVisible) = 0;

    virtual void set_popover(weld::Widget* pPopover) = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
