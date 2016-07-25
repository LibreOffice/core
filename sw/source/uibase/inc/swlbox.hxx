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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SWLBOX_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SWLBOX_HXX

#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include "swdllapi.h"

#include <vector>

class SwBoxEntry;
namespace vcl { class Window; }

typedef std::vector<SwBoxEntry> SwEntryList;

class SW_DLLPUBLIC SwBoxEntry
{
    friend class SwComboBox;

    OUString    aName;

public:
    SwBoxEntry(const OUString& aName);
    SwBoxEntry(const SwBoxEntry& rOrg);
    SwBoxEntry();

    const OUString& GetName() const { return aName;}
};

// for combo boxes
class SW_DLLPUBLIC SwComboBox : public ComboBox
{
    SwEntryList             m_EntryList;
    SwEntryList             m_DelEntryList;
    SwBoxEntry              aDefault;

    SAL_DLLPRIVATE void InsertSorted(SwBoxEntry const& rEntry);
    SAL_DLLPRIVATE void Init();

public:

    SwComboBox(vcl::Window* pParent, WinBits nStyle);
    virtual ~SwComboBox();

    void                    InsertSwEntry(const SwBoxEntry&);
    virtual sal_Int32       InsertEntry(const OUString& rStr, sal_Int32 = COMBOBOX_APPEND) override;

    virtual void            RemoveEntryAt(sal_Int32 nPos) override;

    const SwBoxEntry&       GetSwEntry(sal_Int32) const;
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_SWLBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
