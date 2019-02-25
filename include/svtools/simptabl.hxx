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
#include <vcl/headbar.hxx>
#include <vcl/svtabbx.hxx>
#include <unotools/collatorwrapper.hxx>
#include <vcl/commandevent.hxx>

class SvSimpleTable;
class SVT_DLLPUBLIC SvSimpleTableContainer : public Control
{
private:
    VclPtr<SvSimpleTable>     m_pTable;

protected:
    virtual bool PreNotify( NotifyEvent& rNEvt ) override;

public:
    SvSimpleTableContainer( vcl::Window* pParent, WinBits nBits );
    virtual ~SvSimpleTableContainer() override;
    virtual void dispose() override;

    void SetTable(SvSimpleTable* pTable);
    SvSimpleTable* GetTable();

    virtual void Resize() override;

    virtual void GetFocus() override;

    virtual FactoryFunction GetUITestFactory() const override;
};

class SVT_DLLPUBLIC SvSimpleTable : public SvHeaderTabListBox
{
private:
    SvSimpleTableContainer& m_rParentTableContainer;

    Link<SvSimpleTable*, void> aCommandLink;
    CommandEvent        aCEvt;
    VclPtr<HeaderBar>   aHeaderBar;
    long                nOldPos;
    sal_uInt16          nHeaderItemId;
    bool                bPaintFlag;
    bool                bSortDirection;
    sal_uInt16          nSortCol;

    const CollatorWrapper aCollator;

    DECL_LINK( StartDragHdl, HeaderBar*, void );
    DECL_LINK( DragHdl, HeaderBar*, void );
    DECL_LINK( EndDragHdl, HeaderBar*, void );
    DECL_LINK( HeaderBarClick, HeaderBar*, void );
    DECL_LINK( CompareHdl, const SvSortData&, sal_Int32 );

protected:

    virtual void            NotifyScrolled() override;

    virtual void            SetTabs() override;
    virtual void            Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;

    virtual void            HBarClick();
    void                    HBarDrag();
    void                    HBarEndDrag();

    virtual void            Command( const CommandEvent& rCEvt ) override;

    virtual sal_Int32       ColCompare(SvTreeListEntry*,SvTreeListEntry*);
public:

    SvSimpleTable(SvSimpleTableContainer& rParent, WinBits nBits = WB_BORDER);
    virtual ~SvSimpleTable() override;
    virtual void dispose() override;

    void UpdateViewSize();

    void            InsertHeaderEntry(const OUString& rText,
                            sal_uInt16 nCol=HEADERBAR_APPEND,
                            HeaderBarItemBits nBits = HeaderBarItemBits::STDSTYLE);

    using SvHeaderTabListBox::SetTabs;

    void            ClearHeader();

    void            ShowTable();

    void            EnableTable();
    void            DisableTable();
    bool            IsEnabled() const;

    void            SortByCol(sal_uInt16, bool bDir=true);
    bool            GetSortDirection(){ return bSortDirection;}
    sal_uInt16      GetSortedCol(){ return nSortCol;}
    SvLBoxItem*     GetEntryAtPos( SvTreeListEntry* pEntry, sal_uInt16 nPos ) const;

    const CommandEvent& GetCommandEvent() const { return aCEvt; }
    bool     IsFocusOnCellEnabled() const { return IsCellFocusEnabled(); }
    void            SetCommandHdl( const Link<SvSimpleTable*,void>& rLink ) { aCommandLink = rLink; }

    HeaderBar&      GetTheHeaderBar() { return *aHeaderBar; }
};


#endif // INCLUDED_SVTOOLS_SIMPTABL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
