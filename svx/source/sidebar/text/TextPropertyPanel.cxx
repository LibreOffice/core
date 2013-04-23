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

#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <rtl/ref.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include "sfx2/imagemgr.hxx"
#include <svtools/ctrltool.hxx>
#include <svtools/unitconv.hxx>

#include <vcl/gradient.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include "TextCharacterSpacingControl.hxx"
#include "TextCharacterSpacingPopup.hxx"
#include "TextUnderlineControl.hxx"
#include "TextUnderlinePopup.hxx"
#include <svx/sidebar/ColorControl.hxx>
#include <svx/sidebar/PopupContainer.hxx>

#include <boost/bind.hpp>

using namespace css;
using namespace cssu;
using ::sfx2::sidebar::Theme;
using ::sfx2::sidebar::ControlFactory;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace svx { namespace sidebar {

#undef HAS_IA2


#define TEXT_SECTIONPAGE_HEIGHT_S   SECTIONPAGE_MARGIN_VERTICAL_TOP + CBOX_HEIGHT  + ( TOOLBOX_ITEM_HEIGHT + 2 ) + CONTROL_SPACING_VERTICAL * 1 + SECTIONPAGE_MARGIN_VERTICAL_BOT
#define TEXT_SECTIONPAGE_HEIGHT     SECTIONPAGE_MARGIN_VERTICAL_TOP + CBOX_HEIGHT  + ( TOOLBOX_ITEM_HEIGHT + 2 ) * 2 + CONTROL_SPACING_VERTICAL * 2 + SECTIONPAGE_MARGIN_VERTICAL_BOT

//

//end
PopupControl* TextPropertyPanel::CreateCharacterSpacingControl (PopupContainer* pParent)
{
    return new TextCharacterSpacingControl(pParent, *this, mpBindings);
}

PopupControl* TextPropertyPanel::CreateUnderlinePopupControl (PopupContainer* pParent)
{
    return new TextUnderlineControl(pParent, *this, mpBindings);
}

namespace
{
    Color GetAutomaticColor(void)
    {
        return COL_AUTO;
    }
} // end of anonymous namespace

PopupControl* TextPropertyPanel::CreateFontColorPopupControl (PopupContainer* pParent)
{
    const ResId aResId(SVX_RES(STR_AUTOMATICE));

    return new ColorControl(
        pParent,
        mpBindings,
        SVX_RES(RID_POPUPPANEL_TEXTPAGE_FONT_COLOR),
        SVX_RES(VS_FONT_COLOR),
        ::boost::bind(GetAutomaticColor),
        ::boost::bind(&TextPropertyPanel::SetFontColor, this, _1,_2),
        pParent,
        &aResId);
}

PopupControl* TextPropertyPanel::CreateBrushColorPopupControl (PopupContainer* pParent)
{
    const ResId aResId(SVX_RES(STR_AUTOMATICE));

    return new ColorControl(
        pParent,
        mpBindings,
        SVX_RES(RID_POPUPPANEL_TEXTPAGE_FONT_COLOR),
        SVX_RES(VS_FONT_COLOR),
        ::boost::bind(GetAutomaticColor),
        ::boost::bind(&TextPropertyPanel::SetBrushColor, this, _1,_2),
        pParent,
        &aResId);
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
    const cssu::Reference<css::ui::XSidebar>& rxSidebar)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException(A2S("no parent Window given to TextPropertyPanel::Create"), NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException(A2S("no XFrame given to TextPropertyPanel::Create"), NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException(A2S("no SfxBindings given to TextPropertyPanel::Create"), NULL, 2);

    return new TextPropertyPanel(
        pParent,
        rxFrame,
        pBindings,
        rxSidebar);
}


::sfx2::sidebar::ControllerItem& TextPropertyPanel::GetSpaceController()
{
    return maSpacingControl;
}

TextPropertyPanel::TextPropertyPanel (
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const cssu::Reference<css::ui::XSidebar>& rxSidebar)
    :   Control(pParent, SVX_RES(RID_SIDEBAR_TEXT_PANEL)),
        mpFontNameBox (new SvxSBFontNameBox(this, SVX_RES(CB_SBFONT_FONT))),
        maFontSizeBox       (this, SVX_RES(MB_SBFONT_FONTSIZE)),
        mpToolBoxIncDecBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxIncDec(ControlFactory::CreateToolBox(
                mpToolBoxIncDecBackground.get(),
                SVX_RES(TB_INCREASE_DECREASE))),
        mpToolBoxFontBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxFont(ControlFactory::CreateToolBox(
                mpToolBoxFontBackground.get(),
                SVX_RES(TB_FONT))),
        mpToolBoxFontColorBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxFontColor(ControlFactory::CreateToolBox(
                mpToolBoxFontColorBackground.get(),
                SVX_RES(TB_FONTCOLOR))),
        mpToolBoxScriptBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxScript(ControlFactory::CreateToolBox(
                mpToolBoxScriptBackground.get(),
                SVX_RES(TB_SCRIPT))),
        mpToolBoxScriptSwBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxScriptSw(ControlFactory::CreateToolBox(
                mpToolBoxScriptSwBackground.get(),
                SVX_RES(TB_SCRIPT_SW))),
        mpToolBoxSpacingBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxSpacing(ControlFactory::CreateToolBox(
                mpToolBoxSpacingBackground.get(),
                SVX_RES(TB_SPACING))),
        mpToolBoxHighlightBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxHighlight(ControlFactory::CreateToolBox(
                mpToolBoxHighlightBackground.get(),
                SVX_RES(TB_HIGHLIGHT))),

        mpFontColorUpdater(),
        mpHighlightUpdater(),

        maFontNameControl   (SID_ATTR_CHAR_FONT,        *pBindings, *this, A2S("CharFontName"), rxFrame),
        maFontSizeControl   (SID_ATTR_CHAR_FONTHEIGHT,  *pBindings, *this, A2S("FontHeight"),   rxFrame),
        maWeightControl     (SID_ATTR_CHAR_WEIGHT,      *pBindings, *this, A2S("Bold"),         rxFrame),
        maItalicControl     (SID_ATTR_CHAR_POSTURE,     *pBindings, *this, A2S("Italic"),       rxFrame),
        maUnderlineControl  (SID_ATTR_CHAR_UNDERLINE,   *pBindings, *this, A2S("Underline"),    rxFrame),
        maStrikeControl     (SID_ATTR_CHAR_STRIKEOUT,   *pBindings, *this, A2S("Strikeout"),    rxFrame),
        maShadowControl     (SID_ATTR_CHAR_SHADOWED,    *pBindings, *this, A2S("Shadowed"),     rxFrame),
        maFontColorControl  (SID_ATTR_CHAR_COLOR,       *pBindings, *this, A2S("Color"),        rxFrame),
        maScriptControlSw   (SID_ATTR_CHAR_ESCAPEMENT,  *pBindings, *this, A2S("Escapement"),   rxFrame),
        maSuperScriptControl(SID_SET_SUPER_SCRIPT,      *pBindings, *this, A2S("SuperScript"),  rxFrame),
        maSubScriptControl  (SID_SET_SUB_SCRIPT,        *pBindings, *this, A2S("SubScript"),    rxFrame),
        maSpacingControl    (SID_ATTR_CHAR_KERNING,     *pBindings, *this, A2S("Spacing"),      rxFrame),
        maHighlightControl  (SID_ATTR_BRUSH_CHAR, *pBindings, *this, A2S("CharacterBackgroundPattern"),rxFrame),
        maSDFontGrow        (SID_GROW_FONT_SIZE,        *pBindings, *this, A2S("Grow"),         rxFrame),
        maSDFontShrink      (SID_SHRINK_FONT_SIZE,      *pBindings, *this, A2S("Shrink"),       rxFrame),

        mpFontList          (NULL),
        mbMustDelete        (false),
        mbFocusOnFontSizeCtrl(false),

        maCharSpacePopup(this, ::boost::bind(&TextPropertyPanel::CreateCharacterSpacingControl, this, _1)),
        maUnderlinePopup(this, ::boost::bind(&TextPropertyPanel::CreateUnderlinePopupControl, this, _1)),
        maFontColorPopup(this, ::boost::bind(&TextPropertyPanel::CreateFontColorPopupControl, this, _1)),
        maBrushColorPopup(this, ::boost::bind(&TextPropertyPanel::CreateBrushColorPopupControl, this, _1)),

        mxFrame(rxFrame),
        maContext(),
        mpBindings(pBindings),
        mxSidebar(rxSidebar)
{
    Initialize();
    FreeResource();
}




TextPropertyPanel::~TextPropertyPanel (void)
{
    if(mbMustDelete)
        delete mpFontList;

    // Destroy the toolbox windows.
    mpToolBoxIncDec.reset();
    mpToolBoxFont.reset();
    mpToolBoxFontColor.reset();
    mpToolBoxScript.reset();
    mpToolBoxScriptSw.reset();
    mpToolBoxSpacing.reset();
    mpToolBoxHighlight.reset();

    // Destroy the background windows of the toolboxes.
    mpToolBoxIncDecBackground.reset();
    mpToolBoxFontBackground.reset();
    mpToolBoxFontColorBackground.reset();
    mpToolBoxScriptBackground.reset();
    mpToolBoxScriptSwBackground.reset();
    mpToolBoxSpacingBackground.reset();
    mpToolBoxHighlightBackground.reset();
}




void TextPropertyPanel::SetSpacing(long nKern)
{
    mlKerning = nKern;
}


void TextPropertyPanel::HandleContextChange (
    const ::sfx2::sidebar::EnumContext aContext)
{
    if (maContext == aContext)
    {
        // Nothing to do.
        return;
    }

    maContext = aContext;
    switch (maContext.GetCombinedContext_DI()) //
    {
        case CombinedEnumContext(Application_Calc, Context_Cell):
        case CombinedEnumContext(Application_Calc, Context_Pivot):
        {
            mpToolBoxScript->Hide();
            mpToolBoxScriptSw->Hide();
            mpToolBoxSpacing->Hide();
            mpToolBoxHighlight->Hide();

            Size aSize(PROPERTYPAGE_WIDTH,TEXT_SECTIONPAGE_HEIGHT_S);
            aSize = LogicToPixel( aSize, MapMode(MAP_APPFONT) );
            aSize.setWidth(GetOutputSizePixel().Width());
            SetSizePixel(aSize);
            if (mxSidebar.is())
                mxSidebar->requestLayout();
            break;
        }

        case CombinedEnumContext(Application_WriterVariants, Context_Text):
        case CombinedEnumContext(Application_WriterVariants, Context_Table):
        {
            mpToolBoxScriptSw->Show();
            mpToolBoxScript->Hide();
            mpToolBoxHighlight->Show();
            mpToolBoxSpacing->Show();

            Size aSize(PROPERTYPAGE_WIDTH, TEXT_SECTIONPAGE_HEIGHT);
            aSize = LogicToPixel( aSize, MapMode(MAP_APPFONT) );
            aSize.setWidth(GetOutputSizePixel().Width());
            SetSizePixel(aSize);
            if (mxSidebar.is())
                mxSidebar->requestLayout();
            break;
        }

        case CombinedEnumContext(Application_WriterVariants, Context_DrawText):
        case CombinedEnumContext(Application_WriterVariants, Context_Annotation):
        {
            mpToolBoxScriptSw->Show();
            mpToolBoxScript->Hide();
            mpToolBoxSpacing->Show();
            mpToolBoxHighlight->Hide();

            Size aSize(PROPERTYPAGE_WIDTH,TEXT_SECTIONPAGE_HEIGHT);
            aSize = LogicToPixel( aSize, MapMode(MAP_APPFONT) );
            aSize.setWidth(GetOutputSizePixel().Width());
            SetSizePixel(aSize);
            if (mxSidebar.is())
                mxSidebar->requestLayout();
            break;
        }

        case CombinedEnumContext(Application_Calc, Context_EditCell):
        case CombinedEnumContext(Application_Calc, Context_DrawText):
        case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
        case CombinedEnumContext(Application_DrawImpress, Context_Text):
        case CombinedEnumContext(Application_DrawImpress, Context_Table):
        case CombinedEnumContext(Application_DrawImpress, Context_OutlineText):
        case CombinedEnumContext(Application_DrawImpress, Context_Draw):
        case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
        {
            mpToolBoxScriptSw->Hide();
            mpToolBoxScript->Show();
            mpToolBoxSpacing->Show();
            mpToolBoxHighlight->Hide();

            Size aSize(PROPERTYPAGE_WIDTH,TEXT_SECTIONPAGE_HEIGHT);
            aSize = LogicToPixel( aSize,MapMode(MAP_APPFONT) );
            aSize.setWidth(GetOutputSizePixel().Width());
            SetSizePixel(aSize);
            if (mxSidebar.is())
                mxSidebar->requestLayout();
            break;
        }

        default:
            break;
    }
}




void TextPropertyPanel::DataChanged (const DataChangedEvent& rEvent)
{
    (void)rEvent;

    SetupToolboxItems();
}



void TextPropertyPanel::Initialize (void)
{
    //<<modify fill font list
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = NULL;

    if (pDocSh != NULL)
        pItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );
    if (pItem != NULL)
        mpFontList = ( (SvxFontListItem*)pItem )->GetFontList();
    else
    {
        mpFontList = new FontList( Application::GetDefaultDevice() );
        mbMustDelete = true;
    }

    mpFontNameBox->SetAccessibleName(mpFontNameBox->GetQuickHelpText());
    const FontInfo aFontInfo (mpFontList->Get( OUString( "" ), OUString( "" )));
    maFontSizeBox.Fill(&aFontInfo,mpFontList);
    maFontSizeBox.SetAccessibleName(maFontSizeBox.GetQuickHelpText());

    //toolbox
    SetupToolboxItems();
    InitToolBoxIncDec();
    InitToolBoxFont();
    InitToolBoxFontColor();
    InitToolBoxScript();
    InitToolBoxSpacing();
    InitToolBoxHighlight();

#ifdef HAS_IA2
    mpFontNameBox->SetAccRelationLabeledBy(&mpFontNameBox);
    mpFontNameBox->SetMpSubEditAccLableBy(&mpFontNameBox);
    maFontSizeBox.SetAccRelationLabeledBy(&maFontSizeBox);
    maFontSizeBox.SetMpSubEditAccLableBy(&maFontSizeBox);
    mpToolBoxFont.SetAccRelationLabeledBy(&mpToolBoxFont);
    mpToolBoxIncDec.SetAccRelationLabeledBy(&mpToolBoxIncDec);
    mpToolBoxFontColor.SetAccRelationLabeledBy(&mpToolBoxFontColor);
    mpToolBoxScript.SetAccRelationLabeledBy(&mpToolBoxScript);
    mpToolBoxScriptSw.SetAccRelationLabeledBy(&mpToolBoxScriptSw);
    mpToolBoxSpacing.SetAccRelationLabeledBy(&mpToolBoxSpacing);
    mpToolBoxHighlight.SetAccRelationLabeledBy(&mpToolBoxHighlight);
#endif

    //init state
    mpHeightItem = NULL;
    meWeight = WEIGHT_NORMAL;
    meItalic = ITALIC_NONE;
    mbShadow = false;
    meStrike = STRIKEOUT_NONE;
    mbPostureAvailable = true;
    mbWeightAvailable = true;
    meUnderline = UNDERLINE_NONE;
    meUnderlineColor = COL_AUTO;   //
    maColor = COL_BLACK;
    mbColorAvailable = true;
    maBackColor = COL_AUTO;
    mbBackColorAvailable = true;
    meColorType = FONT_COLOR;
    meEscape = SVX_ESCAPEMENT_OFF;
    mbSuper = false;
    mbSub = false;
    mbKernAvailable = true;
    mbKernLBAvailable = true;
    mlKerning = 0;
    mpFontColorUpdater.reset(new ToolboxButtonColorUpdater(
            SID_ATTR_CHAR_COLOR,
            TBI_FONTCOLOR,
            mpToolBoxFontColor.get(),
            TBX_UPDATER_MODE_CHAR_COLOR_NEW));
    mpHighlightUpdater.reset(new ToolboxButtonColorUpdater(
            SID_ATTR_BRUSH_CHAR,
            TBI_HIGHLIGHT,
            mpToolBoxHighlight.get(),
            TBX_UPDATER_MODE_CHAR_COLOR_NEW));

    //set handler
    mpFontNameBox->SetBindings(mpBindings);
    //add
    Link aLink = LINK(this, TextPropertyPanel, FontSelHdl);
    mpFontNameBox->SetSelectHdl(aLink);
    //add end

    aLink = LINK(this, TextPropertyPanel, FontSizeModifyHdl);
    maFontSizeBox.SetModifyHdl(aLink);
    //add
    aLink = LINK(this, TextPropertyPanel, FontSizeSelHdl);
    maFontSizeBox.SetSelectHdl(aLink);
    //add end
    aLink = LINK(this, TextPropertyPanel, FontSizeLoseFocus);
    maFontSizeBox.SetLoseFocusHdl(aLink);

    // add
    long aSizeBoxHeight = maFontSizeBox.GetSizePixel().getHeight();;
    Point aPosFontSize = maFontSizeBox.GetPosPixel();
    long aPosY = aPosFontSize.getY();
    Point pTBIncDec = mpToolBoxIncDec->GetPosPixel();
    long aIncDecHeight = mpToolBoxIncDec->GetSizePixel().getHeight();
    pTBIncDec.setY(aPosY+aSizeBoxHeight/2-aIncDecHeight/2);
    mpToolBoxIncDec->SetPosPixel(pTBIncDec);
    //end
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
    mpToolBoxFont->SetBackground(Wallpaper());
    mpToolBoxFont->SetPaintTransparent(true);

    Size aTbxSize( mpToolBoxFont->CalcWindowSizePixel() );
    mpToolBoxFont->SetOutputSizePixel( aTbxSize );

    Link aLink  = LINK(this, TextPropertyPanel, ToolboxFontSelectHandler);
    mpToolBoxFont->SetSelectHdl ( aLink );
    aLink = LINK(this, TextPropertyPanel, ToolBoxUnderlineClickHdl);
    mpToolBoxFont->SetDropdownClickHdl(aLink);
}




void TextPropertyPanel::InitToolBoxIncDec()
{
    Size aTbxSize( mpToolBoxIncDec->CalcWindowSizePixel() );
    mpToolBoxIncDec->SetOutputSizePixel( aTbxSize );

    Link aLink = LINK(this, TextPropertyPanel, ToolboxIncDecSelectHdl);
    mpToolBoxIncDec->SetSelectHdl ( aLink );
}




void TextPropertyPanel::InitToolBoxFontColor()
{
    Size aTbxSize( mpToolBoxFontColor->CalcWindowSizePixel() );
    mpToolBoxFontColor->SetOutputSizePixel( aTbxSize );
    mpToolBoxFontColor->SetItemBits( TBI_FONTCOLOR, mpToolBoxFontColor->GetItemBits( TBI_FONTCOLOR ) | TIB_DROPDOWNONLY );

    Link aLink = LINK(this, TextPropertyPanel, ToolBoxFontColorDropHdl);
    mpToolBoxFontColor->SetDropdownClickHdl ( aLink );
    mpToolBoxFontColor->SetSelectHdl ( aLink );

}
void TextPropertyPanel::InitToolBoxScript()
{
    Size aTbxSize( mpToolBoxScriptSw->CalcWindowSizePixel() );
    mpToolBoxScriptSw->SetOutputSizePixel( aTbxSize );

    Link aLink = LINK(this, TextPropertyPanel, ToolBoxSwScriptSelectHdl);
    mpToolBoxScriptSw->SetSelectHdl ( aLink );

    aTbxSize = mpToolBoxScript->CalcWindowSizePixel() ;
    mpToolBoxScript->SetOutputSizePixel( aTbxSize );

    aLink = LINK(this, TextPropertyPanel, ToolBoxScriptSelectHdl);
    mpToolBoxScript->SetSelectHdl ( aLink );
}
void TextPropertyPanel::InitToolBoxSpacing()
{
    Size aTbxSize( mpToolBoxSpacing->CalcWindowSizePixel() );
    mpToolBoxSpacing->SetOutputSizePixel( aTbxSize );
    mpToolBoxSpacing->SetItemBits( TBI_SPACING, mpToolBoxSpacing->GetItemBits( TBI_SPACING ) | TIB_DROPDOWNONLY );

    Link aLink = LINK(this, TextPropertyPanel, SpacingClickHdl);
    mpToolBoxSpacing->SetDropdownClickHdl ( aLink );
    mpToolBoxSpacing->SetSelectHdl( aLink );
}
void TextPropertyPanel::InitToolBoxHighlight()
{
    Size aTbxSize( mpToolBoxHighlight->CalcWindowSizePixel() );
    mpToolBoxHighlight->SetOutputSizePixel( aTbxSize );
    mpToolBoxHighlight->SetItemBits( TBI_HIGHLIGHT, mpToolBoxHighlight->GetItemBits( TBI_HIGHLIGHT ) | TIB_DROPDOWNONLY );

    Link aLink = LINK(this, TextPropertyPanel, ToolBoxHighlightDropHdl);
    mpToolBoxHighlight->SetDropdownClickHdl ( aLink );
    mpToolBoxHighlight->SetSelectHdl( aLink );
}




void TextPropertyPanel::SetupToolboxItems (void)
{
    maSDFontGrow.SetupToolBoxItem(*mpToolBoxIncDec, TBI_INCREASE);
    maSDFontShrink.SetupToolBoxItem(*mpToolBoxIncDec, TBI_DECREASE);

    maWeightControl.SetupToolBoxItem(*mpToolBoxFont, TBI_BOLD);
    maItalicControl.SetupToolBoxItem(*mpToolBoxFont, TBI_ITALIC);
    maUnderlineControl.SetupToolBoxItem(*mpToolBoxFont, TBI_UNDERLINE);
    maStrikeControl.SetupToolBoxItem(*mpToolBoxFont, TBI_STRIKEOUT);
    maShadowControl.SetupToolBoxItem(*mpToolBoxFont, TBI_SHADOWED);

    maFontColorControl.SetupToolBoxItem(*mpToolBoxFontColor, TBI_FONTCOLOR);
    //for sw
    maSuperScriptControl.SetupToolBoxItem(*mpToolBoxScriptSw, TBI_SUPER_SW);
    maSubScriptControl.SetupToolBoxItem(*mpToolBoxScriptSw, TBI_SUB_SW);
    //for sc and sd
    maSubScriptControl.SetupToolBoxItem(*mpToolBoxScript, TBI_SUPER);
    maSubScriptControl.SetupToolBoxItem(*mpToolBoxScript, TBI_SUB);
    maSpacingControl.SetupToolBoxItem(*mpToolBoxSpacing, TBI_SPACING);
    maHighlightControl.SetupToolBoxItem(*mpToolBoxHighlight, TBI_HIGHLIGHT);
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
//add end
IMPL_LINK( TextPropertyPanel, FontSizeModifyHdl, FontSizeBox*, pSizeBox )
{
    if (pSizeBox == &maFontSizeBox)
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
//add
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
//add end
IMPL_LINK(TextPropertyPanel, FontSizeLoseFocus, FontSizeBox*, pSizeBox)
{
    if(pSizeBox == &maFontSizeBox)
    {
        mbFocusOnFontSizeCtrl = false;
    }
    return 0;
}

IMPL_LINK(TextPropertyPanel, ToolboxFontSelectHandler, ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();

    switch (nId)
    {
        case TBI_BOLD:
        {
            EndTracking();
            if(meWeight != WEIGHT_BOLD)
                meWeight = WEIGHT_BOLD;
            else
                meWeight = WEIGHT_NORMAL;
            SvxWeightItem aWeightItem(meWeight, SID_ATTR_CHAR_WEIGHT);
            mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_WEIGHT, SFX_CALLMODE_RECORD, &aWeightItem, 0L);
            UpdateItem(SID_ATTR_CHAR_WEIGHT);
            break;
        }
        case TBI_ITALIC:
        {
            EndTracking();
            if(meItalic != ITALIC_NORMAL)
                meItalic = ITALIC_NORMAL;
            else
                meItalic = ITALIC_NONE;
            SvxPostureItem aPostureItem(meItalic, SID_ATTR_CHAR_POSTURE);
            mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_POSTURE, SFX_CALLMODE_RECORD, &aPostureItem, 0L);
            UpdateItem(SID_ATTR_CHAR_POSTURE);
            break;
        }
        case TBI_UNDERLINE:
        {
            EndTracking();
            if(meUnderline == UNDERLINE_NONE)
            {
                meUnderline = UNDERLINE_SINGLE;
                SvxUnderlineItem aLineItem(meUnderline, SID_ATTR_CHAR_UNDERLINE);
                aLineItem.SetColor(meUnderlineColor);
                mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_UNDERLINE, SFX_CALLMODE_RECORD, &aLineItem, 0L);
            }
            else
            {
                meUnderline = UNDERLINE_NONE;
                SvxUnderlineItem aLineItem(meUnderline, SID_ATTR_CHAR_UNDERLINE);
                mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_UNDERLINE, SFX_CALLMODE_RECORD, &aLineItem, 0L);
            }
            UpdateItem(SID_ATTR_CHAR_UNDERLINE);
        }
        case TBI_STRIKEOUT:
        {
            EndTracking();
            if(meStrike !=  STRIKEOUT_NONE && meStrike != STRIKEOUT_DONTKNOW)
                meStrike = STRIKEOUT_NONE;
            else
                meStrike = STRIKEOUT_SINGLE;
            SvxCrossedOutItem aStrikeItem(meStrike,SID_ATTR_CHAR_STRIKEOUT);
            mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_STRIKEOUT, SFX_CALLMODE_RECORD, &aStrikeItem, 0L);
            UpdateItem(SID_ATTR_CHAR_STRIKEOUT);
            break;
        }
        case TBI_SHADOWED:
        {
            EndTracking();
            mbShadow = !mbShadow;
            SvxShadowedItem aShadowItem(mbShadow, SID_ATTR_CHAR_SHADOWED);
            mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_SHADOWED, SFX_CALLMODE_RECORD, &aShadowItem, 0L);
            UpdateItem(SID_ATTR_CHAR_SHADOWED);
            break;
        }
    }
    return 0;
}




