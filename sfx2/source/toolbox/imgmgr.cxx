/*************************************************************************
 *
 *  $RCSfile: imgmgr.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:38:44 $
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

#include <stdio.h>

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#include <tools/link.hxx>
#include <tools/list.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/miscopt.hxx>
#include <framework/imagesconfiguration.hxx>

#pragma hdrstop

#include "imgmgr.hxx"
#include "sfx.hrc"
#include "app.hxx"
#include "sfxresid.hxx"
#include "tbxmgr.hxx"
#include "bindings.hxx"
#include "statcach.hxx"
#include "module.hxx"
#include "sfxtypes.hxx"
#include "viewfrm.hxx"
#include "objsh.hxx"
#include "cfgmgr.hxx"
#include "macrconf.hxx"

static const USHORT nVersion = 5;
DECLARE_LIST( LinkList, Link * );

class SfxToolBoxArr_Impl;
struct SfxImageManagerData_Impl
{
    sal_Int16       nOutStyle;
    sal_Int16       nSet;
    SfxToolBoxArr_Impl* pToolBoxList;
    SfxObjectShell* pDoc;
};

class SfxBitmapList_Impl;
class SfxImageManager_Impl : public SfxConfigItem
{
public:
    LinkList            m_aList;
    SvtMiscOptions      m_aOpt;
    SfxBitmapList_Impl* m_pUserDefList;
    ImageList*          m_pUserImageList;
    ImageList*          m_pHCUserImageList;
    int                 m_nUserRef;

    void            MakeDefaultImageList( BOOL bHiContrast = FALSE );
    void            MakeUserList();
    void            RebuildUserList();
    Image           GetImage( USHORT nId, SfxModule*, BOOL bBig, BOOL bHiContrast ) const;
    Image           SeekImage(USHORT nId, SfxModule* pModule, BOOL bHiContrast ) const;
    Image           GetDefaultImage( USHORT nId, SfxModule*, BOOL bBig, BOOL bHiContrast ) const;

    virtual BOOL    ReInitialize();
    int             Load( SvStream& );
    BOOL            Store( SvStream& );
    virtual void    UseDefault();
    virtual String  GetStreamName() const;
    virtual int     Load(SotStorage&);
    virtual BOOL    Store(SotStorage&);
    void            AddLink( const Link& );
    void            RemoveLink( const Link& );
    void            LoadBitmap( Bitmap&, SotStorage&, const String& );

                    SfxImageManager_Impl( SfxConfigManager* pCfg );
                    ~SfxImageManager_Impl();
};

// elements common to all ImageManager instances
static ImageList* pImageListSmall=0;
static ImageList* pImageListBig=0;
static ImageList* pImageListHiSmall=0;
static ImageList* pImageListHiBig=0;
//static ImageList* pOffImageList;
//static ImageList* pOffHiImageList;
static ImageList* pImageList;
static ImageList* pHiImageList;
static SfxImageManager_Impl* pGlobalConfig=NULL;
static nRef=0;
static nGlobalRef=0;

// we need a static list of all pImp instances, so all operations that should change all instances can be performed
// which operations ?!

ImageList* GetImageList( BOOL bBig, BOOL bHiContrast = FALSE )
{
    // Has to be changed if we know how the IDs are named!!!
    ImageList*& rpList = bBig ? ( bHiContrast ? pImageListHiBig : pImageListBig ) :
                                ( bHiContrast ? pImageListHiSmall : pImageListSmall );
    if ( !rpList )
    {
        ResMgr *pResMgr = Resource::GetResManager();
        ResId aResId( bBig ? ( bHiContrast ? RID_DEFAULTIMAGELIST_LCH : RID_DEFAULTIMAGELIST_LC ) :
                             ( bHiContrast ? RID_DEFAULTIMAGELIST_SCH : RID_DEFAULTIMAGELIST_SC ));

        aResId.SetRT( RSC_IMAGELIST );

        DBG_ASSERT( pResMgr->IsAvailable(aResId), "No default ImageList!" );

        if ( pResMgr->IsAvailable(aResId) )
            rpList = new ImageList( aResId );
        else
            rpList = new ImageList();
    }

    return rpList;
}

//=========================================================================

sal_Int16 SfxImageManager::GetCurrentSymbolSet()
{
    sal_Int16   eOptSymbolSet = SvtMiscOptions().GetSymbolSet();

    if ( eOptSymbolSet == SFX_SYMBOLS_AUTO )
    {
        // Use system settings, we have to retrieve the toolbar icon size from the
        // Application class
        ULONG nStyleIconSize = Application::GetSettings().GetStyleSettings().GetToolbarIconSize();
        if ( nStyleIconSize == STYLE_TOOLBAR_ICONSIZE_LARGE )
            eOptSymbolSet = SFX_SYMBOLS_LARGE;
        else
            eOptSymbolSet = SFX_SYMBOLS_SMALL;
    }

    return eOptSymbolSet;
}

//=========================================================================

// Enth"alt eine Kombination aus Bitmap und zugeordneter SlotId
struct ToolboxBitmap_Impl
{
    USHORT  nId;
    Bitmap* pBitmap;

    ToolboxBitmap_Impl( USHORT n, const Bitmap& rBmp ) : nId( n )
    {
        pBitmap = new Bitmap( rBmp );
    }

    ~ToolboxBitmap_Impl() { delete pBitmap; }
};

// Enth"alt einen Ptr auf eine registrierte Toolbox und Flags f"ur die
// Ereignisse, bei denen die Toolbox upgedatet werden soll
struct ToolBoxInf_Impl
{
    SfxToolBoxManager*  pMgr;
    SfxModule*  pModule;
    ToolBox *pToolBox;
    USHORT  nFlags;
};

//-------------------------------------------------------------------------

DECL_PTRARRAY( SfxBitmapArr_Impl, ToolboxBitmap_Impl*, 4, 4 );
DECL_PTRARRAY( SfxToolBoxArr_Impl, ToolBoxInf_Impl*, 4, 4 );

//-------------------------------------------------------------------------

// Liste der geladenen Userdef-Bitmaps
class SfxBitmapList_Impl
{
    SfxBitmapArr_Impl*  pList;

public:
friend SvStream& operator >> (SvStream& rStream, SfxBitmapList_Impl& rList);
friend SvStream& operator << (SvStream& rStream, const SfxBitmapList_Impl& rList);

    SfxBitmapList_Impl()
            { pList = new SfxBitmapArr_Impl; }

    ~SfxBitmapList_Impl()
            {
                for (USHORT n=0; n<pList->Count(); n++)
                    delete (*pList)[n];
                delete pList;
            }

    USHORT  GetBitmapCount() const
            { return pList->Count(); }

    USHORT  GetBitmapId(USHORT n) const
            { return (*pList)[n]->nId; }

    USHORT  GetBitmapPos(USHORT nId) const;
    Bitmap* GetBitmap(USHORT nId) const;
    void    AddBitmap (USHORT nId, const Bitmap& rpBmp);
    void    ReplaceBitmap (USHORT nId, const Bitmap& rpBmp);
    void    RemoveBitmap (USHORT nId);
};

//=========================================================================

/*  [Beschreibung]

    Liefert die Position einer Bitmap in der internen Bitmapliste
*/

