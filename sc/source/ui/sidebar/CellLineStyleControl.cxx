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
#include <vcl/i18nhelp.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld/Builder.hxx>
#include <editeng/borderline.hxx>
#include <editeng/lineitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <svl/itemset.hxx>
#include <scresid.hxx>
#include <strings.hrc>

namespace sc::sidebar {

CellLineStylePopup::CellLineStylePopup(weld::Toolbar* pParent, const OUString& rId, SfxDispatcher* pDispatcher)
    : WeldToolbarPopup(nullptr, pParent, u"modules/scalc/ui/floatinglinestyle.ui"_ustr, u"FloatingLineStyle"_ustr)
    , maToolButton(pParent, rId)
    , mpDispatcher(pDispatcher)
    , mxCellLineStyleTreeView(m_xBuilder->weld_tree_view(u"linestyletreeview"_ustr))
    , mxPushButtonMoreOptions(m_xBuilder->weld_button(u"more"_ustr))
{
    mxPushButtonMoreOptions->connect_clicked(LINK(this, CellLineStylePopup, PBClickHdl));

    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetLocaleI18nHelper();
    const std::vector<OUString> aStrings = {
        ScResId(STR_BORDER_HAIRLINE).replaceFirst("%s", rI18nHelper.GetNum(5, 2)),
        ScResId(STR_BORDER_VERY_THIN).replaceFirst("%s", rI18nHelper.GetNum(50, 2)),
        ScResId(STR_BORDER_THIN).replaceFirst("%s", rI18nHelper.GetNum(75, 2)),
        ScResId(STR_BORDER_MEDIUM).replaceFirst("%s", rI18nHelper.GetNum(150, 2)),
        ScResId(STR_BORDER_THICK).replaceFirst("%s", rI18nHelper.GetNum(225, 2)),
        ScResId(STR_BORDER_EXTRA_THICK).replaceFirst("%s", rI18nHelper.GetNum(450, 2)),
        // Numbers in pt are the total width of the double line (inner + outer + distance)
        ScResId(STR_BORDER_DOUBLE_1).replaceFirst("%s", rI18nHelper.GetNum(110, 2)),
        ScResId(STR_BORDER_DOUBLE_1).replaceFirst("%s", rI18nHelper.GetNum(235, 2)),
        ScResId(STR_BORDER_DOUBLE_2).replaceFirst("%s", rI18nHelper.GetNum(300, 2)),
        ScResId(STR_BORDER_DOUBLE_3).replaceFirst("%s", rI18nHelper.GetNum(305, 2)),
        ScResId(STR_BORDER_DOUBLE_4).replaceFirst("%s", rI18nHelper.GetNum(450, 2))
    };

    for (size_t i = 0; i < aStrings.size(); ++i)
    {
        mxCellLineStyleTreeView->append();
        mxCellLineStyleTreeView->set_text(i, aStrings.at(i), 1);

        ScopedVclPtr<VirtualDevice> pDev = CreateImage(i);
        mxCellLineStyleTreeView->set_image(i, *pDev, 0);
    }
    mxCellLineStyleTreeView->columns_autosize();
    const int nHeight = mxCellLineStyleTreeView->get_preferred_size().Height();
    mxCellLineStyleTreeView->set_size_request(-1, nHeight);
    mxCellLineStyleTreeView->queue_resize();

    mxCellLineStyleTreeView->connect_row_activated(LINK(this, CellLineStylePopup, StyleSelectHdl));
}

CellLineStylePopup::~CellLineStylePopup()
{
}

VclPtr<VirtualDevice> CellLineStylePopup::CreateImage(int nIndex)
{
    VclPtr<VirtualDevice> pDev = mxCellLineStyleTreeView->create_virtual_device();
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    pDev->SetBackground(rStyleSettings.GetFieldColor());
    pDev->SetLineColor(rStyleSettings.GetFieldTextColor());
    pDev->SetFillColor(pDev->GetLineColor());;
    pDev->SetOutputSizePixel(Size(50, 26));

    constexpr tools::Long nX = 5;
    constexpr tools::Long nY = 10;
    constexpr tools::Long nTRX = 40;
    switch(nIndex)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            pDev->DrawRect(tools::Rectangle(nX, nY , nTRX, nY + nIndex * 2 + 1 ));
            break;
        case 6:
            pDev->DrawRect(tools::Rectangle(nX, nY , nTRX, nY + 1 ));
            pDev->DrawRect(tools::Rectangle(nX, nY + 3 , nTRX, nY + 4 ));
            break;
        case 7:
            pDev->DrawRect(tools::Rectangle(nX, nY , nTRX, nY + 1 ));
            pDev->DrawRect(tools::Rectangle(nX, nY + 5 , nTRX, nY + 6 ));
            break;
        case 8:
            pDev->DrawRect(tools::Rectangle(nX, nY , nTRX, nY + 1 ));
            pDev->DrawRect(tools::Rectangle(nX, nY + 3 , nTRX, nY + 6 ));
            break;
        case 9:
            pDev->DrawRect(tools::Rectangle(nX, nY , nTRX, nY + 3 ));
            pDev->DrawRect(tools::Rectangle(nX, nY + 5 , nTRX, nY + 6 ));
            break;
        case 10:
            pDev->DrawRect(tools::Rectangle(nX, nY , nTRX, nY + 3 ));
            pDev->DrawRect(tools::Rectangle(nX, nY + 5 , nTRX, nY + 8 ));
            break;
    }

