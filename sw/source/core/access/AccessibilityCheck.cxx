/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <AccessibilityCheck.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <svx/svdpage.hxx>

void AccessibilityCheck::check()
{
    if (m_pDoc == nullptr)
        return;

    OUString sNoAlt("No alt text for graphic '%OBJECT_NAME%'");

    // Check NoTextNodes: Graphic, OLE
    auto const& pNodes = m_pDoc->GetNodes();
    for (sal_uLong n = 0; n < pNodes.Count(); ++n)
    {
        SwNode* pNode = pNodes[n];
        if (pNode)
        {
            if (pNode->GetNodeType() & SwNodeType::NoTextMask)
            {
                SwNoTextNode* pNoTextNode = pNode->GetNoTextNode();
                if (pNoTextNode)
                {
                    OUString sAlternative = pNoTextNode->GetTitle();
                    if (sAlternative.isEmpty())
                    {
                        OUString sName = pNoTextNode->GetFlyFormat()->GetName();
                        AccessibilityIssue aIssue;
                        aIssue.m_aIssueText = sNoAlt.replaceAll("%OBJECT_NAME%", sName);
                        m_aAccessibilityIssueCollection.push_back(aIssue);
                    }
                }
            }
        }
    }

    // Check Shapes, TextBox
    IDocumentDrawModelAccess& rDrawModelAccess = m_pDoc->getIDocumentDrawModelAccess();
    auto* pModel = rDrawModelAccess.GetDrawModel();
    for (sal_uInt16 nPage = 0; nPage < pModel->GetPageCount(); ++nPage)
    {
        SdrPage* pPage = pModel->GetPage(nPage);
        for (size_t nObject = 0; nObject < pPage->GetObjCount(); ++nObject)
        {
            SdrObject* pObject = pPage->GetObj(nObject);
            if (pObject->GetObjIdentifier() == OBJ_CUSTOMSHAPE
                || pObject->GetObjIdentifier() == OBJ_TEXT)
            {
                OUString sAlternative = pObject->GetTitle();
                if (sAlternative.isEmpty())
                {
                    OUString sName = pObject->GetName();
                    AccessibilityIssue aIssue;
                    aIssue.m_aIssueText = sNoAlt.replaceAll("%OBJECT_NAME%", sName);
                    m_aAccessibilityIssueCollection.push_back(aIssue);
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
