/*************************************************************************
 *
 *  $RCSfile: swlbox.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SWLBOX_HXX
#define _SWLBOX_HXX

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

class SwBoxEntry;
class Window;

SV_DECL_PTRARR_DEL(SwEntryLst, SwBoxEntry*, 10, 10)

/*--------------------------------------------------------------------
     Beschreibung: SwBoxEntry
 --------------------------------------------------------------------*/

class SwBoxEntry
{
    friend class SwComboBox;

    BOOL    bModified : 1;
    BOOL    bNew : 1;

public:
    SwBoxEntry(const String& aName, USHORT nId=0);
    SwBoxEntry(const SwBoxEntry& rOrg);
    SwBoxEntry();

    String  aName;
    USHORT  nId;
};

/*--------------------------------------------------------------------
     Beschreibung:  Listbox mit Entry-Verwaltung
 --------------------------------------------------------------------*/

/*class SwListBox : private ListBox
{
    SwEntryLst  aEntryLst;
    SwBoxEntry  aDefault;

    BOOL SeekEntry( const SwBoxEntry& rEntryLst, USHORT* pPos = 0 );
public:
    // die SwListBox ist default NICHT sortiert!
    // Eine Sortierung erfolgt nur ueber das interne Array. Das errechnet
    // die richtige Position in der Box!
    SwListBox( Window* pParent, const ResId& rId );
    ~SwListBox();

    void                    InsertEntry( const SwBoxEntry&,
                                        USHORT nPos = LISTBOX_APPEND );
    void                    InsertEntrySort( const SwBoxEntry& );
    void                    RemoveEntry( USHORT nPos );
    void                    Clear();

    const SwBoxEntry&       GetEntry( USHORT ) const;
    const SwBoxEntry&       GetSelectEntry() const;

    // ueberladene Methoden der ListBox:
    inline void SetSelectHdl( const Link& rLink );
    inline void SetDoubleClickHdl( const Link& rLink );
    inline void SetUpdateMode( BOOL bFlag );
    inline void SelectEntryPos( USHORT nPos, BOOL bSelect = TRUE );
    inline void SelectEntry( const String& rStr, BOOL bSelect = TRUE );
    inline USHORT GetSelectEntryPos( USHORT nSelIndex = 0 ) const;
    inline USHORT GetEntryPos( const String& rStr ) const;
    inline USHORT GetEntryCount() const;
    // fuer zugriffe aufs Control, aber nicht auf die ListBox!
    inline Control& GetControl()    { return *this; }
};

/*--------------------------------------------------------------------
     Beschreibung: fuer ComboBoxen
 --------------------------------------------------------------------*/

enum SwComboBoxStyle
{
    CBS_UPPER       = 0x01,
    CBS_LOWER       = 0x02,
    CBS_ALL         = 0x04,
    CBS_FILENAME    = 0x08,
#ifdef WIN
    CBS_SW_FILENAME = CBS_FILENAME | CBS_LOWER
#else
    CBS_SW_FILENAME = CBS_FILENAME
#endif
};

class SwComboBox : public ComboBox
{
    SwEntryLst              aEntryLst;
    SwEntryLst              aDelEntryLst;
    SwBoxEntry              aDefault;
    USHORT                  nStyle;

    void                    InitComboBox();
    void                    InsertSorted(SwBoxEntry* pEntry);
public:
    SwComboBox(Window* pParent, const ResId& rId,
               USHORT nStyleBits = CBS_ALL);
    ~SwComboBox();

    virtual void            KeyInput( const KeyEvent& rKEvt );

    void                    InsertNewEntry(const SwBoxEntry&);
    void                    InsertEntry(const SwBoxEntry&);

    void                    RemoveEntry(USHORT nPos);

    USHORT                  GetEntryPos(const SwBoxEntry& rEntry) const;
    const SwBoxEntry&       GetEntry(USHORT) const;

    USHORT                  GetRemovedCount() const;
    const SwBoxEntry&       GetRemovedEntry(USHORT nPos) const;

    USHORT                  GetNewCount() const;
    const SwBoxEntry&       GetNewEntry(USHORT nPos) const;

    USHORT                  GetStyle() const            { return nStyle;    }
    void                    SetStyle(const USHORT nSt)  { nStyle = nSt;     }

    String                  GetText() const;
};

/*--------------------------------------------------------------------
     Beschreibung: Inlines
 --------------------------------------------------------------------*/

/*inline void SwListBox::SetSelectHdl( const Link& rLink )
{
    ListBox::SetSelectHdl(rLink);
}
inline void SwListBox::SetDoubleClickHdl( const Link& rLink )
{
    ListBox::SetDoubleClickHdl( rLink );
}
inline void SwListBox::SetUpdateMode( BOOL bFlag )
{
    ListBox::SetUpdateMode(bFlag);
}
inline void SwListBox::SelectEntryPos( USHORT nPos, BOOL bSelect )
{
    ListBox::SelectEntryPos( nPos, bSelect );
}
inline void SwListBox::SelectEntry( const String& rStr, BOOL bSelect )
{
    ListBox::SelectEntry( rStr, bSelect );
}
inline USHORT SwListBox::GetSelectEntryPos( USHORT nSelIndex ) const
{
    return ListBox::GetSelectEntryPos( nSelIndex );
}
inline USHORT SwListBox::GetEntryPos( const String& rStr ) const
{
    return ListBox::GetEntryPos( rStr );
}
inline USHORT SwListBox::GetEntryCount() const
{
    return ListBox::GetEntryCount();
}
*/
#endif  // _SWLBOX_HXX

