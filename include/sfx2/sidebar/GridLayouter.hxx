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
#ifndef SFX_SIDEBAR_GRID_LAYOUTER_HXX
#define SFX_SIDEBAR_GRID_LAYOUTER_HXX

#include "sfx2/dllapi.h"
#include <boost/scoped_ptr.hpp>

class Rectangle;
class Window;

namespace sfx2 { namespace sidebar {

class CellDescriptor;
class ColumnDescriptor;

/** A simple layouter that organizes controls in a grid.
    At the moment only horizontal positions and sizes are processed.
    It can handle all or only a subset of the controls in one panel.
*/
class SFX2_DLLPUBLIC GridLayouter
{
public:
    GridLayouter (Window& rParent);
    ~GridLayouter (void);

    /** Return the cell descriptor for the specified cell.
        This creates empty column data structures as needed.

        By default a cell has only one cell descriptor.  Different
        variants allow different cell descriptors for different
        controls.  This is useful if different controls are displayed
        for different contexts, and, say, one has a fixed width and
        another is to fill the column.

        During layouting only cell descriptors are processed that have
        visible controls.
    */
    CellDescriptor& GetCell (
        const sal_Int32 nRow,
        const sal_Int32 nColumn,
        const sal_Int32 nVariant = 0);

    ColumnDescriptor& GetColumn (
        const sal_Int32 nColumn);

    /** Calculate positions and sizes for all visible controls under
        the control of the grid layouter according to the current size
        of the parent window.
    */
    void Layout (void);

    /** Paint some debug information.
    */
    void Paint (const Rectangle& rBox);

private:
    class Implementation;
    ::boost::scoped_ptr<Implementation> mpImplementation;
};



/** A collection of attributes for a single cell in a grid layout.
    Represents one control.
*/
class SFX2_DLLPUBLIC CellDescriptor
{
public:
    CellDescriptor (void);
    ~CellDescriptor (void);

    /** Set the number of columns covered by the cell.  The default
        value is 1.
    */
    CellDescriptor& SetGridWidth (const sal_Int32 nColumnCount);

    /** Set the control represented by the cell and whose position and
        size will be modified in subsequent calls to
        GridLayouter::Layout().
        The cell is only taken into account in Layout() when the
        control is visible.
    */
    CellDescriptor& SetControl (Window& rWindow);

    /** Set the minimum and maximum width of the cell to the given
        value.
    */
    CellDescriptor& SetFixedWidth (const sal_Int32 nWidth);

    /** Set the minimum and maximum width of the cell to the current
        width of the control.
    */
    CellDescriptor& SetFixedWidth (void);
    CellDescriptor& SetMinimumWidth (const sal_Int32 nWidth);

    /** Set the horizontal offset of the control with respect to the
        containing column.  The offset is only used when the position
        of the control is calculated not when the sizes of columns are
        calculated.
    */
    CellDescriptor& SetOffset (const sal_Int32 nOffset);

    sal_Int32 GetGridWidth (void) const;
    Window* GetControl (void) const;
    sal_Int32 GetMinimumWidth (void) const;
    sal_Int32 GetMaximumWidth (void) const;
    sal_Int32 GetOffset (void) const;

private:
    Window* mpControl;
    sal_Int32 mnGridWidth;
    sal_Int32 mnMinimumWidth;
    sal_Int32 mnMaximumWidth;
    sal_Int32 mnOffset;
};



/** A collection of attributes for a single column in a grid layout.
*/
class SFX2_DLLPUBLIC ColumnDescriptor
{
public:
    ColumnDescriptor (void);
    ~ColumnDescriptor (void);

    ColumnDescriptor& SetWeight (
        const sal_Int32 nWeight);
    ColumnDescriptor& SetMinimumWidth (
        const sal_Int32 nWidth);
    /** Set both minimum and maximum width to the given value.
    */
    ColumnDescriptor& SetFixedWidth (
        const sal_Int32 nWidth);

    /** Set external padding on the left side of the column.
    */
    ColumnDescriptor& SetLeftPadding (
        const sal_Int32 nPadding);

    /** Set external padding on the right side of the column.
    */
    ColumnDescriptor& SetRightPadding (
        const sal_Int32 nPadding);

    sal_Int32 GetWeight (void) const;

    /** Return the minimum width of the column without external
        padding.  This is the value last set with SetMinimumWidth() or SetFixedWidth().
    */
    sal_Int32 GetMinimumWidth (void) const;

    /** Return the maximum width of the column without external
        padding.  This is the value last set with SetFixedWidth().
    */
    sal_Int32 GetMaximumWidth (void) const;

    /** Return the maximum width of the column including external
        padding.
    */
    sal_Int32 GetTotalMaximumWidth (void) const;

    sal_Int32 GetLeftPadding (void) const;
    sal_Int32 GetRightPadding (void) const;

    /** The width of the column is a temporary and internal value that
        is calculated in GridLayouter::Layout().
        Calling this method outside of Layout() does not have any effect.
    */
    void SetWidth (const sal_Int32 nWidth);
    sal_Int32 GetWidth (void) const;

private:
    sal_Int32 mnWeight;
    sal_Int32 mnMinimumWidth;
    sal_Int32 mnMaximumWidth;
    sal_Int32 mnLeftPadding;
    sal_Int32 mnRightPadding;

    // Temporary values set calculated in the Layout() method.
    sal_Int32 mnWidth;
};


} } // end of namespace sfx2::sidebar

#endif
