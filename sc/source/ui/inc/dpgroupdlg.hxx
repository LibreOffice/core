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

#ifndef INCLUDED_SC_SOURCE_UI_INC_DPGROUPDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DPGROUPDLG_HXX

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

    DECL_LINK_TYPED( ClickHdl, Button*, void );

private:
    VclPtr<RadioButton>        mpRbAuto;
    VclPtr<RadioButton>        mpRbMan;
    VclPtr<Edit>               mpEdValue;
};

class ScDPNumGroupEditHelper : public ScDPGroupEditHelper
{
public:
    explicit            ScDPNumGroupEditHelper(
                            RadioButton* pRbAuto, RadioButton* pRbMan,
                            ScDoubleField* pEdValue );

    virtual             ~ScDPNumGroupEditHelper() {}
private:
    virtual bool        ImplGetValue( double& rfValue ) const override;
    virtual void        ImplSetValue( double fValue ) override;

private:
    VclPtr<ScDoubleField>      mpEdValue;
};

class ScDPDateGroupEditHelper : public ScDPGroupEditHelper
{
public:
    explicit            ScDPDateGroupEditHelper(
                            RadioButton* pRbAuto, RadioButton* pRbMan,
                            DateField* pEdValue, const Date& rNullDate );

    virtual ~ScDPDateGroupEditHelper() {}

private:
    virtual bool        ImplGetValue( double& rfValue ) const override;
    virtual void        ImplSetValue( double fValue ) override;

private:
    VclPtr<DateField>          mpEdValue;
    Date                maNullDate;
};

class ScDPNumGroupDlg : public ModalDialog
{
public:
    explicit            ScDPNumGroupDlg( vcl::Window* pParent, const ScDPNumGroupInfo& rInfo );
    virtual             ~ScDPNumGroupDlg();
    virtual void        dispose() override;
    ScDPNumGroupInfo    GetGroupInfo() const;

private:
    VclPtr<RadioButton>         mpRbAutoStart;
    VclPtr<RadioButton>         mpRbManStart;
    VclPtr<ScDoubleField>       mpEdStart;
    VclPtr<RadioButton>         mpRbAutoEnd;
    VclPtr<RadioButton>         mpRbManEnd;
    VclPtr<ScDoubleField>       mpEdEnd;
    VclPtr<ScDoubleField>       mpEdBy;
    ScDPNumGroupEditHelper maStartHelper;
    ScDPNumGroupEditHelper maEndHelper;
};

class ScDPDateGroupDlg : public ModalDialog
{
public:
    explicit            ScDPDateGroupDlg( vcl::Window* pParent, const ScDPNumGroupInfo& rInfo,
                            sal_Int32 nDatePart, const Date& rNullDate );
    virtual             ~ScDPDateGroupDlg();
    virtual void        dispose() override;
    ScDPNumGroupInfo    GetGroupInfo() const;
    sal_Int32           GetDatePart() const;

private:
    DECL_LINK_TYPED( ClickHdl, Button*, void );
    DECL_LINK_TYPED( CheckHdl, SvTreeListBox*, void );

private:
    VclPtr<RadioButton>         mpRbAutoStart;
    VclPtr<RadioButton>         mpRbManStart;
    VclPtr<DateField>           mpEdStart;
    VclPtr<RadioButton>         mpRbAutoEnd;
    VclPtr<RadioButton>         mpRbManEnd;
    VclPtr<DateField>           mpEdEnd;
    VclPtr<RadioButton>         mpRbNumDays;
    VclPtr<RadioButton>         mpRbUnits;
    VclPtr<NumericField>        mpEdNumDays;
    VclPtr<SvxCheckListBox>     mpLbUnits;
    VclPtr<OKButton>            mpBtnOk;
    ScDPDateGroupEditHelper maStartHelper;
    ScDPDateGroupEditHelper maEndHelper;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
