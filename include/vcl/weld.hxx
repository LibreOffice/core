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
#include <vcl/field.hxx>
#include <vcl/virdev.hxx>

namespace Weld
{
    class VCL_DLLPUBLIC Widget
    {
    public:
        virtual void set_sensitive(bool sensitive) = 0;
        virtual bool get_sensitive() const = 0;
        virtual void set_visible(bool visible) = 0;
        virtual bool get_visible() const = 0;
        virtual void grab_focus() = 0;
        virtual bool has_focus() const = 0;
        virtual void show() = 0;
        virtual void hide() = 0;
        void show(bool bShow)
        {
            if (bShow)
                show();
            else
                hide();
        }
        virtual void set_size_request(int nWidth, int nHeight) = 0;
        virtual Size get_preferred_size() const = 0;
        virtual float approximate_char_width() const = 0;
        virtual Size get_pixel_size(const OUString& rText) const = 0;
        virtual OString get_buildable_name() const = 0;

        virtual ~Widget() {}
    };

    class VCL_DLLPUBLIC Container : virtual public Widget
    {
    public:
        virtual void add(Weld::Widget* pWidget) = 0;
    };

    class VCL_DLLPUBLIC Frame : virtual public Container
    {
    public:
        virtual void set_label(const OUString& rText) = 0;
        virtual OUString get_label() const = 0;
    };

    class VCL_DLLPUBLIC Notebook : virtual public Container
    {
    protected:
        Link<const OString&, bool> m_aSwitchPageHdl;
    public:
        virtual int get_current_page() const = 0;
        virtual OString get_current_page_ident() const = 0;
        virtual void set_current_page(int nPage) = 0;
        virtual void set_current_page(const OString& rIdent) = 0;
        virtual int get_n_pages() const = 0;
        virtual Weld::Container* get_page(const OString& rIdent) const = 0;

        void connect_switch_page(const Link<const OString&, bool>& rLink)
        {
            m_aSwitchPageHdl = rLink;
        }
    };

    class VCL_DLLPUBLIC Window : virtual public Container
    {
    public:
        virtual void set_transient_for(Window* pParent) = 0;
        virtual void set_title(const OUString& rTitle) = 0;
        virtual OUString get_title() const = 0;
    };

    class VCL_DLLPUBLIC Dialog : virtual public Window
    {
    public:
        virtual int run() = 0;
        virtual void response(int response) = 0;
    };

    class VCL_DLLPUBLIC ComboBoxText : virtual public Container
    {
    private:
        OUString m_sSavedValue;
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
        virtual OUString get_active_id() const = 0;
        virtual void set_active_id(const OUString& rStr) = 0;
        virtual OUString get_text(int pos) const = 0;
        virtual OUString get_id(int pos) const = 0;
        virtual void append_text(const OUString& rStr) = 0;
        virtual void insert_text(int pos, const OUString& rStr) = 0;
        virtual void append(const OUString& rId, const OUString& rStr) = 0;
        virtual void insert(int pos, const OUString& rId, const OUString& rStr) = 0;
        virtual int find_text(const OUString& rStr) const = 0;
        virtual int get_count() const = 0;
        virtual void make_sorted() = 0;
        virtual void clear() = 0;

        void connect_changed(const Link<ComboBoxText&, void>& rLink)
        {
            m_aChangeHdl = rLink;
        }

        void set_active(const OUString& rStr)
        {
            set_active(find_text(rStr));
        }

        void save_value()
        {
            m_sSavedValue = get_active_text();
        }

        bool get_value_changed_from_saved() const
        {
            return m_sSavedValue != get_active_text();
        }
    };

    class VCL_DLLPUBLIC TreeView : virtual public Container
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

    class VCL_DLLPUBLIC Button : virtual public Container
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

        TriState get_state() const
        {
            if (get_inconsistent())
                return TRISTATE_INDET;
            else if (get_active())
                return TRISTATE_TRUE;
            return TRISTATE_FALSE;
        }

