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
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>
#include <svl/itemset.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/eeitem.hxx>

extern "C" SAL_DLLPUBLIC_EXPORT void makeClassificationEditView(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
{
    rRet = VclPtr<svx::ClassificationEditView>::Create(pParent, WB_BORDER|WB_TABSTOP);
}

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

ClassificationEditView::ClassificationEditView(vcl::Window* pParent, WinBits nBits)
    : Control(pParent, nBits)
{
    EnableRTL(false);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    float fScaleFactor = GetDPIScaleFactor();
    set_width_request(500 * fScaleFactor);
    set_height_request(100 * fScaleFactor);

    SetMapMode(MapMode(MapUnit::MapTwip));
    SetPointer(PointerStyle::Text);
    SetBackground(aBgColor);

    Size aOutputSize(GetOutputSize());
    Size aSize(aOutputSize);
    aSize.setHeight( aSize.Height() * 4 );

    pEdEngine.reset(new ClassificationEditEngine(EditEngine::CreatePool()));
    pEdEngine->SetPaperSize( aSize );
    pEdEngine->SetRefDevice( this );

    pEdEngine->SetControlWord(pEdEngine->GetControlWord() | EEControlBits::MARKFIELDS);

    pEdView.reset(new EditView(pEdEngine.get(), this));
    pEdView->SetOutputArea(tools::Rectangle(Point(0,0), aOutputSize));

    pEdView->SetBackgroundColor(aBgColor);
    pEdEngine->InsertView(pEdView.get());
}

ClassificationEditView::~ClassificationEditView()
{
    disposeOnce();
}

void ClassificationEditView::Resize()
{
    Size aOutputSize(GetOutputSize());
    Size aSize(aOutputSize);
    aSize.setHeight( aSize.Height() * 4 );
    pEdEngine->SetPaperSize(aSize);
    pEdView->SetOutputArea(tools::Rectangle(Point(0,0), aOutputSize));
    Control::Resize();
}

void ClassificationEditView::InsertField(const SvxFieldItem& rFieldItem)
{
    pEdView->InsertField(rFieldItem);
    pEdView->Invalidate();
}

void ClassificationEditView::InvertSelectionWeight()
{
    ESelection aSelection = pEdView->GetSelection();

    for (sal_Int32 nParagraph = aSelection.nStartPara; nParagraph <= aSelection.nEndPara; ++nParagraph)
    {
        FontWeight eFontWeight = WEIGHT_BOLD;

        std::unique_ptr<SfxItemSet> pSet(new SfxItemSet(pEdEngine->GetParaAttribs(nParagraph)));
        if (const SfxPoolItem* pItem = pSet->GetItem(EE_CHAR_WEIGHT, false))
        {
            const SvxWeightItem* pWeightItem = dynamic_cast<const SvxWeightItem*>(pItem);
            if (pWeightItem && pWeightItem->GetWeight() == WEIGHT_BOLD)
                eFontWeight = WEIGHT_NORMAL;
        }
        SvxWeightItem aWeight(eFontWeight, EE_CHAR_WEIGHT);
        pSet->Put(aWeight);
        pEdEngine->SetParaAttribs(nParagraph, *pSet);
    }

    pEdView->Invalidate();
}

void ClassificationEditView::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    pEdView->SetBackgroundColor(aBgColor);

    SetBackground(aBgColor);

    Control::Paint(rRenderContext, rRect);

    pEdView->Paint(rRect);

    if (HasFocus())
        pEdView->ShowCursor();
}

void ClassificationEditView::MouseMove(const MouseEvent& rMEvt)
{
    pEdView->MouseMove(rMEvt);
}

void ClassificationEditView::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (!HasFocus())
        GrabFocus();

    pEdView->MouseButtonDown(rMEvt);
}

void ClassificationEditView::MouseButtonUp(const MouseEvent& rMEvt)
{
    pEdView->MouseButtonUp(rMEvt);
}

void ClassificationEditView::KeyInput(const KeyEvent& rKEvt)
{
    sal_uInt16 nKey =  rKEvt.GetKeyCode().GetModifier() + rKEvt.GetKeyCode().GetCode();

    if (nKey == KEY_TAB || nKey == KEY_TAB + KEY_SHIFT)
    {
        Control::KeyInput( rKEvt );
    }
    else if (!pEdView->PostKeyEvent(rKEvt))
    {
        Control::KeyInput(rKEvt);
    }
}

void ClassificationEditView::Command(const CommandEvent& rCEvt)
{
    pEdView->Command(rCEvt);
}

void ClassificationEditView::GetFocus()
{
    pEdView->ShowCursor();

    Control::GetFocus();
}

} // end sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
