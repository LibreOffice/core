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
#ifndef INCLUDED_SVX_CHECKLBX_HXX
#define INCLUDED_SVX_CHECKLBX_HXX

#include <svtools/treelistbox.hxx>
#include <svtools/svlbitm.hxx>

#include <svx/svxdllapi.h>

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
                                         bool bChildrenOnDemand,
                                         sal_uIntPtr nPos, void* pUserData,
                                         SvLBoxButtonKind eButtonKind ) override;

public:
    SvxCheckListBox( vcl::Window* pParent, WinBits nWinStyle = 0 );
    void SetNormalStaticImage(const Image& rNormalStaticImage);
    virtual ~SvxCheckListBox();
    virtual void dispose() override;

    void            InsertEntry         ( const OUString& rStr,
                                          sal_uLong  nPos = TREELIST_APPEND,
                                          void* pUserData = NULL,
                                          SvLBoxButtonKind eButtonKind =
                                              SvLBoxButtonKind_enabledCheckbox );
    void            RemoveEntry         ( sal_uLong  nPos );

    void            SelectEntryPos      ( sal_uLong  nPos, bool bSelect = true );
    sal_uLong       GetSelectEntryPos   () const;
    void*           GetSelectEntryData  () const { return GetEntryData(GetSelectEntryPos()); }

    OUString        GetText             ( sal_uLong  nPos ) const;
    sal_uLong       GetCheckedEntryCount() const;
    void            CheckEntryPos       ( sal_uLong  nPos, bool bCheck = true );
    bool            IsChecked           ( sal_uLong  nPos ) const;
    void            ToggleCheckButton   ( SvTreeListEntry* pEntry );

    void*           SetEntryData        ( sal_uLong  nPos, void* pNewData );
    void*           GetEntryData        ( sal_uLong  nPos ) const;

    virtual void    MouseButtonDown     ( const MouseEvent& rMEvt ) override;
    virtual void    KeyInput            ( const KeyEvent& rKEvt ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
