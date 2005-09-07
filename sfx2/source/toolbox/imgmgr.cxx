/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imgmgr.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:22:11 $
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

#include <stdio.h>
#include <hash_map>

#include "imgmgr.hxx"
#include "sfx.hrc"
#include "app.hxx"
#include "sfxresid.hxx"
#include "bindings.hxx"
#include "statcach.hxx"
#include "module.hxx"

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif

#include <tools/rcid.h>
#include <tools/link.hxx>
#include <svtools/miscopt.hxx>
#include <vos/mutex.hxx>

#ifndef GCC
#pragma hdrstop
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

const sal_Int32 IMAGELIST_COUNT = 4; // small, small-hi, large, large-hi

struct ToolBoxInf_Impl
{
    ToolBox* pToolBox;
    USHORT   nFlags;
};

class SfxImageManager_Impl
{
public:
    sal_Int16                       m_nSet;
    SvtMiscOptions                  m_aOpt;
    std::vector< ToolBoxInf_Impl* > m_aToolBoxes;
    ImageList*                      m_pImageList[IMAGELIST_COUNT];
    SfxModule*                      m_pModule;

    ImageList*              GetImageList( BOOL bBig, BOOL bHiContrast );
    Image                   GetImage( USHORT nId, BOOL bBig, BOOL bHiContrast );
    void                    SetSymbolSet_Impl( sal_Int16 );

    DECL_LINK( OptionsChanged_Impl, void* );
    DECL_LINK( SettingsChanged_Impl, void* );


    SfxImageManager_Impl( SfxModule* pModule );
    ~SfxImageManager_Impl();
};

typedef std::hash_map< sal_Int64, sal_Int64 > SfxImageManagerMap;

// global image lists
static SfxImageManager_Impl* pGlobalImageManager = 0;
static SfxImageManagerMap    m_ImageManager_ImplMap;
static SfxImageManagerMap    m_ImageManagerMap;
static ImageList*            pImageListSmall=0;
static ImageList*            pImageListBig=0;
static ImageList*            pImageListHiSmall=0;
static ImageList*            pImageListHiBig=0;

static SfxImageManager_Impl* GetImageManager( SfxModule* pModule )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if ( pModule == 0 )
    {
        if ( !pGlobalImageManager )
            pGlobalImageManager = new SfxImageManager_Impl( 0 );
        return pGlobalImageManager;
    }
    else
    {
        SfxImageManager_Impl* pImpl( 0 );
        SfxImageManagerMap::const_iterator pIter = m_ImageManager_ImplMap.find( (sal_Int64)pModule );
        if ( pIter != m_ImageManager_ImplMap.end() )
            pImpl = (SfxImageManager_Impl*)pIter->second;
        else
        {
            pImpl = new SfxImageManager_Impl( pModule );
            m_ImageManager_ImplMap.insert( SfxImageManagerMap::value_type( (sal_Int64)pModule, (sal_Int64)pImpl ));
        }
        return pImpl;
    }
}

// Global image list
static ImageList* GetImageList( BOOL bBig, BOOL bHiContrast )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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

static sal_Int16 impl_convertBools( sal_Bool bLarge, sal_Bool bHiContrast )
{
    sal_Int16 nIndex( 0 );
    if ( bLarge  )
        nIndex += 1;
    if ( bHiContrast )
        nIndex += 2;
    return nIndex;
}

//=========================================================================

SfxImageManager_Impl::SfxImageManager_Impl( SfxModule* pModule ) :
    m_pModule( pModule ),
    m_nSet( SfxImageManager::GetCurrentSymbolSet() )
{
    for ( sal_uInt32 i = 0; i < IMAGELIST_COUNT; i++ )
        m_pImageList[i] = 0;

    m_aOpt.AddListener( LINK( this, SfxImageManager_Impl, OptionsChanged_Impl ) );
    Application::AddEventListener( LINK( this, SfxImageManager_Impl, SettingsChanged_Impl ) );
}

//-------------------------------------------------------------------------

