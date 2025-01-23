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
#include <svl/slstitm.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/undo.hxx>
#include <svl/whiter.hxx>

#include <sfx2/sfxsids.hrc>

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
#include <comphelper/lok.hxx>
#include <objshimp.hxx>
#include "shutdownicon.hxx"

using namespace ::com::sun::star::uno;

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

namespace
{
template <class Cfg, class Item> bool toSet(SfxItemSet& rSet, TypedWhichId<Item> wid)
{
    return rSet.Put(Item(wid, Cfg::get()));
}
template <class Cfg, class Item, class Val>
bool toSet_withDefault(SfxItemSet& rSet, TypedWhichId<Item> wid, Val&& defVal)
{
    return rSet.Put(Item(wid, Cfg::get().value_or(std::move(defVal))));
}
template <class Cfg, class Item> bool toSet_ifRW(SfxItemSet& rSet, TypedWhichId<Item> wid)
{
    return Cfg::isReadOnly() || toSet<Cfg>(rSet, wid);
}

template <class Cfg, class Item>
void toCfg_ifSet(const SfxItemSet& rSet, TypedWhichId<Item> wid,
                 std::shared_ptr<comphelper::ConfigurationChanges> const& batch)
{
    if (const auto* pItem = rSet.GetItemIfSet(wid))
        Cfg::set(pItem->GetValue(), batch);
}
}

void SfxApplication::GetOptions( SfxItemSet& rSet )
{
    SfxWhichIter iter(rSet);
    for (auto nWhich = iter.FirstWhich(); nWhich; nWhich = iter.NextWhich())
    {
        bool bRet = false;
        switch(nWhich)
        {
            case SID_ATTR_BACKUP:
                bRet = true;
                if (!officecfg::Office::Common::Save::Document::CreateBackup::isReadOnly())
                    if (!rSet.Put( SfxBoolItem( SID_ATTR_BACKUP,
                            (officecfg::Office::Common::Save::Document::CreateBackup::get() && !comphelper::LibreOfficeKit::isActive()) )))
                        bRet = false;
                break;
            case SID_ATTR_BACKUP_BESIDE_ORIGINAL:
                bRet = toSet_ifRW<officecfg::Office::Common::Save::Document::BackupIntoDocumentFolder>(
                    rSet, SID_ATTR_BACKUP_BESIDE_ORIGINAL);
                break;
            case SID_ATTR_PRETTYPRINTING:
                bRet = toSet_ifRW<officecfg::Office::Common::Save::Document::PrettyPrinting>(
                    rSet, SID_ATTR_PRETTYPRINTING);
                break;
            case SID_ATTR_WARNALIENFORMAT:
                bRet = toSet_ifRW<officecfg::Office::Common::Save::Document::WarnAlienFormat>(
                    rSet, SID_ATTR_WARNALIENFORMAT);
                break;
            case SID_ATTR_AUTOSAVE:
                bRet = toSet_ifRW<officecfg::Office::Recovery::AutoSave::Enabled>(
                    rSet, SID_ATTR_AUTOSAVE);
                break;
            case SID_ATTR_AUTOSAVEMINUTE:
                bRet = toSet_ifRW<officecfg::Office::Recovery::AutoSave::TimeIntervall>(
                    rSet, SID_ATTR_AUTOSAVEMINUTE);
                break;
            case SID_ATTR_USERAUTOSAVE:
                bRet = toSet_ifRW<officecfg::Office::Recovery::AutoSave::UserAutoSaveEnabled>(
                    rSet, SID_ATTR_USERAUTOSAVE);
                break;
            case SID_ATTR_DOCINFO:
                bRet = toSet_ifRW<officecfg::Office::Common::Save::Document::EditProperty>(
                    rSet, SID_ATTR_DOCINFO);
                break;
            case SID_ATTR_QUICKLAUNCHER:
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
            case SID_SAVEREL_INET:
                bRet = toSet_ifRW<officecfg::Office::Common::Save::URL::Internet>(
                    rSet, SID_SAVEREL_INET);
                break;
            case SID_SAVEREL_FSYS:
                bRet = toSet_ifRW<officecfg::Office::Common::Save::URL::FileSystem>(
                    rSet, SID_SAVEREL_FSYS);
                break;
            case SID_SECURE_URL:
                bRet = true;
                if (!SvtSecurityOptions::IsReadOnly(SvtSecurityOptions::EOption::SecureUrls))
                {
                    std::vector< OUString > seqURLs = SvtSecurityOptions::GetSecureURLs();

                    if( !rSet.Put( SfxStringListItem( SID_SECURE_URL, &seqURLs ) ) )
                        bRet = false;
                }
                break;
            case SID_INET_HTTP_PROXY_NAME:
                bRet = toSet<officecfg::Inet::Settings::ooInetHTTPProxyName>(
                    rSet, SID_INET_HTTP_PROXY_NAME);
                break;
            case SID_INET_HTTP_PROXY_PORT:
                bRet = toSet_withDefault<officecfg::Inet::Settings::ooInetHTTPProxyPort>(
                    rSet, SID_INET_HTTP_PROXY_PORT, 0);
                break;
            case SID_INET_NOPROXY:
                bRet = toSet<officecfg::Inet::Settings::ooInetNoProxy>(rSet, SID_INET_NOPROXY);
                break;

            default:
                SAL_INFO( "sfx.appl", "W1:Wrong ID while getting Options!" );
                break;
        }
        SAL_WARN_IF(!bRet, "sfx.appl", "Putting options failed!");
    }
}

