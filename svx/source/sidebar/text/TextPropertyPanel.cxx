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
#include "SvxSBFontNameBox.hxx"

#include "svx/dialmgr.hxx"

#include <editeng/flstitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/udlnitem.hxx>
#include <rtl/ref.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/sidebar/ControllerFactory.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/SidebarToolBox.hxx>
#include "sfx2/imagemgr.hxx"
#include <svtools/ctrltool.hxx>
#include <svtools/unitconv.hxx>

#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include "TextCharacterSpacingControl.hxx"
#include "TextCharacterSpacingPopup.hxx"
#include "TextUnderlineControl.hxx"
#include "TextUnderlinePopup.hxx"
#include <svx/sidebar/PopupContainer.hxx>

#include <boost/bind.hpp>

using namespace css;
using namespace cssu;
using ::sfx2::sidebar::Theme;
using ::sfx2::sidebar::ControlFactory;

const char UNO_BACKCOLOR[] = ".uno:BackColor";
const char UNO_COLOR[] = ".uno:Color";
const char UNO_FONTCOLOR[] = ".uno:FontColor";
const char UNO_SPACING[] = ".uno:Spacing";
const char UNO_UNDERLINE[] = ".uno:Underline";

namespace svx { namespace sidebar {

#undef HAS_IA2

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
        maFontNameControl   (SID_ATTR_CHAR_FONT,        *pBindings, *this, OUString("CharFontName"), rxFrame),
        maFontSizeControl   (SID_ATTR_CHAR_FONTHEIGHT,  *pBindings, *this, OUString("FontHeight"),   rxFrame),
        maUnderlineControl  (SID_ATTR_CHAR_UNDERLINE,   *pBindings, *this, OUString("Underline"),    rxFrame),
        maSpacingControl    (SID_ATTR_CHAR_KERNING,     *pBindings, *this, OUString("Spacing"),      rxFrame),

        mbFocusOnFontSizeCtrl(false),
        maCharSpacePopup(this, ::boost::bind(&TextPropertyPanel::CreateCharacterSpacingControl, this, _1)),
        maUnderlinePopup(this, ::boost::bind(&TextPropertyPanel::CreateUnderlinePopupControl, this, _1)),
        maContext(),
        mpBindings(pBindings)
{
    get(mpFontNameBox, "font");
    get(mpFontSizeBox, "fontsize");
    get(mpToolBoxFont, "fonteffects");
    get(mpToolBoxIncDec, "fontadjust");
    get(mpToolBoxScript, "position");
    get(mpToolBoxSpacing, "spacingbar");
    get(mpToolBoxFontColor, "colorbar");

    Initialize();
}




TextPropertyPanel::~TextPropertyPanel (void)
{
}




void TextPropertyPanel::SetSpacing(long nKern)
{
    mlKerning = nKern;
}


void TextPropertyPanel::HandleContextChange (
    const ::sfx2::sidebar::EnumContext aContext)
{
    if (maContext == aContext)
        return;

    maContext = aContext;

    if (maContext.GetApplication_DI() == sfx2::sidebar::EnumContext::Application::Application_Calc)
        mpToolBoxIncDec->Hide();
    else
        mpToolBoxIncDec->Show();

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

    UpdateFontColorToolbox(bWriterText);
}

void TextPropertyPanel::UpdateFontColorToolbox (bool bWriterText)
{
    if (bWriterText)
    {
        mpToolBoxFontColor->HideItem(mpToolBoxFontColor->GetItemId(UNO_COLOR));
        mpToolBoxFontColor->ShowItem(mpToolBoxFontColor->GetItemId(UNO_FONTCOLOR));
        mpToolBoxFontColor->ShowItem(mpToolBoxFontColor->GetItemId(UNO_BACKCOLOR));
    }
    else
    {
        mpToolBoxFontColor->ShowItem(mpToolBoxFontColor->GetItemId(UNO_COLOR));
        mpToolBoxFontColor->HideItem(mpToolBoxFontColor->GetItemId(UNO_FONTCOLOR));
        mpToolBoxFontColor->HideItem(mpToolBoxFontColor->GetItemId(UNO_BACKCOLOR));
    }
}


void TextPropertyPanel::DataChanged (const DataChangedEvent& /*rEvent*/)
{
    SetupToolboxItems();
}




void TextPropertyPanel::Initialize (void)
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = NULL;
    const FontList* pFontList = NULL;
    bool  bMustDelete = false;

