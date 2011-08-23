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

#ifndef SC_OLINEWIN_HXX
#define SC_OLINEWIN_HXX

#include "viewdata.hxx"
namespace binfilter {

class ScOutlineArray;


// ============================================================================

enum ScOutlineMode { SC_OUTLINE_HOR, SC_OUTLINE_VER };


// ----------------------------------------------------------------------------

/** The window left of or above the spreadsheet containing the outline groups
    and controls to expand/collapse them. */
class ScOutlineWindow : public Window
{
private:
    ScViewData&                 mrViewData;         /// View data containing the document.
    ScSplitPos                  meWhich;            /// Which area in split window.
    bool                        mbHoriz;            /// true = Horizontal orientation.
    bool                        mbMirrorHdr;        /// true = Header at end of the window (bottom or right).

    ImageList*                  mpSymbols;          /// Symbols for buttons.
    Color                       maLineColor;        /// Line color for expanded groups.
    sal_Int32                   mnHeaderSize;       /// Size of the header area in entry direction.
    sal_Int32                   mnHeaderPos;        /// Position of the header area in entry direction.
    sal_Int32                   mnMainFirstPos;     /// First position of main area in entry direction.
    sal_Int32                   mnMainLastPos;      /// Last position of main area in entry direction.

    sal_uInt16                  mnMTLevel;          /// Mouse tracking: Level of active button.
    sal_uInt16                  mnMTEntry;          /// Mouse tracking: Entry index of active button.
    bool                        mbMTActive;         /// Mouse tracking active?
    bool                        mbMTPressed;        /// Mouse tracking: Button currently drawed pressed?

    Rectangle                   maFocusRect;        /// Focus rectangle on screen.
    sal_uInt16                  mnFocusLevel;       /// Level of focused button.
    sal_uInt16                  mnFocusEntry;       /// Entry index of focused button.
    bool                        mbDontDrawFocus;    /// Do not redraw focus in next Paint().

public:
                                ScOutlineWindow(
                                    Window* pParent,
                                    ScOutlineMode eMode,
                                    ScViewData* pViewData,
                                    ScSplitPos eWhich );
    virtual                     ~ScOutlineWindow();

    /** Sets the size of the header area (width/height dep. on window type). */
    void                        SetHeaderSize( sal_Int32 nNewSize );
    /** Returns the width/height the window needs to show all levels. */
    sal_Int32                   GetDepthSize() const;


private:
    /** Initializes color and image settings. */
    void                        InitSettings();

};


// ============================================================================

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
