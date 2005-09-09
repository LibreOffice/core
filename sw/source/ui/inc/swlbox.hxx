/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swlbox.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:03:29 $
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

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

class SwBoxEntry;
class Window;

SV_DECL_PTRARR_DEL(SwEntryLst, SwBoxEntry*, 10, 10)

/*--------------------------------------------------------------------
     Beschreibung: SwBoxEntry
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwBoxEntry
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

class SW_DLLPUBLIC SwComboBox : public ComboBox
{
    SwEntryLst              aEntryLst;
    SwEntryLst              aDelEntryLst;
    SwBoxEntry              aDefault;
    USHORT                  nStyle;

    SW_DLLPRIVATE void                  InitComboBox();
    SW_DLLPRIVATE void                  InsertSorted(SwBoxEntry* pEntry);

public:
    SwComboBox(Window* pParent, const ResId& rId,
               USHORT nStyleBits = CBS_ALL);
    ~SwComboBox();

    virtual void            KeyInput( const KeyEvent& rKEvt );

    void                    InsertEntry(const SwBoxEntry&);

    void                    RemoveEntry(USHORT nPos);

    USHORT                  GetEntryPos(const SwBoxEntry& rEntry) const;
    const SwBoxEntry&       GetEntry(USHORT) const;

    USHORT                  GetRemovedCount() const;
    const SwBoxEntry&       GetRemovedEntry(USHORT nPos) const;

    USHORT                  GetStyle() const            { return nStyle;    }
    void                    SetStyle(const USHORT nSt)  { nStyle = nSt;     }

    String                  GetText() const;
};

#endif /* _SWLBOX_HXX */
