/*************************************************************************
 *
 *  $RCSfile: fontcache.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: pl $ $Date: 2002-07-20 15:21:18 $
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

#include <stdlib.h>
#include <string.h>
#include <psprint/fontcache.hxx>

#ifndef _OSL_THREAD_H
#include <osl/thread.h>
#endif
#ifndef _UTL_ATOM_HXX_
#include <unotools/atom.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>


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
    String aPrinterPath( getPrinterPath() );
    String aPath;
    xub_StrLen nIndex = 0;
    rtl_TextEncoding aEnconding = osl_getThreadTextEncoding();

    while( nIndex != STRING_NOTFOUND )
    {
        aPath = aPrinterPath.GetToken( 0, ':', nIndex );
        read( ByteString( aPath, aEnconding ) );
    }
    m_bDoFlush = false;
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
        for( FontDirMap::iterator entry_it = dir_it->second.begin(); entry_it != dir_it->second.end(); ++entry_it )
        {
            for( FontCacheEntry::iterator font_it = entry_it->second.begin(); font_it != entry_it->second.end(); ++font_it )
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
    if( ! m_bDoFlush )
        return;

    OUString aPrinterPath( getPrinterPath() );
    String aPath;
    bool bHavePath = false;
    sal_Int32 nIndex = 0;
    SvFileStream aStream;
    while( nIndex >= 0 )
    {
        aPath = aPrinterPath.getToken( 0, ':', nIndex );
        aPath.AppendAscii( "/pspfontcache" );
        aStream.Open( aPath, STREAM_READ | STREAM_WRITE );
        if( aStream.IsOpen() && aStream.IsWritable() )
        {
            bHavePath = true;
            break;
        }
    }

    if( ! bHavePath )
        return;

    aStream.SetLineDelimiter( LINEEND_LF );

    PrintFontManager& rManager( PrintFontManager::get() );
    MultiAtomProvider* pAtoms = rManager.m_pAtoms;

    for( FontCacheData::const_iterator dir_it = m_aCache.begin(); dir_it != m_aCache.end(); ++ dir_it )
    {
        ByteString aDirectory( rManager.getDirectory( dir_it->first ) );
        ByteString aLine( "FontCacheDirectory:" );
        aLine.Append( aDirectory );
        aStream.WriteLine( aLine );

        const FontDirMap& rDir( dir_it->second );
        for( FontDirMap::const_iterator entry_it = rDir.begin(); entry_it != rDir.end(); ++entry_it )
        {
            // insert cache entries
            const FontCacheEntry& rEntry( entry_it->second );
            if( rEntry.begin() == rEntry.end() )
                continue;

            struct stat aStat;
            ByteString aFileName( aDirectory );
            aFileName.Append( '/' );
            aFileName.Append( ByteString( entry_it->first ) );
            if( stat( aFileName.GetBuffer(), &aStat ) )
                continue;

            aLine = "File:";
            aLine.Append( ByteString( entry_it->first ) );
            aStream.WriteLine( aLine );

            int nEntrySize = entry_it->second.size();
            // write: type;mtime;nfonts
            aLine = ByteString::CreateFromInt32( rEntry.front()->m_eType );
            aLine.Append( ';' );
            aLine.Append( ByteString::CreateFromInt64( aStat.st_mtime ) );
            aLine.Append( ';' );
            aLine.Append( ByteString::CreateFromInt32( nEntrySize ) );
            aStream.WriteLine( aLine );

            sal_Int32 nSubEntry = 0;
            for( FontCacheEntry::const_iterator it = rEntry.begin(); it != rEntry.end(); ++it, nSubEntry++ )
            {
                /*
                 *  for each font entry write:
                 *  name[;name[;name]]
                 *  fontnr;PSName;italic;weight;width;pitch;encoding;ascend;descend;leading;vsubst;gxw;gxh;gyw;gyh[;{metricfile,typeflags}]
                 */
                if( nEntrySize > 1 )
                    nSubEntry = static_cast<const PrintFontManager::TrueTypeFontFile*>(*it)->m_nCollectionEntry;

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
                }
                aStream.WriteLine( aLine );
            }
            aStream.WriteLine( ByteString() );
        }
    }
    m_bDoFlush = false;
}

/*
 * FontCache::getValues
 */

