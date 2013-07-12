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
#ifndef _SVX_BOX_HXX
#define _SVX_BOX_HXX

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

    inline bool         IsEntrySelected(const SvxBoxEntry& rEntry) const;
    {
        return ListBox::IsEntrySelected(aEntry.aName);
    }


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

    OUString            GetText() const;

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
