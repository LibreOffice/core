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

#include "CellBorderStyleControl.hxx"
#include "sc.hrc"
#include "scresid.hxx"
#include <CellAppearancePropertyPanel.hrc>
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>
#include "CellAppearancePropertyPanel.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <editeng/lineitem.hxx>
#include <boost/scoped_ptr.hpp>

namespace sc { namespace sidebar {

#define FRM_VALID_LEFT      0x01
#define FRM_VALID_RIGHT     0x02
#define FRM_VALID_TOP       0x04
#define FRM_VALID_BOTTOM    0x08
#define FRM_VALID_HINNER    0x10
#define FRM_VALID_VINNER    0x20
#define FRM_VALID_OUTER     0x0f
#define FRM_VALID_ALL       0xff

CellBorderStyleControl::CellBorderStyleControl(vcl::Window* pParent, CellAppearancePropertyPanel& rPanel)
:   svx::sidebar::PopupControl(pParent, ScResId(RID_POPUPPANEL_APPEARANCE_CELL_BORDERSTYLE)),
    mrCellAppearancePropertyPanel(rPanel),
    maTBBorder1(VclPtr<ToolBox>::Create(this, ScResId(TB_BORDER1))),
    maTBBorder2(VclPtr<ToolBox>::Create(this, ScResId(TB_BORDER2))),
    maTBBorder3(VclPtr<ToolBox>::Create(this, ScResId(TB_BORDER3))),
    maFL1(VclPtr<FixedLine>::Create(this, ScResId(FL_1))),
    maFL2(VclPtr<FixedLine>::Create(this, ScResId(FL_2))),
    mpImageList(0)
{
    Initialize();
    FreeResource();
}

CellBorderStyleControl::~CellBorderStyleControl()
{
    disposeOnce();
}

void CellBorderStyleControl::dispose()
{
    delete[] mpImageList;
    maTBBorder1.disposeAndClear();
    maTBBorder2.disposeAndClear();
    maTBBorder3.disposeAndClear();
    maFL1.disposeAndClear();
    maFL2.disposeAndClear();
    svx::sidebar::PopupControl::dispose();
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

    maTBBorder1->SetItemImage(TBI_BORDER1_NONE, mpImageList[0]);
    maTBBorder1->SetItemImage(TBI_BORDER1_ALL, mpImageList[1]);
    maTBBorder1->SetItemImage(TBI_BORDER1_OUTER, mpImageList[2]);
    maTBBorder1->SetItemImage(TBI_BORDER1_OUTERBOLD, mpImageList[3]);
    maTBBorder1->SetBackground(Wallpaper());
    maTBBorder1->SetPaintTransparent(true);
    Size aTbxSize( maTBBorder1->CalcWindowSizePixel() );
    maTBBorder1->SetOutputSizePixel( aTbxSize );
    maTBBorder1->SetSelectHdl ( LINK(this, CellBorderStyleControl, TB1SelectHdl) );

    maTBBorder2->SetLineCount(2);
    maTBBorder2->InsertItem(TBI_BORDER2_LEFT, mpImageList[4]);
    maTBBorder2->InsertItem(TBI_BORDER2_RIGHT, mpImageList[5]);
    maTBBorder2->InsertItem(TBI_BORDER2_TOP, mpImageList[6]);
    maTBBorder2->InsertItem(TBI_BORDER2_BOT, mpImageList[7]);
    maTBBorder2->InsertBreak();
    maTBBorder2->InsertItem(TBI_BORDER2_BLTR, mpImageList[8]);
    maTBBorder2->InsertItem(TBI_BORDER2_TLBR, mpImageList[9]);
    maTBBorder2->InsertItem(TBI_BORDER2_TOPBOT, mpImageList[10]);
    maTBBorder2->InsertItem(TBI_BORDER2_LEFTRIGHT, mpImageList[11]);
    maTBBorder2->SetBackground(Wallpaper());
    maTBBorder2->SetPaintTransparent(true);
    aTbxSize = maTBBorder2->CalcWindowSizePixel() ;
    maTBBorder2->SetOutputSizePixel( aTbxSize );
    maTBBorder2->SetHelpId(TBI_BORDER2_LEFT, HID_PROPERTYPANEL_SC_BORDER_TBI_LEFT);
    maTBBorder2->SetHelpId(TBI_BORDER2_RIGHT, HID_PROPERTYPANEL_SC_BORDER_TBI_RIGHT);
    maTBBorder2->SetHelpId(TBI_BORDER2_TOP, HID_PROPERTYPANEL_SC_BORDER_TBI_TOP);
    maTBBorder2->SetHelpId(TBI_BORDER2_BOT, HID_PROPERTYPANEL_SC_BORDER_TBI_BOT);
    maTBBorder2->SetHelpId(TBI_BORDER2_BLTR, HID_PROPERTYPANEL_SC_BORDER_TBI_BLTR);
    maTBBorder2->SetHelpId(TBI_BORDER2_TLBR, HID_PROPERTYPANEL_SC_BORDER_TBI_TLBR);
    maTBBorder2->SetHelpId(TBI_BORDER2_TOPBOT, HID_PROPERTYPANEL_SC_BORDER_TBI_TOPBOT);
    maTBBorder2->SetHelpId(TBI_BORDER2_LEFTRIGHT, HID_PROPERTYPANEL_SC_BORDER_TBI_LEFTRIGHT);
    maTBBorder2->SetItemText(TBI_BORDER2_LEFT, ScResId(STR_BORDER_1));
    maTBBorder2->SetItemText(TBI_BORDER2_RIGHT, ScResId(STR_BORDER_2));
    maTBBorder2->SetItemText(TBI_BORDER2_TOP, ScResId(STR_BORDER_3));
    maTBBorder2->SetItemText(TBI_BORDER2_BOT, ScResId(STR_BORDER_4));
    maTBBorder2->SetItemText(TBI_BORDER2_BLTR, ScResId(STR_BORDER_5));
    maTBBorder2->SetItemText(TBI_BORDER2_TLBR, ScResId(STR_BORDER_6));
    maTBBorder2->SetItemText(TBI_BORDER2_TOPBOT, ScResId(STR_BORDER_7));
    maTBBorder2->SetItemText(TBI_BORDER2_LEFTRIGHT, ScResId(STR_BORDER_8));
    maTBBorder2->SetSelectHdl ( LINK(this, CellBorderStyleControl, TB2SelectHdl) );

    maTBBorder3->SetItemImage(TBI_BORDER3_S1, mpImageList[12]);
    maTBBorder3->SetItemImage(TBI_BORDER3_S2, mpImageList[13]);
    maTBBorder3->SetItemImage(TBI_BORDER3_S3, mpImageList[14]);
    maTBBorder3->SetItemImage(TBI_BORDER3_S4, mpImageList[15]);
    maTBBorder3->SetBackground(Wallpaper());
    maTBBorder3->SetPaintTransparent(true);
    aTbxSize = maTBBorder3->CalcWindowSizePixel() ;
    maTBBorder3->SetOutputSizePixel( aTbxSize );
    maTBBorder3->SetSelectHdl ( LINK(this, CellBorderStyleControl, TB3SelectHdl) );
}

IMPL_LINK_TYPED(CellBorderStyleControl, TB1SelectHdl, ToolBox*, pToolBox, void)
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
        mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER_DIAG_BLTR, SfxCallMode::RECORD, &aLineItem1, 0L);
        mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER_DIAG_TLBR, SfxCallMode::RECORD, &aLineItem2, 0L);
        }
        break;
    case TBI_BORDER1_ALL:
        pLeft = pRight = pTop = pBottom = &theDefLine;
        aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::HORI );
        aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::VERT );
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

    aBorderOuter.SetLine( pLeft, SvxBoxItemLine::LEFT );
    aBorderOuter.SetLine( pRight, SvxBoxItemLine::RIGHT );
    aBorderOuter.SetLine( pTop, SvxBoxItemLine::TOP );
    aBorderOuter.SetLine( pBottom, SvxBoxItemLine::BOTTOM );

    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::TOP,       0 != (nValidFlags&FRM_VALID_TOP ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::BOTTOM,    0 != (nValidFlags&FRM_VALID_BOTTOM ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::LEFT,      0 != (nValidFlags&FRM_VALID_LEFT));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::RIGHT,     0 != (nValidFlags&FRM_VALID_RIGHT ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::HORI,      0 != (nValidFlags&FRM_VALID_HINNER ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::VERT,      0 != (nValidFlags&FRM_VALID_VINNER));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::DISTANCE );
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::DISABLE, false );

    mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER, SfxCallMode::RECORD, &aBorderOuter, &aBorderInner, 0L);
    mrCellAppearancePropertyPanel.EndCellBorderStylePopupMode();
}

