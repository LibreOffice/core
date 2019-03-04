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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLECSVCONTROL_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLECSVCONTROL_HXX

#include <memory>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <tools/gen.hxx>
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <editeng/AccessibleStaticTextBase.hxx>
#include <comphelper/uno3.hxx>
#include <vcl/vclptr.hxx>
#include "AccessibleContextBase.hxx"
#include <map>

class ScCsvControl;
namespace utl { class AccessibleStateSetHelper; }

/** Accessible base class used for CSV controls. */
class ScAccessibleCsvControl : public ScAccessibleContextBase
{
private:
    VclPtr<ScCsvControl>               mpControl;          /// Pointer to the VCL control.

public:
    explicit                    ScAccessibleCsvControl(
                                    const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
                                    ScCsvControl& rControl,
                                    sal_uInt16 nRole );
    virtual                     ~ScAccessibleCsvControl() override;

    using ScAccessibleContextBase::disposing;
    virtual void SAL_CALL       disposing() override;

    /** Returns true, if the control is visible. */
    virtual bool isVisible() override;

    // XAccessibleComponent ---------------------------------------------------

    /** Returns the child at the specified point (cell returns NULL). */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& rPoint ) override;

    /** Sets the focus to this control. */
    virtual void SAL_CALL grabFocus() override;

    // events -----------------------------------------------------------------
public:
    /** Sends a GetFocus or LoseFocus event to all listeners. */
    virtual void SendFocusEvent( bool bFocused );
    /** Sends a caret changed event to all listeners. */
    virtual void SendCaretEvent();
    /** Sends a visible area changed event to all listeners. */
    void SendVisibleEvent();
    /** Sends a selection changed event to all listeners. */
    void SendSelectionEvent();
    /** Sends a table model changed event for changed cell contents to all listeners. */
    virtual void SendTableUpdateEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn, bool bAllRows );
    /** Sends a table model changed event for an inserted column to all listeners. */
    virtual void SendInsertColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn );
    /** Sends a table model changed event for a removed column to all listeners. */
    virtual void SendRemoveColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn );

    // helpers ----------------------------------------------------------------
protected:
    /** Returns this object's current bounding box relative to the desktop. */
    virtual tools::Rectangle GetBoundingBoxOnScreen() const override;
    /** Returns this object's current bounding box relative to the parent object. */
    virtual tools::Rectangle GetBoundingBox() const override;

    /** Returns whether the object is alive. Must be called with locked mutex. */
    bool implIsAlive() const { return !rBHelper.bDisposed && !rBHelper.bInDispose && mpControl; }
    /** @throws css::lang::DisposedException if the object is disposed/disposing or any pointer
        is missing. Should be used with locked mutex! */
    void ensureAlive() const;

    /** Returns the VCL control. Assumes a living object. */
    ScCsvControl& implGetControl() const;

    /** Returns the first child of rxParentObj, which has the role nRole.

        @throws css::uno::RuntimeException
    */
    static css::uno::Reference< css::accessibility::XAccessible > implGetChildByRole( const css::uno::Reference< css::accessibility::XAccessible >& rxParentObj, sal_uInt16 nRole );
    /** Creates a StateSetHelper and fills it with DEFUNC, OPAQUE, ENABLED, SHOWING and VISIBLE. */
    ::utl::AccessibleStateSetHelper* implCreateStateSet();

    /** Disposes the object. This is a helper called from destructors only. */
    void implDispose();

    /** Converts the control-relative position to an absolute screen position. */
    Point implGetAbsPos( const Point& rPos ) const;
};

class ScCsvRuler;

typedef ::cppu::ImplHelper1< css::accessibility::XAccessibleText > ScAccessibleCsvRulerImpl;

/** Accessible class representing the CSV ruler control. */
class ScAccessibleCsvRuler : public ScAccessibleCsvControl, public ScAccessibleCsvRulerImpl
{
private:
    OUStringBuffer       maBuffer;   /// Contains the text representation of the ruler.

public:
    explicit                    ScAccessibleCsvRuler( ScCsvRuler& rRuler );
    virtual                     ~ScAccessibleCsvRuler() override;

    // XAccessibleComponent -----------------------------------------------------

    virtual sal_Int32 SAL_CALL getForeground(  ) override;

    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleContext -----------------------------------------------------

    /** Returns the child count (the ruler does not have children). */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override;

