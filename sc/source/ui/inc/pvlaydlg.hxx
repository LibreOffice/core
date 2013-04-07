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
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 ************************************************************************/

#ifndef SC_PVLAYDLG_HXX
#define SC_PVLAYDLG_HXX

#include <memory>
#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>

#include <formula/funcutl.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/morebtn.hxx>
#include <vcl/scrbar.hxx>

#include "anyrefdg.hxx"
#include "fieldwnd.hxx"

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

class ScViewData;
class ScDocument;
struct ScPivotFuncData;
class ScDPObject;

class ScPivotLayoutDlg : public ScAnyRefDlg
{
    /** data source type */
    enum DataSrcType {
        SRC_REF,     /// range reference
        SRC_NAME,    /// range name
        SRC_INVALID  /// invalid range
    };
public:
                        ScPivotLayoutDlg(
                            SfxBindings* pB,
                            SfxChildWindow* pCW,
                            Window* pParent,
                            const ScDPObject& rDPObject,
                            bool bNewOutput );
    virtual             ~ScPivotLayoutDlg();

    void                    NotifyDoubleClick    ( ScPivotFieldType eType, size_t nFieldIndex );
    PointerStyle            NotifyMouseButtonDown( ScPivotFieldType eType, size_t nFieldIndex );
    void                    NotifyFieldFocus     ( ScPivotFieldType eType, bool bGotFocus );
    void                    NotifyMoveFieldToEnd      ( ScPivotFieldType eToType );
    void                    NotifyRemoveField    ( ScPivotFieldType eType, size_t nFieldIndex );

    Size                    GetStdFieldBtnSize() const;

    /**
     * Drop currently dragged field item into specified position.
     *
     * @param rScrPos screen position of mouse cursor.
     * @param eToType type of field at mouse cursor position.
     */
    void DropFieldItem( const Point& rScrPos, ScPivotFieldType eToType );

    /**
     * Get pointer style at current mouse position during dragging of field
     * item.
     */
    PointerStyle GetPointerStyleAtPoint( const Point& rScrPos, ScPivotFieldType eFieldType );

    /**
     * Determine the type of field at mouse cursor position.
     *
     * @param rScrPos mouse cursor position (screen position).
     */
    ScPivotFieldType GetFieldTypeAtPoint( const Point& rScrPos ) const;

protected:
    virtual void        Deactivate();
    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual sal_Bool    IsRefInputMode() const;
    virtual void        SetActive();
    virtual sal_Bool    Close();

private:
    /** Returns the localized function name for the specified (1-based) resource index. */
    inline const OUString& GetFuncName( sal_uInt16 nFuncIdx ) const { return maFuncNames[nFuncIdx-1]; }

    /** Fills the field windows from the current pivot table settings. */
    void                InitFieldWindows();
    /** Sets focus to the specified field control, if it is not empty. */
    void                GrabFieldFocus( ScDPFieldControlBase& rFieldWindow );

    void InitWndSelect(const ScDPLabelDataVector& rLabels);
    void InitWndData(const std::vector<ScPivotField>& rFields);
    void                    InitFieldWindow ( const ::std::vector<ScPivotField>& rFields, ScPivotFieldType eType );
    void                    AdjustDlgSize();
    ScDPLabelData* GetLabelData( SCCOL nCol );
    OUString GetLabelString(SCsCOL nCol);
    bool                    IsOrientationAllowed( SCsCOL nCol, ScPivotFieldType eType );
    OUString GetFuncString( sal_uInt16& rFuncMask, bool bIsValue = true );

    void                    AddField        ( size_t nFromIndex,
                                              ScPivotFieldType eToType, const Point& rAtPos );
    void                    AppendField(size_t nFromIndex, ScPivotFieldType eToType);
    void                    MoveField       ( ScPivotFieldType eFromType, size_t nFromIndex,
                                              ScPivotFieldType eToType, const Point&  rAtPos );
    void                    MoveFieldToEnd(ScPivotFieldType eFromType, size_t nFromIndex, ScPivotFieldType eToType);
    void                    RemoveField     ( ScPivotFieldType eRemType, size_t nRemIndex );

