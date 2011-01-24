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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <cstdlib>
#include <cstring>

#include "fontcache.hxx"

#include "osl/thread.h"

#include "unotools/atom.hxx"

#include "tools/stream.hxx"

#include <unistd.h>
#include <sys/stat.h>

#if OSL_DEBUG_LEVEL >1
#include <cstdio>
#endif

#define FONTCACHEFILE "/user/psprint/pspfontcache"
#define CACHE_MAGIC "PspFontCacheFile format 4"

using namespace std;
using namespace rtl;
using namespace psp;
using namespace utl;

/*
 *  static helpers
 */

/*
 *  FontCache constructor
 */

FontCache::FontCache()
{
    m_bDoFlush = false;
    m_aCacheFile = getOfficePath( UserPath );
    if( m_aCacheFile.Len() )
    {
        m_aCacheFile.AppendAscii( FONTCACHEFILE );
        read();
    }
}

/*
 *  FontCache destructor
 */

FontCache::~FontCache()
{
    clearCache();
}

/*
 *  FontCache::clearCache
 */
void FontCache::clearCache()
{
    for( FontCacheData::iterator dir_it = m_aCache.begin(); dir_it != m_aCache.end(); ++dir_it )
    {
        for( FontDirMap::iterator entry_it = dir_it->second.m_aEntries.begin(); entry_it != dir_it->second.m_aEntries.end(); ++entry_it )
        {
            for( FontCacheEntry::iterator font_it = entry_it->second.m_aEntry.begin(); font_it != entry_it->second.m_aEntry.end(); ++font_it )
                delete *font_it;
        }
    }
    m_aCache.clear();
}

/*
 *  FontCache::Commit
 */

void FontCache::flush()
{
    if( ! m_bDoFlush || ! m_aCacheFile.Len() )
        return;

    SvFileStream aStream;
    aStream.Open( m_aCacheFile, STREAM_WRITE | STREAM_TRUNC );
    if( ! (aStream.IsOpen() && aStream.IsWritable()) )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "FontCache::flush: opening cache file %s failed\n", ByteString( m_aCacheFile, osl_getThreadTextEncoding() ).GetBuffer() );
