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


#include <stdio.h>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <sfx2/imgmgr.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/bindings.hxx>
#include "statcach.hxx"
#include <sfx2/module.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/toolbox.hxx>

#include <tools/rcid.h>
#include <tools/link.hxx>
#include <svtools/miscopt.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>

#include <comphelper/processfactory.hxx>

const sal_uInt32 IMAGELIST_COUNT = 4; // small, small-hi, large, large-hi

struct ToolBoxInf_Impl
{
    ToolBox* pToolBox;
    sal_uInt16   nFlags;
};

class SfxImageManager_Impl
{
public:
    SvtMiscOptions                  m_aOpt;
    std::vector< ToolBoxInf_Impl* > m_aToolBoxes;
    sal_Int16                       m_nSymbolsSize;
    ImageList*                      m_pImageList[IMAGELIST_COUNT];
    SfxModule*                      m_pModule;
    bool                            m_bAppEventListener;

    ImageList*              GetImageList( bool bBig );
    Image                   GetImage( sal_uInt16 nId, bool bBig );
    void                    SetSymbolsSize_Impl( sal_Int16 );

    DECL_LINK( OptionsChanged_Impl, void* );
    DECL_LINK( SettingsChanged_Impl, VclWindowEvent* );


    SfxImageManager_Impl( SfxModule* pModule );
    ~SfxImageManager_Impl();
};

namespace
{
    typedef boost::unordered_map< SfxModule*, boost::shared_ptr<SfxImageManager_Impl> > SfxImageManagerImplMap;

    class theImageManagerImplMap :
        public rtl::Static<SfxImageManagerImplMap, theImageManagerImplMap> {};

    class theGlobalImageManager :
        public rtl::StaticWithArg<SfxImageManager_Impl, SfxModule*,
            theGlobalImageManager> {};
}

static SfxImageManager_Impl* GetImageManager( SfxModule* pModule )
{
    SolarMutexGuard aGuard;

    if ( pModule == 0 )
    {
        return &theGlobalImageManager::get(NULL);
    }
    else
    {
        SfxImageManagerImplMap &rImageManager_ImplMap =
            theImageManagerImplMap::get();
        SfxImageManager_Impl* pImpl( 0 );
        SfxImageManagerImplMap::const_iterator pIter = rImageManager_ImplMap.find(pModule);
        if ( pIter != rImageManager_ImplMap.end() )
            pImpl = pIter->second.get();
        else
        {
            rImageManager_ImplMap[pModule].reset(new SfxImageManager_Impl(pModule));
            pImpl = rImageManager_ImplMap[pModule].get();
        }
        return pImpl;
    }
}

// Global image list
static ImageList* GetImageList( bool bBig )
{
    SolarMutexGuard aGuard;
    ImageList* rpList = NULL;

    ResMgr *pResMgr = SfxApplication::GetOrCreate()->GetOffResManager_Impl();

    ResId aResId( bBig ? ( RID_DEFAULTIMAGELIST_LC ) : ( RID_DEFAULTIMAGELIST_SC ), *pResMgr);

    aResId.SetRT( RSC_IMAGELIST );

    DBG_ASSERT( pResMgr->IsAvailable(aResId), "No default ImageList!" );

    if ( pResMgr->IsAvailable(aResId) )
        rpList = new ImageList( aResId );
    else
        rpList = new ImageList();

    return rpList;
}

static sal_Int16 impl_convertBools( bool bLarge )
{
    sal_Int16 nIndex( 0 );
    if ( bLarge  )
        nIndex += 1;
    return nIndex;
}



SfxImageManager_Impl::SfxImageManager_Impl( SfxModule* pModule )
    : m_pModule(pModule)
    , m_bAppEventListener(false)
{
    m_nSymbolsSize = m_aOpt.GetCurrentSymbolsSize();

    for ( sal_uInt32 i = 0; i < IMAGELIST_COUNT; i++ )
        m_pImageList[i] = 0;

    m_aOpt.AddListenerLink( LINK( this, SfxImageManager_Impl, OptionsChanged_Impl ) );
    Application::AddEventListener( LINK( this, SfxImageManager_Impl, SettingsChanged_Impl ) );
    m_bAppEventListener = true;
}



SfxImageManager_Impl::~SfxImageManager_Impl()
{
    m_aOpt.RemoveListenerLink( LINK( this, SfxImageManager_Impl, OptionsChanged_Impl ) );
    if (m_bAppEventListener)
        Application::RemoveEventListener( LINK( this, SfxImageManager_Impl, SettingsChanged_Impl ) );
    for ( sal_uInt32 i = 0; i < m_aToolBoxes.size(); i++ )
        delete m_aToolBoxes[i];
}



ImageList* SfxImageManager_Impl::GetImageList( bool bBig )
{
    sal_Int32 nIndex = impl_convertBools( bBig );
    if ( !m_pImageList[nIndex] )
    {
        if ( !m_pModule )
            m_pImageList[nIndex] = ::GetImageList( bBig );
        else
            m_pImageList[nIndex] = m_pModule->GetImageList_Impl( bBig );
    }

    return m_pImageList[nIndex];
}



Image SfxImageManager_Impl::GetImage( sal_uInt16 nId, bool bBig )
{
    ImageList* pImageList = GetImageList( bBig );
    if ( pImageList )
        return pImageList->GetImage( nId );
    return Image();
}



