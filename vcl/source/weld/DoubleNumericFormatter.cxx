/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/numformat.hxx>
#include <svl/zformat.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/weld/DoubleNumericFormatter.hxx>

namespace weld
{
DoubleNumericFormatter::DoubleNumericFormatter(weld::Entry& rEntry)
    : EntryFormatter(rEntry)
{
    ResetConformanceTester();
}

DoubleNumericFormatter::DoubleNumericFormatter(weld::FormattedSpinButton& rSpinButton)
    : EntryFormatter(rSpinButton)
{
    ResetConformanceTester();
}

DoubleNumericFormatter::~DoubleNumericFormatter() = default;

void DoubleNumericFormatter::FormatChanged(FORMAT_CHANGE_TYPE nWhat)
{
    ResetConformanceTester();
    EntryFormatter::FormatChanged(nWhat);
}

bool DoubleNumericFormatter::CheckText(const OUString& sText) const
{
    // We'd like to implement this using the NumberFormatter::IsNumberFormat, but unfortunately, this doesn't
    // recognize fragments of numbers (like, for instance "1e", which happens during entering e.g. "1e10")
    // Thus, the roundabout way via a regular expression
    return m_pNumberValidator->isValidNumericFragment(sText);
}

void DoubleNumericFormatter::ResetConformanceTester()
{
    // the thousands and the decimal separator are language dependent
    const SvNumberformat* pFormatEntry = GetOrCreateFormatter().GetEntry(m_nFormatKey);

    sal_Unicode cSeparatorThousand = ',';
    sal_Unicode cSeparatorDecimal = '.';
    if (pFormatEntry)
    {
        LocaleDataWrapper aLocaleInfo(LanguageTag(pFormatEntry->GetLanguage()));

        OUString sSeparator = aLocaleInfo.getNumThousandSep();
        if (!sSeparator.isEmpty())
            cSeparatorThousand = sSeparator[0];

        sSeparator = aLocaleInfo.getNumDecimalSep();
        if (!sSeparator.isEmpty())
            cSeparatorDecimal = sSeparator[0];
    }

    m_pNumberValidator.reset(
        new validation::NumberValidator(cSeparatorThousand, cSeparatorDecimal));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
