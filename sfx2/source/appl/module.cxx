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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#ifndef GCC
#endif

#include <stdio.h>
#include <tools/rcid.h>

#include <cstdarg>
#include <sfx2/module.hxx>
#include <sfx2/app.hxx>
#include "arrdecl.hxx"
#include "sfx2/sfxresid.hxx"
#include <sfx2/msgpool.hxx>
#include <sfx2/tbxctrl.hxx>
#include "sfx2/stbitem.hxx"
#include <sfx2/mnuitem.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/mnumgr.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/intitem.hxx>
#include "sfx2/taskpane.hxx"
#include <tools/diagnose_ex.h>
#include <rtl/strbuf.hxx>

#define SfxModule
#include "sfxslots.hxx"

static SfxModuleArr_Impl* pModules=0;

class SfxModule_Impl
{
public:

    SfxSlotPool*                pSlotPool;
    SfxTbxCtrlFactArr_Impl*     pTbxCtrlFac;
    SfxStbCtrlFactArr_Impl*     pStbCtrlFac;
    SfxMenuCtrlFactArr_Impl*    pMenuCtrlFac;
    SfxChildWinFactArr_Impl*    pFactArr;
    ImageList*                  pImgListSmall;
    ImageList*                  pImgListBig;
    ImageList*                  pImgListHiSmall;
    ImageList*                  pImgListHiBig;

                                SfxModule_Impl();
                                ~SfxModule_Impl();
    ImageList*                  GetImageList( ResMgr*, sal_Bool, sal_Bool bHiContrast = sal_False );
};

SfxModule_Impl::SfxModule_Impl()
 : pSlotPool(0)
{
}

SfxModule_Impl::~SfxModule_Impl()
{
    delete pSlotPool;
    delete pTbxCtrlFac;
    delete pStbCtrlFac;
    delete pMenuCtrlFac;
    delete pFactArr;
    delete pImgListSmall;
    delete pImgListBig;
    delete pImgListHiSmall;
    delete pImgListHiBig;
}

ImageList* SfxModule_Impl::GetImageList( ResMgr* pResMgr, sal_Bool bBig, sal_Bool bHiContrast )
{
    ImageList*& rpList = bBig ? ( bHiContrast ? pImgListHiBig: pImgListBig ) :
                                ( bHiContrast ? pImgListHiSmall : pImgListSmall );
    if ( !rpList )
    {
        ResId aResId( bBig ? ( bHiContrast ? RID_DEFAULTIMAGELIST_LCH : RID_DEFAULTIMAGELIST_LC ) :
                             ( bHiContrast ? RID_DEFAULTIMAGELIST_SCH : RID_DEFAULTIMAGELIST_SC ), *pResMgr );
        aResId.SetRT( RSC_IMAGELIST );

        DBG_ASSERT( pResMgr->IsAvailable(aResId), "No default ImageList!" );

        if ( pResMgr->IsAvailable(aResId) )
            rpList = new ImageList( aResId );
        else
            rpList = new ImageList();
    }

    return rpList; }

TYPEINIT1(SfxModule, SfxShell);

//=========================================================================

SFX_IMPL_INTERFACE(SfxModule,SfxShell,SfxResId(0))
{
}

//====================================================================

ResMgr* SfxModule::GetResMgr()
{
    return pResMgr;
}

//====================================================================
/*
SfxModule::SfxModule( ResMgr* pMgrP, sal_Bool bDummyP,
                      SfxObjectFactory* pFactoryP )
    : pResMgr( pMgrP ), bDummy( bDummyP ), pImpl(0L)
{
    Construct_Impl();
    if ( pFactoryP )
        pFactoryP->SetModule_Impl( this );
}
*/
SfxModule::SfxModule( ResMgr* pMgrP, sal_Bool bDummyP,
                      SfxObjectFactory* pFactoryP, ... )
    : pResMgr( pMgrP ), bDummy( bDummyP ), pImpl(0L)
{
    Construct_Impl();
    va_list pVarArgs;
    va_start( pVarArgs, pFactoryP );
    for ( SfxObjectFactory *pArg = pFactoryP; pArg;
         pArg = va_arg( pVarArgs, SfxObjectFactory* ) )
        pArg->SetModule_Impl( this );
    va_end(pVarArgs);
}

