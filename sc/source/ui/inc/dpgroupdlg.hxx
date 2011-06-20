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

#ifndef SC_DPGROUPDLG_HXX
#define SC_DPGROUPDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <svx/checklbx.hxx>
#include "editfield.hxx"
#include "dpgroup.hxx"

// ============================================================================

class ScDPGroupEditHelper
{
public:
    explicit            ScDPGroupEditHelper(
                            RadioButton& rRbAuto, RadioButton& rRbMan,
                            Edit& rEdValue );

    bool                IsAuto() const;
    double              GetValue() const;
    void                SetValue( bool bAuto, double fValue );

private:
    virtual bool        ImplGetValue( double& rfValue ) const = 0;
    virtual void        ImplSetValue( double fValue ) = 0;

    DECL_LINK( ClickHdl, RadioButton* );

private:
    RadioButton&        mrRbAuto;
    RadioButton&        mrRbMan;
    Edit&               mrEdValue;
};

// ----------------------------------------------------------------------------

class ScDPNumGroupEditHelper : public ScDPGroupEditHelper
{
public:
    explicit            ScDPNumGroupEditHelper(
                            RadioButton& rRbAuto, RadioButton& rRbMan,
                            ScDoubleField& rEdValue );

private:
    virtual bool        ImplGetValue( double& rfValue ) const;
    virtual void        ImplSetValue( double fValue );

private:
    ScDoubleField&      mrEdValue;
};

// ----------------------------------------------------------------------------

class ScDPDateGroupEditHelper : public ScDPGroupEditHelper
{
public:
    explicit            ScDPDateGroupEditHelper(
                            RadioButton& rRbAuto, RadioButton& rRbMan,
                            DateField& rEdValue, const Date& rNullDate );

private:
    virtual bool        ImplGetValue( double& rfValue ) const;
    virtual void        ImplSetValue( double fValue );

private:
    DateField&          mrEdValue;
    Date                maNullDate;
};

// ============================================================================
// ============================================================================

class ScDPNumGroupDlg : public ModalDialog
{
public:
    explicit            ScDPNumGroupDlg( Window* pParent, const ScDPNumGroupInfo& rInfo );

    ScDPNumGroupInfo    GetGroupInfo() const;

private:
    FixedLine           maFlStart;
    RadioButton         maRbAutoStart;
    RadioButton         maRbManStart;
    ScDoubleField       maEdStart;
    FixedLine           maFlEnd;
    RadioButton         maRbAutoEnd;
    RadioButton         maRbManEnd;
    ScDoubleField       maEdEnd;
    FixedLine           maFlBy;
    ScDoubleField       maEdBy;
    OKButton            maBtnOk;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;
    ScDPNumGroupEditHelper maStartHelper;
    ScDPNumGroupEditHelper maEndHelper;
};

// ============================================================================

class ScDPDateGroupDlg : public ModalDialog
{
public:
    explicit            ScDPDateGroupDlg( Window* pParent, const ScDPNumGroupInfo& rInfo,
                            sal_Int32 nDatePart, const Date& rNullDate );

    ScDPNumGroupInfo    GetGroupInfo() const;
    sal_Int32           GetDatePart() const;

private:
    DECL_LINK( ClickHdl, RadioButton* );
    DECL_LINK( CheckHdl, SvxCheckListBox* );

private:
    FixedLine           maFlStart;
    RadioButton         maRbAutoStart;
    RadioButton         maRbManStart;
    DateField           maEdStart;
    FixedLine           maFlEnd;
    RadioButton         maRbAutoEnd;
    RadioButton         maRbManEnd;
    DateField           maEdEnd;
    FixedLine           maFlBy;
    RadioButton         maRbNumDays;
    RadioButton         maRbUnits;
    NumericField        maEdNumDays;
    SvxCheckListBox     maLbUnits;
    OKButton            maBtnOk;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;
    ScDPDateGroupEditHelper maStartHelper;
    ScDPDateGroupEditHelper maEndHelper;
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
