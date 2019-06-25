/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svx/ClassificationEditView.hxx>
#include <svx/ClassificationField.hxx>

#include <vcl/builder.hxx>
#include <vcl/cursor.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>
#include <svl/itemset.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdr/overlay/overlayselection.hxx>

namespace svx {

ClassificationEditEngine::ClassificationEditEngine(SfxItemPool* pItemPool)
    : EditEngine(pItemPool)
{}

OUString ClassificationEditEngine::CalcFieldValue(const SvxFieldItem& rField, sal_Int32 /*nPara*/,
                                                  sal_Int32 /*nPos*/, boost::optional<Color>& /*rTxtColor*/, boost::optional<Color>& /*rFldColor*/)
{
    OUString aString;
    const ClassificationField* pClassificationField = dynamic_cast<const ClassificationField*>(rField.GetField());
    if (pClassificationField)
        aString = pClassificationField->msDescription;
    else
        aString = "Unknown";
    return aString;
}

ClassificationEditView::ClassificationEditView()
{
}

void ClassificationEditView::makeEditEngine()
{
    m_xEditEngine.reset(new ClassificationEditEngine(EditEngine::CreatePool()));
}

ClassificationEditView::~ClassificationEditView()
{
}

void ClassificationEditView::InsertField(const SvxFieldItem& rFieldItem)
{
    m_xEditView->InsertField(rFieldItem);
    m_xEditView->Invalidate();
}

void ClassificationEditView::InvertSelectionWeight()
{
    ESelection aSelection = m_xEditView->GetSelection();

    for (sal_Int32 nParagraph = aSelection.nStartPara; nParagraph <= aSelection.nEndPara; ++nParagraph)
    {
        FontWeight eFontWeight = WEIGHT_BOLD;

        std::unique_ptr<SfxItemSet> pSet(new SfxItemSet(m_xEditEngine->GetParaAttribs(nParagraph)));
        if (const SfxPoolItem* pItem = pSet->GetItem(EE_CHAR_WEIGHT, false))
        {
            const SvxWeightItem* pWeightItem = dynamic_cast<const SvxWeightItem*>(pItem);
            if (pWeightItem && pWeightItem->GetWeight() == WEIGHT_BOLD)
                eFontWeight = WEIGHT_NORMAL;
        }
        SvxWeightItem aWeight(eFontWeight, EE_CHAR_WEIGHT);
        pSet->Put(aWeight);
        m_xEditEngine->SetParaAttribs(nParagraph, *pSet);
    }

    m_xEditView->Invalidate();
}

} // end sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
