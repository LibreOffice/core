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
#include <svx/sidebar/ColorControl.hxx>

#include "ParaPropertyPanel.hxx"
#include "ParaPropertyPanel.hrc"

#include "ParaLineSpacingPopup.hxx"
#include "ParaLineSpacingControl.hxx"
#include "ParaBulletsPopup.hxx"
#include "ParaBulletsControl.hxx"
#include "ParaNumberingPopup.hxx"
#include "ParaNumberingControl.hxx"
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <svx/sidebar/PopupContainer.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/svapp.hxx>
#include <svx/svxids.hrc>
#include <svl/intitem.hxx>
#include "svx/dialmgr.hxx"
#include <sfx2/objsh.hxx>
#include <svtools/unitconv.hxx>
#include <boost/bind.hpp>
using namespace css;
using namespace cssu;
using ::sfx2::sidebar::Theme;
using ::sfx2::sidebar::ControlFactory;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))


namespace svx {namespace sidebar {
#define DEFAULT_VALUE          0
#define TOOLBOX_ITEM1          1
#define TOOLBOX_ITEM2          2
#define TOOLBOX_ITEM3          3
#define TOOLBOX_ITEM4          4

#define MAX_DURCH             5670

#define INDENT_INCREMENT       1
#define INDENT_DECREMENT       2
#define INDENT_STEP            706
#define UL_STEP                58
#define UL_INCREMENT           1
#define UL_DECREMENT           2

#define MAX_SW                  1709400
#define MAX_SC_SD               116220200
#define NEGA_MAXVALUE          -10000000

#define POS_FT_PARA_SPACE                               Point(LogicToPixel(Point(FT_PARA_SPACE_X,FT_PARA_SPACE_Y), MAP_APPFONT))
#define POS_FT_PARA_SPACE2                              Point(LogicToPixel(Point(FT_PARA_SPACE_X,FT_PARA_SPACE_Y2), MAP_APPFONT))
#define POS_TBX_PARA_SPACE_INC_DEC                      Point(LogicToPixel(Point(TBX_PARA_SPACE_INC_DEC_X,TBX_PARA_SPACE_INC_DEC_Y), MAP_APPFONT))
#define POS_TBX_PARA_SPACE_INC_DEC2                     Point(LogicToPixel(Point(TBX_PARA_SPACE_INC_DEC_X,TBX_PARA_SPACE_INC_DEC_Y2), MAP_APPFONT))
#define POS_FT_PARA_INDENT                              Point(LogicToPixel(Point(FT_PARA_INDENT_X,FT_PARA_INDENT_Y), MAP_APPFONT))
#define POS_FT_PARA_INDENT2                             Point(LogicToPixel(Point(FT_PARA_INDENT_X,FT_PARA_INDENT_Y2), MAP_APPFONT))
#define POS_TBX_PARA_INDENT_INC_DEC                     Point(LogicToPixel(Point(TBX_PARA_INDENT_INC_DEC_X,TBX_PARA_INDENT_INC_DEC_Y), MAP_APPFONT))
#define POS_TBX_PARA_INDENT_INC_DEC2                    Point(LogicToPixel(Point(TBX_PARA_INDENT_INC_DEC_X,TBX_PARA_INDENT_INC_DEC_Y2), MAP_APPFONT))
#define POS_MBX_TOP_DIST                                Point(LogicToPixel(Point(MBX_TOP_DIST_X,MBX_TOP_DIST_Y), MAP_APPFONT))
#define POS_MBX_TOP_DIST2                               Point(LogicToPixel(Point(MBX_TOP_DIST_X,MBX_TOP_DIST_Y2), MAP_APPFONT))
#define POS_MBX_BOT_DIST                                Point(LogicToPixel(Point(MBX_BOT_DIST_X,MBX_BOT_DIST_Y), MAP_APPFONT))
#define POS_MBX_BOT_DIST2                               Point(LogicToPixel(Point(MBX_BOT_DIST_X,MBX_BOT_DIST_Y2), MAP_APPFONT))
#define POS_MBX_LEFT_DIST                               Point(LogicToPixel(Point(MBX_LEFT_DIST_X,MBX_LEFT_DIST_Y), MAP_APPFONT))
#define POS_MBX_LEFT_DIST2                              Point(LogicToPixel(Point(MBX_LEFT_DIST_X,MBX_LEFT_DIST_Y2), MAP_APPFONT))
#define POS_MBX_RIGHT_DIST                              Point(LogicToPixel(Point(MBX_RIGHT_DIST_X,MBX_RIGHT_DIST_Y), MAP_APPFONT))
#define POS_MBX_RIGHT_DIST2                             Point(LogicToPixel(Point(MBX_RIGHT_DIST_X,MBX_RIGHT_DIST_Y2), MAP_APPFONT))
#define POS_MBX_FLINE_DIST                              Point(LogicToPixel(Point(MBX_FLINE_DIST_X,MBX_FLINE_DIST_Y), MAP_APPFONT))
#define POS_MBX_FLINE_DIST2                             Point(LogicToPixel(Point(MBX_FLINE_DIST_X,MBX_FLINE_DIST_Y2), MAP_APPFONT))
#define POS_IMG_SPACE1                                  Point(LogicToPixel(Point(IMG_SPACE_X,IMG_SPACE1_Y), MAP_APPFONT))
#define POS_IMG_SPACE12                                 Point(LogicToPixel(Point(IMG_SPACE_X,IMG_SPACE1_Y2), MAP_APPFONT))
#define POS_IMG_SPACE2                                  Point(LogicToPixel(Point(IMG_SPACE_X,IMG_SPACE2_Y), MAP_APPFONT))
#define POS_IMG_SPACE22                                 Point(LogicToPixel(Point(IMG_SPACE_X,IMG_SPACE2_Y2), MAP_APPFONT))
#define POS_IMG_INDENT1                                 Point(LogicToPixel(Point(IMG_INDENT_X,IMG_INDENT1_Y), MAP_APPFONT))
#define POS_IMG_INDENT12                                Point(LogicToPixel(Point(IMG_INDENT_X,IMG_INDENT1_Y2), MAP_APPFONT))
#define POS_IMG_INDENT2                                 Point(LogicToPixel(Point(IMG_INDENT_X,IMG_INDENT2_Y), MAP_APPFONT))
#define POS_IMG_INDENT22                                Point(LogicToPixel(Point(IMG_INDENT_X,IMG_INDENT2_Y2), MAP_APPFONT))
#define POS_IMG_INDENT3                                 Point(LogicToPixel(Point(IMG_INDENT_X,IMG_INDENT3_Y), MAP_APPFONT))
#define POS_IMG_INDENT32                                Point(LogicToPixel(Point(IMG_INDENT_X,IMG_INDENT3_Y2), MAP_APPFONT))
#define TBX_LINE_SPACE                                  Point(LogicToPixel(Point(TBX_LINE_SPACE_X,TBX_LINE_SPACE_Y), MAP_APPFONT))
#define TBX_LINE_SPACE2                                 Point(LogicToPixel(Point(TBX_LINE_SPACE_X,TBX_LINE_SPACE_Y2), MAP_APPFONT))

ParaPropertyPanel* ParaPropertyPanel::Create (
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const cssu::Reference<css::ui::XSidebar>& rxSidebar)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException(A2S("no parent Window given to ParaPropertyPanel::Create"), NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException(A2S("no XFrame given to ParaPropertyPanel::Create"), NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException(A2S("no SfxBindings given to ParaPropertyPanel::Create"), NULL, 2);

    return new ParaPropertyPanel(
        pParent,
        rxFrame,
        pBindings,
        rxSidebar);
}

void ParaPropertyPanel::HandleContextChange (
    const ::sfx2::sidebar::EnumContext aContext)
{
    if (maContext == aContext)
    {
        // Nothing to do.
        return;
    }

    maContext = aContext;
    switch (maContext.GetCombinedContext_DI())
    {
        case CombinedEnumContext(Application_Calc, Context_DrawText):
            maTBxVertAlign->Show();
            maTBxVertAlignBackground->Show();
            maTBxBackColor->Hide();
            maTBxBackColorBackground->Hide();
            maTBxNumBullet->Hide();
            maTBxNumBulletBackground->Hide();
            ReSize(false);
            maTbxIndent_IncDec->Show();
            maTbxIndent_IncDecBackground->Show();
            maTbxProDemote->Hide();
            maTbxProDemoteBackground->Hide();
            break;

        case CombinedEnumContext(Application_DrawImpress, Context_Draw):
        case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
            maTBxVertAlign->Hide();
            maTBxVertAlignBackground->Hide();
            maTBxBackColor->Hide();
            maTBxBackColorBackground->Hide();
            maTBxNumBullet->Show();
            maTBxNumBulletBackground->Show();
            ReSize(true);
            maTbxIndent_IncDec->Hide();
            maTbxIndent_IncDecBackground->Hide();
            maTbxProDemote->Show();
            maTbxProDemoteBackground->Show();
            break;

        case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
            maTBxVertAlign->Show();
            maTBxVertAlignBackground->Show();
            maTBxBackColor->Hide();
            maTBxBackColorBackground->Hide();
            maTBxNumBullet->Show();
            maTBxNumBulletBackground->Show();
            ReSize(true);
            maTbxIndent_IncDec->Hide();
            maTbxIndent_IncDecBackground->Hide();
            maTbxProDemote->Show();
            maTbxProDemoteBackground->Show();
            break;

        case CombinedEnumContext(Application_DrawImpress, Context_Table):
            maTBxVertAlign->Show();
            maTBxVertAlignBackground->Show();
            maTBxBackColor->Hide();
            maTBxBackColorBackground->Hide();
            maTBxNumBullet->Show();
            maTBxNumBulletBackground->Show();
            ReSize(true);
            maTbxIndent_IncDec->Hide();
            maTbxIndent_IncDecBackground->Hide();
            maTbxProDemote->Show();
            maTbxProDemoteBackground->Show();
            break;

        case CombinedEnumContext(Application_WriterVariants, Context_Default):
        case CombinedEnumContext(Application_WriterVariants, Context_Text):
            maTBxVertAlign->Hide();
            maTBxVertAlignBackground->Hide();
            maTBxBackColor->Show();
            maTBxBackColorBackground->Show();
            maTBxNumBullet->Show();

            maTBxNumBulletBackground->Show();
            ReSize(true);
            maTbxIndent_IncDec->Show();
            maTbxIndent_IncDecBackground->Show();
            maTbxProDemote->Hide();
            maTbxProDemoteBackground->Hide();
            break;

        case CombinedEnumContext(Application_WriterVariants, Context_Table):
            maTBxVertAlign->Show();
            maTBxVertAlignBackground->Show();
            maTBxBackColor->Show();
            maTBxBackColorBackground->Show();
            maTBxNumBullet->Show();
            maTBxNumBulletBackground->Show();
            ReSize(true);
            maTbxIndent_IncDec->Show();
            maTbxIndent_IncDecBackground->Show();
            maTbxProDemote->Hide();
            maTbxProDemoteBackground->Hide();
            break;

        case CombinedEnumContext(Application_WriterVariants, Context_DrawText):
            maTBxVertAlign->Show();
            maTBxVertAlignBackground->Show();
            maTBxBackColor->Hide();
            maTBxBackColorBackground->Hide();
            maTBxNumBullet->Hide();
            maTBxNumBulletBackground->Hide();
            ReSize(false);
            maTbxIndent_IncDec->Show();
            maTbxIndent_IncDecBackground->Show();
            maTbxProDemote->Hide();
            maTbxProDemoteBackground->Hide();
            break;

        case CombinedEnumContext(Application_WriterVariants, Context_Annotation):
            maTBxVertAlign->Hide();
            maTBxVertAlignBackground->Hide();
            maTBxBackColor->Hide();
            maTBxBackColorBackground->Hide();
            maTBxNumBullet->Hide();
            maTBxNumBulletBackground->Hide();
            ReSize(false);
            maTbxIndent_IncDec->Show();
            maTbxIndent_IncDecBackground->Show();
            maTbxProDemote->Hide();
            maTbxProDemoteBackground->Hide();
            break;

        case CombinedEnumContext(Application_Calc, Context_EditCell):
        case CombinedEnumContext(Application_Calc, Context_Cell):
        case CombinedEnumContext(Application_Calc, Context_Pivot):
        case CombinedEnumContext(Application_DrawImpress, Context_Text):
        case CombinedEnumContext(Application_DrawImpress, Context_OutlineText):
        /*{
            mpToolBoxScriptSw->Hide();
            mpToolBoxScript->Show();
            mpToolBoxSpacing->Show();
            mpToolBoxHighlight->Hide();

            Size aSize(PROPERTYPAGE_WIDTH,TEXT_SECTIONPAGE_HEIGHT);
            aSize = LogicToPixel( aSize,MapMode(MAP_APPFONT) );
            aSize.setWidth(GetOutputSizePixel().Width());
            SetSizePixel(aSize);
        }*/
            break;

        default:
            break;
    }
}

SfxBindings* ParaPropertyPanel::GetBindings()
{
    return mpBindings;
}




void ParaPropertyPanel::DataChanged (const DataChangedEvent& rEvent)
{
    (void)rEvent;
}

ParaPropertyPanel::~ParaPropertyPanel()
{
    delete mpLnSPItem;
}

void ParaPropertyPanel::ReSize(bool bSize)
{
    if(bSize)
    {
        //Paragraph spacing
        maFTUL->SetPosPixel(POS_FT_PARA_SPACE);
        maTbxUL_IncDec->SetPosPixel(POS_TBX_PARA_SPACE_INC_DEC);
        maTopDist->SetPosPixel(POS_MBX_TOP_DIST);
        maBottomDist->SetPosPixel(POS_MBX_BOT_DIST);
        //Indent
        maFTIndent->SetPosPixel(POS_FT_PARA_INDENT);
        maTbxIndent_IncDec->SetPosPixel(POS_TBX_PARA_INDENT_INC_DEC);
        maTbxProDemote->SetPosPixel(POS_TBX_PARA_INDENT_INC_DEC);
        maLeftIndent->SetPosPixel(POS_MBX_LEFT_DIST);
        maRightIndent->SetPosPixel(POS_MBX_RIGHT_DIST);
        maFLineIndent->SetPosPixel(POS_MBX_FLINE_DIST);
        //Line spacing
        maLineSPTbx->SetPosPixel(TBX_LINE_SPACE);
        //image
        maFISpace1.SetPosPixel(POS_IMG_SPACE1);
        maFISpace2.SetPosPixel(POS_IMG_SPACE2);
        maFIndent1.SetPosPixel(POS_IMG_INDENT1);
        maFIndent2.SetPosPixel(POS_IMG_INDENT2);
        maFIndent3.SetPosPixel(POS_IMG_INDENT3);
        Size aSize(GetOutputSizePixel().Width(),PARA_SECTIONPAGE_HEIGHT);
        aSize = LogicToPixel( aSize, MapMode(MAP_APPFONT) );
        aSize.setWidth(GetOutputSizePixel().Width());
        SetSizePixel(aSize);
    }
    else
    {
        //Paragraph spacing
        maFTUL->SetPosPixel(POS_FT_PARA_SPACE2);
        maTbxUL_IncDec->SetPosPixel(POS_TBX_PARA_SPACE_INC_DEC2);
        maTopDist->SetPosPixel(POS_MBX_TOP_DIST2);
        maBottomDist->SetPosPixel(POS_MBX_BOT_DIST2);
        //Indent
        maFTIndent->SetPosPixel(POS_FT_PARA_INDENT2);
        maTbxIndent_IncDec->SetPosPixel(POS_TBX_PARA_INDENT_INC_DEC2);
        maTbxProDemote->SetPosPixel(POS_TBX_PARA_INDENT_INC_DEC2);
        maLeftIndent->SetPosPixel(POS_MBX_LEFT_DIST2);
        maRightIndent->SetPosPixel(POS_MBX_RIGHT_DIST2);
        maFLineIndent->SetPosPixel(POS_MBX_FLINE_DIST2);
        //Line spacing
        maLineSPTbx->SetPosPixel(TBX_LINE_SPACE2);
        //image
        maFISpace1.SetPosPixel(POS_IMG_SPACE12);
        maFISpace2.SetPosPixel(POS_IMG_SPACE22);
        maFIndent1.SetPosPixel(POS_IMG_INDENT12);
        maFIndent2.SetPosPixel(POS_IMG_INDENT22);
        maFIndent3.SetPosPixel(POS_IMG_INDENT32);
        Size aSize(GetOutputSizePixel().Width(),PARA_SECTIONPAGE_HEIGHT_2);
        aSize = LogicToPixel( aSize, MapMode(MAP_APPFONT) );
        aSize.setWidth(GetOutputSizePixel().Width());
        SetSizePixel(aSize);
    }

    if (mxSidebar.is())
        mxSidebar->requestLayout();
}

void ParaPropertyPanel::EndSpacingPopupMode (void)
{
    maLineSpacePopup.Hide();
}

void ParaPropertyPanel::EndBulletsPopupMode (void)
{
    //i122054, Missed following line, for collapse the bullets popup
    maBulletsPopup.Hide();
}

void ParaPropertyPanel::EndNumberingPopupMode (void)
{
    //i122054, Missed following line, for collapse the numbering popup
    maNumberingPopup.Hide();
}


void ParaPropertyPanel::InitToolBoxAlign()
{
    maAlignToolBox->SetItemImage(TOOLBOX_ITEM1, maLeftAlignControl.GetIcon());
    maAlignToolBox->SetItemImage(TOOLBOX_ITEM2, maCenterAlignControl.GetIcon());
    maAlignToolBox->SetItemImage(TOOLBOX_ITEM3, maRightAlignControl.GetIcon());
    maAlignToolBox->SetItemImage(TOOLBOX_ITEM4, maJustifyAlignControl.GetIcon());

    Link aLink = LINK( this, ParaPropertyPanel, AlignStyleModifyHdl_Impl );
    maAlignToolBox->SetSelectHdl( aLink );
    Size aTbxSize (maAlignToolBox->CalcWindowSizePixel());
    maAlignToolBox->SetOutputSizePixel( aTbxSize );
}

void ParaPropertyPanel::InitToolBoxVertAlign()
{
    maTBxVertAlign->SetItemImage(IID_VERT_TOP, maVertTop.GetIcon());
    maTBxVertAlign->SetItemImage(IID_VERT_CENTER, maVertCenter.GetIcon());
    maTBxVertAlign->SetItemImage(IID_VERT_BOTTOM, maVertBottom.GetIcon());
    maTBxVertAlign->SetSelectHdl(LINK(this,ParaPropertyPanel,VertTbxSelectHandler));
    Size aTbxSize (maTBxVertAlign->CalcWindowSizePixel());
    maTBxVertAlign->SetOutputSizePixel( aTbxSize );
}


void ParaPropertyPanel::InitToolBoxIndent()
{
    Link aLink = LINK( this, ParaPropertyPanel, ModifyIndentHdl_Impl );
    maLeftIndent->SetModifyHdl( aLink );
    maRightIndent->SetModifyHdl( aLink );
    maFLineIndent->SetModifyHdl( aLink );

    if( Application::GetSettings().GetLayoutRTL())
    {
        maTbxIndent_IncDec->SetItemImage(TOOLBOX_ITEM1, maIncIndentControl.GetIcon());
        maTbxIndent_IncDec->SetItemImage(TOOLBOX_ITEM2, maDecIndentControl.GetIcon());
    }
    else
    {
        maTbxIndent_IncDec->SetItemImage(TOOLBOX_ITEM1, maIncIndentControl.GetIcon());
        maTbxIndent_IncDec->SetItemImage(TOOLBOX_ITEM2, maDecIndentControl.GetIcon());
    }
    maTbxIndent_IncDec->SetItemImage(TOOLBOX_ITEM3,maIndHang);
    aLink = LINK( this, ParaPropertyPanel, ClickIndent_IncDec_Hdl_Impl );
    maTbxIndent_IncDec->SetSelectHdl(aLink);
    m_eLRSpaceUnit = maLRSpaceControl.GetCoreMetric();
    Size aTbxSize = maTbxIndent_IncDec->CalcWindowSizePixel();
    maTbxIndent_IncDec->SetOutputSizePixel( aTbxSize );

    if( Application::GetSettings().GetLayoutRTL())
    {
        maTbxProDemote->SetItemImage(TOOLBOX_ITEM1, maOutLineLeftControl.GetIcon());
        maTbxProDemote->SetItemImage(TOOLBOX_ITEM2, maOutLineRightControl.GetIcon());
    }
    else
    {
        maTbxProDemote->SetItemImage(TOOLBOX_ITEM1, maOutLineLeftControl.GetIcon());
        maTbxProDemote->SetItemImage(TOOLBOX_ITEM2, maOutLineRightControl.GetIcon());
    }
    maTbxProDemote->SetItemImage(TOOLBOX_ITEM3,maIndHang);
    aLink = LINK( this, ParaPropertyPanel, ClickProDemote_Hdl_Impl );
    maTbxProDemote->SetSelectHdl(aLink);
    m_eLRSpaceUnit = maLRSpaceControl.GetCoreMetric();
    aTbxSize = maTbxProDemote->CalcWindowSizePixel();
    maTbxProDemote->SetOutputSizePixel( aTbxSize );
}

void ParaPropertyPanel::InitToolBoxBGColor()
{
    mpColorUpdater.reset(new ::svx::ToolboxButtonColorUpdater(SID_BACKGROUND_COLOR, TBI_BACK_COLOR, maTBxBackColor.get(), TBX_UPDATER_MODE_CHAR_COLOR_NEW ));

    maTBxBackColor->SetItemImage(TBI_BACK_COLOR, GetDisplayBackground().GetColor().IsDark()? maImgBackColorHigh : maImgBackColor);
    maTBxBackColor->SetItemBits( TBI_BACK_COLOR, maTBxBackColor->GetItemBits( TBI_BACK_COLOR ) | TIB_DROPDOWNONLY );

    Link aLink = LINK(this, ParaPropertyPanel, ToolBoxBackColorDDHandler);
    maTBxBackColor->SetDropdownClickHdl ( aLink );
    maTBxBackColor->SetSelectHdl ( aLink );
    Size aTbxSize (maTBxBackColor->CalcWindowSizePixel());
    maTBxBackColor->SetOutputSizePixel( aTbxSize );
}

void ParaPropertyPanel::InitToolBoxBulletsNumbering()
{
    maTBxNumBullet->SetItemImage(IID_BULLET, maBulletOnOff.GetIcon());
    maTBxNumBullet->SetItemImage(IID_NUMBER, maNumberOnOff.GetIcon());

    maTBxNumBullet->SetDropdownClickHdl(LINK(this,ParaPropertyPanel,NumBTbxDDHandler));
    maTBxNumBullet->SetSelectHdl(LINK(this,ParaPropertyPanel,NumBTbxSelectHandler));
    Size aTbxSize (maTBxNumBullet->CalcWindowSizePixel());
    maTBxNumBullet->SetOutputSizePixel( aTbxSize );
}
void ParaPropertyPanel::InitToolBoxSpacing()
{
    Link aLink = LINK( this, ParaPropertyPanel, ULSpaceHdl_Impl );
    maTopDist->SetModifyHdl(aLink);
    maBottomDist->SetModifyHdl( aLink );

    maTbxUL_IncDec->SetItemImage(TOOLBOX_ITEM1, maParInc);
    maTbxUL_IncDec->SetItemImage(TOOLBOX_ITEM2, maParDec);
    aLink = LINK( this, ParaPropertyPanel, ClickUL_IncDec_Hdl_Impl );
    maTbxUL_IncDec->SetSelectHdl(aLink);
    m_eULSpaceUnit = maULSpaceControl.GetCoreMetric();
    Size aTbxSize = maTbxUL_IncDec->CalcWindowSizePixel();
    maTbxUL_IncDec->SetOutputSizePixel( aTbxSize );
}
void ParaPropertyPanel::InitToolBoxLineSpacing()
{
    Link aLink = LINK( this, ParaPropertyPanel, ClickLineSPDropDownHdl_Impl );
    maLineSPTbx->SetDropdownClickHdl( aLink );
    maLineSPTbx->SetSelectHdl( aLink );     //support keyinput "ENTER"
    maLineSPTbx->SetItemBits( TOOLBOX_ITEM1, maLineSPTbx->GetItemBits( TOOLBOX_ITEM1 ) | TIB_DROPDOWNONLY );
    maLineSPTbx->SetItemImage(TOOLBOX_ITEM1,maSpace3);
    Size aTbxSize = maLineSPTbx->CalcWindowSizePixel();
    maLineSPTbx->SetOutputSizePixel( aTbxSize );
}

void ParaPropertyPanel::initial()
{
    maFTUL->SetBackground(Wallpaper());
    maFTIndent->SetBackground(Wallpaper());
    maFISpace1.SetBackground(Wallpaper());
    maFISpace2.SetBackground(Wallpaper());
    maFIndent1.SetBackground(Wallpaper());
    maFIndent2.SetBackground(Wallpaper());
    maFIndent3.SetBackground(Wallpaper());

    maFISpace1.SetImage(maSpace1);
    maFISpace2.SetImage(maSpace2);
    maFIndent1.SetImage(maIndent3);
    maFIndent2.SetImage(maIndent2);
    maFIndent3.SetImage(maIndent1);

    GetBindings()->Invalidate(SID_ATTR_PARA_ADJUST_LEFT,sal_True,sal_False);
    GetBindings()->Invalidate(SID_ATTR_PARA_ADJUST_CENTER,sal_True,sal_False);
    GetBindings()->Invalidate(SID_ATTR_PARA_ADJUST_RIGHT,sal_True,sal_False);
    GetBindings()->Invalidate(SID_ATTR_PARA_ADJUST_BLOCK,sal_True,sal_False);

    //toolbox
    InitToolBoxAlign();
    InitToolBoxVertAlign();
    InitToolBoxIndent();
    InitToolBoxBGColor();
    InitToolBoxBulletsNumbering();
    InitToolBoxSpacing();
    InitToolBoxLineSpacing();

#ifdef HAS_IA2
    maAlignToolBox->SetAccRelationLabeledBy(&maAlignToolBox);
    maTBxVertAlign->SetMpSubEditAccLableBy(&maTBxVertAlign);
    maTBxNumBullet->SetAccRelationLabeledBy(&maTBxNumBullet);
    maTBxBackColor->SetMpSubEditAccLableBy(&maTBxBackColor);
    maFTUL->SetAccRelationLabeledBy(&maFTUL);
    maTbxUL_IncDec->SetAccRelationLabeledBy(&maTbxUL_IncDec);
    maTopDist->SetAccRelationLabeledBy(&maTopDist);
    maBottomDist->SetAccRelationLabeledBy(&maBottomDist);
    maLineSPTbx->SetAccRelationLabeledBy(&maLineSPTbx);
    maFTIndent->SetAccRelationLabeledBy(&maFTIndent);
    maTbxIndent_IncDec->SetAccRelationLabeledBy(&maTbxIndent_IncDec);
    maTbxProDemote->SetAccRelationLabeledBy(&maTbxProDemote);
    maLeftIndent->SetAccRelationLabeledBy(&maLeftIndent);
    maRightIndent->SetAccRelationLabeledBy(&maRightIndent);
    maFLineIndent->SetAccRelationLabeledBy(&maFLineIndent);
    mpColorUpdater->SetAccRelationLabeledBy(&mpColorUpdater);
#endif
}

void ParaPropertyPanel::InitImageList(::boost::scoped_ptr<ToolBox>& rTbx, ImageList& rImglst, ImageList& rImgHlst)
{
    sal_Bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

    ImageList& rImgLst = bHighContrast ? rImgHlst : rImglst;

    sal_uInt16 nCount = rTbx->GetItemCount();
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        sal_uInt16 nId = rTbx->GetItemId(i);
        rTbx->SetItemImage( nId, rImgLst.GetImage( nId ) );
    }
}

//===========================for Numbering & Bullet================================================



IMPL_LINK(ParaPropertyPanel, NumBTbxDDHandler, ToolBox*, pToolBox)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();

