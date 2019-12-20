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

#include <svx/svxdllapi.h>
#include <vector>
#include <memory>
#include <rtl/ustring.hxx>
#include <svx/AccessibilityIssue.hxx>

namespace svx
{
class SVX_DLLPUBLIC AccessibilityCheck
{
protected:
    AccessibilityIssueCollection m_aIssueCollection;

public:
    virtual ~AccessibilityCheck() {}

    virtual void check() = 0;

    AccessibilityIssueCollection& getIssueCollecton() { return m_aIssueCollection; }
};

} // end svx namespace

#endif // INCLUDED_SVX_ACCESSIBILITYCHECKDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
