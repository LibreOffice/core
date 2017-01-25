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

#include <linkdlg.hxx>
#include <vcl/svapp.hxx>
#include "helpid.hrc"

#include <tools/urlobj.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <svtools/svtabbx.hxx>
#include "svtools/treelistentry.hxx"

#include <svuidlg.hrc>
#include <sfx2/linkmgr.hxx>
#include <sfx2/linksrc.hxx>
#include <svtools/soerr.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <comphelper/processfactory.hxx>

#include <dialmgr.hxx>


#define FILEOBJECT ( OBJECT_CLIENT_FILE & ~OBJECT_CLIENT_SO )

using namespace sfx2;
using namespace ::com::sun::star;

class SvBaseLinkMemberList {
private:
    std::vector<SvBaseLink*> mLinks;

public:
    ~SvBaseLinkMemberList()
    {
        for( std::vector<SvBaseLink*>::const_iterator it = mLinks.begin(); it != mLinks.end(); ++it )
        {
            SvBaseLink* p = *it;
            if( p )
                p->ReleaseRef();
        }
    }

    size_t size() const { return mLinks.size(); }

    SvBaseLink *operator[](size_t i) const { return mLinks[i]; }

    void push_back(SvBaseLink* p)
    {
        mLinks.push_back(p);
        p->AddFirstRef();
    }
};

// attention, this array is indexed directly (0, 1, ...) in the code
static long nTabs[] =
    {   4, // Number of Tabs
        0, 77, 144, 209
    };


SvBaseLinksDlg::SvBaseLinksDlg( vcl::Window * pParent, LinkManager* pMgr, bool bHtml )
    : ModalDialog( pParent, "BaseLinksDialog", "cui/ui/baselinksdialog.ui"),
    aStrAutolink( CUI_RES( STR_AUTOLINK ) ),
    aStrManuallink( CUI_RES( STR_MANUALLINK ) ),
    aStrBrokenlink( CUI_RES( STR_BROKENLINK ) ),
    aStrCloselinkmsg( CUI_RES( STR_CLOSELINKMSG ) ),
    aStrCloselinkmsgMulti( CUI_RES( STR_CLOSELINKMSG_MULTI ) ),
    aStrWaitinglink( CUI_RES( STR_WAITINGLINK ) ),
    pLinkMgr( nullptr ),
    bHtmlMode(bHtml),
    aUpdateIdle("cui SvBaseLinksDlg UpdateIdle")
{
    get(m_pTbLinks, "TB_LINKS");
    Size aSize(LogicToPixel(Size(257, 87), MapUnit::MapAppFont));
    m_pTbLinks->set_width_request(aSize.Width());
    m_pTbLinks->set_height_request(aSize.Height());
    get(m_pFtFullFileName, "FULL_FILE_NAME");
    get(m_pFtFullSourceName, "FULL_SOURCE_NAME");
    get(m_pFtFullTypeName, "FULL_TYPE_NAME");
    get(m_pRbAutomatic, "AUTOMATIC");
    get(m_pRbManual, "MANUAL");
    get(m_pPbUpdateNow, "UPDATE_NOW");
    get(m_pPbOpenSource, "OPEN");
    get(m_pPbChangeSource, "CHANGE_SOURCE");
    get(m_pPbBreakLink, "BREAK_LINK");

    m_pTbLinks->SetSelectionMode( SelectionMode::Multiple );
    m_pTbLinks->SetTabs( &nTabs[0] );
    FixedText *pFtFiles = get<FixedText>("FILES");
    pFtFiles->set_width_request(LogicToPixel(Size(nTabs[2] - nTabs[1] - 2, 0), MapUnit::MapAppFont).Width());
    FixedText *pFtLinks = get<FixedText>("LINKS");
    pFtLinks->set_width_request(LogicToPixel(Size(nTabs[3] - nTabs[2] - 2, 0), MapUnit::MapAppFont).Width());
    FixedText *pFtTypes = get<FixedText>("TYPE");
    pFtTypes->set_width_request(LogicToPixel(Size(nTabs[4] - nTabs[3] - 2, 0), MapUnit::MapAppFont).Width());
    m_pTbLinks->Resize();  // OS: hack for correct selection

    // UpdateTimer for DDE-/Grf-links, which are waited for
    aUpdateIdle.SetInvokeHandler( LINK( this, SvBaseLinksDlg, UpdateWaitingHdl ) );
    aUpdateIdle.SetPriority( TaskPriority::LOWEST );

    m_pPbOpenSource->Hide();

    m_pTbLinks->SetSelectHdl( LINK( this, SvBaseLinksDlg, LinksSelectHdl ) );
    m_pTbLinks->SetDoubleClickHdl( LINK( this, SvBaseLinksDlg, LinksDoubleClickHdl ) );
    m_pRbAutomatic->SetClickHdl( LINK( this, SvBaseLinksDlg, AutomaticClickHdl ) );
    m_pRbManual->SetClickHdl( LINK( this, SvBaseLinksDlg, ManualClickHdl ) );
    m_pPbUpdateNow->SetClickHdl( LINK( this, SvBaseLinksDlg, UpdateNowClickHdl ) );
    m_pPbChangeSource->SetClickHdl( LINK( this, SvBaseLinksDlg, ChangeSourceClickHdl ) );
    if(!bHtmlMode)
        m_pPbBreakLink->SetClickHdl( LINK( this, SvBaseLinksDlg, BreakLinkClickHdl ) );
    else
        m_pPbBreakLink->Hide();

    SetManager( pMgr );
}

