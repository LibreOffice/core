/*************************************************************************
 *
 *  $RCSfile: LayoutMenu.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-15 08:59:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "LayoutMenu.hxx"

#include "TaskPaneShellManager.hxx"
#include "pres.hxx"
#include "drawdoc.hxx"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#include "sdpage.hxx"
#include "glob.hxx"
#include "glob.hrc"
#include "app.hrc"
#include "res_bmp.hrc"
#include "strings.hrc"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#include "PaneManager.hxx"
#include "SlideSorterViewShell.hxx"

#ifndef _SDRESID_HXX
#include "sdresid.hxx"
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>

using namespace ::sd::toolpanel;
#define LayoutMenu
#include "sdslots.hxx"

using namespace ::com::sun::star::text;
using namespace ::sd::slidesorter;

namespace sd { namespace toolpanel {

SFX_IMPL_INTERFACE(LayoutMenu, SfxShell,
    SdResId(STR_TASKPANELAYOUTMENU))
{
    SFX_POPUPMENU_REGISTRATION(SdResId(RID_TASKPANE_LAYOUTMENU_POPUP));
}

TYPEINIT1(LayoutMenu, SfxShell);

struct snewfoil_value_info
{
    USHORT mnBmpResId;
    USHORT mnHCBmpResId;
    USHORT mnStrResId;
    WritingMode meWritingMode;
    AutoLayout maAutoLayout;
};

static snewfoil_value_info notes[] =
{
    {BMP_FOILN_01, BMP_FOILN_01_H, STR_AUTOLAYOUT_NOTES, WritingMode_LR_TB,
     AUTOLAYOUT_NOTES},
    {0, 0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE},
};

static snewfoil_value_info handout[] =
{
    {BMP_FOILH_01, BMP_FOILH_01_H, STR_AUTOLAYOUT_HANDOUT1, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT1},
    {BMP_FOILH_02, BMP_FOILH_02_H, STR_AUTOLAYOUT_HANDOUT2, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT2},
    {BMP_FOILH_03, BMP_FOILH_03_H, STR_AUTOLAYOUT_HANDOUT3, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT3},
    {BMP_FOILH_04, BMP_FOILH_04_H, STR_AUTOLAYOUT_HANDOUT4, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT4},
    {BMP_FOILH_06, BMP_FOILH_06_H, STR_AUTOLAYOUT_HANDOUT6, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT6},
    {0, 0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE},
};

static snewfoil_value_info standard[] =
{
    {BMP_FOIL_20, BMP_FOIL_20_H, STR_AUTOLAYOUT_NONE, WritingMode_LR_TB,
     AUTOLAYOUT_NONE},
    {BMP_FOIL_00, BMP_FOIL_00_H, STR_AUTOLAYOUT_TITLE, WritingMode_LR_TB,
     AUTOLAYOUT_TITLE},
    {BMP_FOIL_01, BMP_FOIL_01_H, STR_AUTOLAYOUT_ENUM, WritingMode_LR_TB,
     AUTOLAYOUT_ENUM},
    {BMP_FOIL_03, BMP_FOIL_03_H, STR_AUTOLAYOUT_2TEXT, WritingMode_LR_TB,
     AUTOLAYOUT_2TEXT},
    {BMP_FOIL_19, BMP_FOIL_19_H, STR_AUTOLAYOUT_ONLY_TITLE, WritingMode_LR_TB,
     AUTOLAYOUT_ONLY_TITLE},
    {BMP_FOIL_11, BMP_FOIL_11_H, STR_AUTOLAYOUT_OBJ, WritingMode_LR_TB,
     AUTOLAYOUT_OBJ},
    {BMP_FOIL_02, BMP_FOIL_02_H, STR_AUTOLAYOUT_CHART, WritingMode_LR_TB,
     AUTOLAYOUT_CHART},
    {BMP_FOIL_08, BMP_FOIL_08_H, STR_AUTOLAYOUT_TAB, WritingMode_LR_TB,
     AUTOLAYOUT_TAB},
    {BMP_FOIL_09, BMP_FOIL_09_H, STR_AUTOLAYOUT_CLIPTEXT, WritingMode_LR_TB,
     AUTOLAYOUT_CLIPTEXT},
    {BMP_FOIL_04, BMP_FOIL_04_H, STR_AUTOLAYOUT_TEXTCHART, WritingMode_LR_TB,
     AUTOLAYOUT_TEXTCHART},
    {BMP_FOIL_06, BMP_FOIL_06_H, STR_AUTOLAYOUT_TEXTCLIP, WritingMode_LR_TB,
    AUTOLAYOUT_TEXTCLIP},
    {BMP_FOIL_07, BMP_FOIL_07_H, STR_AUTOLAYOUT_CHARTTEXT, WritingMode_LR_TB,
     AUTOLAYOUT_CHARTTEXT},
    {BMP_FOIL_10, BMP_FOIL_10_H, STR_AUTOLAYOUT_TEXTOBJ, WritingMode_LR_TB,
     AUTOLAYOUT_TEXTOBJ},
    {BMP_FOIL_12, BMP_FOIL_12_H, STR_AUTOLAYOUT_TEXT2OBJ, WritingMode_LR_TB,
     AUTOLAYOUT_TEXT2OBJ},
    {BMP_FOIL_13, BMP_FOIL_13_H, STR_AUTOLAYOUT_OBJTEXT, WritingMode_LR_TB,
     AUTOLAYOUT_OBJTEXT},
    {BMP_FOIL_14, BMP_FOIL_14_H, STR_AUTOLAYOUT_OBJOVERTEXT, WritingMode_LR_TB,
     AUTOLAYOUT_OBJOVERTEXT},
    {BMP_FOIL_15, BMP_FOIL_15_H, STR_AUTOLAYOUT_2OBJTEXT, WritingMode_LR_TB,
     AUTOLAYOUT_2OBJTEXT},
    {BMP_FOIL_16, BMP_FOIL_16_H, STR_AUTOLAYOUT_2OBJOVERTEXT,
     WritingMode_LR_TB, AUTOLAYOUT_2OBJOVERTEXT},
    {BMP_FOIL_17, BMP_FOIL_17_H, STR_AUTOLAYOUT_TEXTOVEROBJ, WritingMode_LR_TB,
     AUTOLAYOUT_TEXTOVEROBJ},
    {BMP_FOIL_18, BMP_FOIL_18_H, STR_AUTOLAYOUT_4OBJ, WritingMode_LR_TB,
     AUTOLAYOUT_4OBJ},
    // vertical
    {BMP_FOIL_21, BMP_FOIL_21_H, STR_AL_VERT_TITLE_TEXT_CHART,
     WritingMode_TB_RL, AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART},
    {BMP_FOIL_22, BMP_FOIL_22_H, STR_AL_VERT_TITLE_VERT_OUTLINE,
     WritingMode_TB_RL, AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE},
    {BMP_FOIL_23, BMP_FOIL_23_H, STR_AL_TITLE_VERT_OUTLINE, WritingMode_TB_RL,
     AUTOLAYOUT_TITLE_VERTICAL_OUTLINE},
    {BMP_FOIL_24, BMP_FOIL_24_H, STR_AL_TITLE_VERT_OUTLINE_CLIPART,
     WritingMode_TB_RL, AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART},

    {0, 0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE}
};



LayoutMenu::LayoutMenu (
    TreeNode* pParent,
    DrawDocShell& rDocumentShell,
    ViewShellBase& rViewShellBase,
    bool bUseOwnScrollBar)
    : ValueSet (pParent->GetWindow()),
      TreeNode(pParent),
      DragSourceHelper(this),
      DropTargetHelper(this),
      mrBase (rViewShellBase),
      mbUseOwnScrollBar (bUseOwnScrollBar),
      mnPreferredColumnCount (3)
{
    SetStyle (
        GetStyle()
        & ~(WB_ITEMBORDER)
        | WB_TABSTOP
        );
    if (mbUseOwnScrollBar)
        SetStyle (GetStyle() | WB_VSCROLL);
    SetExtraSpacing(2);
    SetSelectHdl (LINK(this, LayoutMenu, ClickHandler));
    SetPool (&rDocumentShell.GetDoc()->GetPool());

    Fill ();
    pParent->RequestResize();

    mrBase.GetPaneManager().AddEventListener (
        LINK(this, LayoutMenu, ViewShellChangeCallback));
}




LayoutMenu::~LayoutMenu (void)
{
    Clear();
    mrBase.GetPaneManager().RemoveEventListener (
        LINK(this, LayoutMenu, ViewShellChangeCallback));
}




String LayoutMenu::GetSelectedLayoutName (void)
{
    return GetItemText (GetSelectItemId());
}




AutoLayout LayoutMenu::GetSelectedAutoLayout (void)
{
    return *static_cast<AutoLayout*>(GetItemData(GetSelectItemId()));
}




/** The preferred size depends on the preferred number of columns, the
    number of items, and the size of the items.
*/
Size LayoutMenu::GetPreferredSize (void)
{
    Size aItemSize = CalcItemSizePixel (Size());
    Size aPreferredWindowSize = CalcWindowSizePixel (
        aItemSize,
        mnPreferredColumnCount,
        CalculateRowCount (aItemSize, mnPreferredColumnCount));
    return aPreferredWindowSize;
}