void SfxApplication::SetOptions(const SfxItemSet &rSet)
{
    std::shared_ptr< comphelper::ConfigurationChanges > batch(
        comphelper::ConfigurationChanges::create());

    // Backup
    toCfg_ifSet<officecfg::Office::Common::Save::Document::CreateBackup>(
        rSet, SID_ATTR_BACKUP, batch);

    toCfg_ifSet<officecfg::Office::Common::Save::Document::BackupIntoDocumentFolder>(
        rSet, SID_ATTR_BACKUP_BESIDE_ORIGINAL, batch);

    // PrettyPrinting
    toCfg_ifSet<officecfg::Office::Common::Save::Document::PrettyPrinting>(
        rSet, SID_ATTR_PRETTYPRINTING, batch);

    // WarnAlienFormat
    toCfg_ifSet<officecfg::Office::Common::Save::Document::WarnAlienFormat>(
        rSet, SID_ATTR_WARNALIENFORMAT, batch);

    // AutoSave
    toCfg_ifSet<officecfg::Office::Recovery::AutoSave::Enabled>(rSet, SID_ATTR_AUTOSAVE, batch);

    // AutoSave-Time
    toCfg_ifSet<officecfg::Office::Recovery::AutoSave::TimeIntervall>(
        rSet, SID_ATTR_AUTOSAVEMINUTE, batch);

    // UserAutoSave
    toCfg_ifSet<officecfg::Office::Recovery::AutoSave::UserAutoSaveEnabled>(
        rSet, SID_ATTR_USERAUTOSAVE, batch);

    // DocInfo
    toCfg_ifSet<officecfg::Office::Common::Save::Document::EditProperty>(
        rSet, SID_ATTR_DOCINFO, batch);

    // HelpBalloons
    toCfg_ifSet<officecfg::Office::Common::Help::ExtendedTip>(rSet, SID_HELPBALLOONS, batch);

    // HelpTips
    toCfg_ifSet<officecfg::Office::Common::Help::Tip>(rSet, SID_HELPTIPS, batch);

    // SaveRelINet
    toCfg_ifSet<officecfg::Office::Common::Save::URL::Internet>(rSet, SID_SAVEREL_INET, batch);

    // SaveRelFSys
    toCfg_ifSet<officecfg::Office::Common::Save::URL::FileSystem>(rSet, SID_SAVEREL_FSYS, batch);

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

    toCfg_ifSet<officecfg::Inet::Settings::ooInetProxyType>(rSet, SID_INET_PROXY_TYPE, batch);

    toCfg_ifSet<officecfg::Inet::Settings::ooInetHTTPProxyName>(
        rSet, SID_INET_HTTP_PROXY_NAME, batch);
    toCfg_ifSet<officecfg::Inet::Settings::ooInetHTTPProxyPort>(
        rSet, SID_INET_HTTP_PROXY_PORT, batch);
    toCfg_ifSet<officecfg::Inet::Settings::ooInetNoProxy>(rSet, SID_INET_NOPROXY, batch);

    // Secure-Referrer
    if ( const SfxStringListItem *pListItem = rSet.GetItemIfSet(SID_SECURE_URL))
    {
        SvtSecurityOptions::SetSecureURLs( std::vector(pListItem->GetList()) );
    }

    // Store changed data
    batch->commit();
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
