/*************************************************************************
 *
 *  $RCSfile: appcfg.cxx,v $
 *
 *  $Revision: 1.47 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-22 11:08:41 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PropertyValue_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXSLSTITM_HXX //autogen
#include <svtools/slstitm.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSZITEM_HXX
#include <svtools/szitem.hxx>
#endif
#ifndef _UNDO_HXX //autogen
#include <svtools/undo.hxx>
#endif

#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>

#ifndef _SVTOOLS_TTPROPS_HXX // handmade
#include <svtools/ttprops.hxx>
#endif
#ifndef _SFXSIDS_HRC // handmade
#include <sfxsids.hrc>
#endif
#ifndef _SOT_EXCHANGE_HXX //autogen
#include <sot/exchange.hxx>
#endif

#pragma hdrstop

#include <svtools/agprop.hxx>
#include <sj2/sjapplet.hxx>

#ifndef _SFXISETHINT_HXX
#include <svtools/isethint.hxx>
#endif

#include <unotools/configmgr.hxx>
#include <tools/urlobj.hxx>
#include <tools/wldcrd.hxx>
#include <svtools/saveopt.hxx>
#include <svtools/helpopt.hxx>
#include <svtools/undoopt.hxx>
#include <svtools/securityoptions.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/inetoptions.hxx>
#include <svtools/miscopt.hxx>
#include <vcl/toolbox.hxx>
#include <unotools/localfilehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>

#include "docfile.hxx"
#include "viewfrm.hxx"
#include "sfxhelp.hxx"
#include "sfxtypes.hxx"
#include "dispatch.hxx"
#include "objsh.hxx"
#include "objshimp.hxx"
#include "viewsh.hxx"
#include "request.hxx"
#include "evntconf.hxx"
#include "cfgmgr.hxx"
#include "docinf.hxx"
#include "appdata.hxx"
#include "workwin.hxx"
#include <misccfg.hxx>
#include <macrconf.hxx>
#include "appimp.hxx"
#include "helper.hxx"   // SfxContentHelper::...
#include "app.hrc"
#include "sfxresid.hxx"
#include "shutdownicon.hxx"
#include "imgmgr.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

//-------------------------------------------------------------------------

class SfxEventAsyncer_Impl : public SfxListener
{
    SfxEventHint        aHint;
    Timer*              pTimer;

public:

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    SfxEventAsyncer_Impl( const SfxEventHint& rHint );
    ~SfxEventAsyncer_Impl();
    DECL_LINK( TimerHdl, Timer*);
};

// -----------------------------------------------------------------------

void SfxEventAsyncer_Impl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SfxSimpleHint* pHint = PTR_CAST( SfxSimpleHint, &rHint );
    if( pHint && pHint->GetId() == SFX_HINT_DYING && pTimer->IsActive() )
    {
        pTimer->Stop();
        delete this;
    }
}

// -----------------------------------------------------------------------

SfxEventAsyncer_Impl::SfxEventAsyncer_Impl( const SfxEventHint& rHint )
 : aHint( rHint )
{
    if( rHint.GetObjShell() )
        StartListening( *rHint.GetObjShell() );
    pTimer = new Timer;
    pTimer->SetTimeoutHdl( LINK(this, SfxEventAsyncer_Impl, TimerHdl) );
    pTimer->SetTimeout( 0 );
    pTimer->Start();
}

// -----------------------------------------------------------------------

SfxEventAsyncer_Impl::~SfxEventAsyncer_Impl()
{
    delete pTimer;
}

// -----------------------------------------------------------------------

IMPL_LINK(SfxEventAsyncer_Impl, TimerHdl, Timer*, pTimer)
{
    pTimer->Stop();
    SFX_APP()->Broadcast( aHint );
    if ( aHint.GetObjShell() )
    {
        SfxObjectShellRef xRef( aHint.GetObjShell() );
        aHint.GetObjShell()->Broadcast( aHint );
    }

    delete this;
    return 0L;
}

/*
const USHORT* SfxApplication::GetOptionsRanges() const
{
    static USHORT pRange[] =
    {
        0, 0,
        0, 0,
        0, 0,
        0, 0,
        0, 0,
        0, 0,
        0,
    };

    if (0 == pRange[0])
    {
        SfxItemPool &rPool = GetPool();
        pRange[ 0] = SID_OPTIONS_START;
        pRange[ 1] = SID_OPTIONS_FIRSTFREE-1;
        pRange[ 2] = SID_HELPBALLOONS;
        pRange[ 3] = SID_HELPTIPS;
        pRange[ 4] = SID_SECURE_URL;
        pRange[ 5] = SID_SECURE_URL;
        pRange[ 6] = SID_BASIC_ENABLED;
        pRange[ 7] = SID_BASIC_ENABLED;
        pRange[ 8] = SID_AUTO_ADJUSTICONS;
        pRange[ 9] = SID_ICONGRID;
        pRange[ 10 ] = SID_RESTORE_EXPAND_STATE;
        pRange[ 11 ] = SID_RESTORE_EXPAND_STATE;
    }
    return pRange;
}
*/
//--------------------------------------------------------------------

