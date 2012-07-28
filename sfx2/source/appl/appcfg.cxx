/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XFlushable.hpp>

#include <stdlib.h>
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

#include <sfx2/sfxsids.hrc>
#include <sot/exchange.hxx>

#include <svl/isethint.hxx>

#include <officecfg/Inet.hxx>
#include <officecfg/Office/Common.hxx>
#include <unotools/configmgr.hxx>
#include <tools/urlobj.hxx>
#include <unotools/saveopt.hxx>
#include <svtools/helpopt.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/miscopt.hxx>
#include <vcl/toolbox.hxx>
#include <unotools/localfilehelper.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include "sfx2/sfxhelp.hxx"
#include "sfxtypes.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include "objshimp.hxx"
#include <sfx2/viewsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/evntconf.hxx>
#include "appdata.hxx"
#include "workwin.hxx"
#include "helper.hxx"   // SfxContentHelper::...
#include "app.hrc"
#include "sfx2/sfxresid.hxx"
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
        rtl::OStringBuffer aTmp(RTL_CONSTASCII_STRINGPARAM("SfxEvent: "));
        aTmp.append(rtl::OUStringToOString(aHint.GetEventName(), RTL_TEXTENCODING_UTF8));
        OSL_TRACE( "%s", aTmp.getStr() );
    }
#endif
    SFX_APP()->Broadcast( aHint );
    if ( xRef.Is() )
        xRef->Broadcast( aHint );
    delete this;
    return 0L;
}


//--------------------------------------------------------------------

