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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SPLITTBL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SPLITTBL_HXX

#include <svx/stddlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>

class SwWrtShell;

class SwSplitTableDlg : public SvxStandardDialog
{
    VclPtr<RadioButton>    mpContentCopyRB;
    VclPtr<RadioButton>    mpBoxAttrCopyWithParaRB ;
    VclPtr<RadioButton>    mpBoxAttrCopyNoParaRB ;
    VclPtr<RadioButton>    mpBorderCopyRB;

    SwWrtShell      &rShell;
    sal_uInt16          m_nSplit;

protected:
    virtual void Apply() override;

public:
    SwSplitTableDlg( vcl::Window *pParent, SwWrtShell &rSh );
    virtual ~SwSplitTableDlg();
    virtual void dispose() override;

    sal_uInt16 GetSplitMode() const { return m_nSplit; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