USHORT SfxBitmapList_Impl::GetBitmapPos( USHORT nId ) const
{
    USHORT nPos;
    for ( nPos=0; nPos<pList->Count(); nPos++ )
        if ( (*pList)[nPos]->nId == nId ) break;

    if ( nPos < pList->Count() )
        return nPos;
    else
        return USHRT_MAX;
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Liefert die Bitmap an der "uebergebenen Position der internen Bitmapliste
*/

Bitmap* SfxBitmapList_Impl::GetBitmap(USHORT nId) const
{
    USHORT nPos;
    for ( nPos=0; nPos<pList->Count(); nPos++ )
        if ( (*pList)[nPos]->nId == nId ) break;

    if ( nPos < pList->Count() )
        return (*pList)[nPos]->pBitmap;
    else
        return 0;
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    F"ugt der internen Bitmapliste eine Bitmap hinzu, die f"ur die "ubergebene
    SlotId registriert wird. Die Bitmap geht in den Besitz des ImageManagers
    "uber.
*/

void SfxBitmapList_Impl::AddBitmap( USHORT nId, const Bitmap& rBmp )
{
    for ( USHORT nPos=0; nPos<pList->Count(); nPos++ )
        if ( (*pList)[nPos]->nId == nId ) break;

    DBG_ASSERT( nPos>=pList->Count(), "Bitmap mit dieser Id schon vorhanden!" );

    ToolboxBitmap_Impl *pTbBmp = new ToolboxBitmap_Impl( nId, rBmp );
    pList->Append( pTbBmp );
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Ersetzt die Bitmap in der internen Bitmapliste, die f"ur die "ubergebene
    SlotId registriert ist, durch die "ubergebene Bitmap.
    Die Bitmap geht in den Besitz des ImageManagers "uber.
*/

void SfxBitmapList_Impl::ReplaceBitmap( USHORT nId, const Bitmap& rBmp )
{
    RemoveBitmap( nId );
    AddBitmap( nId, rBmp );
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Entfernt die Bitmap in der internen Bitmapliste, die f"ur die "ubergebene
    SlotId registriert ist.
    Da die Bitmap dem ImageManagers geh"ort, wird sie zerst"ort.
*/

void SfxBitmapList_Impl::RemoveBitmap( USHORT nId )
{
    USHORT nPos;
    for (nPos=0; nPos<pList->Count(); nPos++)
        if ((*pList)[nPos]->nId == nId) break;

    DBG_ASSERT(nPos<pList->Count(), "Bitmap mit dieser Id unbekannt!");

    if (nPos<pList->Count())
    {
        delete (*pList)[nPos];
        pList->Remove(nPos);
    }
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Liest die interne Bitmapliste aus einem Stream.
*/

SvStream& operator >> (SvStream& rStream, SfxBitmapList_Impl& rList)
{
    USHORT nCount, nId;
    rStream >> nCount;
    for (USHORT n=0; n<nCount; n++)
    {
        Bitmap aBmp;
        rStream >> nId >> aBmp;
        rList.AddBitmap(nId, aBmp);
    }

    return rStream;
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Speichert die interne Bitmapliste in einem Stream,
*/

SvStream& operator << (SvStream& rStream, const SfxBitmapList_Impl& rList)
{
    rStream << rList.GetBitmapCount();
    for (USHORT n=0; n<rList.GetBitmapCount(); n++)
        rStream << (*rList.pList)[n]->nId << *((*rList.pList)[n]->pBitmap);

    return rStream;
}

SfxImageManager_Impl::SfxImageManager_Impl( SfxConfigManager* pCfgMgr )
    : SfxConfigItem( SFX_ITEMTYPE_IMAGELIST, pCfgMgr )
    , m_pUserDefList( 0 )
    , m_pUserImageList( 0 )
    , m_pHCUserImageList( 0 )
{
//    SetInternal( TRUE );
    Initialize();
}

SfxImageManager_Impl::~SfxImageManager_Impl()
{
    delete m_pUserDefList;
    delete m_pUserImageList;
    delete m_pHCUserImageList;

    if ( this == pGlobalConfig )
        pGlobalConfig = NULL;
}

void SfxImageManager_Impl::AddLink( const Link& rLink )
{
    m_aList.Insert( new Link( rLink ) );
}

void SfxImageManager_Impl::RemoveLink( const Link& rLink )
{
    for ( USHORT n=0; n<m_aList.Count(); n++ )
    {
        if ( (*m_aList.GetObject(n) ) == rLink )
        {
            delete m_aList.Remove(n);
            break;
        }
    }
}

//-------------------------------------------------------------------------

void SfxImageManager_Impl::LoadBitmap( Bitmap& rBmp, SotStorage& rStorage, const String& rURL )
{
    // locate and open bitmap for ImageList
    SotStorageStreamRef xBitmapStream;
    SvStream* pStream = NULL;

    // bitmap may be internal (relative URL) or external (absolute URL)
    INetURLObject aObj( rURL );
    if ( aObj.GetProtocol() != INET_PROT_NOT_VALID )
    {
        // get external stream for bitmap
        pStream = ::utl::UcbStreamHelper::CreateStream( aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_STD_READ );
    }
    else
    {
        // find internal stream by parsing URL, starting in the root of rStorage
        SotStorageRef xBitmapStorage = &rStorage;

        // every string followed by a slash is a directory
        USHORT nTokenCount = rURL.GetTokenCount( '/' );

        // open directories
        for ( USHORT nToken=0; nToken<nTokenCount-1; nToken++ )
            xBitmapStorage = xBitmapStorage->OpenSotStorage( rURL.GetToken( nToken, '/' ), STREAM_STD_READ );

        // now get the stream
        xBitmapStream = xBitmapStorage->OpenSotStream( rURL.GetToken( nToken, '/' ), STREAM_STD_READ );
        pStream = xBitmapStream;
    }

    *pStream >> rBmp;
    if ( !xBitmapStream.Is() )
        DELETEZ( pStream );
}

int SfxImageManager_Impl::Load( SotStorage& rStorage )
{
    SotStorageStreamRef xStream =
            rStorage.OpenSotStream( GetStreamName(), STREAM_STD_READ );
    if ( xStream->GetError() )
        return ERR_READ;
    else
    {
        SfxMacroConfig* pCfg = SfxMacroConfig::GetOrCreate();
        ::framework::ImageListsDescriptor aDescriptor;
        if ( !::framework::ImagesConfiguration::LoadImages( *xStream, aDescriptor ) )
            return ERR_READ;

        if ( !aDescriptor.pImageList || !aDescriptor.pImageList->Count() )
            return ERR_READ;

        ::framework::ImageListItemDescriptor* pList = aDescriptor.pImageList->GetObject(0);
        USHORT nCount = pList->pImageItemList->Count();
        if ( !nCount )
            return ERR_READ;

        Bitmap aBmp;
        Bitmap aHCBmp;
        LoadBitmap( aBmp, rStorage, pList->aURL );

        if ( pList->aHighContrastURL.Len() == 0 )
        {
            // Use normal images for high contrast mode as we need the same number of images
            // for the high contrast image list!! The normal case for 6.1 should be that both
            // lists are filled, so this document was created by 6.0.
            // #98979#
            LoadBitmap( aHCBmp, rStorage, pList->aURL );
        }
        else
            LoadBitmap( aHCBmp, rStorage, pList->aHighContrastURL );

        // get the Ids of the ImageList
        USHORT* pIds = new USHORT[nCount];
        for ( USHORT i=0; i<nCount; i++ )
        {
            const ::framework::ImageItemDescriptor* pItem = pList->pImageItemList->GetObject(i);
            if ( pItem->aCommandURL.CompareToAscii("slot:",5) == COMPARE_EQUAL )
            {
                pIds[i] = (USHORT) pItem->aCommandURL.Copy( 5 ).ToInt32();
            }
            else if ( pItem->aCommandURL.Len() )
            //else if ( pItem->aCommandURL.CompareToAscii("macro:",6) == COMPARE_EQUAL )
            {
                SfxMacroInfo aInfo( pItem->aCommandURL );
                pCfg->GetSlotId( &aInfo );
                pIds[i] = aInfo.GetSlotId();
            }
        }

        delete m_pUserImageList;
        delete m_pHCUserImageList;

        if ( pList->nMaskMode = ::framework::ImageMaskMode_Color )
        {
            m_pUserImageList = new ImageList( aBmp, pList->aMaskColor, nCount, pIds );
            m_pHCUserImageList = new ImageList( aHCBmp, pList->aMaskColor, nCount, pIds );
        }
        else
        {
            Bitmap aMask;
            LoadBitmap( aMask, rStorage, pList->aMaskURL );
            m_pUserImageList = new ImageList( aBmp, aMask, nCount, pIds );

            if ( pList->aHighContrastMaskURL.Len() == 0 )
            {
                // Use normal mask images for high contrast mode as we need the same number of images
                // for the high contrast image list!! The normal case for 6.1 should be that both
                // lists are filled, so this document was created by 6.0.
                // #98979#
                LoadBitmap( aMask, rStorage, pList->aMaskURL );
            }
            else
                LoadBitmap( aMask, rStorage, pList->aHighContrastMaskURL );

            m_pHCUserImageList = new ImageList( aHCBmp, aMask, nCount, pIds );
        }

        DELETEZ( pIds );
        m_pUserDefList = new SfxBitmapList_Impl;

        nCount = aDescriptor.pExternalImageList ? aDescriptor.pExternalImageList->Count() : 0;
        for ( USHORT n=0; n<nCount; n++ )
        {
            ::framework::ExternalImageItemDescriptor* pItem = aDescriptor.pExternalImageList->GetObject(n);
            USHORT nId = 0;
            if ( pItem->aCommandURL.CompareToAscii("slot:",5) == COMPARE_EQUAL )
            {
                nId = (USHORT) pItem->aCommandURL.Copy( 5 ).ToInt32();
            }
            else if ( pItem->aCommandURL.Len() )
            //else if ( pItem->aCommandURL.CompareToAscii("macro:",6) == COMPARE_EQUAL )
            {
                SfxMacroInfo aInfo( pItem->aCommandURL );
                pCfg->GetSlotId( &aInfo );
                nId = aInfo.GetSlotId();
            }

            Bitmap aBmp;
            LoadBitmap( aBmp, rStorage, pItem->aURL );
            m_pUserDefList->AddBitmap( nId, aBmp );
        }
    }

    MakeDefaultImageList();
    SetDefault( FALSE );
    return ERR_OK;
}

BOOL SfxImageManager_Impl::Store( SotStorage& rStorage )
{
    SotStorageStreamRef xStream = rStorage.OpenSotStream( SfxImageManager_Impl::GetStreamName(), STREAM_STD_READWRITE|STREAM_TRUNC );
    if ( xStream->GetError() )
        return FALSE;
    else
    {
        // create a descriptor
        SfxMacroConfig* pCfg = SfxMacroConfig::GetOrCreate();
        ::framework::ImageListsDescriptor aDescriptor;
        aDescriptor.pImageList = new ::framework::ImageListDescriptor;

        // insert userimagelist as one and only list (more is not supported currently, default lists remain in resource)
        ::framework::ImageListItemDescriptor* pList = new ::framework::ImageListItemDescriptor;
        aDescriptor.pImageList->Insert( pList, 0 );

        // bitmaps are stored in an internal bitmap directory
        SotStorageRef xBitmapStorage = rStorage.OpenSotStorage( String::CreateFromAscii("Bitmaps"), STREAM_STD_READWRITE );

        if ( m_pUserImageList->HasMaskColor() )
        {
            // mask color
            pList->nMaskMode = ::framework::ImageMaskMode_Color;
            pList->aMaskColor = m_pUserImageList->GetMaskColor();
        }
        else
        {
            // masking is done by a mask bitmap, store bitmap and set URL
            pList->nMaskMode = ::framework::ImageMaskMode_Bitmap;

            pList->aMaskURL = String::CreateFromAscii("Bitmaps/");
            String aStreamName = String::CreateFromAscii("userimagesmask.bmp");
            pList->aMaskURL += aStreamName;

            // store bitmap
            SotStorageStreamRef xBitmapStream = xBitmapStorage->OpenSotStream( aStreamName, STREAM_STD_READWRITE | STREAM_TRUNC );
            *xBitmapStream << m_pUserImageList->GetMaskBitmap();

            pList->aHighContrastMaskURL = String::CreateFromAscii( "Bitmaps/" );
            aStreamName = String::CreateFromAscii( "hcuserimagesmask.bmp" );
            pList->aHighContrastMaskURL += aStreamName;

            // store bitmap
            xBitmapStream = xBitmapStorage->OpenSotStream( aStreamName, STREAM_STD_READWRITE | STREAM_TRUNC );
            *xBitmapStream << m_pHCUserImageList->GetMaskBitmap();
        }

        // a modified list always contains a userlist
        pList->pImageItemList = new ::framework::ImageItemListDescriptor;
        for ( USHORT i=0; i<m_pUserImageList->GetImageCount(); i++ )
        {
            ::framework::ImageItemDescriptor* pItem = new ::framework::ImageItemDescriptor;

            pItem->nIndex = i;
            USHORT nId = m_pUserImageList->GetImageId(i);
            if ( SfxMacroConfig::IsMacroSlot( nId ) )
            {
                const SfxMacroInfo* pInfo = pCfg->GetMacroInfo( nId );
                pItem->aCommandURL = pInfo->GetURL();
            }
            else if ( nId )
            {
                pItem->aCommandURL = String::CreateFromAscii("slot:");
                pItem->aCommandURL += String::CreateFromInt32( nId );
            }

            pList->pImageItemList->Insert( pItem, pList->pImageItemList->Count() );
        }

        // store URL of bitmap relative to configuration storage; name is "BitmapXXX.bmp", where XXX is an index
        pList->aURL = String::CreateFromAscii("Bitmaps/");
        String aStreamName = String::CreateFromAscii("userimages.bmp");
        pList->aURL += aStreamName;

        // store bitmap
        SotStorageStreamRef xBitmapStream = xBitmapStorage->OpenSotStream( aStreamName, STREAM_STD_READWRITE | STREAM_TRUNC );
        *xBitmapStream << m_pUserImageList->GetBitmap();

        // store high contrast URL of bitmap relative to configuration storage; name is "BitmapXXX.bmp", where XXX is an index
        pList->aHighContrastURL = String::CreateFromAscii("Bitmaps/");
        aStreamName = String::CreateFromAscii("hcuserimages.bmp");
        pList->aHighContrastURL += aStreamName;

        // store high contrast bitmap
        xBitmapStream = xBitmapStorage->OpenSotStream( aStreamName, STREAM_STD_READWRITE | STREAM_TRUNC );
        *xBitmapStream << m_pHCUserImageList->GetBitmap();

        // collect all external bitmaps
        USHORT nCount = m_pUserDefList->GetBitmapCount();
        if ( nCount )
        {
            aDescriptor.pExternalImageList = new ::framework::ExternalImageItemListDescriptor;
            for ( USHORT i=0; i<nCount; i++ )
            {
                ::framework::ExternalImageItemDescriptor* pItem = new ::framework::ExternalImageItemDescriptor;
                USHORT nId = m_pUserDefList->GetBitmapId(i);
                if ( SfxMacroConfig::IsMacroSlot( nId ) )
                {
                    const SfxMacroInfo* pInfo = pCfg->GetMacroInfo( nId );
                    pItem->aCommandURL = pInfo->GetURL();
                }
                else if ( nId )
                {
                    pItem->aCommandURL = String::CreateFromAscii("slot:");
                    pItem->aCommandURL += String::CreateFromInt32( nId );
                }

                // store URL of bitmap relative to configuration storage; name is "BitmapXXX.bmp", where XXX is an index
                pItem->aURL = String::CreateFromAscii("Bitmaps/");
                String aStreamName = String::CreateFromAscii("image");
                aStreamName += String::CreateFromInt32(i);
                aStreamName += String::CreateFromAscii(".bmp");
                pItem->aURL += aStreamName;

                aDescriptor.pExternalImageList->Insert( pItem, aDescriptor.pExternalImageList->Count() );

                // store bitmap
                SotStorageStreamRef xBitmapStream = xBitmapStorage->OpenSotStream( aStreamName, STREAM_STD_READWRITE | STREAM_TRUNC );
                *xBitmapStream << *m_pUserDefList->GetBitmap( nId );
            }
        }

        // store configuration
        xBitmapStorage->Commit();
        return ::framework::ImagesConfiguration::StoreImages( *xStream, aDescriptor );
    }
}


int SfxImageManager_Impl::Load(SvStream& rStream)
{
    USHORT  nFileVersion, nSymbolSet;
    ULONG   nColorCount = Application::GetDefaultDevice()->GetColorCount();

    rStream >> nFileVersion;
    if ( nFileVersion < nVersion)
        return SfxConfigItem::WARNING_VERSION;

    rStream >> nSymbolSet       // ignore !
            >> nColorCount;     // evaluate for possible rebuild of userlist

    MakeUserList();

    // Userdef-Listen einlesen
    rStream >> *m_pUserImageList;
    rStream >> *m_pUserDefList;

    if ( nColorCount != Application::GetDefaultDevice()->GetColorCount() )
        RebuildUserList();

//  SvFileStream aBitmapStream( String("d:\\INPUT.BMP"), STREAM_STD_WRITE);
//  aBitmapStream << pUserImageList->GetBitmap();

    SetDefault( FALSE );
    return SfxConfigItem::ERR_OK;
}

BOOL SfxImageManager_Impl::ReInitialize()
{
    BOOL bRet = SfxConfigItem::ReInitialize();
    if ( bRet )
        for ( USHORT n=0; n<m_aList.Count(); n++ )
            m_aList.GetObject(n)->Call( this );
    return bRet;
}

//-------------------------------------------------------------------------


BOOL SfxImageManager_Impl::Store(SvStream& rStream)
{
//  SvFileStream aBitmapStream(String("d:\\OUTPUT.BMP"), STREAM_STD_WRITE);
//  aBitmapStream << pUserImageList->GetBitmap();

    rStream << nVersion
            << SfxImageManager::GetCurrentSymbolSet()
            << Application::GetDefaultDevice()->GetColorCount();

    rStream << *m_pUserImageList;
    rStream << *m_pUserDefList;

    return TRUE;
}

//-------------------------------------------------------------------------

void SfxImageManager_Impl::UseDefault()
{
    MakeDefaultImageList();
    MakeUserList();
    SetDefault( TRUE );
}

//-------------------------------------------------------------------------

String SfxImageManager_Impl::GetStreamName() const
{
    return SfxConfigItem::GetStreamName( GetType() );
}

//-------------------------------------------------------------------------

void SfxImageManager_Impl::MakeUserList()
{
    if ( m_pUserImageList )
    {
        DELETEZ( m_pUserImageList );
        DELETEZ( m_pHCUserImageList );
        DELETEZ( m_pUserDefList );
    }

    m_pUserDefList = new SfxBitmapList_Impl;
    m_pUserImageList = new ImageList;
    m_pHCUserImageList = new ImageList;
}

//-------------------------------------------------------------------------

void SfxImageManager_Impl::MakeDefaultImageList( BOOL bHiContrast )
{
    USHORT nType=0;
    {
        switch ( SfxImageManager::GetCurrentSymbolSet() )
        {
            case SFX_SYMBOLS_SMALL:
                if ( bHiContrast )
                    pHiImageList = GetImageList( FALSE, TRUE );
                else
                    pImageList = GetImageList( FALSE, FALSE );
                break;
            case SFX_SYMBOLS_LARGE:
                if ( bHiContrast )
                    pHiImageList = GetImageList( TRUE, TRUE );
                else
                    pImageList = GetImageList( TRUE, FALSE );
                break;
            default:
                DBG_ERROR("Unknown Symboltype!");
                break;
        }
    }
}

//-------------------------------------------------------------------------

void SfxImageManager::ExchangeItemImage_Impl( USHORT nId, const Image& rImage )
{
    for ( USHORT n=0; n<pData->pToolBoxList->Count(); n++ )
    {
        ToolBox *pBox = (*pData->pToolBoxList)[n]->pToolBox;
        pBox->SetItemImage( nId, rImage );
    }
}

void SfxImageManager_Impl::RebuildUserList()
{
    // Userliste neu aufbauen
    Color aColor (0xC0, 0xC0, 0xC0);
    VirtualDevice aDev;
    Size aNewSize = pImageList->GetImageSize();
    aDev.SetOutputSizePixel(aNewSize);

    ImageList *pOldList = m_pUserImageList;
    ImageList *pOldHCList = m_pHCUserImageList;
    m_pUserImageList = new ImageList( pOldList->GetImageCount() );
    m_pHCUserImageList = new ImageList( pOldHCList->GetImageCount() );

    for ( USHORT i=0; i<pOldList->GetImageCount(); i++ )
    {
        USHORT nId = pOldList->GetImageId( i );
        Image aImage;

        // Image benutzerdefiniert ?
        if ( m_pUserDefList->GetBitmapPos(nId) != USHRT_MAX )
        {
            Bitmap *pBmp = m_pUserDefList->GetBitmap( nId );
            if ( pBmp->GetSizePixel() != aNewSize )
            {
                aDev.DrawBitmap( Point(), aNewSize, *pBmp );
                aImage = Image( aDev.GetBitmap(Point(), aNewSize), aColor );
            }
            else
                aImage = Image( *pBmp, aColor );

            if ( aImage.GetSizePixel() == aNewSize )
            {
                m_pUserImageList->AddImage( nId, aImage );
                m_pHCUserImageList->AddImage( nId, aImage ); // user images are always used as non high contrast
            }
        }
        else
        {
            aImage = SeekImage( nId, NULL, FALSE ); // look for non high contrast mode image
            m_pUserImageList->AddImage( nId, aImage );
            aImage = SeekImage( nId, NULL, TRUE ); // look for high contrast mode image
            m_pHCUserImageList->AddImage( nId, aImage );
        }
    }

    delete pOldList;
    delete pOldHCList;
}

Image SfxImageManager_Impl::GetImage( USHORT nId, SfxModule *pModule, BOOL bBig, BOOL bHiContrast ) const
{
    if ( !pModule )
        pModule = SFX_APP()->GetActiveModule();
    ImageList *pList=0;
    if ( pModule )
        pList = pModule->GetImageList_Impl( bBig, bHiContrast );

    if ( bHiContrast )
    {
        if ( m_pHCUserImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
            return m_pHCUserImageList->GetImage( nId );
        else if ( pList && pList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
            return pList->GetImage( nId );
        else
            return GetImageList( bBig, bHiContrast )->GetImage( nId );
    }
    else
    {
        if ( m_pUserImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
            return m_pUserImageList->GetImage( nId );
        else if ( pList && pList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
            return pList->GetImage( nId );
        else
            return GetImageList( bBig, bHiContrast )->GetImage( nId );
    }
}

Image SfxImageManager_Impl::GetDefaultImage(USHORT nId, SfxModule* pMod, BOOL bBig, BOOL bHiContrast ) const
{
    if ( !pMod )
        pMod = SFX_APP()->GetActiveModule();
    ImageList *pList=0;
    if ( pMod )
        pList = pMod->GetImageList_Impl( bBig, bHiContrast );

    if ( pList && pList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
        return pList->GetImage( nId );
    else
        return GetImageList( bBig, bHiContrast )->GetImage( nId );
}

Image SfxImageManager_Impl::SeekImage( USHORT nId, SfxModule *pModule, BOOL bHiContrast ) const
{
    BOOL bBig = ( SfxImageManager::GetCurrentSymbolSet() == SFX_SYMBOLS_LARGE );

    if ( !pModule )
        pModule = SFX_APP()->GetActiveModule();
    ImageList *pList=0;
    if ( pModule )
        pList = pModule->GetImageList_Impl( bBig, bHiContrast );

    if ( bHiContrast )
    {
        if ( m_pHCUserImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
            return m_pHCUserImageList->GetImage( nId );
        else if ( pList && pList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
            return pList->GetImage( nId );
        else
            return GetImageList( bBig, bHiContrast )->GetImage( nId );
    }
    else
    {
        if ( m_pUserImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
            return m_pUserImageList->GetImage( nId );
        else if ( pList && pList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
            return pList->GetImage( nId );
        else
            return GetImageList( bBig, bHiContrast )->GetImage( nId );
    }
}

//-------------------------------------------------------------------------

SfxImageManager::SfxImageManager( SfxObjectShell* pDoc )
{
    DBG_MEMTEST();

    // every ImageManager has its own list of toolboxes, so even if it gets a new configuration, the list
    // remains the same!
    pData = new SfxImageManagerData_Impl;
    pData->pToolBoxList = new SfxToolBoxArr_Impl;
    pData->pDoc = pDoc;

    if ( pDoc && pDoc->GetConfigManager() && pDoc->GetConfigManager()->HasConfigItem(SFX_ITEMTYPE_IMAGELIST) )
    {
        pImp = new SfxImageManager_Impl( pDoc->GetConfigManager() );
    }
    else
    {
        if ( !pGlobalConfig )
            pGlobalConfig = new SfxImageManager_Impl( SFX_APP()->GetConfigManager_Impl() );
        pImp = pGlobalConfig;
        nGlobalRef++;
    }

    // internal cached data for comparision in callback
    pData->nSet = SfxImageManager::GetCurrentSymbolSet();
    pData->nOutStyle = pImp->m_aOpt.GetToolboxStyle();

    // register callback for changes of SymbolSet or ToolboxStyle
    pImp->m_aOpt.AddListener( LINK( this, SfxImageManager, OptionsChanged_Impl ) );
    Application::AddEventListener( LINK( this, SfxImageManager, SettingsChanged_Impl ) );

//    SetInternal( TRUE );
    nRef++;
    pImp->AddLink( LINK( this, SfxImageManager, OptionsChanged_Impl ) );
}

//-------------------------------------------------------------------------

SfxImageManager::~SfxImageManager()
{
    pImp->RemoveLink( LINK( this, SfxImageManager, OptionsChanged_Impl ) );

    if ( !--nRef )
    {
        DELETEZ( pImageListSmall );
        DELETEZ( pImageListBig );
        DELETEZ( pImageListHiSmall );
        DELETEZ( pImageListHiBig );
    }

    DELETEZ( pData->pToolBoxList );
    pImp->m_aOpt.RemoveListener( LINK( this, SfxImageManager, OptionsChanged_Impl ) );
    Application::RemoveEventListener( LINK( this, SfxImageManager, SettingsChanged_Impl ) );
    if ( pImp != pGlobalConfig || !--nGlobalRef )
        delete pImp;
    delete pData;
}

SfxConfigManager* SfxImageManager::GetConfigManager()
{
    return pImp->GetConfigManager();
}

USHORT SfxImageManager::GetType() const
{
    return pImp->GetType();
}

BOOL SfxImageManager::Import( SvStream& rInStream, SotStorage& rOutStorage )
{
    SfxImageManager_Impl aImpl( NULL );
    if ( aImpl.Load( rInStream ) == SfxConfigItem::ERR_OK )
        return aImpl.Store( rOutStorage );
    return FALSE;
}

BOOL SfxImageManager::Export( SotStorage& rInStorage, SvStream& rOutStream )
{
    SfxImageManager_Impl aImpl( NULL );
    if ( aImpl.Load( rInStorage ) == SfxConfigItem::ERR_OK )
        return aImpl.Store( rOutStream );
    return FALSE;
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Nimmt eine Referenz auf ein Image auf und erweitert die aktuelle ImageList
    ggf. um ein weiteres Image. Der Zweck dieser Methode ist, da\s verhindert
    werden mu\s, da\s die Toolbox "uber das Image eine Referenz auf die
    Quell-Liste diese am Leben erh"alt.
*/

void SfxImageManager::LockImage( USHORT nId, ToolBox *pBox )
{
    LockImage( nId, pBox, FALSE );
}

void SfxImageManager::LockImage( USHORT nId, ToolBox *pBox, BOOL bHiContrast )
{
    // Das Image mu\s die richtige Gr"o\e haben
    if ( pBox->GetItemImage(nId).GetSizePixel() == pImageList->GetImageSize() )
    {
        // Ist das Image schon vorhanden ?
        ImageList *pUserImageList = pImp->m_pUserImageList;
        ImageList *pHCUserImageList = pImp->m_pHCUserImageList;
        if ( pUserImageList->GetImagePos( nId ) == IMAGELIST_IMAGE_NOTFOUND )
        {
            // Eine physikalische Kopie des Images in der User-Listen machen
            pUserImageList->AddImage( nId, pBox->GetItemImage( nId ) );
            pHCUserImageList->AddImage( nId, pBox->GetItemImage( nId ) );

            if ( SfxMacroConfig::IsMacroSlot(nId) )
                SfxMacroConfig::GetOrCreate()->RegisterSlotId( nId );

            pImp->SetDefault( FALSE );
        }

        // Toolbox should use image from the user image list so that the reference to
        // the source image list will be released!
        if ( bHiContrast )
            pBox->SetItemImage( nId, pHCUserImageList->GetImage( nId ) );
        else
            pBox->SetItemImage( nId, pUserImageList->GetImage( nId ) );
    }
}

//-------------------------------------------------------------------------

Image SfxImageManager::MakeUserImage( USHORT nId, Image& aImage, BOOL bHiContrast )
{
    // Das Image mu\s die richtige Gr"o\e haben
    if ( aImage.GetSizePixel() == pImageList->GetImageSize() )
    {
        // Ist das Image schon vorhanden ?
        ImageList *pUserImageList = pImp->m_pUserImageList;
        ImageList *pHCUserImageList = pImp->m_pHCUserImageList;

        if ( pUserImageList->GetImagePos( nId ) == IMAGELIST_IMAGE_NOTFOUND )
        {
            // Eine physikalische Kopie des Images in den User-Listen machen
            pUserImageList->AddImage( nId, aImage );
            pHCUserImageList->AddImage( nId, aImage );

            if ( SfxMacroConfig::IsMacroSlot(nId) )
                SfxMacroConfig::GetOrCreate()->RegisterSlotId( nId );

            pImp->SetDefault( FALSE );
        }

        // Return image from the user image list so that the reference to
        // the source image list will be released!
        if ( bHiContrast )
            return pHCUserImageList->GetImage( nId );
        else
            return pUserImageList->GetImage( nId );
    }

    return Image();
}

//-------------------------------------------------------------------------

void SfxImageManager::SetSymbolSet_Impl( sal_Int16 nNewSet )
{
    if ( nNewSet != pData->nSet && pImageList )
    {
        pData->nSet = nNewSet;
        Size aOldSize = pImageList->GetImageSize();

        // neue DefaultList erzeugen
        pImageList = NULL;
        pImp->MakeDefaultImageList();
        Size aNewSize = pImageList->GetImageSize();

        if ( !pImp->IsDefault() )
            pImp->RebuildUserList();

        for ( USHORT n=0; n<pData->pToolBoxList->Count(); n++ )
        {
            ToolBoxInf_Impl *pInf = (*pData->pToolBoxList)[n];
            if ( pInf->nFlags & SFX_TOOLBOX_CHANGESYMBOLSET )
            {
                ToolBox *pBox = pInf->pToolBox;
                SfxToolBoxManager* pMgr = pInf->pMgr;

                if ( pMgr )
                {
                    pMgr->RefreshImages_Impl();
                }
                else
                {
                    USHORT nCount = pBox->GetItemCount();
                    for ( USHORT nPos=0; nPos<nCount; nPos++ )
                    {
                        USHORT nId = pBox->GetItemId( nPos );
                        if ( pBox->GetItemType(nPos) == TOOLBOXITEM_BUTTON )
                        {
                            pBox->SetItemImage( nId, GetImage( nId, pInf->pModule ) );
                            SfxStateCache *pCache = SfxViewFrame::Current()->GetBindings().GetStateCache( nId );
                            if ( pCache )
                                pCache->SetCachedState();
                        }
                    }
                }

                if ( !pBox->IsFloatingMode() )
                {
                    Size aActSize( pBox->GetSizePixel() );
                    Size aSize( pBox->CalcWindowSizePixel() );
                    if ( pBox->IsHorizontal() )
                        aSize.Width() = aActSize.Width();
                    else
                        aSize.Height() = aActSize.Height();

                    pBox->SetSizePixel( aSize );
                }
            }
        }
    }
}

//-------------------------------------------------------------------------
/* Retrieves the default image for an ID. The search order is
   Module-Imagelist (if existing), Defaultlist
*/

Image SfxImageManager::GetDefaultImage(USHORT nId, SfxModule* pMod, BOOL bBig, BOOL bHiContrast ) const
{
    return pImp->GetDefaultImage( nId, pMod, bBig, bHiContrast );
}

//-------------------------------------------------------------------------
/*  [Beschreibung]

    Sucht das Image der uebergebenen Id. Suchreihenfolge:

    UserDefListe, Modul-Imageliste( sofern vorhanden ), Default-Liste,
    Officeliste( sofern vorhanden ).
*/

Image SfxImageManager::SeekImage( USHORT nId, SfxModule *pModule ) const
{
    return SeekImage( nId, FALSE, pModule );
}

Image SfxImageManager::SeekImage( USHORT nId, BOOL bHiContrast, SfxModule *pModule ) const
{
    return pImp->SeekImage( nId, pModule, bHiContrast );
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Sucht das Image der uebergebenen Id genauso wie SeekImage, allerdings
    wird die Office-liste nicht durchsucht, d.h. es werden nur die aktuell
    konfigurierten Images ber"ucksichtigt.
*/
Image SfxImageManager::GetImage( USHORT nId, SfxModule *pModule ) const
{
    return GetImage( nId, FALSE, pModule );
}

Image SfxImageManager::GetImage( USHORT nId, BOOL bHiContrast, SfxModule *pModule ) const
{
    return pImp->GetImage( nId, pModule, ( SfxImageManager::GetCurrentSymbolSet() == SFX_SYMBOLS_LARGE ), bHiContrast );
}

Image SfxImageManager::GetImage(USHORT nId, SfxModule* pMod, BOOL bBig ) const
{
    return GetImage( nId, pMod, bBig, FALSE );
}

Image SfxImageManager::GetImage( USHORT nId, SfxModule *pModule, BOOL bBig, BOOL bHiContrast ) const
{
    return pImp->GetImage( nId, pModule, bBig, bHiContrast );
}

Image SfxImageManager::GetImageFromModule_Impl( USHORT nId, SfxModule *pModule )
{
    return GetImageFromModule_Impl( nId, pModule, FALSE );
}

Image SfxImageManager::GetImageFromModule_Impl( USHORT nId, SfxModule *pModule, BOOL bHiContrast )
{
    if ( pModule )
    {
        ImageList *pList = pModule->GetImageList_Impl(( SfxImageManager::GetCurrentSymbolSet() == SFX_SYMBOLS_LARGE ), bHiContrast );
        if ( pList )
            return pList->GetImage( nId );
    }
    else
    {
        if ( bHiContrast )
            return pImageList->GetImage( nId );
        else
            return pHiImageList->GetImage( nId );
    }

    return Image();
}

Image SfxImageManager::GetAndLockImage_Impl( USHORT nId, SfxModule *pModule )
{
    return GetAndLockImage_Impl( nId, FALSE, pModule );
}

Image SfxImageManager::GetAndLockImage_Impl( USHORT nId, BOOL bHiContrast, SfxModule *pModule )
{
    BOOL bBig = ( SfxImageManager::GetCurrentSymbolSet() == SFX_SYMBOLS_LARGE );
    ImageList *pUserImageList = pImp->m_pUserImageList;
    ImageList *pHCUserImageList = pImp->m_pHCUserImageList;

    // Zuerst in der UserImagelist suchen
    if ( bHiContrast )
    {
        if ( pHCUserImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
            return pHCUserImageList->GetImage( nId );
    }
    else
    {
        if ( pUserImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
            return pUserImageList->GetImage( nId );
    }

    if ( pModule )
        return GetImage( nId, pModule, bBig, bHiContrast );
    else
    {
        pModule = SFX_APP()->GetActiveModule();

        // Wenn das Image nicht in der default-Liste steht, das Image aber
        // f"ur diese Liste angefordert wird
        if ( pModule && pImageList->GetImagePos( nId ) == IMAGELIST_IMAGE_NOTFOUND )
        {
            // Dann in der Liste des aktiven Moduls suchen
            ImageList *pList = pModule->GetImageList_Impl( bBig, bHiContrast );
            if ( pList && pList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
            {
                // Das Image in die UserImageList "ubertragen
                if ( bHiContrast )
                {
                    // Add it to the high contrast user image list
                    pHCUserImageList->AddImage( nId, pList->GetImage( nId ) );

                    // Do it for the NON high contrast user image list, too!!
                    ImageList *pNonList = pModule->GetImageList_Impl( bBig, FALSE );
                    if ( pNonList )
                        pUserImageList->AddImage( nId, pNonList->GetImage( nId ) );
                }
                else
                {
                    // Add it to the user image list
                    pUserImageList->AddImage( nId, pList->GetImage( nId ) );

                    // Do it for the high contrast user image list, too!!
                    ImageList *pHiList = pModule->GetImageList_Impl( bBig, TRUE );
                    if ( pHiList )
                        pHCUserImageList->AddImage( nId, pHiList->GetImage( nId ) );
                }

                if ( SfxMacroConfig::IsMacroSlot(nId) )
                    SfxMacroConfig::GetOrCreate()->RegisterSlotId( nId );
                pImp->SetDefault( FALSE );

                if ( bHiContrast )
                    return pHCUserImageList->GetImage( nId );
                else
                    return pUserImageList->GetImage( nId );
            }
        }

        return pImageList->GetImage( nId );
    }
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Erzeugt die Office-Liste, die f"ur das Konfigurieren ben"otigt wird.
*/

void SfxImageManager::StartCustomize()
{
    // no more office list
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Entfernt die nachgeladenen ImageLists wieder aus dem Speicher.
*/

void SfxImageManager::EndCustomize()
{
    // no more office list
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Ersetzt das Image f"ur die "ubergebene Id durch die "ubergebene Bitmap
    bzw. entfernt die Bindung an eine Bitmap.
*/

void SfxImageManager::ReplaceImage( USHORT nId, Bitmap* pBmp )
{
    ImageList *pUserImageList = pImp->m_pUserImageList;
    ImageList *pHCUserImageList = pImp->m_pHCUserImageList;

    SfxBitmapList_Impl* pUserDefList = pImp->m_pUserDefList;
    BOOL bReplaced = FALSE;
    if ( !pBmp && GetImage( nId ).GetSizePixel().Width() )
    {
        // Auf default zuruecksetzen; zuerst das Userdef-Image entfernen
        pUserImageList->RemoveImage( nId );
        pHCUserImageList->RemoveImage( nId );

        // Falls zu der "ubergebenen Id eine UserBitmap vorliegt, wird sie
        // jetzt wieder entfernt
        for ( USHORT i=0; i<pUserDefList->GetBitmapCount(); i++ )
            if ( pUserDefList->GetBitmapId(i) == nId )
                pUserDefList->RemoveBitmap( nId );

        Image aImage = GetImage( nId );

        // Release slot id if we have no default image and this is a macro
        if ( !aImage.GetSizePixel().Width() && SfxMacroConfig::IsMacroSlot(nId) )
            SfxMacroConfig::GetOrCreate()->ReleaseSlotId( nId );

        bReplaced = TRUE;
    }
    else if ( pBmp )
    {
        // Eine neue Bitmap aufnehmen
        Size aSize = pBmp->GetSizePixel();
        BOOL bBitmapCreated = FALSE;
        if ( aSize.Width() > 208 || aSize.Height() > 208 )
        {
            // Use size that fits both toolbar image size (16x16/26x26)
            Size aSz( 208, 208 );
            pBmp = new Bitmap( *pBmp );
            pBmp->Scale( aSz, BMP_SCALE_INTERPOLATE );
            bBitmapCreated = TRUE;
        }

        if ( pUserDefList->GetBitmapPos( nId ) == USHRT_MAX )
            pUserDefList->AddBitmap( nId, *pBmp );
        else
            pUserDefList->ReplaceBitmap( nId, *pBmp );

        Color aColor( GetMaskColor() );
        Image aImage( *pBmp, aColor );
        if ( pBmp->GetSizePixel() != pImageList->GetImageSize() )
        {
            // Scale bitmap to fit current toolbar image size
            Size aSize = pImageList->GetImageSize();
            pBmp->Scale( aSize, BMP_SCALE_INTERPOLATE );
            aImage = Image( *pBmp, aColor );
        }

        // In die User-Liste aufnehmen
        if ( pUserImageList->GetImagePos( nId ) == IMAGELIST_IMAGE_NOTFOUND )
        {
            pUserImageList->AddImage( nId, aImage );
            pHCUserImageList->AddImage( nId, aImage );
        }
        else
        {
            pUserImageList->ReplaceImage( nId, aImage );
            pHCUserImageList->ReplaceImage( nId, aImage );
        }

        if ( SfxMacroConfig::IsMacroSlot(nId) )
            SfxMacroConfig::GetOrCreate()->RegisterSlotId( nId );

        bReplaced = TRUE;
        if ( bBitmapCreated )
            delete pBmp; // Delete temporary bitmap again!
    }

    if ( bReplaced )
    {
        // Alle registrierten Toolboxen auf den Stand bringen
        ExchangeItemImage_Impl( nId, GetImage( nId ) );

        if ( pUserImageList->GetImageCount() )
            pImp->SetDefault( FALSE );
        else
            pImp->SetDefault( TRUE );
    }

//  SvFileStream aBitmapStream(String("d:\\REPLACE.BMP"), STREAM_STD_WRITE);
//  aBitmapStream << pUserImageList->GetBitmap();
}

/*  [Beschreibung]

    Meldet eine Toolbox an, damit sie (je nach Flags) bei "Anderungen von
    Symbolset oder OutStyle benachrichtigt werden kann.
*/

void SfxImageManager::RegisterToolBox( ToolBox *pBox, USHORT nFlags )
{
    ToolBoxInf_Impl *pInf = new ToolBoxInf_Impl;
    pInf->pToolBox = pBox;
    pInf->pMgr = NULL;
    pInf->nFlags = nFlags;
    pInf->pModule = NULL;
    pData->pToolBoxList->Append( pInf );
    pBox->SetOutStyle( pImp->m_aOpt.GetToolboxStyle() );
}

void SfxImageManager::RegisterToolBox( ToolBox *pBox, SfxModule* pModule, USHORT nFlags )
{
    ToolBoxInf_Impl *pInf = new ToolBoxInf_Impl;
    pInf->pToolBox = pBox;
    pInf->pMgr = NULL;
    pInf->nFlags = nFlags;
    pInf->pModule = pModule;
    pData->pToolBoxList->Append( pInf );
    pBox->SetOutStyle( pImp->m_aOpt.GetToolboxStyle() );
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Meldet eine Toolbox wieder ab.
*/

void SfxImageManager::ReleaseToolBox( ToolBox *pBox )
{
    for ( USHORT n=0; n<pData->pToolBoxList->Count(); n++ )
    {
        if ( (*pData->pToolBoxList)[n]->pToolBox == pBox )
        {
            delete (*pData->pToolBoxList)[n];
            pData->pToolBoxList->Remove( n );
            return;
        }
    }
}

void SfxImageManager::RegisterToolBoxManager( SfxToolBoxManager *pMgr, USHORT nFlags )
{
    ToolBoxInf_Impl *pInf = new ToolBoxInf_Impl;
    pInf->pToolBox = &pMgr->GetToolBox();
    pInf->pMgr = pMgr;
    pInf->nFlags = nFlags;
    pData->pToolBoxList->Append( pInf );
    pInf->pToolBox->SetOutStyle( pImp->m_aOpt.GetToolboxStyle() );
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Meldet eine Toolbox wieder ab.
*/

void SfxImageManager::ReleaseToolBoxManager( SfxToolBoxManager *pMgr )
{
    for ( USHORT n=0; n<pData->pToolBoxList->Count(); n++ )
    {
        if ( (*pData->pToolBoxList)[n]->pMgr == pMgr )
        {
            delete (*pData->pToolBoxList)[n];
            pData->pToolBoxList->Remove( n );
            return;
        }
    }
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Gibt die Color zu"ruck, die als Maskenfarbe verwendet wird.
*/

Color SfxImageManager::GetMaskColor() const
{
    if ( pImageList->HasMaskColor() )
        return pImageList->GetMaskColor();
    else
        return Color( COL_LIGHTMAGENTA );
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Versorgt die "ubergebene Toolbox mit allen Images, die f"ur die enthaltenen
    Ids konfiguriert sind.
    Es wird ( genauso wie bei GetImage ) nicht in der Office-Liste gesucht.
*/

void SfxImageManager::SetImages( ToolBox& rToolBox, SfxModule *pModule )
{
    SetImages( rToolBox, pModule, FALSE );
}

void SfxImageManager::SetImages( ToolBox& rToolBox, SfxModule *pModule, BOOL bHiContrast )
{
    BOOL bBig = ( pData->nSet == SFX_SYMBOLS_LARGE );

    if ( !pModule )
        pModule = SFX_APP()->GetActiveModule();
    ImageList *pList=0;
    if ( pModule )
        pList = pModule->GetImageList_Impl( bBig, bHiContrast );

    ImageList *pUserImageList   = bHiContrast ? pImp->m_pHCUserImageList : pImp->m_pUserImageList;
    ImageList *pWorkImageList   = GetImageList( bBig, bHiContrast );
    USHORT nCount = rToolBox.GetItemCount();
    for (USHORT n=0; n<nCount; n++)
    {
        USHORT nId = rToolBox.GetItemId(n);
        switch ( rToolBox.GetItemType(n) )
        {
            case TOOLBOXITEM_BUTTON:
            {
                if ( pUserImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
                    rToolBox.SetItemImage(nId, pUserImageList->GetImage(nId));
                else if ( pList && pList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
                    rToolBox.SetItemImage(nId, pList->GetImage(nId));
                else if ( pWorkImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
                    rToolBox.SetItemImage(nId, pWorkImageList->GetImage(nId));
            }

            case TOOLBOXITEM_SEPARATOR:
            case TOOLBOXITEM_SPACE:
            case TOOLBOXITEM_BREAK:
                break;

            default:
                DBG_ERROR( "invalid item-type in toolbox config" );
        }
    }
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Gibt zur"uck, ob f"ur die angegebene Id eine Bitmap konfiguriert wurde.
*/

BOOL SfxImageManager::IsUserDef_Impl(USHORT nId) const
{
    return ( pImp->m_pUserDefList->GetBitmapPos(nId) != USHRT_MAX );
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Gibt die konfigurierte Bitmap f"ur die angegebene Id zur"uck. Wichtig:
    vorher mu\s mit IsUserDef_Impl "uberpr"uft werden, ob es auch eine gibt.
*/

const Bitmap& SfxImageManager::GetUserDefBitmap_Impl( USHORT nId ) const
{
    SfxBitmapList_Impl* pUserDefList = pImp->m_pUserDefList;
    USHORT nPos = pUserDefList->GetBitmapPos( nId );
    DBG_ASSERT( nPos != USHRT_MAX, "Bitmap nicht vorhanden!" );
    return ( *pUserDefList->GetBitmap(nId) );
}

void SfxImageManager::SetOutStyle_Impl( sal_Int16 nNewStyle )
{
    if ( pData->nOutStyle != nNewStyle )
    {
        pData->nOutStyle = nNewStyle;
        for ( USHORT n=0; n<pData->pToolBoxList->Count(); n++ )
        {
            ToolBoxInf_Impl *pInf = (*pData->pToolBoxList)[n];
            if ( pInf->nFlags & SFX_TOOLBOX_CHANGEOUTSTYLE )
            {
                ToolBox *pBox = pInf->pToolBox;
                pBox->SetOutStyle( nNewStyle );
                if ( !pBox->IsFloatingMode() )
                {
                    Size aActSize( pBox->GetSizePixel() );
                    Size aSize( pBox->CalcWindowSizePixel() );
                    if ( pBox->IsHorizontal() )
                        aSize.Width() = aActSize.Width();
                    else
                        aSize.Height() = aActSize.Height();

                    pBox->SetSizePixel( aSize );
                }
                pBox->Invalidate();
            }
        }
    }
}

Size SfxImageManager::GetImageSize() const
{
    return pImageList->GetImageSize();
}

IMPL_LINK( SfxImageManager, OptionsChanged_Impl, void*, pVoid )
{
    SetOutStyle_Impl( pImp->m_aOpt.GetToolboxStyle() );
    SetSymbolSet_Impl( SfxImageManager::GetCurrentSymbolSet() );

    return 0L;
}

Image SfxImageManager::GetGlobalImage( USHORT nId, BOOL bBig )
{
    if ( !pGlobalConfig )
        return Image();
    return pGlobalConfig->GetImage( nId, NULL, bBig, FALSE );
}

Image SfxImageManager::GetGlobalImage( USHORT nId, BOOL bBig, BOOL bHiContrast )
{
    if ( !pGlobalConfig )
        return Image();
    return pGlobalConfig->GetImage( nId, NULL, bBig, bHiContrast );
}

ImageList* SfxImageManager::GetGlobalDefaultImageList( BOOL bBig, BOOL bHiContrast )
{
    return GetImageList( bBig, bHiContrast );
}

IMPL_LINK( SfxImageManager, SettingsChanged_Impl, void*, pVoid )
{
    // Check if toolbar button size have changed and we have to use system settings
    sal_Int16 nSymbolSet = SfxImageManager::GetCurrentSymbolSet();
    if ( pData->nSet != nSymbolSet )
        SetSymbolSet_Impl( nSymbolSet );

    return 0L;
}

IMPL_LINK( SfxImageManager, ConfigChanged_Impl, void*, pVoid )
{
    BOOL bHiContrast = FALSE;

    if ( pData->pToolBoxList->Count() > 0 )
    {
        // Check whether toolbox is in high contrast mode or not!
        ToolBox *pBox = (*pData->pToolBoxList)[0]->pToolBox;
        Color   aBackColor = pBox->GetBackground().GetColor();
        if ( aBackColor.IsDark() )
            bHiContrast = TRUE;
    }

    for ( USHORT i=0; i<pImageList->GetImageCount(); i++ )
    {
        USHORT nId = pImageList->GetImageId(i);
        ExchangeItemImage_Impl( nId, pImp->GetImage( nId,
                                        pData->pDoc->GetModule(),
                                        ( pData->nSet == SFX_SYMBOLS_LARGE ),
                                        bHiContrast ) );
    }

    return TRUE;
}

BOOL SfxImageManager::CopyConfiguration_Impl( SfxConfigManager& rSource, SfxConfigManager& rDest )
{
    SfxImageManager_Impl aTmp( &rDest );
    aTmp.ReConnect( &rSource );
    return rDest.StoreConfigItem( aTmp );
}
