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

    SW_DLLPRIVATE void InitComboBox();
    SW_DLLPRIVATE void InsertSorted(SwBoxEntry* pEntry);
    SW_DLLPRIVATE void Init();

    using Window::GetStyle;
    using Window::SetStyle;

public:

    SwComboBox(Window* pParent,
               sal_uInt16 nStyleBits = nsSwComboBoxStyle::CBS_ALL);
    SwComboBox(Window* pParent, const ResId& rId,
               sal_uInt16 nStyleBits = nsSwComboBoxStyle::CBS_ALL);
    ~SwComboBox();

    virtual void            KeyInput(const KeyEvent& rKEvt) SAL_OVERRIDE;

    void                    InsertSwEntry(const SwBoxEntry&);
    virtual sal_uInt16      InsertEntry(const OUString& rStr, sal_uInt16 = 0) SAL_OVERRIDE;

    virtual void            RemoveEntryAt(sal_uInt16 nPos) SAL_OVERRIDE;

    sal_uInt16              GetSwEntryPos(const SwBoxEntry& rEntry) const;
    const SwBoxEntry&       GetSwEntry(sal_uInt16) const;

    sal_uInt16                  GetRemovedCount() const;
    const SwBoxEntry&       GetRemovedEntry(sal_uInt16 nPos) const;

    // FIXME ??? what is this
    sal_uInt16                  GetSwStyle() const            { return nStyle;    }
    void                    SetSwStyle(const sal_uInt16 nSt)  { nStyle = nSt;     }

    OUString                GetText() const;
};

#endif /* _SWLBOX_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