    EndTracking();
    pToolBox->SetItemDown( nId, sal_True );

    if (nId == 1)
    {
        maBulletsPopup.UpdateValueSet();
        maBulletsPopup.Show(*pToolBox);
    }
    else if (nId == 2)
    {
        maNumberingPopup.UpdateValueSet();
        maNumberingPopup.Show(*pToolBox);
    }
    pToolBox->SetItemDown( nId, sal_False );
    return 0;
}

IMPL_LINK(ParaPropertyPanel, NumBTbxSelectHandler, ToolBox*, pToolBox)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();
    sal_uInt16 nSID = SID_TABLE_VERT_NONE;

    EndTracking();
    if (nId == 1)
    {
        nSID = FN_NUM_BULLET_ON;
    }
    else if ( nId == 2)
    {
        nSID = FN_NUM_NUMBERING_ON;
    }
    SfxBoolItem aBoolItem(nSID, sal_True);
    GetBindings()->GetDispatcher()->Execute(nSID, SFX_CALLMODE_RECORD, &aBoolItem, 0L);

    return 0;
}


//================================for Vertical Alignment========================================



IMPL_LINK(ParaPropertyPanel, VertTbxSelectHandler, ToolBox*, pToolBox)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();
    sal_uInt16 nSID = SID_TABLE_VERT_NONE;
    EndTracking();
    if (nId == 1)
    {
        nSID = SID_TABLE_VERT_NONE;
        maTBxVertAlign->SetItemState(IID_VERT_TOP, STATE_CHECK);
        maTBxVertAlign->SetItemState(IID_VERT_CENTER, STATE_NOCHECK);
        maTBxVertAlign->SetItemState(IID_VERT_BOTTOM, STATE_NOCHECK);
    }
    else if (nId == 2)
    {
        nSID = SID_TABLE_VERT_CENTER;
        maTBxVertAlign->SetItemState(IID_VERT_TOP, STATE_NOCHECK);
        maTBxVertAlign->SetItemState(IID_VERT_CENTER, STATE_CHECK);
        maTBxVertAlign->SetItemState(IID_VERT_BOTTOM, STATE_NOCHECK);
    }
    else if (nId == 3)
    {
        nSID = SID_TABLE_VERT_BOTTOM;
        maTBxVertAlign->SetItemState(IID_VERT_TOP, STATE_NOCHECK);
        maTBxVertAlign->SetItemState(IID_VERT_CENTER, STATE_NOCHECK);
        maTBxVertAlign->SetItemState(IID_VERT_BOTTOM, STATE_CHECK);
    }
    SfxBoolItem aBoolItem(nSID, sal_True);
    GetBindings()->GetDispatcher()->Execute(nSID, SFX_CALLMODE_RECORD, &aBoolItem, 0L);

    return 0;
}

