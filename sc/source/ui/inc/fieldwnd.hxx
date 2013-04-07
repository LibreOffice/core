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

#ifndef SC_FIELDWND_HXX
#define SC_FIELDWND_HXX

#include <vector>

#include <vcl/ctrl.hxx>
#include <vcl/fixed.hxx>
#include <vcl/scrbar.hxx>

#include "address.hxx"
#include "pivot.hxx"

struct ScPivotField;
class ScPivotLayoutDlg;
class ScAccessibleDataPilotControl;

extern const size_t PIVOTFIELD_INVALID;

/** Type of the pivot table field window. */
enum ScPivotFieldType
{
    PIVOTFIELDTYPE_PAGE = 0,        /// Window for all page fields.
    PIVOTFIELDTYPE_COL,             /// Window for all column fields.
    PIVOTFIELDTYPE_ROW,             /// Window for all row fields.
    PIVOTFIELDTYPE_DATA,            /// Window for all data fields.
    PIVOTFIELDTYPE_SELECT,          /// Selection window with all fields.
    PIVOTFIELDTYPE_UNKNOWN
};

/**
 * Represents a field area in the DataPilot layout dialog.  This base class
 * handles storage of field names and the accessibility object.
 */
class ScDPFieldControlBase : public Control
{
protected:
    struct FieldName
    {
        OUString maText;
        bool mbFits;
        sal_uInt8 mnDupCount;
        FieldName(const OUString& rText, bool bFits, sal_uInt8 nDupCount = 0);
        FieldName(const FieldName& r);

        OUString getDisplayedText() const;
    };
    typedef ::std::vector<FieldName> FieldNames;

public:
    typedef boost::ptr_vector<ScPivotFuncData> FuncDataType;

    struct FuncItem
    {
        SCCOL mnCol;
        sal_uInt16 mnFuncMask;
    };

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
        ScPivotLayoutDlg* pParent, const ResId& rResId, FixedText* pCaption, const char* pcHelpId);
    virtual ~ScDPFieldControlBase();

    virtual void CalcSize() = 0;

    virtual bool IsValidIndex( size_t nIndex ) const = 0;
    /** @return  The pixel position of a field (without bound check). */
    virtual Point GetFieldPosition( size_t nIndex ) = 0;
    /** Calculates the field index at a specific pixel position. */
    virtual size_t GetFieldIndex( const Point& rPos ) = 0;
    /** @return  The pixel size of a field. */
    virtual Size GetFieldSize() const = 0;

    /** @return The description of the control which is used for the accessibility objects. */
    virtual String GetDescription() const = 0;
    /** @return The type of the FieldWindow. */
    virtual ScPivotFieldType GetFieldType() const = 0;
    virtual void ScrollToShowSelection() = 0;
    virtual void ScrollToEnd() = 0;
    virtual void ResetScrollBar() = 0;
    virtual void HandleWheelScroll(long nNotch) = 0;

    /** @return The name of the control without shortcut. */
    OUString GetName() const;
    void SetName(const OUString& rName);

    /** @return  TRUE, if the field with the given index exists. */
    bool            IsExistingIndex( size_t nIndex ) const;

    void AppendField( const OUString& rText, const ScPivotFuncData& rFunc );

    /**
     * Inserts a field using the specified pixel position.
     *
     * @param rPos  The coordinates to insert the field.
     */
    size_t AddField(const OUString& rText, const Point& rPos, const ScPivotFuncData& rFunc);

    bool MoveField(size_t nCurPos, const Point& rPos, size_t& rnIndex);

    /** Remove a field by specified index. */
    void DeleteFieldByIndex( size_t nIndex );

    /** Returns the number of existing fields. */
    size_t          GetFieldCount() const;

    bool            IsEmpty() const;

    /** Removes all fields. */
    void            ClearFields();
    /** Changes the text on an existing field. */
    void SetFieldText(const OUString& rText, size_t nIndex, sal_uInt8 nDupCount);
    /** Returns the text of an existing field. */
    OUString GetFieldText( size_t nIndex ) const;

    /** Calculates a field index at a specific pixel position. Returns in every
        case the index of an existing field.
        @param rnIndex  The index of the field is returned here.
        @return  TRUE, if the index value is valid. */
    void            GetExistingIndex( const Point& rPos, size_t& rnIndex );

    size_t GetSelectedField() const;

    /** Selects the next field. Called i.e. after moving a field from SELECT area. */
    void            SelectNext();
    /** Grabs focus and sets new selection. */
    void            GrabFocusAndSelect( size_t nIndex );

    const ScPivotFuncData& GetFuncData(size_t nIndex) const;
    ScPivotFuncData& GetFuncData(size_t nIndex);

    void GetAllFuncItems(std::vector<FuncItem>& rItems) const;

    sal_uInt8 GetNextDupCount(const ScPivotFuncData& rData, size_t nSelfIndex) const;

    void ConvertToPivotArray(std::vector<ScPivotField>& rFields) const;

    size_t GetFieldIndexByData( const ScPivotFuncData& rData ) const;

    virtual void            Paint( const Rectangle& rRect );

