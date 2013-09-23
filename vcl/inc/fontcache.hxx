/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _PSPRINT_FONTCACHE_HXX
#define _PSPRINT_FONTCACHE_HXX

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>
#include <vcl/fontmanager.hxx>
#include <boost/unordered_map.hpp>

namespace psp
{

class VCL_PLUGIN_PUBLIC FontCache
{
    struct FontDir;
    friend struct FontDir;
    struct FontFile;
    friend struct FontFile;

    typedef std::list< PrintFontManager::PrintFont* > FontCacheEntry;
    struct FontFile
    {
        FontCacheEntry      m_aEntry;
    };

    typedef boost::unordered_map< OString, FontFile, OStringHash > FontDirMap;
    struct FontDir
    {
        sal_Int64   m_nTimestamp;
        bool        m_bNoFiles;
        bool        m_bUserOverrideOnly;
        FontDirMap  m_aEntries;

        FontDir() : m_nTimestamp(0), m_bNoFiles(false), m_bUserOverrideOnly( false ) {}
    };

    typedef boost::unordered_map< int, FontDir > FontCacheData;
    FontCacheData   m_aCache;
    OUString        m_aCacheFile;
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

    bool getFontCacheFile( int nDirID, const OString& rFile, std::list< PrintFontManager::PrintFont* >& rNewFonts ) const;
    void updateFontCacheEntry( const PrintFontManager::PrintFont*, bool bFlush );
    void markEmptyDir( int nDirID, bool bNoFiles = true );

    // returns false for non cached directory
    // a cached but empty directory will return true but not append anything
    bool listDirectory( const OString& rDir, std::list< PrintFontManager::PrintFont* >& rNewFonts ) const;
    // returns true for directoris that contain only user overridden fonts
    bool scanAdditionalFiles( const OString& rDir );

    void flush();
};

} // namespace psp

#endif // _PSPRINT_FONTCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
