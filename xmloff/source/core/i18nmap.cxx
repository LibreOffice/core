/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i18nmap.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:20:58 $
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
#include "precompiled_xmloff.hxx"

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef _XMLOFF_I18NMAP_HXX
#include "i18nmap.hxx"
#endif

using namespace rtl;

class SvI18NMapEntry_Impl
{
    USHORT          nKind;
    OUString        aName;
    OUString        aNewName;

public:

    const OUString& GetNewName() const { return aNewName; }

    SvI18NMapEntry_Impl( USHORT nKnd, const OUString& rName,
                         const OUString& rNewName ) :
        nKind( nKnd ),
        aName( rName ),
        aNewName( rNewName )
    {}

    SvI18NMapEntry_Impl( USHORT nKnd, const OUString& rName ) :
        nKind( nKnd ),
        aName( rName )
    {}

    BOOL operator==( const SvI18NMapEntry_Impl& r ) const
    {
        return nKind == r.nKind &&
               aName == r.aName;
    }

    BOOL operator<( const SvI18NMapEntry_Impl& r ) const
    {
        return nKind < r.nKind ||
               ( nKind == r.nKind &&
                 aName < r.aName);
    }
};

typedef SvI18NMapEntry_Impl *SvI18NMapEntry_ImplPtr;
SV_DECL_PTRARR_SORT_DEL( SvI18NMap_Impl, SvI18NMapEntry_ImplPtr, 20, 5 )
SV_IMPL_OP_PTRARR_SORT( SvI18NMap_Impl, SvI18NMapEntry_ImplPtr )

// ---------------------------------------------------------------------

SvI18NMapEntry_Impl *SvI18NMap::_Find( USHORT nKind,
                                     const OUString& rName ) const
{
    SvI18NMapEntry_Impl *pRet = 0;
    SvI18NMapEntry_Impl aTst( nKind, rName );

    USHORT nPos;
    if( pImpl->Seek_Entry( &aTst, &nPos ) )
    {
        pRet = (*pImpl)[nPos];
    }

    return pRet;
}

SvI18NMap::SvI18NMap() :
    pImpl( 0 )
{
    pImpl = new SvI18NMap_Impl;
}

SvI18NMap::~SvI18NMap()
{
    delete pImpl;
}

void SvI18NMap::Add( USHORT nKind, const OUString& rName,
                     const OUString& rNewName )
{
    SvI18NMapEntry_Impl *pEntry = _Find( nKind, rName );
    DBG_ASSERT( !pEntry, "SvI18NMap::Add: item registered already" );
    if( !pEntry )
    {
        pEntry = new SvI18NMapEntry_Impl( nKind, rName, rNewName );
        pImpl->Insert( pEntry );
    }
}

const OUString& SvI18NMap::Get( USHORT nKind, const OUString& rName ) const
{
    SvI18NMapEntry_Impl *pEntry = _Find( nKind, rName );
    if( pEntry )
        return pEntry->GetNewName();
    else
        return rName;
}