#endif
        return;
    }

    aStream.SetLineDelimiter( LINEEND_LF );
    aStream.WriteLine( ByteString( CACHE_MAGIC ) );

    PrintFontManager& rManager( PrintFontManager::get() );
    MultiAtomProvider* pAtoms = rManager.m_pAtoms;

    for( FontCacheData::const_iterator dir_it = m_aCache.begin(); dir_it != m_aCache.end(); ++ dir_it )
    {
        const FontDirMap& rDir( dir_it->second.m_aEntries );

        ByteString aDirectory( rManager.getDirectory( dir_it->first ) );
        ByteString aLine( "FontCacheDirectory:" );
        aLine.Append( ByteString::CreateFromInt64( dir_it->second.m_nTimestamp ) );
        aLine.Append( ':' );
        aLine.Append( aDirectory );
        if( rDir.empty() && dir_it->second.m_bNoFiles )
            aLine.Insert( "Empty", 0 );
        aStream.WriteLine( aLine );

        for( FontDirMap::const_iterator entry_it = rDir.begin(); entry_it != rDir.end(); ++entry_it )
        {
            // insert cache entries
            const FontCacheEntry& rEntry( entry_it->second.m_aEntry );
            if( rEntry.begin() == rEntry.end() )
                continue;

            aLine = "File:";
            aLine.Append( ByteString( entry_it->first ) );
            aStream.WriteLine( aLine );

            int nEntrySize = entry_it->second.m_aEntry.size();
            // write: type;nfonts
            aLine = ByteString::CreateFromInt32( rEntry.front()->m_eType );
            aLine.Append( ';' );
            aLine.Append( ByteString::CreateFromInt32( nEntrySize ) );
            aStream.WriteLine( aLine );

            sal_Int32 nSubEntry = 0;
            for( FontCacheEntry::const_iterator it = rEntry.begin(); it != rEntry.end(); ++it, nSubEntry++ )
            {
                /*
                 *  for each font entry write:
                 *  name[;name[;name]]
                 *  fontnr;PSName;italic;weight;width;pitch;encoding;ascend;descend;leading;vsubst;gxw;gxh;gyw;gyh;useroverrride;embed;antialias[;{metricfile,typeflags}][;stylename]
                 */
                if( nEntrySize > 1 )
                    nSubEntry = static_cast<const PrintFontManager::TrueTypeFontFile*>(*it)->m_nCollectionEntry;
                else
                    nSubEntry = -1;

                aLine = OUStringToOString( pAtoms->getString( ATOM_FAMILYNAME, (*it)->m_nFamilyName ), RTL_TEXTENCODING_UTF8 );
                for( ::std::list< int >::const_iterator name_it = (*it)->m_aAliases.begin(); name_it != (*it)->m_aAliases.end(); ++name_it )
                {
                    const OUString& rAdd( pAtoms->getString( ATOM_FAMILYNAME, *name_it ) );
                    if( rAdd.getLength() )
                    {
                        aLine.Append( ';' );
                        aLine.Append( ByteString( String( rAdd ), RTL_TEXTENCODING_UTF8 ) );
                    }
                }
                aStream.WriteLine( aLine );

                const OUString& rPSName( pAtoms->getString( ATOM_PSNAME, (*it)->m_nPSName ) );
                aLine = ByteString::CreateFromInt32( nSubEntry );
                aLine.Append( ';' );
                aLine.Append( ByteString( String( rPSName ), RTL_TEXTENCODING_UTF8 ) );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( (*it)->m_eItalic ) );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( (*it)->m_eWeight ) );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( (*it)->m_eWidth ) );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( (*it)->m_ePitch ) );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( (*it)->m_aEncoding ) );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( (*it)->m_nAscend ) );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( (*it)->m_nDescend ) );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( (*it)->m_nLeading ) );
                aLine.Append( ';' );
                aLine.Append( (*it)->m_bHaveVerticalSubstitutedGlyphs ? "1" : "0" );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( (*it)->m_aGlobalMetricX.width ) );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( (*it)->m_aGlobalMetricX.height ) );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( (*it)->m_aGlobalMetricY.width ) );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( (*it)->m_aGlobalMetricY.height ) );
                aLine.Append( ';' );
                aLine.Append( (*it)->m_bUserOverride ? "1" : "0" );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( 0 ) );
                aLine.Append( ';' );
                aLine.Append( ByteString::CreateFromInt32( 0 ) );

                switch( (*it)->m_eType )
                {
                    case fonttype::Type1:
                        aLine.Append( ';' );
                        aLine.Append( ByteString( static_cast<const PrintFontManager::Type1FontFile*>(*it)->m_aMetricFile ) );
                        break;
                    case fonttype::TrueType:
                        aLine.Append( ';' );
                        aLine.Append( ByteString::CreateFromInt32( static_cast<const PrintFontManager::TrueTypeFontFile*>(*it)->m_nTypeFlags ) );
                        break;
                    default: break;
                }
                if( (*it)->m_aStyleName.getLength() )
                {
                    aLine.Append( ';' );
                    aLine.Append( ByteString( String( (*it)->m_aStyleName ), RTL_TEXTENCODING_UTF8 ) );
                }
                aStream.WriteLine( aLine );
            }
            aStream.WriteLine( ByteString() );
        }
    }
    m_bDoFlush = false;
}

/*
 * FontCache::read
 */

void FontCache::read()
{
    PrintFontManager& rManager( PrintFontManager::get() );
    MultiAtomProvider* pAtoms = rManager.m_pAtoms;

    SvFileStream aStream( m_aCacheFile, STREAM_READ );
    if( ! aStream.IsOpen() )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "FontCache::read: opening cache file %s failed\n", ByteString( m_aCacheFile, osl_getThreadTextEncoding() ).GetBuffer() );