SvBaseLinksDlg::~SvBaseLinksDlg()
{
    disposeOnce();
}

void SvBaseLinksDlg::dispose()
{
    m_pTbLinks.clear();
    m_pFtFullFileName.clear();
    m_pFtFullSourceName.clear();
    m_pFtFullTypeName.clear();
    m_pRbAutomatic.clear();
    m_pRbManual.clear();
    m_pPbUpdateNow.clear();
    m_pPbOpenSource.clear();
    m_pPbChangeSource.clear();
    m_pPbBreakLink.clear();
    ModalDialog::dispose();
}

/*************************************************************************
|*    SvBaseLinksDlg::Handler()
*************************************************************************/
IMPL_LINK( SvBaseLinksDlg, LinksSelectHdl, SvTreeListBox *, pSvTabListBox, void )
{
    const sal_uLong nSelectionCount = pSvTabListBox ?
        pSvTabListBox->GetSelectionCount() : 0;
    if(nSelectionCount > 1)
    {
        // possibly deselect old entries in case of multi-selection
        SvTreeListEntry* pEntry = nullptr;
        SvBaseLink* pLink = nullptr;
        pEntry = pSvTabListBox->GetHdlEntry();
        pLink = static_cast<SvBaseLink*>(pEntry->GetUserData());
        sal_uInt16 nObjectType = pLink->GetObjType();
        if((OBJECT_CLIENT_FILE & nObjectType) != OBJECT_CLIENT_FILE)
        {
            pSvTabListBox->SelectAll(false);
            pSvTabListBox->Select(pEntry);
        }
        else
        {
            for( sal_uLong i = 0; i < nSelectionCount; i++)
            {
                pEntry = i == 0 ? pSvTabListBox->FirstSelected() :
                                    pSvTabListBox->NextSelected(pEntry);
                DBG_ASSERT(pEntry, "Wo ist der Entry?");
                if (!pEntry)
                    continue;
                pLink = static_cast<SvBaseLink*>(pEntry->GetUserData());
                DBG_ASSERT(pLink, "Wo ist der Link?");
                if (!pLink)
                    continue;
                if( (OBJECT_CLIENT_FILE & pLink->GetObjType()) != OBJECT_CLIENT_FILE )
                    pSvTabListBox->Select( pEntry, false );

            }
        }

        m_pPbUpdateNow->Enable();

        m_pRbAutomatic->Disable();
        m_pRbManual->Check();
        m_pRbManual->Disable();
    }
    else
    {
        sal_uLong nPos;
        SvBaseLink* pLink = GetSelEntry( &nPos );
        if( !pLink )
            return;

        m_pPbUpdateNow->Enable();

        OUString sType, sLink;
        OUString *pLinkNm = &sLink, *pFilter = nullptr;

        if( FILEOBJECT & pLink->GetObjType() )
        {
            m_pRbAutomatic->Disable();
            m_pRbManual->Check();
            m_pRbManual->Disable();
            if( OBJECT_CLIENT_GRF == pLink->GetObjType() )
            {
                pLinkNm = nullptr;
                pFilter = &sLink;
            }
        }
        else
        {
            m_pRbAutomatic->Enable();
            m_pRbManual->Enable();

            if( SfxLinkUpdateMode::ALWAYS == pLink->GetUpdateMode() )
                m_pRbAutomatic->Check();
            else
                m_pRbManual->Check();
        }

        OUString aFileName;
        sfx2::LinkManager::GetDisplayNames( pLink, &sType, &aFileName, pLinkNm, pFilter );
        aFileName = INetURLObject::decode(aFileName, INetURLObject::DecodeMechanism::Unambiguous);
        m_pFtFullFileName->SetText( aFileName );
        m_pFtFullSourceName->SetText( sLink );
        m_pFtFullTypeName->SetText( sType );
    }
}

