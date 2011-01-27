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
#ifndef _SVX_CHECKLBX_HXX
#define _SVX_CHECKLBX_HXX

// include ---------------------------------------------------------------


#include <svtools/svtreebx.hxx>
#include <svtools/svlbitm.hxx>

#include <vcl/lstbox.hxx>
#include "svx/svxdllapi.h"

// class SvxCheckListBox -------------------------------------------------

class SVX_DLLPUBLIC SvxCheckListBox : public SvTreeListBox
{
    using Window::GetText;

private:
    SvLBoxButtonData*   pCheckButton;

#ifdef _SVX_CHECKLBX_CXX
    SVX_DLLPRIVATE void             Init_Impl();
#endif

    using SvTreeListBox::InsertEntry;
    // Avoid ambiguity with new InsertEntry:
    virtual SvLBoxEntry*    InsertEntry( const XubString& rText, SvLBoxEntry* pParent,
                                         BOOL bChildsOnDemand,
                                         ULONG nPos, void* pUserData,
                                         SvLBoxButtonKind eButtonKind );

public:
    SvxCheckListBox( Window* pParent, WinBits nWinStyle = 0 );
    SvxCheckListBox( Window* pParent, const ResId& rResId );
    SvxCheckListBox( Window* pParent, const ResId& rResId,
                     const Image& rNormalStaticImage );
    ~SvxCheckListBox();

    void            InsertEntry         ( const String& rStr,
                                          USHORT nPos = LISTBOX_APPEND,
                                          void* pUserData = NULL,
                                          SvLBoxButtonKind eButtonKind =
                                              SvLBoxButtonKind_enabledCheckbox );
    void            RemoveEntry         ( USHORT nPos );

    void            SelectEntryPos      ( USHORT nPos, BOOL bSelect = TRUE );
    USHORT          GetSelectEntryPos   () const;

    String          GetText             ( USHORT nPos ) const;
    USHORT          GetCheckedEntryCount() const;
    void            CheckEntryPos       ( USHORT nPos, BOOL bCheck = TRUE );
    BOOL            IsChecked           ( USHORT nPos ) const;
    void            ToggleCheckButton   ( SvLBoxEntry* pEntry );

    void*           SetEntryData        ( USHORT nPos, void* pNewData );
    void*           GetEntryData        ( USHORT nPos ) const;

    virtual void    MouseButtonDown     ( const MouseEvent& rMEvt );
    virtual void    KeyInput            ( const KeyEvent& rKEvt );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
