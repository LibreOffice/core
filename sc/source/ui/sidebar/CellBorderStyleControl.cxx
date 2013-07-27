/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include "precompiled_sc.hxx"

#include <CellBorderStyleControl.hxx>
#include "sc.hrc"
#include "scresid.hxx"
#include <CellAppearancePropertyPanel.hrc>
#include <editeng/boxitem.hxx>
#include <editeng/bolnitem.hxx>
#include <CellAppearancePropertyPanel.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/svapp.hxx>

namespace sc { namespace sidebar {

#define FRM_VALID_LEFT      0x01
#define FRM_VALID_RIGHT     0x02
#define FRM_VALID_TOP       0x04
#define FRM_VALID_BOTTOM    0x08
#define FRM_VALID_HINNER    0x10
#define FRM_VALID_VINNER    0x20
#define FRM_VALID_OUTER     0x0f
#define FRM_VALID_ALL       0xff

CellBorderStyleControl::CellBorderStyleControl(Window* pParent, CellAppearancePropertyPanel& rPanel)
:   svx::sidebar::PopupControl(pParent, ScResId(RID_POPUPPANEL_APPEARANCE_CELL_BORDERSTYLE)),
    mrCellAppearancePropertyPanel(rPanel),
    maTBBorder1(this, ScResId(TB_BORDER1)),
    maTBBorder2(this, ScResId(TB_BORDER2)),
    maTBBorder3(this, ScResId(TB_BORDER3)),
    maFL1(this, ScResId(FL_1)),
    maFL2(this, ScResId(FL_2)),
    mpImageList(0),
    mpImageListH(0)
{
    Initialize();
    FreeResource();
}

CellBorderStyleControl::~CellBorderStyleControl(void)
{
    delete[] mpImageList;
    delete[] mpImageListH;
}

void CellBorderStyleControl::Initialize()
{
    mpImageList = new Image[16];
    mpImageList[0] = Image(ScResId(IMG_BORDER1));
    mpImageList[1] = Image(ScResId(IMG_BORDER2));
    mpImageList[2] = Image(ScResId(IMG_BORDER3));
    mpImageList[3] = Image(ScResId(IMG_BORDER4));
    mpImageList[4] = Image(ScResId(IMG_BORDER5));
    mpImageList[5] = Image(ScResId(IMG_BORDER6));
    mpImageList[6] = Image(ScResId(IMG_BORDER7));
    mpImageList[7] = Image(ScResId(IMG_BORDER8));
    mpImageList[8] = Image(ScResId(IMG_BORDER9));
    mpImageList[9] = Image(ScResId(IMG_BORDER10));
    mpImageList[10] = Image(ScResId(IMG_BORDER11));
    mpImageList[11] = Image(ScResId(IMG_BORDER12));
    mpImageList[12] = Image(ScResId(IMG_BORDER13));
    mpImageList[13] = Image(ScResId(IMG_BORDER14));
    mpImageList[14] = Image(ScResId(IMG_BORDER15));
    mpImageList[15] = Image(ScResId(IMG_BORDER16));

    //high contrast
    mpImageListH = new Image[16];
    mpImageListH[0] = Image(ScResId(IMG_BORDER1_H));
    mpImageListH[1] = Image(ScResId(IMG_BORDER2_H));
    mpImageListH[2] = Image(ScResId(IMG_BORDER3_H));
    mpImageListH[3] = Image(ScResId(IMG_BORDER4_H));
    mpImageListH[4] = Image(ScResId(IMG_BORDER5_H));
    mpImageListH[5] = Image(ScResId(IMG_BORDER6_H));
    mpImageListH[6] = Image(ScResId(IMG_BORDER7_H));
    mpImageListH[7] = Image(ScResId(IMG_BORDER8_H));
    mpImageListH[8] = Image(ScResId(IMG_BORDER9_H));
    mpImageListH[9] = Image(ScResId(IMG_BORDER10_H));
    mpImageListH[10] = Image(ScResId(IMG_BORDER11_H));
    mpImageListH[11] = Image(ScResId(IMG_BORDER12_H));
    mpImageListH[12] = Image(ScResId(IMG_BORDER13_H));
    mpImageListH[13] = Image(ScResId(IMG_BORDER14_H));
    mpImageListH[14] = Image(ScResId(IMG_BORDER15_H));
    mpImageListH[15] = Image(ScResId(IMG_BORDER16_H));

    maTBBorder1.SetItemImage(TBI_BORDER1_NONE, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[0] : mpImageList[0]);
    maTBBorder1.SetItemImage(TBI_BORDER1_ALL, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[1] : mpImageList[1]);
    maTBBorder1.SetItemImage(TBI_BORDER1_OUTER, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[2] : mpImageList[2]);
    maTBBorder1.SetItemImage(TBI_BORDER1_OUTERBOLD, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[3] : mpImageList[3]);
    maTBBorder1.SetBackground(Wallpaper());
    maTBBorder1.SetPaintTransparent(true);
    Size aTbxSize( maTBBorder1.CalcWindowSizePixel() );
    maTBBorder1.SetOutputSizePixel( aTbxSize );
    Link aLink  = LINK(this, CellBorderStyleControl, TB1SelectHdl);
    maTBBorder1.SetSelectHdl ( aLink );

    maTBBorder2.SetLineCount(2);
    maTBBorder2.InsertItem(TBI_BORDER2_LEFT, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[4] : mpImageList[4]);
    maTBBorder2.InsertItem(TBI_BORDER2_RIGHT, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[5] : mpImageList[5]);
    maTBBorder2.InsertItem(TBI_BORDER2_TOP, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[6] : mpImageList[6]);
    maTBBorder2.InsertItem(TBI_BORDER2_BOT, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[7] : mpImageList[7]);
    maTBBorder2.InsertBreak();
    maTBBorder2.InsertItem(TBI_BORDER2_BLTR, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[8] : mpImageList[8]);
    maTBBorder2.InsertItem(TBI_BORDER2_TLBR, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[9] : mpImageList[9]);
    maTBBorder2.InsertItem(TBI_BORDER2_TOPBOT, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[10] : mpImageList[10]);
    maTBBorder2.InsertItem(TBI_BORDER2_LEFTRIGHT, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[11] : mpImageList[11]);
    maTBBorder2.SetBackground(Wallpaper());
    maTBBorder2.SetPaintTransparent(true);
    aTbxSize = maTBBorder2.CalcWindowSizePixel() ;
    maTBBorder2.SetOutputSizePixel( aTbxSize );
    maTBBorder2.SetHelpId(TBI_BORDER2_LEFT, HID_PROPERTYPANEL_SC_BORDER_TBI_LEFT);
    maTBBorder2.SetHelpId(TBI_BORDER2_RIGHT, HID_PROPERTYPANEL_SC_BORDER_TBI_RIGHT);
    maTBBorder2.SetHelpId(TBI_BORDER2_TOP, HID_PROPERTYPANEL_SC_BORDER_TBI_TOP);
    maTBBorder2.SetHelpId(TBI_BORDER2_BOT, HID_PROPERTYPANEL_SC_BORDER_TBI_BOT);
    maTBBorder2.SetHelpId(TBI_BORDER2_BLTR, HID_PROPERTYPANEL_SC_BORDER_TBI_BLTR);
    maTBBorder2.SetHelpId(TBI_BORDER2_TLBR, HID_PROPERTYPANEL_SC_BORDER_TBI_TLBR);
    maTBBorder2.SetHelpId(TBI_BORDER2_TOPBOT, HID_PROPERTYPANEL_SC_BORDER_TBI_TOPBOT);
    maTBBorder2.SetHelpId(TBI_BORDER2_LEFTRIGHT, HID_PROPERTYPANEL_SC_BORDER_TBI_LEFTRIGHT);
    maTBBorder2.SetItemText(TBI_BORDER2_LEFT, ScResId(STR_BORDER_1));
    maTBBorder2.SetItemText(TBI_BORDER2_RIGHT, ScResId(STR_BORDER_2));
    maTBBorder2.SetItemText(TBI_BORDER2_TOP, ScResId(STR_BORDER_3));
    maTBBorder2.SetItemText(TBI_BORDER2_BOT, ScResId(STR_BORDER_4));
    maTBBorder2.SetItemText(TBI_BORDER2_BLTR, ScResId(STR_BORDER_5));
    maTBBorder2.SetItemText(TBI_BORDER2_TLBR, ScResId(STR_BORDER_6));
    maTBBorder2.SetItemText(TBI_BORDER2_TOPBOT, ScResId(STR_BORDER_7));
    maTBBorder2.SetItemText(TBI_BORDER2_LEFTRIGHT, ScResId(STR_BORDER_8));
    aLink  = LINK(this, CellBorderStyleControl, TB2SelectHdl);
    maTBBorder2.SetSelectHdl ( aLink );

    maTBBorder3.SetItemImage(TBI_BORDER3_S1, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[12] : mpImageList[12]);
    maTBBorder3.SetItemImage(TBI_BORDER3_S2, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[13] : mpImageList[13]);
    maTBBorder3.SetItemImage(TBI_BORDER3_S3, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[14] : mpImageList[14]);
    maTBBorder3.SetItemImage(TBI_BORDER3_S4, GetDisplayBackground().GetColor().IsDark() ? mpImageListH[15] : mpImageList[15]);
    maTBBorder3.SetBackground(Wallpaper());
    maTBBorder3.SetPaintTransparent(true);
    aTbxSize = maTBBorder3.CalcWindowSizePixel() ;
    maTBBorder3.SetOutputSizePixel( aTbxSize );
    aLink  = LINK(this, CellBorderStyleControl, TB3SelectHdl);
    maTBBorder3.SetSelectHdl ( aLink );
}

IMPL_LINK(CellBorderStyleControl, TB1SelectHdl, ToolBox*, pToolBox)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();
    ::Color             aColBlack( COL_BLACK );
    SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
    SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
    SvxBorderLine       theDefLine(NULL,1, 0, 0);
    SvxBorderLine       *pLeft = 0, *pRight = 0, *pTop = 0, *pBottom = 0;
    sal_uInt8 nValidFlags = 0;
    switch ( nId )
    {
    case TBI_BORDER1_NONE:
        {
         nValidFlags |= FRM_VALID_ALL;
        SvxLineItem     aLineItem1( SID_ATTR_BORDER_DIAG_BLTR );
        SvxLineItem     aLineItem2( SID_ATTR_BORDER_DIAG_TLBR );
        aLineItem1.SetLine( NULL );     //modify
        aLineItem2.SetLine( NULL );     //modify
        mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER_DIAG_BLTR, SFX_CALLMODE_RECORD, &aLineItem1, 0L);
        mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER_DIAG_TLBR, SFX_CALLMODE_RECORD, &aLineItem2, 0L);
        }
        break;
    case TBI_BORDER1_ALL:
        pLeft = pRight = pTop = pBottom = &theDefLine;
        aBorderInner.SetLine( &theDefLine, BOXINFO_LINE_HORI );
        aBorderInner.SetLine( &theDefLine, BOXINFO_LINE_VERT );
        nValidFlags |= FRM_VALID_ALL;
        break;
    case TBI_BORDER1_OUTER:
        pLeft = pRight = pTop = pBottom = &theDefLine;
        nValidFlags |= FRM_VALID_OUTER;
        break;
    case TBI_BORDER1_OUTERBOLD:
        theDefLine.SetOutWidth(DEF_LINE_WIDTH_2);
        pLeft = pRight = pTop = pBottom = &theDefLine;
        nValidFlags |= FRM_VALID_OUTER;
        break;
    }

    aBorderOuter.SetLine( pLeft, BOX_LINE_LEFT );
    aBorderOuter.SetLine( pRight, BOX_LINE_RIGHT );
    aBorderOuter.SetLine( pTop, BOX_LINE_TOP );
    aBorderOuter.SetLine( pBottom, BOX_LINE_BOTTOM );

    aBorderInner.SetValid( VALID_TOP,       0 != (nValidFlags&FRM_VALID_TOP ));
    aBorderInner.SetValid( VALID_BOTTOM,    0 != (nValidFlags&FRM_VALID_BOTTOM ));
    aBorderInner.SetValid( VALID_LEFT,      0 != (nValidFlags&FRM_VALID_LEFT));
    aBorderInner.SetValid( VALID_RIGHT,     0 != (nValidFlags&FRM_VALID_RIGHT ));
    aBorderInner.SetValid( VALID_HORI,      0 != (nValidFlags&FRM_VALID_HINNER ));
    aBorderInner.SetValid( VALID_VERT,      0 != (nValidFlags&FRM_VALID_VINNER));
    aBorderInner.SetValid( VALID_DISTANCE, true );
    aBorderInner.SetValid( VALID_DISABLE, false );

    mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER, SFX_CALLMODE_RECORD, &aBorderOuter, &aBorderInner, 0L);
    mrCellAppearancePropertyPanel.EndCellBorderStylePopupMode();
    return 0;
}

