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
    explicit            FrameDirectionListBox( Window* pParent, const ResId& rResId );
    explicit            FrameDirectionListBox( Window* pParent, WinBits nBits );
    virtual             ~FrameDirectionListBox();

    /** Inserts a string with corresponding direction enum into the listbox. */
    void                InsertEntryValue(
                            const OUString& rString,
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
