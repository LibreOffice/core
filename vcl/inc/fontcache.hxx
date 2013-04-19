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



#ifndef _PSPRINT_FONTCACHE_HXX
#define _PSPRINT_FONTCACHE_HXX

#include "vcl/dllapi.h"
#include "vcl/fontmanager.hxx"

#include "tools/string.hxx"

#include <hash_map>

namespace psp
{

class VCL_PLUGIN_PUBLIC FontCache
{
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

