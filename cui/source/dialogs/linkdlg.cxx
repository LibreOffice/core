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
#include <sal/log.hxx>
#include <vcl/svapp.hxx>

#include <tools/urlobj.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/weld.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/svtabbx.hxx>
#include <vcl/treelistentry.hxx>

#include <strings.hrc>
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
        for (auto const& link : mLinks)
        {
            if( link )
                link->ReleaseRef();
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

SvBaseLinksDlg::SvBaseLinksDlg(weld::Window * pParent, LinkManager* pMgr, bool bHtmlMode)
    : GenericDialogController(pParent, "cui/ui/baselinksdialog.ui", "BaseLinksDialog")
    , aStrAutolink( CuiResId( STR_AUTOLINK ) )
    , aStrManuallink( CuiResId( STR_MANUALLINK ) )
    , aStrBrokenlink( CuiResId( STR_BROKENLINK ) )
    , aStrCloselinkmsg( CuiResId( STR_CLOSELINKMSG ) )
    , aStrCloselinkmsgMulti( CuiResId( STR_CLOSELINKMSG_MULTI ) )
    , aStrWaitinglink( CuiResId( STR_WAITINGLINK ) )
    , pLinkMgr( nullptr )
    , aUpdateIdle("cui SvBaseLinksDlg UpdateIdle")
    , m_xTbLinks(m_xBuilder->weld_tree_view("TB_LINKS"))
    , m_xFtFullFileName(m_xBuilder->weld_link_button("FULL_FILE_NAME"))
    , m_xFtFullSourceName(m_xBuilder->weld_label("FULL_SOURCE_NAME"))
    , m_xFtFullTypeName(m_xBuilder->weld_label("FULL_TYPE_NAME"))
    , m_xRbAutomatic(m_xBuilder->weld_radio_button("AUTOMATIC"))
    , m_xRbManual(m_xBuilder->weld_radio_button("MANUAL"))
    , m_xPbUpdateNow(m_xBuilder->weld_button("UPDATE_NOW"))
    , m_xPbChangeSource(m_xBuilder->weld_button("CHANGE_SOURCE"))
    , m_xPbBreakLink(m_xBuilder->weld_button("BREAK_LINK"))
    , m_xVirDev(VclPtr<VirtualDevice>::Create())
{
    // expand the point size of the desired font to the equivalent pixel size
    if (vcl::Window* pDefaultDevice = dynamic_cast<vcl::Window*>(Application::GetDefaultDevice()))
        pDefaultDevice->SetPointFont(*m_xVirDev, m_xTbLinks->get_font());
    m_xTbLinks->set_size_request(m_xTbLinks->get_approximate_digit_width() * 90,
                                 m_xTbLinks->get_height_rows(12));

    m_xTbLinks->set_selection_mode(SelectionMode::Multiple);

    std::vector<int> aWidths;
    aWidths.push_back(m_xTbLinks->get_approximate_digit_width() * 30);
    aWidths.push_back(m_xTbLinks->get_approximate_digit_width() * 20);
    aWidths.push_back(m_xTbLinks->get_approximate_digit_width() * 20);
    m_xTbLinks->set_column_fixed_widths(aWidths);

    // UpdateTimer for DDE-/Grf-links, which are waited for
    aUpdateIdle.SetInvokeHandler( LINK( this, SvBaseLinksDlg, UpdateWaitingHdl ) );
    aUpdateIdle.SetPriority( TaskPriority::LOWEST );

    m_xTbLinks->connect_changed( LINK( this, SvBaseLinksDlg, LinksSelectHdl ) );
    m_xTbLinks->connect_row_activated( LINK( this, SvBaseLinksDlg, LinksDoubleClickHdl ) );
    m_xRbAutomatic->connect_clicked( LINK( this, SvBaseLinksDlg, AutomaticClickHdl ) );
    m_xRbManual->connect_clicked( LINK( this, SvBaseLinksDlg, ManualClickHdl ) );
    m_xPbUpdateNow->connect_clicked( LINK( this, SvBaseLinksDlg, UpdateNowClickHdl ) );
    m_xPbChangeSource->connect_clicked( LINK( this, SvBaseLinksDlg, ChangeSourceClickHdl ) );
    if(!bHtmlMode)
        m_xPbBreakLink->connect_clicked( LINK( this, SvBaseLinksDlg, BreakLinkClickHdl ) );
    else
        m_xPbBreakLink->hide();

    SetManager( pMgr );
}

SvBaseLinksDlg::~SvBaseLinksDlg()
{
}

/*************************************************************************
|*    SvBaseLinksDlg::Handler()
*************************************************************************/
IMPL_LINK(SvBaseLinksDlg, LinksSelectHdl, weld::TreeView&, rTreeView, void)
{
    LinksSelectHdl(&rTreeView);
}

void SvBaseLinksDlg::LinksSelectHdl(weld::TreeView* pSvTabListBox)
{
    const int nSelectionCount = pSvTabListBox ?
        pSvTabListBox->count_selected_rows() : 0;
    if (nSelectionCount > 1)
    {
        // possibly deselect old entries in case of multi-selection
        int nSelEntry = pSvTabListBox->get_selected_index();
        SvBaseLink* pLink = reinterpret_cast<SvBaseLink*>(pSvTabListBox->get_id(nSelEntry).toInt64());
        sal_uInt16 nObjectType = pLink->GetObjType();
        if((OBJECT_CLIENT_FILE & nObjectType) != OBJECT_CLIENT_FILE)
        {
            pSvTabListBox->unselect_all();
            pSvTabListBox->select(nSelEntry);
        }
        else
        {
            std::vector<int> aRows = pSvTabListBox->get_selected_rows();
            for (auto nEntry : aRows)
            {
                pLink = reinterpret_cast<SvBaseLink*>(pSvTabListBox->get_id(nEntry).toInt64());
                DBG_ASSERT(pLink, "Where is the Link?");
                if (!pLink)
                    continue;
                if( (OBJECT_CLIENT_FILE & pLink->GetObjType()) != OBJECT_CLIENT_FILE )
                    pSvTabListBox->unselect(nEntry);
            }
        }

        m_xPbUpdateNow->set_sensitive(true);
        m_xRbAutomatic->set_sensitive(false);
        m_xRbManual->set_active(true);
        m_xRbManual->set_sensitive(false);
    }
    else
    {
        int nPos;
        SvBaseLink* pLink = GetSelEntry( &nPos );
        if( !pLink )
            return;

        m_xPbUpdateNow->set_sensitive(true);

        OUString sType, sLink;
        OUString *pLinkNm = &sLink, *pFilter = nullptr;

        if( FILEOBJECT & pLink->GetObjType() )
        {
            m_xRbAutomatic->set_sensitive(false);
            m_xRbManual->set_active(true);
            m_xRbManual->set_sensitive(false);
            if( OBJECT_CLIENT_GRF == pLink->GetObjType() )
            {
                pLinkNm = nullptr;
                pFilter = &sLink;
            }
        }
        else
        {
            m_xRbAutomatic->set_sensitive(true);
            m_xRbManual->set_sensitive(true);

            if( SfxLinkUpdateMode::ALWAYS == pLink->GetUpdateMode() )
                m_xRbAutomatic->set_active(true);
            else
                m_xRbManual->set_active(true);
        }

        OUString aFileName;
        sfx2::LinkManager::GetDisplayNames( pLink, &sType, &aFileName, pLinkNm, pFilter );
        aFileName = INetURLObject::decode(aFileName, INetURLObject::DecodeMechanism::Unambiguous);
        m_xFtFullFileName->set_label( aFileName );
        m_xFtFullFileName->set_uri( aFileName );
        m_xFtFullSourceName->set_label( sLink );
        m_xFtFullTypeName->set_label( sType );
    }
}

IMPL_LINK_NOARG( SvBaseLinksDlg, LinksDoubleClickHdl, weld::TreeView&, void )
{
    ChangeSourceClickHdl(*m_xPbChangeSource);
}

IMPL_LINK_NOARG( SvBaseLinksDlg, AutomaticClickHdl, weld::Button&, void )
{
    int nPos;
    SvBaseLink* pLink = GetSelEntry( &nPos );
    if( pLink && !( FILEOBJECT & pLink->GetObjType() ) &&
        SfxLinkUpdateMode::ALWAYS != pLink->GetUpdateMode() )
        SetType( *pLink, nPos, SfxLinkUpdateMode::ALWAYS );
}

IMPL_LINK_NOARG( SvBaseLinksDlg, ManualClickHdl, weld::Button&, void )
{
    int nPos;
    SvBaseLink* pLink = GetSelEntry( &nPos );
    if( pLink && !( FILEOBJECT & pLink->GetObjType() ) &&
        SfxLinkUpdateMode::ONCALL != pLink->GetUpdateMode())
        SetType( *pLink, nPos, SfxLinkUpdateMode::ONCALL );
}

IMPL_LINK_NOARG(SvBaseLinksDlg, UpdateNowClickHdl, weld::Button&, void)
{
    std::vector< SvBaseLink* > aLnkArr;
    std::vector< sal_Int16 > aPosArr;

    std::vector<int> aRows = m_xTbLinks->get_selected_rows();
    for (int nFndPos : aRows)
    {
        aLnkArr.push_back( reinterpret_cast<SvBaseLink*>( m_xTbLinks->get_id(nFndPos).toInt64() ) );
        aPosArr.push_back( nFndPos );
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
                    SetType( *xLink, aPosArr[ n ], xLink->GetUpdateMode() );
                    break;
                }
        }

        // if somebody is of the opinion to swap his links (SD)
        LinkManager* pNewMgr = pLinkMgr;
        pLinkMgr = nullptr;
        SetManager( pNewMgr );


        OUString sId = OUString::number(reinterpret_cast<sal_Int64>(aLnkArr[0]));
        int nE = m_xTbLinks->find_id(sId);
        if (nE == -1)
            nE = m_xTbLinks->get_selected_index();
        int nSelEntry = m_xTbLinks->get_selected_index();
        if (nE != nSelEntry)
            m_xTbLinks->unselect(nSelEntry);
        m_xTbLinks->select(nE);
        m_xTbLinks->scroll_to_row(nE);

        pNewMgr->CloseCachedComps();
    }
}