#endif
        return;
    }


    ByteString aLine;
    aStream.ReadLine( aLine );
    if( !aLine.Equals( CACHE_MAGIC ) )
    {
        #if OSL_DEBUG_LEVEL >1
        fprintf( stderr, "FontCache::read: cache file %s fails magic test\n", ByteString( m_aCacheFile, osl_getThreadTextEncoding() ).GetBuffer() );
        #endif
        return;
    }

    int nDir = 0;
    FontDirMap* pDir = NULL;
    xub_StrLen nIndex;
    bool bKeepOnlyUserOverridden = false;
    do
    {
        aStream.ReadLine( aLine );
        if( aLine.CompareTo( "FontCacheDirectory:", 19 ) == COMPARE_EQUAL ||
            aLine.CompareTo( "EmptyFontCacheDirectory:", 24 ) == COMPARE_EQUAL )
        {
            bool bEmpty = (aLine.CompareTo( "Empty", 5 ) == COMPARE_EQUAL);
            xub_StrLen nSearchIndex = bEmpty ? 24 : 19;

            OString aDir;
            sal_Int64 nTimestamp = 0;
            xub_StrLen nTEnd = aLine.Search( ':', nSearchIndex );
            if( nTEnd != STRING_NOTFOUND )
            {
                nTimestamp = aLine.Copy( nSearchIndex, nTEnd - nSearchIndex ).ToInt64();
                aDir = aLine.Copy( nTEnd+1 );
            }
            else
            {
                // invalid format, remove
                pDir = NULL;
                nDir = 0;
                m_bDoFlush = true;
                continue;
            }

            // is the directory modified ?
            struct stat aStat;
            if( stat( aDir.getStr(), &aStat )               ||
                ! S_ISDIR(aStat.st_mode) )
            {
                // remove outdated cache data
                pDir = NULL;
                nDir = 0;
                m_bDoFlush = true;
                continue;
            }
            else
            {
                nDir = rManager.getDirectoryAtom( aDir, true );
                m_aCache[ nDir ].m_nTimestamp = (sal_Int64)aStat.st_mtime;
                m_aCache[ nDir ].m_bNoFiles = bEmpty;
                pDir = bEmpty ? NULL : &m_aCache[ nDir ].m_aEntries;
                bKeepOnlyUserOverridden = ((sal_Int64)aStat.st_mtime != nTimestamp);
                m_aCache[ nDir ].m_bUserOverrideOnly = bKeepOnlyUserOverridden;
            }
        }
        else if( pDir && aLine.CompareTo( "File:", 5 ) == COMPARE_EQUAL )
        {
            OString aFile( aLine.Copy( 5 ) );
            aStream.ReadLine( aLine );

            const char* pLine = aLine.GetBuffer();

            fonttype::type eType = (fonttype::type)atoi( pLine );
            if( eType != fonttype::TrueType     &&
                eType != fonttype::Type1        &&
                eType != fonttype::Builtin
                )
                continue;
            while( *pLine && *pLine != ';' )
                pLine++;
            if( *pLine != ';' )
                continue;

            pLine++;
            sal_Int32 nFonts = atoi( pLine );
            for( int n = 0; n < nFonts; n++ )
            {
                aStream.ReadLine( aLine );
                pLine = aLine.GetBuffer();
                int nLen = aLine.Len();

                PrintFontManager::PrintFont* pFont = NULL;
                switch( eType )
                {
                    case fonttype::TrueType:
                        pFont = new PrintFontManager::TrueTypeFontFile();
                        break;
                    case fonttype::Type1:
                        pFont = new PrintFontManager::Type1FontFile();
                        break;
                    case fonttype::Builtin:
                        pFont = new PrintFontManager::BuiltinFont();
                        break;
                    default: break;
                }

                for( nIndex = 0; nIndex < nLen && pLine[nIndex] != ';'; nIndex++ )
                    ;

                pFont->m_nFamilyName = pAtoms->getAtom( ATOM_FAMILYNAME,
                                                        OUString( pLine, nIndex, RTL_TEXTENCODING_UTF8 ),
                                                        sal_True );
                while( nIndex < nLen )
                {
                    xub_StrLen nLastIndex = nIndex+1;
                    for( nIndex = nLastIndex ; nIndex < nLen && pLine[nIndex] != ';'; nIndex++ )
                        ;
                    if( nIndex - nLastIndex )
                    {
                        OUString aAlias( pLine+nLastIndex, nIndex-nLastIndex, RTL_TEXTENCODING_UTF8 );
                        pFont->m_aAliases.push_back( pAtoms->getAtom( ATOM_FAMILYNAME, aAlias, sal_True ) );
                    }
                }
                aStream.ReadLine( aLine );
                pLine = aLine.GetBuffer();
                nLen = aLine.Len();

                // get up to 20 token positions
                const int nMaxTokens = 20;
                int nTokenPos[nMaxTokens];
                nTokenPos[0] = 0;
                int nTokens = 1;
                for( int i = 0; i < nLen; i++ )
                {
                    if( pLine[i] == ';' )
                    {
                        nTokenPos[nTokens++] = i+1;
                        if( nTokens == nMaxTokens )
                            break;
                    }
                }
                if( nTokens < 18 )
                {
                    delete pFont;
                    continue;
                }
                int nCollEntry      = atoi( pLine );
                pFont->m_nPSName    = pAtoms->getAtom( ATOM_PSNAME, OUString( pLine + nTokenPos[1], nTokenPos[2]-nTokenPos[1]-1, RTL_TEXTENCODING_UTF8 ), sal_True );
                pFont->m_eItalic    = (italic::type)atoi( pLine+nTokenPos[2] );
                pFont->m_eWeight    = (weight::type)atoi( pLine+nTokenPos[3] );
                pFont->m_eWidth     = (width::type)atoi( pLine+nTokenPos[4] );
                pFont->m_ePitch     = (pitch::type)atoi( pLine+nTokenPos[5] );
                pFont->m_aEncoding  = (rtl_TextEncoding)atoi( pLine+nTokenPos[6] );
                pFont->m_nAscend    = atoi( pLine + nTokenPos[7] );
                pFont->m_nDescend   = atoi( pLine + nTokenPos[8] );
                pFont->m_nLeading   = atoi( pLine + nTokenPos[9] );
                pFont->m_bHaveVerticalSubstitutedGlyphs
                                    = (atoi( pLine + nTokenPos[10] ) != 0);
                pFont->m_aGlobalMetricX.width
                                    = atoi( pLine + nTokenPos[11] );
                pFont->m_aGlobalMetricX.height
                                    = atoi( pLine + nTokenPos[12] );
                pFont->m_aGlobalMetricY.width
                                    = atoi( pLine + nTokenPos[13] );
                pFont->m_aGlobalMetricY.height
                                    = atoi( pLine + nTokenPos[14] );
                pFont->m_bUserOverride
                                    = (atoi( pLine + nTokenPos[15] ) != 0);
                int nStyleTokenNr = 18;
                switch( eType )
                {
                    case fonttype::TrueType:
                        static_cast<PrintFontManager::TrueTypeFontFile*>(pFont)->m_nTypeFlags = atoi( pLine + nTokenPos[18] );
                        static_cast<PrintFontManager::TrueTypeFontFile*>(pFont)->m_nCollectionEntry = nCollEntry;
                        static_cast<PrintFontManager::TrueTypeFontFile*>(pFont)->m_nDirectory = nDir;
                        static_cast<PrintFontManager::TrueTypeFontFile*>(pFont)->m_aFontFile = aFile;
                        nStyleTokenNr++;
                        break;
                    case fonttype::Type1:
                    {
                        int nTokLen = (nTokens > 19 ) ? nTokenPos[19]-nTokenPos[18]-1 : nLen - nTokenPos[18];
                        static_cast<PrintFontManager::Type1FontFile*>(pFont)->m_aMetricFile = OString( pLine + nTokenPos[18], nTokLen );
                        static_cast<PrintFontManager::Type1FontFile*>(pFont)->m_nDirectory = nDir;
                        static_cast<PrintFontManager::Type1FontFile*>(pFont)->m_aFontFile = aFile;
                        nStyleTokenNr++;
                    }
                    break;
                    case fonttype::Builtin:
                        static_cast<PrintFontManager::BuiltinFont*>(pFont)->m_nDirectory = nDir;
                        static_cast<PrintFontManager::BuiltinFont*>(pFont)->m_aMetricFile = aFile;
                        break;
                    default: break;
                }
                if( nTokens > nStyleTokenNr )
                    pFont->m_aStyleName = OUString::intern( pLine + nTokenPos[nStyleTokenNr],
                                                            nLen - nTokenPos[nStyleTokenNr],
                                                            RTL_TEXTENCODING_UTF8 );

                bool bObsolete = false;
                if( bKeepOnlyUserOverridden )
                {
                    if( pFont->m_bUserOverride )
                    {
                        ByteString aFilePath = rManager.getDirectory( nDir );
                        aFilePath.Append( '/' );
                        aFilePath.Append( ByteString(aFile) );
                        struct stat aStat;
                        if( stat( aFilePath.GetBuffer(), &aStat )   ||
                            ! S_ISREG( aStat.st_mode )              ||
                            aStat.st_size < 16 )
                        {
                            bObsolete = true;
                        }
                        #if OSL_DEBUG_LEVEL > 2
                        else
                            fprintf( stderr, "keeping file %s in outdated cache entry due to user override\n",
                                     aFilePath.GetBuffer() );
                        #endif
                    }
                    else
                        bObsolete = true;
                }
                if( bObsolete )
                {
                    m_bDoFlush = true;
#if OSL_DEBUG_LEVEL > 2
                    fprintf( stderr, "removing obsolete font %s\n", aFile.getStr() );
#endif
                    delete pFont;
                    continue;
                }

                FontCacheEntry& rEntry = (*pDir)[aFile].m_aEntry;
                rEntry.push_back( pFont );
            }
        }
    } while( ! aStream.IsEof() );
}

