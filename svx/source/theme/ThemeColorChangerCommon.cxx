/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/lok.hxx>

#include <docmodel/theme/ColorSet.hxx>

#include <editeng/colritem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/section.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <sal/config.h>

#include <sfx2/lokhelper.hxx>

#include <svx/PaletteManager.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdundo.hxx>
#include <svx/theme/ThemeColorChangerCommon.hxx>
#include <svx/theme/ThemeColorPaletteManager.hxx>
#include <svx/xdef.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflclit.hxx>
#include <tools/json_writer.hxx>

using namespace css;

namespace svx::theme
{
namespace
{
const SvxColorItem* getColorItem(const editeng::Section& rSection)
{
    auto iterator = std::find_if(
        rSection.maAttributes.begin(), rSection.maAttributes.end(),
        [](const SfxPoolItem* pPoolItem) { return pPoolItem->Which() == EE_CHAR_COLOR; });

    if (iterator != rSection.maAttributes.end())
        return static_cast<const SvxColorItem*>(*iterator);
    return nullptr;
}

bool updateEditEngTextSections(model::ColorSet const& rColorSet, SdrObject* pObject, SdrView& rView)
{
    SdrTextObj* pTextObject = DynCastSdrTextObj(pObject);

    if (!pTextObject)
        return false;

    rView.SdrBeginTextEdit(pTextObject);

    auto* pOutlinerView = rView.GetTextEditOutlinerView();
    if (!pOutlinerView)
        return false;

    auto* pEditEngine = pOutlinerView->GetEditView().GetEditEngine();
    if (!pEditEngine)
        return false;

    OutlinerParaObject* pOutlinerParagraphObject = pTextObject->GetOutlinerParaObject();
    if (pOutlinerParagraphObject)
    {
        const EditTextObject& rEditText = pOutlinerParagraphObject->GetTextObject();
        std::vector<editeng::Section> aSections;
        rEditText.GetAllSections(aSections);

        for (editeng::Section const& rSection : aSections)
        {
            const SvxColorItem* pItem = getColorItem(rSection);
            if (!pItem)
                continue;

            model::ComplexColor const& rComplexColor = pItem->getComplexColor();
            if (rComplexColor.isValidThemeType())
            {
                SfxItemSet aSet(pEditEngine->GetAttribs(rSection.mnParagraph, rSection.mnStart,
                                                        rSection.mnEnd,
                                                        GetAttribsFlags::CHARATTRIBS));
                Color aNewColor = rColorSet.resolveColor(rComplexColor);
                std::unique_ptr<SvxColorItem> pNewItem(pItem->Clone());
                pNewItem->setColor(aNewColor);
                aSet.Put(*pNewItem);

                ESelection aSelection(rSection.mnParagraph, rSection.mnStart, rSection.mnParagraph,
                                      rSection.mnEnd);
                pEditEngine->QuickSetAttribs(aSet, aSelection);
            }
        }
    }

    rView.SdrEndTextEdit();

    return true;
}

bool updateObjectAttributes(model::ColorSet const& rColorSet, SdrObject& rObject,
                            SfxUndoManager* pUndoManager)
{
    bool bChanged = false;

    auto aItemSet = rObject.GetMergedItemSet();

    if (const XFillColorItem* pItem = aItemSet.GetItemIfSet(XATTR_FILLCOLOR, false))
    {
        model::ComplexColor const& rComplexColor = pItem->getComplexColor();
        if (rComplexColor.isValidThemeType())
        {
            Color aNewColor = rColorSet.resolveColor(rComplexColor);
            std::unique_ptr<XFillColorItem> pNewItem(pItem->Clone());
            pNewItem->SetColorValue(aNewColor);
            aItemSet.Put(*pNewItem);
            bChanged = true;
        }
    }
    if (const XLineColorItem* pItem = aItemSet.GetItemIfSet(XATTR_LINECOLOR, false))
    {
        model::ComplexColor const& rComplexColor = pItem->getComplexColor();
        if (rComplexColor.isValidThemeType())
        {
            Color aNewColor = rColorSet.resolveColor(rComplexColor);
            std::unique_ptr<XLineColorItem> pNewItem(pItem->Clone());
            pNewItem->SetColorValue(aNewColor);
            aItemSet.Put(*pNewItem);
            bChanged = true;
        }
    }
    if (const SvxColorItem* pItem = aItemSet.GetItemIfSet(EE_CHAR_COLOR, false))
    {
        model::ComplexColor const& rComplexColor = pItem->getComplexColor();
        if (rComplexColor.isValidThemeType())
        {
            Color aNewColor = rColorSet.resolveColor(rComplexColor);
            std::unique_ptr<SvxColorItem> pNewItem(pItem->Clone());
            pNewItem->setColor(aNewColor);
            aItemSet.Put(*pNewItem);
            bChanged = true;
        }
    }
    if (bChanged)
    {
        const bool bUndo = pUndoManager && pUndoManager->IsInListAction();
        if (bUndo)
        {
            pUndoManager->AddUndoAction(
                rObject.getSdrModelFromSdrObject().GetSdrUndoFactory().CreateUndoAttrObject(
                    rObject));
        }
        rObject.SetMergedItemSetAndBroadcast(aItemSet);
    }
    return bChanged;
}

} // end anonymous namespace

/// Updates properties of the SdrObject
void updateSdrObject(model::ColorSet const& rColorSet, SdrObject* pObject, SdrView* pView,
                     SfxUndoManager* pUndoManager)
{
    if (!pObject)
        return;
    updateObjectAttributes(rColorSet, *pObject, pUndoManager);
    if (pView)
        updateEditEngTextSections(rColorSet, pObject, *pView);
}

void notifyLOK(std::shared_ptr<model::ColorSet> const& pColorSet,
               const std::set<Color>& rDocumentColors)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        svx::ThemeColorPaletteManager aManager(pColorSet);
        tools::JsonWriter aTree;

        if (pColorSet)
            aManager.generateJSON(aTree);
        if (rDocumentColors.size())
            PaletteManager::generateJSON(aTree, rDocumentColors);

        SfxLokHelper::notifyAllViews(LOK_CALLBACK_COLOR_PALETTES, aTree.finishAndGetAsOString());
    }
}

} // end svx::theme namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
