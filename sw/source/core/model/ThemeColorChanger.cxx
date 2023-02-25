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
#include <DocumentContentOperationsManager.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentUndoRedo.hxx>

#include <sal/config.h>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <docmodel/uno/UnoThemeColor.hxx>
#include <docmodel/theme/Theme.hxx>
#include <editeng/unoprnms.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace sw
{
namespace
{
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
    model::ColorSet const& mrColorSet;

public:
    ThemeColorHandler(SwDoc& rDocument, model::ColorSet const& rColorSet)
        : mrDocument(rDocument)
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
            if (pTextAttr->Which() == RES_TXTATR_AUTOFMT)
            {
                SwFormatAutoFormat const& rAutoFormatPool(pTextAttr->GetAutoFormat());
                std::shared_ptr<SfxItemSet> pStyleHandle(rAutoFormatPool.GetStyleHandle());
                if (const SvxColorItem* pItem = pStyleHandle->GetItemIfSet(RES_CHRATR_COLOR))
                {
                    model::ThemeColor const& rThemeColor = pItem->GetThemeColor();
                    auto eThemeType = rThemeColor.getType();
                    if (eThemeType != model::ThemeColorType::Unknown)
                    {
                        Color aNewColor = mrColorSet.resolveColor(rThemeColor);
                        auto pNew = pItem->Clone();
                        pNew->SetValue(aNewColor);

                        SwPaM aPam(*pTextNode, pTextAttr->GetStart(), *pTextNode,
                                   pTextAttr->GetAnyEnd());
                        mrDocument.GetDocumentContentOperationsManager().InsertPoolItem(
                            aPam, *pNew, SetAttrMode::APICALL | SetAttrMode::NO_CURSOR_CHANGE);
                    }
                }
            }
        }
    }

    void handleNode(SwNode* pNode) override
    {
        if (!pNode->IsTextNode())
            return;

        updateHints(pNode->GetTextNode());
    }

    void handleSdrObject(SdrObject* pObject) override
    {
        // update current object
        svx::theme::updateSdrObject(mrColorSet, pObject);

        // update child objects
        SdrObjList* pList = pObject->GetSubList();
        if (pList)
        {
            SdrObjListIter aIter(pList, SdrIterMode::DeepWithGroups);
            while (aIter.IsMore())
            {
                svx::theme::updateSdrObject(mrColorSet, aIter.Next());
            }
        }
    }
};

void changeColor(SwFormat* pFormat, model::ColorSet const& rColorSet, SwDoc* pDocument)
{
    const SwAttrSet& rAttrSet = pFormat->GetAttrSet();
    std::unique_ptr<SfxItemSet> pNewSet = rAttrSet.Clone();

    SvxColorItem aColorItem(rAttrSet.GetColor());
    model::ThemeColor const& rThemeColor = aColorItem.GetThemeColor();
    auto eThemeType = rThemeColor.getType();
    if (eThemeType != model::ThemeColorType::Unknown)
    {
        Color aColor = rColorSet.getColor(eThemeType);
        aColor = rThemeColor.applyTransformations(aColor);
        aColorItem.SetValue(aColor);
        pNewSet->Put(aColorItem);
        pDocument->ChgFormat(*pFormat, *pNewSet);
    }
}

} // end anonymous namespace

ThemeColorChanger::ThemeColorChanger(SwDocShell* pDocSh)
    : mpDocSh(pDocSh)
{
}

ThemeColorChanger::~ThemeColorChanger() = default;

void ThemeColorChanger::apply(model::ColorSet const& rColorSet)
{
    SwDoc* pDocument = mpDocSh->GetDoc();
    if (!pDocument)
        return;

    pDocument->GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);

    SdrPage* pPage = pDocument->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    model::Theme* pTheme = pPage->getSdrPageProperties().GetTheme().get();
    if (pTheme)
    {
        pTheme->SetColorSet(std::make_unique<model::ColorSet>(rColorSet));
    }
    else
    {
        pPage->getSdrPageProperties().SetTheme(std::make_unique<model::Theme>("Office"));
        pTheme = pPage->getSdrPageProperties().GetTheme().get();
        pTheme->SetColorSet(std::make_unique<model::ColorSet>(rColorSet));
    }

    SfxStyleSheetBasePool* pPool = mpDocSh->GetStyleSheetPool();
    SwDocStyleSheet* pStyle;

    // Paragraph style color change
    pStyle = static_cast<SwDocStyleSheet*>(pPool->First(SfxStyleFamily::Para));
    while (pStyle)
    {
        SwTextFormatColl* pTextFormatCollection = pStyle->GetCollection();
        if (pTextFormatCollection)
            changeColor(pTextFormatCollection, rColorSet, pDocument);
        pStyle = static_cast<SwDocStyleSheet*>(pPool->Next());
    }

    // Character style color change
    pStyle = static_cast<SwDocStyleSheet*>(pPool->First(SfxStyleFamily::Char));
    while (pStyle)
    {
        SwCharFormat* pCharFormat = pStyle->GetCharFormat();
        if (pCharFormat)
            changeColor(pCharFormat, rColorSet, pDocument);
        pStyle = static_cast<SwDocStyleSheet*>(pPool->Next());
    }

    // Direct format change
    auto pHandler = std::make_shared<ThemeColorHandler>(*pDocument, rColorSet);
    sw::ModelTraverser aModelTraverser(pDocument);
    aModelTraverser.addNodeHandler(pHandler);
    aModelTraverser.traverse();

    pDocument->GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
