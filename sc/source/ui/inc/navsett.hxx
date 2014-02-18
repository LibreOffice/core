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

#ifndef SC_NAVSETT_HXX
#define SC_NAVSETT_HXX

#include <tools/solar.h>

#include <vector>

/** Contains settings of the navigator listbox. This includes the expand state
    of each listbox entry and the index of the selected entry and sub entry. */
class ScNavigatorSettings
{
private:
    ::std::vector< bool >           maExpandedVec;      /// Array of Booleans for expand state.
    sal_uInt16                      mnRootSelected;     /// Index of selected root entry.
    sal_uLong                       mnChildSelected;    /// Index of selected child entry.

public:
                                ScNavigatorSettings();

    inline void                 SetExpanded( sal_uInt16 nIndex, bool bExpand ) { maExpandedVec[ nIndex ] = bExpand; }
    inline bool                 IsExpanded( sal_uInt16 nIndex ) const { return maExpandedVec[ nIndex ]; }

    inline void                 SetRootSelected( sal_uInt16 nIndex ) { mnRootSelected = nIndex; }
    inline sal_uInt16               GetRootSelected() const { return mnRootSelected; }

    inline void                 SetChildSelected( sal_uLong nIndex ) { mnChildSelected = nIndex; }
    inline sal_uLong                GetChildSelected() const { return mnChildSelected; }
};


#endif // SC_NAVSETT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
