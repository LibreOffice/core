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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SELGLOS_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SELGLOS_HXX

#include <vcl/dialog.hxx>

#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>

#include <vcl/button.hxx>

class SwSelGlossaryDlg final : public ModalDialog
{
    VclPtr<ListBox>  m_pGlosBox;

    DECL_LINK(DoubleClickHdl, ListBox&, void);
public:
    SwSelGlossaryDlg(vcl::Window * pParent, const OUString &rShortName);
    virtual ~SwSelGlossaryDlg() override;
    virtual void dispose() override;

    void InsertGlos(const OUString &rRegion, const OUString &rGlosName)
    {
        const OUString aTmp = rRegion + ":" + rGlosName;
        m_pGlosBox->InsertEntry(aTmp);
    }
    sal_Int32 GetSelectedIdx() const
    {
        return m_pGlosBox->GetSelectedEntryPos();
    }
    void SelectEntryPos(sal_Int32 nIdx)
    {
        m_pGlosBox->SelectEntryPos(nIdx);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
