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

class ScDbTableDlg : public ScAnyRefDlgController
{
public:
    ScDbTableDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                ScViewData& rViewData);
    virtual ~ScDbTableDlg() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument& rDoc ) override;

    virtual bool    IsRefInputMode() const override;
    virtual void    SetActive() override;
    virtual void    Close() override;

private:
    bool            bInvalid;

    ScViewData&     m_rViewData;
    const ScDocument&     rDoc;
    bool            bRefInputMode;
    ScAddress::Details aAddrDetails;
    ScRange         theCurArea;

    std::unique_ptr<weld::Frame> m_xAssignFrame;
    std::unique_ptr<formula::RefEdit> m_xEdAssign;
    std::unique_ptr<formula::RefButton> m_xRbAssign;

    std::unique_ptr<weld::Widget> m_xOptions;
    std::unique_ptr<weld::CheckButton> m_xBtnHeader;

    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    std::shared_ptr<weld::MessageDialog> m_xInfoBox;

    void Init();
    void ErrorBox(const OUString& rString);

    DECL_LINK( CancelBtnHdl, weld::Button&, void );
    DECL_LINK( OkBtnHdl, weld::Button&, void );
    DECL_LINK( AssModifyHdl, formula::RefEdit&, void );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
