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

#ifndef SC_FIELDWND_HXX
#define SC_FIELDWND_HXX

#include <utility>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <cppuhelper/weakref.hxx>
#include <rtl/ref.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/fixed.hxx>

#include "address.hxx"
#include "pivot.hxx"

// ============================================================================

class ScPivotLayoutDlg;
class ScAccessibleDataPilotControl;

const size_t PIVOTFIELD_INVALID = static_cast< size_t >( -1 );

// ============================================================================

/** Type of the pivot table field window. */
enum ScPivotFieldType
{
    PIVOTFIELDTYPE_PAGE,            /// Window for all page fields.
    PIVOTFIELDTYPE_COL,             /// Window for all column fields.
    PIVOTFIELDTYPE_ROW,             /// Window for all row fields.
    PIVOTFIELDTYPE_DATA,            /// Window for all data fields.
    PIVOTFIELDTYPE_SELECT           /// Selection window with all fields.
};

/** Type of an end tracking event. */
enum ScPivotFieldEndTracking
{
    ENDTRACKING_SUSPEND,            /// Stop tracking in this window, but tracking still active (in another window).
    ENDTRACKING_CANCEL,             /// Tracking has been cancelled.
    ENDTRACKING_DROP                /// Tracking has ended, a field has been dropped.
};

// ============================================================================

typedef ::std::pair< const ScPivotFuncData*, size_t > ScPivotFuncDataEntry;

// ============================================================================

/** Represents a field area in the pivot table layout dialog. */
class ScPivotFieldWindow : public Control
{
public:
                        ScPivotFieldWindow(
                            ScPivotLayoutDlg* pDialog,
                            const ResId& rResId,
                            ScrollBar& rScrollBar,
                            FixedText* pFtCaption,
                            const ::rtl::OUString& rName,
                            ScPivotFieldType eFieldType,
                            const sal_Char* pcHelpId,
                            PointerStyle eDropPointer,
                            size_t nColCount,
                            size_t nRowCount,
                            long nFieldWidthFactor,
                            long nSpaceSize );

    virtual             ~ScPivotFieldWindow();

    /** Initializes this field window from the passed label data (used for selection window). */
    void                ReadDataLabels( const ScDPLabelDataVector& rLabels );
    /** Initializes this field window from the passed field data (used for row/col/page/data window). */
    void                ReadPivotFields( const ScPivotFieldVector& rPivotFields );

    /** Fills the passed vector with the plain names of all fields from this field window. */
    void                WriteFieldNames( ScDPNameVec& rFieldNames ) const;
    /** Fills the passed pivot field vector with the fields of this field window. */
    void                WritePivotFields( ScPivotFieldVector& rPivotFields ) const;

    /** Returns the type of this field window. */
    inline ScPivotFieldType GetType() const { return meFieldType; }
    /** Returns the mouse pointer style for tracking over this window. */
    inline PointerStyle GetDropPointerStyle() const { return meDropPointer; }
    /** Returns the name of the control without shortcut. */
    inline ::rtl::OUString GetName() const { return maName; }
    /** Returns the description of the control which is used for the accessibility objects. */
    ::rtl::OUString     GetDescription() const;

    /** Returns true, if the window is empty. */
    inline bool         IsEmpty() const { return maFields.empty(); }
    /** Returns the number of existing fields. */
    inline size_t       GetFieldCount() const { return maFields.size(); }
    /** Returns the text of an existing field. */
    ::rtl::OUString     GetFieldText( size_t nFieldIndex ) const;

    /** Returns the index of a field with the specified column identifier. */
    ScPivotFuncDataEntry FindFuncDataByCol( SCCOL nCol ) const;

    /** Returns the pixel size of a field. */
    inline const Size&  GetFieldSize() const { return maFieldSize; }
    /** Returns the pixel position of a field (without bound check). */
    Point               GetFieldPosition( size_t nFieldIndex ) const;
    /** Calculates the field index at a specific pixel position. */
    size_t              GetFieldIndex( const Point& rWindowPos ) const;
    /** Calculates the field insertion index for mouse drop at a specific pixel position. */
    size_t              GetDropIndex( const Point& rWindowPos ) const;

    /** Returns the index of the selected field. */
    inline size_t       GetSelectedIndex() const { return mnSelectIndex; }
    /** Grabs focus and sets the passed selection. */
    void                GrabFocusAndSelect( size_t nIndex );
    /** Selects the next field. */
    void                SelectNextField();

    /** Inserts a new field in front of the specified field. */
    void                InsertField( size_t nInsertIndex, const ScPivotFuncData& rFuncData );
    /** Removes the specified field. */
    bool                RemoveField( size_t nRemoveIndex );
    /** Moves the specified field to a new position. */
    bool                MoveField( size_t nFieldIndex, size_t nInsertIndex );

    /** Returns the selected field (pointer is valid as long as field vector is not changed). */
    const ScPivotFuncData* GetSelectedFuncData() const;
    /** Removes the selected field. */
    void                ModifySelectedField( const ScPivotFuncData& rFuncData );
    /** Removes the selected field. */
    bool                RemoveSelectedField();
    /** Moves the selected field in front of the specified field. */
    bool                MoveSelectedField( size_t nInsertIndex );

