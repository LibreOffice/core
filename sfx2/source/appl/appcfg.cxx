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

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <osl/file.hxx>

#include <stdlib.h>
#include <vcl/msgbox.hxx>
#include <rtl/ustring.hxx>
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
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/idle.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxhelp.hxx>
#include "sfxtypes.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include "objshimp.hxx"
#include <sfx2/viewsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/evntconf.hxx>
#include "appdata.hxx"
#include "workwin.hxx"
#include "helper.hxx"
#include "app.hrc"
#include <sfx2/sfxresid.hxx>
#include "shutdownicon.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;


class SfxEventAsyncer_Impl : public SfxListener
{
    SfxEventHint           aHint;
    std::unique_ptr<Idle>  pIdle;

public:

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    explicit SfxEventAsyncer_Impl(const SfxEventHint& rHint);
    DECL_LINK( IdleHdl, Timer*, void );
};


void SfxEventAsyncer_Impl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if( rHint.GetId() == SfxHintId::Dying && pIdle->IsActive() )
    {
        pIdle->Stop();
        delete this;
    }
}


SfxEventAsyncer_Impl::SfxEventAsyncer_Impl( const SfxEventHint& rHint )
 : aHint( rHint )
{
    if( rHint.GetObjShell() )
        StartListening( *rHint.GetObjShell() );
    pIdle.reset( new Idle("SfxEventASyncer") );
    pIdle->SetInvokeHandler( LINK(this, SfxEventAsyncer_Impl, IdleHdl) );
    pIdle->SetPriority( TaskPriority::HIGHEST );
    pIdle->SetDebugName( "sfx::SfxEventAsyncer_Impl pIdle" );
    pIdle->Start();
}


IMPL_LINK(SfxEventAsyncer_Impl, IdleHdl, Timer*, pAsyncIdle, void)
{
    SfxObjectShellRef xRef( aHint.GetObjShell() );
    pAsyncIdle->Stop();
    SAL_INFO_IF(!xRef.is(), "sfx.appl", "SfxEvent: " << aHint.GetEventName());
    SfxGetpApp()->Broadcast( aHint );
    if ( xRef.is() )
        xRef->Broadcast( aHint );
    delete this;
}


