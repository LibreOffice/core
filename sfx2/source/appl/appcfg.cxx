/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XFlushable.hpp>

#include <stdlib.h>
#include <tools/config.hxx>
#include <vcl/sound.hxx>
#include <vcl/msgbox.hxx>
#include <tools/string.hxx>
#include <svl/itempool.hxx>
#include <svl/aeitem.hxx>
#include <svl/slstitm.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/szitem.hxx>
#include <svl/undo.hxx>

#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>
#include <svtools/ttprops.hxx>
#include <sfx2/sfxsids.hrc>
#include <sot/exchange.hxx>

#include <svl/isethint.hxx>

#include <unotools/configmgr.hxx>
#include <tools/urlobj.hxx>
#include <tools/wldcrd.hxx>
#include <unotools/saveopt.hxx>
#include <svtools/helpopt.hxx>
#include <unotools/undoopt.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/inetoptions.hxx>
#include <svtools/miscopt.hxx>
#include <vcl/toolbox.hxx>
#include <unotools/localfilehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include "sfxhelp.hxx"
#include "sfxtypes.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include "objshimp.hxx"
#include <sfx2/viewsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/evntconf.hxx>
#include "appdata.hxx"
#include "workwin.hxx"
#include <sfx2/macrconf.hxx>
#include "helper.hxx"   // SfxContentHelper::...
#include "app.hrc"
#include "sfxresid.hxx"
#include "shutdownicon.hxx"

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

void SfxEventAsyncer_Impl::Notify( SfxBroadcaster&, const SfxHint& rHint )
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

IMPL_LINK(SfxEventAsyncer_Impl, TimerHdl, Timer*, pAsyncTimer)
{
    (void)pAsyncTimer; // unused variable
    SfxObjectShellRef xRef( aHint.GetObjShell() );
    pAsyncTimer->Stop();
#ifdef DBG_UTIL
    if (!xRef.Is())
    {
        ByteString aTmp( "SfxEvent: ");
        aTmp += ByteString( String( aHint.GetEventName() ), RTL_TEXTENCODING_UTF8 );
        DBG_TRACE( aTmp.GetBuffer() );
    }
#endif
    SFX_APP()->Broadcast( aHint );
    if ( xRef.Is() )
        xRef->Broadcast( aHint );
    delete this;
    return 0L;
}


//--------------------------------------------------------------------

