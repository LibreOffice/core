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

    SVX_DLLPRIVATE void             Init_Impl();

    using SvTreeListBox::InsertEntry;
    // Avoid ambiguity with new InsertEntry:
    virtual SvTreeListEntry*    InsertEntry( const OUString& rText, SvTreeListEntry* pParent,
                                         sal_Bool bChildrenOnDemand,
                                         sal_uIntPtr nPos, void* pUserData,
                                         SvLBoxButtonKind eButtonKind );

public:
    SvxCheckListBox( Window* pParent, WinBits nWinStyle = 0 );
    SvxCheckListBox( Window* pParent, const ResId& rResId );
    SvxCheckListBox( Window* pParent, const ResId& rResId,
                     const Image& rNormalStaticImage );
    ~SvxCheckListBox();

    void            InsertEntry         ( const OUString& rStr,
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