BOOL SfxApplication::GetOptions( SfxItemSet& rSet )
{
    BOOL bRet = FALSE;
    SfxItemPool &rPool = GetPool();
    String aTRUEStr = 0x0031; // ^= '1'

    const USHORT *pRanges = rSet.GetRanges();
    SvtSaveOptions aSaveOptions;
    SvtUndoOptions aUndoOptions;
    SvtHelpOptions aHelpOptions;
    SvtInetOptions aInetOptions;
    SvtSecurityOptions  aSecurityOptions;
    SvtMiscOptions aMiscOptions;

    while ( *pRanges )
    {
        for(USHORT nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich)
        {
            switch(nWhich)
            {
                case SID_ATTR_BUTTON_OUTSTYLE3D :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_BUTTON_OUTSTYLE3D ),
                              aMiscOptions.GetToolboxStyle() != TOOLBOX_STYLE_FLAT)))
                        bRet = TRUE;
                    break;
                case SID_ATTR_BUTTON_BIGSIZE :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_BUTTON_BIGSIZE ),
                              SfxImageManager::GetCurrentSymbolSet() == SFX_SYMBOLS_LARGE)))
                        bRet = TRUE;
                    break;
                case SID_ATTR_BACKUP :
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_BACKUP))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_BACKUP ),aSaveOptions.IsBackup())))
                                bRet = FALSE;
                    }
                    break;
                case SID_ATTR_PRETTYPRINTING:
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_DOPRETTYPRINTING))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_PRETTYPRINTING ), aSaveOptions.IsPrettyPrinting())))
                                bRet = FALSE;
                    }
                    break;
                case SID_ATTR_AUTOSAVE :
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_AUTOSAVE))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_AUTOSAVE ), aSaveOptions.IsAutoSave())))
                                bRet = FALSE;
                    }
                    break;
                case SID_ATTR_AUTOSAVEPROMPT :
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_AUTOSAVEPROMPT))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_AUTOSAVEPROMPT ), aSaveOptions.IsAutoSavePrompt())))
                                bRet = FALSE;
                    }
                    break;
                case SID_ATTR_AUTOSAVEMINUTE :
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_AUTOSAVETIME))
                            if (!rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_ATTR_AUTOSAVEMINUTE ), (UINT16)aSaveOptions.GetAutoSaveTime())))
                                bRet = FALSE;
                    }
                    break;
                case SID_ATTR_DOCINFO :
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_DOCINFSAVE))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_DOCINFO ), aSaveOptions.IsDocInfoSave())))
                                bRet = FALSE;
                    }
                    break;
                case SID_OPT_SAVEGRAPHICSCOMPRESSED :
                {
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVEGRAPHICS))
                        {
                            SfxDocumentInfo *pDocInf = SfxObjectShell::Current() ? &SfxObjectShell::Current()->GetDocInfo() : 0;
                            BOOL bComprGraph = pDocInf ? pDocInf->IsSaveGraphicsCompressed() : aSaveOptions.GetSaveGraphicsMode() == SvtSaveOptions::SaveGraphicsCompressed;
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_OPT_SAVEGRAPHICSCOMPRESSED ),bComprGraph )))
                                bRet = FALSE;
                        }
                    }
                    break;
                }
                case SID_OPT_SAVEORIGINALGRAPHICS :
                {
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVEGRAPHICS))
                        {
                            SfxDocumentInfo *pDocInf = SfxObjectShell::Current() ? &SfxObjectShell::Current()->GetDocInfo() : 0;
                            BOOL bOrigGraph = pDocInf ? pDocInf->IsSaveOriginalGraphics() : aSaveOptions.GetSaveGraphicsMode() == SvtSaveOptions::SaveGraphicsOriginal;
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_OPT_SAVEORIGINALGRAPHICS ), bOrigGraph )))
                                bRet = FALSE;
                        }
                    }
                    break;
                }
                case SID_ATTR_WORKINGSET :
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVEWORKINGSET))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_WORKINGSET ), aSaveOptions.IsSaveWorkingSet())))
                                bRet = FALSE;
                    }
                    break;
                case SID_ATTR_SAVEDOCWINS :
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVEDOCWINS))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_SAVEDOCWINS ), aSaveOptions.IsSaveDocWins())))
                                bRet = FALSE;
                    }
                    break;
                case SID_ATTR_SAVEDOCVIEW :
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVEDOCVIEW))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_SAVEDOCVIEW ), aSaveOptions.IsSaveDocView())))
                                bRet = FALSE;
                    }
                    break;
                case SID_ATTR_METRIC :