/*
 *  FontCache::updateDirTimestamp
 */
void FontCache::updateDirTimestamp( int nDirID )
{
    PrintFontManager& rManager( PrintFontManager::get() );
    const OString& rDir = rManager.getDirectory( nDirID );

    struct stat aStat;
    if( ! stat( rDir.getStr(), &aStat ) )
        m_aCache[ nDirID ].m_nTimestamp = (sal_Int64)aStat.st_mtime;
}


/*
 *  FontCache::copyPrintFont
 */
void FontCache::copyPrintFont( const PrintFontManager::PrintFont* pFrom, PrintFontManager::PrintFont* pTo ) const
{
    if( pFrom->m_eType != pTo->m_eType )
        return;
    switch( pFrom->m_eType )
    {
        case fonttype::TrueType:
            static_cast<PrintFontManager::TrueTypeFontFile*>(pTo)->m_nDirectory = static_cast<const PrintFontManager::TrueTypeFontFile*>(pFrom)->m_nDirectory;
            static_cast<PrintFontManager::TrueTypeFontFile*>(pTo)->m_aFontFile = static_cast<const PrintFontManager::TrueTypeFontFile*>(pFrom)->m_aFontFile;
            static_cast<PrintFontManager::TrueTypeFontFile*>(pTo)->m_nCollectionEntry = static_cast<const PrintFontManager::TrueTypeFontFile*>(pFrom)->m_nCollectionEntry;
            static_cast<PrintFontManager::TrueTypeFontFile*>(pTo)->m_nTypeFlags = static_cast<const PrintFontManager::TrueTypeFontFile*>(pFrom)->m_nTypeFlags;
            break;
        case fonttype::Type1:
            static_cast<PrintFontManager::Type1FontFile*>(pTo)->m_nDirectory = static_cast<const PrintFontManager::Type1FontFile*>(pFrom)->m_nDirectory;
            static_cast<PrintFontManager::Type1FontFile*>(pTo)->m_aFontFile = static_cast<const PrintFontManager::Type1FontFile*>(pFrom)->m_aFontFile;
            static_cast<PrintFontManager::Type1FontFile*>(pTo)->m_aMetricFile = static_cast<const PrintFontManager::Type1FontFile*>(pFrom)->m_aMetricFile;
            break;
        case fonttype::Builtin:
            static_cast<PrintFontManager::BuiltinFont*>(pTo)->m_nDirectory = static_cast<const PrintFontManager::BuiltinFont*>(pFrom)->m_nDirectory;
            static_cast<PrintFontManager::BuiltinFont*>(pTo)->m_aMetricFile = static_cast<const PrintFontManager::BuiltinFont*>(pFrom)->m_aMetricFile;
            break;
        default: break;
    }
    pTo->m_nFamilyName      = pFrom->m_nFamilyName;
    pTo->m_aStyleName       = pFrom->m_aStyleName;
    pTo->m_aAliases         = pFrom->m_aAliases;
    pTo->m_nPSName          = pFrom->m_nPSName;
    pTo->m_eItalic          = pFrom->m_eItalic;
    pTo->m_eWeight          = pFrom->m_eWeight;
    pTo->m_eWidth           = pFrom->m_eWidth;
    pTo->m_ePitch           = pFrom->m_ePitch;
    pTo->m_aEncoding        = pFrom->m_aEncoding;
    pTo->m_aGlobalMetricX   = pFrom->m_aGlobalMetricX;
    pTo->m_aGlobalMetricY   = pFrom->m_aGlobalMetricY;
    pTo->m_nAscend          = pFrom->m_nAscend;
    pTo->m_nDescend         = pFrom->m_nDescend;
    pTo->m_nLeading         = pFrom->m_nLeading;
    pTo->m_nXMin            = pFrom->m_nXMin;
    pTo->m_nYMin            = pFrom->m_nYMin;
    pTo->m_nXMax            = pFrom->m_nXMax;
    pTo->m_nYMax            = pFrom->m_nYMax;
    pTo->m_bHaveVerticalSubstitutedGlyphs = pFrom->m_bHaveVerticalSubstitutedGlyphs;
    pTo->m_bUserOverride    = pFrom->m_bUserOverride;
}