BOOL SfxApplication::GetOptions( SfxItemSet& rSet )
{
    BOOL bRet = FALSE;
    SfxItemPool &rPool = GetPool();
    String aTRUEStr('1');

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
                {
                    if( rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_BUTTON_BIGSIZE ), aMiscOptions.AreCurrentSymbolsLarge() ) ) )
                        bRet = TRUE;
                    break;
                }
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
                case SID_ATTR_WARNALIENFORMAT:
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_WARNALIENFORMAT))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_WARNALIENFORMAT ), aSaveOptions.IsWarnAlienFormat())))
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
                case SID_ATTR_WORKINGSET :
                    {
                        bRet = TRUE;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVEWORKINGSET))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_WORKINGSET ), aSaveOptions.IsSaveWorkingSet())))
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
                {
                    if ( ShutdownIcon::IsQuickstarterInstalled() )
                    {
                        if ( rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_QUICKLAUNCHER ),
                                                    ShutdownIcon::GetAutostart() ) ) )
                            bRet = TRUE;
                    }
                    else
                    {
                        rSet.DisableItem( rPool.GetWhich( SID_ATTR_QUICKLAUNCHER ) );
                        bRet = TRUE;
                    }
                    break;
                }
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
                            if ( !rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_BASIC_ENABLED ), sal::static_int_cast< UINT16 >(aSecurityOptions.GetBasicMode()))))
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
                    if( rSet.Put( SfxUInt16Item ( rPool.GetWhich( SID_INET_PROXY_TYPE ),
                        (UINT16)aInetOptions.GetProxyType() )))
                            bRet = TRUE;
                    break;
                }
                case SID_INET_HTTP_PROXY_NAME :
                {
                    if ( rSet.Put( SfxStringItem ( rPool.GetWhich(SID_INET_HTTP_PROXY_NAME ),
                        aInetOptions.GetProxyHttpName() )))
                            bRet = TRUE;
                    break;
                }
                case SID_INET_HTTP_PROXY_PORT :
                    if ( rSet.Put( SfxInt32Item( rPool.GetWhich(SID_INET_HTTP_PROXY_PORT ),
                        aInetOptions.GetProxyHttpPort() )))
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
                case SID_INET_SECURITY_PROXY_NAME :
                case SID_INET_SECURITY_PROXY_PORT :
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                    DBG_ASSERT( sal_False, "SfxApplication::GetOptions()\nSome INET values no longer supported!\n" );
#endif
                    break;
                case SID_INET_NOPROXY :
                    if( rSet.Put( SfxStringItem ( rPool.GetWhich( SID_INET_NOPROXY),
                        aInetOptions.GetProxyNoProxy() )))
                            bRet = TRUE;
                    break;
                case SID_ATTR_PATHNAME :
                case SID_ATTR_PATHGROUP :
                {
                    SfxAllEnumItem aNames(rPool.GetWhich(SID_ATTR_PATHGROUP));
                    SfxAllEnumItem aValues(rPool.GetWhich(SID_ATTR_PATHNAME));
                    SvtPathOptions aPathCfg;
                    for ( USHORT nProp = SvtPathOptions::PATH_ADDIN;
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
// TODO/CLEANUP: wieso zwei SetOptions Methoden?
void SfxApplication::SetOptions_Impl( const SfxItemSet& rSet )
{
    const SfxPoolItem *pItem = 0;
    SfxItemPool &rPool = GetPool();
    BOOL bResetSession = FALSE;

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
        aMiscOptions.SetSymbolsSize(
            sal::static_int_cast< sal_Int16 >(
                bBigSize ? SFX_SYMBOLS_SIZE_LARGE : SFX_SYMBOLS_SIZE_SMALL ) );
        SfxViewFrame* pCurrViewFrame = SfxViewFrame::GetFirst();
        while ( pCurrViewFrame )
        {
            // update all "final" dispatchers
            if ( !pCurrViewFrame->GetActiveChildFrame_Impl() )
                pCurrViewFrame->GetDispatcher()->Update_Impl(sal_True);
            pCurrViewFrame = SfxViewFrame::GetNext(*pCurrViewFrame);
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

    // WarnAlienFormat
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_ATTR_WARNALIENFORMAT ), TRUE, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA( SfxBoolItem ), "BoolItem expected" );
        aSaveOptions.SetWarnAlienFormat( static_cast< const SfxBoolItem*> ( pItem )->GetValue() );
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

    // offende Dokumente merken
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_WORKINGSET), TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetSaveWorkingSet(((const SfxBoolItem *)pItem)->GetValue());
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
                SfxUndoManager *pShUndoMgr = pSh->GetUndoManager();
                if ( pShUndoMgr )
                    pShUndoMgr->SetMaxUndoActionCount( nUndoCount );
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

    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_PROXY_TYPE), TRUE, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxUInt16Item), "UInt16Item expected" );
        aInetOptions.SetProxyType((SvtInetOptions::ProxyType)( (const SfxUInt16Item*)pItem )->GetValue());
        bResetSession = TRUE;
    }

    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_HTTP_PROXY_NAME ), TRUE, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA(SfxStringItem), "StringItem expected" );
        aInetOptions.SetProxyHttpName( ((const SfxStringItem *)pItem)->GetValue() );
        bResetSession = TRUE;
    }
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_HTTP_PROXY_PORT ), TRUE, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA(SfxInt32Item), "Int32Item expected" );
        aInetOptions.SetProxyHttpPort( ( (const SfxInt32Item*)pItem )->GetValue() );
        bResetSession = TRUE;
    }
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_FTP_PROXY_NAME ), TRUE, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA(SfxStringItem), "StringItem expected" );
        aInetOptions.SetProxyFtpName( ((const SfxStringItem *)pItem)->GetValue() );
        bResetSession = TRUE;
    }
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_FTP_PROXY_PORT ), TRUE, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA(SfxInt32Item), "Int32Item expected" );
        aInetOptions.SetProxyFtpPort( ( (const SfxInt32Item*)pItem )->GetValue() );
        bResetSession = TRUE;
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_NOPROXY, TRUE, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        aInetOptions.SetProxyNoProxy(((const SfxStringItem *)pItem)->GetValue());
        bResetSession = TRUE;
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

    // INet Session neu aufsetzen
    if ( bResetSession )
    {
    }

    // geaenderte Daten speichern
    aInetOptions.flush();
}

//--------------------------------------------------------------------
void SfxApplication::SetOptions(const SfxItemSet &rSet)
{
    SvtPathOptions aPathOptions;

    // Daten werden in DocInfo und IniManager gespeichert
    const SfxPoolItem *pItem = 0;
    SfxItemPool &rPool = GetPool();

    SfxAllItemSet aSendSet( rSet );

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

// alle Dokumente speichern

BOOL SfxApplication::SaveAll_Impl(BOOL bPrompt, BOOL bAutoSave)
{
    bAutoSave = FALSE; // functionality moved to new AutoRecovery Service!

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

    return bFunc;
}

//--------------------------------------------------------------------

SfxMacroConfig* SfxApplication::GetMacroConfig() const
{
    return SfxMacroConfig::GetOrCreate();
}

//--------------------------------------------------------------------
SfxEventConfiguration* SfxApplication::GetEventConfig() const
{
    if (!pAppData_Impl->pEventConfig)
        pAppData_Impl->pEventConfig = new SfxEventConfiguration;
    return pAppData_Impl->pEventConfig;
}

//--------------------------------------------------------------------

//--------------------------------------------------------------------
void SfxApplication::NotifyEvent( const SfxEventHint& rEventHint, bool bSynchron )
{

    SfxObjectShell *pDoc = rEventHint.GetObjShell();
    if ( pDoc && ( pDoc->IsPreview() || !pDoc->Get_Impl()->bInitialized ) )
        return;

    if ( bSynchron )
    {
#ifdef DBG_UTIL
        if (!pDoc)
        {
            ByteString aTmp( "SfxEvent: ");
            aTmp += ByteString( String( rEventHint.GetEventName() ), RTL_TEXTENCODING_UTF8 );
            DBG_TRACE( aTmp.GetBuffer() );
        }
#endif
        Broadcast(rEventHint);
        if ( pDoc )
            pDoc->Broadcast( rEventHint );
    }
    else
        new SfxEventAsyncer_Impl( rEventHint );
}

IMPL_OBJHINT( SfxStringHint, String )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
