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
#ifndef INCLUDED_CUI_SOURCE_INC_NEWTABLEDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_NEWTABLEDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>

#include <svx/stddlg.hxx>
#include <svx/svxdlg.hxx>

class SvxNewTableDialog : public SvxAbstractNewTableDialog, public ModalDialog
{
private:
    NumericField*       mpNumColumns;
    NumericField*       mpNumRows;

public:
    SvxNewTableDialog( Window* pWindow );

    virtual short Execute(void) SAL_OVERRIDE;
    virtual void Apply(void);

    virtual sal_Int32 getRows() const SAL_OVERRIDE;
    virtual sal_Int32 getColumns() const SAL_OVERRIDE;
};

#endif // INCLUDED_CUI_SOURCE_INC_NEWTABLEDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