IMPL_LINK_NOARG( SvBaseLinksDlg, LinksDoubleClickHdl, SvTreeListBox *, bool )
{
    ChangeSourceClickHdl( nullptr );
    return false;
}

IMPL_LINK_NOARG( SvBaseLinksDlg, AutomaticClickHdl, Button*, void )
{
    sal_uLong nPos;
    SvBaseLink* pLink = GetSelEntry( &nPos );
    if( pLink && !( FILEOBJECT & pLink->GetObjType() ) &&
        SfxLinkUpdateMode::ALWAYS != pLink->GetUpdateMode() )
        SetType( *pLink, nPos, SfxLinkUpdateMode::ALWAYS );
}

IMPL_LINK_NOARG( SvBaseLinksDlg, ManualClickHdl, Button*, void )
{
    sal_uLong nPos;
    SvBaseLink* pLink = GetSelEntry( &nPos );
    if( pLink && !( FILEOBJECT & pLink->GetObjType() ) &&
        SfxLinkUpdateMode::ONCALL != pLink->GetUpdateMode())
        SetType( *pLink, nPos, SfxLinkUpdateMode::ONCALL );
}

IMPL_LINK_NOARG(SvBaseLinksDlg, UpdateNowClickHdl, Button*, void)
{
    SvTabListBox& rListBox = *m_pTbLinks;

    std::vector< SvBaseLink* > aLnkArr;
    std::vector< sal_Int16 > aPosArr;

    SvTreeListEntry* pE = rListBox.FirstSelected();
    while( pE )
    {
        sal_uLong nFndPos = rListBox.GetModel()->GetAbsPos( pE );
        if( TREELIST_ENTRY_NOTFOUND != nFndPos )
        {
            aLnkArr.push_back( static_cast< SvBaseLink* >( pE->GetUserData() ) );
            aPosArr.push_back( nFndPos );
        }
        pE = rListBox.NextSelected( pE );
    }

    if( !aLnkArr.empty() )
    {
        for( size_t n = 0; n < aLnkArr.size(); ++n )
        {
            tools::SvRef<SvBaseLink> xLink = aLnkArr[ n ];

            // first look for the entry in the array
            for(const auto & i : pLinkMgr->GetLinks())
                if( xLink == i )
                {
                    xLink->SetUseCache( false );
                    SetType( *xLink, aPosArr[ n ], xLink->GetUpdateMode() );
                    xLink->SetUseCache( true );
                    break;
                }
        }

        // if somebody is of the opinion to swap his links (SD)
        LinkManager* pNewMgr = pLinkMgr;
        pLinkMgr = nullptr;
        SetManager( pNewMgr );


        if( nullptr == (pE = rListBox.GetEntry( aPosArr[ 0 ] )) ||
            pE->GetUserData() != aLnkArr[ 0 ] )
        {
            // search the link
            pE = rListBox.First();
            while( pE )
            {
                if( pE->GetUserData() == aLnkArr[ 0 ] )
                    break;
                pE = rListBox.Next( pE );
            }

            if( !pE )
                pE = rListBox.FirstSelected();
        }

        if( pE )
        {
            SvTreeListEntry* pSelEntry = rListBox.FirstSelected();
            if( pE != pSelEntry )
                rListBox.Select( pSelEntry, false );
            rListBox.Select( pE );
            rListBox.MakeVisible( pE );
        }

        pNewMgr->CloseCachedComps();
    }
}

