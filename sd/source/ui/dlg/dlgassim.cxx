/*************************************************************************
 *
 *  $RCSfile: dlgassim.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:32 $
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

#ifndef _SFX_INIMGR_HXX
#include <sfx2/inimgr.hxx>
#endif

#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif

#include "sdpage.hxx"
#include "sdoutl.hxx"
#include "res_bmp.hrc"

#include "dlgassim.hxx"

SdPageListControl::SdPageListControl( Window* pParent, const ResId& rResId ) :
            SvTreeListBox(pParent, rResId)
{
    // Tree-ListBox mit Linien versehen
    SetWindowBits( WinBits( WB_TABSTOP | WB_BORDER | WB_HASLINES |
                            WB_HASBUTTONS |  WB_HASLINESATROOT |
                            WB_HSCROLL | // #31562#
                            WB_HASBUTTONSATROOT ) );
    SetNodeBitmaps( Bitmap( SdResId( BMP_EXPAND ) ),
                    Bitmap( SdResId( BMP_COLLAPSE ) ) );

    m_pCheckButton = new SvLBoxButtonData();
    m_pCheckButton->aBmps[SV_BMP_UNCHECKED]   = Bitmap( SdResId( BMP_PAGE_OFF ) );
    m_pCheckButton->aBmps[SV_BMP_CHECKED]     = Bitmap( SdResId( BMP_PAGE_ON ) );
    m_pCheckButton->aBmps[SV_BMP_HICHECKED]   = Bitmap( SdResId( BMP_PAGE_ON ) );
    m_pCheckButton->aBmps[SV_BMP_HIUNCHECKED] = Bitmap( SdResId( BMP_PAGE_OFF ) );
    m_pCheckButton->aBmps[SV_BMP_TRISTATE]    = Bitmap( SdResId( BMP_PAGE_OFF ) );
    m_pCheckButton->aBmps[SV_BMP_HITRISTATE]  = Bitmap( SdResId( BMP_PAGE_OFF ) );
    EnableCheckButton( m_pCheckButton );

    SetCheckButtonHdl( LINK(this,SdPageListControl,CheckButtonClickHdl) );
    // Drag&Drop im Navigator zulassen
    // EnableDrop( TRUE );
}

IMPL_LINK( SdPageListControl, CheckButtonClickHdl, SvLBoxButtonData *, EMPTYARG )
{
    SvLBoxTreeList* pModel = GetModel();
    SvLBoxEntry* pEntry = pModel->First();

    while( pEntry )
    {
        if(pModel->IsAtRootDepth(pEntry) && GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED )
            return 0;
        pEntry = pModel->Next( pEntry );
    }

    pEntry = pModel->First();
    SetCheckButtonState( pEntry, SV_BUTTON_CHECKED );

    return 0;
}

SdPageListControl::~SdPageListControl()
{
    delete m_pCheckButton;
}

void SdPageListControl::Clear()
{
    SvTreeListBox::Clear();
}

SvLBoxEntry* SdPageListControl::InsertPage( const String& rPageName )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;

    pEntry->AddItem( new SvLBoxButton( pEntry, 0, m_pCheckButton ) );
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // Sonst Puff!
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rPageName ) );

    GetModel()->Insert( pEntry );

    return pEntry;
}

void SdPageListControl::InsertTitle( SvLBoxEntry* pParent, const String& rTitle )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;
    pEntry->AddItem( new SvLBoxString( pEntry, 0, String() ) );
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // Sonst Puff!
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rTitle ) );
    GetModel()->Insert( pEntry,pParent );
}

void SdPageListControl::Fill( SdDrawDocument* pDoc )
{
    Outliner* pOutliner = pDoc->GetInternalOutliner();

    USHORT nPage = 0;
    const USHORT nMaxPages = pDoc->GetPageCount();
    while( nPage < nMaxPages )
    {
        SdPage* pPage = (SdPage*) pDoc->GetPage( nPage );
        if( pPage->GetPageKind() == PK_STANDARD )
        {
            SvLBoxEntry* pEntry = InsertPage( pPage->GetName() );
            SetCheckButtonState(pEntry, SvButtonState( SV_BUTTON_CHECKED ) );

            SdrTextObj* pTO = (SdrTextObj*)pPage->GetPresObj(PRESOBJ_TEXT);
            if(!pTO)
            {
                // Ermittelt das SdrTextObject mit dem Layout Text dieser Seite
                const ULONG nObjectCount = pPage->GetObjCount();
                for (ULONG nObject = 0; nObject < nObjectCount; nObject++)
                {
                    SdrObject* pObject = pPage->GetObj(nObject);
                    if (pObject->GetObjInventor() == SdrInventor && pObject->GetObjIdentifier() == OBJ_OUTLINETEXT)
                    {
                        pTO = (SdrTextObj*)pObject;
                        break;
                    }
                }
            }

            if (pTO && !pTO->IsEmptyPresObj())
            {
                OutlinerParaObject* pOPO = pTO->GetOutlinerParaObject();
                if (pOPO)
                {
                    pOutliner->Clear();
                    pOutliner->SetText( *pOPO );

                    ULONG nCount = pOutliner->GetParagraphCount();

                    Paragraph* pPara = NULL;

                    for (ULONG nPara = 0; nPara < nCount; nPara++)
                    {
                        pPara = pOutliner->GetParagraph(nPara);
                        if(pPara && pOutliner->GetDepth( nPara ) == 1 )
                        {
                            String aParaText = pOutliner->GetText(pPara);
                            if(aParaText.Len() != 0)
                                InsertTitle( pEntry, aParaText );
                        }
                    }
                }
            }
        }
        nPage++;
    }

    pOutliner->Clear();
}

USHORT SdPageListControl::GetSelectedPage()
{
    SvLBoxEntry* pSelEntry = GetCurEntry();
    USHORT nPage = 0;

    if ( pSelEntry )
    {
        SvLBoxTreeList* pModel = GetModel();
        SvLBoxEntry* pEntry = pModel->First();

        while( pEntry && pEntry != pSelEntry )
        {
            if(pModel->IsAtRootDepth(pEntry))
                nPage++;
            pEntry = pModel->Next( pEntry );
        }

        if(!pModel->IsAtRootDepth(pSelEntry))
            nPage--;
    }
    return nPage;
}

BOOL SdPageListControl::IsPageChecked( USHORT nPage )
{
    SvLBoxEntry* pEntry = GetModel()->GetEntry(nPage);
    return pEntry?GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED: FALSE;
}

// ====================================================================
// TemplateCache
// ====================================================================

// Kennung fuer die Config Datei mit den Cache Daten
static const UINT16 nMagic = (UINT16)0x4127;

TemplateCacheInfo::TemplateCacheInfo()
{
    m_bImpress = FALSE;
    m_bValid   = FALSE;
    m_bModified = TRUE;
}

TemplateCacheInfo::TemplateCacheInfo( const String& rFile, const DateTime& rDateTime, BOOL bImpress )
{
    m_aFile     = rFile;
    m_aDateTime = rDateTime;
    m_bImpress  = bImpress;
    m_bValid    = FALSE;
    m_bModified = TRUE;
}

SvStream& operator >> (SvStream& rIn, TemplateCacheInfo& rInfo)
{
    rIn.ReadByteString( rInfo.m_aFile, RTL_TEXTENCODING_UTF8 );
    USHORT nDay, nMonth, nYear;
    rIn >> nDay;
    rIn >> nMonth;
    rIn >> nYear;
    rInfo.m_aDateTime.SetDay( nDay );
    rInfo.m_aDateTime.SetMonth( nMonth );
    rInfo.m_aDateTime.SetYear( nYear);

    USHORT nHour, nMin, nSec, n100Sec;

    rIn >> nHour;
    rIn >> nMin;
    rIn >> nSec;
    rIn >> n100Sec;

    rInfo.m_aDateTime.SetHour( nHour );
    rInfo.m_aDateTime.SetMin( nMin );
    rInfo.m_aDateTime.SetSec( nSec );
    rInfo.m_aDateTime.Set100Sec( n100Sec );

    rIn >> rInfo.m_bImpress;

    rInfo.m_bModified = FALSE;

    return rIn;
}

SvStream& operator << (SvStream& rOut, const TemplateCacheInfo& rInfo)
{
    USHORT nTemp;

    rOut.WriteByteString( rInfo.m_aFile, RTL_TEXTENCODING_UTF8 );

    nTemp = rInfo.m_aDateTime.GetDay();
    rOut << nTemp;

    nTemp = rInfo.m_aDateTime.GetMonth();
    rOut << nTemp;

    nTemp = rInfo.m_aDateTime.GetYear();
    rOut << nTemp;

    nTemp = rInfo.m_aDateTime.GetHour();
    rOut << nTemp;

    nTemp = rInfo.m_aDateTime.GetMin();
    rOut << nTemp;

    nTemp = rInfo.m_aDateTime.GetSec();
    rOut << nTemp;

    nTemp = rInfo.m_aDateTime.Get100Sec();
    rOut << nTemp;

    rOut << rInfo.m_bImpress;

    return rOut;
}

TemplateCache::TemplateCache()
{
}

TemplateCache::~TemplateCache()
{
    Clear();
}

void TemplateCache::Clear()
{
    for( TemplateCacheDirEntry* pDir = m_aDirs.First(); pDir; pDir = m_aDirs.Next() )
    {
        for( TemplateCacheInfo* pEntry = pDir->m_aFiles.First(); pEntry; pEntry = pDir->m_aFiles.Next() )
            delete pEntry;
        delete pDir;
    }
}

void TemplateCache::Load()
{
    INetURLObject aURL;

    aURL.SetSmartURL( SFX_APP()->GetAppIniManager()->Get( SFX_KEY_USERCONFIG_PATH ) );
    aURL.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "template.sod" ) ) );

    SfxMedium aMedium( aURL.GetMainURL(),
                    STREAM_READ | STREAM_NOCREATE,
                    TRUE,                 // direkt
                    TRUE );               // Download

    SvStream* pStream = aMedium.GetInStream();
    if( !pStream )
        return;

    UINT16 aCheck;
    *pStream >> aCheck;

    if(aCheck != nMagic)
        return;

    UINT16 nDirs;
    *pStream >> nDirs;

    for( UINT16 nDir = 0; pStream->GetError() == SVSTREAM_OK && nDir < nDirs; nDir++ )
    {
        TemplateCacheDirEntry* pDir = new TemplateCacheDirEntry();
        m_aDirs.Insert(pDir);

        pStream->ReadByteString( pDir->m_aPath, RTL_TEXTENCODING_UTF8 );

        UINT16 nFiles;
        *pStream >> nFiles;

        for( UINT16 nFile = 0; pStream->GetError() == SVSTREAM_OK && nFile < nFiles; nFile++ )
        {
            TemplateCacheInfo* pEntry = new TemplateCacheInfo();
            *pStream >> *pEntry;
            pDir->m_aFiles.Insert(pEntry);
        }
    }

    if( pStream->GetError() != SVSTREAM_OK )
    {
        // Ein I/O Problem ist doch kein Beinbruch, dann wird der Cache halt neu erzeugt
        Clear();
    }
}

void TemplateCache::Save()
{
    INetURLObject aURL;

    aURL.SetSmartURL( SFX_APP()->GetAppIniManager()->Get( SFX_KEY_USERCONFIG_PATH ) );
    aURL.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "template.sod" ) ) );

    SfxMedium aMedium( aURL.GetMainURL(),
                    STREAM_WRITE | STREAM_TRUNC,
                    TRUE,                 // direkt
                    FALSE );              // Download

    SvStream* pStream = aMedium.GetInStream();
    if( !pStream )
        return;

    UINT16 nCheck = nMagic;
    *pStream << nCheck;

    const UINT16 nDirs = m_aDirs.Count();
    *pStream << nDirs;

    for( TemplateCacheDirEntry* pDir = m_aDirs.First();
         pStream->GetError() == SVSTREAM_OK && pDir;
         pDir = m_aDirs.Next() )
    {
        pStream->WriteByteString( pDir->m_aPath, RTL_TEXTENCODING_UTF8 );

        const UINT16 nFiles = pDir->m_aFiles.Count();
        *pStream << nFiles;

        for( TemplateCacheInfo* pEntry = pDir->m_aFiles.First();
             pStream->GetError() == SVSTREAM_OK && pEntry;
             pEntry = pDir->m_aFiles.Next() )
        {
            *pStream << *pEntry;
        }
    }
}

TemplateCacheDirEntry* TemplateCache::GetDirEntry( const String& rPath )
{
    TemplateCacheDirEntry* pDir = m_aDirs.GetCurObject();
    if( pDir && pDir->m_aPath == rPath )
        return pDir;

    for( pDir = m_aDirs.First(); pDir; pDir = m_aDirs.Next() )
    {
        if( pDir->m_aPath == rPath )
            return pDir;
    }

    return NULL;
}

TemplateCacheInfo* TemplateCache::GetFileInfo( const String& rPath )
{
    INetURLObject   aUrl; aUrl.SetSmartURL( rPath );
    String          aPath( aUrl.GetPath() );
    String          aName( aUrl.GetName( INetURLObject::DECODE_UNAMBIGUOUS ) );

    TemplateCacheDirEntry* pDir = GetDirEntry( aPath );
    if( NULL != pDir )
    {
        for( TemplateCacheInfo* pEntry = pDir->m_aFiles.First(); pEntry; pEntry = pDir->m_aFiles.Next() )
        {
            if( pEntry->GetFile() == aName )
                return pEntry;
        }
    }

    return NULL;
}

TemplateCacheInfo* TemplateCache::AddFileInfo( const String& rPath )
{
    INetURLObject aUrl; aUrl.SetSmartURL( rPath );
    String aPath( aUrl.GetPath() );
    String aName( aUrl.GetName( INetURLObject::DECODE_UNAMBIGUOUS ) );

    TemplateCacheDirEntry* pDir = GetDirEntry( aPath );
    TemplateCacheInfo* pEntry  = NULL;

    if( pDir == NULL )
    {
        pDir = new TemplateCacheDirEntry();
        pDir->m_aPath = aPath;
        m_aDirs.Insert(pDir);
    }

    if( NULL != pDir )
    {
        for( pEntry = pDir->m_aFiles.First(); pEntry; pEntry = pDir->m_aFiles.Next() )
        {
            if( pEntry->GetFile() == aName)
                return pEntry;
        }

        pEntry = new TemplateCacheInfo();
        pEntry->SetFile( aName );
        pDir->m_aFiles.Insert(pEntry);
    }

    return pEntry;
}

BOOL TemplateCache::ClearInvalidEntrys()
{
    BOOL bModified = FALSE;

    for( TemplateCacheDirEntry* pDir = m_aDirs.First(); pDir; pDir = m_aDirs.Next() )
    {
        for( TemplateCacheInfo* pEntry = pDir->m_aFiles.First(); pEntry; pEntry = pDir->m_aFiles.Next() )
        {
            if(!pEntry->IsValid())
            {
                pDir->m_aFiles.Remove(pEntry);
                delete pEntry;
                bModified = TRUE;
            }
            else if( pEntry->IsModified() )
            {
                bModified = TRUE;
            }
        }

        if( pDir->m_aFiles.Count() == 0 )
        {
            m_aDirs.Remove(pDir);
            delete pDir;
        }
    }

    return bModified;
}