    if (pDocSh != NULL)
        pItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );
    if (pItem != NULL)
        pFontList = ( (SvxFontListItem*)pItem )->GetFontList();
    else
    {
        pFontList = new FontList( Application::GetDefaultDevice() );
        bMustDelete = true;
    }

    const FontInfo aFontInfo (pFontList->Get( OUString( "" ), OUString( "" )));
    mpFontSizeBox->Fill(&aFontInfo,pFontList);

    if (bMustDelete)
        delete pFontList;

    mpFontNameBox->SetAccessibleName(mpFontNameBox->GetQuickHelpText());
    mpFontSizeBox->SetAccessibleName(mpFontSizeBox->GetQuickHelpText());

    //toolbox
    SetupToolboxItems();
    InitToolBoxFont();
    InitToolBoxSpacing();

#ifdef HAS_IA2
    mpFontNameBox->SetAccRelationLabeledBy(mpFontNameBox);
    mpFontNameBox->SetMpSubEditAccLableBy(mpFontNameBox);
    mpFontSizeBox->SetAccRelationLabeledBy(mpFontSizeBox);
    mpFontSizeBox->SetMpSubEditAccLableBy(mpFontSizeBox);
    mpToolBoxFont->SetAccRelationLabeledBy(mpToolBoxFont);
    mpToolBoxIncDec->SetAccRelationLabeledBy(mpToolBoxIncDec);
    mpToolBoxFontColor->SetAccRelationLabeledBy(mpToolBoxFontColor);
    mpToolBoxScript->SetAccRelationLabeledBy(mpToolBoxScript);
    mpToolBoxSpacing->SetAccRelationLabeledBy(mpToolBoxSpacing);
#endif

    //init state
    mpHeightItem = NULL;
    meUnderline = UNDERLINE_NONE;
    meUnderlineColor = COL_AUTO;
    mbKernAvailable = true;
    mbKernLBAvailable = true;
    mlKerning = 0;

    //set handler
    mpFontNameBox->SetBindings(mpBindings);
    Link aLink = LINK(this, TextPropertyPanel, FontSelHdl);
    mpFontNameBox->SetSelectHdl(aLink);
    aLink = LINK(this, TextPropertyPanel, FontSizeModifyHdl);
    mpFontSizeBox->SetModifyHdl(aLink);
    aLink = LINK(this, TextPropertyPanel, FontSizeSelHdl);
    mpFontSizeBox->SetSelectHdl(aLink);
    aLink = LINK(this, TextPropertyPanel, FontSizeLoseFocus);
    mpFontSizeBox->SetLoseFocusHdl(aLink);
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
    Link aLink = LINK(this, TextPropertyPanel, ToolboxFontSelectHandler);
    mpToolBoxFont->SetSelectHdl ( aLink );

    aLink = LINK(this, TextPropertyPanel, ToolBoxUnderlineClickHdl);
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




IMPL_LINK( TextPropertyPanel, FontSelHdl, FontNameBox*, pBox )
{
    if ( !pBox->IsTravelSelect() )
    {
        if( SfxViewShell::Current() )
        {
            Window* pShellWnd = SfxViewShell::Current()->GetWindow();

            if ( pShellWnd )
                pShellWnd->GrabFocus();
        }
    }
    return 0;
}

IMPL_LINK( TextPropertyPanel, FontSizeModifyHdl, FontSizeBox*, pSizeBox )
{
    if (pSizeBox == mpFontSizeBox)
    {
        long nSize = pSizeBox->GetValue();
        mbFocusOnFontSizeCtrl = true;

        float fSize = (float)nSize / 10;
        SfxMapUnit eUnit = maFontSizeControl.GetCoreMetric();
        SvxFontHeightItem aItem( CalcToUnit( fSize, eUnit ), 100, SID_ATTR_CHAR_FONTHEIGHT ) ;

        mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_FONTHEIGHT, SFX_CALLMODE_RECORD, &aItem, 0L );
        mpBindings->Invalidate(SID_ATTR_CHAR_FONTHEIGHT,true,false);
    }
    return 0;
}

