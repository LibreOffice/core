/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sortkeydlg.hxx"
#include "sortdlg.hxx"
#include "sortdlg.hrc"
#include <vcl/layout.hxx>

// =======================================================================

ScSortKeyItem::ScSortKeyItem(Window* pParent)
{
    m_pUIBuilder = new VclBuilder(pParent, getUIRootDir(), "modules/scalc/ui/sortkey.ui");

    get(m_pFrame, "SortKeyFrame");
    get(m_pFlSort, "sortft");
    get(m_pLbSort, "sortlb");
    get(m_pBtnUp, "up");
    get(m_pBtnDown, "down");
}

long ScSortKeyItem::getItemHeight() const
{
    return VclContainer::getLayoutRequisition(*m_pFrame).Height();
}

void ScSortKeyItem::DisableField()
{
    m_pFrame->Disable();
}

// -----------------------------------------------------------------------

void ScSortKeyItem::EnableField()
{
    m_pFrame->Enable();
}

// =======================================================================

ScSortKeyWindow::ScSortKeyWindow(SfxTabPage* pParent, ScSortKeyItems& rSortKeyItems)
    : mrSortKeyItems(rSortKeyItems)
{
    pParent->get(m_pBox, "SortKeyWindow");
    if (!mrSortKeyItems.empty())
        nItemHeight = mrSortKeyItems.front().getItemHeight();
    else
    {
        ScSortKeyItem aTemp(m_pBox);
        nItemHeight = aTemp.getItemHeight();
    }
}

// -----------------------------------------------------------------------

ScSortKeyWindow::~ScSortKeyWindow()
{
    mrSortKeyItems.clear();
}

// -----------------------------------------------------------------------

void ScSortKeyWindow::AddSortKey( sal_uInt16 nItemNumber )
{
    ScSortKeyItem* pSortKeyItem = new ScSortKeyItem(m_pBox);

    // Set Sort key number
    OUString aLine = pSortKeyItem->m_pFlSort->GetText() +
                     OUString::number( nItemNumber );
    pSortKeyItem->m_pFlSort->SetText( aLine );

    mrSortKeyItems.push_back(pSortKeyItem);
}

// -----------------------------------------------------------------------

void ScSortKeyWindow::DoScroll(sal_Int32 nNewPos)
{
    m_pBox->SetPosPixel(Point(0, nNewPos));
}

// =======================================================================

ScSortKeyCtrl::ScSortKeyCtrl(SfxTabPage* pParent, ScSortKeyItems& rItems)
    : m_aSortWin(pParent, rItems)
    , m_rScrolledWindow(*pParent->get<VclScrolledWindow>("SortCriteriaPage"))
    , m_rVertScroll(m_rScrolledWindow.getVertScrollBar())
{
    m_rScrolledWindow.setUserManagedScrolling(true);

    m_rVertScroll.EnableDrag();
    m_rVertScroll.Show(m_rScrolledWindow.GetStyle() & WB_VSCROLL);

    m_rVertScroll.SetRangeMin( 0 );
    m_rVertScroll.SetVisibleSize( 0xFFFF );

    Link aScrollLink = LINK( this, ScSortKeyCtrl, ScrollHdl );
    m_rVertScroll.SetScrollHdl( aScrollLink );
}

void ScSortKeyCtrl::checkAutoVScroll()
{
    WinBits nBits = m_rScrolledWindow.GetStyle();
    if (nBits & WB_VSCROLL)
        return;
    if (nBits & WB_AUTOVSCROLL)
    {
        sal_Bool bShow = m_rVertScroll.GetRangeMax() > m_rVertScroll.GetVisibleSize();
        if (bShow != m_rVertScroll.IsVisible())
            m_rVertScroll.Show(bShow);
    }
}

void ScSortKeyCtrl::setScrollRange()
{
    sal_Int32 nScrollOffset = m_aSortWin.GetItemHeight();
    sal_Int32 nVisibleItems = m_rScrolledWindow.getVisibleChildSize().Height() / nScrollOffset;
    m_rVertScroll.SetPageSize( nVisibleItems - 1 );
    m_rVertScroll.SetVisibleSize( nVisibleItems );
    m_rVertScroll.Scroll();
    checkAutoVScroll();
}

// -----------------------------------------------------------------------

IMPL_LINK( ScSortKeyCtrl, ScrollHdl, ScrollBar*, pScrollBar )
{
    sal_Int32 nOffset = m_aSortWin.GetItemHeight();
    nOffset *= pScrollBar->GetThumbPos();
    m_aSortWin.DoScroll( -nOffset );
    return 0;
}

// -----------------------------------------------------------------------

void ScSortKeyCtrl::AddSortKey( sal_uInt16 nItem )
{
    m_rVertScroll.SetRangeMax( nItem );
    m_rVertScroll.DoScroll( nItem );
    m_aSortWin.AddSortKey( nItem );
    checkAutoVScroll();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
