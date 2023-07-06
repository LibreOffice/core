/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <AccessibilityIssue.hxx>
#include <AccessibilityCheckStrings.hrc>
#include <drawdoc.hxx>
#include <edtwin.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <OnlineAccessibilityCheck.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <view.hxx>
#include <comphelper/lok.hxx>
#include <cui/dlgname.hxx>
#include <svx/svdpage.hxx>

namespace sw
{
AccessibilityIssue::AccessibilityIssue(sfx::AccessibilityIssueID eIssueID)
    : sfx::AccessibilityIssue(eIssueID)
    , m_eIssueObject(IssueObject::UNKNOWN)
    , m_pDoc(nullptr)
    , m_pNode(nullptr)
    , m_pTextFootnote(nullptr)
    , m_nStart(0)
    , m_nEnd(0)
{
}

void AccessibilityIssue::setIssueObject(IssueObject eIssueObject) { m_eIssueObject = eIssueObject; }

void AccessibilityIssue::setDoc(SwDoc& rDoc) { m_pDoc = &rDoc; }

void AccessibilityIssue::setObjectID(OUString const& rID) { m_sObjectID = rID; }

bool AccessibilityIssue::canGotoIssue() const
{
    if (m_pDoc && m_eIssueObject != IssueObject::UNKNOWN
        && m_eIssueObject != IssueObject::DOCUMENT_TITLE)
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
        case IssueObject::TEXTFRAME:
        {
            SwWrtShell* pWrtShell = m_pDoc->GetDocShell()->GetWrtShell();
            bool bSelected = pWrtShell->GotoFly(m_sObjectID, FLYCNTTYPE_ALL, true);
            if (bSelected && pWrtShell->IsFrameSelected())
            {
                pWrtShell->HideCursor();
                pWrtShell->EnterSelFrameMode();
            }
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
        case IssueObject::FORM:
        {
            SwWrtShell* pWrtShell = m_pDoc->GetDocShell()->GetWrtShell();
            bool bIsDesignMode = pWrtShell->GetView().GetFormShell()->IsDesignMode();
            if (bIsDesignMode || (!bIsDesignMode && pWrtShell->WarnSwitchToDesignModeDialog()))
            {
                if (!bIsDesignMode)
                    pWrtShell->GetView().GetFormShell()->SetDesignMode(true);
                pWrtShell->GotoDrawingObject(m_sObjectID);
                if (comphelper::LibreOfficeKit::isActive())
                    pWrtShell->ShowCursor();
            }
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
        case IssueObject::FOOTNOTE:
        {
            SwWrtShell* pWrtShell = m_pDoc->GetDocShell()->GetWrtShell();
            if (m_pTextFootnote)
                pWrtShell->GotoFootnoteAnchor(*m_pTextFootnote);
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
    return m_eIssueObject == IssueObject::GRAPHIC || m_eIssueObject == IssueObject::OLE
           || m_eIssueObject == IssueObject::SHAPE || m_eIssueObject == IssueObject::FORM
           || m_eIssueObject == IssueObject::DOCUMENT_TITLE;
}

void AccessibilityIssue::quickFixIssue() const
{
    if (!m_pDoc)
        return;

    if (m_eIssueObject != IssueObject::UNKNOWN)
        gotoIssue();

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
        case IssueObject::SHAPE:
        case IssueObject::FORM:
        {
            OUString aDesc = SwResId(STR_ENTER_ALT);
            SvxNameDialog aNameDialog(m_pParent, "", aDesc);
            if (aNameDialog.run() == RET_OK)
            {
                SwWrtShell* pWrtShell = m_pDoc->GetDocShell()->GetWrtShell();
                auto pPage = pWrtShell->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
                SdrObject* pObj = pPage->GetObjByName(m_sObjectID);
                if (pObj)
                    pObj->SetTitle(aNameDialog.GetName());
            }
        }
        break;
        case IssueObject::DOCUMENT_TITLE:
        {
            OUString aDesc = SwResId(STR_ENTER_DOCUMENT_TITLE);
            SvxNameDialog aNameDialog(m_pParent, "", aDesc);
            if (aNameDialog.run() == RET_OK)
            {
                SwDocShell* pShell = m_pDoc->GetDocShell();
                if (!pShell)
                    return;

                const uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                    pShell->GetModel(), uno::UNO_QUERY_THROW);
                const uno::Reference<document::XDocumentProperties> xDocumentProperties(
                    xDPS->getDocumentProperties());
                xDocumentProperties->setTitle(aNameDialog.GetName());

                m_pDoc->getOnlineAccessibilityCheck()->resetAndQueueDocumentLevel();
            }
        }
        break;
        default:
            break;
    }
    if (m_pNode)
        m_pDoc->getOnlineAccessibilityCheck()->resetAndQueue(m_pNode);
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
