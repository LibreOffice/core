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

#ifndef SVX_FONTLB_HXX
#define SVX_FONTLB_HXX

#include <svtools/svtabbx.hxx>
#include <svtools/svlbitm.hxx>
#include <vcl/virdev.hxx>
#include "svx/svxdllapi.h"

// ============================================================================

/** A list box string item which stores its text and font. */
class SvLBoxFontString : public SvLBoxString
{
private:
    Font                        maFont;     /// The font used by this item.
    bool                        mbUseColor; /// true = use font color, false = default listbox color.

public:
                                SvLBoxFontString();
                                SvLBoxFontString(
                                    SvTreeListEntry* pEntry,
                                    sal_uInt16 nFlags,
                                    const XubString& rString,
                                    const Font& rFont,
                                    const Color* pColor = NULL );

    virtual                     ~SvLBoxFontString();

    /** Creates a new empty list box item. */
    virtual SvLBoxItem*         Create() const;

    void            InitViewData( SvTreeListBox*,SvTreeListEntry*,SvViewDataItem* );

    /** Paints this entry to the specified position, using the own font settings. */
    void                        Paint(
                                    const Point& rPos,
                                    SvTreeListBox& rDev,
                                    sal_uInt16 nFlags,
                                    SvTreeListEntry* pEntry );
};


// ============================================================================

/** A list box supporting formatted string entries. */
class SVX_DLLPUBLIC SvxFontListBox : public SvTabListBox
{
private:
    Font                        maStdFont;      /// Used for entries without specific font.

    // The following members are used to store additional parameters for InitEntry().
    Font                        maEntryFont;    /// Current entry font used in InitEntry().
    const Color*                mpEntryColor;   /// Current entry color used in InitEntry().
    bool                        mbUseFont;      /// true = Use maEntryFont/mpEntryColor in InitEntry().

public:
                                SvxFontListBox( Window* pParent, const ResId& rResId );

    /** Inserts a list entry and sets the font used for this entry.
        @param pColor  The font color. NULL = use default listbox text color. */
    void                        InsertFontEntry(
                                    const String& rString, const Font& rFont, const Color* pColor = NULL );

    /** Selects/deselects an entry specified by its position in the list box. */
    void                        SelectEntryPos( sal_uInt16 nPos, bool bSelect = true );
    /** Removes a selection. */
    void                        SetNoSelection();

    /** Returns the position of the entry currently selected or LIST_APPEND. */
    sal_uLong                       GetSelectEntryPos() const;
    /** Returns the text of the selected entry or an empty string. */
    XubString                   GetSelectEntry() const;

protected:
    /** Initializes a new SvLBoxFontString entry.
        @descr  Uses current value of maEntryFont to set the entry font (if mbUseFont is true). */
    virtual void                InitEntry(
                                    SvTreeListEntry* pEntry,
                                    const XubString& rEntryText,
                                    const Image& rCollImg,
                                    const Image& rExpImg,
                                    SvLBoxButtonKind eButtonKind );
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
