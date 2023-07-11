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

#include <sal/config.h>

#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/theme/Theme.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>
#include <svx/svditer.hxx>

#include <undodraw.hxx>
#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <scitems.hxx>
#include <scresid.hxx>
#include <globstr.hrc>
#include <document.hxx>
#include <address.hxx>
#include <dociter.hxx>
#include <tabvwsh.hxx>
#include <undostyl.hxx>
#include <undoblk.hxx>

#include <undo/UndoThemeChange.hxx>

namespace sc
{
ThemeColorChanger::ThemeColorChanger(ScDocShell& rDocShell)
    : m_rDocShell(rDocShell)
{
}

ThemeColorChanger::~ThemeColorChanger() = default;

namespace
{
bool changeBorderLine(editeng::SvxBorderLine* pBorderLine, model::ColorSet const& rColorSet)
{
    if (!pBorderLine)
        return false;

    model::ComplexColor const& rComplexColor = pBorderLine->getComplexColor();
    if (rComplexColor.meType == model::ColorType::Scheme)
    {
        auto eThemeType = rComplexColor.meSchemeType;

        if (eThemeType != model::ThemeColorType::Unknown)
        {
            Color aColor = rColorSet.resolveColor(rComplexColor);
            pBorderLine->SetColor(aColor);
            return true;
        }
    }
    return false;
}

bool changeCellItems(SfxItemSet& rItemSet, model::ColorSet const& rColorSet)
{
    const SfxPoolItem* pItem = nullptr;
    bool bChanged = false;
    if (rItemSet.HasItem(ATTR_FONT_COLOR, &pItem))
    {
        auto const* pColorItem = static_cast<const SvxColorItem*>(pItem);
        model::ComplexColor const& rComplexColor = pColorItem->getComplexColor();
        if (rComplexColor.meType == model::ColorType::Scheme)
        {
            auto eThemeType = rComplexColor.meSchemeType;
            if (eThemeType != model::ThemeColorType::Unknown)
            {
                Color aColor = rColorSet.getColor(eThemeType);
                aColor = rComplexColor.applyTransformations(aColor);

                SvxColorItem aColorItem(*pColorItem);
                aColorItem.setColor(aColor);
                rItemSet.Put(aColorItem);
                bChanged = true;
            }
        }
    }
    if (rItemSet.HasItem(ATTR_BACKGROUND, &pItem))
    {
        auto const* pBrushItem = static_cast<const SvxBrushItem*>(pItem);
        model::ComplexColor const& rComplexColor = pBrushItem->getComplexColor();
        if (rComplexColor.meType == model::ColorType::Scheme)
        {
            auto eThemeType = rComplexColor.meSchemeType;
            if (eThemeType != model::ThemeColorType::Unknown)
            {
                Color aColor = rColorSet.getColor(eThemeType);
                aColor = rComplexColor.applyTransformations(aColor);

                SvxBrushItem aNewBrushItem(*pBrushItem);
                aNewBrushItem.SetColor(aColor);
                rItemSet.Put(aNewBrushItem);
                bChanged = true;
            }
        }
    }
    if (rItemSet.HasItem(ATTR_BORDER, &pItem))
    {
        auto const* pBoxItem = static_cast<const SvxBoxItem*>(pItem);
        SvxBoxItem rNewItem(*pBoxItem);
        bool bLineChanged = false;

        bLineChanged = changeBorderLine(rNewItem.GetBottom(), rColorSet) || bChanged;
        bLineChanged = changeBorderLine(rNewItem.GetTop(), rColorSet) || bChanged;
        bLineChanged = changeBorderLine(rNewItem.GetLeft(), rColorSet) || bChanged;
        bLineChanged = changeBorderLine(rNewItem.GetRight(), rColorSet) || bChanged;

        if (bLineChanged)
        {
            rItemSet.Put(rNewItem);
            bChanged = true;
        }
    }
    return bChanged;
}

bool changeStyles(ScDocShell& rDocShell, std::shared_ptr<model::ColorSet> const& pColorSet)
{
    ScDocument& rDocument = rDocShell.GetDocument();
    ScStyleSheetPool* pPool = rDocument.GetStyleSheetPool();
    ScStyleSheet* pStyle = nullptr;
    bool bChanged = false;

    // Paragraph style color change
    pStyle = static_cast<ScStyleSheet*>(pPool->First(SfxStyleFamily::Para));
    while (pStyle)
    {
        ScStyleSaveData aOldData;
        aOldData.InitFromStyle(pStyle);

        auto rItemSet = pStyle->GetItemSet();
        if (changeCellItems(rItemSet, *pColorSet))
        {
            if (rDocument.IsUndoEnabled())
            {
                ScStyleSaveData aNewData;
                aNewData.InitFromStyle(pStyle);
                rDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoModifyStyle>(
                    &rDocShell, SfxStyleFamily::Para, aOldData, aNewData));
            }
            static_cast<SfxStyleSheet*>(pStyle)->Broadcast(SfxHint(SfxHintId::DataChanged));
            bChanged = true;
        }

        pStyle = static_cast<ScStyleSheet*>(pPool->Next());
    }

