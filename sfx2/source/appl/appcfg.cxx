/*************************************************************************
 *
 *  $RCSfile: appcfg.cxx,v $
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

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
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
#ifndef _SFXINIPROP_HXX
#include <svtools/iniprop.hxx>
#endif

#ifndef _INET_WRAPPER_HXX
#include <inet/wrapper.hxx>
#endif
#ifndef _INET_CONFIG_HXX
#include <inet/inetcfg.hxx>
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

#include <tools/urlobj.hxx>
#include <tools/wldcrd.hxx>

#include "viewfrm.hxx"
#include "sfxhelp.hxx"
#include "sfxtypes.hxx"
#include "dispatch.hxx"
#include "objsh.hxx"
#include "objshimp.hxx"
#include "saveopt.hxx"
#include "viewsh.hxx"
#include "request.hxx"
#include "evntconf.hxx"
#include "cfgmgr.hxx"
#include "docinf.hxx"
#include "appdata.hxx"
#include "picklist.hxx"
#include "tbxconf.hxx"
#include "workwin.hxx"
#include <misccfg.hxx>
#include <macrconf.hxx>
#include "appimp.hxx"
#include "helper.hxx"   // SfxContentHelper::...

//-------------------------------------------------------------------------

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

// -----------------------------------------------------------------------

String GetJavaIniEntry_Impl( SfxIniKey eKey, SfxIniManager* pIniMgr )
{
    INetURLObject aObj;
    aObj.SetSmartProtocol( INET_PROT_FILE );
    aObj.SetSmartURL( Config::GetConfigName( pIniMgr->Get( SFX_KEY_USERCONFIG_PATH ),
                                             DEFINE_CONST_UNICODE("java") ) );
    String aIniEntry;
    String aIniPath = aObj.getName();
    if ( pIniMgr->SearchFile( aIniPath ) )
    {
        Config aJavaCfg( aIniPath );
        aJavaCfg.SetGroup( "Java" );
        String aIniKey = pIniMgr->GetKeyName( eKey );
        aIniEntry = (String)S2U(aJavaCfg.ReadKey( U2S(aIniKey) ));
    }
    return aIniEntry;
}

// -----------------------------------------------------------------------

BOOL SetJavaIniEntry_Impl( SfxIniKey eKey, const String& rValue, SfxIniManager* pIniMgr )
{
    String aIniFile =
        Config::GetConfigName( pIniMgr->Get( SFX_KEY_USERCONFIG_PATH ), DEFINE_CONST_UNICODE("java") );
    if ( !SfxContentHelper::Exists( aIniFile ) )
    {
        INetURLObject aObj( aIniFile, INET_PROT_FILE );
        String aIniFileName = aObj.getName();
        aObj.SetSmartURL( pIniMgr->Get( SFX_KEY_CONFIG_DIR ) );
        aObj.insertName( aIniFileName );
        String aShareFile = aObj.PathToFileName();

        if ( !SfxContentHelper::Exists( aShareFile ) ||
             !SfxContentHelper::CopyTo( aShareFile, aIniFile ) )
            return FALSE;
    }
    Config aJavaCfg( aIniFile );
    aJavaCfg.SetGroup( "Java" );
    String aIniKey = pIniMgr->GetKeyName( eKey );
    aJavaCfg.WriteKey( U2S(aIniKey), U2S(rValue) );
    return TRUE;
}

//--------------------------------------------------------------------

BOOL SfxApplication::GetOptions( SfxItemSet& rSet )
{
    BOOL bRet = FALSE;
    SfxIniManager *pIni = GetIniManager();
    SfxItemPool &rPool = GetPool();
    SfxToolBoxConfig *pTbxCfg = SfxToolBoxConfig::GetOrCreate();
    String aTRUEStr = 0x0031; // ^= '1'

    const USHORT *pRanges = rSet.GetRanges();
    while ( *pRanges )
    {
        for(USHORT nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich)
        {
            switch(nWhich)
            {
                case SID_ATTR_BUTTON_OUTSTYLE3D :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_BUTTON_OUTSTYLE3D ),
                              pTbxCfg->GetOutStyle() != TOOLBOX_STYLE_FLAT)))
                        bRet = TRUE;
                    break;
                case SID_ATTR_BUTTON_BIGSIZE :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_BUTTON_BIGSIZE ),
                              pTbxCfg->GetSymbolSet() == SFX_SYMBOLS_LARGE_COLOR)))
                        bRet = TRUE;
                    break;
                case SID_ATTR_BACKUP :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_BACKUP ),
                              pOptions->IsBackup())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_AUTOSAVE :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_AUTOSAVE ),
                              pOptions->IsAutoSave())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_AUTOSAVEPROMPT :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_AUTOSAVEPROMPT ),
                              pOptions->IsAutoSavePrompt())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_AUTOSAVEMINUTE :
                    if(rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_ATTR_AUTOSAVEMINUTE ),
                                pOptions->GetAutoSaveTime())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_DOCINFO :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_DOCINFO ),
                              pOptions->IsDocInfoSave())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_INDEP_METAFILE :
                {
                    SfxDocumentInfo *pDocInf = SfxObjectShell::Current() ? &SfxObjectShell::Current()->GetDocInfo() : 0;
                    BOOL bIndepGraph = pDocInf ? pDocInf->IsPortableGraphics() : pOptions->IsIndepGrfFmt();
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_INDEP_METAFILE ),
                              bIndepGraph)))
                        bRet = TRUE;
                    break;
                }
                case SID_OPT_SAVEGRAPHICSCOMPRESSED :
                {
                    SfxDocumentInfo *pDocInf = SfxObjectShell::Current() ? &SfxObjectShell::Current()->GetDocInfo() : 0;
                    BOOL bComprGraph = pDocInf ? pDocInf->IsSaveGraphicsCompressed() : pOptions->IsSaveGraphicsCompressed();
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_OPT_SAVEGRAPHICSCOMPRESSED ),
                              bComprGraph ) ) )
                        bRet = TRUE;
                    break;
                }
                case SID_OPT_SAVEORIGINALGRAPHICS :
                {
                    SfxDocumentInfo *pDocInf = SfxObjectShell::Current() ? &SfxObjectShell::Current()->GetDocInfo() : 0;
                    BOOL bOrigGraph = pDocInf ? pDocInf->IsSaveOriginalGraphics() : pOptions->IsSaveOriginalGraphics();
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_OPT_SAVEORIGINALGRAPHICS ),
                              bOrigGraph ) ) )
                        bRet = TRUE;
                    break;
                }
                case SID_ATTR_WORKINGSET :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_WORKINGSET ),
                              pOptions->IsSaveWorkingSet())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_SAVEDOCWINS :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_SAVEDOCWINS ),
                              pOptions->IsSaveDocWins())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_SAVEDOCVIEW :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_SAVEDOCVIEW ),
                              pOptions->IsSaveDocView())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_METRIC :
                    if(rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_ATTR_METRIC ),
                                pOptions->GetMetric() ) ) )
                        bRet = TRUE;
                    break;
                case SID_DOCMANAGER :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_DOCMANAGER ),
                                 pOptions->GetDocumentManagerConfig() ) ) )
                        bRet = TRUE;
                    break;
                case SID_HELPBALLOONS :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_HELPBALLOONS ),
                               pOptions->IsHelpBalloons() ) ) )
                        bRet = TRUE;
                    break;
                case SID_HELPTIPS :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_HELPTIPS ),
                               pOptions->IsHelpTips() ) ) )
                        bRet = TRUE;
                    break;
                case SID_ATTR_AUTOHELPAGENT :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_ATTR_AUTOHELPAGENT ),
                               pOptions->IsAutoHelpAgent() ) ) )
                        bRet = TRUE;
                    break;
                case SID_ATTR_WELCOMESCREEN :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_ATTR_WELCOMESCREEN ),
                               pOptions->IsWelcomeScreen() ) ) )
                        bRet = TRUE;
                    break;
                case SID_ATTR_UNDO_COUNT :
                    if(rSet.Put( SfxUInt16Item ( rPool.GetWhich( SID_ATTR_UNDO_COUNT ),
                                 pOptions->GetUndoCount() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_HOMEPAGE   :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_INET_HOMEPAGE   ),
                                 pIni->Get(SFX_KEY_INET_HOME) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_MEMCACHE :
                    if(rSet.Put( SfxUInt16Item ( rPool.GetWhich( SID_INET_MEMCACHE ),
                                 pIni->Get(SFX_KEY_INET_MEMCACHE).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_DISKCACHE :
                    if(rSet.Put( SfxUInt32Item ( rPool.GetWhich( SID_INET_DISKCACHE ),
                                 pIni->Get(SFX_KEY_INET_DISKCACHE).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_EXPIRATION :
                    if(rSet.Put( SfxUInt16Item ( rPool.GetWhich( SID_INET_EXPIRATION ),
                                 pIni->Get(SFX_KEY_INET_CACHEEXPIRATION).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_CACHEJS :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_INET_CACHEJS),
                              pIni->Get(SFX_KEY_INET_CACHEJS).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_CACHEEXPIRED :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_INET_CACHEEXPIRED),
                              pIni->Get(SFX_KEY_INET_CACHEEXPIRED).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_CACHEABORTED :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_INET_CACHEABORTED),
                              pIni->Get(SFX_KEY_INET_CACHEABORTED).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_REVEAL_MAILADDR :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_INET_REVEAL_MAILADDR),
                               (BOOL)(USHORT)pIni->Get(SFX_KEY_INET_REVEAL_MAILADDR).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_SAVEREL_INET :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_SAVEREL_INET ),
                               pOptions->IsSaveRelINet() ) ) )
                        bRet = TRUE;
                    break;
                case SID_SAVEREL_FSYS :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_SAVEREL_FSYS ),
                               pOptions->IsSaveRelFSys() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_SMTPSERVER :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_INET_SMTPSERVER),
                                 pIni->Get(SFX_KEY_INET_SMTPSERVER) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_POPSERVER :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_INET_POPSERVER),
                                 pIni->Get(SFX_KEY_INET_POPSERVER) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_NNTPSERVER :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_INET_NNTPSERVER),
                                 pIni->Get(SFX_KEY_INET_NNTPSERVER) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_MAXNEWS :
                    if(rSet.Put( SfxUInt16Item ( rPool.GetWhich( SID_INET_MAXNEWS),
                                 pIni->Get(SFX_KEY_INET_MAXNEWS).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_MAXHTTPCONS :
                    if(rSet.Put( SfxUInt16Item ( rPool.GetWhich( SID_INET_MAXHTTPCONS),
                                 pIni->Get(SFX_KEY_INET_MAXHTTPCONS).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_MAXFTPCONS :
                    if(rSet.Put( SfxUInt16Item ( rPool.GetWhich( SID_INET_MAXFTPCONS),
                                 pIni->Get(SFX_KEY_INET_MAXFTPCONS).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_SMTPGATEWAY :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_INET_SMTPGATEWAY),
                                 pIni->Get(SFX_KEY_INET_MAILGATEWAY) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_MAILUSERNAME :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_INET_MAILUSERNAME),
                                 pIni->Get(SFX_KEY_INET_MAILUSERNAME) ) ) )
                        bRet = TRUE;
                    break;
                case SID_ATTR_ALLOWFOLDERWEBVIEW :
                    if(rSet.Put( SfxStringItem( rPool.GetWhich(SID_ATTR_ALLOWFOLDERWEBVIEW),
                                 pIni->Get(SFX_KEY_ALLOWFOLDERWEBVIEW) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_MAILPASSWORD :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_INET_MAILPASSWORD),
                                 SfxStringDecode( pIni->Get(SFX_KEY_INET_MAILPASSWORD) ) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_MAILTEXTFORMAT :
                {
                    String aTextFormat = pIni->Get( SFX_KEY_INET_MAILTEXTFORMAT );
                    BYTE nTextFormat = (BYTE)(USHORT)aTextFormat.ToInt32();
                    if(rSet.Put( SfxByteItem( rPool.GetWhich( SID_INET_MAILTEXTFORMAT ),
                               nTextFormat ) ) )
                        bRet = TRUE;
                    break;
                }
                case SID_BASIC_ENABLED :
                    if ( rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_BASIC_ENABLED ),
                                pIni->Get(SFX_KEY_BASIC_ENABLE).ToInt32()) ) )
                        bRet = TRUE;
                    break;

                case SID_INET_JAVA_ENABLE:
                {
                    String aIniEntry = GetJavaIniEntry_Impl( SFX_KEY_JAVA_ENABLE, pIni );
                    if ( rSet.Put( SfxBoolItem(
                            rPool.GetWhich( SID_INET_JAVA_ENABLE ), ( aIniEntry == aTRUEStr ) ) ) )
                        bRet = TRUE;
                    break;
                }

                case SID_INET_EXE_APPLETS :
                {
                    String aIniEntry = GetJavaIniEntry_Impl( SFX_KEY_INET_EXE_APPLETS, pIni );
                    if ( rSet.Put( SfxBoolItem(
                            rPool.GetWhich( SID_INET_EXE_APPLETS ), ( aIniEntry == aTRUEStr ) ) ) )
                        bRet = TRUE;
                    break;
                }
                case SID_INET_JAVA_ACCESSTYPE:
                {
                    SjNetAccess eAccess = NET_HOST;
                    String aNetAccess = GetJavaIniEntry_Impl( SFX_KEY_JAVA_NETACCESS, pIni );
                    if ( aNetAccess.Len() )
                    {
                        if ( aNetAccess.CompareIgnoreCaseToAscii( "UNRESTRICTED" ) == COMPARE_EQUAL )
                            eAccess = NET_UNRESTRICTED;
                        else if ( aNetAccess.CompareIgnoreCaseToAscii( "NONE" ) == COMPARE_EQUAL )
                            eAccess = NET_NONE;
                    }

                    if ( rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_INET_JAVA_ACCESSTYPE ), (USHORT)eAccess ) ) )
                        bRet = TRUE;
                    break;
                }
                case SID_INET_JAVA_SECURITY:
                {
                    String aIniEntry = GetJavaIniEntry_Impl( SFX_KEY_JAVA_SECURITY, pIni );
                    if ( rSet.Put( SfxBoolItem(
                            rPool.GetWhich( SID_INET_JAVA_SECURITY ), ( aIniEntry.CompareToAscii("0") != COMPARE_EQUAL ) ) ) )
                        bRet = TRUE;
                    break;
                }
                case SID_INET_JAVA_CLASSPATH :
                {
                    String aIniEntry = GetJavaIniEntry_Impl( SFX_KEY_JAVA_USERCLASSPATH, pIni );
                    if ( rSet.Put( SfxStringItem( rPool.GetWhich( SID_INET_JAVA_CLASSPATH ), aIniEntry ) ) )
                        bRet = TRUE;
                    break;
                }

                case SID_INET_EXE_PLUGIN  :
                    if ( rSet.Put( SfxBoolItem( rPool.GetWhich( SID_INET_EXE_PLUGIN ),
                        ( pIni->Get( SFX_KEY_INET_EXE_PLUGIN ) == aTRUEStr ) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_USERAGENT :
                    if ( rSet.Put( SfxStringItem( rPool.GetWhich(SID_INET_USERAGENT ),
                                  pIni->Get( SFX_KEY_INET_USERAGENT ) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_NONCACHED_SERVER :
                    if ( rSet.Put( SfxStringItem( rPool.GetWhich( SID_INET_NONCACHED_SERVER ),
                                  pIni->Get( SFX_KEY_INET_NONCACHED_SERVER ) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_DNS_SERVER :
                    if ( rSet.Put( SfxStringItem( rPool.GetWhich(SID_INET_DNS_SERVER),
                                     pIni->Get(SFX_KEY_INET_DNS ) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_DNS_AUTO  :
                    if ( rSet.Put( SfxBoolItem( rPool.GetWhich( SID_INET_DNS_AUTO ),
                            !pIni->Get(SFX_KEY_INET_DNS).Len() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_CHANNELS_ONOFF  :
                    if ( rSet.Put( SfxBoolItem( rPool.GetWhich( SID_INET_CHANNELS_ONOFF ),
                                ( pIni->Get( SFX_KEY_INET_CHANNELS ) == aTRUEStr ) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_COOKIESHANDLE :
                    if ( rSet.Put( SfxUInt16Item ( rPool.GetWhich(SID_INET_COOKIESHANDLE),
                                   pIni->Get(SFX_KEY_INET_COOKIES).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_ATTR_SMARTBEAMER :
                    if ( rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_SMARTBEAMER ),
                                pAppData_Impl->bSmartBeamer ) ) )
                        bRet = TRUE;
                    break;
                case SID_OPT_EXTBRW_ON  :
                    if ( rSet.Put( SfxBoolItem( rPool.GetWhich( SID_OPT_EXTBRW_ON ),
                                ( pIni->Get( SFX_KEY_EXTBRW_ON ) == aTRUEStr ) ) ) )
                        bRet = TRUE;
                    break;
                case SID_OPT_EXTBRW_ON_EXCEPTION :
                {
                    List aList;
                    USHORT nCount = pIni->Get(SFX_KEY_EXTBRW_ON_EXCEPTION).ToInt32();
                    USHORT n;
                    for ( n = 0; n < nCount; ++n )
                        aList.Insert( new String( pIni->Get(SFX_KEY_EXTBRW_ON_EXCEPTION, n) ), LIST_APPEND );
                    if ( rSet.Put( SfxStringListItem( rPool.GetWhich(SID_OPT_EXTBRW_ON_EXCEPTION),
                            &aList ) ) )
                        bRet = TRUE;
                    for ( n = 0; n < nCount; ++n )
                        delete (String*)aList.GetObject(n);
                    aList.Clear();
                    break;
                }
                case SID_OPT_EXTBRW_OFF_EXCEPTION :
                {
                    List aList;
                    USHORT nCount = pIni->Get(SFX_KEY_EXTBRW_OFF_EXCEPTION).ToInt32();
                    USHORT n;
                    for ( n = 0; n < nCount; ++n )
                        aList.Insert( new String( pIni->Get(SFX_KEY_EXTBRW_OFF_EXCEPTION, n) ), LIST_APPEND );
                    if ( rSet.Put( SfxStringListItem( rPool.GetWhich(SID_OPT_EXTBRW_OFF_EXCEPTION),
                            &aList ) ) )
                        bRet = TRUE;
                    for ( n = 0; n < nCount; ++n )
                        delete (String*)aList.GetObject(n);
                    aList.Clear();
                    break;
                }
                case SID_SECURE_URL :
                {
                    List aList;
                    USHORT nCount = pIni->Get( SFX_KEY_SECURE_URL ).ToInt32();
                    USHORT n;
                    for ( n = 0; n < nCount; ++n )
                        aList.Insert( new String( pIni->Get(SFX_KEY_SECURE_URL, n) ), LIST_APPEND );
                    if ( rSet.Put( SfxStringListItem( rPool.GetWhich(SID_SECURE_URL),
                            &aList ) ) )
                        bRet = TRUE;
                    for ( n = 0; n < nCount; ++n )
                        delete (String*)aList.GetObject(n);
                    aList.Clear();
                    break;
                }
                case SID_ICONGRID :
                {
                    String aIconGrid( pIni->Get(SFX_KEY_ICONGRID) );
                    if ( rSet.Put( SfxSizeItem( rPool.GetWhich( SID_ICONGRID ),
                            Size( aIconGrid.GetToken(0).ToInt32(), aIconGrid.GetToken(1).ToInt32() ) ) ) )
                        bRet = TRUE;
                    break;
                }

                case SID_AUTO_ADJUSTICONS :
                {
                    String aIconGrid( pIni->Get(SFX_KEY_ICONGRID) );
                    if ( rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_AUTO_ADJUSTICONS ),
                                (UINT16)aIconGrid.GetToken(2).ToInt32() )) )
                        bRet = TRUE;
                    break;
                }
                case SID_RESTORE_EXPAND_STATE :
                {
                    String aRestoreExpand( pIni->Get( SFX_KEY_AUTOOPEN ));
                    BOOL bRestoreExpand = TRUE;
                    if( aRestoreExpand.Len() )
                        bRestoreExpand = ((USHORT)aRestoreExpand.ToInt32()) > 0 ? TRUE : FALSE;
                    if( rSet.Put( SfxBoolItem( rPool.GetWhich( SID_RESTORE_EXPAND_STATE ),
                            bRestoreExpand )))
                        bRet = TRUE;
                    break;
                }
                case SID_ENABLE_METAFILEPRINT :
                    if( rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ENABLE_METAFILEPRINT ),
                               (BOOL)(USHORT)pIni->Get(SFX_KEY_METAFILEPRINT ).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_PROXY_TYPE :
                    if(rSet.Put( SfxUInt16Item ( rPool.GetWhich( SID_INET_PROXY_TYPE ),
                                 pIni->Get(SFX_KEY_INET_PROXYTYPE).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_FTP_PROXY_NAME :
                    if ( rSet.Put( SfxStringItem ( rPool.GetWhich(SID_INET_FTP_PROXY_NAME ),
                            pIni->Get(SFX_KEY_INET_FTPPROXYNAME) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_FTP_PROXY_PORT :
                    if ( rSet.Put( SfxInt32Item ( rPool.GetWhich(SID_INET_FTP_PROXY_PORT ),
                            pIni->Get(SFX_KEY_INET_FTPPROXYPORT).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_HTTP_PROXY_NAME :
                    if ( rSet.Put( SfxStringItem ( rPool.GetWhich(SID_INET_HTTP_PROXY_NAME ),
                            pIni->Get(SFX_KEY_INET_HTTPPROXYNAME) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_HTTP_PROXY_PORT :
                    if ( rSet.Put( SfxInt32Item( rPool.GetWhich(SID_INET_HTTP_PROXY_PORT ),
                            pIni->Get(SFX_KEY_INET_HTTPPROXYPORT).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_SOCKS_PROXY_NAME :
                    if ( rSet.Put( SfxStringItem ( rPool.GetWhich(SID_INET_SOCKS_PROXY_NAME ),
                            pIni->Get(SFX_KEY_INET_SOCKSPROXYNAME) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_SOCKS_PROXY_PORT :
                    if ( rSet.Put( SfxInt32Item( rPool.GetWhich(SID_INET_SOCKS_PROXY_PORT ),
                            pIni->Get(SFX_KEY_INET_SOCKSPROXYPORT).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_SECURITY_PROXY_NAME :
                    if ( rSet.Put( SfxStringItem ( rPool.GetWhich(SID_INET_SECURITY_PROXY_NAME ),
                            pIni->Get(SFX_KEY_INET_SECURITYPROXYNAME) ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_SECURITY_PROXY_PORT :
                    if ( rSet.Put( SfxInt32Item( rPool.GetWhich(SID_INET_SECURITY_PROXY_PORT ),
                            pIni->Get(SFX_KEY_INET_SECURITYPROXYPORT).ToInt32() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_NOPROXY :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_INET_NOPROXY),
                                 pIni->Get(SFX_KEY_INET_NOPROXY) ) ) )
                        bRet = TRUE;
                    break;
                case SID_ATTR_PATHNAME :
                case SID_ATTR_PATHGROUP :
                {
                    SfxAllEnumItem aNames(rPool.GetWhich(SID_ATTR_PATHGROUP));
                    SfxAllEnumItem aValues(rPool.GetWhich(SID_ATTR_PATHNAME));
                    pIni->Fill(SFX_GROUP_DIR, aNames, aValues);
                    if ( rSet.Put(aNames) || rSet.Put(aValues) )
                        bRet = TRUE;
                    break;
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
    }

    return bRet;
}

//--------------------------------------------------------------------

BOOL SfxApplication::IsSecureURL( const INetURLObject& rURL, const String* pReferer ) const
{
    // unkritisches Protokoll?
    INetProtocol eProt = rURL.GetProtocol();
    if ( INET_PROT_MACRO != eProt && INET_PROT_SLOT != eProt )
        return TRUE;
    if ( rURL.GetMainURL().CompareIgnoreCaseToAscii( "macro://#", 9 ) == COMPARE_EQUAL )
        return TRUE;

    if ( rURL.GetMainURL().CompareToAscii("slot:5500") == COMPARE_EQUAL )
        return TRUE;

    // StarBasic-Modus abholen
    SfxIniManager *pIni = GetIniManager();
       USHORT nStarBasicMode = pIni->Get(SFX_KEY_BASIC_ENABLE).ToInt32();

    // StarBasic gar nicht erlaubt?
    if ( 0 == nStarBasicMode )
        return FALSE;

    // StarBasic immer erlaubt?
    if ( 2 == nStarBasicMode )
        return TRUE;

    // trusted referer?
    BOOL bTrusted = FALSE;
    if ( pReferer && pReferer->Len() )
    {
        // secure-URLs noch nicht geladen?
        if ( !pAppData_Impl->pSecureURLs )
        {
            pAppData_Impl->pSecureURLs = new SvStrings;
            USHORT nCount = pIni->Get(SFX_KEY_SECURE_URL).ToInt32();
            USHORT nPos = 0;
            for ( USHORT n = 0; n < nCount; ++n )
            {
                String aURL = pIni->Get(SFX_KEY_SECURE_URL, n);
                if ( aURL.Len() )
                    pAppData_Impl->pSecureURLs->Insert( new String( aURL ), nPos++ );
            }
        }

        // suchen
        for ( USHORT n = 0; !bTrusted && n < pAppData_Impl->pSecureURLs->Count(); ++n )
        {
            const String* pSecureURL = pAppData_Impl->pSecureURLs->GetObject(n);
            String aSecureURL( *pSecureURL );
            aSecureURL += DEFINE_CONST_UNICODE('*');
            bTrusted = WildCard( aSecureURL ).Matches( *pReferer );
        }
    }

    // gemaess Liste
    return bTrusted;
}

//--------------------------------------------------------------------

BOOL SfxApplication::UseExternBrowser() const
{
    if ( pAppData_Impl->bUseExternBrowser == 2 )
    {
        // Noch nicht initialisiert
        SfxIniManager* pIni = GetIniManager();
        pAppData_Impl->bUseExternBrowser = ((BOOL) (USHORT) pIni->Get(SFX_KEY_EXTBRW_ON).ToInt32() ) &&
                ( pIni->IsInternetExplorerAvailable() || pIni->Get( SFX_KEY_EXTBRW_FILE ).Len() );
    }

    return pAppData_Impl->bUseExternBrowser;
}

//--------------------------------------------------------------------

BOOL SfxApplication::ShouldUseExternalBrowser( const INetURLObject& rURL ) const
{
    SfxIniManager* pIni = GetIniManager();
    if( !pIni->IsInternetExplorerAvailable() && !pIni->Get( SFX_KEY_EXTBRW_FILE ).Len() )
        // Wenn es keinen externen Browser gibt
        return FALSE;

    // Welche Liste ?
    SvStrings*& rpList = UseExternBrowser() ? pAppData_Impl->pExtBrwOnExceptionList
                                      : pAppData_Impl->pExtBrwOffExceptionList;

    // ExceptionList noch nicht geladen?
    if ( !rpList )
    {
        rpList = new SvStrings;
        USHORT nKey = pAppData_Impl->bUseExternBrowser ? SFX_KEY_EXTBRW_ON_EXCEPTION : SFX_KEY_EXTBRW_OFF_EXCEPTION;
        USHORT nCount = pIni->Get( nKey ).ToInt32();
        for ( USHORT i = 0; i < nCount; ++i )
            rpList->Insert( new String( pIni->Get( nKey, i ) ), i );
    }

    BOOL bIn = FALSE;
    String aURL( rURL.GetMainURL().ToLowerAscii() );
    USHORT nCount = rpList->Count();
    String aName = rURL.GetMainURL();

    for ( USHORT n = 0; !bIn && n<nCount; ++n )
    {
        String aURL = rpList->GetObject(n)->ToLowerAscii();
        aURL += DEFINE_CONST_UNICODE('*');
        bIn = WildCard( aURL ).Matches( aName );
    }

    return pAppData_Impl->bUseExternBrowser ? !bIn : bIn;
}

//--------------------------------------------------------------------

void SfxApplication::SetOptions_Impl( const SfxItemSet& rSet )
{
    SfxIniManager *pIni = GetIniManager();
    pIni->EnterLock();
    const SfxPoolItem *pItem = 0;
    SfxItemPool &rPool = GetPool();
    BOOL bResetSession = FALSE;
    BOOL bProxiesModified = FALSE;

    SfxToolBoxConfig *pTbxCfg = SfxToolBoxConfig::GetOrCreate();
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_BUTTON_OUTSTYLE3D), TRUE, &pItem) )
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        USHORT nOutStyle =
            ( (const SfxBoolItem *)pItem)->GetValue() ? 0 : TOOLBOX_STYLE_FLAT;
        pTbxCfg->SetOutStyle( nOutStyle );
    }

    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_BUTTON_BIGSIZE), TRUE, &pItem) )
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL bBigSize = ( (const SfxBoolItem*)pItem )->GetValue();
        pTbxCfg->SetSymbolSet( bBigSize ? SFX_SYMBOLS_LARGE_COLOR : SFX_SYMBOLS_SMALL_COLOR );
        GetWorkWindow_Impl( SfxViewFrame::Current() )->UpdateObjectBars_Impl();
    }

    // Backup
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_BACKUP), TRUE, &pItem) )
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        pOptions->SetBackup( ( (const SfxBoolItem*)pItem )->GetValue() );
    }

    // AutoSave
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOSAVE), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        pOptions->SetAutoSave( ( (const SfxBoolItem*)pItem )->GetValue() );
    }

    // AutoSave-Propt
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOSAVEPROMPT), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        pOptions->SetAutoSavePrompt(((const SfxBoolItem *)pItem)->GetValue());
    }

    // AutoSave-Time
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOSAVEMINUTE), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "UInt16Item expected");
        pOptions->SetAutoSaveTime(((const SfxUInt16Item *)pItem)->GetValue());
    }

    // DocInfo
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_DOCINFO), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        pOptions->SetDocInfoSave(((const SfxBoolItem *)pItem)->GetValue());
    }

    // portable Grafiken
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_INDEP_METAFILE), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL b = ((const SfxBoolItem *)pItem)->GetValue();
        pOptions->SetIndepGrfFmt(b);
    }

    // Grafiken komprimiert speichern
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_OPT_SAVEGRAPHICSCOMPRESSED), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL b = ((const SfxBoolItem *)pItem)->GetValue();
        pOptions->SetSaveGraphicsCompressed(b);
    }

    // Grafiken im Original speichern
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_OPT_SAVEORIGINALGRAPHICS), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL b = ((const SfxBoolItem *)pItem)->GetValue();
        pOptions->SetSaveOriginalGraphics(b);
    }

    // offende Dokumente merken
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_WORKINGSET), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        pOptions->SetSaveWorkingSet(((const SfxBoolItem *)pItem)->GetValue());
    }

    // offene Fenster speichern
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_SAVEDOCWINS), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        pOptions->SetSaveDocWins(((const SfxBoolItem *)pItem)->GetValue());
    }

    // Fenster-Einstellung speichern
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_SAVEDOCVIEW), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        pOptions->SetSaveDocView(((const SfxBoolItem *)pItem)->GetValue());
    }

    // Metric
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_METRIC), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "UInt16Item expected");
        pOptions->SetMetric((FieldUnit)((const SfxUInt16Item*)pItem)->GetValue());
    }

    // Docmanager
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_DOCMANAGER), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        pOptions->SetDocumentManagerConfig(((const SfxStringItem *)pItem)->GetValue());
    }

    // HelpBalloons
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_HELPBALLOONS), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        pOptions->SetHelpBalloons(((const SfxBoolItem *)pItem)->GetValue());
    }

    // HelpTips
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_HELPTIPS), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        pOptions->SetHelpTips(((const SfxBoolItem *)pItem)->GetValue());
    }

    // AutoHelpAgent
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOHELPAGENT ), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        pOptions->SetAutoHelpAgent( ((const SfxBoolItem *)pItem)->GetValue() );
    }

    // AutoHelpAgent-Reset
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_AUTOHELPAGENT_RESET ), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL bReset = ((const SfxBoolItem *)pItem)->GetValue();
        Help* pHelp = Application::GetHelp();
        if ( bReset && pHelp )
            ((SfxHelp_Impl*)pHelp)->ResetPIStarterList();
    }

    // WelcomeScreen
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_WELCOMESCREEN ), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        pOptions->SetWelcomeScreen( ((const SfxBoolItem *)pItem)->GetValue() );
    }

    // WelcomeScreen
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_WELCOMESCREEN_RESET ), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL bReset = ((const SfxBoolItem *)pItem)->GetValue();
        if ( bReset )
        {
            Config aConfig( SfxHelp_Impl::GetHelpAgentConfig() );
            ImplSetLanguageGroup( aConfig, DEFINE_CONST_UNICODE( "WelcomeScreen" ), TRUE );
            USHORT nTips = aConfig.GetKeyCount();
            ByteString aOn = ByteString::CreateFromInt32( 1 );
            for ( USHORT nTip = 0; nTip < nTips; nTip++ )
                aConfig.WriteKey( aConfig.GetKeyName( nTip ), aOn );
        }
    }

    // SaveRelINet
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_SAVEREL_INET), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        pOptions->SetSaveRelINet(((const SfxBoolItem *)pItem)->GetValue());
    }

    // SaveRelFSys
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_SAVEREL_FSYS), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        pOptions->SetSaveRelFSys(((const SfxBoolItem *)pItem)->GetValue());
    }

    // Undo-Count
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_UNDO_COUNT), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "UInt16Item expected");
        USHORT nUndoCount = ((const SfxUInt16Item*)pItem)->GetValue();
        pOptions->SetUndoCount( nUndoCount );

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
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_HOMEPAGE), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        pIni->Set(((const SfxStringItem *)pItem)->GetValue(), SFX_KEY_INET_HOME);
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_MEMCACHE), TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxUInt16Item), "UInt16Item expected" );
        USHORT nCount = ( (const SfxUInt16Item *)pItem )->GetValue();
        pIni->Set( String::CreateFromInt32( nCount ), SFX_KEY_INET_MEMCACHE );
        SfxPickList_Impl::Get()->GetMemCache().ClearToLimit( nCount );
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_DISKCACHE), TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxUInt32Item), "UInt32Item expected" );
        String aValue = String::CreateFromInt32( ( (const SfxUInt32Item*)pItem )->GetValue() );
        pIni->Set( aValue, SFX_KEY_INET_DISKCACHE );
        bResetSession = TRUE;
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_EXPIRATION), TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxUInt16Item), "UInt16Item expected" );
        String aValue = String::CreateFromInt32( ( (const SfxUInt16Item*)pItem )->GetValue() );
        pIni->Set( aValue, SFX_KEY_INET_CACHEEXPIRATION );
        bResetSession = TRUE;
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_CACHEJS), TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxBoolItem), "BoolItem expected" );
        INT32 nFlag = (USHORT)( (const SfxBoolItem*)pItem )->GetValue();
        pIni->Set( String::CreateFromInt32( nFlag ), SFX_KEY_INET_CACHEJS );
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_CACHEEXPIRED), TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxBoolItem), "BoolItem expected" );
        INT32 nFlag = (USHORT)( (const SfxBoolItem*)pItem )->GetValue();
        pIni->Set( String::CreateFromInt32( nFlag ), SFX_KEY_INET_CACHEEXPIRED );
        // Ist auch eine Einstellung fuer die Session
        bResetSession = TRUE;
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_CACHEABORTED), TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxBoolItem), "BoolItem expected" );
        INT32 nFlag = (USHORT)( (const SfxBoolItem*)pItem )->GetValue();
        pIni->Set( String::CreateFromInt32( nFlag ), SFX_KEY_INET_CACHEABORTED );
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_REVEAL_MAILADDR), TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxBoolItem), "BoolItem expected" );
        INT32 nFlag = (USHORT)( (const SfxBoolItem*)pItem )->GetValue();
        pIni->Set( String::CreateFromInt32( nFlag ), SFX_KEY_INET_REVEAL_MAILADDR );
        bResetSession = TRUE;
    }

    // SMTP-Server
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_SMTPSERVER), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        pIni->Set(((const SfxStringItem *)pItem)->GetValue(), SFX_KEY_INET_SMTPSERVER);
        bResetSession = TRUE;
    }

    // POP-Server
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_POPSERVER), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        pIni->Set(((const SfxStringItem *)pItem)->GetValue(), SFX_KEY_INET_POPSERVER);
        bResetSession = TRUE;
    }

    // NNTP-Server
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_NNTPSERVER), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        pIni->Set(((const SfxStringItem *)pItem)->GetValue(), SFX_KEY_INET_NNTPSERVER);
        bResetSession = TRUE;
    }

    // MaxNewsMessages
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_MAXNEWS), TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxUInt16Item), "UInt16Item expected" );
        String aValue = String::CreateFromInt32( ( (const SfxUInt16Item*)pItem )->GetValue() );
        pIni->Set( aValue, SFX_KEY_INET_MAXNEWS );
        bResetSession = TRUE;
    }

    // MaxHttpCons
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_MAXHTTPCONS), TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxUInt16Item), "UInt16Item expected" );
        String aValue = String::CreateFromInt32( ( (const SfxUInt16Item*)pItem )->GetValue() );
        pIni->Set( aValue, SFX_KEY_INET_MAXHTTPCONS );
        bResetSession = TRUE;
    }

    // MaxFtpCons
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_MAXFTPCONS), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "UInt16Item expected");
        String aValue = String::CreateFromInt32( ( (const SfxUInt16Item*)pItem )->GetValue() );
        pIni->Set( aValue, SFX_KEY_INET_MAXFTPCONS);
        bResetSession = TRUE;
    }

    // Mail-Gateway zum SMTP
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_SMTPGATEWAY), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        pIni->Set(((const SfxStringItem *)pItem)->GetValue(), SFX_KEY_INET_MAILGATEWAY);
    }

    // Mail-Username
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_MAILUSERNAME), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        pIni->Set(((const SfxStringItem *)pItem)->GetValue(), SFX_KEY_INET_MAILUSERNAME);
    }

    // Mail-Password
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_MAILPASSWORD), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        String aPassword( ((const SfxStringItem *)pItem)->GetValue() );
        pIni->Set( SfxStringEncode( aPassword ), SFX_KEY_INET_MAILPASSWORD );
    }

    // Mail-Text-Format
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_MAILTEXTFORMAT ), TRUE, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA(SfxByteItem), "ByteItem expected" );
        INT32 nFlag = (USHORT)( (const SfxByteItem*)pItem )->GetValue();
        pIni->Set( String::CreateFromInt32( nFlag ), SFX_KEY_INET_MAILTEXTFORMAT );
    }

    // Proxy-Type
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_PROXY_TYPE), TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxUInt16Item), "UInt16Item expected" );
        String aValue = String::CreateFromInt32( ( (const SfxUInt16Item*)pItem )->GetValue() );
        pIni->Set( aValue, SFX_KEY_INET_PROXYTYPE );
        bResetSession = TRUE;
        bProxiesModified = TRUE;
    }

    // NoProxy
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_NOPROXY, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        pIni->Set(((const SfxStringItem *)pItem)->GetValue(), SFX_KEY_INET_NOPROXY);
        bResetSession = TRUE;
        bProxiesModified = TRUE;
    }

    // StarBasic Enable
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_BASIC_ENABLED, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "SfxInt16Item expected");
        String aValue = String::CreateFromInt32( ( (const SfxUInt16Item*)pItem )->GetValue() );
        pIni->Set( aValue, SFX_KEY_BASIC_ENABLE );
    }

    // Java Enable
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_INET_JAVA_ENABLE, TRUE, &pItem ) )
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        BOOL bJava = ( (const SfxBoolItem*)pItem )->GetValue();
        String aVal = bJava ? 0x0031 : 0x0030; // ^= '1' or '0'
        BOOL bSet = SetJavaIniEntry_Impl( SFX_KEY_JAVA_ENABLE, aVal, pIni );
        bResetSession = TRUE;
    }
    // Applets Enable
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_INET_EXE_APPLETS, TRUE, &pItem ) )
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        BOOL bApplets = ( (const SfxBoolItem*)pItem )->GetValue();
        String aVal = bApplets ? 0x0031 : 0x0030; // ^= '1' or '0'
        BOOL bSet = SetJavaIniEntry_Impl( SFX_KEY_INET_EXE_APPLETS, aVal, pIni );
        bResetSession = TRUE;
    }

    // Java AccessType
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_JAVA_ACCESSTYPE), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "UInt16Item expected");
        SjNetAccess eAccess = (SjNetAccess) ((const SfxUInt16Item *)pItem)->GetValue();
        String aVal;
        if ( eAccess == NET_HOST )
            aVal = DEFINE_CONST_UNICODE("HOST");
        else if ( eAccess == NET_UNRESTRICTED )
            aVal = DEFINE_CONST_UNICODE("UNRESTRICTED");
        else
            aVal = DEFINE_CONST_UNICODE("NONE");
        BOOL bSet = SetJavaIniEntry_Impl( SFX_KEY_JAVA_NETACCESS, aVal, pIni );
        bResetSession = TRUE;
    }

    // Java Security
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_JAVA_SECURITY, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        BOOL bSecurity = ((const SfxBoolItem *)pItem)->GetValue();
        String aVal = bSecurity ? 0x0031 : 0x0030; // ^= '1' or '0'
        BOOL bSet = SetJavaIniEntry_Impl( SFX_KEY_JAVA_SECURITY, aVal, pIni );
        bResetSession = TRUE;
    }

    // Java ClassPath
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_JAVA_CLASSPATH, TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxStringItem), "SfxStringItem expected" );
        String aVal = ( (const SfxStringItem *)pItem )->GetValue();
        BOOL bSet = SetJavaIniEntry_Impl( SFX_KEY_JAVA_USERCLASSPATH, aVal, pIni );
        bResetSession = TRUE;
    }

    // Execute PlugIns
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_EXE_PLUGIN, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        BOOL bExe = ( (const SfxBoolItem *)pItem )->GetValue();
        pIni->Set( bExe ? 0x0031 : 0x0030 /* ^= '1' or '0' */, SFX_KEY_INET_EXE_PLUGIN );
        bResetSession = TRUE;
    }

    // DNS Server
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_DNS_AUTO), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        BOOL bIsAuto = ((const SfxBoolItem *)pItem)->GetValue();
        if( bIsAuto )
            pIni->Set( String(), SFX_KEY_INET_DNS );
        else
        {
            String aDNS;
            if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_DNS_SERVER), TRUE, &pItem) )
            {
                DBG_ASSERT(pItem->ISA(SfxStringItem), "SfxStringItem expected");
                aDNS = ((const SfxStringItem *)pItem)->GetValue();
            }
            pIni->Set( aDNS, SFX_KEY_INET_DNS );
        }
        bResetSession = TRUE;
    }

    // Noncahed Server
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_NONCACHED_SERVER, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        pIni->Set(((const SfxStringItem *)pItem)->GetValue(), SFX_KEY_INET_NONCACHED_SERVER);
        bResetSession = TRUE;
    }

    // UserAgent
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_USERAGENT, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        pIni->Set(((const SfxStringItem *)pItem)->GetValue(), SFX_KEY_INET_USERAGENT);
        bResetSession = TRUE;
    }

    // Proxies
    for ( USHORT nOfs = 0;
          nOfs <= SID_INET_SECURITY_PROXY_PORT - SID_INET_HTTP_PROXY_NAME;
          nOfs += 2 )
    {
        if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_HTTP_PROXY_NAME + nOfs ), TRUE, &pItem ) )
        {
            DBG_ASSERT( pItem->ISA(SfxStringItem), "StringItem expected" );
            String aVal( ((const SfxStringItem *)pItem)->GetValue() );
            pIni->Set( aVal, SFX_KEY_INET_HTTPPROXYNAME+nOfs );
            bResetSession = TRUE;
            bProxiesModified = TRUE;
        }
        if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_HTTP_PROXY_PORT + nOfs ), TRUE, &pItem ) )
        {
            DBG_ASSERT( pItem->ISA(SfxInt32Item), "Int32Item expected" );
            String aValue = String::CreateFromInt32( ( (const SfxInt32Item*)pItem )->GetValue() );
            pIni->Set( aValue, SFX_KEY_INET_HTTPPROXYPORT + nOfs );
            bResetSession = TRUE;
            bProxiesModified = TRUE;
        }
    }

    // Channels
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_CHANNELS_ONOFF, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        BOOL bExe = ((const SfxBoolItem *)pItem)->GetValue();
        pIni->Set( bExe ? 0x0031 : 0x0030 /* ^= '1' or '0' */, SFX_KEY_INET_CHANNELS );
    }

    // Cookies
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_COOKIESHANDLE, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "SfxUInt16Item expected");
        String aValue = String::CreateFromInt32( ( (const SfxUInt16Item*)pItem )->GetValue() );
        pIni->Set( aValue, SFX_KEY_INET_COOKIES );
    }

    // Secure-Referers
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_SECURE_URL, TRUE, &pItem))
    {
        DELETEZ(pAppData_Impl->pSecureURLs);

        DBG_ASSERT(pItem->ISA(SfxStringListItem), "StringListItem expected");
        const List *pList = ((SfxStringListItem*)pItem)->GetList();
        pIni->Set( String::CreateFromInt32( pList->Count() ), SFX_KEY_SECURE_URL );
        for ( USHORT n = 0; n < pList->Count(); ++n )
            pIni->Set( *(const String*)(pList->GetObject(n)), SFX_KEY_SECURE_URL, n );
    }

    // ExternBrowser
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_OPT_EXTBRW_ON, TRUE, &pItem) )
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        BOOL bOn = ((const SfxBoolItem *)pItem)->GetValue();
        pIni->Set( bOn ? 0x0031 : 0x0030 /* ^= '1' or '0' */, SFX_KEY_EXTBRW_ON );
        pAppData_Impl->bUseExternBrowser = bOn &&
                ( pIni->IsInternetExplorerAvailable() || pIni->Get( SFX_KEY_EXTBRW_FILE ).Len() );
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_OPT_EXTBRW_ON_EXCEPTION, TRUE, &pItem))
    {
        DELETEZ(pAppData_Impl->pExtBrwOnExceptionList);
        DBG_ASSERT(pItem->ISA(SfxStringListItem), "StringListItem expected");
        const List* pList = ((SfxStringListItem*)pItem)->GetList();
        ULONG n, nCount = pIni->Get( SFX_KEY_EXTBRW_ON_EXCEPTION ).ToInt32();
        for ( n = 0; n < nCount; ++n )
            pIni->Delete( SFX_KEY_EXTBRW_ON_EXCEPTION, n );
        nCount = pList->Count();
        pIni->Set( String::CreateFromInt32( nCount ), SFX_KEY_EXTBRW_ON_EXCEPTION );
        for ( n = 0; n < nCount; ++n )
            pIni->Set( *(const String*)(pList->GetObject(n)), SFX_KEY_EXTBRW_ON_EXCEPTION, n );
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_OPT_EXTBRW_OFF_EXCEPTION, TRUE, &pItem))
    {
        DELETEZ(pAppData_Impl->pExtBrwOffExceptionList);
        DBG_ASSERT(pItem->ISA(SfxStringListItem), "StringListItem expected");
        const List* pList = ((SfxStringListItem*)pItem)->GetList();
        ULONG n, nCount = pIni->Get( SFX_KEY_EXTBRW_OFF_EXCEPTION ).ToInt32();
        for ( n = 0; n < nCount; ++n )
            pIni->Delete( SFX_KEY_EXTBRW_OFF_EXCEPTION, n );
        nCount = pList->Count();
        pIni->Set( String::CreateFromInt32( nCount ), SFX_KEY_EXTBRW_OFF_EXCEPTION );
        for ( n = 0; n < nCount; ++n )
            pIni->Set( *(const String*)(pList->GetObject(n)), SFX_KEY_EXTBRW_OFF_EXCEPTION, n );
    }

    // EnableMetafilePrint
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_ENABLE_METAFILEPRINT ), TRUE, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA(SfxBoolItem), "BoolItem expected" );
        BOOL bPrint = (( const SfxBoolItem*)pItem )->GetValue();
        pIni->Set( bPrint ? 0x0031 : 0x0030 /* ^= '1' or '0' */, SFX_KEY_METAFILEPRINT );
    }

    // Web-View
    if( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_ALLOWFOLDERWEBVIEW), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "SfxStringItem expected");
        pIni->Set(((const SfxStringItem*)pItem)->GetValue(), SFX_KEY_ALLOWFOLDERWEBVIEW);
    }

    // Explorer
    String aIconGrid( pIni->Get(SFX_KEY_ICONGRID) );
    Size aGrid( aIconGrid.GetToken(0).ToInt32(), aIconGrid.GetToken(1).ToInt32() );
    UINT16 nMode = (UINT16)aIconGrid.GetToken(2).ToInt32();
    BOOL bSet = FALSE;
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_AUTO_ADJUSTICONS, TRUE, &pItem) )
    {
        const SfxUInt16Item *pUInt16Item = (const SfxUInt16Item*) pItem;
        nMode = pUInt16Item->GetValue();
        bSet = TRUE;
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_ICONGRID, TRUE, &pItem) )
    {
        const SfxSizeItem *pSizeItem = (const SfxSizeItem*) pItem;
        aGrid = pSizeItem->GetValue();
        bSet = TRUE;
    }
    if ( bSet )
    {
        aIconGrid = String::CreateFromInt32( aGrid.Width() );
        aIconGrid += DEFINE_CONST_UNICODE( ";" );
        aIconGrid += String::CreateFromInt32( aGrid.Height() );
        aIconGrid += DEFINE_CONST_UNICODE( ";" );
        aIconGrid += String::CreateFromInt32( nMode );
        pIni->Set( aIconGrid, SFX_KEY_ICONGRID );
    }

    if ( SFX_ITEM_SET == rSet.GetItemState( SID_RESTORE_EXPAND_STATE, TRUE, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA(SfxBoolItem), "BoolItem expected" );
        BOOL bRestoreExpand = ( (const SfxBoolItem*)pItem )->GetValue();
        pIni->Set( bRestoreExpand ? 0x0031 : 0x0030 /* ^= '1' or '0' */, SFX_KEY_AUTOOPEN );
    }

    if ( bResetSession )
    {
        NAMESPACE_INET(INetWrapper) aWrapper;
        ::vos::ORef< ::inet::INetConfig > xINetConfig;
        if ( aWrapper.getINetConfig( xINetConfig ) )
        {
            xINetConfig->setDomainNameServer( pIni->Get( SFX_KEY_INET_DNS ) );
            xINetConfig->setUserAgent( pIni->Get( SFX_KEY_INET_USERAGENT ) );

            if ( bProxiesModified )
            {
                ::inet::INetProxyConfig aProxyCfg (xINetConfig->getProxyConfig());
                USHORT bProxy = (USHORT)(pIni->Get (SFX_KEY_INET_PROXYTYPE).ToInt32());
                if (bProxy)
                    aProxyCfg.setNoProxyList(pIni->Get (SFX_KEY_INET_NOPROXY));
                else
                    aProxyCfg.setNoProxyList(DEFINE_CONST_UNICODE("*:*"));

                aProxyCfg.setHttpProxy (
                    pIni->Get (SFX_KEY_INET_HTTPPROXYNAME),
                    pIni->Get (SFX_KEY_INET_HTTPPROXYPORT).ToInt32());

                aProxyCfg.setFtpProxy (
                    pIni->Get (SFX_KEY_INET_FTPPROXYNAME),
                    pIni->Get (SFX_KEY_INET_FTPPROXYPORT).ToInt32());

                aProxyCfg.setSecureSocketProxy (
                    pIni->Get (SFX_KEY_INET_SECURITYPROXYNAME),
                    pIni->Get (SFX_KEY_INET_SECURITYPROXYPORT).ToInt32());

                aProxyCfg.setSocksProxy (
                    pIni->Get (SFX_KEY_INET_SOCKSPROXYNAME),
                    pIni->Get (SFX_KEY_INET_SOCKSPROXYPORT).ToInt32());

                xINetConfig->setProxyConfig (aProxyCfg);
            }
        }
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
    pIni->LeaveLock();
    pIni->Flush();
    SaveConfiguration();
}