    /** Throws an exception (the ruler does not have children). */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 nIndex ) override;

    /** Returns the relation to the grid control. */
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet() override;

    /** Returns the current set of states. */
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet >  SAL_CALL getAccessibleStateSet() override;

    // XAccessibleText --------------------------------------------------------

    /** Return the position of the caret. */
    virtual sal_Int32 SAL_CALL getCaretPosition() override;

    /** Sets the position of the caret. */
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) override;

    /** Returns the specified character. */
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) override;

    /** Returns the attributes of the specified character. */
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) override;

    /** Returns the screen coordinates of the specified character. */
    virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) override;

    /** Returns the count of characters. */
    virtual sal_Int32 SAL_CALL getCharacterCount() override;

    /** Returns the character index at the specified coordinate (object's coordinate system). */
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& rPoint ) override;

    /** Returns the selected text (ruler returns empty string). */
    virtual OUString SAL_CALL getSelectedText() override;

    /** Returns the start index of the selection (ruler returns -1). */
    virtual sal_Int32 SAL_CALL getSelectionStart() override;

    /** Returns the end index of the selection (ruler returns -1). */
    virtual sal_Int32 SAL_CALL getSelectionEnd() override;

    /** Selects a part of the text (ruler does nothing). */
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;

    /** Returns the entire text. */
    virtual OUString SAL_CALL getText() override;

    /** Returns the specified range [Start,End) of the text. */
    virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;

    /** Returns the specified text portion. */
    virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;

    /** Copies the specified text range into the clipboard (ruler does nothing). */
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;

    // XInterface -------------------------------------------------------------

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType ) override;

    virtual void SAL_CALL acquire() throw() override;

    virtual void SAL_CALL release() throw() override;

    // XServiceInfo -----------------------------------------------------------

    /** Returns an identifier for the implementation of this object. */
    virtual OUString SAL_CALL getImplementationName() override;

    // XTypeProvider ----------------------------------------------------------

    /** Returns a sequence with all supported interface types. */
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    /** Returns an implementation ID. */
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // events -----------------------------------------------------------------
public:
    /** Sends a caret changed event to all listeners. */
    virtual void SendCaretEvent() override;

    // helpers ----------------------------------------------------------------
private:
    /** Returns this object's name. */
    virtual OUString createAccessibleName() override;
    /** Returns this object's description. */
    virtual OUString createAccessibleDescription() override;

    /** @throws css::lang::IndexOutOfBoundsException if the specified character position is invalid (outside 0..len-1). */
    void ensureValidIndex( sal_Int32 nIndex ) const;
    /** @throws css::lang::IndexOutOfBoundsException if the specified character position is invalid (outside 0..len). */
    void ensureValidIndexWithEnd( sal_Int32 nIndex ) const;
    /** @throws css::lang::IndexOutOfBoundsException if the specified character range [Start,End) is invalid.
        @descr  If Start>End, swaps Start and End before checking. */
    void ensureValidRange( sal_Int32& rnStartIndex, sal_Int32& rnEndIndex ) const;

    /** Returns the VCL ruler control. Assumes a living object. */
    ScCsvRuler& implGetRuler() const;

    /** Builds the entire string buffer.

        @throws css::uno::RuntimeException
    */
    void constructStringBuffer();
    /** Returns the character count of the text. */
    sal_Int32 implGetTextLength() const;

    /** Returns true, if the character at the specified index has a split. */
    bool implHasSplit( sal_Int32 nApiPos );

    /** Returns the first character index with equal formatting as at nApiPos. */
    sal_Int32 implGetFirstEqualFormatted( sal_Int32 nApiPos );
    /** Returns the last character index with equal formatting as at nApiPos. */
    sal_Int32 implGetLastEqualFormatted( sal_Int32 nApiPos );
};

class ScCsvGrid;

typedef ::cppu::ImplHelper2<
        css::accessibility::XAccessibleTable,
        css::accessibility::XAccessibleSelection >
    ScAccessibleCsvGridImpl;

/** Accessible class representing the CSV grid control. */
class ScAccessibleCsvGrid : public ScAccessibleCsvControl, public ScAccessibleCsvGridImpl
{
protected:
    typedef std::map< sal_Int32, rtl::Reference<ScAccessibleCsvControl> > XAccessibleSet;

private:
    XAccessibleSet maAccessibleChildren;

public:
    explicit                    ScAccessibleCsvGrid( ScCsvGrid& rGrid );
    virtual                     ~ScAccessibleCsvGrid() override;
    using ScAccessibleContextBase::disposing;
    virtual void SAL_CALL       disposing() override;

