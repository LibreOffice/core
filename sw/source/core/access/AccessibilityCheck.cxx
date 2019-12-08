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
#include <ndtxt.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <svx/svdpage.hxx>
#include <swtable.hxx>

namespace sw
{
namespace
{
// TODO move these to string file and look for a better name.
OUString sNoAlt("No alt text for graphic '%OBJECT_NAME%'");
OUString sTableMergeSplit("Table '%OBJECT_NAME%' contains merges or splits");

class NodeCheck
{
protected:
    std::vector<svx::AccessibilityIssue>& m_rAccessibilityIssueCollection;

public:
    NodeCheck(std::vector<svx::AccessibilityIssue>& rAccessibilityIssueCollection)
        : m_rAccessibilityIssueCollection(rAccessibilityIssueCollection)
    {
    }
    virtual ~NodeCheck() {}
    virtual void check(SwNode* pCurrent) = 0;
};

// Check NoTextNodes: Graphic, OLE for alt (title) text
class NoTextNodeAltTextCheck : public NodeCheck
{
    void checkNoTextNode(SwNoTextNode* pNoTextNode)
    {
        if (!pNoTextNode)
            return;

        OUString sAlternative = pNoTextNode->GetTitle();
        if (sAlternative.isEmpty())
        {
            OUString sName = pNoTextNode->GetFlyFormat()->GetName();
            svx::AccessibilityIssue aIssue;
            aIssue.m_aIssueText = sNoAlt.replaceAll("%OBJECT_NAME%", sName);
            m_rAccessibilityIssueCollection.push_back(aIssue);
        }
    }

public:
    NoTextNodeAltTextCheck(std::vector<svx::AccessibilityIssue>& rAccessibilityIssueCollection)
        : NodeCheck(rAccessibilityIssueCollection)
    {
    }

    void check(SwNode* pCurrent) override
    {
        if (pCurrent->GetNodeType() & SwNodeType::NoTextMask)
        {
            SwNoTextNode* pNoTextNode = pCurrent->GetNoTextNode();
            if (pNoTextNode)
                checkNoTextNode(pNoTextNode);
        }
    }
};

// Check Table node if the table is merged and splitted.
class TableNodeMergeSplitCheck : public NodeCheck
{
private:
    void checkTableNode(SwTableNode* pTableNode)
    {
        if (!pTableNode)
            return;

        SwTable const& rTable = pTableNode->GetTable();
        if (rTable.IsTableComplex())
        {
            OUString sName = rTable.GetTableStyleName();
            svx::AccessibilityIssue aIssue;
            aIssue.m_aIssueText = sTableMergeSplit.replaceAll("%OBJECT_NAME%", sName);
            m_rAccessibilityIssueCollection.push_back(aIssue);
        }
        else
        {
            if (rTable.GetTabLines().size() > 1)
            {
                int i = 0;
                size_t nFirstLineSize = 0;
                bool bAllColumnsSameSize = true;

                for (SwTableLine const* pTableLine : rTable.GetTabLines())
                {
                    if (i == 0)
                    {
                        nFirstLineSize = pTableLine->GetTabBoxes().size();
                    }
                    else
                    {
                        size_t nLineSize = pTableLine->GetTabBoxes().size();
                        if (nFirstLineSize != nLineSize)
                        {
                            bAllColumnsSameSize = false;
                        }
                    }
                    i++;
                }
                if (!bAllColumnsSameSize)
                {
                    OUString sName = rTable.GetTableStyleName();
                    svx::AccessibilityIssue aIssue;
                    aIssue.m_aIssueText = sTableMergeSplit.replaceAll("%OBJECT_NAME%", sName);
                    m_rAccessibilityIssueCollection.push_back(aIssue);
                }
            }
        }
    }

public:
    TableNodeMergeSplitCheck(std::vector<svx::AccessibilityIssue>& rAccessibilityIssueCollection)
        : NodeCheck(rAccessibilityIssueCollection)
    {
    }

    void check(SwNode* pCurrent) override
    {
        if (pCurrent->GetNodeType() & SwNodeType::Table)
        {
            SwTableNode* pTableNode = pCurrent->GetTableNode();
            if (pTableNode)
                checkTableNode(pTableNode);
        }
    }
};

} // end anonymous namespace

// Check Shapes, TextBox
void AccessibilityCheck::checkObject(SdrObject* pObject)
{
    if (!pObject)
        return;

    if (pObject->GetObjIdentifier() == OBJ_CUSTOMSHAPE || pObject->GetObjIdentifier() == OBJ_TEXT)
    {
        OUString sAlternative = pObject->GetTitle();
        if (sAlternative.isEmpty())
        {
            OUString sName = pObject->GetName();
            svx::AccessibilityIssue aIssue;
            aIssue.m_aIssueText = sNoAlt.replaceAll("%OBJECT_NAME%", sName);
            m_aAccessibilityIssueCollection.push_back(aIssue);
        }
    }
}

void AccessibilityCheck::check()
{
    if (m_pDoc == nullptr)
        return;

    std::vector<std::unique_ptr<NodeCheck>> aNodeChecks;
    aNodeChecks.push_back(
        std::make_unique<NoTextNodeAltTextCheck>(m_aAccessibilityIssueCollection));
    aNodeChecks.push_back(
        std::make_unique<TableNodeMergeSplitCheck>(m_aAccessibilityIssueCollection));

    auto const& pNodes = m_pDoc->GetNodes();
    SwNode* pNode = nullptr;
    for (sal_uLong n = 0; n < pNodes.Count(); ++n)
    {
        pNode = pNodes[n];
        if (pNode)
        {
            for (std::unique_ptr<NodeCheck>& rpNodeCheck : aNodeChecks)
            {
                rpNodeCheck->check(pNode);
            }
        }
    }

    IDocumentDrawModelAccess& rDrawModelAccess = m_pDoc->getIDocumentDrawModelAccess();
    auto* pModel = rDrawModelAccess.GetDrawModel();
    for (sal_uInt16 nPage = 0; nPage < pModel->GetPageCount(); ++nPage)
    {
        SdrPage* pPage = pModel->GetPage(nPage);
        for (size_t nObject = 0; nObject < pPage->GetObjCount(); ++nObject)
        {
            SdrObject* pObject = pPage->GetObj(nObject);
            if (pObject)
                checkObject(pObject);
        }
    }
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
