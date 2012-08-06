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
#ifndef _SVX_BOX_HXX
#define _SVX_BOX_HXX

// include ---------------------------------------------------------------

#include <tools/string.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <svtools/svtdllapi.h>

// forward ---------------------------------------------------------------

class SvxBoxEntry;

typedef std::vector<SvxBoxEntry*> SvxEntryLst;

// class SvxBoxEntry -----------------------------------------------------

class SVT_DLLPUBLIC SvxBoxEntry
{
    friend class SvxListBox;
    friend class SvxComboBox;

public:
    SvxBoxEntry( const String& aName, sal_uInt16 nId=0 );
    SvxBoxEntry( const SvxBoxEntry& rOrg );
    SvxBoxEntry();

    String                  aName;
    sal_uInt16              nId;

private:
    sal_Bool                bModified;
    sal_Bool                bNew;
};

// class SvxListBox ------------------------------------------------------

class SVT_DLLPUBLIC SvxListBox : public ListBox
{
    using ListBox::InsertEntry;
    using ListBox::RemoveEntry;
    using ListBox::GetEntryPos;
    using ListBox::IsEntrySelected;
    using ListBox::SelectEntry;

public:
    SvxListBox( Window* pParent, WinBits nBits = WB_BORDER );
    SvxListBox( Window* pParent, const ResId& rId );
    ~SvxListBox();

    void                InsertNewEntry( const SvxBoxEntry& rEntry );
    void                InsertEntry( const SvxBoxEntry& rEntry,
                                     sal_uInt16 nId = LISTBOX_ENTRY_NOTFOUND );

    void                RemoveEntry( sal_uInt16 nPos );
    void                RemoveEntry( const SvxBoxEntry& rEntry );

    void                Clear();

    sal_uInt16              GetEntryPos( const SvxBoxEntry& rEntry ) const;
    const SvxBoxEntry&  GetSvxBoxEntry( sal_uInt16 nIdx ) const;

    inline sal_Bool         IsEntrySelected( const SvxBoxEntry& rEntry ) const;
    const SvxBoxEntry&  GetSelectSvxBoxEntry( sal_uInt16 nIdx = 0 ) const;
    inline void         SelectEntry( const SvxBoxEntry& rEntry,
                                     sal_Bool bSelect = sal_True );

    void                ModifyEntry( sal_uInt16 nPos, const String& aName );
    sal_uInt16              GetModifiedCount() const;
    const SvxBoxEntry&  GetModifiedEntry( sal_uInt16 nPos ) const;

    sal_uInt16              GetRemovedCount() const;
    const SvxBoxEntry&  GetRemovedEntry( sal_uInt16 nPos ) const;

    sal_uInt16              GetNewCount() const;
    const SvxBoxEntry&  GetNewEntry( sal_uInt16 nPos ) const;

private:
    SvxEntryLst         aEntryLst;
    SvxEntryLst         aDelEntryLst;
    SvxBoxEntry         aDefault;

    void                InitListBox();
    void                InsertSorted( SvxBoxEntry* pEntry );
};

// inlines ---------------------------------------------------------------

inline sal_Bool SvxListBox::IsEntrySelected( const SvxBoxEntry& aEntry ) const
    { return ListBox::IsEntrySelected( aEntry.aName ); }

inline void SvxListBox::SelectEntry( const SvxBoxEntry& aEntry, sal_Bool bSelect )
    { ListBox::SelectEntry( aEntry.aName, bSelect ); }

// enum SvxComboBoxStyle -------------------------------------------------

enum SvxComboBoxStyle
{
    SVX_CBS_UPPER       = 0x01,
    SVX_CBS_LOWER       = 0x02,
    SVX_CBS_ALL         = 0x04,
    SVX_CBS_FILENAME    = 0x08,
    SVX_CBS_SW_FILENAME = SVX_CBS_FILENAME
};

// class SvxComboBox -----------------------------------------------------

class SVT_DLLPUBLIC SvxComboBox : public ComboBox
{
    using ComboBox::InsertEntry;
    using ComboBox::RemoveEntry;
    using ComboBox::GetEntryPos;
    using Window::SetStyle;

public:
    SvxComboBox( Window* pParent,
                 WinBits nBits = WB_BORDER, sal_uInt16 nStyleBits = SVX_CBS_ALL );
    SvxComboBox( Window* pParent, const ResId& rId, sal_uInt16 nStyleBits = SVX_CBS_ALL );
    ~SvxComboBox();

    void                InsertNewEntry( const SvxBoxEntry& );
    void                InsertEntry( const SvxBoxEntry& );

    void                RemoveEntry( sal_uInt16 nPos );
    void                RemoveEntry( const SvxBoxEntry& rEntry );

    void                Clear();

    sal_uInt16              GetEntryPos( const SvxBoxEntry& rEntry ) const;
    const SvxBoxEntry&  GetEntry( sal_uInt16 nIdx ) const;

    void                ModifyEntry( sal_uInt16 nPos, const String& aName );
    sal_uInt16              GetModifiedCount() const;
    const SvxBoxEntry&  GetModifiedEntry( sal_uInt16 nPos ) const;

    sal_uInt16              GetRemovedCount() const;
    const SvxBoxEntry&  GetRemovedEntry( sal_uInt16 nPos ) const;

    sal_uInt16              GetNewCount() const;
    const SvxBoxEntry&  GetNewEntry( sal_uInt16 nPos ) const;

    sal_uInt16              GetStyle() const             { return nStyle; }
    void                SetStyle( const sal_uInt16 nSt ) { nStyle = nSt; }

    String              GetText() const;

private:
    SvxEntryLst         aEntryLst;
    SvxEntryLst         aDelEntryLst;
    SvxBoxEntry         aDefault;
    sal_uInt16              nStyle;

    virtual void        KeyInput( const KeyEvent& rKEvt );

    void                InitComboBox();
    void                InsertSorted( SvxBoxEntry* pEntry );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