sal_Bool SfxApplication::GetOptions( SfxItemSet& rSet )
{
    sal_Bool bRet = sal_False;
    SfxItemPool &rPool = GetPool();

    const sal_uInt16 *pRanges = rSet.GetRanges();
    SvtSaveOptions aSaveOptions;
    SvtHelpOptions aHelpOptions;
    SvtSecurityOptions  aSecurityOptions;
    SvtMiscOptions aMiscOptions;

    while ( *pRanges )
    {
        for(sal_uInt16 nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich)
        {
            switch(nWhich)
            {
                case SID_ATTR_BUTTON_OUTSTYLE3D :
                    if(rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_BUTTON_OUTSTYLE3D ),
                              aMiscOptions.GetToolboxStyle() != TOOLBOX_STYLE_FLAT)))
                        bRet = sal_True;
                    break;
                case SID_ATTR_BUTTON_BIGSIZE :
                {
                    if( rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_BUTTON_BIGSIZE ), aMiscOptions.AreCurrentSymbolsLarge() ) ) )
                        bRet = sal_True;
                    break;
                }
                case SID_ATTR_BACKUP :
                    {
                        bRet = sal_True;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_BACKUP))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_BACKUP ),aSaveOptions.IsBackup())))
                                bRet = sal_False;
                    }
                    break;
                case SID_ATTR_PRETTYPRINTING:
                    {
                        bRet = sal_True;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_DOPRETTYPRINTING))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_PRETTYPRINTING ), aSaveOptions.IsPrettyPrinting())))
                                bRet = sal_False;
                    }
                    break;
                case SID_ATTR_WARNALIENFORMAT:
                    {
                        bRet = sal_True;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_WARNALIENFORMAT))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_WARNALIENFORMAT ), aSaveOptions.IsWarnAlienFormat())))
                                bRet = sal_False;
                    }
                    break;
                case SID_ATTR_AUTOSAVE :
                    {
                        bRet = sal_True;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_AUTOSAVE))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_AUTOSAVE ), aSaveOptions.IsAutoSave())))
                                bRet = sal_False;
                    }
                    break;
                case SID_ATTR_AUTOSAVEPROMPT :
                    {
                        bRet = sal_True;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_AUTOSAVEPROMPT))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_AUTOSAVEPROMPT ), aSaveOptions.IsAutoSavePrompt())))
                                bRet = sal_False;
                    }
                    break;
                case SID_ATTR_AUTOSAVEMINUTE :
                    {
                        bRet = sal_True;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_AUTOSAVETIME))
                            if (!rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_ATTR_AUTOSAVEMINUTE ), (sal_uInt16)aSaveOptions.GetAutoSaveTime())))
                                bRet = sal_False;
                    }
                    break;
                case SID_ATTR_DOCINFO :
                    {
                        bRet = sal_True;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_DOCINFSAVE))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_DOCINFO ), aSaveOptions.IsDocInfoSave())))
                                bRet = sal_False;
                    }
                    break;
                case SID_ATTR_WORKINGSET :
                    {
                        bRet = sal_True;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVEWORKINGSET))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_WORKINGSET ), aSaveOptions.IsSaveWorkingSet())))
                                bRet = sal_False;
                    }
                    break;
                case SID_ATTR_SAVEDOCVIEW :
                    {
                        bRet = sal_True;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVEDOCVIEW))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_SAVEDOCVIEW ), aSaveOptions.IsSaveDocView())))
                                bRet = sal_False;
                    }
                    break;
                case SID_ATTR_METRIC :
                    break;
                case SID_HELPBALLOONS :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_HELPBALLOONS ),
                               aHelpOptions.IsExtendedHelp() ) ) )
                        bRet = sal_True;
                    break;
                case SID_HELPTIPS :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_HELPTIPS ),
                               aHelpOptions.IsHelpTips() ) ) )
                        bRet = sal_True;
                    break;
                case SID_ATTR_AUTOHELPAGENT :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_ATTR_AUTOHELPAGENT ),
                               aHelpOptions.IsHelpAgentAutoStartMode() ) ) )
                        bRet = sal_True;
                    break;
                case SID_HELPAGENT_TIMEOUT :
                    if ( rSet.Put( SfxInt32Item( rPool.GetWhich( SID_HELPAGENT_TIMEOUT ),
                                                 aHelpOptions.GetHelpAgentTimeoutPeriod() ) ) )
                        bRet = sal_True;
                    break;
                case SID_ATTR_WELCOMESCREEN :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_ATTR_WELCOMESCREEN ),
                               aHelpOptions.IsWelcomeScreen() ) ) )
                        bRet = sal_True;
                    break;
                case SID_HELP_STYLESHEET :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_HELP_STYLESHEET ),
                               aHelpOptions.GetHelpStyleSheet() ) ) )
                        bRet = sal_True;
                break;
                case SID_ATTR_UNDO_COUNT :
                    if (rSet.Put(
                            SfxUInt16Item (
                                rPool.GetWhich(SID_ATTR_UNDO_COUNT),
                                officecfg::Office::Common::Undo::Steps::get())))
                    {
                        bRet = true;
                    }
                    break;
                case SID_ATTR_QUICKLAUNCHER :
                {
                    if ( ShutdownIcon::IsQuickstarterInstalled() )
                    {
                        if ( rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_QUICKLAUNCHER ),
                                                    ShutdownIcon::GetAutostart() ) ) )
                            bRet = sal_True;
                    }
                    else
                    {
                        rSet.DisableItem( rPool.GetWhich( SID_ATTR_QUICKLAUNCHER ) );
                        bRet = sal_True;
                    }
                    break;
                }
                case SID_SAVEREL_INET :
                    {
                        bRet = sal_True;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVERELINET))
                            if (!rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_SAVEREL_INET ), aSaveOptions.IsSaveRelINet() )))
                                bRet = sal_False;
                    }
                    break;
                case SID_SAVEREL_FSYS :
                    {
                        bRet = sal_True;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::E_SAVERELFSYS))
                            if (!rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_SAVEREL_FSYS ), aSaveOptions.IsSaveRelFSys() )))
                                bRet = sal_False;
                    }
                    break;
                case SID_BASIC_ENABLED :
                    {
                        bRet = sal_True;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::E_BASICMODE))
                        {
                            if ( !rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_BASIC_ENABLED ), sal::static_int_cast< sal_uInt16 >(aSecurityOptions.GetBasicMode()))))
                                bRet = sal_False;
                        }
                    }
                    break;
                case SID_INET_EXE_PLUGIN  :
                    {
                        bRet = sal_True;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::E_EXECUTEPLUGINS))
                        {
                            if ( !rSet.Put( SfxBoolItem( SID_INET_EXE_PLUGIN, aSecurityOptions.IsExecutePlugins() ) ) )
                                bRet = sal_False;
                        }
                    }
                    break;
                case SID_MACRO_WARNING :
                    {
                        bRet = sal_True;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::E_WARNING))
                        {
                            if ( !rSet.Put( SfxBoolItem( SID_MACRO_WARNING, aSecurityOptions.IsWarningEnabled() ) ) )
                                bRet = sal_False;
                        }
                    }
                    break;
                case SID_MACRO_CONFIRMATION :
                    {
                        bRet = sal_True;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::E_CONFIRMATION))
                        {
                            if ( !rSet.Put( SfxBoolItem( SID_MACRO_CONFIRMATION, aSecurityOptions.IsConfirmationEnabled() ) ) )
                                bRet = sal_False;
                        }
                    }
                    break;
                case SID_SECURE_URL :
                    {
                        bRet = sal_True;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::E_SECUREURLS))
                        {
                            ::com::sun::star::uno::Sequence< ::rtl::OUString > seqURLs = aSecurityOptions.GetSecureURLs();
                            std::vector<String> aList;
                            sal_uInt32 nCount = seqURLs.getLength();
                            sal_uInt32 nURL;
                            for( nURL=0; nURL<nCount; ++nURL )
                                aList.push_back(seqURLs[nURL]);

                            if( !rSet.Put( SfxStringListItem( rPool.GetWhich(SID_SECURE_URL), &aList ) ) )
                                bRet = sal_False;
                        }
                    }
                    break;
                case SID_INET_PROXY_TYPE :
                    if (rSet.Put(
                            SfxUInt16Item(
                                rPool.GetWhich(SID_INET_PROXY_TYPE),
                                (officecfg::Inet::Settings::ooInetProxyType::
                                 get().get_value_or(0)))))
                    {
                        bRet = true;
                    }
                    break;
                case SID_INET_HTTP_PROXY_NAME :
                    if (rSet.Put(
                            SfxStringItem(
                                rPool.GetWhich(SID_INET_HTTP_PROXY_NAME),
                                officecfg::Inet::Settings::ooInetHTTPProxyName::
                                get())))
                    {
                        bRet = true;
                    }
                    break;
                case SID_INET_HTTP_PROXY_PORT :
                    if (rSet.Put(
                            SfxInt32Item(
                                rPool.GetWhich(SID_INET_HTTP_PROXY_PORT),
                                (officecfg::Inet::Settings::
                                 ooInetHTTPProxyPort::get().get_value_or(0)))))
                    {
                        bRet = true;
                    }
                    break;
                case SID_INET_FTP_PROXY_NAME :
                    if (rSet.Put(
                            SfxStringItem(
                                rPool.GetWhich(SID_INET_FTP_PROXY_NAME),
                                officecfg::Inet::Settings::ooInetFTPProxyName::
                                get())))
                    {
                        bRet = true;
                    }
                    break;
                case SID_INET_FTP_PROXY_PORT :
                    if (rSet.Put(
                            SfxInt32Item(
                                rPool.GetWhich(SID_INET_FTP_PROXY_PORT),
                                (officecfg::Inet::Settings::ooInetFTPProxyPort::
                                 get().get_value_or(0)))))
                    {
                        bRet = true;
                    }
                    break;
                case SID_INET_NOPROXY :
                    if (rSet.Put(
                            SfxStringItem(
                                rPool.GetWhich( SID_INET_NOPROXY),
                                (officecfg::Inet::Settings::ooInetNoProxy::
                                 get()))))
                    {
                        bRet = true;
                    }
                    break;
                case SID_ATTR_PATHNAME :
                case SID_ATTR_PATHGROUP :
                {
                    SfxAllEnumItem aNames(rPool.GetWhich(SID_ATTR_PATHGROUP));
                    SfxAllEnumItem aValues(rPool.GetWhich(SID_ATTR_PATHNAME));
                    SvtPathOptions aPathCfg;
                    for ( sal_uInt16 nProp = SvtPathOptions::PATH_ADDIN;
                          nProp <= SvtPathOptions::PATH_WORK; nProp++ )
                    {
                        aNames.InsertValue( nProp, SfxResId(CONFIG_PATH_START + nProp).toString() );
                        rtl::OUString aValue;
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
                        bRet = sal_True;
                }

                default:
                    DBG_WARNING( "W1:Wrong ID while getting Options!" );
                    break;
            }
