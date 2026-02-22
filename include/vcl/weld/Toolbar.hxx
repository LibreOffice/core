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

namespace vcl
{
enum class ImageType;
}

namespace weld
{
class Menu;

class VCL_DLLPUBLIC Toolbar : virtual public Widget
{
    Link<const OUString&, void> m_aClickHdl;
    Link<const OUString&, void> m_aToggleMenuHdl;

protected:
    friend class ::LOKTrigger;

    void signal_clicked(const OUString& rIdent) { m_aClickHdl.Call(rIdent); }
    void signal_toggle_menu(const OUString& rIdent) { m_aToggleMenuHdl.Call(rIdent); }

public:
    virtual void set_item_sensitive(const OUString& rIdent, bool bSensitive) = 0;
    virtual bool get_item_sensitive(const OUString& rIdent) const = 0;
    virtual void set_item_active(const OUString& rIdent, bool bActive) = 0;
    virtual bool get_item_active(const OUString& rIdent) const = 0;
    virtual void set_menu_item_active(const OUString& rIdent, bool bActive) = 0;
    virtual bool get_menu_item_active(const OUString& rIdent) const = 0;
    virtual void set_item_menu(const OUString& rIdent, weld::Menu* pMenu) = 0;
    virtual void set_item_popover(const OUString& rIdent, weld::Widget* pPopover) = 0;
    virtual void set_item_visible(const OUString& rIdent, bool bVisible) = 0;
    virtual void set_item_help_id(const OUString& rIdent, const OUString& rHelpId) = 0;
    virtual bool get_item_visible(const OUString& rIdent) const = 0;
    virtual void set_item_label(const OUString& rIdent, const OUString& rLabel) = 0;
    virtual OUString get_item_label(const OUString& rIdent) const = 0;
    virtual void set_item_tooltip_text(const OUString& rIdent, const OUString& rTip) = 0;
    virtual OUString get_item_tooltip_text(const OUString& rIdent) const = 0;
    virtual void set_item_icon_name(const OUString& rIdent, const OUString& rIconName) = 0;
    virtual void set_item_image_mirrored(const OUString& rIdent, bool bMirrored) = 0;
    virtual void set_item_image(const OUString& rIdent,
                                const css::uno::Reference<css::graphic::XGraphic>& rIcon)
        = 0;
    virtual void set_item_image(const OUString& rIdent, VirtualDevice* pDevice) = 0;

    virtual void insert_item(int pos, const OUString& rId) = 0;
    virtual void insert_separator(int pos, const OUString& rId) = 0;
    void append_separator(const OUString& rId) { insert_separator(-1, rId); }

    virtual int get_n_items() const = 0;
    virtual OUString get_item_ident(int nIndex) const = 0;
    virtual void set_item_ident(int nIndex, const OUString& rIdent) = 0;
    virtual void set_item_label(int nIndex, const OUString& rLabel) = 0;
    virtual void set_item_image(int nIndex,
                                const css::uno::Reference<css::graphic::XGraphic>& rIcon)
        = 0;
    virtual void set_item_tooltip_text(int nIndex, const OUString& rTip) = 0;
    virtual void set_item_accessible_name(int nIndex, const OUString& rName) = 0;
    virtual void set_item_accessible_name(const OUString& rIdent, const OUString& rName) = 0;

    virtual vcl::ImageType get_icon_size() const = 0;
    virtual void set_icon_size(vcl::ImageType eType) = 0;

    // return what modifiers are held
    virtual sal_uInt16 get_modifier_state() const = 0;

    // This function returns the position a new item should be inserted if dnd
    // is dropped at rPoint
    virtual int get_drop_index(const Point& rPoint) const = 0;

    void connect_clicked(const Link<const OUString&, void>& rLink) { m_aClickHdl = rLink; }
    void connect_menu_toggled(const Link<const OUString&, void>& rLink)
    {
        m_aToggleMenuHdl = rLink;
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
