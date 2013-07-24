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
#include <sfx2/sidebar/Tools.hxx>
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

const char UNO_LEFTPARA[]         = ".uno:LeftPara";
const char UNO_RIGHTPARA[]        = ".uno:RightPara";
const char UNO_CENTERPARA[]       = ".uno:CenterPara";
const char UNO_JUSTIFYPARA[]      = ".uno:JustifyPara";

const char UNO_DEFAULTBULLET[]    = ".uno:DefaultBullet";
const char UNO_DEFAULTNUMBERING[] = ".uno:DefaultNumbering";

const char UNO_PARABACKCOLOR[]    = ".uno:ParaBackColor";

const char UNO_INCREMENTINDENT[]  = ".uno:IncrementIndent";
const char UNO_DECREMENTINDENT[]  = ".uno:DecrementIndent";
const char UNO_HANGINGINDENT[]    = ".uno:HangingIndent";

const char UNO_PROMOTE[]          = ".uno:Promote";
const char UNO_DEMOTE[]           = ".uno:Demote";
const char UNO_HANGINGINDENT2[]   = ".uno:HangingIndent2";

const char UNO_LINESPACING[]      = ".uno:LineSpacing";

const char UNO_CELLVERTTOP[]      = ".uno:CellVertTop";
const char UNO_CELLVERTCENTER[]   = ".uno:CellVertCenter";
const char UNO_CELLVERTBOTTOM[]   = ".uno:CellVertBottom";

const char UNO_PARASPACEINC[]     = ".uno:ParaspaceIncrease";
const char UNO_PARASPACEDEC[]     = ".uno:ParaspaceDecrease";

namespace svx {namespace sidebar {
#define DEFAULT_VALUE          0

#define MAX_DURCH             5670

#define INDENT_STEP            706
#define UL_STEP                58

#define MAX_SW                  1709400
#define MAX_SC_SD               116220200
#define NEGA_MAXVALUE          -10000000

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
            mpTBxVertAlign->Show();
            mpTBxBackColor->Hide();
            mpTBxNumBullet->Hide();
            ReSize(false);
            mpTbxIndent_IncDec->Show();
            mpTbxProDemote->Hide();
            break;

        case CombinedEnumContext(Application_DrawImpress, Context_Draw):
        case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
            mpTBxVertAlign->Hide();
            mpTBxBackColor->Hide();
            mpTBxNumBullet->Show();
            ReSize(true);
            mpTbxIndent_IncDec->Hide();
            mpTbxProDemote->Show();
            break;

        case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
            mpTBxVertAlign->Show();
            mpTBxBackColor->Hide();
            mpTBxNumBullet->Show();
            ReSize(true);
            mpTbxIndent_IncDec->Hide();
            mpTbxProDemote->Show();
            break;

        case CombinedEnumContext(Application_DrawImpress, Context_Table):
            mpTBxVertAlign->Show();
            mpTBxBackColor->Hide();
            mpTBxNumBullet->Show();
            ReSize(true);
            mpTbxIndent_IncDec->Hide();
            mpTbxProDemote->Show();
            break;

        case CombinedEnumContext(Application_WriterVariants, Context_Default):
        case CombinedEnumContext(Application_WriterVariants, Context_Text):
            mpTBxVertAlign->Hide();
            mpTBxBackColor->Show();
            mpTBxNumBullet->Show();

            ReSize(true);
            mpTbxIndent_IncDec->Show();
            mpTbxProDemote->Hide();
            break;

        case CombinedEnumContext(Application_WriterVariants, Context_Table):
            mpTBxVertAlign->Show();
            mpTBxBackColor->Show();
            mpTBxNumBullet->Show();
            ReSize(true);
            mpTbxIndent_IncDec->Show();
            mpTbxProDemote->Hide();
            break;

        case CombinedEnumContext(Application_WriterVariants, Context_DrawText):
            mpTBxVertAlign->Show();
            mpTBxBackColor->Hide();
            mpTBxNumBullet->Hide();
            ReSize(false);
            mpTbxIndent_IncDec->Show();
            mpTbxProDemote->Hide();
            break;