IMPL_LINK(CellBorderStyleControl, TB2SelectHdl, ToolBox *, pToolBox)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();

    if( nId != TBI_BORDER2_BLTR && nId != TBI_BORDER2_TLBR )
    {
        ::Color             aColBlack( COL_BLACK );
        SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
        SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
        SvxBorderLine       theDefLine(NULL, 1, 0, 0);
        SvxBorderLine       *pLeft = 0,
                            *pRight = 0,
                            *pTop = 0,
                            *pBottom = 0;
        sal_uInt8               nValidFlags = 0;
        switch ( nId )
        {
        case TBI_BORDER2_LEFT:
            pLeft = &theDefLine;
            nValidFlags |= FRM_VALID_LEFT;
            break;
        case TBI_BORDER2_RIGHT:
            //
            if(!Application::GetSettings().GetLayoutRTL())
            {
                pRight = &theDefLine;
                nValidFlags |= FRM_VALID_RIGHT;
            }
            else
            {
                pLeft = &theDefLine;
                nValidFlags |= FRM_VALID_LEFT;
            }
            break;
        case TBI_BORDER2_TOP:
            pTop = &theDefLine;
            nValidFlags |= FRM_VALID_TOP;
            break;
        case TBI_BORDER2_BOT:
            pBottom = &theDefLine;
            nValidFlags |= FRM_VALID_BOTTOM;
            break;
        case TBI_BORDER2_TOPBOT:
            pTop =  pBottom = &theDefLine;
            nValidFlags |= FRM_VALID_BOTTOM|FRM_VALID_TOP;
            break;
        case TBI_BORDER2_LEFTRIGHT:
            pLeft = pRight = &theDefLine;
            nValidFlags |=  FRM_VALID_RIGHT|FRM_VALID_LEFT;
            break;
        }
        aBorderOuter.SetLine( pLeft, BOX_LINE_LEFT );
        aBorderOuter.SetLine( pRight, BOX_LINE_RIGHT );
        aBorderOuter.SetLine( pTop, BOX_LINE_TOP );
        aBorderOuter.SetLine( pBottom, BOX_LINE_BOTTOM );

        aBorderInner.SetValid( VALID_TOP,       0 != (nValidFlags&FRM_VALID_TOP ));
        aBorderInner.SetValid( VALID_BOTTOM,    0 != (nValidFlags&FRM_VALID_BOTTOM ));
        aBorderInner.SetValid( VALID_LEFT,      0 != (nValidFlags&FRM_VALID_LEFT));
        aBorderInner.SetValid( VALID_RIGHT,     0 != (nValidFlags&FRM_VALID_RIGHT ));
        aBorderInner.SetValid( VALID_HORI,      0 != (nValidFlags&FRM_VALID_HINNER ));
        aBorderInner.SetValid( VALID_VERT,      0 != (nValidFlags&FRM_VALID_VINNER));
        aBorderInner.SetValid( VALID_DISTANCE, true );
        aBorderInner.SetValid( VALID_DISABLE, false );

        mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER, SFX_CALLMODE_RECORD, &aBorderOuter, &aBorderInner, 0L);
    }
    else if(nId == TBI_BORDER2_BLTR)
    {
        SvxBorderLine aTmp( NULL, 1, 0, 0 );
        SvxLineItem     aLineItem( SID_ATTR_BORDER_DIAG_BLTR );
        aLineItem.SetLine( &aTmp );
        mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER_DIAG_BLTR, SFX_CALLMODE_RECORD, &aLineItem, 0L);
    }
    else if(nId == TBI_BORDER2_TLBR)
    {
        SvxBorderLine aTmp( NULL, 1, 0, 0 );
        SvxLineItem     aLineItem( SID_ATTR_BORDER_DIAG_TLBR );
        aLineItem.SetLine( &aTmp );
        mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER_DIAG_TLBR, SFX_CALLMODE_RECORD, &aLineItem, 0L);
    }

    mrCellAppearancePropertyPanel.EndCellBorderStylePopupMode();
    return 0;
}

