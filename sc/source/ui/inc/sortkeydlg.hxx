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

#ifndef SC_SORTKEYDLG_HXX
#define SC_SORTKEYDLG_HXX

#include <boost/ptr_container/ptr_vector.hpp>

#include "anyrefdg.hxx"
#include "sortdlg.hrc"

#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/ctrl.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/langbox.hxx>

// =======================================================================

struct ScSortKeyItem : public VclBuilderContainer
{
    VclFrame*       m_pFrame;
    FixedText*      m_pFlSort;
    ListBox*        m_pLbSort;
    RadioButton*    m_pBtnUp;
    RadioButton*    m_pBtnDown;

    ScSortKeyItem(Window* pParent);

    void DisableField();
    void EnableField();

    long getItemHeight() const;
};

typedef boost::ptr_vector<ScSortKeyItem> ScSortKeyItems;

// =======================================================================

class ScSortKeyWindow
{
private:
    VclBox*         m_pBox;
    sal_Int32       nItemHeight;

    ScSortKeyItems& mrSortKeyItems;

public:
    ScSortKeyWindow(SfxTabPage* pParent, ScSortKeyItems& mrSortKeyItems);
    ~ScSortKeyWindow();

    void AddSortKey( sal_uInt16 nItem );
    void DoScroll( sal_Int32 nNewPos );
    sal_Int32 GetItemHeight() const { return nItemHeight; }
    sal_Int32 GetTotalHeight() const { return m_pBox->GetSizePixel().Height(); }
};

// =======================================================================

class ScSortKeyCtrl
{
private:
    ScSortKeyWindow  m_aSortWin;
    VclScrolledWindow& m_rScrolledWindow;
    ScrollBar&       m_rVertScroll;

    DECL_LINK(ScrollHdl, ScrollBar*);

    void checkAutoVScroll();

public:
    ScSortKeyCtrl(SfxTabPage* pParent, ScSortKeyItems& mrSortKeyItems);
    void setScrollRange();
    void AddSortKey( sal_uInt16 nItem );
};

#endif // SC_SORTKEYDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
