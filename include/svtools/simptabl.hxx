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

#ifndef _SV_SIMPTABL_HXX
#define _SV_SIMPTABL_HXX

#include "svtools/svtdllapi.h"
#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>

class SvSimpleTable;
class SVT_DLLPUBLIC SvSimpleTableContainer : public Control
{
private:
    SvSimpleTable*     m_pTable;

protected:
    virtual long PreNotify( NotifyEvent& rNEvt );

public:
    SvSimpleTableContainer( Window* pParent, const ResId& rResId );
    SvSimpleTableContainer( Window* pParent, WinBits nBits = WB_BORDER );

    void SetTable(SvSimpleTable* pTable);

    virtual void Resize();

    virtual void GetFocus();
};

class SVT_DLLPUBLIC SvSimpleTable : public SvHeaderTabListBox
{
private:
    SvSimpleTableContainer& m_rParentTableContainer;

    Link                aHeaderBarClickLink;
    Link                aHeaderBarDblClickLink;
    Link                aCommandLink;
    CommandEvent        aCEvt;
    HeaderBar           aHeaderBar;
    long                nOldPos;
    sal_uInt16              nHeaderItemId;
    sal_Bool                bResizeFlag;
    sal_Bool                bPaintFlag;
    sal_Bool                bSortDirection;
    sal_uInt16              nSortCol;

    DECL_LINK( StartDragHdl, HeaderBar* );
    DECL_LINK( DragHdl, HeaderBar* );
    DECL_LINK( EndDragHdl, HeaderBar* );
    DECL_LINK( HeaderBarClick, HeaderBar* );
    DECL_LINK( HeaderBarDblClick, HeaderBar* );
    DECL_LINK( CompareHdl, SvSortData* );

protected:

    virtual void            NotifyScrolled();

    virtual void            SetTabs();
    virtual void            Paint( const Rectangle& rRect );

    virtual void            HBarClick();
    virtual void            HBarDblClick();
    virtual void            HBarStartDrag();
    virtual void            HBarDrag();
    virtual void            HBarEndDrag();

    virtual void            Command( const CommandEvent& rCEvt );

    virtual StringCompare   ColCompare(SvTreeListEntry*,SvTreeListEntry*);
public:

    SvSimpleTable(SvSimpleTableContainer& rParent, WinBits nBits = WB_BORDER);
    ~SvSimpleTable();

    void UpdateViewSize();

    void            InsertHeaderEntry(const OUString& rText,
                            sal_uInt16 nCol=HEADERBAR_APPEND,
                            HeaderBarItemBits nBits = HIB_STDSTYLE);

    void            SetTabs( long* pTabs, MapUnit = MAP_APPFONT );

    void            ClearHeader();

    // to be removed all calls of the related methods are redirected to *Table() methods
    using Window::Show;
    using Window::Hide;
    using Window::Enable;
    using Window::Disable;
    using Window::ToTop;

    void            Show();
    void            Hide();
    void            Enable();
    void            Disable();
    void            ToTop();

    // remove until this line

    void            ShowTable();
    void            HideTable();
    sal_Bool            IsVisible() const;

    void            EnableTable();
    void            DisableTable();
    sal_Bool            IsEnabled() const;

    sal_uInt16          GetSelectedCol();
    void            SortByCol(sal_uInt16,sal_Bool bDir=sal_True);
    sal_Bool            GetSortDirection(){ return bSortDirection;}
    sal_uInt16          GetSortedCol(){ return nSortCol;}
    SvLBoxItem*     GetEntryAtPos( SvTreeListEntry* pEntry, sal_uInt16 nPos ) const;

    CommandEvent    GetCommandEvent()const;
    inline sal_Bool IsFocusOnCellEnabled() const { return IsCellFocusEnabled(); }

    void            SetCommandHdl( const Link& rLink ) { aCommandLink = rLink; }
    const Link&     GetCommandHdl() const { return aCommandLink; }

    void            SetHeaderBarClickHdl( const Link& rLink ) { aHeaderBarClickLink = rLink; }
    const Link&     GetHeaderBarClickHdl() const { return aHeaderBarClickLink; }

    void            SetHeaderBarDblClickHdl( const Link& rLink ) { aHeaderBarDblClickLink = rLink; }
    const Link&     GetHeaderBarDblClickHdl() const { return aHeaderBarDblClickLink; }

    void            SetHeaderBarHelpId(const OString& rHelpId) {aHeaderBar.SetHelpId(rHelpId);}

    HeaderBar&      GetTheHeaderBar() {return aHeaderBar;}
};


#endif // _SV_SIMPTABL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