        case CombinedEnumContext(Application_WriterVariants, Context_Annotation):
            mpTBxVertAlign->Hide();
            mpTBxBackColor->Hide();
            mpTBxNumBullet->Hide();
            ReSize(false);
            mpTbxIndent_IncDec->Show();
            mpTbxProDemote->Hide();
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

void ParaPropertyPanel::ReSize(bool /* bSize */)
{
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
    const sal_uInt16 nIdLeft    = mpAlignToolBox->GetItemId(UNO_LEFTPARA);
    const sal_uInt16 nIdCenter  = mpAlignToolBox->GetItemId(UNO_CENTERPARA);
    const sal_uInt16 nIdRight   = mpAlignToolBox->GetItemId(UNO_RIGHTPARA);
    const sal_uInt16 nIdJustify = mpAlignToolBox->GetItemId(UNO_JUSTIFYPARA);

    mpAlignToolBox->SetItemImage(nIdLeft, maLeftAlignControl.GetIcon());
    mpAlignToolBox->SetItemImage(nIdCenter, maCenterAlignControl.GetIcon());
    mpAlignToolBox->SetItemImage(nIdRight, maRightAlignControl.GetIcon());
    mpAlignToolBox->SetItemImage(nIdJustify, maJustifyAlignControl.GetIcon());

    Link aLink = LINK( this, ParaPropertyPanel, AlignStyleModifyHdl_Impl );
    mpAlignToolBox->SetSelectHdl( aLink );
}

void ParaPropertyPanel::InitToolBoxVertAlign()
{
    const sal_uInt16 nIdVertTop     = mpTBxVertAlign->GetItemId(UNO_CELLVERTTOP);
    const sal_uInt16 nIdVertCenter  = mpTBxVertAlign->GetItemId(UNO_CELLVERTCENTER);
    const sal_uInt16 nIdVertBottom  = mpTBxVertAlign->GetItemId(UNO_CELLVERTBOTTOM);

    mpTBxVertAlign->SetItemImage(nIdVertTop, maVertTop.GetIcon());
    mpTBxVertAlign->SetItemImage(nIdVertCenter, maVertCenter.GetIcon());
    mpTBxVertAlign->SetItemImage(nIdVertBottom, maVertBottom.GetIcon());
    mpTBxVertAlign->SetSelectHdl(LINK(this,ParaPropertyPanel,VertTbxSelectHandler));
}


void ParaPropertyPanel::InitToolBoxIndent()
{
    Link aLink = LINK( this, ParaPropertyPanel, ModifyIndentHdl_Impl );
    mpLeftIndent->SetModifyHdl( aLink );
    mpRightIndent->SetModifyHdl( aLink );
    mpFLineIndent->SetModifyHdl( aLink );

    mpLeftIndent->SetAccessibleName(mpLeftIndent->GetQuickHelpText());
    mpRightIndent->SetAccessibleName(mpRightIndent->GetQuickHelpText());
    mpFLineIndent->SetAccessibleName(mpFLineIndent->GetQuickHelpText());

    const sal_uInt16 nIdIncrement = mpTbxIndent_IncDec->GetItemId(UNO_INCREMENTINDENT);
    const sal_uInt16 nIdDecrement = mpTbxIndent_IncDec->GetItemId(UNO_DECREMENTINDENT);
    const sal_uInt16 nIdHanging   = mpTbxIndent_IncDec->GetItemId(UNO_HANGINGINDENT);

    if( Application::GetSettings().GetLayoutRTL())
    {
        mpTbxIndent_IncDec->SetItemImage(nIdIncrement, maIncIndentControl.GetIcon());
        mpTbxIndent_IncDec->SetItemImage(nIdDecrement, maDecIndentControl.GetIcon());
    }
    else
    {
        mpTbxIndent_IncDec->SetItemImage(nIdIncrement, maIncIndentControl.GetIcon());
        mpTbxIndent_IncDec->SetItemImage(nIdDecrement, maDecIndentControl.GetIcon());
    }
    mpTbxIndent_IncDec->SetItemImage(nIdHanging, maIndHang);

    aLink = LINK( this, ParaPropertyPanel, ClickIndent_IncDec_Hdl_Impl );
    mpTbxIndent_IncDec->SetSelectHdl(aLink);
    m_eLRSpaceUnit = maLRSpaceControl.GetCoreMetric();

    const sal_uInt16 nIdPromote  = mpTbxProDemote->GetItemId(UNO_PROMOTE);
    const sal_uInt16 nIdDemote   = mpTbxProDemote->GetItemId(UNO_DEMOTE);
    const sal_uInt16 nIdHanging2 = mpTbxProDemote->GetItemId(UNO_HANGINGINDENT2);
    if( Application::GetSettings().GetLayoutRTL())
    {
        mpTbxProDemote->SetItemImage(nIdPromote, maOutLineLeftControl.GetIcon());
        mpTbxProDemote->SetItemImage(nIdDemote, maOutLineRightControl.GetIcon());
    }
    else
    {
        mpTbxProDemote->SetItemImage(nIdPromote, maOutLineLeftControl.GetIcon());
        mpTbxProDemote->SetItemImage(nIdDemote, maOutLineRightControl.GetIcon());
    }
    mpTbxProDemote->SetItemImage(nIdHanging2, maIndHang);
    aLink = LINK( this, ParaPropertyPanel, ClickProDemote_Hdl_Impl );
    mpTbxProDemote->SetSelectHdl(aLink);
    m_eLRSpaceUnit = maLRSpaceControl.GetCoreMetric();
}

void ParaPropertyPanel::InitToolBoxBGColor()
{
    const sal_uInt16 nIdBackColor = mpTBxBackColor->GetItemId(UNO_PARABACKCOLOR);

    mpColorUpdater.reset(new ::svx::ToolboxButtonColorUpdater(SID_BACKGROUND_COLOR, nIdBackColor, mpTBxBackColor));

    mpTBxBackColor->SetItemImage(nIdBackColor, GetDisplayBackground().GetColor().IsDark()? maImgBackColorHigh : maImgBackColor);
    mpTBxBackColor->SetItemBits( nIdBackColor, mpTBxBackColor->GetItemBits( nIdBackColor ) | TIB_DROPDOWNONLY );

    Link aLink = LINK(this, ParaPropertyPanel, ToolBoxBackColorDDHandler);
    mpTBxBackColor->SetDropdownClickHdl ( aLink );
    mpTBxBackColor->SetSelectHdl ( aLink );
}

void ParaPropertyPanel::InitToolBoxBulletsNumbering()
{
    const sal_uInt16 nIdBullet = mpTBxNumBullet->GetItemId(UNO_DEFAULTBULLET);
    const sal_uInt16 nIdNumber = mpTBxNumBullet->GetItemId(UNO_DEFAULTNUMBERING);

    mpTBxNumBullet->SetItemImage(nIdBullet, maBulletOnOff.GetIcon());
    mpTBxNumBullet->SetItemImage(nIdNumber, maNumberOnOff.GetIcon());

    mpTBxNumBullet->SetDropdownClickHdl(LINK(this,ParaPropertyPanel,NumBTbxDDHandler));
    mpTBxNumBullet->SetSelectHdl(LINK(this,ParaPropertyPanel,NumBTbxSelectHandler));
}
void ParaPropertyPanel::InitToolBoxSpacing()
{
    Link aLink = LINK( this, ParaPropertyPanel, ULSpaceHdl_Impl );
    mpTopDist->SetModifyHdl(aLink);
    mpBottomDist->SetModifyHdl( aLink );

    mpTopDist->SetAccessibleName(mpTopDist->GetQuickHelpText());
    mpBottomDist->SetAccessibleName(mpBottomDist->GetQuickHelpText());

    // Use a form of image loading that can handle both .uno:<command>
    // and private:graphirepository... syntax.  This is necessary to
    // handle the workaround for accessing the images of commands
    // ParaspaceIncrease and ParaspaceDecrease.
    // See issue 122446 for more details.

    const sal_uInt16 nIdParaSpaceInc = mpTbxUL_IncDec->GetItemId(UNO_PARASPACEINC);
    const sal_uInt16 nIdParaSpaceDec = mpTbxUL_IncDec->GetItemId(UNO_PARASPACEDEC);
    mpTbxUL_IncDec->SetItemImage(
        nIdParaSpaceInc,
        sfx2::sidebar::Tools::GetImage("private:graphicrepository/cmd/sc_paraspaceincrease.png" /* i#122446 */, mxFrame));
    mpTbxUL_IncDec->SetItemImage(
        nIdParaSpaceDec,
        sfx2::sidebar::Tools::GetImage("private:graphicrepository/cmd/sc_paraspacedecrease.png" /* i#122446 */, mxFrame));

    aLink = LINK( this, ParaPropertyPanel, ClickUL_IncDec_Hdl_Impl );
    mpTbxUL_IncDec->SetSelectHdl(aLink);
    m_eULSpaceUnit = maULSpaceControl.GetCoreMetric();
}

void ParaPropertyPanel::InitToolBoxLineSpacing()
{
    Link aLink = LINK( this, ParaPropertyPanel, ClickLineSPDropDownHdl_Impl );
    mpLineSPTbx->SetDropdownClickHdl( aLink );
    mpLineSPTbx->SetSelectHdl( aLink );     //support keyinput "ENTER"

    const sal_uInt16 nIdSpacing = mpLineSPTbx->GetItemId(UNO_LINESPACING);
    mpLineSPTbx->SetItemBits( nIdSpacing, mpLineSPTbx->GetItemBits( nIdSpacing ) | TIB_DROPDOWNONLY );
    mpLineSPTbx->SetItemImage(nIdSpacing, maSpace3);
}

void ParaPropertyPanel::initial()
{
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
    mpAlignToolBox->SetAccRelationLabeledBy(&mpAlignToolBox);
    mpTBxVertAlign->SetMpSubEditAccLableBy(&mpTBxVertAlign);
    mpTBxNumBullet->SetAccRelationLabeledBy(&mpTBxNumBullet);
    mpTBxBackColor->SetMpSubEditAccLableBy(&mpTBxBackColor);
    mpTbxUL_IncDec->SetAccRelationLabeledBy(&mpTbxUL_IncDec);
    mpTopDist->SetAccRelationLabeledBy(&mpTopDist);
    mpBottomDist->SetAccRelationLabeledBy(&mpBottomDist);
    mpLineSPTbx->SetAccRelationLabeledBy(&mpLineSPTbx);
    mpTbxIndent_IncDec->SetAccRelationLabeledBy(&mpTbxIndent_IncDec);
    mpTbxProDemote->SetAccRelationLabeledBy(&mpTbxProDemote);
    mpLeftIndent->SetAccRelationLabeledBy(&mpLeftIndent);
    mpRightIndent->SetAccRelationLabeledBy(&mpRightIndent);
    mpFLineIndent->SetAccRelationLabeledBy(&mpFLineIndent);
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
    const sal_uInt16 nId = pToolBox->GetCurItemId();
    const OUString aCommand(pToolBox->GetItemCommand(nId));

    EndTracking();
    pToolBox->SetItemDown( nId, sal_True );

    if (aCommand == UNO_DEFAULTBULLET)
    {
        maBulletsPopup.UpdateValueSet();
        maBulletsPopup.Show(*pToolBox);
    }
    else if (aCommand == UNO_DEFAULTNUMBERING)
    {
        maNumberingPopup.UpdateValueSet();
        maNumberingPopup.Show(*pToolBox);
    }
    pToolBox->SetItemDown( nId, sal_False );
    return 0;
}

IMPL_LINK(ParaPropertyPanel, NumBTbxSelectHandler, ToolBox*, pToolBox)
{
    const OUString aCommand(pToolBox->GetItemCommand(pToolBox->GetCurItemId()));
    sal_uInt16 nSID = SID_TABLE_VERT_NONE;

    EndTracking();
    if (aCommand == UNO_DEFAULTBULLET)
    {
        nSID = FN_NUM_BULLET_ON;
    }
    else if (aCommand == UNO_DEFAULTNUMBERING)
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
    const OUString aCommand(pToolBox->GetItemCommand(pToolBox->GetCurItemId()));
    sal_uInt16 nSID = SID_TABLE_VERT_NONE;
    EndTracking();

    const sal_uInt16 nIdVertTop    = mpTBxVertAlign->GetItemId(UNO_CELLVERTTOP);
    const sal_uInt16 nIdVertCenter = mpTBxVertAlign->GetItemId(UNO_CELLVERTCENTER);
    const sal_uInt16 nIdVertBottom = mpTBxVertAlign->GetItemId(UNO_CELLVERTBOTTOM);

    if (aCommand == UNO_CELLVERTTOP)
    {
        nSID = SID_TABLE_VERT_NONE;
        mpTBxVertAlign->SetItemState(nIdVertTop, STATE_CHECK);
        mpTBxVertAlign->SetItemState(nIdVertCenter, STATE_NOCHECK);
        mpTBxVertAlign->SetItemState(nIdVertBottom, STATE_NOCHECK);
    }
    else if (aCommand == UNO_CELLVERTCENTER)
    {
        nSID = SID_TABLE_VERT_CENTER;
        mpTBxVertAlign->SetItemState(nIdVertTop, STATE_NOCHECK);
        mpTBxVertAlign->SetItemState(nIdVertCenter, STATE_CHECK);
        mpTBxVertAlign->SetItemState(nIdVertBottom, STATE_NOCHECK);
    }
    else if (aCommand == UNO_CELLVERTBOTTOM)
    {
        nSID = SID_TABLE_VERT_BOTTOM;
        mpTBxVertAlign->SetItemState(nIdVertTop, STATE_NOCHECK);
        mpTBxVertAlign->SetItemState(nIdVertCenter, STATE_NOCHECK);
        mpTBxVertAlign->SetItemState(nIdVertBottom, STATE_CHECK);
    }
    SfxBoolItem aBoolItem(nSID, sal_True);
    GetBindings()->GetDispatcher()->Execute(nSID, SFX_CALLMODE_RECORD, &aBoolItem, 0L);

    return 0;
}

void ParaPropertyPanel::VertStateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    const sal_uInt16 nIdVertTop     = mpTBxVertAlign->GetItemId(UNO_CELLVERTTOP);
    const sal_uInt16 nIdVertCenter  = mpTBxVertAlign->GetItemId(UNO_CELLVERTCENTER);
    const sal_uInt16 nIdVertBottom  = mpTBxVertAlign->GetItemId(UNO_CELLVERTBOTTOM);

