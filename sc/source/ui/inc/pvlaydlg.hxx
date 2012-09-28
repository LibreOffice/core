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

#ifndef SC_PVLAYDLG_HXX
#define SC_PVLAYDLG_HXX

#include <vector>
#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>

#include <vcl/lstbox.hxx>
#include <vcl/scrbar.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/morebtn.hxx>
#include "pivot.hxx"
#include "anyrefdg.hxx"
#include "fieldwnd.hxx"
#include "formula/funcutl.hxx"

/*==========================================================================*\

    An instance of the class ScPivotLayoutDlg is a (semi-)modal dialog
    that can be associated to the mouse fields with column headings of
    the three pivot categories "column", "row" and "data".

    The dialog receives information about these headers (name,
    type (number/string) and function mask) in the label data structure
    Furthermore, three pivot field arrays are passed on, with which the three
    category windows are initialized. A window class is represented by an
    instance of class FieldWindow. Such a window is responsible for the
    representation of data structures on the screen. It reports on mouse
    actions in the dialogue and provides appropriate methods for
    changing the representation. The dialogue provides the adjustment of the
    internal data structures with respect to the representation on the screen.
    Another FieldWindow(Select) provides all table headers for selection,
    is thus "read only".

\*==========================================================================*/

//============================================================================

class ScViewData;
class ScDocument;
struct ScDPFuncData;
class ScDPObject;

//============================================================================

class ScDPLayoutDlg : public ScAnyRefDlg
{
    /** data source type */
    enum DataSrcType {
        SRC_REF,     /// range reference
        SRC_NAME,    /// range name
        SRC_INVALID  /// invalid range
    };
public:
                            ScDPLayoutDlg(
                                SfxBindings* pB,
                                SfxChildWindow* pCW,
                                Window* pParent,
                                const ScDPObject& rDPObject,
                                bool bNewOutput );
    virtual                 ~ScDPLayoutDlg();

    virtual void            SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual sal_Bool            IsRefInputMode() const { return bRefInputMode; }
    virtual void            SetActive();
    virtual sal_Bool            Close();
    virtual void            StateChanged( StateChangedType nStateChange );

    void                    NotifyDoubleClick    ( ScDPFieldType eType, size_t nFieldIndex );
    PointerStyle            NotifyMouseButtonDown( ScDPFieldType eType, size_t nFieldIndex );
    void                    NotifyMouseButtonUp  ( const Point& rAt );
    PointerStyle            NotifyMouseMove      ( const Point& rAt );
    void                    NotifyFieldFocus     ( ScDPFieldType eType, bool bGotFocus );
    void                    NotifyMoveFieldToEnd      ( ScDPFieldType eToType );
    void                    NotifyRemoveField    ( ScDPFieldType eType, size_t nFieldIndex );

    Size                    GetStdFieldBtnSize() const;

protected:
    virtual void            Deactivate();

private:
    typedef boost::ptr_vector<ScDPFuncData> ScDPFuncDataVec;
    typedef boost::scoped_ptr<ScDPObject> ScDPObjectPtr;

    FixedLine               aFlLayout;
    FixedText               aFtPage;
    ScDPPageFieldControl    aWndPage;
    FixedText               aFtCol;
    ScDPColFieldControl     aWndCol;
    FixedText               aFtRow;
    ScDPRowFieldControl     aWndRow;
    FixedText               aFtData;
    ScDPDataFieldControl    aWndData;
    ScDPSelectFieldControl  aWndSelect;
    FixedInfo               aFtInfo;

    FixedLine               aFlAreas;

    // DP source selection
    FixedText               aFtInArea;
    ::formula::RefEdit      aEdInPos;
    ::formula::RefButton    aRbInPos;

    // DP output location
    ListBox                 aLbOutPos;
    FixedText               aFtOutArea;
    formula::RefEdit        aEdOutPos;
    formula::RefButton      aRbOutPos;

    CheckBox                aBtnIgnEmptyRows;
    CheckBox                aBtnDetectCat;
    CheckBox                aBtnTotalCol;
    CheckBox                aBtnTotalRow;
    CheckBox                aBtnFilter;
    CheckBox                aBtnDrillDown;

    OKButton                aBtnOk;
    CancelButton            aBtnCancel;
    HelpButton              aBtnHelp;
    PushButton              aBtnRemove;
    PushButton              aBtnOptions;
    MoreButton              aBtnMore;

    const rtl::OUString     aStrUndefined;
    const rtl::OUString     aStrNewTable;
    std::vector<rtl::OUString> aFuncNameArr;

