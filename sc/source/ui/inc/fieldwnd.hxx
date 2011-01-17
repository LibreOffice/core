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

#include <vector>
#include "address.hxx"
#include <vcl/ctrl.hxx>
#include <vcl/fixed.hxx>
#include <cppuhelper/weakref.hxx>

#define PAGE_SIZE   16      // count of visible fields for scrollbar
#define LINE_SIZE   8       // count of fields per column for scrollbar
#define MAX_FIELDS  8       // maximum count of fields for row/col/data area
#define MAX_PAGEFIELDS 10   // maximum count of fields for page area

#define OWIDTH  PivotGlobal::nObjWidth
#define OHEIGHT PivotGlobal::nObjHeight
#define SSPACE  PivotGlobal::nSelSpace

class ScDPLayoutDlg;
class ScAccessibleDataPilotControl;

//===================================================================

/** Type of content area. */
enum ScDPFieldType
{
    TYPE_PAGE,              /// Area for all page fields.
    TYPE_ROW,               /// Area for all row fields.
    TYPE_COL,               /// Area for all column fields.
    TYPE_DATA,              /// Area for all data fields.
    TYPE_SELECT             /// Selection area with all fields.
};

//-------------------------------------------------------------------

/** Represents a field area in the DataPilot layout dialog. */
class ScDPFieldWindow : public Control
{
private:
    typedef ::std::pair< String, bool > FieldString;    // true = text fits into button

    String                  aName;          /// name of the control, used in Accessibility
    ScDPLayoutDlg*          pDlg;           /// Parent dialog.
    Rectangle               aWndRect;       /// Area rectangle in pixels.
    FixedText*              pFtCaption;     /// FixedText containing the name of the control.
    Point                   aTextPos;       /// Position of the caption text.
    std::vector< FieldString > aFieldArr;   /// String array of the field names and flags, if text fits into button.
    ScDPFieldType           eType;          /// Type of this area.
    Color                   aFaceColor;     /// Color for dialog background.
    Color                   aWinColor;      /// Color for window background.
    Color                   aTextColor;     /// Color for text in buttons.
    Color                   aWinTextColor;  /// Color for text in field windows.
    size_t                  nFieldSize;     /// Maximum count of fields.
    size_t                  nFieldSelected; /// Currently selected field.

    com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > xAccessible;
    ScAccessibleDataPilotControl* pAccessible;

    /** Initilize the object. */
    void                    Init();

    /** Reads all needed style settings. */
    void                    GetStyleSettings();

    /** Draws the background. */
    void                    DrawBackground( OutputDevice& rDev );
    /** Draws a field into the specified rectangle. */
    void                    DrawField(
                                OutputDevice& rDev,
                                const Rectangle& rRect,
                                FieldString& rText,
                                bool bFocus );

    /** @return  sal_True, if the field index is inside of the control area. */
    bool                    IsValidIndex( size_t nIndex ) const;
    /** @return  sal_True, if the field with the given index exists. */
    bool                    IsExistingIndex( size_t nIndex ) const;
    /** @return  sal_True, if the field with the given index exists and the text is
                    too long for the button control. */
    bool                    IsShortenedText( size_t nIndex ) const;
    /** @return  The new selection index after moving to the given direction. */
    size_t                  CalcNewFieldIndex( SCsCOL nDX, SCsROW nDY ) const;

    /** Sets selection to the field with index nIndex. */
    void                    SetSelection( size_t nIndex );
    /** Sets selection to first field. */
    void                    SetSelectionHome();
    /** Sets selection to last field. */
    void                    SetSelectionEnd();
    /** Sets selection to new position relative to current. */
    void                    MoveSelection( sal_uInt16 nKeyCode, SCsCOL nDX, SCsROW nDY );

    /** Moves the selected field to nDestIndex. */
    void                    MoveField( size_t nDestIndex );
    /** Moves the selected field to the given direction. */
    void                    MoveFieldRel( SCsCOL nDX, SCsROW nDY );

