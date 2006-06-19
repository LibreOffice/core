/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fontcache.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:22:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _PSPRINT_FONTCACHE_HXX
#define _PSPRINT_FONTCACHE_HXX

#ifndef _PSPRINT_FONTMANAGER_HXX_
#include <psprint/fontmanager.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#include <hash_map>

namespace psp
{

class FontCache
{
    struct FontDir;
    friend class FontDir;
    struct FontFile;
    friend class FontFile;

    typedef std::list< PrintFontManager::PrintFont* > FontCacheEntry;
    struct FontFile
    {
        FontCacheEntry      m_aEntry;
    };

    typedef std::hash_map< ::rtl::OString, FontFile, ::rtl::OStringHash > FontDirMap;
    struct FontDir
    {
        sal_Int64   m_nTimestamp;
        bool        m_bNoFiles;
        bool        m_bUserOverrideOnly;
        FontDirMap  m_aEntries;

        FontDir() : m_nTimestamp(0), m_bNoFiles(false), m_bUserOverrideOnly( false ) {}
    };

    typedef std::hash_map< int, FontDir > FontCacheData;
    FontCacheData   m_aCache;
    String          m_aCacheFile;
    bool            m_bDoFlush;

    void read();
    void clearCache();

    void copyPrintFont( const PrintFontManager::PrintFont* pFrom, PrintFontManager::PrintFont* pTo ) const;
    bool equalsPrintFont( const PrintFontManager::PrintFont* pLeft, PrintFontManager::PrintFont* pRight ) const;
    PrintFontManager::PrintFont* clonePrintFont( const PrintFontManager::PrintFont* pFont ) const;

    void createCacheDir( int nDirID );
public:
    FontCache();
    ~FontCache();

    bool getFontCacheFile( int nDirID, const rtl::OString& rFile, std::list< PrintFontManager::PrintFont* >& rNewFonts ) const;
    void updateFontCacheEntry( const PrintFontManager::PrintFont*, bool bFlush );
    void markEmptyDir( int nDirID, bool bNoFiles = true );

    // returns false for non cached directory
    // a cached but empty directory will return true but not append anything
    bool listDirectory( const rtl::OString& rDir, std::list< PrintFontManager::PrintFont* >& rNewFonts ) const;
    // returns true for directoris that contain only user overridden fonts
    bool scanAdditionalFiles( const rtl::OString& rDir );

    void flush();

    void updateDirTimestamp( int nDirID );
};

} // namespace psp

#endif // _PSPRINT_FONTCACHE_HXX