void ParaPropertyPanel::VertStateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    if (eState < SFX_ITEM_DONTCARE)
    {
        maTBxVertAlign->SetItemState(IID_VERT_TOP, STATE_NOCHECK);
        maTBxVertAlign->SetItemState(IID_VERT_CENTER, STATE_NOCHECK);
        maTBxVertAlign->SetItemState(IID_VERT_BOTTOM, STATE_NOCHECK);
        maTBxVertAlign->EnableItem(IID_VERT_TOP, sal_False);
        maTBxVertAlign->EnableItem(IID_VERT_CENTER, sal_False);
        maTBxVertAlign->EnableItem(IID_VERT_BOTTOM, sal_False);
    }
    else
    {
        maTBxVertAlign->EnableItem(IID_VERT_TOP, sal_True);
        maTBxVertAlign->EnableItem(IID_VERT_CENTER, sal_True);
        maTBxVertAlign->EnableItem(IID_VERT_BOTTOM, sal_True);
        if ( (eState >= SFX_ITEM_DEFAULT) && (pState->ISA(SfxBoolItem)))
        {
            const SfxBoolItem* pItem= (const SfxBoolItem*)pState;
            sal_Bool aBool = (sal_Bool)pItem->GetValue();

            if (aBool)
            {
                if (nSID == SID_TABLE_VERT_NONE)
                {
                    maTBxVertAlign->SetItemState(IID_VERT_TOP, STATE_CHECK);
                    maTBxVertAlign->SetItemState(IID_VERT_CENTER, STATE_NOCHECK);
                    maTBxVertAlign->SetItemState(IID_VERT_BOTTOM, STATE_NOCHECK);
                }
                else if (nSID == SID_TABLE_VERT_CENTER)
                {
                    maTBxVertAlign->SetItemState(IID_VERT_TOP, STATE_NOCHECK);
                    maTBxVertAlign->SetItemState(IID_VERT_CENTER, STATE_CHECK);
                    maTBxVertAlign->SetItemState(IID_VERT_BOTTOM, STATE_NOCHECK);
                }
                else if (nSID == SID_TABLE_VERT_BOTTOM)
                {
                    maTBxVertAlign->SetItemState(IID_VERT_TOP, STATE_NOCHECK);
                    maTBxVertAlign->SetItemState(IID_VERT_CENTER, STATE_NOCHECK);
                    maTBxVertAlign->SetItemState(IID_VERT_BOTTOM, STATE_CHECK);
                }
            }
            else
            {
                if (nSID == SID_TABLE_VERT_NONE)
                {
                    maTBxVertAlign->SetItemState(IID_VERT_TOP, STATE_NOCHECK);
                }
                else if (nSID == SID_TABLE_VERT_CENTER)
                {
                    maTBxVertAlign->SetItemState(IID_VERT_CENTER, STATE_NOCHECK);
                }
                else if (nSID == SID_TABLE_VERT_BOTTOM)
                {
                    maTBxVertAlign->SetItemState(IID_VERT_BOTTOM, STATE_NOCHECK);
                }
            }
        }
        else
        {
            maTBxVertAlign->SetItemState(IID_VERT_TOP, STATE_NOCHECK);
            maTBxVertAlign->SetItemState(IID_VERT_CENTER, STATE_NOCHECK);
            maTBxVertAlign->SetItemState(IID_VERT_BOTTOM, STATE_NOCHECK);
        }
    }
}
//==================================for Background color=====================

