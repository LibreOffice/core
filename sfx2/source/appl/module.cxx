/*************************************************************************
 *
 *  $RCSfile: module.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:27 $
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

#pragma hdrstop

//#include <tools/solar.h>

#include <cstdarg>
#include "module.hxx"
#include "app.hxx"
#include "arrdecl.hxx"
#include "sfxresid.hxx"
#include "msgpool.hxx"
#include "tbxctrl.hxx"
#include "stbitem.hxx"
#include "mnuitem.hxx"
#include "childwin.hxx"
#include "mnumgr.hxx"
#include "docfac.hxx"
#include "accmgr.hxx"
#include "objface.hxx"
#include "viewfrm.hxx"

#define SfxModule
#include <sfxslots.hxx>

static SfxModuleArr_Impl* pModules=0;

class SfxModule_Impl
{
public:

    SfxSlotPool*                pSlotPool;
    SfxTbxCtrlFactArr_Impl*     pTbxCtrlFac;
    SfxStbCtrlFactArr_Impl*     pStbCtrlFac;
    SfxMenuCtrlFactArr_Impl*    pMenuCtrlFac;
    SfxChildWinFactArr_Impl*    pFactArr;
    ImageList*                  pImgList;
    SfxSymbolSet                eSet;

                                SfxModule_Impl();
                                ~SfxModule_Impl();
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
    delete pImgList;
}

TYPEINIT1(SfxModule, SfxShell);

//=========================================================================

SFX_IMPL_INTERFACE(SfxModule,SfxShell,SfxResId(0))
{
}

//====================================================================

SfxFileDialog* SfxModule::CreateDocFileDialog
(
    ULONG                   nFlags, //  arithmetische Veroderung der u.g. Sfx-Winbits
    const SfxObjectFactory& rFact,  /*  <SfxObjectFactory>, deren Filter angezeigt
                                        werden sollten */
    const SfxItemSet* pSet          // to forward on FileDialog
)
{
    return SFX_APP()->CreateDocFileDialog( nFlags, rFact, pSet );
}

//====================================================================

ModalDialog* SfxModule::CreateAboutDialog()
{
    return SFX_APP()->CreateAboutDialog();
}

//====================================================================

BOOL SfxModule::QueryUnload()
{
    return TRUE;
}

//====================================================================

SfxModule* SfxModule::Load()

/*  [Description]

    This methode must be overrided in subclasses to load the real module.
    In your dummy subclass you implement it as follows ('...' is the name
    of your library, e.g. 'Sd'):

        SfxModule* ...ModuleDummy::Load()
        {
            return LoadLib...() ? ..._MOD() : 0;
        }

    In your real subclass you implement it as follows:

        SfxModule* ...Module::Load()
        {
            return this;
        }

    The code must be behind the invocation of SFX_...OBJECTFACTORY_LIB,
    which defines the function LoadLib...().
*/

{
    return 0;
}

//====================================================================

void SfxModule::Free()

/*  [Description]

    This methode must be overrided in subclasses to free the real module.
    In your real subclass you implement it as follows ('...' is the name
    of your library, e.g. 'Sd'):

        void ...Module::Free()
        {
            FreeLib...();
        }

    The code must be behind the invocation of SFX_...OBJECTFACTORY_LIB,
    which defines the function LoadLib...().
*/

{
}


//====================================================================

ResMgr* SfxModule::GetResMgr()
{
    return pResMgr;
}

//====================================================================
/*
SfxModule::SfxModule( ResMgr* pMgrP, BOOL bDummyP,
                      SfxObjectFactory* pFactoryP )
    : pResMgr( pMgrP ), bDummy( bDummyP ), pImpl(0L)
{
    Construct_Impl();
    if ( pFactoryP )
        pFactoryP->SetModule_Impl( this );
}
*/
SfxModule::SfxModule( ResMgr* pMgrP, BOOL bDummyP,
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
        pImpl->pImgList=0;

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
            for( USHORT nPos = rArr.Count(); nPos--; )
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
    for (USHORT nFactory=0; nFactory<pImpl->pFactArr->Count(); ++nFactory)
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

void SfxModule::RegisterChildWindowContext( USHORT nId,
        SfxChildWinContextFactory *pFact)
{
    DBG_ASSERT( pImpl, "Kein echtes Modul!" );

    USHORT nCount = pImpl->pFactArr->Count();
    for (USHORT nFactory=0; nFactory<nCount; ++nFactory)
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
    for ( USHORT n=0; n<pImpl->pTbxCtrlFac->Count(); n++ )
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
    for ( USHORT n=0; n<pImpl->pStbCtrlFac->Count(); n++ )
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
    for ( USHORT n=0; n<pImpl->pMenuCtrlFac->Count(); n++ )
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

ImageList* SfxModule::GetImageList_Impl( SfxSymbolSet eSet )
{
    if ( !pImpl->pImgList || pImpl->eSet != eSet )
    {
        pImpl->eSet = eSet;
        delete pImpl->pImgList;
        pImpl->pImgList = new ImageList(
                ResId( eSet == SFX_SYMBOLS_SMALL_COLOR ?
                RID_DEFAULTIMAGELIST_SC : RID_DEFAULTIMAGELIST_LC,
                GetResMgr() ) );
    }

    return pImpl->pImgList;
}
/* ASDBG
Reflection* SfxModule::GetReflection( UsrUik aUIK )
{
    return NULL;
}
*/
SfxTabPage* SfxModule::CreateTabPage( USHORT nId, Window* pParent, const SfxItemSet& rSet )
{
    return NULL;
}

SfxModuleArr_Impl& SfxModule::GetModules_Impl()
{
    if( !pModules )
        pModules = new SfxModuleArr_Impl;
    return *pModules;
};

void SfxModule::Invalidate( USHORT nId )
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst(); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame ) )
        if ( pFrame->GetObjectShell()->GetModule() == this )
            Invalidate_Impl( pFrame->GetBindings(), nId );
}

