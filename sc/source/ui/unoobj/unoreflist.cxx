/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#include "unoreflist.hxx"
#include "document.hxx"

//------------------------------------------------------------------------

ScUnoRefList::ScUnoRefList()
{
}

ScUnoRefList::~ScUnoRefList()
{
}

void ScUnoRefList::Add( sal_Int64 nId, const ScRangeList& rOldRanges )
{
    aEntries.push_back( ScUnoRefEntry( nId, rOldRanges ) );
}

void ScUnoRefList::Undo( ScDocument* pDoc )
{
    std::list<ScUnoRefEntry>::const_iterator aEnd( aEntries.end() );
    for ( std::list<ScUnoRefEntry>::const_iterator aIter( aEntries.begin() );
          aIter != aEnd; ++aIter )
    {
        ScUnoRefUndoHint aHint( *aIter );
        pDoc->BroadcastUno( aHint );
    }
}

//------------------------------------------------------------------------

TYPEINIT1(ScUnoRefUndoHint, SfxHint);

ScUnoRefUndoHint::ScUnoRefUndoHint( const ScUnoRefEntry& rRefEntry ) :
    aEntry( rRefEntry )
{
}

ScUnoRefUndoHint::~ScUnoRefUndoHint()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