//                    if(rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_ATTR_METRIC ),
//                                pOptions->GetMetric() ) ) )
//                        bRet = TRUE;
                    break;
                case SID_HELPBALLOONS :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_HELPBALLOONS ),
                               aHelpOptions.IsExtendedHelp() ) ) )
                        bRet = TRUE;
                    break;
                case SID_HELPTIPS :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_HELPTIPS ),
                               aHelpOptions.IsHelpTips() ) ) )
                        bRet = TRUE;
                    break;
                case SID_ATTR_AUTOHELPAGENT :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_ATTR_AUTOHELPAGENT ),
                               aHelpOptions.IsHelpAgentAutoStartMode() ) ) )
                        bRet = TRUE;
                    break;
                case SID_HELPAGENT_TIMEOUT :
                    if ( rSet.Put( SfxInt32Item( rPool.GetWhich( SID_HELPAGENT_TIMEOUT ),
                                                 aHelpOptions.GetHelpAgentTimeoutPeriod() ) ) )
                        bRet = TRUE;
                    break;
                case SID_ATTR_WELCOMESCREEN :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_ATTR_WELCOMESCREEN ),
                               aHelpOptions.IsWelcomeScreen() ) ) )
                        bRet = TRUE;
                    break;
                case SID_HELP_STYLESHEET :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_HELP_STYLESHEET ),
                               aHelpOptions.GetHelpStyleSheet() ) ) )
                        bRet = TRUE;
                break;
                case SID_ATTR_UNDO_COUNT :
                    if(rSet.Put( SfxUInt16Item ( rPool.GetWhich( SID_ATTR_UNDO_COUNT ),
                                 (UINT16)aUndoOptions.GetUndoCount() ) ) )
                        bRet = TRUE;
                    break;
                case SID_ATTR_QUICKLAUNCHER :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_ATTR_QUICKLAUNCHER ),
                        ShutdownIcon::GetAutostart()  ) ) )
                        bRet = TRUE;
                    break;
                case SID_SAVEREL_INET :
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVERELINET))
                            if (!rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_SAVEREL_INET ), aSaveOptions.IsSaveRelINet() )))
                                bRet = FALSE;
                    }
                    break;
                case SID_SAVEREL_FSYS :
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVERELFSYS))
                            if (!rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_SAVEREL_FSYS ), aSaveOptions.IsSaveRelFSys() )))
                                bRet = FALSE;
                    }
                    break;
                case SID_BASIC_ENABLED :
                    {
                        bRet = TRUE;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::E_BASICMODE))
                        {
                            if ( !rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_BASIC_ENABLED ), aSecurityOptions.GetBasicMode())))
                                bRet = FALSE;
                        }
                    }
                    break;
                case SID_INET_EXE_PLUGIN  :
                    {
                        bRet = TRUE;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::E_EXECUTEPLUGINS))
                        {
                            if ( !rSet.Put( SfxBoolItem( SID_INET_EXE_PLUGIN, aSecurityOptions.IsExecutePlugins() ) ) )
                                bRet = FALSE;
                        }
                    }
                    break;
                case SID_MACRO_WARNING :
                    {
                        bRet = TRUE;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::E_WARNING))
                        {
                            if ( !rSet.Put( SfxBoolItem( SID_MACRO_WARNING, aSecurityOptions.IsWarningEnabled() ) ) )
                                bRet = FALSE;
                        }
                    }
                    break;
                case SID_MACRO_CONFIRMATION :
                    {
                        bRet = TRUE;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::E_CONFIRMATION))
                        {
                            if ( !rSet.Put( SfxBoolItem( SID_MACRO_CONFIRMATION, aSecurityOptions.IsConfirmationEnabled() ) ) )
                                bRet = FALSE;
                        }
                    }
                    break;
                case SID_SECURE_URL :
                    {
                        bRet = TRUE;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::E_SECUREURLS))
                        {
                            ::com::sun::star::uno::Sequence< ::rtl::OUString > seqURLs = aSecurityOptions.GetSecureURLs();
                            List aList;
                            sal_uInt32 nCount = seqURLs.getLength();
                            sal_uInt32 nURL;
                            for( nURL=0; nURL<nCount; ++nURL )
                            {
                                aList.Insert( new String( seqURLs[nURL] ), LIST_APPEND );
                            }
                            if( !rSet.Put( SfxStringListItem( rPool.GetWhich(SID_SECURE_URL),
                                    &aList ) ) )
                            {
                                bRet = FALSE;
                            }
                            for( nURL=0; nURL<nCount; ++nURL )
                            {
                                delete (String*)aList.GetObject(nURL);
                            }
                            aList.Clear();
                        }
                    }
                    break;
                case SID_ENABLE_METAFILEPRINT :
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                    DBG_ASSERT(sal_False, "SfxApplication::GetOptions()\nSoffice.ini key \"Common\\MetafilePrint\" is obsolete! .. How I can support SID_ENABLE_METAFILEPRINT any longer?\n");
#endif
                    break;
                case SID_INET_PROXY_TYPE :
                {
                    if ( IsPlugin() )
                    {
                        UINT16 nType = 1; // default is "use browser settings"!
                        String sName    ; // set it only for type=2! otherwise=defaults!
                        INT32  nPort = 0;
                        // Use propertyset of remote(!) login dialog service to get right informations ....
                        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xRemoteProxyConfig( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_OUSTRING("com.sun.star.comp.framework.LoginDialog")), ::com::sun::star::uno::UNO_QUERY );
                        if( xRemoteProxyConfig.is() == sal_True )
                        {
                            ::com::sun::star::uno::Any aPropValue = xRemoteProxyConfig->getPropertyValue( DEFINE_CONST_OUSTRING("UseProxy") );
                            ::rtl::OUString sProxyType;
                            aPropValue >>= sProxyType;
                            if( sProxyType.compareToAscii("none")==0 )
                                nType = 0;
                            else
                            if( sProxyType.compareToAscii("browser")==0 )
                                nType = 1;
                            else
                            if( sProxyType.compareToAscii("custom")==0 )
                            {
                                nType = 2;
                                aPropValue = xRemoteProxyConfig->getPropertyValue( DEFINE_CONST_OUSTRING("SecurityProxy") );
                                ::rtl::OUString sProxyValues;
                                aPropValue >>= sProxyValues;

                                if( sProxyValues.indexOf( (sal_Unicode)':' ) > 0 )
                                {
                                    sal_Int32 nToken = 0;
                                    sName = sProxyValues.getToken( 0, (sal_Unicode)':', nToken );
                                    if( nToken != -1 )
                                        nPort = sProxyValues.getToken( 0, (sal_Unicode)':', nToken ).toInt32();
                                }
                            }
                        }
                        if(
                            ( rSet.Put( SfxUInt16Item ( rPool.GetWhich( SID_INET_PROXY_TYPE      ), nType )))    &&
                            ( rSet.Put( SfxStringItem ( rPool.GetWhich( SID_INET_HTTP_PROXY_NAME ), sName )))    &&
                            ( rSet.Put( SfxInt32Item  ( rPool.GetWhich( SID_INET_HTTP_PROXY_PORT ), nPort )))
                          )
                        {
                            bRet = TRUE;
                        }
                    }
                    else if( rSet.Put( SfxUInt16Item ( rPool.GetWhich( SID_INET_PROXY_TYPE ),
                                (UINT16)aInetOptions.GetProxyType() )))
                            bRet = TRUE;
                    break;
                }
                case SID_INET_HTTP_PROXY_NAME :
                {
                    if ( IsPlugin() )
                    {
                        // This value is neccessary for SID_INET_PROXY_TYPE=2 only!
                        // So we do nothing here! No defaults (otherwise we overwrite real values!)
                        // no right values (they are superflous then) ...
                        // We set it for SID_INET_PROXY_TYPE queries only!!!
                        bRet = TRUE;
                    }
                    else if ( rSet.Put( SfxStringItem ( rPool.GetWhich(SID_INET_HTTP_PROXY_NAME ),
                            aInetOptions.GetProxyHttpName() )))
                        bRet = TRUE;
                    break;
                }
                case SID_INET_HTTP_PROXY_PORT :
                    if ( IsPlugin() )
                    {
                        // This value is neccessary for SID_INET_PROXY_TYPE=2 only!
                        // So we do nothing here! No defaults (otherwise we overwrite real values!)
                        // no right values (they are superflous then) ...
                        // We set it for SID_INET_PROXY_TYPE queries only!!!
                        bRet = TRUE;
                    }
                    else if ( rSet.Put( SfxInt32Item( rPool.GetWhich(SID_INET_HTTP_PROXY_PORT ),
                            aInetOptions.GetProxyHttpPort() )))
                        bRet = TRUE;
                    break;
                case SID_INET_FTP_PROXY_NAME :
                    if ( !IsPlugin() && rSet.Put( SfxStringItem ( rPool.GetWhich(SID_INET_FTP_PROXY_NAME ),
                            aInetOptions.GetProxyFtpName() )))
                        bRet = TRUE;
                    break;
                case SID_INET_FTP_PROXY_PORT :
                    if ( !IsPlugin() && rSet.Put( SfxInt32Item ( rPool.GetWhich(SID_INET_FTP_PROXY_PORT ),
                            aInetOptions.GetProxyFtpPort() )))
                        bRet = TRUE;
                    break;
                case SID_INET_SECURITY_PROXY_NAME :
                case SID_INET_SECURITY_PROXY_PORT :
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                    DBG_ASSERT( sal_False, "SfxApplication::GetOptions()\nSome INET values no longer supported!\n" );
#endif
                    break;
                case SID_INET_NOPROXY :
                    if( !IsPlugin() && rSet.Put( SfxStringItem ( rPool.GetWhich( SID_INET_NOPROXY),
                                aInetOptions.GetProxyNoProxy() )))
                        bRet = TRUE;
                    break;
                case SID_ATTR_PATHNAME :
                case SID_ATTR_PATHGROUP :
                {
                    SfxAllEnumItem aNames(rPool.GetWhich(SID_ATTR_PATHGROUP));
                    SfxAllEnumItem aValues(rPool.GetWhich(SID_ATTR_PATHNAME));
                    SvtPathOptions aPathCfg;
                    for ( int nProp = SvtPathOptions::PATH_ADDIN;
                          nProp <= SvtPathOptions::PATH_WORK; nProp++ )
                    {
                        const String aName( SfxResId( CONFIG_PATH_START + nProp ) );
                        aNames.InsertValue( nProp, aName );
                        String aValue;
                        switch ( nProp )
                        {
                            case SvtPathOptions::PATH_ADDIN:        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aPathCfg.GetAddinPath(), aValue ); break;
                            case SvtPathOptions::PATH_AUTOCORRECT:  aValue = aPathCfg.GetAutoCorrectPath(); break;
                            case SvtPathOptions::PATH_AUTOTEXT:     aValue = aPathCfg.GetAutoTextPath(); break;
                            case SvtPathOptions::PATH_BACKUP:       aValue = aPathCfg.GetBackupPath(); break;
                            case SvtPathOptions::PATH_BASIC:        aValue = aPathCfg.GetBasicPath(); break;
                            case SvtPathOptions::PATH_BITMAP:       aValue = aPathCfg.GetBitmapPath(); break;
                            case SvtPathOptions::PATH_CONFIG:       aValue = aPathCfg.GetConfigPath(); break;
                            case SvtPathOptions::PATH_DICTIONARY:   aValue = aPathCfg.GetDictionaryPath(); break;
                            case SvtPathOptions::PATH_FAVORITES:    aValue = aPathCfg.GetFavoritesPath(); break;
                            case SvtPathOptions::PATH_FILTER:       ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aPathCfg.GetFilterPath(), aValue ); break;
                            case SvtPathOptions::PATH_GALLERY:      aValue = aPathCfg.GetGalleryPath(); break;
                            case SvtPathOptions::PATH_GRAPHIC:      aValue = aPathCfg.GetGraphicPath(); break;
                            case SvtPathOptions::PATH_HELP:         ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aPathCfg.GetHelpPath(), aValue ); break;
                            case SvtPathOptions::PATH_LINGUISTIC:   aValue = aPathCfg.GetLinguisticPath(); break;
                            case SvtPathOptions::PATH_MODULE:       ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aPathCfg.GetModulePath(), aValue ); break;
                            case SvtPathOptions::PATH_PALETTE:      aValue = aPathCfg.GetPalettePath(); break;
                            case SvtPathOptions::PATH_PLUGIN:       ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aPathCfg.GetPluginPath(), aValue ); break;
                            case SvtPathOptions::PATH_STORAGE:      ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aPathCfg.GetStoragePath(), aValue ); break;
                            case SvtPathOptions::PATH_TEMP:         aValue = aPathCfg.GetTempPath(); break;
                            case SvtPathOptions::PATH_TEMPLATE:     aValue = aPathCfg.GetTemplatePath(); break;
                            case SvtPathOptions::PATH_USERCONFIG:   aValue = aPathCfg.GetUserConfigPath(); break;
                            case SvtPathOptions::PATH_USERDICTIONARY: aValue = aPathCfg.GetUserDictionaryPath(); break;
                            case SvtPathOptions::PATH_WORK:         aValue = aPathCfg.GetWorkPath(); break;
                        }
                        aValues.InsertValue( nProp, aValue );
                    }

                    if ( rSet.Put(aNames) || rSet.Put(aValues) )
                        bRet = TRUE;
                }

                default:
                    DBG_WARNING( "W1:Wrong ID while getting Options!" );
                    break;
            }
