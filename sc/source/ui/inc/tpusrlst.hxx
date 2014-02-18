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

#ifndef SC_TPUSRLST_HXX
#define SC_TPUSRLST_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/vclmedit.hxx>

//========================================================================

class ScUserList;
class ScDocument;
class ScViewData;
class ScRangeUtil;

class ScTpUserLists : public SfxTabPage
{
public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rAttrSet );
    virtual sal_Bool        FillItemSet     ( SfxItemSet& rCoreAttrs );
    virtual void        Reset           ( const SfxItemSet& rCoreAttrs );
    using SfxTabPage::DeactivatePage;
    virtual int         DeactivatePage  ( SfxItemSet* pSet = NULL );

private:
            ScTpUserLists( Window*              pParent,
                           const SfxItemSet&    rArgSet );
            ~ScTpUserLists();

private:
    FixedText*          mpFtLists;
    ListBox*            mpLbLists;
    FixedText*          mpFtEntries;
    VclMultiLineEdit*   mpEdEntries;
    FixedText*          mpFtCopyFrom;
    Edit*               mpEdCopyFrom;

    PushButton*         mpBtnNew;
    PushButton*         mpBtnDiscard;

    PushButton*         mpBtnAdd;
    PushButton*         mpBtnModify;

    PushButton*         mpBtnRemove;

    PushButton*         mpBtnCopy;

    const OUString      aStrQueryRemove;
    const OUString      aStrCopyList;
    const OUString      aStrCopyFrom;
    const OUString      aStrCopyErr;

    const sal_uInt16    nWhichUserLists;
    ScUserList*     pUserLists;

    ScDocument*     pDoc;
    ScViewData*     pViewData;
    ScRangeUtil*    pRangeUtil;
    OUString        aStrSelectedArea;

    bool            bModifyMode;
    bool            bCancelMode;
    bool            bCopyDone;
    sal_uInt16      nCancelPos;

#ifdef _TPUSRLST_CXX
private:
    void    Init                ();
    sal_uInt16  UpdateUserListBox   ();
    void    UpdateEntries       ( size_t nList );
    void    MakeListStr         ( OUString& rListStr );
    void    AddNewList          ( const OUString& rEntriesStr );
    void    RemoveList          ( size_t nList );
    void    ModifyList          ( sal_uInt16        nSelList,
                                  const OUString& rEntriesStr );
    void    CopyListFromArea    ( const ScRefAddress& rStartPos,
                                  const ScRefAddress& rEndPos );

    // Handler:
    DECL_LINK( LbSelectHdl,     ListBox* );
    DECL_LINK( BtnClickHdl,     PushButton* );
    DECL_LINK( EdEntriesModHdl, VclMultiLineEdit* );
#endif
};



#endif // SC_TPUSRLST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