/*
 *  FontCache::equalsPrintFont
 */
bool FontCache::equalsPrintFont( const PrintFontManager::PrintFont* pLeft, PrintFontManager::PrintFont* pRight ) const
{
    if( pLeft->m_eType != pRight->m_eType )
        return false;
    switch( pLeft->m_eType )
    {
        case fonttype::TrueType:
        {
            const PrintFontManager::TrueTypeFontFile* pLT = static_cast<const PrintFontManager::TrueTypeFontFile*>(pLeft);
            const PrintFontManager::TrueTypeFontFile* pRT = static_cast<const PrintFontManager::TrueTypeFontFile*>(pRight);
            if( pRT->m_nDirectory       != pLT->m_nDirectory        ||
                pRT->m_aFontFile        != pLT->m_aFontFile         ||
                pRT->m_nCollectionEntry != pLT->m_nCollectionEntry  ||
                pRT->m_nTypeFlags       != pLT->m_nTypeFlags )
                return false;
        }
        break;
        case fonttype::Type1:
        {
            const PrintFontManager::Type1FontFile* pLT = static_cast<const PrintFontManager::Type1FontFile*>(pLeft);
            const PrintFontManager::Type1FontFile* pRT = static_cast<const PrintFontManager::Type1FontFile*>(pRight);
            if( pRT->m_nDirectory       != pLT->m_nDirectory        ||
                pRT->m_aFontFile        != pLT->m_aFontFile         ||
                pRT->m_aMetricFile      != pLT->m_aMetricFile )
                return false;
        }
        break;
        case fonttype::Builtin:
        {
            const PrintFontManager::BuiltinFont* pLT = static_cast<const PrintFontManager::BuiltinFont*>(pLeft);
            const PrintFontManager::BuiltinFont* pRT = static_cast<const PrintFontManager::BuiltinFont*>(pRight);
            if( pRT->m_nDirectory       != pLT->m_nDirectory        ||
                pRT->m_aMetricFile      != pLT->m_aMetricFile )
                return false;
        }
        break;
        default: break;
    }
    if( pRight->m_nFamilyName       != pLeft->m_nFamilyName     ||
        pRight->m_aStyleName        != pLeft->m_aStyleName      ||
        pRight->m_nPSName           != pLeft->m_nPSName         ||
        pRight->m_eItalic           != pLeft->m_eItalic         ||
        pRight->m_eWeight           != pLeft->m_eWeight         ||
        pRight->m_eWidth            != pLeft->m_eWidth          ||
        pRight->m_ePitch            != pLeft->m_ePitch          ||
        pRight->m_aEncoding         != pLeft->m_aEncoding       ||
        pRight->m_aGlobalMetricX    != pLeft->m_aGlobalMetricX  ||
        pRight->m_aGlobalMetricY    != pLeft->m_aGlobalMetricY  ||
        pRight->m_nAscend           != pLeft->m_nAscend         ||
        pRight->m_nDescend          != pLeft->m_nDescend        ||
        pRight->m_nLeading          != pLeft->m_nLeading        ||
        pRight->m_nXMin             != pLeft->m_nXMin           ||
        pRight->m_nYMin             != pLeft->m_nYMin           ||
        pRight->m_nXMax             != pLeft->m_nXMax           ||
        pRight->m_nYMax             != pLeft->m_nYMax           ||
        pRight->m_bHaveVerticalSubstitutedGlyphs != pLeft->m_bHaveVerticalSubstitutedGlyphs ||
        pRight->m_bUserOverride     != pLeft->m_bUserOverride
        )
        return false;
    std::list< int >::const_iterator lit, rit;
    for( lit = pLeft->m_aAliases.begin(), rit = pRight->m_aAliases.begin();
         lit != pLeft->m_aAliases.end() && rit != pRight->m_aAliases.end() && (*lit) == (*rit);
         ++lit, ++rit )
        ;
    return lit == pLeft->m_aAliases.end() && rit == pRight->m_aAliases.end();
}