IMPL_LINK(TextPropertyPanel, ToolboxIncDecSelectHdl, ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();

    // font size +/- enhancement in sd
    switch (maContext.GetCombinedContext_DI())
    {
        case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
        case CombinedEnumContext(Application_DrawImpress, Context_Text):
        case CombinedEnumContext(Application_DrawImpress, Context_Table):
        case CombinedEnumContext(Application_DrawImpress, Context_OutlineText):
        case CombinedEnumContext(Application_DrawImpress, Context_Draw):
        case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
            if(nId == TBI_INCREASE)
            {
                EndTracking();
                SfxVoidItem aItem(SID_GROW_FONT_SIZE);
                mpBindings->GetDispatcher()->Execute( SID_GROW_FONT_SIZE, SFX_CALLMODE_RECORD, &aItem, 0L );
            }
            else if(nId == TBI_DECREASE)
            {
                EndTracking();
                SfxVoidItem aItem(SID_SHRINK_FONT_SIZE);
                mpBindings->GetDispatcher()->Execute( SID_SHRINK_FONT_SIZE, SFX_CALLMODE_RECORD, &aItem, 0L );
            }
            break;

        default:
            if(nId == TBI_INCREASE)
            {
                EndTracking();
                mbFocusOnFontSizeCtrl = false;
                sal_Int64 iValue = maFontSizeBox.GetValue();
                int iPos = maFontSizeBox.GetValuePos(iValue, FUNIT_NONE);
                long nSize = iValue;
                if(iPos != LISTBOX_ENTRY_NOTFOUND)
                    nSize = maFontSizeBox.GetValue(iPos+1 , FUNIT_NONE);
                else if(iValue >= 100 && iValue < 105)
                    nSize = 105;
                else if(iValue >= 105 && iValue < 110)
                    nSize = 110;
                else if(iValue < 960)
                {
                    nSize = (nSize / 10) * 10 + 10;
                    while(maFontSizeBox.GetValuePos(nSize, FUNIT_NONE) == LISTBOX_ENTRY_NOTFOUND)
                        nSize += 10;
                }
                else
                {
                    nSize = iValue;
                }

                float fSize = (float)nSize / 10;

                SfxMapUnit eUnit = maFontSizeControl.GetCoreMetric();
                SvxFontHeightItem aItem( CalcToUnit( fSize, eUnit ), 100, SID_ATTR_CHAR_FONTHEIGHT ) ;

                mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_FONTHEIGHT, SFX_CALLMODE_RECORD, &aItem, 0L );
                mpBindings->Invalidate(SID_ATTR_CHAR_FONTHEIGHT,true,false);
                maFontSizeBox.SetValue( nSize );
            }
            else if(nId == TBI_DECREASE)
            {
                EndTracking();
                mbFocusOnFontSizeCtrl = false;
                sal_Int64 iValue = maFontSizeBox.GetValue();
                int iPos = maFontSizeBox.GetValuePos(iValue, FUNIT_NONE);
                long nSize = iValue;
                if(iPos != LISTBOX_ENTRY_NOTFOUND)
                    nSize = maFontSizeBox.GetValue(iPos-1 , FUNIT_NONE);
                else if(iValue > 100 && iValue <= 105)
                    nSize = 100;
                else if(iValue > 105 && iValue <= 110)
                    nSize = 105;
                else if(iValue > 960)
                {
                    nSize = 960;
                }
                else if(iValue > 60)
                {
                    nSize = (nSize / 10) * 10 ;
                    while(maFontSizeBox.GetValuePos(nSize, FUNIT_NONE) == LISTBOX_ENTRY_NOTFOUND)
                        nSize -= 10;
                }
                else
                {
                    nSize = iValue;
                }

                float fSize = (float)nSize / 10;

                SfxMapUnit eUnit = maFontSizeControl.GetCoreMetric();
                SvxFontHeightItem aItem( CalcToUnit( fSize, eUnit ), 100, SID_ATTR_CHAR_FONTHEIGHT ) ;

                mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_FONTHEIGHT, SFX_CALLMODE_RECORD, &aItem, 0L );
                mpBindings->Invalidate(SID_ATTR_CHAR_FONTHEIGHT,true,false);
                maFontSizeBox.SetValue( nSize );
            }
    }
    UpdateItem(SID_ATTR_CHAR_FONTHEIGHT);

    return 0;
}