void SfxModule::Construct_Impl()
{
    if( !bDummy )
    {
        SfxApplication *pApp = SFX_APP();
        SfxModuleArr_Impl& rArr = GetModules_Impl();
        SfxModule* pPtr = (SfxModule*)this;
        rArr.C40_INSERT( SfxModule, pPtr, rArr.Count() );
        pImpl = new SfxModule_Impl;
        pImpl->pSlotPool = new SfxSlotPool( &pApp->GetAppSlotPool_Impl(), pResMgr );

        pImpl->pTbxCtrlFac=0;
        pImpl->pStbCtrlFac=0;
        pImpl->pMenuCtrlFac=0;
        pImpl->pFactArr=0;
        pImpl->pImgListSmall=0;
        pImpl->pImgListBig=0;
        pImpl->pImgListHiSmall=0;
        pImpl->pImgListHiBig=0;

        SetPool( &pApp->GetPool() );
    }
}

//====================================================================

SfxModule::~SfxModule()
{
    if( !bDummy )
    {
        if ( SFX_APP()->Get_Impl() )
        {
            // Das Modul wird noch vor dem DeInitialize zerst"ort, also auis dem Array entfernen
            SfxModuleArr_Impl& rArr = GetModules_Impl();
            for( sal_uInt16 nPos = rArr.Count(); nPos--; )
            {
                if( rArr[ nPos ] == this )
                {
                    rArr.Remove( nPos );
                    break;
                }
            }

            delete pImpl;
        }

        delete pResMgr;
    }
}

//-------------------------------------------------------------------------

SfxSlotPool* SfxModule::GetSlotPool() const
{
    return pImpl->pSlotPool;
}

//-------------------------------------------------------------------------

void SfxModule::RegisterChildWindow(SfxChildWinFactory *pFact)
{
    DBG_ASSERT( pImpl, "Kein echtes Modul!" );

    if (!pImpl->pFactArr)
        pImpl->pFactArr = new SfxChildWinFactArr_Impl;

//#ifdef DBG_UTIL
    for (sal_uInt16 nFactory=0; nFactory<pImpl->pFactArr->Count(); ++nFactory)
    {
        if (pFact->nId ==  (*pImpl->pFactArr)[nFactory]->nId)
        {
            pImpl->pFactArr->Remove( nFactory );
            DBG_ERROR("ChildWindow mehrfach registriert!");
            return;
        }
    }
//#endif

    pImpl->pFactArr->C40_INSERT(
        SfxChildWinFactory, pFact, pImpl->pFactArr->Count() );
}

//-------------------------------------------------------------------------

void SfxModule::RegisterChildWindowContext( sal_uInt16 nId,
        SfxChildWinContextFactory *pFact)
{
    DBG_ASSERT( pImpl, "Kein echtes Modul!" );

    sal_uInt16 nCount = pImpl->pFactArr->Count();
    for (sal_uInt16 nFactory=0; nFactory<nCount; ++nFactory)
    {
        SfxChildWinFactory *pF = (*pImpl->pFactArr)[nFactory];
        if ( nId == pF->nId )
        {
            if ( !pF->pArr )
                pF->pArr = new SfxChildWinContextArr_Impl;
            pF->pArr->C40_INSERT( SfxChildWinContextFactory, pFact, pF->pArr->Count() );
            return;
        }
    }

    DBG_ERROR( "Kein ChildWindow fuer diesen Context!" );
}

//-------------------------------------------------------------------------

