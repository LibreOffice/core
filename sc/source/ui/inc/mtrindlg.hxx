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

#ifndef INCLUDED_SC_SOURCE_UI_INC_MTRINDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_MTRINDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

class ScMetricInputDlg : public ModalDialog
{
public:
    ScMetricInputDlg( vcl::Window*        pParent,
                      const OString& sDialogName,
                      long           nCurrent,
                      long           nDefault,
                      FieldUnit      eFUnit    = FUNIT_MM,
                      sal_uInt16     nDecimals = 2,
                      long           nMaximum  = 1000,
                      long           nMinimum  = 0,
                      long           nFirst    = 1,
                      long           nLast     = 100 );
    virtual ~ScMetricInputDlg();
    virtual void dispose() override;

    long GetInputValue( FieldUnit eUnit = FUNIT_TWIP ) const;

private:
    VclPtr<MetricField>    m_pEdValue;
    VclPtr<CheckBox>       m_pBtnDefVal;
    long            nDefaultValue;
    long            nCurrentValue;

    DECL_LINK_TYPED(SetDefValHdl, Button*, void);
    DECL_LINK(ModifyHdl, void *);
};

#endif // INCLUDED_SC_SOURCE_UI_INC_MTRINDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
