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
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <vcl/settings.hxx>
#include <editeng/lineitem.hxx>
#include <memory>

namespace sc { namespace sidebar {

#define FRM_VALID_LEFT      0x01
#define FRM_VALID_RIGHT     0x02
#define FRM_VALID_TOP       0x04
#define FRM_VALID_BOTTOM    0x08
#define FRM_VALID_HINNER    0x10
#define FRM_VALID_VINNER    0x20
#define FRM_VALID_OUTER     0x0f
#define FRM_VALID_ALL       0xff

CellBorderStylePopup::CellBorderStylePopup(SfxDispatcher* pDispatcher)
    : FloatingWindow(SfxGetpApp()->GetTopWindow(), "FloatingBorderStyle", "modules/scalc/ui/floatingborderstyle.ui")
    , mpDispatcher(pDispatcher)
{
    get(maTBBorder1, "border1");
    get(maTBBorder2, "border2");
    get(maTBBorder3, "border3");
    Initialize();
}

CellBorderStylePopup::~CellBorderStylePopup()
{
    disposeOnce();
}

void CellBorderStylePopup::dispose()
{
    maTBBorder1.clear();
    maTBBorder2.clear();
    maTBBorder3.clear();
    FloatingWindow::dispose();
}

void CellBorderStylePopup::Initialize()
{
    maTBBorder1->SetSelectHdl ( LINK(this, CellBorderStylePopup, TB1SelectHdl) );

    maTBBorder2->SetLineCount(2);
    maTBBorder2->InsertBreak(4);
    maTBBorder2->SetSelectHdl ( LINK(this, CellBorderStylePopup, TB2SelectHdl) );

    maTBBorder3->SetSelectHdl ( LINK(this, CellBorderStylePopup, TB3SelectHdl) );
}

IMPL_LINK(CellBorderStylePopup, TB1SelectHdl, ToolBox*, pToolBox, void)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();
    SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
    SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
    editeng::SvxBorderLine theDefLine(nullptr, SvxBorderLineWidth::Thin);
    editeng::SvxBorderLine *pLeft = nullptr, *pRight = nullptr, *pTop = nullptr, *pBottom = nullptr;
    sal_uInt8 nValidFlags = 0;

    if (nId == maTBBorder1->GetItemId("none"))
    {
        nValidFlags |= FRM_VALID_ALL;
        SvxLineItem     aLineItem1( SID_ATTR_BORDER_DIAG_BLTR );
        SvxLineItem     aLineItem2( SID_ATTR_BORDER_DIAG_TLBR );
        aLineItem1.SetLine( nullptr );     //modify
        aLineItem2.SetLine( nullptr );     //modify
        mpDispatcher->ExecuteList(
            SID_ATTR_BORDER_DIAG_BLTR, SfxCallMode::RECORD, { &aLineItem1 });
        mpDispatcher->ExecuteList(
            SID_ATTR_BORDER_DIAG_TLBR, SfxCallMode::RECORD, { &aLineItem2 });
    }
    else if (nId == maTBBorder1->GetItemId("all"))
    {
        pLeft = pRight = pTop = pBottom = &theDefLine;
        aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::HORI );
        aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::VERT );
        nValidFlags |= FRM_VALID_ALL;
    }
    else if (nId == maTBBorder1->GetItemId("outside"))
    {
        pLeft = pRight = pTop = pBottom = &theDefLine;
        nValidFlags |= FRM_VALID_OUTER;
    }
    else if (nId == maTBBorder1->GetItemId("thickbox"))
    {
        theDefLine.SetWidth(SvxBorderLineWidth::Thick);
        pLeft = pRight = pTop = pBottom = &theDefLine;
        nValidFlags |= FRM_VALID_OUTER;
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

    mpDispatcher->ExecuteList(
        SID_ATTR_BORDER, SfxCallMode::RECORD, { &aBorderOuter, &aBorderInner });
    EndPopupMode();
}