void SfxImageManager_Impl::SetSymbolsSize_Impl( sal_Int16 nNewSymbolsSize )
{
    SolarMutexGuard aGuard;

    if ( nNewSymbolsSize != m_nSymbolsSize )
    {
        m_nSymbolsSize = nNewSymbolsSize;
        bool bLarge( m_nSymbolsSize == SFX_SYMBOLS_SIZE_LARGE );

        for ( sal_uInt32 n=0; n < m_aToolBoxes.size(); n++ )
        {
            ToolBoxInf_Impl *pInf = m_aToolBoxes[n];
            if ( pInf->nFlags & SFX_TOOLBOX_CHANGESYMBOLSET )
            {
                ToolBox *pBox       = pInf->pToolBox;
                sal_uInt16  nCount      = pBox->GetItemCount();
                for ( sal_uInt16 nPos=0; nPos<nCount; nPos++ )
                {
                    sal_uInt16 nId = pBox->GetItemId( nPos );
                    if ( pBox->GetItemType(nPos) == TOOLBOXITEM_BUTTON )
                    {
                        pBox->SetItemImage( nId, GetImage( nId, bLarge ) );
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



IMPL_LINK_NOARG(SfxImageManager_Impl, OptionsChanged_Impl)
{
    SetSymbolsSize_Impl( m_aOpt.GetCurrentSymbolsSize() );
    return 0L;
}



IMPL_LINK( SfxImageManager_Impl, SettingsChanged_Impl, VclWindowEvent*, pEvent)
{
    if (pEvent)
    {
        switch (pEvent->GetId())
        {
            case VCLEVENT_OBJECT_DYING:
                if (m_bAppEventListener)
                {
                    Application::RemoveEventListener( LINK( this, SfxImageManager_Impl, SettingsChanged_Impl ) );
                    m_bAppEventListener = false;
                }
                break;
            case VCLEVENT_APPLICATION_DATACHANGED:
                // Check if toolbar button size have changed and we have to use system settings
                {
                    sal_Int16 nSymbolsSize = m_aOpt.GetCurrentSymbolsSize();
                    if (m_nSymbolsSize != nSymbolsSize)
                        SetSymbolsSize_Impl(nSymbolsSize);
                }
                break;
            default:
                break;
        }
    }
    return 0L;
}





SfxImageManager::SfxImageManager( SfxModule* pModule )
{
    pImp = ::GetImageManager( pModule );
}



SfxImageManager::~SfxImageManager()
{
}



namespace
{
    typedef boost::unordered_map< SfxModule*, boost::shared_ptr<SfxImageManager> > SfxImageManagerMap;

    class theImageManagerMap :
        public rtl::Static<SfxImageManagerMap, theImageManagerMap> {};
}

SfxImageManager* SfxImageManager::GetImageManager( SfxModule* pModule )
{
    SolarMutexGuard aGuard;
    SfxImageManager* pSfxImageManager(0);

    SfxImageManagerMap &rImageManagerMap = theImageManagerMap::get();

    SfxImageManagerMap::const_iterator pIter = rImageManagerMap.find(pModule);
    if ( pIter != rImageManagerMap.end() )
        pSfxImageManager = pIter->second.get();
    else
    {
        rImageManagerMap[pModule].reset(new SfxImageManager(pModule));
        pSfxImageManager = rImageManagerMap[pModule].get();
    }
    return pSfxImageManager;
}



Image SfxImageManager::GetImage( sal_uInt16 nId, bool bBig ) const
{
    ImageList* pImageList = pImp->GetImageList( bBig );
    if ( pImageList && pImageList->HasImageAtPos( nId ) )
        return pImageList->GetImage( nId );
    return Image();
}



Image SfxImageManager::GetImage( sal_uInt16 nId ) const
{
    bool bLarge = SvtMiscOptions().AreCurrentSymbolsLarge();
    return GetImage( nId, bLarge );
}



Image SfxImageManager::SeekImage( sal_uInt16 nId, bool bBig ) const
{
    bool bGlobal = ( pImp->m_pModule == 0 );
    ImageList* pImageList = pImp->GetImageList( bBig );
    if ( pImageList && pImageList->HasImageAtPos( nId ) )
        return pImageList->GetImage( nId );
    else if ( !bGlobal )
    {
        pImageList = ::GetImageManager( 0 )->GetImageList( bBig );
        if ( pImageList )
            return pImageList->GetImage( nId );
    }
    return Image();
}



Image SfxImageManager::SeekImage( sal_uInt16 nId ) const
{
    bool bLarge = SvtMiscOptions().AreCurrentSymbolsLarge();
    return SeekImage( nId, bLarge );
}



void SfxImageManager::RegisterToolBox( ToolBox *pBox, sal_uInt16 nFlags )
{
    SolarMutexGuard aGuard;

    ToolBoxInf_Impl* pInf = new ToolBoxInf_Impl;
    pInf->pToolBox = pBox;
    pInf->nFlags   = nFlags;
    pImp->m_aToolBoxes.push_back( pInf );
}



void SfxImageManager::ReleaseToolBox( ToolBox *pBox )
{
    SolarMutexGuard aGuard;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
