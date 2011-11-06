/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_DPGROUPDLG_HXX
#define SC_DPGROUPDLG_HXX

#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
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

