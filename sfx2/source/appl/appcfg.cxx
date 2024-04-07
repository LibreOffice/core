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
#include <officecfg/Office/Recovery.hxx>
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
    ~SfxEventAsyncer_Impl();
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
    pIdle.reset( new Idle("sfx::SfxEventAsyncer_Impl pIdle") );
    pIdle->SetInvokeHandler( LINK(this, SfxEventAsyncer_Impl, IdleHdl) );
    pIdle->SetPriority( TaskPriority::HIGH_IDLE );
    pIdle->Start();
}


SfxEventAsyncer_Impl::~SfxEventAsyncer_Impl()
{
    if (aHint.GetObjShell())
        EndListening(*aHint.GetObjShell());
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

    const WhichRangesContainer& pRanges = rSet.GetRanges();
    SvtMiscOptions aMiscOptions;

    for (auto const & pRange : pRanges)
    {
        for(sal_uInt16 nWhich = pRange.first; nWhich <= pRange.second; ++nWhich)
        {
            switch(nWhich)
            {
                case SID_ATTR_BUTTON_BIGSIZE :
                {
                    if( rSet.Put( SfxBoolItem( SID_ATTR_BUTTON_BIGSIZE, aMiscOptions.AreCurrentSymbolsLarge() ) ) )
                        bRet = true;
                    break;
                }
                case SID_ATTR_BACKUP :
                    {
                        bRet = true;
                        if (!officecfg::Office::Common::Save::Document::CreateBackup::isReadOnly())
                            if (!rSet.Put( SfxBoolItem( SID_ATTR_BACKUP,
                                    officecfg::Office::Common::Save::Document::CreateBackup::get() )))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_PRETTYPRINTING:
                    {
                        bRet = true;
                        if (!officecfg::Office::Common::Save::Document::PrettyPrinting::isReadOnly())
                            if (!rSet.Put( SfxBoolItem( SID_ATTR_PRETTYPRINTING,
                                    officecfg::Office::Common::Save::Document::PrettyPrinting::get())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_WARNALIENFORMAT:
                    {
                        bRet = true;
                        if (!officecfg::Office::Common::Save::Document::WarnAlienFormat::isReadOnly())
                            if (!rSet.Put( SfxBoolItem( SID_ATTR_WARNALIENFORMAT,
                                    officecfg::Office::Common::Save::Document::WarnAlienFormat::get() )))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_AUTOSAVE :
                    {
                        bRet = true;
                        if (!officecfg::Office::Common::Save::Document::AutoSave::isReadOnly())
                            if (!rSet.Put( SfxBoolItem( SID_ATTR_AUTOSAVE,
                                    officecfg::Office::Common::Save::Document::AutoSave::get() )))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_AUTOSAVEPROMPT :
                    {
                        bRet = true;
                        if (!officecfg::Office::Common::Save::Document::AutoSavePrompt::isReadOnly())
                            if (!rSet.Put( SfxBoolItem( SID_ATTR_AUTOSAVEPROMPT,
                                    officecfg::Office::Common::Save::Document::AutoSavePrompt::get())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_AUTOSAVEMINUTE :
                    {
                        bRet = true;
                        if (!officecfg::Office::Common::Save::Document::AutoSaveTimeIntervall::isReadOnly())
                            if (!rSet.Put( SfxUInt16Item( SID_ATTR_AUTOSAVEMINUTE,
                                    officecfg::Office::Common::Save::Document::AutoSaveTimeIntervall::get() )))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_USERAUTOSAVE :
                    {
                        bRet = true;
                        if (!officecfg::Office::Recovery::AutoSave::UserAutoSaveEnabled::isReadOnly())
                            if (!rSet.Put( SfxBoolItem( SID_ATTR_USERAUTOSAVE,
                                    officecfg::Office::Recovery::AutoSave::UserAutoSaveEnabled::get() )))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_DOCINFO :
                    {
                        bRet = true;
                        if (!officecfg::Office::Common::Save::Document::EditProperty::isReadOnly())
                            if (!rSet.Put( SfxBoolItem( SID_ATTR_DOCINFO,
                                    officecfg::Office::Common::Save::Document::EditProperty::get())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_WORKINGSET :
                    {
                        bRet = true;
                        if (!officecfg::Office::Common::Save::WorkingSet::isReadOnly())
                            if (!rSet.Put( SfxBoolItem( SID_ATTR_WORKINGSET,
                                    officecfg::Office::Common::Save::WorkingSet::get())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_SAVEDOCVIEW :
                    {
                        bRet = true;
                        if (!officecfg::Office::Common::Save::Document::ViewInfo::isReadOnly())
                            if (!rSet.Put( SfxBoolItem( SID_ATTR_SAVEDOCVIEW, officecfg::Office::Common::Save::Document::ViewInfo::get())))
                                bRet = false;
                    }
                    break;
                case SID_ATTR_METRIC :
                    break;
                case SID_HELPBALLOONS :
                    if(rSet.Put( SfxBoolItem ( SID_HELPBALLOONS,
                            officecfg::Office::Common::Help::ExtendedTip::get() ) ) )
                        bRet = true;
                    break;
                case SID_HELPTIPS :
                    if(rSet.Put( SfxBoolItem ( SID_HELPTIPS,
                            officecfg::Office::Common::Help::Tip::get() ) ) )
                        bRet = true;
                    break;
                case SID_HELP_STYLESHEET :
                    if(rSet.Put( SfxStringItem ( SID_HELP_STYLESHEET,
                               officecfg::Office::Common::Help::HelpStyleSheet::get() ) ) )
                        bRet = true;
                break;
                case SID_ATTR_UNDO_COUNT :
                    if (rSet.Put(
                            SfxUInt16Item (
                                SID_ATTR_UNDO_COUNT,
                                officecfg::Office::Common::Undo::Steps::get())))
                    {
                        bRet = true;
                    }
                    break;
                case SID_ATTR_QUICKLAUNCHER :
                {
                    if ( ShutdownIcon::IsQuickstarterInstalled() )
                    {
                        if ( rSet.Put( SfxBoolItem( SID_ATTR_QUICKLAUNCHER,
                                                    ShutdownIcon::GetAutostart() ) ) )
                            bRet = true;
                    }
                    else
                    {
                        rSet.DisableItem( SID_ATTR_QUICKLAUNCHER );
                        bRet = true;
                    }
                    break;
                }
                case SID_SAVEREL_INET :
                    {
                        bRet = true;
                        if (!officecfg::Office::Common::Save::URL::Internet::isReadOnly())
                            if (!rSet.Put( SfxBoolItem ( SID_SAVEREL_INET,
                                            officecfg::Office::Common::Save::URL::Internet::get() )))
                                bRet = false;
                    }
                    break;
                case SID_SAVEREL_FSYS :
                    {
                        bRet = true;
                        if (!officecfg::Office::Common::Save::URL::FileSystem::isReadOnly())
                            if (!rSet.Put( SfxBoolItem ( SID_SAVEREL_FSYS,
                                officecfg::Office::Common::Save::URL::FileSystem::get() )))
                                bRet = false;
                    }
                    break;
                case SID_SECURE_URL :
                    {
                        bRet = true;
                        if (!SvtSecurityOptions::IsReadOnly(SvtSecurityOptions::EOption::SecureUrls))
                        {
                            std::vector< OUString > seqURLs = SvtSecurityOptions::GetSecureURLs();

                            if( !rSet.Put( SfxStringListItem( SID_SECURE_URL, &seqURLs ) ) )
                                bRet = false;
                        }
                    }
                    break;
                case SID_INET_PROXY_TYPE :
                    if (rSet.Put(
                            SfxUInt16Item(
                                SID_INET_PROXY_TYPE,
                                (officecfg::Inet::Settings::ooInetProxyType::
                                 get().value_or(0)))))
                    {
                        bRet = true;
                    }
                    break;
                case SID_INET_HTTP_PROXY_NAME :
                    if (rSet.Put(
                            SfxStringItem(
                                SID_INET_HTTP_PROXY_NAME,
                                officecfg::Inet::Settings::ooInetHTTPProxyName::
                                get())))
                    {
                        bRet = true;
                    }
                    break;
                case SID_INET_HTTP_PROXY_PORT :
                    if (rSet.Put(
                            SfxInt32Item(
                                SID_INET_HTTP_PROXY_PORT,
                                (officecfg::Inet::Settings::
                                 ooInetHTTPProxyPort::get().value_or(0)))))
                    {
                        bRet = true;
                    }
                    break;
                case SID_INET_FTP_PROXY_NAME :
                    if (rSet.Put(
                            SfxStringItem(
                                SID_INET_FTP_PROXY_NAME,
                                officecfg::Inet::Settings::ooInetFTPProxyName::
                                get())))
                    {
                        bRet = true;
                    }
                    break;
                case SID_INET_FTP_PROXY_PORT :
                    if (rSet.Put(
                            SfxInt32Item(
                                SID_INET_FTP_PROXY_PORT,
                                (officecfg::Inet::Settings::ooInetFTPProxyPort::
                                 get().value_or(0)))))
                    {
                        bRet = true;
                    }
                    break;
                case SID_INET_NOPROXY :
                    if (rSet.Put(
                            SfxStringItem(
                                SID_INET_NOPROXY,
                                (officecfg::Inet::Settings::ooInetNoProxy::
                                 get()))))
                    {
                        bRet = true;
                    }
                    break;
                case SID_ATTR_PATHNAME :
                {
                    SfxAllEnumItem aValues(SID_ATTR_PATHNAME);
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
    SfxItemPool &rPool = GetPool();

    SvtMiscOptions aMiscOptions;
    std::shared_ptr< comphelper::ConfigurationChanges > batch(
        comphelper::ConfigurationChanges::create());

    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet(SID_ATTR_BUTTON_BIGSIZE) )
    {
        bool bBigSize = pItem->GetValue();
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
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet(SID_ATTR_BACKUP) )
    {
        officecfg::Office::Common::Save::Document::CreateBackup::set(
                pItem->GetValue(),
                batch );
    }

    // PrettyPrinting
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_ATTR_PRETTYPRINTING ) )
    {
        officecfg::Office::Common::Save::Document::PrettyPrinting::set(
            pItem->GetValue(),
            batch );
    }

    // WarnAlienFormat
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_ATTR_WARNALIENFORMAT ) )
    {
        officecfg::Office::Common::Save::Document::WarnAlienFormat::set(
                pItem->GetValue(),
                batch);
    }

    // AutoSave
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_ATTR_AUTOSAVE ))
    {
        officecfg::Office::Common::Save::Document::AutoSave::set(
            pItem->GetValue(),
            batch);
    }

    // AutoSave-Prompt
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_ATTR_AUTOSAVEPROMPT ))
    {
        officecfg::Office::Common::Save::Document::AutoSavePrompt::set(
            pItem->GetValue(),
            batch);
    }

    // AutoSave-Time
    if ( const SfxUInt16Item *pItem = rSet.GetItemIfSet(SID_ATTR_AUTOSAVEMINUTE ))
    {
        officecfg::Office::Common::Save::Document::AutoSaveTimeIntervall::set(
                pItem->GetValue(),
                batch);
    }

    // UserAutoSave
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet(SID_ATTR_USERAUTOSAVE))
    {
        officecfg::Office::Recovery::AutoSave::UserAutoSaveEnabled::set(
                pItem->GetValue(),
                batch);
    }

    // DocInfo
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet(SID_ATTR_DOCINFO) )
    {
        officecfg::Office::Common::Save::Document::EditProperty::set(
            pItem->GetValue(),
            batch);
    }

    // Mark open Documents
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet(SID_ATTR_WORKINGSET))
    {
        officecfg::Office::Common::Save::WorkingSet::set(
            pItem->GetValue(),
            batch);
    }

    // Save window settings
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet(SID_ATTR_SAVEDOCVIEW))
    {
        officecfg::Office::Common::Save::Document::ViewInfo::set(pItem->GetValue(), batch);
    }

    // Metric
    const SfxPoolItem* pItem1 = nullptr;
    if ( SfxItemState::SET == rSet.GetItemState(rPool.GetWhich(SID_ATTR_METRIC), true, &pItem1))
    {
        DBG_ASSERT(dynamic_cast< const SfxUInt16Item *>( pItem1 ) !=  nullptr, "UInt16Item expected");
    }

    // HelpBalloons
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet(SID_HELPBALLOONS))
    {
        officecfg::Office::Common::Help::ExtendedTip::set(
                pItem->GetValue(),
                batch);
    }

    // HelpTips
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet(SID_HELPTIPS))
    {
        officecfg::Office::Common::Help::Tip::set(
                pItem->GetValue(),
                batch);
    }

    if ( const SfxStringItem *pItem = rSet.GetItemIfSet(SID_HELP_STYLESHEET))
    {
        OUString sStyleSheet = pItem->GetValue();
        officecfg::Office::Common::Help::HelpStyleSheet::set(sStyleSheet, batch);
    }

    // SaveRelINet
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet(SID_SAVEREL_INET))
    {
        officecfg::Office::Common::Save::URL::Internet::set(
            pItem->GetValue(),
            batch);
    }

    // SaveRelFSys
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet(SID_SAVEREL_FSYS))
    {
        officecfg::Office::Common::Save::URL::FileSystem::set(
            pItem->GetValue(),
            batch);
    }

    // Undo-Count
    if ( const SfxUInt16Item *pItem = rSet.GetItemIfSet(SID_ATTR_UNDO_COUNT))
    {
        sal_uInt16 nUndoCount = pItem->GetValue();
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
    if ( const SfxBoolItem *pItem = rSet.GetItemIfSet(SID_ATTR_QUICKLAUNCHER))
    {
        ShutdownIcon::SetAutostart( pItem->GetValue() );
    }

    if ( const SfxUInt16Item *pItem = rSet.GetItemIfSet(SID_INET_PROXY_TYPE))
    {
        officecfg::Inet::Settings::ooInetProxyType::set(
            pItem->GetValue(), batch);
    }

    if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_INET_HTTP_PROXY_NAME ) )
    {
        officecfg::Inet::Settings::ooInetHTTPProxyName::set(
            pItem->GetValue(), batch);
    }
    if ( const SfxInt32Item *pItem = rSet.GetItemIfSet( SID_INET_HTTP_PROXY_PORT ) )
    {
        officecfg::Inet::Settings::ooInetHTTPProxyPort::set(
            pItem->GetValue(), batch);
    }
    if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_INET_FTP_PROXY_NAME ) )
    {
        officecfg::Inet::Settings::ooInetFTPProxyName::set(
            pItem->GetValue(), batch);
    }
    if (const SfxInt32Item *pItem = rSet.GetItemIfSet( SID_INET_FTP_PROXY_PORT ) )
    {
        officecfg::Inet::Settings::ooInetFTPProxyPort::set(
            pItem->GetValue(), batch);
    }
    if ( const SfxStringItem* pStringItem = rSet.GetItemIfSet(SID_INET_NOPROXY))
    {
        officecfg::Inet::Settings::ooInetNoProxy::set(
            pStringItem->GetValue(), batch);
    }

    // Secure-Referrer
    if ( const SfxStringListItem *pListItem = rSet.GetItemIfSet(SID_SECURE_URL))
    {
        SvtSecurityOptions::SetSecureURLs( std::vector(pListItem->GetList()) );
    }

    // Store changed data
    batch->commit();
}


void SfxApplication::SetOptions(const SfxItemSet &rSet)
{
    SvtPathOptions aPathOptions;

    // Data is saved in DocInfo and IniManager

    SfxAllItemSet aSendSet( rSet );

    // PathName
    if ( const SfxAllEnumItem* pEnumItem = rSet.GetItemIfSet(SID_ATTR_PATHNAME))
    {
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

        aSendSet.ClearItem( SID_ATTR_PATHNAME );
    }

    SetOptions_Impl( rSet );

    // Undo-Count
    Broadcast( SfxItemSetHint( rSet ) );
}


void SfxApplication::NotifyEvent( const SfxEventHint& rEventHint, bool bSynchron )
{
    rtl::Reference<SfxObjectShell> pDoc = rEventHint.GetObjShell();
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
