/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_HACKERY_HXX
#define INCLUDED_VCL_HACKERY_HXX

#include <rtl/ustring.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>

class SalFrame;

namespace Hackery
{
    class VCL_DLLPUBLIC Widget
    {
    public:
        virtual void set_sensitive(bool sensitive) = 0;
        virtual void grab_focus() = 0;
        virtual ~Widget() {}
    };

    class VCL_DLLPUBLIC Dialog : virtual public Widget
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
        virtual OUString get_active_text() const = 0;
        virtual OUString get_text(int pos) const = 0;
        virtual void insert_text(const OUString& rStr, int pos) = 0;
        virtual int find_text(const OUString& rStr) const = 0;
        virtual int get_count() const = 0;

        void connect_changed(const Link<ComboBoxText&, void>& rLink)
        {
            m_aChangeHdl= rLink;
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
        void connect_clicked(const Link<Button&, void>& rLink)
        {
            m_aClickHdl = rLink;
        }
    };

    class VCL_DLLPUBLIC CheckButton : virtual public Button
    {
    protected:
        Link<CheckButton&, void> m_aToggleHdl;

        void signal_toggled()
        {
            m_aToggleHdl.Call(*this);
        }
    public:
        virtual void set_active(bool active) = 0;
        virtual bool get_active() const = 0;

        void connect_toggled(const Link<CheckButton&, void>& rLink)
        {
            m_aToggleHdl = rLink;
        }
    };

    class VCL_DLLPUBLIC RadioButton : virtual public Button
    {
    protected:
        Link<RadioButton&, void> m_aToggleHdl;

        void signal_toggled()
        {
            m_aToggleHdl.Call(*this);
        }
    public:
        virtual void set_active(bool active) = 0;
        virtual bool get_active() const = 0;

        void connect_toggled(const Link<RadioButton&, void>& rLink)
        {
            m_aToggleHdl = rLink;
        }
    };

    class VCL_DLLPUBLIC SpinButton : virtual public Widget
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
        virtual void set_text(const OUString& rText) = 0;

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

    class VCL_DLLPUBLIC Builder
    {
    public:
        virtual Dialog* get_dialog(const OString &id) = 0;
        virtual Button* get_button(const OString &id) = 0;
        virtual RadioButton* get_radio_button(const OString &id) = 0;
        virtual CheckButton* get_check_button(const OString &id) = 0;
        virtual SpinButton* get_spin_button(const OString &id) = 0;
        virtual ComboBoxText* get_combo_box_text(const OString &id) = 0;
        virtual Label* get_label(const OString &id) = 0;
        virtual ~Builder() {}
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