void SfxApplication::GetOptions( SfxItemSet& rSet )
{
    bool bRet = false;
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
                        bRet = true;
                    break;
                case SID_ATTR_BUTTON_BIGSIZE :
                {
                    if( rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_BUTTON_BIGSIZE ), aMiscOptions.AreCurrentSymbolsLarge() ) ) )
                        bRet = true;
                    break;
                }
                case SID_ATTR_BACKUP :
                    {
                        bRet = true;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::EOption::Backup))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_BACKUP ),aSaveOptions.IsBackup())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_PRETTYPRINTING:
                    {
                        bRet = true;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::EOption::DoPrettyPrinting))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_PRETTYPRINTING ), aSaveOptions.IsPrettyPrinting())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_WARNALIENFORMAT:
                    {
                        bRet = true;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::EOption::WarnAlienFormat))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_WARNALIENFORMAT ), aSaveOptions.IsWarnAlienFormat())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_AUTOSAVE :
                    {
                        bRet = true;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::EOption::AutoSave))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_AUTOSAVE ), aSaveOptions.IsAutoSave())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_AUTOSAVEPROMPT :
                    {
                        bRet = true;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::EOption::AutoSavePrompt))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_AUTOSAVEPROMPT ), aSaveOptions.IsAutoSavePrompt())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_AUTOSAVEMINUTE :
                    {
                        bRet = true;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::EOption::AutoSaveTime))
                            if (!rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_ATTR_AUTOSAVEMINUTE ), (sal_uInt16)aSaveOptions.GetAutoSaveTime())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_USERAUTOSAVE :
                    {
                        bRet = true;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::EOption::UserAutoSave))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_USERAUTOSAVE ), aSaveOptions.IsUserAutoSave())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_DOCINFO :
                    {
                        bRet = true;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::EOption::DocInfSave))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_DOCINFO ), aSaveOptions.IsDocInfoSave())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_WORKINGSET :
                    {
                        bRet = true;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::EOption::SaveWorkingSet))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_WORKINGSET ), aSaveOptions.IsSaveWorkingSet())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_SAVEDOCVIEW :
                    {
                        bRet = true;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::EOption::SaveDocView))
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_SAVEDOCVIEW ), aSaveOptions.IsSaveDocView())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_METRIC :
                    break;
                case SID_HELPBALLOONS :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_HELPBALLOONS ),
                               aHelpOptions.IsExtendedHelp() ) ) )
                        bRet = true;
                    break;
                case SID_HELPTIPS :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_HELPTIPS ),
                               aHelpOptions.IsHelpTips() ) ) )
                        bRet = true;
                    break;
                case SID_ATTR_WELCOMESCREEN :
                    if(rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_ATTR_WELCOMESCREEN ),
                               aHelpOptions.IsWelcomeScreen() ) ) )
                        bRet = true;
                    break;
                case SID_HELP_STYLESHEET :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_HELP_STYLESHEET ),
                               aHelpOptions.GetHelpStyleSheet() ) ) )
                        bRet = true;
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
                            bRet = true;
                    }
                    else
                    {
                        rSet.DisableItem( rPool.GetWhich( SID_ATTR_QUICKLAUNCHER ) );
                        bRet = true;
                    }
                    break;
                }
                case SID_SAVEREL_INET :
                    {
                        bRet = true;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::EOption::SaveRelInet))
                            if (!rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_SAVEREL_INET ), aSaveOptions.IsSaveRelINet() )))
                                bRet = false;
                    }
                    break;
                case SID_SAVEREL_FSYS :
                    {
                        bRet = true;
                        if (!aSaveOptions.IsReadOnly(SvtSaveOptions::EOption::SaveRelFsys))
                            if (!rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_SAVEREL_FSYS ), aSaveOptions.IsSaveRelFSys() )))
                                bRet = false;
                    }
                    break;
                case SID_BASIC_ENABLED :
                    {
                        bRet = true;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::EOption::BasicMode))
                        {
                            if ( !rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_BASIC_ENABLED ), sal::static_int_cast< sal_uInt16 >(aSecurityOptions.GetBasicMode()))))
                                bRet = false;
                        }
                    }
                    break;
                case SID_INET_EXE_PLUGIN  :
                    {
                        bRet = true;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::EOption::ExecutePlugins))
                        {
                            if ( !rSet.Put( SfxBoolItem( SID_INET_EXE_PLUGIN, aSecurityOptions.IsExecutePlugins() ) ) )
                                bRet = false;
                        }
                    }
                    break;
                case SID_MACRO_WARNING :
                    {
                        bRet = true;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::EOption::Warning))
                        {
                            if ( !rSet.Put( SfxBoolItem( SID_MACRO_WARNING, aSecurityOptions.IsWarningEnabled() ) ) )
                                bRet = false;
                        }
                    }
                    break;
                case SID_MACRO_CONFIRMATION :
                    {
                        bRet = true;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::EOption::Confirmation))
                        {
                            if ( !rSet.Put( SfxBoolItem( SID_MACRO_CONFIRMATION, aSecurityOptions.IsConfirmationEnabled() ) ) )
                                bRet = false;
                        }
                    }
                    break;
                case SID_SECURE_URL :
                    {
                        bRet = true;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::EOption::SecureUrls))
                        {
                            css::uno::Sequence< OUString > seqURLs = aSecurityOptions.GetSecureURLs();
                            std::vector<OUString> aList;
                            sal_uInt32 nCount = seqURLs.getLength();
                            for( sal_uInt32 nURL=0; nURL<nCount; ++nURL )
                                aList.push_back(seqURLs[nURL]);

                            if( !rSet.Put( SfxStringListItem( rPool.GetWhich(SID_SECURE_URL), &aList ) ) )
                                bRet = false;
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
                        OUString aValue;
                        switch ( nProp )
                        {
                            case SvtPathOptions::PATH_ADDIN:        osl::FileBase::getFileURLFromSystemPath( aPathCfg.GetAddinPath(), aValue ); break;
                            case SvtPathOptions::PATH_AUTOCORRECT:  aValue = aPathCfg.GetAutoCorrectPath(); break;
                            case SvtPathOptions::PATH_AUTOTEXT:     aValue = aPathCfg.GetAutoTextPath(); break;
                            case SvtPathOptions::PATH_BACKUP:       aValue = aPathCfg.GetBackupPath(); break;
                            case SvtPathOptions::PATH_BASIC:        aValue = aPathCfg.GetBasicPath(); break;
                            case SvtPathOptions::PATH_BITMAP:       aValue = aPathCfg.GetBitmapPath(); break;
                            case SvtPathOptions::PATH_CONFIG:       aValue = aPathCfg.GetConfigPath(); break;
                            case SvtPathOptions::PATH_DICTIONARY:   aValue = aPathCfg.GetDictionaryPath(); break;
                            case SvtPathOptions::PATH_FAVORITES:    aValue = aPathCfg.GetFavoritesPath(); break;
                            case SvtPathOptions::PATH_FILTER:       osl::FileBase::getFileURLFromSystemPath( aPathCfg.GetFilterPath(), aValue ); break;
                            case SvtPathOptions::PATH_GALLERY:      aValue = aPathCfg.GetGalleryPath(); break;
                            case SvtPathOptions::PATH_GRAPHIC:      aValue = aPathCfg.GetGraphicPath(); break;
                            case SvtPathOptions::PATH_HELP:         osl::FileBase::getFileURLFromSystemPath( aPathCfg.GetHelpPath(), aValue ); break;
                            case SvtPathOptions::PATH_LINGUISTIC:   aValue = aPathCfg.GetLinguisticPath(); break;
                            case SvtPathOptions::PATH_MODULE:       osl::FileBase::getFileURLFromSystemPath( aPathCfg.GetModulePath(), aValue ); break;
                            case SvtPathOptions::PATH_PALETTE:      aValue = aPathCfg.GetPalettePath(); break;
                            case SvtPathOptions::PATH_PLUGIN:       osl::FileBase::getFileURLFromSystemPath( aPathCfg.GetPluginPath(), aValue ); break;
                            case SvtPathOptions::PATH_STORAGE:      osl::FileBase::getFileURLFromSystemPath( aPathCfg.GetStoragePath(), aValue ); break;
                            case SvtPathOptions::PATH_TEMP:         aValue = aPathCfg.GetTempPath(); break;
                            case SvtPathOptions::PATH_TEMPLATE:     aValue = aPathCfg.GetTemplatePath(); break;
                            case SvtPathOptions::PATH_USERCONFIG:   aValue = aPathCfg.GetUserConfigPath(); break;
                            case SvtPathOptions::PATH_WORK:         aValue = aPathCfg.GetWorkPath(); break;
                        }
                        aValues.InsertValue( nProp, aValue );
                    }

                    if ( rSet.Put(aNames) || rSet.Put(aValues) )
                        bRet = true;
                }
                break;

                default:
                    SAL_INFO( "sfx.appl", "W1:Wrong ID while getting Options!" );
                    break;
            }
            SAL_WARN_IF(!bRet, "sfx.appl", "Putting options failed!");
        }
        pRanges++;
    }
}

