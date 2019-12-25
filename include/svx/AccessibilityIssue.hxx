/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SVX_ACCESSIBILITYISSUE_HXX
#define INCLUDED_SVX_ACCESSIBILITYISSUE_HXX

#include <svx/svxdllapi.h>
#include <rtl/ustring.hxx>
#include <vector>
#include <memory>

namespace svx
{
enum class AccessibilityIssueID
{
    UNSPECIFIED, // TODO: remove - temporary
    DOCUMENT_TITLE,
    DOCUMENT_LANGUAGE,
    STYLE_LANGUAGE
};

class SVX_DLLPUBLIC AccessibilityIssue
{
public:
    AccessibilityIssue(AccessibilityIssueID eIssueID = AccessibilityIssueID::UNSPECIFIED)
        : m_eIssueID(eIssueID)
    {
    }
    virtual ~AccessibilityIssue() {}
    virtual bool canGotoIssue() const = 0;
    virtual void gotoIssue() const = 0;

    AccessibilityIssueID m_eIssueID;
    OUString m_aIssueText;
};

class SVX_DLLPUBLIC AccessibilityIssueCollection
{
private:
    std::vector<std::shared_ptr<AccessibilityIssue>> m_aIssues;

public:
    std::vector<std::shared_ptr<AccessibilityIssue>>& getIssues() { return m_aIssues; }
};

} // end svx namespace

#endif // INCLUDED_SVX_ACCESSIBILITYISSUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
