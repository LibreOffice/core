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

#include <svtools/treelistbox.hxx>
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
    virtual SvTreeListEntry*    InsertEntry( const XubString& rText, SvTreeListEntry* pParent,
                                         sal_Bool bChildrenOnDemand,
                                         sal_uIntPtr nPos, void* pUserData,
                                         SvLBoxButtonKind eButtonKind );

public:
    SvxCheckListBox( Window* pParent, WinBits nWinStyle = 0 );
    SvxCheckListBox( Window* pParent, const ResId& rResId );
    SvxCheckListBox( Window* pParent, const ResId& rResId,
                     const Image& rNormalStaticImage );
    ~SvxCheckListBox();

    void            InsertEntry         ( const String& rStr,
                                          sal_uInt16 nPos = LISTBOX_APPEND,
                                          void* pUserData = NULL,
                                          SvLBoxButtonKind eButtonKind =
                                              SvLBoxButtonKind_enabledCheckbox );
    void            RemoveEntry         ( sal_uInt16 nPos );

    void            SelectEntryPos      ( sal_uInt16 nPos, sal_Bool bSelect = sal_True );
    sal_uInt16          GetSelectEntryPos   () const;

    String          GetText             ( sal_uInt16 nPos ) const;
    sal_uInt16          GetCheckedEntryCount() const;
    void            CheckEntryPos       ( sal_uInt16 nPos, sal_Bool bCheck = sal_True );
    sal_Bool            IsChecked           ( sal_uInt16 nPos ) const;
    void            ToggleCheckButton   ( SvTreeListEntry* pEntry );

    void*           SetEntryData        ( sal_uInt16 nPos, void* pNewData );
    void*           GetEntryData        ( sal_uInt16 nPos ) const;

    virtual void    MouseButtonDown     ( const MouseEvent& rMEvt );
    virtual void    KeyInput            ( const KeyEvent& rKEvt );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
