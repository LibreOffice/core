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

#include <custsdlg.hxx>

#include <strings.hrc>
#include <sdresid.hxx>

#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <cusshow.hxx>
#include <customshowlist.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <unotools/charclass.hxx>
#include <tools/debug.hxx>

// SdCustomShowDlg
SdCustomShowDlg::SdCustomShowDlg(weld::Window* pWindow, SdDrawDocument& rDrawDoc)
    : GenericDialogController(pWindow, "modules/simpress/ui/customslideshows.ui", "CustomSlideShows")
    , rDoc(rDrawDoc)
    , pCustomShowList(nullptr)
    , bModified(false)
    , m_xLbCustomShows(m_xBuilder->weld_tree_view("customshowlist"))
    , m_xCbxUseCustomShow(m_xBuilder->weld_check_button("usecustomshows"))
    , m_xBtnNew(m_xBuilder->weld_button("new"))
    , m_xBtnEdit(m_xBuilder->weld_button("edit"))
    , m_xBtnRemove(m_xBuilder->weld_button("delete"))
    , m_xBtnCopy(m_xBuilder->weld_button("copy"))
    , m_xBtnHelp(m_xBuilder->weld_button("help"))
    , m_xBtnStartShow(m_xBuilder->weld_button("startshow"))
    , m_xBtnOK(m_xBuilder->weld_button("ok"))
{
    m_xLbCustomShows->set_size_request(m_xLbCustomShows->get_approximate_digit_width() * 32,
                                       m_xLbCustomShows->get_height_rows(8));

    Link<weld::Button&,void> aLink( LINK( this, SdCustomShowDlg, ClickButtonHdl ) );
    m_xBtnNew->connect_clicked( aLink );
    m_xBtnEdit->connect_clicked( aLink );
    m_xBtnRemove->connect_clicked( aLink );
    m_xBtnCopy->connect_clicked( aLink );
    m_xCbxUseCustomShow->connect_clicked( aLink );
    m_xLbCustomShows->connect_changed( LINK( this, SdCustomShowDlg, SelectListBoxHdl ) );

    m_xBtnStartShow->connect_clicked( LINK( this, SdCustomShowDlg, StartShowHdl ) ); // for test

    // get CustomShow list of docs
    pCustomShowList = rDoc.GetCustomShowList();
    if( pCustomShowList )
    {
        long nPosToSelect = pCustomShowList->GetCurPos();
        // fill ListBox with CustomShows
        for( SdCustomShow* pCustomShow = pCustomShowList->First();
             pCustomShow != nullptr;
             pCustomShow = pCustomShowList->Next() )
        {
            m_xLbCustomShows->append_text(pCustomShow->GetName());
        }
        m_xLbCustomShows->select(nPosToSelect);
        pCustomShowList->Seek( nPosToSelect );
    }

    m_xCbxUseCustomShow->set_active(pCustomShowList && rDoc.getPresentationSettings().mbCustomShow);

    CheckState();
}

SdCustomShowDlg::~SdCustomShowDlg()
{
}

void SdCustomShowDlg::CheckState()
{
    int nPos = m_xLbCustomShows->get_selected_index();

    bool bEnable = nPos != -1;
    m_xBtnEdit->set_sensitive( bEnable );
    m_xBtnRemove->set_sensitive( bEnable );
    m_xBtnCopy->set_sensitive( bEnable );
    m_xCbxUseCustomShow->set_sensitive( bEnable );
    m_xBtnStartShow->set_sensitive(true);

    if (bEnable && pCustomShowList)
        pCustomShowList->Seek( nPos );
}

IMPL_LINK( SdCustomShowDlg, ClickButtonHdl, weld::Button&, r, void )
{
    SelectHdl(&r);
}

IMPL_LINK( SdCustomShowDlg, SelectListBoxHdl, weld::TreeView&, rListBox, void )
{
    SelectHdl(&rListBox);
}

