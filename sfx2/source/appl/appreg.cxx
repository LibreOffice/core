/*************************************************************************
 *
 *  $RCSfile: appreg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: as $ $Date: 2000-11-08 14:25:41 $
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

#ifndef _SV_CONFIG_HXX
#include <vcl/config.hxx>
#endif
#if SUPD<613//MUSTINI
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#endif

#include <app.hxx>

#pragma hdrstop

#include "appdata.hxx"
#include "arrdecl.hxx"
#include "intfrm.hxx"
#include "ipfrm.hxx"
#include "fsetobsh.hxx"
#include "fsetview.hxx"
#include "tbxcust.hxx"
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

//===================================================================

void SfxApplication::Registrations_Impl()
{
    // Interfaces
    SfxApplication::RegisterInterface();
    SfxModule::RegisterInterface();
    SfxViewFrame::RegisterInterface();
    SfxInternalFrame::RegisterInterface();
    SfxInPlaceFrame::RegisterInterface();
    SfxTopViewFrame::RegisterInterface();
    SfxObjectShell::RegisterInterface();
    SfxViewShell::RegisterInterface();
//    SfxFrameSetObjectShell::RegisterInterface();
//    SfxFrameSetViewShell::RegisterInterface();
//    SfxFrameSetView_Impl::RegisterInterface();
//    SfxFrameSetSourceView_Impl::RegisterInterface();

    // ChildWindows
    SfxNavigatorWrapper::RegisterChildWindow();
    SfxToolboxCustomWindow::RegisterChildWindow();
    SfxMailChildWinWrapper_Impl::RegisterChildWindow();
    SfxMailChildWinWrapper_Impl::RegisterChildWindow();
    SfxPartChildWnd_Impl::RegisterChildWindow();

/*! (pb) no help at this time
    SfxHelpPIWrapper::RegisterChildWindow();
    SfxHelpTipsWrapper::RegisterChildWindow();
*/
    SfxTemplateDialogWrapper::RegisterChildWindow(TRUE);

    // Controller
    SfxObjectVerbsControl::RegisterControl(SID_OBJECT);
    SfxMenuControl::RegisterControl(SID_QUITAPP);
    SfxMenuControl::RegisterControl(SID_SAVEASDOC);
    SfxMenuControl::RegisterControl(SID_SAVEDOC);
    SfxMenuControl::RegisterControl(SID_OPENDOC);
    SfxMenuControl::RegisterControl(SID_CLOSEDOC);
    SfxMenuControl::RegisterControl(SID_OBJECTMENU0);
    SfxMenuControl::RegisterControl(SID_OBJECTMENU1);
    SfxMenuControl::RegisterControl(SID_OBJECTMENU2);
    SfxMenuControl::RegisterControl(SID_OBJECTMENU3);
    SfxMenuControl::RegisterControl(SID_UNDO);
    SfxMenuControl::RegisterControl(SID_REDO);
    SfxMenuControl::RegisterControl(SID_REPEAT);
    SfxMenuControl::RegisterControl(SID_MAIL_SENDDOC);
    SfxAppMenuControl_Impl::RegisterControl( SID_NEWDOCDIRECT );
    SfxAppMenuControl_Impl::RegisterControl( SID_BOOKMARKS );
    SfxAppMenuControl_Impl::RegisterControl( SID_AUTOPILOTMENU );
    SfxToolBoxControl::RegisterControl(SID_UNDO);
    SfxToolBoxControl::RegisterControl(SID_REDO);
    SfxToolBoxControl::RegisterControl(SID_REPEAT);
    SfxURLToolBoxControl_Impl::RegisterControl(SID_OPENURL);
    SfxCancelToolBoxControl_Impl::RegisterControl(SID_BROWSE_STOP);
    SfxAppToolBoxControl_Impl::RegisterControl( SID_NEWDOCDIRECT );
    SfxAppToolBoxControl_Impl::RegisterControl( SID_BOOKMARKS );
    SfxAppToolBoxControl_Impl::RegisterControl( SID_AUTOPILOTMENU );
    SfxDragToolBoxControl_Impl::RegisterControl( SID_TOPDOC );
    SfxFrameEditToolBoxControl_Impl::RegisterControl(SID_FRAME_NAME);
    SfxFrameEditToolBoxControl_Impl::RegisterControl(SID_FRAME_CONTENT);
    SfxFrameSpacingControl_Impl::RegisterControl(SID_FRAMESPACING);
    SfxHistoryToolBoxControl_Impl::RegisterControl( SID_BROWSE_FORWARD );
    SfxHistoryToolBoxControl_Impl::RegisterControl( SID_BROWSE_BACKWARD );
    SfxReloadToolBoxControl_Impl::RegisterControl( SID_RELOAD );
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
        delete pInterfaces;
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

const SfxObjectFactory& SfxApplication::GetDefaultFactory() const
{
    return SfxObjectFactory::GetDefaultFactory();
}
