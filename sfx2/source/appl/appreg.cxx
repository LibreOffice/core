/*************************************************************************
 *
 *  $RCSfile: appreg.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-18 16:02:20 $
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

#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif

#include <app.hxx>

#ifndef GCC
#pragma hdrstop
#endif

#include "appdata.hxx"
#include "arrdecl.hxx"
#include "intfrm.hxx"
#include "sfxhelp.hxx"
#include "templdlg.hxx"
#include "objmnctl.hxx"
#include "inettbc.hxx"
#include "stbitem.hxx"
#include "tbedctrl.hxx"
#include "navigat.hxx"
#include "module.hxx"
#include "topfrm.hxx"
#include "appimp.hxx"
#include "mailchildwin.hxx"
#include "partwnd.hxx"
#include "sfxsids.hrc"
#include "recfloat.hxx"

//===================================================================

void SfxApplication::Registrations_Impl()
{
    // Interfaces
    SfxApplication::RegisterInterface();
    SfxModule::RegisterInterface();
    SfxViewFrame::RegisterInterface();
    SfxInternalFrame::RegisterInterface();
    SfxTopViewFrame::RegisterInterface();
    SfxObjectShell::RegisterInterface();
    SfxViewShell::RegisterInterface();

    // ChildWindows
    SfxRecordingFloatWrapper_Impl::RegisterChildWindow();
    SfxNavigatorWrapper::RegisterChildWindow();
    SfxPartChildWnd_Impl::RegisterChildWindow();
    SfxTemplateDialogWrapper::RegisterChildWindow(TRUE);

    // Controller
    SfxToolBoxControl::RegisterControl(SID_REPEAT);
    SfxURLToolBoxControl_Impl::RegisterControl(SID_OPENURL);
    SfxCancelToolBoxControl_Impl::RegisterControl(SID_BROWSE_STOP);
    SfxAppToolBoxControl_Impl::RegisterControl( SID_NEWDOCDIRECT );
    SfxAppToolBoxControl_Impl::RegisterControl( SID_AUTOPILOTMENU );
};

//--------------------------------------------------------------------

void SfxApplication::RegisterToolBoxControl( SfxModule *pMod, SfxTbxCtrlFactory *pFact )
{
    if ( pMod )
    {
        pMod->RegisterToolBoxControl( pFact );
        return;
    }

#ifdef DBG_UTIL
    for ( USHORT n=0; n<pImp->pTbxCtrlFac->Count(); n++ )
    {
        SfxTbxCtrlFactory *pF = (*pImp->pTbxCtrlFac)[n];
        if ( pF->nTypeId && pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            DBG_WARNING("TbxController-Registrierung ist nicht eindeutig!");
        }
    }
#endif

    pImp->pTbxCtrlFac->C40_INSERT( SfxTbxCtrlFactory, pFact, pImp->pTbxCtrlFac->Count() );
}

//--------------------------------------------------------------------

void SfxApplication::RegisterStatusBarControl( SfxModule *pMod, SfxStbCtrlFactory *pFact )
{
    if ( pMod )
    {
        pMod->RegisterStatusBarControl( pFact );
        return;
    }

#ifdef DBG_UTIL
    for ( USHORT n=0; n<pImp->pStbCtrlFac->Count(); n++ )
    {
        SfxStbCtrlFactory *pF = (*pImp->pStbCtrlFac)[n];
        if ( pF->nTypeId && pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            DBG_WARNING("StbController-Registrierung ist nicht eindeutig!");
        }
    }
#endif

    pImp->pStbCtrlFac->C40_INSERT( SfxStbCtrlFactory, pFact, pImp->pStbCtrlFac->Count() );
}

//--------------------------------------------------------------------

void SfxApplication::RegisterMenuControl( SfxModule *pMod, SfxMenuCtrlFactory *pFact )
{
    if ( pMod )
    {
        pMod->RegisterMenuControl( pFact );
        return;
    }

#ifdef DBG_UTIL
    for ( USHORT n=0; n<pImp->pMenuCtrlFac->Count(); n++ )
    {
        SfxMenuCtrlFactory *pF = (*pImp->pMenuCtrlFac)[n];
        if ( pF->nTypeId && pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            DBG_WARNING("MenuController-Registrierung ist nicht eindeutig!");
        }
    }
#endif

    pImp->pMenuCtrlFac->C40_INSERT( SfxMenuCtrlFactory, pFact, pImp->pMenuCtrlFac->Count() );
}

//--------------------------------------------------------------------

void SfxApplication::SetInterfaceByIdImpl( SfxInterfaceId eId,
                                           SfxInterface* pIF )
{
    if ( (USHORT) eId >= nInterfaces )
    {
        USHORT nNewInterfaces = eId + 4;
        SfxInterface **pNewInterfaces = new SfxInterface*[nNewInterfaces];
        memcpy( pNewInterfaces, pInterfaces,
                sizeof(SfxInterface*) * nInterfaces );
        memset( pNewInterfaces+nInterfaces, 0,
                sizeof(SfxInterface*) * (nNewInterfaces-nInterfaces) );
        delete[] pInterfaces;
        nInterfaces = nNewInterfaces;
        pInterfaces = pNewInterfaces;
    }

    DBG_ASSERT( 0 == pInterfaces[ USHORT(eId) ],
                "interface registered more than once" );
#if defined(DBG_UTIL) && defined(MSC)
    USHORT nId = USHORT(eId);
    if ( 0 != pInterfaces[ nId ] )
    {
        ByteString aMsg( pIF->GetClassName() );
        aMsg += " registers over ";
        aMsg += pInterfaces[ nId ]->GetClassName();
        DbgTrace( aMsg.GetBuffer() );
    }
#endif

    pInterfaces[ USHORT(eId) ] = pIF;
}
/*
const SfxObjectFactory& SfxApplication::GetDefaultFactory() const
{
    return SfxObjectFactory::GetDefaultFactory();
}
*/
