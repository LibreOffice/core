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
#include <AccessibilityCheckStrings.hrc>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <ndtxt.hxx>
#include <docsh.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <svx/svdpage.hxx>
#include <swtable.hxx>
#include <com/sun/star/frame/XModel.hpp>
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
#include <svl/itemiter.hxx>
#include <o3tl/vector_utils.hxx>

namespace sw
{
namespace
{
std::shared_ptr<sw::AccessibilityIssue>
lclAddIssue(sfx::AccessibilityIssueCollection& rIssueCollection, OUString const& rText,
            sfx::AccessibilityIssueID eIssue = sfx::AccessibilityIssueID::UNSPECIFIED)
{
    auto pIssue = std::make_shared<sw::AccessibilityIssue>(eIssue);
    pIssue->m_aIssueText = rText;
    rIssueCollection.getIssues().push_back(pIssue);
    return pIssue;
}

class BaseCheck
{
protected:
    sfx::AccessibilityIssueCollection& m_rIssueCollection;

public:
    BaseCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
        : m_rIssueCollection(rIssueCollection)
    {
    }
    virtual ~BaseCheck() {}
};

class NodeCheck : public BaseCheck
{
public:
    NodeCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
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
        if (!sAlternative.isEmpty())
            return;

        OUString sName = pNoTextNode->GetFlyFormat()->GetName();

        OUString sIssueText = SwResId(STR_NO_ALT).replaceAll("%OBJECT_NAME%", sName);

        if (pNoTextNode->IsOLENode())
        {
            auto pIssue = lclAddIssue(m_rIssueCollection, sIssueText,
                                      sfx::AccessibilityIssueID::NO_ALT_OLE);
            pIssue->setDoc(pNoTextNode->GetDoc());
            pIssue->setIssueObject(IssueObject::OLE);
            pIssue->setObjectID(pNoTextNode->GetFlyFormat()->GetName());
        }
        else if (pNoTextNode->IsGrfNode())
        {
            auto pIssue = lclAddIssue(m_rIssueCollection, sIssueText,
                                      sfx::AccessibilityIssueID::NO_ALT_GRAPHIC);
            pIssue->setDoc(pNoTextNode->GetDoc());
            pIssue->setIssueObject(IssueObject::GRAPHIC);
            pIssue->setObjectID(pNoTextNode->GetFlyFormat()->GetName());
        }
    }

public:
    NoTextNodeAltTextCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
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
    void addTableIssue(SwTable const& rTable, SwDoc* pDoc)
    {
        const SwTableFormat* pFormat = rTable.GetFrameFormat();
        OUString sName = pFormat->GetName();
        OUString sIssueText = SwResId(STR_TABLE_MERGE_SPLIT).replaceAll("%OBJECT_NAME%", sName);
        auto pIssue = lclAddIssue(m_rIssueCollection, sIssueText,
                                  sfx::AccessibilityIssueID::TABLE_MERGE_SPLIT);
        pIssue->setDoc(pDoc);
        pIssue->setIssueObject(IssueObject::TABLE);
        pIssue->setObjectID(sName);
    }

    void checkTableNode(SwTableNode* pTableNode)
    {
        if (!pTableNode)
            return;

        SwTable const& rTable = pTableNode->GetTable();
        SwDoc* pDoc = pTableNode->GetDoc();
        if (rTable.IsTableComplex())
        {
            addTableIssue(rTable, pDoc);
        }
        else
        {
            if (rTable.GetTabLines().size() > 1)
            {
                int i = 0;
                size_t nFirstLineSize = 0;
                bool bAllColumnsSameSize = true;
                bool bCellSpansOverMoreRows = false;

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

                    // Check for row span in each table box (cell)
                    for (SwTableBox const* pBox : pTableLine->GetTabBoxes())
                    {
                        if (pBox->getRowSpan() > 1)
                            bCellSpansOverMoreRows = true;
                    }
                }
                if (!bAllColumnsSameSize || bCellSpansOverMoreRows)
                {
                    addTableIssue(rTable, pDoc);
                }
            }
        }
    }

public:
    TableNodeMergeSplitCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
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
    SwTextNode* m_pPreviousTextNode;

    const std::vector<std::pair<OUString, OUString>> m_aNumberingCombinations{
        { "1.", "2." }, { "(1)", "(2)" }, { "1)", "2)" },   { "a.", "b." }, { "(a)", "(b)" },
        { "a)", "b)" }, { "A.", "B." },   { "(A)", "(B)" }, { "A)", "B)" }
    };

public:
    NumberingCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
        : NodeCheck(rIssueCollection)
        , m_pPreviousTextNode(nullptr)
    {
    }

