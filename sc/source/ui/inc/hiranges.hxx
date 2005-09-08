/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hiranges.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:32:48 $
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

#ifndef SC_HIRANGES_HXX
#define SC_HIRANGES_HXX

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

//==================================================================

struct ScHighlightEntry
{
    ScRange     aRef;
    Color       aColor;

    ScHighlightEntry( const ScRange& rR, const Color& rC ) :
        aRef(rR), aColor(rC) {}
};

class ScHighlightRanges
{
    List        aEntries;

public:
            ScHighlightRanges();
            ~ScHighlightRanges();

    ULONG   Count() const                       { return aEntries.Count(); }
    void    Insert( ScHighlightEntry* pNew )    { aEntries.Insert(pNew, LIST_APPEND); }
    ScHighlightEntry* GetObject( ULONG nIndex ) const
                        { return (ScHighlightEntry*)aEntries.GetObject(nIndex); }
};



#endif


