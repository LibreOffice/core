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
#ifndef INCLUDED_CUI_SOURCE_INC_INSRC_HXX
#define INCLUDED_CUI_SOURCE_INC_INSRC_HXX

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include <svx/stddlg.hxx>

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>

class SvxInsRowColDlg : public SvxAbstractInsRowColDlg
{
    VclPtr<ModalDialog>    m_pDialog;
    VclPtr<NumericField>   m_pCountEdit;

    VclPtr<RadioButton>    m_pBeforeBtn;
    VclPtr<RadioButton>    m_pAfterBtn;

    OUString   aRow;
    OUString   aCol;

    bool bColumn;

public:
    SvxInsRowColDlg( vcl::Window* pParent, bool bCol, const OString& sHelpId );
    virtual ~SvxInsRowColDlg() override;
    virtual void dispose() override;

    virtual short Execute() override;

    virtual bool isInsertBefore() const override;
    virtual sal_uInt16 getInsertCount() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
