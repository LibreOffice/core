/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <basegfx/range/b2irange.hxx>
#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>
#include <vcl/weld/Widget.hxx>
#include <utility>

#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>

#include <assert.h>
#include <vector>

namespace com::sun::star::awt
{
class XWindow;
}
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

namespace tools
{
class JsonWriter;
}

class LOKTrigger;

namespace weld
{
class DialogController;
class EntryTreeView;
class IconView;
class MetricSpinButton;
class TreeView;

class VCL_DLLPUBLIC Container : virtual public Widget
{
    Link<Container&, void> m_aContainerFocusChangedHdl;

protected:
    void signal_container_focus_changed() { m_aContainerFocusChangedHdl.Call(*this); }

public:
    // remove from old container and add to new container in one go
    // new container can be null to just remove from old container
    virtual void move(weld::Widget* pWidget, weld::Container* pNewParent) = 0;
    // create an XWindow as a child of this container. The XWindow is
    // suitable to contain css::awt::XControl items
    virtual css::uno::Reference<css::awt::XWindow> CreateChildFrame() = 0;
    // rLink is called when the focus transitions from a widget outside the container
    // to a widget inside the container or vice versa
    virtual void connect_container_focus_changed(const Link<Container&, void>& rLink)
    {
        m_aContainerFocusChangedHdl = rLink;
    }
    // causes a child of the container to have the keyboard focus
    virtual void child_grab_focus() = 0;
};

class VCL_DLLPUBLIC Box : virtual public Container
{
public:
    // Moves child to a new position in the list of children
    virtual void reorder_child(weld::Widget* pWidget, int position) = 0;
    // Sort ok/cancel etc buttons in platform order
    virtual void sort_native_button_order() = 0;
};

class VCL_DLLPUBLIC Grid : virtual public Container
{
public:
    virtual void set_child_left_attach(weld::Widget& rWidget, int nAttach) = 0;
    virtual int get_child_left_attach(weld::Widget& rWidget) const = 0;
    virtual void set_child_column_span(weld::Widget& rWidget, int nCols) = 0;
    virtual void set_child_top_attach(weld::Widget& rWidget, int nAttach) = 0;
    virtual int get_child_top_attach(weld::Widget& rWidget) const = 0;
};

class VCL_DLLPUBLIC Frame : virtual public Container
{
public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
};

class VCL_DLLPUBLIC ScreenShotEntry
{
public:
    ScreenShotEntry(OUString aHelpId, const basegfx::B2IRange& rB2IRange)
        : msHelpId(std::move(aHelpId))
        , maB2IRange(rB2IRange)
    {
    }

    const basegfx::B2IRange& getB2IRange() const { return maB2IRange; }

    const OUString& GetHelpId() const { return msHelpId; }

private:
    OUString msHelpId;
    basegfx::B2IRange maB2IRange;
};

typedef std::vector<ScreenShotEntry> ScreenShotCollection;

class VCL_DLLPUBLIC WaitObject
{
private:
    weld::Widget* m_pWindow;

public:
    WaitObject(weld::Widget* pWindow)
        : m_pWindow(pWindow)
    {
        if (m_pWindow)
            m_pWindow->set_busy_cursor(true);
    }
    ~WaitObject()
    {
        if (m_pWindow)
            m_pWindow->set_busy_cursor(false);
    }
};

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

class VCL_DLLPUBLIC Toggleable : virtual public Widget
{
    friend class ::LOKTrigger;

protected:
    Link<Toggleable&, void> m_aToggleHdl;
    TriState m_eSavedValue = TRISTATE_FALSE;

    void signal_toggled()
    {
        if (notify_events_disabled())
            return;
        m_aToggleHdl.Call(*this);
    }

    virtual void do_set_active(bool active) = 0;

public:
    void set_active(bool active)
    {
        disable_notify_events();
        do_set_active(active);
        enable_notify_events();
    }

    virtual bool get_active() const = 0;

    virtual TriState get_state() const
    {
        if (get_active())
            return TRISTATE_TRUE;
        return TRISTATE_FALSE;
    }

    void save_state() { m_eSavedValue = get_state(); }
    TriState get_saved_state() const { return m_eSavedValue; }
    bool get_state_changed_from_saved() const { return m_eSavedValue != get_state(); }

    virtual void connect_toggled(const Link<Toggleable&, void>& rLink) { m_aToggleHdl = rLink; }
};

class VCL_DLLPUBLIC ToggleButton : virtual public Button, virtual public Toggleable
{
    friend class ::LOKTrigger;
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

struct VCL_DLLPUBLIC TriStateEnabled
{
    TriState eState;
    bool bTriStateEnabled;
    TriStateEnabled()
        : eState(TRISTATE_INDET)
        , bTriStateEnabled(true)
    {
    }
    void CheckButtonToggled(CheckButton& rToggle);
};

class VCL_DLLPUBLIC RadioButton : virtual public Toggleable
{
public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual void set_label_wrap(bool wrap) = 0;
};

enum class LabelType
{
    Normal,
    Warning,
    Error,
    Title, // this is intended to be used against the background set by set_title_background
};

class VCL_DLLPUBLIC Label : virtual public Widget
{
public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual void set_mnemonic_widget(Widget* pTarget) = 0;
    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_font(const vcl::Font& rFont) = 0;
    virtual void set_label_type(LabelType eType) = 0;
    /*
       If you want to set a warning or error state, see set_label_type
       instead.
    */
    virtual void set_font_color(const Color& rColor) = 0;
};

enum class Placement
{
    Under,
    End
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
