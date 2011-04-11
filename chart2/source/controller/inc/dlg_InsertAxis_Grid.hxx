/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
