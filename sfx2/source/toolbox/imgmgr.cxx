/*************************************************************************
 *
 *  $RCSfile: imgmgr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mba $ $Date: 2001-09-06 08:47:59 $
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
#include <svtools/miscopt.hxx>

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
    LinkList        aList;
    SvtMiscOptions  aOpt;
    SfxBitmapList_Impl* pUserDefList;
    ImageList*      pUserImageList;

    void            MakeDefaultImageList();
    void            MakeUserList();
    void            RebuildUserList();
    Image           GetImage( USHORT nId, SfxModule*, BOOL ) const;
    Image           SeekImage(USHORT nId, SfxModule* pModule ) const;

    virtual BOOL    ReInitialize();
    int             Load( SvStream& );
    BOOL            Store( SvStream& );
    virtual void    UseDefault();
    virtual String  GetStreamName() const;
    virtual int     Load(SotStorage&);
    virtual BOOL    Store(SotStorage&);
    void            AddLink( const Link& );
    void            RemoveLink( const Link& );

                    SfxImageManager_Impl( SfxConfigManager* pCfg );
                    ~SfxImageManager_Impl();
};

// elements common to all ImageManager instances
static ImageList* pImageListSmall=0;
static ImageList* pImageListBig=0;
static ImageList* pOffImageList;
static ImageList* pImageList;
static SfxImageManager_Impl* pGlobalConfig=NULL;
static nRef=0;
static nGlobalRef=0;

// we need a static list of all pImp instances, so all operations that should change all instances can be performed
// which operations ?!

ImageList* GetImageList( BOOL bBig )
{
    ImageList*& rpList = bBig ? pImageListBig : pImageListSmall;
    if ( !rpList )
    {
        ResMgr *pResMgr = Resource::GetResManager();
        ResId aResId( bBig ? RID_DEFAULTIMAGELIST_LC : RID_DEFAULTIMAGELIST_SC );
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
    , pUserImageList( 0 )
    , pUserDefList( 0 )
{
//    SetInternal( TRUE );
    Initialize();
}

SfxImageManager_Impl::~SfxImageManager_Impl()
{
    delete pUserDefList;
    delete pUserImageList;
    if ( this == pGlobalConfig )
        pGlobalConfig = NULL;
}

void SfxImageManager_Impl::AddLink( const Link& rLink )
{
    aList.Insert( new Link( rLink ) );
}

void SfxImageManager_Impl::RemoveLink( const Link& rLink )
{
    for ( USHORT n=0; n<aList.Count(); n++ )
    {
        if ( (*aList.GetObject(n) ) == rLink )
        {
            delete aList.Remove(n);
            break;
        }
    }
}

//-------------------------------------------------------------------------

int SfxImageManager_Impl::Load( SotStorage& rStorage )
{
    SotStorageStreamRef xStream =
            rStorage.OpenSotStream( GetStreamName(), STREAM_STD_READ );
    if ( xStream->GetError() )
        return ERR_READ;

    int nRet = Load( *xStream );
    if ( nRet == ERR_OK )
        MakeDefaultImageList();
    return nRet;
}

BOOL SfxImageManager_Impl::Store( SotStorage& rStorage )
{
    SotStorageStreamRef xStream = rStorage.OpenSotStream( SfxImageManager_Impl::GetStreamName(), STREAM_STD_READWRITE|STREAM_TRUNC );
    if ( xStream->GetError() )
        return FALSE;
    else
        return Store( *xStream );
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
    rStream >> *pUserImageList;
    rStream >> *pUserDefList;

    if ( nColorCount != Application::GetDefaultDevice()->GetColorCount() )
        RebuildUserList();

//  SvFileStream aBitmapStream( String("d:\\INPUT.BMP"), STREAM_STD_WRITE);
//  aBitmapStream << pUserImageList->GetBitmap();

    return SfxConfigItem::ERR_OK;
}

BOOL SfxImageManager_Impl::ReInitialize()
{
    BOOL bRet = SfxConfigItem::ReInitialize();
    if ( bRet )
        for ( USHORT n=0; n<aList.Count(); n++ )
            aList.GetObject(n)->Call( this );
    return bRet;
}

//-------------------------------------------------------------------------

BOOL SfxImageManager_Impl::Store(SvStream& rStream)
{
//  SvFileStream aBitmapStream(String("d:\\OUTPUT.BMP"), STREAM_STD_WRITE);
//  aBitmapStream << pUserImageList->GetBitmap();

    rStream << nVersion
            << aOpt.GetSymbolSet()
            << Application::GetDefaultDevice()->GetColorCount();

    rStream << *pUserImageList;
    rStream << *pUserDefList;

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
    if ( pUserImageList )
    {
        DELETEZ( pUserImageList );
        DELETEZ( pUserDefList );
    }

    pUserDefList = new SfxBitmapList_Impl;
    pUserImageList = new ImageList;
}

//-------------------------------------------------------------------------

void SfxImageManager_Impl::MakeDefaultImageList()
{
    USHORT nType=0;
    switch ( aOpt.GetSymbolSet() )
    {
        case SFX_SYMBOLS_SMALL:
            pImageList = GetImageList( FALSE );
            break;
        case SFX_SYMBOLS_LARGE:
            pImageList = GetImageList( TRUE );
            break;
        default:
            DBG_ERROR("Unknown Symboltype!");
            break;
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

    ImageList *pOldList = pUserImageList;
    pUserImageList = new ImageList( pOldList->GetImageCount() );
    for ( USHORT i=0; i<pOldList->GetImageCount(); i++ )
    {
        USHORT nId = pOldList->GetImageId( i );
        Image aImage;

        // Image benutzerdefiniert ?
        if ( pUserDefList->GetBitmapPos(nId) != USHRT_MAX )
        {
            Bitmap *pBmp = pUserDefList->GetBitmap( nId );
            if ( pBmp->GetSizePixel() != aNewSize )
            {
                aDev.DrawBitmap( Point(), aNewSize, *pBmp );
                aImage = Image( aDev.GetBitmap(Point(), aNewSize), aColor );
            }
            else
                aImage = Image( *pBmp, aColor );
        }
        else
            aImage = SeekImage( nId, NULL );

        if ( aImage.GetSizePixel() == aNewSize )
            pUserImageList->AddImage( nId, aImage );
    }

    delete pOldList;
}

Image SfxImageManager_Impl::GetImage( USHORT nId, SfxModule *pModule, BOOL bBig ) const
{
    if ( !pModule )
        pModule = SFX_APP()->GetActiveModule();
    ImageList *pList=0;
    if ( pModule )
        pList = pModule->GetImageList_Impl( bBig );

    if ( pUserImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
        return pUserImageList->GetImage( nId );
    else if ( pList && pList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
        return pList->GetImage( nId );
    else
        return GetImageList( bBig )->GetImage( nId );
}

Image SfxImageManager_Impl::SeekImage( USHORT nId, SfxModule *pModule ) const
{
    if ( !pModule )
        pModule = SFX_APP()->GetActiveModule();
    ImageList *pList=0;
    if ( pModule )
        pList = pModule->GetImageList_Impl( aOpt.GetSymbolSet() == SFX_SYMBOLS_LARGE );

    if ( pUserImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
        return pUserImageList->GetImage( nId );
    else if ( pList && pList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
        return pList->GetImage( nId );
    else if ( pImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
        return pImageList->GetImage( nId );
    else if ( pOffImageList )
        return pOffImageList->GetImage( nId );
    else
        return pImageList->GetImage(nId); // leeres Image zur"uckgeben
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
    pData->nSet = pImp->aOpt.GetSymbolSet();
    pData->nOutStyle = pImp->aOpt.GetToolboxStyle();

    // register callback for changes of SymbolSet or ToolboxStyle
    pImp->aOpt.AddListener( LINK( this, SfxImageManager, OptionsChanged_Impl ) );

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
        DELETEZ( pOffImageList );
        DELETEZ( pImageListSmall );
        DELETEZ( pImageListBig );
    }

    DELETEZ( pData->pToolBoxList );
    pImp->aOpt.RemoveListener( LINK( this, SfxImageManager, OptionsChanged_Impl ) );
    if ( pImp != pGlobalConfig || !--nGlobalRef )
        delete pImp;
    delete pData;
}

BOOL SfxImageManager::Import( SvStream& rInStream, SotStorage& rOutStorage )
{
/*
    SfxImageManager_Impl aImpl( NULL );
    aImpl.Load( rInStream );
    aImpl.Store( rOutStorage );
 */
    return TRUE;
}

