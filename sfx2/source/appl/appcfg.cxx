/*************************************************************************
 *
 *  $RCSfile: appcfg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: mba $ $Date: 2000-11-16 15:30:58 $
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
#if SUPD<613//MUSTINI
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
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
#if SUPD<613//MUSTINI
#ifndef _SFXINIPROP_HXX
#include <svtools/iniprop.hxx>
#endif
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
#include <svtools/saveopt.hxx>
#include <svtools/helpopt.hxx>
#include <svtools/undoopt.hxx>
#include <svtools/securityoptions.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/inetoptions.hxx>

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

//--------------------------------------------------------------------

BOOL SfxApplication::GetOptions( SfxItemSet& rSet )
{
    BOOL bRet = FALSE;
#if SUPD<613//MUSTINI
    SfxIniManager *pIni = GetIniManager();
#endif
    SfxItemPool &rPool = GetPool();
    SfxToolBoxConfig *pTbxCfg = SfxToolBoxConfig::GetOrCreate();
    String aTRUEStr = 0x0031; // ^= '1'

    const USHORT *pRanges = rSet.GetRanges();
    SvtSaveOptions aSaveOptions;
    SvtUndoOptions aUndoOptions;
    SvtHelpOptions aHelpOptions;
    SvtInetOptions aInetOptions;
    SvtSecurityOptions  aSecurityOptions;
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
                              aSaveOptions.IsBackup())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_AUTOSAVE :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_AUTOSAVE ),
                              aSaveOptions.IsAutoSave())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_AUTOSAVEPROMPT :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_AUTOSAVEPROMPT ),
                              aSaveOptions.IsAutoSavePrompt())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_AUTOSAVEMINUTE :
                    if(rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_ATTR_AUTOSAVEMINUTE ),
                                aSaveOptions.GetAutoSaveTime())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_DOCINFO :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_DOCINFO ),
                              aSaveOptions.IsDocInfoSave())))
                        bRet = TRUE;
                    break;
                case SID_OPT_SAVEGRAPHICSCOMPRESSED :
                {
                    SfxDocumentInfo *pDocInf = SfxObjectShell::Current() ? &SfxObjectShell::Current()->GetDocInfo() : 0;
                    BOOL bComprGraph = pDocInf ? pDocInf->IsSaveGraphicsCompressed() :
                        aSaveOptions.GetSaveGraphicsMode() == SvtSaveOptions::SaveGraphicsCompressed;
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_OPT_SAVEGRAPHICSCOMPRESSED ),
                              bComprGraph ) ) )
                        bRet = TRUE;
                    break;
                }
                case SID_OPT_SAVEORIGINALGRAPHICS :
                {
                    SfxDocumentInfo *pDocInf = SfxObjectShell::Current() ? &SfxObjectShell::Current()->GetDocInfo() : 0;
                    BOOL bOrigGraph = pDocInf ? pDocInf->IsSaveOriginalGraphics() :
                        aSaveOptions.GetSaveGraphicsMode() == SvtSaveOptions::SaveGraphicsOriginal;
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_OPT_SAVEORIGINALGRAPHICS ),
                              bOrigGraph ) ) )
                        bRet = TRUE;
                    break;
                }
                case SID_ATTR_WORKINGSET :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_WORKINGSET ),
                              aSaveOptions.IsSaveWorkingSet())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_SAVEDOCWINS :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_SAVEDOCWINS ),
                              aSaveOptions.IsSaveDocWins())))
                        bRet = TRUE;
                    break;
                case SID_ATTR_SAVEDOCVIEW :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_SAVEDOCVIEW ),
                              aSaveOptions.IsSaveDocView())))
                        bRet = TRUE;
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
                case SID_ATTR_WELCOMESCREEN :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_ATTR_WELCOMESCREEN ),
                               aHelpOptions.IsWelcomeScreen() ) ) )
                        bRet = TRUE;
                    break;
                case SID_ATTR_UNDO_COUNT :
                    if(rSet.Put( SfxUInt16Item ( rPool.GetWhich( SID_ATTR_UNDO_COUNT ),
                                 aUndoOptions.GetUndoCount() ) ) )
                        bRet = TRUE;
                    break;

                case SID_INET_HOMEPAGE          :
                case SID_INET_MEMCACHE          :
                case SID_INET_DISKCACHE         :
                case SID_INET_EXPIRATION        :
                case SID_INET_CACHEJS           :
                case SID_INET_CACHEEXPIRED      :
                case SID_INET_CACHEABORTED      :
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                    DBG_ASSERT( sal_False, "SfxApplication::GetOptions()\nSome INET values not no longer supported!\n" );
#endif
                    break;

                case SID_INET_REVEAL_MAILADDR :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_INET_REVEAL_MAILADDR),
                                aInetOptions.GetProtocolRevealMailAddress() ) ) )
                        bRet = TRUE;
                    break;
                case SID_SAVEREL_INET :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_SAVEREL_INET ),
                               aSaveOptions.IsSaveRelINet() ) ) )
                        bRet = TRUE;
                    break;
                case SID_SAVEREL_FSYS :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_SAVEREL_FSYS ),
                               aSaveOptions.IsSaveRelFSys() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_SMTPSERVER :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_INET_SMTPSERVER),
                                aInetOptions.GetSmtpServerName() )))
                        bRet = TRUE;
                    break;

                case SID_INET_POPSERVER :
                case SID_INET_NNTPSERVER :
                case SID_INET_MAXNEWS :
                case SID_INET_MAXHTTPCONS :
                case SID_INET_MAXFTPCONS :
                case SID_INET_SMTPGATEWAY :
                case SID_INET_MAILUSERNAME :
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                    DBG_ASSERT( sal_False, "SfxApplication::GetOptions()\nSome INET values not longer supported!\n" );
#endif
                    break;
                case SID_ATTR_ALLOWFOLDERWEBVIEW :
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                    DBG_ASSERT(sal_False,"SfxApplication::GetOptions()\nINI-Key \"Common\\AllowFolderWebView\" is obsolete ... SID_ATTR_ALLOWFOLDERWEBVIEW isn't supported any longer!\n");
#endif
                    break;
                case SID_INET_MAILPASSWORD :
                case SID_INET_MAILTEXTFORMAT :
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                    DBG_ASSERT( sal_False, "SfxApplication::GetOptions()\nConfig item for INET values not implemented yet!\n" );
#endif
                    break;
                case SID_BASIC_ENABLED :
                    if ( rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_BASIC_ENABLED ),
                                aSecurityOptions.GetBasicMode())))
                        bRet = TRUE;
                    break;
                case SID_INET_JAVA_ENABLE:
                {
                    DBG_ERROR( "Not implemented!" );
/*
                    String aIniEntry = GetJavaIniEntry_Impl( DEFINE_CONST_UNICODE("Java") );
                    if ( rSet.Put( SfxBoolItem(
                            rPool.GetWhich( SID_INET_JAVA_ENABLE ), ( aIniEntry == aTRUEStr ) ) ) )
                        bRet = TRUE;
*/
                    break;
                }

                case SID_INET_EXE_APPLETS :
                {
                    DBG_ERROR( "Not implemented!" );
/*
                    String aIniEntry = GetJavaIniEntry_Impl( DEFINE_CONST_UNICODE("Applets") );
                    if ( rSet.Put( SfxBoolItem(
                            rPool.GetWhich( SID_INET_EXE_APPLETS ), ( aIniEntry == aTRUEStr ) ) ) )
                        bRet = TRUE;
 */
                    break;
                }
                case SID_INET_JAVA_ACCESSTYPE:
                {
                    DBG_ERROR( "Not implemented!" );
/*
                    SjNetAccess eAccess = NET_HOST;
                    String aNetAccess = GetJavaIniEntry_Impl( DEFINE_CONST_UNICODE("NetAccess") );
                    if ( aNetAccess.Len() )
                    {
                        if ( aNetAccess.CompareIgnoreCaseToAscii( "UNRESTRICTED" ) == COMPARE_EQUAL )
                            eAccess = NET_UNRESTRICTED;
                        else if ( aNetAccess.CompareIgnoreCaseToAscii( "NONE" ) == COMPARE_EQUAL )
                            eAccess = NET_NONE;
                    }

                    if ( rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_INET_JAVA_ACCESSTYPE ), (USHORT)eAccess ) ) )
                        bRet = TRUE;
 */
                    break;
                }
                case SID_INET_JAVA_SECURITY:
                {
                    DBG_ERROR( "Not implemented!" );
/*
                    String aIniEntry = GetJavaIniEntry_Impl( DEFINE_CONST_UNICODE("Security") );
                    if ( rSet.Put( SfxBoolItem(
                            rPool.GetWhich( SID_INET_JAVA_SECURITY ), ( aIniEntry.CompareToAscii("0") != COMPARE_EQUAL ) ) ) )
                        bRet = TRUE;
 */
                    break;
                }
                case SID_INET_JAVA_CLASSPATH :
                {
                    DBG_ERROR( "Not implemented!" );
/*
                    String aIniEntry = GetJavaIniEntry_Impl( DEFINE_CONST_UNICODE("UserClassPath") );
                    if ( rSet.Put( SfxStringItem( rPool.GetWhich( SID_INET_JAVA_CLASSPATH ), aIniEntry ) ) )
                        bRet = TRUE;
 */
                    break;
                }

                case SID_INET_EXE_PLUGIN  :
                case SID_INET_USERAGENT :
                case SID_INET_NONCACHED_SERVER :
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                    DBG_ASSERT( sal_False, "SfxApplication::GetOptions()\nSome INET values no longer supported!\n" );
#endif
                    break;

                case SID_INET_DNS_SERVER :
                    if ( rSet.Put( SfxStringItem( rPool.GetWhich(SID_INET_DNS_SERVER),
                                     aInetOptions.GetDnsIpAddress() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_DNS_AUTO  :
                    if ( rSet.Put( SfxBoolItem( rPool.GetWhich( SID_INET_DNS_AUTO ),
                                !aInetOptions.GetDnsIpAddress().getLength() ) ) )
                        bRet = TRUE;
                    break;
                case SID_INET_CHANNELS_ONOFF  :
                case SID_INET_COOKIESHANDLE :
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                    DBG_ASSERT( sal_False, "SfxApplication::GetOptions()\nSome INET values no longer supported!\n" );
#endif
                    break;

                case SID_SECURE_URL :
                {
                    ::com::sun::star::uno::Sequence< ::rtl::OUString > seqURLs = aSecurityOptions.GetSecureURLs();
                    List aList;
                    sal_uInt32 nCount = seqURLs.getLength();
                    sal_uInt32 nURL;
                    for( nURL=0; nURL<nCount; ++nURL )
                    {
                        aList.Insert( new String( seqURLs[nURL] ), LIST_APPEND );
                    }
                    if( rSet.Put( SfxStringListItem( rPool.GetWhich(SID_SECURE_URL),
                            &aList ) ) )
                    {
                        bRet = TRUE;
                    }
                    for( nURL=0; nURL<nCount; ++nURL )
                    {
                        delete (String*)aList.GetObject(nURL);
                    }
                    aList.Clear();
                    break;
                }
                case SID_ICONGRID :
                case SID_AUTO_ADJUSTICONS :
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                    DBG_ASSERT(sal_False, "SfxApplication::GetOptions()\nSoffice.ini key \"View\\IconGrid\" is obsolete! .. How I can support SID_ICONGRID & SID_AUTO_ADJUSTICONS any longer?\n");
#endif
                    break;

                case SID_RESTORE_EXPAND_STATE :
                {
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                    DBG_ASSERT(sal_False, "SfxApplication::GetOptions()\nSoffice.ini key \"View\\AutoOpen\" is obsolete! .. How I can support SID_RESTORE_EXPAND_STATE any longer?\n");
#endif

                    break;
                }
                case SID_ENABLE_METAFILEPRINT :
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                    DBG_ASSERT(sal_False, "SfxApplication::GetOptions()\nSoffice.ini key \"Common\\MetafilePrint\" is obsolete! .. How I can support SID_ENABLE_METAFILEPRINT any longer?\n");
#endif

                    break;
                case SID_INET_PROXY_TYPE :
                    if(rSet.Put( SfxUInt16Item ( rPool.GetWhich( SID_INET_PROXY_TYPE ),
                                aInetOptions.GetProxyType() )))
                        bRet = TRUE;
                    break;
                case SID_INET_FTP_PROXY_NAME :
                    if ( rSet.Put( SfxStringItem ( rPool.GetWhich(SID_INET_FTP_PROXY_NAME ),
                            aInetOptions.GetProxyFtpName() )))
                        bRet = TRUE;
                    break;
                case SID_INET_FTP_PROXY_PORT :
                    if ( rSet.Put( SfxInt32Item ( rPool.GetWhich(SID_INET_FTP_PROXY_PORT ),
                            aInetOptions.GetProxyFtpPort() )))
                        bRet = TRUE;
                    break;
                case SID_INET_HTTP_PROXY_NAME :
                    if ( rSet.Put( SfxStringItem ( rPool.GetWhich(SID_INET_HTTP_PROXY_NAME ),
                            aInetOptions.GetProxyHttpName() )))
                        bRet = TRUE;
                    break;
                case SID_INET_HTTP_PROXY_PORT :
                    if ( rSet.Put( SfxInt32Item( rPool.GetWhich(SID_INET_HTTP_PROXY_PORT ),
                            aInetOptions.GetProxyHttpPort() )))
                        bRet = TRUE;
                    break;
                case SID_INET_SOCKS_PROXY_NAME :
                    if ( rSet.Put( SfxStringItem ( rPool.GetWhich(SID_INET_SOCKS_PROXY_NAME ),
                            aInetOptions.GetProxySocksName() )))
                        bRet = TRUE;
                    break;
                case SID_INET_SOCKS_PROXY_PORT :
                    if ( rSet.Put( SfxInt32Item( rPool.GetWhich(SID_INET_SOCKS_PROXY_PORT ),
                            aInetOptions.GetProxySocksPort() )))
                        bRet = TRUE;
                    break;
                case SID_INET_SECURITY_PROXY_NAME :
                case SID_INET_SECURITY_PROXY_PORT :
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                    DBG_ASSERT( sal_False, "SfxApplication::GetOptions()\nSome INET values no longer supported!\n" );
#endif
                    break;

                case SID_INET_NOPROXY :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_INET_NOPROXY),
                                aInetOptions.GetProxyNoProxy() )))
                        bRet = TRUE;
                    break;
                case SID_ATTR_PATHNAME :
                case SID_ATTR_PATHGROUP :
                {
                    SfxAllEnumItem aNames(rPool.GetWhich(SID_ATTR_PATHGROUP));
                    SfxAllEnumItem aValues(rPool.GetWhich(SID_ATTR_PATHNAME));
                    SvtPathOptions aPathCFG;
                    sal_uInt32 nCount   = aNames.GetValueCount();
                    sal_uInt32 nPosition= 0;
                    for( nPosition=0; nPosition<nCount; ++nPosition )
                    {
                        String sPathType    = aNames.GetValueTextByPos(nPosition);
                        String sValue;
                        if( sPathType.CompareToAscii("Addin") )
                        {
                            sValue = aPathCFG.GetAddinPath();
                        }
                        else
                        if( sPathType.CompareToAscii("AutoCorrect") )
                        {
                            sValue = aPathCFG.GetAutoCorrectPath();
                        }
                        else
                        if( sPathType.CompareToAscii("AutoPilot") )
                        {
                            sValue = aPathCFG.GetAutoPilotPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Backup") )
                        {
                            sValue = aPathCFG.GetBackupPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Basic") )
                        {
                            sValue = aPathCFG.GetBasicPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Bitmap") )
                        {
                            sValue = aPathCFG.GetBitmapPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Config") )
                        {
                            sValue = aPathCFG.GetConfigPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Database") )
                        {
                            sValue = aPathCFG.GetDatabasePath();
                        }
                        else
                        if( sPathType.CompareToAscii("Dictionary") )
                        {
                            sValue = aPathCFG.GetDictionaryPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Favorites") )
                        {
                            sValue = aPathCFG.GetFavoritesPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Filter") )
                        {
                            sValue = aPathCFG.GetFilterPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Gallery") )
                        {
                            sValue = aPathCFG.GetGalleryPath();
                        }
                        else
                        if( sPathType.CompareToAscii("AutoText") )
                        {
                            sValue = aPathCFG.GetAutoTextPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Graphic") )
                        {
                            sValue = aPathCFG.GetGraphicPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Help") )
                        {
                            sValue = aPathCFG.GetHelpPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Linguistic") )
                        {
                            sValue = aPathCFG.GetLinguisticPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Module") )
                        {
                            sValue = aPathCFG.GetModulePath();
                        }
                        else
                        if( sPathType.CompareToAscii("NewMenu") )
                        {
                            sValue = aPathCFG.GetNewMenuPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Palette") )
                        {
                            sValue = aPathCFG.GetPalettePath();
                        }
                        else
                        if( sPathType.CompareToAscii("Plugin") )
                        {
                            sValue = aPathCFG.GetPluginPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Storage") )
                        {
                            sValue = aPathCFG.GetStoragePath();
                        }
                        else
                        if( sPathType.CompareToAscii("Temp") )
                        {
                            sValue = aPathCFG.GetTempPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Template") )
                        {
                            sValue = aPathCFG.GetTemplatePath();
                        }
                        else
                        if( sPathType.CompareToAscii("Trash") )
                        {
                            sValue = aPathCFG.GetTrashPath();
                        }
                        else
                        if( sPathType.CompareToAscii("UserConfig") )
                        {
                            sValue = aPathCFG.GetUserConfigPath();
                        }
                        else
                        if( sPathType.CompareToAscii("UserDictionary") )
                        {
                            sValue = aPathCFG.GetUserDictionaryPath();
                        }
                        else
                        if( sPathType.CompareToAscii("Work") )
                        {
                            sValue = aPathCFG.GetWorkPath();
                        }
                        else
                        {
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                            DBG_ASSERT(sal_False,"SfxApplication::GetOptions()\nUnsupported path name detected!\n");
#endif
                        }
                        aValues.InsertValue(nPosition,sValue);
                    }

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
    return SvtSecurityOptions().IsSecureURL( rURL.GetMainURL(), *pReferer );
}
//--------------------------------------------------------------------

void SfxApplication::SetOptions_Impl( const SfxItemSet& rSet )
{
#if SUPD<613//MUSTINI
    SfxIniManager *pIni = GetIniManager();
    pIni->EnterLock();
#endif
    const SfxPoolItem *pItem = 0;
    SfxItemPool &rPool = GetPool();
    BOOL bResetSession = FALSE;
    BOOL bProxiesModified = FALSE;

    SfxToolBoxConfig *pTbxCfg = SfxToolBoxConfig::GetOrCreate();
    SvtSaveOptions aSaveOptions;
    SvtUndoOptions aUndoOptions;
    SvtHelpOptions aHelpOptions;
    SvtSecurityOptions aSecurityOptions;
    SvtPathOptions aPathOptions;
    SvtInetOptions aInetOptions;
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
        aSaveOptions.SetBackup( ( (const SfxBoolItem*)pItem )->GetValue() );
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

    if(
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_HOMEPAGE), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_MEMCACHE), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_DISKCACHE), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_EXPIRATION), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_CACHEJS), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_CACHEEXPIRED), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_CACHEABORTED), TRUE, &pItem))
    )
    {
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
        DBG_ASSERT(sal_False, "SfxApplication::SetOptions_Impl()\nSome INET keys no longer supported!\n");
#endif
    }

    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_REVEAL_MAILADDR), TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxBoolItem), "BoolItem expected" );
        aInetOptions.SetProtocolRevealMailAddress( ((const SfxBoolItem*)pItem)->GetValue() );
        bResetSession = TRUE;
    }

    // SMTP-Server
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_SMTPSERVER), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        aInetOptions.SetSmtpServerName(((const SfxStringItem *)pItem)->GetValue());
        bResetSession = TRUE;
    }
    if(
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_POPSERVER), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_NNTPSERVER), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_MAXNEWS), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_MAXHTTPCONS), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_MAXFTPCONS), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_SMTPGATEWAY), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_MAILUSERNAME), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_MAILPASSWORD), TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_MAILTEXTFORMAT ), TRUE, &pItem ) )
    )
    {
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
        DBG_ASSERT(sal_False, "SfxApplication::SetOptions_Impl()\nSome INET keys no longer supported!\n");
#endif
    }


    // Proxy-Type
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_PROXY_TYPE), TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxUInt16Item), "UInt16Item expected" );
        aInetOptions.SetProxyType((SvtInetOptions::ProxyType)( (const SfxUInt16Item*)pItem )->GetValue());
        bResetSession = TRUE;
        bProxiesModified = TRUE;
    }

    // NoProxy
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_NOPROXY, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        aInetOptions.SetProxyNoProxy(((const SfxStringItem *)pItem)->GetValue());
        bResetSession = TRUE;
        bProxiesModified = TRUE;
    }

    // StarBasic Enable
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_BASIC_ENABLED, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "SfxInt16Item expected");
        aSecurityOptions.SetBasicMode( (EBasicSecurityMode)( (const SfxUInt16Item*)pItem )->GetValue() );
    }

    // Java Enable
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_INET_JAVA_ENABLE, TRUE, &pItem ) )
    {
                    DBG_ERROR( "Not implemented!" );
/*
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        BOOL bJava = ( (const SfxBoolItem*)pItem )->GetValue();
        String aVal = bJava ? 0x0031 : 0x0030; // ^= '1' or '0'
        BOOL bSet = SetJavaIniEntry_Impl( DEFINE_CONST_UNICODE("Java"), aVal );
        bResetSession = TRUE;
 */
    }
    // Applets Enable
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_INET_EXE_APPLETS, TRUE, &pItem ) )
    {
                    DBG_ERROR( "Not implemented!" );
/*
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        BOOL bApplets = ( (const SfxBoolItem*)pItem )->GetValue();
        String aVal = bApplets ? 0x0031 : 0x0030; // ^= '1' or '0'
        BOOL bSet = SetJavaIniEntry_Impl( DEFINE_CONST_UNICODE("Applets"), aVal );
        bResetSession = TRUE;
 */
    }

    // Java AccessType
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_JAVA_ACCESSTYPE), TRUE, &pItem))
    {
                    DBG_ERROR( "Not implemented!" );
/*
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "UInt16Item expected");
        SjNetAccess eAccess = (SjNetAccess) ((const SfxUInt16Item *)pItem)->GetValue();
        String aVal;
        if ( eAccess == NET_HOST )
            aVal = DEFINE_CONST_UNICODE("HOST");
        else if ( eAccess == NET_UNRESTRICTED )
            aVal = DEFINE_CONST_UNICODE("UNRESTRICTED");
        else
            aVal = DEFINE_CONST_UNICODE("NONE");
        BOOL bSet = SetJavaIniEntry_Impl( DEFINE_CONST_UNICODE("NetAccess"), aVal );
        bResetSession = TRUE;
 */
    }

    // Java Security
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_JAVA_SECURITY, TRUE, &pItem))
    {
                    DBG_ERROR( "Not implemented!" );
/*
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        BOOL bSecurity = ((const SfxBoolItem *)pItem)->GetValue();
        String aVal = bSecurity ? 0x0031 : 0x0030; // ^= '1' or '0'
        BOOL bSet = SetJavaIniEntry_Impl( DEFINE_CONST_UNICODE("Security"), aVal );
        bResetSession = TRUE;
 */
    }

    // Java ClassPath
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_JAVA_CLASSPATH, TRUE, &pItem))
    {
                    DBG_ERROR( "Not implemented!" );
/*
        DBG_ASSERT( pItem->ISA(SfxStringItem), "SfxStringItem expected" );
        String aVal = ( (const SfxStringItem *)pItem )->GetValue();
        BOOL bSet = SetJavaIniEntry_Impl( DEFINE_CONST_UNICODE("UserClassPath"), aVal );
        bResetSession = TRUE;
 */
    }

    // Execute PlugIns
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_EXE_PLUGIN, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        BOOL bExe = ( (const SfxBoolItem *)pItem )->GetValue();
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
        DBG_ASSERT(sal_False, "SfxApplication::SetOptions_Impl()\nSome INET keys no longer supported!\n");
