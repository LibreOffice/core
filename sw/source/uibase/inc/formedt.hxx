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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_FORMEDT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_FORMEDT_HXX

#include <svx/stddlg.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/button.hxx>

#include <vcl/fixed.hxx>

#include <vcl/edit.hxx>

#include "toxe.hxx"

class SwWrtShell;
class SwForm;

// insert marker for index entry
class SwIdxFormDlg : public SvxStandardDialog
{
    void            Apply() SAL_OVERRIDE;

    VclPtr<ListBox>         aEntryLB;
    VclPtr<OKButton>        aOKBtn;
    VclPtr<CancelButton>    aCancelBT;
    VclPtr<FixedText>       aLevelFT;
    VclPtr<Edit>            aEntryED;
    VclPtr<PushButton>      aEntryBT;
    VclPtr<PushButton>      aTabBT;
    VclPtr<PushButton>      aPageBT;
    VclPtr<PushButton>      aJumpBT;
    VclPtr<FixedLine>       aEntryFL;
    VclPtr<FixedText>       aLevelFT2;
    VclPtr<ListBox>         aLevelLB;
    VclPtr<FixedText>       aTemplateFT;
    VclPtr<ListBox>         aParaLayLB;
    VclPtr<PushButton>      aStdBT;
    VclPtr<PushButton>      aAssignBT;
    VclPtr<FixedLine>       aFormatFL;

    SwWrtShell     &rSh;
    SwForm         *pForm;
    sal_uInt16          nAktLevel;
    bool            bLastLinkIsEnd;

public:
    SwIdxFormDlg( vcl::Window* pParent, SwWrtShell &rShell, const SwForm& rForm );
    virtual ~SwIdxFormDlg();
    virtual void dispose() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
