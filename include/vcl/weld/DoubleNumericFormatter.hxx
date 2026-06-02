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

namespace weld
{
class Entry;
class FormattedSpinButton;

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) DoubleNumericFormatter final : public EntryFormatter
{
public:
    DoubleNumericFormatter(weld::Entry& rEntry);
    DoubleNumericFormatter(weld::FormattedSpinButton& rSpinButton);

    SAL_DLLPRIVATE virtual ~DoubleNumericFormatter() override;

private:
    SAL_DLLPRIVATE virtual bool CheckText(const OUString& sText) const override;

    SAL_DLLPRIVATE virtual void FormatChanged(FORMAT_CHANGE_TYPE nWhat) override;
    SAL_DLLPRIVATE void ResetConformanceTester();

    std::unique_ptr<validation::NumberValidator> m_pNumberValidator;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
