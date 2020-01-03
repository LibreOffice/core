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
#include <AccessibilityIssue.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <ndtxt.hxx>
#include <docsh.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <svx/svdpage.hxx>
#include <swtable.hxx>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <unoparagraph.hxx>
#include <tools/urlobj.hxx>
#include <editeng/langitem.hxx>
#include <charatr.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <ftnidx.hxx>
#include <txtftn.hxx>

namespace sw
{
namespace
{
// TODO move these to string file and look for a better name.
OUString sNoAlt("No alt text for graphic '%OBJECT_NAME%'");
OUString sTableMergeSplit("Table '%OBJECT_NAME%' contains merges or splits");
OUString sFakeNumbering("Fake numbering '%NUMBERING%'");
OUString sHyperlinkTextIsLink("Hyperlink text is the same as the link address '%LINK%'");
OUString sDocumentDefaultLanguage("Document default language is not set");
OUString sStyleNoLanguage("Style '%STYLE_NAME%' has no language set");
OUString sDocumentTitle("Document title is not set");
OUString sTextContrast("Text contrast is too low.");
OUString sTextBlinking("Blinking text.");
OUString sAvoidFootnotes("Avoid footnotes.");
OUString sAvoidEndnotes("Avoid endnotes.");

std::shared_ptr<sw::AccessibilityIssue>
lclAddIssue(svx::AccessibilityIssueCollection& rIssueCollection, OUString const& rText,
            svx::AccessibilityIssueID eIssue = svx::AccessibilityIssueID::UNSPECIFIED)
{
    auto pIssue = std::make_shared<sw::AccessibilityIssue>(eIssue);
    pIssue->m_aIssueText = rText;
    rIssueCollection.getIssues().push_back(pIssue);
    return pIssue;
}

class BaseCheck
{
protected:
    svx::AccessibilityIssueCollection& m_rIssueCollection;

public:
    BaseCheck(svx::AccessibilityIssueCollection& rIssueCollection)
        : m_rIssueCollection(rIssueCollection)
    {
    }
    virtual ~BaseCheck() {}
};

class NodeCheck : public BaseCheck
{
public:
    NodeCheck(svx::AccessibilityIssueCollection& rIssueCollection)
        : BaseCheck(rIssueCollection)
    {
    }

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

            OUString sIssueText = sNoAlt.replaceAll("%OBJECT_NAME%", sName);

            auto pIssue = lclAddIssue(m_rIssueCollection, sIssueText);

            if (pNoTextNode->IsOLENode())
            {
                pIssue->setDoc(pNoTextNode->GetDoc());
                pIssue->setIssueObject(IssueObject::OLE);
                pIssue->setObjectID(pNoTextNode->GetFlyFormat()->GetName());
            }
            else if (pNoTextNode->IsGrfNode())
            {
                pIssue->setDoc(pNoTextNode->GetDoc());
                pIssue->setIssueObject(IssueObject::GRAPHIC);
                pIssue->setObjectID(pNoTextNode->GetFlyFormat()->GetName());
            }
        }
    }

public:
    NoTextNodeAltTextCheck(svx::AccessibilityIssueCollection& rIssueCollection)
        : NodeCheck(rIssueCollection)
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

// Check Table node if the table is merged and split.
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
            OUString sIssueText = sTableMergeSplit.replaceAll("%OBJECT_NAME%", sName);
            lclAddIssue(m_rIssueCollection, sIssueText);
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
                    OUString sIssueText = sTableMergeSplit.replaceAll("%OBJECT_NAME%", sName);
                    lclAddIssue(m_rIssueCollection, sIssueText);
                }
            }
        }
    }

public:
    TableNodeMergeSplitCheck(svx::AccessibilityIssueCollection& rIssueCollection)
        : NodeCheck(rIssueCollection)
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

class NumberingCheck : public NodeCheck
{
private:
    SwTextNode* pPreviousTextNode;

    const std::vector<std::pair<OUString, OUString>> constNumberingCombinations{
        { "1.", "2." }, { "(1)", "(2)" }, { "1)", "2)" },   { "a.", "b." }, { "(a)", "(b)" },
        { "a)", "b)" }, { "A.", "B." },   { "(A)", "(B)" }, { "A)", "B)" }
    };

public:
    NumberingCheck(svx::AccessibilityIssueCollection& rIssueCollection)
        : NodeCheck(rIssueCollection)
        , pPreviousTextNode(nullptr)
    {
    }