#ifdef DBG_UTIL
            if ( !bRet )
                OSL_FAIL( "Putting options failed!" );
#endif
        }
        pRanges++;
    }

    return bRet;
}

//--------------------------------------------------------------------
sal_Bool SfxApplication::IsSecureURL( const INetURLObject& rURL, const String* pReferer ) const
{
    return SvtSecurityOptions().IsSecureURL( rURL.GetMainURL( INetURLObject::NO_DECODE ), *pReferer );
}
//--------------------------------------------------------------------
// TODO/CLEANUP: Why two SetOptions Methods?
void SfxApplication::SetOptions_Impl( const SfxItemSet& rSet )
{
    const SfxPoolItem *pItem = 0;
    SfxItemPool &rPool = GetPool();

    SvtSaveOptions aSaveOptions;
    SvtHelpOptions aHelpOptions;
    SvtSecurityOptions aSecurityOptions;
    SvtPathOptions aPathOptions;
    SvtMiscOptions aMiscOptions;
    boost::shared_ptr< comphelper::ConfigurationChanges > batch(
        comphelper::ConfigurationChanges::create());
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_BUTTON_OUTSTYLE3D), sal_True, &pItem) )
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        sal_uInt16 nOutStyle =
            ( (const SfxBoolItem *)pItem)->GetValue() ? 0 : TOOLBOX_STYLE_FLAT;
        aMiscOptions.SetToolboxStyle( nOutStyle );
    }

    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_BUTTON_BIGSIZE), sal_True, &pItem) )
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        sal_Bool bBigSize = ( (const SfxBoolItem*)pItem )->GetValue();
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
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_BACKUP), sal_True, &pItem) )
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetBackup( ( (const SfxBoolItem*)pItem )->GetValue() );
    }

    // PrettyPrinting
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_ATTR_PRETTYPRINTING ), sal_True, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA( SfxBoolItem ), "BoolItem expected" );
        aSaveOptions.SetPrettyPrinting( static_cast< const SfxBoolItem*> ( pItem )->GetValue() );
    }

    // WarnAlienFormat
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_ATTR_WARNALIENFORMAT ), sal_True, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA( SfxBoolItem ), "BoolItem expected" );
        aSaveOptions.SetWarnAlienFormat( static_cast< const SfxBoolItem*> ( pItem )->GetValue() );
    }

    // AutoSave
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOSAVE), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetAutoSave( ( (const SfxBoolItem*)pItem )->GetValue() );
    }

    // AutoSave-Propt
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOSAVEPROMPT), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetAutoSavePrompt(((const SfxBoolItem *)pItem)->GetValue());
    }

    // AutoSave-Time
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOSAVEMINUTE), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "UInt16Item expected");
        aSaveOptions.SetAutoSaveTime(((const SfxUInt16Item *)pItem)->GetValue());
    }

    // DocInfo
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_DOCINFO), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetDocInfoSave(((const SfxBoolItem *)pItem)->GetValue());
    }

    // Mark open Documents
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_WORKINGSET), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetSaveWorkingSet(((const SfxBoolItem *)pItem)->GetValue());
    }

    // Save window settings
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_SAVEDOCVIEW), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetSaveDocView(((const SfxBoolItem *)pItem)->GetValue());
    }

    // Metric
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_METRIC), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "UInt16Item expected");
    }

    // HelpBalloons
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_HELPBALLOONS), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aHelpOptions.SetExtendedHelp(((const SfxBoolItem *)pItem)->GetValue());
    }

    // HelpTips
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_HELPTIPS), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aHelpOptions.SetHelpTips(((const SfxBoolItem *)pItem)->GetValue());
    }

    // AutoHelpAgent
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOHELPAGENT ), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aHelpOptions.SetHelpAgentAutoStartMode( ((const SfxBoolItem *)pItem)->GetValue() );
    }

    // help agent timeout
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_HELPAGENT_TIMEOUT ), sal_True, &pItem ) )
    {
        DBG_ASSERT(pItem->ISA(SfxInt32Item), "Int32Item expected");
        aHelpOptions.SetHelpAgentTimeoutPeriod( ( (const SfxInt32Item*)pItem )->GetValue() );
    }

    // WelcomeScreen
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_WELCOMESCREEN ), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aHelpOptions.SetWelcomeScreen( ((const SfxBoolItem *)pItem)->GetValue() );
    }

    // WelcomeScreen
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_WELCOMESCREEN_RESET ), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        sal_Bool bReset = ((const SfxBoolItem *)pItem)->GetValue();
        if ( bReset )
        {
            OSL_FAIL( "Not implemented, may be EOL!" );
        }                                                   }

    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_HELP_STYLESHEET ), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        aHelpOptions.SetHelpStyleSheet( ((const SfxStringItem *)pItem)->GetValue() );
    }

    // SaveRelINet
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_SAVEREL_INET), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetSaveRelINet(((const SfxBoolItem *)pItem)->GetValue());
    }

    // SaveRelFSys
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_SAVEREL_FSYS), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        aSaveOptions.SetSaveRelFSys(((const SfxBoolItem *)pItem)->GetValue());
    }

    // Undo-Count
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_UNDO_COUNT), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "UInt16Item expected");
        sal_uInt16 nUndoCount = ((const SfxUInt16Item*)pItem)->GetValue();
        officecfg::Office::Common::Undo::Steps::set(nUndoCount, batch);

        // To catch all Undo-Managers: Iterate over all Frames
        for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst();
              pFrame;
              pFrame = SfxViewFrame::GetNext(*pFrame) )
        {
            // Get the Dispatcher of the Frames
            SfxDispatcher *pDispat = pFrame->GetDispatcher();
            pDispat->Flush();

            // Iterate over all SfxShells on the Dispatchers Stack
            sal_uInt16 nIdx = 0;
            for ( SfxShell *pSh = pDispat->GetShell(nIdx);
                  pSh;
                  ++nIdx, pSh = pDispat->GetShell(nIdx) )
            {
                ::svl::IUndoManager *pShUndoMgr = pSh->GetUndoManager();
                if ( pShUndoMgr )
                    pShUndoMgr->SetMaxUndoActionCount( nUndoCount );
            }
        }
    }

    // Office autostart
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_QUICKLAUNCHER), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "BoolItem expected");
        ShutdownIcon::SetAutostart( ( (const SfxBoolItem*)pItem )->GetValue() != sal_False );
    }

    // StarBasic Enable
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_BASIC_ENABLED, sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxUInt16Item), "SfxInt16Item expected");
        aSecurityOptions.SetBasicMode( (EBasicSecurityMode)( (const SfxUInt16Item*)pItem )->GetValue() );
    }

    // Execute PlugIns
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_EXE_PLUGIN, sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        aSecurityOptions.SetExecutePlugins( ( (const SfxBoolItem *)pItem )->GetValue() );
    }

    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_INET_PROXY_TYPE), sal_True, &pItem))
    {
        DBG_ASSERT( pItem->ISA(SfxUInt16Item), "UInt16Item expected" );
        officecfg::Inet::Settings::ooInetProxyType::set(
            static_cast< SfxUInt16Item const * >(pItem)->GetValue(), batch);
    }

    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_HTTP_PROXY_NAME ), sal_True, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA(SfxStringItem), "StringItem expected" );
        officecfg::Inet::Settings::ooInetHTTPProxyName::set(
            static_cast< SfxStringItem const * >(pItem)->GetValue(), batch);
    }
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_HTTP_PROXY_PORT ), sal_True, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA(SfxInt32Item), "Int32Item expected" );
        officecfg::Inet::Settings::ooInetHTTPProxyPort::set(
            static_cast< SfxInt32Item const * >(pItem)->GetValue(), batch);
    }
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_FTP_PROXY_NAME ), sal_True, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA(SfxStringItem), "StringItem expected" );
        officecfg::Inet::Settings::ooInetFTPProxyName::set(
            static_cast< SfxStringItem const * >(pItem)->GetValue(), batch);
    }
    if ( SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich( SID_INET_FTP_PROXY_PORT ), sal_True, &pItem ) )
    {
        DBG_ASSERT( pItem->ISA(SfxInt32Item), "Int32Item expected" );
        officecfg::Inet::Settings::ooInetFTPProxyPort::set(
            static_cast< SfxInt32Item const * >(pItem)->GetValue(), batch);
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_INET_NOPROXY, sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringItem), "StringItem expected");
        officecfg::Inet::Settings::ooInetNoProxy::set(
            static_cast< SfxStringItem const * >(pItem)->GetValue(), batch);
    }

    // Secure-Referers
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_SECURE_URL, sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxStringListItem), "StringListItem expected");
        const std::vector<String> &aList = ((SfxStringListItem*)pItem)->GetList();
        sal_uInt32 nCount = aList.size();
        ::com::sun::star::uno::Sequence< ::rtl::OUString > seqURLs(nCount);
        for( sal_uInt32 nPosition=0;nPosition<nCount;++nPosition)
            seqURLs[nPosition] = aList[nPosition];

        aSecurityOptions.SetSecureURLs( seqURLs );
    }

    if ( SFX_ITEM_SET == rSet.GetItemState(SID_MACRO_WARNING, sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        aSecurityOptions.SetWarningEnabled( ( (const SfxBoolItem *)pItem )->GetValue() );
    }
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_MACRO_CONFIRMATION, sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxBoolItem), "SfxBoolItem expected");
        aSecurityOptions.SetConfirmationEnabled( ( (const SfxBoolItem *)pItem )->GetValue() );
    }

    // Store changed data
    batch->commit();
}

