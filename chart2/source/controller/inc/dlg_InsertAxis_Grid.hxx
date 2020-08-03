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
#include <com/sun/star/uno/Sequence.hxx>

namespace chart
{

struct InsertAxisOrGridDialogData
{
    css::uno::Sequence< sal_Bool > aPossibilityList;
    css::uno::Sequence< sal_Bool > aExistenceList;

    InsertAxisOrGridDialogData();
};

/*************************************************************************
|*
|* insert Axis dialog (also base for grid dialog)
|*
\************************************************************************/
class SchAxisDlg : public weld::GenericDialogController
{
protected:
    std::unique_ptr<weld::CheckButton> m_xCbPrimaryX;
    std::unique_ptr<weld::CheckButton> m_xCbPrimaryY;
    std::unique_ptr<weld::CheckButton> m_xCbPrimaryZ;
    std::unique_ptr<weld::CheckButton> m_xCbSecondaryX;
    std::unique_ptr<weld::CheckButton> m_xCbSecondaryY;
    std::unique_ptr<weld::CheckButton> m_xCbSecondaryZ;

public:
    SchAxisDlg(weld::Window* pParent, const InsertAxisOrGridDialogData& rInput, bool bAxisDlg = true);
    void getResult( InsertAxisOrGridDialogData& rOutput );
};

/*************************************************************************
|*
|* Grid dialog
|*
\************************************************************************/
class SchGridDlg : public SchAxisDlg
{
public:
    SchGridDlg(weld::Window* pParent, const InsertAxisOrGridDialogData& rInput);
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
