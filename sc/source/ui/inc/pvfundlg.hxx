/*************************************************************************
 *
 *  $RCSfile: pvfundlg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 13:00:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_PVFUNDLG_HXX
#define SC_PVFUNDLG_HXX

#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDREFERENCE_HPP_
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDSORTINFO_HPP_
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#endif

#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _MOREBTN_HXX
#include <vcl/morebtn.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SVX_CHECKLBX_HXX
#include <svx/checklbx.hxx>
#endif
#ifndef SFX_ITEMCONNECT_HXX
#include <sfx2/itemconnect.hxx>
#endif

#ifndef SC_PIVOT_HXX
#include "pivot.hxx"
#endif

// ============================================================================

typedef sfx::ListBoxWrapper< sal_Int32 > ScDPListBoxWrapper;

class ScDPObject;

// ============================================================================

class ScDPFunctionListBox : public MultiListBox
{
public:
    explicit            ScDPFunctionListBox( Window* pParent, const ResId& rResId );
    explicit            ScDPFunctionListBox( Window* pParent, WinBits nStyle = 0 );

    void                SetSelection( USHORT nFuncMask );
    USHORT              GetSelection() const;

private:
    void                FillFunctionNames();
};

// ============================================================================

class ScDPFunctionDlg : public ModalDialog
{
public:
    explicit            ScDPFunctionDlg( Window* pParent, const ScDPLabelDataVec& rLabelVec,
                            const ScDPLabelData& rLabelData, const ScDPFuncData& rFuncData );

    USHORT              GetFuncMask() const;
    ::com::sun::star::sheet::DataPilotFieldReference GetFieldRef() const;

private:
    void                Init( const ScDPLabelData& rLabelData, const ScDPFuncData& rFuncData );

    DECL_LINK( SelectHdl, ListBox* );
    DECL_LINK( DblClickHdl, MultiListBox* );

private:
    FixedLine           maFlFunc;
    ScDPFunctionListBox maLbFunc;
    FixedText           maFtNameLabel;
    FixedInfo           maFtName;
    FixedLine           maFlDisplay;
    FixedText           maFtType;
    ListBox             maLbType;
    FixedText           maFtBaseField;
    ListBox             maLbBaseField;
    FixedText           maFtBaseItem;
    ListBox             maLbBaseItem;
    OKButton            maBtnOk;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;
    MoreButton          maBtnMore;

    ScDPListBoxWrapper  maLbTypeWrp;        /// Wrapper for direct usage of API constants.

    const ScDPLabelDataVec& mrLabelVec;     /// Data of all labels.
    bool                mbEmptyItem;        /// true = Empty base item in listbox.
};

// ============================================================================

class ScDPSubtotalDlg : public ModalDialog
{
public:
    explicit            ScDPSubtotalDlg( Window* pParent, ScDPObject& rDPObj,
                            const ScDPLabelData& rLabelData, const ScDPFuncData& rFuncData,
                            const ScDPNameVec& rDataFields, bool bEnableLayout );

    USHORT              GetFuncMask() const;

    void                FillLabelData( ScDPLabelData& rLabelData ) const;

private:
    void                Init( const ScDPLabelData& rLabelData, const ScDPFuncData& rFuncData );

    DECL_LINK( DblClickHdl, MultiListBox* );
    DECL_LINK( RadioClickHdl, RadioButton* );
    DECL_LINK( ClickHdl, PushButton* );

private:
    FixedLine           maFlSubt;
    RadioButton         maRbNone;
    RadioButton         maRbAuto;
    RadioButton         maRbUser;
    ScDPFunctionListBox maLbFunc;
    FixedText           maFtNameLabel;
    FixedInfo           maFtName;
    CheckBox            maCbShowAll;
    OKButton            maBtnOk;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;
    PushButton          maBtnOptions;

    ScDPObject&         mrDPObj;            /// The DataPilot object (for member names).
    const ScDPNameVec&  mrDataFields;       /// The list of all data field names.

    ScDPLabelData       maLabelData;        /// Cache for sub dialog.
    bool                mbEnableLayout;     /// true = Enable Layout mode controls.
};

// ============================================================================

class ScDPSubtotalOptDlg : public ModalDialog
{
public:
    explicit            ScDPSubtotalOptDlg( Window* pParent, ScDPObject& rDPObj,
                            const ScDPLabelData& rLabelData, const ScDPNameVec& rDataFields,
                            bool bEnableLayout );

    void                FillLabelData( ScDPLabelData& rLabelData ) const;

private:
    void                Init( const ScDPNameVec& rDataFields, bool bEnableLayout );
    void                InitHideListBox();

    DECL_LINK( RadioClickHdl, RadioButton* );
    DECL_LINK( CheckHdl, CheckBox* );
    DECL_LINK( SelectHdl, ListBox* );

private:
    FixedLine           maFlSortBy;
    ListBox             maLbSortBy;
    RadioButton         maRbSortAsc;
    RadioButton         maRbSortDesc;
    RadioButton         maRbSortMan;
    FixedLine           maFlLayout;
    FixedText           maFtLayout;
    ListBox             maLbLayout;
    CheckBox            maCbLayoutEmpty;
    FixedLine           maFlAutoShow;
    CheckBox            maCbShow;
    NumericField        maNfShow;
    FixedText           maFtShow;
    FixedText           maFtShowFrom;
    ListBox             maLbShowFrom;
    FixedText           maFtShowUsing;
    ListBox             maLbShowUsing;
    FixedLine           maFlHide;
    SvxCheckListBox     maLbHide;
    FixedText           maFtHierarchy;
    ListBox             maLbHierarchy;
    OKButton            maBtnOk;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;

    ScDPListBoxWrapper  maLbLayoutWrp;      /// Wrapper for direct usage of API constants.
    ScDPListBoxWrapper  maLbShowFromWrp;    /// Wrapper for direct usage of API constants.

    ScDPObject&         mrDPObj;            /// The DataPilot object (for member names).
    ScDPLabelData       maLabelData;        /// Cache for members data.
};

// ============================================================================

class ScDPShowDetailDlg : public ModalDialog
{
public:
    explicit            ScDPShowDetailDlg( Window* pParent, ScDPObject& rDPObj, USHORT nOrient );

    virtual short       Execute();

    String              GetDimensionName() const;

private:
    DECL_LINK( DblClickHdl, ListBox* );

private:
    FixedText           maFtDims;
    ListBox             maLbDims;
    OKButton            maBtnOk;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;
};

// ============================================================================

#endif

