/*************************************************************************
 *
 *  $RCSfile: unxmgr.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:18:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <cstdarg>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <osl/thread.h>

#include <vcl/svapp.hxx>
#include <plugin/impl.hxx>

using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::plugin;

// Unix specific implementation
static PluginDescription** CheckPlugin( const ByteString& rPath, int& rDescriptions )
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    PluginDescription** pRet = NULL;
    rDescriptions = 0;

    ByteString aCommand( "pluginapp.bin \"" );
    aCommand.Append( rPath );
    aCommand.Append( '"' );

    FILE* pResult = popen( aCommand.GetBuffer(), "r" );
    if( pResult )
    {
        ByteString aMIME;
        char buf[256];
        while( fgets( buf, sizeof( buf ), pResult ) )
            aMIME += buf;
        pclose( pResult );
        if( aMIME.Len() > 0 )
        {
            if( aMIME.GetChar( aMIME.Len()-1 ) == '\n' )
                aMIME.Erase( aMIME.Len()-1 );
            char cTok = ';';
            if( aMIME.GetTokenCount( ';' ) > 2 )
                cTok = ';';
            if( aMIME.GetTokenCount( ':' ) > 2 )
                cTok = ':';
            ByteString aExtension = aMIME.GetToken( 1, cTok );
            int nExtensions = aExtension.GetTokenCount( ',' );
            pRet = new PluginDescription*[ nExtensions ];
            for( int i = 0; i < nExtensions; i++ )
            {
                pRet[i] = new PluginDescription;
                pRet[i]->PluginName = String( rPath, aEncoding );
                pRet[i]->Mimetype   = String( aMIME.GetToken( 0, cTok ), aEncoding );
                ByteString aExt( "*." );
                aExt += aExtension.GetToken( i, ',' ).EraseLeadingChars().EraseTrailingChars();
                pRet[i]->Extension  = String( aExt, aEncoding );
                pRet[i]->Description= String( aMIME.GetToken( 2, cTok ), aEncoding );
            }
            rDescriptions = nExtensions;
        }
    }
    return pRet;
}

Sequence<PluginDescription> XPluginManager_Impl::getPluginDescriptions() throw()
{
    static Sequence<PluginDescription> aDescriptions;
    static BOOL bHavePlugins = FALSE;
    if( ! bHavePlugins )
    {
        rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
        list<PluginDescription*> aPlugins;
        int i;

        // unix: search for plugins in /usr/lib/netscape/plugins,
        //       ~/.netscape/plugins und NPX_PLUGIN_PATH
        // additionally: search in PluginsPath
        static const char* pHome = getenv( "HOME" );
        static const char* pNPXPluginPath = getenv( "NPX_PLUGIN_PATH" );

        ByteString aSearchPath( "/usr/lib/netscape/plugins" );
        if( pHome )
        {
            aSearchPath.Append( ':' );
            aSearchPath.Append( pHome );
            aSearchPath += "/.netscape/plugins";
        }
        if( pNPXPluginPath )
        {
            aSearchPath.Append( ':' );
            aSearchPath += pNPXPluginPath;
        }

        const Sequence< ::rtl::OUString >& rPaths( PluginManager::getAdditionalSearchPaths() );
        for( i = 0; i < rPaths.getLength(); i++ )
        {
            aSearchPath += ":";
            aSearchPath += ByteString( String( rPaths.getConstArray()[i] ), aEncoding );
        }


        long aBuffer[ sizeof( struct dirent ) + _PC_NAME_MAX +1 ];
        int nPaths = aSearchPath.GetTokenCount( ':' );
        for( i = 0; i < nPaths; i++ )
        {
            ByteString aPath( aSearchPath.GetToken( i, ':' ) );
            if( aPath.Len() )
            {
                DIR* pDIR = opendir( aPath.GetBuffer() );
                struct dirent* pDirEnt = NULL;
                while( pDIR && ! readdir_r( pDIR, (struct dirent*)aBuffer, &pDirEnt ) && pDirEnt )
                {
                    struct stat aStat;
                    ByteString aFileName( aPath );
                    aFileName += "/";
                    aFileName += ((struct dirent*)aBuffer)->d_name;
                    if( ! stat( aFileName.GetBuffer(), &aStat )
                        && S_ISREG( aStat.st_mode )
                        && strncmp( ((struct dirent*)aBuffer)->d_name, "libnullplugin", 13 )
                        // cannot use flash since it needs a java runtime
                        && strncmp( ((struct dirent*)aBuffer)->d_name, "libflashplayer.so", 17 )
                        )
                    {
#if OSL_DEBUG_LEVEL > 1
                        fprintf( stderr, "Trying plugin %s ... ", aFileName.GetBuffer() );
#endif
                        int nStructs;
                        PluginDescription** pStructs =
                            CheckPlugin( aFileName, nStructs );
                        if( pStructs )
                        {
#if OSL_DEBUG_LEVEL > 1
                            fprintf( stderr, "success: %d\n", nStructs );
#endif
                            for( int i = 0; i < nStructs; i++ )
                            {
                                aPlugins.push_back( pStructs[i] );
#if OSL_DEBUG_LEVEL > 1
                                fprintf( stderr, "Mimetype: %s\nExtension: %s\n"
                                         "Description: %s\n",
                                         ::rtl::OUStringToOString( pStructs[i]->Mimetype, aEncoding ).getStr(),
                                         ::rtl::OUStringToOString( pStructs[i]->Extension, aEncoding ).getStr(),
                                         ::rtl::OUStringToOString( pStructs[i]->Description, aEncoding ).getStr()
                                         );
#endif
                            }
                            delete pStructs;
                        }
#if OSL_DEBUG_LEVEL > 1
                        else
                            fprintf(stderr, "failed\n" );
#endif
                    }
                }
                if( pDIR )
                    closedir( pDIR );
            }
        }
        aDescriptions = Sequence<PluginDescription>( aPlugins.size() );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "found %d plugins\n", aPlugins.size() );
#endif
        list<PluginDescription*>::iterator iter;
        for( iter = aPlugins.begin(), i=0; iter != aPlugins.end(); ++iter ,i++ )
        {
            aDescriptions.getArray()[ i ] = **iter;
            delete *iter;
        }
        aPlugins.clear();
        bHavePlugins = TRUE;
    }
    return aDescriptions;
}