    bool                    GetPivotArrays( ::std::vector<ScPivotField>& rPageFields,
                                            ::std::vector<ScPivotField>& rColFields,
                                            ::std::vector<ScPivotField>& rRowFields,
                                            ::std::vector<ScPivotField>& rDataFields );

    void UpdateSrcRange();
    void UpdateOutputPos();
    void OutputPosUpdated();
    void MoreBtnClicked();

    void                    RepaintFieldWindows();

    /**
     * Initialize control sizes and the dialog size which depends on them.
     */
    void InitControlAndDlgSizes();

    ScDPFieldControlBase* GetFieldWindow(ScPivotFieldType eType);

    /**
     * Get pointers to field windows that are <b>not</b> the window of
     * specified type.  The select window type is not included.
     */
    void GetOtherFieldWindows(
        ScPivotFieldType eType, ScDPFieldControlBase*& rpWnd1, ScDPFieldControlBase*& rpWnd2);

    // Handler
    DECL_LINK( ClickHdl, PushButton * );
    DECL_LINK( OkHdl, void * );
    DECL_LINK( CancelHdl, void * );
    DECL_LINK( MoreClickHdl, void * );
    DECL_LINK( EdOutModifyHdl, void * );
    DECL_LINK( EdInModifyHdl, void * );
    DECL_LINK( SelAreaHdl, void * );
    DECL_LINK( GetRefEditFocusHdl, formula::RefEdit* );

private:
    typedef boost::scoped_ptr<ScDPObject> ScDPObjectPtr;

    FixedLine               maFlLayout;
    FixedText               maFtPage;
    ScDPPageFieldControl    maWndPage;
    FixedText               maFtCol;
    ScDPColFieldControl     maWndCol;
    FixedText               maFtRow;
    ScDPRowFieldControl     maWndRow;
    FixedText               maFtData;
    ScDPDataFieldControl    maWndData;
    ScDPSelectFieldControl  maWndSelect;
    FixedInfo               maFtInfo;

    std::vector<ScDPFieldControlBase*> maFieldCtrls;

    FixedLine               maFlAreas;
    FixedText               maFtInArea;
    ::formula::RefEdit      maEdInPos;
    ::formula::RefButton    maRbInPos;
    ListBox                 maLbOutPos;
    FixedText               maFtOutArea;
    formula::RefEdit        maEdOutPos;
    formula::RefButton      maRbOutPos;
    CheckBox                maBtnIgnEmptyRows;
    CheckBox                maBtnDetectCat;
    CheckBox                maBtnTotalCol;
    CheckBox                maBtnTotalRow;
    CheckBox                maBtnFilter;
    CheckBox                maBtnDrillDown;

    OKButton                maBtnOk;
    CancelButton            maBtnCancel;
    HelpButton              maBtnHelp;
    PushButton              maBtnRemove;
    PushButton              maBtnOptions;
    MoreButton              maBtnMore;
    std::vector<OUString> maFuncNames;     /// Localized function names from resource.
    boost::ptr_vector<OUString> maRefStrs; /// Reference strings stored with the output list box.
    ScDPObjectPtr           mxDlgDPObject;      /// Clone of the pivot table object this dialog is based on.
    ScPivotParam            maPivotData;        /// The pivot table field configuration.
    ScDPLabelDataVector     maLabelData;        /// Information about all dimensions.

    ScViewData*             mpViewData;
    ScDocument*             mpDoc;
    formula::RefEdit*       mpRefInputEdit;

    const OUString     maStrUndefined;
    const OUString     maStrNewTable;
    OUString           maOutputRefStr; /// Used only for caching in UI.

    ScPivotFieldType           meDnDFromType;
    size_t                  mnDnDFromIndex;
    bool                    mbIsDrag;

    ScPivotFieldType        meLastActiveType;        /// Type of last active area.
    size_t                  mnOffset;                /// Offset of first field in TYPE_SELECT area.

    ScRange                 maOldRange;
    bool                    mbRefInputMode;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#endif
