/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_SORTKEYDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SORTKEYDLG_HXX

#include <vector>
#include <memory>

#include "anyrefdg.hxx"

#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/ctrl.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/langbox.hxx>

struct ScSortKeyItem : public VclBuilderContainer
{
    VclPtr<VclFrame>       m_pFrame;
    VclPtr<FixedText>      m_pFlSort;
    VclPtr<ListBox>        m_pLbSort;
    VclPtr<RadioButton>    m_pBtnUp;
    VclPtr<RadioButton>    m_pBtnDown;

    ScSortKeyItem(vcl::Window* pParent);

    void DisableField();
    void EnableField();

    long getItemHeight() const;
};

typedef std::vector<std::unique_ptr<ScSortKeyItem> > ScSortKeyItems;

class ScSortKeyWindow
{
private:
    VclPtr<VclBox>  m_pBox;
    sal_Int32       nItemHeight;

    ScSortKeyItems& mrSortKeyItems;

public:
    ScSortKeyWindow(SfxTabPage* pParent, ScSortKeyItems& mrSortKeyItems);
    ~ScSortKeyWindow();
    void dispose();

    void AddSortKey( sal_uInt16 nItem );
    void DoScroll( sal_Int32 nNewPos );
    sal_Int32 GetItemHeight() const { return nItemHeight; }
};

class ScSortKeyCtrl
{
private:
    ScSortKeyWindow  m_aSortWin;
    VclScrolledWindow& m_rScrolledWindow;
    ScrollBar&       m_rVertScroll;

    DECL_LINK_TYPED(ScrollHdl, ScrollBar*, void);

    void checkAutoVScroll();

public:
    ScSortKeyCtrl(SfxTabPage* pParent, ScSortKeyItems& mrSortKeyItems);
    void dispose();
    void setScrollRange();
    void AddSortKey( sal_uInt16 nItem );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_SORTKEYDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
