/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "TextPropertyPanel.hrc"
#include "TextPropertyPanel.hxx"

#include <editeng/kernitem.hxx>
#include <editeng/udlnitem.hxx>
#include <rtl/ref.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/unitconv.hxx>

#include <vcl/toolbox.hxx>
#include "TextCharacterSpacingControl.hxx"
#include "TextCharacterSpacingPopup.hxx"
#include "TextUnderlineControl.hxx"
#include "TextUnderlinePopup.hxx"
#include <svx/sidebar/PopupContainer.hxx>

#include <boost/bind.hpp>

using namespace css;
using namespace cssu;

const char UNO_SPACING[] = ".uno:Spacing";
const char UNO_UNDERLINE[] = ".uno:Underline";

namespace svx { namespace sidebar {

PopupControl* TextPropertyPanel::CreateCharacterSpacingControl (PopupContainer* pParent)
{
    return new TextCharacterSpacingControl(pParent, *this, mpBindings);
}

PopupControl* TextPropertyPanel::CreateUnderlinePopupControl (PopupContainer* pParent)
{
    return new TextUnderlineControl(pParent, *this, mpBindings);
}

long TextPropertyPanel::GetSelFontSize()
{
    long nH = 240;
    SfxMapUnit eUnit = maSpacingControl.GetCoreMetric();
    if (mpHeightItem)
        nH = LogicToLogic(  mpHeightItem->GetHeight(), (MapUnit)eUnit, MAP_TWIP );
    return nH;
}

TextPropertyPanel* TextPropertyPanel::Create (
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const ::sfx2::sidebar::EnumContext& rContext)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to TextPropertyPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to TextPropertyPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException("no SfxBindings given to TextPropertyPanel::Create", NULL, 2);

    return new TextPropertyPanel(
        pParent,
        rxFrame,
        pBindings,
        rContext);
}

::sfx2::sidebar::ControllerItem& TextPropertyPanel::GetSpaceController()
{
    return maSpacingControl;
}

TextPropertyPanel::TextPropertyPanel ( Window* pParent, const cssu::Reference<css::frame::XFrame>& rxFrame, SfxBindings* pBindings, const ::sfx2::sidebar::EnumContext& /*rContext*/ )
    : PanelLayout(pParent, "SidebarTextPanel", "svx/ui/sidebartextpanel.ui", rxFrame),
        maFontSizeControl   (SID_ATTR_CHAR_FONTHEIGHT,  *pBindings, *this, OUString("FontHeight"),   rxFrame),
        maUnderlineControl  (SID_ATTR_CHAR_UNDERLINE,   *pBindings, *this, OUString("Underline"),    rxFrame),
        maSpacingControl    (SID_ATTR_CHAR_KERNING,     *pBindings, *this, OUString("Spacing"),      rxFrame),

