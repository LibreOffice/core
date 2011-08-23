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

// ============================================================================

#ifndef _SC_ACCESSIBLECSVCONTROL_HXX
#define _SC_ACCESSIBLECSVCONTROL_HXX

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLETEXT_HPP_
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLETABLE_HPP_
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _SVX_ACCESSILE_STATIC_TEXT_BASE_HXX_
#include <bf_svx/AccessibleStaticTextBase.hxx>
#endif

#ifndef _SC_ACCESSIBLECONTEXTBASE_HXX
#include "AccessibleContextBase.hxx"
#endif
namespace utl { class AccessibleStateSetHelper; }
namespace binfilter {


// ============================================================================

class ScCsvControl;
//STRIP008 namespace utl { class AccessibleStateSetHelper; }

/** Accessible base class used for CSV controls. */
class ScAccessibleCsvControl : public ScAccessibleContextBase
{
protected:
    typedef ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > XAccessibleRef;
    typedef ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleRelationSet > XAccessibleRelationSetRef;
    typedef ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet > XAccessibleStateSetRef;

    typedef ::com::sun::star::awt::Point        AwtPoint;
    typedef ::com::sun::star::awt::Size         AwtSize;
    typedef ::com::sun::star::awt::Rectangle    AwtRectangle;

private:
    ScCsvControl*               mpControl;          /// Pointer to the VCL control.

public:
    explicit                    ScAccessibleCsvControl(
                                    const XAccessibleRef& rxParent,
                                    ScCsvControl& rControl,
                                    sal_uInt16 nRole );
    virtual                     ~ScAccessibleCsvControl();
    virtual void SAL_CALL       disposing();

    // XAccessibleComponent ---------------------------------------------------

    /** Returns the child at the specified point (cell returns NULL). */
    virtual XAccessibleRef SAL_CALL getAccessibleAtPoint( const AwtPoint& rPoint )
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns true, if the control is visible. */
    virtual sal_Bool SAL_CALL isVisible() throw( ::com::sun::star::uno::RuntimeException );

    /** Sets the focus to this control. */
    virtual void SAL_CALL grabFocus() throw( ::com::sun::star::uno::RuntimeException );

    // events -----------------------------------------------------------------
public:
    /** Sends a GetFocus or LoseFocus event to all listeners. */
    virtual void SendFocusEvent( bool bFocused );
    /** Sends a caret changed event to all listeners. */
    virtual void SendCaretEvent();
    /** Sends a visible area changed event to all listeners. */
    virtual void SendVisibleEvent();
    /** Sends a selection changed event to all listeners. */
    virtual void SendSelectionEvent();
    /** Sends a table model changed event for changed cell contents to all listeners. */
    virtual void SendTableUpdateEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn, bool bAllRows );
    /** Sends a table model changed event for an inserted column to all listeners. */
    virtual void SendInsertColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn );
    /** Sends a table model changed event for a removed column to all listeners. */
    virtual void SendRemoveColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn );

    // helpers ----------------------------------------------------------------
protected:
    /** Returns this object's current bounding box relative to the desktop. */
    virtual Rectangle GetBoundingBoxOnScreen() const throw( ::com::sun::star::uno::RuntimeException );
    /** Returns this object's current bounding box relative to the parent object. */
    virtual Rectangle GetBoundingBox() const throw( ::com::sun::star::uno::RuntimeException );

    /** Creates a new UUID in rSeq, if it is empty. Locks mutex internally. */
    void getUuid( ::com::sun::star::uno::Sequence< sal_Int8 >& rSeq );

    /** Returns whether the object is alive. Must be called with locked mutex. */
    inline bool implIsAlive() const { return !rBHelper.bDisposed && !rBHelper.bInDispose && mpControl; }
    /** Throws an exception, if the object is disposed/disposing or any pointer
        is missing. Should be used with locked mutex! */
    void ensureAlive() const throw( ::com::sun::star::lang::DisposedException );

    /** Returns the VCL control. Assumes a living object. */
    ScCsvControl& implGetControl() const;

    /** Returns the first child of rxParentObj, which has the role nRole. */
    XAccessibleRef implGetChildByRole( const XAccessibleRef& rxParentObj, sal_uInt16 nRole )
        throw( ::com::sun::star::uno::RuntimeException );
    /** Creates a StateSetHelper and fills it with DEFUNC, OPAQUE, ENABLED, SHOWING and VISIBLE. */
    ::utl::AccessibleStateSetHelper* implCreateStateSet();

    /** Disposes the object. This is a helper called from destructors only. */
    void implDispose();

    /** Converts the screen position rPos to a position relative to this control. */
    Point implGetRelPos( const Point& rPos ) const;
    /** Converts the control-relative position to an absolute screen position. */
    Point implGetAbsPos( const Point& rPos ) const;
};


