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

#include <sfx2/tabdlg.hxx>
#include <vcl/weld.hxx>

class ScSortDlg : public SfxTabDialogController
{
public:
    ScSortDlg(weld::Window* pParent, const SfxItemSet* pArgSet);

    void SetHeaders( bool bHeaders )  { bIsHeaders = bHeaders; }
    void SetByRows ( bool bByRows  )  { bIsByRows = bByRows; }
    bool GetHeaders() const           { return bIsHeaders; }
    bool GetByRows () const           { return bIsByRows; }

private:
    bool    bIsHeaders;
    bool    bIsByRows;
};

class ScSortWarningDlg : public weld::GenericDialogController
{
public:
    ScSortWarningDlg(weld::Window* pParent, std::u16string_view rExtendText, std::u16string_view rCurrentText);
    virtual ~ScSortWarningDlg() override;
    DECL_LINK(BtnHdl, weld::Button&, void);
private:
    std::unique_ptr<weld::Label> m_xFtText;
    std::unique_ptr<weld::Button> m_xBtnExtSort;
    std::unique_ptr<weld::Button> m_xBtnCurSort;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
