/*************************************************************************
 *
 *  $RCSfile: fltfnc.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pb $ $Date: 2000-09-26 11:11:24 $
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

#include "fltfnc.hxx"

#ifndef _EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif
#ifndef _CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _SB_SBMETH_HXX //autogen
#include <basic/sbmeth.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <svtools/sbxobj.hxx>
#endif
#ifndef __SBX_SBXMETHOD_HXX //autogen
#include <svtools/sbxmeth.hxx>
#endif
#ifndef _SBXCORE_HXX //autogen
#include <svtools/sbxcore.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _RTL_USTRING_HXX //autogen
#include <rtl/ustring.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _EXTATTR_HXX
#include <svtools/extattr.hxx>
#endif
#ifndef _LCKBITEM_HXX //autogen
#include <svtools/lckbitem.hxx>
#endif
#ifndef _INETTYPE_HXX //autogen
#include <svtools/inettype.hxx>
#endif
#ifndef _INET_CONFIG_HXX //autogen
#include <inet/inetcfg.hxx>
#endif
#ifndef _SFXRECTITEM_HXX
#include <svtools/rectitem.hxx>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFO_HPP_
#include <com/sun/star/document/XDocumentInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XSTANDALONEDOCUMENTINFO_HPP_
#include <com/sun/star/document/XStandaloneDocumentInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTIONEVENT_HPP_
#include <com/sun/star/frame/FrameActionEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTION_HPP_
#include <com/sun/star/frame/FrameAction.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMELOADER_HPP_
#include <com/sun/star/frame/XFrameLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLOADEVENTLISTENER_HPP_
#include <com/sun/star/frame/XLoadEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFILTERDETECT_HPP_
#include <com/sun/star/frame/XFilterDetect.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINMODE_HPP_
#include <com/sun/star/plugin/PluginMode.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINDESCRIPTION_HPP_
#include <com/sun/star/plugin/PluginDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINEXCEPTION_HPP_
#include <com/sun/star/plugin/PluginException.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINVARIABLE_HPP_
#include <com/sun/star/plugin/PluginVariable.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_XPLUGIN_HPP_
#include <com/sun/star/plugin/XPlugin.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_XPLUGINMANAGER_HPP_
#include <com/sun/star/plugin/XPluginManager.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_XPLUGINCONTEXT_HPP_
#include <com/sun/star/plugin/XPluginContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_XIMPLEMENTATIONLOADER_HPP_
#include <com/sun/star/loader/XImplementationLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_CANNOTACTIVATEFACTORYEXCEPTION_HPP_
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <unotools/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_CHAOS_DOCUMENTHEADERFIELD_HPP_
#include <com/sun/star/chaos/DocumentHeaderField.hpp>
#endif

#include <sal/types.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>
#include <rtl/ustring.hxx>
#include <vos/process.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::chaos;
using namespace ::rtl;
using namespace ::vos;

#ifndef _SV_SOUND_HXX
#include <vcl/sound.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#ifndef _SVTOOLS_CTYPEITM_HXX
#include <svtools/ctypeitm.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif

#include "inimgr.hxx"
#include "app.hxx"
#include "fltdlg.hxx"
#include "sfxhelp.hxx"
#include "sfxbasic.hxx"
#include "docfilt.hxx"
#include "docfac.hxx"
#include "sfxtypes.hxx"
#include "sfxuno.hxx"
#include "docfile.hxx"
#include "progress.hxx"
#include "loadenv.hxx"
#include "iodlg.hxx"
#include "openflag.hxx"
#include "bastyp.hrc"
#include "sfxresid.hxx"
#include "doctempl.hxx"
#include "frame.hxx"
#include "dispatch.hxx"
#include "urlframe.hxx"
#include "picklist.hxx"
#include "topfrm.hxx"
#include "plugwin.hxx"
#include "helper.hxx"

// wg. EXPLORER_BROWSER
#include "request.hxx"
#include "nfltdlg.hxx"
#include "arrdecl.hxx"
#include "appdata.hxx"
#include "appuno.hxx"
#include "viewfrm.hxx"
#include "ucbhelp.hxx"

#define SFX_STR_OVERRIDE "Override"

#ifndef ERRCODE_SFX_RESTART
#define ERRCODE_SFX_RESTART 0
#endif

#define EXPLORER_URL_FILTER         "explorer"
#define SFX_FILTERNAME_COMPONENT    "component"
#define SFX_FILTERNAME_HELPENTRY    "helpentry"

class SfxSound_Impl
{
    Sound*      pSound;
    String      aSoundFile;
    sal_Bool        bDeleteFile;
    DECL_LINK(  Done, void* );
    DECL_LINK(  Start, void* );

public:
                SfxSound_Impl( const String& rSound, sal_Bool bTemp );
                ~SfxSound_Impl()
                { delete pSound; }
};

SfxSound_Impl::SfxSound_Impl( const String& rSound, sal_Bool bTemp )
    : aSoundFile( rSound )
    , pSound( new Sound )
    , bDeleteFile( bTemp )
{
    Application::PostUserEvent( LINK( this, SfxSound_Impl, Start ) );
}

IMPL_LINK( SfxSound_Impl, Start, void*, pVoid )
{
    INetURLObject aObj;
    aObj.SetSmartProtocol( INET_PROT_FILE );
    aObj.SetSmartURL( aSoundFile );
    if ( pSound->SetSoundName( aObj.PathToFileName() ) )
    {
        pSound->SetNotifyHdl( LINK( this, SfxSound_Impl, Done ) );
        pSound->Play();
    }
    else
        delete this;
    if ( bDeleteFile )
        SfxContentHelper::Kill( aObj.GetMainURL() );
    return 0L;
}

IMPL_LINK( SfxSound_Impl, Done, void*, pVoid )
{
    if ( !pSound->IsPlaying() || pSound->GetLastError() )
        delete this;
    return 0L;
}

class SfxAsyncChaosFactory_Impl : public SfxListener
{
//(dv)  CntAnchorRef    xAnchor;
    SfxItemSet*     pArgs;

public:
                    SfxAsyncChaosFactory_Impl( /*(dv)CntAnchorRef& rAnchor,*/ SfxItemSet* pSet );
                    ~SfxAsyncChaosFactory_Impl();
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};


SfxAsyncChaosFactory_Impl::SfxAsyncChaosFactory_Impl( /*(dv)CntAnchorRef& rAnchor, */SfxItemSet* pSet )
    : /*(dv)xAnchor( &rAnchor )
    , */ pArgs( pSet )
{
    if (!pArgs)
        pArgs = new SfxAllItemSet( SFX_APP()->GetPool() );
//(dv)  StartListening( *xAnchor );
}

SfxAsyncChaosFactory_Impl::~SfxAsyncChaosFactory_Impl()
{
//(dv)  EndListening( *xAnchor );
    delete pArgs;
}

void SfxAsyncChaosFactory_Impl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
#if 0   //(dv)
    if( rHint.ISA(CntStatusHint) )
    {
        const CntStatusHint& rStatusHint = (const CntStatusHint&)rHint;
        CntStatus eStatus = rStatusHint.GetStatus();
        sal_uInt16 nWhich = rStatusHint.GetRequest() ? rStatusHint.GetRequest()->Which() : 0;
        if ( nWhich == WID_INSERT )
        {
            if( eStatus == CNT_STATUS_DONE )
            {
                pArgs->Put( SfxStringItem ( SID_FILE_NAME, xAnchor->GetViewURL() ), SID_FILE_NAME );
                SFX_APP()->GetDispatcher_Impl()->Execute( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, *pArgs );
                delete this;
            }
            else if( eStatus == CNT_STATUS_ERROR )
            {
                ErrCode nErr = rStatusHint.GetError();
                nErr &= ~(ERRCODE_DYNAMIC_MASK);
                if( nErr == ERRCODE_ABORT )
                    delete this;
            }
        }
    }
#endif  //(dv)
}

struct FlagMapping_Impl
{
    const char* pName;
    SfxFilterFlags nValue;
};

static const FlagMapping_Impl aMap[] =
{
    "Import",           SFX_FILTER_IMPORT,
    "Export",           SFX_FILTER_EXPORT,
    "Internal",         SFX_FILTER_INTERNAL,
    "Template",         SFX_FILTER_TEMPLATE,
    "TemplatePath",     SFX_FILTER_TEMPLATEPATH,
    "Own",              SFX_FILTER_OWN,
    "Alien",            SFX_FILTER_ALIEN,
    "Asynchron",        SFX_FILTER_ASYNC,
    "Readonly",         SFX_FILTER_OPENREADONLY,
    "UsesOptions",      SFX_FILTER_USESOPTIONS,
    "NotInstalled",     SFX_FILTER_MUSTINSTALL,
    "ConsultService",   SFX_FILTER_CONSULTSERVICE,
    "NotInChooser",     SFX_FILTER_NOTINCHOOSER,
    "NotInFileDialog",  SFX_FILTER_NOTINFILEDLG,
    "Packed",           SFX_FILTER_PACKED,
    "SilentExport",     SFX_FILTER_SILENTEXPORT,
    "Prefered",         SFX_FILTER_PREFERED,
    "BrowserPrefered",  SFX_FILTER_BROWSERPREFERED,
    0, 0
};

SfxFilterFlags NameToFlag_Impl( const String& aName )
/*   [Beschreibung]

     Konvertiert einen Namen aus der Install.ini in das zug. SFX_FILTER_FLAG

 */

{
    sal_uInt16 n = 0;
    for( const char* pc = aMap[0].pName; pc;
         pc = aMap[++n].pName)
        if( aName.EqualsAscii(pc) )
            return aMap[n].nValue;
    DBG_ERROR("Name nicht gefunden" );
    return 0;
}

String FlagsToName_Impl( SfxFilterFlags nValue )
/*   [Beschreibung]

     Konvertiert ein SFX_FILTER_FLAG in einen Text, der in die Install.ini
     geschrieben werden kann
 */
{
    ByteString aRet;
    sal_uInt16 n = 0;
    for( const char* pc = aMap[0].pName; pc;
         pc = aMap[++n].pName)
        if( nValue & aMap[n].nValue )
        {
            if( !aRet.Len() )
                aRet = pc;
            else
            {
                aRet+="|";
                aRet+=pc;
            }
        }

    return String(S2U(aRet));
}

//----------------------------------------------------------------

inline String ToUpper_Impl( const String &rStr )
{
    String aRet(rStr);
    Application::GetAppInternational().ToUpper(aRet);
    return aRet;
}

//----------------------------------------------------------------

struct LoadArg_Impl
{
    String aGroup;
    sal_Bool bInstallIni;
    SfxFilterFlags nOrFlags;
    SfxFilterFlags nNotFlags;
};

class SfxFilterContainer_Impl
{
public:
    SfxFilterContainer_Impl() : bLoadPending( sal_False ), pArg( 0 ) {}
    SfxFilterList_Impl aList;
    String aName;
    LoadArg_Impl* pArg;
    sal_Bool bLoadPending;
    SfxFilterContainerFlags eFlags;
};


//----------------------------------------------------------------

SfxFilterContainer::SfxFilterContainer( const String& rName )
{
    pImpl = new SfxFilterContainer_Impl;
    pImpl->eFlags = 0;
    pImpl->aName = rName;
}

//----------------------------------------------------------------

SfxFilterContainer::~SfxFilterContainer()
{
    DELETEZ( pImpl->pArg );
    SfxFilterList_Impl& rList = pImpl->aList;
    sal_uInt16 nCount = (sal_uInt16 )rList.Count();
    for( sal_uInt16 n = 0; n<nCount; n++ )
        delete rList.GetObject( n );
    delete pImpl;
}

SfxFilterContainerFlags SfxFilterContainer::GetFlags() const
{
    return pImpl->eFlags;
}

void SfxFilterContainer::SetFlags( SfxFilterContainerFlags eFlags )
{
    pImpl->eFlags = eFlags;
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterContainer::GetFilter4Protocol(
    SfxMedium& rMed, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
/*   [Beschreibung]

     Gibt den Filter zu einem bestimmten Protokoll zurueck. Diese Methode
     darf sich nicht auf Inhalte beziehen, sondern soll allein anhand von
     Protokoll / ::com::sun::star::util::URL einen Filter auswaehlen.
     In der Basisimplementierung werden einige Protokolle direkt auf
     GetFilter4FilterName abgebildet, man zur Integration des Filters
     in die Filterdetection einfach einen Filter des Protokollnames anlegen
     kann.
 */
{
    const INetURLObject& rObj = rMed.GetURLObject();
    String aName = rMed.GetURLObject().GetMainURL();
    // Hier noch pruefen, ob das Doc im Browser dargestellt werden soll
    SFX_ITEMSET_ARG( rMed.GetItemSet(), pTargetItem, SfxStringItem,
                     SID_TARGETNAME, sal_False);
    if( pTargetItem && pTargetItem->GetValue().EqualsAscii("_beamer") )
        return 0;
    return GetFilter4Protocol( aName, nMust, nDont );
}

const SfxFilter* SfxFilterContainer::GetFilter4Protocol( const String& rName, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    String aName( rName );
    ForceFilterLoad_Impl();
    aName.ToLowerAscii();
    sal_uInt16 nCount = ( sal_uInt16 ) pImpl->aList.Count();
    for( sal_uInt16 n = 0; n < nCount; n++ )
    {
        const SfxFilter* pFilter = pImpl->aList.GetObject( n );
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        String aString( pFilter->GetURLPattern());
        if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) &&
             WildCard(aString ).Matches( aName ) )
            return pFilter;
    }
    return 0;
}

//----------------------------------------------------------------

sal_Bool SfxFilterContainer::IsUsableForRedirects() const
/*   [Beschreibung]

     Alle FilterContainer, die hier stehen, werden fuer den InternetFileTypen
     Dialog benutzt (koennen dort MIME Types usw. zugeordnet werden).
 */
{
    String aName = pImpl->aName;
    // Bloed, aber erstmal notwendig. Spaeter ueber MussAenderung beim
    // Impl Makro
    return  aName.EqualsAscii("swriter") || aName.EqualsAscii("simpress") || aName.EqualsAscii("scalc") ||
        aName.EqualsAscii("simage") || aName.EqualsAscii("schart") || aName.EqualsAscii("smath") ||
        aName.EqualsAscii("PlugIn") || aName.EqualsAscii("swriter/web") || aName.EqualsAscii("sdraw") ||
        aName.EqualsAscii(SFX_FCONTNR_EXTAPP) ||
//        aName == String( SfxResId( STR_FILTER_OTHERS ) );
        aName.EqualsAscii("soffice");
}

//----------------------------------------------------------------

sal_uInt32 SfxFilterContainer::Execute( SfxMedium& rMedium, SfxFrame*& pFrame) const
{
    return ERRCODE_ABORT;
}

//----------------------------------------------------------------

