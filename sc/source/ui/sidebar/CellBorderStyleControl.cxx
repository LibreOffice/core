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

#include <CellBorderStyleControl.hxx>
#include "sc.hrc"
#include "scresid.hxx"
#include <CellAppearancePropertyPanel.hrc>
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>
#include <CellAppearancePropertyPanel.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/svapp.hxx>
#include <editeng/lineitem.hxx>

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
    mpImageList(0)
{
    Initialize();
    FreeResource();
}

CellBorderStyleControl::~CellBorderStyleControl(void)
{
    delete[] mpImageList;
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

    maTBBorder1.SetItemImage(TBI_BORDER1_NONE, mpImageList[0]);
    maTBBorder1.SetItemImage(TBI_BORDER1_ALL, mpImageList[1]);
    maTBBorder1.SetItemImage(TBI_BORDER1_OUTER, mpImageList[2]);
    maTBBorder1.SetItemImage(TBI_BORDER1_OUTERBOLD, mpImageList[3]);
    maTBBorder1.SetBackground(Wallpaper());
    maTBBorder1.SetPaintTransparent(true);
    Size aTbxSize( maTBBorder1.CalcWindowSizePixel() );
    maTBBorder1.SetOutputSizePixel( aTbxSize );
    Link aLink  = LINK(this, CellBorderStyleControl, TB1SelectHdl);
    maTBBorder1.SetSelectHdl ( aLink );

    maTBBorder2.SetLineCount(2);
    maTBBorder2.InsertItem(TBI_BORDER2_LEFT, mpImageList[4]);
    maTBBorder2.InsertItem(TBI_BORDER2_RIGHT, mpImageList[5]);
    maTBBorder2.InsertItem(TBI_BORDER2_TOP, mpImageList[6]);
    maTBBorder2.InsertItem(TBI_BORDER2_BOT, mpImageList[7]);
    maTBBorder2.InsertBreak();
    maTBBorder2.InsertItem(TBI_BORDER2_BLTR, mpImageList[8]);
    maTBBorder2.InsertItem(TBI_BORDER2_TLBR, mpImageList[9]);
    maTBBorder2.InsertItem(TBI_BORDER2_TOPBOT, mpImageList[10]);
    maTBBorder2.InsertItem(TBI_BORDER2_LEFTRIGHT, mpImageList[11]);
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

    maTBBorder3.SetItemImage(TBI_BORDER3_S1, mpImageList[12]);
    maTBBorder3.SetItemImage(TBI_BORDER3_S2, mpImageList[13]);
    maTBBorder3.SetItemImage(TBI_BORDER3_S3, mpImageList[14]);
    maTBBorder3.SetItemImage(TBI_BORDER3_S4, mpImageList[15]);
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
    SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
    SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
    editeng::SvxBorderLine theDefLine(NULL, 1);
    editeng::SvxBorderLine *pLeft = 0, *pRight = 0, *pTop = 0, *pBottom = 0;
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
        theDefLine.SetWidth(DEF_LINE_WIDTH_2);
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
        SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
        SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
        editeng::SvxBorderLine theDefLine(NULL, 1);
        editeng::SvxBorderLine       *pLeft = 0,
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
        editeng::SvxBorderLine aTmp( NULL, 1 );
        SvxLineItem     aLineItem( SID_ATTR_BORDER_DIAG_BLTR );
        aLineItem.SetLine( &aTmp );
        mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER_DIAG_BLTR, SFX_CALLMODE_RECORD, &aLineItem, 0L);
    }
    else if(nId == TBI_BORDER2_TLBR)
    {
        editeng::SvxBorderLine aTmp( NULL, 1 );
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

    SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
    SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
    editeng::SvxBorderLine *pTop = 0 ,
                        *pBottom = 0 ;
    sal_uInt8               nValidFlags = 0;
    using namespace ::com::sun::star::table::BorderLineStyle;

    //FIXME: properly adapt to new line border model

    switch ( nId )
    {
    case TBI_BORDER3_S1:
        pBottom = new editeng::SvxBorderLine(NULL, DEF_LINE_WIDTH_2 );
        nValidFlags |= FRM_VALID_BOTTOM;
        break;
    case TBI_BORDER3_S2:
        pBottom = new editeng::SvxBorderLine(NULL);
        pBottom->GuessLinesWidths(DOUBLE, DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_1);
        nValidFlags |= FRM_VALID_BOTTOM;
        break;
    case TBI_BORDER3_S3:
        pBottom = new editeng::SvxBorderLine(NULL, DEF_LINE_WIDTH_2 );
        pTop = new editeng::SvxBorderLine(NULL, 1);
        nValidFlags |= FRM_VALID_BOTTOM|FRM_VALID_TOP;
        break;
    case TBI_BORDER3_S4:
        pBottom = new editeng::SvxBorderLine(NULL);
        pBottom->GuessLinesWidths(DOUBLE, DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_1);
        pTop = new editeng::SvxBorderLine(NULL, 1);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
