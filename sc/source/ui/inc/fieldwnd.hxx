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

#ifndef SC_FIELDWND_HXX
#define SC_FIELDWND_HXX

#include <utility>
#include <vector>
#include "address.hxx"
#include "dpglobal.hxx"
#include <vcl/ctrl.hxx>
#include <vcl/fixed.hxx>
#include <vcl/scrbar.hxx>
#include <cppuhelper/weakref.hxx>

class ScDPLayoutDlg;
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

/**
 * Represents a field area in the DataPilot layout dialog.  This base class
 * handles storage of field names and the accessibility object.
 */
class ScDPFieldControlBase : public Control
{
protected:
    typedef ::std::pair<String, bool> FieldName;    // true = text fits into button
    typedef ::std::vector<FieldName> FieldNames;

public:

    /**
     * Custom scroll bar to pass the command event to its parent window.
     * We need this to pass the mouse wheel events to its parent field
     * control to have mouse wheel events appaer to be properly handled by the
     * scroll bar.
     */
    class ScrollBar : public ::ScrollBar
    {
    public:
        ScrollBar(Window* pParent, WinBits nStyle);
        virtual void Command( const CommandEvent& rCEvt );
    private:
        Window* mpParent;
    };

    ScDPFieldControlBase(
        ScDPLayoutDlg* pParent, const ResId& rResId, FixedText* pCaption );
    virtual ~ScDPFieldControlBase();

    virtual void CalcSize() = 0;

    virtual bool IsValidIndex( size_t nIndex ) const = 0;
    /** @return  The pixel position of a field (without bound check). */
    virtual Point GetFieldPosition( size_t nIndex ) = 0;
    /** Calculates the field index at a specific pixel position.
        @param rnIndex  The index of the field is returned here.
        @return  TRUE, if the index value is valid. */
    virtual bool GetFieldIndex( const Point& rPos, size_t& rnIndex ) = 0;
    /** @return  The pixel size of a field. */
    virtual Size GetFieldSize() const = 0;

    /** @return The description of the control which is used for the accessibility objects. */
    virtual String GetDescription() const = 0;
    /** @return The type of the FieldWindow. */
    virtual ScDPFieldType GetFieldType() const = 0;
    virtual void ScrollToShowSelection() = 0;
    virtual void ScrollToEnd() = 0;
    virtual void ResetScrollBar() = 0;
    virtual void HandleWheelScroll(long nNotch) = 0;

    /** Reads the FixedText's text with mnemonic and hides the FixedText. */
    void            UseMnemonic();

    /** @return The name of the control without shortcut. */
    ::rtl::OUString GetName() const;
    void SetName(const ::rtl::OUString& rName);

    /** @return  TRUE, if the field with the given index exists. */
    bool            IsExistingIndex( size_t nIndex ) const;

    /** Inserts a field to the specified index. */
    void            AddField( const String& rText, size_t nNewIndex );

    /** Inserts a field using the specified pixel position.
        @param rPos  The coordinates to insert the field.
        @param rnIndex  The new index of the field is returned here.
        @return  true, if the field has been created. */
    bool            AddField( const String& rText, const Point& rPos, size_t& rnIndex );

    bool            AppendField(const String& rText, size_t& rnIndex);

    /** Removes a field from the specified index. */
    void            DelField( size_t nDelIndex );

    /** @return  The count of existing fields. */
    size_t          GetFieldCount() const;

    bool            IsEmpty() const;

    /** Removes all fields. */
    void            ClearFields();
    /** Changes the text on an existing field. */
    void            SetFieldText( const String& rText, size_t nIndex );
    /** Returns the text of an existing field. */
    const String&   GetFieldText( size_t nIndex ) const;

    /** Calculates a field index at a specific pixel position. Returns in every
        case the index of an existing field.
        @param rnIndex  The index of the field is returned here.
        @return  TRUE, if the index value is valid. */
    void            GetExistingIndex( const Point& rPos, size_t& rnIndex );