protected:
    virtual void            StateChanged( StateChangedType nStateChange );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );
    virtual void            KeyInput( const KeyEvent& rKEvt );
    virtual void            Command( const CommandEvent& rCEvt );
    virtual void            MouseButtonDown( const MouseEvent& rMEvt );
    virtual void            MouseButtonUp( const MouseEvent& rMEvt );
    virtual void            MouseMove( const MouseEvent& rMEvt );
    virtual void            GetFocus();
    virtual void            LoseFocus();

protected:
    FieldNames& GetFieldNames();
    const FieldNames& GetFieldNames() const;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    void FieldFocusChanged(size_t nOldSelected, size_t nFieldSelected);

    /** Updates the tab stop style bits. */
    void UpdateStyle();

    /** Draw background color for the whole control area. */
    void DrawBackground( OutputDevice& rDev );

    /** Draw a single field button. */
    void DrawField( OutputDevice& rDev, const Rectangle& rRect,
                    FieldName& rText, bool bFocus );

    void AppendPaintable(Window* p);
    void DrawPaintables();
    void DrawInvertSelection();
    Size GetStdFieldBtnSize() const;

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
    /** Moves the selected field to nDestIndex. */
    void                    MoveField( size_t nDestIndex );
    /** Moves the selected field to the given direction. */
    void                    MoveFieldRel( SCsCOL nDX, SCsROW nDY );

    /** Selects a field and adjusts scrolling position to make the field visible. */
    void                    MoveSelection( size_t nSelectedIndex );
    /** Selects a field at a new position relative to the current. */
    void                    MoveSelection( SCsCOL nDX, SCsROW nDY );

    sal_uInt8 GetNextDupCount(const OUString& rFieldText) const;

private:
    typedef ::std::vector<Window*> Paintables;
    Paintables maPaintables;

    FuncDataType maFuncData;
    FieldNames maFieldNames;   /// String array of the field names and flags, if text fits into button.
    ScPivotLayoutDlg*  mpDlg;
    FixedText*      mpCaption;     /// FixedText containing the name of the control.
    OUString maName;

    size_t mnFieldSelected; /// Currently selected field.

    // Hold a helpful reference while we work with our weakref.
    class AccessRef
    {
        com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > mxRef;
    public:
        AccessRef( const com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > & rAccessible );
        ScAccessibleDataPilotControl *operator -> () const;
        bool is() { return mxRef.is(); }
    };

    com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > mxAccessible;
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
        ScPivotLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption, const char* pcHelpId);

    virtual                 ~ScDPHorFieldControl();

    virtual void            CalcSize();
    virtual bool            IsValidIndex( size_t nIndex ) const;
    virtual Point           GetFieldPosition(size_t nIndex);
    virtual Size            GetFieldSize() const;
    virtual size_t          GetFieldIndex( const Point& rPos );
    virtual String          GetDescription() const;

    virtual void ScrollToEnd();
    virtual void ScrollToShowSelection();
    virtual void ResetScrollBar();
    virtual void HandleWheelScroll(long nNotch);

private:
    bool GetFieldBtnPosSize(size_t nPos, Point& rPos, Size& rSize);
    void HandleScroll();

    DECL_LINK(ScrollHdl, void*);
    DECL_LINK(EndScrollHdl, void*);

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
        ScPivotLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption, const char* pcHelpId);
    virtual ~ScDPPageFieldControl();

    virtual ScPivotFieldType GetFieldType() const;
    virtual String GetDescription() const;
};

// ============================================================================

class ScDPColFieldControl : public ScDPHorFieldControl
{
public:
    ScDPColFieldControl(
        ScPivotLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption, const char* pcHelpId);
    virtual ~ScDPColFieldControl();

    virtual ScPivotFieldType GetFieldType() const;
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
        ScPivotLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption, const char* pcHelpId);

    virtual                 ~ScDPRowFieldControl();

    virtual void            CalcSize();
    virtual bool            IsValidIndex( size_t nIndex ) const;
    virtual Point           GetFieldPosition( size_t nIndex );
    virtual Size            GetFieldSize() const;
    virtual size_t          GetFieldIndex( const Point& rPos );
    virtual String          GetDescription() const;
    virtual ScPivotFieldType   GetFieldType() const;

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

    DECL_LINK(ScrollHdl, void*);
    DECL_LINK(EndScrollHdl, void*);

private:

    ScDPFieldControlBase::ScrollBar maScroll;
    size_t                  mnColumnBtnCount;
};

// ============================================================================

class ScDPSelectFieldControl : public ScDPHorFieldControl
{
public:
    ScDPSelectFieldControl(
        ScPivotLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption, const char* pcHelpId);
    virtual ~ScDPSelectFieldControl();

    virtual ScPivotFieldType GetFieldType() const;
    virtual String GetDescription() const;
};

// ============================================================================

class ScDPDataFieldControl : public ScDPHorFieldControl
{
public:
    ScDPDataFieldControl(
        ScPivotLayoutDlg* pParent, const ResId& rResId, FixedText* pCaption, const char* pcHelpId);
    virtual ~ScDPDataFieldControl();

    virtual ScPivotFieldType GetFieldType() const;
    virtual Size GetFieldSize() const;
    virtual String GetDescription() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
