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

#ifndef SC_RFINDLST_HXX
#define SC_RFINDLST_HXX

#ifndef _COLOR_HXX //autogen
#include <tools/color.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
namespace binfilter {

//==================================================================

struct ScRangeFindData
{
    ScRange		aRef;
    USHORT		nFlags;
    xub_StrLen	nSelStart;
    xub_StrLen	nSelEnd;

    ScRangeFindData( const ScRange& rR, USHORT nF, xub_StrLen nS, xub_StrLen nE ) :
        aRef(rR), nFlags(nF), nSelStart(nS), nSelEnd(nE) {}
};

class ScRangeFindList
{
    List		aEntries;
    String		aDocName;
    BOOL		bHidden;

public:
            ~ScRangeFindList();

    ULONG	Count() const						{ return aEntries.Count(); }
    ScRangeFindData* GetObject( ULONG nIndex ) const
                        { return (ScRangeFindData*)aEntries.GetObject(nIndex); }
    const String&	GetDocName() const			{ return aDocName; }
    BOOL			IsHidden() const			{ return bHidden; }
};



} //namespace binfilter
#endif