// ============================================================================

class ScCsvRuler;

typedef ::cppu::ImplHelper1<
        ::com::sun::star::accessibility::XAccessibleText >
    ScAccessibleCsvRulerImpl;

/** Accessible class representing the CSV ruler control. */
class ScAccessibleCsvRuler : public ScAccessibleCsvControl, public ScAccessibleCsvRulerImpl
{
protected:
    typedef ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue > PropertyValueSeq;

private:
    ::rtl::OUStringBuffer       maBuffer;   /// Contains the text representation of the ruler.

public:
    explicit                    ScAccessibleCsvRuler( ScCsvRuler& rRuler );
    virtual                     ~ScAccessibleCsvRuler();

    // XAccessibleComponent -----------------------------------------------------

    virtual sal_Int32 SAL_CALL getForeground(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getBackground(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext -----------------------------------------------------

    /** Returns the child count (the ruler does not have children). */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Throws an exception (the ruler does not have childern). */
    virtual XAccessibleRef SAL_CALL getAccessibleChild( sal_Int32 nIndex )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    /** Returns the relation to the grid control. */
    virtual XAccessibleRelationSetRef SAL_CALL getAccessibleRelationSet()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the current set of states. */
    virtual XAccessibleStateSetRef SAL_CALL getAccessibleStateSet()
        throw( ::com::sun::star::uno::RuntimeException );

    // XAccessibleText --------------------------------------------------------

    /** Return the position of the caret. */
    virtual sal_Int32 SAL_CALL getCaretPosition() throw( ::com::sun::star::uno::RuntimeException );

    /** Sets the position of the caret. */
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    /** Returns the specified character. */
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    /** Returns the attributes of the specified character. */
    virtual PropertyValueSeq SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    /** Returns the screen coordinates of the specified character. */
    virtual AwtRectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    /** Returns the count of characters. */
    virtual sal_Int32 SAL_CALL getCharacterCount() throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the character index at the specified coordinate (object's coordinate system). */
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const AwtPoint& rPoint )
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the selected text (ruler returns empty string). */
    virtual ::rtl::OUString SAL_CALL getSelectedText() throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the start index of the selection (ruler returns -1). */
    virtual sal_Int32 SAL_CALL getSelectionStart() throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the end index of the selection (ruler returns -1). */
    virtual sal_Int32 SAL_CALL getSelectionEnd() throw( ::com::sun::star::uno::RuntimeException );

    /** Selects a part of the text (ruler does nothing). */
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    /** Returns the entire text. */
    virtual ::rtl::OUString SAL_CALL getText() throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the specified range [Start,End) of the text. */
    virtual ::rtl::OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    /** Returns the specified text portion. */
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    /** Copies the specified text range into the clipboard (ruler does nothing). */
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    // XInterface -------------------------------------------------------------

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL acquire() throw();

    virtual void SAL_CALL release() throw();

    // XServiceInfo -----------------------------------------------------------

    /** Returns an identifier for the implementation of this object. */
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException );

    // XTypeProvider ----------------------------------------------------------

    /** Returns a sequence with all supported interface types. */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns an implementation ID. */
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( ::com::sun::star::uno::RuntimeException );

    // events -----------------------------------------------------------------
public:
    /** Sends a caret changed event to all listeners. */
    virtual void SendCaretEvent();

    // helpers ----------------------------------------------------------------
private:
    /** Returns this object's name. */
    virtual ::rtl::OUString SAL_CALL createAccessibleName()
        throw( ::com::sun::star::uno::RuntimeException );
    /** Returns this object's description. */
    virtual ::rtl::OUString SAL_CALL createAccessibleDescription()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Throws an exception, if the specified character position is invalid (outside 0..len-1). */
    void ensureValidIndex( sal_Int32 nIndex ) const
        throw( ::com::sun::star::lang::IndexOutOfBoundsException );
    /** Throws an exception, if the specified character position is invalid (outside 0..len). */
    void ensureValidIndexWithEnd( sal_Int32 nIndex ) const
        throw( ::com::sun::star::lang::IndexOutOfBoundsException );
    /** Throws an exception, if the specified character range [Start,End) is invalid.
        @descr  If Start>End, swaps Start and End before checking. */
    void ensureValidRange( sal_Int32& rnStartIndex, sal_Int32& rnEndIndex ) const
        throw( ::com::sun::star::lang::IndexOutOfBoundsException );

    /** Returns the VCL ruler control. Assumes a living object. */
    ScCsvRuler& implGetRuler() const;

    /** Builds the entire string buffer. */
    void constructStringBuffer() throw( ::com::sun::star::uno::RuntimeException );
    /** Returns the character count of the text. */
    sal_Int32 implGetTextLength() const;

    /** Returns true, if the character at the specified index has a split. */
    bool implHasSplit( sal_Int32 nApiPos );

    /** Returns the first character index with equal formatting as at nApiPos. */
    sal_Int32 implGetFirstEqualFormatted( sal_Int32 nApiPos );
    /** Returns the last character index with equal formatting as at nApiPos. */
    sal_Int32 implGetLastEqualFormatted( sal_Int32 nApiPos );
};


// ============================================================================

class ScCsvGrid;

typedef ::cppu::ImplHelper2<
        ::com::sun::star::accessibility::XAccessibleTable,
        ::com::sun::star::accessibility::XAccessibleSelection >
    ScAccessibleCsvGridImpl;

/** Accessible class representing the CSV grid control. */
class ScAccessibleCsvGrid : public ScAccessibleCsvControl, public ScAccessibleCsvGridImpl
{
protected:
    typedef ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleTable > XAccessibleTableRef;

public:
    explicit                    ScAccessibleCsvGrid( ScCsvGrid& rGrid );
    virtual                     ~ScAccessibleCsvGrid();