    void check(SwNode* pCurrent) override
    {
        if (!pCurrent->IsTextNode())
            return;

        if (m_pPreviousTextNode)
        {
            for (auto& rPair : m_aNumberingCombinations)
            {
                if (pCurrent->GetTextNode()->GetText().startsWith(rPair.second)
                    && m_pPreviousTextNode->GetText().startsWith(rPair.first))
                {
                    OUString sNumbering = rPair.first + " " + rPair.second + "...";
                    OUString sIssueText
                        = SwResId(STR_FAKE_NUMBERING).replaceAll("%NUMBERING%", sNumbering);
                    lclAddIssue(m_rIssueCollection, sIssueText);
                }
            }
        }
        m_pPreviousTextNode = pCurrent->GetTextNode();
    }
};

class HyperlinkCheck : public NodeCheck
{
private:
    void checkTextRange(uno::Reference<text::XTextRange> const& xTextRange)
    {
        uno::Reference<beans::XPropertySet> xProperties(xTextRange, uno::UNO_QUERY);
        if (!xProperties->getPropertySetInfo()->hasPropertyByName("HyperLinkURL"))
            return;

        OUString sHyperlink;
        xProperties->getPropertyValue("HyperLinkURL") >>= sHyperlink;
        if (!sHyperlink.isEmpty())
        {
            OUString sText = xTextRange->getString();
            if (INetURLObject(sText) == INetURLObject(sHyperlink))
            {
                OUString sIssueText
                    = SwResId(STR_HYPERLINK_TEXT_IS_LINK).replaceFirst("%LINK%", sHyperlink);
                lclAddIssue(m_rIssueCollection, sIssueText);
            }
        }
    }

public:
    HyperlinkCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
        : NodeCheck(rIssueCollection)
    {
    }

    void check(SwNode* pCurrent) override
    {
        if (!pCurrent->IsTextNode())
            return;

        SwTextNode* pTextNode = pCurrent->GetTextNode();
        uno::Reference<text::XTextContent> xParagraph
            = SwXParagraph::CreateXParagraph(*pTextNode->GetDoc(), pTextNode);
        if (!xParagraph.is())
            return;

        uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
        while (xRunEnum->hasMoreElements())
        {
            uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
            if (xRun.is())
            {
                checkTextRange(xRun);
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
        sal_Int32 nParaBackColor(COL_AUTO);
        uno::Reference<beans::XPropertySet> xParagraphProperties(xParagraph, uno::UNO_QUERY);
        if (!(xParagraphProperties->getPropertyValue("ParaBackColor") >>= nParaBackColor))
        {
            SAL_WARN("sw.a11y", "ParaBackColor void");
            return;
        }

        uno::Reference<beans::XPropertySet> xProperties(xTextRange, uno::UNO_QUERY);
        if (!xProperties.is())
            return;

        // Foreground color
        sal_Int32 nCharColor = {}; // spurious -Werror=maybe-uninitialized
        if (!(xProperties->getPropertyValue("CharColor") >>= nCharColor))
        { // not sure this is impossible, can the default be void?
            SAL_WARN("sw.a11y", "CharColor void");
            return;
        }
        Color aForegroundColor(nCharColor);
        if (aForegroundColor == COL_AUTO)
            return;

        const SwPageDesc* pPageDescription = pTextNode->FindPageDesc();
        const SwFrameFormat& rPageFormat = pPageDescription->GetMaster();
        const SwAttrSet& rPageSet = rPageFormat.GetAttrSet();

        const XFillStyleItem* pXFillStyleItem(
            rPageSet.GetItem<XFillStyleItem>(XATTR_FILLSTYLE, false));
        Color aPageBackground(COL_AUTO);

        if (pXFillStyleItem && pXFillStyleItem->GetValue() == css::drawing::FillStyle_SOLID)
        {
            const XFillColorItem* rXFillColorItem
                = rPageSet.GetItem<XFillColorItem>(XATTR_FILLCOLOR, false);
            aPageBackground = rXFillColorItem->GetColorValue();
        }

        sal_Int32 nCharBackColor(COL_AUTO);

        if (!(xProperties->getPropertyValue("CharBackColor") >>= nCharBackColor))
        {
            SAL_WARN("sw.a11y", "CharBackColor void");
            return;
        }
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
            lclAddIssue(m_rIssueCollection, SwResId(STR_TEXT_CONTRAST));
        }
    }

public:
    TextContrastCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
        : NodeCheck(rIssueCollection)
    {
    }

    void check(SwNode* pCurrent) override
    {
        if (!pCurrent->IsTextNode())
            return;

        SwTextNode* pTextNode = pCurrent->GetTextNode();
        uno::Reference<text::XTextContent> xParagraph;
        xParagraph = SwXParagraph::CreateXParagraph(*pTextNode->GetDoc(), pTextNode);
        if (!xParagraph.is())
            return;

        uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
        while (xRunEnum->hasMoreElements())
        {
            uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
            if (xRun.is())
                checkTextRange(xRun, xParagraph, pTextNode);
        }
    }
};

class TextFormattingCheck : public NodeCheck
{
private:
public:
    TextFormattingCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
        : NodeCheck(rIssueCollection)
    {
    }

