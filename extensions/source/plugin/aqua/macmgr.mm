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

#include "rtl/ustrbuf.hxx"
#include "rtl/strbuf.hxx"

#include "plugin/impl.hxx"
#include "osl/file.h"
#include "osl/module.hxx"

using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::plugin;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringBuffer;
using ::rtl::OStringBuffer;
using ::rtl::OUStringToOString;
using ::rtl::OStringToOUString;

namespace plugstringhelper
{

rtl::OUString getString( CFStringRef i_xString )
{
    rtl::OUStringBuffer aBuf;
    if( i_xString )
    {
        CFIndex nChars = CFStringGetLength( i_xString );
        CFRange aRange = { 0, nChars };
        aBuf.setLength( nChars );
        CFStringGetCharacters( i_xString, aRange, static_cast< UniChar* >(const_cast<sal_Unicode*>(aBuf.getStr())) );
    }
    return aBuf.makeStringAndClear();
}

rtl::OUString getString( CFURLRef i_xURL )
{
    CFStringRef xString = CFURLGetString( i_xURL );
    return getString( xString );
}

CFMutableStringRef createString( const rtl::OUString& i_rString )
{
    CFMutableStringRef xString = CFStringCreateMutable( NULL, 0 );
    if( xString )
        CFStringAppendCharacters( xString, i_rString.getStr(), i_rString.getLength() );
    return xString;
}

CFURLRef createURL( const rtl::OUString& i_rString )
{

    CFMutableStringRef xMutableString = createString( i_rString );
    CFURLRef xURL = CFURLCreateWithString( NULL, xMutableString, NULL );
    CFRelease( xMutableString );
    return xURL;
}

rtl::OUString getURLFromPath( const rtl::OUString& i_rPath )
{
    CFMutableStringRef xMutableString = createString( i_rPath );
    CFURLRef xURL = CFURLCreateWithFileSystemPath( NULL, xMutableString, kCFURLPOSIXPathStyle, true );
    CFRelease( xMutableString );
    CFStringRef xString = CFURLGetString( xURL );
    rtl::OUString aRet = getString( xString );
    CFRelease( xURL );
    return aRet;
}

CFURLRef createURLFromPath( const rtl::OUString& i_rPath )
{
    CFMutableStringRef xMutableString = createString( i_rPath );
    CFURLRef xURL = CFURLCreateWithFileSystemPath( NULL, xMutableString, kCFURLPOSIXPathStyle, true );
    return xURL;
}

rtl::OUString CFURLtoOSLURL( CFURLRef i_xURL )
{
    // make URL absolute
    CFURLRef xAbsURL = CFURLCopyAbsoluteURL( i_xURL );
    // copy system path
    CFStringRef xSysPath = CFURLCopyFileSystemPath( xAbsURL ? xAbsURL : i_xURL, kCFURLPOSIXPathStyle );
    if( xAbsURL )
        CFRelease( xAbsURL );
    rtl::OUString aSysPath( getString( xSysPath ) );
    CFRelease( xSysPath );
    rtl::OUString aFileURL;
    osl_getFileURLFromSystemPath( aSysPath.pData, &aFileURL.pData );
    return aFileURL;
}

}

using namespace plugstringhelper;

