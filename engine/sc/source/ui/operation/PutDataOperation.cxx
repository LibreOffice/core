/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/PutDataOperation.hxx>

#include <scitems.hxx>
#include <docfunc.hxx>
#include <docsh.hxx>
#include <address.hxx>
#include <document.hxx>
#include <editutil.hxx>
#include <markdata.hxx>
#include <patattr.hxx>
#include <rangelst.hxx>

#include <editeng/editobj.hxx>

#include <memory>
#include <vector>

namespace sc
{
namespace
{
struct RememberItem
{
    sal_Int32 nIndex;
    SfxItemSet aItemSet;

    RememberItem(SfxItemSet aSet, sal_Int32 nTempIndex)
        : nIndex(nTempIndex)
        , aItemSet(std::move(aSet))
    {
    }
};
} // end anonymous namespace

PutDataOperation::PutDataOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                   const ScAddress& rPosition, ScEditEngineDefaulter& rEngine,
                                   bool bApi)
    : Operation(OperationType::PutData, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrPosition(rPosition)
    , mrEngine(rEngine)
{
}

bool PutDataOperation::runImplementation()
{
    bool bRet = false;
    ScDocument& rDoc = mrDocShell.GetDocument();
    ScEditAttrTester aTester(&mrEngine);
    bool bEditCell = aTester.NeedsObject();
    if (bEditCell)
    {
        // #i61702# With bLoseContent set, the content of mrEngine isn't restored
        // (used in loading XML, where after the removeActionLock call the API object's
        // EditEngine isn't accessed again.
        bool bLoseContent = rDoc.IsImportingXML();

        const bool bUpdateMode = mrEngine.SetUpdateLayout(false);

        std::vector<std::unique_ptr<RememberItem>> aRememberItems;

        //  All paragraph attributes must be removed before calling CreateTextObject,
        //  not only alignment, so the object doesn't contain the cell attributes as
        //  paragraph attributes. Before removing the attributes store them in a vector to
        //  set them back to the EditEngine.
        sal_Int32 nCount = mrEngine.GetParagraphCount();
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            const SfxItemSet& rOld = mrEngine.GetParaAttribs(i);
            if (rOld.Count())
            {
                if (!bLoseContent)
                {
                    aRememberItems.push_back(
                        std::make_unique<RememberItem>(mrEngine.GetParaAttribs(i), i));
                }
                mrEngine.SetParaAttribs(i, SfxItemSet(*rOld.GetPool(), rOld.GetRanges()));
            }
        }

        // A copy of pNewData will be stored in the cell.
        std::unique_ptr<EditTextObject> pNewData(mrEngine.CreateTextObject());
        bRet = mrDocFunc.SetEditCell(mrPosition, *pNewData, !mbApi);

        // Set the paragraph attributes back to the EditEngine.
        for (const auto& rxItem : aRememberItems)
        {
            mrEngine.SetParaAttribs(rxItem->nIndex, rxItem->aItemSet);
        }

        // #i61702# if the content isn't accessed, there's no need to set the UpdateMode again
        if (bUpdateMode && !bLoseContent)
            mrEngine.SetUpdateLayout(true);
    }
    else
    {
        OUString aText = mrEngine.GetText();
        if (aText.isEmpty())
        {
            bool bNumFmtSet = false;
            bRet = mrDocFunc.SetNormalString(bNumFmtSet, mrPosition, aText, mbApi);
        }
        else
            bRet = mrDocFunc.SetStringCell(mrPosition, aText, !mbApi);
    }

    if (!(bRet && aTester.NeedsCellAttr()))
        return bRet;

    const SfxItemSet& rEditAttr = aTester.GetAttribs();
    ScPatternAttr aPattern(rDoc.getCellAttributeHelper());
    aPattern.GetFromEditItemSet(&rEditAttr);
    aPattern.DeleteUnchanged(rDoc.GetPattern(mrPosition.Col(), mrPosition.Row(), mrPosition.Tab()));
    aPattern.ItemSetClearItem(ATTR_HOR_JUSTIFY); // wasn't removed above if no edit object
    if (aPattern.GetItemSet().Count() > 0)
    {
        ScMarkData aMark(rDoc.GetSheetLimits());
        aMark.SelectTable(mrPosition.Tab(), true);
        aMark.SetMarkArea(ScRange(mrPosition));
        mrDocFunc.ApplyAttributes(aMark, aPattern, mbApi);
    }

    return bRet;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