#ifdef DBG_UTIL
            if ( !bRet )
                DBG_ERROR( "Putting options failed!" );
#endif
        }
        pRanges++;
    }

    return bRet;
}

//--------------------------------------------------------------------
BOOL SfxApplication::IsSecureURL( const INetURLObject& rURL, const String* pReferer ) const
{
    return SvtSecurityOptions().IsSecureURL( rURL.GetMainURL( INetURLObject::NO_DECODE ), *pReferer );
}
//--------------------------------------------------------------------

void SfxApplication::SetOptions_Impl( const SfxItemSet& rSet )
{
    const SfxPoolItem *pItem = 0;
    SfxItemPool &rPool = GetPool();
    BOOL bResetSession = FALSE;
    BOOL bProxiesModified = FALSE;

    SvtSaveOptions aSaveOptions;
    SvtUndoOptions aUndoOptions;
    SvtHelpOptions aHelpOptions;
    SvtSecurityOptions aSecurityOptions;
    SvtPathOptions aPathOptions;
    SvtInetOptions aInetOptions;
    SvtMiscOptions aMiscOptions;
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_BUTTON_OUTSTYLE3D), TRUE, &pItem) )
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        USHORT nOutStyle =
            ( (const SfxBoolItem *)pItem)->GetValue() ? 0 : TOOLBOX_STYLE_FLAT;
        aMiscOptions.SetToolboxStyle( nOutStyle );
    }

    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_BUTTON_BIGSIZE), TRUE, &pItem) )
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL bBigSize = ( (const SfxBoolItem*)pItem )->GetValue();
        aMiscOptions.SetSymbolSet( bBigSize ? SFX_SYMBOLS_LARGE : SFX_SYMBOLS_SMALL );
        SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
        while ( pViewFrame )
        {
            // update all "final" dispatchers
            if ( !pViewFrame->GetActiveChildFrame_Impl() )
                pViewFrame->GetDispatcher()->Update_Impl(sal_True);
            pViewFrame = SfxViewFrame::GetNext(*pViewFrame);
        }
    }

    // Backup
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_BACKUP), TRUE, &pItem) )
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetBackup( ( (const SfxBoolItem*)pItem )->GetValue() );
    }

    // PrettyPrinting
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_ATTR_PRETTYPRINTING ), TRUE, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA( SfxBoolItem ), "BoolItem expected" );
        aSaveOptions.SetPrettyPrinting( static_cast< const SfxBoolItem*> ( pItem )->GetValue() );
    }

    // AutoSave
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOSAVE), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetAutoSave( ( (const SfxBoolItem*)pItem )->GetValue() );
    }

    // AutoSave-Propt
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOSAVEPROMPT), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetAutoSavePrompt(((const SfxBoolItem *)pItem)->GetValue());
    }

    // AutoSave-Time
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOSAVEMINUTE), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "UInt16Item expected");
        aSaveOptions.SetAutoSaveTime(((const SfxUInt16Item *)pItem)->GetValue());
    }

    // DocInfo
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_DOCINFO), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetDocInfoSave(((const SfxBoolItem *)pItem)->GetValue());
    }

    // Grafiken komprimiert speichern
    SvtSaveOptions::SaveGraphicsMode eMode = SvtSaveOptions::SaveGraphicsNormal;
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_OPT_SAVEGRAPHICSCOMPRESSED), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL b = ((const SfxBoolItem *)pItem)->GetValue();
        if ( b )
            eMode = SvtSaveOptions::SaveGraphicsCompressed;
    }

    // Grafiken im Original speichern
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_OPT_SAVEORIGINALGRAPHICS), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL b = ((const SfxBoolItem *)pItem)->GetValue();
        if ( b )
            eMode = SvtSaveOptions::SaveGraphicsOriginal;
    }

    if ( eMode != aSaveOptions.GetSaveGraphicsMode() )
        aSaveOptions.SetSaveGraphicsMode( eMode );

    // offende Dokumente merken
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_WORKINGSET), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetSaveWorkingSet(((const SfxBoolItem *)pItem)->GetValue());
    }

    // offene Fenster speichern
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_SAVEDOCWINS), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetSaveDocWins(((const SfxBoolItem *)pItem)->GetValue());
    }

    // Fenster-Einstellung speichern
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_SAVEDOCVIEW), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetSaveDocView(((const SfxBoolItem *)pItem)->GetValue());
    }

    // Metric
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_METRIC), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "UInt16Item expected");
//        pOptions->SetMetric((FieldUnit)((const SfxUInt16Item*)pItem)->GetValue());
    }

    // HelpBalloons
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_HELPBALLOONS), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aHelpOptions.SetExtendedHelp(((const SfxBoolItem *)pItem)->GetValue());
    }

    // HelpTips
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_HELPTIPS), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aHelpOptions.SetHelpTips(((const SfxBoolItem *)pItem)->GetValue());
    }

    // AutoHelpAgent
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOHELPAGENT ), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aHelpOptions.SetHelpAgentAutoStartMode( ((const SfxBoolItem *)pItem)->GetValue() );
    }

    // help agent timeout
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_HELPAGENT_TIMEOUT ), TRUE, &pItem ) )
    {
        DBG_ASSERT(pItem->ISA(SfxInt32Item), "Int32Item expected");
        aHelpOptions.SetHelpAgentTimeoutPeriod( ( (const SfxInt32Item*)pItem )->GetValue() );
    }

    // WelcomeScreen
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_WELCOMESCREEN ), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aHelpOptions.SetWelcomeScreen( ((const SfxBoolItem *)pItem)->GetValue() );
    }

    // WelcomeScreen
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_WELCOMESCREEN_RESET ), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL bReset = ((const SfxBoolItem *)pItem)->GetValue();
        if ( bReset )
        {
            DBG_ERROR( "Not implemented, may be EOL!" );
        }                                                   }

    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_HELP_STYLESHEET ), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        aHelpOptions.SetHelpStyleSheet( ((const SfxStringItem *)pItem)->GetValue() );
    }

    // SaveRelINet
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_SAVEREL_INET), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetSaveRelINet(((const SfxBoolItem *)pItem)->GetValue());
    }

    // SaveRelFSys
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_SAVEREL_FSYS), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetSaveRelFSys(((const SfxBoolItem *)pItem)->GetValue());
    }

    // Undo-Count
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_UNDO_COUNT), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "UInt16Item expected");
        USHORT nUndoCount = ((const SfxUInt16Item*)pItem)->GetValue();
        aUndoOptions.SetUndoCount( nUndoCount );

        // um alle Undo-Manager zu erwischen: "uber alle Frames iterieren
        for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst();
              pFrame;
              pFrame = SfxViewFrame::GetNext(*pFrame) )
        {
            // den Dispatcher des Frames rausholen
            SfxDispatcher *pDispat = pFrame->GetDispatcher();
            pDispat->Flush();

            // "uber alle SfxShells auf dem Stack des Dispatchers iterieren
            USHORT nIdx = 0;
            for ( SfxShell *pSh = pDispat->GetShell(nIdx);
                  pSh;
                  ++nIdx, pSh = pDispat->GetShell(nIdx) )
            {
                SfxUndoManager *pUndoMgr = pSh->GetUndoManager();
                if ( pUndoMgr )
                    pUndoMgr->SetMaxUndoActionCount( nUndoCount );
            }
        }
    }

    // Office autostart
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_QUICKLAUNCHER), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        ShutdownIcon::SetAutostart( ( (const SfxBoolItem*)pItem )->GetValue() != FALSE );
    }

    // StarBasic Enable
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_BASIC_ENABLED, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "SfxInt16Item expected");
        aSecurityOptions.SetBasicMode( (EBasicSecurityMode)( (const SfxUInt16Item*)pItem )->GetValue() );
    }

    // Execute PlugIns
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_EXE_PLUGIN, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        aSecurityOptions.SetExecutePlugins( ( (const SfxBoolItem *)pItem )->GetValue() );
        bResetSession = TRUE;
    }

    if ( IsPlugin() )
    {
        sal_Int32 nMode = 0;
        String aServerName;
        String aPortNumber;
        if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_PROXY_TYPE), TRUE, &pItem))
        {
            DBG_ASSERT( pItem->ISA(SfxUInt16Item), "UInt16Item expected" );
            nMode = ((const SfxUInt16Item*)pItem )->GetValue();
        }

        if ( nMode == 2 )
        {
            if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_HTTP_PROXY_NAME ), TRUE, &pItem ) )
            {
                DBG_ASSERT( pItem->ISA(SfxStringItem), "StringItem expected" );
                aServerName = ((const SfxStringItem *)pItem)->GetValue();
            }
            if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_HTTP_PROXY_PORT ), TRUE, &pItem ) )
            {
                DBG_ASSERT( pItem->ISA(SfxInt32Item), "Int32Item expected" );
                aPortNumber = String::CreateFromInt32( ((const SfxInt32Item*)pItem )->GetValue() );
            }

            if ( !aServerName.Len() || !aPortNumber.Len() )
                nMode = 0;
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xRemoteProxyConfig( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_OUSTRING("com.sun.star.comp.framework.LoginDialog")), ::com::sun::star::uno::UNO_QUERY );
        if( xRemoteProxyConfig.is() == sal_True )
        {
            ::com::sun::star::uno::Any aPropValue;
            if( nMode==0 )  aPropValue <<= DEFINE_CONST_OUSTRING("none");
            else
            if( nMode==1 )  aPropValue <<= DEFINE_CONST_OUSTRING("browser");
            else
            if( nMode==2 )  aPropValue <<= DEFINE_CONST_OUSTRING("custom");

            xRemoteProxyConfig->setPropertyValue( DEFINE_CONST_OUSTRING("UseProxy"), aPropValue );
            if( nMode == 2 )
            {
                ::rtl::OUStringBuffer sProxyValue;
                sProxyValue.append     ( aServerName );
                sProxyValue.appendAscii( ":"         );
                sProxyValue.append     ( aPortNumber );
                aPropValue <<= sProxyValue.makeStringAndClear();
                xRemoteProxyConfig->setPropertyValue( DEFINE_CONST_OUSTRING("SecurityProxy"), aPropValue );
            }
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushable > xFlush( xRemoteProxyConfig, ::com::sun::star::uno::UNO_QUERY );
            if( xFlush.is() == sal_True )
                xFlush->flush();
        }
    }
    else
    {
        if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_PROXY_TYPE), TRUE, &pItem))
        {
            DBG_ASSERT( pItem->ISA(SfxUInt16Item), "UInt16Item expected" );
            aInetOptions.SetProxyType((SvtInetOptions::ProxyType)( (const SfxUInt16Item*)pItem )->GetValue());
            bResetSession = TRUE;
            bProxiesModified = TRUE;
        }

        if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_HTTP_PROXY_NAME ), TRUE, &pItem ) )
        {
            DBG_ASSERT( pItem->ISA(SfxStringItem), "StringItem expected" );
            aInetOptions.SetProxyHttpName( ((const SfxStringItem *)pItem)->GetValue() );
            bResetSession = TRUE;
            bProxiesModified = TRUE;
        }
        if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_HTTP_PROXY_PORT ), TRUE, &pItem ) )
        {
            DBG_ASSERT( pItem->ISA(SfxInt32Item), "Int32Item expected" );
            aInetOptions.SetProxyHttpPort( ( (const SfxInt32Item*)pItem )->GetValue() );
            bResetSession = TRUE;
            bProxiesModified = TRUE;
        }
        if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_FTP_PROXY_NAME ), TRUE, &pItem ) )
        {
            DBG_ASSERT( pItem->ISA(SfxStringItem), "StringItem expected" );
            aInetOptions.SetProxyFtpName( ((const SfxStringItem *)pItem)->GetValue() );
            bResetSession = TRUE;
            bProxiesModified = TRUE;
        }
        if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_FTP_PROXY_PORT ), TRUE, &pItem ) )
        {
            DBG_ASSERT( pItem->ISA(SfxInt32Item), "Int32Item expected" );
            aInetOptions.SetProxyFtpPort( ( (const SfxInt32Item*)pItem )->GetValue() );
            bResetSession = TRUE;
            bProxiesModified = TRUE;
        }