void SdCustomShowDlg::SelectHdl(void const *p)
{
    // new CustomShow
    if (p == m_xBtnNew.get())
    {
        std::unique_ptr<SdCustomShow> pCustomShow;
        SdDefineCustomShowDlg aDlg(m_xDialog.get(), rDoc, pCustomShow);
        if (aDlg.run() == RET_OK)
        {
            if( pCustomShow )
            {
                if( !pCustomShowList )
                    pCustomShowList = rDoc.GetCustomShowList( true );

                SdCustomShow* pCustomShowTmp = pCustomShow.get();
                pCustomShowList->push_back( std::move(pCustomShow) );
                pCustomShowList->Last();
                m_xLbCustomShows->append_text( pCustomShowTmp->GetName() );
                m_xLbCustomShows->select_text( pCustomShowTmp->GetName() );
            }

            if (aDlg.IsModified())
                bModified = true;
        }
    }
    // edit CustomShow
    else if( p == m_xBtnEdit.get() )
    {
        int nPos = m_xLbCustomShows->get_selected_index();
        if (nPos != -1)
        {
            DBG_ASSERT( pCustomShowList, "pCustomShowList does not exist" );
            std::unique_ptr<SdCustomShow>& pCustomShow = (*pCustomShowList)[ nPos ];
            SdDefineCustomShowDlg aDlg(m_xDialog.get(), rDoc, pCustomShow);

            if (aDlg.run() == RET_OK)
            {
                pCustomShowList->Seek(nPos);
                m_xLbCustomShows->remove(nPos);
                m_xLbCustomShows->insert_text(nPos, pCustomShow->GetName());
                m_xLbCustomShows->select(nPos);
                if (aDlg.IsModified())
                    bModified = true;
            }
        }
    }
    // delete CustomShow
    else if( p == m_xBtnRemove.get() )
    {
        int nPos = m_xLbCustomShows->get_selected_index();
        if (nPos != -1)
        {
            pCustomShowList->erase( pCustomShowList->begin() + nPos );
            m_xLbCustomShows->remove(nPos);
            m_xLbCustomShows->select(nPos == 0 ? nPos : nPos - 1);
            bModified = true;
        }
    }
    // copy CustomShow
    else if( p == m_xBtnCopy.get() )
    {
        int nPos = m_xLbCustomShows->get_selected_index();
        if (nPos != -1)
        {
            std::unique_ptr<SdCustomShow> pShow(new SdCustomShow( *(*pCustomShowList)[nPos] ));
            OUString aStr( pShow->GetName() );
            OUString aStrCopy( SdResId( STR_COPY_CUSTOMSHOW ) );

            sal_Int32 nStrPos = aStr.indexOf( aStrCopy );
            sal_Int32 nNum = 1;
            if( nStrPos < 0 )
            {
                aStr = aStr + " ("  + aStrCopy + OUString::number( nNum ) + ")";
                nStrPos = aStr.indexOf( aStrCopy );
            }
            nStrPos = nStrPos + aStrCopy.getLength();
            // that we do not access into the nirvana (--> endless loop)
            if( nStrPos >= aStr.getLength() )
            {
                aStr = aStr + " " + OUString::number( nNum );
            }

            // check name...
            bool bDifferent = false;
            //long nPosToSelect = pCustomShowList->GetCurPos();
            while( !bDifferent )
            {
                bDifferent = true;
                for( SdCustomShow* pCustomShow = pCustomShowList->First();
                     pCustomShow != nullptr && bDifferent;
                     pCustomShow = pCustomShowList->Next() )
                {
                    if( aStr == pCustomShow->GetName() )
                        bDifferent = false;
                }
                if( !bDifferent )
                {
                    // replace number by a number increased by 1

                    const CharClass* pCharClass = rDoc.GetCharClass();
                    while( pCharClass->isDigit( aStr, nStrPos ) )
                        aStr = aStr.replaceAt( nStrPos, 1, "" );
                    aStr = aStr.copy( 0, nStrPos) + OUString::number( ++nNum ) + aStr.copy( nStrPos);
                }

            }
            //pCustomShowList->Seek( nPosToSelect );
            pShow->SetName( aStr );

            auto pShowTmp = pShow.get();
            pCustomShowList->push_back( std::move(pShow) );
            pCustomShowList->Last();
            m_xLbCustomShows->append_text(pShowTmp->GetName());
            m_xLbCustomShows->select_text(pShowTmp->GetName());

            bModified = true;
        }
    }
    else if( p == m_xLbCustomShows.get() )
    {
        int nPos = m_xLbCustomShows->get_selected_index();
        if (nPos != -1)
            pCustomShowList->Seek(nPos);

        bModified = true;
    }
    else if( p == m_xCbxUseCustomShow.get() )
    {
        bModified = true;
    }

    CheckState();
}

// StartShow-Hdl
IMPL_LINK_NOARG(SdCustomShowDlg, StartShowHdl, weld::Button&, void)
{
    m_xDialog->response(RET_YES);
}

