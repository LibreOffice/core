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
#ifndef _SELGLOS_HXX
#define _SELGLOS_HXX

#include <vcl/dialog.hxx>

#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>

#include <vcl/button.hxx>

class SwSelGlossaryDlg : public ModalDialog
{
protected:
    ListBox*  m_pGlosBox;

    DECL_LINK(DoubleClickHdl, ListBox*);
public:
    SwSelGlossaryDlg(Window * pParent, const OUString &rShortName);
    void InsertGlos(const OUString &rRegion, const OUString &rGlosName)
    {
        const OUString aTmp = rRegion + ":" + rGlosName;
        m_pGlosBox->InsertEntry(aTmp);
    }
    sal_uInt16 GetSelectedIdx() const
    {
        return m_pGlosBox->GetSelectEntryPos();
    }
    void SelectEntryPos(sal_uInt16 nIdx)
    {
        m_pGlosBox->SelectEntryPos(nIdx);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
