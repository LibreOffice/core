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

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#include <vector>
namespace binfilter {

/** Contains settings of the navigator listbox. This includes the expand state
    of each listbox entry and the index of the selected entry and sub entry. */
class ScNavigatorSettings
{
private:
    ::std::vector< BOOL >       maExpandedVec;      /// Array of Booleans for expand state.
    USHORT                      mnRootSelected;     /// Index of selected root entry.
    ULONG                       mnChildSelected;    /// Index of selected child entry.

public:
                                ScNavigatorSettings();

    inline void                 SetExpanded( USHORT nIndex, BOOL bExpand ) { maExpandedVec[ nIndex ] = bExpand; }
    inline BOOL                 IsExpanded( USHORT nIndex ) const { return maExpandedVec[ nIndex ]; }

    inline void                 SetRootSelected( USHORT nIndex ) { mnRootSelected = nIndex; }
    inline USHORT               GetRootSelected() const { return mnRootSelected; }

    inline void                 SetChildSelected( ULONG nIndex ) { mnChildSelected = nIndex; }
    inline ULONG                GetChildSelected() const { return mnChildSelected; }
};


} //namespace binfilter
#endif // SC_NAVSETT_HXX