/*
 *  FontCache::clonePrintFont
 */
PrintFontManager::PrintFont* FontCache::clonePrintFont( const PrintFontManager::PrintFont* pOldFont ) const
{
    PrintFontManager::PrintFont* pFont = NULL;
    switch( pOldFont->m_eType )
    {
        case fonttype::TrueType:
            pFont = new PrintFontManager::TrueTypeFontFile();
            break;
        case fonttype::Type1:
            pFont = new PrintFontManager::Type1FontFile();
            break;
        case fonttype::Builtin:
            pFont = new PrintFontManager::BuiltinFont();
            break;
        default: break;
    }
    if( pFont )
    {
        copyPrintFont( pOldFont, pFont );
    }
    return pFont;
 }

/*
 *  FontCache::getFontCacheFile
 */
bool FontCache::getFontCacheFile( int nDirID, const OString& rFile, list< PrintFontManager::PrintFont* >& rNewFonts ) const
{
    bool bSuccess = false;

    FontCacheData::const_iterator dir = m_aCache.find( nDirID );
    if( dir != m_aCache.end() )
    {
        FontDirMap::const_iterator entry = dir->second.m_aEntries.find( rFile );
        if( entry != dir->second.m_aEntries.end() )
        {
            for( FontCacheEntry::const_iterator font = entry->second.m_aEntry.begin(); font != entry->second.m_aEntry.end(); ++font )
            {
                bSuccess = true;
                PrintFontManager::PrintFont* pFont = clonePrintFont( *font );
                rNewFonts.push_back( pFont );
            }
        }
    }
    return bSuccess;
}

