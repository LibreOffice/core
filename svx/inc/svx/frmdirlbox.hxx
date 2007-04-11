/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: frmdirlbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:55:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVX_FRMDIRLBOX_HXX
#define _SVX_FRMDIRLBOX_HXX

#ifndef _LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef SFX_ITEMCONNECT_HXX
#include <sfx2/itemconnect.hxx>
#endif
#ifndef _SVX_FRMDIR_HXX
#include <svx/frmdir.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SvxFrameDirectionItem;

namespace svx {

// ============================================================================

/** This listbox contains entries to select horizontal text direction.

    The control works on the SvxFrameDirection enumeration (i.e. left-to-right,
    right-to-left), used i.e. in conjunction with the SvxFrameDirectionItem.
 */
class SVX_DLLPUBLIC FrameDirectionListBox : public ListBox
{
public:
    explicit            FrameDirectionListBox( Window* pParent, WinBits nStyle = WB_BORDER );
    explicit            FrameDirectionListBox( Window* pParent, const ResId& rResId );
    virtual             ~FrameDirectionListBox();

    /** Inserts a string with corresponding direction enum into the listbox. */
    void                InsertEntryValue(
                            const String& rString,
                            SvxFrameDirection eDirection,
                            sal_uInt16 nPos = LISTBOX_APPEND );
    /** Removes the entry, that represents the specified frame direction. */
    void                RemoveEntryValue( SvxFrameDirection eDirection );

    /** Selects the specified frame direction. */
    void                SelectEntryValue( SvxFrameDirection eDirection );
    /** Returns the currently selected frame direction. */
    SvxFrameDirection   GetSelectEntryValue() const;

    /** Saves the currently selected frame direction. */
    inline void         SaveValue() { meSaveValue = GetSelectEntryValue(); }
    /** Returns the frame direction saved with SaveValue(). */
    inline SvxFrameDirection GetSavedValue() const { return meSaveValue; }

private:
    SvxFrameDirection   meSaveValue;    /// Saved value for later comparison.
};

typedef FrameDirectionListBox FrameDirListBox;

// ============================================================================

/** Wrapper for usage of a FrameDirectionListBox in item connections. */
class SVX_DLLPUBLIC FrameDirListBoxWrapper : public sfx::SingleControlWrapper< FrameDirListBox, SvxFrameDirection >
{
public:
    explicit            FrameDirListBoxWrapper( FrameDirListBox& rListBox );

    virtual bool        IsControlDontKnow() const;
    virtual void        SetControlDontKnow( bool bSet );

    virtual SvxFrameDirection GetControlValue() const;
    virtual void        SetControlValue( SvxFrameDirection eValue );
};

/** Wrapper for usage of a SvxFrameDirectionItem in item connections. */
typedef sfx::ValueItemWrapper< SvxFrameDirectionItem, SvxFrameDirection, sal_uInt16 > FrameDirItemWrapper;

/** An item<->control connection for a FrameDirectionListBox. */
typedef sfx::ItemControlConnection< FrameDirItemWrapper, FrameDirListBoxWrapper > FrameDirListBoxConnection;

// ============================================================================

} // namespace svx

#endif