//--------------------------------------------------------------------
void SfxApplication::SetOptions(const SfxItemSet &rSet)
{
    SvtPathOptions aPathOptions;

    // Data is saved in DocInfo and IniManager
    const SfxPoolItem *pItem = 0;
    SfxItemPool &rPool = GetPool();

    SfxAllItemSet aSendSet( rSet );

    // PathName
    if ( SFX_ITEM_SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_PATHNAME), sal_True, &pItem))
    {
        DBG_ASSERT(pItem->ISA(SfxAllEnumItem), "AllEnumItem expected");
        const SfxAllEnumItem* pEnumItem = (const SfxAllEnumItem *)pItem;
        sal_uInt32 nCount = pEnumItem->GetValueCount();
        rtl::OUString aNoChangeStr( ' ' );
        for( sal_uInt32 nPath=0; nPath<nCount; ++nPath )
        {
            rtl::OUString sValue = pEnumItem->GetValueTextByPos((sal_uInt16)nPath);
            if ( sValue != aNoChangeStr )
            {
                switch( nPath )
                {
                    case SvtPathOptions::PATH_ADDIN:
                    {
                        rtl::OUString aTmp;
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
                        rtl::OUString aTmp;
                        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( sValue, aTmp ) )
                            aPathOptions.SetFilterPath( aTmp );
                        break;
                    }
                    case SvtPathOptions::PATH_GALLERY:      aPathOptions.SetGalleryPath( sValue );break;
                    case SvtPathOptions::PATH_GRAPHIC:      aPathOptions.SetGraphicPath( sValue );break;
                    case SvtPathOptions::PATH_HELP:
                    {
                        rtl::OUString aTmp;
                        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( sValue, aTmp ) )
                            aPathOptions.SetHelpPath( aTmp );
                        break;
                    }

                    case SvtPathOptions::PATH_LINGUISTIC:   aPathOptions.SetLinguisticPath( sValue );break;
                    case SvtPathOptions::PATH_MODULE:
                    {
                        rtl::OUString aTmp;
                        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( sValue, aTmp ) )
                            aPathOptions.SetModulePath( aTmp );
                        break;
                    }

                    case SvtPathOptions::PATH_PALETTE:      aPathOptions.SetPalettePath( sValue );break;
                    case SvtPathOptions::PATH_PLUGIN:
                    {
                        rtl::OUString aTmp;
                        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( sValue, aTmp ) )
                            aPathOptions.SetPluginPath( aTmp );
                        break;
                    }

                    case SvtPathOptions::PATH_STORAGE:
                    {
                        rtl::OUString aTmp;
                        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( sValue, aTmp ) )
                            aPathOptions.SetStoragePath( aTmp );
                        break;
                    }

                    case SvtPathOptions::PATH_TEMP:         aPathOptions.SetTempPath( sValue );break;
                    case SvtPathOptions::PATH_TEMPLATE:     aPathOptions.SetTemplatePath( sValue );break;
                    case SvtPathOptions::PATH_USERCONFIG:   aPathOptions.SetUserConfigPath( sValue );break;
                    case SvtPathOptions::PATH_WORK:         aPathOptions.SetWorkPath( sValue );break;
                    default: SAL_WARN( "sfx2.appl", "SfxApplication::SetOptions_Impl() Invalid path number found for set directories!" );
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
            rtl::OStringBuffer aTmp(RTL_CONSTASCII_STRINGPARAM("SfxEvent: "));
            aTmp.append(rtl::OUStringToOString(rEventHint.GetEventName(), RTL_TEXTENCODING_UTF8));
            OSL_TRACE( "%s", aTmp.getStr() );
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
