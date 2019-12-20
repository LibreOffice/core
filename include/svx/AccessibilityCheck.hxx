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

namespace svx
{
enum class AccessibilityIssueID
{
    UNSPECIFIED, // TODO: remove - temporary
    DOCUMENT_TITLE,
    DOCUMENT_LANGUAGE,
    STYLE_LANGUAGE
};

class SVX_DLLPUBLIC AccessibilityCheckResult
{
public:
    AccessibilityIssueID m_eIssueID;
    OUString m_aIssueText;

    AccessibilityCheckResult(AccessibilityIssueID eIssueID = AccessibilityIssueID::UNSPECIFIED)
        : m_eIssueID(eIssueID)
    {
    }
    virtual ~AccessibilityCheckResult() {}
    virtual void gotoIssue() const = 0;
};

class SVX_DLLPUBLIC AccessibilityCheckResultCollection
{
private:
    std::vector<std::shared_ptr<AccessibilityCheckResult>> m_aResults;

public:
    std::vector<std::shared_ptr<AccessibilityCheckResult>>& getResults() { return m_aResults; }
};

class SVX_DLLPUBLIC AccessibilityCheck
{
protected:
    AccessibilityCheckResultCollection m_aResultCollection;

public:
    virtual ~AccessibilityCheck() {}

    virtual void check() = 0;

    AccessibilityCheckResultCollection& getResultCollecton() { return m_aResultCollection; }
};

} // end svx namespace

#endif // INCLUDED_SVX_ACCESSIBILITYCHECKDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