    /** Updates the tab stop style bits. */
    void                    UpdateStyle();

protected:
    virtual void            Paint( const Rectangle& rRect );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );
    virtual void            MouseButtonDown( const MouseEvent& rMEvt );
    virtual void            MouseButtonUp( const MouseEvent& rMEvt );
    virtual void            MouseMove( const MouseEvent& rMEvt );
    virtual void            KeyInput( const KeyEvent& rKEvt );
    virtual void            GetFocus();
    virtual void            LoseFocus();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

public:
                            ScDPFieldWindow(
                                ScDPLayoutDlg* pDialog,
                                const ResId& rResId,
                                ScDPFieldType eFieldType,
                                FixedText* pFtFieldCaption );
                            ScDPFieldWindow(
                                ScDPLayoutDlg* pDialog,
                                const ResId& rResId,
                                ScDPFieldType eFieldType,
                                const String& aName );
    virtual                 ~ScDPFieldWindow();

    /** Reads the FixedText's text with mnemonic and hides the FixedText. */
    void                    UseMnemonic();

    /** Draws the complete control. */
    void                    Redraw();

    /** @return  The pixel position of a field (without bound check). */
    Point                   GetFieldPosition( size_t nIndex ) const;
    /** @return  The pixel size of a field. */
    Size                    GetFieldSize() const;

    /** @return  The index of the selected field. */
    inline bool             IsEmpty() const { return aFieldArr.empty(); }
    /** @return  The index of the selected field. */
    inline size_t           GetSelectedField() const { return nFieldSelected; }
    /** @return  The pixel position of the last possible field. */
    Point                   GetLastPosition() const;

    /** @return  The count of existing fields. */
    inline size_t           GetFieldCount() const { return aFieldArr.size(); }
    /** Inserts a field to the specified index. */
    void                    AddField( const String& rText, size_t nNewIndex );
    /** Removes a field from the specified index. */
    void                    DelField( size_t nDelIndex );
    /** Removes all fields. */
    void                    ClearFields();
    /** Changes the text on an existing field. */
    void                    SetFieldText( const String& rText, size_t nIndex );
    /** Returns the text of an existing field. */
    const String&           GetFieldText( size_t nIndex ) const;

    /** Inserts a field using the specified pixel position.
        @param rPos  The coordinates to insert the field.
        @param rnIndex  The new index of the field is returned here.
        @return  sal_True, if the field has been created. */
    bool                    AddField( const String& rText, const Point& rPos, size_t& rnIndex );
    /** Calculates the field index at a specific pixel position.
        @param rnIndex  The index of the field is returned here.
        @return  sal_True, if the index value is valid. */
    bool                    GetFieldIndex( const Point& rPos, size_t& rnIndex ) const;
    /** Calculates a field index at a specific pixel position. Returns in every
        case the index of an existing field.
        @param rnIndex  The index of the field is returned here.
        @return  sal_True, if the index value is valid. */
    void                    GetExistingIndex( const Point& rPos, size_t& rnIndex );

    /** Notifies this control that the offset of the first field has been changed.
        The control has to adjust the selection to keep the same field selected
        on scrolling with scrollbar. */
    void                    ModifySelectionOffset( long nOffsetDiff );
    /** Selects the next field. Called i.e. after moving a field from SELECT area. */
    void                    SelectNext();

    /** @return The name of the control without shortcut. */
    inline String           GetName() const { return aName; }

    /** @return The description of the control which is used for the accessibility objects. */
    String                  GetDescription() const;

    /** Grabs focus and sets new selection. */
    void                    GrabFocusWithSel( size_t nIndex );

    /** @return The type of the FieldWindow. */
    inline ScDPFieldType    GetType() const { return eType; }
};

//===================================================================

#endif // SC_FIELDWND_HXX