/*        if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_SOCKS_PROXY_NAME ), TRUE, &pItem ) )
        {
            DBG_ASSERT( pItem->ISA(SfxStringItem), "StringItem expected" );
            aInetOptions.SetProxySocksName( ((const SfxStringItem *)pItem)->GetValue() );
            bResetSession = TRUE;
            bProxiesModified = TRUE;
        }
        if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_SOCKS_PROXY_PORT ), TRUE, &pItem ) )
        {
            DBG_ASSERT( pItem->ISA(SfxInt32Item), "Int32Item expected" );
            aInetOptions.SetProxySocksPort( ( (const SfxInt32Item*)pItem )->GetValue() );
            bResetSession = TRUE;
            bProxiesModified = TRUE;
        }
        if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_DNS_AUTO), TRUE, &pItem))
        {
            DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
            BOOL bIsAuto = ((const SfxBoolItem *)pItem)->GetValue();
            if( bIsAuto )
            {
                aInetOptions.SetDnsIpAddress( String() );
            }
            else
            {
                String aDNS;
                if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_DNS_SERVER), TRUE, &pItem) )
                {
                    DBG_ASSERT(pItem->ISA(SfxStringItem), "SfxStringItem expected");
                    aDNS = ((const SfxStringItem *)pItem)->GetValue();
                }
                aInetOptions.SetDnsIpAddress( aDNS );
            }
            bResetSession = TRUE;
        }*/
        if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_NOPROXY, TRUE, &pItem))
        {
            DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
            aInetOptions.SetProxyNoProxy(((const SfxStringItem *)pItem)->GetValue());
            bResetSession = TRUE;
            bProxiesModified = TRUE;
        }
    }

    // Secure-Referers
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_SECURE_URL, TRUE, &pItem))
    {
        DELETEZ(pAppData_Impl->pSecureURLs);

        DBG_ASSERT(pItem->ISA(SfxStringListItem), "StringListItem expected");
        const List *pList = ((SfxStringListItem*)pItem)->GetList();
        sal_uInt32 nCount = pList->Count();
        ::com::sun::star::uno::Sequence< ::rtl::OUString > seqURLs(nCount);
        for( sal_uInt32 nPosition=0;nPosition<nCount;++nPosition)
        {
            seqURLs[nPosition] = *(const String*)(pList->GetObject(nPosition));
        }
        aSecurityOptions.SetSecureURLs( seqURLs );
    }

    if ( SFX_ITEM_SET == rSet.GetItemState(SID_MACRO_WARNING, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        aSecurityOptions.SetWarningEnabled( ( (const SfxBoolItem *)pItem )->GetValue() );
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_MACRO_CONFIRMATION, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        aSecurityOptions.SetConfirmationEnabled( ( (const SfxBoolItem *)pItem )->GetValue() );
    }

    // EnableMetafilePrint
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_ENABLE_METAFILEPRINT ), TRUE, &pItem ) )
    {
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
        DBG_ASSERT(sal_False, "SfxApplication::SetOptions_Impl()\nsoffice.ini key \"MetafilPrint\" not supported any longer!\n");
#endif
    }

    // AutoSave starten oder anhalten
    UpdateAutoSave_Impl();

    // INet Session neu aufsetzen
    if ( bResetSession )
    {
        try
        {
            SjApplet2::settingsChanged();
        }
        catch ( ... )
        {
            DBG_ERRORFILE( "SjApplet2::settingsChanged() throws an exception" );
        }
    }

    // geaenderte Daten speichern
    aInetOptions.flush();
    SaveConfiguration();
}

