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

#include <CellLineStyleControl.hxx>
#include "sc.hrc"
#include "scresid.hxx"
#include <CellAppearancePropertyPanel.hrc>
#include <CellLineStyleValueSet.hxx>
#include <vcl/i18nhelp.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/lineitem.hxx>
#include <CellAppearancePropertyPanel.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

namespace sc { namespace sidebar {

CellLineStyleControl::CellLineStyleControl(Window* pParent, CellAppearancePropertyPanel& rPanel)
:   svx::sidebar::PopupControl(pParent, ScResId(RID_POPUPPANEL_APPEARANCE_CELL_LINESTYLE)),
    mrCellAppearancePropertyPanel(rPanel),
    maPushButtonMoreOptions(this, ScResId(PB_OPTIONS)),
    maCellLineStyleValueSet(this, ScResId(VS_STYLE)),
    mbVSfocus(true)
{
    Initialize();
    FreeResource();
}

void CellLineStyleControl::Initialize()
{
    //maPushButtonMoreOptions.SetIcoPosX(2);
    Link aLink = LINK(this, CellLineStyleControl, PBClickHdl);
    maPushButtonMoreOptions.SetClickHdl(aLink);

    maCellLineStyleValueSet.SetStyle(maCellLineStyleValueSet.GetStyle()| WB_3DLOOK |  WB_NO_DIRECTSELECT);
    maCellLineStyleValueSet.SetControlBackground(GetSettings().GetStyleSettings().GetMenuColor());
    maCellLineStyleValueSet.SetColor(GetSettings().GetStyleSettings().GetMenuColor());

    for(sal_uInt16 i = 1 ; i <= 9 ; i++)
    {
        maCellLineStyleValueSet.InsertItem(i);
    }

    maStr[0] = GetSettings().GetLocaleI18nHelper().GetNum( 5, 2 ) + "pt";
    maStr[1] = GetSettings().GetLocaleI18nHelper().GetNum( 250, 2 ) + "pt";
    maStr[2] = GetSettings().GetLocaleI18nHelper().GetNum( 400, 2 ) + "pt";
    maStr[3] = GetSettings().GetLocaleI18nHelper().GetNum( 500, 2 ) + "pt";
    maStr[4] = GetSettings().GetLocaleI18nHelper().GetNum( 110, 2 ) + "pt";
    maStr[5] = GetSettings().GetLocaleI18nHelper().GetNum( 260, 2 ) + "pt";
    maStr[6] = GetSettings().GetLocaleI18nHelper().GetNum( 450, 2 ) + "pt";
    maStr[7] = GetSettings().GetLocaleI18nHelper().GetNum( 505, 2 ) + "pt";
    maStr[8] = GetSettings().GetLocaleI18nHelper().GetNum( 750, 2 ) + "pt";
    maCellLineStyleValueSet.SetUnit(&maStr[0]);

    for (sal_uInt16 i = 1; i <= CELL_LINE_STYLE_ENTRIES; ++i)
    {
        maCellLineStyleValueSet.SetItemText(i, maStr[i-1]);
    }

    SetAllNoSel();
    aLink = LINK(this, CellLineStyleControl, VSSelectHdl);
    maCellLineStyleValueSet.SetSelectHdl(aLink);
    maCellLineStyleValueSet.StartSelection();
    maCellLineStyleValueSet.Show();
}

void CellLineStyleControl::GetFocus()
{
    if(!mbVSfocus)
    {
        maPushButtonMoreOptions.GrabFocus();
    }
    else
    {
        maCellLineStyleValueSet.GrabFocus();
    }
}

void CellLineStyleControl::SetAllNoSel()
{
    maCellLineStyleValueSet.SelectItem(0);
    maCellLineStyleValueSet.SetNoSelection();
    maCellLineStyleValueSet.Format();
    Invalidate();
    maCellLineStyleValueSet.StartSelection();
}

IMPL_LINK(CellLineStyleControl, VSSelectHdl, void *, pControl)
{
    if(pControl == &maCellLineStyleValueSet)
    {
        const sal_uInt16 iPos(maCellLineStyleValueSet.GetSelectItemId());
        SvxLineItem aLineItem(SID_FRAME_LINESTYLE);
        using namespace ::com::sun::star::table::BorderLineStyle;
        editeng::SvxBorderStyle nStyle = SOLID;
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
                nStyle = DOUBLE;
                break;
            case 6:
                n1 = DEF_LINE_WIDTH_0;
                n2 = DEF_LINE_WIDTH_0;
                n3 = DEF_LINE_WIDTH_2;
                nStyle = DOUBLE;
                break;
            case 7:
                n1 = DEF_LINE_WIDTH_1;
                n2 = DEF_LINE_WIDTH_2;
                n3 = DEF_LINE_WIDTH_1;
                nStyle = DOUBLE;
                break;
            case 8:
                n1 = DEF_LINE_WIDTH_2;
                n2 = DEF_LINE_WIDTH_0;
                n3 = DEF_LINE_WIDTH_2;
                nStyle = DOUBLE;
                break;
            case 9:
                n1 = DEF_LINE_WIDTH_2;
                n2 = DEF_LINE_WIDTH_2;
                n3 = DEF_LINE_WIDTH_2;
                nStyle = DOUBLE;
                break;
            default:
                break;
        }

        editeng::SvxBorderLine aTmp;
        aTmp.GuessLinesWidths(nStyle, n1, n2, n3);
        aLineItem.SetLine( &aTmp );
        mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_FRAME_LINESTYLE, SFX_CALLMODE_RECORD, &aLineItem, 0L);
        SetAllNoSel();
        mrCellAppearancePropertyPanel.EndCellLineStylePopupMode();
    }

