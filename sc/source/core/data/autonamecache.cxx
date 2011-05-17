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



// INCLUDE ---------------------------------------------------------------

#include <unotools/transliterationwrapper.hxx>

#include "autonamecache.hxx"
#include "dociter.hxx"
#include "cell.hxx"
#include "queryparam.hxx"

// -----------------------------------------------------------------------

ScAutoNameCache::ScAutoNameCache( ScDocument* pD ) :
    pDoc( pD ),
    nCurrentTab( 0 )    // doesn't matter - aNames is empty
{
}

ScAutoNameCache::~ScAutoNameCache()
{
}

const ScAutoNameAddresses& ScAutoNameCache::GetNameOccurrences( const String& rName, SCTAB nTab )
{
    if ( nTab != nCurrentTab )
    {
        // the lists are valid only for one sheet, so they are cleared when another sheet is used
        aNames.clear();
        nCurrentTab = nTab;
    }

    ScAutoNameHashMap::const_iterator aFound = aNames.find( rName );
    if ( aFound != aNames.end() )
        return aFound->second;          // already initialized

    ScAutoNameAddresses& rAddresses = aNames[rName];

    ScCellIterator aIter( pDoc, ScRange( 0, 0, nCurrentTab, MAXCOL, MAXROW, nCurrentTab ) );
    for ( ScBaseCell* pCell = aIter.GetFirst(); pCell; pCell = aIter.GetNext() )
    {
        // don't check code length here, always use the stored result
        // (AutoCalc is disabled during CompileXML)

        if ( pCell->HasStringData() )
        {
            String aStr;
            CellType eType = pCell->GetCellType();
            switch ( eType )
            {
                case CELLTYPE_STRING:
                    ((ScStringCell*)pCell)->GetString( aStr );
                break;
                case CELLTYPE_FORMULA:
                    ((ScFormulaCell*)pCell)->GetString( aStr );
                break;
                case CELLTYPE_EDIT:
                    ((ScEditCell*)pCell)->GetString( aStr );
                break;
                case CELLTYPE_NONE:
                case CELLTYPE_VALUE:
                case CELLTYPE_NOTE:
                case CELLTYPE_SYMBOLS:
#ifdef DBG_UTIL
                case CELLTYPE_DESTROYED:
#endif
                    ;   // nothing, prevent compiler warning
                break;
            }
            if ( ScGlobal::GetpTransliteration()->isEqual( aStr, rName ) )
            {
                rAddresses.push_back( ScAddress( aIter.GetCol(), aIter.GetRow(), aIter.GetTab() ) );
            }
        }
    }

    return rAddresses;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