#endif

        bResetSession = TRUE;
    }

    // DNS Server
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
    }

    if(
        ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_NONCACHED_SERVER, TRUE, &pItem))   ||
        ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_USERAGENT, TRUE, &pItem))
    )
    {
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
        DBG_ASSERT(sal_False, "SfxApplication::SetOptions_Impl()\nSome INET keys no longer supported!\n");
#endif
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
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_SOCKS_PROXY_NAME ), TRUE, &pItem ) )
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

    if(
        ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_CHANNELS_ONOFF, TRUE, &pItem)) ||
        ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_COOKIESHANDLE, TRUE, &pItem))
    )
    {
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
        DBG_ASSERT(sal_False, "SfxApplication::SetOptions_Impl()\nSome INET keys no longer supported!\n");
#endif
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

    // EnableMetafilePrint
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_ENABLE_METAFILEPRINT ), TRUE, &pItem ) )
    {
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
        DBG_ASSERT(sal_False, "SfxApplication::SetOptions_Impl()\nsoffice.ini key \"MetafilPrint\" not supported any longer!\n");
#endif
    }

    // Web-View
    if( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_ALLOWFOLDERWEBVIEW), TRUE, &pItem))
    {
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
        DBG_ASSERT(sal_False, "SfxApplication::SetOptions_Impl()\nsoffice.ini key \"AllowFolderWebView\" not supported any longer!\n");
#endif
    }

    if ( bResetSession )
    {
        NAMESPACE_INET(INetWrapper) aWrapper;
        ::vos::ORef< ::inet::INetConfig > xINetConfig;
        if ( aWrapper.getINetConfig( xINetConfig ) )
        {
            xINetConfig->setDomainNameServer( aInetOptions.GetDnsIpAddress() );

            if ( bProxiesModified )
            {
                ::inet::INetProxyConfig aProxyCfg (xINetConfig->getProxyConfig());
                SvtInetOptions::ProxyType eProxy = (SvtInetOptions::ProxyType)aInetOptions.GetProxyType();

                if (eProxy == SvtInetOptions::ProxyType::NONE)
                    aProxyCfg.setNoProxyList(aInetOptions.GetProxyNoProxy());
                else
                    aProxyCfg.setNoProxyList(DEFINE_CONST_UNICODE("*:*"));

                aProxyCfg.setHttpProxy (
                    aInetOptions.GetProxyHttpName(),
                    aInetOptions.GetProxyHttpPort());

                aProxyCfg.setFtpProxy (
                    aInetOptions.GetProxyFtpName(),
                    aInetOptions.GetProxyFtpPort());

                aProxyCfg.setSocksProxy (
                    aInetOptions.GetProxySocksName(),
                    aInetOptions.GetProxySocksPort());

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
#if SUPD<613//MUSTINI
    pIni->LeaveLock();
    pIni->Flush();
#endif
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
#if SUPD<613//MUSTINI
        GetAppIniManager()->Set(SFX_GROUP_DIR, *(const SfxAllEnumItem *)pItem);
#else
        const SfxAllEnumItem* pEnumItem = (const SfxAllEnumItem *)pItem;
        sal_uInt32 nCount = pEnumItem->GetValueCount();
        for( sal_uInt32 nPath=0; nPath<nCount; ++nPath )
        {
            String sValue = pEnumItem->GetValueTextByPos(nPath);
            switch( nPath )
            {
            case 50 :   aPathOptions.SetSubIniPath( sValue );
                        break;
            case 51 :   aPathOptions.SetConfigPath( sValue );
                        break;
            case 52 :   aPathOptions.SetWorkPath( sValue );
                        break;
            case 53 :   break;  // GRAPHICSPATH
            case 54 :   aPathOptions.SetBitmapPath( sValue );
                        break;
            case 55 :   aPathOptions.SetBasicPath( sValue );
                        break;
            case 56 :   aPathOptions.SetDatabasePath( sValue );
                        break;
            case 57 :   aPathOptions.SetPalettePath( sValue );
                        break;
            case 58 :   aPathOptions.SetBackupPath( sValue );
                        break;
            case 59 :   aPathOptions.SetModulePath( sValue );
                        break;
            case 60 :   aPathOptions.SetTemplatePath( sValue );
                        break;
            case 61 :   aPathOptions.SetAutoTextPath( sValue );
                        break;
            case 62 :   aPathOptions.SetDictionaryPath( sValue );
                        break;
            case 63 :   break;  // DESKTOPPATH
            case 64 :   aPathOptions.SetHelpPath( sValue );
                        break;
            case 65 :   break;  // BOOKMARKPATH
            case 66 :   aPathOptions.SetGalleryPath( sValue );
                        break;
            case 67 :   aPathOptions.SetNewMenuPath( sValue );
                        break;
            case 68 :   break;  // AGENTPATH
            case 69 :   aPathOptions.SetAutoPilotPath( sValue );
                        break;
            case 70 :   break;  // EXPLORERPATH
            case 71 :   aPathOptions.SetTrashPath( sValue );
                        break;
            case 72 :   aPathOptions.SetStoragePath( sValue );
                        break;
            case 73 :   break;     //SFX_KEY_STARTMENU_DIR
            case 74 :   break;     //SFX_KEY_DOWNLOAD_DIR
            case 75 :   break;     //SFX_KEY_AUTOSTART_DIR
            case 76 :   break;     //SFX_KEY_QUICKSTART_DIR
            case 77 :   break;     //SFX_KEY_GROUP_DIR
            case 78 :   aPathOptions.SetPluginPath( sValue );
                        break;
            case 79 :   aPathOptions.SetFavoritesPath( sValue );
                        break;
            case 80 :   aPathOptions.SetFilterPath( sValue );
                        break;
            case 81 :   aPathOptions.SetAddinPath( sValue );
                        break;
            case 82 :   aPathOptions.SetUserConfigPath( sValue );
                        break;
            case 83 :   aPathOptions.SetUserDictionaryPath( sValue );
                        break;
            case 84 :   aPathOptions.SetLinguisticPath( sValue );
                        break;
            case 85 :   aPathOptions.SetAutoCorrectPath( sValue );
                        break;
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
            default: DBG_ASSERT(sal_False, "SfxApplication::SetOptions_Impl()\nInvalid path number found for set directories!\n");
#endif
            }
        }
#endif
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
#if SUPD<613//MUSTINI
    SfxApplication* pApp = SFX_APP();
    SfxIniManager* pIni = pApp->GetIniManager();

    // AppIniManger?
    SfxAppIniManagerProperty* pAppIniMgr = PTR_CAST(SfxAppIniManagerProperty, &rProp);
    if ( pAppIniMgr )
    {
        pAppIniMgr->SetIniManager( pIni );
        return;
    }
#endif
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


