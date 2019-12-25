/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESSIBILITYISSUE_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESSIBILITYISSUE_HXX

#include <svx/AccessibilityIssue.hxx>
#include <doc.hxx>

namespace sw
{
enum class IssueObject
{
    UNKNOWN,
    GRAPHIC,
    OLE,
};

class SW_DLLPUBLIC AccessibilityIssue final : public svx::AccessibilityIssue
{
private:
    IssueObject m_eIssueObject;
    SwDoc* m_pDoc;
    OUString m_sObjectID;

public:
    AccessibilityIssue(svx::AccessibilityIssueID eIssueID = svx::AccessibilityIssueID::UNSPECIFIED);

    void setIssueObject(IssueObject eIssueObject);
    void setDoc(SwDoc* pDoc);
    void setObjectID(OUString const& rID);

    bool canGotoIssue() const override;
    void gotoIssue() const override;
};

} // end sw namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