IMPL_LINK(ParaPropertyPanel, ToolBoxBackColorDDHandler,ToolBox*, pToolBox)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();
    OSL_ASSERT(nId == TBI_BACK_COLOR);
    if(nId == TBI_BACK_COLOR)
    {
        pToolBox->SetItemDown( nId, true );
        maBGColorPopup.Show(*pToolBox);
        maBGColorPopup.SetCurrentColor(maColor, mbColorAvailable);
    }
    return 0;
}

IMPL_LINK( ParaPropertyPanel, ImplPopupModeEndHdl, FloatingWindow*, EMPTYARG )
{
    return 0;
}

void ParaPropertyPanel::ShowMenu (void)
{
    if (mpBindings != NULL)
    {
        SfxDispatcher* pDispatcher = mpBindings->GetDispatcher();
        if (pDispatcher != NULL)
            pDispatcher->Execute(SID_PARA_DLG, SFX_CALLMODE_ASYNCHRON);
    }
}

void ParaPropertyPanel::ParaBKGStateChanged(sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState)
{
    if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxColorItem))
    {
        const SvxColorItem* pItem =  (const SvxColorItem*)pState;
        maColor = pItem->GetValue();
        mbColorAvailable = sal_True;
        mpColorUpdater->Update(maColor);
    }
    else
    {
        mbColorAvailable = sal_False;
        maColor.SetColor(COL_AUTO);
        mpColorUpdater->Update(maColor);
    }
}

void ParaPropertyPanel::SetBGColor (
    const String& /*rsColorName*/,
    const Color aColor)
{
    SvxColorItem aColorItem(aColor, SID_BACKGROUND_COLOR);
    mpBindings->GetDispatcher()->Execute(SID_BACKGROUND_COLOR, SFX_CALLMODE_RECORD, &aColorItem, 0L);
    maColor = aColor;
}

