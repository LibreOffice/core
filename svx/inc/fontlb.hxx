/*************************************************************************
 *
 *  $RCSfile: fontlb.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dr $ $Date: 2002-07-23 10:47:34 $
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

#ifndef SVX_FONTLB_HXX
#define SVX_FONTLB_HXX

#ifndef _SVTABBOX_HXX
#include <svtools/svtabbx.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif


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
class SvxFontListBox : public SvTabListBox
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

    /** Returns the position of the entry currently selected or LIST_ENTRY_NOTFOUND. */
    sal_uInt32                  GetSelectEntryPos() const;
    /** Returns the text of the selected entry or an empty string. */
    XubString                   GetSelectEntry() const;

protected:
    /** Initializes a new SvLBoxFontString entry.
        @descr  Uses current value of maEntryFont to set the entry font (if mbUseFont is true). */
    virtual void                InitEntry(
                                    SvLBoxEntry* pEntry,
                                    const XubString& rEntryText,
                                    const Image& rCollImg,
                                    const Image& rExpImg );
};


// ============================================================================

#endif