IMPL_LINK(CellBorderStyleControl, TB3SelectHdl, ToolBox *, pToolBox)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();

    ::Color             aColBlack( COL_BLACK );
    SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
    SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
    SvxBorderLine       *pTop = 0 ,
                        *pBottom = 0 ;
    sal_uInt8               nValidFlags = 0;

    switch ( nId )
    {
    case TBI_BORDER3_S1:
        pBottom = new SvxBorderLine(NULL, DEF_LINE_WIDTH_2, 0, 0);
        nValidFlags |= FRM_VALID_BOTTOM;
        break;
    case TBI_BORDER3_S2:
        pBottom = new SvxBorderLine(NULL, DEF_DOUBLE_LINE0_OUT, DEF_DOUBLE_LINE0_IN, DEF_DOUBLE_LINE0_DIST);
        nValidFlags |= FRM_VALID_BOTTOM;
        break;
    case TBI_BORDER3_S3:
        pBottom = new SvxBorderLine(NULL, DEF_LINE_WIDTH_2, 0, 0);
        pTop = new SvxBorderLine(NULL, 1, 0, 0);
        nValidFlags |= FRM_VALID_BOTTOM|FRM_VALID_TOP;
        break;
    case TBI_BORDER3_S4:
        pBottom = new SvxBorderLine(NULL, DEF_DOUBLE_LINE0_OUT, DEF_DOUBLE_LINE0_IN, DEF_DOUBLE_LINE0_DIST);
        pTop = new SvxBorderLine(NULL, 1, 0, 0);
        nValidFlags |= FRM_VALID_BOTTOM|FRM_VALID_TOP;
        break;
    }

    aBorderOuter.SetLine( pTop, BOX_LINE_TOP );
    aBorderOuter.SetLine( pBottom, BOX_LINE_BOTTOM );
    aBorderOuter.SetLine( NULL, BOX_LINE_LEFT );
    aBorderOuter.SetLine( NULL, BOX_LINE_RIGHT );

    aBorderInner.SetValid( VALID_TOP,       0 != (nValidFlags&FRM_VALID_TOP ));
    aBorderInner.SetValid( VALID_BOTTOM,    0 != (nValidFlags&FRM_VALID_BOTTOM ));
    aBorderInner.SetValid( VALID_LEFT,      0 != (nValidFlags&FRM_VALID_LEFT ));
    aBorderInner.SetValid( VALID_RIGHT,     0 != (nValidFlags&FRM_VALID_RIGHT ));
    aBorderInner.SetValid( VALID_HORI,      0 != (nValidFlags&FRM_VALID_HINNER ));
    aBorderInner.SetValid( VALID_VERT,      0 != (nValidFlags&FRM_VALID_VINNER));
    aBorderInner.SetValid( VALID_DISTANCE, true );
    aBorderInner.SetValid( VALID_DISABLE, false );

    mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER, SFX_CALLMODE_RECORD, &aBorderOuter, &aBorderInner, 0L);

    delete pTop;
    delete pBottom;

    mrCellAppearancePropertyPanel.EndCellBorderStylePopupMode();
    return 0;
}

} } // end of namespace svx::sidebar

// eof
