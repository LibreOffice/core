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
#ifndef _SFX_MAP_HXX
#define _SFX_MAP_HXX

#include <tools.hxx>
#include <sfxitems.hxx>
namespace binfilter {

class SfxInterface;
class SfxDispatcher;
class SfxShell;

class SfxMap: public SfxWhichMap
{
    const SfxInterface	*pInterface;
    const SfxDispatcher *pDispatcher;
    BOOL				bRecursive;
public:
    inline				SfxMap();
    inline              SfxMap(const SfxDispatcher &rDispatcher );
                        SfxMap(const SfxShell &, BOOL bRecursiveSearch = FALSE );
    inline				SfxMap( const SfxMap & rMap);

    virtual USHORT		GetWhich(USHORT nSlot) const;
    virtual USHORT		GetSlot(USHORT nWhich) const;
    virtual SfxWhichMap*Clone() const;
};

//--------------------------------------------------------------------

inline SfxMap::SfxMap():
        SfxWhichMap( FALSE ),
        pInterface( 0 ),
        pDispatcher( 0 ),
        bRecursive( FALSE )
{
}

inline SfxMap::SfxMap( const SfxMap & rMap):
        SfxWhichMap( TRUE ),
        pInterface( rMap.pInterface ),
        pDispatcher( rMap.pDispatcher ),
        bRecursive( rMap.bRecursive )
{
}

inline SfxMap::SfxMap(const SfxDispatcher &rDispatcher ):
        SfxWhichMap( TRUE ),
        pInterface( 0 ),
        pDispatcher( &rDispatcher ),
        bRecursive( 0 )
{
}

}//end of namespace binfilter
#endif