IMPL_LINK(TextPropertyPanel, ToolBoxUnderlineClickHdl, ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();
    OSL_ASSERT(nId == TBI_UNDERLINE);
    if(nId == TBI_UNDERLINE)
    {
        pToolBox->SetItemDown( nId, true );
        maUnderlinePopup.Rearrange(meUnderline);
        maUnderlinePopup.Show(*pToolBox);

    }
    return 0L;
}




IMPL_LINK(TextPropertyPanel, ToolBoxFontColorDropHdl,ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();
    if(nId == TBI_FONTCOLOR)
    {
        meColorType = FONT_COLOR;

        pToolBox->SetItemDown( nId, true );

        maFontColorPopup.Show(*pToolBox);
        maFontColorPopup.SetCurrentColor(maColor, mbColorAvailable);
    }
    return 0;
}




IMPL_LINK(TextPropertyPanel, ToolBoxSwScriptSelectHdl, ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();
    if( nId == TBI_SUPER_SW )
    {
        if(meEscape != SVX_ESCAPEMENT_SUPERSCRIPT)
        {
            meEscape = SVX_ESCAPEMENT_SUPERSCRIPT;
            SvxEscapementItem aSupItem(DFLT_ESC_SUPER, DFLT_ESC_PROP, SID_ATTR_CHAR_ESCAPEMENT);
            mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_ESCAPEMENT, SFX_CALLMODE_RECORD, &aSupItem, 0L );
        }
        else
        {
            meEscape = SVX_ESCAPEMENT_OFF;
            SvxEscapementItem aNoneItem(0, 100, SID_ATTR_CHAR_ESCAPEMENT);
            mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_ESCAPEMENT, SFX_CALLMODE_RECORD, &aNoneItem, 0L );
        }
    }
    else if(TBI_SUB_SW == nId)
    {
        if(meEscape != SVX_ESCAPEMENT_SUBSCRIPT)
        {
            meEscape = (SvxEscapement)SVX_ESCAPEMENT_SUBSCRIPT;
            SvxEscapementItem aSubItem(DFLT_ESC_SUB, DFLT_ESC_PROP, SID_ATTR_CHAR_ESCAPEMENT);
            mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_ESCAPEMENT, SFX_CALLMODE_RECORD, &aSubItem, 0L );
        }
        else
        {
            meEscape = SVX_ESCAPEMENT_OFF;
            SvxEscapementItem aNoneItem(0, 100, SID_ATTR_CHAR_ESCAPEMENT);
            mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_ESCAPEMENT, SFX_CALLMODE_RECORD, &aNoneItem, 0L );
        }
    }
    UpdateItem(SID_ATTR_CHAR_ESCAPEMENT);

    return 0;
}