IMPL_LINK_NOARG(SvBaseLinksDlg, ChangeSourceClickHdl, weld::Button&, void)
{
    std::vector<int> aRows = m_xTbLinks->get_selected_rows();
    if (aRows.size() > 1)
    {
        try
        {
            uno::Reference<ui::dialogs::XFolderPicker2> xFolderPicker = ui::dialogs::FolderPicker::create(comphelper::getProcessComponentContext());

            OUString sType, sFile, sLinkName;
            OUString sFilter;
            SvBaseLink* pLink = reinterpret_cast<SvBaseLink*>(m_xTbLinks->get_id(aRows[0]).toInt64());
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

                for (auto nRow : aRows)
                {
                    pLink = reinterpret_cast<SvBaseLink*>(m_xTbLinks->get_id(nRow).toInt64());
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
            SAL_WARN("cui.dialogs", "SvBaseLinksDlg: " << e);
        }
    }
    else
    {
        int nPos;
        SvBaseLink* pLink = GetSelEntry( &nPos );
        if ( pLink && !pLink->GetLinkSourceName().isEmpty() )
            pLink->Edit(m_xDialog.get(), LINK(this, SvBaseLinksDlg, EndEditHdl));
    }
}

IMPL_LINK_NOARG( SvBaseLinksDlg, BreakLinkClickHdl, weld::Button&, void )
{
    bool bModified = false;
    if (m_xTbLinks->count_selected_rows() <= 1)
    {
        int nPos;
        tools::SvRef<SvBaseLink> xLink = GetSelEntry( &nPos );
        if( !xLink.is() )
            return;

        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                       aStrCloselinkmsg));
        xQueryBox->set_default_response(RET_YES);

        if (RET_YES == xQueryBox->run())
        {
            m_xTbLinks->remove(nPos);

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
                m_xTbLinks->set_cursor(nPos ? --nPos : 0);
            }
            bModified = true;
        }
    }
    else
    {
        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                       aStrCloselinkmsgMulti));
        xQueryBox->set_default_response(RET_YES);

        if (RET_YES == xQueryBox->run())
        {
            std::vector<int> aRows = m_xTbLinks->get_selected_rows();
            SvBaseLinkMemberList aLinkList;
            for (auto nRow : aRows)
            {
                SvBaseLink* pLink = reinterpret_cast<SvBaseLink*>(m_xTbLinks->get_id(nRow).toInt64());
                if (pLink)
                    aLinkList.push_back(pLink);
            }
            std::sort(aRows.begin(), aRows.end());
            for (auto it = aRows.rbegin(); it != aRows.rend(); ++it)
                m_xTbLinks->remove(*it);
            for (size_t i = 0; i < aLinkList.size(); ++i)
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
        if (!m_xTbLinks->n_children())
        {
            m_xRbAutomatic->set_sensitive(false);
            m_xRbManual->set_sensitive(false);
            m_xPbUpdateNow->set_sensitive(false);
            m_xPbChangeSource->set_sensitive(false);
            m_xPbBreakLink->set_sensitive(false);

            m_xFtFullSourceName->set_label( "" );
            m_xFtFullTypeName->set_label( "" );
        }
        if( pLinkMgr && pLinkMgr->GetPersist() )
            pLinkMgr->GetPersist()->SetModified();
    }
}

