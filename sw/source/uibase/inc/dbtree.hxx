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

#include <svtools/treelistbox.hxx>
#include <com/sun/star/uno/Reference.h>

#include "swdllapi.h"
#include <swtypes.hxx>

class SwDBTreeList_Impl;
class SwWrtShell;

class SW_DLLPUBLIC SwDBTreeList : public SvTreeListBox
{
    ImageList       aImageList;
    Image           aDBBMP;
    Image           aTableBMP;
    Image           aQueryBMP;

    OUString        sDefDBName;
    bool            bInitialized;
    bool            bShowColumns;

    SwDBTreeList_Impl* pImpl;

    DECL_DLLPRIVATE_LINK_TYPED( DBCompare, const SvSortData&, sal_Int32 );

    SAL_DLLPRIVATE void          InitTreeList();
    SAL_DLLPRIVATE virtual void  RequestingChildren( SvTreeListEntry* pParent ) override;

    SAL_DLLPRIVATE virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    SAL_DLLPRIVATE virtual void  StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

    using SvTreeListBox::Select;

public:
    SwDBTreeList(vcl::Window* pParent, WinBits nStyle);
    virtual ~SwDBTreeList();
    virtual void dispose() override;
    virtual Size GetOptimalSize() const override;

    OUString GetDBName( OUString& rTableName, OUString& rColumnName, sal_Bool* pbIsTable = 0);

    void    Select( const OUString& rDBName, const OUString& rTableName,
                    const OUString& rColumnName );

    void    ShowColumns(bool bShowCol);
    void    SetWrtShell(SwWrtShell& rSh);

    void    AddDataSource(const OUString& rSource);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