IMPL_LINK( TextPropertyPanel, FontSizeSelHdl, FontSizeBox*, pSizeBox )
{
    if ( !pSizeBox->IsTravelSelect() )
    {
        if( SfxViewShell::Current() )
        {
            Window* pShellWnd = SfxViewShell::Current()->GetWindow();

            if ( pShellWnd )
                pShellWnd->GrabFocus();
        }
    }

    return 0;
}

IMPL_LINK(TextPropertyPanel, FontSizeLoseFocus, FontSizeBox*, pSizeBox)
{
    if(pSizeBox == mpFontSizeBox)
    {
        mbFocusOnFontSizeCtrl = false;
    }
    return 0;
}

IMPL_LINK(TextPropertyPanel, ToolboxFontSelectHandler, ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();
    const OUString aCommand(pToolBox->GetItemCommand(nId));

    EndTracking();

    if (aCommand == UNO_UNDERLINE)
        meUnderline = UNDERLINE_NONE;

    dispatch(aCommand);

    return 0;
}




IMPL_LINK(TextPropertyPanel, ToolBoxUnderlineClickHdl, ToolBox*, pToolBox)
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
    bool bIsControlEnabled (bIsEnabled);

    switch(nSID)
    {
    case SID_ATTR_CHAR_FONT:
        {
            if (  eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxFontItem) )
            {
                const SvxFontItem* pFontItem = (const SvxFontItem*)pState;
                mpFontNameBox->SetText( pFontItem->GetFamilyName() );
            }
            else
            {
                mpFontNameBox->SetText( "" );
                if (SFX_ITEM_DISABLED == eState)
                    bIsControlEnabled = false;
            }
            mpFontNameBox->Enable(bIsControlEnabled);
        }
        break;
    case SID_ATTR_CHAR_FONTHEIGHT:
        {
            if (  eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxFontHeightItem) )
            {
                mpHeightItem = (SvxFontHeightItem*)pState;//const SvxFontHeightItem*
                SfxMapUnit eUnit = maFontSizeControl.GetCoreMetric();
                const sal_Int64 nValue (CalcToPoint(mpHeightItem->GetHeight(), eUnit, 10 ));

                if( mbFocusOnFontSizeCtrl )
                    return;

                mpFontSizeBox->SetValue(nValue);
                mpFontSizeBox->LoseFocus();
            }
            else
            {
                mpHeightItem = NULL;
                mpFontSizeBox->SetText( "" );
                if ( eState <= SFX_ITEM_READONLY )
                    bIsControlEnabled = false;
            }
            mpFontSizeBox->Enable(bIsControlEnabled);
        }
        break;
    case SID_ATTR_CHAR_UNDERLINE:
        {
            if( eState >= SFX_ITEM_DEFAULT)
            {
                if(pState->ISA(SvxUnderlineItem))
                {
                    const SvxUnderlineItem* pItem = (const SvxUnderlineItem*)pState;
                    meUnderline = (FontUnderline)pItem->GetValue();
                    meUnderlineColor = pItem->GetColor();
                }
            }
            else
            {
                meUnderline = UNDERLINE_NONE;
            }
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

void TextPropertyPanel::SetUnderline(FontUnderline  eUnderline)
{
    meUnderline = eUnderline;
}

} } // end of namespace svx::sidebar
