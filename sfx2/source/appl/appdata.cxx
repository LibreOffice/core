/*************************************************************************
 *
 *  $RCSfile: appdata.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:26 $
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
#ifndef _INIMAN_HXX
#include <svtools/iniman.hxx>
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
#pragma hdrstop

#include <vcl/menu.hxx>

#include "viewfrm.hxx"
#include "appdata.hxx"
#include "bitset.hxx"
#include "dispatch.hxx"
#include "event.hxx"
#include "sfxtypes.hxx"
#include "sfxdir.hxx"
#include "doctempl.hxx"

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
#include "inimgr.hxx"
#include "ucbhelp.hxx"
#include "appuno.hxx"

class SfxModelCollection;
class SvVerbList;
class SfxProgress;
class SfxConfigManager;
class SfxChildWinFactArr_Impl;
class SfxToolBoxConfig;
class SfxMacro;
class SfxDdeDocTopics_Impl;
class DdeService;
class SfxEventConfiguration;
class SfxMacroConfig;
class SvFactory;
class SfxItemPool;
class SfxInitLinkList;
class NotesData;
class SfxFilterMatcher;
class DemoData_Impl;
class SfxExplorer;
class SfxMenuBarManager;
class StopButtonTimer_Impl;
class SvUShorts;
class SfxNodeData_Impl;
class SfxSIDList_Impl;
struct SfxFrameObjectFactoryPtr;
struct SfxPluginObjectFactoryPtr;
class ISfxTemplateCommon;
class SfxAnchorJobList_Impl;
class SfxFilterMatcher;

#ifdef _USE_NAMESPACE
using namespace ::vos;
#endif

extern int put_GlobalOffline( int bOffline );

class SfxIniDefaultManager
{};

//--------------------------------------------------------------------
// SfxAppData_Impl
//--------------------------------------------------------------------

SfxAppData_Impl::SfxAppData_Impl( SfxApplication* pApp ) :
        pAppCfg (0),
        pProgress(0),
        pPool(0),
        pFactArr(0),
        pMacro(0),
        pODKLib(0),
        nConfigManagerAvailable(-1),
        pDdeService( 0 ),
        pDocTopics( 0 ),
        pEventConfig(0),
        pSfxApplicationObjectFactory(0),
        nAsynchronCalls(0),
        pVerbs(0),
        nBasicCallLevel(0),
        nRescheduleLocks(0),
        nInReschedule(0),
        pDemoData(0),
        pInitLinkList(0),
        pMatcher( 0 ),
        pExplorer(0),
        pSfxPlugInObjectShellFactory( 0 ),
        pDefFocusWin( 0 ),
        pAppManager( 0 ),
        pSfxFrameObjectFactoryPtr( 0 ),
        pIniDefMgr( 0 ),
        pPlugService( 0 ),
        pStopButtonTimer( 0 ),
        pCancelMgr( 0 ),
        nDocModalMode(0),
        pDisabledSlotList( 0 ),
        pFilterIni( 0 ),
        pISfxModule( 0 ),
        pSfxPluginObjectFactoryPtr( 0 ),
        pNodeData_Impl( 0 ),
        pSIDList_Impl( 0 ),
        pTemplateCommon( 0 ),
        pActiveInplaceWindow( 0 ),
        pAnchorJobList( 0 ),
        pLabelResMgr( 0 ),
        nDemoKind( 0 ),
        pTopFrames( new SfxFrameArr_Impl ),
        pTrash(0),
        pSecureURLs(0),
        nAutoTabPageId(0),
        pDataLockBytesFactory( 0 ),
        nExecutingSID( 0 ),
        pImageLockBytesFactory( 0 ),
        pInfoLockBytesFactory( 0 ),
        pNewMenu( 0 ),
        pBookmarkMenu( 0 ),
        pAutoPilotMenu( 0 ),
        pStartMenu( 0 ),
        pAppDispatch(NULL),
        pTriggerTopic(0),
        pDdeService2(0),
        pExtBrwOnExceptionList(NULL),
        pExtBrwOffExceptionList(NULL),
        pMiscConfig(0),
        pThisDocument(0),
        bMultiQuickSearch( 2 ),
        bShowFsysExtension( 2 ),
        bUseExternBrowser(2),                   // Zugriff auf INIMAN jetzt noch nicht m"oglich
        bAccelEnabled(sal_False),
        bIBMTitle(sal_False),
        bOLEResize(sal_False),
        bDirectAliveCount(sal_False),
        bInQuit(sal_False),
        bStbWasVisible(sal_False),
        bSessionFailed(sal_False),
        bInvalidateOnUnlock(sal_False),
        bBean( sal_False ),
        bMinimized( sal_False ),
        bInvisible( sal_False ),
        bInException( sal_False ),
        bLateInit_BrowseRegistrationPage(sal_False),
        nAppEvent( 0 ),
        pTemplates( 0 )
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

IMPL_STATIC_LINK( SfxAppData_Impl, CreateDataLockBytesFactory, void*, EMPTYARG)
{
    pThis->GetDocumentTemplates();
//    pThis->pDataLockBytesFactory = new DataLockByteFactory_Impl;
//    pThis->pImageLockBytesFactory = new ImageLockByteFactory_Impl;
//    pThis->pInfoLockBytesFactory = new InfoLockByteFactory_Impl;
    return 0;
}

/*
ImageLockByteFactory_Impl::ImageLockByteFactory_Impl()
    : SvLockBytesFactory( DEFINE_CONST_UNICODE("private:image/*") )
{
}

struct ImageMap_Impl
{
    const char* pName;
    sal_uInt16  nResId;
};

static ImageMap_Impl aImageMap[] =
{
    "sfx", RID_IMAGE_SPECIAL,
    "tlx", RID_IMAGE_USER1,
    "pb", RID_IMAGE_USER2,
    "mi", RID_IMAGE_USER3,
    "mba", RID_IMAGE_USER4,
    "internal-icon-baddata", RID_IMAGE_BADDATA,
    "internal-gopher-binary", RID_IMAGE_BINARY,
    "internal-icon-delayed", RID_IMAGE_DELAYED,
    "internal-icon-embed", RID_IMAGE_EMBED,
    "internal-gopher-image", RID_IMAGE_IMAGE,
    "internal-gopher-index", RID_IMAGE_INDEX,
    "internal-gopher-menu", RID_IMAGE_MENU,
    "internal-gopher-movie", RID_IMAGE_MOVIE,
    "internal-icon-notfound", RID_IMAGE_NOTFOUND,
    "internal-gopher-sound", RID_IMAGE_SOUND,
    "internal-gopher-telnet", RID_IMAGE_TELNET,
    "internal-gopher-text", RID_IMAGE_TEXT,
    "internal-gopher-unknown", RID_IMAGE_UNKNOWN,
    "internal-icon-insecure", RID_IMAGE_UNSECURE
};
*/

