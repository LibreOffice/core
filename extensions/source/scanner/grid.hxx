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

#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>

class GridWindow;

enum class ResetType
{
    LINEAR_ASCENDING = 0,
    LINEAR_DESCENDING = 1,
    RESET = 2,
    EXPONENTIAL = 3
};

class GridDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Button> m_xOKButton;
    std::unique_ptr<weld::ComboBox> m_xResetTypeBox;
    std::unique_ptr<weld::Button> m_xResetButton;
    std::unique_ptr<GridWindow> m_xGridWindow;
    std::unique_ptr<weld::CustomWeld> m_xGridWindowWND;

    DECL_LINK(ClickButtonHdl, weld::Button&, void);

public:
    GridDialog(weld::Window* pParent, double* pXValues, double* pYValues, int nValues);
    virtual ~GridDialog() override;
    void setBoundings(double fMinX, double fMinY, double fMaxX, double fMaxY);
    double* getNewYValues();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
