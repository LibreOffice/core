/*************************************************************************
 *
 *  $RCSfile: fieldwnd.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 15:09:36 $
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

#ifndef SC_FIELDWND_HXX
#define SC_FIELDWND_HXX


#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#define MAX_LABELS  256
#define PAGE_SIZE   16      // count of visible fields for scrollbar
#define LINE_SIZE   8       // count of fields per column for scrollbar
#define MAX_FIELDS  8       // maximum count of fields for row/col/data area

#define OWIDTH  PivotGlobal::nObjWidth
#define OHEIGHT PivotGlobal::nObjHeight
#define SSPACE  PivotGlobal::nSelSpace

class ScDPLayoutDlg;
class ScAccessibleDataPilotControl;

//===================================================================

/** Type of content area. */
enum ScDPFieldType
{
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
    String                  aName;          /// name of the control, used in Accessibility
    ScDPLayoutDlg*          pDlg;           /// Parent dialog.
    Rectangle               aWndRect;       /// Area rectangle in pixels.
    FixedText*              pFtCaption;     /// FixedText containing the name of the control.
    Point                   aTextPos;       /// Position of the caption text.
    String**                aFieldArr;      /// Pointer to string array of the field names.
    ScDPFieldType           eType;          /// Type of this area.
    Color                   aFaceColor;     /// Color for dialog background.
    Color                   aWinColor;      /// Color for window background.
    Color                   aTextColor;     /// Color for text in buttons.
    Color                   aWinTextColor;  /// Color for text in field windows.
    long                    nFieldSize;     /// Maximum count of fields.
    long                    nFieldCount;    /// Count of existing fields.
    long                    nFieldSelected; /// Currently selected field.

    com::sun::star::uno::WeakReference< ::drafts::com::sun::star::accessibility::XAccessible > xAccessible;
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
                                const String& rText,
                                BOOL bSelected );

    /** @return  TRUE, if the field index is inside of the control area. */
    BOOL                    IsValidIndex( long nIndex ) const;
    /** @return  TRUE, if the field with the given index exists. */
    BOOL                    IsExistingIndex( long nIndex ) const;
    /** @return  The new selection index after moving to the given direction. */
    long                    CalcNewFieldIndex( short nDX, short nDY ) const;

    /** Sets selection to the field with index nIndex. */
    void                    SetSelection( long nIndex );
    /** Sets selection to first field. */
    void                    SetSelectionHome();
    /** Sets selection to last field. */
    void                    SetSelectionEnd();
    /** Sets selection to new position relative to current. */
    void                    MoveSelection( USHORT nKeyCode, short nDX, short nDY );

    /** Moves the selected field to nDestIndex. */
    void                    MoveField( long nDestIndex );
    /** Moves the selected field to the given direction. */
    void                    MoveFieldRel( short nDX, short nDY );

protected:
    virtual void            Paint( const Rectangle& rRect );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );
    virtual void            MouseButtonDown( const MouseEvent& rMEvt );
    virtual void            MouseButtonUp( const MouseEvent& rMEvt );
    virtual void            MouseMove( const MouseEvent& rMEvt );
    virtual void            KeyInput( const KeyEvent& rKEvt );
    virtual void            GetFocus();
    virtual void            LoseFocus();
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > CreateAccessible();

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

    /** Draws the complete control. */
    void                    Redraw();

    /** @return  The pixel position of a field (without bound check). */
    Point                   GetFieldPosition( long nIndex ) const;
    /** @return  The pixel size of a field. */
    Size                    GetFieldSize() const;

    /** @return  The index of the selected field. */
    inline BOOL             IsEmpty() const             { return nFieldCount == 0; }
    /** @return  The index of the selected field. */
    inline long             GetSelectedField() const    { return nFieldSelected; }
    /** @return  The pixel position of the last possible field. */
    Point                   GetLastPosition() const;

    /** @return  The count of existing fields. */
    long                    GetFieldCount() const       { return nFieldCount; }
    /** Inserts a field to the specified index. */
    void                    AddField( const String& rText, long nNewIndex );
    /** Removes a field from the specified index. */
    void                    DelField( long nDelIndex );
    /** Removes all fields. */
    void                    ClearFields();
    /** Changes the text on an existing field. */
    void                    SetFieldText( const String& rText, long nIndex );
    /** Returns the text of an existing field. */
    const String&           GetFieldText(long nIndex) const;

    /** Inserts a field using the specified pixel position.
        @param rPos  The coordinates to insert the field.
        @param rnIndex  The new index of the field is returned here.
        @return  TRUE, if the field has been created. */
    BOOL                    AddField( const String& rText, const Point& rPos, long& rnIndex );
    /** Calculates the field index at a specific pixel position.
        @param rnIndex  The index of the field is returned here.
        @return  TRUE, if the index value is valid. */
    BOOL                    GetFieldIndex( const Point& rPos, long& rnIndex ) const;
    /** Calculates a field index at a specific pixel position. Returns in every
        case the index of an existing field.
        @param rnIndex  The index of the field is returned here.
        @return  TRUE, if the index value is valid. */
    void                    GetExistingIndex( const Point& rPos, long& rnIndex );

    /** Notifies this control that the offset of the first field has been changed.
        The control has to adjust the selection to keep the same field selected
        on scrolling with scrollbar. */
    void                    ModifySelectionOffset( long nOffsetDiff );
    /** Selects the next field. Called i.e. after moving a field from SELECT area. */
    void                    SelectNext();

    /** @return The name of the control without shortcut. */
    String                  GetName()const              { return aName; }

    /** @return The description of the control which is used for the accessibility objects. */
    String                  GetDescription()const;

    /** Grabs focus and sets new selection. */
    void                    GrabFocusWithSel( long nIndex );

    /** @return The type of the FieldWindow. */
    ScDPFieldType           GetType() const { return eType; }
};

//===================================================================

#endif // SC_FIELDWND_HXX
