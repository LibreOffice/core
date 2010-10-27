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

#ifndef _SVX_FRMDIRLBOX_HXX
#define _SVX_FRMDIRLBOX_HXX

#include <vcl/lstbox.hxx>
#include <sfx2/itemconnect.hxx>
#include <editeng/frmdir.hxx>
#include "svx/svxdllapi.h"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