//--------------------------------------------------------------------

void SfxApplication::SetOptions(const SfxItemSet &rSet)
{
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
        GetAppIniManager()->Set(SFX_GROUP_DIR, *(const SfxAllEnumItem *)pItem);
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
    if ( pOptions->IsAutoSave() )
    {
        pImp->pAutoSaveTimer->SetTimeout( pOptions->GetAutoSaveTime() * 60000 );
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
    FASTBOOL bAutoSave = pOptions->IsAutoSave() &&
        !bDispatcherLocked && !Application::IsInModalMode() &&
        !Application::IsUICaptured() && Application::GetLastInputInterval() > 300;
    if ( bAutoSave )
    {
        SfxViewShell *pVSh = pViewFrame ? pViewFrame->GetViewShell() : 0;
        bAutoSave = pVSh && pVSh->GetWindow() &&
                    !pVSh->GetWindow()->IsMouseCaptured() ;
    }

    if ( bAutoSave )
    {
        SaveAll_Impl( pOptions->IsAutoSavePrompt(), TRUE );
        pImp->bAutoSaveNow = FALSE;
        pImp->pAutoSaveTimer->SetTimeout( pOptions->GetAutoSaveTime() * 60000 );
        pImp->pAutoSaveTimer->Start();
    }
    else if ( pOptions->IsAutoSave() )
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
            SfxViewFrame::GetFirst(pDoc) )
        {
            if ( pDoc->GetProgress() == 0 )
            {
                if (!pDoc->IsModified() ||
                    bAutoSave && pDoc->GetModifyTime()<pImp->aAutoSaveTime)
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
                        bFunc = FALSE; // kein Abbruch mehr!
                }
                else if ( nRet == RET_CANCEL )
                {
                    bFunc = FALSE; // kein Abbruch mehr!
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

void SfxApplication::RegisterEvent(USHORT nId, const String& rEventName)
{
    if (!pAppData_Impl->pEventConfig)
        pAppData_Impl->pEventConfig = new SfxEventConfiguration;
    pAppData_Impl->pEventConfig->RegisterEvent(nId, rEventName);
}

//--------------------------------------------------------------------

SfxEventConfiguration* SfxApplication::GetEventConfig() const
{
    return pAppData_Impl->pEventConfig;
}

//--------------------------------------------------------------------

void SfxApplication::SaveConfiguration() const
{
    if ( !bDowning )
    {
        // bei bDowning koennten falsche Sachen gespeichert werden bishin
        // zu Abstuerzen
        if (!pAppData_Impl->pAppCfg->SaveConfig())
            HandleConfigError_Impl((USHORT)pCfgMgr->GetErrorCode());
    }
}

//--------------------------------------------------------------------

SfxConfigManager* SfxApplication::GetAppConfigManager_Impl() const
{
    return pAppData_Impl->pAppCfg;
}
//--------------------------------------------------------------------

void SfxApplication::NotifyEvent( const SfxEventHint& rEventHint, FASTBOOL bSynchron )
{
    DBG_ASSERT(pAppData_Impl->pEventConfig,"Keine Events angemeldet!");

    SfxObjectShell *pDoc = rEventHint.GetObjShell();
    if ( !pDoc || !pDoc->IsPreview() )
        pAppData_Impl->pEventConfig->ExecuteEvent( rEventHint.GetEventId(), pDoc, bSynchron, rEventHint.GetArgs() );
    Broadcast(rEventHint);
    if ( pDoc )
        pDoc->Broadcast( rEventHint );
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

void SfxApplicationClass::Property( ApplicationProperty& rProp )
{
    SfxApplication* pApp = SFX_APP();
    SfxIniManager* pIni = pApp->GetIniManager();

    // AppIniManger?
    SfxAppIniManagerProperty* pAppIniMgr = PTR_CAST(SfxAppIniManagerProperty, &rProp);
    if ( pAppIniMgr )
    {
        pAppIniMgr->SetIniManager( pIni );
        return;
    }

    TTProperties* pTTProperties = PTR_CAST( TTProperties, &rProp );
    if ( pTTProperties )
    {
        pTTProperties->nPropertyVersion = TT_PROPERTIES_VERSION;
        switch ( pTTProperties->nActualPR )
        {
            case TT_PR_SLOTS:
            {
                pTTProperties->nSidOpenUrl = SID_OPENURL;
                pTTProperties->nSidFileName = SID_FILE_NAME;
                pTTProperties->nSidNewDocDirect = SID_NEWDOCDIRECT;
                pTTProperties->nSidCopy = SID_COPY;
                pTTProperties->nSidPaste = SID_PASTE;
                pTTProperties->nSidSourceView = SID_SOURCEVIEW;
                pTTProperties->nSidSelectAll = SID_SELECTALL;
                pTTProperties->nSidReferer = SID_REFERER;
                pTTProperties->nActualPR = 0;
            }
            break;
            case TT_PR_DISPATCHER:
            {
                SfxDispatcher* pDispatcher = SfxViewFrame::Current()->GetDispatcher();
                if ( !pDispatcher )
                    pTTProperties->nActualPR = TT_PR_ERR_NODISPATCHER;
                else
                {
                    pDispatcher->SetExecuteMode(EXECUTEMODE_DIALOGASYNCHRON);
                    if ( pDispatcher->ExecuteFunction(
                            pTTProperties->mnSID, pTTProperties->mppArgs, pTTProperties->mnMode )
                         == EXECUTE_NO )
                        pTTProperties->nActualPR = TT_PR_ERR_NOEXECUTE;
                    else
                        pTTProperties->nActualPR = 0;
                }
            }
            break;
            case TT_PR_IMG:
            {
                SvDataMemberObjectRef aDataObject = new SvDataMemberObject();
                SvData* pDataBmp = new SvData( FORMAT_BITMAP );
                pDataBmp->SetData( pTTProperties->mpBmp );
                aDataObject->Append( pDataBmp );
                aDataObject->CopyClipboard();
                pTTProperties->nActualPR = 0;
            }
            break;
            default:
            {
                pTTProperties->nPropertyVersion = 0;
            }
        }
        return;
    }
}

IMPL_OBJHINT( SfxStringHint, String )

OfaMiscCfg* SfxApplication::GetMiscConfig()
{
    if ( !pAppData_Impl->pMiscConfig )
    {
        pAppData_Impl->pMiscConfig = new OfaMiscCfg;
        pAppData_Impl->pMiscConfig->Initialize();
    }

    return pAppData_Impl->pMiscConfig;
}


