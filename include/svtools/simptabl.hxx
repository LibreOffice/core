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

#ifndef INCLUDED_SVTOOLS_SIMPTABL_HXX
#define INCLUDED_SVTOOLS_SIMPTABL_HXX

#include <svtools/svtdllapi.h>
#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>
#include <unotools/intlwrapper.hxx>

class SvSimpleTable;
class SVT_DLLPUBLIC SvSimpleTableContainer : public Control
{
private:
    VclPtr<SvSimpleTable>     m_pTable;

protected:
    virtual bool PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

public:
    SvSimpleTableContainer( vcl::Window* pParent, WinBits nBits = WB_BORDER );
    virtual ~SvSimpleTableContainer();
    virtual void dispose() SAL_OVERRIDE;

    void SetTable(SvSimpleTable* pTable);

    virtual void Resize() SAL_OVERRIDE;

    virtual void GetFocus() SAL_OVERRIDE;
};

class SVT_DLLPUBLIC SvSimpleTable : public SvHeaderTabListBox
{
private:
    SvSimpleTableContainer& m_rParentTableContainer;

    Link<>              aHeaderBarClickLink;
    Link<>              aHeaderBarDblClickLink;
    Link<>              aCommandLink;
    CommandEvent        aCEvt;
    VclPtr<HeaderBar>   aHeaderBar;
    long                nOldPos;
    sal_uInt16          nHeaderItemId;
    bool                bPaintFlag;
    bool                bSortDirection;
    sal_uInt16          nSortCol;

    const CollatorWrapper aCollator;

    DECL_LINK_TYPED( StartDragHdl, HeaderBar*, void );
    DECL_LINK_TYPED( DragHdl, HeaderBar*, void );
    DECL_LINK_TYPED( EndDragHdl, HeaderBar*, void );
    DECL_LINK_TYPED( HeaderBarClick, HeaderBar*, void );
    DECL_LINK_TYPED( HeaderBarDblClick, HeaderBar*, void );
    DECL_LINK( CompareHdl, SvSortData* );

protected:

    virtual void            NotifyScrolled() SAL_OVERRIDE;

    virtual void            SetTabs() SAL_OVERRIDE;
    virtual void            Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) SAL_OVERRIDE;

    virtual void            HBarClick();
    void                    HBarDblClick();
    void                    HBarStartDrag();
    void                    HBarDrag();
    void                    HBarEndDrag();

    virtual void            Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;

    virtual sal_Int32       ColCompare(SvTreeListEntry*,SvTreeListEntry*);
public:

    SvSimpleTable(SvSimpleTableContainer& rParent, WinBits nBits = WB_BORDER);
    virtual ~SvSimpleTable();
    virtual void dispose() SAL_OVERRIDE;

    void UpdateViewSize();

    void            InsertHeaderEntry(const OUString& rText,
                            sal_uInt16 nCol=HEADERBAR_APPEND,
                            HeaderBarItemBits nBits = HeaderBarItemBits::STDSTYLE);

    void            SetTabs(const long* pTabs, MapUnit = MAP_APPFONT);

    void            ClearHeader();

    void            ShowTable();
    void            HideTable();
    bool            IsVisible() const;

    void            EnableTable();
    void            DisableTable();
    bool            IsEnabled() const;

    sal_uInt16      GetSelectedCol();
    void            SortByCol(sal_uInt16, bool bDir=true);
    bool            GetSortDirection(){ return bSortDirection;}
    sal_uInt16      GetSortedCol(){ return nSortCol;}
    SvLBoxItem*     GetEntryAtPos( SvTreeListEntry* pEntry, sal_uInt16 nPos ) const;

    CommandEvent    GetCommandEvent()const { return aCEvt;}
    inline bool     IsFocusOnCellEnabled() const { return IsCellFocusEnabled(); }
    void            SetCommandHdl( const Link<>& rLink ) { aCommandLink = rLink; }

    void            SetHeaderBarClickHdl( const Link<>& rLink ) { aHeaderBarClickLink = rLink; }
    HeaderBar&      GetTheHeaderBar() { return *aHeaderBar.get(); }
};


#endif // INCLUDED_SVTOOLS_SIMPTABL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