class BinaryGlobalLoader : public Resource
{
public:
    BinaryGlobalLoader( SvStream& rStrm, sal_uInt16 nId );
};

class BinaryResLoader : public Resource
{
public:
    BinaryResLoader( SvStream& rStrm, const ResId& rId );
};

BinaryGlobalLoader::BinaryGlobalLoader( SvStream& rStrm, sal_uInt16 nId )
    : Resource( SfxResId( RID_IMAGE_RES ) )
{
    BinaryResLoader( rStrm, ResId(nId ) );
    FreeResource();
}

BinaryResLoader::BinaryResLoader( SvStream& rStrm, const ResId& rId )
    : Resource ( rId )
{
    char*  pStr;
    sal_uInt16 nStrLen;
    sal_uInt16 nBytesLeft;

    nStrLen = *((sal_uInt16*)GetClassRes());
    IncrementRes( sizeof( short ) );

    nBytesLeft = GetRemainSizeRes();

    while( nBytesLeft )
    {
        sal_uInt16 nOut = Min( nStrLen, nBytesLeft );
        sal_uInt16 nInc = Min( (sal_uInt16)(nStrLen+2), nBytesLeft );

        pStr = (char*)GetClassRes();
        rStrm.Write( pStr, nOut );
        IncrementRes( nInc );
        nBytesLeft = (nInc != nBytesLeft) ? GetRemainSizeRes() : 0;
    }
}
/*
SvLockBytesRef ImageLockByteFactory_Impl::CreateLockBytes(
    const String& rName, String& rMime )
{
    SvLockBytesRef xRet;
    String aImage( rName, GetWildcard().Len() - 1,
                   rName.Len() - GetWildcard().Len() + 1 );
    for( sal_uInt16 nPos = sizeof( aImageMap ) / sizeof( ImageMap_Impl ); nPos-- ; )
        if( aImage == aImageMap[ nPos ].pName )
        {
            SvCacheStream* pStream = new SvCacheStream;
            BinaryGlobalLoader aLoader(
                *pStream, aImageMap[ nPos ].nResId );
            xRet = new SvLockBytes( pStream );
            if( aImage == "sfx" )
                rMime = "text/html";
            else
                rMime = "image/gif";
            return xRet;
        }
    return xRet;
}

DataLockByteFactory_Impl::DataLockByteFactory_Impl()
    : SvLockBytesFactory( "data:*" )
{
}


SvLockBytesRef DataLockByteFactory_Impl::CreateLockBytes(
    const String& rName, String& rMime )
{
#ifndef DBG_MI
    String aPath( rName, rName.Search( ':' ) + 1, STRING_LEN );
    String aType = aPath.GetToken( 0, ',' );
    sal_uInt16 nTok = aType.GetTokenCount( ';' );
    sal_Bool b64 = sal_False;
    if( nTok > 1 )
    {
        String aLast = aType.GetToken( nTok - 1,  ';');
        if( aLast == "base64" )
            b64 = sal_True;
    }
    rMime = aType.GetToken( 0, ';' );
    sal_uInt16 nContent = aPath.Search( ',' );
    String aContent;
    SvCacheStream* pCache = new SvCacheStream;
    if( nContent != STRING_NOTFOUND )
    {
        aContent = String( aPath, nContent + 1, STRING_LEN );
        if( b64 )
        {
            SvMemoryStream aTmp;
            aTmp << aContent.GetBuffer();
            aTmp.Seek( 0L);
            INetIStream::Decode64( aTmp, *pCache );
        }
        else
        {
            INetURLObject::Unescape( aContent );
            *pCache << aContent.GetBuffer();
        }
        pCache->Seek( 0L );
    }
    SvLockBytesRef xRef = new SvLockBytes( pCache, sal_True );
#else
    SvLockBytesRef xRef;
#endif
    return xRef;
}


InfoLockByteFactory_Impl::InfoLockByteFactory_Impl()
    : SvLockBytesFactory( "private:info/*" )
{
}
*/
String MakeAnchor_Impl( const String& rURL )
{
    String aRet( DEFINE_CONST_UNICODE("<a target=\"_blank\" href=\"") );
    aRet += rURL;
    aRet += DEFINE_CONST_UNICODE("\">");
    aRet += rURL;
    aRet += DEFINE_CONST_UNICODE("</a>");
    return aRet;
}