sal_Int32 LayoutMenu::GetPreferredWidth (sal_Int32 nHeight)
{
    sal_Int32 nPreferredWidth = 100;
    if (GetItemCount() > 0)
    {
        Image aImage = GetItemImage(GetItemId(0));
        Size aItemSize = CalcItemSizePixel (aImage.GetSizePixel());
        if (nHeight>0 && aItemSize.Height()>0)
        {
            int nRowCount = nHeight / aItemSize.Height();
            if (nRowCount <= 0)
                nRowCount = 1;
            int nColumnCount = (GetItemCount() + nRowCount-1) / nRowCount;
            nPreferredWidth = nColumnCount * aItemSize.Width();
        }
    }

    return nPreferredWidth;
}




sal_Int32 LayoutMenu::GetPreferredHeight (sal_Int32 nWidth)
{
    sal_Int32 nPreferredHeight = 200;
    if ( ! mbUseOwnScrollBar && GetItemCount()>0)
    {
        Image aImage = GetItemImage(GetItemId(0));
        Size aItemSize = CalcItemSizePixel (aImage.GetSizePixel());
        if (nWidth>0 && aItemSize.Width()>0)
        {
            aItemSize.Width() += 8;
            aItemSize.Height() += 8;
            int nColumnCount = nWidth / aItemSize.Width();
            if (nColumnCount <= 0)
                nColumnCount = 1;
            else if (nColumnCount > 4)
                nColumnCount = 4;
            int nRowCount = (GetItemCount() + nColumnCount-1) / nColumnCount;
            nPreferredHeight = nRowCount * aItemSize.Height();
        }
    }
    return nPreferredHeight;
}