    if (eState < SFX_ITEM_DONTCARE)
    {
        mpTBxVertAlign->SetItemState(nIdVertTop, STATE_NOCHECK);
        mpTBxVertAlign->SetItemState(nIdVertCenter, STATE_NOCHECK);
        mpTBxVertAlign->SetItemState(nIdVertBottom, STATE_NOCHECK);
        mpTBxVertAlign->EnableItem(nIdVertTop, sal_False);
        mpTBxVertAlign->EnableItem(nIdVertCenter, sal_False);
        mpTBxVertAlign->EnableItem(nIdVertBottom, sal_False);
    }
    else
    {
        mpTBxVertAlign->EnableItem(nIdVertTop, sal_True);
        mpTBxVertAlign->EnableItem(nIdVertCenter, sal_True);
        mpTBxVertAlign->EnableItem(nIdVertBottom, sal_True);
        if ( (eState >= SFX_ITEM_DEFAULT) && (pState->ISA(SfxBoolItem)))
        {
            const SfxBoolItem* pItem= (const SfxBoolItem*)pState;
            sal_Bool aBool = (sal_Bool)pItem->GetValue();

            if (aBool)
            {
                if (nSID == SID_TABLE_VERT_NONE)
                {
                    mpTBxVertAlign->SetItemState(nIdVertTop, STATE_CHECK);
                    mpTBxVertAlign->SetItemState(nIdVertCenter, STATE_NOCHECK);
                    mpTBxVertAlign->SetItemState(nIdVertBottom, STATE_NOCHECK);
                }
                else if (nSID == SID_TABLE_VERT_CENTER)
                {
                    mpTBxVertAlign->SetItemState(nIdVertTop, STATE_NOCHECK);
                    mpTBxVertAlign->SetItemState(nIdVertCenter, STATE_CHECK);
                    mpTBxVertAlign->SetItemState(nIdVertBottom, STATE_NOCHECK);
                }
                else if (nSID == SID_TABLE_VERT_BOTTOM)
                {
                    mpTBxVertAlign->SetItemState(nIdVertTop, STATE_NOCHECK);
                    mpTBxVertAlign->SetItemState(nIdVertCenter, STATE_NOCHECK);
                    mpTBxVertAlign->SetItemState(nIdVertBottom, STATE_CHECK);
                }
            }
            else
            {
                if (nSID == SID_TABLE_VERT_NONE)
                {
                    mpTBxVertAlign->SetItemState(nIdVertTop, STATE_NOCHECK);
                }
                else if (nSID == SID_TABLE_VERT_CENTER)
                {
                    mpTBxVertAlign->SetItemState(nIdVertCenter, STATE_NOCHECK);
                }
                else if (nSID == SID_TABLE_VERT_BOTTOM)
                {
                    mpTBxVertAlign->SetItemState(nIdVertBottom, STATE_NOCHECK);
                }
            }
        }
        else
        {
            mpTBxVertAlign->SetItemState(nIdVertTop, STATE_NOCHECK);
            mpTBxVertAlign->SetItemState(nIdVertCenter, STATE_NOCHECK);
            mpTBxVertAlign->SetItemState(nIdVertBottom, STATE_NOCHECK);
        }
    }
}
//==================================for Background color=====================

IMPL_LINK(ParaPropertyPanel, ToolBoxBackColorDDHandler,ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();
    const OUString aCommand(pToolBox->GetItemCommand(nId));

    if(aCommand == UNO_PARABACKCOLOR)
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
    const sal_uInt16 nIdLeft    = pBox->GetItemId(UNO_LEFTPARA);
    const sal_uInt16 nIdRight   = pBox->GetItemId(UNO_RIGHTPARA);
    const sal_uInt16 nIdCenter  = pBox->GetItemId(UNO_CENTERPARA);
    const sal_uInt16 nIdJustify = pBox->GetItemId(UNO_JUSTIFYPARA);

    const OUString aCommand(pBox->GetItemCommand(pBox->GetCurItemId()));

        if( aCommand == UNO_LEFTPARA )
        {
            pBox->SetItemState(nIdLeft, STATE_CHECK);
            pBox->SetItemState(nIdCenter, STATE_NOCHECK);
            pBox->SetItemState(nIdRight, STATE_NOCHECK);
            pBox->SetItemState(nIdJustify, STATE_NOCHECK);
            SfxBoolItem aBoolItem( SID_ATTR_PARA_ADJUST_LEFT,  sal_True );
            GetBindings()->GetDispatcher()->Execute(SID_ATTR_PARA_ADJUST_LEFT, SFX_CALLMODE_RECORD, &aBoolItem, 0L);
        }
        else if( aCommand == UNO_CENTERPARA )
        {
            pBox->SetItemState(nIdCenter, STATE_CHECK);
            pBox->SetItemState(nIdLeft, STATE_NOCHECK);
            pBox->SetItemState(nIdRight, STATE_NOCHECK);
            pBox->SetItemState(nIdJustify, STATE_NOCHECK);
            SfxBoolItem aBoolItem( SID_ATTR_PARA_ADJUST_CENTER, sal_True );
            GetBindings()->GetDispatcher()->Execute(SID_ATTR_PARA_ADJUST_CENTER, SFX_CALLMODE_RECORD, &aBoolItem, 0L);
        }
        else if( aCommand == UNO_RIGHTPARA )
        {
            pBox->SetItemState(nIdRight, STATE_CHECK);
            pBox->SetItemState(nIdLeft, STATE_NOCHECK);
            pBox->SetItemState(nIdCenter, STATE_NOCHECK);
            pBox->SetItemState(nIdJustify, STATE_NOCHECK);
            SfxBoolItem aBoolItem( SID_ATTR_PARA_ADJUST_RIGHT, sal_True );
            GetBindings()->GetDispatcher()->Execute(SID_ATTR_PARA_ADJUST_RIGHT, SFX_CALLMODE_RECORD, &aBoolItem, 0L);
        }
        else if( aCommand == UNO_JUSTIFYPARA )
        {
            pBox->SetItemState(nIdJustify, STATE_CHECK);
            pBox->SetItemState(nIdLeft, STATE_NOCHECK);
            pBox->SetItemState(nIdRight, STATE_NOCHECK);
            pBox->SetItemState(nIdCenter, STATE_NOCHECK);
            SfxBoolItem aBoolItem( SID_ATTR_PARA_ADJUST_BLOCK, sal_True );
            GetBindings()->GetDispatcher()->Execute(SID_ATTR_PARA_ADJUST_BLOCK, SFX_CALLMODE_RECORD, &aBoolItem, 0L);
        }
    return 0;
}

//==================================for Paragraph Indent=====================
IMPL_LINK_NOARG( ParaPropertyPanel, ModifyIndentHdl_Impl)
{
    SvxLRSpaceItem aMargin( SID_ATTR_PARA_LRSPACE );
    aMargin.SetTxtLeft( (const long)GetCoreValue( *mpLeftIndent, m_eLRSpaceUnit ) );
    aMargin.SetRight( (const long)GetCoreValue( *mpRightIndent, m_eLRSpaceUnit ) );
    aMargin.SetTxtFirstLineOfst( (const short)GetCoreValue( *mpFLineIndent, m_eLRSpaceUnit ) );

    GetBindings()->GetDispatcher()->Execute(
        SID_ATTR_PARA_LRSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
    return 0;
}

IMPL_LINK(ParaPropertyPanel, ClickIndent_IncDec_Hdl_Impl, ToolBox *, pControl)
{
    const OUString aCommand(pControl->GetItemCommand(pControl->GetCurItemId()));

        if (aCommand == UNO_INCREMENTINDENT)
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
                    aMargin.SetRight( (const long)GetCoreValue( *mpRightIndent, m_eLRSpaceUnit ) );
                    aMargin.SetTxtFirstLineOfst( (const short)GetCoreValue( *mpFLineIndent, m_eLRSpaceUnit ) );

                    GetBindings()->GetDispatcher()->Execute(
                        SID_ATTR_PARA_LRSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
                }
            }
        }
        else if (aCommand == UNO_DECREMENTINDENT)
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
                    aMargin.SetRight( (const long)GetCoreValue( *mpRightIndent, m_eLRSpaceUnit ) );
                    aMargin.SetTxtFirstLineOfst( (const short)GetCoreValue( *mpFLineIndent, m_eLRSpaceUnit ) );