static int parsePlist( CFBundleRef i_xBundle, const rtl::OUString& i_rBundleURL , list< PluginDescription* >& io_rDescriptions )
{
    CFTypeRef xMimeDict = CFBundleGetValueForInfoDictionaryKey( i_xBundle, CFSTR("WebPluginMIMETypes"));
    int nMimetypes = 0;
    if( xMimeDict == 0 ||
        CFGetTypeID(xMimeDict) != CFDictionaryGetTypeID() ||
        (nMimetypes = CFDictionaryGetCount( static_cast<CFDictionaryRef>(xMimeDict))) <= 0 )
    {
        return 0;
    }

    // prepare an array of key and value refs
    std::vector< CFTypeRef > aKeys( nMimetypes, CFTypeRef(NULL) );
    std::vector< CFTypeRef > aValues( nMimetypes, CFTypeRef(NULL) );
    CFDictionaryGetKeysAndValues(static_cast<CFDictionaryRef>(xMimeDict), &aKeys[0], &aValues[0] );

    int nAdded = 0;
    for( int i = 0; i < nMimetypes; i++ )
    {
        // get the mimetype
        CFTypeRef xKey = aKeys[i];
        if( ! xKey || CFGetTypeID(xKey) != CFStringGetTypeID() )
            continue;
        rtl::OUString aMimetype = getString( (CFStringRef)xKey );

        // the correspoding value should be a dictionary
        CFTypeRef xDict = aValues[i];
        if( ! xDict || CFGetTypeID( xDict ) != CFDictionaryGetTypeID() )
            continue;

        // get the extension list
        CFTypeRef xExtArray = CFDictionaryGetValue( (CFDictionaryRef)xDict,  CFSTR("WebPluginExtensions" ) );
        if( !xExtArray || CFGetTypeID( xExtArray ) != CFArrayGetTypeID() )
            continue;

        OUStringBuffer aExtBuf;
        int nExtensions = CFArrayGetCount( (CFArrayRef)xExtArray );
        for( int n = 0; n < nExtensions; n++ )
        {
            CFTypeRef xExt = CFArrayGetValueAtIndex( (CFArrayRef)xExtArray, n );
            if( xExt && CFGetTypeID( xExt ) == CFStringGetTypeID() )
            {
                if( aExtBuf.getLength() > 0 )
                    aExtBuf.append( sal_Unicode(';') );
                OUString aExt( getString( (CFStringRef)xExt ) );
                if( aExt.indexOfAsciiL( "*.", 2 ) != 0 )
                    aExtBuf.appendAscii( "*." );
                aExtBuf.append( aExt );
            }
        }

        // get the description string
        CFTypeRef xDescString = CFDictionaryGetValue( (CFDictionaryRef)xDict,  CFSTR("WebPluginTypeDescription" ) );
        if( !xDescString || CFGetTypeID( xDescString ) != CFStringGetTypeID() )
            continue;
        rtl::OUString aDescription = getString( (CFStringRef)xDescString );

        PluginDescription* pNew = new PluginDescription;
        // set plugin name (path to library)
        pNew->PluginName    = i_rBundleURL;
        // set mimetype
        pNew->Mimetype  = aMimetype;
        // set extension line
        pNew->Extension = aExtBuf.makeStringAndClear();
        // set description
        pNew->Description= aDescription;

        io_rDescriptions.push_back( pNew );
        nAdded++;

#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr,
                 "Inserting from PList:\n"
                 "    Mimetype: %s\n"
                 "    Extension: %s\n"
                 "    Description: %s\n",
                 OUStringToOString( pNew->Mimetype, RTL_TEXTENCODING_UTF8 ).getStr(),
                 OUStringToOString( pNew->Extension, RTL_TEXTENCODING_UTF8 ).getStr(),
                 OUStringToOString( pNew->Description, RTL_TEXTENCODING_UTF8 ).getStr()
                 );
#endif

    }

    return nAdded;
}

static int parseMimeString( const rtl::OUString& i_rBundleURL , list< PluginDescription* >& io_rDescriptions, const char* i_pMime )
{
    if( ! i_pMime )
        return 0;

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    OStringBuffer aMIME;
    aMIME.append( i_pMime );

    if( aMIME.getLength() < 1 )
        return 0;

    OString aLine = aMIME.makeStringAndClear();

    int nAdded = 0;
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
        pNew->PluginName    = i_rBundleURL;
        // set mimetype
        pNew->Mimetype  = OStringToOUString( aMimetype, aEncoding );
        // set extension line
        pNew->Extension = OStringToOUString( aExtension.makeStringAndClear(), aEncoding );
        // set description
        pNew->Description= OStringToOUString( aDesc, aEncoding );
        io_rDescriptions.push_back( pNew );
        nAdded++;

#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr,
                 "Inserting from mime string:\n"
                 "    Mimetype: %s\n"
                 "    Extension: %s\n"
                 "    Description: %s\n",
                 OUStringToOString( pNew->Mimetype, aEncoding ).getStr(),
                 OUStringToOString( pNew->Extension, aEncoding ).getStr(),
                 OUStringToOString( pNew->Description, aEncoding ).getStr()
                 );