    void check(SwNode* pCurrent) override
    {
        if (pCurrent->IsTextNode())
        {
            if (pPreviousTextNode)
            {
                for (auto& rPair : constNumberingCombinations)
                {
                    if (pCurrent->GetTextNode()->GetText().startsWith(rPair.second)
                        && pPreviousTextNode->GetText().startsWith(rPair.first))
                    {
                        OUString sNumbering = rPair.first + " " + rPair.second + "...";
                        OUString sIssueText = sFakeNumbering.replaceAll("%NUMBERING%", sNumbering);
                        lclAddIssue(m_rIssueCollection, sIssueText);
                    }
                }
            }
            pPreviousTextNode = pCurrent->GetTextNode();
        }
    }
};

class HyperlinkCheck : public NodeCheck
{
private:
    void checkTextRange(uno::Reference<text::XTextRange> const& xTextRange)
    {
        uno::Reference<beans::XPropertySet> xProperties(xTextRange, uno::UNO_QUERY);
        if (xProperties->getPropertySetInfo()->hasPropertyByName("HyperLinkURL"))
        {
            OUString sHyperlink;
            xProperties->getPropertyValue("HyperLinkURL") >>= sHyperlink;
            if (!sHyperlink.isEmpty())
            {
                OUString sText = xTextRange->getString();
                if (INetURLObject(sText) == INetURLObject(sHyperlink))
                {
                    OUString sIssueText = sHyperlinkTextIsLink.replaceFirst("%LINK%", sHyperlink);
                    lclAddIssue(m_rIssueCollection, sIssueText);
                }
            }
        }
    }

public:
    HyperlinkCheck(svx::AccessibilityIssueCollection& rIssueCollection)
        : NodeCheck(rIssueCollection)
    {
    }

    void check(SwNode* pCurrent) override
    {
        if (pCurrent->IsTextNode())
        {
            SwTextNode* pTextNode = pCurrent->GetTextNode();
            uno::Reference<text::XTextContent> xParagraph
                = SwXParagraph::CreateXParagraph(*pTextNode->GetDoc(), pTextNode);
            if (xParagraph.is())
            {
                uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph,
                                                                             uno::UNO_QUERY);
                uno::Reference<container::XEnumeration> xRunEnum
                    = xRunEnumAccess->createEnumeration();
                while (xRunEnum->hasMoreElements())
                {
                    uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
                    if (xRun.is())
                    {
                        checkTextRange(xRun);
                    }
                }
            }
        }
    }
};