IMPL_LINK_NOARG( SvBaseLinksDlg, ChangeSourceClickHdl, Button *, void )
{
    sal_uLong nSelectionCount = m_pTbLinks->GetSelectionCount();
    if(nSelectionCount > 1)
    {
        try
        {
            uno::Reference<ui::dialogs::XFolderPicker2> xFolderPicker = ui::dialogs::FolderPicker::create(comphelper::getProcessComponentContext());

            OUString sType, sFile, sLinkName;
            OUString sFilter;
            SvTreeListEntry* pEntry = m_pTbLinks->FirstSelected();
            SvBaseLink* pLink = static_cast<SvBaseLink*>(pEntry->GetUserData());
            sfx2::LinkManager::GetDisplayNames( pLink, &sType, &sFile );
            INetURLObject aUrl(sFile);
            if(aUrl.GetProtocol() == INetProtocol::File)
            {
                OUString sOldPath(aUrl.PathToFileName());
                sal_Int32 nLen = aUrl.GetName().getLength();
                sOldPath = sOldPath.copy(0, sOldPath.getLength() - nLen);
                xFolderPicker->setDisplayDirectory(sOldPath);
            }
            if (xFolderPicker->execute() == ui::dialogs::ExecutableDialogResults::OK)
            {
                OUString aPath = xFolderPicker->getDirectory();

                for( sal_uLong i = 0; i < nSelectionCount; i++)
                {
                    pEntry = i==0 ?
                        m_pTbLinks->FirstSelected() :
                        m_pTbLinks->NextSelected( pEntry );
                    DBG_ASSERT(pEntry,"Where is the entry?");
                    if (!pEntry)
                        continue;
                    pLink = static_cast<SvBaseLink*>(pEntry->GetUserData());
                    DBG_ASSERT(pLink,"Where is the link?");
                    if (!pLink)
                        continue;
                    sfx2::LinkManager::GetDisplayNames( pLink, &sType, &sFile, &sLinkName, &sFilter );
                    INetURLObject aUrl_(sFile);
                    INetURLObject aUrl2(aPath, INetProtocol::File);
                    aUrl2.insertName( aUrl_.getName() );
                    OUString sNewLinkName;
                    MakeLnkName( sNewLinkName, nullptr ,
                            aUrl2.GetMainURL(INetURLObject::DecodeMechanism::ToIUri), sLinkName, &sFilter);
                    pLink->SetLinkSourceName( sNewLinkName );
                    pLink->Update();
                }
                if( pLinkMgr->GetPersist() )
                    pLinkMgr->GetPersist()->SetModified();
                LinkManager* pNewMgr = pLinkMgr;
                pLinkMgr = nullptr;
                SetManager( pNewMgr );
            }
        }
        catch (uno::Exception & e)
        {
            SAL_WARN("cui.dialogs", "SvBaseLinksDlg: caught UNO exception: " << e.Message);
        }
    }
    else
    {
        sal_uLong nPos;
        SvBaseLink* pLink = GetSelEntry( &nPos );
        if ( pLink && !pLink->GetLinkSourceName().isEmpty() )
            pLink->Edit( this, LINK( this, SvBaseLinksDlg, EndEditHdl ) );
    }
}

IMPL_LINK_NOARG( SvBaseLinksDlg, BreakLinkClickHdl, Button*, void )
{
    bool bModified = false;
    if(m_pTbLinks->GetSelectionCount() <= 1)
    {
        sal_uLong nPos;
        tools::SvRef<SvBaseLink> xLink = GetSelEntry( &nPos );
        if( !xLink.is() )
            return;

        ScopedVclPtrInstance< QueryBox > aBox( this, WB_YES_NO | WB_DEF_YES, aStrCloselinkmsg );

        if( RET_YES == aBox->Execute() )
        {
            m_pTbLinks->GetModel()->Remove( m_pTbLinks->GetEntry( nPos ) );

            // close object, if it's still existing
            bool bNewLnkMgr = OBJECT_CLIENT_FILE == xLink->GetObjType();

            // tell the link that it will be resolved!
            xLink->Closed();

            // if somebody has forgotten to deregister himself
            if( xLink.is() )
                pLinkMgr->Remove( xLink.get() );

            if( bNewLnkMgr )
            {
                LinkManager* pNewMgr = pLinkMgr;
                pLinkMgr = nullptr;
                SetManager( pNewMgr );

                SvTreeListEntry* pEntry = m_pTbLinks->GetEntry( nPos ? --nPos : 0 );
                if( pEntry )
                    m_pTbLinks->SetCurEntry( pEntry );
            }
            bModified = true;
        }
    }
    else
    {
        ScopedVclPtrInstance< QueryBox > aBox( this, WB_YES_NO | WB_DEF_YES, aStrCloselinkmsgMulti );

        if( RET_YES == aBox->Execute() )
        {

            SvBaseLinkMemberList aLinkList;
            SvTreeListEntry* pEntry = m_pTbLinks->FirstSelected();
            while ( pEntry )
            {
                void * pUD = pEntry->GetUserData();
                if( pUD )
                    aLinkList.push_back( static_cast<SvBaseLink*>(pUD) );
                pEntry = m_pTbLinks->NextSelected(pEntry);
            }
            m_pTbLinks->RemoveSelection();
            for( sal_uLong i = 0; i < aLinkList.size(); i++ )
            {
                tools::SvRef<SvBaseLink> xLink = aLinkList[i];
                // tell the link that it will be resolved!
                xLink->Closed();

                // if somebody has forgotten to deregister himself
                pLinkMgr->Remove( xLink.get() );
                bModified = true;
            }
            // then remove all selected entries
        }
    }
    if(bModified)
    {
        if( !m_pTbLinks->GetEntryCount() )
        {
            m_pRbAutomatic->Disable();
            m_pRbManual->Disable();
            m_pPbUpdateNow->Disable();
            m_pPbChangeSource->Disable();
            m_pPbBreakLink->Disable();

            m_pFtFullSourceName->SetText( "" );
            m_pFtFullTypeName->SetText( "" );
        }
        if( pLinkMgr && pLinkMgr->GetPersist() )
            pLinkMgr->GetPersist()->SetModified();
    }
}

