/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: autonamecache.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 10:47:14 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <unotools/transliterationwrapper.hxx>

#include "autonamecache.hxx"
#include "dociter.hxx"
#include "cell.hxx"

// -----------------------------------------------------------------------

ScAutoNameCache::ScAutoNameCache( ScDocument* pD ) :
    pDoc( pD ),
    nCurrentTab( 0 )    // doesn't matter - aNames is empty
{
}

ScAutoNameCache::~ScAutoNameCache()
{
}

const ScAutoNameAddresses& ScAutoNameCache::GetNameOccurences( const String& rName, SCTAB nTab )
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
                case CELLTYPE_DESTROYED:
                    ;   // nothing, prevent compiler warning
                break;
            }
            if ( ScGlobal::pTransliteration->isEqual( aStr, rName ) )
            {
                rAddresses.push_back( ScAddress( aIter.GetCol(), aIter.GetRow(), aIter.GetTab() ) );
            }
        }
    }

    return rAddresses;
}