    // XAccessibleComponent ---------------------------------------------------

    /** Returns the cell at the specified point. */
    virtual XAccessibleRef SAL_CALL getAccessibleAtPoint( const AwtPoint& rPoint )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getForeground(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getBackground(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext -----------------------------------------------------

    /** Returns the child count (count of cells in the table). */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the specified child cell. */
    virtual XAccessibleRef SAL_CALL getAccessibleChild( sal_Int32 nIndex )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    /** Returns the relation to the ruler control. */
    virtual XAccessibleRelationSetRef SAL_CALL getAccessibleRelationSet()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the current set of states. */
    virtual XAccessibleStateSetRef SAL_CALL getAccessibleStateSet()
        throw( ::com::sun::star::uno::RuntimeException );

    // XAccessibleTable -------------------------------------------------------

    /** Returns the number of rows in the table. */
    virtual sal_Int32 SAL_CALL getAccessibleRowCount()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the number of columns in the table. */
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the description of the specified row in the table. */
    virtual ::rtl::OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow )
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the description text of the specified column in the table. */
    virtual ::rtl::OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn )
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the number of rows occupied at a specified row and column.
        @descr  Returns always 1 (Merged cells not supported). */
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the number of rows occupied at a specified row and column.
        @descr  Returns always 1 (Merged cells not supported). */
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the row headers as an AccessibleTable. */
    virtual XAccessibleTableRef SAL_CALL getAccessibleRowHeaders()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the column headers as an AccessibleTable. */
    virtual XAccessibleTableRef SAL_CALL getAccessibleColumnHeaders()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the selected rows as a sequence. */
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleRows()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the selected columns as a sequence. */
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleColumns()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns true, if the specified row is selected. */
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow )
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns true, if the specified column is selected. */
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn )
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the accessible cell object at the specified position. */
    virtual XAccessibleRef SAL_CALL getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    /** Returns the caption object of the table. */
    virtual XAccessibleRef SAL_CALL getAccessibleCaption()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the summary description object of the table. */
    virtual XAccessibleRef SAL_CALL getAccessibleSummary()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns true, if the cell at a specified position is selected. */
    virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
        throw( ::com::sun::star::uno::RuntimeException );

    // XAccessibleExtendedTable -----------------------------------------------

    /** Returns the child index of the cell at the specified position. */
    virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the row index of the specified child. */
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex )
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the column index of the specified child. */
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex )
        throw( ::com::sun::star::uno::RuntimeException );

    // XAccessibleSelection ---------------------------------------------------

    /** Selects the specified child (selects the entire column or the entire table). */
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    /** Returns true, if the specified child is selected. */
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    /** Deselects all cells. */
    virtual void SAL_CALL clearAccessibleSelection()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Selects all cells. */
    virtual void SAL_CALL selectAllAccessibleChildren()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the count of selected children. */
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the child with the specified index in all selected children. */
    virtual XAccessibleRef SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    /** Deselects the child with the specified index in all selected children. */
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    // XInterface -------------------------------------------------------------

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL acquire() throw();

    virtual void SAL_CALL release() throw();

    // XServiceInfo -----------------------------------------------------------

    /** Returns an identifier for the implementation of this object. */
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException );

    // XTypeProvider ----------------------------------------------------------

    /** Returns a sequence with all supported interface types. */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns an implementation ID. */
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( ::com::sun::star::uno::RuntimeException );

    // events -----------------------------------------------------------------