    return pDev;
}

void CellLineStylePopup::GrabFocus()
{
    mxCellLineStyleTreeView->grab_focus();
}

IMPL_LINK_NOARG(CellLineStylePopup, StyleSelectHdl, weld::TreeView&, bool)
{
    const int nIndex = mxCellLineStyleTreeView->get_selected_index();
    if (nIndex < 0)
        return false;

    SvxLineItem aLineItem(SID_FRAME_LINESTYLE);
    SvxBorderLineStyle nStyle = SvxBorderLineStyle::SOLID;
    sal_uInt16 n1 = 0;
    sal_uInt16 n2 = 0;
    sal_uInt16 n3 = 0;

    //FIXME: fully for new border line possibilities

    switch(nIndex)
    {
        case 0:
            n1 = SvxBorderLineWidth::Hairline;
            break;
        case 1:
            n1 = SvxBorderLineWidth::VeryThin;
            break;
        case 2:
            n1 = SvxBorderLineWidth::Thin;
            break;
        case 3:
            n1 = SvxBorderLineWidth::Medium;
            break;
        case 4:
            n1 = SvxBorderLineWidth::Thick;
            break;
        case 5:
            n1 = SvxBorderLineWidth::ExtraThick;
            break;
        case 6:
            n1 = SvxBorderLineWidth::Hairline;
            n2 = SvxBorderLineWidth::Hairline;
            n3 = SvxBorderLineWidth::Medium;
            nStyle = SvxBorderLineStyle::DOUBLE;
            break;
        case 7:
            n1 = SvxBorderLineWidth::Hairline;
            n2 = SvxBorderLineWidth::Hairline;
            n3 = SvxBorderLineWidth::Thick;
            nStyle = SvxBorderLineStyle::DOUBLE;
            break;
        case 8:
            n1 = SvxBorderLineWidth::Thin;
            n2 = SvxBorderLineWidth::Medium;
            n3 = SvxBorderLineWidth::Thin;
            nStyle = SvxBorderLineStyle::DOUBLE;
            break;
        case 9:
            n1 = SvxBorderLineWidth::Medium;
            n2 = SvxBorderLineWidth::Hairline;
            n3 = SvxBorderLineWidth::Medium;
            nStyle = SvxBorderLineStyle::DOUBLE;
            break;
        case 10:
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
    mxCellLineStyleTreeView->unselect_all();

    maToolButton.set_inactive();
    return true;
}

IMPL_LINK_NOARG(CellLineStylePopup, PBClickHdl, weld::Button&, void)
{
    mpDispatcher->Execute(SID_CELL_FORMAT_BORDER, SfxCallMode::ASYNCHRON);
    maToolButton.set_inactive();
}

void CellLineStylePopup::SetLineStyleSelect(sal_uInt16 out, sal_uInt16 in, sal_uInt16 dis)
{
    mxCellLineStyleTreeView->grab_focus();

    //FIXME: fully for new border line possibilities

    if (out == SvxBorderLineWidth::Hairline && in == 0 && dis == 0)
    {
        mxCellLineStyleTreeView->select(0);
    }
    else if (out == SvxBorderLineWidth::VeryThin && in == 0 && dis == 0)
    {
        mxCellLineStyleTreeView->select(1);
    }
    else if (out == SvxBorderLineWidth::Thin && in == 0 && dis == 0)
    {
        mxCellLineStyleTreeView->select(2);
    }
    else if (out == SvxBorderLineWidth::Medium && in == 0 && dis == 0)
    {
        mxCellLineStyleTreeView->select(3);
    }
    else if (out == SvxBorderLineWidth::Thick && in == 0 && dis == 0)
    {
        mxCellLineStyleTreeView->select(4);
    }
    else if (out == SvxBorderLineWidth::ExtraThick && in == 0 && dis == 0)
    {
        mxCellLineStyleTreeView->select(5);
    }
    else if (out == SvxBorderLineWidth::Hairline && in == SvxBorderLineWidth::Hairline
             && dis == SvxBorderLineWidth::Thin)
    {
        mxCellLineStyleTreeView->select(6);
    }
    else if (out == SvxBorderLineWidth::Hairline && in == SvxBorderLineWidth::Hairline
             && dis == SvxBorderLineWidth::Medium)
    {
        mxCellLineStyleTreeView->select(7);
    }
    else if (out == SvxBorderLineWidth::Thin && in == SvxBorderLineWidth::Medium
             && dis == SvxBorderLineWidth::Thin)
    {
        mxCellLineStyleTreeView->select(8);
    }
    else if (out == SvxBorderLineWidth::Medium && in == SvxBorderLineWidth::Hairline
             && dis == SvxBorderLineWidth::Medium)
    {
        mxCellLineStyleTreeView->select(9);
    }
    else if (out == SvxBorderLineWidth::Medium && in == SvxBorderLineWidth::Medium
             && dis == SvxBorderLineWidth::Medium)
    {
        mxCellLineStyleTreeView->select(10);
    }
    else
    {
        mxCellLineStyleTreeView->unselect_all();
        mxPushButtonMoreOptions->grab_focus();
    }
}

} // end of namespace sc::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
