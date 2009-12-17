/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unoreflist.hxx,v $
 * $Revision: 1.3 $
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

#ifndef SC_UNOREFLIST_HXX
#define SC_UNOREFLIST_HXX

#include <list>
#include <svl/hint.hxx>
#include "rangelst.hxx"


struct ScUnoRefEntry
{
    sal_Int64   nObjectId;
    ScRangeList aRanges;

    ScUnoRefEntry( sal_Int64 nId, const ScRangeList& rOldRanges ) :
        nObjectId( nId ),
        aRanges( rOldRanges )
    {
    }
};

/** List of RefUpdate changes made to UNO objects during ScUpdateRefHint broadcast.
*/

class ScUnoRefList
{
private:
    ::std::list<ScUnoRefEntry> aEntries;

public:
                ScUnoRefList();
                ~ScUnoRefList();

    void        Add( sal_Int64 nId, const ScRangeList& rOldRanges );
    void        Undo( ScDocument* pDoc );

    bool        IsEmpty() const     { return aEntries.empty(); }
};

/** Hint to restore a UNO object to its old state (used during undo).
*/

class ScUnoRefUndoHint : public SfxHint
{
    ScUnoRefEntry   aEntry;

public:
                TYPEINFO();
                ScUnoRefUndoHint( const ScUnoRefEntry& rRefEntry );
                ~ScUnoRefUndoHint();

    sal_Int64   GetObjectId() const         { return aEntry.nObjectId; }
    const ScRangeList& GetRanges() const    { return aEntry.aRanges; }
};


#endif