    // XAccessibleComponent ---------------------------------------------------

    /** Returns the cell at the specified point. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& rPoint ) override;

    virtual sal_Int32 SAL_CALL getForeground(  ) override;

    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleContext -----------------------------------------------------

    /** Returns the child count (count of cells in the table). */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override;

    /** Returns the specified child cell. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 nIndex ) override;

    /** Returns the relation to the ruler control. */
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet() override;

    /** Returns the current set of states. */
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet >  SAL_CALL getAccessibleStateSet() override;

    // XAccessibleTable -------------------------------------------------------

    /** Returns the number of rows in the table. */
    virtual sal_Int32 SAL_CALL getAccessibleRowCount() override;

    /** Returns the number of columns in the table. */
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount() override;

    /** Returns the description of the specified row in the table. */
    virtual OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow ) override;

    /** Returns the description text of the specified column in the table. */
    virtual OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn ) override;

    /** Returns the number of rows occupied at a specified row and column.
        @descr  Returns always 1 (Merged cells not supported). */
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;

    /** Returns the number of rows occupied at a specified row and column.
        @descr  Returns always 1 (Merged cells not supported). */
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;

    /** Returns the row headers as an AccessibleTable. */
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL getAccessibleRowHeaders() override;

    /** Returns the column headers as an AccessibleTable. */
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL getAccessibleColumnHeaders() override;

    /** Returns the selected rows as a sequence. */
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleRows() override;

    /** Returns the selected columns as a sequence. */
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleColumns() override;

    /** Returns true, if the specified row is selected. */
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow ) override;

    /** Returns true, if the specified column is selected. */
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn ) override;

    /** Returns the accessible cell object at the specified position. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) override;

    /** Returns the caption object of the table. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleCaption() override;

    /** Returns the summary description object of the table. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleSummary() override;

    /** Returns true, if the cell at a specified position is selected. */
    virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) override;

    /** Returns the child index of the cell at the specified position. */
    virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) override;

    /** Returns the row index of the specified child. */
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex ) override;

    /** Returns the column index of the specified child. */
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex ) override;

    // XAccessibleSelection ---------------------------------------------------

    /** Selects the specified child (selects the entire column or the entire table). */
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) override;

    /** Returns true, if the specified child is selected. */
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) override;

    /** Deselects all cells. */
    virtual void SAL_CALL clearAccessibleSelection() override;

    /** Selects all cells. */
    virtual void SAL_CALL selectAllAccessibleChildren() override;

    /** Returns the count of selected children. */
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount() override;

    /** Returns the child with the specified index in all selected children. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) override;

    /** Deselects the child with the specified index in all selected children. */
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) override;

    // XInterface -------------------------------------------------------------

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType ) override;

    virtual void SAL_CALL acquire() throw() override;

    virtual void SAL_CALL release() throw() override;

    // XServiceInfo -----------------------------------------------------------

    /** Returns an identifier for the implementation of this object. */
    virtual OUString SAL_CALL getImplementationName() override;

    // XTypeProvider ----------------------------------------------------------

    /** Returns a sequence with all supported interface types. */
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    /** Returns an implementation ID. */
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // events -----------------------------------------------------------------
public:
    /** Sends a GetFocus or LoseFocus event to all listeners. */
    virtual void SendFocusEvent( bool bFocused ) override;
    /** Sends a table model changed event for changed cell contents to all listeners. */
    virtual void SendTableUpdateEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn, bool bAllRows ) override;
    /** Sends a table model changed event for an inserted column to all listeners. */
    virtual void SendInsertColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn ) override;
    /** Sends a table model changed event for a removed column to all listeners. */
    virtual void SendRemoveColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn ) override;

    // helpers ----------------------------------------------------------------