sal_Int32 LayoutMenu::GetMinimumWidth (void)
{
    sal_Int32 nMinimumWidth = 0;
    if (GetItemCount()>0)
    {
        Image aImage = GetItemImage(GetItemId(0));
        Size aItemSize = CalcItemSizePixel (aImage.GetSizePixel());
        nMinimumWidth = aItemSize.Width();
    }
    return nMinimumWidth;
}




bool LayoutMenu::IsResizable (void)
{
    return true;
}




::Window* LayoutMenu::GetWindow (void)
{
    return this;
}




void LayoutMenu::Paint (const Rectangle& rRect)
{
    SetBackground (GetSettings().GetStyleSettings().GetWindowColor());

    ValueSet::Paint (rRect);

    SetBackground (Wallpaper());
}




void LayoutMenu::Resize (void)
{
    Size aWindowSize = GetOutputSizePixel();
    if (IsVisible() && aWindowSize.Width() > 0)
    {
        // Calculate the number of rows and columns.
        if (GetItemCount() > 0)
        {
            Image aImage = GetItemImage(GetItemId(0));
            Size aItemSize = CalcItemSizePixel (
                aImage.GetSizePixel());
            aItemSize.Width() += 8;
            aItemSize.Height() += 8;
            int nColumnCount = aWindowSize.Width() / aItemSize.Width();
            if (nColumnCount < 1)
                nColumnCount = 1;
            else if (nColumnCount > 4)
                nColumnCount = 4;

            int nRowCount = CalculateRowCount (aItemSize, nColumnCount);

            SetColCount (nColumnCount);
            SetLineCount (nRowCount);
        }
    }

    ValueSet::Resize ();
}