        maCharSpacePopup(this, ::boost::bind(&TextPropertyPanel::CreateCharacterSpacingControl, this, _1)),
        maUnderlinePopup(this, ::boost::bind(&TextPropertyPanel::CreateUnderlinePopupControl, this, _1)),
        maContext(),
        mpBindings(pBindings)
{
    get(mpToolBoxFont, "fonteffects");
    get(mpToolBoxIncDec, "fontadjust");
    get(mpToolBoxSpacing, "spacingbar");
    get(mpToolBoxFontColorSw, "colorbar");
    get(mpToolBoxFontColor, "colorsingle");

    //toolbox
    SetupToolboxItems();
    InitToolBoxFont();
    InitToolBoxSpacing();

    //init state
    mpHeightItem = NULL;
    meUnderline = UNDERLINE_NONE;
    meUnderlineColor = COL_AUTO;
    mbKernAvailable = true;
    mbKernLBAvailable = true;
    mlKerning = 0;
}

TextPropertyPanel::~TextPropertyPanel (void)
{
}

void TextPropertyPanel::HandleContextChange (
    const ::sfx2::sidebar::EnumContext aContext)
{
    if (maContext == aContext)
        return;

    maContext = aContext;
    mpToolBoxIncDec->Show(maContext.GetApplication_DI() != sfx2::sidebar::EnumContext::Application_Calc);

    bool bWriterText = false;
    switch (maContext.GetCombinedContext_DI())
    {
        case CombinedEnumContext(Application_Calc, Context_Cell):
        case CombinedEnumContext(Application_Calc, Context_Pivot):
            mpToolBoxSpacing->Disable();
            break;

        case CombinedEnumContext(Application_Calc, Context_EditCell):
        case CombinedEnumContext(Application_Calc, Context_DrawText):
        case CombinedEnumContext(Application_WriterVariants, Context_DrawText):
        case CombinedEnumContext(Application_WriterVariants, Context_Annotation):
        case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
        case CombinedEnumContext(Application_DrawImpress, Context_Text):
        case CombinedEnumContext(Application_DrawImpress, Context_Table):
        case CombinedEnumContext(Application_DrawImpress, Context_OutlineText):
        case CombinedEnumContext(Application_DrawImpress, Context_Draw):
        case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
            mpToolBoxSpacing->Enable();
            break;

        case CombinedEnumContext(Application_WriterVariants, Context_Text):
        case CombinedEnumContext(Application_WriterVariants, Context_Table):
            mpToolBoxSpacing->Enable();
            bWriterText = true;
            break;

        default:
            break;
    }

    mpToolBoxFontColor->Show(!bWriterText);
    mpToolBoxFontColorSw->Show(bWriterText);
}

void TextPropertyPanel::DataChanged (const DataChangedEvent& /*rEvent*/)
{
    SetupToolboxItems();
}

void TextPropertyPanel::EndSpacingPopupMode (void)
{
    maCharSpacePopup.Hide();
}

void TextPropertyPanel::EndUnderlinePopupMode (void)
{
    maUnderlinePopup.Hide();
}

void TextPropertyPanel::InitToolBoxFont()
{
    Link aLink = LINK(this, TextPropertyPanel, UnderlineClickHdl);
    mpToolBoxFont->SetDropdownClickHdl(aLink);
}

void TextPropertyPanel::InitToolBoxSpacing()
{
    const sal_uInt16 nId = mpToolBoxSpacing->GetItemId(UNO_SPACING);
    mpToolBoxSpacing->SetItemBits(nId, mpToolBoxSpacing->GetItemBits(nId) | TIB_DROPDOWNONLY);

    Link aLink = LINK(this, TextPropertyPanel, SpacingClickHdl);
    mpToolBoxSpacing->SetDropdownClickHdl ( aLink );
    mpToolBoxSpacing->SetSelectHdl( aLink );
}

void TextPropertyPanel::SetupToolboxItems (void)
{
    maUnderlineControl.SetupToolBoxItem(*mpToolBoxFont, mpToolBoxFont->GetItemId(UNO_UNDERLINE));
    maSpacingControl.SetupToolBoxItem(*mpToolBoxSpacing, mpToolBoxSpacing->GetItemId(UNO_SPACING));
}

IMPL_LINK(TextPropertyPanel, UnderlineClickHdl, ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();
    const OUString aCommand(pToolBox->GetItemCommand(nId));

    if (aCommand == UNO_UNDERLINE)
    {
        pToolBox->SetItemDown( nId, true );
        maUnderlinePopup.Rearrange(meUnderline);
        maUnderlinePopup.Show(*pToolBox);
    }

    return 0L;
}

IMPL_LINK(TextPropertyPanel, SpacingClickHdl, ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();
    const OUString aCommand(pToolBox->GetItemCommand(nId));

    if (aCommand == UNO_SPACING)
    {
        pToolBox->SetItemDown( nId, true );
        maCharSpacePopup.Rearrange(mbKernLBAvailable,mbKernAvailable,mlKerning);
        maCharSpacePopup.Show(*pToolBox);
    }

    return 0L;
}

void TextPropertyPanel::NotifyItemUpdate (
    const sal_uInt16 nSID,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    switch(nSID)
    {
    case SID_ATTR_CHAR_FONTHEIGHT:
        {
            if (  eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxFontHeightItem) )
                mpHeightItem = (SvxFontHeightItem*)pState;
            else
                mpHeightItem = NULL;
        }
        break;
    case SID_ATTR_CHAR_UNDERLINE:
        {
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxUnderlineItem) )
            {
                const SvxUnderlineItem* pItem = (const SvxUnderlineItem*)pState;
                meUnderline = (FontUnderline)pItem->GetValue();
                meUnderlineColor = pItem->GetColor();
            }
            else
                meUnderline = UNDERLINE_NONE;
        }
        break;
    case SID_ATTR_CHAR_KERNING:
        {
            if ( SFX_ITEM_AVAILABLE == eState )
            {
                mbKernLBAvailable = true;

                if(pState->ISA(SvxKerningItem))
                {
                    const SvxKerningItem* pKerningItem  = (const SvxKerningItem*)pState;
                    mlKerning = (long)pKerningItem->GetValue();
                    mbKernAvailable = true;
                }
                else
                {
                    mlKerning = 0;
                    mbKernAvailable =false;
                }
            }
            else if (SFX_ITEM_DISABLED == eState)
            {
                mbKernLBAvailable = false;
                mbKernAvailable = false;
                mlKerning = 0;
            }
            else
            {
                mbKernLBAvailable = true;
                mbKernAvailable = false;
                mlKerning = 0;
            }
            mpToolBoxSpacing->Enable(bIsEnabled);
        }
        break;
    }
}

Color& TextPropertyPanel::GetUnderlineColor()
{
    return meUnderlineColor;
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
