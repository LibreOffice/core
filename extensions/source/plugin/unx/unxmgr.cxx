/*************************************************************************
 *
 *  $RCSfile: unxmgr.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2003-05-28 12:39:14 $
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

using namespace rtl;
using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::plugin;

// Unix specific implementation
static bool CheckPlugin( const ByteString& rPath, list< PluginDescription* >& rDescriptions )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "Trying plugin %s ... ", rPath.GetBuffer() );
#endif

    xub_StrLen nPos = rPath.SearchBackward( '/' );
    if( nPos == STRING_NOTFOUND )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "no absolute path to plugin\n" );
#endif
        return false;
    }

    ByteString aBaseName = rPath.Copy( nPos+1 );
    if( aBaseName.Equals( "libnullplugin.so" ) )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "don't like %s\n", aBaseName.GetBuffer() );
#endif
        return false;
    }

    struct stat aStat;
    if( stat( rPath.GetBuffer(), &aStat ) || ! S_ISREG( aStat.st_mode ) )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "%s is not a regular file\n", rPath.GetBuffer() );
#endif
        return false;
    }


    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    ByteString aCommand( "pluginapp.bin \"" );
    aCommand.Append( rPath );
    aCommand.Append( '"' );

    FILE* pResult = popen( aCommand.GetBuffer(), "r" );
    int nDescriptions = 0;
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
            xub_StrLen nIndex = 0;
            while( nIndex != STRING_NOTFOUND )
            {
                ByteString aType = aMIME.GetToken( 0, ';', nIndex );

                PluginDescription* pNew = new PluginDescription;
                pNew->PluginName    = String( rPath, aEncoding );
                pNew->Mimetype  = String( aType.GetToken( 0, ':' ), aEncoding );
                ByteString aExt( "*." );
                aExt += aType.GetToken( 1, ':' ).EraseLeadingChars().EraseTrailingChars();
                pNew->Extension = String( aExt, aEncoding );
                pNew->Description= String( aType.GetToken( 2, ':' ), aEncoding );
                rDescriptions.push_back( pNew );
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "Mimetype: %s\nExtension: %s\n"
                         "Description: %s\n",
                         OUStringToOString( pNew->Mimetype, aEncoding ).getStr(),
                         OUStringToOString( pNew->Extension, aEncoding ).getStr(),
                         OUStringToOString( pNew->Description, aEncoding ).getStr()
                         );
#endif
            }
        }
#if OSL_DEBUG_LEVEL > 1
        else
            fprintf( stderr, "result of \"%s\" contains no mimtype:\n%s\n",
                     aCommand.GetBuffer(),
                     aMIME.GetBuffer() );
#endif
    }
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "command \"%s\" failed\n", aCommand.GetBuffer() );
#endif
    return nDescriptions > 0;
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
                    char* pBaseName = ((struct dirent*)aBuffer)->d_name;
                    if( pBaseName[0] != '.' ||
                        pBaseName[1] != '.' ||
                        pBaseName[2] != 0 )
                    {
                        ByteString aFileName( aPath );
                        aFileName += "/";
                        aFileName += pBaseName;
                        CheckPlugin( aFileName, aPlugins );
                    }
                }
                if( pDIR )
                    closedir( pDIR );
            }
        }

        // try ~/.mozilla/pluginreg.dat
        ByteString aMozPluginreg( pHome );
        aMozPluginreg.Append( "/.mozilla/pluginreg.dat" );
        FILE* fp = fopen( aMozPluginreg.GetBuffer(), "r" );
        if( fp )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "parsing %s\n", aMozPluginreg.GetBuffer() );
#endif
            char aLine[1024];
            while( fgets( aLine, sizeof( aLine ), fp ) )
            {
                int nLineLen = strlen( aLine );
                int nDotPos;
                for( nDotPos = nLineLen-1; nDotPos > 0 && aLine[nDotPos] != ':'; nDotPos-- )
                    ;
                if( aLine[0] == '/' && aLine[nDotPos] == ':' && aLine[nDotPos+1] == '$' )
                    CheckPlugin( ByteString( aLine, nDotPos ), aPlugins );
            }
            fclose( fp );
        }

        // create return value
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

