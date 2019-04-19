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

#include <vcl/weld.hxx>

class SwWrtShell;
class SwFieldMgr;
class SwScriptField;

namespace sfx2 { class FileDialogHelper; }

class SwJavaEditDialog : public weld::GenericDialogController
{
private:
    OUString       m_aText;
    OUString       m_aType;

    bool           m_bNew;
    bool           m_bIsUrl;

    SwScriptField*          m_pField;
    std::unique_ptr<SwFieldMgr>             m_pMgr;
    SwWrtShell*             m_pSh;
    std::unique_ptr<sfx2::FileDialogHelper> m_pFileDlg;

    std::unique_ptr<weld::Entry>        m_xTypeED;
    std::unique_ptr<weld::RadioButton>  m_xUrlRB;
    std::unique_ptr<weld::RadioButton>  m_xEditRB;
    std::unique_ptr<weld::Button>       m_xUrlPB;
    std::unique_ptr<weld::Entry>        m_xUrlED;
    std::unique_ptr<weld::TextView>     m_xEditED;
    std::unique_ptr<weld::Button>       m_xOKBtn;
    std::unique_ptr<weld::Button>       m_xPrevBtn;
    std::unique_ptr<weld::Button>       m_xNextBtn;

    DECL_LINK(OKHdl, weld::Button&, void);
    DECL_LINK(PrevHdl, weld::Button&, void);
    DECL_LINK(NextHdl, weld::Button&, void);
    DECL_LINK(RadioButtonHdl, weld::Button&, void);
    DECL_LINK(InsertFileHdl, weld::Button&, void);
    DECL_LINK(DlgClosedHdl, sfx2::FileDialogHelper *, void);

    void            CheckTravel();
    void            SetField();

public:
    SwJavaEditDialog(weld::Window* pParent, SwWrtShell* pWrtSh);
    virtual ~SwJavaEditDialog() override;

    const OUString& GetScriptText() const { return m_aText; }

    const OUString& GetScriptType() const { return m_aType; }

    bool IsUrl() const { return m_bIsUrl; }
    bool IsNew() const { return m_bNew; }
    bool IsUpdate() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
