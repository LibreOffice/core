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
#include <docfunc.hxx>
#include <tabvwsh.hxx>
#include <undostyl.hxx>
#include <undoblk.hxx>
#include <SparklineGroup.hxx>
#include <SparklineList.hxx>

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
    if (rComplexColor.isValidThemeType())
    {
        Color aColor = rColorSet.resolveColor(rComplexColor);
        pBorderLine->SetColor(aColor);
        return true;
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
        if (rComplexColor.isValidThemeType())
        {
            Color aColor = rColorSet.resolveColor(rComplexColor);

            SvxColorItem aColorItem(*pColorItem);
            aColorItem.setColor(aColor);
            rItemSet.Put(aColorItem);
            bChanged = true;
        }
    }
    if (rItemSet.HasItem(ATTR_BACKGROUND, &pItem))
    {
        auto const* pBrushItem = static_cast<const SvxBrushItem*>(pItem);
        model::ComplexColor const& rComplexColor = pBrushItem->getComplexColor();
        if (rComplexColor.isValidThemeType())
        {
            Color aColor = rColorSet.resolveColor(rComplexColor);

            SvxBrushItem aNewBrushItem(*pBrushItem);
            aNewBrushItem.SetColor(aColor);
            rItemSet.Put(aNewBrushItem);
            bChanged = true;
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

bool changeStyles(ScDocShell& rDocShell, model::ColorSet const& rColorSet)
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
        if (changeCellItems(rItemSet, rColorSet))
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

bool changeSheets(ScDocShell& rDocShell, ScTabViewShell* pViewShell, ScDrawLayer* pModel,
                  model::ColorSet const& rColorSet)
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
                bool bItemChanged = changeCellItems(rItemSet, rColorSet);
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
            SdrView* pView = nullptr;
            if (pViewShell)
                pView = pViewShell->GetScDrawView();

            SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
            SdrObjListIter aIter(pPage, SdrIterMode::DeepNoGroups);
            for (SdrObject* pObject = aIter.Next(); pObject; pObject = aIter.Next())
            {
                svx::theme::updateSdrObject(rColorSet, pObject, pView, rDocShell.GetUndoManager());
            }

            std::unique_ptr<SdrUndoGroup> pUndo = pModel->GetCalcUndo();

            if (pUndo)
            {
                bChanged = true;
                auto pUndoDraw = std::make_unique<ScUndoDraw>(std::move(pUndo), &rDocShell);
                rDocShell.GetUndoManager()->AddUndoAction(std::move(pUndoDraw));
            }
        }
    }

    return bChanged;
}

model::ComplexColor modifyComplexColor(model::ComplexColor const& rComplexColor,
                                       model::ColorSet const& rColorSet)
{
    model::ComplexColor aComplexColor(rComplexColor);

    if (aComplexColor.isValidThemeType())
    {
        Color aColor = rColorSet.resolveColor(aComplexColor);
        aComplexColor.setFinalColor(aColor);
    }
    return aComplexColor;
}

void changeSparklines(ScDocShell& rDocShell, model::ColorSet const& rColorSet)
{
    ScDocument& rDocument = rDocShell.GetDocument();
    auto& rDocFunc = rDocShell.GetDocFunc();
    for (SCTAB nTab = 0; nTab < rDocument.GetTableCount(); ++nTab)
    {
        auto* pSparklineList = rDocument.GetSparklineList(nTab);
        if (pSparklineList && !pSparklineList->getSparklineGroups().empty())
        {
            auto const& rSparklineGroups = pSparklineList->getSparklineGroups();
            for (auto const& rSparklineGroup : rSparklineGroups)
            {
                auto aAttributes = rSparklineGroup->getAttributes();

                aAttributes.setColorAxis(modifyComplexColor(aAttributes.getColorAxis(), rColorSet));
                aAttributes.setColorSeries(
                    modifyComplexColor(aAttributes.getColorSeries(), rColorSet));
                aAttributes.setColorNegative(
                    modifyComplexColor(aAttributes.getColorNegative(), rColorSet));
                aAttributes.setColorMarkers(
                    modifyComplexColor(aAttributes.getColorMarkers(), rColorSet));
                aAttributes.setColorHigh(modifyComplexColor(aAttributes.getColorHigh(), rColorSet));
                aAttributes.setColorLow(modifyComplexColor(aAttributes.getColorLow(), rColorSet));
                aAttributes.setColorFirst(
                    modifyComplexColor(aAttributes.getColorFirst(), rColorSet));
                aAttributes.setColorLast(modifyComplexColor(aAttributes.getColorLast(), rColorSet));
                rDocFunc.ChangeSparklineGroupAttributes(rSparklineGroup, aAttributes);
            }
        }
    }
}

std::shared_ptr<model::Theme> getTheme(ScDocShell& rDocShell)
{
    ScDrawLayer* pModel = rDocShell.GetDocument().GetDrawLayer();

    auto pTheme = pModel->getTheme();
    if (!pTheme)
    {
        pTheme = std::make_shared<model::Theme>("Office");
        pModel->setTheme(pTheme);
    }
    return pTheme;
}

void changeThemeColorInTheDocModel(ScDocShell& rDocShell,
                                   std::shared_ptr<model::ColorSet> const& pColorSet)
{
    auto pTheme = getTheme(rDocShell);
    std::shared_ptr<model::ColorSet> pNewColorSet = pColorSet;
    std::shared_ptr<model::ColorSet> pOldColorSet = pTheme->getColorSet();
    pTheme->setColorSet(pNewColorSet);

    ScDocument& rDocument = rDocShell.GetDocument();
    if (rDocument.IsUndoEnabled())
    {
        auto pUndoThemeChange
            = std::make_unique<sc::UndoThemeChange>(rDocShell, pOldColorSet, pNewColorSet);
        rDocShell.GetUndoManager()->AddUndoAction(std::move(pUndoThemeChange));
    }
}

} // end anonymous ns

void ThemeColorChanger::doApply(std::shared_ptr<model::ColorSet> const& pColorSet)
{
    // Can't change to an empty color set
    if (!pColorSet)
        return;

    m_rDocShell.MakeDrawLayer();

    ScDocShellModificator aModificator(m_rDocShell);
    ScDocument& rDocument = m_rDocShell.GetDocument();
    auto pUndoManager = m_rDocShell.GetUndoManager();

    const bool bUndo(rDocument.IsUndoEnabled());

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ViewShellId nViewShellId(-1);
    if (pViewShell)
        nViewShellId = pViewShell->GetViewShellId();

    if (bUndo)
    {
        OUString aUndo = ScResId(STR_UNDO_THEME_COLOR_CHANGE);
        pUndoManager->EnterListAction(aUndo, aUndo, 0, nViewShellId);
    }

    bool bChanged = false;
    bChanged = changeStyles(m_rDocShell, *pColorSet) || bChanged;
    bChanged
        = changeSheets(m_rDocShell, pViewShell, rDocument.GetDrawLayer(), *pColorSet) || bChanged;
    changeSparklines(m_rDocShell, *pColorSet);

    changeThemeColorInTheDocModel(m_rDocShell, pColorSet);

    if (bUndo)
    {
        pUndoManager->LeaveListAction();
    }

    m_rDocShell.SetDrawModified();
    aModificator.SetDocumentModified();
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