IMPL_LINK_NOARG( SvBaseLinksDlg, UpdateWaitingHdl, Timer*, void )
{
    m_xTbLinks->freeze();
    for (int nPos = m_xTbLinks->n_children(); nPos; --nPos)
    {
        tools::SvRef<SvBaseLink> xLink( reinterpret_cast<SvBaseLink*>(m_xTbLinks->get_id(nPos).toInt64()) );
        if( xLink.is() )
        {
            OUString sCur( ImplGetStateStr( *xLink ) ),
                    sOld( m_xTbLinks->get_text(nPos, 3) );
            if( sCur != sOld )
                m_xTbLinks->set_text(nPos, sCur, 3);
        }
    }
    m_xTbLinks->thaw();
}

IMPL_LINK( SvBaseLinksDlg, EndEditHdl, sfx2::SvBaseLink&, _rLink, void )
{
    int nPos;
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
            m_xTbLinks->remove(nPos);
            int nToUnselect = m_xTbLinks->get_selected_index();
            InsertEntry(_rLink, nPos, true);
            if (nToUnselect != -1)
                m_xTbLinks->unselect(nToUnselect);
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

    if (pNewMgr)
    {
        // update has to be stopped before clear
        m_xTbLinks->freeze();
    }

    m_xTbLinks->clear();
    pLinkMgr = pNewMgr;

    if( pLinkMgr )
    {
        SvBaseLinks& rLnks = const_cast<SvBaseLinks&>(pLinkMgr->GetLinks());
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

        m_xTbLinks->thaw();

        if( !rLnks.empty() )
        {
            m_xTbLinks->set_cursor(0);
            m_xTbLinks->select(0);
            LinksSelectHdl( nullptr );
        }
    }
}

