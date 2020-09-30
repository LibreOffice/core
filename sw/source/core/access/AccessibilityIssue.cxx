/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <AccessibilityIssue.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>

namespace sw
{
AccessibilityIssue::AccessibilityIssue(sfx::AccessibilityIssueID eIssueID)
    : sfx::AccessibilityIssue(eIssueID)
    , m_eIssueObject(IssueObject::UNKNOWN)
    , m_pDoc(nullptr)
    , m_pNode(nullptr)
    , m_nStart(0)
    , m_nEnd(0)
{
}

void AccessibilityIssue::setIssueObject(IssueObject eIssueObject) { m_eIssueObject = eIssueObject; }

void AccessibilityIssue::setDoc(SwDoc& rDoc) { m_pDoc = &rDoc; }

void AccessibilityIssue::setObjectID(OUString const& rID) { m_sObjectID = rID; }

bool AccessibilityIssue::canGotoIssue() const
{
    if (m_eIssueObject != IssueObject::UNKNOWN)
        return true;
    return false;
}

void AccessibilityIssue::gotoIssue() const
{
    if (!m_pDoc)
        return;

    switch (m_eIssueObject)
    {
        case IssueObject::GRAPHIC:
        case IssueObject::OLE:
        {
            SwWrtShell* pWrtShell = m_pDoc->GetDocShell()->GetWrtShell();
            pWrtShell->GotoFly(m_sObjectID, FLYCNTTYPE_ALL, true);
        }
        break;
        case IssueObject::TABLE:
        {
            SwWrtShell* pWrtShell = m_pDoc->GetDocShell()->GetWrtShell();
            pWrtShell->GotoTable(m_sObjectID);
        }
        break;
        case IssueObject::TEXT:
        {
            SwWrtShell* pWrtShell = m_pDoc->GetDocShell()->GetWrtShell();
            SwContentNode* pContentNode = m_pNode->GetContentNode();
            SwPosition aPoint(*pContentNode, m_nStart);
            SwPosition aMark(*pContentNode, m_nEnd);
            pWrtShell->EnterStdMode();
            pWrtShell->StartAllAction();
            SwPaM* pPaM = pWrtShell->GetCursor();
            *pPaM->GetPoint() = aPoint;
            pPaM->SetMark();
            *pPaM->GetMark() = aMark;
            pWrtShell->EndAllAction();
        }
        break;
        default:
            break;
    }
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
