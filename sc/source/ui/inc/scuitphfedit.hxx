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

#pragma once

#include "tphfedit.hxx"
#include <sfx2/tabdlg.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>

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
    virtual bool    FillItemSet ( SfxItemSet* rCoreSet ) override;
    virtual void        Reset       ( const SfxItemSet* rCoreSet ) override;

    void            SetNumType(SvxNumType eNumType);
    void            ClearTextAreas();

protected:
                ScHFEditPage( weld::Container* pPage, weld::DialogController* pController,
                              const SfxItemSet& rCoreSet,
                              sal_uInt16        nWhich,
                              bool              bHeader );
    virtual     ~ScHFEditPage() override;

private:
    sal_uInt16 nWhich;
    bool m_bDropDownActive;
    sal_Int64 m_nTimeToggled;

    std::unique_ptr<weld::Label> m_xFtDefinedHF;
    std::unique_ptr<weld::ComboBox> m_xLbDefined;
    std::unique_ptr<weld::Label> m_xFtCustomHF;
    std::unique_ptr<weld::Button> m_xBtnText;
    std::unique_ptr<weld::MenuButton> m_xBtnFile;
    std::unique_ptr<weld::Button> m_xBtnTable;
    std::unique_ptr<weld::Button> m_xBtnPage;
    std::unique_ptr<weld::Button> m_xBtnLastPage;
    std::unique_ptr<weld::Button> m_xBtnDate;
    std::unique_ptr<weld::Button> m_xBtnTime;

    std::unique_ptr<weld::Label> m_xFtConfidential;
    std::unique_ptr<weld::Label> m_xFtPage;
    std::unique_ptr<weld::Label> m_xFtOfQuestion;
    std::unique_ptr<weld::Label> m_xFtOf;
    std::unique_ptr<weld::Label> m_xFtNone;
    std::unique_ptr<weld::Label> m_xFtCreatedBy;
    std::unique_ptr<weld::Label> m_xFtCustomized;

    std::unique_ptr<weld::Widget> m_xLeft;
    std::unique_ptr<weld::Widget> m_xRight;

    std::unique_ptr<ScEditWindow> m_xWndLeft;
    std::unique_ptr<ScEditWindow> m_xWndCenter;
    std::unique_ptr<ScEditWindow> m_xWndRight;
    std::unique_ptr<weld::CustomWeld> m_xWndLeftWnd;
    std::unique_ptr<weld::CustomWeld> m_xWndCenterWnd;
    std::unique_ptr<weld::CustomWeld> m_xWndRightWnd;

    ScEditWindow * m_pEditFocus; ///one of m_pWndLeft, m_pWndCenter, m_pWndRight

    DECL_LINK( ObjectSelectHdl, ScEditWindow&, void );

private:
    void InitPreDefinedList();
    void ProcessDefinedListSel(ScHFEntryId eSel, bool bTravelling);
    void InsertToDefinedList();
    void RemoveFromDefinedList();
    void SetSelectDefinedList();
    bool IsPageEntry(EditEngine*pEngine, const EditTextObject* pTextObj);
    static bool IsDateEntry(const EditTextObject* pTextObj);
    static bool IsExtFileNameEntry(const EditTextObject* pTextObj);
    DECL_LINK( ListHdl_Impl, weld::ComboBox&, void);
    DECL_LINK( ListToggleHdl_Impl, weld::ComboBox&, void);
    DECL_LINK( ClickHdl, weld::Button&, void );
    DECL_LINK( MenuHdl, const OString&, void );
};

class ScRightHeaderEditPage : public ScHFEditPage
{
public:
    static std::unique_ptr<SfxTabPage>  Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet );
    ScRightHeaderEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet );
};

class ScLeftHeaderEditPage : public ScHFEditPage
{
public:
    static std::unique_ptr<SfxTabPage>  Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet );
    ScLeftHeaderEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet );
};

class ScRightFooterEditPage : public ScHFEditPage
{
public:
    static std::unique_ptr<SfxTabPage>  Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet );
    ScRightFooterEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet );
};

class ScLeftFooterEditPage : public ScHFEditPage
{
public:
    static std::unique_ptr<SfxTabPage>  Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet );
    ScLeftFooterEditPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