    return(0L);
}

IMPL_LINK(CellLineStyleControl, PBClickHdl, PushButton *, pPBtn)
{
    if(pPBtn == &maPushButtonMoreOptions)
    {
        if(mrCellAppearancePropertyPanel.GetBindings())
        {
            mrCellAppearancePropertyPanel.GetBindings()->GetDispatcher()->Execute(SID_CELL_FORMAT_BORDER, SFX_CALLMODE_ASYNCHRON);
        }

        mrCellAppearancePropertyPanel.EndCellLineStylePopupMode();
    }

    return 0;
}

void CellLineStyleControl::SetLineStyleSelect(sal_uInt16 out, sal_uInt16 in, sal_uInt16 dis)
{
    SetAllNoSel();
    mbVSfocus = true;

    //FIXME: fully for new border line possibilities

    if(out == DEF_LINE_WIDTH_0 && in == 0 && dis == 0)  //1
    {
        maCellLineStyleValueSet.SetSelItem(1);
    }
    else if(out == DEF_LINE_WIDTH_2 && in == 0 && dis == 0) //2
    {
        maCellLineStyleValueSet.SetSelItem(2);
    }
    else if(out == DEF_LINE_WIDTH_3 && in == 0 && dis == 0) //3
    {
        maCellLineStyleValueSet.SetSelItem(3);
    }
    else if(out == DEF_LINE_WIDTH_4 && in == 0 && dis == 0) //4
    {
        maCellLineStyleValueSet.SetSelItem(4);
    }
    else if(out == DEF_LINE_WIDTH_0 && in == DEF_LINE_WIDTH_0 && dis == DEF_LINE_WIDTH_1) //5
    {
        maCellLineStyleValueSet.SetSelItem(5);
    }
    else if(out == DEF_LINE_WIDTH_0 && in == DEF_LINE_WIDTH_0 && dis == DEF_LINE_WIDTH_2) //6
    {
        maCellLineStyleValueSet.SetSelItem(6);
    }
    else if(out == DEF_LINE_WIDTH_1 && in == DEF_LINE_WIDTH_2 && dis == DEF_LINE_WIDTH_1) //7
    {
        maCellLineStyleValueSet.SetSelItem(7);
    }
    else if(out == DEF_LINE_WIDTH_2 && in == DEF_LINE_WIDTH_0 && dis == DEF_LINE_WIDTH_2) //8
    {
        maCellLineStyleValueSet.SetSelItem(8);
    }
    else if(out == DEF_LINE_WIDTH_2 && in == DEF_LINE_WIDTH_2 && dis == DEF_LINE_WIDTH_2) //9
    {
        maCellLineStyleValueSet.SetSelItem(9);
    }
    else
    {
        maCellLineStyleValueSet.SetSelItem(0);
        mbVSfocus = false;
    }

    maCellLineStyleValueSet.Format();
    maCellLineStyleValueSet.StartSelection();
}

} } // end of namespace svx::sidebar

// eof