#endif
    }
    return nAdded;
}

// this is so ugly it you want to tear your eyes out
static rtl::OUString GetNextPluginStringFromHandle(Handle h, short *index)
{
    char* pPascalBytes = (*h + *index);
    sal_uInt32 nLen = (unsigned char)pPascalBytes[0];
    rtl::OStringBuffer aBuf( nLen );
    aBuf.append( pPascalBytes+1, nLen );
    *index += nLen + 1;
    return rtl::OStringToOUString( aBuf.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
}

static int parseMimeResource( CFBundleRef i_xBundle,
                              oslModule& i_rMod,
                              const rtl::OUString& i_rBundleURL,
                              list< PluginDescription* >& io_rDescriptions )
{
    int nAdded = 0;
    // just to hurt our eyes more there is an alternative mimetype function plus the possibility
    // of a resource fork. Must be a case of think different.
    #if defined __LP64__
    int
    #else
    SInt16
    #endif
    xRes = 0;
    BPSupportedMIMETypes aMIMETypesStrangeStruct = {kBPSupportedMIMETypesStructVers_1, NULL, NULL};

    BP_GetSupportedMIMETypesUPP pBPGetSupp = (BP_GetSupportedMIMETypesUPP)osl_getAsciiFunctionSymbol( i_rMod, "BP_GetSupportedMIMETypes" );
    if( pBPGetSupp &&
        noErr == pBPGetSupp( &aMIMETypesStrangeStruct, 0 ) &&
        aMIMETypesStrangeStruct.typeStrings )
    {
        HLock( aMIMETypesStrangeStruct.typeStrings );
        if( aMIMETypesStrangeStruct.infoStrings )  // it's possible some plugins have infoStrings missing
            HLock( aMIMETypesStrangeStruct.infoStrings );
    }
    else // Try to get data from the resource fork
    {
        xRes = CFBundleOpenBundleResourceMap( i_xBundle );
        if( xRes > 0 )
        {
            aMIMETypesStrangeStruct.typeStrings = Get1Resource('STR#', 128);
            if( aMIMETypesStrangeStruct.typeStrings )
            {
                DetachResource( aMIMETypesStrangeStruct.typeStrings );
                HLock( aMIMETypesStrangeStruct.typeStrings );
                aMIMETypesStrangeStruct.infoStrings = Get1Resource('STR#', 127);
                if( aMIMETypesStrangeStruct.infoStrings )
                {
                    DetachResource( aMIMETypesStrangeStruct.infoStrings );
                    HLock( aMIMETypesStrangeStruct.infoStrings );
                }
            }
        }
    }

    if( aMIMETypesStrangeStruct.typeStrings && aMIMETypesStrangeStruct.infoStrings )
    {
        short nVariantCount = (**(short**)aMIMETypesStrangeStruct.typeStrings) / 2;
        // Fill in the info struct based on the data in the BPSupportedMIMETypes struct
        // this is an array of pascal string of unknown (!) encoding
        // whoever thought of this deserves a fair beating
        short mimeIndex = 2;
        short descriptionIndex = 2;
        for( int i = 0; i < nVariantCount; i++ )
        {
            rtl::OUString aMimetype = GetNextPluginStringFromHandle( aMIMETypesStrangeStruct.typeStrings, &mimeIndex );
            rtl::OUString aExtLine = GetNextPluginStringFromHandle( aMIMETypesStrangeStruct.typeStrings, &mimeIndex );
            rtl::OUString aDescription;
            if( aMIMETypesStrangeStruct.infoStrings )
                aDescription = GetNextPluginStringFromHandle( aMIMETypesStrangeStruct.infoStrings, &descriptionIndex );

            // create extension list string
            sal_Int32 nExtIndex = 0;
            OUStringBuffer aExtension;
            while( nExtIndex != -1 )
            {
                OUString aExt = aExtLine.getToken( 0, ',', nExtIndex);
                if( aExt.indexOfAsciiL( "*.", 2 ) != 0 )
                    aExtension.appendAscii( "*." );
                aExtension.append( aExt );
                if( nExtIndex != -1 )
                    aExtension.append( sal_Unicode(';') );
            }

            PluginDescription* pNew = new PluginDescription;
            // set plugin name (path to library)
            pNew->PluginName    = i_rBundleURL;
            // set mimetype
            pNew->Mimetype  = aMimetype;
            // set extension line
            pNew->Extension = aExtension.makeStringAndClear();
            // set description
            pNew->Description= aDescription;
            io_rDescriptions.push_back( pNew );
            nAdded++;

            #if OSL_DEBUG_LEVEL > 1
            fprintf( stderr,
                     "Inserting from resource:\n"
                     "    Mimetype: %s\n"
                     "    Extension: %s\n"
                     "    Description: %s\n",
                     OUStringToOString( pNew->Mimetype, RTL_TEXTENCODING_UTF8 ).getStr(),
                     OUStringToOString( pNew->Extension, RTL_TEXTENCODING_UTF8 ).getStr(),
                     OUStringToOString( pNew->Description, RTL_TEXTENCODING_UTF8 ).getStr()
                     );
            #endif
        }
    }


    // clean up
    if( aMIMETypesStrangeStruct.typeStrings )
    {
        HUnlock( aMIMETypesStrangeStruct.typeStrings );
        DisposeHandle( aMIMETypesStrangeStruct.typeStrings );
    }
    if( aMIMETypesStrangeStruct.infoStrings )
    {
        HUnlock( aMIMETypesStrangeStruct.infoStrings );
        DisposeHandle( aMIMETypesStrangeStruct.infoStrings );
    }
    if( xRes )
        CFBundleCloseBundleResourceMap( i_xBundle, xRes );

    return nAdded;
}

// check some known bad plugins to avoid crashes
static bool checkBlackList( CFBundleRef i_xBundle )
{
    rtl::OUString aBundleName;
    CFTypeRef bundlename = CFBundleGetValueForInfoDictionaryKey( i_xBundle, CFSTR("CFBundleName"));
    if( bundlename && CFGetTypeID(bundlename) == CFStringGetTypeID() )
        aBundleName = getString( static_cast<CFStringRef>(bundlename) );

    rtl::OUString aBundleVersion;
    CFTypeRef bundleversion = CFBundleGetValueForInfoDictionaryKey( i_xBundle, CFSTR("CFBundleVersion"));
    if( bundleversion && CFGetTypeID(bundleversion) == CFStringGetTypeID() )
        aBundleVersion = getString( static_cast<CFStringRef>(bundleversion) );

    bool bReject = false;
    // #i102735# VLC plugin prior to 1.0 tends to crash
    if( aBundleName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "VLC Plug-in" ) ) )
    {
        sal_Int32 nIndex = 0;
        rtl::OUString aMajor( aBundleVersion.getToken( 0, '.', nIndex ) );
        if( aMajor.toInt32() < 1 )
        {
            bReject = true;
        }
    }
    // #i103674# Garmin Communicator Plugin crashes
    else if( aBundleName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Garmin Communicator Plugin" ) ) )
    {
        bReject = true;
    }

    #if OSL_DEBUG_LEVEL > 1
    if( bReject )
        fprintf( stderr, "rejecting plugin \"%s\" version %s\n",
                 rtl::OUStringToOString( aBundleName, RTL_TEXTENCODING_UTF8 ).getStr(),
                 rtl::OUStringToOString( aBundleVersion, RTL_TEXTENCODING_UTF8 ).getStr()
                 );
    #endif

    return bReject;
}