//--------------------------------------------------------------------

void SfxApplication::SetOptions(const SfxItemSet &rSet)
{
    SvtPathOptions aPathOptions;

    // Daten werden in DocInfo und IniManager gespeichert
    SfxDocumentInfo *pDocInf = SfxObjectShell::Current()
                                ? &SfxObjectShell::Current()->GetDocInfo()
                                : 0;
    const SfxPoolItem *pItem = 0;
    SfxItemPool &rPool = GetPool();

    SfxAllItemSet aSendSet( rSet );

    // portable Grafiken
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_INDEP_METAFILE), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL b = ((const SfxBoolItem *)pItem)->GetValue();
        if ( pDocInf )
            pDocInf->SetPortableGraphics(b);
    }

    // Grafiken komprimiert speichern
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_OPT_SAVEGRAPHICSCOMPRESSED), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL b = ((const SfxBoolItem *)pItem)->GetValue();
        if ( pDocInf )
            pDocInf->SetSaveGraphicsCompressed(b);
    }

    // Grafiken im Original speichern
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_OPT_SAVEORIGINALGRAPHICS), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL b = ((const SfxBoolItem *)pItem)->GetValue();
        if ( pDocInf )
            pDocInf->SetSaveOriginalGraphics(b);
    }

    // PathName
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_PATHNAME), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxAllEnumItem), "AllEnumItem expected");
        const SfxAllEnumItem* pEnumItem = (const SfxAllEnumItem *)pItem;
        sal_uInt32 nCount = pEnumItem->GetValueCount();
        String aNoChangeStr( ' ' );
        for( sal_uInt32 nPath=0; nPath<nCount; ++nPath )
        {
            String sValue = pEnumItem->GetValueTextByPos((USHORT)nPath);
            if ( sValue != aNoChangeStr )
            {
                switch( nPath )
                {
                    case SvtPathOptions::PATH_ADDIN:
                    {
                        String aTmp;
                        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( sValue, aTmp ) )
                            aPathOptions.SetAddinPath( aTmp );
                        break;
                    }

                    case SvtPathOptions::PATH_AUTOCORRECT:  aPathOptions.SetAutoCorrectPath( sValue );break;
                    case SvtPathOptions::PATH_AUTOTEXT:     aPathOptions.SetAutoTextPath( sValue );break;
                    case SvtPathOptions::PATH_BACKUP:       aPathOptions.SetBackupPath( sValue );break;
                    case SvtPathOptions::PATH_BASIC:        aPathOptions.SetBasicPath( sValue );break;
                    case SvtPathOptions::PATH_BITMAP:       aPathOptions.SetBitmapPath( sValue );break;
                    case SvtPathOptions::PATH_CONFIG:       aPathOptions.SetConfigPath( sValue );break;
                    case SvtPathOptions::PATH_DICTIONARY:   aPathOptions.SetDictionaryPath( sValue );break;
                    case SvtPathOptions::PATH_FAVORITES:    aPathOptions.SetFavoritesPath( sValue );break;
                    case SvtPathOptions::PATH_FILTER:
                    {
                        String aTmp;
                        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( sValue, aTmp ) )
                            aPathOptions.SetFilterPath( aTmp );
                        break;
                    }
                    case SvtPathOptions::PATH_GALLERY:      aPathOptions.SetGalleryPath( sValue );break;
                    case SvtPathOptions::PATH_GRAPHIC:      aPathOptions.SetGraphicPath( sValue );break;
                    case SvtPathOptions::PATH_HELP:
                    {
                        String aTmp;
                        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( sValue, aTmp ) )
                            aPathOptions.SetHelpPath( aTmp );
                        break;
                    }

                    case SvtPathOptions::PATH_LINGUISTIC:   aPathOptions.SetLinguisticPath( sValue );break;
                    case SvtPathOptions::PATH_MODULE:
                    {
                        String aTmp;
                        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( sValue, aTmp ) )
                            aPathOptions.SetModulePath( aTmp );
                        break;
                    }

                    case SvtPathOptions::PATH_PALETTE:      aPathOptions.SetPalettePath( sValue );break;
                    case SvtPathOptions::PATH_PLUGIN:
                    {
                        String aTmp;
                        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( sValue, aTmp ) )
                            aPathOptions.SetPluginPath( aTmp );
                        break;
                    }

                    case SvtPathOptions::PATH_STORAGE:
                    {
                        String aTmp;
                        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( sValue, aTmp ) )
                            aPathOptions.SetStoragePath( aTmp );
                        break;
                    }

                    case SvtPathOptions::PATH_TEMP:         aPathOptions.SetTempPath( sValue );break;
                    case SvtPathOptions::PATH_TEMPLATE:     aPathOptions.SetTemplatePath( sValue );break;
                    case SvtPathOptions::PATH_USERCONFIG:   aPathOptions.SetUserConfigPath( sValue );break;
                    case SvtPathOptions::PATH_USERDICTIONARY:aPathOptions.SetUserDictionaryPath( sValue );break;
                    case SvtPathOptions::PATH_WORK:         aPathOptions.SetWorkPath( sValue );break;
                    default: DBG_ERRORFILE("SfxApplication::SetOptions_Impl()\nInvalid path number found for set directories!");
                }
            }
        }

        aSendSet.ClearItem( rPool.GetWhich( SID_ATTR_PATHNAME ) );
    }

    SetOptions_Impl( rSet );

    // Undo-Count
    Broadcast( SfxItemSetHint( rSet ) );
}

