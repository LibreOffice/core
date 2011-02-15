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

#include <memory>
#include <vector>

#include <formula/funcutl.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/morebtn.hxx>
#include <vcl/scrbar.hxx>

#include "anyrefdg.hxx"
#include "fieldwnd.hxx"

// ============================================================================

class ScViewData;
class ScDocument;
class ScRangeData;
class ScDPObject;

// ============================================================================

class ScPivotLayoutDlg : public ScAnyRefDlg
{
public:
                        ScPivotLayoutDlg(
                            SfxBindings* pB,
                            SfxChildWindow* pCW,
                            Window* pParent,
                            const ScDPObject& rDPObject );
    virtual             ~ScPivotLayoutDlg();

    ScDPLabelData*      GetLabelData( SCCOL nCol, size_t* pnIndex = 0 );
    String              GetFuncString( sal_uInt16& rnFuncMask, bool bIsValue = true );

    void                NotifyStartTracking( ScPivotFieldWindow& rSourceWindow );
    void                NotifyDoubleClick( ScPivotFieldWindow& rSourceWindow );
    void                NotifyFieldRemoved( ScPivotFieldWindow& rSourceWindow );

protected:
    virtual void        Tracking( const TrackingEvent& rTEvt );
    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual sal_Bool    IsRefInputMode() const;
    virtual void        SetActive();
    virtual sal_Bool    Close();

private:
    /** Returns the localized function name for the specified (1-based) resource index. */
    inline const String& GetFuncName( sal_uInt16 nFuncIdx ) const { return maFuncNames[ nFuncIdx - 1 ]; }
    /** Returns the specified field window. */
    ScPivotFieldWindow& GetFieldWindow( ScPivotFieldType eFieldType );

    /** Fills the field windows from the current pivot table settings. */
    void                InitFieldWindows();
    /** Sets focus to the specified field window, if it is not empty. */
    void                GrabFieldFocus( ScPivotFieldWindow& rFieldWindow );

    /** Returns true, if the specified field can be inserted into the specified field window. */
    bool                IsInsertAllowed( const ScPivotFieldWindow& rSourceWindow, const ScPivotFieldWindow& rTargetWindow );
    /** Moves the selected field in the source window to the specified window. */
    bool                MoveField( ScPivotFieldWindow& rSourceWindow, ScPivotFieldWindow& rTargetWindow, size_t nInsertIndex, bool bMoveExisting );

    // Handler
    DECL_LINK( ClickHdl, PushButton * );
    DECL_LINK( OkHdl, OKButton * );
    DECL_LINK( CancelHdl, CancelButton * );
    DECL_LINK( MoreClickHdl, MoreButton * );
    DECL_LINK( EdOutModifyHdl, Edit * );
    DECL_LINK( EdInModifyHdl, Edit * );
    DECL_LINK( SelAreaHdl, ListBox * );
    DECL_LINK( ChildEventListener, VclWindowEvent* );

private:
    typedef ::std::auto_ptr< ScDPObject > ScDPObjectPtr;

    FixedLine           maFlLayout;
    ScrollBar           maScrPage;
    FixedText           maFtPage;
    ScPivotFieldWindow  maWndPage;
    ScrollBar           maScrCol;
    FixedText           maFtCol;
    ScPivotFieldWindow  maWndCol;
    ScrollBar           maScrRow;
    FixedText           maFtRow;
    ScPivotFieldWindow  maWndRow;
    ScrollBar           maScrData;
    FixedText           maFtData;
    ScPivotFieldWindow  maWndData;
    FixedLine           maFlSelect;
    ScrollBar           maScrSelect;
    ScPivotFieldWindow  maWndSelect;
    FixedInfo           maFtInfo;

    FixedLine           maFlAreas;
    FixedText           maFtInArea;
    ::formula::RefEdit  maEdInPos;
    ::formula::RefButton maRbInPos;
    ListBox             maLbOutPos;
    FixedText           maFtOutArea;
    formula::RefEdit    maEdOutPos;
    formula::RefButton  maRbOutPos;
    CheckBox            maBtnIgnEmptyRows;
    CheckBox            maBtnDetectCat;
    CheckBox            maBtnTotalCol;
    CheckBox            maBtnTotalRow;
    CheckBox            maBtnFilter;
    CheckBox            maBtnDrillDown;

    OKButton            maBtnOk;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;
    PushButton          maBtnRemove;
    PushButton          maBtnOptions;
    MoreButton          maBtnMore;

    ::std::vector< String > maFuncNames;    /// Localized function names from resource.

    ScDPObjectPtr       mxDlgDPObject;      /// Clone of the pivot table object this dialog is based on.
    ScPivotParam        maPivotData;        /// The pivot table field configuration.
    ScDPLabelDataVector maLabelData;        /// Information about all dimensions.

    ScViewData*         mpViewData;
    ScDocument*         mpDoc;
    ScPivotFieldWindow* mpFocusWindow;      /// Pointer to the field window that currently has the focus.
    ScPivotFieldWindow* mpTrackingWindow;   /// Pointer to the field window that has started mouse tracking.
    ScPivotFieldWindow* mpDropWindow;       /// Pointer to the field window that shows an insertion cursor.
    ::formula::RefEdit* mpActiveEdit;
    bool                mbRefInputMode;
};

// ============================================================================

#endif
