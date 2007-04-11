/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svxbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:29:56 $
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
#ifndef _SVX_BOX_HXX
#define _SVX_BOX_HXX

// include ---------------------------------------------------------------

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

// forward ---------------------------------------------------------------

class SvxBoxEntry;
class SvxListBase;

//#if 0 // _SOLAR__PRIVATE
SV_DECL_PTRARR( SvxEntryLst, SvxBoxEntry*, 10, 10 )
//#else
//typedef SvPtrarr SvxEntryLst;
//#endif

// class SvxBoxEntry -----------------------------------------------------

class SvxBoxEntry
{
    friend class SvxListBox;
    friend class SvxComboBox;

public:
    SvxBoxEntry( const String& aName, USHORT nId=0 );
    SvxBoxEntry( const SvxBoxEntry& rOrg );
    SvxBoxEntry();

    String              aName;
    USHORT              nId;

private:
    BOOL                bModified;
    BOOL                bNew;
};

// class SvxListBox ------------------------------------------------------

class SvxListBox : public ListBox
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
                                     USHORT nId = LISTBOX_ENTRY_NOTFOUND );

    void                RemoveEntry( USHORT nPos );
    void                RemoveEntry( const SvxBoxEntry& rEntry );

    void                Clear();

    USHORT              GetEntryPos( const SvxBoxEntry& rEntry ) const;
    const SvxBoxEntry&  GetSvxBoxEntry( USHORT nIdx ) const;

    inline BOOL         IsEntrySelected( const SvxBoxEntry& rEntry ) const;
    const SvxBoxEntry&  GetSelectSvxBoxEntry( USHORT nIdx = 0 ) const;
    inline void         SelectEntry( const SvxBoxEntry& rEntry,
                                     BOOL bSelect = TRUE );

    void                ModifyEntry( USHORT nPos, const String& aName );
    USHORT              GetModifiedCount() const;
    const SvxBoxEntry&  GetModifiedEntry( USHORT nPos ) const;

    USHORT              GetRemovedCount() const;
    const SvxBoxEntry&  GetRemovedEntry( USHORT nPos ) const;

    USHORT              GetNewCount() const;
    const SvxBoxEntry&  GetNewEntry( USHORT nPos ) const;

private:
    SvxEntryLst         aEntryLst;
    SvxEntryLst         aDelEntryLst;
    SvxBoxEntry         aDefault;

    void                InitListBox();
    void                InsertSorted( SvxBoxEntry* pEntry );
};

// inlines ---------------------------------------------------------------

inline BOOL SvxListBox::IsEntrySelected( const SvxBoxEntry& aEntry ) const
    { return ListBox::IsEntrySelected( aEntry.aName ); }

inline void SvxListBox::SelectEntry( const SvxBoxEntry& aEntry, BOOL bSelect )
    { ListBox::SelectEntry( aEntry.aName, bSelect ); }

// enum SvxComboBoxStyle -------------------------------------------------

enum SvxComboBoxStyle
{
    SVX_CBS_UPPER       = 0x01,
    SVX_CBS_LOWER       = 0x02,
    SVX_CBS_ALL         = 0x04,
    SVX_CBS_FILENAME    = 0x08,
#ifdef WIN
    SVX_CBS_SW_FILENAME = SVX_CBS_FILENAME | SVX_CBS_LOWER
#else
    SVX_CBS_SW_FILENAME = SVX_CBS_FILENAME
#endif
};

// class SvxComboBox -----------------------------------------------------

class SvxComboBox : public ComboBox
{
    using ComboBox::InsertEntry;
    using ComboBox::RemoveEntry;
    using ComboBox::GetEntryPos;
    using Window::SetStyle;

public:
    SvxComboBox( Window* pParent,
                 WinBits nBits = WB_BORDER, USHORT nStyleBits = SVX_CBS_ALL );
    SvxComboBox( Window* pParent, const ResId& rId, USHORT nStyleBits = SVX_CBS_ALL );
    ~SvxComboBox();

    void                InsertNewEntry( const SvxBoxEntry& );
    void                InsertEntry( const SvxBoxEntry& );

    void                RemoveEntry( USHORT nPos );
    void                RemoveEntry( const SvxBoxEntry& rEntry );

    void                Clear();

    USHORT              GetEntryPos( const SvxBoxEntry& rEntry ) const;
    const SvxBoxEntry&  GetEntry( USHORT nIdx ) const;

    void                ModifyEntry( USHORT nPos, const String& aName );
    USHORT              GetModifiedCount() const;
    const SvxBoxEntry&  GetModifiedEntry( USHORT nPos ) const;

    USHORT              GetRemovedCount() const;
    const SvxBoxEntry&  GetRemovedEntry( USHORT nPos ) const;

    USHORT              GetNewCount() const;
    const SvxBoxEntry&  GetNewEntry( USHORT nPos ) const;

    USHORT              GetStyle() const             { return nStyle; }
    void                SetStyle( const USHORT nSt ) { nStyle = nSt; }

    String              GetText() const;

private:
    SvxEntryLst         aEntryLst;
    SvxEntryLst         aDelEntryLst;
    SvxBoxEntry         aDefault;
    USHORT              nStyle;

    virtual void        KeyInput( const KeyEvent& rKEvt );

    void                InitComboBox();
    void                InsertSorted( SvxBoxEntry* pEntry );
};

#endif

