/*************************************************************************
 *
 *  $RCSfile: smdll.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:26 $
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

#ifndef _SOT_FACTORY_HXX //autogen
#include <sot/factory.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_MODCTRL_HXX //autogen
#include <svx/modctrl.hxx>
#endif
#ifndef _SVX_ZOOMCTRL_HXX //autogen
#include <svx/zoomctrl.hxx>
#endif


#ifndef _SMDLL_HXX
#include "smdll.hxx"
#endif
#ifndef DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef TOOLBOX_HXX
#include "toolbox.hxx"
#endif
#ifndef VIEW_HXX
#include "view.hxx"
#endif
#ifndef _STARMATH_HRC
#include "starmath.hrc"
#endif

BOOL SmDLL::bInitialized = FALSE;

/*************************************************************************
|*
|* Initialisierung
|*
\************************************************************************/
void SmDLL::Init()
{
    if ( bInitialized )
        return;

    bInitialized = TRUE;

    // called directly after loading the DLL
    // do whatever you want, you may use Sd-DLL too

    // the SdModule must be created
#if 0
    SmDocShell::RegisterFactory(SDT_SMA_DOCFACTPRIO);
#endif

    SmModuleDummy** ppShlPtr = (SmModuleDummy**) GetAppData(SHL_SM);

#if 1
    SvFactory* pFact = PTR_CAST(SvFactory,(*ppShlPtr)->pSmDocShellFactory);
    delete (*ppShlPtr);
    (*ppShlPtr) = new SmModule(pFact);
    (*ppShlPtr)->pSmDocShellFactory = pFact;
#else
    (*ppShlPtr) = new SmModule( &SmDocShell::Factory() );
#endif

    String aResDll(C2S("sm"));
    aResDll += String::CreateFromInt32( SOLARUPD );

    SfxModule *p = SM_MOD1();
    SmModule *pp = (SmModule *) p;

    SmModule::RegisterInterface(pp);

    SmDocShell::Factory().RegisterMenuBar( SmResId(RID_SMMENU) );
    SmDocShell::Factory().RegisterAccel ( SmResId(RID_SMACCEL) );
    SmDocShell::RegisterInterface(pp);

    SmViewShell::RegisterFactory(1);
    SmViewShell::RegisterInterface(pp);

    SvxZoomStatusBarControl::RegisterControl( SID_ATTR_ZOOM, pp );
    SvxModifyControl::RegisterControl( SID_TEXTSTATUS, pp );

    SmToolBoxWrapper::RegisterChildWindow(TRUE);
    SmCmdBoxWrapper::RegisterChildWindow(TRUE);

    ((SmModule*) *ppShlPtr)->InitManager ();
}

/*************************************************************************
|*
|* Deinitialisierung
|*
\************************************************************************/
void SmDLL::Exit()
{
    // the SdModule must be destroyed
    SmModuleDummy** ppShlPtr = (SmModuleDummy**) GetAppData(SHL_SM);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;

    *GetAppData(SHL_SM) = 0;
}


