/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_SORTKEYDLG_HXX
#define SC_SORTKEYDLG_HXX

#include <boost/ptr_container/ptr_vector.hpp>

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
