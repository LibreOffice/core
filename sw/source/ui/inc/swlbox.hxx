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
#ifndef _SWLBOX_HXX
#define _SWLBOX_HXX

#include <svl/svarray.hxx>

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#include "swdllapi.h"

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

    String  aName;
    USHORT  nId;

public:
    SwBoxEntry(const String& aName, USHORT nId=0);
    SwBoxEntry(const SwBoxEntry& rOrg);
    SwBoxEntry();

    const String& GetName() const { return aName;}
};

/*--------------------------------------------------------------------
     Beschreibung: fuer ComboBoxen
 --------------------------------------------------------------------*/

typedef USHORT SwComboBoxStyle;
namespace nsSwComboBoxStyle
{
    const SwComboBoxStyle CBS_UPPER         = 0x01;
    const SwComboBoxStyle CBS_LOWER         = 0x02;
    const SwComboBoxStyle CBS_ALL           = 0x04;
    const SwComboBoxStyle CBS_FILENAME      = 0x08;
    const SwComboBoxStyle CBS_SW_FILENAME   = CBS_FILENAME;
}

class SW_DLLPUBLIC SwComboBox : public ComboBox
{
    SwEntryLst              aEntryLst;
    SwEntryLst              aDelEntryLst;
    SwBoxEntry              aDefault;
    USHORT                  nStyle;

    SW_DLLPRIVATE void                  InitComboBox();
    SW_DLLPRIVATE void                  InsertSorted(SwBoxEntry* pEntry);

    using ComboBox::InsertEntry;
    using ComboBox::RemoveEntry;
    using Window::SetStyle;

public:

    using ComboBox::GetEntryPos;

    SwComboBox(Window* pParent, const ResId& rId,
               USHORT nStyleBits = nsSwComboBoxStyle::CBS_ALL);
    ~SwComboBox();

    virtual void            KeyInput( const KeyEvent& rKEvt );

    void                    InsertEntry(const SwBoxEntry&);
    USHORT                  InsertEntry( const XubString& rStr, USHORT = 0)
                            {        InsertEntry( SwBoxEntry( rStr ) ); return 0;    }

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
