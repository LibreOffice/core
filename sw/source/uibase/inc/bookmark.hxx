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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_BOOKMARK_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_BOOKMARK_HXX

#include <svx/stddlg.hxx>
#include <vcl/fixed.hxx>
#include <ndtxt.hxx>

#include <vcl/button.hxx>
#include <svtools/simptabl.hxx>
#include <pam.hxx>

#include "swlbox.hxx"
#include "IMark.hxx"

class SwWrtShell;
class SfxRequest;

class BookmarkTable : public SvSimpleTable
{
    /*virtual bool    PreNotify(NotifyEvent& rNEvt) override;*/
    SvTreeListEntry*    GetRowByBookmarkName(OUString name);
public:
    BookmarkTable(SvSimpleTableContainer& rParent, WinBits nStyle);
    void                InsertBookmark(sw::mark::IMark* mark);
    sw::mark::IMark*    GetBookmarkByName(OUString name);
    void                SelectByName(OUString sName);

    static const OUString aForbiddenChars;
};

class SwInsertBookmarkDlg: public SvxStandardDialog
{
    VclPtr<SvSimpleTableContainer>      m_BookmarksContainer;
    VclPtr<BookmarkTable>               m_pBookmarksBox;
    VclPtr<Edit>                        m_pEditBox;
    VclPtr<OKButton>                    m_pOkBtn;
    VclPtr<PushButton>                  m_pDeleteBtn;
    VclPtr<PushButton>                  m_pGotoBtn;


    OUString        sRemoveWarning;
    SwWrtShell      &rSh;
    SfxRequest&     rReq;

    DECL_LINK_TYPED(ModifyHdl, Edit&, void);
    DECL_LINK_TYPED(DeleteHdl, Button*, void);
    DECL_LINK_TYPED(GotoHdl, Button*, void);
    DECL_LINK_TYPED(SelectionChangedHdl, SvTreeListBox*, void);

    virtual void Apply() override;

public:
    SwInsertBookmarkDlg(vcl::Window *pParent, SwWrtShell &rSh, SfxRequest& rReq);
    virtual ~SwInsertBookmarkDlg();
    virtual void dispose() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
