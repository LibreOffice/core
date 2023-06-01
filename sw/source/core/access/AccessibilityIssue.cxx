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
#include <AccessibilityCheckStrings.hrc>
#include <edtwin.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <view.hxx>
#include <comphelper/lok.hxx>
#include <cui/dlgname.hxx>

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
    if (m_pDoc && m_eIssueObject != IssueObject::UNKNOWN)
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
            if (comphelper::LibreOfficeKit::isActive())
                pWrtShell->ShowCursor();
        }
        break;
        case IssueObject::SHAPE:
        {
            SwWrtShell* pWrtShell = m_pDoc->GetDocShell()->GetWrtShell();
            pWrtShell->GotoDrawingObject(m_sObjectID);
            if (comphelper::LibreOfficeKit::isActive())
                pWrtShell->ShowCursor();
        }
        break;
        case IssueObject::TABLE:
        {
            SwWrtShell* pWrtShell = m_pDoc->GetDocShell()->GetWrtShell();
            pWrtShell->GotoTable(m_sObjectID);
            if (comphelper::LibreOfficeKit::isActive())
                pWrtShell->ShowCursor();
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
            if (comphelper::LibreOfficeKit::isActive())
                pWrtShell->ShowCursor();
        }
        break;
        default:
            break;
    }
    m_pDoc->GetDocShell()->GetView()->GetEditWin().GrabFocus();
}

bool AccessibilityIssue::canQuickFixIssue() const
{
    return m_eIssueObject == IssueObject::GRAPHIC || m_eIssueObject == IssueObject::OLE;
}

void AccessibilityIssue::quickFixIssue() const
{
    if (!m_pDoc)
        return;

    switch (m_eIssueObject)
    {
        case IssueObject::GRAPHIC:
        case IssueObject::OLE:
        {
            OUString aDesc = SwResId(STR_ENTER_ALT);
            SvxNameDialog aNameDialog(m_pParent, "", aDesc);
            if (aNameDialog.run() == RET_OK)
            {
                SwFlyFrameFormat* pFlyFormat
                    = const_cast<SwFlyFrameFormat*>(m_pDoc->FindFlyByName(m_sObjectID));
                if (pFlyFormat)
                    m_pDoc->SetFlyFrameTitle(*pFlyFormat, aNameDialog.GetName());
            }
        }
        break;
        default:
            break;
    }
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
