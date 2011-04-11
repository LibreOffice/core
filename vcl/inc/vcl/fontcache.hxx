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

#ifndef _PSPRINT_FONTCACHE_HXX
#define _PSPRINT_FONTCACHE_HXX

#include "vcl/dllapi.h"
#include "vcl/fontmanager.hxx"

#include "tools/string.hxx"

#include <boost/unordered_map.hpp>

namespace psp
{

class VCL_PLUGIN_PUBLIC FontCache
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

    typedef boost::unordered_map< ::rtl::OString, FontFile, ::rtl::OStringHash > FontDirMap;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
