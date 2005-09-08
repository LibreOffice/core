/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: navsett.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:40:04 $
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

#ifndef SC_NAVSETT_HXX
#define SC_NAVSETT_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#include <vector>

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


#endif // SC_NAVSETT_HXX