IMPL_LINK(TextPropertyPanel, ToolBoxScriptSelectHdl, ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();
    if( nId == TBI_SUPER )
    {
        mbSuper = !mbSuper;
        SfxBoolItem aSupItem(SID_SET_SUPER_SCRIPT, mbSuper);
        mpBindings->GetDispatcher()->Execute( SID_SET_SUPER_SCRIPT, SFX_CALLMODE_RECORD, &aSupItem, 0L );
        UpdateItem(SID_SET_SUPER_SCRIPT);
    }
    else if(TBI_SUB == nId)
    {

        mbSub = !mbSub;
        SfxBoolItem aSubItem(SID_SET_SUB_SCRIPT, mbSub );
        mpBindings->GetDispatcher()->Execute( SID_SET_SUB_SCRIPT, SFX_CALLMODE_RECORD, &aSubItem, 0L );
        UpdateItem(SID_SET_SUB_SCRIPT);
    }
    return 0;
}




IMPL_LINK(TextPropertyPanel, ToolBoxHighlightDropHdl, ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();
    if(nId == TBI_HIGHLIGHT)
    {
        meColorType = BACK_COLOR;

        pToolBox->SetItemDown( nId, true );
        maBrushColorPopup.Show(*pToolBox);
        maBrushColorPopup.SetCurrentColor(maBackColor, mbBackColorAvailable);

    }
    return 0;
}