BOOL SfxImageManager::Export( SotStorage& rInStorage, SvStream& rOutStream )
{
/*
    SfxImageManager_Impl aImpl( NULL );
    aImpl.Load( rInStorage );
    aImpl.Store( rOutStream );
 */
    return TRUE;
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
    // Neue Images kommen aus der Office-Liste
    if ( !pOffImageList || pOffImageList->GetImagePos( nId ) == IMAGELIST_IMAGE_NOTFOUND )
        return;

    // Das Image mu\s die richtige Gr"o\e haben
    if ( pBox->GetItemImage(nId).GetSizePixel() == pImageList->GetImageSize() )
    {
        // Ist das Image schon vorhanden ?
        ImageList *pUserImageList = pImp->pUserImageList;
        if ( pUserImageList->GetImagePos( nId ) == IMAGELIST_IMAGE_NOTFOUND )
        {
            // Eine physikalische Kopie des Images in der User-Liste machen
            pUserImageList->AddImage( nId, pBox->GetItemImage( nId ) );
            pImp->SetDefault( FALSE );
        }

        // In der Toolbox dieses neue Image benutzen, so da\s die Referenz
        // auf die Quellliste wieder entfernt wird.
        pBox->SetItemImage( nId, pUserImageList->GetImage( nId ) );
    }
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

        ImageList *pOld = pOffImageList;
        if ( pOld || !pImp->IsDefault() )
        {
            // Auch die Officeliste neu erzeugen bzw. neu anlegen
            DELETEZ( pOffImageList );
            StartCustomize();
        }

        if ( !pImp->IsDefault() )
            pImp->RebuildUserList();

        // Wenn es die Officeliste vorher nicht gab, muss sie jetzt wieder entfernt werden
        if ( !pOld )
            EndCustomize();

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

/*  [Beschreibung]

    Sucht das Image der uebergebenen Id. Suchreihenfolge:

    UserDefListe, Modul-Imageliste( sofern vorhanden ), Default-Liste,
    Officeliste( sofern vorhanden ).
*/

Image SfxImageManager::SeekImage( USHORT nId, SfxModule *pModule ) const
{
    return pImp->SeekImage( nId, pModule );
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Sucht das Image der uebergebenen Id genauso wie SeekImage, allerdings
    wird die Office-liste nicht durchsucht, d.h. es werden nur die aktuell
    konfigurierten Images ber"ucksichtigt.
*/
Image SfxImageManager::GetImage( USHORT nId, SfxModule *pModule ) const
{
    return pImp->GetImage( nId, pModule, ( pImp->aOpt.GetSymbolSet() == SFX_SYMBOLS_LARGE ) );
}

Image SfxImageManager::GetImage( USHORT nId, SfxModule *pModule, BOOL bBig ) const
{
    return pImp->GetImage( nId, pModule, bBig );
}

Image SfxImageManager::GetImageFromModule_Impl( USHORT nId, SfxModule *pModule )
{
    if ( pModule )
    {
        ImageList *pList = pModule->GetImageList_Impl( pImp->aOpt.GetSymbolSet() == SFX_SYMBOLS_LARGE );
        if ( pList )
            return pList->GetImage( nId );
    }
    else
    {
        if ( pOffImageList )
            return pOffImageList->GetImage( nId );
        else
            return pImageList->GetImage( nId );
    }

    return Image();
}

Image SfxImageManager::GetAndLockImage_Impl( USHORT nId, SfxModule *pModule )
{
    ImageList *pUserImageList = pImp->pUserImageList;

    // Zuerst in der UserImagelist suchen
    if ( pUserImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
        return pUserImageList->GetImage( nId );
    else if ( pModule )
        return GetImage( nId, pModule );
    else
    {
        pModule = SFX_APP()->GetActiveModule();

        // Wenn das Image nicht in der default-Liste steht, das Image aber
        // f"ur diese Liste angefordert wird
        if ( pModule && pImageList->GetImagePos( nId ) == IMAGELIST_IMAGE_NOTFOUND )
        {
            // Dann in der Liste des aktiven Moduls suchen
            ImageList *pList = pModule->GetImageList_Impl( pImp->aOpt.GetSymbolSet() == SFX_SYMBOLS_LARGE );
            if ( pList && pList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
            {
                // Das Image in die UserImageList "ubertragen
                pUserImageList->AddImage( nId, pList->GetImage( nId ) );
                pImp->SetDefault( FALSE );
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
    // Officeliste schon angelegt ?
    if ( pOffImageList )
        return;

    USHORT nType=0;
    switch ( pImp->aOpt.GetSymbolSet() )
    {
        case SFX_SYMBOLS_SMALL:
            nType = RID_OFFICEIMAGELIST_SC;
            break;
        case SFX_SYMBOLS_LARGE:
            nType = RID_OFFICEIMAGELIST_LC;
            break;
        default:
            DBG_ERROR("Unbekannter Symboltyp!");
            break;
    }

    // Die Office-Imagelist wird vom default-ResMgr bereitgestellt
    ResMgr *pResMgr = Resource::GetResManager();
    ResId aResId( nType );
    aResId.SetRT( RSC_IMAGELIST );

    DBG_ASSERT( pResMgr->IsAvailable(aResId),
            "Keine default ImageList vorhanden!" );

    if ( pResMgr->IsAvailable(aResId) )
        pOffImageList = new ImageList( aResId );
    else
        pOffImageList = new ImageList();
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Entfernt die nachgeladenen ImageLists wieder aus dem Speicher.
*/

void SfxImageManager::EndCustomize()
{
    DELETEZ( pOffImageList);
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Ersetzt das Image f"ur die "ubergebene Id durch die "ubergebene Bitmap
    bzw. entfernt die Bindung an eine Bitmap.
*/

void SfxImageManager::ReplaceImage( USHORT nId, Bitmap* pBmp )
{
    ImageList *pUserImageList = pImp->pUserImageList;
    SfxBitmapList_Impl* pUserDefList = pImp->pUserDefList;
    BOOL bReplaced = FALSE;
    if ( !pBmp && GetImage( nId ).GetSizePixel().Width() )
    {
        // Auf default zuruecksetzen; zuerst das Userdef-Image entfernen
        pUserImageList->RemoveImage( nId );

        // Falls zu der "ubergebenen Id eine UserBitmap vorliegt, wird sie
        // jetzt wieder entfernt
        for ( USHORT i=0; i<pUserDefList->GetBitmapCount(); i++ )
            if ( pUserDefList->GetBitmapId(i) == nId )
                pUserDefList->RemoveBitmap( nId );

        Image aImage = GetImage( nId );
        if ( !aImage.GetSizePixel().Width() )
        {
            // Kein default-Image vorhanden, vielleicht eines in Officeliste?
            ImageList *pOldOffImageList = pOffImageList;
            if ( !pOldOffImageList )
                StartCustomize();
            aImage = pOffImageList->GetImage( nId );

            // Wenn es die OfficeListe vorher nicht gab, mu\s sie jetzt wieder
            // entfernt werden
            if ( !pOldOffImageList )
                EndCustomize();
        }

        if ( aImage.GetSizePixel().Width() )
        {
            // F"ur die Id soll wieder ein Image aktiviert werden, das nicht
            // defaultm"assig vorhanden ist
            pUserImageList->AddImage( nId, aImage );
        }

        bReplaced = TRUE;
    }
    else if ( pBmp )
    {
        // Eine neue Bitmap aufnehmen
        Size aSize = pBmp->GetSizePixel();
        if ( aSize.Width() > 208 || aSize.Height() > 208 )
        {
            // Maximale Gr"o\se, die f"ur 16x16 und 26x26-BMP n"otig ist
            VirtualDevice aDev;
            Size aSz( 208, 208 );
            aDev.SetOutputSizePixel( aSz );
            aDev.DrawBitmap( Point(), aSz, *pBmp );
            delete pBmp;
            pBmp = new Bitmap( aDev.GetBitmap( Point(), aSz ) );
        }

        if ( pUserDefList->GetBitmapPos( nId ) == USHRT_MAX )
            pUserDefList->AddBitmap( nId, *pBmp );
        else
            pUserDefList->ReplaceBitmap( nId, *pBmp );

        Color aColor ( 0xC0, 0xC0, 0xC0 );
        Image aImage( *pBmp, aColor );
        if ( pBmp->GetSizePixel() != pImageList->GetImageSize() )
        {
            // Evtl. Imagegr"o\se anpassen
            VirtualDevice aDev;
            Size aSize = pImageList->GetImageSize();
            aDev.SetOutputSizePixel( aSize );
            aDev.DrawBitmap( Point(), aSize, *pBmp );
            aImage = Image( aDev.GetBitmap(Point(), aSize), aColor );
        }

        // In die User-Liste aufnehmen
        if ( pUserImageList->GetImagePos( nId ) == IMAGELIST_IMAGE_NOTFOUND )
            pUserImageList->AddImage( nId, aImage );
        else
            pUserImageList->ReplaceImage( nId, aImage );

        bReplaced = TRUE;
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
    pBox->SetOutStyle( pImp->aOpt.GetToolboxStyle() );
}

void SfxImageManager::RegisterToolBox( ToolBox *pBox, SfxModule* pModule, USHORT nFlags )
{
    ToolBoxInf_Impl *pInf = new ToolBoxInf_Impl;
    pInf->pToolBox = pBox;
    pInf->pMgr = NULL;
    pInf->nFlags = nFlags;
    pInf->pModule = pModule;
    pData->pToolBoxList->Append( pInf );
    pBox->SetOutStyle( pImp->aOpt.GetToolboxStyle() );
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
    pInf->pToolBox->SetOutStyle( pImp->aOpt.GetToolboxStyle() );
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
        return Color(0xC0, 0xC0, 0xC0);
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Versorgt die "ubergebene Toolbox mit allen Images, die f"ur die enthaltenen
    Ids konfiguriert sind.
    Es wird ( genauso wie bei GetImage ) nicht in der Office-Liste gesucht.
*/

void SfxImageManager::SetImages( ToolBox& rToolBox, SfxModule *pModule )
{
    if ( !pModule )
        pModule = SFX_APP()->GetActiveModule();
    ImageList *pList=0;
    if ( pModule )
        pList = pModule->GetImageList_Impl( pImp->aOpt.GetSymbolSet() == SFX_SYMBOLS_LARGE );

    ImageList *pUserImageList = pImp->pUserImageList;
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
                else if ( pImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
                    rToolBox.SetItemImage(nId, pImageList->GetImage(nId));
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
    return ( pImp->pUserDefList->GetBitmapPos(nId) != USHRT_MAX );
}

//-------------------------------------------------------------------------

/*  [Beschreibung]

    Gibt die konfigurierte Bitmap f"ur die angegebene Id zur"uck. Wichtig:
    vorher mu\s mit IsUserDef_Impl "uberpr"uft werden, ob es auch eine gibt.
*/

const Bitmap& SfxImageManager::GetUserDefBitmap_Impl( USHORT nId ) const
{
    SfxBitmapList_Impl* pUserDefList = pImp->pUserDefList;
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
    SetOutStyle_Impl( pImp->aOpt.GetToolboxStyle() );
    SetSymbolSet_Impl( pImp->aOpt.GetSymbolSet() );

    return 0L;
}

Image SfxImageManager::GetGlobalImage( USHORT nId, BOOL bBig )
{
    if ( !pGlobalConfig )
        return Image();
    return pGlobalConfig->GetImage( nId, NULL, bBig );
}

IMPL_LINK( SfxImageManager, ConfigChanged_Impl, void*, pVoid )
{
    for ( USHORT i=0; i<pImageList->GetImageCount(); i++ )
    {
        USHORT nId = pImageList->GetImageId(i);
        ExchangeItemImage_Impl( nId, pImp->GetImage( nId, pData->pDoc->GetModule(), ( pData->nSet == SFX_SYMBOLS_LARGE ) ) );
    }

    return TRUE;
}
