/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/dialog/ThemeColorValueSet.hxx>
#include <vcl/event.hxx>

namespace svx
{
constexpr tools::Long BORDER = 4;
constexpr tools::Long SIZE = 16;
constexpr tools::Long LABEL_HEIGHT = 16;
constexpr tools::Long LABEL_TEXT_HEIGHT = 14;
constexpr tools::Long constElementNumber = 8;

void ThemeColorValueSet::insert(model::ColorSet const& rColorSet)
{
    maColorSets.push_back(std::cref(rColorSet));
    InsertItem(maColorSets.size());
}

void ThemeColorValueSet::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    ValueSet::SetDrawingArea(pDrawingArea);
    SetStyle(WB_TABSTOP | WB_ITEMBORDER | WB_DOUBLEBORDER);
    Size aSize(BORDER * 7 + SIZE * 6 + BORDER * 2, BORDER * 3 + SIZE * 2 + LABEL_HEIGHT);
    SetItemWidth(aSize.Width());
    SetItemHeight(aSize.Height());
}

void ThemeColorValueSet::UserDraw(const UserDrawEvent& rUserDrawEvent)
{
    vcl::RenderContext* pDev = rUserDrawEvent.GetRenderContext();
    tools::Rectangle aRect = rUserDrawEvent.GetRect();
    const Point aPosition = aRect.GetPos();
    const sal_uInt16 nItemId = rUserDrawEvent.GetItemId();
    model::ColorSet const& rColorSet = maColorSets[nItemId - 1];

    Size aSize = aRect.GetSize();
    Size aMin(BORDER * 7 + SIZE * constElementNumber / 2 + BORDER * 2,
              BORDER * 3 + SIZE * 2 + LABEL_HEIGHT);
    tools::Long startX = (aSize.Width() / 2.0) - (aMin.Width() / 2.0);
    tools::Long x = BORDER;
    tools::Long y1 = BORDER + LABEL_HEIGHT;
    tools::Long y2 = y1 + SIZE + BORDER;

    pDev->SetLineColor(COL_LIGHTGRAY);
    pDev->SetFillColor(COL_LIGHTGRAY);
    tools::Rectangle aNameRect(aPosition, Size(aSize.Width(), LABEL_HEIGHT));
    pDev->DrawRect(aNameRect);

    vcl::Font aFont;
    OUString aName = rColorSet.getName();
    aFont.SetFontHeight(LABEL_TEXT_HEIGHT);
    pDev->SetFont(aFont);

    Size aTextSize(pDev->GetTextWidth(aName), pDev->GetTextHeight());

    Point aPoint(aPosition.X() + (aNameRect.GetWidth() / 2.0) - (aTextSize.Width() / 2.0),
                 aPosition.Y() + (aNameRect.GetHeight() / 2.0) - (aTextSize.Height() / 2.0));

    pDev->DrawText(aPoint, aName);

    pDev->SetLineColor(COL_LIGHTGRAY);
    pDev->SetFillColor();

    for (sal_uInt32 i = 2; i < 10; i += 2)
    {
        pDev->SetFillColor(rColorSet.getColor(model::convertToThemeColorType(i)));
        pDev->DrawRect(tools::Rectangle(Point(aPosition.X() + x + startX, aPosition.Y() + y1),
                                        Size(SIZE, SIZE)));

        pDev->SetFillColor(rColorSet.getColor(model::convertToThemeColorType(i + 1)));
        pDev->DrawRect(tools::Rectangle(Point(aPosition.X() + x + startX, aPosition.Y() + y2),
                                        Size(SIZE, SIZE)));

        x += SIZE + BORDER;
        if (i == 2 || i == 8)
            x += BORDER;
    }
}

void ThemeColorValueSet::StyleUpdated()
{
    SetFormat();
    Invalidate();
    ValueSet::StyleUpdated();
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
