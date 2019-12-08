/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SVX_ACCESSIBILITYCHECK_HXX
#define INCLUDED_SVX_ACCESSIBILITYCHECK_HXX

#include <vector>
#include <rtl/ustring.hxx>
#include <svx/svxdllapi.h>

namespace svx
{
class SVX_DLLPUBLIC AccessibilityCheckResult final
{
public:
    OUString m_aIssueText;
};

class SVX_DLLPUBLIC AccessibilityCheck
{
protected:
    std::vector<AccessibilityCheckResult> m_aResultCollection;

public:
    virtual ~AccessibilityCheck() {}

    virtual void check() = 0;

    std::vector<AccessibilityCheckResult> const& getResultCollecton()
    {
        return m_aResultCollection;
    }
};

} // end svx namespace

#endif // INCLUDED_SVX_ACCESSIBILITYCHECKDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
