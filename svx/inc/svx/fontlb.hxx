/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SVX_FONTLB_HXX
#define SVX_FONTLB_HXX

#ifndef _SVTABBOX_HXX
#include <svtools/svtabbx.hxx>
#endif
#include <vcl/virdev.hxx>
#include "svx/svxdllapi.h"
#include <layout/layout.hxx>

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
                                    SvLBoxEntry* pEntry,
                                    sal_uInt16 nFlags,
                                    const XubString& rString,
                                    const Font& rFont,
                                    const Color* pColor = NULL );

    virtual                     ~SvLBoxFontString();

    /** Creates a new empty list box item. */
    virtual SvLBoxItem*         Create() const;

    void            InitViewData( SvLBox*,SvLBoxEntry*,SvViewDataItem* );

    /** Paints this entry to the specified position, using the own font settings. */
    void                        Paint(
                                    const Point& rPos,
                                    SvLBox& rDev,
                                    sal_uInt16 nFlags,
                                    SvLBoxEntry* pEntry );
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
                                    SvLBoxEntry* pEntry,
                                    const XubString& rEntryText,
                                    const Image& rCollImg,
                                    const Image& rExpImg,
                                    SvLBoxButtonKind eButtonKind );
};

#if ENABLE_LAYOUT

namespace layout
{
class SvxFontListBoxImpl;
class SVX_DLLPUBLIC SvxFontListBox : public ListBox
{
    /*DECL_GET_IMPL( SvxFontListBox );
    DECL_CONSTRUCTORS( SvxFontListBox, ListBox, WB_BORDER );
    DECL_GET_WINDOW (SvxFontListBox);*/

public:
    SvxFontListBox( Context*, const char* );
    ~SvxFontListBox ();
    sal_uInt16 InsertFontEntry (String const& entry, Font const& font, Color const* color=0);
};
};

#endif


// ============================================================================

#endif