void FontCache::read( const OString& rPath )
{
    String aFilePath = String( ByteString( rPath ), osl_getThreadTextEncoding() );
    aFilePath.AppendAscii( "/pspfontcache" );
    PrintFontManager& rManager( PrintFontManager::get() );
    MultiAtomProvider* pAtoms = rManager.m_pAtoms;

    SvFileStream aStream( aFilePath, STREAM_READ );
    if( ! aStream.IsOpen() )
        return;

    m_bDoFlush = true;

    ByteString aLine;
    OString aDir( rPath );
    int nDir = rManager.getDirectoryAtom( aDir, true );
    FontDirMap* pDir = &m_aCache[ nDir ];
    xub_StrLen nIndex;
    do
    {
        aStream.ReadLine( aLine );
        if( aLine.CompareTo( "FontCacheDirectory:", 19 ) == COMPARE_EQUAL )
        {
            aDir = aLine.Copy( 19 );
            nDir = rManager.getDirectoryAtom( aDir, true );
            pDir = &m_aCache[ nDir ];
        }
        else if( aLine.CompareTo( "File:", 5 ) == COMPARE_EQUAL )
        {
            OString aFile( aLine.Copy( 5 ) );
            aStream.ReadLine( aLine );

            nIndex = 0;
            fonttype::type eType = (fonttype::type)aLine.GetToken( 0, ';', nIndex ).ToInt32();
            if( eType != fonttype::TrueType     &&
                eType != fonttype::Type1        &&
                eType != fonttype::Builtin
                )
                continue;
            if( nIndex == STRING_NOTFOUND )
                continue;

            sal_Int64 nTime = aLine.GetToken( 0, ';', nIndex ).ToInt64();
            if( nIndex == STRING_NOTFOUND )
                continue;

            sal_Int32 nFonts = aLine.GetToken( 0, ';', nIndex ).ToInt32();

            // check whether file is outdated
            struct stat aStat;
            ByteString aFileName( aDir );
            aFileName.Append( '/' );
            aFileName.Append( ByteString( aFile ) );
            if( stat( aFileName.GetBuffer(), &aStat )
                || nTime < (sal_Int64)aStat.st_mtime
                || ! S_ISREG(aStat.st_mode)
                )
                continue;

            for( int n = 0; n < nFonts; n++ )
            {
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
                }
                aStream.ReadLine( aLine );
                nIndex = 0;

                pFont->m_nFamilyName = pAtoms->getAtom( ATOM_FAMILYNAME,
                                                        String( aLine.GetToken( 0, ';', nIndex ), RTL_TEXTENCODING_UTF8 ),
                                                        sal_True );
                while( nIndex != STRING_NOTFOUND )
                {
                    String aAlias( aLine.GetToken( 0, ';', nIndex ), RTL_TEXTENCODING_UTF8 );
                    if( aAlias.Len() )
                        pFont->m_aAliases.push_back( pAtoms->getAtom( ATOM_FAMILYNAME, aAlias, sal_True ) );
                }

#define CHECKINDEX() if( nIndex == STRING_NOTFOUND ) { delete pFont; continue; }
                aStream.ReadLine( aLine );
                nIndex = 0;

                int nCollEntry = aLine.GetToken( 0, ';', nIndex ).ToInt32();
                CHECKINDEX();
                pFont->m_nPSName = pAtoms->getAtom( ATOM_PSNAME, String( aLine.GetToken( 0, ';', nIndex ), RTL_TEXTENCODING_UTF8 ), sal_True );
                CHECKINDEX();
                pFont->m_eItalic = (italic::type)aLine.GetToken( 0, ';', nIndex ).ToInt32();
                CHECKINDEX();
                pFont->m_eWeight = (weight::type)aLine.GetToken( 0, ';', nIndex ).ToInt32();
                CHECKINDEX();
                pFont->m_eWidth = (width::type)aLine.GetToken( 0, ';', nIndex ).ToInt32();
                CHECKINDEX();
                pFont->m_ePitch = (pitch::type)aLine.GetToken( 0, ';', nIndex ).ToInt32();
                CHECKINDEX();
                pFont->m_aEncoding = (rtl_TextEncoding)aLine.GetToken( 0, ';', nIndex ).ToInt32();
                CHECKINDEX();
                pFont->m_nAscend = aLine.GetToken( 0, ';', nIndex ).ToInt32();
                CHECKINDEX();
                pFont->m_nDescend = aLine.GetToken( 0, ';', nIndex ).ToInt32();
                CHECKINDEX();
                pFont->m_nLeading = aLine.GetToken( 0, ';', nIndex ).ToInt32();
                CHECKINDEX();
                pFont->m_bHaveVerticalSubstitutedGlyphs = aLine.GetToken( 0, ';', nIndex ).CompareIgnoreCaseToAscii( "true" ) == COMPARE_EQUAL ? true : false;
                CHECKINDEX();
                pFont->m_aGlobalMetricX.width = aLine.GetToken( 0, ';', nIndex ).ToInt32();
                CHECKINDEX();
                pFont->m_aGlobalMetricX.height = aLine.GetToken( 0, ';', nIndex ).ToInt32();
                CHECKINDEX();
                pFont->m_aGlobalMetricY.width = aLine.GetToken( 0, ';', nIndex ).ToInt32();
                CHECKINDEX();
                pFont->m_aGlobalMetricY.height = aLine.GetToken( 0, ';', nIndex ).ToInt32();

                switch( eType )
                {
                    case fonttype::TrueType:
                        CHECKINDEX();
                        static_cast<PrintFontManager::TrueTypeFontFile*>(pFont)->m_nTypeFlags = aLine.GetToken( 0, ';', nIndex ).ToInt32();
                        static_cast<PrintFontManager::TrueTypeFontFile*>(pFont)->m_nCollectionEntry = nCollEntry;
                        static_cast<PrintFontManager::TrueTypeFontFile*>(pFont)->m_nDirectory = nDir;
                        static_cast<PrintFontManager::TrueTypeFontFile*>(pFont)->m_aFontFile = aFile;
                        break;
                    case fonttype::Type1:
                    {
                        CHECKINDEX();
                        ByteString aMetricFile( aLine.GetToken( 0, ';', nIndex ) );
                        static_cast<PrintFontManager::Type1FontFile*>(pFont)->m_aMetricFile =  aMetricFile;
                        static_cast<PrintFontManager::Type1FontFile*>(pFont)->m_nDirectory = nDir;
                        static_cast<PrintFontManager::Type1FontFile*>(pFont)->m_aFontFile = aFile;
                        aFileName = aDir;
                        aFileName.Append( '/' );
                        aFileName.Append( aMetricFile );
                        if( stat( aFileName.GetBuffer(), &aStat ) ||
                            ! S_ISREG( aStat.st_mode )
                            )
                        {
                            delete pFont;
                            continue;
                        }
                    }
                    break;
                    case fonttype::Builtin:
                        static_cast<PrintFontManager::BuiltinFont*>(pFont)->m_nDirectory = nDir;
                        static_cast<PrintFontManager::BuiltinFont*>(pFont)->m_aMetricFile = aFile;
                        break;
                }
                (*pDir)[ aFile ].push_back( pFont );
            }
        }
    } while( ! aStream.IsEof() );
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
    }
    pTo->m_nFamilyName      = pFrom->m_nFamilyName;
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
bool FontCache::getFontCacheFile( int nDirID, const OString& rDir, const OString& rFile, list< PrintFontManager::PrintFont* >& rNewFonts )
{
    bool bSuccess = false;

    FontCacheData::const_iterator dir = m_aCache.find( nDirID );
    if( dir == m_aCache.end() )
        read( rDir );
    dir = m_aCache.find( nDirID );
    if( dir != m_aCache.end() )
    {
        FontDirMap::const_iterator entry = dir->second.find( rFile );
        if( entry != dir->second.end() )
        {
            bSuccess = true;
            for( list< PrintFontManager::PrintFont* >::const_iterator font = entry->second.begin(); font != entry->second.end(); ++font )
            {
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
        entry = dir->second.find( aFile );
        if( entry != dir->second.end() )
        {
            for( font = entry->second.begin(); font != entry->second.end(); ++font )
            {
                if( (*font)->m_eType == pFont->m_eType &&
                    ( (*font)->m_eType != fonttype::TrueType ||
                      static_cast<const PrintFontManager::TrueTypeFontFile*>(*font)->m_nCollectionEntry == static_cast<const PrintFontManager::TrueTypeFontFile*>(pFont)->m_nCollectionEntry
                      ) )
                    break;
            }
            if( font != entry->second.end() )
                pCacheFont = *font;
        }
    }

    if( pCacheFont )
        copyPrintFont( pFont, pCacheFont );
    else
    {
        pCacheFont = clonePrintFont( pFont );
        m_aCache[nDirID][aFile].push_back( pCacheFont );
    }
    m_bDoFlush = true;
    if( bFlush )
        flush();
}
