/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "hintids.hxx"
#include <svx/gallery.hxx>
#include <vcl/msgbox.hxx>
#include <editeng/brshitem.hxx>
#include <svl/urihelper.hxx>
#include <vcl/graph.hxx>

#include "swtypes.hxx"
#include "docsh.hxx"
#include "insrule.hxx"
#include "swvset.hxx"
#include "app.hrc"
#include "misc.hrc"
#include "helpid.h"

SwInsertGrfRulerDlg::SwInsertGrfRulerDlg( Window* pParent )
    : SfxModalDialog(pParent, "HorizontalRuleDialog", "modules/swriter/ui/horizontalrule.ui")
    , m_sSimple(SW_RESSTR(STR_SIMPLE))
{
    get(m_pOkPB, "ok");
    get(m_pExampleVS, "rulers");

    m_pExampleVS->SetLineCount(6);
    m_pExampleVS->SetColCount(1);
    m_pExampleVS->SetDoubleClickHdl(LINK(this, SwInsertGrfRulerDlg, DoubleClickHdl));

    // determine graphic name
    GalleryExplorer::BeginLocking(GALLERY_THEME_RULERS);
    GalleryExplorer::FillObjList(GALLERY_THEME_RULERS, m_aGrfNames);
    Color aColor(COL_WHITE);
    m_pExampleVS->InsertItem(1, 1);
    m_pExampleVS->SetItemText(1, m_sSimple);

    for(sal_uInt16 i = 1; i <= m_aGrfNames.size(); ++i)
    {
        m_pExampleVS->InsertItem( i + 1, i);
        m_pExampleVS->SetItemText( i + 1, m_aGrfNames[i-1]);
    }

    m_pExampleVS->SelectItem(1);
}

SwInsertGrfRulerDlg::~SwInsertGrfRulerDlg()
{
    GalleryExplorer::EndLocking(GALLERY_THEME_RULERS);
}

bool SwInsertGrfRulerDlg::IsSimpleLine() const
{
    return m_pExampleVS->GetSelectItemId() == 1;
}

OUString SwInsertGrfRulerDlg::GetGraphicName() const
{
    sal_uInt16 nSelPos = m_pExampleVS->GetSelectItemId();

    sal_uInt16 nSel = nSelPos - 2; //align selection position with ValueSet index
    if(nSel < m_aGrfNames.size())
    {
        return URIHelper::SmartRel2Abs(INetURLObject(), m_aGrfNames[nSel],
            URIHelper::GetMaybeFileHdl());
    }
    return OUString();
}

SwRulerValueSet::SwRulerValueSet(Window* pParent, WinBits nWinStyle)
    : SvxBmpNumValueSet(pParent, nWinStyle)
{
    SetStyle(GetStyle() & ~WB_ITEMBORDER);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwRulerValueSet(Window *pParent, VclBuilder::stringmap &)
{
    return new SwRulerValueSet(pParent, WB_ITEMBORDER | WB_TABSTOP);
}

SwRulerValueSet::~SwRulerValueSet()
{
}

void SwRulerValueSet::UserDraw( const UserDrawEvent& rUDEvt )
{
    Rectangle aRect = rUDEvt.GetRect();
    OutputDevice*  pDev = rUDEvt.GetDevice();
    sal_uInt16  nItemId = rUDEvt.GetItemId();
    Point aBLPos = aRect.TopLeft();

    // item count starts with 1 and the 1st is the simple line
    if(nItemId > 1)
    {
        Graphic aGraphic;
        if(GalleryExplorer::GetGraphicObj( GALLERY_THEME_RULERS, nItemId - 2,
                                                            &aGraphic))
        {
            Size aGrfSize = aGraphic.GetPrefSize();
            if(aGrfSize.Width() && aGrfSize.Height())
            {
                int nRelGrf = aGrfSize.Height() * 100 / aGrfSize.Width();
                Size aWinSize = aRect.GetSize();
                Size aPaintSize = aWinSize;
                int nRelWin = aWinSize.Height() * 100 / aWinSize.Width();
                if(nRelGrf > nRelWin)
                {
                    aPaintSize.Width() = aWinSize.Height() * 100 / nRelGrf;
                    aBLPos.X() += (aWinSize.Width() - aPaintSize.Width()) /2;
                }
                else
                {
                    aPaintSize.Height() = aWinSize.Width() * nRelGrf/100;
                    aBLPos.Y() += (aWinSize.Height() - aPaintSize.Height()) /2;
                }
                aBLPos.X() -= aPaintSize.Width() /2;
                aBLPos.Y() -= aPaintSize.Height() /2;

                aPaintSize.Width() *= 2;
                aPaintSize.Height() *= 2;
                if(aPaintSize.Height() < 2)
                    aPaintSize.Height() = 2;
                Region aRegion = pDev->GetClipRegion();
                pDev->SetClipRegion(aRect);
                aGraphic.Draw(pDev, aBLPos, aPaintSize);
                pDev->SetClipRegion(aRegion);
            }
        }
        else
        {
            SetGrfNotFound(sal_True);
        }
    }
    else
    {
        // paint text for simple line
        Font aOldFont = pDev->GetFont();
        Font aFont = pDev->GetFont();
        Size aSize = aFont.GetSize();
        int nRectHeight = aRect.GetHeight();
        aSize.Height() = nRectHeight * 2 / 3;
        aFont.SetSize(aSize);
        pDev->SetFont(aFont);
        String aText(GetItemText(nItemId));
        aSize.Width() = pDev->GetTextWidth(aText);
        aSize.Height() = pDev->GetTextHeight();
        Point aPos(aBLPos);
        aPos.Y() += (nRectHeight - aSize.Height()) / 2;
        aPos.X() += (aRect.GetWidth() - aSize.Width()) / 2;
        pDev->DrawText(aPos, aText);

        pDev->SetFont(aOldFont);
    }
}

IMPL_LINK_NOARG(SwInsertGrfRulerDlg, DoubleClickHdl)
{
    EndDialog(RET_OK);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
