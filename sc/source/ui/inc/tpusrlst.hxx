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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TPUSRLST_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TPUSRLST_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/vclmedit.hxx>

class ScUserList;
class ScDocument;
class ScViewData;
class ScRangeUtil;

class ScTpUserLists : public SfxTabPage
{
    friend class VclPtr<ScTpUserLists>;
public:
    static  VclPtr<SfxTabPage> Create          ( vcl::Window*               pParent,
                                          const SfxItemSet*     rAttrSet );
    virtual bool        FillItemSet     ( SfxItemSet* rCoreAttrs ) override;
    virtual void        Reset           ( const SfxItemSet* rCoreAttrs ) override;
    using SfxTabPage::DeactivatePage;
    virtual DeactivateRC   DeactivatePage  ( SfxItemSet* pSet ) override;

private:
            ScTpUserLists( vcl::Window*              pParent,
                           const SfxItemSet&    rArgSet );
            virtual ~ScTpUserLists();
    virtual void dispose() override;

private:
    VclPtr<FixedText>          mpFtLists;
    VclPtr<ListBox>            mpLbLists;
    VclPtr<FixedText>          mpFtEntries;
    VclPtr<VclMultiLineEdit>   mpEdEntries;
    VclPtr<FixedText>          mpFtCopyFrom;
    VclPtr<Edit>               mpEdCopyFrom;

    VclPtr<PushButton>         mpBtnNew;
    VclPtr<PushButton>         mpBtnDiscard;

    VclPtr<PushButton>         mpBtnAdd;
    VclPtr<PushButton>         mpBtnModify;

    VclPtr<PushButton>         mpBtnRemove;

    VclPtr<PushButton>         mpBtnCopy;

    const OUString      aStrQueryRemove;
    const OUString      aStrCopyList;
    const OUString      aStrCopyFrom;
    const OUString      aStrCopyErr;

    const sal_uInt16    nWhichUserLists;
    ScUserList*     pUserLists;

    ScDocument*     pDoc;
    ScViewData*     pViewData;
    OUString        aStrSelectedArea;

    bool            bModifyMode;
    bool            bCancelMode;
    bool            bCopyDone;
    sal_Int32       nCancelPos;

    void    Init                ();
    size_t  UpdateUserListBox   ();
    void    UpdateEntries       ( size_t nList );
    static void MakeListStr     ( OUString& rListStr );
    void    AddNewList          ( const OUString& rEntriesStr );
    void    RemoveList          ( size_t nList );
    void    ModifyList          ( size_t          nSelList,
                                  const OUString& rEntriesStr );
    void    CopyListFromArea    ( const ScRefAddress& rStartPos,
                                  const ScRefAddress& rEndPos );

    // Handler:
    DECL_LINK_TYPED( LbSelectHdl, ListBox&, void );
    DECL_LINK_TYPED( BtnClickHdl, Button*, void );
    DECL_LINK_TYPED( EdEntriesModHdl, Edit&, void);
};

#endif // INCLUDED_SC_SOURCE_UI_INC_TPUSRLST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