    return bChanged;
}

bool changeSheets(ScDocShell& rDocShell, ScDrawLayer* pModel,
                  std::shared_ptr<model::ColorSet> const& pColorSet)
{
    ScDocument& rDocument = rDocShell.GetDocument();
    bool bChanged = false;

    for (SCTAB nTab = 0; nTab < rDocument.GetTableCount(); nTab++)
    {
        // Change Cell / Text attributes
        {
            ScDocAttrIterator aAttributeIterator(rDocument, nTab, 0, 0, rDocument.MaxCol(),
                                                 rDocument.MaxRow());
            SCCOL nCol = 0;
            SCROW nRow1 = 0;
            SCROW nRow2 = 0;

            while (const ScPatternAttr* pPattern = aAttributeIterator.GetNext(nCol, nRow1, nRow2))
            {
                if (!pPattern || !pPattern->IsVisible())
                    continue;

                ScPatternAttr aNewPattern(*pPattern);
                auto& rItemSet = aNewPattern.GetItemSet();
                bool bItemChanged = changeCellItems(rItemSet, *pColorSet);
                bChanged = bChanged || bItemChanged;

                if (bItemChanged && rDocument.IsUndoEnabled())
                {
                    ScRange aRange(nCol, nRow1, nTab, nCol, nRow2, nTab);
                    ScMarkData aMark(rDocument.GetSheetLimits());
                    aMark.SetMarkArea(aRange);

                    ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
                    pUndoDoc->InitUndo(rDocument, nTab, nTab);
                    pUndoDoc->AddUndoTab(nTab, nTab);

                    aMark.MarkToMulti();

                    rDocument.CopyToDocument(aRange, InsertDeleteFlags::ATTRIB, true, *pUndoDoc,
                                             &aMark);

                    auto pUndo = std::make_unique<ScUndoSelectionAttr>(
                        &rDocShell, aMark, aRange.aStart.Col(), aRange.aStart.Row(),
                        aRange.aStart.Tab(), aRange.aEnd.Col(), aRange.aEnd.Row(),
                        aRange.aEnd.Tab(), std::move(pUndoDoc), true, &aNewPattern);

                    ScEditDataArray* pDataArray = pUndo->GetDataArray();

                    rDocument.ApplySelectionPattern(aNewPattern, aMark, pDataArray);

                    rDocShell.GetUndoManager()->AddUndoAction(std::move(pUndo));
                }
            }
        }

        // Change all SdrObjects
        {
            pModel->BeginCalcUndo(true);

            SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
            SdrObjListIter aIter(pPage, SdrIterMode::DeepNoGroups);
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                svx::theme::updateSdrObject(*pColorSet, pObject);
                pObject = aIter.Next();
            }

            std::unique_ptr<SdrUndoGroup> pUndo = pModel->GetCalcUndo();

            if (pUndo)
            {
                bChanged = true;
                pUndo->SetComment("Hi!");
                auto pUndoDraw = std::make_unique<ScUndoDraw>(std::move(pUndo), &rDocShell);
                rDocShell.GetUndoManager()->AddUndoAction(std::move(pUndoDraw));
            }
        }
    }

    return bChanged;
}

void changeTheTheme(ScDocShell& rDocShell, std::shared_ptr<model::ColorSet> const& pColorSet)
{
    ScDocument& rDocument = rDocShell.GetDocument();
    ScDrawLayer* pModel = rDocument.GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(0);

    auto pTheme = pPage->getSdrPageProperties().GetTheme();
    if (!pTheme)
    {
        pTheme = std::make_shared<model::Theme>("Office");
        pPage->getSdrPageProperties().SetTheme(pTheme);
    }

    std::shared_ptr<model::ColorSet> pNewColorSet = pColorSet;
    std::shared_ptr<model::ColorSet> pOldColorSet = pTheme->getColorSet();
    pTheme->setColorSet(pNewColorSet);

    if (rDocument.IsUndoEnabled())
    {
        auto pUndoThemeChange
            = std::make_unique<sc::UndoThemeChange>(rDocShell, pOldColorSet, pNewColorSet);
        rDocShell.GetUndoManager()->AddUndoAction(std::move(pUndoThemeChange));
    }
}

} // end anonymous ns

void ThemeColorChanger::apply(std::shared_ptr<model::ColorSet> const& pColorSet)
{
    m_rDocShell.MakeDrawLayer();

    ScDocShellModificator aModificator(m_rDocShell);
    ScDocument& rDocument = m_rDocShell.GetDocument();
    auto pUndoManager = m_rDocShell.GetUndoManager();

    const bool bUndo(rDocument.IsUndoEnabled());

    ViewShellId nViewShellId(-1);
    if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
        nViewShellId = pViewSh->GetViewShellId();

    if (bUndo)
    {
        OUString aUndo = ScResId(STR_UNDO_THEME_COLOR_CHANGE);
        pUndoManager->EnterListAction(aUndo, aUndo, 0, nViewShellId);
    }

    bool bChanged = false;
    bChanged = changeStyles(m_rDocShell, pColorSet) || bChanged;
    bChanged = changeSheets(m_rDocShell, rDocument.GetDrawLayer(), pColorSet) || bChanged;

    changeTheTheme(m_rDocShell, pColorSet);

    if (bUndo)
    {
        pUndoManager->LeaveListAction();
    }

    m_rDocShell.SetDrawModified();
    aModificator.SetDocumentModified();
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
