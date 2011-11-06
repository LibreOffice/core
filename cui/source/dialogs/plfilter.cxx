/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

#include <set>
#include <map>
#include <unotools/processfactory.hxx>

#include <tools/debug.hxx>
#include <vcl/stdtext.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/plugin/PluginDescription.hpp>
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
    Reference< XMultiServiceFactory > xMan( ::utl::getProcessServiceFactory() );
    Reference< XPluginManager > xPMgr( xMan->createInstance(
        rtl::OUString::createFromAscii("com.sun.star.plugin.PluginManager") ), UNO_QUERY );

    if ( xPMgr.is() )
    {
        FilterMap aMap;

        // mimetypes zusammenfassen: eine description, mehrere extensions

        Sequence<PluginDescription > aDescriptions( xPMgr->getPluginDescriptions() );
        const PluginDescription * pDescriptions = aDescriptions.getConstArray();
        for ( sal_uInt32 nPos = aDescriptions.getLength(); nPos--; )
        {
            const PluginDescription & rDescr = pDescriptions[nPos];

            StrSet& rTypes = aMap[ rDescr.Description ];
            String aExtension( rDescr.Extension );

            for ( sal_uInt16 nCnt = aExtension.GetTokenCount( ';' ); nCnt--; )
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
    else
        ShowServiceNotAvailableError( NULL,
            String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.plugin.PluginManager" ) ), sal_True );
}

