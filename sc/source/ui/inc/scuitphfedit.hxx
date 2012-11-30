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

#ifndef SCUI_TPHFEDIT_HXX
#define SCUI_TPHFEDIT_HXX

#include "tphfedit.hxx"

//===================================================================
enum ScHFEntryId
{
    eNoneEntry  ,
    ePageEntry  ,
    ePagesEntry ,
    eSheetEntry ,
    eConfidentialEntry ,
    eFileNamePageEntry ,
    eExtFileNameEntry ,
    ePageSheetEntry  ,
    ePageFileNameEntry  ,
    ePageExtFileNameEntry  ,
    eUserNameEntry  ,
    eCreatedByEntry ,
    eEntryCount
};

class EditTextObject;


class ScHFEditPage : public SfxTabPage
{
public:
    virtual sal_Bool    FillItemSet ( SfxItemSet& rCoreSet );
    virtual void    Reset       ( const SfxItemSet& rCoreSet );

    void            SetNumType(SvxNumType eNumType);
    void            ClearTextAreas();

protected:
                ScHFEditPage( Window*           pParent,
                              sal_uInt16            nResId,
                              const SfxItemSet& rCoreSet,
                              sal_uInt16            nWhich, bool bHeader );
    virtual     ~ScHFEditPage();

private:
    FixedText       aFtLeft;
    ScEditWindow    aWndLeft;
    FixedText       aFtCenter;
    ScEditWindow    aWndCenter;
    FixedText       aFtRight;
    ScEditWindow    aWndRight;
    FixedText       maFtDefinedHF;
        ListBox                 maLbDefined;
    FixedText       maFtCustomHF;
    ImageButton     aBtnText;
    ScExtIButton    aBtnFile;
    ImageButton     aBtnTable;
    ImageButton     aBtnPage;
    ImageButton     aBtnLastPage;
    ImageButton     aBtnDate;
    ImageButton     aBtnTime;
    FixedLine       aFlInfo;
    FixedInfo       aFtInfo;
    ScPopupMenu     aPopUpFile;

    sal_uInt16          nWhich;
    String          aCmdArr[6];

private:
#ifdef _TPHFEDIT_CXX
    void FillCmdArr();
    void InitPreDefinedList();
    void ProcessDefinedListSel(ScHFEntryId eSel, bool bTravelling = false);
    void InsertToDefinedList();
    void RemoveFromDefinedList();
    void SetSelectDefinedList();
    bool IsPageEntry(EditEngine*pEngine, EditTextObject* pTextObj);
    bool IsDateEntry(EditTextObject* pTextObj);
    bool IsExtFileNameEntry(EditTextObject* pTextObj);
    DECL_LINK( ListHdl_Impl, ListBox* );
    DECL_LINK( ClickHdl, ImageButton* );
    DECL_LINK( MenuHdl, ScExtIButton* );
#endif
};

//===================================================================

class ScRightHeaderEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rCoreSet );

private:
    ScRightHeaderEditPage( Window* pParent, const SfxItemSet& rSet );
};

//===================================================================

class ScLeftHeaderEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rCoreSet );

private:
    ScLeftHeaderEditPage( Window* pParent, const SfxItemSet& rSet );
};

//===================================================================

class ScRightFooterEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rCoreSet );

private:
    ScRightFooterEditPage( Window* pParent, const SfxItemSet& rSet );
};

//===================================================================

class ScLeftFooterEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rCoreSet );

private:
    ScLeftFooterEditPage( Window* pParent, const SfxItemSet& rSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
