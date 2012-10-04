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
#ifndef _SVX_INSRC_HXX
#define _SVX_INSRC_HXX

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include <svx/stddlg.hxx>

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>

class SvxInsRowColDlg : public SvxAbstractInsRowColDlg, public ModalDialog
{
    FixedText       aCount;
    NumericField    aCountEdit;
    FixedLine        aInsFL;

    RadioButton     aBeforeBtn;
    RadioButton     aAfterBtn;
    FixedLine        aPosFL;

    rtl::OUString   aRow;
    rtl::OUString   aCol;

    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;

    bool            bColumn;

public:
    SvxInsRowColDlg( Window* pParent, bool bCol, const rtl::OString& sHelpId );

    virtual short Execute(void);

    virtual bool isInsertBefore() const;
    virtual sal_uInt16 getInsertCount() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