// CheckState
bool SdCustomShowDlg::IsCustomShow() const
{
    return (m_xCbxUseCustomShow->get_sensitive() && m_xCbxUseCustomShow->get_active());
}

// SdDefineCustomShowDlg
SdDefineCustomShowDlg::SdDefineCustomShowDlg(weld::Window* pWindow, SdDrawDocument& rDrawDoc, std::unique_ptr<SdCustomShow>& rpCS)
    : GenericDialogController(pWindow, "modules/simpress/ui/definecustomslideshow.ui", "DefineCustomSlideShow")
    , rDoc(rDrawDoc)
    , rpCustomShow(rpCS)
    , bModified(false)
    , m_xEdtName(m_xBuilder->weld_entry("customname"))
    , m_xLbPages(m_xBuilder->weld_tree_view("pages"))
    , m_xBtnAdd(m_xBuilder->weld_button("add"))
    , m_xBtnRemove(m_xBuilder->weld_button("remove"))
    , m_xLbCustomPages(m_xBuilder->weld_tree_view("custompages"))
    , m_xBtnOK(m_xBuilder->weld_button("ok"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
    , m_xBtnHelp(m_xBuilder->weld_button("help"))
{
    Link<weld::Button&,void> aLink = LINK( this, SdDefineCustomShowDlg, ClickButtonHdl );
    m_xBtnAdd->connect_clicked( aLink );
    m_xBtnRemove->connect_clicked( aLink );
    m_xEdtName->connect_changed( LINK( this, SdDefineCustomShowDlg, ClickButtonEditHdl ) );
    m_xLbPages->connect_changed( LINK( this, SdDefineCustomShowDlg, ClickButtonHdl4 ) ); // because of status
    m_xLbCustomPages->connect_changed( LINK( this, SdDefineCustomShowDlg, ClickButtonHdl3 ) ); // because of status

    m_xBtnOK->connect_clicked( LINK( this, SdDefineCustomShowDlg, OKHdl ) );

    m_xLbPages->set_selection_mode(SelectionMode::Multiple);

    // shape 'em a bit
    m_xLbPages->set_size_request(m_xLbPages->get_approximate_digit_width() * 24, m_xLbPages->get_height_rows(10));
    m_xLbCustomPages->set_size_request(m_xLbPages->get_approximate_digit_width() * 24, m_xLbCustomPages->get_height_rows(10));

    // fill Listbox with page names of Docs
    for( long nPage = 0;
         nPage < rDoc.GetSdPageCount( PageKind::Standard );
         nPage++ )
    {
        SdPage* pPage = rDoc.GetSdPage( static_cast<sal_uInt16>(nPage), PageKind::Standard );
        m_xLbPages->append_text(pPage->GetName());
    }
    // aLbPages.SelectEntryPos( 0 );

    if( rpCustomShow )
    {
        aOldName = rpCustomShow->GetName();
        m_xEdtName->set_text( aOldName );

        // fill ListBox with CustomShow pages
        for( const auto& rpPage : rpCustomShow->PagesVector() )
        {
            m_xLbCustomPages->append(OUString::number(reinterpret_cast<sal_uInt64>(rpPage)), rpPage->GetName(), "");
        }
    }
    else
    {
        rpCustomShow.reset(new SdCustomShow);
        m_xEdtName->set_text( SdResId( STR_NEW_CUSTOMSHOW ) );
        m_xEdtName->select_region(0, -1);
        rpCustomShow->SetName( m_xEdtName->get_text() );
    }

    m_xBtnOK->set_sensitive( false );
    CheckState();
}

SdDefineCustomShowDlg::~SdDefineCustomShowDlg()
{
}

// CheckState
void SdDefineCustomShowDlg::CheckState()
{
    bool bPages = m_xLbPages->count_selected_rows() > 0;
    bool bCSPages = m_xLbCustomPages->get_selected_index() != -1;
    bool bCount = m_xLbCustomPages->n_children() > 0;

    m_xBtnOK->set_sensitive( bCount );
    m_xBtnAdd->set_sensitive( bPages );
    m_xBtnRemove->set_sensitive( bCSPages );
}

IMPL_LINK( SdDefineCustomShowDlg, ClickButtonHdl, weld::Button&, rWidget, void )
{
    ClickButtonHdl2(&rWidget);
}

IMPL_LINK( SdDefineCustomShowDlg, ClickButtonHdl3, weld::TreeView&, rWidget, void )
{
    ClickButtonHdl2(&rWidget);
}

IMPL_LINK( SdDefineCustomShowDlg, ClickButtonHdl4, weld::TreeView&, rListBox, void )
{
    ClickButtonHdl2(&rListBox);
}

IMPL_LINK( SdDefineCustomShowDlg, ClickButtonEditHdl, weld::Entry&, rEdit, void )
{
    ClickButtonHdl2(&rEdit);
}

// ButtonHdl()
void SdDefineCustomShowDlg::ClickButtonHdl2(void const * p)
{
    if( p == m_xBtnAdd.get() )
    {
        auto aRows = m_xLbPages->get_selected_rows();
        if (!aRows.empty())
        {
            int nPosCP = m_xLbCustomPages->get_selected_index();
            if (nPosCP != -1)
                ++nPosCP;

            for (auto i : aRows)
            {
                OUString aStr = m_xLbPages->get_text(i);
                SdPage* pPage = rDoc.GetSdPage(i, PageKind::Standard);
                OUString sId(OUString::number(reinterpret_cast<sal_uInt64>(pPage)));
                m_xLbCustomPages->insert(nPosCP, aStr, &sId, nullptr, nullptr);
                m_xLbCustomPages->select(nPosCP != -1 ? nPosCP : m_xLbCustomPages->n_children() - 1);

                if (nPosCP != -1)
                    ++nPosCP;
            }
            bModified = true;
        }
    }
    else if (p == m_xBtnRemove.get())
    {
        int nPos = m_xLbCustomPages->get_selected_index();
        if (nPos != -1)
        {
            m_xLbCustomPages->remove(nPos);
            m_xLbCustomPages->select(nPos == 0 ? nPos : nPos - 1);
            bModified = true;
        }
    }
    else if( p == m_xEdtName.get() )
    {
        bModified = true;
    }

    CheckState();
}

/**
 * Checks the page pointer of the Show since entries can be moved and copied
 * by TreeLB.
 */
void SdDefineCustomShowDlg::CheckCustomShow()
{
    bool bDifferent = false;

    // compare count
    size_t nCount = m_xLbCustomPages->n_children();
    if (rpCustomShow->PagesVector().size() != nCount)
    {
        rpCustomShow->PagesVector().clear();
        bDifferent = true;
    }

    // compare page pointer
    if( !bDifferent )
    {
        size_t i = 0;
        for (const auto& rpPage : rpCustomShow->PagesVector())
        {
            SdPage* pPage = reinterpret_cast<SdPage*>(m_xLbCustomPages->get_id(i).toUInt64());
            if (rpPage != pPage)
            {
                rpCustomShow->PagesVector().clear();
                bDifferent = true;
                break;
            }

            ++i;
        }
    }

    // set new page pointer
    if( bDifferent )
    {
        for (size_t i = 0; i < nCount; ++i)
        {
            SdPage* pPage = reinterpret_cast<SdPage*>(m_xLbCustomPages->get_id(i).toUInt64());
            rpCustomShow->PagesVector().push_back(pPage);
        }
        bModified = true;
    }

    // compare name and set name if necessary
    OUString aStr( m_xEdtName->get_text() );
    if( rpCustomShow->GetName() != aStr )
    {
        rpCustomShow->SetName( aStr );
        bModified = true;
    }
}

// OK-Hdl
IMPL_LINK_NOARG(SdDefineCustomShowDlg, OKHdl, weld::Button&, void)
{
    // check name...
    bool bDifferent = true;
    SdCustomShowList* pCustomShowList = rDoc.GetCustomShowList();
    if( pCustomShowList )
    {
        OUString aName( m_xEdtName->get_text() );
        SdCustomShow* pCustomShow;

        long nPosToSelect = pCustomShowList->GetCurPos();
        for( pCustomShow = pCustomShowList->First();
             pCustomShow != nullptr;
             pCustomShow = pCustomShowList->Next() )
        {
            if( aName == pCustomShow->GetName() && aName != aOldName )
                bDifferent = false;
        }
        pCustomShowList->Seek( nPosToSelect );
    }

    if( bDifferent )
    {
        CheckCustomShow();

        m_xDialog->response(RET_OK);
    }
    else
    {
        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(m_xDialog.get(),
                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                   SdResId(STR_WARN_NAME_DUPLICATE)));
        xWarn->run();
        m_xEdtName->grab_focus();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