sal_uInt16 SfxFilterContainer::GetFilterCount() const
{
    // Dazu muessen die Filter geladen werden
    ForceFilterLoad_Impl();
    return (sal_uInt16) pImpl->aList.Count();
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterContainer::GetFilter( sal_uInt16 nPos ) const
{
    ForceFilterLoad_Impl();
    return pImpl->aList.GetObject( nPos );
}

IMPL_STATIC_LINK( SfxFilterContainer, LoadHdl_Impl, void*, EMPTYARG )
/*   [Beschreibung]

     Handler, der ueber LateInit das echte Laden des
     SfxFilterContainers aus;loesst.  */
{
    if( pThis->pImpl->bLoadPending )
    {
        pThis->pImpl->bLoadPending = sal_False;
        pThis->RealLoad_Impl();
    }
    return 0;
}

//----------------------------------------------------------------

void SfxFilterContainer::RealLoad_Impl()
/*   [Beschreibung]

     Eigentliches Laden der Filter eines Containers aus der install.ini
 */

{
    static sal_Bool bRecurse = sal_False;

    LoadArg_Impl* pArg = pImpl->pArg;
    String aString( pArg->aGroup);
    aString+=DEFINE_CONST_UNICODE("-Filters");
    if( pArg->bInstallIni )
    {
        if( !bRecurse )
        {
            bRecurse = sal_True;
            sal_uInt32 nCount = pImpl->aList.Count();
            RealLoad_Impl();
            bRecurse = sal_False;
            // Falls wir keine lokalisierten Filter gefunden haben, nehmen wir
            // die alten
            if( pImpl->aList.Count() != nCount )
            {
//              DELETEZ( pImpl->pArg );
                return;
            }
        }
        else
        {
            aString += '-';
            aString += String::CreateFromAscii(ResMgr::GetLang());
        }
    }
    Config* pConfig = SFX_APP()->GetFilterIni();
    SfxIniManager *pMgr = SFX_INIMANAGER();
    SfxIniManager *pSubMgr = pMgr->GetSubManager();
    if( pSubMgr )
        pMgr = pSubMgr;

    if( pArg->bInstallIni )
        pConfig->SetGroup( U2S(aString) );

    sal_uInt16 nCount = pArg->bInstallIni ? pConfig->GetKeyCount() : pMgr->GetKeyCount( aString );
    String aOver( DEFINE_CONST_UNICODE(SFX_STR_OVERRIDE) );
    String aName, aLine, aUIType, aMimeType, aClipFormat, aMacType, aTypeName, aWild, aFlags, aDefaultTemplate, aUserData;
    for( sal_uInt16 n = 0; n < nCount; n++ )
    {
        aName = pArg->bInstallIni ? String(S2U(pConfig->GetKeyName( n ))) : pMgr->GetKeyName( aString, n );
        aLine = pArg->bInstallIni ? String(S2U(pConfig->ReadKey( n ))) : pMgr->ReadKey( aString, aName );
        sal_uInt16 nTokCount = aLine.GetTokenCount( ',' );
        if( nTokCount < 8 )
        {
#ifdef DBG_UTIL
            ByteString aMsg( "Falsches FilterFormat: " );
            aMsg += U2S(aLine).getStr();
            DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
            continue;
        }

#ifdef DBG_UTIL
        if( nTokCount < 10 )
        {
            ByteString aMsg( "Obsoletes FilterFormat: " );
            aMsg += U2S(aLine).getStr();
            DBG_ERRORFILE( aMsg.GetBuffer() );
        }
        static bWarned = sal_False;
        if( nTokCount > 11 && !bWarned )
        {
            bWarned = sal_True;
            ByteString aMsg( "Neueres FilterFormat: " );
            aMsg += U2S(aLine).getStr();
            DBG_ERRORFILE( aMsg.GetBuffer() );
        }
#endif
        // Override-Filter haben keinen Namen
        if( aName.Match( aOver ) >= aOver.Len() )
            aName.Erase();

        sal_uInt16 i = 0;
        if( nTokCount >= 10 )
            aUIType = aLine.GetToken( i++, ',' );
        else
            aUIType = aName;

        aMimeType = aLine.GetToken( i++, ',' );
        aClipFormat = aLine.GetToken( i++, ',' );
        aMacType = aLine.GetToken( i++, ',' );
        aTypeName = aLine.GetToken( i++, ',' );
        aWild = aLine.GetToken( i++, ',' );
        sal_uInt16 nDocIconId = aLine.GetToken( i++, ',' ).ToInt32();
        aUserData = aLine.GetToken( i++, ',' );
        sal_uInt32 nVersion = SOFFICE_FILEFORMAT_NOW;
        if( nTokCount >= 8 )
            nVersion = aLine.GetToken( i++, ',' ).ToInt32();
        aFlags = aLine.GetToken( i++, ',' );
        if( nTokCount >= 11 )
            aDefaultTemplate = aLine.GetToken( i++, ',' );
        else aDefaultTemplate.Erase();
        SfxFilterFlags nFlags = pArg->nOrFlags;
        nTokCount = aFlags.GetTokenCount('|');
        for( i = 0; i < nTokCount; i++ )
        {
            String aTok = aFlags.GetToken( i, '|' );
            nFlags |= NameToFlag_Impl( aTok );
        }

        nFlags &= ~pArg->nNotFlags;
        sal_uInt32 nClipId = 0;
        if( aClipFormat.Len() )
            nClipId = Exchange::RegisterFormatName( aClipFormat );
        SfxFilter* pFilter = new SfxFilter(
            aName, aWild, nFlags, nClipId, aMacType, aTypeName, nDocIconId,
            aMimeType, this, aUserData );
        pFilter->SetUIName( aUIType );
        pFilter->SetDefaultTemplate( aDefaultTemplate );
        if( nVersion )
            pFilter->SetVersion( nVersion );
        AddFilter( pFilter, GetFilterCount() );
    }
//    if( !bRecurse ) DELETEZ( pImpl->pArg );
}

//----------------------------------------------------------------

void SfxFilterContainer::ForceFilterLoad_Impl() const
/*   [Beschreibung]

     Erzwingt das echte Laden der Filter vor Eintritt des Lateinits
 */
{
    if( pImpl->bLoadPending )
    {
        ((SfxFilterContainer*)this)->LoadHdl_Impl(
            (SfxFilterContainer*)this ,0 );
    }
}

//----------------------------------------------------------------
/*   [Beschreibung]

     Da die meisten GetFilter4xxx Routinen in einem Container einfach eine
     Abfrage ueber alle enthaltenen Filter durchlaufen, sind diese in
     einem Makro kodiert.

     nMust sind die Flags, die gesetzt sein muessen, damit der Filter
     in Betracht gezogen wird, nDont duerfen nicht gesetzt sein.
 */

#define IMPL_CONTAINER_LOOP( aMethod, aArgType, aTest )         \
const SfxFilter* SfxFilterContainer::aMethod(                   \
    aArgType aArg, SfxFilterFlags nMust, SfxFilterFlags nDont ) const           \
{                                                               \
    ForceFilterLoad_Impl();                                     \
    sal_uInt16 nCount = ( sal_uInt16 ) pImpl->aList.Count();            \
    for( sal_uInt16 n = 0; n < nCount; n++ )                        \
    {                                                           \
        const SfxFilter* pFilter = pImpl->aList.GetObject( n ); \
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();              \
        if ( (nFlags & nMust) == nMust &&                       \
             !(nFlags & nDont ) && aTest )                      \
            return pFilter;                                     \
    }                                                           \
    return 0;                                                   \
}

/*   [Beschreibung]

     Ermitelt einen Filter nach seinem Namen. Dieser enthaelt vorangestellt
     den Namen des Containers, falls er nicht im DefaultContainer steckt.
 */

IMPL_CONTAINER_LOOP( GetFilter, const String&,
                     pFilter->GetName().CompareIgnoreCaseToAscii( aArg ) == COMPARE_EQUAL )

/*   [Beschreibung]
     Ermitelt einen Filter nach seinem Mimetypen.
 */
IMPL_CONTAINER_LOOP(
    GetFilter4Mime, const String&,
    pFilter->GetMimeType().CompareIgnoreCaseToAscii( aArg ) == COMPARE_EQUAL )

/*   [Beschreibung]
     Ermitelt einen Filter nach seinem FilterNamen. Dies ist der Name ohne
     vorangestellten Containernamen.
 */
IMPL_CONTAINER_LOOP(
    GetFilter4FilterName, const String&,
    pFilter->GetFilterName().CompareIgnoreCaseToAscii(aArg ) == COMPARE_EQUAL )

/*   [Beschreibung]
     Ermitelt einen Filter nach seiner ClipboardID. Diese steckt im Storage.
 */
IMPL_CONTAINER_LOOP(
    GetFilter4ClipBoardId, sal_uInt32, aArg && pFilter->GetFormat() == aArg )

#ifdef MAC
#define CONDITION \
    String( aArg ).Erase( 5 ) == String( pFilter->GetTypeName() ).Erase( 5 )
#else
#define CONDITION \
    aArg == pFilter->GetTypeName()
#endif
/*   [Beschreibung]
     Ermitelt einen Filter nach seinen Extended Attributes.
     Nur auf MAC und OS/2 von Interesse.
 */
IMPL_CONTAINER_LOOP(
    GetFilter4EA, const String&, CONDITION )
#undef CONDITION

/*   [Beschreibung]
     Ermitelt einen Filter nach seiner Extension.
     (2Ah = '*')
 */
IMPL_CONTAINER_LOOP(
    GetFilter4Extension, const String&,
    pFilter->GetWildcard() != String() && pFilter->GetWildcard() != DEFINE_CONST_UNICODE("*.*") && pFilter->GetWildcard() != 0x002A &&
    WildCard( ToUpper_Impl( pFilter->GetWildcard()() ), ';' ) == ToUpper_Impl( aArg ))

IMPL_CONTAINER_LOOP(
    GetFilter4UIName, const String&,
    pFilter->GetUIName() == aArg )

//----------------------------------------------------------------

const String SfxFilterContainer::GetName() const
{
    return pImpl->aName;
}

//----------------------------------------------------------------

void SfxFilterContainer::DeleteFilter( const SfxFilter* pFilter )
/*   [Beschreibung]

     Loescht einen Filter aus seinem Container und den Filter selbst.
 */
{
    ForceFilterLoad_Impl();
    pImpl->aList.Remove( (SfxFilter*)pFilter );
    delete (SfxFilter*)pFilter;
}

//----------------------------------------------------------------

void SfxFilterContainer::AddFilter( SfxFilter* pFilter, sal_uInt16 nPos )
{
/*   [Beschreibung]

     Fuegt einen Filter in einen Container ein.
 */
    ForceFilterLoad_Impl();
    if ( pImpl->pArg )
    {
        pFilter->nFormatType |= pImpl->pArg->nOrFlags;
        pFilter->nFormatType &= ~pImpl->pArg->nNotFlags;
    }

    if ( !pFilter->GetFilterName().Len() ||
         !GetFilter4FilterName( pFilter->GetFilterName() ))
        pImpl->aList.Insert( pFilter, nPos );
#if !defined(PRODUCT)
    else
        delete pFilter;
#endif
}

//----------------------------------------------------------------

void SfxFilterContainer::LoadFilters(
    const String& rGroup,  // ConfigGruppe, aus der gelesen werden soll
    sal_Bool bInstallIni,      // Falls sal_True aus install.ini lesen,
                           // sonst soffice3.ini
    SfxFilterFlags nOrFlags,       // Flags zusaetzlich zu denen in der Ini
    SfxFilterFlags nNotFlags )     // Flags, die nicht aus der ini genommen werden
/*   [Beschreibung]

     Stoesst das Laden eines FilterContainers an. Das eigentliche
     Laden erfolgt im LateInitHandler bzw. in ForceFilterLoad_Impl,
     falls vor LateInit auf den FilterContainer zugegriffen wird.  */
{
    LoadArg_Impl* pArg = new LoadArg_Impl;
    pArg->aGroup = rGroup;
    pArg->bInstallIni = bInstallIni;
    pArg->nOrFlags = nOrFlags;
    pArg->nNotFlags = nNotFlags;
    pImpl->pArg = pArg;
    SFX_APP()->InsertLateInitHdl( STATIC_LINK(
        this, SfxFilterContainer, LoadHdl_Impl ) );
    pImpl->bLoadPending = sal_True;
}

//----------------------------------------------------------------

void SfxFilterContainer::SaveFilters( const String& rGroup, SfxFilterFlags nMask ) const
/*   [Beschreibung]

     Speichert die Filter des Containers in der soffice3.ini.
 */
{
    String aString( rGroup);
    aString+=DEFINE_CONST_UNICODE("-Filters");
    SfxIniManager *pMgr = SFX_INIMANAGER();
    SfxIniManager *pSubMgr = pMgr->GetSubManager();
    if( pSubMgr )
        pMgr = pSubMgr;

    pMgr->DeleteGroup( aString );

    SfxFilterMatcher aMatcher( ( SfxFilterContainer* ) this );
    SfxFilterMatcherIter aIter( &aMatcher, nMask );
    sal_uInt16 n = 1;
    String aLine, aKey;
    for( const SfxFilter* pFilter = aIter.First(); pFilter; pFilter = aIter.Next() )
    {
        if ( pFilter->GetURLPattern().Len() )
            // Filter mit URLPattern k"onnen wir nicht einlesen, also auch nicht speichern!
            continue;

        aLine = pFilter->GetUIName();
        aLine += ',';
        aLine += pFilter->GetMimeType();
        aLine+=',';
        sal_uInt32 nFormat = pFilter->GetFormat();
        if( nFormat )
            aLine += Exchange::GetFormatName( pFilter->GetFormat() );
        aLine += ',';
        aLine += pFilter->GetMacType();
        aLine+=',';
        aLine += pFilter->GetRealTypeName();
        aLine+=',';
        aLine += pFilter->GetWildcard()();
        aLine+=',';
        aLine += String::CreateFromInt32( pFilter->GetDocIconId() );
        aLine+=',';
        aLine += pFilter->GetUserData();
        aLine+=',';
        if( pFilter->GetVersion() != SOFFICE_FILEFORMAT_NOW )
            aLine += String::CreateFromInt32( pFilter->GetVersion() );
        aLine += ',';
        aLine += FlagsToName_Impl( pFilter->GetFilterFlags() );
        aLine += ',';
        aLine += pFilter->GetDefaultTemplate();
        aKey = pFilter->GetFilterName();
        if ( !aKey.Len() )
            aKey = DEFINE_CONST_UNICODE( SFX_STR_OVERRIDE );
        aKey += String::CreateFromInt32( n++ );
        pMgr->WriteKey( aString, aKey, aLine );
    }
}

//-------------------------------------------------------------------------

sal_uInt32 SfxFilterContainer::GetFilter4Content(
    SfxMedium& rMedium, const SfxFilter** ppFilter,
    SfxFilterFlags, SfxFilterFlags ) const
{
    return 0;
}

//-------------------------------------------------------------------------

const SfxFilter* SfxExternalAppFilterContainer::GetFilter4Protocol(
    SfxMedium& rMed, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
/*   [Beschreibung]

     Diese Methode ist ueberladen, um zu verhindern, dass fuer jedes Protokoll
     ein Filter erzeugt wird (denn in dem SfxExternalAppFilterContainer ist
     jeder Filter vorhanden, so wie nach ihm gefragt wird und
     GetFilter4Protocol forwarded in der Basisimplementierung an
     GetFilter4FilterName ).
 */
{
    return 0;
}

//----------------------------------------------------------------

sal_uInt32 SfxExecutableFilterContainer::Choose_Impl( SfxMedium& rMedium ) const
{
    SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
    SfxFilterDialog *pDlg =
        new SfxFilterDialog(
            0, &rMedium, rMatcher, 0, 0 );
    const sal_Bool bOk = RET_OK == pDlg->Execute();
    if (bOk)
    {
        const SfxFilter* pFilter  = rMatcher.GetFilter4UIName(
            pDlg->GetSelectEntry() );
        delete pDlg;
        rMedium.SetFilter( pFilter );
        return ERRCODE_NONE;
    }
    delete pDlg;
    return ERRCODE_ABORT;
}

SV_DECL_VARARR( SfxExecutableFilterTypes_Impl, SfxExecutableFilterType_Impl,
                5, 5 )
SV_IMPL_VARARR( SfxExecutableFilterTypes_Impl, SfxExecutableFilterType_Impl )


void SfxExecutableFilterContainer::AddExeFilter(
    SfxFilter* pFilter, SfxExecutableFilterType_Impl eType,
    const String& rPattern )
{
    AddFilter( pFilter, GetFilterCount() );
    pTypes->Insert( eType, pTypes->Count() );
    if( rPattern.Len() )
        pFilter->SetURLPattern( rPattern );
}


#define SFX_EXE_FILTERFLAGS (SFX_FILTER_IMPORT | SFX_FILTER_NOTINFILEDLG |\
        SFX_FILTER_EXECUTABLE|SFX_FILTER_NOTINCHOOSER|SFX_FILTER_INTERNAL)

//----------------------------------------------------------------

SfxExecutableFilterContainer::SfxExecutableFilterContainer()
//    : SfxFilterContainer( String( SfxResId( STR_FILTER_OTHERS ) ) )
    : SfxFilterContainer( DEFINE_CONST_UNICODE("soffice") )
/*   [Beschreibung]

     Der SfxExecutableFilterContainer enthaelt alle Filter, die
     nicht direkt zum Laden eines Dokumentes fuehren, sondern
     statt dessen ausgefuehrt werden. Indirekt koennen durchaus
     Docs geladen werden, wenn z.B. der bugdoc Filter ein
     Dokument raussucht.
 */
{
    pTypes = new SfxExecutableFilterTypes_Impl;
    String aEmpty;
    sal_uInt16 nPos = 0;

// Was ist denn hiermit ??
#ifdef SOLAR_JAVA
    AddExeFilter( new SfxFilter( "JavaApplication", DEFINE_CONST_UNICODE("*.class"), SFX_EXE_FILTERFLAGS | SFX_FILTER_ASYNC, this ),
                    SFX_EXE_FILTER_COMPONENT, DEFINE_CONST_UNICODE("private:java/*") );
#endif
/*
    AddExeFilter( new SfxFilter( DEFINE_CONST_UNICODE("ExternBrowser"), DEFINE_CONST_UNICODE("*.htm;*.html"), SFX_EXE_FILTERFLAGS | SFX_FILTER_ASYNC, 0,
                        aEmpty, aEmpty, 0, DEFINE_CONST_UNICODE("text/html"),this, aEmpty ),
                    SFX_EXE_FILTER_EXTERNBROWSER, aEmpty );
*/
    AddExeFilter( new SfxFilter( "bookmark", DEFINE_CONST_UNICODE("*.*"), SFX_EXE_FILTERFLAGS, this ),
                    SFX_EXE_FILTER_BOOKMARK, aEmpty );
    AddExeFilter( new SfxFilter( "link", DEFINE_CONST_UNICODE("*.lnk"), SFX_EXE_FILTERFLAGS, this ),
                    SFX_EXE_FILTER_BOOKMARK, aEmpty );
    AddExeFilter( new SfxFilter( SFX_FILTERNAME_HELPENTRY, DEFINE_CONST_UNICODE("*.svh"), SFX_EXE_FILTERFLAGS, this ),
                    SFX_EXE_FILTER_HELPENTRY, aEmpty );
    AddExeFilter( new SfxFilter( "helpid", aEmpty, SFX_EXE_FILTERFLAGS, this ),
                      SFX_EXE_FILTER_HELPID, DEFINE_CONST_UNICODE("private:helpid/*") );
    AddExeFilter( new SfxFilter( "bugid", aEmpty, SFX_EXE_FILTERFLAGS, this),
                      SFX_EXE_FILTER_BUGID, DEFINE_CONST_UNICODE("bugid:*") );
    AddExeFilter( new SfxFilter( "mailto", aEmpty, SFX_EXE_FILTERFLAGS, this),
                     SFX_EXE_FILTER_MAILTO, DEFINE_CONST_UNICODE("mailto:*") );
    AddExeFilter( new SfxFilter( "command", aEmpty, SFX_EXE_FILTERFLAGS, this),
                     SFX_EXE_FILTER_COMMAND, DEFINE_CONST_UNICODE("file:///{*}/*") );
    AddExeFilter( new SfxFilter( "macro", aEmpty, SFX_EXE_FILTERFLAGS, this),
                      SFX_EXE_FILTER_MACRO,  DEFINE_CONST_UNICODE("macro:*") );
    AddExeFilter( new SfxFilter( "uno", aEmpty, SFX_EXE_FILTERFLAGS, this),
                      SFX_EXE_FILTER_UNO, DEFINE_CONST_UNICODE(".uno:*") );
    AddExeFilter( new SfxFilter( "slot", aEmpty, SFX_EXE_FILTERFLAGS, this),
                      SFX_EXE_FILTER_SLOT, DEFINE_CONST_UNICODE("slot:*") );
    AddExeFilter( new SfxFilter( "factory", aEmpty, SFX_EXE_FILTERFLAGS, this),
                      SFX_EXE_FILTER_FACTORY, DEFINE_CONST_UNICODE("private:factory/*") );
    AddExeFilter( new SfxFilter( "bugdoc", aEmpty, SFX_EXE_FILTERFLAGS, this),
                      SFX_EXE_FILTER_BUGDOC, DEFINE_CONST_UNICODE("bugdoc:*") );
    AddExeFilter( new SfxFilter( "exe", aEmpty, SFX_EXE_FILTERFLAGS, this),
                      SFX_EXE_FILTER_EXECUTE, DEFINE_CONST_UNICODE("internal") );
    AddExeFilter( new SfxFilter( DEFINE_CONST_UNICODE(EXPLORER_URL_FILTER), aEmpty, SFX_EXE_FILTERFLAGS, 0, aEmpty, aEmpty, 0, aEmpty, this, aEmpty),
                    SFX_EXE_FILTER_EXPLORER, aEmpty );
    AddExeFilter( new SfxFilter( "sound", DEFINE_CONST_UNICODE("*.wav;*.au"), SFX_EXE_FILTERFLAGS, this),
                      SFX_EXE_FILTER_SOUND, DEFINE_CONST_UNICODE("internal") );
/*
    AddExeFilter( new SfxFilter( SFX_FILTER_DOWNLOAD, DEFINE_CONST_UNICODE("*.*"), SFX_FILTER_IMPORT | SFX_FILTER_NOTINFILEDLG | SFX_FILTER_EXECUTABLE | SFX_FILTER_ASYNC, this),
                      SFX_EXE_FILTER_DOWNLOAD, aEmpty );
*/
    // Und nun die Filter, die auch im Dateitypendialog sichtbar sein sollen
    SfxFilter *pFilter = new SfxFilter( DEFINE_CONST_UNICODE("choose"), DEFINE_CONST_UNICODE("*.*"),
        SFX_FILTER_IMPORT | SFX_FILTER_NOTINFILEDLG | SFX_FILTER_EXECUTABLE| SFX_FILTER_NOTINCHOOSER, 0, aEmpty, aEmpty, 0, aEmpty, this, aEmpty);
    pFilter->SetUIName( SfxResId( STR_FILTER_CHOOSER ) );
    AddExeFilter( pFilter, SFX_EXE_FILTER_CHOOSER, aEmpty);

    pFilter = new SfxFilter( "extern", DEFINE_CONST_UNICODE("*.*"), SFX_FILTER_IMPORT | SFX_FILTER_EXECUTABLE, this );
    pFilter->SetUIName( SfxResId( STR_FILTER_EXTERNAL ) );
    AddExeFilter( pFilter, SFX_EXE_FILTER_EXECUTE, DEFINE_CONST_UNICODE("external") );

    pFilter = new SfxFilter( "component", DEFINE_CONST_UNICODE("*.*"), SFX_FILTER_IMPORT | SFX_FILTER_EXECUTABLE | SFX_FILTER_ASYNC, this );
    pFilter->SetUIName( SfxResId( STR_FILTER_COMPONENT ) );
    AddExeFilter( pFilter, SFX_EXE_FILTER_COMPONENT, aEmpty );
}

SfxExecutableFilterContainer::~SfxExecutableFilterContainer()
{
    delete pTypes;
}

sal_Bool SfxIsHelpEntryURL( const String &rURL, const String &rExtMask )
{
    INetURLObject aURL( rURL );
    if ( aURL.GetProtocol() == INET_PROT_FILE && aURL.GetMainURL().Len() > 8 )
    {
        String aExt = aURL.getExtension().ToLowerAscii();
        INetURLObject aHelpDir( SFX_INIMANAGER()->Get( SFX_KEY_HELP_DIR ), INET_PROT_FILE );
        if ( WildCard( aHelpDir.GetMainURL() ).Matches( rURL ) && WildCard( rExtMask ).Matches( aExt ) )
            return sal_True;
    }
    return sal_False;
}

const SfxFilter* SfxExecutableFilterContainer::GetFilter4Protocol(
    SfxMedium& rMed, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
/*   [Beschreibung]
 */
{
    INetProtocol eProt;
    String aName;
    const INetURLObject &rURL = rMed.GetURLObject();
    aName = rURL.GetMainURL();
    eProt = rURL.GetProtocol();

    // ::com::sun::star::util::URL within Help-Directory?
    if ( eProt == INET_PROT_FILE && SfxIsHelpEntryURL( aName, DEFINE_CONST_UNICODE("htm*") ) )
        return SfxFilterContainer::GetFilter4FilterName( DEFINE_CONST_UNICODE(SFX_FILTERNAME_HELPENTRY), nMust, nDont );

    return SfxFilterContainer::GetFilter4Protocol( rMed, nMust, nDont );
}

const SfxFilter* SfxExecutableFilterContainer::CheckForFolder( SfxMedium& rMed, SfxFilterFlags nMust, SfxFilterFlags nDont )
{
    INetProtocol eProt;
    String aName;
    const INetURLObject &rURL = rMed.GetURLObject();
    aName = rURL.GetMainURL();
    eProt = rURL.GetProtocol();

    Reference < XContent > xContent( rMed.GetContent() );
    FASTBOOL bHandled = sal_False;
    sal_Bool bTest = sal_True;

    if ( INET_PROT_FTP == eProt && SvBinding::ShouldUseFtpProxy( aName ) )
        bTest = sal_False;

    if ( bTest )
    {
        if ( xContent.is() )
        {
            Any aAny( UCB_Helper::GetProperty( xContent, WID_FLAG_IS_FOLDER ) );
            Any aNotherAny( UCB_Helper::GetProperty( xContent, WID_FLAG_IS_DOCUMENT ) );
            sal_Bool bIsDocument = TRUE;
            sal_Bool bIsFolder = FALSE;
            aAny >>= bIsFolder;
            aNotherAny >>= bIsDocument;
            bHandled = bIsFolder && !bIsDocument;
        }
    }

    return bHandled ? GetExplorerFilter() : NULL;
}

//----------------------------------------------------------------

const SfxFilter* SfxExecutableFilterContainer::GetDownloadFilter()
{
    String aName( DEFINE_CONST_UNICODE(SFX_FILTER_DOWNLOAD) );
    return SFX_APP()->GetFilterMatcher().GetFilter4FilterName( aName );
}

const SfxFilter* SfxExecutableFilterContainer::GetComponentFilter()
{
    String aName( DEFINE_CONST_UNICODE(SFX_FILTERNAME_COMPONENT) );
    return SFX_APP()->GetFilterMatcher().GetContainer( DEFINE_CONST_UNICODE("soffice") )->GetFilter4FilterName( aName );
}

//----------------------------------------------------------------

const SfxFilter* SfxExecutableFilterContainer::GetChooserFilter()
{
//    String aName( SfxResId( STR_FILTER_OTHERS ) );
    String aName( DEFINE_CONST_UNICODE("soffice") );
    aName+=DEFINE_CONST_UNICODE(": ");
    aName+=DEFINE_CONST_UNICODE("choose");
//    aName+=String( SfxResId( STR_FILTER_CHOOSER ) );
    return SFX_APP()->GetFilterMatcher().GetFilter( aName );
}

const SfxFilter* SfxExecutableFilterContainer::GetExplorerFilter()
{
    String aName( DEFINE_CONST_UNICODE(EXPLORER_URL_FILTER) );
    return SFX_APP()->GetFilterMatcher().GetFilter4FilterName( aName );
}

//----------------------------------------------------------------

String SfxExecutableFilterContainer::GetBugdocName_Impl(
    const String& rName ) const
/*   [Beschreibung]

     Sucht ein Bugdoc in den ueblichen Verzeichnissen
 */
{
/*! (pb)
    static const char* pNames[] =
    {
        "q:\\sw\\bugdoc",
        "q:\\sd\\bugdoc",
        "q:\\sc\\bugdoc",
        "q:\\sch\\bugdoc",
        "q:\\solar\\bugdoc",
        "q:\\bugdoc",
        0
    };
    sal_uInt32 nNumber = rName.ToInt32();
    String aMatch = rName;
    aMatch += '*';
    sal_uInt16 n = 1;
    const char* pName = pNames[ 0 ];
    while( pName )
    {
        DirEntry aEntry( String::CreateFromAscii(pName) );
        for( sal_uInt32 nBase = ( nNumber / 500 + 1 ) * 500;
             nBase - nNumber < 5000; nBase+=500 )
        {
            DirEntry aAkt( aEntry );
            String aBis( DEFINE_CONST_UNICODE("bis") );
            aBis += String::CreateFromInt32( nBase );
            aAkt += DirEntry( aBis );
            if( aAkt.Exists() )
            {
                aAkt += DirEntry( aMatch );
                Dir aDir( aAkt );
                if( aDir.Count() )
                    return aDir[ 0 ].GetFull();
            }
        }
        pName = pNames[ n++ ];
    }
*/
    return String();
}

//----------------------------------------------------------------

sal_uInt32 SfxExecutableFilterContainer::Execute(
    SfxMedium& rMedium, SfxFrame*& rpFrame ) const
/*   [Beschreibung]

     Fuehrt die Funktionalitaet eines mit SFX_FILTER_EXECUTABLE registrierten
     Filters aus.
 */
{
    SfxApplication* pApp = SFX_APP();
    const SfxFilter* pFilter = rMedium.GetFilter();
    DBG_ASSERT( pFilter, "Medium braucht Filter" );

    // Referer und TargetPath zum Speichern
    SfxStringItem aReferer( SID_REFERER, String() );
    SfxStringItem aPath( SID_TARGETPATH, String() );

    // Wurden welche mitgeschickt ?
    SFX_ITEMSET_ARG(rMedium.GetItemSet(), pReferer, SfxStringItem, SID_REFERER, sal_False);
    SFX_ITEMSET_ARG(rMedium.GetItemSet(), pPath, SfxStringItem, SID_TARGETPATH, sal_False);
    if ( !pReferer )
        pReferer = &aReferer;
    if ( !pPath )
    {
        // Wenn kein TargetPath, dann diesen mit dem Referer belegen
        if ( pReferer )
            aPath.SetValue( pReferer->GetValue() );
        rMedium.GetItemSet()->Put( aPath, SID_TARGETPATH );
        pPath = &aPath;
    }

    SfxFrameItem aFrameItem( SID_DOCFRAME, rpFrame );
    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pTarget, SfxStringItem,
                     SID_TARGETPATH, sal_False);

    sal_Bool bPreview = rMedium.IsPreview_Impl();
    const SfxPoolItem* ppInternalArgs[] =
    {
        &aFrameItem, pReferer, pTarget, 0
    };

    SfxExecutableFilterType_Impl eType;
    for( sal_uInt16 nPos = GetFilterCount(); nPos--; )
        if( GetFilter( nPos ) == pFilter )
        {
            eType = pTypes->GetObject( nPos );
            break;
        }
    switch( eType )
    {
        case SFX_EXE_FILTER_SOUND:
        {
            // Achtung: GetPhysicalName nicht im ctor, IsTemporary() k"onnte vorher aufgerufen werden !!
            String aName = rMedium.GetPhysicalName();
            new SfxSound_Impl( aName, rMedium.IsTemporary() );
            rMedium.SetTemporary( sal_False );
            return ERRCODE_ABORT;
            break;
        }

        case SFX_EXE_FILTER_COMMAND:
        {
            if ( bPreview )
                return ERRCODE_ABORT;

#if 0   //(dv)
            CntAnchorRef xAnchor = rMedium.GetAnchor(); // MI: ???
            if ( !xAnchor.Is() )
                return ERRCODE_IO_UNKNOWN; // MI: kann das ueberhaupt passieren?
#ifndef TF_UCB
            xAnchor->Put( CntCmpCommandItem( WID_COMPONENT_COMMAND, ::rtl::OUString("executeDefaultCommand") ) );
#else
            xAnchor->Put( CntCmpCommandItem( WID_COMPONENT_COMMAND, "executeDefaultCommand" ) );
#endif
#endif  //(dv)
            return ERRCODE_ABORT;
            break;
        }

        case SFX_EXE_FILTER_EXPLORER:
        {
            if ( bPreview )
                return ERRCODE_ABORT;

            // ansonsten als Komponente laden
        }

/*
        case SFX_EXE_FILTER_COMPONENT:
        {
            rMedium.Close();

            const INetURLObject& rURLObj = rMedium.GetURLObject();
            String aName( rURLObj.GetMainURL() );
            SfxURLTransformer aTrans ( ::utl::getProcessServiceFactory() );
            ::rtl::OUString aTargetURL = aName;

            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
            TransformItems( SID_OPENDOC, *rMedium.GetItemSet(), aArgs );

            sal_uInt16 nRet = rpFrame->LoadComponent_Impl( aTargetURL, aArgs, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrameLoader > (), rMedium.GetItemSet() );
            if ( nRet == RET_NEWTASK )
                rMedium.GetItemSet()->Put( SfxStringItem( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") ) );

            if ( nRet == RET_OK || nRet == RET_CANCEL )
                return ERRCODE_ABORT;
            else if ( nRet == RET_NEWTASK )
                return ERRCODE_SFX_RESTART;
            else                        // nRet == RET_PENDING !
                return ERRCODE_IO_PENDING;
            break;
        }

        case SFX_EXE_FILTER_EXTERNBROWSER:
        {
            String aExtBrw( SfxIniManager::Get()->Get( SFX_KEY_EXTBRW_FILE ) );
            if ( !rpFrame->IsTop() && aExtBrw.Len() )
            {
                // external Browser in subframes only possible if IExplorer
                const SfxFilter* pFilter = SFX_APP()->GetFilterMatcher().GetFilter4Mime( DEFINE_CONST_UNICODE("text/html"), SFX_FILTER_IMPORT | SFX_FILTER_EXPORT );
                rMedium.SetFilter(pFilter);

                return pFilter ? ERRCODE_SFX_RESTART : ERRCODE_IO_NOTSUPPORTED;
            }

            rMedium.Close();

            // put filter into itemset, otherwise mediums filter will become component filter!
            rMedium.GetItemSet()->Put( SfxStringItem( SID_FILTER_NAME, pFilter->GetName() ) );

            const INetURLObject& rURLObj = rMedium.GetURLObject();
            INetProtocol eProt = rURLObj.GetProtocol();
            String aName;
            if ( eProt != INET_PROT_FILE && eProt != INET_PROT_HTTP && eProt != INET_PROT_HTTPS && eProt != INET_PROT_FTP )
            {
                aName = rMedium.GetPhysicalName();
                rMedium.SetTemporary( sal_False );
            }
            else
                aName = rURLObj.GetMainURL();

            if ( aExtBrw.Len() )
            {
                String aFile( '\"');
                aFile += aExtBrw;
                aFile += '\"';
                aFile += ' ';
                aFile += '\"';
                aFile += aName;
                aFile += '\"';
                sal_Bool bOk = System::StartProcess( Application::GetAppWindow(), aFile );
                return bOk ? ERRCODE_ABORT : ERRCODE_IO_NOTSUPPORTED;
            }

            SfxURLTransformer aTrans ( ::utl::getProcessServiceFactory() );
            ::rtl::OUString aTargetURL = aName;

            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
            TransformItems( SID_OPENDOC, *rMedium.GetItemSet(), aArgs );

            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMgr( ::utl::getProcessServiceFactory() );
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xReg( xMgr->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.FrameLoaderFactory") ), ::com::sun::star::uno::UNO_QUERY );
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrameLoader >  xLoader( xReg->createInstance( DEFINE_CONST_UNICODE("private:iexplorer") ), ::com::sun::star::uno::UNO_QUERY );

            sal_uInt16 nRet = rpFrame->LoadComponent_Impl( aTargetURL, aArgs, xLoader, rMedium.GetItemSet() );
            if ( nRet == RET_NEWTASK )
                rMedium.GetItemSet()->Put( SfxStringItem( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") ) );

            if ( nRet == RET_OK || nRet == RET_CANCEL )
                return ERRCODE_ABORT;
            else if ( nRet == RET_NEWTASK )
                return ERRCODE_SFX_RESTART;
            else                        // nRet == RET_PENDING !
                return ERRCODE_IO_PENDING;
            break;
        }
*/
/*
        case SFX_EXE_FILTER_DOWNLOAD:
        {
            if ( bPreview )
                return ERRCODE_ABORT;

            rMedium.Close();

            const INetURLObject& rURLObj = rMedium.GetURLObject();
            String aName( rURLObj.GetMainURL() );
            ::rtl::OUString aTargetURL = aName;


            // Downloader always uses his own special task
            SfxFrame *pFrame = SfxTopFrame::Create();
            rMedium.GetItemSet()->Put( SfxRectangleItem( SID_VIEW_POS_SIZE, Rectangle( 0, 0, 500, 250 ) ) );

            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
            TransformItems( SID_OPENDOC, *rMedium.GetItemSet(), aArgs );
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMgr( ::utl::getProcessServiceFactory() );
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrameLoader >  xFrameLoader ( xMgr->createInstance( DEFINE_CONST_UNICODE("com.sun.star.comp.sfx2.DownloaderLoader") ), ::com::sun::star::uno::UNO_QUERY );
            if ( xFrameLoader.is() )
            {
                USHORT nRet = pFrame->LoadComponent_Impl( aTargetURL, aArgs, xFrameLoader, rMedium.GetItemSet() );
                if ( nRet == RET_OK )
                    pFrame->GetCurrentDocument()->SetFlags( pFrame->GetCurrentDocument()->GetFlags() | SFXOBJECTSHELL_DONTREPLACE );
            }

            return ERRCODE_ABORT;
            break;
        }
 */
        case SFX_EXE_FILTER_CHOOSER:
            if ( bPreview )
                return ERRCODE_ABORT;
            return Choose_Impl( rMedium ); break;
        case SFX_EXE_FILTER_MAILTO:
        {
            if ( bPreview )
                return ERRCODE_ABORT;

            // an SID_MAIL_SENDDOC forwarden
            SfxAllItemSet aSet( pApp->GetPool() );
            aSet.Put( SfxStringItem( SID_MAIL_RECIPIENT, rMedium.GetURLObject().GetMainURL() ) );
            aSet.Put( SfxBoolItem( SID_MAIL_ATTACH, sal_False ) );

            String aRefererStr = pReferer ? pReferer->GetValue() : String();
            if ( aRefererStr.CompareToAscii("private:msgid/", 14 ) != COMPARE_EQUAL )
                aSet.Put( aFrameItem );

            // look for subject and body
            SfxItemSet* pSet = rMedium.GetItemSet();
            if( pSet )
            {
                SFX_ITEMSET_ARG( pSet, pSendDirect, SfxBoolItem, SID_MAIL_SEND, sal_False);
                if ( pSendDirect )
                    aSet.Put( *pSendDirect );

                SFX_ITEMSET_ARG( pSet, pToPost, SfxStringItem, SID_POSTSTRING, sal_False);
                if ( pToPost )
                {
                    aSet.Put( SfxStringItem( SID_MAIL_TEXT, pToPost->GetValue() ) );

                    // Subject nur setzen, wenn nicht in mailto-::com::sun::star::util::URL
                    String aParam = rMedium.GetURLObject().GetParam().ToLowerAscii();
                    FASTBOOL bSubjectFound = sal_False;
                    sal_uInt16 nIdx = 0;
                    while ( nIdx != STRING_NOTFOUND )
                    {
                        String aParamTok = aParam.GetToken( 0, '&', nIdx );
                        aParamTok.EraseLeadingChars();
                        if ( aParamTok.SearchAscii( "subject=" ) == 0 )
                        {
                            bSubjectFound = sal_True;
                            break;
                        }
                    }

                    if ( !bSubjectFound )
                        aSet.Put( SfxStringItem( SID_MAIL_SUBJECT, DEFINE_CONST_UNICODE("Mail Posted by StarOffice") ) );
                }
            }

            if ( rpFrame->GetCurrentDocument() || !rpFrame->IsTop() )
            {
                SfxFrame *pFrame = SfxTopFrame::Create();
                aSet.Put( SfxFrameItem( SID_DOCFRAME, pFrame ) );
            }

            pApp->GetDispatcher_Impl()->Execute( SID_MAIL_SENDDOC, SFX_CALLMODE_SYNCHRON, aSet );
            return ERRCODE_ABORT;
        }
        case SFX_EXE_FILTER_HELPENTRY:
        {
            rMedium.Close();

            // Redirection within Help-Directory
            INetURLObject aObj( rMedium.GetName() );
            if ( aObj.getExtension().CompareIgnoreCaseToAscii( "svh" ) == COMPARE_EQUAL )
            {
                SfxHelp::ShowHelp( HELP_INDEX, sal_False, U2S( aObj.PathToFileName() ).getStr() );
                return ERRCODE_ABORT;
            }
            else
            {
                INetURLObject aHelpDir( SFX_INIMANAGER()->Get(SFX_KEY_HELP_DIR ), INET_PROT_FILE );
                aHelpDir.setFinalSlash();
                String aNewURL = DEFINE_CONST_UNICODE(".component:Help/Content.Contents;\001");
                aNewURL += aObj.getBase();
                aNewURL += DEFINE_CONST_UNICODE("\001");
                aNewURL += aHelpDir.GetMainURL();
                aNewURL += String::CreateFromAscii( ResMgr::GetLang() );
                aNewURL += DEFINE_CONST_UNICODE("/text/sbasic/starone/ref/");
                aNewURL += aObj.getName();
                aNewURL += DEFINE_CONST_UNICODE("\001.Content");
                rMedium.SetName( aNewURL, sal_True );
                rMedium.SetPhysicalName( String() );
                rMedium.Init_Impl();
                rMedium.SetFilter(0);
                return ERRCODE_SFX_RESTART;
            }
        }
/*
        case SFX_EXE_FILTER_BOOKMARK:
        {
            rMedium.Close();
            String aTitle, aFrame, aOpen, aWorkDir, aArguments, aURL;

            Reference < XContent > xContent( rMedium.GetContent() );
            Any aAny( UCB_Helper::GetProperty( xContent, WID_TARGET_URL ) );
            OUString aTarget;
            if ( !( aAny >>= aTarget ) || !aTarget.len() )
                return ERRCODE_SFX_INVALIDLINK;
            else
                aURL = String(aTarget);

            aAny = UCB_Helper::GetProperty( xContent, WID_DOCUMENT_HEADER );
            Sequence < DocumentHeaderField > aSeq;
            if ( aAny >>= aSeq )
            {
                USHORT nCount = aSeq.getLength();
                for ( USHORT n = 0; n < nCount; ++n )
                {
                    String aName( aSeq.getArray()[ n ].Name );
                    String aValue( aSeq.getArray()[ n ].Value );
                    if ( aName.ToLowerAscii().EqualsAscii("title") )
                        aTitle = aValue;

                    if ( aName.ToLowerAscii().EqualsAscii( "workingdirectory" ) )
                    {
                        aWorkDir = aValue;
                        if ( aWorkDir.Len() )
                        {
                            INetURLObject aObj( aWorkDir, INET_PROT_FILE );
                            aWorkDir = aObj.PathToFileName();
                        }
                    }

                    if ( aName.ToLowerAscii().EqualsAscii( "arguments" ) )
                        aArguments = aValue;

                    if ( aName.EqualsAscii( "frame" ) )
                        aFrame = aValue;

                    if ( aName.EqualsAscii( "openas" ) )
                        aOpen = aValue;
                }
            }

            INetURLObject aPresObj( rMedium.GetName() );
            sal_Bool bIsURLFile = aPresObj.GetExtension().EqualsAscii( "url" );
            if ( aArguments.Len() || aWorkDir.Len() )
            {
                if ( bPreview )
                    return ERRCODE_ABORT;

                String aName( aURL );
                Any aAny( UCB_Helper::GetProperty( aName, WID_FLAG_IS_FOLDER ) );
                BOOL bIsFolder = FALSE;
                if ( !( aAny >>= bIsFolder ) || !bIsFolder )
                {
                    INetURLObject aFake( "macro:" );
                    if ( pApp->IsSecureURL( aFake, &rMedium.GetReferer() ) )
                    {
                        String aName;
                        if ( bIsURLFile )
                        {
                            INetURLObject aObj;
                            aObj.SetSmartProtocol( INET_PROT_FILE );
                            aObj.SetSmartURL( aURL );
                            aName += '\"';
                            aName += aObj.PathToFileName();
                            aName += '\"';

                            if ( aArguments.Len() )
                            {
                                aName += ' ';
                                // Don't quote the arguments because StartProcess doesn't know which
                                // quotes should be removed and which not. If the argument needs to
                                // be quoted it has to be quoted in the URL file itself
                                aName += aArguments;
                            }
                        }
                        else if ( rMedium.GetPhysicalName().Len() )
                        {
                            // Bei propriet"aren Formaten lassen wir lieber das BS die Arbeit tun
                            aName += '\"';
                            aName += rMedium.GetPhysicalName();
                            aName += '\"';
                        }

                        sal_Bool bOK = sal_False;
                        if ( aWorkDir.Len() )
                            bOK = System::StartProcess( NULL, aName, aWorkDir );
                        else
                            bOK = System::StartProcess( NULL, aName );
                        return bOK ? ERRCODE_ABORT : ERRCODE_IO_NOTSUPPORTED;
                    }
                }
            }

            if ( aOpen.EqualsAscii(INTERNETSHORTCUT_TEMPLATE_TAG) )
                rMedium.GetItemSet()->Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );

            if ( aFrame.Len() )
                rMedium.GetItemSet()->Put( SfxStringItem( SID_TARGETNAME, aFrame ) );
            rMedium.GetItemSet()->Put( SfxStringItem( SID_REFERER, rMedium.GetName() ) );
            rMedium.GetItemSet()->Put( aPath, SID_TARGETPATH );
            rMedium.SetName( aURL, sal_True );
            rMedium.SetOrigFilter_Impl( 0 );
            rMedium.SetPhysicalName( String() );
            rMedium.Init_Impl();
            rMedium.SetFilter( 0 );

            SFX_ITEMSET_ARG( rMedium.GetItemSet(), pROItem, SfxBoolItem, SID_DOC_READONLY, sal_False);
            sal_Bool bReadOnly = pROItem ? pROItem->GetValue() : sal_False;
            rMedium.SetOpenMode( bReadOnly ? SFX_STREAM_READONLY : SFX_STREAM_READWRITE, sal_False );

            return ERRCODE_SFX_RESTART;
        }

        case SFX_EXE_FILTER_HELPID:
        {
            if ( bPreview )
                return ERRCODE_ABORT;

            rMedium.Close();

#ifdef WNT
            String aName( rMedium.GetName() );
            if ( aName.GetTokenCount( '/' ) == 2 )
            {
                String aHelpFile( aName.GetToken( 1, '/' ) );
                if ( WildCard( DEFINE_CONST_UNICODE("*.chm") ).Matches( aHelpFile ) )
                {
                    String aHelp( DEFINE_CONST_UNICODE("hh.exe ") );
                    aHelp += aHelpFile;
                    System::StartProcess( NULL, aHelp );
                }
            }
#endif

            String aHID( rMedium.GetURLObject().GetMark() );
            String aFileName;
            sal_uInt32 nHID;
            if ( aHID.Search( ':' ) != STRING_NOTFOUND )
            {
                aFileName = aHID.GetToken( 0, ':' );
                nHID = aHID.GetToken( 1, ':' ).ToInt32();
            }
            else
            {
                nHID = aHID.ToInt32();
            }
            SfxHelp::ShowHelp( nHID, sal_False, aFileName.Len() ? U2S(aFileName).getStr() : NULL );
            return ERRCODE_ABORT;
        }

        case SFX_EXE_FILTER_EXECUTE:
        {
            if ( bPreview )
                return ERRCODE_ABORT;

            rMedium.SetTemporary( sal_False );
            INetURLObject aURL( "macro:" );
            if ( pApp->IsSecureURL( aURL, &rMedium.GetReferer() ) && rMedium.GetPhysicalName().Len() )
            {
                String aName( '\"');
                aName += rMedium.GetPhysicalName();
                aName += '\"';
                rMedium.SetTemporary( sal_False );
                rMedium.CloseInStream();
                sal_Bool bOk = System::StartProcess( NULL, aName );
                return bOk ? ERRCODE_ABORT : ERRCODE_IO_NOTEXISTS;
            }
            return ERRCODE_IO_ACCESSDENIED;
        }
 */
        case SFX_EXE_FILTER_BUGID:
        {
            String aPathName = DEFINE_CONST_UNICODE("http://webserver1.stardiv.de/Bugtracker/Source/Body_ReportDetail.asp?ID=");
            aPathName += rMedium.GetURLObject().GetURLPath();
            rMedium.SetName( aPathName );
            rMedium.SetPhysicalName( String() );
            rMedium.Init_Impl();
            rMedium.SetFilter( 0 );
            return ERRCODE_SFX_RESTART;
        }

        case SFX_EXE_FILTER_BUGDOC:
        {
            String aPathName = GetBugdocName_Impl(
                rMedium.GetURLObject().GetURLPath() );
            if( aPathName.Len() )
            {
                rMedium.SetName( aPathName );
                rMedium.SetPhysicalName( String() );
                rMedium.Init_Impl();
                rMedium.SetFilter( 0 );
                return ERRCODE_SFX_RESTART;
            }
            else return ERRCODE_SFX_INVALIDLINK;
        }
        case SFX_EXE_FILTER_MACRO:
        {
            if ( bPreview )
                return ERRCODE_ABORT;

            pApp->EnterBasicCall();

            // macro:-::com::sun::star::util::URL analysiern
            // 'macro://#lib.mod.proc(args)' => Macro via App-BASIC-Mgr
            // 'macro:#lib.mod.proc(args)' => Macro via zugehoerigen Doc-BASIC-Mgr
            // 'macro:obj.method(args)' => Object via App-BASIC-Mgr
            String aMacro = rMedium.GetURLObject().GetMainURL();
            sal_uInt16 nHashPos = aMacro.Search( '#' );
            sal_uInt16 nArgsPos = aMacro.Search( '(' );
            BasicManager *pBasMgr = 0;
            ErrCode nErr;

            // wird Macro angesprochen (also KEIN Object)?
            if ( STRING_NOTFOUND != nHashPos && nHashPos < nArgsPos )
            {
                // BasManager ermitteln
                String aBasMgrName( INetURLObject::decode(aMacro.Copy( 6, nHashPos-6 ), INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET) );
                if ( aBasMgrName.EqualsAscii("//") )
                    pBasMgr = pApp->GetBasicManager();
                else if ( !aBasMgrName.Len() )
                    pBasMgr = SfxObjectShell::Current()->GetBasicManager();
                else
                    for ( SfxObjectShell *pObjSh = SfxObjectShell::GetFirst();
                          pObjSh && !pBasMgr;
                          pObjSh = SfxObjectShell::GetNext(*pObjSh) )
                        if ( aBasMgrName == pObjSh->GetTitle(SFX_TITLE_APINAME) )
                            pBasMgr = pObjSh->GetBasicManager();
                if ( pBasMgr )
                {
                    // Funktion suchen
                    String aQualifiedMethod( INetURLObject::decode(aMacro.Copy( nHashPos+1 ), INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET) );
                    String aArgs;
                    if ( STRING_NOTFOUND != nArgsPos )
                    {
                        aArgs = aQualifiedMethod.Copy( nArgsPos - nHashPos - 1 );
                        aQualifiedMethod.Erase( nArgsPos - nHashPos - 1 );
                    }
                    SbxMethod *pMethod = SfxQueryMacro( pBasMgr, aQualifiedMethod );

                    // falls gefunden Funktion ueber ihren Parent ausfuehren
                    if ( pMethod )
                    {
                        String aQuotedArgs;
                        if ( aArgs.Len()<2 || aArgs.GetBuffer()[1] == '\"')
                            aQuotedArgs = aArgs;
                        else
                        {
                            // Klammern entfernen
                            aArgs.Erase(0,1);
                            aArgs.Erase( aArgs.Len()-1,1);

                            aQuotedArgs = '(';

                            // Alle Parameter mit T"uddelchen
                            sal_uInt16 nCount = aArgs.GetTokenCount(',');
                            for ( sal_uInt16 n=0; n<nCount; n++ )
                            {
                                aQuotedArgs += '\"';
                                aQuotedArgs += aArgs.GetToken( n, ',' );
                                aQuotedArgs += '\"';
                                if ( n<nCount-1 )
                                    aQuotedArgs += ',';
                            }

                            aQuotedArgs += ')';
                        }

                        String aCall( '[' );
                        aCall += pMethod->GetName();
                        aCall += aQuotedArgs;
                        aCall += ']';
                        pMethod->GetParent()->Execute( aCall );
                        nErr = SbxBase::GetError();
                    }
                    else
                        nErr = ERRCODE_BASIC_PROC_UNDEFINED;
                }
                else
                    nErr = ERRCODE_IO_NOTEXISTS;
            }
            else
            {
                // (optional Objekt-qualifizierte) Basic-Funktion ausfuehren
                String aCall( '[' );
                aCall += INetURLObject::decode(aMacro.Copy(6), INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET);
                aCall += ']';
                pApp->GetBasicManager()->GetLib(0)->Execute( aCall );
                nErr = SbxBase::GetError();
            }

            pApp->LeaveBasicCall();
            SbxBase::ResetError();
            return nErr ? nErr : ERRCODE_ABORT;
        }
/*
        case SFX_EXE_FILTER_FACTORY:
        {
            INetURLObject aObj( rMedium.GetURLObject() );
            String aParam;
            if ( aObj.HasParam() && (sal_uInt16) aObj.GetParam().ToInt32() )
            {
                aParam = aObj.GetParam();
                aObj.SetParam("");
            }

            String aPathName( aObj.GetMainURL() );
            const SfxObjectFactory* pFact = SfxObjectFactory::GetFactory( aPathName );
            if ( pFact )
            {
                if ( aParam.Len() )
                {
                    sal_uInt16 nSlotId = aParam.ToInt32();
                    SfxModule* pMod = pFact->GetModule()->Load();
                    pMod->ExecuteSlot( SfxRequest( nSlotId, SFX_CALLMODE_SYNCHRON, pMod->GetPool() ) );
                    return ERRCODE_ABORT;
                }

                SfxAllItemSet aSet( SFX_APP()->GetPool() );
                sal_uInt16 nSlotId = SID_NEWDOCDIRECT;
                if( pFact->GetStandardTemplate().Len() )
                {
                    aSet.Put( SfxStringItem ( SID_FILE_NAME, pFact->GetStandardTemplate() ) );
                    aSet.Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
                    nSlotId = SID_OPENDOC;
                }
                else
                    aSet.Put( SfxStringItem ( SID_NEWDOCDIRECT, aPathName ) );

                aSet.Put( aFrameItem );

                SFX_ITEMSET_ARG( rMedium.GetItemSet(), pViewId, SfxUInt16Item, SID_VIEW_ID, sal_False );
                if ( pViewId )
                    aSet.Put( *pViewId );

                SFX_ITEMSET_ARG( rMedium.GetItemSet(), pHidden, SfxBoolItem, SID_HIDDEN, sal_False );
                if ( pHidden )
                    aSet.Put( *pHidden );

                if ( rMedium.IsPreview_Impl() )
                    aSet.Put( SfxBoolItem( SID_PREVIEW, sal_True ) );

                SFX_ITEMSET_ARG( rMedium.GetItemSet(), pReadonly, SfxBoolItem, SID_DOC_READONLY, sal_False );
                if ( pReadonly )
                    aSet.Put( *pReadonly );

                if ( pTarget )
                    aSet.Put( *pTarget );
                if ( pReferer )
                    aSet.Put( *pReferer );
                if ( pPath )
                    aSet.Put( *pPath );

                SFX_ITEMSET_ARG( rMedium.GetItemSet(), pAPI, SfxBoolItem, SID_SILENT, sal_False );
                SfxCallMode eMode = SFX_CALLMODE_SYNCHRON;
                if ( pAPI && pAPI->GetValue() )
                    eMode |= SFX_CALLMODE_API;

                const SfxPoolItem *pRet = pApp->GetAppDispatcher().Execute( nSlotId, eMode, aSet );
                if ( pRet )
                    rMedium.GetItemSet()->Put( *pRet, SID_OPENURL );

                return ERRCODE_ABORT;
            }
            else
            {
                // Chaos-Factory ??
                String aFact( aPathName );
                String aPrefix( DEFINE_CONST_UNICODE("private:factory/") );
                if ( aPrefix.Len() == aFact.Match( aPrefix ) )
                    // Factory-Namen extrahieren
                    aFact.Erase( 0, aPrefix.Len() );

                // ContentType ermitteln
                INetContentType eType = INetContentTypes::GetContentType( aFact );
                if ( eType != CONTENT_TYPE_UNKNOWN )
                {
                    // ChaosObjekte werden immer in der Explorer-Root angelegt
                    String aExplorerRoot( SFX_INIMANAGER()->Get( SFX_KEY_EXPLORER_DIR ) );
                    INetURLObject aExp;
                    aExp.SetSmartURL( aExplorerRoot );
#if 0   //(dv)
                    CntAnchorRef xRoot = new CntAnchor( NULL, aExp.GetMainURL() );

                    // Chaos-Factory daf"ur besorgen
                    sal_uInt16 nPos = CntHelperFunctions::GetFactoryPosByContentType( *xRoot, eType, sal_True, 0, 0, 0, CNT_CREATION_FLAG_HIDDEN );
                    if ( nPos != CNTFACTORY_NOTFOUND )
                    {
                        SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
                        if ( rpFrame && rpFrame->GetCurrentDocument() )
                            pSet->Put( SfxFrameItem( SID_DOCFRAME, rpFrame ), SID_DOCFRAME );
                        if ( pReferer )
                            pSet->Put( *pReferer, SID_REFERER );
                        CntAnchorRef xRef = CntHelperFunctions::CreateAnchor( *xRoot, nPos, 0, sal_False );
                        if ( xRef.Is() && !xRef->GetError() )
                        {
                            new SfxAsyncChaosFactory_Impl( xRef, pSet );
                            xRef->Put( SfxVoidItem(WID_INSERT) );
                            return ERRCODE_ABORT;
                        }
                    }
#endif  //(dv)
                }
            }

            return ERRCODE_IO_NOTSUPPORTED;
        }

        case SFX_EXE_FILTER_UNO:
        {
            if ( bPreview )
                return ERRCODE_ABORT;

            SfxViewFrame *pView = SfxViewFrame::Current();
            if ( pView )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >  xFrame;
                SfxFrame *pFrame = pView->GetFrame();
                while ( pFrame )
                {
                    // Falls es eine Mutter-Komponente gibt, diese ansprechen
                    if ( pFrame->HasComponent() )
                        xFrame = pFrame->GetFrameInterface();
                    pFrame = pFrame->GetParentFrame();
                }

                if ( !xFrame.is() )
                    xFrame = pView->GetFrame()->GetFrameInterface();

                ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  xProv( xFrame, ::com::sun::star::uno::UNO_QUERY );
                if ( xProv.is())
                {
                    ::com::sun::star::util::URL aURL;
                    aURL.Complete = rMedium.GetName();
                    SfxURLTransformer aTrans ( ::utl::getProcessServiceFactory() );
                    aTrans.parseStrict( aURL );
                    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp = xProv->queryDispatch( aURL, ::rtl::OUString(), 0 );
                    if ( xDisp.is() )
                        xDisp->dispatch( aURL, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >() );
                }
            }

            return ERRCODE_ABORT;
        }
*/
        case SFX_EXE_FILTER_SLOT:
        {
            if ( bPreview )
                return ERRCODE_ABORT;

            const INetURLObject &rURL = rMedium.GetURLObject();
            SfxUInt16Item aTabPageNoItem( SID_TABPAGE, rURL.GetMark().ToInt32() );
            const SfxPoolItem* pArgs[] =
            {
                &aTabPageNoItem, 0
            };

            String aSlots( rURL.GetURLPath() );
            sal_uInt16 nCount = aSlots.GetTokenCount(',');
            for ( sal_uInt16 nSlot=0; nSlot<nCount; nSlot++ )
            {
                sal_uInt16 nSlotId = (sal_uInt16) aSlots.GetToken( nSlot, ',' ).ToInt32();
                pApp->GetDispatcher_Impl()->Execute( nSlotId, SFX_CALLMODE_SYNCHRON, pArgs, 0, ppInternalArgs );
            }

            return ERRCODE_ABORT;
        }

        default:
            return ERRCODE_IO_NOTSUPPORTED;
    }
}

//----------------------------------------------------------------

SfxExternalAppFilterContainer::SfxExternalAppFilterContainer( )
    : SfxFilterContainer( DEFINE_CONST_UNICODE(SFX_FCONTNR_EXTAPP) )
/*   [Beschreibung]

     Der SfxExternalAppFilterContainer enthaellt jeden Filter, nach
     dem er per GetFilter4FilterName gefragt wird. Er ist fuer
     Redirects von Bedeutung. Im Execute oeffnet er die durch den
     Filter bezeichnete Applikation.  */

{
}

//----------------------------------------------------------------

sal_uInt32 SfxExternalAppFilterContainer::Execute( SfxMedium& rMedium, SfxFrame*&  ) const
{
    const SfxFilter* pFilter = rMedium.GetFilter();
    DBG_ASSERT( pFilter, "Medium braucht Filter" );
    const UniString& rPhysicalName = rMedium.GetPhysicalName();
    // GetPhysicalName() setzt das tmp-Flag zurueck auf sal_True
    rMedium.SetTemporary( sal_False );

    OProcess aApp( pFilter->GetFilterName(), rPhysicalName );
    OProcess::TProcessOption eOption =
        (OProcess::TProcessOption)( OProcess::TOption_SearchPath | OProcess::TOption_Detached );
    sal_Bool bOk = ( aApp.execute( eOption ) == OProcess::E_None );
    return bOk ? ERRCODE_ABORT : ERRCODE_IO_NOTEXISTS;
}

//----------------------------------------------------------------

sal_uInt16 SfxExternalAppFilterContainer::GetFilterCount() const
{
    return SfxFilterContainer::GetFilterCount();
}

//----------------------------------------------------------------

const SfxFilter* SfxExternalAppFilterContainer::GetFilter4FilterName(
    const String& rName, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
/*   [Beschreibung]

     Erzeugt einen entspr. Filter, falls er noch nicht existiert und gibt
     diesen zurueck.
 */
{
    String aEmpty;
    SfxFilterFlags nFlags = SFX_FILTER_IMPORT|SFX_FILTER_NOTINFILEDLG|
        SFX_FILTER_EXECUTABLE;
    if ( (nFlags & nMust) != nMust ||
         (nFlags & nDont ) )
        return 0;
    const SfxFilter* pFilt =
        SfxFilterContainer::GetFilter4FilterName( rName );
    if( !pFilt )
    {
        SfxFilter* pFilter = new SfxFilter(
            rName, DEFINE_CONST_UNICODE("none"), nFlags, 0, aEmpty, aEmpty, 0, aEmpty, this,
            aEmpty );
        pImpl->aList.Insert( pFilter, (sal_uInt32)0 );
        pFilt = pFilter;
    }
    return pFilt;
}

//----------------------------------------------------------------

SfxFactoryFilterContainer::SfxFactoryFilterContainer(
    const String& rName, const SfxObjectFactory& rFactP )
    : SfxFilterContainer( rName ), rFact( rFactP ), pFunc(0)
/*   [Beschreibung]

     Im SfxFactoryFilterContainer befinden sich die Filter einer
     SfxObjectFactory. Wird kein DetectFilter registriert, so wird
     fuer die DefaultFactory <SfxApplication::DetectFilter> genommen
 */
{
    //Defaults in den Container der DefaultFactory laden
    SetFlags( GetFlags() | SFX_FILTER_CONTAINER_FACTORY );
    if( &SfxObjectFactory::GetDefaultFactory() == &rFactP )
    {
        pFunc = &SfxFilterMatcher::AppDetectFilter;
    }
}
//----------------------------------------------------------------

sal_uInt32 SfxFactoryFilterContainer::GetFilter4Content(
    SfxMedium& rMedium, const SfxFilter** ppFilter, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    ForceFilterLoad_Impl();
    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pTargetItem, SfxStringItem,
                     SID_TARGETNAME, sal_False);
    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pDontItem, SfxBoolItem,
                     SID_FILTER_DONTDETECTONCONTENT, sal_False);
    if( pDontItem && pDontItem->GetValue() )
        return 0;

    if ( *ppFilter && ( (*ppFilter)->GetFilterFlags() & SFX_FILTER_STARONEFILTER ) )
        return 0;

    if( pFunc )
    {
        sal_uInt32 nErr = (*pFunc)(rMedium, ppFilter, nMust, nDont);
        DBG_ASSERT( !*ppFilter ||
                    (((*ppFilter)->GetFilterFlags() & nMust ) == nMust &&
                     ((*ppFilter)->GetFilterFlags() & nDont ) == 0 ),
                    "DetectFilter Spec nicht eingehalten" );
        return nErr;
    }
    else
        return 0;
}