        void save_state()
        {
            m_eSavedValue = get_state();
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

        bool get_state_changed_from_saved() const
        {
            return m_eSavedValue != get_state();
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
    private:
        OUString m_sSavedValue;
    protected:
        Link<Entry&, void> m_aChangeHdl;
        Link<OUString&, bool> m_aInsertTextHdl;

        void signal_changed()
        {
            m_aChangeHdl.Call(*this);
        }

        void signal_insert_text(OUString &rString);
    public:
        virtual void set_text(const OUString& rText) = 0;
        virtual OUString get_text() const = 0;
        virtual void set_width_chars(int nChars) = 0;

        void connect_changed(const Link<Entry&, void>& rLink)
        {
            m_aChangeHdl = rLink;
        }

        void connect_insert_text(const Link<OUString&, bool>& rLink)
        {
            m_aInsertTextHdl = rLink;
        }

        void save_value()
        {
            m_sSavedValue = get_text();
        }

        bool get_value_changed_from_saved() const
        {
            return m_sSavedValue != get_text();
        }
    };

    class VCL_DLLPUBLIC SpinButton : virtual public Entry
    {
    protected:
        Link<SpinButton&, void> m_aValueChangedHdl;
        Link<SpinButton&, void> m_aOutputHdl;

        void signal_value_changed()
        {
            m_aValueChangedHdl.Call(*this);
        }

        bool signal_output()
        {
            if (!m_aOutputHdl.IsSet())
                return false;
            m_aOutputHdl.Call(*this);
            return true;
        }

    public:
        virtual void set_value(int value) = 0;
        virtual int get_value() const = 0;
        virtual void set_range(int min, int max) = 0;
        virtual void get_range(int& min, int& max) const = 0;
        virtual void set_increments(int step, int page) = 0;
        virtual void get_increments(int& step, int& page) const = 0;
        virtual void set_digits(unsigned int digits) = 0;
        virtual unsigned int get_digits() const = 0;

        void connect_value_changed(const Link<SpinButton&, void>& rLink)
        {
            m_aValueChangedHdl = rLink;
        }

        void connect_output(const Link<SpinButton&, void>& rLink)
        {
            m_aOutputHdl = rLink;
        }

        int normalize(int nValue) const
        {
            return (nValue * Power10(get_digits()));
        }

        int denormalize(int nValue) const;

        static unsigned int Power10(unsigned int n);
    };

    class VCL_DLLPUBLIC MetricSpinButton
    {
    protected:
        FieldUnit m_eSrcUnit;
        std::unique_ptr<Weld::SpinButton> m_xSpinButton;
        Link<MetricSpinButton&, void> m_aValueChangedHdl;

        DECL_LINK(spin_button_value_changed, Weld::SpinButton&, void);
        DECL_LINK(spin_button_output, Weld::SpinButton&, void);

        void signal_value_changed()
        {
            m_aValueChangedHdl.Call(*this);
        }

        int ConvertValue(int nValue, FieldUnit eInUnit, FieldUnit eOutUnit) const;
        OUString format_number(int nValue) const;
        void update_width_chars();
    public:
        MetricSpinButton(SpinButton* pSpinButton)
            : m_eSrcUnit(FUNIT_CM)
            , m_xSpinButton(pSpinButton)
        {
            update_width_chars();
            m_xSpinButton->connect_output(LINK(this, MetricSpinButton, spin_button_output));
            m_xSpinButton->connect_value_changed(LINK(this, MetricSpinButton, spin_button_value_changed));
        }

        FieldUnit get_unit() const
        {
            return m_eSrcUnit;
        }

        void set_unit(FieldUnit eUnit)
        {
            m_eSrcUnit = eUnit;
            update_width_chars();
        }

        void set_value(int nValue, FieldUnit eValueUnit)
        {
            m_xSpinButton->set_value(ConvertValue(nValue, eValueUnit, m_eSrcUnit));
        }

        int get_value(FieldUnit eDestUnit) const
        {
            int nValue = m_xSpinButton->get_value();
            return ConvertValue(nValue, m_eSrcUnit, eDestUnit);
        }

        void set_range(int min, int max, FieldUnit eValueUnit)
        {
            min = ConvertValue(min, eValueUnit, m_eSrcUnit);
            max = ConvertValue(max, eValueUnit, m_eSrcUnit);
            m_xSpinButton->set_range(min, max);
            update_width_chars();
        }

        void get_range(int& min, int& max, FieldUnit eDestUnit) const
        {
            m_xSpinButton->get_range(min, max);
            min = ConvertValue(min, m_eSrcUnit, eDestUnit);
            max = ConvertValue(max, m_eSrcUnit, eDestUnit);
        }

        void set_increments(int step, int page, FieldUnit eValueUnit)
        {
            step = ConvertValue(step, eValueUnit, m_eSrcUnit);
            page = ConvertValue(page, eValueUnit, m_eSrcUnit);
            m_xSpinButton->set_increments(step, page);
        }

        void get_increments(int& step, int& page, FieldUnit eDestUnit) const
        {
            m_xSpinButton->get_increments(step, page);
            step = ConvertValue(step, m_eSrcUnit, eDestUnit);
            page = ConvertValue(page, m_eSrcUnit, eDestUnit);
        }

        void connect_value_changed(const Link<MetricSpinButton&, void>& rLink)
        {
            m_aValueChangedHdl = rLink;
        }

        int normalize(int nValue) const { return m_xSpinButton->normalize(nValue); }
        int denormalize(int nValue) const { return m_xSpinButton->denormalize(nValue); }
        void set_sensitive(bool sensitive) { m_xSpinButton->set_sensitive(sensitive); }
        bool get_sensitive() const { return m_xSpinButton->get_sensitive(); }
        bool get_visible() const { return m_xSpinButton->get_visible(); }
        void grab_focus() { m_xSpinButton->grab_focus(); }
        bool has_focus() const { return m_xSpinButton->has_focus(); }
        void show() { m_xSpinButton->show(); }
        void hide() {m_xSpinButton->hide(); }
        void set_digits(unsigned int digits) { m_xSpinButton->set_digits(digits); }
        unsigned int get_digits() const { return m_xSpinButton->get_digits(); }
        void save_value() { m_xSpinButton->save_value(); }
        bool get_value_changed_from_saved() const { return m_xSpinButton->get_value_changed_from_saved(); }
        void set_text(const OUString& rText) { m_xSpinButton->set_text(rText); }
        OUString get_text() const { return m_xSpinButton->get_text(); }
        void set_size_request(int nWidth, int nHeight) { m_xSpinButton->set_size_request(nWidth, nHeight); }
        Size get_preferred_size() const { return m_xSpinButton->get_preferred_size(); }
    };

    class VCL_DLLPUBLIC Label : virtual public Widget
    {
    public:
        virtual void set_label(const OUString& rText) = 0;
    };

    class VCL_DLLPUBLIC TextView : virtual public Container
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
    private:
        OString m_sHelpRoot;
    public:
        Builder(const OUString& rUIFile)
            : m_sHelpRoot(OUStringToOString(rUIFile, RTL_TEXTENCODING_UTF8))
        {
            sal_Int32 nIdx = m_sHelpRoot.lastIndexOf('.');
            if (nIdx != -1)
                m_sHelpRoot = m_sHelpRoot.copy(0, nIdx);
            m_sHelpRoot = m_sHelpRoot + OString('/');
        }
        virtual Dialog* weld_dialog(const OString &id) = 0;
        virtual Window* weld_window(const OString &id) = 0;
        virtual Widget* weld_widget(const OString &id) = 0;
        virtual Container* weld_container(const OString &id) = 0;
        virtual Button* weld_button(const OString &id) = 0;
        virtual Frame* weld_frame(const OString &id) = 0;
        virtual Notebook* weld_notebook(const OString &id) = 0;
        virtual RadioButton* weld_radio_button(const OString &id) = 0;
        virtual CheckButton* weld_check_button(const OString &id) = 0;
        virtual SpinButton* weld_spin_button(const OString &id) = 0;
        MetricSpinButton* weld_metric_spin_button(const OString &id)
        {
            return new MetricSpinButton(weld_spin_button(id));
        }
        virtual ComboBoxText* weld_combo_box_text(const OString &id) = 0;
        virtual TreeView* weld_tree_view(const OString &id) = 0;
        virtual Label* weld_label(const OString &id) = 0;
        virtual TextView* weld_text_view(const OString &id) = 0;
        virtual Entry* weld_entry(const OString &id) = 0;
        virtual DrawingArea* weld_drawing_area(const OString &id) = 0;
        OString get_help_id(const Widget& rWidget) const
        {
            return m_sHelpRoot + rWidget.get_buildable_name();
        }
        virtual ~Builder() {}
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
