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

#include <set>
#include <map>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <vcl/stdtext.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/plugin/PluginDescription.hpp>
#include <com/sun/star/plugin/PluginManager.hpp>
#include <com/sun/star/plugin/XPluginManager.hpp>

using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::plugin;

struct ltstr
{
    bool operator()( const String& s1, const String& s2 ) const
    {
        return ( s1.CompareTo( s2 ) == COMPARE_LESS );
    }
};

typedef set< String, ltstr > StrSet;
typedef map< String, StrSet, ltstr > FilterMap;


//==================================================================================================
void fillNetscapePluginFilters( Sequence< rtl::OUString >& rPluginNames, Sequence< rtl::OUString >& rPluginTypes )
{
    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
    Reference< XPluginManager > xPMgr( PluginManager::create(xContext) );

    FilterMap aMap;

    // sum up the mimetypes: one description, multiple extensions

    Sequence<PluginDescription > aDescriptions( xPMgr->getPluginDescriptions() );
    const PluginDescription * pDescriptions = aDescriptions.getConstArray();
    for ( sal_uInt32 nPos = aDescriptions.getLength(); nPos--; )
    {
        const PluginDescription & rDescr = pDescriptions[nPos];

        StrSet& rTypes = aMap[ rDescr.Description ];
        String aExtension( rDescr.Extension );

        for ( sal_uInt16 nCnt = comphelper::string::getTokenCount(aExtension,  ';'); nCnt--; )
        {
            // no default plugins anymore
            String aExt( aExtension.GetToken( nCnt, ';' ) );
            if ( aExt.CompareToAscii( "*.*" ) != COMPARE_EQUAL )
                rTypes.insert( aExt );
        }
    }

    rPluginNames = Sequence< rtl::OUString >( aMap.size() );
    rPluginTypes = Sequence< rtl::OUString >( aMap.size() );
    rtl::OUString* pPluginNames = rPluginNames.getArray();
    rtl::OUString* pPluginTypes = rPluginTypes.getArray();
    int nIndex = 0;
    for ( FilterMap::iterator iPos = aMap.begin(); iPos != aMap.end(); ++iPos )
    {
        String aText( (*iPos).first );
        String aType;
        StrSet& rTypes = (*iPos).second;
        StrSet::iterator i = rTypes.begin();
        while ( i != rTypes.end() )
        {
            aType += (*i);
            ++i;
            if ( i != rTypes.end() )
                aType += ';';
        }

        if ( aType.Len() )
        {
            aText += String::CreateFromAscii( " (" );
            aText += aType;
            aText += ')';
            pPluginNames[nIndex] = aText;
            pPluginTypes[nIndex] = aType;
            nIndex++;
        }
    }
    rPluginNames.realloc( nIndex );
    rPluginTypes.realloc( nIndex );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
