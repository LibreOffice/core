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
    void            Apply() override;

    VclPtr<ListBox>         m_aEntryLB;
    VclPtr<OKButton>        m_aOKBtn;
    VclPtr<CancelButton>    m_aCancelBT;
    VclPtr<FixedText>       m_aLevelFT;
    VclPtr<Edit>            m_aEntryED;
    VclPtr<PushButton>      m_aEntryBT;
    VclPtr<PushButton>      m_aTabBT;
    VclPtr<PushButton>      m_aPageBT;
    VclPtr<PushButton>      m_aJumpBT;
    VclPtr<FixedLine>       m_aEntryFL;
    VclPtr<FixedText>       m_aLevelFT2;
    VclPtr<ListBox>         m_aLevelLB;
    VclPtr<FixedText>       m_aTemplateFT;
    VclPtr<ListBox>         m_aParaLayLB;
    VclPtr<PushButton>      m_aStdBT;
    VclPtr<PushButton>      m_aAssignBT;
    VclPtr<FixedLine>       m_aFormatFL;

public:
    SwIdxFormDlg( vcl::Window* pParent, SwWrtShell &rShell, const SwForm& rForm );
    virtual ~SwIdxFormDlg();
    virtual void dispose() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