/*
 *  FontCache::updateFontCacheEntry
 */
void FontCache::updateFontCacheEntry( const PrintFontManager::PrintFont* pFont, bool bFlush )
{
    PrintFontManager& rManager( PrintFontManager::get() );

    OString aFile;
    int nDirID = 0;
    switch( pFont->m_eType )
    {
        case fonttype::TrueType:
            nDirID = static_cast<const PrintFontManager::TrueTypeFontFile*>(pFont)->m_nDirectory;
            aFile = static_cast<const PrintFontManager::TrueTypeFontFile*>(pFont)->m_aFontFile;
            break;
        case fonttype::Type1:
            nDirID = static_cast<const PrintFontManager::Type1FontFile*>(pFont)->m_nDirectory;
            aFile = static_cast<const PrintFontManager::Type1FontFile*>(pFont)->m_aFontFile;
            break;
        case fonttype::Builtin:
            nDirID = static_cast<const PrintFontManager::BuiltinFont*>(pFont)->m_nDirectory;
            aFile = static_cast<const PrintFontManager::BuiltinFont*>(pFont)->m_aMetricFile;
            break;
        default:
            return;
    }
    FontCacheData::const_iterator dir = m_aCache.find( nDirID );
    FontDirMap::const_iterator entry;
    FontCacheEntry::const_iterator font;
    PrintFontManager::PrintFont* pCacheFont = NULL;

    if( dir != m_aCache.end() )
    {
        entry = dir->second.m_aEntries.find( aFile );
        if( entry != dir->second.m_aEntries.end() )
        {
            for( font = entry->second.m_aEntry.begin(); font != entry->second.m_aEntry.end(); ++font )
            {
                if( (*font)->m_eType == pFont->m_eType &&
                    ( (*font)->m_eType != fonttype::TrueType ||
                      static_cast<const PrintFontManager::TrueTypeFontFile*>(*font)->m_nCollectionEntry == static_cast<const PrintFontManager::TrueTypeFontFile*>(pFont)->m_nCollectionEntry
                      ) )
                    break;
            }
            if( font != entry->second.m_aEntry.end() )
                pCacheFont = *font;
        }
    }
    else
        createCacheDir( nDirID );

    if( pCacheFont )
    {
        if( ! equalsPrintFont( pFont, pCacheFont ) )
        {
            copyPrintFont( pFont, pCacheFont );
            m_bDoFlush = true;
        }
    }
    else
    {
        pCacheFont = clonePrintFont( pFont );
        m_aCache[nDirID].m_aEntries[aFile].m_aEntry.push_back( pCacheFont );

        ByteString aPath = rManager.getDirectory( nDirID );
        aPath.Append( '/' );
        aPath.Append( ByteString( aFile ) );
        m_bDoFlush = true;
    }
    if( bFlush )
        flush();
}