// Based on https://www.w3.org/TR/WCAG21/#dfn-relative-luminance
double calculateRelativeLuminance(Color const& rColor)
{
    // Convert to BColor which has R, G, B colors components
    // represented by a floating point number from [0.0, 1.0]
    const basegfx::BColor aBColor = rColor.getBColor();

    double r = aBColor.getRed();
    double g = aBColor.getGreen();
    double b = aBColor.getBlue();

    // Calculate the values according to the described algorithm
    r = (r <= 0.03928) ? r / 12.92 : std::pow((r + 0.055) / 1.055, 2.4);
    g = (g <= 0.03928) ? g / 12.92 : std::pow((g + 0.055) / 1.055, 2.4);
    b = (b <= 0.03928) ? b / 12.92 : std::pow((b + 0.055) / 1.055, 2.4);

    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

// TODO move to common color tools (BColorTools maybe)
// Based on https://www.w3.org/TR/WCAG21/#dfn-contrast-ratio
double calculateContrastRatio(Color const& rColor1, Color const& rColor2)
{
    const double fLuminance1 = calculateRelativeLuminance(rColor1);
    const double fLuminance2 = calculateRelativeLuminance(rColor2);
    const std::pair<const double, const double> aMinMax = std::minmax(fLuminance1, fLuminance2);

    // (L1 + 0.05) / (L2 + 0.05)
    // L1 is the lighter color (greater luminance value)
    // L2 is the darker color (smaller luminance value)
    return (aMinMax.second + 0.05) / (aMinMax.first + 0.05);
}

class TextContrastCheck : public NodeCheck
{
private:
    void checkTextRange(uno::Reference<text::XTextRange> const& xTextRange,
                        uno::Reference<text::XTextContent> const& xParagraph, SwTextNode* pTextNode)
    {
        sal_Int32 nParaBackColor;
        uno::Reference<beans::XPropertySet> xParagraphProperties(xParagraph, uno::UNO_QUERY);
        xParagraphProperties->getPropertyValue("ParaBackColor") >>= nParaBackColor;

        uno::Reference<beans::XPropertySet> xProperties(xTextRange, uno::UNO_QUERY);
        if (xProperties.is())
        {
            // Foreground color
            sal_Int32 nCharColor;
            xProperties->getPropertyValue("CharColor") >>= nCharColor;
            Color aForegroundColor(nCharColor);
            if (aForegroundColor == COL_AUTO)
                return;

            const SwPageDesc* pPageDescription = pTextNode->FindPageDesc();
            const SwFrameFormat& rPageFormat = pPageDescription->GetMaster();
            const SwAttrSet& rPageSet = rPageFormat.GetAttrSet();

            const XFillStyleItem* pXFillStyleItem(
                rPageSet.GetItem<XFillStyleItem>(XATTR_FILLSTYLE, false));
            Color aPageBackground;

            if (pXFillStyleItem && pXFillStyleItem->GetValue() == css::drawing::FillStyle_SOLID)
            {
                const XFillColorItem* rXFillColorItem
                    = rPageSet.GetItem<XFillColorItem>(XATTR_FILLCOLOR, false);
                aPageBackground = rXFillColorItem->GetColorValue();
            }

            sal_Int32 nCharBackColor;
            sal_Int16 eRelief;

            xProperties->getPropertyValue("CharBackColor") >>= nCharBackColor;
            xProperties->getPropertyValue("CharRelief") >>= eRelief;

            // Determine the background color
            // Try Character background (highlight)
            Color aBackgroundColor(nCharBackColor);

            // If not character background color, try paragraph background color
            if (aBackgroundColor == COL_AUTO)
                aBackgroundColor = Color(nParaBackColor);

            // If not paragraph background color, try page color
            if (aBackgroundColor == COL_AUTO)
                aBackgroundColor = aPageBackground;

            // If not page color, assume white background color
            if (aBackgroundColor == COL_AUTO)
                aBackgroundColor = COL_WHITE;

            double fContrastRatio = calculateContrastRatio(aForegroundColor, aBackgroundColor);
            if (fContrastRatio < 4.5)
            {
                lclAddIssue(m_rIssueCollection, sTextContrast);
            }
        }
    }

public:
    TextContrastCheck(svx::AccessibilityIssueCollection& rIssueCollection)
        : NodeCheck(rIssueCollection)
    {
    }

    void check(SwNode* pCurrent) override
    {
        if (pCurrent->IsTextNode())
        {
            SwTextNode* pTextNode = pCurrent->GetTextNode();
            uno::Reference<text::XTextContent> xParagraph;
            xParagraph = SwXParagraph::CreateXParagraph(*pTextNode->GetDoc(), pTextNode);
            if (xParagraph.is())
            {
                uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph,
                                                                             uno::UNO_QUERY);
                uno::Reference<container::XEnumeration> xRunEnum
                    = xRunEnumAccess->createEnumeration();
                while (xRunEnum->hasMoreElements())
                {
                    uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
                    if (xRun.is())
                        checkTextRange(xRun, xParagraph, pTextNode);
                }
            }
        }
    }
};

class BlinkingTextCheck : public NodeCheck
{
private:
    void checkTextRange(uno::Reference<text::XTextRange> const& xTextRange)
    {
        uno::Reference<beans::XPropertySet> xProperties(xTextRange, uno::UNO_QUERY);
        if (xProperties.is() && xProperties->getPropertySetInfo()->hasPropertyByName("CharFlash"))
        {
            bool bBlinking = false;
            xProperties->getPropertyValue("CharFlash") >>= bBlinking;

            if (bBlinking)
            {
                lclAddIssue(m_rIssueCollection, sTextBlinking);
            }
        }
    }

public:
    BlinkingTextCheck(svx::AccessibilityIssueCollection& rIssueCollection)
        : NodeCheck(rIssueCollection)
    {
    }

    void check(SwNode* pCurrent) override
    {
        if (pCurrent->IsTextNode())
        {
            SwTextNode* pTextNode = pCurrent->GetTextNode();
            uno::Reference<text::XTextContent> xParagraph;
            xParagraph = SwXParagraph::CreateXParagraph(*pTextNode->GetDoc(), pTextNode);
            if (xParagraph.is())
            {
                uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph,
                                                                             uno::UNO_QUERY);
                uno::Reference<container::XEnumeration> xRunEnum
                    = xRunEnumAccess->createEnumeration();
                while (xRunEnum->hasMoreElements())
                {
                    uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
                    if (xRun.is())
                        checkTextRange(xRun);
                }
            }
        }
    }
};

class DocumentCheck : public BaseCheck
{
public:
    DocumentCheck(svx::AccessibilityIssueCollection& rIssueCollection)
        : BaseCheck(rIssueCollection)
    {
    }

    virtual void check(SwDoc* pDoc) = 0;
};

