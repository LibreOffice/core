/*************************************************************************
 *
 *  $RCSfile: unxmgr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:51 $
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
#ifdef SOLARIS
#include <limits>
#endif

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#include <sys/stat.h>
#include <dlfcn.h>

#include <vcl/svapp.hxx>
#include <plugin/impl.hxx>

#include <dirent.h>

// Unix specific implementation
static ::com::sun::star::plugin::PluginDescription** CheckPlugin( const ByteString& rPath, int& rDescriptions )
{
    ::com::sun::star::plugin::PluginDescription** pRet = NULL;
    rDescriptions = 0;

    struct stat aStat;
    if( stat( rPath.GetBuffer(), &aStat ) )
        return NULL;
    void *pLib = dlopen( rPath.GetBuffer(), RTLD_LAZY );
    if( ! pLib )
        return NULL;

    char*(*pNP_GetMIMEDescription)() = (char*(*)())
        dlsym( pLib, "NP_GetMIMEDescription" );
    if( pNP_GetMIMEDescription )
    {
        ByteString aMIME = pNP_GetMIMEDescription();
        char cTok = ';';
        if( aMIME.GetTokenCount( ';' ) > 2 )
            cTok = ';';
        if( aMIME.GetTokenCount( ':' ) > 2 )
            cTok = ':';
        ByteString aExtension = aMIME.GetToken( 1, cTok );
        int nExtensions = aExtension.GetTokenCount( ',' );
        pRet = new ::com::sun::star::plugin::PluginDescription*[ nExtensions ];
        for( int i = 0; i < nExtensions; i++ )
        {
            pRet[i] = new ::com::sun::star::plugin::PluginDescription;
            pRet[i]->PluginName = String( rPath, gsl_getSystemTextEncoding() );
            pRet[i]->Mimetype   = String( aMIME.GetToken( 0, cTok ), gsl_getSystemTextEncoding() );
            String aExt( RTL_CONSTASCII_USTRINGPARAM(  "*." ) );
            aExt += String( aExtension.GetToken( i, ',' ).EraseLeadingChars().EraseTrailingChars(), gsl_getSystemTextEncoding() );
            pRet[i]->Extension  = aExt;
            pRet[i]->Description= String( aMIME.GetToken( 2, cTok ), gsl_getSystemTextEncoding() );
        }
        rDescriptions = nExtensions;
    }
    // some libraries register atexit handlers when loaded
    // (e.g. g++ made libraries register global destructors)
    // not closing them does prevent them to be called when already unloaded
//  dlclose( pLib );
    return pRet;
}

Sequence< ::com::sun::star::plugin::PluginDescription > XPluginManager_Impl::getPluginDescriptions()
{
    static Sequence< ::com::sun::star::plugin::PluginDescription > aDescriptions;
    static BOOL bHavePlugins = FALSE;
    if( ! bHavePlugins )
    {
        NAMESPACE_STD(list)< ::com::sun::star::plugin::PluginDescription* > aPlugins;
        int i;

        // unix: search for plugins in /usr/lib/netscape/plugins,
        //       ~/.netscape/plugins und NPX_PLUGIN_PATH
        // additionally: search in PluginsPath

        const char* pEnv = getenv( "HOME" );

        String aSearchPath( RTL_CONSTASCII_USTRINGPARAM( "/usr/lib/netscape/plugins:" ) );
        aSearchPath += String( pEnv ? pEnv : "", gsl_getSystemTextEncoding() );
        aSearchPath += String(  RTL_CONSTASCII_USTRINGPARAM( "/.netscape/plugins:" ) );
        pEnv = getenv( "NPX_PLUGIN_PATH" );
        aSearchPath += String( pEnv ? pEnv : "", gsl_getSystemTextEncoding() );

        const Sequence< ::rtl::OUString >& rPaths = PluginManager::getAdditionalSearchPaths();
        for( i = 0; i < rPaths.getLength(); i++ )
        {
            aSearchPath += ':';
            aSearchPath += String( rPaths.getConstArray()[i] );
        }


        int nPaths = aSearchPath.GetTokenCount( ':' );
        for( i = 0; i < nPaths; i++ )
        {
            ByteString aPath( aSearchPath.GetToken( i, ':' ), gsl_getSystemTextEncoding() );
            if( aPath.Len() )
            {
                DIR* pDIR = opendir( aPath.GetBuffer() );
                struct dirent aEntry;
                struct dirent* pEntry;

                while( pDIR && readdir_r( pDIR, &aEntry, &pEntry ) )
                {
                    struct stat aStat;
                    ByteString aFile( aPath );
                    aFile += '/';
                    aFile += aEntry.d_name;
                    if( ! stat( aFile.GetBuffer(), &aStat ) &&
                        S_ISREG( aStat.st_mode )            &&
                        ! strncmp( aEntry.d_name, "libnullplugin", 13 )
                        )
                    {
                        int nStructs;
                        ::com::sun::star::plugin::PluginDescription** pStructs =
                            CheckPlugin( aFile, nStructs );
                        if( pStructs )
                        {
                            for( int i = 0; i < nStructs; i++ )
                                aPlugins.push_back( pStructs[i] );
                            delete pStructs;
                        }
                    }
                }
            }
        }
        aDescriptions = Sequence< ::com::sun::star::plugin::PluginDescription >( aPlugins.size() );
        NAMESPACE_STD(list)< ::com::sun::star::plugin::PluginDescription* >::iterator iter;
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

