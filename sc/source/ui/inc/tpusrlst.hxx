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

#ifndef SC_TPUSRLST_HXX
#define SC_TPUSRLST_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/svmedit.hxx>

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
    FixedText       aFtLists;
    ListBox         aLbLists;
    FixedText       aFtEntries;
    MultiLineEdit   aEdEntries;
    FixedText       aFtCopyFrom;
    Edit            aEdCopyFrom;
    PushButton      aBtnNew;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    PushButton      aBtnCopy;

    const String    aStrQueryRemove;
    const String    aStrNew;
    const String    aStrCancel;
    const String    aStrAdd;
    const String    aStrModify;
    const String    aStrCopyList;
    const String    aStrCopyFrom;
    const String    aStrCopyErr;

    const sal_uInt16    nWhichUserLists;
    ScUserList*     pUserLists;

    ScDocument*     pDoc;
    ScViewData*     pViewData;
    ScRangeUtil*    pRangeUtil;
    String          aStrSelectedArea;

    sal_Bool            bModifyMode;
    sal_Bool            bCancelMode;
    sal_Bool            bCopyDone;
    sal_uInt16          nCancelPos;

#ifdef _TPUSRLST_CXX
private:
    void    Init                ();
    sal_uInt16  UpdateUserListBox   ();
    void    UpdateEntries       ( size_t nList );
    void    MakeListStr         ( String& rListStr );
    void    AddNewList          ( const String& rEntriesStr );
    void    RemoveList          ( size_t nList );
    void    ModifyList          ( sal_uInt16        nSelList,
                                  const String& rEntriesStr );
    void    CopyListFromArea    ( const ScRefAddress& rStartPos,
                                  const ScRefAddress& rEndPos );

    // Handler:
    DECL_LINK( LbSelectHdl,     ListBox* );
    DECL_LINK( BtnClickHdl,     PushButton* );
    DECL_LINK( EdEntriesModHdl, MultiLineEdit* );
#endif
};



#endif // SC_TPUSRLST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