void PrepareLine_Impl( SfxObjectShell* pSh, String& aCur )
{
    aCur.SearchAndReplaceAscii(
            "$(FACTORY)", S2U(pSh->GetFactory().GetShortName()) );
    SfxMedium* pMed = pSh->GetMedium();
    String aString;
    if(pMed)
        aString = MakeAnchor_Impl( pMed->GetName());
    else
        aString = String();
    aCur.SearchAndReplaceAscii( "$(URL)", aString);
    String aURL;
    if( pMed ) aURL = pMed->GetPreRedirectedURL();
    if( aURL == pMed->GetName() ) aURL.Erase();
    aCur.SearchAndReplaceAscii(
        "$(ORIGURL)", MakeAnchor_Impl( aURL ) );
    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pSh );
    aCur.SearchAndReplaceAscii(
        "$(VISIBLE)", ( pFrame && pFrame->IsVisible() ) ? '+' : '-' );

    String aJSDirty;
    String aJSExec;
    aCur.SearchAndReplaceAscii(     "$(JSEXEC)", aJSExec );
    aCur.SearchAndReplaceAscii(     "$(JSDIRTY)", aJSDirty );
    aCur.SearchAndReplaceAscii(     "$(FORBID)", pSh->Get_Impl()->bForbidCaching ? 0x002B : 0x002D ); // 2Bh = '+', 2Dh = '-'

    MemCache_Impl& rCache = SfxPickList_Impl::Get()->GetMemCache();
    sal_uInt16 nPos;
    for( nPos = (sal_uInt16)rCache.Count(); nPos--; )
        if( pSh == *rCache.GetObject( nPos ) )
            break;

    aCur.SearchAndReplaceAscii( "$(CACHED)", nPos == USHRT_MAX ? 0x002D : 0x002B ); // 2Bh = '+', 2Dh = '-'

    aCur.SearchAndReplaceAscii(
        "$(EXPIRE)", pMed && pMed->IsExpired() ? 0x002B : 0x002D ); // 2Bh = '+', 2Dh = '-'
    String aPost;
    if( pMed )
    {
        SFX_ITEMSET_ARG(
            pMed->GetItemSet(), pPostItem, SfxStringItem, SID_POSTSTRING,
            sal_False );
        if( pPostItem ) aPost = pPostItem->GetValue();
    }
    aCur.SearchAndReplaceAscii( "$(POSTSTRING)", aPost);
}

