/*************************************************************************
 *
 *  $RCSfile: appdata.cxx,v $
 *
 *  $Revision: 1.3 $
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
#ifndef _CACHESTR_HXX //autogen
#include <tools/cachestr.hxx>
#endif
#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _INETSTRM_HXX //autogen
#include <svtools/inetstrm.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif

#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>
#include <vos/mutex.hxx>

#include <vcl/menu.hxx>

#ifndef _LOGINERR_HXX
#include <svtools/loginerr.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DATETIMEITEM_HXX //autogen
#include <svtools/dateitem.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#include "viewfrm.hxx"
#include "appdata.hxx"
#include "dispatch.hxx"
#include "event.hxx"
#include "sfxtypes.hxx"
#include "sfxdir.hxx"
#include "doctempl.hxx"
#include "dataurl.hxx"
#include "arrdecl.hxx"
#include "docfac.hxx"
#include "picklist.hxx"
#include "docfile.hxx"
#include "request.hxx"
#include "referers.hxx"
#include "app.hrc"
#include "sfxresid.hxx"
#include "objshimp.hxx"
#if SUPD<613//MUSTINI
#include "inimgr.hxx"
#endif
#include "ucbhelp.hxx"
#include "appuno.hxx"

SfxAppData_Impl::SfxAppData_Impl( SfxApplication* pApp ) :
        pAppCfg (0),
        pProgress(0),
        pPool(0),
        pFactArr(0),
        pDdeService( 0 ),
        pDocTopics( 0 ),
        pEventConfig(0),
        nAsynchronCalls(0),
        pVerbs(0),
        nBasicCallLevel(0),
        nRescheduleLocks(0),
        nInReschedule(0),
        pInitLinkList(0),
        pMatcher( 0 ),
        pSfxPlugInObjectShellFactory( 0 ),
        pDefFocusWin( 0 ),
        pSfxFrameObjectFactoryPtr( 0 ),
        pStopButtonTimer( 0 ),
        pCancelMgr( 0 ),
        nDocModalMode(0),
        pDisabledSlotList( 0 ),
        pFilterIni( 0 ),
        pSfxPluginObjectFactoryPtr( 0 ),
        pTemplateCommon( 0 ),
        pLabelResMgr( 0 ),
        pTopFrames( new SfxFrameArr_Impl ),
        pSecureURLs(0),
        nAutoTabPageId(0),
        nExecutingSID( 0 ),
        pNewMenu( 0 ),
        pAutoPilotMenu( 0 ),
        pAppDispatch(NULL),
        pTriggerTopic(0),
        pDdeService2(0),
        pMiscConfig(0),
        pThisDocument(0),
        bPlugged(sal_False),
        bOLEResize(sal_False),
        bDirectAliveCount(sal_False),
        bInQuit(sal_False),
        bInvalidateOnUnlock(sal_False),
        bBean( sal_False ),
        bMinimized( sal_False ),
        bInvisible( sal_False ),
        bInException( sal_False ),
        nAppEvent( 0 ),
        pTemplates( 0 ),
        pSaveOptions( 0 ),
        pUndoOptions( 0 ),
        pHelpOptions( 0 )
{
    StartListening( *pApp );
}

SfxAppData_Impl::~SfxAppData_Impl()
{
#ifdef DBG_UTIL
    delete pTopFrames;
    delete pCancelMgr;
    delete pFilterIni;
    delete pSecureURLs;
#endif
}

IMPL_STATIC_LINK( SfxAppData_Impl, CreateDocumentTemplates, void*, EMPTYARG)
{
    pThis->GetDocumentTemplates();
    return 0;
}

void SfxAppData_Impl::UpdateApplicationSettings( sal_Bool bDontHide )
{
    AllSettings aAllSet = Application::GetSettings();
    StyleSettings aStyleSet = aAllSet.GetStyleSettings();
    sal_uInt32 nStyleOptions = aStyleSet.GetOptions();
    if ( bDontHide )
        nStyleOptions &= ~STYLE_OPTION_HIDEDISABLED;
    else
        nStyleOptions |= STYLE_OPTION_HIDEDISABLED;
    aStyleSet.SetOptions( nStyleOptions );
    aAllSet.SetStyleSettings( aStyleSet );
    Application::SetSettings( aAllSet );
}

SfxDocumentTemplates* SfxAppData_Impl::GetDocumentTemplates()
{
    if ( !pTemplates )
    {
        pTemplates = new SfxDocumentTemplates;
        pTemplates->Construct();
    }

    return pTemplates;
}

void SfxAppData_Impl::Notify( SfxBroadcaster &rBC, const SfxHint &rHint )
{
#if SUPD<613//MUSTINI
    const SfxIniManagerHint* pIniManHint = PTR_CAST(SfxIniManagerHint, &rHint);
    if ( pIniManHint && pIniManHint->GetIniKey() == SFX_KEY_DONTHIDE_DISABLEDENTRIES )
    {
        sal_Bool bDontHide = (sal_Bool)(sal_uInt16)pIniManHint->GetNewValue().ToInt32();
        UpdateApplicationSettings( bDontHide );
    }
#endif
}

