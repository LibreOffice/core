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

#include <comphelper/propertyvalue.hxx>
#include <comphelper/dispatchcommand.hxx>

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
#include <svx/svxdlg.hxx>

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
        && m_eIssueObject != IssueObject::DOCUMENT_TITLE
        && m_eIssueObject != IssueObject::DOCUMENT_BACKGROUND
        && m_eIssueObject != IssueObject::LANGUAGE_NOT_SET)
        return true;
    return false;
}

void AccessibilityIssue::gotoIssue() const
{
    if (!m_pDoc)
        return;

    /* Copying the issueobject because the EnterSelFrameMode ends up calling some sidebar functions
    that recreate the list of a11y issues and the AccessibilityIssue objects are stored by value in a vector
    and the vector is being mutated there and so the instance is overwritten with something else. */
    AccessibilityIssue TempObject(*this);

    switch (TempObject.m_eIssueObject)
    {
        case IssueObject::GRAPHIC:
        case IssueObject::OLE:
        case IssueObject::TEXTFRAME:
        {
            SwWrtShell* pWrtShell = TempObject.m_pDoc->GetDocShell()->GetWrtShell();
            bool bSelected = pWrtShell->GotoFly(TempObject.m_sObjectID, FLYCNTTYPE_ALL, true);
            if (bSelected && pWrtShell->IsFrameSelected())
            {
                pWrtShell->HideCursor();
                pWrtShell->EnterSelFrameMode();
            }

            if (!bSelected && TempObject.m_eIssueObject == IssueObject::TEXTFRAME)
                pWrtShell->GotoDrawingObject(TempObject.m_sObjectID);

            if (comphelper::LibreOfficeKit::isActive())
                pWrtShell->ShowCursor();
        }
        break;
        case IssueObject::SHAPE:
        {
            SwWrtShell* pWrtShell = TempObject.m_pDoc->GetDocShell()->GetWrtShell();
            pWrtShell->GotoDrawingObject(TempObject.m_sObjectID);
            if (comphelper::LibreOfficeKit::isActive())
                pWrtShell->ShowCursor();
        }
        break;
        case IssueObject::FORM:
        {
            SwWrtShell* pWrtShell = TempObject.m_pDoc->GetDocShell()->GetWrtShell();
            bool bIsDesignMode = pWrtShell->GetView().GetFormShell()->IsDesignMode();
            if (bIsDesignMode || (!bIsDesignMode && pWrtShell->WarnSwitchToDesignModeDialog()))
            {
                if (!bIsDesignMode)
                    pWrtShell->GetView().GetFormShell()->SetDesignMode(true);
                pWrtShell->GotoDrawingObject(TempObject.m_sObjectID);
                if (comphelper::LibreOfficeKit::isActive())
                    pWrtShell->ShowCursor();
            }
        }
        break;
        case IssueObject::TABLE:
        {
            SwWrtShell* pWrtShell = TempObject.m_pDoc->GetDocShell()->GetWrtShell();
            pWrtShell->GotoTable(TempObject.m_sObjectID);
            if (comphelper::LibreOfficeKit::isActive())
                pWrtShell->ShowCursor();
        }
        break;
        case IssueObject::TEXT:
        {
            SwWrtShell* pWrtShell = TempObject.m_pDoc->GetDocShell()->GetWrtShell();
            SwContentNode* pContentNode = TempObject.m_pNode->GetContentNode();
            SwPosition aPoint(*pContentNode, TempObject.m_nStart);
            SwPosition aMark(*pContentNode, TempObject.m_nEnd);
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
        case IssueObject::FOOTENDNOTE:
        {
            SwWrtShell* pWrtShell = TempObject.m_pDoc->GetDocShell()->GetWrtShell();
            if (TempObject.m_pTextFootnote)
                pWrtShell->GotoFootnoteAnchor(*TempObject.m_pTextFootnote);
            if (comphelper::LibreOfficeKit::isActive())
                pWrtShell->ShowCursor();
        }
        break;
        default:
            break;
    }
    TempObject.m_pDoc->GetDocShell()->GetView()->GetEditWin().GrabFocus();
}

bool AccessibilityIssue::canQuickFixIssue() const
{
    return m_eIssueObject == IssueObject::GRAPHIC || m_eIssueObject == IssueObject::OLE
           || m_eIssueObject == IssueObject::SHAPE || m_eIssueObject == IssueObject::FORM
           || m_eIssueObject == IssueObject::DOCUMENT_TITLE
           || m_eIssueObject == IssueObject::DOCUMENT_BACKGROUND
           || m_eIssueObject == IssueObject::LANGUAGE_NOT_SET;
}

void AccessibilityIssue::quickFixIssue() const
{
    if (!m_pDoc)
        return;

    if (canGotoIssue())
        gotoIssue();

    switch (m_eIssueObject)
    {
        case IssueObject::GRAPHIC:
        case IssueObject::OLE:
        {
            SwFlyFrameFormat* pFlyFormat
                = const_cast<SwFlyFrameFormat*>(m_pDoc->FindFlyByName(m_sObjectID));
            if (pFlyFormat)
            {
                OUString aDescription(pFlyFormat->GetObjDescription());
                OUString aTitle(pFlyFormat->GetObjTitle());
                bool isDecorative(pFlyFormat->IsDecorative());

                SwWrtShell* pWrtShell = m_pDoc->GetDocShell()->GetWrtShell();
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractSvxObjectTitleDescDialog> pDlg(
                    pFact->CreateSvxObjectTitleDescDialog(pWrtShell->GetView().GetFrameWeld(),
                                                          aTitle, aDescription, isDecorative));

                if (pDlg->Execute() == RET_OK)
                {
                    pDlg->GetTitle(aTitle);
                    pDlg->GetDescription(aDescription);
                    pDlg->IsDecorative(isDecorative);

                    m_pDoc->SetFlyFrameTitle(*pFlyFormat, aTitle);
                    m_pDoc->SetFlyFrameDescription(*pFlyFormat, aDescription);
                    m_pDoc->SetFlyFrameDecorative(*pFlyFormat, isDecorative);

                    pWrtShell->SetModified();
                }
            }
        }
        break;
        case IssueObject::SHAPE:
        case IssueObject::FORM:
        {
            SwWrtShell* pWrtShell = m_pDoc->GetDocShell()->GetWrtShell();
            auto pPage = pWrtShell->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
            SdrObject* pObj = pPage->GetObjByName(m_sObjectID);
            if (pObj)
            {
                OUString aTitle(pObj->GetTitle());
                OUString aDescription(pObj->GetDescription());
                bool isDecorative(pObj->IsDecorative());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractSvxObjectTitleDescDialog> pDlg(
                    pFact->CreateSvxObjectTitleDescDialog(pWrtShell->GetView().GetFrameWeld(),
                                                          aTitle, aDescription, isDecorative));

                if (RET_OK == pDlg->Execute())
                {
                    pDlg->GetTitle(aTitle);
                    pDlg->GetDescription(aDescription);
                    pDlg->IsDecorative(isDecorative);

                    pObj->SetTitle(aTitle);
                    pObj->SetDescription(aDescription);
                    pObj->SetDecorative(isDecorative);

                    pWrtShell->SetModified();
                }
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
        case IssueObject::DOCUMENT_BACKGROUND:
        {
            uno::Reference<frame::XModel> xModel(m_pDoc->GetDocShell()->GetModel(),
                                                 uno::UNO_QUERY_THROW);

            comphelper::dispatchCommand(".uno:PageAreaDialog",
                                        xModel->getCurrentController()->getFrame(), {});
        }
        break;
        case IssueObject::LANGUAGE_NOT_SET:
        {
            uno::Reference<frame::XModel> xModel(m_pDoc->GetDocShell()->GetModel(),
                                                 uno::UNO_QUERY_THROW);

            if (m_sObjectID.isEmpty())
            {
                // open the dialog "Tools/Options/Language Settings - Language"
                uno::Sequence<beans::PropertyValue> aArgs{ comphelper::makePropertyValue(
                    "Language", OUString("*")) };

                comphelper::dispatchCommand(".uno:LanguageStatus",
                                            xModel->getCurrentController()->getFrame(), aArgs);
            }
            else
            {
                uno::Sequence<beans::PropertyValue> aArgs{
                    comphelper::makePropertyValue("Param", m_sObjectID),
                    comphelper::makePropertyValue("Family", sal_Int16(SfxStyleFamily::Para))
                };

                comphelper::dispatchCommand(".uno:EditStyleFont",
                                            xModel->getCurrentController()->getFrame(), aArgs);
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