void SfxModule::RegisterToolBoxControl( SfxTbxCtrlFactory *pFact )
{
    if (!pImpl->pTbxCtrlFac)
        pImpl->pTbxCtrlFac = new SfxTbxCtrlFactArr_Impl;

#ifdef DBG_UTIL
    for ( sal_uInt16 n=0; n<pImpl->pTbxCtrlFac->Count(); n++ )
    {
        SfxTbxCtrlFactory *pF = (*pImpl->pTbxCtrlFac)[n];
        if ( pF->nTypeId && pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            DBG_WARNING("TbxController-Registrierung ist nicht eindeutig!");
        }
    }
#endif

    pImpl->pTbxCtrlFac->C40_INSERT( SfxTbxCtrlFactory, pFact, pImpl->pTbxCtrlFac->Count() );
}

//-------------------------------------------------------------------------

void SfxModule::RegisterStatusBarControl( SfxStbCtrlFactory *pFact )
{
    if (!pImpl->pStbCtrlFac)
        pImpl->pStbCtrlFac = new SfxStbCtrlFactArr_Impl;

#ifdef DBG_UTIL
    for ( sal_uInt16 n=0; n<pImpl->pStbCtrlFac->Count(); n++ )
    {
        SfxStbCtrlFactory *pF = (*pImpl->pStbCtrlFac)[n];
        if ( pF->nTypeId && pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            DBG_WARNING("StbController-Registrierung ist nicht eindeutig!");
        }
    }
#endif

    pImpl->pStbCtrlFac->C40_INSERT( SfxStbCtrlFactory, pFact, pImpl->pStbCtrlFac->Count() );
}

//-------------------------------------------------------------------------

void SfxModule::RegisterMenuControl( SfxMenuCtrlFactory *pFact )
{
    if (!pImpl->pMenuCtrlFac)
        pImpl->pMenuCtrlFac = new SfxMenuCtrlFactArr_Impl;

#ifdef DBG_UTIL
    for ( sal_uInt16 n=0; n<pImpl->pMenuCtrlFac->Count(); n++ )
    {
        SfxMenuCtrlFactory *pF = (*pImpl->pMenuCtrlFac)[n];
        if ( pF->nTypeId && pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            DBG_WARNING("MenuController-Registrierung ist nicht eindeutig!");
        }
    }
#endif

    pImpl->pMenuCtrlFac->C40_INSERT( SfxMenuCtrlFactory, pFact, pImpl->pMenuCtrlFac->Count() );
}

//-------------------------------------------------------------------------

SfxTbxCtrlFactArr_Impl*  SfxModule::GetTbxCtrlFactories_Impl() const
{
    return pImpl->pTbxCtrlFac;
}

//-------------------------------------------------------------------------

SfxStbCtrlFactArr_Impl*  SfxModule::GetStbCtrlFactories_Impl() const
{
    return pImpl->pStbCtrlFac;
}

//-------------------------------------------------------------------------

SfxMenuCtrlFactArr_Impl* SfxModule::GetMenuCtrlFactories_Impl() const
{
    return pImpl->pMenuCtrlFac;
}

//-------------------------------------------------------------------------

SfxChildWinFactArr_Impl* SfxModule::GetChildWinFactories_Impl() const
{
    return pImpl->pFactArr;
}

ImageList* SfxModule::GetImageList_Impl( sal_Bool bBig )
{
    return pImpl->GetImageList( pResMgr, bBig, sal_False );
}

ImageList* SfxModule::GetImageList_Impl( sal_Bool bBig, sal_Bool bHiContrast )
{
    return pImpl->GetImageList( pResMgr, bBig, bHiContrast );
}

SfxTabPage* SfxModule::CreateTabPage( sal_uInt16, Window*, const SfxItemSet& )
{
    return NULL;
}

SfxModuleArr_Impl& SfxModule::GetModules_Impl()
{
    if( !pModules )
        pModules = new SfxModuleArr_Impl;
    return *pModules;
};

void SfxModule::DestroyModules_Impl()
{
    if ( pModules )
    {
        SfxModuleArr_Impl& rModules = *pModules;
        for( sal_uInt16 nPos = rModules.Count(); nPos--; )
    {
        SfxModule* pMod = rModules.GetObject(nPos);
        delete pMod;
    }
    }
}

