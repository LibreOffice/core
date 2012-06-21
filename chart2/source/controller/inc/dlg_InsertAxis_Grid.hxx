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
#ifndef _CHART2_DLG_INSERT_AXIS_GRID_HXX
#define _CHART2_DLG_INSERT_AXIS_GRID_HXX

// header for class ModalDialog
#include <vcl/dialog.hxx>
// header for class FixedLine
#include <vcl/fixed.hxx>
// header for class CheckBox
#include <vcl/button.hxx>
#include <com/sun/star/uno/Sequence.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

struct InsertAxisOrGridDialogData
{
    ::com::sun::star::uno::Sequence< sal_Bool > aPossibilityList;
    ::com::sun::star::uno::Sequence< sal_Bool > aExistenceList;

    InsertAxisOrGridDialogData();
};

/*************************************************************************
|*
|* insert Axis dialog (also base for grid dialog)
|*
\************************************************************************/
class SchAxisDlg : public ModalDialog
{
protected:
    FixedLine           aFlPrimary;
    FixedLine           aFlPrimaryGrid;
    CheckBox            aCbPrimaryX;
    CheckBox            aCbPrimaryY;
    CheckBox            aCbPrimaryZ;

    FixedLine           aFlSecondary;
    FixedLine           aFlSecondaryGrid;
    CheckBox            aCbSecondaryX;
    CheckBox            aCbSecondaryY;
    CheckBox            aCbSecondaryZ;

    OKButton            aPbOK;
    CancelButton        aPbCancel;
    HelpButton          aPbHelp;

public:
    SchAxisDlg( Window* pParent, const InsertAxisOrGridDialogData& rInput, sal_Bool bAxisDlg=true );
    virtual ~SchAxisDlg();

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
    SchGridDlg( Window* pParent, const InsertAxisOrGridDialogData& rInput );
    virtual ~SchGridDlg();
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
