/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgassim.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:01:43 $
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

#ifndef _DATETIMEITEM_HXX //autogen
#include <svtools/dateitem.hxx>
#endif

#ifndef _DRAWDOC_HXX
#include "drawdoc.hxx"
#endif

#ifndef _SVDOTEXT_HXX // class SdrTextObject
#include <svx/svdotext.hxx>
#endif

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#include "sdresid.hxx"

class SdPageListControl : public SvTreeListBox
{
private:
    SvLBoxButtonData*   m_pCheckButton;

    SvLBoxEntry* InsertPage( const String& rPageName );
    void InsertTitle( SvLBoxEntry* pEntry, const String& rTitle );

public:
    SdPageListControl( Window* pParent, const ResId& rResId );
    ~SdPageListControl();

    void Fill( SdDrawDocument* pDoc );
    void Clear();

    USHORT GetSelectedPage();
    BOOL IsPageChecked( USHORT nPage );

    DECL_LINK( CheckButtonClickHdl, SvLBoxButtonData * );

    virtual void DataChanged( const DataChangedEvent& rDCEvt );

};

class TemplateCacheInfo
{
private:
    DateTime    m_aDateTime;
    String      m_aFile;
    BOOL        m_bImpress;
    BOOL        m_bValid;
    BOOL        m_bModified;

public:
    TemplateCacheInfo();
    TemplateCacheInfo( const String& rFile, const DateTime& rDateTime, BOOL bImpress );

    BOOL IsValid() const { return m_bValid; }
    void SetValid( BOOL bValid = TRUE ) { m_bValid = bValid; }

    BOOL IsImpress() const { return m_bImpress; }
    void SetImpress( BOOL bImpress = TRUE ) { m_bImpress = bImpress; }

    const String& GetFile() const { return m_aFile; }
    void SetFile( const String& rFile ) { m_aFile = rFile; }

    const DateTime& GetDateTime() const { return m_aDateTime; }
    void SetDateTime( const DateTime& rDateTime ) { m_aDateTime = rDateTime; }

    BOOL IsModified() const { return m_bModified; }
    void SetModified( BOOL bModified = TRUE ) { m_bModified = bModified; }

    friend SvStream& operator >> (SvStream& rIn, TemplateCacheInfo& rInfo);
    friend SvStream& operator << (SvStream& rOut, const TemplateCacheInfo& rInfo);
};

DECLARE_LIST( TemplateCacheInfoList, TemplateCacheInfo * )

class TemplateCacheDirEntry
{
public:
    String                      m_aPath;
    TemplateCacheInfoList       m_aFiles;
};

DECLARE_LIST( TemplateCacheDirEntryList, TemplateCacheDirEntry * )

class TemplateCache
{
private:
    TemplateCacheDirEntryList   m_aDirs;
    TemplateCacheDirEntry* GetDirEntry( const String& rPath );
    void Clear();
public:
    TemplateCache();
    ~TemplateCache();

    void Load();
    void Save();

    TemplateCacheInfo* GetFileInfo( const String& rPath );
    TemplateCacheInfo* AddFileInfo( const String& rPath );

    BOOL ClearInvalidEntrys();
};
