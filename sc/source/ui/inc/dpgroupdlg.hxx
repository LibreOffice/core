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

#ifndef SC_DPGROUPDLG_HXX
#define SC_DPGROUPDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <svx/checklbx.hxx>
#include "editfield.hxx"
#include "dpgroup.hxx"



class ScDPGroupEditHelper
{
public:
    explicit            ScDPGroupEditHelper(
                            RadioButton& rRbAuto, RadioButton& rRbMan,
                            Edit& rEdValue );

    bool                IsAuto() const;
    double              GetValue() const;
    void                SetValue( bool bAuto, double fValue );

protected:
    ~ScDPGroupEditHelper() {}

private:
    virtual bool        ImplGetValue( double& rfValue ) const = 0;
    virtual void        ImplSetValue( double fValue ) = 0;

    DECL_LINK( ClickHdl, RadioButton* );

private:
    RadioButton&        mrRbAuto;
    RadioButton&        mrRbMan;
    Edit&               mrEdValue;
};



class ScDPNumGroupEditHelper : public ScDPGroupEditHelper
{
public:
    explicit            ScDPNumGroupEditHelper(
                            RadioButton& rRbAuto, RadioButton& rRbMan,
                            ScDoubleField& rEdValue );

    virtual ~ScDPNumGroupEditHelper() {}

private:
    virtual bool        ImplGetValue( double& rfValue ) const;
    virtual void        ImplSetValue( double fValue );

private:
    ScDoubleField&      mrEdValue;
};



class ScDPDateGroupEditHelper : public ScDPGroupEditHelper
{
public:
    explicit            ScDPDateGroupEditHelper(
                            RadioButton& rRbAuto, RadioButton& rRbMan,
                            DateField& rEdValue, const Date& rNullDate );

    virtual ~ScDPDateGroupEditHelper() {}

private:
    virtual bool        ImplGetValue( double& rfValue ) const;
    virtual void        ImplSetValue( double fValue );

private:
    DateField&          mrEdValue;
    Date                maNullDate;
};




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



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