//----------------------------------------------------------------

DECLARE_LIST( SfxFContainerList_Impl, SfxFilterContainer * );

class SfxFilterMatcher_Impl
{
public:
    sal_uInt16 nAkt;
    SfxFContainerList_Impl aList;
    sal_Bool bDeleteContainers;
};

//----------------------------------------------------------------

SfxFilterMatcher::SfxFilterMatcher( SfxFilterContainer* pCont)
{
    pImpl = new SfxFilterMatcher_Impl;
    pImpl->bDeleteContainers = sal_False;
    AddContainer( pCont );
}

//----------------------------------------------------------------

SfxFilterMatcher::SfxFilterMatcher()
{
    pImpl = new SfxFilterMatcher_Impl;
    pImpl->bDeleteContainers = sal_False;
}

SfxFilterMatcher::SfxFilterMatcher(sal_Bool bDeleteContainers)
{
    pImpl = new SfxFilterMatcher_Impl;
    pImpl->bDeleteContainers = bDeleteContainers;
}

//----------------------------------------------------------------

SfxFilterMatcher::~SfxFilterMatcher()
{
    if ( pImpl->bDeleteContainers )
    {
        for ( sal_uInt32 n = pImpl->aList.Count(); n--; )
        {
            SfxFilterContainer *pFCont = pImpl->aList.Remove(n);
            delete pFCont;
        }
    }
    delete pImpl;
}

