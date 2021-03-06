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
#include <global.hxx>

class ScDeleteContentsDlg : public weld::GenericDialogController
{
private:
    bool m_bObjectsDisabled;

    std::unique_ptr<weld::CheckButton> m_xBtnDelAll;
    std::unique_ptr<weld::CheckButton> m_xBtnDelStrings;
    std::unique_ptr<weld::CheckButton> m_xBtnDelNumbers;
    std::unique_ptr<weld::CheckButton> m_xBtnDelDateTime;
    std::unique_ptr<weld::CheckButton> m_xBtnDelFormulas;
    std::unique_ptr<weld::CheckButton> m_xBtnDelNotes;
    std::unique_ptr<weld::CheckButton> m_xBtnDelAttrs;
    std::unique_ptr<weld::CheckButton> m_xBtnDelObjects;
    std::unique_ptr<weld::Button> m_xBtnOk;

    static bool bPreviousAllCheck;
    static InsertDeleteFlags nPreviousChecks;

    void DisableChecks(bool bDelAllChecked);
    DECL_LINK(DelAllHdl, weld::Button&, void);

public:
    ScDeleteContentsDlg(weld::Window* pParent);
    virtual ~ScDeleteContentsDlg() override;
    void DisableObjects();

    InsertDeleteFlags GetDelContentsCmdBits() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
