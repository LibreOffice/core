/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <ThemeColorChanger.hxx>
#include <ModelTraverser.hxx>
#include <txtftn.hxx>
#include <txtfrm.hxx>
#include <docstyle.hxx>
#include <drawdoc.hxx>
#include <ndnotxt.hxx>
#include <ndtxt.hxx>
#include <format.hxx>
#include <charatr.hxx>
#include <paratr.hxx>
#include <frmatr.hxx>
#include <wrtsh.hxx>

#include <DocumentContentOperationsManager.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <UndoThemeChange.hxx>
#include <UndoManager.hxx>

#include <svx/xflclit.hxx>

#include <svl/undo.hxx>
#include <sal/config.h>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/theme/IThemeColorChanger.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/theme/Theme.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/boxitem.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace sw
{
namespace
{
bool changeBorderLine(editeng::SvxBorderLine* pBorderLine, model::ColorSet const& rColorSet)
{
    if (!pBorderLine)
        return false;

    model::ComplexColor const& rComplexColor = pBorderLine->getComplexColor();
    if (rComplexColor.isValidThemeType())
    {
        Color aColor = rColorSet.resolveColor(rComplexColor);
        pBorderLine->SetColor(aColor);
        return true;
    }
    return false;
}

/** Handler for ModelTraverser that recalculates and updates the theme colors.
 *
 * It checks all the SdrObjects and updates fill, line and text theme colors.
 * For writer nodes it checks all the text nodes and updates the direct
 * formatting in all hints.
 *
 */
class ThemeColorHandler : public sw::ModelTraverseHandler
{
    SwDoc& mrDocument;
    SwDocShell* mpDocShell;
    model::ColorSet const& mrColorSet;

public:
    ThemeColorHandler(SwDoc& rDocument, SwDocShell* pDocShell, model::ColorSet const& rColorSet)
        : mrDocument(rDocument)
        , mpDocShell(pDocShell)
        , mrColorSet(rColorSet)
    {
    }

    /// Updates hints for a text node
    void updateHints(SwTextNode* pTextNode)
    {
        if (!pTextNode->HasHints())
            return;

        SwpHints& rHints = pTextNode->GetSwpHints();
        for (size_t i = 0; i < rHints.Count(); ++i)
        {
            const SwTextAttr* pTextAttr = rHints.Get(i);
            SwPaM aPam(*pTextNode, pTextAttr->GetStart(), *pTextNode, pTextAttr->GetAnyEnd());
            if (pTextAttr->Which() == RES_TXTATR_AUTOFMT)
            {
                SwFormatAutoFormat const& rAutoFormatPool(pTextAttr->GetAutoFormat());
                std::shared_ptr<SfxItemSet> pStyleHandle(rAutoFormatPool.GetStyleHandle());
                if (const SvxColorItem* pItem = pStyleHandle->GetItemIfSet(RES_CHRATR_COLOR))
                {
                    model::ComplexColor const& rComplexColor = pItem->getComplexColor();
                    if (rComplexColor.isValidThemeType())
                    {
                        Color aNewColor = mrColorSet.resolveColor(rComplexColor);
                        auto pNew = pItem->Clone();
                        pNew->SetValue(aNewColor);

                        mrDocument.getIDocumentContentOperations().InsertPoolItem(
                            aPam, *pNew, SetAttrMode::APICALL | SetAttrMode::NO_CURSOR_CHANGE);
                    }
                }
                if (const SvxUnderlineItem* pItem
                    = pStyleHandle->GetItemIfSet(RES_CHRATR_UNDERLINE))
                {
                    model::ComplexColor const& rComplexColor = pItem->getComplexColor();
                    if (rComplexColor.isValidThemeType())
                    {
                        Color aNewColor = mrColorSet.resolveColor(rComplexColor);
                        auto pNew = pItem->Clone();
                        pNew->SetColor(aNewColor);

                        mrDocument.getIDocumentContentOperations().InsertPoolItem(
                            aPam, *pNew, SetAttrMode::APICALL | SetAttrMode::NO_CURSOR_CHANGE);
                    }
                }
                if (const SvxOverlineItem* pItem = pStyleHandle->GetItemIfSet(RES_CHRATR_OVERLINE))
                {
                    model::ComplexColor const& rComplexColor = pItem->getComplexColor();
                    if (rComplexColor.isValidThemeType())
                    {
                        Color aNewColor = mrColorSet.resolveColor(rComplexColor);
                        auto pNew = pItem->Clone();
                        pNew->SetColor(aNewColor);

                        mrDocument.getIDocumentContentOperations().InsertPoolItem(
                            aPam, *pNew, SetAttrMode::APICALL | SetAttrMode::NO_CURSOR_CHANGE);
                    }
                }
            }
        }
    }

    void updateParagraphAttributes(SwTextNode* pTextNode)
    {
        if (!pTextNode->HasSwAttrSet())
            return;

        SwAttrSet const& aAttrSet = pTextNode->GetSwAttrSet();

        if (const SvxColorItem* pItem = aAttrSet.GetItemIfSet(RES_CHRATR_COLOR, false))
        {
            model::ComplexColor const& rComplexColor = pItem->getComplexColor();
            if (rComplexColor.isValidThemeType())
            {
                Color aNewColor = mrColorSet.resolveColor(rComplexColor);
                std::unique_ptr<SvxColorItem> pNewItem(pItem->Clone());
                pNewItem->setColor(aNewColor);
                pTextNode->SetAttr(*pNewItem);
            }
        }

        if (const SvxUnderlineItem* pItem = aAttrSet.GetItemIfSet(RES_CHRATR_UNDERLINE, false))
        {
            model::ComplexColor const& rComplexColor = pItem->getComplexColor();
            if (rComplexColor.isValidThemeType())
            {
                Color aNewColor = mrColorSet.resolveColor(rComplexColor);
                std::unique_ptr<SvxUnderlineItem> pNewItem(pItem->Clone());
                pNewItem->SetColor(aNewColor);
                pTextNode->SetAttr(*pNewItem);
            }
        }

        if (const SvxOverlineItem* pItem = aAttrSet.GetItemIfSet(RES_CHRATR_OVERLINE, false))
        {
            model::ComplexColor const& rComplexColor = pItem->getComplexColor();
            if (rComplexColor.isValidThemeType())
            {
                Color aNewColor = mrColorSet.resolveColor(rComplexColor);
                std::unique_ptr<SvxOverlineItem> pNewItem(pItem->Clone());
                pNewItem->SetColor(aNewColor);
                pTextNode->SetAttr(*pNewItem);
            }
        }

        if (const XFillColorItem* pItem = aAttrSet.GetItemIfSet(XATTR_FILLCOLOR, false))
        {
            model::ComplexColor const& rComplexColor = pItem->getComplexColor();
            if (rComplexColor.isValidThemeType())
            {
                Color aNewColor = mrColorSet.resolveColor(rComplexColor);
                std::unique_ptr<XFillColorItem> pNewItem(pItem->Clone());
                pNewItem->SetColorValue(aNewColor);
                pTextNode->SetAttr(*pNewItem);
            }
        }

        if (const SvxBoxItem* pItem = aAttrSet.GetItemIfSet(RES_BOX, false))
        {
            std::unique_ptr<SvxBoxItem> pNewItem(pItem->Clone());

            bool bChanged = false;

            bChanged = changeBorderLine(pNewItem->GetBottom(), mrColorSet) || bChanged;
            bChanged = changeBorderLine(pNewItem->GetTop(), mrColorSet) || bChanged;
            bChanged = changeBorderLine(pNewItem->GetLeft(), mrColorSet) || bChanged;
            bChanged = changeBorderLine(pNewItem->GetRight(), mrColorSet) || bChanged;

            if (bChanged)
            {
                pTextNode->SetAttr(*pNewItem);
            }
        }
    }

    void handleNode(SwNode* pNode) override
    {
        if (!pNode->IsTextNode())
            return;

        updateHints(pNode->GetTextNode());
        updateParagraphAttributes(pNode->GetTextNode());
    }

    void handleSdrObject(SdrObject* pObject) override
    {
        SwWrtShell* pWrtShell = mpDocShell->GetWrtShell();
        SdrView* pView = pWrtShell->GetDrawView();

        SfxUndoManager* pManager = &mrDocument.GetUndoManager();

        // update current object
        svx::theme::updateSdrObject(mrColorSet, pObject, pView, pManager);

        // update child objects
        SdrObjList* pList = pObject->GetSubList();
        if (pList)
        {
            SdrObjListIter aIter(pList, SdrIterMode::DeepWithGroups);
            while (aIter.IsMore())
            {
                svx::theme::updateSdrObject(mrColorSet, aIter.Next(), pView, pManager);
            }
        }
    }
};

bool changeOverlineColor(SwAttrSet const& rSet, SfxItemSet& rNewSet,
                         model::ColorSet const& rColorSet)
{
    SvxOverlineItem aItem(rSet.Get(RES_CHRATR_OVERLINE, false));
    model::ComplexColor const& rComplexColor = aItem.getComplexColor();
    if (!rComplexColor.isValidThemeType())
        return false;
    Color aColor = rColorSet.resolveColor(rComplexColor);
    aItem.SetColor(aColor);
    rNewSet.Put(aItem);
    return true;
}

bool changeUnderlineColor(SwAttrSet const& rSet, SfxItemSet& rNewSet,
                          model::ColorSet const& rColorSet)
{
    SvxUnderlineItem aItem(rSet.Get(RES_CHRATR_UNDERLINE, false));
    model::ComplexColor const& rComplexColor = aItem.getComplexColor();
    if (!rComplexColor.isValidThemeType())
        return false;
    Color aColor = rColorSet.resolveColor(rComplexColor);
    aItem.SetColor(aColor);
    rNewSet.Put(aItem);
    return true;
}

bool changeColor(SwAttrSet const& rSet, SfxItemSet& rNewSet, model::ColorSet const& rColorSet)
{
    SvxColorItem aColorItem(rSet.GetColor(false));
    model::ComplexColor const& rComplexColor = aColorItem.getComplexColor();
    if (!rComplexColor.isValidThemeType())
        return false;
    Color aColor = rColorSet.resolveColor(rComplexColor);
    aColorItem.SetValue(aColor);
    rNewSet.Put(aColorItem);
    return true;
}

bool changeBackground(SwAttrSet const& rSet, SfxItemSet& rNewSet, model::ColorSet const& rColorSet)
{
    XFillColorItem aFillItem(rSet.Get(XATTR_FILLCOLOR, false));
    model::ComplexColor const& rComplexColor = aFillItem.getComplexColor();
    if (!rComplexColor.isValidThemeType())
        return false;
    Color aColor = rColorSet.resolveColor(rComplexColor);
    aFillItem.SetColorValue(aColor);
    rNewSet.Put(aFillItem);
    return true;
}

bool changeBox(SwAttrSet const& rSet, SfxItemSet& rNewSet, model::ColorSet const& rColorSet)
{
    SvxBoxItem aBoxItem(rSet.GetBox(false));
    bool bChange = false;

    bChange = changeBorderLine(aBoxItem.GetBottom(), rColorSet) || bChange;
    bChange = changeBorderLine(aBoxItem.GetTop(), rColorSet) || bChange;
    bChange = changeBorderLine(aBoxItem.GetLeft(), rColorSet) || bChange;
    bChange = changeBorderLine(aBoxItem.GetRight(), rColorSet) || bChange;

    if (bChange)
        rNewSet.Put(aBoxItem);
    return bChange;
}

} // end anonymous namespace

ThemeColorChanger::ThemeColorChanger(SwDocShell* pDocSh)
    : mpDocSh(pDocSh)
{
}

ThemeColorChanger::~ThemeColorChanger() = default;

void ThemeColorChanger::doApply(std::shared_ptr<model::ColorSet> const& pColorSet)
{
    SwDoc* pDocument = mpDocSh->GetDoc();
    if (!pDocument)
        return;

    pDocument->GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);

    SdrModel* pModel = pDocument->getIDocumentDrawModelAccess().GetDrawModel();

    auto pTheme = pModel->getTheme();
    if (!pTheme)
    {
        pTheme = std::make_shared<model::Theme>("Office");
        pModel->setTheme(pTheme);
    }

    std::shared_ptr<model::ColorSet> pNewColorSet = pColorSet;
    std::shared_ptr<model::ColorSet> pOldColorSet = pTheme->getColorSet();
    pTheme->setColorSet(pNewColorSet);

    auto pUndoThemeChange
        = std::make_unique<sw::UndoThemeChange>(*pDocument, pOldColorSet, pNewColorSet);
    pDocument->GetIDocumentUndoRedo().AppendUndo(std::move(pUndoThemeChange));

    // Page styles
    for (size_t nIndex = 0; nIndex < pDocument->GetPageDescCnt(); ++nIndex)
    {
        auto& rPageDesc = pDocument->GetPageDesc(nIndex);
        SwFrameFormat& rPageFormat = rPageDesc.GetMaster();
        const SwAttrSet& rAttrSet = rPageFormat.GetAttrSet();
        SwAttrSet aNewSet = rAttrSet.CloneAsValue();

        bool bChanged = false;
        bChanged = bChanged || changeBackground(rAttrSet, aNewSet, *pColorSet);
        bChanged = bChanged || changeBox(rAttrSet, aNewSet, *pColorSet);

        if (bChanged)
        {
            rPageFormat.SetFormatAttr(aNewSet);
            pDocument->ChgPageDesc(nIndex, rPageDesc);
        }
    }

    SfxStyleSheetBasePool* pPool = mpDocSh->GetStyleSheetPool();
    SwDocStyleSheet* pStyle;

    // Frame style color change
    pStyle = static_cast<SwDocStyleSheet*>(pPool->First(SfxStyleFamily::Frame));
    while (pStyle)
    {
        SwFrameFormat* pFrameFormat = pStyle->GetFrameFormat();
        if (pFrameFormat)
        {
            const SwAttrSet& rAttrSet = pFrameFormat->GetAttrSet();
            SwAttrSet aNewSet = rAttrSet.CloneAsValue();

            bool bChanged = false;
            bChanged = changeBackground(rAttrSet, aNewSet, *pColorSet) || bChanged;
            bChanged = changeBox(rAttrSet, aNewSet, *pColorSet) || bChanged;

            if (bChanged)
                pDocument->ChgFormat(*pFrameFormat, aNewSet);
        }
        pStyle = static_cast<SwDocStyleSheet*>(pPool->Next());
    }

    // Paragraph style color change
    pStyle = static_cast<SwDocStyleSheet*>(pPool->First(SfxStyleFamily::Para));
    while (pStyle)
    {
        SwTextFormatColl* pTextFormatCollection = pStyle->GetCollection();
        if (pTextFormatCollection)
        {
            const SwAttrSet& rAttrSet = pTextFormatCollection->GetAttrSet();
            SwAttrSet aNewSet = rAttrSet.CloneAsValue();

            bool bChanged = false;
            bChanged = changeColor(rAttrSet, aNewSet, *pColorSet) || bChanged;
            bChanged = changeOverlineColor(rAttrSet, aNewSet, *pColorSet) || bChanged;
            bChanged = changeUnderlineColor(rAttrSet, aNewSet, *pColorSet) || bChanged;
            bChanged = changeBox(rAttrSet, aNewSet, *pColorSet) || bChanged;
            bChanged = changeBackground(rAttrSet, aNewSet, *pColorSet) || bChanged;

            if (bChanged)
            {
                pDocument->ChgFormat(*pTextFormatCollection, aNewSet);
                pPool->Broadcast(SfxStyleSheetHint(SfxHintId::StyleSheetModified, *pStyle));
            }
        }
        pStyle = static_cast<SwDocStyleSheet*>(pPool->Next());
    }

    // Character style color change
    pStyle = static_cast<SwDocStyleSheet*>(pPool->First(SfxStyleFamily::Char));
    while (pStyle)
    {
        SwCharFormat* pCharFormat = pStyle->GetCharFormat();
        if (pCharFormat)
        {
            const SwAttrSet& rAttrSet = pCharFormat->GetAttrSet();
            SwAttrSet aNewSet = rAttrSet.CloneAsValue();

            bool bChanged = false;
            bChanged = changeColor(rAttrSet, aNewSet, *pColorSet) || bChanged;
            bChanged = changeOverlineColor(rAttrSet, aNewSet, *pColorSet) || bChanged;
            bChanged = changeUnderlineColor(rAttrSet, aNewSet, *pColorSet) || bChanged;
            if (bChanged)
                pDocument->ChgFormat(*pCharFormat, aNewSet);
        }
        pStyle = static_cast<SwDocStyleSheet*>(pPool->Next());
    }

    // Direct format change
    auto pHandler = std::make_shared<ThemeColorHandler>(*pDocument, mpDocSh, *pColorSet);
    sw::ModelTraverser aModelTraverser(pDocument);
    aModelTraverser.addNodeHandler(pHandler);
    aModelTraverser.traverse();

    mpDocSh->Broadcast(SfxHint(SfxHintId::ThemeColorsChanged));

    pDocument->GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
