/*************************************************************************
 *
 *  $RCSfile: fontcache.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-11 17:17:21 $
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

#ifndef _PSPRINT_FONTCACHE_HXX
#define _PSPRINT_FONTCACHE_HXX

#ifndef _PSPRINT_FONTMANAGER_HXX_
#include <psprint/fontmanager.hxx>
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
        sal_Int64           m_nTimestamp;
        FontCacheEntry      m_aEntry;
    };

    typedef std::hash_map< ::rtl::OString, FontFile, ::rtl::OStringHash > FontDirMap;
    struct FontDir
    {
        sal_Int64   m_nTimestamp;
        bool        m_bNoFiles;
        FontDirMap  m_aEntries;

        FontDir() : m_nTimestamp(0), m_bNoFiles(false) {}
    };

    typedef std::hash_map< int, FontDir > FontCacheData;
    FontCacheData   m_aCache;
    bool            m_bDoFlush;

    void read( const ::rtl::OString& rPath );
    void clearCache();

    void copyPrintFont( const PrintFontManager::PrintFont* pFrom, PrintFontManager::PrintFont* pTo ) const;
    PrintFontManager::PrintFont* clonePrintFont( const PrintFontManager::PrintFont* pFont ) const;

    void createCacheDir( int nDirID );
public:
    FontCache();
    ~FontCache();

    bool getFontCacheFile( int nDirID, const rtl::OString& rDir, const rtl::OString& rFile, std::list< PrintFontManager::PrintFont* >& rNewFonts ) const;
    void updateFontCacheEntry( const PrintFontManager::PrintFont*, bool bFlush );
    void markEmptyDir( int nDirID, bool bNoFiles = true );

    // returns false for non cached directory
    // a cached but empty directory will return true but not append anything
    bool listDirectory( const rtl::OString& rDir, std::list< PrintFontManager::PrintFont* >& rNewFonts ) const;

    void flush();

    void updateDirTimestamp( int nDirID );
};

} // namespace psp

#endif // _PSPRINT_FONTCACHE_HXX
