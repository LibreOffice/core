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

#include "CellLineStyleControl.hxx"
#include "CellLineStyleValueSet.hxx"
#include <vcl/i18nhelp.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <editeng/borderline.hxx>
#include <editeng/lineitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>

namespace sc::sidebar {

CellLineStylePopup::CellLineStylePopup(weld::Toolbar* pParent, const OString& rId, SfxDispatcher* pDispatcher)
    : WeldToolbarPopup(nullptr, pParent, "modules/scalc/ui/floatinglinestyle.ui", "FloatingLineStyle")
    , maToolButton(pParent, rId)
    , mpDispatcher(pDispatcher)
    , mxCellLineStyleValueSet(new sc::sidebar::CellLineStyleValueSet)
    , mxCellLineStyleValueSetWin(new weld::CustomWeld(*m_xBuilder, "valueset", *mxCellLineStyleValueSet))
    , mxPushButtonMoreOptions(m_xBuilder->weld_button("more"))
{
    Initialize();
}

CellLineStylePopup::~CellLineStylePopup()
{
}

void CellLineStylePopup::Initialize()
{
    mxPushButtonMoreOptions->connect_clicked(LINK(this, CellLineStylePopup, PBClickHdl));

    mxCellLineStyleValueSet->SetStyle(mxCellLineStyleValueSet->GetStyle()| WB_3DLOOK |  WB_NO_DIRECTSELECT);

    for(sal_uInt16 i = 1 ; i <= 9 ; i++)
    {
        mxCellLineStyleValueSet->InsertItem(i);
    }


    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetLocaleI18nHelper();
    maStr[0] = rI18nHelper.GetNum( 5, 2 ) + "pt";
    maStr[1] = rI18nHelper.GetNum( 250, 2 ) + "pt";
    maStr[2] = rI18nHelper.GetNum( 400, 2 ) + "pt";
    maStr[3] = rI18nHelper.GetNum( 500, 2 ) + "pt";
    maStr[4] = rI18nHelper.GetNum( 110, 2 ) + "pt";
    maStr[5] = rI18nHelper.GetNum( 260, 2 ) + "pt";
    maStr[6] = rI18nHelper.GetNum( 450, 2 ) + "pt";
    maStr[7] = rI18nHelper.GetNum( 505, 2 ) + "pt";
    maStr[8] = rI18nHelper.GetNum( 750, 2 ) + "pt";
    mxCellLineStyleValueSet->SetUnit(&maStr[0]);

    for (sal_uInt16 i = 1; i <= CELL_LINE_STYLE_ENTRIES; ++i)
    {
        mxCellLineStyleValueSet->SetItemText(i, maStr[i-1]);
    }

    SetAllNoSel();
    mxCellLineStyleValueSet->SetSelectHdl(LINK(this, CellLineStylePopup, VSSelectHdl));
}

void CellLineStylePopup::GrabFocus()
{
    mxCellLineStyleValueSet->GrabFocus();
}

void CellLineStylePopup::SetAllNoSel()
{
    mxCellLineStyleValueSet->SelectItem(0);
    mxCellLineStyleValueSet->SetNoSelection();
    mxCellLineStyleValueSet->SetFormat();
    mxCellLineStyleValueSet->Invalidate();
}

IMPL_LINK_NOARG(CellLineStylePopup, VSSelectHdl, ValueSet*, void)
{
    const sal_uInt16 iPos(mxCellLineStyleValueSet->GetSelectedItemId());
    SvxLineItem aLineItem(SID_FRAME_LINESTYLE);
    SvxBorderLineStyle nStyle = SvxBorderLineStyle::SOLID;
    sal_uInt16 n1 = 0;
    sal_uInt16 n2 = 0;
    sal_uInt16 n3 = 0;

    //FIXME: fully for new border line possibilities

    switch(iPos)
    {
        case 1:
            n1 = DEF_LINE_WIDTH_0;
            break;
        case 2:
            n1 = DEF_LINE_WIDTH_2;
            break;
        case 3:
            n1 = DEF_LINE_WIDTH_3;
            break;
        case 4:
            n1 = DEF_LINE_WIDTH_4;
            break;
        case 5:
            n1 = DEF_LINE_WIDTH_0;
            n2 = DEF_LINE_WIDTH_0;
            n3 = DEF_LINE_WIDTH_1;
            nStyle = SvxBorderLineStyle::DOUBLE;
            break;
        case 6:
            n1 = DEF_LINE_WIDTH_0;
            n2 = DEF_LINE_WIDTH_0;
            n3 = DEF_LINE_WIDTH_2;
            nStyle = SvxBorderLineStyle::DOUBLE;
            break;
        case 7:
            n1 = DEF_LINE_WIDTH_1;
            n2 = DEF_LINE_WIDTH_2;
            n3 = DEF_LINE_WIDTH_1;
            nStyle = SvxBorderLineStyle::DOUBLE;
            break;
        case 8:
            n1 = DEF_LINE_WIDTH_2;
            n2 = DEF_LINE_WIDTH_0;
            n3 = DEF_LINE_WIDTH_2;
            nStyle = SvxBorderLineStyle::DOUBLE;
            break;
        case 9:
            n1 = DEF_LINE_WIDTH_2;
            n2 = DEF_LINE_WIDTH_2;
            n3 = DEF_LINE_WIDTH_2;
            nStyle = SvxBorderLineStyle::DOUBLE;
            break;
        default:
            break;
    }

    editeng::SvxBorderLine aTmp;
    aTmp.GuessLinesWidths(nStyle, n1, n2, n3);
    aLineItem.SetLine( &aTmp );
    mpDispatcher->ExecuteList(
        SID_FRAME_LINESTYLE, SfxCallMode::RECORD, { &aLineItem });
    SetAllNoSel();

    maToolButton.set_inactive();
}

IMPL_LINK_NOARG(CellLineStylePopup, PBClickHdl, weld::Button&, void)
{
    mpDispatcher->Execute(SID_CELL_FORMAT_BORDER, SfxCallMode::ASYNCHRON);
    maToolButton.set_inactive();
}

void CellLineStylePopup::SetLineStyleSelect(sal_uInt16 out, sal_uInt16 in, sal_uInt16 dis)
{
    mxCellLineStyleValueSet->GrabFocus();
    SetAllNoSel();

    //FIXME: fully for new border line possibilities

    if(out == DEF_LINE_WIDTH_0 && in == 0 && dis == 0)  //1
    {
        mxCellLineStyleValueSet->SetSelItem(1);
    }
    else if(out == DEF_LINE_WIDTH_2 && in == 0 && dis == 0) //2
    {
        mxCellLineStyleValueSet->SetSelItem(2);
    }
    else if(out == DEF_LINE_WIDTH_3 && in == 0 && dis == 0) //3
    {
        mxCellLineStyleValueSet->SetSelItem(3);
    }
    else if(out == DEF_LINE_WIDTH_4 && in == 0 && dis == 0) //4
    {
        mxCellLineStyleValueSet->SetSelItem(4);
    }
    else if(out == DEF_LINE_WIDTH_0 && in == DEF_LINE_WIDTH_0 && dis == DEF_LINE_WIDTH_1) //5
    {
        mxCellLineStyleValueSet->SetSelItem(5);
    }
    else if(out == DEF_LINE_WIDTH_0 && in == DEF_LINE_WIDTH_0 && dis == DEF_LINE_WIDTH_2) //6
    {
        mxCellLineStyleValueSet->SetSelItem(6);
    }
    else if(out == DEF_LINE_WIDTH_1 && in == DEF_LINE_WIDTH_2 && dis == DEF_LINE_WIDTH_1) //7
    {
        mxCellLineStyleValueSet->SetSelItem(7);
    }
    else if(out == DEF_LINE_WIDTH_2 && in == DEF_LINE_WIDTH_0 && dis == DEF_LINE_WIDTH_2) //8
    {
        mxCellLineStyleValueSet->SetSelItem(8);
    }
    else if(out == DEF_LINE_WIDTH_2 && in == DEF_LINE_WIDTH_2 && dis == DEF_LINE_WIDTH_2) //9
    {
        mxCellLineStyleValueSet->SetSelItem(9);
    }
    else
    {
        mxCellLineStyleValueSet->SetSelItem(0);
        mxPushButtonMoreOptions->grab_focus();
    }
    mxCellLineStyleValueSet->SetFormat();
    mxCellLineStyleValueSet->Invalidate();
}

} // end of namespace sc::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
