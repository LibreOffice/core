/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: autonamecache.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 13:51:20 $
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

#ifndef SC_AUTONAMECACHE_HXX
#define SC_AUTONAMECACHE_HXX

#include <vector>
#include <hash_map>

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif
#include "global.hxx"

typedef ::std::vector< ScAddress > ScAutoNameAddresses;
typedef ::std::hash_map< String, ScAutoNameAddresses, ScStringHashCode, ::std::equal_to< String > > ScAutoNameHashMap;

//
//  Cache for faster lookup of automatic names during CompileXML
//  (during CompileXML, no document content is changed)
//

class ScAutoNameCache
{
    ScAutoNameHashMap   aNames;
    ScDocument*         pDoc;
    SCTAB               nCurrentTab;

public:
            ScAutoNameCache( ScDocument* pD );
            ~ScAutoNameCache();

    const ScAutoNameAddresses& GetNameOccurences( const String& rName, SCTAB nTab );
};

#endif