public:
    /** Sends a GetFocus or LoseFocus event to all listeners. */
    virtual void SendFocusEvent( bool bFocused );
    /** Sends a table model changed event for changed cell contents to all listeners. */
    virtual void SendTableUpdateEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn, bool bAllRows );
    /** Sends a table model changed event for an inserted column to all listeners. */
    virtual void SendInsertColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn );
    /** Sends a table model changed event for a removed column to all listeners. */
    virtual void SendRemoveColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn );

    // helpers ----------------------------------------------------------------
private:
    /** Returns this object's name. */
    virtual ::rtl::OUString SAL_CALL createAccessibleName()
        throw( ::com::sun::star::uno::RuntimeException );
    /** Returns this object's description. */
    virtual ::rtl::OUString SAL_CALL createAccessibleDescription()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Throws an exception, if nIndex is not a valid child index. */
    void ensureValidIndex( sal_Int32 nIndex ) const
        throw( ::com::sun::star::lang::IndexOutOfBoundsException );
    /** Throws an exception, if the specified position is invalid. */
    void ensureValidPosition( sal_Int32 nRow, sal_Int32 nColumn ) const
        throw( ::com::sun::star::lang::IndexOutOfBoundsException );

    /** Returns the VCL grid control. Assumes a living object. */
    ScCsvGrid& implGetGrid() const;

    /** Returns true, if the specified column (including header) is selected. */
    bool implIsColumnSelected( sal_Int32 nColumn ) const;
    /** Selects the specified column (including header). */
    void implSelectColumn( sal_Int32 nColumn, bool bSelect );

    /** Returns the count of visible rows in the table (including header). */
    sal_Int32 implGetRowCount() const;
    /** Returns the total column count in the table (including header). */
    sal_Int32 implGetColumnCount() const;
    /** Returns the count of selected columns in the table. */
    sal_Int32 implGetSelColumnCount() const;
    /** Returns the total cell count in the table (including header). */
    inline sal_Int32 implGetCellCount() const { return implGetRowCount() * implGetColumnCount(); }

    /** Returns the row index from cell index (including header). */
    inline sal_Int32 implGetRow( sal_Int32 nIndex ) const { return nIndex / implGetColumnCount(); }
    /** Returns the column index from cell index (including header). */
    inline sal_Int32 implGetColumn( sal_Int32 nIndex ) const { return nIndex % implGetColumnCount(); }
    /** Returns the absolute column index of the nSelColumn-th selected column. */
    sal_Int32 implGetSelColumn( sal_Int32 nSelColumn ) const;
    /** Returns the child index from cell position (including header). */
    inline sal_Int32 implGetIndex( sal_Int32 nRow, sal_Int32 nColumn ) const { return nRow * implGetColumnCount() + nColumn; }

    /** Returns the contents of the specified cell (including header). Indexes must be valid. */
    String implGetCellText( sal_Int32 nRow, sal_Int32 nColumn ) const;
    /** Creates a new accessible object of the specified cell. Indexes must be valid. */
    ScAccessibleCsvControl* implCreateCellObj( sal_Int32 nRow, sal_Int32 nColumn ) const;
};


