/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoreflist.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:40:05 $
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

#ifndef SC_UNOREFLIST_HXX
#define SC_UNOREFLIST_HXX

#include <list>

#ifndef _SFXHINT_HXX
#include <svtools/hint.hxx>
#endif

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif


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