void SvBaseLinksDlg::InsertEntry(const SvBaseLink& rLink, int nPos, bool bSelect)
{
    OUString sFileNm, sLinkNm, sTypeNm, sFilter;

    sfx2::LinkManager::GetDisplayNames( &rLink, &sTypeNm, &sFileNm, &sLinkNm, &sFilter );

    auto nWidthPixel = m_xTbLinks->get_column_width(0);
    OUString aTxt = m_xVirDev->GetEllipsisString(sFileNm, nWidthPixel, DrawTextFlags::PathEllipsis);
    INetURLObject aPath( sFileNm, INetProtocol::File );
    OUString aFileName = aPath.getName();
    aFileName = INetURLObject::decode(aFileName, INetURLObject::DecodeMechanism::Unambiguous);

    if( aFileName.getLength() > aTxt.getLength() )
        aTxt = aFileName;
    else if (!aFileName.isEmpty() && aTxt.indexOf(aFileName, aTxt.getLength() - aFileName.getLength()) == -1)
        // filename not in string
        aTxt = aFileName;

    if (nPos == -1)
        nPos = m_xTbLinks->n_children();
    m_xTbLinks->insert(nPos);
    m_xTbLinks->set_text(nPos, aTxt, 0);
    m_xTbLinks->set_id(nPos, OUString::number(reinterpret_cast<sal_Int64>(&rLink)));
    if( OBJECT_CLIENT_GRF == rLink.GetObjType() )
        m_xTbLinks->set_text(nPos, sFilter, 1);
    else
        m_xTbLinks->set_text(nPos, sLinkNm, 1);
    m_xTbLinks->set_text(nPos, sTypeNm, 2);
    m_xTbLinks->set_text(nPos, ImplGetStateStr(rLink), 3);
    if (bSelect)
        m_xTbLinks->select(nPos);
}

SvBaseLink* SvBaseLinksDlg::GetSelEntry(int* pPos)
{
    int nPos = m_xTbLinks->get_selected_index();
    if (nPos != -1)
    {
        if (pPos)
            *pPos = nPos;
        return reinterpret_cast<SvBaseLink*>(m_xTbLinks->get_id(nPos).toInt64());
    }
    return nullptr;
}

void SvBaseLinksDlg::SetType(SvBaseLink& rLink,
                             int nSelPos,
                             SfxLinkUpdateMode nType)
{
    rLink.SetUpdateMode( nType );
    rLink.Update();
    m_xTbLinks->set_text(nSelPos, ImplGetStateStr(rLink), 3);
    if (pLinkMgr->GetPersist())
        pLinkMgr->GetPersist()->SetModified();
}

void SvBaseLinksDlg::SetActLink( SvBaseLink const * pLink )
{
    if( pLinkMgr )
    {
        const SvBaseLinks& rLnks = pLinkMgr->GetLinks();
        int nSelect = 0;
        for(const auto & rLinkRef : rLnks)
        {
            // #109573# only visible links have been inserted into the TreeListBox,
            // invisible ones have to be skipped here
            if( rLinkRef->IsVisible() )
            {
                if( pLink == rLinkRef.get() )
                {
                    m_xTbLinks->select(nSelect);
                    LinksSelectHdl( nullptr );
                    return ;
                }
                ++nSelect;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