// ============================================================================

/** Accessible class representing a cell of the CSV grid control. */
class ScAccessibleCsvCell : public ScAccessibleCsvControl, public accessibility::AccessibleStaticTextBase
{
protected:
    typedef ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue >    PropertyValueSeq;
    typedef ::std::auto_ptr< SvxEditSource >        SvxEditSourcePtr;

private:
    String                      maCellText; /// The text contents of this cell.
    sal_Int32                   mnLine;     /// The grid line index (core index).
    sal_uInt32                  mnColumn;   /// The grid column index (core index).
    sal_Int32                   mnIndex;    /// The index of the cell in the table.

public:
    explicit                    ScAccessibleCsvCell(
                                    ScCsvGrid& rGrid,
                                    const String& rCellText,
                                    sal_Int32 nRow, sal_Int32 nColumn );
    virtual                     ~ScAccessibleCsvCell();
    virtual void SAL_CALL       disposing();

    // XAccessibleComponent ---------------------------------------------------

    /** Sets the focus to the column of this cell. */
    virtual void SAL_CALL grabFocus() throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getForeground(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getBackground(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext -----------------------------------------------------

    /** Returns the child count. */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the specified child. */
    virtual XAccessibleRef SAL_CALL getAccessibleChild( sal_Int32 nIndex )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    /** Returns the index of this cell in the table. */
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the relation to the ruler control. */
    virtual XAccessibleRelationSetRef SAL_CALL getAccessibleRelationSet()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the current set of states. */
    virtual XAccessibleStateSetRef SAL_CALL getAccessibleStateSet()
        throw( ::com::sun::star::uno::RuntimeException );

    // XInterface -------------------------------------------------------------

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL acquire() throw();

    virtual void SAL_CALL release() throw();

    // XServiceInfo -----------------------------------------------------------

    /** Returns an identifier for the implementation of this object. */
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException );

    // XTypeProvider ----------------------------------------------------------

    /** Returns a sequence with all supported interface types. */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns an implementation ID. */
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( ::com::sun::star::uno::RuntimeException );

    // helpers ----------------------------------------------------------------
protected:
    /** Returns this object's current bounding box relative to the desktop. */
    virtual Rectangle GetBoundingBoxOnScreen() const throw( ::com::sun::star::uno::RuntimeException );
    /** Returns this object's current bounding box relative to the parent object. */
    virtual Rectangle GetBoundingBox() const throw( ::com::sun::star::uno::RuntimeException );

private:
    /** Returns this object's name. */
    virtual ::rtl::OUString SAL_CALL createAccessibleName()
        throw( ::com::sun::star::uno::RuntimeException );
    /** Returns this object's description. */
    virtual ::rtl::OUString SAL_CALL createAccessibleDescription()
        throw( ::com::sun::star::uno::RuntimeException );

    /** Returns the VCL grid control. Assumes a living object. */
    ScCsvGrid& implGetGrid() const;
    /** Returns the pixel position of the cell (rel. to parent), regardless of visibility. */
    Point implGetRealPos() const;
    /** Returns the width of the character count */
    sal_uInt32 implCalcPixelWidth(sal_uInt32 nChars) const;
    /** Returns the pixel size of the cell, regardless of visibility. */
    Size implGetRealSize() const;
    /** Returns the bounding box of the cell relative in the table. */
    Rectangle implGetBoundingBox() const;

    /** Creates the edit source the text helper needs. */
    ::std::auto_ptr< SvxEditSource > implCreateEditSource();
};


// ============================================================================

} //namespace binfilter
#endif