// TODO/CLEANUP: Why two SetOptions Methods?
void SfxApplication::SetOptions_Impl( const SfxItemSet& rSet )
{
    const SfxPoolItem *pItem = nullptr;
    SfxItemPool &rPool = GetPool();

    SvtSaveOptions aSaveOptions;
    SvtHelpOptions aHelpOptions;
    SvtSecurityOptions aSecurityOptions;
    SvtMiscOptions aMiscOptions;
    std::shared_ptr< comphelper::ConfigurationChanges > batch(
        comphelper::ConfigurationChanges::create());
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_BUTTON_OUTSTYLE3D), true, &pItem) )
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        sal_uInt16 nOutStyle =
            static_cast<const SfxBoolItem *>(pItem)->GetValue() ? 0 : TOOLBOX_STYLE_FLAT;
        aMiscOptions.SetToolboxStyle( nOutStyle );
    }

    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_BUTTON_BIGSIZE), true, &pItem) )
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        bool bBigSize = static_cast<const SfxBoolItem*>(pItem)->GetValue();
        aMiscOptions.SetSymbolsSize(
            sal::static_int_cast< sal_Int16 >(
                bBigSize ? SFX_SYMBOLS_SIZE_LARGE : SFX_SYMBOLS_SIZE_SMALL ) );
        SfxViewFrame* pCurrViewFrame = SfxViewFrame::GetFirst();
        while ( pCurrViewFrame )
        {
            // update all "final" dispatchers
            if ( !pCurrViewFrame->GetActiveChildFrame_Impl() )
                pCurrViewFrame->GetDispatcher()->Update_Impl(true);
            pCurrViewFrame = SfxViewFrame::GetNext(*pCurrViewFrame);
        }
    }

    // Backup
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_BACKUP), true, &pItem) )
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        aSaveOptions.SetBackup( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
    }

    // PrettyPrinting
    if ( SfxItemState::SET == rSet.GetItemState( rPool.GetWhich( SID_ATTR_PRETTYPRINTING ), true, &pItem ) )
    {
        DBG_ASSERT( dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected" );
        aSaveOptions.SetPrettyPrinting( static_cast< const SfxBoolItem*> ( pItem )->GetValue() );
    }

    // WarnAlienFormat
    if ( SfxItemState::SET == rSet.GetItemState( rPool.GetWhich( SID_ATTR_WARNALIENFORMAT ), true, &pItem ) )
    {
        DBG_ASSERT( dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected" );
        aSaveOptions.SetWarnAlienFormat( static_cast< const SfxBoolItem*> ( pItem )->GetValue() );
    }

    // AutoSave
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOSAVE), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        aSaveOptions.SetAutoSave( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
    }

    // AutoSave-Prompt
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOSAVEPROMPT), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        aSaveOptions.SetAutoSavePrompt(static_cast<const SfxBoolItem *>(pItem)->GetValue());
    }

    // AutoSave-Time
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_AUTOSAVEMINUTE), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxUInt16Item *>( pItem ) !=  nullptr, "UInt16Item expected");
        aSaveOptions.SetAutoSaveTime(static_cast<const SfxUInt16Item *>(pItem)->GetValue());
    }

    // UserAutoSave
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_USERAUTOSAVE), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        aSaveOptions.SetUserAutoSave( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
    }

    // DocInfo
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_DOCINFO), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        aSaveOptions.SetDocInfoSave(static_cast<const SfxBoolItem *>(pItem)->GetValue());
    }

    // Mark open Documents
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_WORKINGSET), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        aSaveOptions.SetSaveWorkingSet(static_cast<const SfxBoolItem *>(pItem)->GetValue());
    }

    // Save window settings
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_SAVEDOCVIEW), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        aSaveOptions.SetSaveDocView(static_cast<const SfxBoolItem *>(pItem)->GetValue());
    }

    // Metric
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_METRIC), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxUInt16Item *>( pItem ) !=  nullptr, "UInt16Item expected");
    }

    // HelpBalloons
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_HELPBALLOONS), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        aHelpOptions.SetExtendedHelp(static_cast<const SfxBoolItem *>(pItem)->GetValue());
    }

    // HelpTips
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_HELPTIPS), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        aHelpOptions.SetHelpTips( static_cast<const SfxBoolItem *>(pItem)->GetValue());
    }

    // WelcomeScreen
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_WELCOMESCREEN ), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        aHelpOptions.SetWelcomeScreen( static_cast<const SfxBoolItem *>(pItem)->GetValue() );
    }

    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_HELP_STYLESHEET ), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxStringItem *>( pItem ) !=  nullptr, "StringItem expected");
        aHelpOptions.SetHelpStyleSheet( static_cast<const SfxStringItem *>(pItem)->GetValue() );
    }

    // SaveRelINet
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_SAVEREL_INET), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        aSaveOptions.SetSaveRelINet(static_cast<const SfxBoolItem *>(pItem)->GetValue());
    }

    // SaveRelFSys
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_SAVEREL_FSYS), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        aSaveOptions.SetSaveRelFSys(static_cast<const SfxBoolItem *>(pItem)->GetValue());
    }

    // Undo-Count
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_UNDO_COUNT), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxUInt16Item *>( pItem ) !=  nullptr, "UInt16Item expected");
        sal_uInt16 nUndoCount = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
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
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_QUICKLAUNCHER), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        ShutdownIcon::SetAutostart( static_cast<const SfxBoolItem*>( pItem )->GetValue() );
    }

    // StarBasic Enable
    if ( SfxItemState::SET == rSet.GetItemState(SID_BASIC_ENABLED, true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxUInt16Item *>( pItem ) !=  nullptr, "SfxInt16Item expected");
        aSecurityOptions.SetBasicMode( (EBasicSecurityMode)static_cast<const SfxUInt16Item*>( pItem )->GetValue() );
    }

    // Execute PlugIns
    if ( SfxItemState::SET == rSet.GetItemState(SID_INET_EXE_PLUGIN, true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "SfxBoolItem expected");
        aSecurityOptions.SetExecutePlugins( static_cast<const SfxBoolItem *>( pItem )->GetValue() );
    }

    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_INET_PROXY_TYPE), true, &pItem))
    {
        DBG_ASSERT( dynamic_cast< const SfxUInt16Item *>( pItem ) !=  nullptr, "UInt16Item expected" );
        officecfg::Inet::Settings::ooInetProxyType::set(
            static_cast< SfxUInt16Item const * >(pItem)->GetValue(), batch);
    }

    if ( SfxItemState::SET == rSet.GetItemState( rPool.GetWhich( SID_INET_HTTP_PROXY_NAME ), true, &pItem ) )
    {
        DBG_ASSERT( dynamic_cast< const SfxStringItem *>( pItem ) !=  nullptr, "StringItem expected" );
        officecfg::Inet::Settings::ooInetHTTPProxyName::set(
            static_cast< SfxStringItem const * >(pItem)->GetValue(), batch);
    }
    if ( SfxItemState::SET == rSet.GetItemState( rPool.GetWhich( SID_INET_HTTP_PROXY_PORT ), true, &pItem ) )
    {
        DBG_ASSERT( dynamic_cast< const SfxInt32Item *>( pItem ) !=  nullptr, "Int32Item expected" );
        officecfg::Inet::Settings::ooInetHTTPProxyPort::set(
            static_cast< SfxInt32Item const * >(pItem)->GetValue(), batch);
    }
    if ( SfxItemState::SET == rSet.GetItemState( rPool.GetWhich( SID_INET_FTP_PROXY_NAME ), true, &pItem ) )
    {
        DBG_ASSERT( dynamic_cast< const SfxStringItem *>( pItem ) !=  nullptr, "StringItem expected" );
        officecfg::Inet::Settings::ooInetFTPProxyName::set(
            static_cast< SfxStringItem const * >(pItem)->GetValue(), batch);
    }
    if ( SfxItemState::SET == rSet.GetItemState( rPool.GetWhich( SID_INET_FTP_PROXY_PORT ), true, &pItem ) )
    {
        DBG_ASSERT( dynamic_cast< const SfxInt32Item *>( pItem ) !=  nullptr, "Int32Item expected" );
        officecfg::Inet::Settings::ooInetFTPProxyPort::set(
            static_cast< SfxInt32Item const * >(pItem)->GetValue(), batch);
    }
    if ( SfxItemState::SET == rSet.GetItemState(SID_INET_NOPROXY, true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxStringItem *>( pItem ) !=  nullptr, "StringItem expected");
        officecfg::Inet::Settings::ooInetNoProxy::set(
            static_cast< SfxStringItem const * >(pItem)->GetValue(), batch);
    }

    // Secure-Referers
    if ( SfxItemState::SET == rSet.GetItemState(SID_SECURE_URL, true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxStringListItem *>( pItem ) !=  nullptr, "StringListItem expected");
        css::uno::Sequence< OUString > seqURLs;
        static_cast<const SfxStringListItem*>(pItem)->GetStringList(seqURLs);
        aSecurityOptions.SetSecureURLs( seqURLs );
    }

    if ( SfxItemState::SET == rSet.GetItemState(SID_MACRO_WARNING, true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "SfxBoolItem expected");
        aSecurityOptions.SetWarningEnabled( static_cast<const SfxBoolItem *>(pItem)->GetValue() );
    }
    if ( SfxItemState::SET == rSet.GetItemState(SID_MACRO_CONFIRMATION, true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "SfxBoolItem expected");
        aSecurityOptions.SetConfirmationEnabled( static_cast<const SfxBoolItem *>(pItem)->GetValue() );
    }

    // Store changed data
    batch->commit();
}


