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
#ifndef _SWLBOX_HXX
#define _SWLBOX_HXX

#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include "swdllapi.h"
#include <boost/ptr_container/ptr_vector.hpp>

class SwBoxEntry;
class Window;

typedef boost::ptr_vector<SwBoxEntry> SwEntryLst;

/*--------------------------------------------------------------------
     Description: SwBoxEntry
 --------------------------------------------------------------------*/
class SW_DLLPUBLIC SwBoxEntry
{
    friend class SwComboBox;

    sal_Bool    bModified : 1;
    sal_Bool    bNew : 1;

    String  aName;
    sal_uInt16  nId;

public:
    SwBoxEntry(const String& aName, sal_uInt16 nId=0);
    SwBoxEntry(const SwBoxEntry& rOrg);
    SwBoxEntry();

    const String& GetName() const { return aName;}
};

/*--------------------------------------------------------------------
     Description: for combo boxes
 --------------------------------------------------------------------*/
typedef sal_uInt16 SwComboBoxStyle;

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
    sal_uInt16                  nStyle;

    SW_DLLPRIVATE void                  InitComboBox();
    SW_DLLPRIVATE void                  InsertSorted(SwBoxEntry* pEntry);

    using ComboBox::InsertEntry;
    using ComboBox::RemoveEntry;
    using Window::SetStyle;

public:

    using ComboBox::GetEntryPos;

    SwComboBox(Window* pParent, const ResId& rId,
               sal_uInt16 nStyleBits = nsSwComboBoxStyle::CBS_ALL);
    ~SwComboBox();

    virtual void            KeyInput( const KeyEvent& rKEvt );

    void                    InsertEntry(const SwBoxEntry&);
    sal_uInt16                  InsertEntry( const XubString& rStr, sal_uInt16 = 0)
                            {        InsertEntry( SwBoxEntry( rStr ) ); return 0;    }

    void                    RemoveEntry(sal_uInt16 nPos);

    sal_uInt16                  GetEntryPos(const SwBoxEntry& rEntry) const;
    const SwBoxEntry&       GetEntry(sal_uInt16) const;

    sal_uInt16                  GetRemovedCount() const;
    const SwBoxEntry&       GetRemovedEntry(sal_uInt16 nPos) const;

    sal_uInt16                  GetStyle() const            { return nStyle;    }
    void                    SetStyle(const sal_uInt16 nSt)  { nStyle = nSt;     }

    String                  GetText() const;
};

#endif /* _SWLBOX_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