IMPL_LINK_NOARG( SvBaseLinksDlg, UpdateWaitingHdl, Timer*, void )
{
    m_pTbLinks->SetUpdateMode(false);
    for( sal_uLong nPos = m_pTbLinks->GetEntryCount(); nPos; )
    {
        SvTreeListEntry* pBox = m_pTbLinks->GetEntry( --nPos );
        tools::SvRef<SvBaseLink> xLink( static_cast<SvBaseLink*>(pBox->GetUserData()) );
        if( xLink.is() )
        {
            OUString sCur( ImplGetStateStr( *xLink ) ),
                    sOld( SvTabListBox::GetEntryText( pBox, 3 ) );
            if( sCur != sOld )
                m_pTbLinks->SetEntryText( sCur, pBox, 3 );
        }
    }
    m_pTbLinks->SetUpdateMode(true);
}

IMPL_LINK( SvBaseLinksDlg, EndEditHdl, sfx2::SvBaseLink&, _rLink, void )
{
    sal_uLong nPos;
    GetSelEntry( &nPos );

    if( _rLink.WasLastEditOK() )
    {
        // StarImpress/Draw swap the LinkObjects themselves!
        // So search for the link in the manager; if it does not exist
        // anymore, fill the list completely new. Otherwise only the
        // edited link needs to be refreshed.
        bool bLinkFnd = false;
        for( size_t n = pLinkMgr->GetLinks().size(); n;  )
            if( &_rLink == &(*pLinkMgr->GetLinks()[ --n ]) )
            {
                bLinkFnd = true;
                break;
            }

        if( bLinkFnd )
        {
            m_pTbLinks->SetUpdateMode(false);
            m_pTbLinks->GetModel()->Remove( m_pTbLinks->GetEntry( nPos ) );
            SvTreeListEntry* pToUnselect = m_pTbLinks->FirstSelected();
            InsertEntry( _rLink, nPos, true );
            if(pToUnselect)
                m_pTbLinks->Select(pToUnselect, false);
            m_pTbLinks->SetUpdateMode(true);
        }
        else
        {
            LinkManager* pNewMgr = pLinkMgr;
            pLinkMgr = nullptr;
            SetManager( pNewMgr );
        }
        if (pLinkMgr && pLinkMgr->GetPersist())
            pLinkMgr->GetPersist()->SetModified();
    }
}

OUString SvBaseLinksDlg::ImplGetStateStr( const SvBaseLink& rLnk )
{
    OUString sRet;
    if( !rLnk.GetObj() )
        sRet = aStrBrokenlink;
    else if( rLnk.GetObj()->IsPending() )
    {
        sRet = aStrWaitinglink;
        aUpdateIdle.Start();
    }
    else if( SfxLinkUpdateMode::ALWAYS == rLnk.GetUpdateMode() )
        sRet = aStrAutolink;
    else
        sRet = aStrManuallink;

    return sRet;
}

