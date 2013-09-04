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
                                    const OUString& rString,
                                    const Font& rFont,
                                    const Color* pColor = NULL );

    virtual                     ~SvLBoxFontString();

    /** Creates a new empty list box item. */
    virtual SvLBoxItem*         Create() const;

    void            InitViewData( SvTreeListBox*,SvTreeListEntry*,SvViewDataItem* );

    /** Paints this entry to the specified position, using the own font settings. */
    virtual void Paint(
        const Point& rPos, SvTreeListBox& rOutDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry);
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
    SvxFontListBox(Window* pParent, WinBits nStyle = WB_BORDER);

    /** Inserts a list entry and sets the font used for this entry.
        @param pColor  The font color. NULL = use default listbox text color. */
    void                        InsertFontEntry(
                                    const OUString& rString, const Font& rFont, const Color* pColor = NULL );

    /** Selects/deselects an entry specified by its position in the list box. */
    void                        SelectEntryPos( sal_uInt16 nPos, bool bSelect = true );
    /** Removes a selection. */
    void                        SetNoSelection();

    /** Returns the position of the entry currently selected or LIST_APPEND. */
    sal_uLong                   GetSelectEntryPos() const;
    /** Returns the text of the selected entry or an empty string. */
    OUString                    GetSelectEntry() const;

protected:
    /** Initializes a new SvLBoxFontString entry.
        @descr  Uses current value of maEntryFont to set the entry font (if mbUseFont is true). */
    virtual void                InitEntry(
                                    SvTreeListEntry* pEntry,
                                    const OUString& rEntryText,
                                    const Image& rCollImg,
                                    const Image& rExpImg,
                                    SvLBoxButtonKind eButtonKind);
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