static int getPluginDescriptions( CFBundleRef i_xBundle , list< PluginDescription* >& io_rDescriptions )
{
    int nDescriptions = 0;
    if( ! i_xBundle )
        return nDescriptions;

    if( checkBlackList( i_xBundle ) )
        return 0;

    rtl::OUString aPlugURL;
    CFURLRef xURL = CFBundleCopyBundleURL( i_xBundle );
    aPlugURL = getString( xURL );
    CFRelease( xURL );

    #if OSL_DEBUG_LEVEL > 1
    rtl::OUString aPlugName, aPlugDescription;
    CFTypeRef name = CFBundleGetValueForInfoDictionaryKey( i_xBundle, CFSTR("WebPluginName"));
    if( name && CFGetTypeID(name) == CFStringGetTypeID() )
      aPlugName = getString( static_cast<CFStringRef>(name) );

    CFTypeRef description = CFBundleGetValueForInfoDictionaryKey( i_xBundle, CFSTR("WebPluginDescription"));
    if( description && CFGetTypeID(description) == CFStringGetTypeID() )
        aPlugDescription = getString( static_cast<CFStringRef>(description) );

    fprintf( stderr, "URL: %s\nname: %s\ndescription: %s\n",
        rtl::OUStringToOString( aPlugURL, RTL_TEXTENCODING_UTF8 ).getStr(),
        rtl::OUStringToOString( aPlugName, RTL_TEXTENCODING_UTF8 ).getStr(),
        rtl::OUStringToOString( aPlugDescription, RTL_TEXTENCODING_UTF8 ).getStr()
        );
    #endif


    // get location of plugin library
    CFURLRef xLibURL = CFBundleCopyExecutableURL( i_xBundle );
    if( ! xLibURL )
        return 0;
    // get the file system path
    rtl::OUString aModuleURL( CFURLtoOSLURL( xLibURL ) );
    CFRelease( xLibURL );

    #if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "exec URL = %s\n", rtl::OUStringToOString( aModuleURL, RTL_TEXTENCODING_UTF8 ).getStr() );
    #endif

    /* TODO: originally the C++ wrapper for oslModule was used here, but that led to
       mysterious crashes in the event loop (pointing to heap corruption). Why using
       the C style oslModule should fix this is completely unknown. It may be that
       we have just hidden the heap corruption a little more.
    */
    oslModule aMod = osl_loadModule( aModuleURL.pData, SAL_LOADMODULE_DEFAULT );
    if( ! aMod )
        return 0;

    // check for at least the init function of a plugin
    if( ! osl_getAsciiFunctionSymbol( aMod, "NP_Initialize") &&
        ! osl_getAsciiFunctionSymbol( aMod, "NP_GetEntryPoints" ) )
    {
        return 0;
    }

    // ask the plist of the bundle for mimetypes
    nDescriptions = parsePlist( i_xBundle, aPlugURL, io_rDescriptions );
    if( nDescriptions )
    {
        osl_unloadModule( aMod );
        return nDescriptions;
    }

    // resolve the symbol that might get us the mimetypes
    const char* (*pGetMimeDescription)() = (const char*(*)())osl_getAsciiFunctionSymbol( aMod, "_NP_GetMIMEDescription" );
    if( pGetMimeDescription )
    {
        const char* pMime = pGetMimeDescription();
        if( pMime )
        {
            nDescriptions = parseMimeString( aPlugURL, io_rDescriptions, pMime );
            if( nDescriptions )
            {
                osl_unloadModule( aMod );
                return nDescriptions;
            }
        }
    }

    // and as last resort check the resource of the bundle
    nDescriptions = parseMimeResource( i_xBundle, aMod, aPlugURL, io_rDescriptions );
    osl_unloadModule( aMod );

    return nDescriptions;
}

