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
#ifndef INCLUDED_EXTENSIONS_SOURCE_SCANNER_GRID_HXX
#define INCLUDED_EXTENSIONS_SOURCE_SCANNER_GRID_HXX

#include <vcl/window.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>

class GridWindow;

enum resetType
{
    LINEAR_ASCENDING = 0,
    LINEAR_DESCENDING = 1,
    RESET = 2,
    EXPONENTIAL = 3
};

class GridDialog : public ModalDialog
{
    VclPtr<OKButton>       m_pOKButton;

    VclPtr<ListBox>        m_pResetTypeBox;
    VclPtr<PushButton>     m_pResetButton;

    VclPtr<GridWindow>     m_pGridWindow;

    DECL_LINK_TYPED( ClickButtonHdl, Button*, void );

public:
    GridDialog(double* pXValues, double* pYValues, int nValues,
                vcl::Window* pParent, bool bCutValues = true);
    virtual ~GridDialog();
    virtual void dispose() override;
    void setBoundings(double fMinX, double fMinY, double fMaxX, double fMaxY);
    double* getNewYValues();
};

#endif // INCLUDED_EXTENSIONS_SOURCE_SCANNER_GRID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