    /** Called from dialog when tracking starts in this field window. */
    void                NotifyStartTracking();
    /** Called from dialog while tracking in this field window. */
    void                NotifyTracking( const Point& rWindowPos );
    /** Called from dialog when tracking ends in this field window. */
    void                NotifyEndTracking( ScPivotFieldEndTracking eEndType );

protected:
    virtual void        Paint( const Rectangle& rRect );
    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual void        GetFocus();
    virtual void        LoseFocus();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        CreateAccessible();

private:
    /** A structure containing all data needed for a field in this window. */
    struct ScPivotWindowField
    {
        ScPivotFuncData     maFuncData;         /// Field data from source pivot table.
        ::rtl::OUString     maFieldName;        /// Name displayed on the field button.
        bool                mbClipped;          /// True = field text does not fit into button.
        explicit            ScPivotWindowField( const ScDPLabelData& rLabelData );
        explicit            ScPivotWindowField( ScPivotLayoutDlg& rDialog, const ScPivotField& rField, bool bDataWindow );
        explicit            ScPivotWindowField( ScPivotLayoutDlg& rDialog, const ScPivotFuncData& rFuncData, bool bDataWindow );
        void                InitFieldName( ScPivotLayoutDlg& rDialog, bool bDataWindow );
    };

    /** Specifies how the selection cursor can move in the window. */
    enum MoveType { PREV_FIELD, NEXT_FIELD, PREV_LINE, NEXT_LINE, PREV_PAGE, NEXT_PAGE, FIRST_FIELD, LAST_FIELD };

    /** Calculates a scroll position to make the passed field visible. Tries to
        stick to current scroll position if possible. */
    size_t              RecalcVisibleIndex( size_t nSelectIndex ) const;

    /** Sets selection to the specified field and changes scrolling position. */
    void                SetSelectionUnchecked( size_t nSelectIndex, size_t nFirstVisIndex );
    /** Selects a field and adjusts scrolling position to make the field visible. */
    void                MoveSelection( size_t nSelectIndex );
    /** Sets selection to a new position relative to current. */
    void                MoveSelection( MoveType eMoveType );
    /** Moves the selected field to a new position relative to current. */
    void                MoveSelectedField( MoveType eMoveType );

    /** Inserts the passed field into the vector and notifies accessibility object. */
    void                InsertFieldUnchecked( size_t nInsertIndex, const ScPivotWindowField& rField );
    /** Removes the specified field from the vector and notifies accessibility object. */
    void                RemoveFieldUnchecked( size_t nRemoveIndex );

    /** Draws the background. */
    void                DrawBackground( OutputDevice& rDev );
    /** Draws the specified field. */
    void                DrawField( OutputDevice& rDev, size_t nFieldIndex );
    /** Draws the insertion cursor. */
    void                DrawInsertionCursor( OutputDevice& rDev );

    /** Returns a reference to the accessiblity object, if still alive. */
    ::rtl::Reference< ScAccessibleDataPilotControl > GetAccessibleControl();

    DECL_LINK( ScrollHdl, ScrollBar* );

private:
    typedef ::std::vector< ScPivotWindowField > ScPivotWindowFieldVector;

    ScPivotLayoutDlg*   mpDialog;           /// Parent pivot table dialog.
    ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible >
                        mxAccessible;       /// Weak reference to the accessible object.
    ScAccessibleDataPilotControl*
                        mpAccessible;       /// Pointer to the accessible implementation.
    ScrollBar&          mrScrollBar;        /// Scrollbar of the select window.
    FixedText*          mpFtCaption;        /// Associated fixedtext control with caption text and mnemonic.
    ::rtl::OUString     maName;             /// Name of the control, used for accessibility.
    ScPivotWindowFieldVector maFields;      /// Vector with all fields contained in this window.
    Size                maFieldSize;        /// Size of a single field in pixels.
    Size                maSpaceSize;        /// Size between fields in pixels.
    ScPivotFieldType    meFieldType;        /// Type of this field window.
    PointerStyle        meDropPointer;      /// Mouse pointer style for tracking over this window.
    size_t              mnColCount;         /// Number of field columns.
    size_t              mnRowCount;         /// Number of field rows.
    size_t              mnLineSize;         /// Number of fields per line (depending on scrolling orientation).
    size_t              mnPageSize;         /// Number of visible fields.
    size_t              mnFirstVisIndex;    /// Index of first visible field (scrolling offset).
    size_t              mnSelectIndex;      /// Currently selected field.
    size_t              mnInsCursorIndex;   /// Position of the insertion cursor.
    size_t              mnOldFirstVisIndex; /// Stores original scroll position during auto scrolling.
    size_t              mnAutoScrollDelay;  /// Initial counter before auto scrolling starts on tracking.
    bool                mbVertical;         /// True = sort fields vertically.
    bool                mbIsTrackingSource; /// True = this field window is the source while tracking.
};

// ============================================================================

#endif