    void checkAutoFormat(SwTextNode* pTextNode, const SwTextAttr* pTextAttr)
    {
        const SwFormatAutoFormat& rAutoFormat = pTextAttr->GetAutoFormat();
        SfxItemIter aItemIter(*rAutoFormat.GetStyleHandle());
        const SfxPoolItem* pItem = aItemIter.GetCurItem();
        std::vector<OUString> aFormattings;
        while (pItem)
        {
            OUString sFormattingType;
            switch (pItem->Which())
            {
                case RES_CHRATR_WEIGHT:
                case RES_CHRATR_CJK_WEIGHT:
                case RES_CHRATR_CTL_WEIGHT:
                    sFormattingType = "Weight";
                    break;
                case RES_CHRATR_POSTURE:
                case RES_CHRATR_CJK_POSTURE:
                case RES_CHRATR_CTL_POSTURE:
                    sFormattingType = "Posture";
                    break;

                case RES_CHRATR_SHADOWED:
                    sFormattingType = "Shadowed";
                    break;

                case RES_CHRATR_COLOR:
                    sFormattingType = "Font Color";
                    break;

                case RES_CHRATR_FONTSIZE:
                case RES_CHRATR_CJK_FONTSIZE:
                case RES_CHRATR_CTL_FONTSIZE:
                    sFormattingType = "Font Size";
                    break;

                case RES_CHRATR_FONT:
                case RES_CHRATR_CJK_FONT:
                case RES_CHRATR_CTL_FONT:
                    sFormattingType = "Font";
                    break;

                case RES_CHRATR_EMPHASIS_MARK:
                    sFormattingType = "Emphasis Mark";
                    break;

                case RES_CHRATR_UNDERLINE:
                    sFormattingType = "Underline";
                    break;

                case RES_CHRATR_OVERLINE:
                    sFormattingType = "Overline";
                    break;

                case RES_CHRATR_CROSSEDOUT:
                    sFormattingType = "Strikethrough";
                    break;

                case RES_CHRATR_RELIEF:
                    sFormattingType = "Relief";
                    break;

                case RES_CHRATR_CONTOUR:
                    sFormattingType = "Outline";
                    break;
                default:
                    break;
            }
            if (!sFormattingType.isEmpty())
                aFormattings.push_back(sFormattingType);
            pItem = aItemIter.NextItem();
        }
        if (aFormattings.empty())
            return;

        o3tl::remove_duplicates(aFormattings);
        auto pIssue = lclAddIssue(m_rIssueCollection, SwResId(STR_TEXT_FORMATTING_CONVEYS_MEANING),
                                  sfx::AccessibilityIssueID::TEXT_FORMATTING);
        pIssue->setIssueObject(IssueObject::TEXT);
        pIssue->setNode(pTextNode);
        SwDoc* pDocument = pTextNode->GetDoc();
        pIssue->setDoc(pDocument);
        pIssue->setStart(pTextAttr->GetStart());
        pIssue->setEnd(pTextAttr->GetAnyEnd());
    }
    void check(SwNode* pCurrent) override
    {
        if (!pCurrent->IsTextNode())
            return;

        SwTextNode* pTextNode = pCurrent->GetTextNode();
        if (pTextNode->HasHints())
        {
            SwpHints& rHints = pTextNode->GetSwpHints();
            for (size_t i = 0; i < rHints.Count(); ++i)
            {
                const SwTextAttr* pTextAttr = rHints.Get(i);
                if (pTextAttr->Which() == RES_TXTATR_AUTOFMT)
                {
                    checkAutoFormat(pTextNode, pTextAttr);
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
                lclAddIssue(m_rIssueCollection, SwResId(STR_TEXT_BLINKING));
            }
        }
    }

public:
    BlinkingTextCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
        : NodeCheck(rIssueCollection)
    {
    }

    void check(SwNode* pCurrent) override
    {
        if (!pCurrent->IsTextNode())
            return;

        SwTextNode* pTextNode = pCurrent->GetTextNode();
        uno::Reference<text::XTextContent> xParagraph;
        xParagraph = SwXParagraph::CreateXParagraph(*pTextNode->GetDoc(), pTextNode);
        if (!xParagraph.is())
            return;

        uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
        while (xRunEnum->hasMoreElements())
        {
            uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
            if (xRun.is())
                checkTextRange(xRun);
        }
    }
};

class HeaderCheck : public NodeCheck
{
private:
    int m_nPreviousLevel;

public:
    HeaderCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
        : NodeCheck(rIssueCollection)
        , m_nPreviousLevel(0)
    {
    }

