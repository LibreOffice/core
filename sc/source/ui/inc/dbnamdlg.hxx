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

#include <vector>
#include "anyrefdg.hxx"
#include <dbdata.hxx>

class ScViewData;
class ScDocument;

class ScDbNameDlg : public ScAnyRefDlgController
{
public:
    ScDbNameDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                ScViewData& rViewData);
    virtual ~ScDbNameDlg() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument& rDoc ) override;

    virtual bool    IsRefInputMode() const override;
    virtual void    SetActive() override;
    virtual void    Close() override;

private:
    bool            bSaved;
    bool            bInvalid;

    OUString        aStrAdd;
    OUString        aStrModify;
    OUString        aStrInvalid;

    OUString        aStrSource;
    OUString        aStrOperations;

    ScViewData&     m_rViewData;
    const ScDocument& rDoc;
    bool            bRefInputMode;
    ScAddress::Details aAddrDetails;

    ScDBCollection  aLocalDbCol;
    ScRange         theCurArea;
    std::vector<ScRange> aRemoveList;

    std::unique_ptr<weld::EntryTreeView> m_xEdName;

    std::unique_ptr<weld::Frame> m_xAssignFrame;
    std::unique_ptr<formula::RefEdit> m_xEdAssign;
    std::unique_ptr<formula::RefButton> m_xRbAssign;

    std::unique_ptr<weld::Widget> m_xOptions;
    std::unique_ptr<weld::CheckButton> m_xBtnHeader;
    std::unique_ptr<weld::CheckButton> m_xBtnTotals;
    std::unique_ptr<weld::CheckButton> m_xBtnDoSize;
    std::unique_ptr<weld::CheckButton> m_xBtnKeepFmt;
    std::unique_ptr<weld::CheckButton> m_xBtnStripData;
    std::unique_ptr<weld::Label> m_xFTSource;
    std::unique_ptr<weld::Label> m_xFTOperations;

    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;
    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnRemove;

    std::unique_ptr<weld::Button> m_xModifyPB;
    std::unique_ptr<weld::Label> m_xInvalidFT;

    std::unique_ptr<weld::Label> m_xFrameLabel;
    std::unique_ptr<weld::Expander> m_xExpander;
private:
    void            Init();
    void            UpdateNames();
    void            UpdateDBData( const OUString& rStrName );
    void            SetInfoStrings( const ScDBData* pDBData );

    DECL_LINK( CancelBtnHdl, weld::Button&, void );
    DECL_LINK( OkBtnHdl, weld::Button&, void );
    DECL_LINK( AddBtnHdl, weld::Button&, void );
    DECL_LINK( RemoveBtnHdl, weld::Button&, void );
    DECL_LINK( NameModifyHdl, weld::ComboBox&, void );
    DECL_LINK( AssModifyHdl, formula::RefEdit&, void );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
