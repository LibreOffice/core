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

#ifndef INCLUDED_SC_SOURCE_UI_INC_SCUITPHFEDIT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SCUITPHFEDIT_HXX

#include "tphfedit.hxx"

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
class EditEngine;

class ScHFEditPage : public SfxTabPage
{
public:
    virtual bool    FillItemSet ( SfxItemSet* rCoreSet ) SAL_OVERRIDE;
    virtual void        Reset       ( const SfxItemSet* rCoreSet ) SAL_OVERRIDE;

    void            SetNumType(SvxNumType eNumType);
    void            ClearTextAreas();

protected:
                ScHFEditPage( vcl::Window*           pParent,
                              const SfxItemSet& rCoreSet,
                              sal_uInt16        nWhich,
                              bool              bHeader );
    virtual     ~ScHFEditPage();
    virtual void dispose() SAL_OVERRIDE;

private:
    VclPtr<ScEditWindow>    m_pWndLeft;
    VclPtr<ScEditWindow>    m_pWndCenter;
    VclPtr<ScEditWindow>    m_pWndRight;
    VclPtr<FixedText>       m_pFtDefinedHF;
    VclPtr<ListBox>         m_pLbDefined;
    VclPtr<FixedText>       m_pFtCustomHF;
    VclPtr<PushButton>      m_pBtnText;
    VclPtr<ScExtIButton>    m_pBtnFile;
    VclPtr<PushButton>      m_pBtnTable;
    VclPtr<PushButton>      m_pBtnPage;
    VclPtr<PushButton>      m_pBtnLastPage;
    VclPtr<PushButton>      m_pBtnDate;
    VclPtr<PushButton>      m_pBtnTime;

    VclPtr<FixedText>       m_pFtConfidential;
    VclPtr<FixedText>       m_pFtPage;
    VclPtr<FixedText>       m_pFtOfQuestion;
    VclPtr<FixedText>       m_pFtOf;
    VclPtr<FixedText>       m_pFtNone;
    VclPtr<FixedText>       m_pFtCreatedBy;
    VclPtr<FixedText>       m_pFtCustomized;

    sal_uInt16       nWhich;
    OUString         aCmdArr[6];

    DECL_LINK_TYPED( ObjectSelectHdl, ScEditWindow&, void );

private:
    void FillCmdArr();
    void InitPreDefinedList();
    void ProcessDefinedListSel(ScHFEntryId eSel, bool bTravelling = false);
    void InsertToDefinedList();
    void RemoveFromDefinedList();
    void SetSelectDefinedList();
    bool IsPageEntry(EditEngine*pEngine, EditTextObject* pTextObj);
    static bool IsDateEntry(EditTextObject* pTextObj);
    static bool IsExtFileNameEntry(EditTextObject* pTextObj);
    DECL_LINK( ListHdl_Impl, ListBox* );
    DECL_LINK_TYPED( ClickHdl, Button*, void );
    DECL_STATIC_LINK( ScHFEditPage, MenuHdl, ScExtIButton* );
};

class ScRightHeaderEditPage : public ScHFEditPage
{
    friend class VclPtr<ScRightHeaderEditPage>;
public:
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rCoreSet );

private:
    ScRightHeaderEditPage( vcl::Window* pParent, const SfxItemSet& rSet );
};

class ScLeftHeaderEditPage : public ScHFEditPage
{
    friend class VclPtr<ScLeftHeaderEditPage>;
public:
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rCoreSet );

private:
    ScLeftHeaderEditPage( vcl::Window* pParent, const SfxItemSet& rSet );
};

class ScRightFooterEditPage : public ScHFEditPage
{
    friend class VclPtr<ScRightFooterEditPage>;
public:
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rCoreSet );

private:
    ScRightFooterEditPage( vcl::Window* pParent, const SfxItemSet& rSet );
};

class ScLeftFooterEditPage : public ScHFEditPage
{
    friend class VclPtr<ScLeftFooterEditPage>;
public:
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rCoreSet );

private:
    ScLeftFooterEditPage( vcl::Window* pParent, const SfxItemSet& rSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