    ScDPFieldType           eDnDFromType;
    size_t                  nDnDFromIndex;
    bool                    bIsDrag;

    ::formula::RefEdit*     pEditActive;

    Rectangle               aRectPage;
    Rectangle               aRectRow;
    Rectangle               aRectCol;
    Rectangle               aRectData;
    Rectangle               aRectSelect;

    ScDPLabelDataVec        aLabelDataArr; // (nCol, Field name, Number/Text)

    ScDPFieldType           eLastActiveType;        /// Type of last active area.
    size_t                  nOffset;                /// Offset of first field in TYPE_SELECT area.

    ScDPFuncDataVec         aSelectArr;
    ScDPFuncDataVec         aPageArr;
    ScDPFuncDataVec         aColArr;
    ScDPFuncDataVec         aRowArr;
    ScDPFuncDataVec         aDataArr;

    ScDPObjectPtr           xDlgDPObject;
    ScRange                 aOldRange;
    ScPivotParam            thePivotData;
    ScViewData*             pViewData;
    ScDocument*             pDoc;
    bool                    bRefInputMode;

private:
    void                    Init            (bool bNewOutput);
    void InitWndSelect(const ScDPLabelDataVec& rLabels);
    void InitWndData(const std::vector<PivotField>& rFields);
    void                    InitFieldWindow ( const ::std::vector<PivotField>& rFields, ScDPFieldType eType );
    void                    InitFocus       ();
    void                    InitFields      ();
    void                    CalcWndSizes    ();
    void                    AdjustDlgSize();
    Point                   DlgPos2WndPos   ( const Point& rPt, Window& rWnd );
    ScDPLabelData*          GetLabelData    ( SCsCOL nCol, size_t* pPos = NULL );
    rtl::OUString GetLabelString(SCsCOL nCol);
    bool                    IsOrientationAllowed( SCsCOL nCol, ScDPFieldType eType );
    rtl::OUString GetFuncString( sal_uInt16& rFuncMask, bool bIsValue = true );
    bool Contains( ScDPFuncDataVec* pArr, const ScDPFuncData& rData, size_t& nAt );
    void                    Remove          ( ScDPFuncDataVec* pArr, size_t nAt );
    void                    Insert          ( ScDPFuncDataVec* pArr, const ScDPFuncData& rFData, size_t nAt );

    void                    AddField        ( size_t nFromIndex,
                                              ScDPFieldType eToType, const Point& rAtPos );
    void                    AppendField(size_t nFromIndex, ScDPFieldType eToType);
    void                    MoveField       ( ScDPFieldType eFromType, size_t nFromIndex,
                                              ScDPFieldType eToType, const Point&  rAtPos );
    void                    MoveFieldToEnd(ScDPFieldType eFromType, size_t nFromIndex, ScDPFieldType eToType);
    void                    RemoveField     ( ScDPFieldType eRemType, size_t nRemIndex );

    bool                    GetPivotArrays( ::std::vector<PivotField>& rPageFields,
                                            ::std::vector<PivotField>& rColFields,
                                            ::std::vector<PivotField>& rRowFields,
                                            ::std::vector<PivotField>& rDataFields );

    void                    UpdateSrcRange();
    void                    RepaintFieldWindows();

    ScDPFieldControlBase* GetFieldWindow(ScDPFieldType eType);

    /**
     * Get pointers to field windows that are <b>not</b> the window of
     * specified type.  The select window type is not included.
     */
    void GetOtherFieldWindows(
        ScDPFieldType eType, ScDPFieldControlBase*& rpWnd1, ScDPFieldControlBase*& rpWnd2);

    ScDPFuncDataVec* GetFieldDataArray(ScDPFieldType eType);

    /**
     * Like GetOtherFieldWindows(), get pointers to data arrays of the fields
     * that are <b>not</b> the specified field type.
     */
    void GetOtherDataArrays(
        ScDPFieldType eType, ScDPFuncDataVec*& rpArr1, ScDPFuncDataVec*& rpArr2);

    sal_uInt8 GetNextDupCount(const ScDPFuncDataVec& rArr, const ScDPFuncData& rData, size_t nDataIndex) const;

    // Handler
    DECL_LINK( ClickHdl, PushButton * );
    DECL_LINK( SelAreaHdl, void * );
    DECL_LINK( MoreClickHdl, void * );
    DECL_LINK( EdModifyHdl, void * );
    DECL_LINK( EdInModifyHdl, void * );
    DECL_LINK( OkHdl, void * );
    DECL_LINK( CancelHdl, void * );
    DECL_LINK( GetFocusHdl, Control* );
};

#endif // SC_PVLAYDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