IMPL_LINK(CellBorderStylePopup, TB2SelectHdl, ToolBox *, pToolBox, void)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();

    if (nId == pToolBox->GetItemId("diagup"))
    {
        editeng::SvxBorderLine aTmp( nullptr, SvxBorderLineWidth::Thin );
        SvxLineItem     aLineItem( SID_ATTR_BORDER_DIAG_BLTR );
        aLineItem.SetLine( &aTmp );
        mpDispatcher->ExecuteList(
            SID_ATTR_BORDER_DIAG_BLTR, SfxCallMode::RECORD, { &aLineItem });
    }
    else if (nId == pToolBox->GetItemId("diagdown"))
    {
        editeng::SvxBorderLine aTmp( nullptr, SvxBorderLineWidth::Thin );
        SvxLineItem     aLineItem( SID_ATTR_BORDER_DIAG_TLBR );
        aLineItem.SetLine( &aTmp );
        mpDispatcher->ExecuteList(
            SID_ATTR_BORDER_DIAG_TLBR, SfxCallMode::RECORD, { &aLineItem });
    }
    else
    {
        SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
        SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
        editeng::SvxBorderLine theDefLine(nullptr, SvxBorderLineWidth::Thin);
        editeng::SvxBorderLine       *pLeft = nullptr,
                            *pRight = nullptr,
                            *pTop = nullptr,
                            *pBottom = nullptr;
        sal_uInt8               nValidFlags = 0;
        if (nId == pToolBox->GetItemId("left"))
        {
            pLeft = &theDefLine;
            nValidFlags |= FRM_VALID_LEFT;
        }
        else if (nId == pToolBox->GetItemId("right"))
        {
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
        }
        else if (nId == pToolBox->GetItemId("top"))
        {
            pTop = &theDefLine;
            nValidFlags |= FRM_VALID_TOP;
        }
        else if (nId == pToolBox->GetItemId("bottom"))
        {
            pBottom = &theDefLine;
            nValidFlags |= FRM_VALID_BOTTOM;
        }
        else if (nId == pToolBox->GetItemId("topbottom"))
        {
            pTop =  pBottom = &theDefLine;
            nValidFlags |= FRM_VALID_BOTTOM|FRM_VALID_TOP;
        }
        else if (nId == pToolBox->GetItemId("leftright"))
        {
            pLeft = pRight = &theDefLine;
            nValidFlags |=  FRM_VALID_RIGHT|FRM_VALID_LEFT;
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

        mpDispatcher->ExecuteList(
            SID_ATTR_BORDER, SfxCallMode::RECORD, { &aBorderOuter, &aBorderInner});
    }

    EndPopupMode();
}

IMPL_LINK(CellBorderStylePopup, TB3SelectHdl, ToolBox *, pToolBox, void)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();

    SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
    SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
    std::unique_ptr<editeng::SvxBorderLine> pTop;
    std::unique_ptr<editeng::SvxBorderLine> pBottom;
    sal_uInt8               nValidFlags = 0;
    using namespace ::com::sun::star::table::BorderLineStyle;

    //FIXME: properly adapt to new line border model

    if (nId == maTBBorder3->GetItemId("thickbottom"))
    {
        pBottom.reset(new editeng::SvxBorderLine(nullptr, SvxBorderLineWidth::Thick));
        nValidFlags |= FRM_VALID_BOTTOM;
    }
    else if (nId == maTBBorder3->GetItemId("doublebottom"))
    {
        pBottom.reset(new editeng::SvxBorderLine(nullptr));
        pBottom->GuessLinesWidths(SvxBorderLineStyle::DOUBLE, SvxBorderLineWidth::Hairline,
                                  SvxBorderLineWidth::Hairline, SvxBorderLineWidth::Thin);
        nValidFlags |= FRM_VALID_BOTTOM;
    }
    else if (nId == maTBBorder3->GetItemId("topthickbottom"))
    {
        pBottom.reset(new editeng::SvxBorderLine(nullptr, SvxBorderLineWidth::Thick));
        pTop.reset(new editeng::SvxBorderLine(nullptr, SvxBorderLineWidth::Thin));
        nValidFlags |= FRM_VALID_BOTTOM|FRM_VALID_TOP;
    }
    else if (nId == maTBBorder3->GetItemId("topdoublebottom"))
    {
        pBottom.reset(new editeng::SvxBorderLine(nullptr));
        pBottom->GuessLinesWidths(SvxBorderLineStyle::DOUBLE, SvxBorderLineWidth::Hairline,
                                  SvxBorderLineWidth::Hairline, SvxBorderLineWidth::Thin);
        pTop.reset(new editeng::SvxBorderLine(nullptr, SvxBorderLineWidth::Thin));
        nValidFlags |= FRM_VALID_BOTTOM|FRM_VALID_TOP;
    }

    aBorderOuter.SetLine( pTop.get(), SvxBoxItemLine::TOP );
    aBorderOuter.SetLine( pBottom.get(), SvxBoxItemLine::BOTTOM );
    aBorderOuter.SetLine( nullptr, SvxBoxItemLine::LEFT );
    aBorderOuter.SetLine( nullptr, SvxBoxItemLine::RIGHT );

    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::TOP,       0 != (nValidFlags&FRM_VALID_TOP ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::BOTTOM,    0 != (nValidFlags&FRM_VALID_BOTTOM ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::LEFT,      0 != (nValidFlags&FRM_VALID_LEFT ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::RIGHT,     0 != (nValidFlags&FRM_VALID_RIGHT ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::HORI,      0 != (nValidFlags&FRM_VALID_HINNER ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::VERT,      0 != (nValidFlags&FRM_VALID_VINNER));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::DISTANCE );
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::DISABLE, false );

    mpDispatcher->ExecuteList(
        SID_ATTR_BORDER, SfxCallMode::RECORD, { &aBorderOuter, &aBorderInner });

    pTop.reset();
    pBottom.reset();

    EndPopupMode();
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