//--------------------------------------------------------------------

void SfxApplication::UpdateAutoSave_Impl()
{
    pImp->pAutoSaveTimer->Stop();

    // AutoSave soll ab jetzt neu anlaufen
    SvtSaveOptions aSaveOptions;
    if ( aSaveOptions.IsAutoSave() )
    {
        pImp->pAutoSaveTimer->SetTimeout( aSaveOptions.GetAutoSaveTime() * 60000 );
        pImp->pAutoSaveTimer->Start();
    }
}

//--------------------------------------------------------------------


Timer* SfxApplication::GetAutoSaveTimer_Impl()
{
    return pImp->pAutoSaveTimer;
}

//--------------------------------------------------------------------

IMPL_LINK( SfxApplication, AutoSaveHdl_Impl, Timer*, pTimer )
{
    SvtSaveOptions aSaveOptions;
    FASTBOOL bAutoSave = aSaveOptions.IsAutoSave() &&
        !bDispatcherLocked &&
        !Application::IsUICaptured() && Application::GetLastInputInterval() > 300;
    if ( bAutoSave )
    {
        SfxViewShell *pVSh = pViewFrame ? pViewFrame->GetViewShell() : 0;
        bAutoSave = pVSh && pVSh->GetWindow() &&
                    !pVSh->GetWindow()->IsMouseCaptured() ;
    }

    if ( bAutoSave )
    {
        SaveAll_Impl( aSaveOptions.IsAutoSavePrompt(), TRUE );
        pImp->bAutoSaveNow = FALSE;
        pImp->pAutoSaveTimer->SetTimeout( aSaveOptions.GetAutoSaveTime() * 60000 );
        pImp->pAutoSaveTimer->Start();
    }
    else if ( aSaveOptions.IsAutoSave() )
    {
        // Wenn wir gelockt sind, dann in 5 Sekunden nochmal probieren
        pImp->bAutoSaveNow = TRUE;
        pImp->pAutoSaveTimer->SetTimeout( 5000 );
        pImp->pAutoSaveTimer->Start();

#ifndef PRODUCT
        Sound::Beep();
#endif
    }

    return 0;
}