IMPL_LINK_TYPED(CellBorderStyleControl, TB2SelectHdl, ToolBox *, pToolBox, void)
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

            if(!AllSettings::GetLayoutRTL())
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
        aBorderOuter.SetLine( pLeft, SvxBoxItemLine::LEFT );
        aBorderOuter.SetLine( pRight, SvxBoxItemLine::RIGHT );
        aBorderOuter.SetLine( pTop, SvxBoxItemLine::TOP );
        aBorderOuter.SetLine( pBottom, SvxBoxItemLine::BOTTOM );

        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::TOP,       0 != (nValidFlags&FRM_VALID_TOP ));
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::BOTTOM,    0 != (nValidFlags&FRM_VALID_BOTTOM ));
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::LEFT,      0 != (nValidFlags&FRM_VALID_LEFT));
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::RIGHT,     0 != (nValidFlags&FRM_VALID_RIGHT ));
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::HORI,      0 != (nValidFlags&FRM_VALID_HINNER ));
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::VERT,      0 != (nValidFlags&FRM_VALID_VINNER));
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::DISTANCE );
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::DISABLE, false );

        mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER, SfxCallMode::RECORD, &aBorderOuter, &aBorderInner, 0L);
    }
    else if(nId == TBI_BORDER2_BLTR)
    {
        editeng::SvxBorderLine aTmp( NULL, 1 );
        SvxLineItem     aLineItem( SID_ATTR_BORDER_DIAG_BLTR );
        aLineItem.SetLine( &aTmp );
        mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER_DIAG_BLTR, SfxCallMode::RECORD, &aLineItem, 0L);
    }
    else if(nId == TBI_BORDER2_TLBR)
    {
        editeng::SvxBorderLine aTmp( NULL, 1 );
        SvxLineItem     aLineItem( SID_ATTR_BORDER_DIAG_TLBR );
        aLineItem.SetLine( &aTmp );
        mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER_DIAG_TLBR, SfxCallMode::RECORD, &aLineItem, 0L);
    }

    mrCellAppearancePropertyPanel.EndCellBorderStylePopupMode();
}

