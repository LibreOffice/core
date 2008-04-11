/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: macmgr.cxx,v $
 * $Revision: 1.4 $
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

#include <cstdarg>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <osl/thread.h>
#include <rtl/strbuf.hxx>

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
        OStringBuffer aMIME;
        char buf[256];
        while( fgets( buf, sizeof( buf ), pResult ) )
        {
            for( size_t i = 0; i < sizeof(buf) && buf[i]; ++i )
            {
                if( buf[i] == '\n' )
                    buf[i] = ';';
            }
            aMIME.append( buf );
        }
        pclose( pResult );

        if( aMIME.getLength() > 0 )
        {
            OString aLine = aMIME.makeStringAndClear();

            sal_Int32 nIndex = 0;
            while( nIndex != -1 )
            {
                OString aType = aLine.getToken( 0, ';', nIndex );

                sal_Int32 nTypeIndex = 0;
                OString aMimetype   = aType.getToken( 0, ':', nTypeIndex );
                OString aExtLine    = aType.getToken( 0, ':', nTypeIndex );
                if( nTypeIndex < 0 ) // ensure at least three tokens
                    continue;
                OString aDesc       = aType.getToken( 0, ':', nTypeIndex );

                // create extension list string
                sal_Int32 nExtIndex = 0;
                OStringBuffer aExtension;
                while( nExtIndex != -1 )
                {
                    OString aExt = aExtLine.getToken( 0, ',', nExtIndex);
                    if( aExt.indexOf( "*." ) != 0 )
                        aExtension.append( "*." );
                    aExtension.append( aExt );
                    if( nExtIndex != -1 )
                        aExtension.append( ';' );
                }

                PluginDescription* pNew = new PluginDescription;
                // set plugin name (path to library)
                pNew->PluginName    = OStringToOUString( rPath, aEncoding );
                // set mimetype
                pNew->Mimetype  = OStringToOUString( aMimetype, aEncoding );
                // set extension line
                pNew->Extension = OStringToOUString( aExtension.makeStringAndClear(), aEncoding );
                // set description
                pNew->Description= OStringToOUString( aDesc, aEncoding );
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
            fprintf( stderr, "result of \"%s\" contains no mimtype\n",
                     aCommand.GetBuffer() );
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