//----------------------------------------------------------------

void SfxFilterMatcher::AddContainer( SfxFilterContainer* pC )
{
    pImpl->aList.Insert( pC, pImpl->aList.Count() );
}

//----------------------------------------------------------------

sal_uInt16 SfxFilterMatcher::GetContainerCount() const
{
    return (sal_uInt16)pImpl->aList.Count();
}

//----------------------------------------------------------------

SfxFilterContainer* SfxFilterMatcher::GetContainer( sal_uInt16 nPos ) const
{
    return pImpl->aList.GetObject( nPos );
}

//----------------------------------------------------------------

SfxFilterContainer* SfxFilterMatcher::GetContainer( const String &rName ) const
{
    SfxFContainerList_Impl& rList = pImpl->aList;
    sal_uInt16 nCount = (sal_uInt16) rList.Count();
    for( sal_uInt16 n = 0; n < nCount; n++ )
        if( rList.GetObject( n )->GetName() == rName )
            return rList.GetObject( n );
    return 0;
}

//----------------------------------------------------------------

sal_uInt32  SfxFilterMatcher::GuessFilterIgnoringContent(
    SfxMedium& rMedium, const SfxFilter**ppFilter,
    SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    String aFileName = rMedium.GetName();
    sal_uInt32 nErr = ERRCODE_NONE;
    const SfxFilter* pFilter = *ppFilter;
    const INetURLObject& rObj = rMedium.GetURLObject();

#if defined(WNT) || defined(WIN) || defined(OS2)
    if( !pFilter )
    {
        // Hier kommen zunaechst fest eingehackte Regeln:
        if( rObj.GetProtocol() == INET_PROT_FILE && rObj.GetExtension().CompareIgnoreCaseToAscii( "exe" ) == COMPARE_EQUAL )
        {
            String aStr( DEFINE_CONST_UNICODE("soffice") );
//          String aStr( SfxResId( STR_FILTER_OTHERS ) );
            aStr += DEFINE_CONST_UNICODE(": exe");
            pFilter = GetFilter( aStr, nMust, nDont );
        }
    }
#endif

    SfxFilterContainer *pCont = GetContainer( DEFINE_CONST_UNICODE("soffice") );
    if ( pCont )
    {
        if ( !pFilter )
            pFilter = SfxExecutableFilterContainer::CheckForFolder( rMedium, nMust, nDont );
/*
        if ( !pFilter )
        {
            // Wenn eine Komponente f"ur das vorhandene ::com::sun::star::util::URL-Pattern registriert ist und sie auch
            // mit der speziellen ::com::sun::star::util::URL was anfangen kann, gibt createObject einen Loader zur"uck
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMgr( ::utl::getProcessServiceFactory() );
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xReg( xMgr->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.FrameLoaderFactory") ), ::com::sun::star::uno::UNO_QUERY );
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  aRef = xReg->createInstance( aFileName );
            if ( aRef.is() )
                pFilter = pCont->GetFilter4FilterName(DEFINE_CONST_UNICODE("component"), nMust, nDont );
        }
 */
    }

    if( !pFilter )
        pFilter = SFX_APP()->GetFilterMatcher().GetFilter4Protocol( rMedium );

    sal_Bool bCheckExternBrowser = rMedium.IsAllowedForExternalBrowser();
    if ( bCheckExternBrowser && !GetContainer( DEFINE_CONST_UNICODE("soffice") ) )
        bCheckExternBrowser = sal_False;
    if ( bCheckExternBrowser && ( (nDont & SFX_EXE_FILTERFLAGS ) || (( nMust & SFX_EXE_FILTERFLAGS ) != nMust ) ) )
        bCheckExternBrowser = sal_False;

    if( pFilter )
    {
        pFilter = ResolveRedirection( pFilter, rMedium );
    }
    else if ( rMedium.ProvidesData_Impl() )
    {
        // Falls Medium Remote, Zunaechst ueber Mimetypen pruefen ( nicht bei ExternBrowser, sofern dabei angeladen wird )
        if( !pFilter && rMedium.SupportsMIME_Impl() && ( !bCheckExternBrowser || rObj.GetProtocol() != INET_PROT_HTTP && rObj.GetProtocol() != INET_PROT_HTTPS ) )
        {
            // Mime Typen holen
            String aMime;
            nErr = rMedium.GetMIMEAndRedirect( aMime );
            nErr = ERRCODE_TOERROR( nErr );
            if ( nErr == ERRCODE_IO_PENDING )
                return nErr;
            else if( !nErr && aMime.Len() )
            {
                if ( aMime.EqualsAscii(CONTENT_TYPE_STR_X_CNT_HTTPFILE) )
                    // FilterDetection nur "uber den Content
                    return ERRCODE_NONE;
                else
                    pFilter = GetFilter4Mime( aMime, nMust, nDont );

                if ( pFilter && aMime.EqualsAscii(CONTENT_TYPE_STR_APP_OCTSTREAM) )
                {
                    // Damit eigene Formate wie sdw auch bei falsch konfiguriertem Server erkannt werden, bevor
                    // wir GetInStream rufen
                    const SfxFilter* pMimeFilter = pFilter;
                    pFilter = GetFilter4Extension( rMedium.GetURLObject().GetName(), nMust, nDont );
                    pFilter = ResolveRedirection( pFilter, rMedium );
                    if( pFilter )
                    {
                        nErr = ERRCODE_NONE;
                        if ( ( pFilter == SfxExecutableFilterContainer::GetChooserFilter() ||
                             pFilter->GetFilterName().EqualsAscii(SFX_FILTER_DOWNLOAD) ) && !rMedium.IsRemote() )
                                // Chooser/Downloader bei MIME nur wenn remote
                                pFilter = 0;
                    }

                    if ( !pFilter )
                        pFilter = pMimeFilter;
                }

                // Bei MIME Typen keinen Storage anfordern
                if( !nErr && rMedium.SupportsMIME_Impl() && rMedium.ProvidesData_Impl() &&
                    pFilter && !pFilter->UsesStorage() )
                {
                    rMedium.GetInStream();
                    nErr = rMedium.GetError();
                }
            }

            if( nErr )
            {
                if ( nErr == ERRCODE_SFX_CONSULTUSER )
                    *ppFilter = pFilter;
                return nErr;
            }
            else if ( pFilter && aMime.EqualsAscii(CONTENT_TYPE_STR_TEXT_HTML) )
            {
                // MIME ist vertrauenswuerdig ( !? )
                nErr = ERRCODE_SFX_NEVERCHECKCONTENT;
            }
        }

        if( !pFilter && rMedium.ProvidesFile_Impl() && rMedium.IsDownloadDone_Impl() )
        {
            // dann ueber Storage CLSID
            // Remote macht das keinen Sinn, wenn der Download noch la"uft
            if( rMedium.IsStorage() )
            {
                SvStorageRef aStor = rMedium.GetStorage();
                if ( !aStor.Is() )
                    return ERRCODE_IO_GENERAL;
                pFilter = GetFilter4ClipBoardId( aStor->GetFormat(), nMust, nDont );
            }

            // Als naechstes ueber Extended Attributes pruefen
            String aNewFileName;
            if( !pFilter )
            {
                aNewFileName = rMedium.GetPhysicalName();
                SvEaMgr aMgr( aNewFileName );
                String aType;
                if( aMgr.GetFileType( aType ))
                {
                    // Plain text verwerfen wir, damit unter OS/2
                    // html Dateien mit attribut Text als html geladen werden
                    if( aType.CompareIgnoreCaseToAscii( "Plain Text" ) != COMPARE_EQUAL )
                        pFilter = GetFilter4EA( aType, nMust, nDont );
                }
            }
        }

        // Zu allerletzt ueber Extension mappen
        pFilter = ResolveRedirection( pFilter, rMedium );
        if( !pFilter )
        {
            pFilter = GetFilter4Extension( rMedium.GetURLObject().GetName(), nMust, nDont );
            if( !pFilter || pFilter->GetWildcard()==DEFINE_CONST_UNICODE("*.*") || pFilter->GetWildcard() == '*' )
                pFilter = 0;
            pFilter = ResolveRedirection( pFilter, rMedium );
            if( pFilter &&
                ( pFilter == SfxExecutableFilterContainer::GetChooserFilter() ||
                  pFilter->GetFilterName().EqualsAscii(SFX_FILTER_DOWNLOAD) ) &&
                !rMedium.IsRemote() )
                pFilter = 0;
        }
    }

    if ( bCheckExternBrowser &&
        ( ( pFilter && pFilter->GetMimeType().EqualsAscii(CONTENT_TYPE_STR_TEXT_HTML) ) ||
        (!pFilter && ( rObj.GetProtocol() == INET_PROT_HTTP || rObj.GetProtocol() == INET_PROT_HTTPS || rObj.GetProtocol() == INET_PROT_FTP && rMedium.SupportsMIME_Impl() ) ) ) )
    {
        const SfxFilter* pBrwFilter = GetFilter4FilterName( DEFINE_CONST_UNICODE("ExternBrowser"), nMust, nDont );
        if ( pBrwFilter )
        {
            rMedium.GetItemSet()->Put( SfxBoolItem (SID_DOC_READONLY, sal_True ) );
            rMedium.SetOpenMode( SFX_STREAM_READONLY, sal_True );
            pFilter = pBrwFilter;
            if ( rMedium.GetFilter() )
                rMedium.SetFilter( 0 );
/*(dv)          CntAnchorRef xAnchor( rMedium.GetAnchor() );
            if ( xAnchor.Is() )
                xAnchor->Put( CntContentTypeItem(WID_CONTENT_TYPE, CONTENT_TYPE_TEXT_HTML ) );
*/      }
    }

    if ( !pFilter && rObj.GetProtocol() == INET_PROT_FTP )
    {
        pFilter = SfxExecutableFilterContainer::GetDownloadFilter();
        nErr = ERRCODE_SFX_CONSULTUSER;
    }

    *ppFilter = pFilter;
    return nErr;
}