void SfxModule::Invalidate( sal_uInt16 nId )
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst(); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame ) )
        if ( pFrame->GetObjectShell()->GetModule() == this )
            Invalidate_Impl( pFrame->GetBindings(), nId );
}

sal_Bool SfxModule::IsActive() const
{
    SfxViewFrame* pFrame = SfxViewFrame::Current();
    if ( pFrame && pFrame->GetObjectShell()->GetFactory().GetModule() == this )
        return sal_True;
    return sal_False;
}

bool SfxModule::IsChildWindowAvailable( const sal_uInt16 i_nId, const SfxViewFrame* i_pViewFrame ) const
{
    if ( i_nId != SID_TASKPANE )
        // by default, assume it is
        return true;

    const SfxViewFrame* pViewFrame = i_pViewFrame ? i_pViewFrame : GetFrame();
    ENSURE_OR_RETURN( pViewFrame, "SfxModule::IsChildWindowAvailable: no frame to ask for the module identifier!", false );
    return ::sfx2::ModuleTaskPane::ModuleHasToolPanels( pViewFrame->GetFrame().GetFrameInterface() );
}

SfxModule* SfxModule::GetActiveModule( SfxViewFrame* pFrame )
{
    if ( !pFrame )
        pFrame = SfxViewFrame::Current();
    SfxObjectShell* pSh = 0;
    if( pFrame )
        pSh = pFrame->GetObjectShell();
    return pSh ? pSh->GetModule() : 0;
}

FieldUnit SfxModule::GetModuleFieldUnit( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > const & i_frame )
{
    ENSURE_OR_RETURN( i_frame.is(), "SfxModule::GetModuleFieldUnit: invalid frame!", FUNIT_100TH_MM );

    // find SfxViewFrame for the given XFrame
    SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
    while ( pViewFrame != NULL )
    {
        if ( pViewFrame->GetFrame().GetFrameInterface() == i_frame )
            break;
        pViewFrame = SfxViewFrame::GetNext( *pViewFrame );
    }
    ENSURE_OR_RETURN( pViewFrame != NULL, "SfxModule::GetModuleFieldUnit: unable to find an SfxViewFrame for the given XFrame", FUNIT_100TH_MM );

    // find the module
    SfxModule const * pModule = GetActiveModule( pViewFrame );
    ENSURE_OR_RETURN( pModule != NULL, "SfxModule::GetModuleFieldUnit: no SfxModule for the given frame!", FUNIT_100TH_MM );

    SfxPoolItem const * pItem = pModule->GetItem( SID_ATTR_METRIC );
    if ( pItem == NULL )
    {
#if OSL_DEBUG_LEVEL > 0
        ::rtl::OStringBuffer message;
        message.append( "SfxModule::GetFieldUnit: no metric item in the module implemented by '" );
        message.append( typeid( *pModule ).name() );
        message.append( "'!" );
        OSL_ENSURE( false, message.makeStringAndClear().getStr() );
#endif
        return FUNIT_100TH_MM;
    }
    return (FieldUnit)( (SfxUInt16Item*)pItem )->GetValue();
}

FieldUnit SfxModule::GetCurrentFieldUnit()
{
    FieldUnit eUnit = FUNIT_INCH;
    SfxModule* pModule = GetActiveModule();
    if ( pModule )
    {
        const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );
        DBG_ASSERT( pItem, "GetFieldUnit(): no item" );
        if ( pItem )
            eUnit = (FieldUnit)( (SfxUInt16Item*)pItem )->GetValue();
    }
    else
        DBG_ERRORFILE( "GetModuleFieldUnit(): no module found" );
    return eUnit;
}

FieldUnit SfxModule::GetFieldUnit() const
{
    FieldUnit eUnit = FUNIT_INCH;
    const SfxPoolItem* pItem = GetItem( SID_ATTR_METRIC );
    DBG_ASSERT( pItem, "GetFieldUnit(): no item" );
    if ( pItem )
        eUnit = (FieldUnit)( (SfxUInt16Item*)pItem )->GetValue();
    return eUnit;
}
