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

#ifndef SC_PVFUNDLG_HXX
#define SC_PVFUNDLG_HXX

#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>

#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/morebtn.hxx>
#include <vcl/field.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/checklbx.hxx>
#include <sfx2/itemconnect.hxx>
#include "pivot.hxx"

#include <boost/unordered_map.hpp>

// ============================================================================

typedef sfx::ListBoxWrapper< sal_Int32 > ScDPListBoxWrapper;

class ScDPObject;

// ============================================================================

class ScDPFunctionListBox : public MultiListBox
{
public:
    explicit            ScDPFunctionListBox( Window* pParent, const ResId& rResId );

    void                SetSelection( sal_uInt16 nFuncMask );
    sal_uInt16          GetSelection() const;

private:
    void                FillFunctionNames();
};

// ============================================================================

class ScDPFunctionDlg : public ModalDialog
{
    typedef ::boost::unordered_map< OUString, OUString, OUStringHash > NameMapType;
public:
    explicit            ScDPFunctionDlg( Window* pParent, const ScDPLabelDataVector& rLabelVec,
                            const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData );

    sal_uInt16              GetFuncMask() const;
    ::com::sun::star::sheet::DataPilotFieldReference GetFieldRef() const;

private:
    void                Init( const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData );

    const OUString& GetBaseFieldName(const OUString& rLayoutName) const;
    const OUString& GetBaseItemName(const OUString& rLayoutName) const;

    /** Searches for a listbox entry, starts search at specified position. */
    sal_uInt16 FindBaseItemPos( const String& rEntry, sal_uInt16 nStartPos ) const;

    DECL_LINK( SelectHdl, ListBox* );
    DECL_LINK( DblClickHdl, void* );

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

    NameMapType         maBaseFieldNameMap; // cache for base field display -> original name.
    NameMapType         maBaseItemNameMap;  // cache for base item display -> original name.

    ScDPListBoxWrapper  maLbTypeWrp;        /// Wrapper for direct usage of API constants.

    const ScDPLabelDataVector& mrLabelVec;  /// Data of all labels.
    bool                mbEmptyItem;        /// true = Empty base item in listbox.
};

// ============================================================================

class ScDPSubtotalDlg : public ModalDialog
{
public:
    explicit            ScDPSubtotalDlg( Window* pParent, ScDPObject& rDPObj,
                            const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData,
                            const ScDPNameVec& rDataFields, bool bEnableLayout );

    sal_uInt16              GetFuncMask() const;

    void                FillLabelData( ScDPLabelData& rLabelData ) const;

private:
    void                Init( const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData );

    DECL_LINK(DblClickHdl, void *);
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

    ScDPName GetFieldName(const OUString& rLayoutName) const;

    /** Searches for a listbox entry, starts search at specified position. */
    sal_uInt16 FindListBoxEntry( const ListBox& rLBox, const String& rEntry, sal_uInt16 nStartPos ) const;

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

    typedef ::boost::unordered_map<OUString, ScDPName, OUStringHash> NameMapType;
    NameMapType maDataFieldNameMap; /// Cache for displayed name to field name mapping.
};

// ============================================================================

class ScDPShowDetailDlg : public ModalDialog
{
public:
    explicit            ScDPShowDetailDlg( Window* pParent, ScDPObject& rDPObj, sal_uInt16 nOrient );

    virtual short       Execute();

    /**
     * @return String internal name of the selected field.  Note that this may
     *         be different from the name displayed in the dialog if the field
     *         has a layout name.
     */
    OUString GetDimensionName() const;

private:
    DECL_LINK( DblClickHdl, ListBox* );

private:
    FixedText           maFtDims;
    ListBox             maLbDims;
    OKButton            maBtnOk;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;

    typedef ::boost::unordered_map<String, long, ScStringHashCode> DimNameIndexMap;
    DimNameIndexMap     maNameIndexMap;
    ScDPObject&         mrDPObj;
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
