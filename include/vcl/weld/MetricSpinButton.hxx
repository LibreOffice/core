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
#include <vcl/weld/SpinButton.hxx>

enum class FieldUnit : sal_uInt16;

namespace weld
{
class VCL_DLLPUBLIC MetricSpinButton final
{
    FieldUnit m_eSrcUnit;
    std::unique_ptr<weld::SpinButton> m_xSpinButton;
    Link<MetricSpinButton&, void> m_aValueChangedHdl;

    DECL_LINK(spin_button_value_changed, weld::SpinButton&, void);
    DECL_LINK(spin_button_output, sal_Int64, OUString);
    DECL_LINK(spin_button_input, const OUString&, std::optional<int>);

    void signal_value_changed() { m_aValueChangedHdl.Call(*this); }

    sal_Int64 ConvertValue(sal_Int64 nValue, FieldUnit eInUnit, FieldUnit eOutUnit) const;
    OUString format_number(sal_Int64 nValue) const;
    void update_width_chars();

public:
    MetricSpinButton(std::unique_ptr<SpinButton> pSpinButton, FieldUnit eSrcUnit)
        : m_eSrcUnit(eSrcUnit)
        , m_xSpinButton(std::move(pSpinButton))
    {
        update_width_chars();
        m_xSpinButton->set_value_formatter(LINK(this, MetricSpinButton, spin_button_output));
        m_xSpinButton->set_text_parser(LINK(this, MetricSpinButton, spin_button_input));
        m_xSpinButton->connect_value_changed(
            LINK(this, MetricSpinButton, spin_button_value_changed));
        m_xSpinButton->set_text(format_number(m_xSpinButton->get_value()));
    }

    static OUString MetricToString(FieldUnit rUnit);

    FieldUnit get_unit() const { return m_eSrcUnit; }

    void set_unit(FieldUnit eUnit);

    sal_Int64 convert_value_to(sal_Int64 nValue, FieldUnit eValueUnit) const
    {
        return ConvertValue(nValue, m_eSrcUnit, eValueUnit);
    }

    sal_Int64 convert_value_from(sal_Int64 nValue, FieldUnit eValueUnit) const
    {
        return ConvertValue(nValue, eValueUnit, m_eSrcUnit);
    }

    void set_value(sal_Int64 nValue, FieldUnit eValueUnit)
    {
        m_xSpinButton->set_value(convert_value_from(nValue, eValueUnit));
    }

    sal_Int64 get_value(FieldUnit eDestUnit) const
    {
        return convert_value_to(m_xSpinButton->get_value(), eDestUnit);
    }

    // typically you only need to call this if set_text (e.g. with "") was
    // previously called to display some arbitrary text instead of the
    // formatted value and now you want to show it as formatted again
    void reformat()
    {
        const OUString sText = format_number(m_xSpinButton->get_value());
        m_xSpinButton->set_text(sText);
    }

    void set_range(sal_Int64 min, sal_Int64 max, FieldUnit eValueUnit)
    {
        min = convert_value_from(min, eValueUnit);
        max = convert_value_from(max, eValueUnit);
        m_xSpinButton->set_range(min, max);
        update_width_chars();
    }

    void get_range(sal_Int64& min, sal_Int64& max, FieldUnit eDestUnit) const
    {
        m_xSpinButton->get_range(min, max);
        min = convert_value_to(min, eDestUnit);
        max = convert_value_to(max, eDestUnit);
    }

    void set_min(sal_Int64 min, FieldUnit eValueUnit)
    {
        sal_Int64 dummy, max;
        get_range(dummy, max, eValueUnit);
        set_range(min, max, eValueUnit);
    }

    void set_max(sal_Int64 max, FieldUnit eValueUnit)
    {
        sal_Int64 min, dummy;
        get_range(min, dummy, eValueUnit);
        set_range(min, max, eValueUnit);
    }

    sal_Int64 get_min(FieldUnit eValueUnit) const
    {
        sal_Int64 min, dummy;
        get_range(min, dummy, eValueUnit);
        return min;
    }

    sal_Int64 get_max(FieldUnit eValueUnit) const
    {
        sal_Int64 dummy, max;
        get_range(dummy, max, eValueUnit);
        return max;
    }

    void set_increments(sal_Int64 step, sal_Int64 page, FieldUnit eValueUnit)
    {
        step = convert_value_from(step, eValueUnit);
        page = convert_value_from(page, eValueUnit);
        m_xSpinButton->set_increments(step, page);
    }

    void get_increments(sal_Int64& step, sal_Int64& page, FieldUnit eDestUnit) const
    {
        m_xSpinButton->get_increments(step, page);
        step = convert_value_to(step, eDestUnit);
        page = convert_value_to(page, eDestUnit);
    }

    void connect_value_changed(const Link<MetricSpinButton&, void>& rLink)
    {
        m_aValueChangedHdl = rLink;
    }

    sal_Int64 normalize(sal_Int64 nValue) const { return m_xSpinButton->normalize(nValue); }
    sal_Int64 denormalize(sal_Int64 nValue) const { return m_xSpinButton->denormalize(nValue); }
    void set_sensitive(bool sensitive) { m_xSpinButton->set_sensitive(sensitive); }
    bool get_sensitive() const { return m_xSpinButton->get_sensitive(); }
    bool get_visible() const { return m_xSpinButton->get_visible(); }
    void grab_focus() { m_xSpinButton->grab_focus(); }
    bool has_focus() const { return m_xSpinButton->has_focus(); }
    void show() { m_xSpinButton->show(); }
    void set_visible(bool bShow) { m_xSpinButton->set_visible(bShow); }
    void hide() { m_xSpinButton->hide(); }
    void set_digits(unsigned int digits);
    void set_accessible_name(const OUString& rName) { m_xSpinButton->set_accessible_name(rName); }
    unsigned int get_digits() const { return m_xSpinButton->get_digits(); }
    void save_value() { m_xSpinButton->save_value(); }
    bool get_value_changed_from_saved() const
    {
        return m_xSpinButton->get_value_changed_from_saved();
    }
    void set_text(const OUString& rText) { m_xSpinButton->set_text(rText); }
    OUString get_text() const { return m_xSpinButton->get_text(); }
    void set_size_request(int nWidth, int nHeight)
    {
        m_xSpinButton->set_size_request(nWidth, nHeight);
    }
    Size get_size_request() const { return m_xSpinButton->get_size_request(); }
    Size get_preferred_size() const { return m_xSpinButton->get_preferred_size(); }
    void connect_focus_in(const Link<Widget&, void>& rLink)
    {
        m_xSpinButton->connect_focus_in(rLink);
    }
    void connect_focus_out(const Link<Widget&, void>& rLink)
    {
        m_xSpinButton->connect_focus_out(rLink);
    }
    OUString get_buildable_name() const { return m_xSpinButton->get_buildable_name(); }
    void set_help_id(const OUString& rName) { m_xSpinButton->set_help_id(rName); }
    void set_position(int nCursorPos) { m_xSpinButton->set_position(nCursorPos); }
    // set the width of the underlying widget in characters, this setting is
    // invalidated when changing the units, range or digits, so to have effect
    // must come after changing those values
    void set_width_chars(int nChars) { m_xSpinButton->set_width_chars(nChars); }
    int get_width_chars() const { return m_xSpinButton->get_width_chars(); }
    weld::SpinButton& get_widget() { return *m_xSpinButton; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