void LayoutMenu::MouseButtonDown (const MouseEvent& rEvent)
{
    if (rEvent.IsRight())
    {
        GrabFocus ();
        ReleaseMouse();
        if (GetShellManager() != NULL)
            GetShellManager()->MoveToTop (this);
        if (GetDispatcher() != NULL)
        {
            USHORT nIndex = GetItemId (rEvent.GetPosPixel());
            if (nIndex > 0)
            {
                SelectItem (nIndex);
                GetDispatcher()->ExecutePopup(
                    SdResId(RID_TASKPANE_LAYOUTMENU_POPUP));
            }
        }
    }
    else
        ValueSet::MouseButtonDown (rEvent);
}




void LayoutMenu::Execute (SfxRequest& rRequest)
{
    switch (rRequest.GetSlot())
    {
        case SID_TP_APPLY_TO_SELECTED_SLIDES:
            AssignLayoutToSelectedSlides (GetSelectedAutoLayout());
            rRequest.Done();
            break;

        case SID_INSERTPAGE:
            // Because we can not attach arguments to this slot we have to
            // forward this request manually to the main view shell can not
            // rely on the SFX to do it for us.
            InsertPageWithLayout (GetSelectedAutoLayout());
            break;
    }
}




void LayoutMenu::InsertPageWithLayout (AutoLayout aLayout)
{
    ViewShell* pViewShell = mrBase.GetMainViewShell();
    if (pViewShell == NULL)
    {
        // Because we process the SID_INSERTPAGE slot ourselves we have to
        // call ExecuteSlot() at the main view shell directly or otherwise
        // would end up being called ourselves.
        SfxRequest aRequest (CreateRequest (SID_INSERTPAGE, aLayout));
        pViewShell->ExecuteSlot (aRequest, BOOL(FALSE));
    }
}




void LayoutMenu::GetState (SfxItemSet& rItemSet)
{
}




int LayoutMenu::CalculateRowCount (const Size& rItemSize, int nColumnCount)
{
    int nRowCount = 0;

    if (GetItemCount() > 0)
    {
        nRowCount = GetOutputSizePixel().Height() / rItemSize.Height();
        if (nRowCount < 1)
            nRowCount = 1;
    }

    return nRowCount;
}




IMPL_LINK(LayoutMenu, ClickHandler, ValueSet*, pValueSet)
{
    AssignLayoutToSelectedSlides (GetSelectedAutoLayout());
    return 0;
}




/** The specified layout is assigned to the current page of the view shell
    in the center pane.
*/
void LayoutMenu::AssignLayoutToSelectedSlides (AutoLayout aLayout)
{
    ViewShell* pViewShell = mrBase.GetMainViewShell();
    if (pViewShell != NULL)
    {
        SfxRequest aRequest (CreateRequest(SID_MODIFYPAGE, aLayout));
        pViewShell->ExecuteSlot (aRequest, BOOL(FALSE));
    }
}




