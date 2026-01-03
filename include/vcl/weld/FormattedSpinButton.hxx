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

namespace weld
{
class EntryFormatter;

// Similar to a SpinButton, but input and output formatting and range/value
// are managed by a more complex Formatter which can support doubles.
class VCL_DLLPUBLIC FormattedSpinButton : virtual public Entry
{
    friend class ::LOKTrigger;

    Link<FormattedSpinButton&, void> m_aValueChangedHdl;

protected:
    void signal_value_changed() { m_aValueChangedHdl.Call(*this); }

public:
    virtual Formatter& GetFormatter() = 0;
    // does not take ownership, and so must be deregistered if pFormatter
    // is destroyed
    virtual void SetFormatter(weld::EntryFormatter* pFormatter) = 0;

    void connect_value_changed(const Link<FormattedSpinButton&, void>& rLink)
    {
        m_aValueChangedHdl = rLink;
    }

private:
    friend class EntryFormatter;
    virtual void sync_range_from_formatter() = 0;
    virtual void sync_value_from_formatter() = 0;
    virtual void sync_increments_from_formatter() = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