IMPL_LINK_TYPED(CellBorderStyleControl, TB3SelectHdl, ToolBox *, pToolBox, void)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();

    SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
    SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
    boost::scoped_ptr<editeng::SvxBorderLine> pTop;
    boost::scoped_ptr<editeng::SvxBorderLine> pBottom;
    sal_uInt8               nValidFlags = 0;
    using namespace ::com::sun::star::table::BorderLineStyle;

    //FIXME: properly adapt to new line border model

    switch ( nId )
    {
    case TBI_BORDER3_S1:
        pBottom.reset(new editeng::SvxBorderLine(NULL, DEF_LINE_WIDTH_2 ));
        nValidFlags |= FRM_VALID_BOTTOM;
        break;
    case TBI_BORDER3_S2:
        pBottom.reset(new editeng::SvxBorderLine(NULL));
        pBottom->GuessLinesWidths(DOUBLE, DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_1);
        nValidFlags |= FRM_VALID_BOTTOM;
        break;
    case TBI_BORDER3_S3:
        pBottom.reset(new editeng::SvxBorderLine(NULL, DEF_LINE_WIDTH_2 ));
        pTop.reset(new editeng::SvxBorderLine(NULL, 1));
        nValidFlags |= FRM_VALID_BOTTOM|FRM_VALID_TOP;
        break;
    case TBI_BORDER3_S4:
        pBottom.reset(new editeng::SvxBorderLine(NULL));
        pBottom->GuessLinesWidths(DOUBLE, DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_1);
        pTop.reset(new editeng::SvxBorderLine(NULL, 1));
        nValidFlags |= FRM_VALID_BOTTOM|FRM_VALID_TOP;
        break;
    }

    aBorderOuter.SetLine( pTop.get(), SvxBoxItemLine::TOP );
    aBorderOuter.SetLine( pBottom.get(), SvxBoxItemLine::BOTTOM );
    aBorderOuter.SetLine( NULL, SvxBoxItemLine::LEFT );
    aBorderOuter.SetLine( NULL, SvxBoxItemLine::RIGHT );

    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::TOP,       0 != (nValidFlags&FRM_VALID_TOP ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::BOTTOM,    0 != (nValidFlags&FRM_VALID_BOTTOM ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::LEFT,      0 != (nValidFlags&FRM_VALID_LEFT ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::RIGHT,     0 != (nValidFlags&FRM_VALID_RIGHT ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::HORI,      0 != (nValidFlags&FRM_VALID_HINNER ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::VERT,      0 != (nValidFlags&FRM_VALID_VINNER));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::DISTANCE );
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::DISABLE, false );

    mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_ATTR_BORDER, SfxCallMode::RECORD, &aBorderOuter, &aBorderInner, 0L);

    pTop.reset();
    pBottom.reset();

    mrCellAppearancePropertyPanel.EndCellBorderStylePopupMode();
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