// Unix specific implementation
static bool CheckPlugin( const rtl::OUString& rPath, list< PluginDescription* >& rDescriptions )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "Trying path %s ... ", rtl::OUStringToOString( rPath, RTL_TEXTENCODING_UTF8 ).getStr() );
#endif
    CFURLRef xURL = createURL( rPath );

    CFArrayRef xBundles = CFBundleCreateBundlesFromDirectory( NULL, xURL, CFSTR("plugin") );
    if( ! xBundles )
        return false;

    CFIndex nBundles = CFArrayGetCount( xBundles );

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "got %d bundles\n", (int)nBundles );
#endif

    int nDescriptions = 0;
    for( CFIndex i = 0; i < nBundles; i++ )
    {
        CFBundleRef xBundle = (CFBundleRef)CFArrayGetValueAtIndex( xBundles, i );
        nDescriptions += getPluginDescriptions( xBundle, rDescriptions );

        CFRelease( xBundle );
    }
    CFRelease( xBundles );


    return nDescriptions > 0;
}

static rtl::OUString FindFolderURL(  FSVolumeRefNum vRefNum, OSType folderType )
{
    rtl::OUString aRet;

    FSRef aFSRef;
    OSErr err = FSFindFolder( vRefNum, folderType, kDontCreateFolder, &aFSRef );
    if( err == noErr )
    {
        CFURLRef xURL = CFURLCreateFromFSRef( NULL, &aFSRef );
        aRet = getString( xURL );
        CFRelease( xURL );
    }

    return aRet;
}