void SvBaseLinksDlg::SetManager( LinkManager* pNewMgr )
{
    if( pLinkMgr == pNewMgr )
        return;

    if( pNewMgr )
        // update has to be stopped before clear
        m_pTbLinks->SetUpdateMode( false );

    m_pTbLinks->Clear();
    pLinkMgr = pNewMgr;

    if( pLinkMgr )
    {
        SvBaseLinks& rLnks = (SvBaseLinks&)pLinkMgr->GetLinks();
        for( size_t n = 0; n < rLnks.size(); ++n )
        {
            tools::SvRef<SvBaseLink>& rLinkRef = rLnks[ n ];
            if( !rLinkRef.is() )
            {
                rLnks.erase( rLnks.begin() + n );
                --n;
                continue;
            }
            if( rLinkRef->IsVisible() )
                InsertEntry( *rLinkRef );
        }

        if( !rLnks.empty() )
        {
            SvTreeListEntry* pEntry = m_pTbLinks->GetEntry( 0 );
            m_pTbLinks->SetCurEntry( pEntry );
            m_pTbLinks->Select( pEntry );
            LinksSelectHdl( nullptr );
        }
        m_pTbLinks->SetUpdateMode( true );
        m_pTbLinks->Invalidate();
    }
}


void SvBaseLinksDlg::InsertEntry( const SvBaseLink& rLink, sal_uLong nPos, bool bSelect )
{
    OUString aEntry, sFileNm, sLinkNm, sTypeNm, sFilter;

    sfx2::LinkManager::GetDisplayNames( &rLink, &sTypeNm, &sFileNm, &sLinkNm, &sFilter );

    // GetTab(0) gives the position of the bitmap which is automatically inserted by the TabListBox.
    // So the first text column's width is Tab(2)-Tab(1).
    long nWidthPixel = m_pTbLinks->GetLogicTab( 2 ) - m_pTbLinks->GetLogicTab( 1 );
    nWidthPixel -= SV_TAB_BORDER;
    OUString aTxt = m_pTbLinks->GetEllipsisString( sFileNm, nWidthPixel, DrawTextFlags::PathEllipsis );
    INetURLObject aPath( sFileNm, INetProtocol::File );
    OUString aFileName = aPath.getName();
    aFileName = INetURLObject::decode(aFileName, INetURLObject::DecodeMechanism::Unambiguous);

    if( aFileName.getLength() > aTxt.getLength() )
        aTxt = aFileName;
    else if( aTxt.indexOf( aFileName, aTxt.getLength() - aFileName.getLength() ) == -1 )
        // filename not in string
        aTxt = aFileName;

    aEntry = aTxt + "\t";
    if( OBJECT_CLIENT_GRF == rLink.GetObjType() )
        aEntry += sFilter;
    else
        aEntry += sLinkNm;
    aEntry += "\t" + sTypeNm + "\t" + ImplGetStateStr( rLink );

    SvTreeListEntry * pE = m_pTbLinks->InsertEntryToColumn( aEntry, nPos );
    pE->SetUserData( const_cast<SvBaseLink *>(&rLink) );
    if(bSelect)
        m_pTbLinks->Select(pE);
}

SvBaseLink* SvBaseLinksDlg::GetSelEntry( sal_uLong* pPos )
{
    SvTreeListEntry* pE = m_pTbLinks->FirstSelected();
    sal_uLong nPos;
    if( pE && TREELIST_ENTRY_NOTFOUND !=
        ( nPos = m_pTbLinks->GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );

        if( pPos )
            *pPos = nPos;
        return static_cast<SvBaseLink*>(pE->GetUserData());
    }
    return nullptr;
}

void SvBaseLinksDlg::SetType( SvBaseLink& rLink,
                                    sal_uLong nSelPos,
                                    SfxLinkUpdateMode nType )
{
    rLink.SetUpdateMode( nType );
    rLink.Update();
    SvTreeListEntry* pBox = m_pTbLinks->GetEntry( nSelPos );
    m_pTbLinks->SetEntryText( ImplGetStateStr( rLink ), pBox, 3 );
    if( pLinkMgr->GetPersist() )
        pLinkMgr->GetPersist()->SetModified();
}

void SvBaseLinksDlg::SetActLink( SvBaseLink * pLink )
{
    if( pLinkMgr )
    {
        const SvBaseLinks& rLnks = pLinkMgr->GetLinks();
        sal_uLong nSelect = 0;
        for(const auto & rLinkRef : rLnks)
        {
            // #109573# only visible links have been inserted into the TreeListBox,
            // invisible ones have to be skipped here
            if( rLinkRef->IsVisible() )
            {
                if( pLink == rLinkRef.get() )
                {
                    m_pTbLinks->Select( m_pTbLinks->GetEntry( nSelect ) );
                    LinksSelectHdl( nullptr );
                    return ;
                }
                nSelect++;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
