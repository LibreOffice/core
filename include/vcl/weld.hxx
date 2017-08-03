/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_WELD_HXX
#define INCLUDED_VCL_WELD_HXX

#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>
#include <vcl/virdev.hxx>

namespace Weld
{
    class VCL_DLLPUBLIC Widget
    {
    public:
        virtual void set_sensitive(bool sensitive) = 0;
        virtual bool get_sensitive() const = 0;
        virtual bool get_visible() const = 0;
        virtual void grab_focus() = 0;
        virtual void show() = 0;
        virtual void hide() = 0;
        void show(bool bShow)
        {
            if (bShow)
                show();
            else
                hide();
        }
        virtual void set_width_request(int nWidth) = 0;
        virtual void set_height_request(int nHeight) = 0;
        virtual Size get_preferred_size() const = 0;
        virtual float approximate_char_width() const = 0;
        virtual ~Widget() {}
    };

    class VCL_DLLPUBLIC Frame : virtual public Widget
    {
    public:
        virtual void set_label(const OUString& rText) = 0;
        virtual OUString get_label() const = 0;
    };

    class VCL_DLLPUBLIC Window : virtual public Widget
    {
    public:
        virtual void set_transient_for(Window* pParent) = 0;
        virtual void set_title(const OUString& rTitle) = 0;
    };

    class VCL_DLLPUBLIC Dialog : virtual public Window
    {
    public:
        virtual int run() = 0;
        virtual void response(int response) = 0;
    };

    class VCL_DLLPUBLIC ComboBoxText : virtual public Widget
    {
    protected:
        Link<ComboBoxText&, void> m_aChangeHdl;

        void signal_changed()
        {
            m_aChangeHdl.Call(*this);
        }
    public:
        virtual int get_active() const = 0;
        virtual void set_active(int pos) = 0;
        virtual OUString get_active_text() const = 0;
        virtual OUString get_text(int pos) const = 0;
        virtual void append_text(const OUString& rStr) = 0;
        virtual void insert_text(const OUString& rStr, int pos) = 0;
        virtual int find_text(const OUString& rStr) const = 0;
        virtual int get_count() const = 0;
        virtual void make_sorted() = 0;

        void connect_changed(const Link<ComboBoxText&, void>& rLink)
        {
            m_aChangeHdl = rLink;
        }

        void set_active(const OUString& rStr)
        {
            set_active(find_text(rStr));
        }
    };

    class VCL_DLLPUBLIC TreeView : virtual public Widget
    {
    protected:
        Link<TreeView&, void> m_aChangeHdl;
        Link<TreeView&, void> m_aRowActivatedHdl;

        void signal_changed()
        {
            m_aChangeHdl.Call(*this);
        }

        void signal_row_activated()
        {
            m_aRowActivatedHdl.Call(*this);
        }

    public:

        virtual void append(const OUString& rText) = 0;
        virtual void insert(const OUString& rText, int pos) = 0;
        virtual int n_children() const = 0;
        virtual void select(int pos) = 0;
        virtual void remove(int pos) = 0;
        virtual int find(const OUString& rText) const = 0;
        virtual void move_before(int pos, int before) = 0;
        virtual void clear() = 0;
        virtual OUString get_selected() = 0;
        virtual int get_selected_index() = 0;
        virtual OUString get(int pos) = 0;
        virtual int get_height_rows(int nRows) const = 0;

        virtual void freeze() = 0;
        virtual void thaw() = 0;

        void connect_changed(const Link<TreeView&, void>& rLink)
        {
            m_aChangeHdl = rLink;
        }

        void connect_row_activated(const Link<TreeView&, void>& rLink)
        {
            m_aRowActivatedHdl = rLink;
        }

        void select(const OUString& rText)
        {
            select(find(rText));
        }

        void remove(const OUString& rText)
        {
            remove(find(rText));
        }
    };

    class VCL_DLLPUBLIC Button : virtual public Widget
    {
    protected:
        Link<Button&, void> m_aClickHdl;

        void signal_clicked()
        {
            m_aClickHdl.Call(*this);
        }
    public:
        virtual void set_label(const OUString& rText) = 0;
        virtual OUString get_label() const = 0;

        void connect_clicked(const Link<Button&, void>& rLink)
        {
            m_aClickHdl = rLink;
        }
    };

    class VCL_DLLPUBLIC ToggleButton : virtual public Button
    {
    protected:
        Link<ToggleButton&, void> m_aToggleHdl;
        TriState m_eSavedValue = TRISTATE_FALSE;