SfxImageManager_Impl::~SfxImageManager_Impl()
{
    m_aOpt.RemoveListener( LINK( this, SfxImageManager_Impl, OptionsChanged_Impl ) );
    Application::RemoveEventListener( LINK( this, SfxImageManager_Impl, SettingsChanged_Impl ) );

    for ( sal_uInt32 i = 0; i < m_aToolBoxes.size(); i++ )
        delete m_aToolBoxes[i];
}

//-------------------------------------------------------------------------

ImageList* SfxImageManager_Impl::GetImageList( BOOL bBig, BOOL bHiContrast )
{
    sal_Int32 nIndex = impl_convertBools( bBig, bHiContrast );
    if ( !m_pImageList[nIndex] )
    {
        if ( !m_pModule )
            m_pImageList[nIndex] = ::GetImageList( bBig, bHiContrast );
        else
            m_pImageList[nIndex] = m_pModule->GetImageList_Impl( bBig, bHiContrast );
    }

    return m_pImageList[nIndex];
}

//-------------------------------------------------------------------------

Image SfxImageManager_Impl::GetImage( USHORT nId, BOOL bBig, BOOL bHiContrast )
{
    ImageList* pImageList = GetImageList( bBig, bHiContrast );
    if ( pImageList && pImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
        return pImageList->GetImage( nId );
    return Image();
}

//-------------------------------------------------------------------------

void SfxImageManager_Impl::SetSymbolSet_Impl( sal_Int16 nNewSet )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if ( nNewSet != m_nSet )
    {
        m_nSet = nNewSet;
        BOOL bLarge( m_nSet == SFX_SYMBOLS_LARGE );

        for ( sal_uInt32 n=0; n < m_aToolBoxes.size(); n++ )
        {
            ToolBoxInf_Impl *pInf = m_aToolBoxes[n];
            if ( pInf->nFlags & SFX_TOOLBOX_CHANGESYMBOLSET )
            {
                ToolBox *pBox       = pInf->pToolBox;
                BOOL    bHiContrast = pBox->GetBackground().GetColor().IsDark();
                USHORT  nCount      = pBox->GetItemCount();
                for ( USHORT nPos=0; nPos<nCount; nPos++ )
                {
                    USHORT nId = pBox->GetItemId( nPos );
                    if ( pBox->GetItemType(nPos) == TOOLBOXITEM_BUTTON )
                    {
                        pBox->SetItemImage( nId, GetImage( nId, bLarge, bHiContrast ) );
                        SfxStateCache *pCache = SfxViewFrame::Current()->GetBindings().GetStateCache( nId );
                        if ( pCache )
                            pCache->SetCachedState();
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

IMPL_LINK( SfxImageManager_Impl, OptionsChanged_Impl, void*, pVoid )
{
    SetSymbolSet_Impl( SfxImageManager::GetCurrentSymbolSet() );
    return 0L;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxImageManager_Impl, SettingsChanged_Impl, void*, pVoid )
{
    // Check if toolbar button size have changed and we have to use system settings
    sal_Int16 nSymbolSet = SfxImageManager::GetCurrentSymbolSet();
    if ( m_nSet != nSymbolSet )
        SetSymbolSet_Impl( nSymbolSet );
    return 0L;
}

//-------------------------------------------------------------------------

sal_Int16 SfxImageManager::GetCurrentSymbolSet()
{
    sal_Int16 eOptSymbolSet = SvtMiscOptions().GetSymbolSet();

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

SfxImageManager::SfxImageManager( SfxModule* pModule )
{
    pImp = ::GetImageManager( pModule );
}

//-------------------------------------------------------------------------

SfxImageManager::~SfxImageManager()
{
}

//-------------------------------------------------------------------------

SfxImageManager* SfxImageManager::GetImageManager( SfxModule* pModule )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    SfxImageManagerMap::const_iterator pIter = m_ImageManagerMap.find( sal_Int64( pModule ));
    if ( pIter != m_ImageManagerMap.end() )
        return (SfxImageManager *)pIter->second;
    else
    {
        SfxImageManager* pSfxImageManager = new SfxImageManager( pModule );
        m_ImageManagerMap.insert( SfxImageManagerMap::value_type(
            sal_Int64( pModule ), sal_Int64( pSfxImageManager )));
        return pSfxImageManager;
    }
}

//-------------------------------------------------------------------------

Image SfxImageManager::GetImage( USHORT nId, BOOL bBig, BOOL bHiContrast ) const
{
    ImageList* pImageList = pImp->GetImageList( bBig, bHiContrast );
    if ( pImageList && pImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
        return pImageList->GetImage( nId );
    return Image();
}

//-------------------------------------------------------------------------

Image SfxImageManager::GetImage( USHORT nId, BOOL bHiContrast ) const
{
    BOOL bLarge = ( GetCurrentSymbolSet() == SFX_SYMBOLS_LARGE );
    return GetImage( nId, bLarge, bHiContrast );
}

//-------------------------------------------------------------------------

Image SfxImageManager::SeekImage( USHORT nId, BOOL bBig, BOOL bHiContrast ) const
{
    sal_Bool bGlobal = ( pImp->m_pModule == 0 );
    ImageList* pImageList = pImp->GetImageList( bBig, bHiContrast );
    if ( pImageList && pImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
        return pImageList->GetImage( nId );
    else if ( !bGlobal )
    {
        pImageList = ::GetImageManager( 0 )->GetImageList( bBig, bHiContrast );
        if ( pImageList && pImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
            return pImageList->GetImage( nId );
    }
    return Image();
}

//-------------------------------------------------------------------------

Image SfxImageManager::SeekImage( USHORT nId, BOOL bHiContrast ) const
{
    BOOL bLarge = ( GetCurrentSymbolSet() == SFX_SYMBOLS_LARGE );
    return SeekImage( nId, bLarge, bHiContrast );
}

//-------------------------------------------------------------------------

void SfxImageManager::RegisterToolBox( ToolBox *pBox, USHORT nFlags )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    ToolBoxInf_Impl* pInf = new ToolBoxInf_Impl;
    pInf->pToolBox = pBox;
    pInf->nFlags   = nFlags;
    pImp->m_aToolBoxes.push_back( pInf );
}

//-------------------------------------------------------------------------

void SfxImageManager::ReleaseToolBox( ToolBox *pBox )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    for ( sal_uInt32 n=0; n < pImp->m_aToolBoxes.size(); n++ )
    {
        if ((pImp->m_aToolBoxes[n])->pToolBox == pBox )
        {
            delete pImp->m_aToolBoxes[n];
            pImp->m_aToolBoxes.erase( pImp->m_aToolBoxes.begin() + n );
            return;
        }
    }
}

//-------------------------------------------------------------------------

void SfxImageManager::SetImages( ToolBox& rToolBox, BOOL bHiContrast, BOOL bLarge )
{
    SetImagesForceSize( rToolBox, bLarge, bHiContrast );
}

//-------------------------------------------------------------------------

void SfxImageManager::SetImagesForceSize( ToolBox& rToolBox, BOOL bHiContrast, BOOL bLarge )
{
    ImageList* pImageList = pImp->GetImageList( bLarge, bHiContrast );

    USHORT nCount = rToolBox.GetItemCount();
    for (USHORT n=0; n<nCount; n++)
    {
        USHORT nId = rToolBox.GetItemId(n);
        switch ( rToolBox.GetItemType(n) )
        {
            case TOOLBOXITEM_BUTTON:
            {
                if ( pImageList && pImageList->GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND )
                    rToolBox.SetItemImage( nId, pImageList->GetImage( nId ));
                else
                    rToolBox.SetItemImage( nId, Image() );
            }

            case TOOLBOXITEM_SEPARATOR:
            case TOOLBOXITEM_SPACE:
            case TOOLBOXITEM_BREAK:
                break;
        }
    }
}

void SfxImageManager::SetImages( ToolBox& rToolBox )
{
    BOOL bLarge = ( pImp->m_nSet == SFX_SYMBOLS_LARGE );
    BOOL bHiContrast = rToolBox.GetBackground().GetColor().IsDark();
    SetImagesForceSize( rToolBox, bHiContrast, bLarge );
}