//==================================for Paragraph Alignment=====================
IMPL_LINK( ParaPropertyPanel, AlignStyleModifyHdl_Impl, ToolBox*, pBox )
{
    switch (pBox->GetCurItemId())
    {
    case BT_SUBSTLEFT:
        {
            pBox->SetItemState(BT_SUBSTLEFT,STATE_CHECK);
            pBox->SetItemState(BT_SUBSTCENTER,STATE_NOCHECK);
            pBox->SetItemState(BT_SUBSTRIGHT,STATE_NOCHECK);
            pBox->SetItemState(BT_SUBSTJUSTIFY,STATE_NOCHECK);
            SfxBoolItem aBoolItem( SID_ATTR_PARA_ADJUST_LEFT,  sal_True );
            GetBindings()->GetDispatcher()->Execute(SID_ATTR_PARA_ADJUST_LEFT, SFX_CALLMODE_RECORD, &aBoolItem, 0L);
        }
        break;
    case BT_SUBSTCENTER:
        {
            pBox->SetItemState(BT_SUBSTCENTER,STATE_CHECK);
            pBox->SetItemState(BT_SUBSTLEFT,STATE_NOCHECK);
            pBox->SetItemState(BT_SUBSTRIGHT,STATE_NOCHECK);
            pBox->SetItemState(BT_SUBSTJUSTIFY,STATE_NOCHECK);
            SfxBoolItem aBoolItem( SID_ATTR_PARA_ADJUST_CENTER, sal_True );
            GetBindings()->GetDispatcher()->Execute(SID_ATTR_PARA_ADJUST_CENTER, SFX_CALLMODE_RECORD, &aBoolItem, 0L);
        }
        break;
    case BT_SUBSTRIGHT:
        {
            pBox->SetItemState(BT_SUBSTRIGHT,STATE_CHECK);
            pBox->SetItemState(BT_SUBSTLEFT,STATE_NOCHECK);
            pBox->SetItemState(BT_SUBSTCENTER,STATE_NOCHECK);
            pBox->SetItemState(BT_SUBSTJUSTIFY,STATE_NOCHECK);
            SfxBoolItem aBoolItem( SID_ATTR_PARA_ADJUST_RIGHT, sal_True );
            GetBindings()->GetDispatcher()->Execute(SID_ATTR_PARA_ADJUST_RIGHT, SFX_CALLMODE_RECORD, &aBoolItem, 0L);
        }
        break;
    case BT_SUBSTJUSTIFY:
        {
            pBox->SetItemState(BT_SUBSTJUSTIFY,STATE_CHECK);
            pBox->SetItemState(BT_SUBSTLEFT,STATE_NOCHECK);
            pBox->SetItemState(BT_SUBSTRIGHT,STATE_NOCHECK);
            pBox->SetItemState(BT_SUBSTCENTER,STATE_NOCHECK);
            SfxBoolItem aBoolItem( SID_ATTR_PARA_ADJUST_BLOCK, sal_True );
            GetBindings()->GetDispatcher()->Execute(SID_ATTR_PARA_ADJUST_BLOCK, SFX_CALLMODE_RECORD, &aBoolItem, 0L);
        }
        break;
    }

    return 0;
}
//==================================for Paragraph Indent=====================
IMPL_LINK_NOARG( ParaPropertyPanel, ModifyIndentHdl_Impl)
{
    SvxLRSpaceItem aMargin( SID_ATTR_PARA_LRSPACE );
    aMargin.SetTxtLeft( (const long)GetCoreValue( *maLeftIndent.get(), m_eLRSpaceUnit ) );
    aMargin.SetRight( (const long)GetCoreValue( *maRightIndent.get(), m_eLRSpaceUnit ) );
    aMargin.SetTxtFirstLineOfst( (const short)GetCoreValue( *maFLineIndent.get(), m_eLRSpaceUnit ) );

    GetBindings()->GetDispatcher()->Execute(
        SID_ATTR_PARA_LRSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
    return 0;
}

IMPL_LINK(ParaPropertyPanel, ClickIndent_IncDec_Hdl_Impl, ToolBox *, pControl)
{
    switch (pControl->GetCurItemId())
    {
    case INDENT_INCREMENT:
        {
            switch (maContext.GetCombinedContext_DI())
            {
            case CombinedEnumContext(Application_WriterVariants, Context_Default):
            case CombinedEnumContext(Application_WriterVariants, Context_Text):
            case CombinedEnumContext(Application_WriterVariants, Context_Table):
                {
                    SfxBoolItem aMargin( SID_INC_INDENT, sal_True );
                    GetBindings()->GetDispatcher()->Execute(
                        SID_INC_INDENT, SFX_CALLMODE_RECORD, &aMargin, 0L);
                }
                break;
            default:
                {
                    SvxLRSpaceItem aMargin( SID_ATTR_PARA_LRSPACE );

                    maTxtLeft += INDENT_STEP;
                    sal_Int64 nVal = OutputDevice::LogicToLogic( maTxtLeft, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
                    nVal = OutputDevice::LogicToLogic( (long)nVal, MAP_100TH_MM, (MapUnit)m_eLRSpaceUnit );
                    aMargin.SetTxtLeft( (const long)nVal );
                    aMargin.SetRight( (const long)GetCoreValue( *maRightIndent.get(), m_eLRSpaceUnit ) );
                    aMargin.SetTxtFirstLineOfst( (const short)GetCoreValue( *maFLineIndent.get(), m_eLRSpaceUnit ) );

                    GetBindings()->GetDispatcher()->Execute(
                        SID_ATTR_PARA_LRSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
                }
            }
        }
        break;
    case INDENT_DECREMENT:
        {
            switch (maContext.GetCombinedContext_DI())
            {
            case CombinedEnumContext(Application_WriterVariants, Context_Default):
            case CombinedEnumContext(Application_WriterVariants, Context_Text):
            case CombinedEnumContext(Application_WriterVariants, Context_Table):
                {
                    SfxBoolItem aMargin( SID_DEC_INDENT, sal_True );
                    GetBindings()->GetDispatcher()->Execute(
                        SID_DEC_INDENT, SFX_CALLMODE_RECORD, &aMargin, 0L);
                }
                break;
            default:
                {
                    if((maTxtLeft - INDENT_STEP) < 0)
                        maTxtLeft = DEFAULT_VALUE;
                    else
                        maTxtLeft -= INDENT_STEP;

                    SvxLRSpaceItem aMargin( SID_ATTR_PARA_LRSPACE );

                    sal_Int64 nVal = OutputDevice::LogicToLogic( maTxtLeft, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
                    nVal = OutputDevice::LogicToLogic( (long)nVal, MAP_100TH_MM, (MapUnit)m_eLRSpaceUnit );

                    aMargin.SetTxtLeft( (const long)nVal );
                    aMargin.SetRight( (const long)GetCoreValue( *maRightIndent.get(), m_eLRSpaceUnit ) );
                    aMargin.SetTxtFirstLineOfst( (const short)GetCoreValue( *maFLineIndent.get(), m_eLRSpaceUnit ) );

                    GetBindings()->GetDispatcher()->Execute(
                        SID_ATTR_PARA_LRSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
                }
            }
        }
        break;
    case ID_HANGING_INDENT:
        {
            SvxLRSpaceItem aMargin( SID_ATTR_PARA_LRSPACE );
            aMargin.SetTxtLeft( (const long)GetCoreValue( *maLeftIndent.get(), m_eLRSpaceUnit ) + (const short)GetCoreValue( *maFLineIndent.get(), m_eLRSpaceUnit ) );
            aMargin.SetRight( (const long)GetCoreValue( *maRightIndent.get(), m_eLRSpaceUnit ) );
            aMargin.SetTxtFirstLineOfst( ((const short)GetCoreValue( *maFLineIndent.get(), m_eLRSpaceUnit ))*(-1) );

            GetBindings()->GetDispatcher()->Execute(
                SID_ATTR_PARA_LRSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
        }
        break;
    }
    return( 0L );
}

IMPL_LINK(ParaPropertyPanel, ClickProDemote_Hdl_Impl, ToolBox *, pControl)
{
    switch (pControl->GetCurItemId())
    {
        case BT_TBX_INDENT_PROMOTE:
        {
            GetBindings()->GetDispatcher()->Execute( SID_OUTLINE_RIGHT, SFX_CALLMODE_RECORD );
        }
        break;
        case BT_TBX_INDENT_DEMOTE:
        {
            GetBindings()->GetDispatcher()->Execute( SID_OUTLINE_LEFT, SFX_CALLMODE_RECORD );
        }
        break;
        case SD_HANGING_INDENT:
        {
            SvxLRSpaceItem aMargin( SID_ATTR_PARA_LRSPACE );
            aMargin.SetTxtLeft( (const long)GetCoreValue( *maLeftIndent.get(), m_eLRSpaceUnit ) + (const short)GetCoreValue( *maFLineIndent.get(), m_eLRSpaceUnit ) );
            aMargin.SetRight( (const long)GetCoreValue( *maRightIndent.get(), m_eLRSpaceUnit ) );
            aMargin.SetTxtFirstLineOfst( ((const short)GetCoreValue( *maFLineIndent.get(), m_eLRSpaceUnit ))*(-1) );

            GetBindings()->GetDispatcher()->Execute( SID_ATTR_PARA_LRSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
        }
        break;
    }
    return( 0L );
}
//==================================for Paragraph Line Spacing=====================

IMPL_LINK( ParaPropertyPanel, ClickLineSPDropDownHdl_Impl, ToolBox*, pBox )
{

    const sal_uInt16 nId = pBox->GetCurItemId();
    OSL_ASSERT(nId == TOOLBOX_ITEM1);
    if(nId == TOOLBOX_ITEM1)
    {
        pBox->SetItemDown( nId, true );
        maLineSpacePopup.Rearrange(meLnSpState,m_eMetricUnit,mpLnSPItem,maContext);
        maLineSpacePopup.Show(*pBox);

    }
    return (0L);
}

//==================================for Paragraph Spacing=====================
IMPL_LINK_NOARG( ParaPropertyPanel, ULSpaceHdl_Impl)
{
    SvxULSpaceItem aMargin( SID_ATTR_PARA_ULSPACE );
    aMargin.SetUpper( (sal_uInt16)GetCoreValue( *maTopDist.get(), m_eULSpaceUnit ) );
    aMargin.SetLower( (sal_uInt16)GetCoreValue( *maBottomDist.get(), m_eULSpaceUnit ) );

    GetBindings()->GetDispatcher()->Execute(
        SID_ATTR_PARA_ULSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
    return 0L;
}

IMPL_LINK(ParaPropertyPanel, ClickUL_IncDec_Hdl_Impl, ToolBox *, pControl)
{
    switch (pControl->GetCurItemId())
        {
        case UL_INCREMENT:
             {
                 SvxULSpaceItem aMargin( SID_ATTR_PARA_ULSPACE );

                 maUpper += UL_STEP;
                 sal_Int64 nVal = OutputDevice::LogicToLogic( maUpper, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
                 nVal = OutputDevice::LogicToLogic( (long)nVal, MAP_100TH_MM, (MapUnit)m_eLRSpaceUnit );
                 aMargin.SetUpper( (const sal_uInt16)nVal );

                 maLower += UL_STEP;
                 nVal = OutputDevice::LogicToLogic( maLower, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
                 nVal = OutputDevice::LogicToLogic( (long)nVal, MAP_100TH_MM, (MapUnit)m_eLRSpaceUnit );
                 aMargin.SetLower( (const sal_uInt16)nVal );

                 GetBindings()->GetDispatcher()->Execute(
                     SID_ATTR_PARA_ULSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
             }
             break;
        case UL_DECREMENT:
            {
                 SvxULSpaceItem aMargin( SID_ATTR_PARA_ULSPACE );

                 if( maUpper >= UL_STEP )
                 {
                    maUpper -= UL_STEP;
                    sal_Int64   nVal = OutputDevice::LogicToLogic( maUpper, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
                    nVal = OutputDevice::LogicToLogic( (long)nVal, MAP_100TH_MM, (MapUnit)m_eLRSpaceUnit );
                    aMargin.SetUpper( (const sal_uInt16)nVal );
                 }
                 else
                    aMargin.SetUpper( DEFAULT_VALUE );
                 if( maLower >= UL_STEP )
                 {
                    maLower -= UL_STEP;
                    sal_Int64   nVal = OutputDevice::LogicToLogic( maLower, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
                    nVal = OutputDevice::LogicToLogic( (long)nVal, MAP_100TH_MM, (MapUnit)m_eLRSpaceUnit );
                    aMargin.SetLower( (const sal_uInt16)nVal );
                 }
                 else
                    aMargin.SetLower( DEFAULT_VALUE );

                 GetBindings()->GetDispatcher()->Execute(
                     SID_ATTR_PARA_ULSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
            }
            break;
        }
    return( 0L );
}

//==================================for Paragraph State change=====================
void ParaPropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;

    switch (nSID)
    {
    case SID_ATTR_METRIC:
        {
            m_eMetricUnit = GetCurrentUnit(eState,pState);
            if( m_eMetricUnit!=m_last_eMetricUnit )
            {
                SetFieldUnit( *maLeftIndent.get(), m_eMetricUnit );
                SetFieldUnit( *maRightIndent.get(), m_eMetricUnit );
                SetFieldUnit( *maFLineIndent.get(), m_eMetricUnit );
                SetFieldUnit( *maTopDist.get(), m_eMetricUnit );
                SetFieldUnit( *maBottomDist.get(), m_eMetricUnit );
            }
            m_last_eMetricUnit = m_eMetricUnit;
        }
        break;

    case SID_ATTR_PARA_LRSPACE:
        StateChangedIndentImpl( nSID, eState, pState );
        break;

    case SID_ATTR_PARA_LINESPACE:
        StateChangedLnSPImpl( nSID, eState, pState );
        break;

    case SID_ATTR_PARA_ULSPACE:
        StateChangedULImpl( nSID, eState, pState );
        break;

    case SID_ATTR_PARA_ADJUST_LEFT:
    case SID_ATTR_PARA_ADJUST_CENTER:
    case SID_ATTR_PARA_ADJUST_RIGHT:
    case SID_ATTR_PARA_ADJUST_BLOCK:
        StateChangedAlignmentImpl( nSID, eState, pState );
        break;

    case SID_OUTLINE_LEFT:
    case SID_OUTLINE_RIGHT:
        StateChangeOutLineImpl( nSID, eState, pState );
        break;

    case SID_INC_INDENT:
    case SID_DEC_INDENT:
        StateChangeIncDecImpl( nSID, eState, pState );
        break;

    case FN_NUM_NUMBERING_ON:
    case FN_NUM_BULLET_ON:
        StateChangeBulletNumImpl( nSID, eState, pState );
        break;

    case FN_BUL_NUM_RULE_INDEX:
    case FN_NUM_NUM_RULE_INDEX:
        StateChangeBulletNumRuleImpl( nSID, eState, pState );
        break;

    case SID_TABLE_VERT_NONE:
    case SID_TABLE_VERT_CENTER:
    case SID_TABLE_VERT_BOTTOM:
        VertStateChanged( nSID, eState, pState);
        break;

    case SID_BACKGROUND_COLOR:
        ParaBKGStateChanged(nSID, eState, pState);
        break;
    }
}




void ParaPropertyPanel::StateChangedAlignmentImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if( eState >= SFX_ITEM_AVAILABLE )
    {
        const SfxBoolItem* pItem = (const SfxBoolItem*)pState;
        bool IsChecked = (bool)pItem->GetValue();
        switch (nSID)
        {
        case SID_ATTR_PARA_ADJUST_LEFT:
            {
                if(IsChecked)
                {
                    maAlignToolBox->SetItemState(BT_SUBSTLEFT,STATE_CHECK);
                    maAlignToolBox->SetItemState(BT_SUBSTCENTER,STATE_NOCHECK);
                    maAlignToolBox->SetItemState(BT_SUBSTRIGHT,STATE_NOCHECK);
                    maAlignToolBox->SetItemState(BT_SUBSTJUSTIFY,STATE_NOCHECK);
                }
                else
                    maAlignToolBox->SetItemState(BT_SUBSTLEFT,STATE_NOCHECK);
            }
            break;
        case SID_ATTR_PARA_ADJUST_CENTER:
            {
                if(IsChecked)
                {
                    maAlignToolBox->SetItemState(BT_SUBSTCENTER,STATE_CHECK);
                    maAlignToolBox->SetItemState(BT_SUBSTLEFT,STATE_NOCHECK);
                    maAlignToolBox->SetItemState(BT_SUBSTRIGHT,STATE_NOCHECK);
                    maAlignToolBox->SetItemState(BT_SUBSTJUSTIFY,STATE_NOCHECK);
                }
                else
                    maAlignToolBox->SetItemState(BT_SUBSTCENTER,STATE_NOCHECK);
            }
            break;
        case SID_ATTR_PARA_ADJUST_RIGHT:
            {
                if(IsChecked)
                {
                    maAlignToolBox->SetItemState(BT_SUBSTRIGHT,STATE_CHECK);
                    maAlignToolBox->SetItemState(BT_SUBSTLEFT,STATE_NOCHECK);
                    maAlignToolBox->SetItemState(BT_SUBSTCENTER,STATE_NOCHECK);
                    maAlignToolBox->SetItemState(BT_SUBSTJUSTIFY,STATE_NOCHECK);
                }
                else
                    maAlignToolBox->SetItemState(BT_SUBSTRIGHT,STATE_NOCHECK);
            }
            break;
        case SID_ATTR_PARA_ADJUST_BLOCK:
            {
                if(IsChecked)
                {
                    maAlignToolBox->SetItemState(BT_SUBSTJUSTIFY,STATE_CHECK);
                    maAlignToolBox->SetItemState(BT_SUBSTLEFT,STATE_NOCHECK);
                    maAlignToolBox->SetItemState(BT_SUBSTCENTER,STATE_NOCHECK);
                    maAlignToolBox->SetItemState(BT_SUBSTRIGHT,STATE_NOCHECK);
                }
                else
                    maAlignToolBox->SetItemState(BT_SUBSTJUSTIFY,STATE_NOCHECK);
            }
            break;
        }
    }
    else if( eState == SFX_ITEM_DISABLED )
    {
    }
    else if ( eState == SFX_ITEM_DONTCARE )
    {
        switch (nSID)
        {
        case SID_ATTR_PARA_ADJUST_LEFT:
            maAlignToolBox->SetItemState(BT_SUBSTLEFT,STATE_NOCHECK);
            break;
        case SID_ATTR_PARA_ADJUST_CENTER:
            maAlignToolBox->SetItemState(BT_SUBSTCENTER,STATE_NOCHECK);
            break;
        case SID_ATTR_PARA_ADJUST_RIGHT:
            maAlignToolBox->SetItemState(BT_SUBSTRIGHT,STATE_NOCHECK);
            break;
        case SID_ATTR_PARA_ADJUST_BLOCK:
            maAlignToolBox->SetItemState(BT_SUBSTJUSTIFY,STATE_NOCHECK);
            break;
        }
    }
}

void ParaPropertyPanel::StateChangedIndentImpl( sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    switch (maContext.GetCombinedContext_DI())
    {

    case CombinedEnumContext(Application_WriterVariants, Context_DrawText):
    case CombinedEnumContext(Application_WriterVariants, Context_Annotation):
    case CombinedEnumContext(Application_Calc, Context_DrawText):
        {
            maLeftIndent->SetMin( DEFAULT_VALUE );
            maRightIndent->SetMin( DEFAULT_VALUE );
            maFLineIndent->SetMin( DEFAULT_VALUE );
            maTbxIndent_IncDec->Show();
            maTbxProDemote->Hide();
        }
        break;
    case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
    case CombinedEnumContext(Application_DrawImpress, Context_Draw):
    case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
    case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
    case CombinedEnumContext(Application_DrawImpress, Context_Table):
        {
            maLeftIndent->SetMin( DEFAULT_VALUE );
            maRightIndent->SetMin( DEFAULT_VALUE );
            maFLineIndent->SetMin( DEFAULT_VALUE );
            maTbxIndent_IncDec->Hide();
            maTbxProDemote->Show();
        }
        break;
    case CombinedEnumContext(Application_WriterVariants, Context_Default):
    case CombinedEnumContext(Application_WriterVariants, Context_Text):
    case CombinedEnumContext(Application_WriterVariants, Context_Table):
        {
            maLeftIndent->SetMin( NEGA_MAXVALUE, FUNIT_100TH_MM );
            maRightIndent->SetMin( NEGA_MAXVALUE, FUNIT_100TH_MM );
            maFLineIndent->SetMin( NEGA_MAXVALUE, FUNIT_100TH_MM );
            maTbxIndent_IncDec->Show();
            maTbxProDemote->Hide();
        }
        break;
    }

    if( pState && eState >= SFX_ITEM_AVAILABLE )
    {
        SvxLRSpaceItem* pSpace = ( SvxLRSpaceItem*)pState;
        maTxtLeft = pSpace->GetTxtLeft();
        maTxtLeft = OutputDevice::LogicToLogic( maTxtLeft, (MapUnit)m_eLRSpaceUnit, MAP_100TH_MM );
        maTxtLeft = OutputDevice::LogicToLogic( maTxtLeft, MAP_100TH_MM, (MapUnit)(SFX_MAPUNIT_TWIP) );

        long aTxtRight = pSpace->GetRight();
        aTxtRight = OutputDevice::LogicToLogic( aTxtRight, (MapUnit)m_eLRSpaceUnit, MAP_100TH_MM );
        aTxtRight = OutputDevice::LogicToLogic( aTxtRight, MAP_100TH_MM, (MapUnit)(SFX_MAPUNIT_TWIP) );

        long aTxtFirstLineOfst = pSpace->GetTxtFirstLineOfst();
        aTxtFirstLineOfst = OutputDevice::LogicToLogic( aTxtFirstLineOfst, (MapUnit)m_eLRSpaceUnit, MAP_100TH_MM );
        aTxtFirstLineOfst = OutputDevice::LogicToLogic( aTxtFirstLineOfst, MAP_100TH_MM, (MapUnit)(SFX_MAPUNIT_TWIP) );

        long nVal = OutputDevice::LogicToLogic( maTxtLeft, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
        nVal = (long)maLeftIndent->Normalize( (long)nVal );
        maLeftIndent->SetValue( nVal, FUNIT_100TH_MM );

        if ( maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Text)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Default)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Table))
        {
            maFLineIndent->SetMin( nVal*(-1), FUNIT_100TH_MM );
        }

        long nrVal = OutputDevice::LogicToLogic( aTxtRight, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
        nrVal = (long)maRightIndent->Normalize( (long)nrVal );
        maRightIndent->SetValue( nrVal, FUNIT_100TH_MM );

        long nfVal = OutputDevice::LogicToLogic( aTxtFirstLineOfst, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
        nfVal = (long)maFLineIndent->Normalize( (long)nfVal );
        maFLineIndent->SetValue( nfVal, FUNIT_100TH_MM );

        switch (maContext.GetCombinedContext_DI())
        {
        case CombinedEnumContext(Application_WriterVariants, Context_DrawText):
        case CombinedEnumContext(Application_WriterVariants, Context_Text):
        case CombinedEnumContext(Application_WriterVariants, Context_Default):
        case CombinedEnumContext(Application_WriterVariants, Context_Table):
        case CombinedEnumContext(Application_WriterVariants, Context_Annotation):
            {
                maLeftIndent->SetMax( MAX_SW - nrVal, FUNIT_100TH_MM );
                maRightIndent->SetMax( MAX_SW - nVal, FUNIT_100TH_MM );
                maFLineIndent->SetMax( MAX_SW - nVal - nrVal, FUNIT_100TH_MM );
            }
            break;
        case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
        case CombinedEnumContext(Application_DrawImpress, Context_Draw):
        case CombinedEnumContext(Application_DrawImpress, Context_Table):
        case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
            {
                maLeftIndent->SetMax( MAX_SC_SD - nrVal, FUNIT_100TH_MM );
                maRightIndent->SetMax( MAX_SC_SD - nVal, FUNIT_100TH_MM );
                maFLineIndent->SetMax( MAX_SC_SD - nVal - nrVal, FUNIT_100TH_MM );
            }
        }

        maTbxIndent_IncDec->Enable();
        maTbxIndent_IncDec->EnableItem(ID_HANGING_INDENT, sal_True);
        if ( maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Text)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Default)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Table) )
        {
            maTbxIndent_IncDec->EnableItem(BT_TBX_INDENT_INC, sal_True);
            maTbxIndent_IncDec->EnableItem(BT_TBX_INDENT_DEC, sal_True);
        }

        maTbxProDemote->EnableItem(SD_HANGING_INDENT, sal_True);
    }
    else if( eState == SFX_ITEM_DISABLED )
    {
        maLeftIndent-> Disable();
        maRightIndent->Disable();
        maFLineIndent->Disable();
        maTbxIndent_IncDec->Disable();
        if( maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Text)  &&
            maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Default) &&
            maContext.GetCombinedContext_DI() !=  CombinedEnumContext(Application_WriterVariants, Context_Table) )
            maTbxIndent_IncDec->Disable();
        else
            maTbxIndent_IncDec->EnableItem(ID_HANGING_INDENT, sal_False);

        //      maTbxProDemote->Disable();
        maTbxProDemote->EnableItem(SD_HANGING_INDENT, sal_False);
    }
    else
    {
        maLeftIndent->SetEmptyFieldValue();
        maRightIndent->SetEmptyFieldValue();
        maFLineIndent->SetEmptyFieldValue();
        if( maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Text)  &&
            maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Default) &&
            maContext.GetCombinedContext_DI() !=  CombinedEnumContext(Application_WriterVariants, Context_Table) )
            maTbxIndent_IncDec->Disable();
        else
            maTbxIndent_IncDec->EnableItem(ID_HANGING_INDENT, sal_False);
        maTbxProDemote->EnableItem(SD_HANGING_INDENT, sal_False);
    }
}

void ParaPropertyPanel::StateChangedLnSPImpl( sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    meLnSpState = eState;

    if( pState && eState >= SFX_ITEM_AVAILABLE )
    {
        if(mpLnSPItem)
            delete mpLnSPItem;
        mpLnSPItem = ( SvxLineSpacingItem *)pState->Clone();
    }
}

void ParaPropertyPanel::StateChangedULImpl( sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    maTopDist->SetMax( maTopDist->Normalize( MAX_DURCH ), MapToFieldUnit(m_eULSpaceUnit) );
    maBottomDist->SetMax( maBottomDist->Normalize( MAX_DURCH ), MapToFieldUnit(m_eULSpaceUnit) );

    if( pState && eState >= SFX_ITEM_AVAILABLE )
    {
        SvxULSpaceItem* pOldItem = (SvxULSpaceItem*)pState;

        maUpper = pOldItem->GetUpper();
        maUpper = OutputDevice::LogicToLogic( maUpper, (MapUnit)m_eULSpaceUnit, MAP_100TH_MM );
        maUpper = OutputDevice::LogicToLogic( maUpper, MAP_100TH_MM, (MapUnit)(SFX_MAPUNIT_TWIP) );

        maLower = pOldItem->GetLower();
        maLower = OutputDevice::LogicToLogic( maLower, (MapUnit)m_eULSpaceUnit, MAP_100TH_MM );
        maLower = OutputDevice::LogicToLogic( maLower, MAP_100TH_MM, (MapUnit)(SFX_MAPUNIT_TWIP) );

        sal_Int64 nVal = OutputDevice::LogicToLogic( maUpper, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
        nVal = maTopDist->Normalize( nVal );
        maTopDist->SetValue( nVal, FUNIT_100TH_MM );

        nVal = OutputDevice::LogicToLogic( maLower, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
        nVal = maBottomDist->Normalize( nVal );
        maBottomDist->SetValue( nVal, FUNIT_100TH_MM );
        maTbxUL_IncDec->Enable();
    }
    else if(eState == SFX_ITEM_DISABLED )
    {
        maTopDist->Disable();
        maBottomDist->Disable();
        maTbxUL_IncDec->Disable();
    }
    else
    {
        maTopDist->SetEmptyFieldValue();
        maBottomDist->SetEmptyFieldValue();
        maTbxUL_IncDec->Disable();
    }
}

void ParaPropertyPanel::StateChangeOutLineImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if (nSID==SID_OUTLINE_LEFT)
    {
        if( pState && eState == SFX_ITEM_UNKNOWN )
            mbOutLineLeft = 1;
        else
            mbOutLineLeft = 0;
    }
    if (nSID==SID_OUTLINE_RIGHT)
    {
        if( pState && eState == SFX_ITEM_UNKNOWN )
            mbOutLineRight = 1;
        else
            mbOutLineRight = 0;
    }
    if(mbOutLineLeft)
        maTbxProDemote->EnableItem(BT_TBX_INDENT_DEMOTE, sal_True);
    else
        maTbxProDemote->EnableItem(BT_TBX_INDENT_DEMOTE, sal_False);

    if(mbOutLineRight)
        maTbxProDemote->EnableItem(BT_TBX_INDENT_PROMOTE, sal_True);
    else
        maTbxProDemote->EnableItem(BT_TBX_INDENT_PROMOTE, sal_False);

}

void ParaPropertyPanel::StateChangeIncDecImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( ( maContext.GetCombinedContext_DI() == CombinedEnumContext(Application_WriterVariants, Context_Text)
           || maContext.GetCombinedContext_DI() == CombinedEnumContext(Application_WriterVariants, Context_Default)
           || maContext.GetCombinedContext_DI() ==  CombinedEnumContext(Application_WriterVariants, Context_Table) )
         && ( nSID == SID_INC_INDENT || nSID == SID_DEC_INDENT ) )
    {
        // Writer's text shell is the only one which provides reasonable states for Slots SID_INC_INDENT and SID_DEC_INDENT
        // - namely SFX_ITEM_UNKNOWN and SFX_ITEM_DISABLED
        maTbxIndent_IncDec->EnableItem(
            nSID == SID_INC_INDENT ? BT_TBX_INDENT_INC : BT_TBX_INDENT_DEC,
            ( pState && eState == SFX_ITEM_UNKNOWN ) ? sal_True : sal_False );
    }
}


// Add toggle state for numbering and bullet icons
void ParaPropertyPanel::StateChangeBulletNumImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( (eState >= SFX_ITEM_DEFAULT) && (pState->ISA(SfxBoolItem)) )
    {
        const SfxBoolItem* pItem= (const SfxBoolItem*)pState;
        const sal_Bool aBool = (sal_Bool)pItem->GetValue();
        if (nSID==FN_NUM_NUMBERING_ON)
        {
            maTBxNumBullet->SetItemState(
                IID_NUMBER,
                aBool ? STATE_CHECK : STATE_NOCHECK );
        }
        else if (nSID==FN_NUM_BULLET_ON)
        {
            maTBxNumBullet->SetItemState(
                IID_BULLET,
                aBool ? STATE_CHECK : STATE_NOCHECK );
        }
    }
}


void ParaPropertyPanel::StateChangeBulletNumRuleImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( eState >= SFX_ITEM_DEFAULT && pState->ISA(SfxUInt16Item) )
    {
        sal_uInt16 nValue = (sal_uInt16)0xFFFF;
        {
            const SfxUInt16Item* pIt = (const SfxUInt16Item*)pState;
            if ( pIt )
                nValue = pIt->GetValue();
        }

        if ( nSID == FN_BUL_NUM_RULE_INDEX )
        {
            mnBulletTypeIndex = nValue;
        }
        else if ( nSID == FN_NUM_NUM_RULE_INDEX )
        {
            mnNumTypeIndex = nValue;
        }
    }
}


FieldUnit ParaPropertyPanel::GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState )
{
    FieldUnit eUnit = FUNIT_NONE;

    if ( pState && eState >= SFX_ITEM_DEFAULT )
        eUnit = (FieldUnit)( (const SfxUInt16Item*)pState )->GetValue();
    else
    {
        SfxViewFrame* pFrame = SfxViewFrame::Current();
        SfxObjectShell* pSh = NULL;
        if ( pFrame )
            pSh = pFrame->GetObjectShell();
        if ( pSh )  //the object shell is not always available during reload
        {
            SfxModule* pModule = pSh->GetModule();
            if ( pModule )
            {
                const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );
                if ( pItem )
                    eUnit = (FieldUnit)( (SfxUInt16Item*)pItem )->GetValue();
            }
            else
            {
                DBG_ERRORFILE( "GetModuleFieldUnit(): no module found" );
            }
        }
    }

    return eUnit;
}


PopupControl* ParaPropertyPanel::CreateLineSpacingControl (PopupContainer* pParent)
{
    return new ParaLineSpacingControl(pParent, *this);
}

PopupControl* ParaPropertyPanel::CreateBulletsPopupControl (PopupContainer* pParent)
{
    return new ParaBulletsControl(pParent, *this);
}

PopupControl* ParaPropertyPanel::CreateNumberingPopupControl (PopupContainer* pParent)
{
    return new ParaNumberingControl(pParent, *this);
}

namespace
{
    Color GetNoBackgroundColor(void)
    {
        return COL_TRANSPARENT;
    }
} // end of anonymous namespace

PopupControl* ParaPropertyPanel::CreateBGColorPopupControl (PopupContainer* pParent)
{
    const ResId aResId(SVX_RES(STR_NOFILL));

    return new ColorControl(
        pParent,
        mpBindings,
        SVX_RES(RID_POPUPPANEL_PARAPAGE_BACK_COLOR),
        SVX_RES(VS_FONT_COLOR),
        ::boost::bind(GetNoBackgroundColor),
        ::boost::bind(&ParaPropertyPanel::SetBGColor, this, _1,_2),
        pParent,
        &aResId);
}


ParaPropertyPanel::ParaPropertyPanel(Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const cssu::Reference<css::ui::XSidebar>& rxSidebar)
    : Control(pParent, SVX_RES(RID_SIDEBAR_PARA_PANEL)),
      maAlignToolBoxBackground(ControlFactory::CreateToolBoxBackground(this)),
      maAlignToolBox (ControlFactory::CreateToolBox(maAlignToolBoxBackground.get(),SVX_RES(TBX_HORIZONTALALIGNMENT))),
      maTBxVertAlignBackground(ControlFactory::CreateToolBoxBackground(this)),
      maTBxVertAlign (ControlFactory::CreateToolBox(maTBxVertAlignBackground.get(),SVX_RES(TBX_VERT_ALIGN))),
      maTBxNumBulletBackground(ControlFactory::CreateToolBoxBackground(this)),
      maTBxNumBullet (ControlFactory::CreateToolBox(maTBxNumBulletBackground.get(),SVX_RES(TBX_NUM_BULLET))),
      maTBxBackColorBackground(ControlFactory::CreateToolBoxBackground(this)),
      maTBxBackColor (ControlFactory::CreateToolBox(maTBxBackColorBackground.get(),SVX_RES(TBX_BACK_COLOR))),
      maFTUL (new FixedText(this, SVX_RES(FT_SPACING))),
      maTbxUL_IncDecBackground(ControlFactory::CreateToolBoxBackground(this)),
      maTbxUL_IncDec (ControlFactory::CreateToolBox(maTbxUL_IncDecBackground.get(),SVX_RES(TBX_UL_INC_DEC))),
      maFTIndent (new FixedText(this, SVX_RES(FT_INDENT))),
      maTbxIndent_IncDecBackground(ControlFactory::CreateToolBoxBackground(this)),
      maTbxIndent_IncDec (ControlFactory::CreateToolBox(maTbxIndent_IncDecBackground.get(),SVX_RES(TBX_INDENT_INC_DEC))),

      maTopDist (new SvxRelativeField(this, SVX_RES(MF_ABOVE_PARASPACING))),
      maLeftIndent (new SvxRelativeField(this, SVX_RES(MF_BEFORE_INDENT))),

      maBottomDist (new SvxRelativeField(this, SVX_RES(MF_BELOW_PARASPACING))),
      maRightIndent (new SvxRelativeField(this, SVX_RES(MF_AFTER_INDENT))),

      maLineSPTbxBackground(ControlFactory::CreateToolBoxBackground(this)),
      maLineSPTbx (ControlFactory::CreateToolBox(maLineSPTbxBackground.get(),SVX_RES(TBX_LINESP))),
      maFLineIndent (new SvxRelativeField(this, SVX_RES(MF_FL_INDENT))),

      maTbxProDemoteBackground(ControlFactory::CreateToolBoxBackground(this)),
      maTbxProDemote (ControlFactory::CreateToolBox(maTbxProDemoteBackground.get(),SVX_RES(TBX_INDENT_PRO_DEMOTE))),
      mpColorUpdater (),
      maFISpace1 ( this, SVX_RES( FI_SPACE1)),
      maFISpace2 ( this, SVX_RES( FI_SPACE2)),
      maFIndent1 ( this, SVX_RES( FI_INDENT1)),
      maFIndent2 ( this, SVX_RES( FI_INDENT2)),
      maFIndent3 ( this, SVX_RES( FI_INDENT3)),
      maSpace1 (SVX_RES(IMG_SPACE1)),
      maSpace2 (SVX_RES(IMG_SPACE2)),
      maSpace3 (SVX_RES(IMG_SPACE3)),
      maIndent1 (SVX_RES(IMG_INDENT1)),
      maIndent2 (SVX_RES(IMG_INDENT2)),
      maIndent3 (SVX_RES(IMG_INDENT3)),
      maIndHang (SVX_RES(IMG_INDENT_HANG)),
      maParInc (SVX_RES(IMG_PARA_INC)),
      maParDec (SVX_RES(IMG_PARA_DEC)),
      maNumBImageList (SVX_RES(IL_NUM_BULLET)),
      maNumBImageListH (SVX_RES(ILH_NUM_BULLET)),
      maNumBImageListRTL (SVX_RES(IL_NUM_BULLET_RTL)),
      maImgBackColorHigh (SVX_RES(IMG_BACK_COLOR_H)),
      maImgBackColor (SVX_RES(IMG_BACK_COLOR)),
      maTxtLeft (0),
      mpLnSPItem (NULL),
      meLnSpState (SFX_ITEM_DONTCARE),
      mbOutLineLeft (false),
      mbOutLineRight (false),
      maUpper (0),
      maLower (0),
      mnBulletTypeIndex ((sal_uInt16)0xFFFF),
      mnNumTypeIndex ((sal_uInt16)0xFFFF),
      maColor (COL_AUTO),
      mbColorAvailable (true),
      m_eMetricUnit(FUNIT_NONE),
      m_last_eMetricUnit(FUNIT_NONE),
      m_eLRSpaceUnit(),
      m_eULSpaceUnit(),
      maLeftAlignControl(SID_ATTR_PARA_ADJUST_LEFT, *pBindings, *this, A2S("LeftPara"), rxFrame),
      maCenterAlignControl(SID_ATTR_PARA_ADJUST_CENTER, *pBindings, *this, A2S("CenterPara"), rxFrame),
      maRightAlignControl(SID_ATTR_PARA_ADJUST_RIGHT, *pBindings, *this, A2S("RightPara"), rxFrame),
      maJustifyAlignControl(SID_ATTR_PARA_ADJUST_BLOCK, *pBindings, *this, A2S("JustifyPara"), rxFrame),
      maLRSpaceControl (SID_ATTR_PARA_LRSPACE,*pBindings,*this),
      maLNSpaceControl (SID_ATTR_PARA_LINESPACE, *pBindings,*this),
      maULSpaceControl (SID_ATTR_PARA_ULSPACE, *pBindings,*this),
      maOutLineLeftControl(SID_OUTLINE_LEFT, *pBindings, *this, A2S("OutlineRight"), rxFrame),
      maOutLineRightControl(SID_OUTLINE_RIGHT, *pBindings, *this, A2S("OutlineLeft"), rxFrame),
      maDecIndentControl(SID_DEC_INDENT, *pBindings,*this, A2S("DecrementIndent"), rxFrame),
      maIncIndentControl(SID_INC_INDENT, *pBindings,*this, A2S("IncrementIndent"), rxFrame),
      maVertTop (SID_TABLE_VERT_NONE, *pBindings, *this, A2S("CellVertTop"), rxFrame),
      maVertCenter (SID_TABLE_VERT_CENTER, *pBindings,*this, A2S("CellVertCenter"), rxFrame),
      maVertBottom (SID_TABLE_VERT_BOTTOM,*pBindings,*this, A2S("CellVertBottom"), rxFrame),
      maBulletOnOff(FN_NUM_BULLET_ON, *pBindings, *this, A2S("DefaultBullet"), rxFrame),
      maNumberOnOff(FN_NUM_NUMBERING_ON, *pBindings, *this, A2S("DefaultNumbering"), rxFrame),
      maBackColorControl (SID_BACKGROUND_COLOR, *pBindings,*this),
      m_aMetricCtl (SID_ATTR_METRIC, *pBindings,*this),
      maBulletNumRuleIndex (FN_BUL_NUM_RULE_INDEX, *pBindings,*this),
      maNumNumRuleIndex (FN_NUM_NUM_RULE_INDEX, *pBindings,*this),
      mxFrame(rxFrame),
      maContext(),
      mpBindings(pBindings),
      maLineSpacePopup(this, ::boost::bind(&ParaPropertyPanel::CreateLineSpacingControl, this, _1)),
      maBulletsPopup(this, ::boost::bind(&ParaPropertyPanel::CreateBulletsPopupControl, this, _1)),
      maNumberingPopup(this, ::boost::bind(&ParaPropertyPanel::CreateNumberingPopupControl, this, _1)),
      maBGColorPopup(this, ::boost::bind(&ParaPropertyPanel::CreateBGColorPopupControl, this, _1)),
      mxSidebar(rxSidebar)
{
    initial();
    FreeResource();
}

} } // end of namespace svx::sidebar
