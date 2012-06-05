/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *  Copyright (C) 2012 Albert Thuswaldner <albert.thuswaldner@gmail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "sortkeydlg.hxx"
#include "sortdlg.hxx"
#include "sortdlg.hrc"

// =======================================================================

ScSortKeyItem::ScSortKeyItem( Window* pParent ) :
    //
    aFlSort        ( pParent, ScResId( FL_SORT  ) ),
    aLbSort        ( pParent, ScResId( LB_SORT  ) ),
    aBtnUp         ( pParent, ScResId( BTN_UP   ) ),
    aBtnDown       ( pParent, ScResId( BTN_DOWN ) )
{
}

// -----------------------------------------------------------------------

void ScSortKeyItem::DisableField()
{
    aFlSort.Disable();
    aLbSort.Disable();
    aBtnUp.Disable();
    aBtnDown.Disable();
}

// -----------------------------------------------------------------------

void ScSortKeyItem::EnableField()
{
    aFlSort.Enable();
    aLbSort.Enable();
    aBtnUp.Enable();
    aBtnDown.Enable();
}

// =======================================================================

ScSortKeyWindow::ScSortKeyWindow( Window* pParent, const ResId& rResId, ScSortKeyItems& rSortKeyItems ) :
    Window( pParent, rResId ),
    //
    aFlSort        ( this, ScResId( FL_SORT  ) ),
    aLbSort        ( this, ScResId( LB_SORT  ) ),
    aBtnUp         ( this, ScResId( BTN_UP   ) ),
    aBtnDown       ( this, ScResId( BTN_DOWN ) ),
    //
    nScrollPos     ( 0 ),
    mrSortKeyItems( rSortKeyItems )
{
    aFlSort.Hide();
    aLbSort.Hide();
    aBtnUp.Hide();
    aBtnDown.Hide();

    nItemHeight = aBtnDown.GetPosPixel().Y() + aBtnDown.GetSizePixel().Height();
}

// -----------------------------------------------------------------------

ScSortKeyWindow::~ScSortKeyWindow()
{
    mrSortKeyItems.clear();
}

// -----------------------------------------------------------------------

void ScSortKeyWindow::AddSortKey( sal_uInt16 nItemNumber )
{
    ScSortKeyItem* pSortKeyItem = new ScSortKeyItem( this );

    // Set Sort key number
    String aLine = pSortKeyItem->aFlSort.GetText();
    aLine += String::CreateFromInt32( nItemNumber );
    pSortKeyItem->aFlSort.SetText( aLine );

    mrSortKeyItems.push_back( pSortKeyItem );

    Window* pWindows[] = {  &aFlSort, &aLbSort, &aBtnUp, &aBtnDown, NULL };

    Window* pNewWindows[] = { &pSortKeyItem->aFlSort, &pSortKeyItem->aLbSort,
                              &pSortKeyItem->aBtnUp,  &pSortKeyItem->aBtnDown, NULL };
    Window** pCurrent = pWindows;
    Window** pNewCurrent = pNewWindows;
    while ( *pCurrent )
    {
        Size aSize = (*pCurrent)->GetSizePixel();
        Point aPos = (*pCurrent)->GetPosPixel();
        aPos.Y() += ( nItemNumber - 1 ) * GetItemHeight();
        aPos.Y() += nScrollPos;
        (*pNewCurrent)->SetPosSizePixel( aPos, aSize );
        (*pNewCurrent)->Show();
        pCurrent++;
        pNewCurrent++;
    }
}

// -----------------------------------------------------------------------

void ScSortKeyWindow::DoScroll( sal_Int32 nNewPos )
{
    nScrollPos += nNewPos;
    ScSortKeyItems::iterator pIter;
    for ( pIter = mrSortKeyItems.begin(); pIter != mrSortKeyItems.end(); ++pIter )
    {
        Window* pNewWindows[] = { &pIter->aFlSort, &pIter->aLbSort,
                                  &pIter->aBtnUp,  &pIter->aBtnDown, NULL };

        Window** pCurrent = pNewWindows;
        while ( *pCurrent )
        {
            Point aPos = (*pCurrent)->GetPosPixel();
            aPos.Y() += nNewPos;
            (*pCurrent)->SetPosPixel( aPos );
            pCurrent++;
        }
    }
}

// =======================================================================

ScSortKeyCtrl::ScSortKeyCtrl( Window* pParent, const ScResId& rResId, ScSortKeyItems& rItems ):
    Control( pParent, rResId),
    //
    aSortWin    ( this, ResId( WIN_MANAGESORTKEY, *rResId.GetResMgr() ), rItems ),
    aVertScroll ( this, ResId( SB_SORT, *rResId.GetResMgr() ) ),
    nThumbPos   ( 0 )
{
    aVertScroll.EnableDrag();
    aVertScroll.Show();

    FreeResource();

    aVertScroll.SetRangeMin( 0 );
    sal_Int32 nScrollOffset = aSortWin.GetItemHeight();
    sal_Int32 nVisibleItems = aSortWin.GetSizePixel().Height() / nScrollOffset;
    aVertScroll.SetRangeMax( nVisibleItems );
    aVertScroll.SetPageSize( nVisibleItems - 1 );
    aVertScroll.SetVisibleSize( nVisibleItems );

    Link aScrollLink = LINK( this, ScSortKeyCtrl, ScrollHdl );
    aVertScroll.SetScrollHdl( aScrollLink );
}

// -----------------------------------------------------------------------

ScSortKeyCtrl::~ScSortKeyCtrl()
{
}

// -----------------------------------------------------------------------

IMPL_LINK( ScSortKeyCtrl, ScrollHdl, ScrollBar*, pScrollBar )
{
    sal_Int32 nOffset = aSortWin.GetItemHeight();
    nOffset *= ( nThumbPos - pScrollBar->GetThumbPos() );
    nThumbPos = pScrollBar->GetThumbPos();
    aSortWin.DoScroll( nOffset );
    return 0;
}

// -----------------------------------------------------------------------

void ScSortKeyCtrl::AddSortKey( sal_uInt16 nItem )
{
    aVertScroll.SetRangeMax( nItem );
    aVertScroll.DoScroll( nItem );
    aSortWin.AddSortKey( nItem );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
