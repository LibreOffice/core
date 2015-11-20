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


#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#include <cstdarg>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <osl/thread.h>
#include <rtl/strbuf.hxx>
#include <tools/appendunixshellword.hxx>

#include <vcl/svapp.hxx>
#include <plugin/impl.hxx>

using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::plugin;


// Unix specific implementation
static bool CheckPlugin( const OString& rPath, list< PluginDescription* >& rDescriptions )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "Trying plugin %s ... ", rPath.getStr() );
#endif

    sal_Int32 nPos = rPath.lastIndexOf('/');
    if (nPos == -1)
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "no absolute path to plugin\n" );
#endif
        return false;
    }

    OString aBaseName = rPath.copy(nPos+1);
    if (aBaseName == "libnullplugin.so")
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "don't like %s\n", aBaseName.getStr() );
#endif
        return false;
    }

    struct stat aStat;
    if (stat(rPath.getStr(), &aStat) || !S_ISREG(aStat.st_mode))
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "%s is not a regular file\n", rPath.getStr() );
#endif
        return false;
    }

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    OString path;
    if (!UnxPluginComm::getPluginappPath(&path))
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "cannot construct path to pluginapp.bin\n" );
#endif
        return false;
    }
    OStringBuffer cmd;
    tools::appendUnixShellWord(&cmd, path);
    cmd.append(' ');
    tools::appendUnixShellWord(&cmd, rPath);
    OString aCommand(cmd.makeStringAndClear());

    FILE* pResult = popen( aCommand.getStr(), "r" );
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

        if( !aMIME.isEmpty() )
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
                     aCommand.getStr() );
#endif
    }
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "command \"%s\" failed\n", aCommand.getStr() );
#endif
    return nDescriptions > 0;
}

static void CheckPluginRegistryFiles( const OString& rPath, list< PluginDescription* >& rDescriptions )
{
    OStringBuffer aPath( 1024 );
    aPath.append( rPath );
    aPath.append( "/pluginreg.dat" );
    FILE* fp = fopen( aPath.getStr(), "r" );
    if( fp )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "parsing %s\n", aPath.getStr() );
#endif
        char aLine[1024];
        while( fgets( aLine, sizeof( aLine ), fp ) )
        {
            int nLineLen = strlen( aLine );
            int nDotPos;
            for( nDotPos = nLineLen-1; nDotPos > 0 && aLine[nDotPos] != ':'; nDotPos-- )
                ;
            if( aLine[0] == '/' && aLine[nDotPos] == ':' && aLine[nDotPos+1] == '$' )
                CheckPlugin( OString(aLine, nDotPos), rDescriptions );
        }
        fclose( fp );
    }

    // check subdirectories
    DIR* pDIR = opendir( rPath.getStr() );
    struct dirent* pDirEnt = nullptr;
    struct stat aStat;
    struct dirent u;
    while( pDIR && ! readdir_r( pDIR, &u.asDirent, &pDirEnt ) && pDirEnt )
    {
        char* pBaseName = u.asDirent.d_name;
        if( rtl_str_compare( ".", pBaseName ) && rtl_str_compare( "..", pBaseName ) )
        {
            OStringBuffer aBuf( 1024 );
            aBuf.append( rPath );
            aBuf.append( '/' );
            aBuf.append( pBaseName );

            if( ! stat( aBuf.getStr(), &aStat ) )
            {
                if( S_ISDIR( aStat.st_mode ) )
                    CheckPluginRegistryFiles( aBuf.makeStringAndClear(), rDescriptions );
            }
        }
    }
    if( pDIR )
        closedir( pDIR );
}

Sequence<PluginDescription> XPluginManager_Impl::impl_getPluginDescriptions() throw(css::uno::RuntimeException, std::exception)
{
    static Sequence<PluginDescription> aDescriptions;
    static bool bHavePlugins = false;
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

        // netscape!, quick, beam me back to the 90's when Motif roamed the earth
        OStringBuffer aSearchBuffer("/usr/lib/netscape/plugins");
        if( pHome )
            aSearchBuffer.append(':').append(pHome).append("/.netscape/plugins");
        if( pNPXPluginPath )
            aSearchBuffer.append(':').append(pNPXPluginPath);

        const Sequence< OUString >& rPaths( PluginManager::getAdditionalSearchPaths() );
        for( i = 0; i < rPaths.getLength(); i++ )
        {
            aSearchBuffer.append(':').append(OUStringToOString(
                rPaths.getConstArray()[i], aEncoding));
        }

        OString aSearchPath = aSearchBuffer.makeStringAndClear();

        sal_Int32 nIndex = 0;
        struct dirent u;
        do
        {
            OString aPath(aSearchPath.getToken(0, ':', nIndex));
            if (!aPath.isEmpty())
            {
                DIR* pDIR = opendir(aPath.getStr());
                struct dirent* pDirEnt = nullptr;
                while( pDIR && ! readdir_r( pDIR, &u.asDirent, &pDirEnt ) && pDirEnt )
                {
                    char* pBaseName = u.asDirent.d_name;
                    if( pBaseName[0] != '.' ||
                        pBaseName[1] != '.' ||
                        pBaseName[2] != 0 )
                    {
                        OStringBuffer aFileName(aPath);
                        aFileName.append('/').append(pBaseName);
                        CheckPlugin( aFileName.makeStringAndClear(), aPlugins );
                    }
                }
                if( pDIR )
                    closedir( pDIR );
            }
        }
        while ( nIndex >= 0 );

        // try ~/.mozilla/pluginreg.dat
        OStringBuffer aBuf(256);
        aBuf.append( pHome );
        aBuf.append( "/.mozilla" );
        CheckPluginRegistryFiles( aBuf.makeStringAndClear(), aPlugins );

        // create return value
        aDescriptions = Sequence<PluginDescription>( aPlugins.size() );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "found %" SAL_PRI_SIZET "u plugins\n", aPlugins.size() );
#endif
        list<PluginDescription*>::iterator iter;
        for( iter = aPlugins.begin(), i=0; iter != aPlugins.end(); ++iter ,i++ )
        {
            aDescriptions.getArray()[ i ] = **iter;
            delete *iter;
        }
        aPlugins.clear();
        bHavePlugins = true;
    }
    return aDescriptions;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
