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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_INPDLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_INPDLG_HXX

#include <svx/stddlg.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/vclmedit.hxx>

class SwInputField;
class SwSetExpField;
class SwUserFieldType;
class SwField;
class SwWrtShell;

// insert fields
class SwFieldInputDlg: public SvxStandardDialog
{
    virtual void    Apply() override;
    virtual void    StateChanged( StateChangedType ) override;

    SwWrtShell&       rSh;
    SwInputField*     pInpField;
    SwSetExpField*    pSetField;
    SwUserFieldType*  pUsrType;

    VclPtr<Edit>             m_pLabelED;

    VclPtr<VclMultiLineEdit> m_pEditED;

    VclPtr<OKButton>         m_pOKBT;
    VclPtr<PushButton>       m_pNextBT;

    DECL_LINK_TYPED(NextHdl, Button*, void);
public:
    SwFieldInputDlg(  vcl::Window *pParent, SwWrtShell &rSh,
                    SwField* pField, bool bNextButton = false );
    virtual ~SwFieldInputDlg();
    virtual void dispose() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