    size_t GetSelectedField() const;
    void SetSelectedField(size_t nSelected);

    /** Notifies this control that the offset of the first field has been changed.
        The control has to adjust the selection to keep the same field selected
        on scrolling with scrollbar. */
    void            ModifySelectionOffset( long nOffsetDiff );
    /** Selects the next field. Called i.e. after moving a field from SELECT area. */
    void            SelectNext();
    /** Grabs focus and sets new selection. */
    void            GrabFocusWithSel( size_t nIndex );

    virtual void            Paint( const Rectangle& rRect );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );
    virtual void            Command( const CommandEvent& rCEvt );
    virtual void            MouseButtonDown( const MouseEvent& rMEvt );
    virtual void            MouseButtonUp( const MouseEvent& rMEvt );
    virtual void            MouseMove( const MouseEvent& rMEvt );
    virtual void            KeyInput( const KeyEvent& rKEvt );
    virtual void            GetFocus();
    virtual void            LoseFocus();

protected:
    FieldNames& GetFieldNames();
    const FieldNames& GetFieldNames() const;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    void FieldFocusChanged(size_t nOldSelected, size_t nFieldSelected);
    void AccessibleSetFocus(bool bOn);

    /** Updates the tab stop style bits. */
    void UpdateStyle();

    /** Draws the background. */
    void DrawBackground( OutputDevice& rDev );

    /** Draws a field into the specified rectangle. */
    void DrawField(
        OutputDevice& rDev, const Rectangle& rRect, FieldName& rText, bool bFocus );

    ScDPLayoutDlg* GetParentDlg() const;

    void AppendPaintable(Window* p);
    void DrawPaintables();
    void DrawInvertSelection();

    /** @return  The new selection index after moving to the given direction. */
    virtual size_t CalcNewFieldIndex( SCsCOL nDX, SCsROW nDY ) const = 0;

    /**
     * @param nIndex logical index of a field name, independent of scroll
     *               offsets.
     * @return Display position of the button.  The first displayed button is
     *         always 0 no matter what the current scroll offset is.  In case
     *         the field specified by the index is outside the visible range,
     *         <code>INVALID_INDEX</code> is returned.
     */
    virtual size_t GetDisplayPosition(size_t nIndex) const = 0;

    /** Draws the complete control. */
    virtual void Redraw() = 0;

private:
    /** @return  TRUE, if the field with the given index exists and the text is
                    too long for the button control. */
    bool                    IsShortenedText( size_t nIndex ) const;

    /** Moves the selected field to nDestIndex. */
    void                    MoveField( size_t nDestIndex );
    /** Moves the selected field to the given direction. */
    void                    MoveFieldRel( SCsCOL nDX, SCsROW nDY );

    /** Sets selection to the field with index nIndex. */
    void                    SetSelection( size_t nIndex );
    /** Sets selection to first field. */
    void                    SetSelectionHome();
    /** Sets selection to last field. */
    void                    SetSelectionEnd();
    /** Sets selection to new position relative to current. */
    void                    MoveSelection( SCsCOL nDX, SCsROW nDY );

private:
    typedef ::std::vector<Window*> Paintables;
    Paintables maPaintables;

    FieldNames maFieldNames;   /// String array of the field names and flags, if text fits into button.
    ScDPLayoutDlg*  mpDlg;
    FixedText*      mpCaption;     /// FixedText containing the name of the control.
    ::rtl::OUString maName;

    size_t mnFieldSelected; /// Currently selected field.

    com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > xAccessible;
    ScAccessibleDataPilotControl* pAccessible;
};

// ============================================================================

/**
 * Base class for field control with a horizontal scroll bar at the bottom.
 * Page, column, data and select fields are derived from this class.
 */
class ScDPHorFieldControl : public ScDPFieldControlBase
{
protected:
    virtual size_t          CalcNewFieldIndex(SCsCOL nDX, SCsROW nDY) const;
    virtual size_t          GetDisplayPosition(size_t nIndex) const;
    virtual void            Redraw();

public:
    ScDPHorFieldControl(
        ScDPLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption );

    virtual                 ~ScDPHorFieldControl();

    virtual void            CalcSize();
    virtual bool            IsValidIndex( size_t nIndex ) const;
    virtual Point           GetFieldPosition(size_t nIndex);
    virtual Size            GetFieldSize() const;
    virtual bool            GetFieldIndex( const Point& rPos, size_t& rnIndex );
    virtual String          GetDescription() const;

    virtual void ScrollToEnd();
    virtual void ScrollToShowSelection();
    virtual void ResetScrollBar();
    virtual void HandleWheelScroll(long nNotch);

private:
    bool GetFieldBtnPosSize(size_t nPos, Point& rPos, Size& rSize);
    void HandleScroll();

    DECL_LINK(ScrollHdl, ScrollBar*);
    DECL_LINK(EndScrollHdl, ScrollBar*);

private:

    ScrollBar               maScroll;

    size_t                  mnFieldBtnRowCount;
    size_t                  mnFieldBtnColCount;
};

// ============================================================================

class ScDPPageFieldControl : public ScDPHorFieldControl
{
public:
    ScDPPageFieldControl(
        ScDPLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption);
    virtual ~ScDPPageFieldControl();

    virtual ScDPFieldType GetFieldType() const;
    virtual String GetDescription() const;
};

// ============================================================================

class ScDPColFieldControl : public ScDPHorFieldControl
{
public:
    ScDPColFieldControl(
        ScDPLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption);
    virtual ~ScDPColFieldControl();

    virtual ScDPFieldType GetFieldType() const;
    virtual String GetDescription() const;
};

// ============================================================================

/**
 * Row field control with a vertical scroll bar.
 */
class ScDPRowFieldControl : public ScDPFieldControlBase
{
public:
    ScDPRowFieldControl(
        ScDPLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption );

    virtual                 ~ScDPRowFieldControl();

    virtual void            CalcSize();
    virtual bool            IsValidIndex( size_t nIndex ) const;
    virtual Point           GetFieldPosition( size_t nIndex );
    virtual Size            GetFieldSize() const;
    virtual bool            GetFieldIndex( const Point& rPos, size_t& rnIndex );
    virtual String          GetDescription() const;
    virtual ScDPFieldType   GetFieldType() const;

    virtual void ScrollToEnd();
    virtual void ScrollToShowSelection();
    virtual void ResetScrollBar();
    virtual void HandleWheelScroll(long nNotch);

protected:
    virtual size_t          CalcNewFieldIndex( SCsCOL nDX, SCsROW nDY ) const;
    virtual size_t          GetDisplayPosition(size_t nIndex) const;
    virtual void            Redraw();

private:
    bool GetFieldBtnPosSize(size_t nPos, Point& rPos, Size& rSize);
    void HandleScroll();

    DECL_LINK(ScrollHdl, ScrollBar*);
    DECL_LINK(EndScrollHdl, ScrollBar*);

private:

    ScDPFieldControlBase::ScrollBar maScroll;
    size_t                  mnColumnBtnCount;
};

// ============================================================================

class ScDPSelectFieldControl : public ScDPHorFieldControl
{
public:
    ScDPSelectFieldControl(
        ScDPLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption);
    virtual ~ScDPSelectFieldControl();

    virtual ScDPFieldType GetFieldType() const;
    virtual String GetDescription() const;
};

// ============================================================================

class ScDPDataFieldControl : public ScDPHorFieldControl
{
public:
    ScDPDataFieldControl( ScDPLayoutDlg* pParent, const ResId& rResId, FixedText* pCaption );
    virtual ~ScDPDataFieldControl();

    virtual ScDPFieldType GetFieldType() const;
    virtual Size GetFieldSize() const;
    virtual String GetDescription() const;
};

#endif // SC_FIELDWND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
