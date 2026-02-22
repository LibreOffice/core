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
#include <vcl/weld/Entry.hxx>

#include <optional>

namespace weld
{
class VCL_DLLPUBLIC SpinButton : virtual public Entry
{
    friend class ::LOKTrigger;

    Link<SpinButton&, void> m_aValueChangedHdl;
    Link<sal_Int64, OUString> m_aFormatValueHdl;
    Link<const OUString&, std::optional<int>> m_aParseTextHdl;

    // helper methods to convert between sal_Int64 value and
    // floating point number it represents (depending on get_digits())
    double convert_value_to_double(sal_Int64 nValue) const;
    sal_Int64 convert_double_to_value(double fDouble) const;

protected:
    void signal_value_changed()
    {
        if (notify_events_disabled())
            return;
        m_aValueChangedHdl.Call(*this);
    }

    /** If a custom value formatter was set via <a>set_value_formatter</a>,
     *  that one gets called to create a text representation of the value
     *  and that one gets returned.
     *  Otherwise, an empty std::optional is returned.
     */
    std::optional<OUString> format_floating_point_value(double fValue);

    /** If a custom text parser (which parses a value from the given text)
     *  is set and the text can be parsed, this method sets that value
     *  in <a>result</a> and returns <a>TRISTATE_TRUE</a>.
     *  Returns <a>TRISTATE_FALSE</a> if a custom handler is set, but the text
     *  cannot be parsed.
     *  Returns <a>TRISTATE_INDET</a> if no custom input handler is set.
     */
    TriState parse_text(const OUString& rText, double* pResult);

    // methods to implement in subclasses which use floating point values directly;
    // public methods using sal_Int64 values whose floating point value depends on get_digits()
    // take care of conversion
    virtual void set_floating_point_value(double fValue) = 0;
    virtual double get_floating_point_value() const = 0;
    virtual void set_floating_point_range(double fMin, double fMax) = 0;
    virtual void get_floating_point_range(double& rMin, double& rMax) const = 0;
    virtual void set_floating_point_increments(double fStep, double fPage) = 0;
    virtual void get_floating_point_increments(double& rStep, double& rPage) const = 0;

public:
    void set_value(sal_Int64 value);
    sal_Int64 get_value() const { return convert_double_to_value(get_floating_point_value()); }

    void set_range(sal_Int64 min, sal_Int64 max);
    void get_range(sal_Int64& min, sal_Int64& max) const;

    void set_min(sal_Int64 min);
    void set_max(sal_Int64 max);
    sal_Int64 get_min() const;
    sal_Int64 get_max() const;

    void set_increments(sal_Int64 step, sal_Int64 page);
    void get_increments(sal_Int64& step, sal_Int64& page) const;

    virtual void set_digits(unsigned int digits) = 0;
    virtual unsigned int get_digits() const = 0;

    void connect_value_changed(const Link<SpinButton&, void>& rLink) { m_aValueChangedHdl = rLink; }

    /** Set a value formatter that receives the value as a parameter and returns the
     *  text representation to display in the SpinButton.
     */
    void set_value_formatter(const Link<sal_Int64, OUString>& rLink) { m_aFormatValueHdl = rLink; }

    /** Set a parser that receives the text as a parameter and returns the value
     *  parsed from the text, or an empty std::optional if a value cannot be
     *  parsed from the text.
     */
    void set_text_parser(const Link<const OUString&, std::optional<int>>& rLink)
    {
        m_aParseTextHdl = rLink;
    }

    sal_Int64 normalize(sal_Int64 nValue) const { return (nValue * Power10(get_digits())); }

    sal_Int64 denormalize(sal_Int64 nValue) const;

    static unsigned int Power10(unsigned int n);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
