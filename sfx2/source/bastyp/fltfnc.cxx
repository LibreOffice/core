/*************************************************************************
 *
 *  $RCSfile: fltfnc.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: pb $ $Date: 2000-11-23 09:47:21 $
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
#include <comphelper/processfactory.hxx>
#endif

#include <sal/types.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>
#include <rtl/ustring.hxx>
#include <vos/process.hxx>
#include <svtools/pathoptions.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;
using namespace ::vos;

#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#ifndef _SVTOOLS_CTYPEITM_HXX
#include <svtools/ctypeitm.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

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
    String aRet( rStr );
    CharClass aCharClass( Application::GetSettings().GetLocale() );
    aCharClass.toUpper( aRet );
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
    DBG_ASSERT( pConfig, "can not load the filter ini" );
    if( pArg->bInstallIni )
        pConfig->SetGroup( U2S(aString) );

    sal_uInt16 nCount = pArg->bInstallIni ? pConfig->GetKeyCount() : 0;
    String aOver( DEFINE_CONST_UNICODE(SFX_STR_OVERRIDE) );
    String aName, aLine, aUIType, aMimeType, aClipFormat, aMacType, aTypeName, aWild, aFlags, aDefaultTemplate, aUserData;
    for( sal_uInt16 n = 0; n < nCount; n++ )
    {
        aName = pArg->bInstallIni ? String(S2U(pConfig->GetKeyName( n ))) : String();
        aLine = pArg->bInstallIni ? String(S2U(pConfig->ReadKey( n ))) : String();
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
}

//-------------------------------------------------------------------------

sal_uInt32 SfxFilterContainer::GetFilter4Content(
    SfxMedium& rMedium, const SfxFilter** ppFilter,
    SfxFilterFlags, SfxFilterFlags ) const
{
    return 0;
}

//-------------------------------------------------------------------------
#if 0
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

String SfxExecutableFilterContainer::GetBugdocName_Impl(
    const String& rName ) const
/*   [Beschreibung]

     Sucht ein Bugdoc in den ueblichen Verzeichnissen
 */
{
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

    return String();
}

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

#endif

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

    if( !pFilter )
        pFilter = SFX_APP()->GetFilterMatcher().GetFilter4Protocol( rMedium );

    sal_Bool bCheckExternBrowser = sal_False;
    if( !pFilter )
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
                    if( pFilter )
                        nErr = ERRCODE_NONE;
                    else
                        pFilter = pMimeFilter;
                }

                // Bei MIME Typen keinen Storage anfordern
                if( !nErr && rMedium.SupportsMIME_Impl() && pFilter && !pFilter->UsesStorage() )
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

        if( !pFilter && rMedium.IsDownloadDone_Impl() )
        {
            // dann ueber Storage CLSID
            // Remote macht das keinen Sinn, wenn der Download noch la"uft
            SvStorageRef aStor = rMedium.GetStorage();
            if ( aStor.Is() )
                pFilter = GetFilter4ClipBoardId( aStor->GetFormat(), nMust, nDont );

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
        if( !pFilter )
        {
            pFilter = GetFilter4Extension( rMedium.GetURLObject().GetName(), nMust, nDont );
            if( !pFilter || pFilter->GetWildcard()==DEFINE_CONST_UNICODE("*.*") || pFilter->GetWildcard() == '*' )
                pFilter = 0;
        }
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
    const SfxFilter* pOldFilter = *ppFilter;
    const SfxFilter* pFilter = pOldFilter;

    sal_Bool bConsultUser = sal_False;
    sal_Bool bSupportsMime = rMedium.SupportsMIME_Impl();

    // Zunaechst, falls Filter mitkommt einmal testen, ob dieser in Ordnung ist.
    ErrCode nErr = ERRCODE_NONE;
    if( pFilter && ( pFilter->GetFilterContainer()->GetFlags() & SFX_FILTER_CONTAINER_FACTORY ) )
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
            *ppFilter = pFilter;

        if( nErr && nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET && nErr != ERRCODE_SFX_NEVERCHECKCONTENT )
            return nErr;

        if( nErr == ERRCODE_ABORT )
            pFilter = 0;

        // Jetzt wird geprueft, ob das Modul auch einverstanden ist; ist das nicht der Fall, wird auf
        // jeden Fall auf ConsultUser umgeschaltet
        if( pFilter )
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

        rMedium.GetStorage();
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

        rMedium.GetStorage();
        nErr = GetFilter4Content( rMedium, &pFilter, nMust, nDont );
        CHECKERROR();
        if( nErr && (nErr != ERRCODE_ABORT && nErr != ERRCODE_SFX_FORCEQUIET ) )
            return nErr;

        if( nErr == ERRCODE_ABORT && pFilter )
            bConsultUser = sal_True;
    }

    *ppFilter = pFilter;

    if ( ( ( pOldFilter && pOldFilter!=pFilter &&
            ( !pOldFilter->IsOwnTemplateFormat() || !pFilter->IsOwnFormat() ) ) ) &&
             nErr != ERRCODE_SFX_FORCEQUIET || bConsultUser )
        return ERRCODE_SFX_CONSULTUSER;

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
        if ( !pFilter )
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

sal_Bool SfxFilterContainer::IsUsableForRedirects() const
{
    return FALSE;
}

const SfxFilter* SfxExecutableFilterContainer::GetDownloadFilter()
{
    return NULL;
}

const SfxFilter* SfxExecutableFilterContainer::GetChooserFilter()
{
    return NULL;
}