void SfxApplication::SetOptions(const SfxItemSet &rSet)
{
    SvtPathOptions aPathOptions;

    // Data is saved in DocInfo and IniManager
    const SfxPoolItem *pItem = nullptr;
    SfxItemPool &rPool = GetPool();

    SfxAllItemSet aSendSet( rSet );

    // PathName
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_PATHNAME), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxAllEnumItem *>( pItem ) !=  nullptr, "AllEnumItem expected");
        const SfxAllEnumItem* pEnumItem = static_cast<const SfxAllEnumItem *>(pItem);
        sal_uInt32 nCount = pEnumItem->GetValueCount();
        OUString aNoChangeStr( ' ' );
        for( sal_uInt32 nPath=0; nPath<nCount; ++nPath )
        {
            OUString sValue = pEnumItem->GetValueTextByPos((sal_uInt16)nPath);
            if ( sValue != aNoChangeStr )
            {
                switch( nPath )
                {
                    case SvtPathOptions::PATH_ADDIN:
                    {
                        OUString aTmp;
                        if( osl::FileBase::getSystemPathFromFileURL( sValue, aTmp ) == osl::FileBase::E_None )
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
                        OUString aTmp;
                        if( osl::FileBase::getSystemPathFromFileURL( sValue, aTmp ) == osl::FileBase::E_None )
                            aPathOptions.SetFilterPath( aTmp );
                        break;
                    }
                    case SvtPathOptions::PATH_GALLERY:      aPathOptions.SetGalleryPath( sValue );break;
                    case SvtPathOptions::PATH_GRAPHIC:      aPathOptions.SetGraphicPath( sValue );break;
                    case SvtPathOptions::PATH_HELP:
                    {
                        OUString aTmp;
                        if( osl::FileBase::getSystemPathFromFileURL( sValue, aTmp ) == osl::FileBase::E_None )
                            aPathOptions.SetHelpPath( aTmp );
                        break;
                    }

                    case SvtPathOptions::PATH_LINGUISTIC:   aPathOptions.SetLinguisticPath( sValue );break;
                    case SvtPathOptions::PATH_MODULE:
                    {
                        OUString aTmp;
                        if( osl::FileBase::getSystemPathFromFileURL( sValue, aTmp ) == osl::FileBase::E_None )
                            aPathOptions.SetModulePath( aTmp );
                        break;
                    }

                    case SvtPathOptions::PATH_PALETTE:      aPathOptions.SetPalettePath( sValue );break;
                    case SvtPathOptions::PATH_PLUGIN:
                    {
                        OUString aTmp;
                        if( osl::FileBase::getSystemPathFromFileURL( sValue, aTmp ) == osl::FileBase::E_None )
                            aPathOptions.SetPluginPath( aTmp );
                        break;
                    }

                    case SvtPathOptions::PATH_STORAGE:
                    {
                        OUString aTmp;
                        if( osl::FileBase::getSystemPathFromFileURL( sValue, aTmp ) == osl::FileBase::E_None )
                            aPathOptions.SetStoragePath( aTmp );
                        break;
                    }

                    case SvtPathOptions::PATH_TEMP:         aPathOptions.SetTempPath( sValue );break;
                    case SvtPathOptions::PATH_TEMPLATE:     aPathOptions.SetTemplatePath( sValue );break;
                    case SvtPathOptions::PATH_USERCONFIG:   aPathOptions.SetUserConfigPath( sValue );break;
                    case SvtPathOptions::PATH_WORK:         aPathOptions.SetWorkPath( sValue );break;
                    default: SAL_WARN( "sfx.appl", "SfxApplication::SetOptions_Impl() Invalid path number found for set directories!" );
                }
            }
        }

        aSendSet.ClearItem( rPool.GetWhich( SID_ATTR_PATHNAME ) );
    }

    SetOptions_Impl( rSet );

    // Undo-Count
    Broadcast( SfxItemSetHint( rSet ) );
}


void SfxApplication::NotifyEvent( const SfxEventHint& rEventHint, bool bSynchron )
{
    SfxObjectShell *pDoc = rEventHint.GetObjShell();
    if ( pDoc && ( pDoc->IsPreview() || !pDoc->Get_Impl()->bInitialized ) )
        return;

    if ( bSynchron )
    {
        SAL_INFO_IF(!pDoc, "sfx.appl", "SfxEvent: " << rEventHint.GetEventName());
        Broadcast(rEventHint);
        if ( pDoc )
            pDoc->Broadcast( rEventHint );
    }
    else
        new SfxEventAsyncer_Impl( rEventHint );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
