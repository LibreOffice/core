/*************************************************************************
 *
 *  $RCSfile: sddll.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:30 $
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

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#include <svx/editeng.hxx>
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#pragma hdrstop

#include "sddll.hxx"
#include "docshell.hxx"
#include "grdocsh.hxx"
#include "sdresid.hxx"
#include "sdobjfac.hxx"
#include "cfgids.hxx"


/*************************************************************************
|*
|* Init
|*
\************************************************************************/

void SdDLL::Init()
{
    // called directly after loading the DLL
    // do whatever you want, you may use Sd-DLL too

    // the SdModule must be created
    SdModuleDummy** ppShlPtr = (SdModuleDummy**) GetAppData(SHL_DRAW);
#ifndef SO3
    SvFactory* pDrawFact    = (*ppShlPtr)->pSdDrawDocShellFactory;
    SvFactory* pGraphicFact = (*ppShlPtr)->pSdGraphicDocShellFactory;
#else
    SvFactory* pDrawFact    = (SvFactory*)(*ppShlPtr)->pSdDrawDocShellFactory;
    SvFactory* pGraphicFact = (SvFactory*)(*ppShlPtr)->pSdGraphicDocShellFactory;
#endif
    delete (*ppShlPtr);
    (*ppShlPtr) = new SdModule(pDrawFact, pGraphicFact);
    (*ppShlPtr)->pSdDrawDocShellFactory    = pDrawFact;
    (*ppShlPtr)->pSdGraphicDocShellFactory = pGraphicFact;

    if (SFX_APP()->HasFeature(SFX_FEATURE_SIMPRESS))
    {
        SdDrawDocShell::Factory().RegisterMenuBar( SdResId( RID_DRAW_DEFAULTMENU ) );
        SdDrawDocShell::Factory().RegisterPluginMenuBar( SdResId( RID_DRAW_PORTALMENU ) );
        SdDrawDocShell::Factory().RegisterAccel( SdResId( RID_DRAW_DEFAULTACCEL ) );
    }

    if (SFX_APP()->HasFeature(SFX_FEATURE_SDRAW))
    {
        SdGraphicDocShell::Factory().RegisterMenuBar( SdResId( RID_GRAPHIC_DEFAULTMENU ) );
        SdGraphicDocShell::Factory().RegisterPluginMenuBar( SdResId( RID_GRAPHIC_PORTALMENU ) );
        SdGraphicDocShell::Factory().RegisterAccel( SdResId( RID_GRAPHIC_DEFAULTACCEL ) );
    }

    // register your view-factories here
    RegisterFactorys();

    // register your shell-interfaces here
    RegisterInterfaces();

    // register your controllers here
    RegisterControllers();

    // Objekt-Factory eintragen
    SdrObjFactory::InsertMakeUserDataHdl(LINK(&aSdObjectFactory, SdObjectFactory, MakeUserData));
}



/*************************************************************************
|*
|* Exit
|*
\************************************************************************/

void SdDLL::Exit()
{
    // called directly befor unloading the DLL
    // do whatever you want, Sd-DLL is accessible

    // Objekt-Factory austragen
    SdrObjFactory::RemoveMakeUserDataHdl(LINK(&aSdObjectFactory, SdObjectFactory, MakeUserData));

    // the SdModule must be destroyed
    SdModuleDummy** ppShlPtr = (SdModuleDummy**) GetAppData(SHL_DRAW);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;
}





