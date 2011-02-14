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

#ifndef SC_NAVSETT_HXX
#define SC_NAVSETT_HXX

#include <tools/solar.h>

#include <vector>

/** Contains settings of the navigator listbox. This includes the expand state
    of each listbox entry and the index of the selected entry and sub entry. */
class ScNavigatorSettings
{
private:
    ::std::vector< sal_Bool >       maExpandedVec;      /// Array of Booleans for expand state.
    sal_uInt16                      mnRootSelected;     /// Index of selected root entry.
    sal_uLong                       mnChildSelected;    /// Index of selected child entry.

public:
                                ScNavigatorSettings();

    inline void                 SetExpanded( sal_uInt16 nIndex, sal_Bool bExpand ) { maExpandedVec[ nIndex ] = bExpand; }
    inline sal_Bool                 IsExpanded( sal_uInt16 nIndex ) const { return maExpandedVec[ nIndex ]; }

    inline void                 SetRootSelected( sal_uInt16 nIndex ) { mnRootSelected = nIndex; }
    inline sal_uInt16               GetRootSelected() const { return mnRootSelected; }

    inline void                 SetChildSelected( sal_uLong nIndex ) { mnChildSelected = nIndex; }
    inline sal_uLong                GetChildSelected() const { return mnChildSelected; }
};


#endif // SC_NAVSETT_HXX