private:
    /** Returns this object's name. */
    virtual OUString createAccessibleName() override;
    /** Returns this object's description. */
    virtual OUString createAccessibleDescription() override;

    /** @throws css::lang::IndexOutOfBoundsException if nIndex is not a valid child index. */
    void ensureValidIndex( sal_Int32 nIndex ) const;
    /** @Throws css::lang::IndexOutOfBoundsException if the specified position is invalid. */
    void ensureValidPosition( sal_Int32 nRow, sal_Int32 nColumn ) const;

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
    sal_Int32 implGetCellCount() const { return implGetRowCount() * implGetColumnCount(); }

    /** Returns the row index from cell index (including header). */
    sal_Int32 implGetRow( sal_Int32 nIndex ) const { return nIndex / implGetColumnCount(); }
    /** Returns the column index from cell index (including header). */
    sal_Int32 implGetColumn( sal_Int32 nIndex ) const { return nIndex % implGetColumnCount(); }
    /** Returns the absolute column index of the nSelColumn-th selected column. */
    sal_Int32 implGetSelColumn( sal_Int32 nSelColumn ) const;
    /** Returns the child index from cell position (including header). */
    sal_Int32 implGetIndex( sal_Int32 nRow, sal_Int32 nColumn ) const { return nRow * implGetColumnCount() + nColumn; }

    /** Returns the contents of the specified cell (including header). Indexes must be valid. */
    OUString implGetCellText( sal_Int32 nRow, sal_Int32 nColumn ) const;
    /** Creates a new accessible object of the specified cell. Indexes must be valid. */
    ScAccessibleCsvControl* implCreateCellObj( sal_Int32 nRow, sal_Int32 nColumn ) const;

    css::uno::Reference<css::accessibility::XAccessible> getAccessibleCell(sal_Int32 nRow, sal_Int32 nColumn);
};

/** Accessible class representing a cell of the CSV grid control. */
class ScAccessibleCsvCell : public ScAccessibleCsvControl, public accessibility::AccessibleStaticTextBase
{
protected:
    typedef ::std::unique_ptr< SvxEditSource >      SvxEditSourcePtr;

private:
    OUString                    maCellText; /// The text contents of this cell.
    sal_Int32 const             mnLine;     /// The grid line index (core index).
    sal_uInt32 const            mnColumn;   /// The grid column index (core index).
    sal_Int32 const             mnIndex;    /// The index of the cell in the table.

public:
    explicit                    ScAccessibleCsvCell(
                                    ScCsvGrid& rGrid,
                                    const OUString& rCellText,
                                    sal_Int32 nRow, sal_Int32 nColumn );
    virtual                     ~ScAccessibleCsvCell() override;

    using ScAccessibleCsvControl::disposing;
    virtual void SAL_CALL       disposing() override;

    // XAccessibleComponent ---------------------------------------------------

    /** Sets the focus to the column of this cell. */
    virtual void SAL_CALL grabFocus() override;

    virtual sal_Int32 SAL_CALL getForeground(  ) override;

    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleContext -----------------------------------------------------

    /** Returns the child count. */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override;

    /** Returns the specified child. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 nIndex ) override;

    /** Returns the index of this cell in the table. */
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() override;

    /** Returns the relation to the ruler control. */
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet() override;

    /** Returns the current set of states. */
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet >  SAL_CALL getAccessibleStateSet() override;

    // XInterface -------------------------------------------------------------

    DECLARE_XINTERFACE()

    // XTypeProvider ----------------------------------------------------------

    DECLARE_XTYPEPROVIDER()

    // XServiceInfo -----------------------------------------------------------

    /** Returns an identifier for the implementation of this object. */
    virtual OUString SAL_CALL getImplementationName() override;

    // helpers ----------------------------------------------------------------
protected:
    /** Returns this object's current bounding box relative to the desktop. */
    virtual tools::Rectangle GetBoundingBoxOnScreen() const override;
    /** Returns this object's current bounding box relative to the parent object. */
    virtual tools::Rectangle GetBoundingBox() const override;

private:
    /** Returns this object's name. */
    virtual OUString createAccessibleName() override;
    /** Returns this object's description. */
    virtual OUString createAccessibleDescription() override;

    /** Returns the VCL grid control. Assumes a living object. */
    ScCsvGrid& implGetGrid() const;
    /** Returns the pixel position of the cell (rel. to parent), regardless of visibility. */
    Point implGetRealPos() const;
    /** Returns the width of the character count */
    sal_uInt32 implCalcPixelWidth(sal_uInt32 nChars) const;
    /** Returns the pixel size of the cell, regardless of visibility. */
    Size implGetRealSize() const;
    /** Returns the bounding box of the cell relative in the table. */
    tools::Rectangle implGetBoundingBox() const;

    /** Creates the edit source the text helper needs. */
    ::std::unique_ptr< SvxEditSource > implCreateEditSource();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
