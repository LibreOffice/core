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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DBTREE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DBTREE_HXX

#include <memory>
#include <vcl/treelistbox.hxx>
#include <vcl/weld.hxx>

#include <swdllapi.h>
#include <swtypes.hxx>

class SwDBTreeList_Impl;
class SwWrtShell;

class SW_DLLPUBLIC SwDBTreeList : public SvTreeListBox
{
    bool            bInitialized;
    bool            bShowColumns;

    rtl::Reference<SwDBTreeList_Impl> pImpl;

    DECL_DLLPRIVATE_LINK( DBCompare, const SvSortData&, sal_Int32 );

    SAL_DLLPRIVATE void          InitTreeList();
    SAL_DLLPRIVATE virtual void  RequestingChildren( SvTreeListEntry* pParent ) override;

    SAL_DLLPRIVATE virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    SAL_DLLPRIVATE virtual void  StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

    using SvTreeListBox::Select;

public:
    SwDBTreeList(vcl::Window* pParent, WinBits nStyle);
    virtual ~SwDBTreeList() override;
    virtual void dispose() override;
    virtual Size GetOptimalSize() const override;

    OUString GetDBName( OUString& rTableName, OUString& rColumnName, sal_Bool* pbIsTable = nullptr);

    void    Select( const OUString& rDBName, const OUString& rTableName,
                    const OUString& rColumnName );

    void    ShowColumns(bool bShowCol);
    void    SetWrtShell(SwWrtShell& rSh);

    void    AddDataSource(const OUString& rSource);
};

class SW_DLLPUBLIC DBTreeList
{
    bool            bInitialized;
    bool            bShowColumns;

    rtl::Reference<SwDBTreeList_Impl> pImpl;
    std::unique_ptr<weld::TreeView> m_xTreeView;
#if 0

    DECL_DLLPRIVATE_LINK( DBCompare, const SvSortData&, sal_Int32 );
#endif
    DECL_DLLPRIVATE_LINK(RequestingChildrenHdl, const weld::TreeIter&, bool);
    SAL_DLLPRIVATE void          InitTreeList();

public:
    DBTreeList(std::unique_ptr<weld::TreeView> xTreeView);
    ~DBTreeList();

    OUString GetDBName(OUString& rTableName, OUString& rColumnName, sal_Bool* pbIsTable = nullptr);

    void    Select( const OUString& rDBName, const OUString& rTableName,
                    const OUString& rColumnName );

    void    SetWrtShell(SwWrtShell& rSh);

    void    AddDataSource(const OUString& rSource);

    void connect_changed(const Link<weld::TreeView&, void>& rLink) { m_xTreeView->connect_changed(rLink); }
    std::unique_ptr<weld::TreeIter> make_iterator(const weld::TreeIter* pOrig = nullptr) const { return m_xTreeView->make_iterator(pOrig); }
    bool get_selected(weld::TreeIter* pIter) const { return m_xTreeView->get_selected(pIter); }
    int get_iter_depth(const weld::TreeIter& rIter) const { return m_xTreeView->get_iter_depth(rIter); }
    void set_size_request(int nWidth, int nHeight) { m_xTreeView->set_size_request(nWidth, nHeight); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
