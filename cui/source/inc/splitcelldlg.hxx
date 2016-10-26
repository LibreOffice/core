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
#ifndef INCLUDED_CUI_SOURCE_INC_SPLITCELLDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_SPLITCELLDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>

#include <svx/stddlg.hxx>
#include <svx/svxdlg.hxx>

class SvxSplitTableDlg : public SvxAbstractSplittTableDialog
{
    VclPtr<SvxStandardDialog> m_pDialog;
    VclPtr<NumericField> m_pCountEdit;
    VclPtr<RadioButton>  m_pHorzBox;
    VclPtr<RadioButton>  m_pVertBox;
    VclPtr<CheckBox>     m_pPropCB;

    long                mnMaxVertical;
    long                mnMaxHorizontal;

public:
    SvxSplitTableDlg(vcl::Window *pParent, bool bIsTableVertical, long nMaxVertical, long nMaxHorizontal );
    virtual ~SvxSplitTableDlg() override;
    virtual void dispose() override;

    DECL_LINK( ClickHdl, Button *, void );

    virtual bool IsHorizontal() const override;
    virtual bool IsProportional() const override;
    virtual long GetCount() const override;

    virtual short Execute() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