/*
 *  FontCache::listDirectory
 */
bool FontCache::listDirectory( const OString& rDir, std::list< PrintFontManager::PrintFont* >& rNewFonts ) const
{
    PrintFontManager& rManager( PrintFontManager::get() );
    int nDirID = rManager.getDirectoryAtom( rDir );
    FontCacheData::const_iterator dir = m_aCache.find( nDirID );
    bool bFound = (dir != m_aCache.end());

    if( bFound && !dir->second.m_bNoFiles )
    {
        for( FontDirMap::const_iterator file = dir->second.m_aEntries.begin(); file != dir->second.m_aEntries.end(); ++file )
        {
            for( FontCacheEntry::const_iterator font = file->second.m_aEntry.begin(); font != file->second.m_aEntry.end(); ++font )
            {
                PrintFontManager::PrintFont* pFont = clonePrintFont( *font );
                rNewFonts.push_back( pFont );
            }
        }
    }
    return bFound;
}

/*
 *  FontCache::listDirectory
 */
bool FontCache::scanAdditionalFiles( const OString& rDir )
{
    PrintFontManager& rManager( PrintFontManager::get() );
    int nDirID = rManager.getDirectoryAtom( rDir );
    FontCacheData::const_iterator dir = m_aCache.find( nDirID );
    bool bFound = (dir != m_aCache.end());

    return (bFound && dir->second.m_bUserOverrideOnly);
}

/*
 *  FontCache::createCacheDir
 */
void FontCache::createCacheDir( int nDirID )
{
    PrintFontManager& rManager( PrintFontManager::get() );

    const OString& rDir = rManager.getDirectory( nDirID );
    struct stat aStat;
    if( ! stat( rDir.getStr(), &aStat ) )
        m_aCache[nDirID].m_nTimestamp = (sal_Int64)aStat.st_mtime;
}

/*
 *  FontCache::markEmptyDir
 */
void FontCache::markEmptyDir( int nDirID, bool bNoFiles )
{
    createCacheDir( nDirID );
    m_aCache[nDirID].m_bNoFiles = bNoFiles;
    m_bDoFlush = true;
}
