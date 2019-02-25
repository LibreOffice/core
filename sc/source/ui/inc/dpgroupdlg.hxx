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

#include <vcl/weld.hxx>
#include <dpnumgroupinfo.hxx>

class DoubleField;
class SvtCalendarBox;

class ScDPGroupEditHelper
{
public:
    explicit ScDPGroupEditHelper(weld::RadioButton& rRbAuto,
                                 weld::RadioButton& rRbMan,
                                 weld::Widget& rEdValue);

    bool                IsAuto() const;
    double              GetValue() const;
    void                SetValue( bool bAuto, double fValue );

protected:
    ~ScDPGroupEditHelper() {}

private:
    virtual bool        ImplGetValue( double& rfValue ) const = 0;
    virtual void        ImplSetValue( double fValue ) = 0;

    DECL_LINK(ClickHdl, weld::Button&, void);

private:
    weld::RadioButton& mrRbAuto;
    weld::RadioButton& mrRbMan;
    weld::Widget&      mrEdValue;
};

class ScDPNumGroupEditHelper : public ScDPGroupEditHelper
{
public:
    explicit ScDPNumGroupEditHelper(weld::RadioButton& rRbAuto,
                                    weld::RadioButton& rRbMan,
                                    DoubleField& rEdValue);

    virtual             ~ScDPNumGroupEditHelper() {}
private:
    virtual bool        ImplGetValue( double& rfValue ) const override;
    virtual void        ImplSetValue( double fValue ) override;

private:
    DoubleField&        mrEdValue;
};

class ScDPDateGroupEditHelper : public ScDPGroupEditHelper
{
public:
    explicit ScDPDateGroupEditHelper(weld::RadioButton& rRbAuto,
                                     weld::RadioButton& rRbMan,
                                     SvtCalendarBox& rEdValue,
                                     const Date& rNullDate);

    virtual ~ScDPDateGroupEditHelper() {}

private:
    virtual bool        ImplGetValue( double& rfValue ) const override;
    virtual void        ImplSetValue( double fValue ) override;

private:
    SvtCalendarBox&     mrEdValue;
    Date const          maNullDate;
};

class ScDPNumGroupDlg : public weld::GenericDialogController
{
public:
    explicit ScDPNumGroupDlg(weld::Window* pParent, const ScDPNumGroupInfo& rInfo);
    virtual ~ScDPNumGroupDlg() override;
    ScDPNumGroupInfo    GetGroupInfo() const;

private:
    std::unique_ptr<weld::RadioButton> mxRbAutoStart;
    std::unique_ptr<weld::RadioButton> mxRbManStart;
    std::unique_ptr<DoubleField> mxEdStart;
    std::unique_ptr<weld::RadioButton> mxRbAutoEnd;
    std::unique_ptr<weld::RadioButton> mxRbManEnd;
    std::unique_ptr<DoubleField> mxEdEnd;
    std::unique_ptr<DoubleField> mxEdBy;
    ScDPNumGroupEditHelper maStartHelper;
    ScDPNumGroupEditHelper maEndHelper;
};

class ScDPDateGroupDlg : public weld::GenericDialogController
{
public:
    explicit ScDPDateGroupDlg(weld::Window* pParent, const ScDPNumGroupInfo& rInfo,
                              sal_Int32 nDatePart, const Date& rNullDate);
    virtual ~ScDPDateGroupDlg() override;
    ScDPNumGroupInfo GetGroupInfo() const;
    sal_Int32 GetDatePart() const;

private:
    DECL_LINK(ClickHdl, weld::Button&, void);

    typedef std::pair<int, int> row_col;
    DECL_LINK(CheckHdl, const row_col&, void);

private:
    std::unique_ptr<weld::RadioButton> mxRbAutoStart;
    std::unique_ptr<weld::RadioButton> mxRbManStart;
    std::unique_ptr<SvtCalendarBox> mxEdStart;
    std::unique_ptr<weld::RadioButton> mxRbAutoEnd;
    std::unique_ptr<weld::RadioButton> mxRbManEnd;
    std::unique_ptr<SvtCalendarBox> mxEdEnd;
    std::unique_ptr<weld::RadioButton> mxRbNumDays;
    std::unique_ptr<weld::RadioButton> mxRbUnits;
    std::unique_ptr<weld::SpinButton> mxEdNumDays;
    std::unique_ptr<weld::TreeView> mxLbUnits;
    std::unique_ptr<weld::Button> mxBtnOk;
    ScDPDateGroupEditHelper maStartHelper;
    ScDPDateGroupEditHelper maEndHelper;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
