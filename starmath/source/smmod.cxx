/*************************************************************************
 *
 *  $RCSfile: smmod.cxx,v $
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


#ifndef _GLOBNAME_HXX //autogen
#include <tools/globname.hxx>
#endif
#ifndef _SV_STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _SFXMSG_HXX //autogen
#include <sfx2/msg.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_HRC //autogen
#include <sfx2/sfx.hrc>
#endif
#ifndef _FACTORY_HXX //autogen
#include <so3/factory.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SMMOD_HXX
#include "smmod.hxx"
#endif
#ifndef SYMBOL_HXX
#include "symbol.hxx"
#endif
#ifndef _CONFIG_HXX
#include "config.hxx"
#endif
#ifndef _DIALOG_HXX
#include "dialog.hxx"
#endif
#include "starmath.hrc"

TYPEINIT1( SmModuleDummy, SfxModule );
TYPEINIT1( SmModule, SmModuleDummy );

#define SmModule
#include "smslots.hxx"

SFX_IMPL_INTERFACE(SmModule, SfxModule, SmResId(RID_APPLICATION))
{
    SFX_STATUSBAR_REGISTRATION(SmResId(RID_STATUSBAR));
}

SmModule::SmModule(SvFactory* pObjFact) :
    SmModuleDummy(SFX_APP()->CreateResManager("sm"), FALSE, pObjFact)
{
    SetName( C2S("StarMath" ));

    pConfig = new SmConfig;
    pConfig->Initialize();

    pSymSetManager = new SmSymSetManager;
    pRectCache     = new SmRectCache;
}

SmModule::~SmModule()
{
    delete pConfig;
    delete pSymSetManager;
    delete pRectCache;
}

void SmModule::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);

    for (USHORT nWh = aIter.FirstWhich(); 0 != nWh; nWh = aIter.NextWhich())
        switch (nWh)
        {
            case SID_CONFIGEVENT :
                rSet.DisableItem(SID_CONFIGEVENT);
                break;
        }
}

void SmModule::FillStatusBar(StatusBar &rBar)
{
    rBar.InsertItem(SID_TEXTSTATUS, 300, SIB_LEFT | SIB_IN);
    rBar.InsertItem(SID_ATTR_ZOOM, rBar.GetTextWidth(C2S(" 100% ")));
    rBar.InsertItem(SID_MODIFYSTATUS, rBar.GetTextWidth(C2S(" * ")));
}

SfxModule *SmModule::Load()
{
    return this;
}

void SmModule::Free()
{
}

void SmModule::InitManager()
{
    pSymSetManager->Init();
}



SfxModule *SmModuleDummy::Load()
{
    return 0;
}
/* -----------------15.02.99 12:45-------------------
 *
 * --------------------------------------------------*/
SfxItemSet*  SmModule::CreateItemSet( USHORT nId )
{
    SfxItemSet*  pRet = 0;
    if(nId == SID_SM_EDITOPTIONS)
    {
        pRet = new SfxItemSet(GetPool(),
                             //TP_SMPRINT
                             SID_PRINTSIZE,         SID_PRINTSIZE,
                             SID_PRINTZOOM,         SID_PRINTZOOM,
                             SID_PRINTTITLE,        SID_PRINTTITLE,
                             SID_PRINTTEXT,         SID_PRINTTEXT,
                             SID_PRINTFRAME,        SID_PRINTFRAME,
                             SID_NO_RIGHT_SPACES,   SID_NO_RIGHT_SPACES,
                             0 );

            GetConfig()->ConfigToItemSet(*pRet);
    }
    return pRet;
}
/* -----------------15.02.99 12:45-------------------
 *
 * --------------------------------------------------*/
void SmModule::ApplyItemSet( USHORT nId, const SfxItemSet& rSet )
{
    if(nId == SID_SM_EDITOPTIONS)
    {
        GetConfig()->ItemSetToConfig(rSet);
    }
}
/* -----------------15.02.99 12:45-------------------
 *
 * --------------------------------------------------*/
SfxTabPage*  SmModule::CreateTabPage( USHORT nId, Window* pParent, const SfxItemSet& rSet )
{
    SfxTabPage*  pRet = 0;
    if(nId == SID_SM_TP_PRINTOPTIONS)
        pRet = SmPrintOptionsTabPage::Create( pParent, rSet );
    return pRet;

}