        void signal_toggled()
        {
            m_aToggleHdl.Call(*this);
        }
    public:
        virtual void set_active(bool active) = 0;
        virtual bool get_active() const = 0;

        virtual void set_inconsistent(bool inconsistent) = 0;
        virtual bool get_inconsistent() const = 0;

        void save_state()
        {
            if (get_inconsistent())
                m_eSavedValue = TRISTATE_INDET;
            else if (get_active())
                m_eSavedValue = TRISTATE_TRUE;
            else
                m_eSavedValue = TRISTATE_FALSE;
        }

        TriState get_saved_state() const
        {
            return m_eSavedValue;
        }

        void set_state(TriState eState)
        {
            switch (eState)
            {
                case TRISTATE_INDET:
                    set_inconsistent(true);
                    break;
                case TRISTATE_TRUE:
                    set_inconsistent(false);
                    set_active(true);
                    break;
                case TRISTATE_FALSE:
                    set_inconsistent(false);
                    set_active(false);
                    break;
            }
        }

        void connect_toggled(const Link<ToggleButton&, void>& rLink)
        {
            m_aToggleHdl = rLink;
        }
    };

    class VCL_DLLPUBLIC CheckButton : virtual public ToggleButton
    {
    };

    class VCL_DLLPUBLIC RadioButton : virtual public ToggleButton
    {
    };

    class VCL_DLLPUBLIC Entry : virtual public Widget
    {
    protected:
        Link<Entry&, void> m_aChangeHdl;

        void signal_changed()
        {
            m_aChangeHdl.Call(*this);
        }
    public:
        virtual void set_text(const OUString& rText) = 0;
        virtual OUString get_text() const = 0;

        void connect_changed(const Link<Entry&, void>& rLink)
        {
            m_aChangeHdl = rLink;
        }
    };

    class VCL_DLLPUBLIC SpinButton : virtual public Entry
    {
    protected:
        Link<SpinButton&, void> m_aValueChangedHdl;

        void signal_value_changed()
        {
            m_aValueChangedHdl.Call(*this);
        }
    public:
        virtual int get_value_as_int() const = 0;
        virtual void set_value(double value) = 0;
        virtual void set_range(double min, double max) = 0;

        void connect_value_changed(const Link<SpinButton&, void>& rLink)
        {
            m_aValueChangedHdl = rLink;
        }
    };

    class VCL_DLLPUBLIC Label : virtual public Widget
    {
    public:
        virtual void set_label(const OUString& rText) = 0;
    };

    class VCL_DLLPUBLIC TextView : virtual public Widget
    {
    public:
        virtual void set_text(const OUString& rText) = 0;
        virtual OUString get_text() const = 0;
        virtual Selection get_selection() const = 0;
        virtual void set_selection(const Selection&) = 0;
    };

    class VCL_DLLPUBLIC DrawingArea : virtual public Widget
    {
    protected:
        Link<VirtualDevice&, void> m_aDrawHdl;
        Link<const Size&, void> m_aSizeAllocateHdl;
    public:
        void connect_draw(const Link<VirtualDevice&, void>& rLink)
        {
            m_aDrawHdl = rLink;
        }
        void connect_size_allocate(const Link<const Size&, void>& rLink)
        {
            m_aSizeAllocateHdl = rLink;
        }
        virtual void queue_draw() = 0;
    };

    class VCL_DLLPUBLIC Builder
    {
    public:
        virtual Dialog* get_dialog(const OString &id) = 0;
        virtual Window* get_window(const OString &id) = 0;
        virtual Widget* get_widget(const OString &id) = 0;
        virtual Button* get_button(const OString &id) = 0;
        virtual Frame* get_frame(const OString &id) = 0;
        virtual RadioButton* get_radio_button(const OString &id) = 0;
        virtual CheckButton* get_check_button(const OString &id) = 0;
        virtual SpinButton* get_spin_button(const OString &id) = 0;
        virtual ComboBoxText* get_combo_box_text(const OString &id) = 0;
        virtual TreeView* get_tree_view(const OString &id) = 0;
        virtual Label* get_label(const OString &id) = 0;
        virtual TextView* get_text_view(const OString &id) = 0;
        virtual Entry* get_entry(const OString &id) = 0;
        virtual DrawingArea* get_drawing_area(const OString &id) = 0;
        virtual ~Builder() {}
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