                    GetBindings()->GetDispatcher()->Execute(
                        SID_ATTR_PARA_LRSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
                }
            }
        }
        else if (aCommand == UNO_HANGINGINDENT)
        {
            SvxLRSpaceItem aMargin( SID_ATTR_PARA_LRSPACE );
            aMargin.SetTxtLeft( (const long)GetCoreValue( *mpLeftIndent, m_eLRSpaceUnit ) + (const short)GetCoreValue( *mpFLineIndent, m_eLRSpaceUnit ) );
            aMargin.SetRight( (const long)GetCoreValue( *mpRightIndent, m_eLRSpaceUnit ) );
            aMargin.SetTxtFirstLineOfst( ((const short)GetCoreValue( *mpFLineIndent, m_eLRSpaceUnit ))*(-1) );

            GetBindings()->GetDispatcher()->Execute(
                SID_ATTR_PARA_LRSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
        }

    return( 0L );
}

IMPL_LINK(ParaPropertyPanel, ClickProDemote_Hdl_Impl, ToolBox *, pControl)
{
    const OUString aCommand(pControl->GetItemCommand(pControl->GetCurItemId()));

        if (aCommand == UNO_PROMOTE)
        {
            GetBindings()->GetDispatcher()->Execute( SID_OUTLINE_RIGHT, SFX_CALLMODE_RECORD );
        }
        else if (aCommand == UNO_DEMOTE)
        {
            GetBindings()->GetDispatcher()->Execute( SID_OUTLINE_LEFT, SFX_CALLMODE_RECORD );
        }
        else if (aCommand == UNO_HANGINGINDENT2)
        {
            SvxLRSpaceItem aMargin( SID_ATTR_PARA_LRSPACE );
            aMargin.SetTxtLeft( (const long)GetCoreValue( *mpLeftIndent, m_eLRSpaceUnit ) + (const short)GetCoreValue( *mpFLineIndent, m_eLRSpaceUnit ) );
            aMargin.SetRight( (const long)GetCoreValue( *mpRightIndent, m_eLRSpaceUnit ) );
            aMargin.SetTxtFirstLineOfst( ((const short)GetCoreValue( *mpFLineIndent, m_eLRSpaceUnit ))*(-1) );

            GetBindings()->GetDispatcher()->Execute( SID_ATTR_PARA_LRSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
        }

    return( 0L );
}
//==================================for Paragraph Line Spacing=====================

IMPL_LINK( ParaPropertyPanel, ClickLineSPDropDownHdl_Impl, ToolBox*, pBox )
{
    const sal_uInt16 nId = pBox->GetCurItemId();
    const OUString aCommand(pBox->GetItemCommand(nId));

    if (aCommand == UNO_LINESPACING)
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
    aMargin.SetUpper( (sal_uInt16)GetCoreValue( *mpTopDist, m_eULSpaceUnit ) );
    aMargin.SetLower( (sal_uInt16)GetCoreValue( *mpBottomDist, m_eULSpaceUnit ) );

    GetBindings()->GetDispatcher()->Execute(
        SID_ATTR_PARA_ULSPACE, SFX_CALLMODE_RECORD, &aMargin, 0L);
    return 0L;
}

IMPL_LINK(ParaPropertyPanel, ClickUL_IncDec_Hdl_Impl, ToolBox *, pControl)
{
    const OUString aCommand(pControl->GetItemCommand(pControl->GetCurItemId()));

             if( aCommand ==  UNO_PARASPACEINC)
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
             else if( aCommand == UNO_PARASPACEDEC)
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
                SetFieldUnit( *mpLeftIndent, m_eMetricUnit );
                SetFieldUnit( *mpRightIndent, m_eMetricUnit );
                SetFieldUnit( *mpFLineIndent, m_eMetricUnit );
                SetFieldUnit( *mpTopDist, m_eMetricUnit );
                SetFieldUnit( *mpBottomDist, m_eMetricUnit );
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
    const sal_uInt16 nIdLeft    = mpAlignToolBox->GetItemId(UNO_LEFTPARA);
    const sal_uInt16 nIdRight   = mpAlignToolBox->GetItemId(UNO_RIGHTPARA);
    const sal_uInt16 nIdCenter  = mpAlignToolBox->GetItemId(UNO_CENTERPARA);
    const sal_uInt16 nIdJustify = mpAlignToolBox->GetItemId(UNO_JUSTIFYPARA);

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
                    mpAlignToolBox->SetItemState(nIdLeft,STATE_CHECK);
                    mpAlignToolBox->SetItemState(nIdCenter,STATE_NOCHECK);
                    mpAlignToolBox->SetItemState(nIdRight,STATE_NOCHECK);
                    mpAlignToolBox->SetItemState(nIdJustify,STATE_NOCHECK);
                }
                else
                    mpAlignToolBox->SetItemState(nIdLeft,STATE_NOCHECK);
            }
            break;
        case SID_ATTR_PARA_ADJUST_CENTER:
            {
                if(IsChecked)
                {
                    mpAlignToolBox->SetItemState(nIdCenter,STATE_CHECK);
                    mpAlignToolBox->SetItemState(nIdLeft,STATE_NOCHECK);
                    mpAlignToolBox->SetItemState(nIdRight,STATE_NOCHECK);
                    mpAlignToolBox->SetItemState(nIdJustify,STATE_NOCHECK);
                }
                else
                    mpAlignToolBox->SetItemState(nIdCenter,STATE_NOCHECK);
            }
            break;
        case SID_ATTR_PARA_ADJUST_RIGHT:
            {
                if(IsChecked)
                {
                    mpAlignToolBox->SetItemState(nIdRight,STATE_CHECK);
                    mpAlignToolBox->SetItemState(nIdLeft,STATE_NOCHECK);
                    mpAlignToolBox->SetItemState(nIdCenter,STATE_NOCHECK);
                    mpAlignToolBox->SetItemState(nIdJustify,STATE_NOCHECK);
                }
                else
                    mpAlignToolBox->SetItemState(nIdRight,STATE_NOCHECK);
            }
            break;
        case SID_ATTR_PARA_ADJUST_BLOCK:
            {
                if(IsChecked)
                {
                    mpAlignToolBox->SetItemState(nIdJustify,STATE_CHECK);
                    mpAlignToolBox->SetItemState(nIdLeft,STATE_NOCHECK);
                    mpAlignToolBox->SetItemState(nIdCenter,STATE_NOCHECK);
                    mpAlignToolBox->SetItemState(nIdRight,STATE_NOCHECK);
                }
                else
                    mpAlignToolBox->SetItemState(nIdJustify,STATE_NOCHECK);
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
            mpAlignToolBox->SetItemState(nIdLeft,STATE_NOCHECK);
            break;
        case SID_ATTR_PARA_ADJUST_CENTER:
            mpAlignToolBox->SetItemState(nIdCenter,STATE_NOCHECK);
            break;
        case SID_ATTR_PARA_ADJUST_RIGHT:
            mpAlignToolBox->SetItemState(nIdRight,STATE_NOCHECK);
            break;
        case SID_ATTR_PARA_ADJUST_BLOCK:
            mpAlignToolBox->SetItemState(nIdJustify,STATE_NOCHECK);
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
            mpLeftIndent->SetMin( DEFAULT_VALUE );
            mpRightIndent->SetMin( DEFAULT_VALUE );
            mpFLineIndent->SetMin( DEFAULT_VALUE );
            mpTbxIndent_IncDec->Show();
            mpTbxProDemote->Hide();
        }
        break;
    case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
    case CombinedEnumContext(Application_DrawImpress, Context_Draw):
    case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
    case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
    case CombinedEnumContext(Application_DrawImpress, Context_Table):
        {
            mpLeftIndent->SetMin( DEFAULT_VALUE );
            mpRightIndent->SetMin( DEFAULT_VALUE );
            mpFLineIndent->SetMin( DEFAULT_VALUE );
            mpTbxIndent_IncDec->Hide();
            mpTbxProDemote->Show();
        }
        break;
    case CombinedEnumContext(Application_WriterVariants, Context_Default):
    case CombinedEnumContext(Application_WriterVariants, Context_Text):
    case CombinedEnumContext(Application_WriterVariants, Context_Table):
        {
            mpLeftIndent->SetMin( NEGA_MAXVALUE, FUNIT_100TH_MM );
            mpRightIndent->SetMin( NEGA_MAXVALUE, FUNIT_100TH_MM );
            mpFLineIndent->SetMin( NEGA_MAXVALUE, FUNIT_100TH_MM );
            mpTbxIndent_IncDec->Show();
            mpTbxProDemote->Hide();
        }
        break;
    }

    const sal_uInt16 nIdHangingIndent   = mpTbxIndent_IncDec->GetItemId(UNO_HANGINGINDENT);
    const sal_uInt16 nIdHangingIndent2  = mpTbxIndent_IncDec->GetItemId(UNO_HANGINGINDENT2);
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
        nVal = (long)mpLeftIndent->Normalize( (long)nVal );
        mpLeftIndent->SetValue( nVal, FUNIT_100TH_MM );

        if ( maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Text)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Default)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Table))
        {
            mpFLineIndent->SetMin( nVal*(-1), FUNIT_100TH_MM );
        }

        long nrVal = OutputDevice::LogicToLogic( aTxtRight, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
        nrVal = (long)mpRightIndent->Normalize( (long)nrVal );
        mpRightIndent->SetValue( nrVal, FUNIT_100TH_MM );

        long nfVal = OutputDevice::LogicToLogic( aTxtFirstLineOfst, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
        nfVal = (long)mpFLineIndent->Normalize( (long)nfVal );
        mpFLineIndent->SetValue( nfVal, FUNIT_100TH_MM );

        switch (maContext.GetCombinedContext_DI())
        {
        case CombinedEnumContext(Application_WriterVariants, Context_DrawText):
        case CombinedEnumContext(Application_WriterVariants, Context_Text):
        case CombinedEnumContext(Application_WriterVariants, Context_Default):
        case CombinedEnumContext(Application_WriterVariants, Context_Table):
        case CombinedEnumContext(Application_WriterVariants, Context_Annotation):
            {
                mpLeftIndent->SetMax( MAX_SW - nrVal, FUNIT_100TH_MM );
                mpRightIndent->SetMax( MAX_SW - nVal, FUNIT_100TH_MM );
                mpFLineIndent->SetMax( MAX_SW - nVal - nrVal, FUNIT_100TH_MM );
            }
            break;
        case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
        case CombinedEnumContext(Application_DrawImpress, Context_Draw):
        case CombinedEnumContext(Application_DrawImpress, Context_Table):
        case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
            {
                mpLeftIndent->SetMax( MAX_SC_SD - nrVal, FUNIT_100TH_MM );
                mpRightIndent->SetMax( MAX_SC_SD - nVal, FUNIT_100TH_MM );
                mpFLineIndent->SetMax( MAX_SC_SD - nVal - nrVal, FUNIT_100TH_MM );
            }
        }

        mpTbxIndent_IncDec->Enable();

        const sal_uInt16 nIdIncrIndent  = mpTbxIndent_IncDec->GetItemId(UNO_INCREMENTINDENT);
        const sal_uInt16 nIdDecrIndent  = mpTbxIndent_IncDec->GetItemId(UNO_DECREMENTINDENT);

        mpTbxIndent_IncDec->EnableItem(nIdHangingIndent, sal_True);
        if ( maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Text)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Default)
             && maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Table) )
        {
            mpTbxIndent_IncDec->EnableItem(nIdIncrIndent, sal_True);
            mpTbxIndent_IncDec->EnableItem(nIdDecrIndent, sal_True);
        }

        mpTbxProDemote->EnableItem(nIdHangingIndent2, sal_True);
    }
    else if( eState == SFX_ITEM_DISABLED )
    {
        mpLeftIndent-> Disable();
        mpRightIndent->Disable();
        mpFLineIndent->Disable();
        mpTbxIndent_IncDec->Disable();
        if( maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Text)  &&
            maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Default) &&
            maContext.GetCombinedContext_DI() !=  CombinedEnumContext(Application_WriterVariants, Context_Table) )
            mpTbxIndent_IncDec->Disable();
        else
            mpTbxIndent_IncDec->EnableItem(nIdHangingIndent, sal_False);

        //      maTbxProDemote->Disable();
        mpTbxProDemote->EnableItem(nIdHangingIndent2, sal_False);
    }
    else
    {
        mpLeftIndent->SetEmptyFieldValue();
        mpRightIndent->SetEmptyFieldValue();
        mpFLineIndent->SetEmptyFieldValue();
        if( maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Text)  &&
            maContext.GetCombinedContext_DI() != CombinedEnumContext(Application_WriterVariants, Context_Default) &&
            maContext.GetCombinedContext_DI() !=  CombinedEnumContext(Application_WriterVariants, Context_Table) )
            mpTbxIndent_IncDec->Disable();
        else
            mpTbxIndent_IncDec->EnableItem(nIdHangingIndent, sal_False);
        mpTbxProDemote->EnableItem(nIdHangingIndent2, sal_False);
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
    mpTopDist->SetMax( mpTopDist->Normalize( MAX_DURCH ), MapToFieldUnit(m_eULSpaceUnit) );
    mpBottomDist->SetMax( mpBottomDist->Normalize( MAX_DURCH ), MapToFieldUnit(m_eULSpaceUnit) );

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
        nVal = mpTopDist->Normalize( nVal );
        mpTopDist->SetValue( nVal, FUNIT_100TH_MM );

        nVal = OutputDevice::LogicToLogic( maLower, (MapUnit)(SFX_MAPUNIT_TWIP), MAP_100TH_MM );
        nVal = mpBottomDist->Normalize( nVal );
        mpBottomDist->SetValue( nVal, FUNIT_100TH_MM );
        mpTbxUL_IncDec->Enable();
    }
    else if(eState == SFX_ITEM_DISABLED )
    {
        mpTopDist->Disable();
        mpBottomDist->Disable();
        mpTbxUL_IncDec->Disable();
    }
    else
    {
        mpTopDist->SetEmptyFieldValue();
        mpBottomDist->SetEmptyFieldValue();
        mpTbxUL_IncDec->Disable();
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

    const sal_uInt16 nIdDemote = mpTbxProDemote->GetItemId(UNO_DEMOTE);
    if(mbOutLineLeft)
        mpTbxProDemote->EnableItem(nIdDemote, sal_True);
    else
        mpTbxProDemote->EnableItem(nIdDemote, sal_False);

    const sal_uInt16 nIdPromote = mpTbxProDemote->GetItemId(UNO_PROMOTE);
    if(mbOutLineRight)
        mpTbxProDemote->EnableItem(nIdPromote, sal_True);
    else
        mpTbxProDemote->EnableItem(nIdPromote, sal_False);

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
        const sal_uInt16 nIdIncrIndent = mpTbxIndent_IncDec->GetItemId(UNO_INCREMENTINDENT);
        const sal_uInt16 nIdDecrIndent = mpTbxIndent_IncDec->GetItemId(UNO_DECREMENTINDENT);

        mpTbxIndent_IncDec->EnableItem(
            nSID == SID_INC_INDENT ? nIdIncrIndent : nIdDecrIndent,
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

        const sal_uInt16 nIdNumber = mpTBxNumBullet->GetItemId(UNO_DEFAULTNUMBERING);
        const sal_uInt16 nIdBullet = mpTBxNumBullet->GetItemId(UNO_DEFAULTBULLET);

        if (nSID==FN_NUM_NUMBERING_ON)
        {
            mpTBxNumBullet->SetItemState(
                nIdNumber,
                aBool ? STATE_CHECK : STATE_NOCHECK );
        }
        else if (nSID==FN_NUM_BULLET_ON)
        {
            mpTBxNumBullet->SetItemState(
                nIdBullet,
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
    : PanelLayout(pParent, "ParaPropertyPanel", "svx/ui/sidebarparagraph.ui", rxFrame),

      mpColorUpdater (),
      maSpace3 (SVX_RES(IMG_SPACE3)),
      maIndHang (SVX_RES(IMG_INDENT_HANG)),
      maNumBImageList (SVX_RES(IL_NUM_BULLET)),
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
    //Alignment
    get(mpAlignToolBox, "horizontalalignment");
    get(mpTBxVertAlign, "verticalalignment");
    //NumBullet&Backcolor
    get(mpTBxNumBullet, "numberbullet");
    get(mpTBxBackColor, "backgroundcolor");
    //Paragraph spacing
    get(mpTopDist,      "aboveparaspacing");
    get(mpBottomDist,   "belowparaspacing");
    get(mpTbxIndent_IncDec, "indent");
    get(mpTbxProDemote, "promotedemote");
    get(mpLineSPTbx,    "linespacing");
    get(mpTbxUL_IncDec, "paraspacing");
    get(mpLeftIndent,   "beforetextindent");
    get(mpRightIndent,  "aftertextindent");
    get(mpFLineIndent,  "firstlineindent");

    initial();
}

} } // end of namespace svx::sidebar
