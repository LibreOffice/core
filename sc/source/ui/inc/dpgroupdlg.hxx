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
                            RadioButton* rRbAuto, RadioButton* rRbMan,
                            Edit* rEdValue );

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
    RadioButton*        mpRbAuto;
    RadioButton*        mpRbMan;
    Edit*               mpEdValue;
};



class ScDPNumGroupEditHelper : public ScDPGroupEditHelper
{
public:
    explicit            ScDPNumGroupEditHelper(
                            RadioButton* pRbAuto, RadioButton* pRbMan,
                            ScDoubleField* pEdValue );

    virtual ~ScDPNumGroupEditHelper() {}

private:
    virtual bool        ImplGetValue( double& rfValue ) const SAL_OVERRIDE;
    virtual void        ImplSetValue( double fValue ) SAL_OVERRIDE;

private:
    ScDoubleField*      mpEdValue;
};



class ScDPDateGroupEditHelper : public ScDPGroupEditHelper
{
public:
    explicit            ScDPDateGroupEditHelper(
                            RadioButton* pRbAuto, RadioButton* pRbMan,
                            DateField* pEdValue, const Date& rNullDate );

    virtual ~ScDPDateGroupEditHelper() {}

private:
    virtual bool        ImplGetValue( double& rfValue ) const SAL_OVERRIDE;
    virtual void        ImplSetValue( double fValue ) SAL_OVERRIDE;

private:
    DateField*          mpEdValue;
    Date                maNullDate;
};




class ScDPNumGroupDlg : public ModalDialog
{
public:
    explicit            ScDPNumGroupDlg( Window* pParent, const ScDPNumGroupInfo& rInfo );

    ScDPNumGroupInfo    GetGroupInfo() const;

private:
    RadioButton*         mpRbAutoStart;
    RadioButton*         mpRbManStart;
    ScDoubleField*       mpEdStart;
    RadioButton*         mpRbAutoEnd;
    RadioButton*         mpRbManEnd;
    ScDoubleField*       mpEdEnd;
    ScDoubleField*       mpEdBy;
    OKButton*            mpBtnOk;
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
    RadioButton*         mpRbAutoStart;
    RadioButton*         mpRbManStart;
    DateField*           mpEdStart;
    RadioButton*         mpRbAutoEnd;
    RadioButton*         mpRbManEnd;
    DateField*           mpEdEnd;
    RadioButton*         mpRbNumDays;
    RadioButton*         mpRbUnits;
    NumericField*        mpEdNumDays;
    SvxCheckListBox*     mpLbUnits;
    OKButton*            mpBtnOk;
    ScDPDateGroupEditHelper maStartHelper;
    ScDPDateGroupEditHelper maEndHelper;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
