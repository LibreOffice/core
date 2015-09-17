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

#ifndef INCLUDED_SC_SOURCE_UI_INC_PVFUNDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_PVFUNDLG_HXX

#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>

#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/morebtn.hxx>
#include <vcl/field.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/checklbx.hxx>
#include <sfx2/itemconnect.hxx>
#include "pivot.hxx"

#include <memory>
#include <unordered_map>

typedef sfx::ListBoxWrapper< sal_Int32 > ScDPListBoxWrapper;

class ScDPObject;

class ScDPFunctionListBox : public ListBox
{
public:
    ScDPFunctionListBox(vcl::Window* pParent, WinBits nStyle);

    void                SetSelection( sal_uInt16 nFuncMask );
    sal_uInt16          GetSelection() const;

private:
    void                FillFunctionNames();
};

class ScDPFunctionDlg : public ModalDialog
{
    typedef std::unordered_map< OUString, OUString, OUStringHash > NameMapType;
public:
    explicit            ScDPFunctionDlg( vcl::Window* pParent, const ScDPLabelDataVector& rLabelVec,
                            const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData );
    virtual ~ScDPFunctionDlg();
    virtual void            dispose() SAL_OVERRIDE;
    sal_uInt16              GetFuncMask() const;
    ::com::sun::star::sheet::DataPilotFieldReference GetFieldRef() const;

private:
    void                Init( const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData );

    const OUString& GetBaseFieldName(const OUString& rLayoutName) const;
    const OUString& GetBaseItemName(const OUString& rLayoutName) const;

    /** Searches for a listbox entry, starts search at specified position. */
    sal_Int32 FindBaseItemPos( const OUString& rEntry, sal_Int32 nStartPos ) const;

    DECL_LINK( SelectHdl, ListBox* );
    DECL_LINK_TYPED( DblClickHdl, ListBox&, void );

private:
    VclPtr<ScDPFunctionListBox> mpLbFunc;
    VclPtr<FixedText>           mpFtName;
    VclPtr<ListBox>             mpLbType;
    VclPtr<FixedText>           mpFtBaseField;
    VclPtr<ListBox>             mpLbBaseField;
    VclPtr<FixedText>           mpFtBaseItem;
    VclPtr<ListBox>             mpLbBaseItem;
    VclPtr<OKButton>            mpBtnOk;

    NameMapType          maBaseFieldNameMap; // cache for base field display -> original name.
    NameMapType          maBaseItemNameMap;  // cache for base item display -> original name.

    std::unique_ptr<ScDPListBoxWrapper>  mxLbTypeWrp;        /// Wrapper for direct usage of API constants.

    const ScDPLabelDataVector& mrLabelVec;  /// Data of all labels.
    bool                 mbEmptyItem;        /// true = Empty base item in listbox.
};

class ScDPSubtotalDlg : public ModalDialog
{
public:
    explicit            ScDPSubtotalDlg( vcl::Window* pParent, ScDPObject& rDPObj,
                            const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData,
                            const ScDPNameVec& rDataFields, bool bEnableLayout );
    virtual             ~ScDPSubtotalDlg();
    virtual void        dispose() SAL_OVERRIDE;
    sal_uInt16          GetFuncMask() const;

    void                FillLabelData( ScDPLabelData& rLabelData ) const;

private:
    void                Init( const ScDPLabelData& rLabelData, const ScPivotFuncData& rFuncData );

    DECL_LINK_TYPED( DblClickHdl, ListBox&, void );
    DECL_LINK_TYPED( RadioClickHdl, Button*, void );
    DECL_LINK_TYPED( ClickHdl, Button*, void );

private:
    VclPtr<RadioButton>         mpRbNone;
    VclPtr<RadioButton>         mpRbAuto;
    VclPtr<RadioButton>         mpRbUser;
    VclPtr<ScDPFunctionListBox> mpLbFunc;
    VclPtr<FixedText>           mpFtName;
    VclPtr<CheckBox>            mpCbShowAll;
    VclPtr<OKButton>            mpBtnOk;
    VclPtr<PushButton>          mpBtnOptions;