SfxRequest LayoutMenu::CreateRequest (
    USHORT nSlotId,
    AutoLayout aLayout)
{
    SfxRequest aRequest (mrBase.GetViewFrame(), nSlotId);

    do
    {
        SdrLayerAdmin& rLayerAdmin (mrBase.GetDocument()->GetLayerAdmin());
        BYTE aBackground (rLayerAdmin.GetLayerID(
            String(SdResId(STR_LAYER_BCKGRND)), FALSE));
        BYTE aBackgroundObject (rLayerAdmin.GetLayerID(
            String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE));
        ViewShell* pViewShell = mrBase.GetMainViewShell();
        if (pViewShell == NULL)
            break;
        SdPage* pPage = pViewShell->GetActualPage();
        if (pPage == NULL)
            break;

        SetOfByte aVisibleLayers (pPage->TRG_GetMasterPageVisibleLayers());

        aRequest.AppendItem(
            SfxStringItem (ID_VAL_PAGENAME, pPage->GetName()));
        aRequest.AppendItem(SfxUInt32Item (ID_VAL_WHATLAYOUT, aLayout));
        aRequest.AppendItem(
            SfxBoolItem(ID_VAL_ISPAGEBACK, aVisibleLayers.IsSet(aBackground)));
        aRequest.AppendItem(
            SfxBoolItem(
                ID_VAL_ISPAGEOBJ,
                aVisibleLayers.IsSet(aBackgroundObject)));
    }
    while (false);

    return aRequest;
}




IMPL_LINK(LayoutMenu, ViewShellChangeCallback, PaneManagerEvent*, pEvent)
{
    Fill();
    return 0;
}




void LayoutMenu::Fill (void)
{
    const bool bHighContrast = GetDisplayBackground().GetColor().IsDark() != 0;
    SvtLanguageOptions aLanguageOptions;
    sal_Bool bVertical = aLanguageOptions.IsVerticalTextEnabled();
    SdDrawDocument* pDocument = mrBase.GetDocument();
    sal_Bool bRightToLeft = (pDocument!=NULL
        && pDocument->GetDefaultWritingMode() == WritingMode_RL_TB);

    snewfoil_value_info* pInfo;
    switch (mrBase.GetPaneManager().GetViewShellType(
        PaneManager::PT_CENTER))
    {
        case ViewShell::ST_NOTES:
            pInfo = notes;
            break;

        case ViewShell::ST_HANDOUT:
            pInfo = handout;
            break;

        case ViewShell::ST_IMPRESS:
        case ViewShell::ST_SLIDE_SORTER:
            pInfo = standard;
            break;

        default:
            pInfo = NULL;
    }

    Clear();
    for (sal_uInt16 i=1; pInfo!=NULL&&pInfo->mnBmpResId!=0; i++,pInfo++)
    {
        if ((WritingMode_TB_RL != pInfo->meWritingMode) || bVertical)
        {
            Bitmap aBmp (SdResId (bHighContrast
                             ? pInfo->mnHCBmpResId
                             : pInfo->mnBmpResId));

            if (bRightToLeft && (WritingMode_TB_RL != pInfo->meWritingMode))
                aBmp.Mirror (BMP_MIRROR_HORZ);

            InsertItem (i, aBmp, String (SdResId (pInfo->mnStrResId)));
            SetItemData (i, new AutoLayout(pInfo->maAutoLayout));
        }
    }
}




void LayoutMenu::Clear (void)
{
    for (USHORT nId=1; nId<=GetItemCount(); nId++)
        delete static_cast<AutoLayout*>(GetItemData(nId));
    ValueSet::Clear();
}



void LayoutMenu::StartDrag (sal_Int8 nAction, const Point& rPosPixel)
{
}




sal_Int8 LayoutMenu::AcceptDrop (const AcceptDropEvent& rEvent)
{
    return 0;
}




sal_Int8 LayoutMenu::ExecuteDrop (const ExecuteDropEvent& rEvent)
{
    return 0;
}



} } // end of namespace ::sd::toolpanel
