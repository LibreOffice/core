/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dpgroupdlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:21:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SVX_CHECKLBX_HXX
#include <svx/checklbx.hxx>
#endif
#ifndef SC_EDITFIELD_HXX
#include "editfield.hxx"
#endif

#ifndef SC_DPGROUP_HXX
#include "dpgroup.hxx"
#endif

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

