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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_FRMDLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_FRMDLG_HXX

#include <sfx2/tabdlg.hxx>
#include <sfx2/viewfrm.hxx>

class SwWrtShell;

// frame dialog
class SwFrameDlg : public SfxTabDialogController
{
    bool const m_bFormat;
    bool const m_bNew;
    const SfxItemSet&   m_rSet;
    OUString const m_sDlgType;
    SwWrtShell* m_pWrtShell;

    virtual void PageCreated(const OString& rId, SfxTabPage &rPage) override;

public:
    SwFrameDlg(SfxViewFrame const *pFrame, weld::Window* pParent,
               const SfxItemSet& rCoreSet,
               bool bNewFrame,
               const OUString& sResType,
               bool bFormat,
               const OString& sDefPage = OString(),
               const OUString* pFormatStr  = nullptr);

    virtual ~SwFrameDlg() override;

    SwWrtShell*  GetWrtShell()   { return m_pWrtShell; }
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_FRMDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
