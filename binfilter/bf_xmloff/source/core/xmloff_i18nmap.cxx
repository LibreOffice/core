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

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SVARRAY_HXX
#include <bf_svtools/svarray.hxx>
#endif

#ifndef _XMLOFF_I18NMAP_HXX
#include "i18nmap.hxx"
#endif
namespace binfilter {

using namespace rtl;

class SvI18NMapEntry_Impl
{
    USHORT			nKind;
    OUString		aName;
    OUString		aNewName;

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


}//end of namespace binfilter
