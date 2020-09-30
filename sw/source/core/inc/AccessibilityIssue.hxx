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

#include <sfx2/AccessibilityIssue.hxx>
#include <doc.hxx>

namespace sw
{
enum class IssueObject
{
    UNKNOWN,
    GRAPHIC,
    OLE,
    TABLE,
    TEXT,
};

class AccessibilityIssue final : public sfx::AccessibilityIssue
{
private:
    IssueObject m_eIssueObject;
    SwDoc* m_pDoc;
    OUString m_sObjectID;
    std::vector<OUString> m_aIssueAdditionalInfo;
    SwNode* m_pNode;

    sal_Int32 m_nStart;
    sal_Int32 m_nEnd;

public:
    AccessibilityIssue(sfx::AccessibilityIssueID eIssueID = sfx::AccessibilityIssueID::UNSPECIFIED);

    void setIssueObject(IssueObject eIssueObject);
    void setDoc(SwDoc& rDoc);
    void setObjectID(OUString const& rID);
    void setNode(SwNode* pNode) { m_pNode = pNode; }

    void setStart(sal_Int32 nStart) { m_nStart = nStart; }

    void setEnd(sal_Int32 nEnd) { m_nEnd = nEnd; }

    std::vector<OUString> const& getAdditionalInfo() const { return m_aIssueAdditionalInfo; }

    void setAdditionalInfo(std::vector<OUString> const& rIssueAdditionalInfo)
    {
        m_aIssueAdditionalInfo = rIssueAdditionalInfo;
    }

    bool canGotoIssue() const override;
    void gotoIssue() const override;
};

} // end sw namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
