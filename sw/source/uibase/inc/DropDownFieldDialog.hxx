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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DROPDOWNFIELDDIALOG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DROPDOWNFIELDDIALOG_HXX

#include <svx/stddlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>

class SwDropDownField;
class SwField;
class SwWrtShell;

// Dialog to edit drop down field selection
namespace sw
{
class DropDownFieldDialog : public SvxStandardDialog
{
    VclPtr<ListBox>            m_pListItemsLB;

    VclPtr<OKButton>           m_pOKPB;
    VclPtr<PushButton>         m_pNextPB;

    VclPtr<PushButton>         m_pEditPB;

    SwWrtShell          &rSh;
    SwDropDownField*    pDropField;

    DECL_LINK_TYPED(ButtonHdl, Button*, void);
    virtual void    Apply() override;
public:
    DropDownFieldDialog(   vcl::Window *pParent, SwWrtShell &rSh,
                                SwField* pField, bool bNextButton = false );
    virtual ~DropDownFieldDialog();
    virtual void dispose() override;
};
} //namespace sw

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
