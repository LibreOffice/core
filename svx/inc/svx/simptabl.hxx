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

#ifndef _SVX_SIMPTABL_HXX
#define _SVX_SIMPTABL_HXX

#include <vcl/morebtn.hxx>

#include <vcl/combobox.hxx>

#include <vcl/group.hxx>
#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>
#include <vcl/lstbox.hxx>
#include "svx/svxdllapi.h"

class SvxSimpleTable;
class SVX_DLLPUBLIC SvxSimpleTableContainer : public Control
{
private:
    SvxSimpleTable*     m_pTable;

protected:
    virtual long PreNotify( NotifyEvent& rNEvt );

public:
    SvxSimpleTableContainer( Window* pParent, const ResId& rResId );

    void SetTable(SvxSimpleTable* pTable);

    virtual void SetSizePixel(const Size& rNewSize);
    virtual void GetFocus();
};

class SVX_DLLPUBLIC SvxSimpleTable : public SvHeaderTabListBox
{
private:
    SvxSimpleTableContainer& m_rParentTableContainer;

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

    SvxSimpleTable(SvxSimpleTableContainer& rParent, WinBits nBits = WB_BORDER);
    ~SvxSimpleTable();

    void UpdateViewSize();

    void            InsertHeaderEntry(const rtl::OUString& rText,
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

    void            SetHeaderBarHelpId(const rtl::OString& rHelpId) {aHeaderBar.SetHelpId(rHelpId);}

    HeaderBar&      GetTheHeaderBar() {return aHeaderBar;}
};


#endif // _SVX_SIMPTABL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