/*
SvLockBytesRef InfoLockByteFactory_Impl::CreateLockBytes(
    const String& rName, String& rMime )
{
    SvLockBytesRef xRef;
    String aPath( rName, rName.Search( '/' ) + 1, STRING_LEN );

    if( aPath == "docs" )
    {
        rMime = "text/html";
        SvCacheStream* pStream = new SvCacheStream;
        *pStream << "<html><body>";
        String aStr( SfxResId( STR_MEMINFO_HEADER ) );

        aStr.SearchAndReplace( "$(TOTAL)", SfxObjectShell::Count() );
        sal_uInt16 nVisCount = 0;
        SfxObjectShell* pSh;
        for( pSh = SfxObjectShell::GetFirst( 0, sal_True );
             pSh; pSh = SfxObjectShell::GetNext( *pSh, 0, sal_True ) )
            nVisCount++;
        aStr.SearchAndReplace( "$(VISIBLE)", nVisCount );
        aStr.SearchAndReplace(
            "$(CACHE)", SfxPickList_Impl::Get()->GetMemCache().Count() );
        *pStream << aStr.GetBuffer();

        String aLine( SfxResId( STR_MEMINFO_OBJINFO ) );
        String aCur;
        for( pSh = SfxObjectShell::GetFirst( 0, sal_False );
             pSh; pSh = SfxObjectShell::GetNext( *pSh, 0, sal_False ) )
        {
            aCur = aLine;
            PrepareLine_Impl( pSh, aCur );
            *pStream << aCur.GetBuffer();
        }
        *pStream << "</body></html>";
        xRef = new SvLockBytes( pStream, sal_True );
    }
    return xRef;
}
*/
//--------------------------------------------------------------------

void SfxAppData_Impl::Notify( SfxBroadcaster &rBC, const SfxHint &rHint )
{
    // FileSystem?
    const SfxDirEntryHint* pDirEntryHint = PTR_CAST(SfxDirEntryHint, &rHint);
    if ( pDirEntryHint )
        UCB_Helper::ExecuteCommand( pDirEntryHint->GetObject(), WID_UPDATE );

    // aus INet?
#if 0//(mba)
    INetHint *pINetHint = PTR_CAST(INetHint, &rHint);
    sal_Bool bInvalidateStop = pINetHint &&
                pINetHint->GetId() != INET_WRITE_STATUS &&
                pINetHint->GetId() != INET_READ_STATUS;
    if ( !bInvalidateStop )
    {
        // vom CancelManager?
        SfxSimpleHint *pSimpleHint = PTR_CAST(SfxSimpleHint, &rHint);
        bInvalidateStop = pSimpleHint &&
                          ( pSimpleHint->GetId() & SFX_HINT_CANCELLABLE );
    }

    // k"onnte etwas stoppbar geworden sein (oder nicht mehr)
    if ( bInvalidateStop )
    {
        SfxViewFrame *pFrame = SfxViewFrame::Current();

        // Da viele Notifies aus anderen Threads als dem Main thread kommen ( Chaos! )
        IMutex &rMutex = Application::GetSolarMutex();
        if ( pFrame && rMutex.tryToAcquire() )
        {
            SfxBindings &rBind = pFrame->GetBindings();
            if( rBind.IsInUpdate() )
            {
                DBG_WARNING( "W1:Cancelable während StatusUpdate angelegt!" );
            }
            else
            {
                rBind.Invalidate( SID_BROWSE_STOP );
                rBind.Update( SID_BROWSE_STOP );
                rBind.Invalidate( SID_BROWSE_STOP );// geht sonst nicht aus
            }

            rMutex.release();
        }
    }
#endif

    const SfxIniManagerHint* pIniManHint = PTR_CAST(SfxIniManagerHint, &rHint);
    if ( pIniManHint && pIniManHint->GetIniKey() == SFX_KEY_DONTHIDE_DISABLEDENTRIES )
    {
        sal_Bool bDontHide = (sal_Bool)(sal_uInt16)pIniManHint->GetNewValue().ToInt32();
        UpdateApplicationSettings( bDontHide );
    }
}

//------------------------------------------------------------------------

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

