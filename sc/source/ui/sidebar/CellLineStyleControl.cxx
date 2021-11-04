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
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/settings.hxx>
#include <editeng/borderline.hxx>
#include <editeng/lineitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <scresid.hxx>
#include <strings.hrc>

namespace sc { namespace sidebar {

CellLineStylePopup::CellLineStylePopup(SfxDispatcher* pDispatcher)
    : FloatingWindow(SfxGetpApp()->GetTopWindow(), "FloatingLineStyle", "modules/scalc/ui/floatinglinestyle.ui")
    , mpDispatcher(pDispatcher)
    , maCellLineStyleValueSet(VclPtr<sc::sidebar::CellLineStyleValueSet>::Create(get<vcl::Window>("box")))
{
    get(maPushButtonMoreOptions, "more");
    Initialize();
}

CellLineStylePopup::~CellLineStylePopup()
{
    disposeOnce();
}

void CellLineStylePopup::dispose()
{
    maPushButtonMoreOptions.clear();
    maCellLineStyleValueSet.disposeAndClear();
    FloatingWindow::dispose();
}

void CellLineStylePopup::Initialize()
{
    Link<Button*,void> aLink = LINK(this, CellLineStylePopup, PBClickHdl);
    maPushButtonMoreOptions->SetClickHdl(aLink);

    maCellLineStyleValueSet->SetStyle(maCellLineStyleValueSet->GetStyle()| WB_3DLOOK |  WB_NO_DIRECTSELECT);
    maCellLineStyleValueSet->SetControlBackground(GetSettings().GetStyleSettings().GetMenuColor());
    maCellLineStyleValueSet->SetColor(GetSettings().GetStyleSettings().GetMenuColor());

    for(sal_uInt16 i = 1 ; i <= CELL_LINE_STYLE_ENTRIES ; i++)
    {
        maCellLineStyleValueSet->InsertItem(i);
    }

    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetLocaleI18nHelper();
    maStr[0] = ScResId(STR_BORDER_HAIRLINE).replaceFirst("%s", rI18nHelper.GetNum(5, 2));
    maStr[1] = ScResId(STR_BORDER_VERY_THIN).replaceFirst("%s", rI18nHelper.GetNum(50, 2));
    maStr[2] = ScResId(STR_BORDER_THIN).replaceFirst("%s", rI18nHelper.GetNum(75, 2));
    maStr[3] = ScResId(STR_BORDER_MEDIUM).replaceFirst("%s", rI18nHelper.GetNum(150, 2));
    maStr[4] = ScResId(STR_BORDER_THICK).replaceFirst("%s", rI18nHelper.GetNum(225, 2));
    maStr[5] = ScResId(STR_BORDER_EXTRA_THICK).replaceFirst("%s", rI18nHelper.GetNum(450, 2));

    // Numbers in pt are the total width of the double line (inner + outer + distance)
    maStr[6] = ScResId(STR_BORDER_DOUBLE_1).replaceFirst("%s", rI18nHelper.GetNum(110, 2));
    maStr[7] = ScResId(STR_BORDER_DOUBLE_1).replaceFirst("%s", rI18nHelper.GetNum(235, 2));
    maStr[8] = ScResId(STR_BORDER_DOUBLE_2).replaceFirst("%s", rI18nHelper.GetNum(300, 2));
    maStr[9] = ScResId(STR_BORDER_DOUBLE_3).replaceFirst("%s", rI18nHelper.GetNum(305, 2));
    maStr[10] = ScResId(STR_BORDER_DOUBLE_4).replaceFirst("%s", rI18nHelper.GetNum(450, 2));
    maCellLineStyleValueSet->SetUnit(&maStr[0]);

    for (sal_uInt16 i = 1; i <= CELL_LINE_STYLE_ENTRIES; ++i)
    {
        maCellLineStyleValueSet->SetItemText(i, maStr[i-1]);
    }

    SetAllNoSel();
    maCellLineStyleValueSet->SetSelectHdl(LINK(this, CellLineStylePopup, VSSelectHdl));
    maCellLineStyleValueSet->StartSelection();
    maCellLineStyleValueSet->Show();
}

void CellLineStylePopup::SetAllNoSel()
{
    maCellLineStyleValueSet->SelectItem(0);
    maCellLineStyleValueSet->SetNoSelection();
    maCellLineStyleValueSet->SetFormat();
    maCellLineStyleValueSet->Invalidate();
    Invalidate();
    maCellLineStyleValueSet->StartSelection();
}

IMPL_LINK(CellLineStylePopup, VSSelectHdl, ValueSet*, pControl, void)
{
    if(pControl == maCellLineStyleValueSet.get())
    {
        const sal_uInt16 iPos(maCellLineStyleValueSet->GetSelectedItemId());
        SvxLineItem aLineItem(SID_FRAME_LINESTYLE);
        SvxBorderLineStyle nStyle = SvxBorderLineStyle::SOLID;
        sal_uInt16 n1 = 0;
        sal_uInt16 n2 = 0;
        sal_uInt16 n3 = 0;

        //FIXME: fully for new border line possibilities

        switch(iPos)
        {
            case 1:
                n1 = SvxBorderLineWidth::Hairline;
                break;
            case 2:
                n1 = SvxBorderLineWidth::VeryThin;
                break;
            case 3:
                n1 = SvxBorderLineWidth::Thin;
                break;
            case 4:
                n1 = SvxBorderLineWidth::Medium;
                break;
            case 5:
                n1 = SvxBorderLineWidth::Thick;
                break;
            case 6:
                n1 = SvxBorderLineWidth::ExtraThick;
                break;
            case 7:
                n1 = SvxBorderLineWidth::Hairline;
                n2 = SvxBorderLineWidth::Hairline;
                n3 = SvxBorderLineWidth::Medium;
                nStyle = SvxBorderLineStyle::DOUBLE;
                break;
            case 8:
                n1 = SvxBorderLineWidth::Hairline;
                n2 = SvxBorderLineWidth::Hairline;
                n3 = SvxBorderLineWidth::Thick;
                nStyle = SvxBorderLineStyle::DOUBLE;
                break;
            case 9:
                n1 = SvxBorderLineWidth::Thin;
                n2 = SvxBorderLineWidth::Medium;
                n3 = SvxBorderLineWidth::Thin;
                nStyle = SvxBorderLineStyle::DOUBLE;
                break;
            case 10:
                n1 = SvxBorderLineWidth::Medium;
                n2 = SvxBorderLineWidth::Hairline;
                n3 = SvxBorderLineWidth::Medium;
                nStyle = SvxBorderLineStyle::DOUBLE;
                break;
            case 11:
                n1 = SvxBorderLineWidth::Medium;
                n2 = SvxBorderLineWidth::Medium;
                n3 = SvxBorderLineWidth::Medium;
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
        EndPopupMode();
    }
}

IMPL_LINK(CellLineStylePopup, PBClickHdl, Button *, pPBtn, void)
{
    if (pPBtn == maPushButtonMoreOptions.get())
    {
        mpDispatcher->Execute(SID_CELL_FORMAT_BORDER, SfxCallMode::ASYNCHRON);
        EndPopupMode();
    }
}

void CellLineStylePopup::SetLineStyleSelect(sal_uInt16 out, sal_uInt16 in, sal_uInt16 dis)
{
    maCellLineStyleValueSet->GrabFocus();
    SetAllNoSel();

    //FIXME: fully for new border line possibilities

    if(out == SvxBorderLineWidth::Hairline && in == 0 && dis == 0)  //1
    {
        maCellLineStyleValueSet->SetSelItem(1);
    }
    else if(out == SvxBorderLineWidth::VeryThin && in == 0 && dis == 0) //2
    {
        maCellLineStyleValueSet->SetSelItem(2);
    }
    else if(out == SvxBorderLineWidth::Thin && in == 0 && dis == 0) //3
    {
        maCellLineStyleValueSet->SetSelItem(3);
    }
    else if(out == SvxBorderLineWidth::Medium && in == 0 && dis == 0) //4
    {
        maCellLineStyleValueSet->SetSelItem(4);
    }
    else if(out == SvxBorderLineWidth::Thick && in == 0 && dis == 0) //5
    {
        maCellLineStyleValueSet->SetSelItem(5);
    }
    else if(out == SvxBorderLineWidth::ExtraThick && in == 0 && dis == 0) //6
    {
        maCellLineStyleValueSet->SetSelItem(6);
    }
    else if(out == SvxBorderLineWidth::Hairline && in == SvxBorderLineWidth::Hairline && dis == SvxBorderLineWidth::Thin) //7
    {
        maCellLineStyleValueSet->SetSelItem(7);
    }
    else if(out == SvxBorderLineWidth::Hairline && in == SvxBorderLineWidth::Hairline && dis == SvxBorderLineWidth::Medium) //8
    {
        maCellLineStyleValueSet->SetSelItem(8);
    }
    else if(out == SvxBorderLineWidth::Thin && in == SvxBorderLineWidth::Medium && dis == SvxBorderLineWidth::Thin) //9
    {
        maCellLineStyleValueSet->SetSelItem(9);
    }
    else if(out == SvxBorderLineWidth::Medium && in == SvxBorderLineWidth::Hairline && dis == SvxBorderLineWidth::Medium) //10
    {
        maCellLineStyleValueSet->SetSelItem(10);
    }
    else if(out == SvxBorderLineWidth::Medium && in == SvxBorderLineWidth::Medium && dis == SvxBorderLineWidth::Medium) //11
    {
        maCellLineStyleValueSet->SetSelItem(11);
    }

    else
    {
        maCellLineStyleValueSet->SetSelItem(0);
        maPushButtonMoreOptions->GrabFocus();
    }
    maCellLineStyleValueSet->SetFormat();
    maCellLineStyleValueSet->Invalidate();
    maCellLineStyleValueSet->StartSelection();
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
