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
#include <vcl/weld/EntryFormatter.hxx>

#include <optional>

namespace weld
{
class Entry;
class FormattedSpinButton;

class VCL_DLLPUBLIC LongCurrencyFormatter final : public EntryFormatter
{
public:
    LongCurrencyFormatter(weld::Entry& rEntry);
    LongCurrencyFormatter(weld::FormattedSpinButton& rSpinButton);

    void SetUseThousandSep(bool b);
    void SetCurrencySymbol(const OUString& rStr);

    SAL_DLLPRIVATE virtual ~LongCurrencyFormatter() override;

private:
    DECL_DLLPRIVATE_LINK(FormatOutputHdl, double, std::optional<OUString>);
    DECL_DLLPRIVATE_LINK(ParseInputHdl, const OUString&, Formatter::ParseResult);

    SAL_DLLPRIVATE void Init();

    OUString m_aCurrencySymbol;
    bool m_bThousandSep;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