// Check default language
class DocumentDefaultLanguageCheck : public DocumentCheck
{
public:
    DocumentDefaultLanguageCheck(svx::AccessibilityIssueCollection& rIssueCollection)
        : DocumentCheck(rIssueCollection)
    {
    }

    void check(SwDoc* pDoc) override
    {
        // TODO maybe - also check RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CTL_LANGUAGE if CJK or CTL are enabled
        const SvxLanguageItem& rLang = pDoc->GetDefault(RES_CHRATR_LANGUAGE);
        LanguageType eLanguage = rLang.GetLanguage();
        if (eLanguage == LANGUAGE_NONE)
        {
            lclAddIssue(m_rIssueCollection, sDocumentDefaultLanguage,
                        svx::AccessibilityIssueID::DOCUMENT_LANGUAGE);
        }
        else
        {
            for (SwTextFormatColl* pTextFormatCollection : *pDoc->GetTextFormatColls())
            {
                const SwAttrSet& rAttrSet = pTextFormatCollection->GetAttrSet();
                if (rAttrSet.GetLanguage(false).GetLanguage() == LANGUAGE_NONE)
                {
                    OUString sName = pTextFormatCollection->GetName();
                    OUString sIssueText = sStyleNoLanguage.replaceAll("%STYLE_NAME%", sName);
                    lclAddIssue(m_rIssueCollection, sIssueText,
                                svx::AccessibilityIssueID::STYLE_LANGUAGE);
                }
            }
        }
    }
};

class DocumentTitleCheck : public DocumentCheck
{
public:
    DocumentTitleCheck(svx::AccessibilityIssueCollection& rIssueCollection)
        : DocumentCheck(rIssueCollection)
    {
    }

    void check(SwDoc* pDoc) override
    {
        SwDocShell* pShell = pDoc->GetDocShell();
        if (pShell)
        {
            const uno::Reference<document::XDocumentPropertiesSupplier> xDPS(pShell->GetModel(),
                                                                             uno::UNO_QUERY_THROW);
            const uno::Reference<document::XDocumentProperties> xDocumentProperties(
                xDPS->getDocumentProperties());
            OUString sTitle = xDocumentProperties->getTitle();
            if (sTitle.isEmpty())
            {
                lclAddIssue(m_rIssueCollection, sDocumentTitle,
                            svx::AccessibilityIssueID::DOCUMENT_TITLE);
            }
        }
    }
};

class FootnoteEndnoteCheck : public DocumentCheck
{
public:
    FootnoteEndnoteCheck(svx::AccessibilityIssueCollection& rIssueCollection)
        : DocumentCheck(rIssueCollection)
    {
    }

    void check(SwDoc* pDoc) override
    {
        for (SwTextFootnote const* pTextFootnote : pDoc->GetFootnoteIdxs())
        {
            SwFormatFootnote const& rFootnote = pTextFootnote->GetFootnote();
            if (rFootnote.IsEndNote())
            {
                lclAddIssue(m_rIssueCollection, sAvoidEndnotes);
            }
            else
            {
                lclAddIssue(m_rIssueCollection, sAvoidFootnotes);
            }
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
            OUString sIssueText = sNoAlt.replaceAll("%OBJECT_NAME%", sName);
            lclAddIssue(m_aIssueCollection, sIssueText);
        }
    }
}

void AccessibilityCheck::check()
{
    if (m_pDoc == nullptr)
        return;

    std::vector<std::unique_ptr<DocumentCheck>> aDocumentChecks;
    aDocumentChecks.push_back(std::make_unique<DocumentDefaultLanguageCheck>(m_aIssueCollection));
    aDocumentChecks.push_back(std::make_unique<DocumentTitleCheck>(m_aIssueCollection));
    aDocumentChecks.push_back(std::make_unique<FootnoteEndnoteCheck>(m_aIssueCollection));

    for (std::unique_ptr<DocumentCheck>& rpDocumentCheck : aDocumentChecks)
    {
        rpDocumentCheck->check(m_pDoc);
    }

    std::vector<std::unique_ptr<NodeCheck>> aNodeChecks;
    aNodeChecks.push_back(std::make_unique<NoTextNodeAltTextCheck>(m_aIssueCollection));
    aNodeChecks.push_back(std::make_unique<TableNodeMergeSplitCheck>(m_aIssueCollection));
    aNodeChecks.push_back(std::make_unique<NumberingCheck>(m_aIssueCollection));
    aNodeChecks.push_back(std::make_unique<HyperlinkCheck>(m_aIssueCollection));
    aNodeChecks.push_back(std::make_unique<TextContrastCheck>(m_aIssueCollection));
    aNodeChecks.push_back(std::make_unique<BlinkingTextCheck>(m_aIssueCollection));

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