//----------------------------------------------------------------

#define CHECKERROR()                                            \
if( nErr == 1 || nErr == USHRT_MAX || nErr == ULONG_MAX )       \
{                                                               \
    ByteString aText = "Fehler in FilterDetection: Returnwert ";\
    aText += ByteString::CreateFromInt32(nErr);                 \
    if( pFilter )                                               \
    {                                                           \
        aText += ' ';                                           \
        aText += ByteString(U2S(pFilter->GetName()));           \
    }                                                           \
    DBG_ERROR( aText.GetBuffer() );                             \
    nErr = ERRCODE_ABORT;                                       \
}

//----------------------------------------------------------------

sal_uInt32  SfxFilterMatcher::GuessFilter(
    SfxMedium& rMedium, const SfxFilter**ppFilter,
    SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    const SfxFilter* pOldFilter = ResolveRedirection( *ppFilter, rMedium );
    const SfxFilter* pFilter = pOldFilter;

    sal_Bool bConsultUser = sal_False;
    sal_Bool bProvidesData = rMedium.ProvidesData_Impl();
    sal_Bool bSupportsMime = rMedium.SupportsMIME_Impl();

    // Zunaechst, falls Filter mitkommt einmal testen, ob dieser in Ordnung ist.
    ErrCode nErr = ERRCODE_NONE;
    if( pFilter && bProvidesData && ( pFilter->GetFilterContainer()->GetFlags() & SFX_FILTER_CONTAINER_FACTORY ) )
    {
        rMedium.StartDownload();
        if ( !rMedium.IsDownloadDone_Impl() )
        {
            if ( pFilter->GetFilterFlags() & SFX_FILTER_ASYNC )
            {
                // kurzzeitig auf synchron schalten solange die Apps sich weigern asynchron
                // zu detecten
                rMedium.ForceSynchronStream_Impl( sal_True );
                if ( !rMedium.GetInStream() )
                {
                    ErrCode e = rMedium.GetErrorCode();
                    if ( e == ERRCODE_NONE )
                    {
                        *ppFilter = pFilter;
                        return ERRCODE_IO_PENDING;
                    }
                    else
                        return e;
                }
            }
            else
            {
                 *ppFilter = pFilter;
                return ERRCODE_IO_PENDING;
            }
        }

        if( bSupportsMime && !pFilter->UsesStorage() )
            rMedium.GetInStream();

        nErr = pFilter->GetFilterContainer()->GetFilter4Content( rMedium, &pFilter, nMust, nDont );
        CHECKERROR();

        rMedium.ForceSynchronStream_Impl( sal_False );

        // ABORT bedeutet Filter ungueltig
        if( nErr && (nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET ) )
            return nErr;

        // War der Filter ungueltig oder wurde ein anderer zurueckgegeben,
        // so detecten wir selbst (wg. redirection)
        if( nErr == ERRCODE_ABORT && pFilter )
            bConsultUser = sal_True;

        if( nErr != ERRCODE_SFX_FORCEQUIET && pOldFilter && pFilter != pOldFilter )
            pFilter = 0;
    }

    if( !pFilter )
    {
        bConsultUser = sal_False;
//DV !!!! don't close InStream when using the new medium
//      rMedium.CloseInStream();

        // Als erstes Protocol, MIME-Type, Extension etc. probieren
        nErr = GuessFilterIgnoringContent( rMedium, &pFilter, nMust, nDont );
        if ( nErr == ERRCODE_IO_PENDING )
        {
            *ppFilter = pFilter;
            return nErr;
        }

        if ( pFilter && nErr == ERRCODE_SFX_CONSULTUSER )
        {
            pFilter = ResolveRedirection( pFilter, rMedium );
            *ppFilter = pFilter;
        }

        if ( !pFilter && SFX_APP()->ShouldUseExternalBrowser( rMedium.GetName() ) )
        {
            sal_Bool bIExplorer = sal_False;
            INetURLObject aObj( rMedium.GetName() );
            if ( aObj.GetProtocol() == INET_PROT_HTTP )
            {
                bIExplorer = sal_True;
            }
            else if ( aObj.GetProtocol() == INET_PROT_FILE || aObj.GetProtocol() == INET_PROT_FTP )
            {
                String aExt = aObj.GetExtension().ToLowerAscii();
                if ( aExt.EqualsAscii("htm") || aExt.EqualsAscii("html") )
                    bIExplorer = sal_True;
            }

            if ( bIExplorer )
                DBG_ERROR( "Das wäre ihr Preis gewesen ... " );
        }

/*
        // Erst jetzt auch das Betriebssystem einschalten
        if ( !pFilter && 0 != GetContainer( DEFINE_CONST_UNICODE(SFX_FCONTNR_EXTAPP) ) )
        {
            sal_Bool bExternalAllowed = sal_True;
            SFX_ITEMSET_ARG( rMedium.GetItemSet(), pFlags, SfxStringItem, SID_OPTIONS, sal_False);
            if ( pFlags )
            {
                // Werte auf einzelne Items verteilen
                String aFileFlags = pFlags->GetValue();
                aFileFlags.ToUpperAscii();
                if ( STRING_NOTFOUND != aFileFlags.Search( 'B' ) )
                    bExternalAllowed = sal_False;
                else if ( STRING_NOTFOUND != aFileFlags.Search( 'H' ) )
                    bExternalAllowed = sal_False;
            }

            INetURLObject aURL( "macro:" );
            String aReferer( rMedium.GetReferer() );
            if ( !SFX_APP()->IsSecureURL( aURL, &aReferer ) )
                bExternalAllowed = sal_False;

            if ( bExternalAllowed && rMedium.GetURLObject().GetProtocol() == INET_PROT_HTTP )
                bExternalAllowed = sal_False;

            if ( aReferer.EqualsAscii("private:OpenEvent") )
                bExternalAllowed = sal_False;

            if ( bExternalAllowed && rMedium.GetPhysicalName().Len() )
            {
                String aName( '\"');
                aName += rMedium.GetPhysicalName();
                aName += '\"';
                rMedium.SetTemporary(sal_False);

                // Wenn "uber das Betriebssystem geladen werden kann, wird der aktuelle Ladevorgang
                // abgebrochen
                if ( System::StartProcess( NULL, aName ) )
                    return ERRCODE_ABORT;
            }
        }
*/
        if( nErr && nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET && nErr != ERRCODE_SFX_NEVERCHECKCONTENT )
            return nErr;

        if( nErr == ERRCODE_ABORT )
            pFilter = 0;

        pFilter = ResolveRedirection( pFilter, rMedium );

        // Jetzt wird geprueft, ob das Modul auch einverstanden ist; ist das nicht der Fall, wird auf
        // jeden Fall auf ConsultUser umgeschaltet
        if( pFilter && bProvidesData )
        {
            if( nErr == ERRCODE_SFX_NEVERCHECKCONTENT )
                nErr = ERRCODE_NONE;
            else if( pFilter->GetFilterContainer()->GetFlags() & SFX_FILTER_CONTAINER_FACTORY )
            {
                rMedium.StartDownload();
                if ( !rMedium.IsDownloadDone_Impl() )
                {
                    if ( !pFilter->UsesStorage() && rMedium.GetInStream() && ( pFilter->GetFilterFlags() & SFX_FILTER_ASYNC ) )
                        // kurzzeitig auf synchron schalten solange die Apps sich weigern asynchron
                        // zu detecten
                        rMedium.ForceSynchronStream_Impl( sal_True );
                    else
                    {
                        *ppFilter = pFilter;
                        return ERRCODE_IO_PENDING;
                    }
                }

                const SfxFilter* pTmpFilter = pFilter;
                nErr = pFilter->GetFilterContainer()->GetFilter4Content( rMedium, &pFilter, nMust, nDont );
                CHECKERROR();

                rMedium.ForceSynchronStream_Impl( sal_False );

                // ABORT bedeutet Filter ungueltig
                if( nErr && (nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET ) )
                     return nErr;

                if( nErr == ERRCODE_ABORT && pFilter )
                    pFilter = 0;
            }
        }
    }

    // Jetzt einmal drueberiterieren und nur die perfekten Filter nehmen
    if( !pFilter )
    {
        // Achtung: hier k"onnte auch asynchron detected werden!
        rMedium.StartDownload();
        if ( !rMedium.IsDownloadDone_Impl() )
            return ERRCODE_IO_PENDING;

        rMedium.IsStorage();
        nErr = rMedium.GetErrorCode();
        if( nErr )
            return nErr;
        nErr = GetFilter4Content( rMedium, &pFilter, nMust, nDont, sal_True );
        CHECKERROR();
        if( nErr && (nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET ) )
            return nErr;

        if( nErr == ERRCODE_ABORT && pFilter )
            pFilter = 0;
    }

    // Letzte Moeglichkeit ist die Befragung aller ObjectFactories.
    if( !pFilter )
    {
        // Achtung: hier k"onnte auch asynchron detected werden!
        if ( !rMedium.IsDownloadDone_Impl() )
            return ERRCODE_IO_PENDING;

//DV !!!! don't close InStream when using the new Medium
//rMedium.CloseInStream();

        rMedium.IsStorage();
        nErr = GetFilter4Content( rMedium, &pFilter, nMust, nDont );
        CHECKERROR();
        if( nErr && (nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET ) )
            return nErr;

        if( nErr == ERRCODE_ABORT && pFilter )
            bConsultUser = sal_True;
    }

    if ( pFilter &&
            ( pFilter == SfxExecutableFilterContainer::GetChooserFilter() && pFilter != pOldFilter ||
            pFilter->GetFilterName().EqualsAscii(SFX_FILTER_DOWNLOAD) && !rMedium.IsRemote() ) )
        pFilter = NULL;

    *ppFilter = pFilter;

    if ( ( ( pOldFilter && pOldFilter!=pFilter &&
            ( !pOldFilter->IsOwnTemplateFormat() || !pFilter->IsOwnFormat() ) ) ) &&
             nErr != ERRCODE_SFX_FORCEQUIET || bConsultUser )
        return ERRCODE_SFX_CONSULTUSER;
/*
    if( rMedium.SupportsMIME_Impl() && bProvidesData &&
        pFilter && !pFilter->UsesStorage() )
        rMedium.GetInStream();    // Bei MIME Typen keinen Storage anfordern
*/
    if( !pOldFilter )
        pOldFilter = pFilter;

    // Checken, ob Daten vorliegen. Dies verhindert Fehler, die in
    // GetFilter4Content auftreten und die Auswahlbox hochbringen.
    ErrCode nMediumError = rMedium.GetErrorCode();
    if( nMediumError )
        return nMediumError;

    *ppFilter = pFilter;
    if ( ( nErr ||
          ( pOldFilter && pOldFilter!=pFilter
            && ( !pOldFilter->IsOwnTemplateFormat() ||
                 !pFilter->IsOwnFormat() ) ))
             && nErr != ERRCODE_SFX_FORCEQUIET )
        nErr = ERRCODE_SFX_CONSULTUSER;

    if( nErr == ERRCODE_SFX_FORCEQUIET )
        nErr = 0;
    return nErr;
};