    void check(SwNode* pCurrent) override
    {
        if (!pCurrent->IsTextNode())
            return;

        SwTextNode* pTextNode = pCurrent->GetTextNode();
        SwTextFormatColl* pCollection = pTextNode->GetTextColl();
        int nLevel = pCollection->GetAssignedOutlineStyleLevel();
        if (nLevel < 0)
            return;

        if (nLevel > m_nPreviousLevel && std::abs(nLevel - m_nPreviousLevel) > 1)
        {
            lclAddIssue(m_rIssueCollection, SwResId(STR_HEADINGS_NOT_IN_ORDER));
        }
        m_nPreviousLevel = nLevel;
    }
};

// ISO 142891-1 : 7.14
class NonInteractiveFormCheck : public NodeCheck
{
public:
    NonInteractiveFormCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
        : NodeCheck(rIssueCollection)
    {
    }

    void check(SwNode* pCurrent) override
    {
        if (!pCurrent->IsTextNode())
            return;

        SwTextNode* pTextNode = pCurrent->GetTextNode();
        // Detecting if there are multiple underscores in a text
        if (pTextNode->GetText().indexOf(rtl::OUString("___")) != -1)
        {
            // A dummy warning to check if dectection works properly.
            lclAddIssue(m_rIssueCollection, SwResId(STR_HEADINGS_NOT_IN_ORDER));
        }
    }
};

class DocumentCheck : public BaseCheck
{
public:
    DocumentCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
        : BaseCheck(rIssueCollection)
    {
    }

    virtual void check(SwDoc* pDoc) = 0;
};

// Check default language
class DocumentDefaultLanguageCheck : public DocumentCheck
{
public:
    DocumentDefaultLanguageCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
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
            lclAddIssue(m_rIssueCollection, SwResId(STR_DOCUMENT_DEFAULT_LANGUAGE),
                        sfx::AccessibilityIssueID::DOCUMENT_LANGUAGE);
        }
        else
        {
            for (SwTextFormatColl* pTextFormatCollection : *pDoc->GetTextFormatColls())
            {
                const SwAttrSet& rAttrSet = pTextFormatCollection->GetAttrSet();
                if (rAttrSet.GetLanguage(false).GetLanguage() == LANGUAGE_NONE)
                {
                    OUString sName = pTextFormatCollection->GetName();
                    OUString sIssueText
                        = SwResId(STR_STYLE_NO_LANGUAGE).replaceAll("%STYLE_NAME%", sName);
                    lclAddIssue(m_rIssueCollection, sIssueText,
                                sfx::AccessibilityIssueID::STYLE_LANGUAGE);
                }
            }
        }
    }
};

class DocumentTitleCheck : public DocumentCheck
{
public:
    DocumentTitleCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
        : DocumentCheck(rIssueCollection)
    {
    }

    void check(SwDoc* pDoc) override
    {
        SwDocShell* pShell = pDoc->GetDocShell();
        if (!pShell)
            return;

        const uno::Reference<document::XDocumentPropertiesSupplier> xDPS(pShell->GetModel(),
                                                                         uno::UNO_QUERY_THROW);
        const uno::Reference<document::XDocumentProperties> xDocumentProperties(
            xDPS->getDocumentProperties());
        OUString sTitle = xDocumentProperties->getTitle();
        if (sTitle.isEmpty())
        {
            lclAddIssue(m_rIssueCollection, SwResId(STR_DOCUMENT_TITLE),
                        sfx::AccessibilityIssueID::DOCUMENT_TITLE);
        }
    }
};

class FootnoteEndnoteCheck : public DocumentCheck
{
public:
    FootnoteEndnoteCheck(sfx::AccessibilityIssueCollection& rIssueCollection)
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
                lclAddIssue(m_rIssueCollection, SwResId(STR_AVOID_ENDNOTES));
            }
            else
            {
                lclAddIssue(m_rIssueCollection, SwResId(STR_AVOID_FOOTNOTES));
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
            OUString sIssueText = SwResId(STR_NO_ALT).replaceAll("%OBJECT_NAME%", sName);
            lclAddIssue(m_aIssueCollection, sIssueText, sfx::AccessibilityIssueID::NO_ALT_SHAPE);
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
    aNodeChecks.push_back(std::make_unique<HeaderCheck>(m_aIssueCollection));
    aNodeChecks.push_back(std::make_unique<TextFormattingCheck>(m_aIssueCollection));
    aNodeChecks.push_back(std::make_unique<NonInteractiveFormCheck>(m_aIssueCollection));

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