Sequence<PluginDescription> XPluginManager_Impl::impl_getPluginDescriptions() throw()
{
    static Sequence<PluginDescription> aDescriptions;
    static sal_Bool bHavePlugins = sal_False;
    if( ! bHavePlugins )
    {
        std::list<PluginDescription*> aPlugins;

        static const char* pNPXPluginPath = getenv( "MOZ_PLUGIN_PATH" );

        // get directories
        std::list< rtl::OUString > aPaths;
        if( pNPXPluginPath )
        {
            CFMutableStringRef xMutableString = CFStringCreateMutable( NULL, 0 );
            CFStringAppendCString( xMutableString, pNPXPluginPath, kCFStringEncodingUTF8 );
            CFURLRef xURL = CFURLCreateWithFileSystemPath( NULL, xMutableString, kCFURLPOSIXPathStyle, true );
            CFRelease( xMutableString );
            aPaths.push_back( getString( xURL ) );
            CFRelease( xURL );
        }

        rtl::OUString aPath = FindFolderURL( kUserDomain, kInternetPlugInFolderType );
        if( aPath.getLength() )
            aPaths.push_back( aPath );
        aPath = FindFolderURL( kLocalDomain, kInternetPlugInFolderType );
        if( aPath.getLength() )
            aPaths.push_back( aPath );
        aPath = FindFolderURL( kOnAppropriateDisk, kInternetPlugInFolderType );
        if( aPath.getLength() )
            aPaths.push_back( aPath );


        const Sequence< ::rtl::OUString >& rPaths( PluginManager::getAdditionalSearchPaths() );
        for( sal_Int32 i = 0; i < rPaths.getLength(); i++ )
        {
            aPaths.push_back( getURLFromPath( rPaths.getConstArray()[i] ) );
        }

        for( std::list< rtl::OUString >::const_iterator it = aPaths.begin(); it != aPaths.end(); ++it )
        {
            rtl::OUString aPath( *it );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "check path %s\n", rtl::OUStringToOString( *it, RTL_TEXTENCODING_UTF8 ).getStr() );
#endif
            CheckPlugin( aPath, aPlugins );
        }


        // create return value
        aDescriptions = Sequence<PluginDescription>( aPlugins.size() );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "found %d plugins\n", (int)aPlugins.size() );
#endif
        list<PluginDescription*>::iterator iter;
        sal_Int32 nPlug = 0;
        for( iter = aPlugins.begin(); iter != aPlugins.end(); ++iter )
        {
            aDescriptions.getArray()[ nPlug++ ] = **iter;
            delete *iter;
        }
        aPlugins.clear();
        bHavePlugins = sal_True;
    }
    return aDescriptions;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