//--------------------------------------------------------------------

// alle Dokumente speichern

BOOL SfxApplication::SaveAll_Impl(BOOL bPrompt, BOOL bAutoSave)
{
    BOOL bFunc = TRUE;
    short nRet;

    for ( SfxObjectShell *pDoc = SfxObjectShell::GetFirst();
          pDoc;
          pDoc = SfxObjectShell::GetNext(*pDoc) )
    {
        if( SFX_CREATE_MODE_STANDARD == pDoc->GetCreateMode() &&
            SfxViewFrame::GetFirst(pDoc) &&
            !pDoc->IsInModalMode() &&
            !pDoc->HasModalViews() )
        {
            if ( pDoc->GetProgress() == 0 )
            {
                if ( !pDoc->IsModified() )
                    continue;

                if ( bPrompt || (bAutoSave && !pDoc->HasName()) )
                    nRet = QuerySave_Impl( *pDoc, bAutoSave );
                else
                    nRet = RET_YES;

                if ( nRet == RET_YES )
                {
                    SfxRequest aReq( SID_SAVEDOC, 0, pDoc->GetPool() );
                    const SfxPoolItem *pPoolItem = pDoc->ExecuteSlot( aReq );
                    if ( !pPoolItem || !pPoolItem->ISA(SfxBoolItem) ||
                        !( (const SfxBoolItem*) pPoolItem )->GetValue() )
                        bFunc = FALSE;
                }
                else if ( nRet == RET_CANCEL )
                {
                    bFunc = FALSE;
                    break;
                }
                else if ( nRet == RET_NO )
                {
                }
            }
        }
    }
    pImp->aAutoSaveTime=Time();

    return bFunc;
}

//--------------------------------------------------------------------

SfxMacroConfig* SfxApplication::GetMacroConfig() const
{
    return SfxMacroConfig::GetOrCreate();
}

//--------------------------------------------------------------------
#if SUPD < 623
void SfxApplication::RegisterEvent(USHORT nId, const String& rEventName)
{
    if (!pAppData_Impl->pEventConfig)
        pAppData_Impl->pEventConfig = new SfxEventConfiguration;
    String aDummy( "untitled event", RTL_TEXTENCODING_ASCII_US );
    SfxEventConfiguration::RegisterEvent(nId, rEventName, aDummy);
}
#endif

//--------------------------------------------------------------------

SfxEventConfiguration* SfxApplication::GetEventConfig() const
{
    if (!pAppData_Impl->pEventConfig)
        pAppData_Impl->pEventConfig = new SfxEventConfiguration;
    return pAppData_Impl->pEventConfig;
}

//--------------------------------------------------------------------

void SfxApplication::SaveConfiguration() const
{
    // Workingset schreiben?
//    if ( SvtOptions().IsSaveWorkingSet() )
//        SfxTaskManager::SaveWorkingSet();
//(mba/task): Implementierung fehlt

    if ( !pCfgMgr->StoreConfiguration() )
        HandleConfigError_Impl( (sal_uInt16)pCfgMgr->GetErrorCode() );

    utl::ConfigManager::GetConfigManager()->StoreConfigItems();
}

//--------------------------------------------------------------------
void SfxApplication::NotifyEvent( const SfxEventHint& rEventHint, FASTBOOL bSynchron )
{
    DBG_ASSERT(pAppData_Impl->pEventConfig,"Keine Events angemeldet!");

    SfxObjectShell *pDoc = rEventHint.GetObjShell();
    if ( pDoc )
    {
        if ( pDoc->IsPreview() )
            return;
        SFX_ITEMSET_ARG( pDoc->GetMedium()->GetItemSet(), pItem, SfxBoolItem, SID_HIDDEN, sal_False );
        if ( pItem && pItem->GetValue() )
            bSynchron = TRUE;
    }

    // load on demand
    pAppData_Impl->pEventConfig->GetAppEventConfig_Impl();

    if ( bSynchron )
    {
        Broadcast(rEventHint);
        if ( pDoc )
            pDoc->Broadcast( rEventHint );
    }
    else
        new SfxEventAsyncer_Impl( rEventHint );
}

//-------------------------------------------------------------------------
/* ASOBSOLETE
static void CorrectUpdateNumber_Impl(String& rName)
{
    String aUPD( SOLARUPD );
    USHORT nLen = aUPD.Len();
    USHORT nCount,nPos=0;
    do
    {
        nCount=0;
        xub_StrLen nNameLength = rName.Len();
        for ( USHORT i=nPos; i<nNameLength; i++ )
        {
            if ( rName.GetChar(i).CompareToAscii('?') == COMPARE_EQUAL )
            {
                if ( nCount == 0 )
                    nPos=i;
                nCount++;
            }
            else if ( nCount == nLen )
                break;
            else
                nCount=0;
        }
        if ( nCount == nLen )
        {
            rName.Replace( aUPD, nPos );
            nPos += nCount;
        }
    }
    while ( nCount );
}
*/

IMPL_OBJHINT( SfxStringHint, String )

SfxMiscCfg* SfxApplication::GetMiscConfig()
{
    if ( !pAppData_Impl->pMiscConfig )
        pAppData_Impl->pMiscConfig = new SfxMiscCfg;

    return pAppData_Impl->pMiscConfig;
}