//----------------------------------------------------------------
sal_Bool SfxFilterMatcher::IsFilterInstalled( const SfxFilter* pFilter )
{
    Window *pWindow = SFX_APP()->GetTopWindow();
    if ( pFilter->GetFilterFlags() & SFX_FILTER_MUSTINSTALL )
    {
        // Hier k"onnte noch eine Nachinstallation angeboten werden
        String aText( SfxResId( STR_FILTER_NOT_INSTALLED ) );
        aText.SearchAndReplaceAscii( "$(FILTER)", pFilter->GetUIName() );
        QueryBox aQuery( pWindow, WB_YES_NO | WB_DEF_YES, aText );
        short nRet = aQuery.Execute();
        if ( nRet == RET_YES )
        {
#ifdef DBG_UTIL
            // Setup starten
            InfoBox( pWindow, DEFINE_CONST_UNICODE("Hier soll jetzt das Setup starten!") ).Execute();
#endif
            // Installation mu\s hier noch mitteilen, ob es geklappt hat, dann kann das
            // Filterflag gel"oscht werden
        }

        return ( !(pFilter->GetFilterFlags() & SFX_FILTER_MUSTINSTALL) );
    }
    else if ( pFilter->GetFilterFlags() & SFX_FILTER_CONSULTSERVICE )
    {
        String aText( SfxResId( STR_FILTER_CONSULT_SERVICE ) );
        aText.SearchAndReplaceAscii( "$(FILTER)", pFilter->GetUIName() );
        InfoBox ( pWindow, aText ).Execute();
        return sal_False;
    }
    else
        return sal_True;
}