    ScDPObject&          mrDPObj;            /// The DataPilot object (for member names).
    const ScDPNameVec&   mrDataFields;       /// The list of all data field names.

    ScDPLabelData        maLabelData;        /// Cache for sub dialog.
    bool                 mbEnableLayout;     /// true = Enable Layout mode controls.
};

class ScDPSubtotalOptDlg : public ModalDialog
{
public:
    explicit            ScDPSubtotalOptDlg( vcl::Window* pParent, ScDPObject& rDPObj,
                            const ScDPLabelData& rLabelData, const ScDPNameVec& rDataFields,
                            bool bEnableLayout );
    virtual              ~ScDPSubtotalOptDlg();
    virtual void        dispose() SAL_OVERRIDE;
    void                FillLabelData( ScDPLabelData& rLabelData ) const;

private:
    void                Init( const ScDPNameVec& rDataFields, bool bEnableLayout );
    void                InitHideListBox();

    ScDPName GetFieldName(const OUString& rLayoutName) const;

    /** Searches for a listbox entry, starts search at specified position. */
    sal_Int32 FindListBoxEntry( const ListBox& rLBox, const OUString& rEntry, sal_Int32 nStartPos ) const;

    DECL_LINK_TYPED( RadioClickHdl, Button*, void );
    DECL_LINK_TYPED( CheckHdl, Button*, void );
    DECL_LINK( SelectHdl, ListBox* );

private:
    VclPtr<ListBox>            m_pLbSortBy;
    VclPtr<RadioButton>        m_pRbSortAsc;
    VclPtr<RadioButton>        m_pRbSortDesc;
    VclPtr<RadioButton>        m_pRbSortMan;
    VclPtr<VclContainer>       m_pLayoutFrame;
    VclPtr<ListBox>            m_pLbLayout;
    VclPtr<CheckBox>           m_pCbLayoutEmpty;
    VclPtr<CheckBox>           m_pCbRepeatItemLabels;
    VclPtr<CheckBox>           m_pCbShow;
    VclPtr<NumericField>       m_pNfShow;
    VclPtr<FixedText>          m_pFtShow;
    VclPtr<FixedText>          m_pFtShowFrom;
    VclPtr<ListBox>            m_pLbShowFrom;
    VclPtr<FixedText>          m_pFtShowUsing;
    VclPtr<ListBox>            m_pLbShowUsing;
    VclPtr<VclContainer>       m_pHideFrame;
    VclPtr<SvxCheckListBox>    m_pLbHide;
    VclPtr<FixedText>          m_pFtHierarchy;
    VclPtr<ListBox>            m_pLbHierarchy;

    std::unique_ptr<ScDPListBoxWrapper> m_xLbLayoutWrp;      /// Wrapper for direct usage of API constants.
    std::unique_ptr<ScDPListBoxWrapper> m_xLbShowFromWrp;    /// Wrapper for direct usage of API constants.

    ScDPObject&         mrDPObj;            /// The DataPilot object (for member names).
    ScDPLabelData       maLabelData;        /// Cache for members data.

    typedef std::unordered_map<OUString, ScDPName, OUStringHash> NameMapType;
    NameMapType maDataFieldNameMap; /// Cache for displayed name to field name mapping.
};

class ScDPShowDetailDlg : public ModalDialog
{
public:
    explicit            ScDPShowDetailDlg( vcl::Window* pParent, ScDPObject& rDPObj, sal_uInt16 nOrient );
    virtual             ~ScDPShowDetailDlg();
    virtual void        dispose() SAL_OVERRIDE;
    virtual short       Execute() SAL_OVERRIDE;

    /**
     * @return String internal name of the selected field.  Note that this may
     *         be different from the name displayed in the dialog if the field
     *         has a layout name.
     */
    OUString GetDimensionName() const;

private:
    DECL_LINK_TYPED( DblClickHdl, ListBox&, void );

private:
    VclPtr<ListBox>            mpLbDims;
    VclPtr<OKButton>           mpBtnOk;

    typedef std::unordered_map<OUString, long, OUStringHash> DimNameIndexMap;
    DimNameIndexMap     maNameIndexMap;
    ScDPObject&         mrDPObj;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
