/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SFX2_ACCESSIBILITYISSUE_HXX
#define INCLUDED_SFX2_ACCESSIBILITYISSUE_HXX

#include <sfx2/dllapi.h>
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
    STYLE_LANGUAGE,
    NO_ALT_OLE,
    NO_ALT_GRAPHIC,
    NO_ALT_SHAPE,
    TABLE_MERGE_SPLIT,
    TEXT_FORMATTING,
};

class SFX2_DLLPUBLIC AccessibilityIssue
{
public:
    AccessibilityIssue(AccessibilityIssueID eIssueID = AccessibilityIssueID::UNSPECIFIED);
    virtual ~AccessibilityIssue();

    virtual bool canGotoIssue() const = 0;
    virtual void gotoIssue() const = 0;

    AccessibilityIssueID m_eIssueID;
    OUString m_aIssueText;
};

class SFX2_DLLPUBLIC AccessibilityIssueCollection
{
private:
    std::vector<std::shared_ptr<AccessibilityIssue>> m_aIssues;

public:
    AccessibilityIssueCollection() = default;

    std::vector<std::shared_ptr<AccessibilityIssue>>& getIssues();
};

} // end sfx namespace

#endif // INCLUDED_SFX2_ACCESSIBILITYISSUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