IMPL_LINK(TextPropertyPanel, SpacingClickHdl, ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();
    OSL_ASSERT(nId == TBI_SPACING);
    if(nId == TBI_SPACING)
    {
        pToolBox->SetItemDown( nId, true );
        maCharSpacePopup.Rearrange(mbKernLBAvailable,mbKernAvailable,mlKerning);
        maCharSpacePopup.Show(*pToolBox);

    }
    return 0L;
}




IMPL_LINK( TextPropertyPanel, ImplPopupModeEndHdl, FloatingWindow*, EMPTYARG )
{
    return 0;
}




void TextPropertyPanel::NotifyItemUpdate (
    const sal_uInt16 nSID,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    switch(nSID)
    {
        case SID_ATTR_CHAR_FONT:
        {
            bool bIsControlEnabled (bIsEnabled);
            if (  eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxFontItem) )
            {
                const SvxFontItem* pFontItem = (const SvxFontItem*)pState;
                mpFontNameBox->SetText( pFontItem->GetFamilyName() );
            }
            else
            {
                mpFontNameBox->SetText( String() );
                if (SFX_ITEM_DISABLED == eState)
                    bIsControlEnabled = false;
            }
            mpFontNameBox->Enable(bIsControlEnabled);
            break;
        }
        case SID_ATTR_CHAR_FONTHEIGHT:
        {
            bool bIsControlEnabled (bIsEnabled);
            if (  eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxFontHeightItem) )
            {
                mpHeightItem = (SvxFontHeightItem*)pState;//const SvxFontHeightItem*
                SfxMapUnit eUnit = maFontSizeControl.GetCoreMetric();
                long iValue = (long)CalcToPoint( mpHeightItem->GetHeight(), eUnit, 10 );
                mpToolBoxIncDec->Enable();

                mpToolBoxIncDec->SetItemState(TBI_INCREASE, STATE_NOCHECK);
                mpToolBoxIncDec->SetItemState(TBI_DECREASE, STATE_NOCHECK);

                if( mbFocusOnFontSizeCtrl )
                    return;

                maFontSizeBox.SetValue( iValue );
                maFontSizeBox.LoseFocus();

                UpdateItem(SID_SHRINK_FONT_SIZE);
                UpdateItem(SID_GROW_FONT_SIZE);
            }
            else
            {
                mpHeightItem = NULL;
                maFontSizeBox.SetText( String() );
                //increase decrease diabled when multi-seletion have different font size

                // font size +/- enhancement in sd
                switch(maContext.GetCombinedContext_DI())
                {
                    case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
                    case CombinedEnumContext(Application_DrawImpress, Context_Text):
                    case CombinedEnumContext(Application_DrawImpress, Context_Table):
                    case CombinedEnumContext(Application_DrawImpress, Context_OutlineText):
                    case CombinedEnumContext(Application_DrawImpress, Context_Draw):
                    case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
                    case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
                        break;

                    default:
                        mpToolBoxIncDec->Disable();
                }
                if ( eState <= SFX_ITEM_READONLY )
                    bIsControlEnabled = false;
            }
            maFontSizeBox.Enable(bIsControlEnabled);
            break;
        }

        case SID_ATTR_CHAR_WEIGHT:
            mbWeightAvailable = (eState >= SFX_ITEM_DONTCARE);
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxWeightItem))
            {
                const SvxWeightItem* pItem = (const SvxWeightItem*)pState;
                meWeight = (FontWeight)pItem->GetValue();
            }
            else
            {
                meWeight = WEIGHT_NORMAL;
            }
            mpToolBoxFont->EnableItem(TBI_BOLD, mbWeightAvailable && bIsEnabled);
            mpToolBoxFont->SetItemState(TBI_BOLD, meWeight==WEIGHT_BOLD ? STATE_CHECK : STATE_NOCHECK);
            break;

        case SID_ATTR_CHAR_POSTURE:
            mbPostureAvailable = (eState >= SFX_ITEM_DONTCARE);
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxPostureItem))
            {
                const SvxPostureItem* pItem = (const SvxPostureItem*)pState;
                meItalic = (FontItalic)pItem->GetValue();
            }
            else
            {
                meItalic = ITALIC_NONE;
            }
            mpToolBoxFont->EnableItem(TBI_ITALIC, mbPostureAvailable && bIsEnabled);
            mpToolBoxFont->SetItemState(TBI_ITALIC, meItalic==ITALIC_NORMAL ? STATE_CHECK : STATE_NOCHECK);
            break;

        case SID_ATTR_CHAR_UNDERLINE:
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
            mpToolBoxFont->EnableItem(TBI_UNDERLINE, bIsEnabled);
            mpToolBoxFont->SetItemState(TBI_UNDERLINE, meUnderline==UNDERLINE_NONE ? STATE_NOCHECK : STATE_CHECK);
            break;

        case SID_ATTR_CHAR_SHADOWED:
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxShadowedItem))
            {
                const SvxShadowedItem* pItem = (const SvxShadowedItem*)pState;
                mbShadow = pItem->GetValue();
            }
            else
            {
                mbShadow = false;
            }
            mpToolBoxFont->EnableItem(TBI_SHADOWED, bIsEnabled);
            mpToolBoxFont->SetItemState(TBI_SHADOWED, mbShadow ? STATE_CHECK : STATE_NOCHECK);
            break;

        case SID_ATTR_CHAR_STRIKEOUT:
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxCrossedOutItem))
            {
                const SvxCrossedOutItem* pItem = (const SvxCrossedOutItem*)pState;
                meStrike = (FontStrikeout)pItem->GetValue();
            }
            else
            {
                meStrike = STRIKEOUT_NONE;
            }
            mpToolBoxFont->EnableItem(TBI_STRIKEOUT, bIsEnabled);
            mpToolBoxFont->SetItemState(TBI_STRIKEOUT,
                meStrike!=STRIKEOUT_NONE && meStrike!=STRIKEOUT_DONTKNOW
                    ? STATE_CHECK
                    : STATE_NOCHECK);
            break;

        case SID_ATTR_CHAR_COLOR:
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxColorItem))
            {
                const SvxColorItem* pItem =  (const SvxColorItem*)pState;
                maColor = pItem->GetValue();
                mbColorAvailable = true;
                if (mpFontColorUpdater)
                    mpFontColorUpdater->Update(maColor);
            }
            else
            {
                mbColorAvailable = false;
                maColor.SetColor(COL_AUTO);
                if (mpFontColorUpdater)
                    mpFontColorUpdater->Update(maColor);
            }
            mpToolBoxFontColor->EnableItem(TBI_FONTCOLOR, bIsEnabled);
            break;

        case SID_ATTR_BRUSH_CHAR:
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxBrushItem))
            {
                const SvxBrushItem* pItem =  (const SvxBrushItem*)pState;
                maBackColor = pItem->GetColor();
                mbBackColorAvailable = true;
                if (mpHighlightUpdater)
                    mpHighlightUpdater->Update(maBackColor);
            }
            else
            {
                mbBackColorAvailable = false;
                maBackColor.SetColor(COL_AUTO);
                if (mpHighlightUpdater)
                    mpHighlightUpdater->Update(maBackColor);
            }
            mpToolBoxHighlight->EnableItem(TBI_HIGHLIGHT, bIsEnabled);
            break;

        case SID_ATTR_CHAR_ESCAPEMENT:
        {
            bool bIsItemEnabled (true);
            if (eState == SFX_ITEM_AVAILABLE)
            {
                if (pState->ISA(SvxEscapementItem))
                {
                    const SvxEscapementItem* pItem = (const SvxEscapementItem *)pState;
                    short nEsc = pItem->GetEsc();
                    if(nEsc == 0)
                    {
                        meEscape = SVX_ESCAPEMENT_OFF;
                        mpToolBoxScriptSw->SetItemState(TBI_SUPER_SW, STATE_NOCHECK);
                        mpToolBoxScriptSw->SetItemState(TBI_SUB_SW, STATE_NOCHECK);
                    }
                    else if(nEsc > 0)
                    {
                        meEscape = SVX_ESCAPEMENT_SUPERSCRIPT;
                        mpToolBoxScriptSw->SetItemState(TBI_SUPER_SW, STATE_CHECK);
                        mpToolBoxScriptSw->SetItemState(TBI_SUB_SW, STATE_NOCHECK);
                    }
                    else
                    {
                        meEscape = SVX_ESCAPEMENT_SUBSCRIPT;
                        mpToolBoxScriptSw->SetItemState(TBI_SUPER_SW, STATE_NOCHECK);
                        mpToolBoxScriptSw->SetItemState(TBI_SUB_SW, STATE_CHECK);
                    }
                }
                else
                {
                    meEscape = SVX_ESCAPEMENT_OFF;
                    mpToolBoxScriptSw->SetItemState(TBI_SUPER_SW, STATE_NOCHECK);
                    mpToolBoxScriptSw->SetItemState(TBI_SUB_SW, STATE_NOCHECK);
                }
            }
            else if (eState == SFX_ITEM_DISABLED)
            {
                bIsItemEnabled = false;
            }
            else
            {
                meEscape = SVX_ESCAPEMENT_OFF;
            }
            mpToolBoxScriptSw->EnableItem(TBI_SUPER, bIsItemEnabled && bIsEnabled);
            mpToolBoxScriptSw->EnableItem(TBI_SUB, bIsItemEnabled && bIsEnabled);
            break;
        }

        case SID_SET_SUB_SCRIPT:
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SfxBoolItem))
            {
                const SfxBoolItem* pItem = (const SfxBoolItem*)pState;
                mbSub = pItem->GetValue();
            }
            else
            {
                mbSub = false;
            }
            mpToolBoxScript->EnableItem(TBI_SUB, bIsEnabled);
            mpToolBoxScript->SetItemState(TBI_SUB, mbSub ? STATE_CHECK : STATE_NOCHECK);
            break;

        case SID_SET_SUPER_SCRIPT:
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SfxBoolItem))
            {
                const SfxBoolItem* pItem = (const SfxBoolItem*)pState;
                mbSuper = pItem->GetValue();
            }
            else
            {
                mbSuper = false;
            }
            mpToolBoxScript->EnableItem(TBI_SUPER, bIsEnabled);
            mpToolBoxScript->SetItemState(TBI_SUPER, mbSuper ? STATE_CHECK : STATE_NOCHECK);
            break;

        case SID_ATTR_CHAR_KERNING:
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
            mpToolBoxSpacing->EnableItem(TBI_SPACING, bIsEnabled);
            break;

            // font size +/- enhancement in sd
        case SID_SHRINK_FONT_SIZE:
        case SID_GROW_FONT_SIZE:
            switch(maContext.GetCombinedContext_DI())
            {
                case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
                case CombinedEnumContext(Application_DrawImpress, Context_Text):
                case CombinedEnumContext(Application_DrawImpress, Context_Table):
                case CombinedEnumContext(Application_DrawImpress, Context_OutlineText):
                case CombinedEnumContext(Application_DrawImpress, Context_Draw):
                case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
                case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
                    if(eState == SFX_ITEM_DISABLED)
                        mpToolBoxIncDec->Disable();
                    else
                        mpToolBoxIncDec->Enable();
                break;
            }
            const sal_Int32 nSize (maFontSizeBox.GetValue());
            if (nSID == SID_GROW_FONT_SIZE)
                mpToolBoxIncDec->EnableItem(TBI_INCREASE, bIsEnabled && nSize<960);
            else
                mpToolBoxIncDec->EnableItem(TBI_DECREASE, bIsEnabled && nSize>60);
            break;
    }
}




