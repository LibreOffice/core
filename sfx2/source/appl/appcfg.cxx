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

#include <memory>
#include <comphelper/sequence.hxx>
#include <osl/file.hxx>

#include <rtl/ustring.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/aeitem.hxx>
#include <svl/slstitm.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/undo.hxx>

#include <sfx2/sfxsids.hrc>

#include <svl/isethint.hxx>

#include <officecfg/Inet.hxx>
#include <officecfg/Office/Common.hxx>
#include <unotools/saveopt.hxx>
#include <svtools/helpopt.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/imgdef.hxx>
#include <sal/log.hxx>
#include <vcl/idle.hxx>

#include <sfx2/app.hxx>
#include <sfx2/event.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <objshimp.hxx>
#include "shutdownicon.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;

namespace {

class SfxEventAsyncer_Impl : public SfxListener
{
    SfxEventHint           aHint;
    std::unique_ptr<Idle>  pIdle;

public:

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    explicit SfxEventAsyncer_Impl(const SfxEventHint& rHint);
    DECL_LINK( IdleHdl, Timer*, void );
};

}

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
    pIdle->SetPriority( TaskPriority::HIGH_IDLE );
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

    const WhichRangesContainer& pRanges = rSet.GetRanges();
    SvtSaveOptions aSaveOptions;
    SvtHelpOptions aHelpOptions;
    SvtSecurityOptions  aSecurityOptions;
    SvtMiscOptions aMiscOptions;

    for (auto const & pRange : pRanges)
    {
        for(sal_uInt16 nWhich = pRange.first; nWhich <= pRange.second; ++nWhich)
        {
            switch(nWhich)
            {
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
                        if (!officecfg::Office::Common::Save::Document::PrettyPrinting::isReadOnly())
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_PRETTYPRINTING ),
                                    officecfg::Office::Common::Save::Document::PrettyPrinting::get())))
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
                            if (!rSet.Put( SfxUInt16Item( rPool.GetWhich( SID_ATTR_AUTOSAVEMINUTE ), static_cast<sal_uInt16>(aSaveOptions.GetAutoSaveTime()))))
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
                        if (!officecfg::Office::Common::Save::WorkingSet::isReadOnly())
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_WORKINGSET ),
                                    officecfg::Office::Common::Save::WorkingSet::get())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_SAVEDOCVIEW :
                    {
                        bRet = true;
                        if (!officecfg::Office::Common::Save::Document::ViewInfo::isReadOnly())
                            if (!rSet.Put( SfxBoolItem( rPool.GetWhich( SID_ATTR_SAVEDOCVIEW ), officecfg::Office::Common::Save::Document::ViewInfo::get())))
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
                case SID_HELP_STYLESHEET :
                    if(rSet.Put( SfxStringItem ( rPool.GetWhich( SID_HELP_STYLESHEET ),
                               officecfg::Office::Common::Help::HelpStyleSheet::get() ) ) )
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
                        if (!officecfg::Office::Common::Save::URL::Internet::isReadOnly())
                            if (!rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_SAVEREL_INET ),
                                            officecfg::Office::Common::Save::URL::Internet::get() )))
                                bRet = false;
                    }
                    break;
                case SID_SAVEREL_FSYS :
                    {
                        bRet = true;
                        if (!officecfg::Office::Common::Save::URL::FileSystem::isReadOnly())
                            if (!rSet.Put( SfxBoolItem ( rPool.GetWhich( SID_SAVEREL_FSYS ),
                                officecfg::Office::Common::Save::URL::FileSystem::get() )))
                                bRet = false;
                    }
                    break;
                case SID_SECURE_URL :
                    {
                        bRet = true;
                        if (!aSecurityOptions.IsReadOnly(SvtSecurityOptions::EOption::SecureUrls))
                        {
                            css::uno::Sequence< OUString > seqURLs = aSecurityOptions.GetSecureURLs();
                            auto aList = comphelper::sequenceToContainer<std::vector<OUString>>(seqURLs);

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
                                 get().value_or(0)))))
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
                                 ooInetHTTPProxyPort::get().value_or(0)))))
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
                                 get().value_or(0)))))
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
                {
                    SfxAllEnumItem aValues(rPool.GetWhich(SID_ATTR_PATHNAME));
                    SvtPathOptions aPathCfg;
                    for ( sal_uInt16 nProp = static_cast<sal_uInt16>(SvtPathOptions::Paths::AddIn);
                          nProp <= static_cast<sal_uInt16>(SvtPathOptions::Paths::Work); nProp++ )
                    {
                        OUString aValue;
                        switch ( static_cast<SvtPathOptions::Paths>(nProp) )
                        {
                            case SvtPathOptions::Paths::AddIn:        osl::FileBase::getFileURLFromSystemPath( aPathCfg.GetAddinPath(), aValue ); break;
                            case SvtPathOptions::Paths::AutoCorrect:  aValue = aPathCfg.GetAutoCorrectPath(); break;
                            case SvtPathOptions::Paths::AutoText:     aValue = aPathCfg.GetAutoTextPath(); break;
                            case SvtPathOptions::Paths::Backup:       aValue = aPathCfg.GetBackupPath(); break;
                            case SvtPathOptions::Paths::Basic:        aValue = aPathCfg.GetBasicPath(); break;
                            case SvtPathOptions::Paths::Bitmap:       aValue = aPathCfg.GetBitmapPath(); break;
                            case SvtPathOptions::Paths::Config:       aValue = aPathCfg.GetConfigPath(); break;
                            case SvtPathOptions::Paths::Dictionary:   aValue = aPathCfg.GetDictionaryPath(); break;
                            case SvtPathOptions::Paths::Favorites:    aValue = aPathCfg.GetFavoritesPath(); break;
                            case SvtPathOptions::Paths::Filter:       osl::FileBase::getFileURLFromSystemPath( aPathCfg.GetFilterPath(), aValue ); break;
                            case SvtPathOptions::Paths::Gallery:      aValue = aPathCfg.GetGalleryPath(); break;
                            case SvtPathOptions::Paths::Graphic:      aValue = aPathCfg.GetGraphicPath(); break;
                            case SvtPathOptions::Paths::Help:         osl::FileBase::getFileURLFromSystemPath( aPathCfg.GetHelpPath(), aValue ); break;
                            case SvtPathOptions::Paths::Linguistic:   aValue = aPathCfg.GetLinguisticPath(); break;
                            case SvtPathOptions::Paths::Module:       osl::FileBase::getFileURLFromSystemPath( aPathCfg.GetModulePath(), aValue ); break;
                            case SvtPathOptions::Paths::Palette:      aValue = aPathCfg.GetPalettePath(); break;
                            case SvtPathOptions::Paths::Plugin:       osl::FileBase::getFileURLFromSystemPath( aPathCfg.GetPluginPath(), aValue ); break;
                            case SvtPathOptions::Paths::Storage:      osl::FileBase::getFileURLFromSystemPath( aPathCfg.GetStoragePath(), aValue ); break;
                            case SvtPathOptions::Paths::Temp:         aValue = aPathCfg.GetTempPath(); break;
                            case SvtPathOptions::Paths::Template:     aValue = aPathCfg.GetTemplatePath(); break;
                            case SvtPathOptions::Paths::UserConfig:   aValue = aPathCfg.GetUserConfigPath(); break;
                            case SvtPathOptions::Paths::Work:         aValue = aPathCfg.GetWorkPath(); break;
                            default: break;
                        }
                        aValues.SetTextByPos( nProp, aValue );
                    }

                    if (rSet.Put(aValues))
                        bRet = true;
                }
                break;

                default:
                    SAL_INFO( "sfx.appl", "W1:Wrong ID while getting Options!" );
                    break;
            }
            SAL_WARN_IF(!bRet, "sfx.appl", "Putting options failed!");
        }
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
        officecfg::Office::Common::Save::Document::PrettyPrinting::set(
            static_cast< const SfxBoolItem*> ( pItem )->GetValue(),
            batch );
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
        officecfg::Office::Common::Save::WorkingSet::set(
            static_cast<const SfxBoolItem *>(pItem)->GetValue(),
            batch);
    }

    // Save window settings
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_SAVEDOCVIEW), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        officecfg::Office::Common::Save::Document::ViewInfo::set(static_cast<const SfxBoolItem *>(pItem)->GetValue(), batch);
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

    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_HELP_STYLESHEET ), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxStringItem *>( pItem ) !=  nullptr, "StringItem expected");
        OUString sStyleSheet = static_cast<const SfxStringItem *>(pItem)->GetValue();
        officecfg::Office::Common::Help::HelpStyleSheet::set(sStyleSheet, batch);
    }

    // SaveRelINet
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_SAVEREL_INET), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        officecfg::Office::Common::Save::URL::Internet::set(
            static_cast<const SfxBoolItem *>(pItem)->GetValue(),
            batch);
    }

    // SaveRelFSys
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_SAVEREL_FSYS), true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr, "BoolItem expected");
        officecfg::Office::Common::Save::URL::FileSystem::set(
            static_cast<const SfxBoolItem *>(pItem)->GetValue(),
            batch);
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
                SfxUndoManager *pShUndoMgr = pSh->GetUndoManager();
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

    // Secure-Referrer
    if ( SfxItemState::SET == rSet.GetItemState(SID_SECURE_URL, true, &pItem))
    {
        DBG_ASSERT(dynamic_cast< const SfxStringListItem *>( pItem ) !=  nullptr, "StringListItem expected");
        css::uno::Sequence< OUString > seqURLs;
        static_cast<const SfxStringListItem*>(pItem)->GetStringList(seqURLs);
        aSecurityOptions.SetSecureURLs( seqURLs );
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
        sal_uInt32 nCount = pEnumItem->GetTextCount();
        OUString aNoChangeStr( ' ' );
        for( sal_uInt32 nPath=0; nPath<nCount; ++nPath )
        {
            const OUString& sValue = pEnumItem->GetTextByPos(static_cast<sal_uInt16>(nPath));
            if ( sValue != aNoChangeStr )
            {
                switch( static_cast<SvtPathOptions::Paths>(nPath) )
                {
                    case SvtPathOptions::Paths::AddIn:
                    {
                        OUString aTmp;
                        if( osl::FileBase::getSystemPathFromFileURL( sValue, aTmp ) == osl::FileBase::E_None )
                            aPathOptions.SetAddinPath( aTmp );
                        break;
                    }

                    case SvtPathOptions::Paths::AutoCorrect:  aPathOptions.SetAutoCorrectPath( sValue );break;
                    case SvtPathOptions::Paths::AutoText:     aPathOptions.SetAutoTextPath( sValue );break;
                    case SvtPathOptions::Paths::Backup:       aPathOptions.SetBackupPath( sValue );break;
                    case SvtPathOptions::Paths::Basic:        aPathOptions.SetBasicPath( sValue );break;
                    case SvtPathOptions::Paths::Bitmap:       aPathOptions.SetBitmapPath( sValue );break;
                    case SvtPathOptions::Paths::Config:       aPathOptions.SetConfigPath( sValue );break;
                    case SvtPathOptions::Paths::Dictionary:   aPathOptions.SetDictionaryPath( sValue );break;
                    case SvtPathOptions::Paths::Favorites:    aPathOptions.SetFavoritesPath( sValue );break;
                    case SvtPathOptions::Paths::Filter:
                    {
                        OUString aTmp;
                        if( osl::FileBase::getSystemPathFromFileURL( sValue, aTmp ) == osl::FileBase::E_None )
                            aPathOptions.SetFilterPath( aTmp );
                        break;
                    }
                    case SvtPathOptions::Paths::Gallery:      aPathOptions.SetGalleryPath( sValue );break;
                    case SvtPathOptions::Paths::Graphic:      aPathOptions.SetGraphicPath( sValue );break;
                    case SvtPathOptions::Paths::Help:
                    {
                        OUString aTmp;
                        if( osl::FileBase::getSystemPathFromFileURL( sValue, aTmp ) == osl::FileBase::E_None )
                            aPathOptions.SetHelpPath( aTmp );
                        break;
                    }

                    case SvtPathOptions::Paths::Linguistic:   aPathOptions.SetLinguisticPath( sValue );break;
                    case SvtPathOptions::Paths::Module:
                    {
                        OUString aTmp;
                        if( osl::FileBase::getSystemPathFromFileURL( sValue, aTmp ) == osl::FileBase::E_None )
                            aPathOptions.SetModulePath( aTmp );
                        break;
                    }

                    case SvtPathOptions::Paths::Palette:      aPathOptions.SetPalettePath( sValue );break;
                    case SvtPathOptions::Paths::Plugin:
                    {
                        OUString aTmp;
                        if( osl::FileBase::getSystemPathFromFileURL( sValue, aTmp ) == osl::FileBase::E_None )
                            aPathOptions.SetPluginPath( aTmp );
                        break;
                    }

                    case SvtPathOptions::Paths::Storage:
                    {
                        OUString aTmp;
                        if( osl::FileBase::getSystemPathFromFileURL( sValue, aTmp ) == osl::FileBase::E_None )
                            aPathOptions.SetStoragePath( aTmp );
                        break;
                    }

                    case SvtPathOptions::Paths::Temp:         aPathOptions.SetTempPath( sValue );break;
                    case SvtPathOptions::Paths::Template:     aPathOptions.SetTemplatePath( sValue );break;
                    case SvtPathOptions::Paths::UserConfig:   aPathOptions.SetUserConfigPath( sValue );break;
                    case SvtPathOptions::Paths::Work:         aPathOptions.SetWorkPath( sValue );break;
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
