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

#pragma once

#include <vcl/weld.hxx>

class ScMetricInputDlg : public weld::GenericDialogController
{
public:
    ScMetricInputDlg( weld::Window*   pParent,
                      const OString& sDialogName,
                      tools::Long           nCurrent,
                      tools::Long           nDefault,
                      FieldUnit      eFUnit,
                      sal_uInt16     nDecimals,
                      tools::Long           nMaximum,
                      tools::Long           nMinimum);
    virtual ~ScMetricInputDlg() override;

    int GetInputValue() const;

private:
    std::unique_ptr<weld::MetricSpinButton> m_xEdValue;
    std::unique_ptr<weld::CheckButton> m_xBtnDefVal;
    int nDefaultValue;
    int nCurrentValue;

    DECL_LINK(SetDefValHdl, weld::Button&, void);
    DECL_LINK(ModifyHdl, weld::MetricSpinButton&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
