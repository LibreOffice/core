/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sfx2/dllapi.h>

#include <vcl/weld.hxx>
#include <rtl/ustring.hxx>

#include <vector>
#include <memory>

namespace sfx
{
enum class AccessibilityIssueID
{
    UNSPECIFIED, // TODO: remove - temporary
    DOCUMENT_TITLE,
    DOCUMENT_LANGUAGE,
    DOCUMENT_BACKGROUND,
    STYLE_LANGUAGE,
    LINKED_GRAPHIC,
    NO_ALT_OLE,
    NO_ALT_GRAPHIC,
    NO_ALT_SHAPE,
    TABLE_MERGE_SPLIT,
    TEXT_FORMATTING,
    DIRECT_FORMATTING,
    TABLE_FORMATTING,
    HYPERLINK_IS_TEXT,
    HYPERLINK_SHORT,
    FAKE_FOOTNOTE,
    FAKE_CAPTION,
    MANUAL_NUMBERING
};

class SFX2_DLLPUBLIC AccessibilityIssue
{
public:
    AccessibilityIssue(AccessibilityIssueID eIssueID = AccessibilityIssueID::UNSPECIFIED);
    AccessibilityIssue(AccessibilityIssue const&) = default;
    virtual ~AccessibilityIssue();

    virtual bool canGotoIssue() const = 0;
    virtual void gotoIssue() const = 0;

    virtual bool canQuickFixIssue() const = 0;
    virtual void quickFixIssue() const = 0;

    void setParent(weld::Window* pParent) { m_pParent = pParent; }

    AccessibilityIssue& operator=(const AccessibilityIssue&) = default;

    AccessibilityIssueID m_eIssueID;
    OUString m_aIssueText;

    weld::Window* m_pParent;
};

class SFX2_DLLPUBLIC AccessibilityIssueCollection
{
private:
    std::vector<std::shared_ptr<AccessibilityIssue>> m_aIssues;

public:
    AccessibilityIssueCollection() = default;

    std::vector<std::shared_ptr<AccessibilityIssue>>& getIssues();

    void clear();
};

} // end sfx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