sal_uInt32 SfxFilterMatcher::DetectFilter( SfxMedium& rMedium, const SfxFilter**ppFilter, sal_Bool bPlugIn, sal_Bool bAPI ) const
/*  [Beschreibung]

    Hier wird noch die Filterauswahlbox hochgezogen. Sonst GuessFilter
 */

{
    const SfxFilter* pOldFilter = rMedium.GetFilter();
    if ( pOldFilter )
    {
        if( !IsFilterInstalled( pOldFilter ) )
            pOldFilter = 0;
        else
        {
            SFX_ITEMSET_ARG( rMedium.GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
            if ( ( pOldFilter->GetFilterFlags() & SFX_FILTER_PACKED ) && pSalvageItem )
                // Salvage is always done without packing
                pOldFilter = 0;
        }
    }

    const SfxFilter* pFilter = pOldFilter;

    sal_Bool bPreview = rMedium.IsPreview_Impl(), bInsert = sal_False;
    SFX_ITEMSET_ARG(rMedium.GetItemSet(), pReferer, SfxStringItem, SID_REFERER, FALSE);
    if ( bPreview && rMedium.IsRemote() && ( !pReferer || pReferer->GetValue().CompareToAscii("private:searchfolder:",21 ) != COMPARE_EQUAL ) )
        return ERRCODE_ABORT;

    ErrCode nErr = GuessFilter( rMedium, &pFilter );
    if ( nErr == ERRCODE_ABORT )
        return nErr;

    if ( nErr == ERRCODE_IO_PENDING )
    {
        *ppFilter = pFilter;
        return nErr;
    }

    if ( !pFilter )
    {
        const SfxFilter* pInstallFilter = NULL;

        // Jetzt auch Filter testen, die nicht installiert sind ( ErrCode ist irrelevant )
        GuessFilter( rMedium, &pInstallFilter, SFX_FILTER_IMPORT, SFX_FILTER_CONSULTSERVICE );
        if ( pInstallFilter )
        {
            if ( IsFilterInstalled( pInstallFilter ) )
                // Eventuell wurde der Filter nachinstalliert
                pFilter = pInstallFilter;
        }
        else
        {
            // Jetzt auch Filter testen, die erst von Star bezogen werden m"ussen ( ErrCode ist irrelevant )
            GuessFilter( rMedium, &pInstallFilter, SFX_FILTER_IMPORT, 0 );
            if ( pInstallFilter )
                IsFilterInstalled( pInstallFilter );
        }
    }

    sal_Bool bHidden = bPreview;
    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pFlags, SfxStringItem, SID_OPTIONS, sal_False);
    if ( !bHidden && pFlags )
    {
        String aFlags( pFlags->GetValue() );
        aFlags.ToUpperAscii();
        if( STRING_NOTFOUND != aFlags.Search( 'H' ) )
            bHidden = sal_True;
    }

    if ( ( !pFilter || nErr == ERRCODE_SFX_CONSULTUSER ) && !bAPI && !bHidden )
    {
        if ( !pFilter && rMedium.IsRemote() )
            pFilter = SfxExecutableFilterContainer::GetDownloadFilter();
        else if ( !pFilter )
            pFilter = pOldFilter;

        String aTmpName;
        if ( pFilter )
            aTmpName = pFilter->GetUIName();

        SfxFilterMatcher *pMatcher;
        if( bPlugIn && pFilter )
            pMatcher = new SfxFilterMatcher( (SfxFilterContainer *) pFilter->GetFilterContainer() );
        else
            pMatcher = (SfxFilterMatcher*) this;

        SfxFilterDialog *pDlg = new SfxFilterDialog( 0, &rMedium, *pMatcher, pFilter ? &aTmpName: 0, 0 );
        const sal_Bool bOk = RET_OK == pDlg->Execute();
        if (bOk)
            pFilter  = pMatcher->GetFilter4UIName( pDlg->GetSelectEntry());

        if( bPlugIn && pFilter )
            delete pMatcher;
        delete pDlg;

        if ( !bOk)
            nErr = ERRCODE_ABORT;
        else
            nErr = ERRCODE_NONE;

        pFilter = ResolveRedirection( pFilter, rMedium );
    }

    *ppFilter = pFilter;

    if ( bHidden || bAPI && nErr == ERRCODE_SFX_CONSULTUSER )
        nErr = pFilter ? ERRCODE_NONE : ERRCODE_ABORT;
    return nErr;
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcher::GetDefaultFilter() const
/*  [Beschreibung]

    Returned den ersten Filter, der nicht internal ist und im Filedialog
    auftaucht
    */
{
    SfxFilterMatcherIter aIter(
        this, 0, SFX_FILTER_INTERNAL | SFX_FILTER_NOTINFILEDLG );
    return aIter.First();
}

//----------------------------------------------------------------

sal_uInt32 SfxFilterMatcher::GetFilter4Content(
    SfxMedium& rMedium, const SfxFilter** ppFilter,  SfxFilterFlags nMust, SfxFilterFlags nDont, sal_Bool bOnlyGoodOnes ) const
{
    sal_uInt32 nErr = ERRCODE_NONE;
    SfxFContainerList_Impl& rList = pImpl->aList;
    sal_uInt16 nCount = (sal_uInt16)rList.Count();
    for( sal_uInt16 n = 0; n<nCount; n++ )
    {
        const SfxFilter* pFilter = 0;
        nErr = rList.GetObject( n )->GetFilter4Content(
            rMedium, &pFilter, nMust, nDont );
        CHECKERROR();
        if( nErr == ERRCODE_ABORT && bOnlyGoodOnes ) pFilter = 0;
        if( pFilter )
        {
            *ppFilter = pFilter;
            return nErr;
        }
    }
    return 0;
}

//----------------------------------------------------------------

#define IMPL_LOOP( Type, ArgType )                              \
const SfxFilter* SfxFilterMatcher::Type(                        \
    ArgType rStr, SfxFilterFlags nMust, SfxFilterFlags nDont ) const            \
{                                                               \
    SfxApplication* pApp = SFX_APP();                           \
    if( this == &pApp->GetFilterMatcher() )                     \
        pApp->ForcePendingInitFactories();                      \
    SfxFContainerList_Impl& rList = pImpl->aList;               \
    sal_uInt16 nCount = (sal_uInt16)rList.Count();                      \
    for( sal_uInt16 n = 0; n<nCount; n++ )                          \
    {                                                           \
        const SfxFilter* pFilter =                              \
            rList.GetObject( n )->Type(rStr, nMust, nDont );    \
        if( pFilter )                                           \
            return pFilter;                                     \
    }                                                           \
    return 0;                                                   \
}

IMPL_LOOP( GetFilter4Mime, const String& )
IMPL_LOOP( GetFilter4EA, const String& )
IMPL_LOOP( GetFilter4Extension, const String& )
IMPL_LOOP( GetFilter4Protocol, SfxMedium& )
IMPL_LOOP( GetFilter4ClipBoardId, sal_uInt32 )
IMPL_LOOP( GetFilter, const String& )
IMPL_LOOP( GetFilter4FilterName, const String& )
IMPL_LOOP( GetFilter4UIName, const String& )

//----------------------------------------------------------------

sal_uInt32 SfxFilterMatcher::AppDetectFilter(
    SfxMedium& rMedium, const SfxFilter** ppFilter,  SfxFilterFlags nMust, SfxFilterFlags nDont )
{
    SfxFilterFlags nClass = nMust & SFX_FILTER_TEMPLATE ?
        SFX_FILTER_TEMPLATE : USHRT_MAX;
    return SFX_APP()->DetectFilter( rMedium, ppFilter, (sal_uInt16)nMust );
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcher::ResolveRedirection( const SfxFilter* pFilter, SfxMedium& rMedium ) const
{
    return ResolveRedirection( pFilter, rMedium.GetURLObject().GetURLPath() );
}

const SfxFilter* SfxFilterMatcher::ResolveRedirection( const SfxFilter* pFilter, const String& rURLPath ) const
{
    while( sal_True )
    {
        if( !pFilter || !(pFilter->GetFilterFlags() & SFX_FILTER_REDIRECT))
            return pFilter;
        {
            if( !(pFilter->GetFilterFlags() & SFX_FILTER_ALIEN ) )
            // Es ist der Officefilter
            {
                sal_Bool bFound = sal_False;
                String aName = ToUpper_Impl( rURLPath );

                // jetzt auch die Nicht-Own-Filter f"ur das MsgDoc
                SfxFilterMatcherIter aIter( this, SFX_FILTER_IMPORT );
                for( const SfxFilter* pAppFilter = aIter.First(); pAppFilter;
                     pAppFilter = aIter.Next() )
                {
                    SfxFilterFlags nFlags = pAppFilter->GetFilterFlags();
                    // aber nicht den OfficeFilter
                    WildCard aCard( ToUpper_Impl( pAppFilter->GetWildcard()() ), ';' );
                    if ( aCard() == DEFINE_CONST_UNICODE("*.*") || aCard() == '*' )
                        continue;

                    if( aCard.Matches( aName ) && pAppFilter != pFilter )
                    {
                        pFilter = pAppFilter;
                        bFound = sal_True;
                        break;
                    }
                }

                if (!bFound)
                {
                    DBG_ERROR( "AppFilter nicht gefunden" );
                    return 0;
                }
            }
            else
            {
                String aData = pFilter->GetUserData();
                if( aData.GetTokenCount( '|' )!=2 )
                {
                    DBG_ERROR( "Schlechtes Redirectionsformat" );
                    return 0;
                }
                else
                {
                    String aContainer = aData.GetToken( 0, '|' );
                    String aFilter = aData.GetToken( 1, '|' );
                    SfxFilterContainer* pCont = GetContainer( aContainer );
                    if( pCont )
                        pFilter = pCont->GetFilter4FilterName( aFilter );
                    else
                        return 0;
                }
            }
        }
    }
}

//----------------------------------------------------------------

IMPL_STATIC_LINK( SfxFilterMatcher, MaybeFileHdl_Impl, String*, pString )
{
    const SfxFilter* pFilter =
        pThis->GetFilter4Extension( *pString, SFX_FILTER_IMPORT );
    if( pFilter && !pFilter->GetWildcard().Matches( String() ) &&
        pFilter->GetWildcard() != DEFINE_CONST_UNICODE("*.*") && pFilter->GetWildcard() != '*' )
        return sal_True;
    return sal_False;
}

//----------------------------------------------------------------

SfxFilterMatcherIter::SfxFilterMatcherIter(
    const SfxFilterMatcher* pMatchP,
    SfxFilterFlags nOrMaskP, SfxFilterFlags nAndMaskP )
    : pMatch( pMatchP->pImpl),
      nOrMask( nOrMaskP ), nAndMask( nAndMaskP )
{
    // Iterator auf AppFilterMatcher -> DoInitFactory
    SfxApplication* pApp = SFX_APP();
    if( pMatchP == &pApp->GetFilterMatcher() )
        pApp->ForcePendingInitFactories();
    if( nOrMask == 0xffff ) //Wg. Fehlbuild auf s
        nOrMask = 0;
}

//----------------------------------------------------------------

const SfxFilter *SfxFilterMatcherIter::Forward_Impl()
{
    sal_uInt16 nCount = (sal_uInt16 )pMatch->aList.Count();
    for( ++nAktContainer; nAktContainer < nCount ; nAktContainer++ )
    {
        pCont = pMatch->aList.GetObject( nAktContainer );
        sal_uInt16 nCnt = pCont->GetFilterCount();
        if( nCnt )
        {
            nBorder=nCnt;
            nAktFilter = 0;
            return pCont->GetFilter( 0 );
        }
    }
    return 0;
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcherIter::First_Impl()
{
    nAktFilter = -1;
    nAktContainer = -1;
    nBorder = 0;

    return Forward_Impl();
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcherIter::Next_Impl()
{
    if( ++nAktFilter < nBorder )
        return pCont->GetFilter( nAktFilter );
    return Forward_Impl();
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcherIter::First()
{
    const SfxFilter* pFilter;
    for( pFilter = First_Impl(); pFilter; pFilter = Next_Impl())
    {
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if( ((nFlags & nOrMask) == nOrMask ) && !(nFlags & nAndMask ) )
            break;
    }
    return pFilter;
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcherIter::Next()
{
    const SfxFilter* pFilter;
    for( pFilter = Next_Impl(); pFilter; pFilter = Next_Impl())
    {
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if( ((nFlags & nOrMask) == nOrMask ) && !(nFlags & nAndMask ) )
            break;
    }
    return pFilter;
}

//----------------------------------------------------------------

SfxFrameWindowFactory::SfxFrameWindowFactory(
    FactoryFunc pFuncP, String aNameP ) :
    pFunc( pFuncP ), aName( aNameP.ToUpperAscii() )
{
}

SfxPluginFilterContainer::SfxPluginFilterContainer()
    : SfxFilterContainer( DEFINE_CONST_UNICODE("plugin") )
    , bInitialized( Application::IsRemoteServer() )
{
    SFX_APP()->InsertLateInitHdl( LINK( this, SfxPluginFilterContainer, LoadHdl_Impl ) );
}

sal_uInt32 SfxPluginFilterContainer::Execute( SfxMedium& rMedium, SfxFrame*& pFrame) const
{
/*
    rMedium.Close();
    ::rtl::OUString aTargetURL = rMedium.GetURLObject().GetMainURL();
    rMedium.GetItemSet()->Put( SfxStringItem( SID_CONTENTTYPE, rMedium.GetFilter()->GetMimeType() ) );
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
    TransformItems( SID_OPENDOC, *rMedium.GetItemSet(), aArgs );
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrameLoader >  xLoader( (::cppu::OWeakObject*)new PluginLoader(::utl::getProcessServiceFactory()), ::com::sun::star::uno::UNO_QUERY );
    rMedium.GetItemSet()->Put( SfxStringItem( SID_FILTER_NAME, rMedium.GetFilter()->GetName() ) );
    USHORT nRet = pFrame->LoadComponent_Impl( aTargetURL, aArgs, xLoader, rMedium.GetItemSet() );

    if ( nRet == RET_NEWTASK )
        rMedium.GetItemSet()->Put( SfxStringItem( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") ) );

    if ( nRet == RET_OK || nRet == RET_CANCEL )
        return ERRCODE_ABORT;
    else if ( nRet == RET_NEWTASK )
        return ERRCODE_SFX_RESTART;
    else                        // nRet == RET_PENDING !
        return ERRCODE_IO_PENDING;
 */
    return ERRCODE_IO_NOTSUPPORTED;
}

sal_uInt16 SfxPluginFilterContainer::GetFilterCount() const
{
    if ( !bInitialized  )
        SAL_CONST_CAST(SfxPluginFilterContainer*, this)->LoadHdl_Impl( 0 );
    return SfxFilterContainer::GetFilterCount();
}

IMPL_LINK( SfxPluginFilterContainer, LoadHdl_Impl, void*, pVoid )
{
    if ( bInitialized )
        return 0;

    bInitialized = sal_True;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMan = ::utl::getProcessServiceFactory();
    ::com::sun::star::uno::Reference< ::com::sun::star::plugin::XPluginManager >  xPlugMgr( xMan->createInstance( DEFINE_CONST_UNICODE("com.sun.star.plugin.PluginManager") ), ::com::sun::star::uno::UNO_QUERY );
    if( xPlugMgr.is() )
    {
        // Alle MIME-Types, f"ur die Plugins registriert sind, werden als
        // Filter registriert
        ::com::sun::star::uno::Sequence< ::com::sun::star::plugin::PluginDescription > aDescr = xPlugMgr->getPluginDescriptions();
        const ::com::sun::star::plugin::PluginDescription *pArr = aDescr.getConstArray();
        for ( sal_uInt16 n=0; n<aDescr.getLength(); n++ )
        {
            ::rtl::OUString aExt = pArr[n].Extension;
            if ( aExt.compareToAscii("*") != 0 )
            {
                // Den default-Filter wollen wir nicht, da die Filter dazu
                // dienen sollen, registrierte PlugIns beim Laden als Dokument
                // zu erkennen

                String aExtension( aExt );

                // Fix fuer 3.0 PlugIns mit mehreren FileExt.
                sal_uInt16 nPos = 0;
                while( (nPos = aExtension.SearchAndReplace(',', ';', nPos)) != STRING_NOTFOUND )
                    ;
//                  aExtension.Insert("*.", nPos+1);
//              aExtension.Insert( "*.", 0 );
                String aPlug = pArr[n].Description;
                aPlug += DEFINE_CONST_UNICODE(" (PlugIn)");
                String aEmpty;
                SfxFilter *pFilter = new SfxFilter( aPlug, aExtension,
                    SFX_FILTER_PLUGIN|SFX_FILTER_IMPORT|SFX_FILTER_EXECUTABLE,
                    0, String(), aPlug, 0,  pArr[n].Mimetype, this, String() );
                AddFilter( pFilter, GetFilterCount() );
            }
        }
    }
    else
        new SfxAsyncServiceErrorHandler_Impl( DEFINE_CONST_UNICODE("PluginManager") );

    return 0;
}

const SfxFilter* SfxPluginFilterContainer::GetFilter4Protocol( SfxMedium& rMed, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    if ( !bInitialized )
        SAL_CONST_CAST(SfxPluginFilterContainer*, this)->LoadHdl_Impl( 0 );
    return SfxFilterContainer::GetFilter4Protocol( rMed, nMust, nDont );
}