void TextPropertyPanel::UpdateItem (const sal_uInt16 nSlotId)
{
    switch (nSlotId)
    {
        case SID_ATTR_CHAR_FONT:
            maFontNameControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_FONTHEIGHT:
            maFontSizeControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_WEIGHT:
            maWeightControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_POSTURE:
            maItalicControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_UNDERLINE:
            maUnderlineControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_STRIKEOUT:
            maStrikeControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_SHADOWED:
            maShadowControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_COLOR:
            maFontColorControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_ESCAPEMENT:
            maScriptControlSw.RequestUpdate();
            break;
        case SID_SET_SUPER_SCRIPT:
            maSuperScriptControl.RequestUpdate();
            break;
        case SID_SET_SUB_SCRIPT:
            maSubScriptControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_KERNING:
            maSpacingControl.RequestUpdate();
            break;
        case SID_ATTR_BRUSH_CHAR:
            maHighlightControl.RequestUpdate();
            break;
        case SID_GROW_FONT_SIZE:
            maSDFontGrow.RequestUpdate();
            break;
        case SID_SHRINK_FONT_SIZE:
            maSDFontShrink.RequestUpdate();
            break;
    }
}





void TextPropertyPanel::SetFontColor (
    const String& /*rsColorName*/,
    const Color aColor)
{
    SvxColorItem aColorItem(aColor, SID_ATTR_CHAR_COLOR);
    mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_COLOR, SFX_CALLMODE_RECORD, &aColorItem, 0L);
    maColor = aColor;
}

void TextPropertyPanel::SetBrushColor (
    const String& /*rsColorName*/,
    const Color aColor)
{
    SvxBrushItem aBrushItem(aColor, SID_ATTR_BRUSH_CHAR);
    mpBindings->GetDispatcher()->Execute(SID_ATTR_BRUSH_CHAR, SFX_CALLMODE_RECORD, &aBrushItem, 0L);
    maBackColor = aColor;
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
