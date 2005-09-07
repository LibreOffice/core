/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: appdata.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:32:46 $
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
#ifndef _CACHESTR_HXX //autogen
#include <tools/cachestr.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
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
#include "comphelper/processfactory.hxx"

#include "viewfrm.hxx"
#include "appdata.hxx"
#include "dispatch.hxx"
#include "event.hxx"
#include "sfxtypes.hxx"
#include "sfxdir.hxx"
#include "doctempl.hxx"
//#include "dataurl.hxx"
#include "arrdecl.hxx"
#include "docfac.hxx"
#include "docfile.hxx"
#include "request.hxx"
#include "referers.hxx"
#include "app.hrc"
#include "sfxresid.hxx"
#include "objshimp.hxx"
#include "appuno.hxx"
#include "imestatuswindow.hxx"

SfxAppData_Impl::SfxAppData_Impl( SfxApplication* pApp ) :
        bServer( false ),
        pDdeService( 0 ),
        pDocTopics( 0 ),
        pTriggerTopic(0),
        pDdeService2(0),
        pFactArr(0),
        pSfxPluginObjectFactoryPtr( 0 ),
        pSfxPlugInObjectShellFactory( 0 ),
        pSfxFrameObjectFactoryPtr( 0 ),
        pTopFrames( new SfxFrameArr_Impl ),
        pInitLinkList(0),
        pMatcher( 0 ),
        pCancelMgr( 0 ),
        pLabelResMgr( 0 ),
        pAppDispatch(NULL),
        pTemplates( 0 ),
        pFilterIni( 0 ),
        pPool(0),
        pEventConfig(0),
        pDisabledSlotList( 0 ),
        pSecureURLs(0),
        pMiscConfig(0),
        pSaveOptions( 0 ),
        pUndoOptions( 0 ),
        pHelpOptions( 0 ),
        pThisDocument(0),
        pProgress(0),
        pDefFocusWin( 0 ),
        pTemplateCommon( 0 ),
        nDocModalMode(0),
        nAutoTabPageId(0),
        nExecutingSID( 0 ),
        nBasicCallLevel(0),
        nRescheduleLocks(0),
        nInReschedule(0),
        nAsynchronCalls(0),
        bPlugged(sal_False),
        bDirectAliveCount(sal_False),
        bInQuit(sal_False),
        bInvalidateOnUnlock(sal_False),
        bBean( sal_False ),
        bMinimized( sal_False ),
        bInvisible( sal_False ),
        bInException( sal_False ),
        nAppEvent( 0 ),
        m_xImeStatusWindow(new sfx2::appl::ImeStatusWindow(
                               *pApp, comphelper::getProcessServiceFactory()))
{
    StartListening( *pApp );
}

SfxAppData_Impl::~SfxAppData_Impl()
{
//#ifdef DBG_UTIL
    DeInitDDE();
    delete pTopFrames;
    delete pCancelMgr;
    delete pFilterIni;
    delete pSecureURLs;
//#endif
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
        pTemplates = new SfxDocumentTemplates;
    else
        pTemplates->ReInitFromComponent();
    return pTemplates;
}

void SfxAppData_Impl::Notify( SfxBroadcaster &rBC, const SfxHint &rHint )
{
    SfxSimpleHint* pHint = PTR_CAST( SfxSimpleHint, &rHint );
    if( pHint && pHint->GetId() == SFX_HINT_CANCELLABLE )
    {
/*
        // vom Cancel-Manager
        for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst();
              pFrame;
              pFrame = SfxViewFrame::GetNext(*pFrame) )
        {
            SfxBindings &rBind = pFrame->GetBindings();
            rBind.Invalidate( SID_BROWSE_STOP );
            if ( !rBind.IsInRegistrations() )
                rBind.Update( SID_BROWSE_STOP );
            rBind.Invalidate( SID_BROWSE_STOP );
        }
 */
    }
}

